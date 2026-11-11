# libs/fusion/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| all.test | RUN | 0.332678 | 0.592677 | 0.807399 | 0.001646 | 0.001445 | 0.001840 |
| any.test | RUN | 0.322646 | 0.550110 | 0.757986 | 0.001836 | 0.001560 | 0.001861 |
| clear.test | RUN | 0.239349 | 0.515386 | 0.647284 | 0.001516 | 0.001638 | 0.001800 |
| copy.test | RUN | 0.208398 | 0.381484 | 0.512630 | 0.001683 | 0.001713 | 0.002034 |
| count.test | RUN | 0.229206 | 0.426130 | 0.580988 | 0.001774 | 0.001649 | 0.001924 |
| count_if.test | RUN | 0.217107 | 0.396918 | 0.538266 | 0.001485 | 0.001603 | 0.001862 |
| erase.test | RUN | 0.288543 | 0.712956 | 0.905564 | 0.001624 | 0.001894 | 0.001975 |
| erase_key.test | RUN | 0.282328 | 0.732837 | 0.830023 | 0.001520 | 0.001651 | 0.001933 |
| filter.test | RUN | 0.243408 | 0.650048 | 0.807827 | 0.001832 | 0.001768 | 0.001936 |
| filter_if.test | RUN | 0.274082 | 0.756263 | 0.937720 | 0.001537 | 0.001579 | 0.001684 |
| find.test | RUN | 0.252873 | 0.476419 | 0.687411 | 0.001623 | 0.001810 | 0.002889 |
| find_if.test | RUN | 0.241575 | 0.443847 | 0.573145 | 0.001825 | 0.001415 | 0.001948 |
| fold.test | RUN | 0.256788 | 0.524494 | 0.766452 | 0.001766 | 0.001843 | 0.002088 |
| for_each.test | RUN | 0.233494 | 0.482051 | 0.675894 | 0.001704 | 0.001840 | 0.001990 |
| insert.test | RUN | 0.335451 | 0.859907 | 1.077074 | 0.001818 | 0.001760 | 0.001956 |
| insert_range.test | RUN | 0.333076 | 0.863745 | 1.075511 | 0.001749 | 0.001542 | 0.001980 |
| iter_fold.test | RUN | 0.399354 | 0.816661 | 1.257700 | 0.001877 | 0.001916 | 0.001874 |
| move.test | RUN | 0.209554 | 0.397374 | 0.528147 | 0.001738 | 0.001621 | 0.001815 |
| none.test | RUN | 0.320817 | 0.566036 | 0.766144 | 0.001858 | 0.001778 | 0.001914 |
| pop_back.test | RUN | 0.322316 | 0.766063 | 0.914722 | 0.001959 | 0.001526 | 0.001643 |
| pop_front.test | RUN | 0.265735 | 0.636300 | 0.751430 | 0.001842 | 0.001566 | 0.001848 |
| push_back.test | RUN | 0.328302 | 0.891049 | 1.022300 | 0.001711 | 0.001990 | 0.001708 |
| push_front.test | RUN | 0.295969 | 0.765133 | 0.852243 | 0.001724 | 0.001760 | 0.001998 |
| remove.test | RUN | 0.322813 | 0.853258 | 1.029370 | 0.001512 | 0.002050 | 0.001856 |
| remove_if.test | RUN | 0.297463 | 0.710569 | 0.863033 | 0.001859 | 0.001693 | 0.001908 |
| replace.test | RUN | 0.283347 | 0.699652 | 0.796014 | 0.001665 | 0.001948 | 0.001802 |
| replace_if.test | RUN | 0.276722 | 0.693671 | 0.774251 | 0.001700 | 0.002060 | 0.002056 |
| reverse_fold.test | RUN | 0.443284 | 0.992572 | 1.367986 | 0.001785 | 0.001948 | 0.001928 |
| reverse_iter_fold.test | RUN | 0.448614 | 1.021055 | 1.467820 | 0.001941 | 0.001833 | 0.001944 |
| reverse.test | RUN | 0.279464 | 0.739647 | 0.816208 | 0.001686 | 0.002017 | 0.001957 |
| segmented_for_each.test | RUN | 0.467066 | 1.389266 | 1.648955 | 0.001614 | 0.001540 | 0.001721 |
| segmented_find.test | RUN | 0.296488 | 0.698902 | 0.920006 | 0.001571 | 0.001904 | 0.001955 |
| segmented_find_if.test | RUN | 0.300539 | 0.736346 | 0.925031 | 0.001712 | 0.001890 | 0.002038 |
| segmented_fold.test | RUN | 0.363332 | 0.927451 | 1.185879 | 0.002072 | 0.001917 | 0.001864 |
| transform.test | RUN | 0.330223 | 0.765383 | 0.868659 | 0.001583 | 0.001914 | 0.001956 |
| join.test | RUN | 0.258304 | 0.438121 | 0.599614 | 0.001835 | 0.001802 | 0.001910 |
| zip.test | RUN | 0.410748 | 0.843452 | 1.139025 | 0.001790 | 0.001945 | 0.001775 |
| zip2.test | RUN | 0.372570 | 0.814542 | 1.059219 | 0.001711 | 0.001649 | 0.001857 |
| zip_ignore.test | RUN | 0.282365 | 0.577293 | 0.707198 | 0.001427 | 0.001792 | 0.001606 |
| flatten.test | RUN | 0.272676 | 0.616424 | 0.733935 | 0.001495 | 0.001889 | 0.001994 |
| ticket-5490.test | COMPILE | 0.143858 | 0.277313 | 0.338366 | — | — | — |
| as_deque.test | RUN | 0.252282 | 0.571296 | 0.816762 | 0.001634 | 0.001834 | 0.001947 |
| as_list.test | RUN | 0.281756 | 0.726192 | 0.900473 | 0.001482 | 0.001820 | 0.001662 |
| as_map.test | RUN | 0.275835 | 0.721088 | 0.888705 | 0.001682 | 0.001472 | 0.002024 |
| as_map_assoc.test | RUN | 0.276738 | 0.669085 | 0.822010 | 0.001504 | 0.001962 | 0.001619 |
| as_set.test | RUN | 0.310236 | 0.835144 | 0.989972 | 0.001660 | 0.001550 | 0.001688 |
| as_vector.test | RUN | 0.288743 | 0.750962 | 0.887710 | 0.001338 | 0.001889 | 0.002024 |
| boost_tuple.test | RUN | 0.283859 | 0.755300 | 0.887743 | 0.001439 | 0.001602 | 0.001928 |
| boost_tuple_iterator.test | RUN | 0.203406 | 0.440437 | 0.519468 | 0.001502 | 0.001682 | 0.001564 |
| cons.test | RUN | 0.357346 | 0.828486 | 1.039031 | 0.001408 | 0.001725 | 0.001588 |
| convert_boost_tuple.test | RUN | 0.233654 | 0.625561 | 0.678966 | 0.001697 | 0.001765 | 0.001669 |
| convert_deque.test | RUN | 0.235265 | 0.623654 | 0.670967 | 0.001592 | 0.001750 | 0.001778 |
| convert_list.test | RUN | 0.232484 | 0.650468 | 0.692286 | 0.001506 | 0.001610 | 0.001429 |
| convert_std_pair.test | RUN | 0.258673 | 0.626854 | 0.702205 | 0.001548 | 0.001747 | 0.001582 |
| convert_std_tuple.test | RUN | 0.244997 | 0.616863 | 0.677070 | 0.001572 | 0.001669 | 0.001896 |
| convert_vector.test | RUN | 0.245438 | 0.619166 | 0.696824 | 0.001626 | 0.001600 | 0.001548 |
| filter_view.test | RUN | 0.298263 | 0.661635 | 0.830969 | 0.001738 | 0.001786 | 0.002062 |
| hash.test | RUN | 0.287219 | 0.588037 | 0.744864 | 0.001686 | 0.001898 | 0.001932 |
| io.test | RUN | 0.303160 | 0.852964 | 0.955002 | 0.001363 | 0.002049 | 0.002164 |
| iterator_range.test | RUN | 0.277036 | 0.638812 | 0.817067 | 0.001380 | 0.001860 | 0.001703 |
| joint_view.test | RUN | 0.355331 | 1.049883 | 1.243399 | 0.001847 | 0.001685 | 0.001714 |
| list_comparison.test | RUN | 0.210515 | 0.438441 | 0.585101 | 0.001506 | 0.001786 | 0.001676 |
| list_construction.test | RUN | 0.207861 | 0.422496 | 0.534371 | 0.001490 | 0.003684 | 0.001937 |
| list_copy.test | RUN | 0.317935 | 0.822786 | 0.944373 | 0.001693 | 0.001491 | 0.001622 |
| list_iterator.test | RUN | 0.214701 | 0.458371 | 0.532987 | 0.001496 | 0.001797 | 0.001826 |
| list_hash.test | RUN | 0.276787 | 0.873624 | 0.766444 | 0.001592 | 0.001844 | 0.001691 |
| list_make.test | RUN | 0.213721 | 0.468436 | 0.603700 | 0.001699 | 0.001626 | 0.001810 |
| list_misc.test | RUN | 0.273426 | 0.537013 | 0.716129 | 0.001586 | 0.001827 | 0.001920 |
| list_mutate.test | RUN | 0.191932 | 0.366313 | 0.475642 | 0.001432 | 0.001738 | 0.001926 |
| list_nest.test | RUN | 0.387871 | 1.041662 | 1.406076 | 0.001420 | 0.001509 | 0.001951 |
| list_tie.test | RUN | 0.213958 | 0.404601 | 0.539328 | 0.001522 | 0.001917 | 0.002138 |
| list_value_at.test | RUN | 0.206116 | 0.394929 | 0.589511 | 0.001742 | 0.001816 | 0.001934 |
| deque_comparison.test | RUN | 0.217249 | 0.459576 | 0.584339 | 0.001558 | 0.001604 | 0.001832 |
| deque_construction.test | RUN | 0.210665 | 0.410559 | 0.568265 | 0.001809 | 0.001840 | 0.001786 |
| deque_copy.test | RUN | 0.297054 | 0.697891 | 0.890671 | 0.001443 | 0.001778 | 0.001790 |
| deque_iterator.test | RUN | 0.207989 | 0.430474 | 0.522694 | 0.001589 | 0.001731 | 0.001966 |
| deque_hash.test | RUN | 0.291518 | 0.778111 | 0.769293 | 0.001374 | 0.001872 | 0.001886 |
| deque_is_constructible.test | COMPILE | 0.115628 | 0.198776 | 0.253857 | — | — | — |
| deque_make.test | RUN | 0.214105 | 0.442972 | 0.562619 | 0.001811 | 0.001726 | 0.001906 |
| deque_misc.test | RUN | 0.270141 | 0.556914 | 0.688233 | 0.001477 | 0.001273 | 0.001954 |
| deque_move.test | RUN | 0.212985 | 0.462179 | 0.550117 | 0.001568 | 0.001452 | 0.001900 |
| deque_mutate.test | RUN | 0.192478 | 0.367052 | 0.485072 | 0.001642 | 0.001753 | 0.001784 |
| deque_nest.test | RUN | 0.566973 | 1.590916 | 2.169215 | 0.001730 | 0.001924 | 0.001898 |
| deque_tie.test | RUN | 0.206579 | 0.369384 | 0.514808 | 0.001742 | 0.001780 | 0.001720 |
| deque_value_at.test | RUN | 0.201335 | 0.369687 | 0.485594 | 0.001436 | 0.001508 | 0.001568 |
| front_extended_deque.test | RUN | 0.288115 | 0.546180 | 0.755584 | 0.001694 | 0.001850 | 0.001794 |
| back_extended_deque.test | RUN | 0.278759 | 0.571603 | 0.734969 | 0.001483 | 0.001860 | 0.001571 |
| make_list.test | RUN | 0.222304 | 0.467449 | 0.590699 | 0.001552 | 0.001879 | 0.001769 |
| make_vector.test | RUN | 0.220221 | 0.466466 | 0.618469 | 0.001490 | 0.001744 | 0.001622 |
| map.test | RUN | 0.285373 | 0.704423 | 0.828004 | 0.001739 | 0.001697 | 0.001729 |
| map_comparison.test | RUN | 0.217219 | 0.448782 | 0.559478 | 0.003873 | 0.001860 | 0.001845 |
| map_construction.test | RUN | 0.215407 | 0.443830 | 0.549635 | 0.004014 | 0.001726 | 0.001627 |
| map_copy.test | RUN | 0.260194 | 0.576638 | 0.675251 | 0.001554 | 0.001834 | 0.001776 |
| map_misc.test | RUN | 0.317704 | 0.548538 | 0.675497 | 0.001701 | 0.001712 | 0.001521 |
| map_move.test | RUN | 0.215214 | 0.451017 | 0.555440 | 0.001426 | 0.001586 | 0.001493 |
| map_mutate.test | RUN | 0.195689 | 0.370314 | 0.472090 | 0.001503 | 0.001551 | 0.003638 |
| map_tie.test | RUN | 0.196844 | 0.388063 | 0.487791 | 0.001426 | 0.001831 | 0.002020 |
| nil.test | RUN | 0.140967 | 0.274035 | 0.371589 | 0.001782 | 0.001816 | 0.001872 |
| nview.test | RUN | 0.263848 | 0.555704 | 0.721639 | 0.001350 | 0.001916 | 0.002034 |
| reverse_view.test | RUN | 0.317867 | 0.896131 | 1.050960 | 0.004087 | 0.001898 | 0.001848 |
| segmented_iterator_range.test | RUN | 0.413195 | 1.058909 | 1.341557 | 0.001870 | 0.001826 | 0.001839 |
| set.test | RUN | 0.322215 | 0.620884 | 0.732452 | 0.001360 | 0.001876 | 0.002058 |
| single_view.test | RUN | 0.256491 | 0.540897 | 0.686356 | 0.001753 | 0.001641 | 0.001570 |
| std_pair.test | RUN | 0.298327 | 0.726755 | 0.840977 | 0.001710 | 0.001563 | 0.001515 |
| boost_array.test | RUN | 0.257215 | 0.491705 | 0.637306 | 0.001888 | 0.001768 | 0.001796 |
| array.test | RUN | 0.259676 | 0.516943 | 0.621925 | 0.001752 | 0.001767 | 0.001840 |
| std_array.test | RUN | 0.263429 | 0.506330 | 0.622835 | 0.001762 | 0.001856 | 0.001536 |
| tuple_comparison.test | RUN | 0.234626 | 0.461393 | 0.610309 | 0.001778 | 0.001797 | 0.001836 |
| tuple_construction.test | RUN | 0.235747 | 0.459524 | 0.623905 | 0.001638 | 0.001587 | 0.001638 |
| tuple_conversion.test | RUN | 0.503085 | 1.389931 | 2.035474 | 0.001167 | 0.001826 | 0.001920 |
| tuple_copy.test | RUN | 0.321724 | 0.800646 | 1.077182 | 0.001820 | 0.001611 | 0.001620 |
| tuple_element.test | RUN | 0.220525 | 0.409879 | 0.545916 | 0.001500 | 0.001636 | 0.001812 |
| tuple_make.test | RUN | 0.241234 | 0.488327 | 0.598630 | 0.001765 | 0.001822 | 0.001966 |
| tuple_misc.test | RUN | 0.292018 | 0.595852 | 0.774976 | 0.001651 | 0.001884 | 0.001700 |
| tuple_mutate.test | RUN | 0.209062 | 0.390367 | 0.508274 | 0.001875 | 0.001914 | 0.001853 |
| tuple_nest.test | RUN | 0.861975 | 2.955466 | 3.802872 | 0.001904 | 0.001831 | 0.001916 |
| tuple_hash.test | RUN | 0.300570 | 0.817309 | 0.758719 | 0.003986 | 0.001332 | 0.001763 |
| tuple_tie.test | RUN | 0.240302 | 0.446252 | 0.562306 | 0.001848 | 0.001862 | 0.001790 |
| tuple_traits__maybe_variadic.test | RUN | 0.246194 | 0.509801 | 0.614003 | 0.001624 | 0.001637 | 0.001963 |
| tuple_traits__no_variadic.test | RUN | 0.275954 | 0.521075 | 0.648272 | 0.001552 | 0.001906 | 0.001933 |
| transform_view.test | RUN | 0.350623 | 0.890908 | 1.074092 | 0.001664 | 0.001802 | 0.001933 |
| identity_view.test | RUN | 0.328134 | 0.816035 | 1.011091 | 0.001584 | 0.001872 | 0.001595 |
| vector_comparison.test | RUN | 0.225109 | 0.488936 | 0.633999 | 0.002304 | 0.001816 | 0.001950 |
| vector_construction.test | RUN | 0.225200 | 0.426630 | 0.572706 | 0.001660 | 0.001766 | 0.001950 |
| vector_conversion.test | RUN | 0.572834 | 1.858596 | 2.601085 | 0.001837 | 0.001932 | 0.002030 |
| vector_copy.test | RUN | 0.306307 | 0.793047 | 1.089756 | 0.001307 | 0.001832 | 0.001907 |
| vector_iterator.test | RUN | 0.203159 | 0.438165 | 0.575513 | 0.001561 | 0.001764 | 0.001715 |
| vector_make.test | RUN | 0.217189 | 0.451620 | 0.598171 | 0.001518 | 0.001748 | 0.001717 |
| vector_misc.test | RUN | 0.257623 | 0.550214 | 0.728223 | 0.001775 | 0.001717 | 0.001905 |
| vector_move.test | RUN | 0.206972 | 0.449866 | 0.535972 | 0.001482 | 0.001588 | 0.001820 |
| vector_mutate.test | RUN | 0.188956 | 0.384621 | 0.476526 | 0.001269 | 0.001422 | 0.001886 |
| vector_n.test | RUN | 0.229227 | 0.499994 | 0.658324 | 0.001835 | 0.001799 | 0.002159 |
| vector_nest.test | RUN | 0.823531 | 3.047900 | 3.742921 | 0.001758 | 0.001725 | 0.001622 |
| vector_hash.test | RUN | 0.267517 | 0.839205 | 0.789226 | 0.001839 | 0.001820 | 0.001722 |
| vector_tie.test | RUN | 0.197639 | 0.404044 | 0.544407 | 0.001905 | 0.001576 | 0.001972 |
| vector_traits__maybe_variadic.test | RUN | 0.217455 | 0.469863 | 0.515028 | 0.001777 | 0.001900 | 0.001848 |
| vector_traits__no_variadic.test | RUN | 0.244762 | 0.503094 | 0.579094 | 0.001736 | 0.001718 | 0.001602 |
| vector_value_at.test | RUN | 0.202682 | 0.372603 | 0.472904 | 0.001475 | 0.001796 | 0.001791 |
| zip_view.test | RUN | 0.434813 | 0.869989 | 1.269702 | 0.001906 | 0.001570 | 0.001729 |
| zip_view2.test | RUN | 0.350035 | 0.695803 | 0.949882 | 0.001595 | 0.001253 | 0.001632 |
| zip_view_ignore.test | RUN | 0.335906 | 0.678869 | 0.837061 | 0.001477 | 0.001695 | 0.001710 |
| repetitive_view.test | RUN | 0.532736 | 1.146308 | 1.587844 | 0.001750 | 0.001798 | 0.001562 |
| deduce_sequence.test | RUN | 0.252116 | 0.514638 | 0.621443 | 0.001702 | 0.001776 | 0.001742 |
| adapt_adt_named.test | RUN | 0.301912 | 0.696895 | 0.823505 | 0.001647 | 0.001708 | 0.001789 |
| adapt_adt_named_empty.test | RUN | 0.261482 | 0.557648 | 0.639029 | 0.001832 | 0.001920 | 0.001596 |
| adapt_adt.test | RUN | 0.336176 | 0.912320 | 0.963350 | 0.001540 | 0.001689 | 0.001804 |
| adapt_adt_empty.test | RUN | 0.270759 | 0.569638 | 0.648692 | 0.001799 | 0.001761 | 0.001845 |
| adapt_assoc_adt_named.test | RUN | 0.344762 | 0.750678 | 0.882589 | 0.001534 | 0.001735 | 0.001878 |
| adapt_assoc_adt_named_empty.test | RUN | 0.274004 | 0.561980 | 0.676546 | 0.001580 | 0.001603 | 0.001863 |
| adapt_assoc_adt.test | RUN | 0.339297 | 0.740248 | 0.938416 | 0.001617 | 0.001445 | 0.001810 |
| adapt_assoc_adt_empty.test | RUN | 0.259433 | 0.563890 | 0.672474 | 0.001693 | 0.001697 | 0.001928 |
| adapt_assoc_struct_named.test | RUN | 0.323171 | 0.767239 | 0.876106 | 0.001626 | 0.001770 | 0.001620 |
| adapt_assoc_struct_named_empty.test | RUN | 0.256307 | 0.578443 | 0.645583 | 0.001683 | 0.001896 | 0.001774 |
| adapt_assoc_struct.test | RUN | 0.281523 | 0.705651 | 0.800927 | 0.001326 | 0.001678 | 0.001720 |
| adapt_assoc_struct_empty.test | RUN | 0.245954 | 0.593082 | 0.655773 | 0.001658 | 0.001578 | 0.001475 |
| adapt_assoc_tpl_adt.test | RUN | 0.326238 | 0.778958 | 0.949189 | 0.001565 | 0.001519 | 0.001868 |
| adapt_assoc_tpl_adt_empty.test | RUN | 0.248377 | 0.568711 | 0.652775 | 0.001396 | 0.001959 | 0.001933 |
| adapt_assoc_tpl_struct.test | RUN | 0.259550 | 0.657831 | 0.778742 | 0.001559 | 0.001524 | 0.001712 |
| adapt_assoc_tpl_struct_empty.test | RUN | 0.237428 | 0.578242 | 0.646574 | 0.001502 | 0.001463 | 0.001212 |
| adapt_struct_named.test | RUN | 0.269346 | 0.679106 | 0.767300 | 0.001514 | 0.001705 | 0.001800 |
| adapt_struct_named_empty.test | RUN | 0.251087 | 0.562410 | 0.651807 | 0.001495 | 0.001803 | 0.001876 |
| adapt_struct.test | RUN | 0.314908 | 0.769129 | 0.891004 | 0.001476 | 0.001751 | 0.001762 |
| adapt_struct_empty.test | RUN | 0.256233 | 0.560216 | 0.676703 | 0.001514 | 0.001670 | 0.001544 |
| adapt_tpl_adt.test | RUN | 0.311376 | 0.888390 | 0.938714 | 0.001410 | 0.001460 | 0.001510 |
| adapt_tpl_adt_empty.test | RUN | 0.256496 | 0.570682 | 0.644892 | 0.001530 | 0.001669 | 0.001742 |
| adapt_tpl_struct.test | RUN | 0.249166 | 0.604365 | 0.729825 | 0.001610 | 0.001697 | 0.001615 |
| adapt_tpl_struct_empty.test | RUN | 0.239922 | 0.581458 | 0.640400 | 0.001426 | 0.001848 | 0.001531 |
| adt_attribute_proxy.test | RUN | 0.275041 | 0.548574 | 0.686571 | 0.001720 | 0.001779 | 0.001760 |
| define_struct.test | RUN | 0.331467 | 0.776916 | 0.947663 | 0.001589 | 0.001610 | 0.001550 |
| define_struct_empty.test | RUN | 0.290930 | 0.642305 | 0.780275 | 0.001972 | 0.001630 | 0.001672 |
| define_struct_move.test | RUN | 0.198040 | 0.360528 | 0.448384 | 0.001871 | 0.001614 | 0.001463 |
| define_struct_inline.test | RUN | 0.364625 | 0.909152 | 1.058859 | 0.001740 | 0.001619 | 0.001866 |
| define_struct_inline_empty.test | RUN | 0.308697 | 0.644245 | 0.772347 | 0.001494 | 0.001749 | 0.001686 |
| define_struct_inline_move.test | RUN | 0.208479 | 0.346526 | 0.457701 | 0.001767 | 0.001729 | 0.001680 |
| define_assoc_struct.test | RUN | 0.326327 | 0.720295 | 0.892620 | 0.001790 | 0.001772 | 0.001835 |
| define_assoc_struct_empty.test | RUN | 0.308549 | 0.630484 | 0.772176 | 0.001914 | 0.001812 | 0.001882 |
| define_assoc_struct_move.test | RUN | 0.210937 | 0.346010 | 0.446146 | 0.001632 | 0.001690 | 0.001804 |
| define_tpl_struct.test | RUN | 0.338873 | 0.733419 | 0.879139 | 0.001778 | 0.001565 | 0.001477 |
| define_tpl_struct_empty.test | RUN | 0.317260 | 0.621441 | 0.769070 | 0.001492 | 0.001571 | 0.001920 |
| define_tpl_struct_move.test | RUN | 0.213514 | 0.362859 | 0.471696 | 0.001746 | 0.001606 | 0.001690 |
| define_tpl_struct_inline.test | RUN | 0.362936 | 0.867555 | 0.981298 | 0.001655 | 0.001680 | 0.001825 |
| define_tpl_struct_inline_empty.test | RUN | 0.357201 | 0.633989 | 0.767604 | 0.001960 | 0.001638 | 0.001786 |
| define_tpl_struct_inline_move.test | RUN | 0.243215 | 0.353656 | 0.459960 | 0.001778 | 0.001564 | 0.001891 |
| define_assoc_tpl_struct.test | RUN | 0.391028 | 0.755815 | 0.882719 | 0.002350 | 0.001790 | 0.001850 |
| define_assoc_tpl_struct_empty.test | RUN | 0.321073 | 0.635196 | 0.771288 | 0.001880 | 0.001462 | 0.001660 |
| define_assoc_tpl_struct_move.test | RUN | 0.212325 | 0.352411 | 0.467783 | 0.001515 | 0.001784 | 0.001930 |
| std_tuple.test | RUN | 0.218245 | 0.406991 | 0.499502 | 0.001777 | 0.001754 | 0.001582 |
| std_tuple_iterator.test | RUN | 0.219625 | 0.426500 | 0.482438 | 0.001886 | 0.001800 | 0.001816 |
| ref_vector.test | RUN | 0.243987 | 0.425691 | 0.530728 | 0.001636 | 0.001695 | 0.001934 |
| flatten_view.test | RUN | 0.260013 | 0.524007 | 0.648978 | 0.001539 | 0.001609 | 0.001431 |
| github-159.test | COMPILE | 0.120736 | 0.198818 | 0.241089 | — | — | — |
| github-176.test | RUN | 0.244314 | 0.459160 | 0.547951 | 0.001806 | 0.001646 | 0.001604 |
| size.test | COMPILE | 0.206827 | 0.336492 | 0.408901 | — | — | — |
| fused.test | RUN | 0.285675 | 0.506416 | 0.628691 | 0.001476 | 0.001770 | 0.001659 |
| fused_function_object.test | RUN | 0.215344 | 0.383863 | 0.477912 | 0.001878 | 0.001586 | 0.001794 |
| fused_procedure.test | RUN | 0.272168 | 0.472451 | 0.596885 | 0.001818 | 0.001550 | 0.001519 |
| unfused.test | RUN | 0.215011 | 0.401347 | 0.502184 | 0.001753 | 0.001736 | 0.001686 |
| unfused_typed.test | RUN | 0.218246 | 0.433139 | 0.506660 | 0.001803 | 0.001757 | 0.001853 |
| make_fused.test | RUN | 0.280636 | 0.490904 | 0.621975 | 0.001806 | 0.001519 | 0.001672 |
| make_fused_function_object.test | RUN | 0.208531 | 0.377139 | 0.480379 | 0.001529 | 0.001626 | 0.001479 |
| make_fused_procedure.test | RUN | 0.274762 | 0.483204 | 0.587694 | 0.001782 | 0.001661 | 0.001748 |
| make_unfused.test | RUN | 0.219212 | 0.380347 | 0.484268 | 0.002132 | 0.001742 | 0.001862 |
| invoke.test | RUN | 0.543588 | 1.354797 | 1.713230 | 0.001870 | 0.001556 | 0.001662 |
| invoke_function_object.test | RUN | 0.278593 | 0.483278 | 0.607537 | 0.001915 | 0.001748 | 0.001725 |
| invoke_procedure.test | RUN | 0.439636 | 1.037930 | 1.257056 | 0.001737 | 0.001732 | 0.001859 |
| swap.test | RUN | 0.336917 | 0.642462 | 0.754510 | 0.001462 | 0.001820 | 0.001638 |
| is_sequence.test | COMPILE | 0.060523 | 0.093748 | 0.100851 | — | — | — |
| is_view.test | COMPILE | 0.059942 | 0.098174 | 0.105157 | — | — | — |
| pair_deque.test | COMPILE | 0.117586 | 0.209393 | 0.237535 | — | — | — |
| pair_list.test | COMPILE | 0.117397 | 0.200429 | 0.235657 | — | — | — |
| pair_map.test | COMPILE | 0.121885 | 0.210294 | 0.254967 | — | — | — |
| pair_set.test | COMPILE | 0.127491 | 0.206450 | 0.255847 | — | — | — |
| pair_vector.test | COMPILE | 0.121033 | 0.204539 | 0.245610 | — | — | — |
| pair_nest.test | COMPILE | 0.050235 | 0.086343 | 0.098988 | — | — | — |
| index_sequence.test | COMPILE | 0.046704 | 0.058849 | 0.076568 | — | — | — |
| and.test | COMPILE | 0.036264 | 0.044933 | 0.059673 | — | — | — |
| tag_of.test | COMPILE | 0.057135 | 0.090124 | 0.102213 | — | — | — |
| unused.test | COMPILE | 0.028186 | 0.059454 | 0.066608 | — | — | — |
| tag_of_fallback.test | COMPILE | 0.060020 | 0.082945 | 0.095462 | — | — | — |
| make_pair_r-value.test | COMPILE | 0.050075 | 0.083691 | 0.070328 | — | — | — |
| quick.test | RUN | 0.160951 | 0.278706 | 0.313868 | 0.000634 | 0.003126 | 0.001261 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.015× | 1.022×, 1.008× | 0.978× |
| clang | 1.007× | 0.996×, 1.018× | 1.008× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adapt_adt.test | 10 | 0.000965s | 0.000998s | 0.001018s | byte equality observed |
| adapt_adt_empty.test | 10 | 0.000971s | 0.000989s | 0.000982s | byte equality observed |
| adapt_adt_named.test | 10 | 0.000970s | 0.001000s | 0.000978s | byte equality observed |
| adapt_adt_named_empty.test | 10 | 0.000960s | 0.001001s | 0.000991s | byte equality observed |
| adapt_assoc_adt.test | 10 | 0.000973s | 0.000995s | 0.000973s | byte equality observed |
| adapt_assoc_adt_empty.test | 10 | 0.000956s | 0.000996s | 0.000982s | byte equality observed |
| adapt_assoc_adt_named.test | 10 | 0.000978s | 0.000999s | 0.000990s | byte equality observed |
| adapt_assoc_adt_named_empty.test | 10 | 0.000957s | 0.000985s | 0.000981s | byte equality observed |
| adapt_assoc_struct.test | 10 | 0.000971s | 0.000984s | 0.000987s | byte equality observed |
| adapt_assoc_struct_empty.test | 10 | 0.000985s | 0.000989s | 0.000971s | byte equality observed |
| adapt_assoc_struct_named.test | 10 | 0.000975s | 0.000975s | 0.000979s | byte equality observed |
| adapt_assoc_struct_named_empty.test | 10 | 0.000968s | 0.000977s | 0.000960s | byte equality observed |
| adapt_assoc_tpl_adt.test | 10 | 0.000975s | 0.001008s | 0.001007s | byte equality observed |
| adapt_assoc_tpl_adt_empty.test | 10 | 0.000977s | 0.000984s | 0.001001s | byte equality observed |
| adapt_assoc_tpl_struct.test | 10 | 0.001011s | 0.001013s | 0.001046s | byte equality observed |
| adapt_assoc_tpl_struct_empty.test | 10 | 0.000970s | 0.000979s | 0.000999s | byte equality observed |
| adapt_struct.test | 10 | 0.000989s | 0.000995s | 0.000995s | byte equality observed |
| adapt_struct_empty.test | 10 | 0.000974s | 0.000981s | 0.000993s | byte equality observed |
| adapt_struct_named.test | 10 | 0.000989s | 0.000982s | 0.000976s | byte equality observed |
| adapt_struct_named_empty.test | 10 | 0.000977s | 0.000967s | 0.000977s | byte equality observed |
| adapt_tpl_adt.test | 10 | 0.000978s | 0.000980s | 0.000992s | byte equality observed |
| adapt_tpl_adt_empty.test | 10 | 0.000975s | 0.000985s | 0.000979s | byte equality observed |
| adapt_tpl_struct.test | 10 | 0.000956s | 0.000985s | 0.000995s | byte equality observed |
| adapt_tpl_struct_empty.test | 10 | 0.000957s | 0.000995s | 0.000978s | byte equality observed |
| adt_attribute_proxy.test | 10 | 0.000962s | 0.000964s | 0.000959s | byte equality observed |
| all.test | 10 | 0.000949s | 0.000970s | 0.000968s | byte equality observed |
| any.test | 10 | 0.000966s | 0.000958s | 0.000966s | byte equality observed |
| array.test | 10 | 0.000949s | 0.000980s | 0.000964s | byte equality observed |
| as_deque.test | 10 | 0.000956s | 0.000995s | 0.000981s | byte equality observed |
| as_list.test | 10 | 0.000996s | 0.001004s | 0.001000s | byte equality observed |
| as_map.test | 10 | 0.000983s | 0.001025s | 0.000993s | byte equality observed |
| as_map_assoc.test | 10 | 0.000981s | 0.001011s | 0.000977s | byte equality observed |
| as_set.test | 10 | 0.000988s | 0.001014s | 0.000991s | byte equality observed |
| as_vector.test | 10 | 0.000997s | 0.001027s | 0.001016s | byte equality observed |
| back_extended_deque.test | 10 | 0.000945s | 0.001002s | 0.000950s | byte equality observed |
| boost_array.test | 10 | 0.000971s | 0.000984s | 0.000979s | byte equality observed |
| boost_tuple.test | 10 | 0.000972s | 0.001024s | 0.000992s | byte equality observed |
| boost_tuple_iterator.test | 10 | 0.000955s | 0.000966s | 0.000955s | byte equality observed |
| clear.test | 10 | 0.000973s | 0.001019s | 0.000971s | byte equality observed |
| cons.test | 10 | 0.001001s | 0.001021s | 0.000991s | byte equality observed |
| convert_boost_tuple.test | 10 | 0.000962s | 0.000990s | 0.000974s | byte equality observed |
| convert_deque.test | 10 | 0.000957s | 0.000995s | 0.000957s | byte equality observed |
| convert_list.test | 10 | 0.000970s | 0.000978s | 0.000955s | byte equality observed |
| convert_std_pair.test | 10 | 0.000976s | 0.000990s | 0.000959s | byte equality observed |
| convert_std_tuple.test | 10 | 0.000967s | 0.000985s | 0.000946s | byte equality observed |
| convert_vector.test | 10 | 0.000940s | 0.000986s | 0.000946s | byte equality observed |
| copy.test | 10 | 0.000977s | 0.000971s | 0.000936s | byte equality observed |
| count.test | 10 | 0.000955s | 0.000979s | 0.000942s | byte equality observed |
| count_if.test | 10 | 0.000946s | 0.001009s | 0.000934s | byte equality observed |
| deduce_sequence.test | 10 | 0.000950s | 0.000972s | 0.000988s | byte equality observed |
| define_assoc_struct.test | 10 | 0.000969s | 0.000997s | 0.000970s | byte equality observed |
| define_assoc_struct_empty.test | 10 | 0.000975s | 0.000996s | 0.000986s | byte equality observed |
| define_assoc_struct_move.test | 10 | 0.000937s | 0.000988s | 0.000971s | byte equality observed |
| define_assoc_tpl_struct.test | 10 | 0.000975s | 0.001000s | 0.000975s | byte equality observed |
| define_assoc_tpl_struct_empty.test | 10 | 0.000972s | 0.001018s | 0.000968s | byte equality observed |
| define_assoc_tpl_struct_move.test | 10 | 0.000960s | 0.001000s | 0.000949s | byte equality observed |
| define_struct.test | 10 | 0.000969s | 0.001013s | 0.000959s | byte equality observed |
| define_struct_empty.test | 10 | 0.000973s | 0.001000s | 0.000961s | byte equality observed |
| define_struct_inline.test | 10 | 0.000983s | 0.001012s | 0.000969s | byte equality observed |
| define_struct_inline_empty.test | 10 | 0.000973s | 0.000999s | 0.000961s | byte equality observed |
| define_struct_inline_move.test | 10 | 0.000956s | 0.001006s | 0.000952s | byte equality observed |
| define_struct_move.test | 10 | 0.000958s | 0.000975s | 0.000972s | byte equality observed |
| define_tpl_struct.test | 10 | 0.000982s | 0.000999s | 0.000979s | byte equality observed |
| define_tpl_struct_empty.test | 10 | 0.000967s | 0.001005s | 0.000976s | byte equality observed |
| define_tpl_struct_inline.test | 10 | 0.001006s | 0.001017s | 0.000986s | byte equality observed |
| define_tpl_struct_inline_empty.test | 10 | 0.000963s | 0.001000s | 0.000972s | byte equality observed |
| define_tpl_struct_inline_move.test | 10 | 0.000952s | 0.000981s | 0.000948s | byte equality observed |
| define_tpl_struct_move.test | 10 | 0.000957s | 0.000979s | 0.000940s | byte equality observed |
| deque_comparison.test | 10 | 0.000951s | 0.000955s | 0.000954s | byte equality observed |
| deque_construction.test | 10 | 0.000952s | 0.000986s | 0.000953s | byte equality observed |
| deque_copy.test | 10 | 0.000931s | 0.000991s | 0.000953s | byte equality observed |
| deque_hash.test | 10 | 0.000956s | 0.000991s | 0.000941s | byte equality observed |
| deque_iterator.test | 10 | 0.000927s | 0.000970s | 0.000936s | byte equality observed |
| deque_make.test | 10 | 0.000943s | 0.000997s | 0.000940s | byte equality observed |
| deque_misc.test | 10 | 0.000950s | 0.000987s | 0.000950s | byte equality observed |
| deque_move.test | 10 | 0.000958s | 0.000998s | 0.000962s | byte equality observed |
| deque_mutate.test | 10 | 0.000955s | 0.000981s | 0.000938s | byte equality observed |
| deque_nest.test | 10 | 0.000948s | 0.000978s | 0.000970s | byte equality observed |
| deque_tie.test | 10 | 0.000943s | 0.000983s | 0.000947s | byte equality observed |
| deque_value_at.test | 10 | 0.000954s | 0.000961s | 0.000955s | byte equality observed |
| erase.test | 10 | 0.000981s | 0.001009s | 0.000985s | byte equality observed |
| erase_key.test | 10 | 0.001007s | 0.001008s | 0.000987s | byte equality observed |
| filter.test | 10 | 0.000975s | 0.000998s | 0.000961s | byte equality observed |
| filter_if.test | 10 | 0.000983s | 0.001031s | 0.000998s | byte equality observed |
| filter_view.test | 10 | 0.000973s | 0.001002s | 0.001008s | byte equality observed |
| find.test | 10 | 0.000993s | 0.000998s | 0.000984s | byte equality observed |
| find_if.test | 10 | 0.000974s | 0.000995s | 0.000996s | byte equality observed |
| flatten.test | 10 | 0.000961s | 0.000978s | 0.000950s | byte equality observed |
| flatten_view.test | 10 | 0.000958s | 0.000974s | 0.000985s | byte equality observed |
| fold.test | 10 | 0.000975s | 0.000992s | 0.000970s | byte equality observed |
| for_each.test | 10 | 0.000988s | 0.001018s | 0.000982s | byte equality observed |
| front_extended_deque.test | 10 | 0.000948s | 0.000967s | 0.000958s | byte equality observed |
| fused.test | 10 | 0.000958s | 0.000960s | 0.000975s | byte equality observed |
| fused_function_object.test | 10 | 0.000955s | 0.000974s | 0.000968s | byte equality observed |
| fused_procedure.test | 10 | 0.000947s | 0.000994s | 0.000943s | byte equality observed |
| github-176.test | 10 | 0.000943s | 0.000979s | 0.000969s | byte equality observed |
| hash.test | 10 | 0.000959s | 0.000986s | 0.000967s | byte equality observed |
| identity_view.test | 10 | 0.000969s | 0.001017s | 0.000992s | byte equality observed |
| insert.test | 10 | 0.000983s | 0.001030s | 0.000981s | byte equality observed |
| insert_range.test | 10 | 0.000983s | 0.001039s | 0.000990s | byte equality observed |
| invoke.test | 10 | 0.000946s | 0.000978s | 0.000949s | byte equality observed |
| invoke_function_object.test | 10 | 0.000961s | 0.000981s | 0.000963s | byte equality observed |
| invoke_procedure.test | 10 | 0.000956s | 0.000972s | 0.000960s | byte equality observed |
| io.test | 10 | 0.001073s | 0.001109s | 0.001096s | byte equality observed |
| iter_fold.test | 10 | 0.000969s | 0.000998s | 0.000996s | byte equality observed |
| iterator_range.test | 10 | 0.000994s | 0.001029s | 0.000996s | byte equality observed |
| join.test | 10 | 0.000953s | 0.000969s | 0.000948s | byte equality observed |
| joint_view.test | 10 | 0.001028s | 0.001073s | 0.001008s | byte equality observed |
| list_comparison.test | 10 | 0.000934s | 0.000971s | 0.000953s | byte equality observed |
| list_construction.test | 10 | 0.000968s | 0.000962s | 0.000965s | byte equality observed |
| list_copy.test | 10 | 0.000969s | 0.000981s | 0.000974s | byte equality observed |
| list_hash.test | 10 | 0.000958s | 0.001005s | 0.000964s | byte equality observed |
| list_iterator.test | 10 | 0.000963s | 0.000991s | 0.000954s | byte equality observed |
| list_make.test | 10 | 0.000977s | 0.000947s | 0.000958s | byte equality observed |
| list_misc.test | 10 | 0.000971s | 0.000953s | 0.000957s | byte equality observed |
| list_mutate.test | 10 | 0.000941s | 0.000957s | 0.000974s | byte equality observed |
| list_nest.test | 10 | 0.000942s | 0.000977s | 0.000954s | byte equality observed |
| list_tie.test | 10 | 0.000957s | 0.001005s | 0.000959s | byte equality observed |
| list_value_at.test | 10 | 0.000970s | 0.001012s | 0.000959s | byte equality observed |
| make_fused.test | 10 | 0.000948s | 0.000991s | 0.000977s | byte equality observed |
| make_fused_function_object.test | 10 | 0.000932s | 0.000961s | 0.000953s | byte equality observed |
| make_fused_procedure.test | 10 | 0.000956s | 0.000965s | 0.000976s | byte equality observed |
| make_list.test | 10 | 0.000978s | 0.000978s | 0.000963s | byte equality observed |
| make_unfused.test | 10 | 0.000955s | 0.000974s | 0.000985s | byte equality observed |
| make_vector.test | 10 | 0.000975s | 0.000939s | 0.000972s | byte equality observed |
| map.test | 10 | 0.000995s | 0.000994s | 0.000991s | byte equality observed |
| map_comparison.test | 10 | 0.000954s | 0.000965s | 0.000947s | byte equality observed |
| map_construction.test | 10 | 0.000935s | 0.000963s | 0.000941s | byte equality observed |
| map_copy.test | 10 | 0.000945s | 0.000968s | 0.000973s | byte equality observed |
| map_misc.test | 10 | 0.000961s | 0.000985s | 0.000948s | byte equality observed |
| map_move.test | 10 | 0.000987s | 0.000999s | 0.000984s | byte equality observed |
| map_mutate.test | 10 | 0.000947s | 0.000979s | 0.000974s | byte equality observed |
| map_tie.test | 10 | 0.000963s | 0.000985s | 0.000968s | byte equality observed |
| move.test | 10 | 0.000958s | 0.000966s | 0.000962s | byte equality observed |
| nil.test | 10 | 0.000958s | 0.000974s | 0.000970s | byte equality observed |
| none.test | 10 | 0.000952s | 0.000959s | 0.000964s | byte equality observed |
| nview.test | 10 | 0.000962s | 0.000989s | 0.000985s | byte equality observed |
| pop_back.test | 10 | 0.000999s | 0.001008s | 0.000997s | byte equality observed |
| pop_front.test | 10 | 0.001003s | 0.000996s | 0.000981s | byte equality observed |
| push_back.test | 10 | 0.000998s | 0.001005s | 0.000999s | byte equality observed |
| push_front.test | 10 | 0.000990s | 0.000996s | 0.000994s | byte equality observed |
| quick.test | 10 | 0.000441s | 0.000934s | 0.000937s | byte equality observed |
| ref_vector.test | 10 | 0.000980s | 0.000975s | 0.000975s | byte equality observed |
| remove.test | 10 | 0.000993s | 0.000989s | 0.001015s | byte equality observed |
| remove_if.test | 10 | 0.001013s | 0.000994s | 0.001005s | byte equality observed |
| repetitive_view.test | 10 | 0.000945s | 0.000950s | 0.000955s | byte equality observed |
| replace.test | 10 | 0.000975s | 0.001010s | 0.000978s | byte equality observed |
| replace_if.test | 10 | 0.000980s | 0.001013s | 0.000993s | byte equality observed |
| reverse.test | 10 | 0.000994s | 0.001027s | 0.000977s | byte equality observed |
| reverse_fold.test | 10 | 0.000962s | 0.000998s | 0.000998s | byte equality observed |
| reverse_iter_fold.test | 10 | 0.000966s | 0.000991s | 0.000973s | byte equality observed |
| reverse_view.test | 10 | 0.000974s | 0.001019s | 0.000994s | byte equality observed |
| segmented_find.test | 10 | 0.000955s | 0.000967s | 0.000974s | byte equality observed |
| segmented_find_if.test | 10 | 0.000957s | 0.000985s | 0.000952s | byte equality observed |
| segmented_fold.test | 10 | 0.000997s | 0.001014s | 0.000970s | byte equality observed |
| segmented_for_each.test | 10 | 0.000992s | 0.000998s | 0.000991s | byte equality observed |
| segmented_iterator_range.test | 10 | 0.000999s | 0.001016s | 0.001002s | byte equality observed |
| set.test | 10 | 0.001000s | 0.001003s | 0.000987s | byte equality observed |
| single_view.test | 10 | 0.000995s | 0.001014s | 0.000993s | byte equality observed |
| std_array.test | 10 | 0.000954s | 0.000969s | 0.000965s | byte equality observed |
| std_pair.test | 10 | 0.000995s | 0.001007s | 0.001006s | byte equality observed |
| std_tuple.test | 10 | 0.000958s | 0.000997s | 0.000987s | byte equality observed |
| std_tuple_iterator.test | 10 | 0.000975s | 0.000979s | 0.000967s | byte equality observed |
| swap.test | 10 | 0.000957s | 0.000977s | 0.000956s | byte equality observed |
| transform.test | 10 | 0.000983s | 0.001031s | 0.000994s | byte equality observed |
| transform_view.test | 10 | 0.000991s | 0.001018s | 0.001019s | byte equality observed |
| tuple_comparison.test | 10 | 0.000957s | 0.000995s | 0.000981s | byte equality observed |
| tuple_construction.test | 10 | 0.000974s | 0.000973s | 0.000951s | byte equality observed |
| tuple_conversion.test | 10 | 0.000977s | 0.000955s | 0.000976s | byte equality observed |
| tuple_copy.test | 10 | 0.000971s | 0.000981s | 0.000972s | byte equality observed |
| tuple_element.test | 10 | 0.000963s | 0.000989s | 0.000973s | byte equality observed |
| tuple_hash.test | 10 | 0.000985s | 0.000964s | 0.000963s | byte equality observed |
| tuple_make.test | 10 | 0.000971s | 0.000982s | 0.000955s | byte equality observed |
| tuple_misc.test | 10 | 0.000960s | 0.000971s | 0.000958s | byte equality observed |
| tuple_mutate.test | 10 | 0.000960s | 0.000973s | 0.000964s | byte equality observed |
| tuple_nest.test | 10 | 0.000960s | 0.000978s | 0.000988s | byte equality observed |
| tuple_tie.test | 10 | 0.000937s | 0.000969s | 0.000976s | byte equality observed |
| tuple_traits__maybe_variadic.test | 10 | 0.000947s | 0.000957s | 0.000976s | byte equality observed |
| tuple_traits__no_variadic.test | 10 | 0.000945s | 0.000970s | 0.001001s | byte equality observed |
| unfused.test | 10 | 0.000956s | 0.000972s | 0.000992s | byte equality observed |
| unfused_typed.test | 10 | 0.000963s | 0.000957s | 0.000971s | byte equality observed |
| vector_comparison.test | 10 | 0.000952s | 0.000957s | 0.000973s | byte equality observed |
| vector_construction.test | 10 | 0.000937s | 0.000967s | 0.000974s | byte equality observed |
| vector_conversion.test | 10 | 0.000951s | 0.000958s | 0.000959s | byte equality observed |
| vector_copy.test | 10 | 0.000946s | 0.000999s | 0.000962s | byte equality observed |
| vector_hash.test | 10 | 0.000959s | 0.000983s | 0.000968s | byte equality observed |
| vector_iterator.test | 10 | 0.000948s | 0.000992s | 0.000950s | byte equality observed |
| vector_make.test | 10 | 0.000954s | 0.000983s | 0.000962s | byte equality observed |
| vector_misc.test | 10 | 0.000931s | 0.000971s | 0.000956s | byte equality observed |
| vector_move.test | 10 | 0.000959s | 0.000985s | 0.000988s | byte equality observed |
| vector_mutate.test | 10 | 0.000947s | 0.000967s | 0.000966s | byte equality observed |
| vector_n.test | 10 | 0.000984s | 0.000992s | 0.000998s | byte equality observed |
| vector_nest.test | 10 | 0.000961s | 0.000964s | 0.000967s | byte equality observed |
| vector_tie.test | 10 | 0.000967s | 0.000976s | 0.000966s | byte equality observed |
| vector_traits__maybe_variadic.test | 10 | 0.000961s | 0.001028s | 0.000982s | byte equality observed |
| vector_traits__no_variadic.test | 10 | 0.000961s | 0.001021s | 0.000966s | byte equality observed |
| vector_value_at.test | 10 | 0.000963s | 0.000977s | 0.000967s | byte equality observed |
| zip.test | 10 | 0.000966s | 0.000978s | 0.000980s | byte equality observed |
| zip2.test | 10 | 0.000942s | 0.000960s | 0.000963s | byte equality observed |
| zip_ignore.test | 10 | 0.000957s | 0.000984s | 0.000950s | byte equality observed |
| zip_view.test | 10 | 0.000946s | 0.000954s | 0.000957s | byte equality observed |
| zip_view2.test | 10 | 0.000953s | 0.000998s | 0.000948s | byte equality observed |
| zip_view_ignore.test | 10 | 0.000944s | 0.000979s | 0.000950s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
