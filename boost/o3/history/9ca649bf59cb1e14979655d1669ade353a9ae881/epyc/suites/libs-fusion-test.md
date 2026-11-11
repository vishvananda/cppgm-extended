# libs/fusion/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| all.test | RUN | 0.399915 | 0.749300 | 1.037878 | 0.001593 | 0.001627 | 0.001523 |
| any.test | RUN | 0.383831 | 0.691406 | 0.992386 | 0.001746 | 0.001705 | 0.001884 |
| clear.test | RUN | 0.277726 | 0.649839 | 0.805446 | 0.001650 | 0.001840 | 0.001880 |
| copy.test | RUN | 0.240759 | 0.486460 | 0.654897 | 0.001677 | 0.001875 | 0.001658 |
| count.test | RUN | 0.276248 | 0.528354 | 0.726972 | 0.001842 | 0.001809 | 0.001844 |
| count_if.test | RUN | 0.257177 | 0.495890 | 0.656220 | 0.001672 | 0.001988 | 0.001601 |
| erase.test | RUN | 0.354122 | 0.879839 | 1.093045 | 0.001794 | 0.001982 | 0.001542 |
| erase_key.test | RUN | 0.344982 | 0.867291 | 1.031830 | 0.001404 | 0.001794 | 0.001725 |
| filter.test | RUN | 0.307720 | 0.731528 | 0.947579 | 0.001912 | 0.001932 | 0.001930 |
| filter_if.test | RUN | 0.344793 | 0.866585 | 1.076862 | 0.001702 | 0.001946 | 0.001916 |
| find.test | RUN | 0.310355 | 0.602383 | 0.839161 | 0.001597 | 0.001831 | 0.001576 |
| find_if.test | RUN | 0.297827 | 0.565418 | 0.772146 | 0.001672 | 0.001596 | 0.001745 |
| fold.test | RUN | 0.322980 | 0.680068 | 0.930448 | 0.001904 | 0.001943 | 0.001781 |
| for_each.test | RUN | 0.283155 | 0.619687 | 0.824478 | 0.001682 | 0.002062 | 0.001938 |
| insert.test | RUN | 0.400130 | 1.082261 | 1.299497 | 0.001630 | 0.001838 | 0.001599 |
| insert_range.test | RUN | 0.412592 | 1.088995 | 1.316537 | 0.001700 | 0.001566 | 0.001880 |
| iter_fold.test | RUN | 0.478090 | 1.052483 | 1.542737 | 0.001933 | 0.001901 | 0.001709 |
| move.test | RUN | 0.248583 | 0.503795 | 0.674150 | 0.001731 | 0.001725 | 0.001710 |
| none.test | RUN | 0.394863 | 0.736115 | 1.012803 | 0.001668 | 0.001745 | 0.001573 |
| pop_back.test | RUN | 0.382513 | 0.945851 | 1.175035 | 0.001640 | 0.001841 | 0.001807 |
| pop_front.test | RUN | 0.330120 | 0.783179 | 0.934814 | 0.001866 | 0.001849 | 0.001702 |
| push_back.test | RUN | 0.398218 | 1.131237 | 1.267933 | 0.001822 | 0.001940 | 0.001813 |
| push_front.test | RUN | 0.354223 | 0.917579 | 1.041303 | 0.001800 | 0.001874 | 0.001816 |
| remove.test | RUN | 0.387065 | 1.065701 | 1.256391 | 0.001633 | 0.001990 | 0.001765 |
| remove_if.test | RUN | 0.349518 | 0.868695 | 1.069689 | 0.001789 | 0.001719 | 0.001163 |
| replace.test | RUN | 0.344656 | 0.837918 | 0.986372 | 0.001695 | 0.001915 | 0.001814 |
| replace_if.test | RUN | 0.334422 | 0.832189 | 0.973006 | 0.001913 | 0.001887 | 0.001771 |
| reverse_fold.test | RUN | 0.529744 | 1.217833 | 1.714504 | 0.001801 | 0.001764 | 0.001672 |
| reverse_iter_fold.test | RUN | 0.543395 | 1.218484 | 1.806502 | 0.001537 | 0.001834 | 0.001881 |
| reverse.test | RUN | 0.334820 | 0.865838 | 1.019212 | 0.001771 | 0.001846 | 0.001703 |
| segmented_for_each.test | RUN | 0.559791 | 1.652822 | 2.076401 | 0.002602 | 0.002002 | 0.001666 |
| segmented_find.test | RUN | 0.359751 | 0.872014 | 1.163803 | 0.001711 | 0.001771 | 0.001568 |
| segmented_find_if.test | RUN | 0.378068 | 0.860479 | 1.164540 | 0.001857 | 0.001648 | 0.001677 |
| segmented_fold.test | RUN | 0.481441 | 1.146944 | 1.480687 | 0.001854 | 0.001866 | 0.001873 |
| transform.test | RUN | 0.353171 | 0.899556 | 1.084634 | 0.001619 | 0.001820 | 0.001879 |
| join.test | RUN | 0.271413 | 0.563694 | 0.748389 | 0.001472 | 0.001934 | 0.001820 |
| zip.test | RUN | 0.469939 | 1.044666 | 1.480489 | 0.001887 | 0.001928 | 0.001787 |
| zip2.test | RUN | 0.457793 | 1.028208 | 1.393086 | 0.001768 | 0.001760 | 0.001880 |
| zip_ignore.test | RUN | 0.366370 | 0.694538 | 0.926550 | 0.001610 | 0.001750 | 0.001548 |
| flatten.test | RUN | 0.329392 | 0.778295 | 0.915062 | 0.001570 | 0.001882 | 0.001566 |
| ticket-5490.test | COMPILE | 0.177169 | 0.332768 | 0.401923 | — | — | — |
| as_deque.test | RUN | 0.317371 | 0.730336 | 1.014682 | 0.001598 | 0.001864 | 0.001544 |
| as_list.test | RUN | 0.361877 | 0.889266 | 1.069345 | 0.001641 | 0.001895 | 0.001500 |
| as_map.test | RUN | 0.357784 | 0.876985 | 1.078776 | 0.001399 | 0.001626 | 0.001646 |
| as_map_assoc.test | RUN | 0.352071 | 0.825154 | 1.033596 | 0.001626 | 0.001860 | 0.001457 |
| as_set.test | RUN | 0.410940 | 1.025940 | 1.259259 | 0.001642 | 0.001906 | 0.001477 |
| as_vector.test | RUN | 0.375787 | 0.926778 | 1.115732 | 0.001501 | 0.001758 | 0.001686 |
| boost_tuple.test | RUN | 0.370189 | 0.874570 | 1.056414 | 0.001434 | 0.001653 | 0.001644 |
| boost_tuple_iterator.test | RUN | 0.266434 | 0.532080 | 0.664056 | 0.001754 | 0.001723 | 0.001624 |
| cons.test | RUN | 0.445633 | 1.010807 | 1.293259 | 0.001572 | 0.001674 | 0.001577 |
| convert_boost_tuple.test | RUN | 0.291286 | 0.769442 | 0.844365 | 0.001700 | 0.001782 | 0.001637 |
| convert_deque.test | RUN | 0.296377 | 0.767460 | 0.856813 | 0.001948 | 0.001550 | 0.001419 |
| convert_list.test | RUN | 0.299875 | 0.781772 | 0.846929 | 0.001323 | 0.001588 | 0.001548 |
| convert_std_pair.test | RUN | 0.310052 | 0.778037 | 0.866699 | 0.001821 | 0.001716 | 0.001508 |
| convert_std_tuple.test | RUN | 0.292765 | 0.758191 | 0.848487 | 0.001708 | 0.001842 | 0.001486 |
| convert_vector.test | RUN | 0.292078 | 0.762807 | 0.845907 | 0.001552 | 0.001584 | 0.001632 |
| filter_view.test | RUN | 0.337132 | 0.803083 | 1.008388 | 0.001486 | 0.001606 | 0.001939 |
| hash.test | RUN | 0.355068 | 0.759795 | 0.976818 | 0.001839 | 0.001722 | 0.001594 |
| io.test | RUN | 0.353110 | 1.066272 | 1.141288 | 0.001716 | 0.002120 | 0.002047 |
| iterator_range.test | RUN | 0.325869 | 0.792043 | 0.964863 | 0.001632 | 0.001774 | 0.001546 |
| joint_view.test | RUN | 0.422502 | 1.320742 | 1.465826 | 0.001864 | 0.002048 | 0.001874 |
| list_comparison.test | RUN | 0.260821 | 0.547274 | 0.684371 | 0.002209 | 0.001821 | 0.001842 |
| list_construction.test | RUN | 0.255883 | 0.546374 | 0.686808 | 0.001596 | 0.001782 | 0.001584 |
| list_copy.test | RUN | 0.378096 | 0.946033 | 1.157068 | 0.001574 | 0.001920 | 0.001704 |
| list_iterator.test | RUN | 0.258294 | 0.515761 | 0.665037 | 0.001579 | 0.001831 | 0.001712 |
| list_hash.test | RUN | 0.352675 | 1.035583 | 0.961968 | 0.001689 | 0.001996 | 0.001874 |
| list_make.test | RUN | 0.265804 | 0.577763 | 0.732659 | 0.001948 | 0.001731 | 0.001742 |
| list_misc.test | RUN | 0.367270 | 0.686212 | 0.880261 | 0.001809 | 0.001949 | 0.001812 |
| list_mutate.test | RUN | 0.264521 | 0.450755 | 0.602164 | 0.001766 | 0.001578 | 0.001826 |
| list_nest.test | RUN | 0.483795 | 1.307725 | 1.659126 | 0.001936 | 0.001490 | 0.001784 |
| list_tie.test | RUN | 0.256172 | 0.516113 | 0.683280 | 0.001788 | 0.001806 | 0.001784 |
| list_value_at.test | RUN | 0.259728 | 0.494334 | 0.631119 | 0.001843 | 0.001745 | 0.001771 |
| deque_comparison.test | RUN | 0.260828 | 0.568878 | 0.696974 | 0.001526 | 0.001670 | 0.001656 |
| deque_construction.test | RUN | 0.258517 | 0.508246 | 0.723350 | 0.001902 | 0.001926 | 0.001686 |
| deque_copy.test | RUN | 0.369651 | 0.913254 | 1.081211 | 0.001709 | 0.001714 | 0.001700 |
| deque_iterator.test | RUN | 0.254241 | 0.540171 | 0.695637 | 0.001644 | 0.001890 | 0.001549 |
| deque_hash.test | RUN | 0.347358 | 0.988098 | 0.953500 | 0.001539 | 0.001854 | 0.002148 |
| deque_is_constructible.test | COMPILE | 0.145938 | 0.246884 | 0.318998 | — | — | — |
| deque_make.test | RUN | 0.265050 | 0.545977 | 0.714152 | 0.001828 | 0.001954 | 0.001804 |
| deque_misc.test | RUN | 0.343708 | 0.651552 | 0.885141 | 0.001636 | 0.001853 | 0.001379 |
| deque_move.test | RUN | 0.266492 | 0.567173 | 0.706704 | 0.001673 | 0.001768 | 0.001746 |
| deque_mutate.test | RUN | 0.242658 | 0.459172 | 0.591592 | 0.001767 | 0.001728 | 0.001834 |
| deque_nest.test | RUN | 0.708543 | 2.044222 | 2.662850 | 0.001799 | 0.001880 | 0.001745 |
| deque_tie.test | RUN | 0.252711 | 0.501641 | 0.671554 | 0.001672 | 0.001892 | 0.001576 |
| deque_value_at.test | RUN | 0.248292 | 0.453833 | 0.619521 | 0.001587 | 0.001559 | 0.001787 |
| front_extended_deque.test | RUN | 0.356482 | 0.689610 | 0.933905 | 0.001846 | 0.001865 | 0.001844 |
| back_extended_deque.test | RUN | 0.342734 | 0.687252 | 0.966795 | 0.001555 | 0.001855 | 0.001813 |
| make_list.test | RUN | 0.266844 | 0.591200 | 0.720347 | 0.001756 | 0.001827 | 0.001882 |
| make_vector.test | RUN | 0.283896 | 0.555893 | 0.710413 | 0.001654 | 0.001806 | 0.001672 |
| map.test | RUN | 0.339794 | 0.851671 | 1.036726 | 0.001729 | 0.001804 | 0.001733 |
| map_comparison.test | RUN | 0.270894 | 0.540317 | 0.698640 | 0.001589 | 0.001884 | 0.001697 |
| map_construction.test | RUN | 0.266993 | 0.533588 | 0.706432 | 0.001549 | 0.001732 | 0.001652 |
| map_copy.test | RUN | 0.324259 | 0.676847 | 0.879768 | 0.001798 | 0.001902 | 0.001777 |
| map_misc.test | RUN | 0.328464 | 0.675705 | 0.875038 | 0.001510 | 0.001700 | 0.001751 |
| map_move.test | RUN | 0.260760 | 0.558323 | 0.708593 | 0.001684 | 0.001516 | 0.001418 |
| map_mutate.test | RUN | 0.242879 | 0.469631 | 0.620539 | 0.001594 | 0.001670 | 0.001040 |
| map_tie.test | RUN | 0.249142 | 0.463308 | 0.631944 | 0.001666 | 0.001744 | 0.001650 |
| nil.test | RUN | 0.173507 | 0.319247 | 0.435854 | 0.001877 | 0.001904 | 0.001774 |
| nview.test | RUN | 0.330437 | 0.661785 | 0.884557 | 0.001788 | 0.001912 | 0.001871 |
| reverse_view.test | RUN | 0.399499 | 1.104953 | 1.344639 | 0.001821 | 0.001988 | 0.001878 |
| segmented_iterator_range.test | RUN | 0.485705 | 1.352075 | 1.685835 | 0.001834 | 0.001759 | 0.001706 |
| set.test | RUN | 0.323558 | 0.756384 | 0.935754 | 0.001688 | 0.002058 | 0.001820 |
| single_view.test | RUN | 0.293169 | 0.669898 | 0.837519 | 0.001819 | 0.001940 | 0.001804 |
| std_pair.test | RUN | 0.372908 | 0.868391 | 1.053549 | 0.001837 | 0.001932 | 0.001611 |
| boost_array.test | RUN | 0.318133 | 0.612823 | 0.834276 | 0.001775 | 0.001928 | 0.001782 |
| array.test | RUN | 0.310820 | 0.615660 | 0.780290 | 0.001575 | 0.001824 | 0.001728 |
| std_array.test | RUN | 0.312641 | 0.607280 | 0.791694 | 0.001627 | 0.001642 | 0.001672 |
| tuple_comparison.test | RUN | 0.284448 | 0.574065 | 0.697368 | 0.001851 | 0.001886 | 0.001489 |
| tuple_construction.test | RUN | 0.280690 | 0.569314 | 0.759689 | 0.001798 | 0.001857 | 0.001510 |
| tuple_conversion.test | RUN | 0.598093 | 1.753959 | 2.476770 | 0.001600 | 0.001757 | 0.001739 |
| tuple_copy.test | RUN | 0.405195 | 1.015166 | 1.304926 | 0.001580 | 0.001849 | 0.001472 |
| tuple_element.test | RUN | 0.262510 | 0.546599 | 0.682551 | 0.001752 | 0.001852 | 0.001762 |
| tuple_make.test | RUN | 0.285091 | 0.607694 | 0.748692 | 0.001710 | 0.001966 | 0.001608 |
| tuple_misc.test | RUN | 0.355642 | 0.726193 | 0.983387 | 0.001447 | 0.001766 | 0.001629 |
| tuple_mutate.test | RUN | 0.254015 | 0.487728 | 0.672574 | 0.001632 | 0.001628 | 0.001652 |
| tuple_nest.test | RUN | 1.007566 | 3.564881 | 4.535073 | 0.001633 | 0.001837 | 0.001843 |
| tuple_hash.test | RUN | 0.364633 | 1.026021 | 1.004034 | 0.001813 | 0.001969 | 0.001717 |
| tuple_tie.test | RUN | 0.265655 | 0.527765 | 0.746503 | 0.001676 | 0.001970 | 0.001652 |
| tuple_traits__maybe_variadic.test | RUN | 0.296228 | 0.658355 | 0.795134 | 0.001822 | 0.001798 | 0.001876 |
| tuple_traits__no_variadic.test | RUN | 0.314850 | 0.680996 | 0.827442 | 0.001685 | 0.001923 | 0.001860 |
| transform_view.test | RUN | 0.409933 | 1.099711 | 1.310490 | 0.001744 | 0.001833 | 0.001522 |
| identity_view.test | RUN | 0.384012 | 1.030982 | 1.246271 | 0.001797 | 0.001597 | 0.001903 |
| vector_comparison.test | RUN | 0.268425 | 0.582573 | 0.703283 | 0.001720 | 0.001888 | 0.002107 |
| vector_construction.test | RUN | 0.262226 | 0.541249 | 0.668158 | 0.001412 | 0.001784 | 0.001881 |
| vector_conversion.test | RUN | 0.705931 | 2.346571 | 3.124180 | 0.001702 | 0.001769 | 0.001716 |
| vector_copy.test | RUN | 0.379056 | 1.033447 | 1.240503 | 0.001510 | 0.001658 | 0.001714 |
| vector_iterator.test | RUN | 0.251506 | 0.538660 | 0.710780 | 0.001688 | 0.001685 | 0.001465 |
| vector_make.test | RUN | 0.270013 | 0.572514 | 0.750025 | 0.001595 | 0.001736 | 0.001584 |
| vector_misc.test | RUN | 0.336709 | 0.741700 | 0.901238 | 0.001638 | 0.001529 | 0.001730 |
| vector_move.test | RUN | 0.265074 | 0.563447 | 0.705110 | 0.001899 | 0.001321 | 0.001690 |
| vector_mutate.test | RUN | 0.238437 | 0.462688 | 0.644005 | 0.001510 | 0.001936 | 0.001671 |
| vector_n.test | RUN | 0.287159 | 0.600433 | 0.834019 | 0.001829 | 0.001935 | 0.001744 |
| vector_nest.test | RUN | 1.011248 | 3.760452 | 4.446100 | 0.001786 | 0.001684 | 0.001800 |
| vector_hash.test | RUN | 0.341690 | 0.949372 | 0.935676 | 0.001639 | 0.001810 | 0.001888 |
| vector_tie.test | RUN | 0.256237 | 0.511276 | 0.656808 | 0.001731 | 0.001563 | 0.001716 |
| vector_traits__maybe_variadic.test | RUN | 0.266831 | 0.626522 | 0.684566 | 0.001694 | 0.001958 | 0.001727 |
| vector_traits__no_variadic.test | RUN | 0.297142 | 0.633346 | 0.714268 | 0.001622 | 0.001550 | 0.001834 |
| vector_value_at.test | RUN | 0.242317 | 0.488292 | 0.672030 | 0.001615 | 0.001637 | 0.001618 |
| zip_view.test | RUN | 0.493022 | 1.188766 | 1.574510 | 0.001871 | 0.001802 | 0.001347 |
| zip_view2.test | RUN | 0.410882 | 0.862582 | 1.243010 | 0.001571 | 0.001751 | 0.001478 |
| zip_view_ignore.test | RUN | 0.370363 | 0.803210 | 1.095507 | 0.001711 | 0.001536 | 0.001485 |
| repetitive_view.test | RUN | 0.608584 | 1.518706 | 2.083993 | 0.001597 | 0.001895 | 0.001872 |
| deduce_sequence.test | RUN | 0.308765 | 0.630988 | 0.840951 | 0.001835 | 0.001750 | 0.001836 |
| adapt_adt_named.test | RUN | 0.363800 | 0.912778 | 1.064171 | 0.001550 | 0.001812 | 0.001828 |
| adapt_adt_named_empty.test | RUN | 0.326069 | 0.712632 | 0.883936 | 0.001671 | 0.001761 | 0.001798 |
| adapt_adt.test | RUN | 0.395821 | 1.154792 | 1.297881 | 0.001614 | 0.001965 | 0.001670 |
| adapt_adt_empty.test | RUN | 0.315225 | 0.712211 | 0.867680 | 0.001781 | 0.001738 | 0.001722 |
| adapt_assoc_adt_named.test | RUN | 0.405901 | 0.920645 | 1.084853 | 0.001722 | 0.001973 | 0.001719 |
| adapt_assoc_adt_named_empty.test | RUN | 0.321337 | 0.709847 | 0.868679 | 0.001816 | 0.001839 | 0.001712 |
| adapt_assoc_adt.test | RUN | 0.404712 | 0.969507 | 1.178716 | 0.001661 | 0.001914 | 0.001828 |
| adapt_assoc_adt_empty.test | RUN | 0.322153 | 0.710167 | 0.864767 | 0.002404 | 0.001725 | 0.001689 |
| adapt_assoc_struct_named.test | RUN | 0.405051 | 0.907442 | 1.093072 | 0.001505 | 0.001751 | 0.001837 |
| adapt_assoc_struct_named_empty.test | RUN | 0.325119 | 0.710881 | 0.861244 | 0.001765 | 0.002022 | 0.001824 |
| adapt_assoc_struct.test | RUN | 0.365370 | 0.873437 | 1.017379 | 0.001700 | 0.001700 | 0.001591 |
| adapt_assoc_struct_empty.test | RUN | 0.307136 | 0.710162 | 0.887022 | 0.001709 | 0.001881 | 0.001568 |
| adapt_assoc_tpl_adt.test | RUN | 0.429438 | 0.995357 | 1.212443 | 0.001501 | 0.001975 | 0.001793 |
| adapt_assoc_tpl_adt_empty.test | RUN | 0.329287 | 0.713430 | 0.856218 | 0.001659 | 0.001975 | 0.001811 |
| adapt_assoc_tpl_struct.test | RUN | 0.355636 | 0.836650 | 0.981097 | 0.001589 | 0.001776 | 0.001684 |
| adapt_assoc_tpl_struct_empty.test | RUN | 0.320980 | 0.719621 | 0.870876 | 0.001786 | 0.001862 | 0.001612 |
| adapt_struct_named.test | RUN | 0.363669 | 0.881090 | 1.004188 | 0.001756 | 0.001752 | 0.001814 |
| adapt_struct_named_empty.test | RUN | 0.330522 | 0.699722 | 0.843913 | 0.001661 | 0.001682 | 0.001641 |
| adapt_struct.test | RUN | 0.386477 | 0.991936 | 1.156808 | 0.001977 | 0.001942 | 0.001755 |
| adapt_struct_empty.test | RUN | 0.316456 | 0.726282 | 0.849342 | 0.001812 | 0.001903 | 0.001816 |
| adapt_tpl_adt.test | RUN | 0.399693 | 1.137006 | 1.170779 | 0.001551 | 0.001810 | 0.001916 |
| adapt_tpl_adt_empty.test | RUN | 0.322075 | 0.713128 | 0.890358 | 0.001741 | 0.001910 | 0.001864 |
| adapt_tpl_struct.test | RUN | 0.333891 | 0.791714 | 0.936095 | 0.001770 | 0.001762 | 0.001625 |
| adapt_tpl_struct_empty.test | RUN | 0.320032 | 0.724862 | 0.869862 | 0.001582 | 0.001630 | 0.001692 |
| adt_attribute_proxy.test | RUN | 0.359669 | 0.703073 | 0.920885 | 0.001756 | 0.001931 | 0.001784 |
| define_struct.test | RUN | 0.418797 | 0.981930 | 1.179063 | 0.001758 | 0.001903 | 0.001815 |
| define_struct_empty.test | RUN | 0.376261 | 0.793267 | 0.975540 | 0.001570 | 0.001844 | 0.001744 |
| define_struct_move.test | RUN | 0.256928 | 0.459740 | 0.608666 | 0.001670 | 0.001956 | 0.001815 |
| define_struct_inline.test | RUN | 0.439331 | 1.162244 | 1.350896 | 0.001819 | 0.001850 | 0.001889 |
| define_struct_inline_empty.test | RUN | 0.368699 | 0.792315 | 1.004031 | 0.001287 | 0.001703 | 0.001874 |
| define_struct_inline_move.test | RUN | 0.258234 | 0.473891 | 0.611104 | 0.001630 | 0.001569 | 0.001563 |
| define_assoc_struct.test | RUN | 0.415822 | 0.920832 | 1.119955 | 0.001858 | 0.001854 | 0.001809 |
| define_assoc_struct_empty.test | RUN | 0.374747 | 0.811550 | 0.965109 | 0.001710 | 0.001928 | 0.001886 |
| define_assoc_struct_move.test | RUN | 0.246056 | 0.457815 | 0.613696 | 0.001774 | 0.001951 | 0.001741 |
| define_tpl_struct.test | RUN | 0.413175 | 0.941775 | 1.137363 | 0.001731 | 0.001810 | 0.001630 |
| define_tpl_struct_empty.test | RUN | 0.376777 | 0.818142 | 1.001552 | 0.001861 | 0.001852 | 0.001458 |
| define_tpl_struct_move.test | RUN | 0.256277 | 0.457912 | 0.604530 | 0.001645 | 0.001758 | 0.001450 |
| define_tpl_struct_inline.test | RUN | 0.428624 | 1.090252 | 1.255544 | 0.004201 | 0.001641 | 0.001714 |
| define_tpl_struct_inline_empty.test | RUN | 0.383775 | 0.793327 | 1.006806 | 0.002541 | 0.001726 | 0.001871 |
| define_tpl_struct_inline_move.test | RUN | 0.267870 | 0.483061 | 0.623539 | 0.001525 | 0.001872 | 0.001706 |
| define_assoc_tpl_struct.test | RUN | 0.440330 | 0.949558 | 1.166374 | 0.001857 | 0.002021 | 0.001836 |
| define_assoc_tpl_struct_empty.test | RUN | 0.386997 | 0.803638 | 1.010799 | 0.001634 | 0.001951 | 0.001514 |
| define_assoc_tpl_struct_move.test | RUN | 0.255331 | 0.471882 | 0.595585 | 0.001664 | 0.001674 | 0.001646 |
| std_tuple.test | RUN | 0.254735 | 0.517823 | 0.653181 | 0.001522 | 0.001792 | 0.001510 |
| std_tuple_iterator.test | RUN | 0.256959 | 0.540720 | 0.675513 | 0.001718 | 0.001853 | 0.001622 |
| ref_vector.test | RUN | 0.294277 | 0.549462 | 0.755671 | 0.001754 | 0.001928 | 0.001912 |
| flatten_view.test | RUN | 0.307589 | 0.684312 | 0.842885 | 0.001767 | 0.001814 | 0.001738 |
| github-159.test | COMPILE | 0.143752 | 0.255692 | 0.299036 | — | — | — |
| github-176.test | RUN | 0.291130 | 0.611714 | 0.715529 | 0.001707 | 0.001958 | 0.001726 |
| size.test | COMPILE | 0.244993 | 0.437821 | 0.595094 | — | — | — |
| fused.test | RUN | 0.334835 | 0.634491 | 0.847710 | 0.001649 | 0.001987 | 0.001784 |
| fused_function_object.test | RUN | 0.255419 | 0.490226 | 0.644686 | 0.001517 | 0.001721 | 0.001727 |
| fused_procedure.test | RUN | 0.328150 | 0.638281 | 0.806152 | 0.001547 | 0.001741 | 0.001595 |
| unfused.test | RUN | 0.272366 | 0.505409 | 0.664163 | 0.001716 | 0.001901 | 0.001419 |
| unfused_typed.test | RUN | 0.262825 | 0.550915 | 0.677883 | 0.001702 | 0.001710 | 0.001814 |
| make_fused.test | RUN | 0.337587 | 0.639975 | 0.814694 | 0.001187 | 0.001775 | 0.001730 |
| make_fused_function_object.test | RUN | 0.263343 | 0.501479 | 0.672161 | 0.001714 | 0.001774 | 0.001372 |
| make_fused_procedure.test | RUN | 0.328794 | 0.627575 | 0.823635 | 0.001848 | 0.001882 | 0.001734 |
| make_unfused.test | RUN | 0.261421 | 0.512422 | 0.650645 | 0.001820 | 0.001784 | 0.001784 |
| invoke.test | RUN | 0.622193 | 1.804027 | 2.225392 | 0.001821 | 0.001677 | 0.001664 |
| invoke_function_object.test | RUN | 0.322029 | 0.657813 | 0.842300 | 0.001815 | 0.001765 | 0.001721 |
| invoke_procedure.test | RUN | 0.513566 | 1.328333 | 1.658118 | 0.001596 | 0.001848 | 0.001585 |
| swap.test | RUN | 0.393680 | 0.823866 | 1.102963 | 0.001683 | 0.001952 | 0.001780 |
| is_sequence.test | COMPILE | 0.076912 | 0.120233 | 0.138160 | — | — | — |
| is_view.test | COMPILE | 0.075788 | 0.122520 | 0.139444 | — | — | — |
| pair_deque.test | COMPILE | 0.147915 | 0.262643 | 0.342791 | — | — | — |
| pair_list.test | COMPILE | 0.143172 | 0.258092 | 0.345455 | — | — | — |
| pair_map.test | COMPILE | 0.150699 | 0.267160 | 0.324237 | — | — | — |
| pair_set.test | COMPILE | 0.153365 | 0.271676 | 0.327247 | — | — | — |
| pair_vector.test | COMPILE | 0.149320 | 0.262742 | 0.306459 | — | — | — |
| pair_nest.test | COMPILE | 0.057528 | 0.111513 | 0.123358 | — | — | — |
| index_sequence.test | COMPILE | 0.056903 | 0.077529 | 0.096317 | — | — | — |
| and.test | COMPILE | 0.045361 | 0.060920 | 0.063024 | — | — | — |
| tag_of.test | COMPILE | 0.073632 | 0.113838 | 0.128853 | — | — | — |
| unused.test | COMPILE | 0.034765 | 0.075383 | 0.078373 | — | — | — |
| tag_of_fallback.test | COMPILE | 0.065763 | 0.103309 | 0.135413 | — | — | — |
| make_pair_r-value.test | COMPILE | 0.054635 | 0.090348 | 0.126888 | — | — | — |
| quick.test | RUN | 0.194142 | 0.359748 | 0.399974 | 0.000524 | 0.001351 | 0.001245 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.000× | 0.998×, 1.001× | 0.995× |
| clang | 0.998× | 1.001×, 0.995× | 0.996× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adapt_adt.test | 10 | 0.001079s | 0.001054s | 0.001054s | byte equality observed |
| adapt_adt_empty.test | 10 | 0.001058s | 0.001056s | 0.001054s | byte equality observed |
| adapt_adt_named.test | 10 | 0.001077s | 0.001049s | 0.001088s | byte equality observed |
| adapt_adt_named_empty.test | 10 | 0.001053s | 0.001037s | 0.001070s | byte equality observed |
| adapt_assoc_adt.test | 10 | 0.001052s | 0.001051s | 0.001053s | byte equality observed |
| adapt_assoc_adt_empty.test | 10 | 0.001037s | 0.001057s | 0.001038s | byte equality observed |
| adapt_assoc_adt_named.test | 10 | 0.001058s | 0.001078s | 0.001032s | byte equality observed |
| adapt_assoc_adt_named_empty.test | 10 | 0.001049s | 0.001053s | 0.001030s | byte equality observed |
| adapt_assoc_struct.test | 10 | 0.001088s | 0.001086s | 0.001070s | byte equality observed |
| adapt_assoc_struct_empty.test | 10 | 0.001058s | 0.001051s | 0.001050s | byte equality observed |
| adapt_assoc_struct_named.test | 10 | 0.001074s | 0.001045s | 0.001064s | byte equality observed |
| adapt_assoc_struct_named_empty.test | 10 | 0.001056s | 0.001049s | 0.001021s | byte equality observed |
| adapt_assoc_tpl_adt.test | 10 | 0.001060s | 0.001062s | 0.001044s | byte equality observed |
| adapt_assoc_tpl_adt_empty.test | 10 | 0.001076s | 0.001058s | 0.001036s | byte equality observed |
| adapt_assoc_tpl_struct.test | 10 | 0.001071s | 0.001073s | 0.001070s | byte equality observed |
| adapt_assoc_tpl_struct_empty.test | 10 | 0.001067s | 0.001039s | 0.001034s | byte equality observed |
| adapt_struct.test | 10 | 0.001070s | 0.001055s | 0.001064s | byte equality observed |
| adapt_struct_empty.test | 10 | 0.001052s | 0.001049s | 0.001053s | byte equality observed |
| adapt_struct_named.test | 10 | 0.001073s | 0.001069s | 0.001051s | byte equality observed |
| adapt_struct_named_empty.test | 10 | 0.001042s | 0.001052s | 0.001066s | byte equality observed |
| adapt_tpl_adt.test | 10 | 0.001057s | 0.001060s | 0.001058s | byte equality observed |
| adapt_tpl_adt_empty.test | 10 | 0.001059s | 0.001031s | 0.001050s | byte equality observed |
| adapt_tpl_struct.test | 10 | 0.001053s | 0.001065s | 0.001083s | byte equality observed |
| adapt_tpl_struct_empty.test | 10 | 0.001051s | 0.001044s | 0.001056s | byte equality observed |
| adt_attribute_proxy.test | 10 | 0.001017s | 0.001030s | 0.001025s | byte equality observed |
| all.test | 10 | 0.001065s | 0.001054s | 0.001047s | byte equality observed |
| any.test | 10 | 0.001032s | 0.001040s | 0.001016s | byte equality observed |
| array.test | 10 | 0.001054s | 0.001049s | 0.001016s | byte equality observed |
| as_deque.test | 10 | 0.001039s | 0.001019s | 0.001036s | byte equality observed |
| as_list.test | 10 | 0.001102s | 0.001082s | 0.001061s | byte equality observed |
| as_map.test | 10 | 0.001057s | 0.001055s | 0.001067s | byte equality observed |
| as_map_assoc.test | 10 | 0.001036s | 0.001053s | 0.001105s | byte equality observed |
| as_set.test | 10 | 0.001061s | 0.001058s | 0.001099s | byte equality observed |
| as_vector.test | 10 | 0.001056s | 0.001077s | 0.001101s | byte equality observed |
| back_extended_deque.test | 10 | 0.001021s | 0.001008s | 0.001055s | byte equality observed |
| boost_array.test | 10 | 0.001023s | 0.001047s | 0.001066s | byte equality observed |
| boost_tuple.test | 10 | 0.001077s | 0.001067s | 0.001069s | byte equality observed |
| boost_tuple_iterator.test | 10 | 0.001032s | 0.001059s | 0.001072s | byte equality observed |
| clear.test | 10 | 0.001068s | 0.001078s | 0.001041s | byte equality observed |
| cons.test | 10 | 0.001077s | 0.001060s | 0.001046s | byte equality observed |
| convert_boost_tuple.test | 10 | 0.001047s | 0.001007s | 0.001038s | byte equality observed |
| convert_deque.test | 10 | 0.001045s | 0.001048s | 0.001008s | byte equality observed |
| convert_list.test | 10 | 0.001045s | 0.001055s | 0.000980s | byte equality observed |
| convert_std_pair.test | 10 | 0.001043s | 0.001042s | 0.001018s | byte equality observed |
| convert_std_tuple.test | 10 | 0.001054s | 0.001081s | 0.001007s | byte equality observed |
| convert_vector.test | 10 | 0.001030s | 0.001068s | 0.001020s | byte equality observed |
| copy.test | 10 | 0.001025s | 0.001027s | 0.001022s | byte equality observed |
| count.test | 10 | 0.001058s | 0.001031s | 0.001015s | byte equality observed |
| count_if.test | 10 | 0.001057s | 0.001037s | 0.001110s | byte equality observed |
| deduce_sequence.test | 10 | 0.001066s | 0.001041s | 0.001027s | byte equality observed |
| define_assoc_struct.test | 10 | 0.001069s | 0.001069s | 0.001061s | byte equality observed |
| define_assoc_struct_empty.test | 10 | 0.001053s | 0.001075s | 0.001028s | byte equality observed |
| define_assoc_struct_move.test | 10 | 0.001046s | 0.001030s | 0.001020s | byte equality observed |
| define_assoc_tpl_struct.test | 10 | 0.001066s | 0.001065s | 0.001062s | byte equality observed |
| define_assoc_tpl_struct_empty.test | 10 | 0.001060s | 0.001057s | 0.001048s | byte equality observed |
| define_assoc_tpl_struct_move.test | 10 | 0.001015s | 0.001045s | 0.001030s | byte equality observed |
| define_struct.test | 10 | 0.001064s | 0.001054s | 0.001030s | byte equality observed |
| define_struct_empty.test | 10 | 0.001043s | 0.001048s | 0.001041s | byte equality observed |
| define_struct_inline.test | 10 | 0.001076s | 0.001061s | 0.001071s | byte equality observed |
| define_struct_inline_empty.test | 10 | 0.001055s | 0.001052s | 0.001079s | byte equality observed |
| define_struct_inline_move.test | 10 | 0.001014s | 0.001042s | 0.000997s | byte equality observed |
| define_struct_move.test | 10 | 0.001022s | 0.001051s | 0.001054s | byte equality observed |
| define_tpl_struct.test | 10 | 0.001051s | 0.001050s | 0.001062s | byte equality observed |
| define_tpl_struct_empty.test | 10 | 0.001051s | 0.001039s | 0.001066s | byte equality observed |
| define_tpl_struct_inline.test | 10 | 0.001054s | 0.001082s | 0.001073s | byte equality observed |
| define_tpl_struct_inline_empty.test | 10 | 0.001044s | 0.001068s | 0.001057s | byte equality observed |
| define_tpl_struct_inline_move.test | 10 | 0.001042s | 0.001017s | 0.001018s | byte equality observed |
| define_tpl_struct_move.test | 10 | 0.001035s | 0.001013s | 0.001000s | byte equality observed |
| deque_comparison.test | 10 | 0.001041s | 0.001045s | 0.001030s | byte equality observed |
| deque_construction.test | 10 | 0.001030s | 0.001027s | 0.001050s | byte equality observed |
| deque_copy.test | 10 | 0.001022s | 0.001021s | 0.001043s | byte equality observed |
| deque_hash.test | 10 | 0.001066s | 0.001073s | 0.001028s | byte equality observed |
| deque_iterator.test | 10 | 0.001026s | 0.001038s | 0.001030s | byte equality observed |
| deque_make.test | 10 | 0.001067s | 0.001039s | 0.001047s | byte equality observed |
| deque_misc.test | 10 | 0.001057s | 0.001061s | 0.001074s | byte equality observed |
| deque_move.test | 10 | 0.001048s | 0.001063s | 0.001049s | byte equality observed |
| deque_mutate.test | 10 | 0.001034s | 0.001018s | 0.001060s | byte equality observed |
| deque_nest.test | 10 | 0.001027s | 0.001071s | 0.001006s | byte equality observed |
| deque_tie.test | 10 | 0.001043s | 0.001025s | 0.001033s | byte equality observed |
| deque_value_at.test | 10 | 0.001036s | 0.001035s | 0.001038s | byte equality observed |
| erase.test | 10 | 0.001061s | 0.001055s | 0.001056s | byte equality observed |
| erase_key.test | 10 | 0.001075s | 0.001062s | 0.001053s | byte equality observed |
| filter.test | 10 | 0.001043s | 0.001021s | 0.001029s | byte equality observed |
| filter_if.test | 10 | 0.001073s | 0.001056s | 0.001039s | byte equality observed |
| filter_view.test | 10 | 0.001073s | 0.001048s | 0.001069s | byte equality observed |
| find.test | 10 | 0.001084s | 0.001059s | 0.001074s | byte equality observed |
| find_if.test | 10 | 0.001054s | 0.001055s | 0.001064s | byte equality observed |
| flatten.test | 10 | 0.001033s | 0.001019s | 0.001061s | byte equality observed |
| flatten_view.test | 10 | 0.001032s | 0.001037s | 0.001052s | byte equality observed |
| fold.test | 9 | 0.001049s | 0.001053s | 0.001060s | byte equality observed |
| for_each.test | 10 | 0.001069s | 0.001073s | 0.001039s | byte equality observed |
| front_extended_deque.test | 9 | 0.001026s | 0.001041s | 0.001011s | byte equality observed |
| fused.test | 10 | 0.001036s | 0.001039s | 0.000989s | byte equality observed |
| fused_function_object.test | 10 | 0.001021s | 0.001038s | 0.001039s | byte equality observed |
| fused_procedure.test | 10 | 0.001017s | 0.001034s | 0.001051s | byte equality observed |
| github-176.test | 10 | 0.001039s | 0.001061s | 0.001040s | byte equality observed |
| hash.test | 10 | 0.001029s | 0.001044s | 0.001009s | byte equality observed |
| identity_view.test | 10 | 0.001055s | 0.001084s | 0.001071s | byte equality observed |
| insert.test | 10 | 0.001092s | 0.001056s | 0.001091s | byte equality observed |
| insert_range.test | 10 | 0.001085s | 0.001067s | 0.001068s | byte equality observed |
| invoke.test | 10 | 0.001047s | 0.001034s | 0.001025s | byte equality observed |
| invoke_function_object.test | 10 | 0.001044s | 0.001026s | 0.001025s | byte equality observed |
| invoke_procedure.test | 10 | 0.001031s | 0.001037s | 0.001018s | byte equality observed |
| io.test | 10 | 0.001169s | 0.001190s | 0.001177s | byte equality observed |
| iter_fold.test | 10 | 0.001091s | 0.001075s | 0.001041s | byte equality observed |
| iterator_range.test | 10 | 0.001082s | 0.001063s | 0.001071s | byte equality observed |
| join.test | 10 | 0.001036s | 0.001023s | 0.001038s | byte equality observed |
| joint_view.test | 10 | 0.001108s | 0.001095s | 0.001094s | byte equality observed |
| list_comparison.test | 10 | 0.001033s | 0.001022s | 0.001056s | byte equality observed |
| list_construction.test | 10 | 0.001035s | 0.001023s | 0.001050s | byte equality observed |
| list_copy.test | 10 | 0.001041s | 0.001029s | 0.001027s | byte equality observed |
| list_hash.test | 10 | 0.001034s | 0.001054s | 0.001038s | byte equality observed |
| list_iterator.test | 10 | 0.001017s | 0.001054s | 0.001020s | byte equality observed |
| list_make.test | 10 | 0.001034s | 0.001046s | 0.001024s | byte equality observed |
| list_misc.test | 10 | 0.001074s | 0.001014s | 0.001029s | byte equality observed |
| list_mutate.test | 10 | 0.001014s | 0.001041s | 0.001017s | byte equality observed |
| list_nest.test | 10 | 0.001037s | 0.001038s | 0.001017s | byte equality observed |
| list_tie.test | 10 | 0.001079s | 0.001017s | 0.001016s | byte equality observed |
| list_value_at.test | 10 | 0.001012s | 0.001061s | 0.001021s | byte equality observed |
| make_fused.test | 10 | 0.001014s | 0.001009s | 0.001034s | byte equality observed |
| make_fused_function_object.test | 10 | 0.001029s | 0.001024s | 0.001070s | byte equality observed |
| make_fused_procedure.test | 10 | 0.001036s | 0.001020s | 0.001043s | byte equality observed |
| make_list.test | 10 | 0.001047s | 0.001049s | 0.001044s | byte equality observed |
| make_unfused.test | 10 | 0.001018s | 0.001010s | 0.001025s | byte equality observed |
| make_vector.test | 10 | 0.001034s | 0.001037s | 0.001024s | byte equality observed |
| map.test | 10 | 0.001074s | 0.001065s | 0.001049s | byte equality observed |
| map_comparison.test | 10 | 0.001032s | 0.001024s | 0.001045s | byte equality observed |
| map_construction.test | 10 | 0.001039s | 0.001082s | 0.001050s | byte equality observed |
| map_copy.test | 10 | 0.001033s | 0.001034s | 0.001047s | byte equality observed |
| map_misc.test | 10 | 0.001035s | 0.001018s | 0.001038s | byte equality observed |
| map_move.test | 10 | 0.001076s | 0.001047s | 0.001049s | byte equality observed |
| map_mutate.test | 10 | 0.001017s | 0.001024s | 0.001038s | byte equality observed |
| map_tie.test | 10 | 0.001016s | 0.001067s | 0.001025s | byte equality observed |
| move.test | 10 | 0.001023s | 0.001011s | 0.000999s | byte equality observed |
| nil.test | 10 | 0.001030s | 0.001047s | 0.001007s | byte equality observed |
| none.test | 10 | 0.001043s | 0.001012s | 0.001006s | byte equality observed |
| nview.test | 10 | 0.001045s | 0.001020s | 0.001008s | byte equality observed |
| pop_back.test | 10 | 0.001066s | 0.001089s | 0.001056s | byte equality observed |
| pop_front.test | 10 | 0.001082s | 0.001073s | 0.001052s | byte equality observed |
| push_back.test | 10 | 0.001083s | 0.001086s | 0.001060s | byte equality observed |
| push_front.test | 10 | 0.001097s | 0.001063s | 0.001086s | byte equality observed |
| quick.test | 10 | 0.000482s | 0.001009s | 0.001014s | byte equality observed |
| ref_vector.test | 10 | 0.001050s | 0.001034s | 0.001045s | byte equality observed |
| remove.test | 10 | 0.001089s | 0.001076s | 0.001075s | byte equality observed |
| remove_if.test | 10 | 0.001083s | 0.001068s | 0.001068s | byte equality observed |
| repetitive_view.test | 10 | 0.001014s | 0.001037s | 0.001028s | byte equality observed |
| replace.test | 10 | 0.001041s | 0.001071s | 0.001066s | byte equality observed |
| replace_if.test | 10 | 0.001071s | 0.001051s | 0.001072s | byte equality observed |
| reverse.test | 10 | 0.001080s | 0.001083s | 0.001072s | byte equality observed |
| reverse_fold.test | 10 | 0.001073s | 0.001058s | 0.001066s | byte equality observed |
| reverse_iter_fold.test | 10 | 0.001059s | 0.001098s | 0.001061s | byte equality observed |
| reverse_view.test | 10 | 0.001050s | 0.001068s | 0.001062s | byte equality observed |
| segmented_find.test | 10 | 0.001047s | 0.001043s | 0.001021s | byte equality observed |
| segmented_find_if.test | 10 | 0.001026s | 0.001050s | 0.001043s | byte equality observed |
| segmented_fold.test | 10 | 0.001069s | 0.001086s | 0.001050s | byte equality observed |
| segmented_for_each.test | 10 | 0.001057s | 0.001111s | 0.001066s | byte equality observed |
| segmented_iterator_range.test | 10 | 0.001065s | 0.001080s | 0.001077s | byte equality observed |
| set.test | 10 | 0.001046s | 0.001080s | 0.001058s | byte equality observed |
| single_view.test | 10 | 0.001047s | 0.001051s | 0.001057s | byte equality observed |
| std_array.test | 10 | 0.001037s | 0.001024s | 0.001029s | byte equality observed |
| std_pair.test | 10 | 0.001069s | 0.001076s | 0.001082s | byte equality observed |
| std_tuple.test | 10 | 0.001045s | 0.001054s | 0.001066s | byte equality observed |
| std_tuple_iterator.test | 10 | 0.001039s | 0.001069s | 0.001024s | byte equality observed |
| swap.test | 10 | 0.001046s | 0.001053s | 0.001050s | byte equality observed |
| transform.test | 10 | 0.001062s | 0.001086s | 0.001074s | byte equality observed |
| transform_view.test | 10 | 0.001070s | 0.001067s | 0.001060s | byte equality observed |
| tuple_comparison.test | 10 | 0.001009s | 0.001028s | 0.001054s | byte equality observed |
| tuple_construction.test | 10 | 0.001053s | 0.000995s | 0.001011s | byte equality observed |
| tuple_conversion.test | 10 | 0.001044s | 0.001013s | 0.001076s | byte equality observed |
| tuple_copy.test | 10 | 0.001008s | 0.001015s | 0.001007s | byte equality observed |
| tuple_element.test | 10 | 0.001026s | 0.001050s | 0.001029s | byte equality observed |
| tuple_hash.test | 10 | 0.001013s | 0.001050s | 0.001021s | byte equality observed |
| tuple_make.test | 10 | 0.001012s | 0.001069s | 0.001006s | byte equality observed |
| tuple_misc.test | 10 | 0.001028s | 0.001035s | 0.000998s | byte equality observed |
| tuple_mutate.test | 10 | 0.001023s | 0.001016s | 0.000999s | byte equality observed |
| tuple_nest.test | 10 | 0.001051s | 0.001029s | 0.001038s | byte equality observed |
| tuple_tie.test | 10 | 0.001037s | 0.001013s | 0.001051s | byte equality observed |
| tuple_traits__maybe_variadic.test | 10 | 0.001040s | 0.001018s | 0.001015s | byte equality observed |
| tuple_traits__no_variadic.test | 10 | 0.001036s | 0.001056s | 0.001005s | byte equality observed |
| unfused.test | 10 | 0.001025s | 0.001072s | 0.001038s | byte equality observed |
| unfused_typed.test | 10 | 0.001047s | 0.001070s | 0.001017s | byte equality observed |
| vector_comparison.test | 10 | 0.001044s | 0.001062s | 0.001026s | byte equality observed |
| vector_construction.test | 10 | 0.001034s | 0.001059s | 0.001012s | byte equality observed |
| vector_conversion.test | 10 | 0.001055s | 0.001028s | 0.001002s | byte equality observed |
| vector_copy.test | 10 | 0.001021s | 0.001032s | 0.001023s | byte equality observed |
| vector_hash.test | 10 | 0.001036s | 0.001048s | 0.001010s | byte equality observed |
| vector_iterator.test | 10 | 0.001014s | 0.001043s | 0.001021s | byte equality observed |
| vector_make.test | 10 | 0.001058s | 0.001065s | 0.001025s | byte equality observed |
| vector_misc.test | 10 | 0.001037s | 0.001070s | 0.001016s | byte equality observed |
| vector_move.test | 10 | 0.001042s | 0.001061s | 0.001049s | byte equality observed |
| vector_mutate.test | 10 | 0.001024s | 0.001042s | 0.001014s | byte equality observed |
| vector_n.test | 10 | 0.001070s | 0.001077s | 0.001042s | byte equality observed |
| vector_nest.test | 10 | 0.001035s | 0.001027s | 0.001063s | byte equality observed |
| vector_tie.test | 10 | 0.001039s | 0.001060s | 0.001038s | byte equality observed |
| vector_traits__maybe_variadic.test | 10 | 0.001049s | 0.001062s | 0.001038s | byte equality observed |
| vector_traits__no_variadic.test | 10 | 0.001065s | 0.001045s | 0.001028s | byte equality observed |
| vector_value_at.test | 10 | 0.001025s | 0.001014s | 0.001061s | byte equality observed |
| zip.test | 10 | 0.001033s | 0.001025s | 0.001037s | byte equality observed |
| zip2.test | 10 | 0.001024s | 0.001034s | 0.001044s | byte equality observed |
| zip_ignore.test | 10 | 0.001046s | 0.001041s | 0.001057s | byte equality observed |
| zip_view.test | 10 | 0.001030s | 0.001066s | 0.001024s | byte equality observed |
| zip_view2.test | 10 | 0.001060s | 0.001030s | 0.001038s | byte equality observed |
| zip_view_ignore.test | 10 | 0.001028s | 0.001020s | 0.001033s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
