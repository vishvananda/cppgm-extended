# libs/fusion/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| all.test | RUN | 0.399376 | 0.735969 | 1.012897 | 0.001331 | 0.001621 | 0.001562 |
| any.test | RUN | 0.386839 | 0.707326 | 0.996310 | 0.001792 | 0.001538 | 0.001844 |
| clear.test | RUN | 0.278478 | 0.638591 | 0.807662 | 0.001721 | 0.001814 | 0.001805 |
| copy.test | RUN | 0.242694 | 0.475548 | 0.653094 | 0.001771 | 0.001934 | 0.001675 |
| count.test | RUN | 0.269002 | 0.527863 | 0.716236 | 0.001892 | 0.001934 | 0.001703 |
| count_if.test | RUN | 0.256126 | 0.496133 | 0.665678 | 0.001939 | 0.001639 | 0.001743 |
| erase.test | RUN | 0.347518 | 0.869423 | 1.080913 | 0.001714 | 0.001853 | 0.001788 |
| erase_key.test | RUN | 0.341651 | 0.840472 | 1.006347 | 0.001756 | 0.001706 | 0.001846 |
| filter.test | RUN | 0.313594 | 0.710274 | 0.935827 | 0.001683 | 0.001916 | 0.001828 |
| filter_if.test | RUN | 0.344239 | 0.857932 | 1.085853 | 0.001657 | 0.001737 | 0.001865 |
| find.test | RUN | 0.307646 | 0.607984 | 0.801475 | 0.001497 | 0.001780 | 0.001956 |
| find_if.test | RUN | 0.293681 | 0.540775 | 0.795015 | 0.001830 | 0.001749 | 0.001696 |
| fold.test | RUN | 0.315760 | 0.675060 | 0.930211 | 0.001896 | 0.001880 | 0.001816 |
| for_each.test | RUN | 0.288228 | 0.608049 | 0.798382 | 0.001555 | 0.001864 | 0.001955 |
| insert.test | RUN | 0.393863 | 1.087536 | 1.310063 | 0.001709 | 0.001903 | 0.001806 |
| insert_range.test | RUN | 0.398399 | 1.088655 | 1.329865 | 0.001598 | 0.001665 | 0.001761 |
| iter_fold.test | RUN | 0.476872 | 1.027152 | 1.573133 | 0.001804 | 0.001960 | 0.001865 |
| move.test | RUN | 0.250708 | 0.496211 | 0.686594 | 0.001768 | 0.001789 | 0.001648 |
| none.test | RUN | 0.407219 | 0.698469 | 1.050780 | 0.001596 | 0.001933 | 0.001562 |
| pop_back.test | RUN | 0.376633 | 0.942908 | 1.185937 | 0.001825 | 0.001908 | 0.001922 |
| pop_front.test | RUN | 0.324477 | 0.770398 | 0.934910 | 0.001875 | 0.001900 | 0.001758 |
| push_back.test | RUN | 0.382094 | 1.086879 | 1.260066 | 0.001900 | 0.001925 | 0.001697 |
| push_front.test | RUN | 0.350967 | 0.906217 | 1.066774 | 0.001829 | 0.001870 | 0.001872 |
| remove.test | RUN | 0.383720 | 1.014290 | 1.290278 | 0.001870 | 0.001786 | 0.001732 |
| remove_if.test | RUN | 0.351781 | 0.855954 | 1.096696 | 0.001906 | 0.001491 | 0.001495 |
| replace.test | RUN | 0.338392 | 0.824349 | 1.029294 | 0.001861 | 0.001932 | 0.001846 |
| replace_if.test | RUN | 0.330231 | 0.837472 | 1.004117 | 0.001889 | 0.002048 | 0.001763 |
| reverse_fold.test | RUN | 0.516058 | 1.170373 | 1.742595 | 0.001826 | 0.001721 | 0.001733 |
| reverse_iter_fold.test | RUN | 0.541966 | 1.209222 | 1.836036 | 0.001819 | 0.001808 | 0.001552 |
| reverse.test | RUN | 0.328926 | 0.857817 | 1.033781 | 0.001794 | 0.002202 | 0.001472 |
| segmented_for_each.test | RUN | 0.546401 | 1.640375 | 2.091676 | 0.001951 | 0.001868 | 0.001734 |
| segmented_find.test | RUN | 0.352777 | 0.857158 | 1.187445 | 0.001794 | 0.001678 | 0.001809 |
| segmented_find_if.test | RUN | 0.360706 | 0.865317 | 1.198572 | 0.001656 | 0.001804 | 0.001855 |
| segmented_fold.test | RUN | 0.453193 | 1.132785 | 1.464645 | 0.002178 | 0.001963 | 0.001840 |
| transform.test | RUN | 0.341498 | 0.891989 | 1.068712 | 0.001822 | 0.001926 | 0.001816 |
| join.test | RUN | 0.269001 | 0.548711 | 0.782713 | 0.001360 | 0.001813 | 0.001847 |
| zip.test | RUN | 0.466543 | 1.040945 | 1.437653 | 0.001869 | 0.001873 | 0.001830 |
| zip2.test | RUN | 0.441199 | 0.994762 | 1.382165 | 0.001698 | 0.001762 | 0.001887 |
| zip_ignore.test | RUN | 0.348306 | 0.713050 | 0.936657 | 0.001510 | 0.001631 | 0.001900 |
| flatten.test | RUN | 0.326514 | 0.753778 | 0.949002 | 0.001762 | 0.001716 | 0.001988 |
| ticket-5490.test | COMPILE | 0.183658 | 0.334366 | 0.388439 | — | — | — |
| as_deque.test | RUN | 0.322608 | 0.725833 | 1.007550 | 0.001577 | 0.001850 | 0.001823 |
| as_list.test | RUN | 0.351747 | 0.891856 | 1.089093 | 0.001384 | 0.001676 | 0.001617 |
| as_map.test | RUN | 0.345974 | 0.862095 | 1.089177 | 0.001750 | 0.001894 | 0.001736 |
| as_map_assoc.test | RUN | 0.352259 | 0.806824 | 1.014769 | 0.001627 | 0.001625 | 0.001759 |
| as_set.test | RUN | 0.389650 | 1.009445 | 1.263951 | 0.001720 | 0.001788 | 0.001776 |
| as_vector.test | RUN | 0.353127 | 0.907740 | 1.122357 | 0.001786 | 0.001889 | 0.001614 |
| boost_tuple.test | RUN | 0.354033 | 0.905541 | 1.100788 | 0.001750 | 0.001839 | 0.001715 |
| boost_tuple_iterator.test | RUN | 0.250349 | 0.540581 | 0.684279 | 0.001483 | 0.001654 | 0.001627 |
| cons.test | RUN | 0.443070 | 1.009562 | 1.300739 | 0.001774 | 0.001718 | 0.001804 |
| convert_boost_tuple.test | RUN | 0.292752 | 0.762268 | 0.854812 | 0.001770 | 0.001760 | 0.001780 |
| convert_deque.test | RUN | 0.298657 | 0.770556 | 0.856448 | 0.001751 | 0.001624 | 0.001717 |
| convert_list.test | RUN | 0.288254 | 0.789054 | 0.864407 | 0.001669 | 0.001796 | 0.001545 |
| convert_std_pair.test | RUN | 0.311633 | 0.769003 | 0.873520 | 0.001815 | 0.001865 | 0.001458 |
| convert_std_tuple.test | RUN | 0.292141 | 0.749979 | 0.839096 | 0.001576 | 0.001644 | 0.001639 |
| convert_vector.test | RUN | 0.294580 | 0.767471 | 0.862127 | 0.001399 | 0.001564 | 0.001695 |
| filter_view.test | RUN | 0.352134 | 0.804254 | 1.031049 | 0.001914 | 0.001813 | 0.001933 |
| hash.test | RUN | 0.346378 | 0.724270 | 0.991752 | 0.001536 | 0.001864 | 0.001798 |
| io.test | RUN | 0.343053 | 1.031051 | 1.157965 | 0.001664 | 0.002060 | 0.001974 |
| iterator_range.test | RUN | 0.319581 | 0.769018 | 0.948110 | 0.001749 | 0.002023 | 0.001877 |
| joint_view.test | RUN | 0.426502 | 1.318153 | 1.437470 | 0.001851 | 0.001919 | 0.001954 |
| list_comparison.test | RUN | 0.260238 | 0.552269 | 0.675567 | 0.001723 | 0.001790 | 0.001533 |
| list_construction.test | RUN | 0.257098 | 0.504545 | 0.683767 | 0.001588 | 0.001688 | 0.001761 |
| list_copy.test | RUN | 0.365524 | 0.942394 | 1.191923 | 0.001778 | 0.001571 | 0.001840 |
| list_iterator.test | RUN | 0.253790 | 0.519242 | 0.670724 | 0.001649 | 0.001930 | 0.001777 |
| list_hash.test | RUN | 0.337588 | 1.010028 | 0.972654 | 0.001588 | 0.001642 | 0.001505 |
| list_make.test | RUN | 0.269448 | 0.581423 | 0.741575 | 0.001976 | 0.001775 | 0.001550 |
| list_misc.test | RUN | 0.345126 | 0.687896 | 0.909458 | 0.001836 | 0.001851 | 0.002028 |
| list_mutate.test | RUN | 0.241874 | 0.445966 | 0.613481 | 0.001226 | 0.001913 | 0.001809 |
| list_nest.test | RUN | 0.468502 | 1.308546 | 1.645031 | 0.001547 | 0.001840 | 0.001746 |
| list_tie.test | RUN | 0.253104 | 0.503607 | 0.708661 | 0.001562 | 0.001857 | 0.001884 |
| list_value_at.test | RUN | 0.251862 | 0.512281 | 0.634266 | 0.001653 | 0.001820 | 0.001860 |
| deque_comparison.test | RUN | 0.265401 | 0.562095 | 0.697024 | 0.001337 | 0.001490 | 0.001600 |
| deque_construction.test | RUN | 0.252221 | 0.539804 | 0.718632 | 0.001807 | 0.001904 | 0.001832 |
| deque_copy.test | RUN | 0.354051 | 0.897263 | 1.093122 | 0.001568 | 0.001617 | 0.001928 |
| deque_iterator.test | RUN | 0.249920 | 0.554207 | 0.706986 | 0.001516 | 0.001797 | 0.001612 |
| deque_hash.test | RUN | 0.351900 | 0.985482 | 0.967553 | 0.001605 | 0.001930 | 0.001578 |
| deque_is_constructible.test | COMPILE | 0.145011 | 0.240402 | 0.321288 | — | — | — |
| deque_make.test | RUN | 0.257970 | 0.547393 | 0.710647 | 0.001746 | 0.001912 | 0.001798 |
| deque_misc.test | RUN | 0.329922 | 0.686942 | 0.899521 | 0.001581 | 0.001576 | 0.001482 |
| deque_move.test | RUN | 0.262062 | 0.558520 | 0.714890 | 0.001776 | 0.001685 | 0.001661 |
| deque_mutate.test | RUN | 0.240684 | 0.458219 | 0.594485 | 0.001605 | 0.001879 | 0.001766 |
| deque_nest.test | RUN | 0.672313 | 2.056693 | 2.648944 | 0.001822 | 0.001577 | 0.001628 |
| deque_tie.test | RUN | 0.252695 | 0.488508 | 0.649848 | 0.001380 | 0.001546 | 0.001578 |
| deque_value_at.test | RUN | 0.240423 | 0.478966 | 0.640928 | 0.001690 | 0.001602 | 0.001532 |
| front_extended_deque.test | RUN | 0.347165 | 0.719248 | 0.986686 | 0.001733 | 0.001934 | 0.001803 |
| back_extended_deque.test | RUN | 0.351315 | 0.707540 | 0.942762 | 0.001780 | 0.001847 | 0.001848 |
| make_list.test | RUN | 0.273235 | 0.571873 | 0.752606 | 0.001705 | 0.001834 | 0.001479 |
| make_vector.test | RUN | 0.275582 | 0.593693 | 0.781861 | 0.001666 | 0.001833 | 0.001636 |
| map.test | RUN | 0.341662 | 0.857930 | 1.027416 | 0.001765 | 0.001885 | 0.001790 |
| map_comparison.test | RUN | 0.264220 | 0.567533 | 0.738693 | 0.001773 | 0.001860 | 0.001868 |
| map_construction.test | RUN | 0.268098 | 0.534757 | 0.709443 | 0.001796 | 0.001820 | 0.001657 |
| map_copy.test | RUN | 0.317889 | 0.687491 | 0.885429 | 0.001728 | 0.001965 | 0.001772 |
| map_misc.test | RUN | 0.333524 | 0.681346 | 0.872657 | 0.001631 | 0.001713 | 0.001537 |
| map_move.test | RUN | 0.269008 | 0.561861 | 0.703923 | 0.001680 | 0.001703 | 0.001641 |
| map_mutate.test | RUN | 0.239648 | 0.486176 | 0.624382 | 0.001500 | 0.001803 | 0.001515 |
| map_tie.test | RUN | 0.250622 | 0.460259 | 0.613959 | 0.001623 | 0.001789 | 0.001801 |
| nil.test | RUN | 0.175520 | 0.324457 | 0.437233 | 0.001826 | 0.001891 | 0.001774 |
| nview.test | RUN | 0.329176 | 0.673378 | 0.939209 | 0.001873 | 0.001941 | 0.001877 |
| reverse_view.test | RUN | 0.396224 | 1.109725 | 1.325553 | 0.001913 | 0.001756 | 0.001903 |
| segmented_iterator_range.test | RUN | 0.497372 | 1.333957 | 1.654438 | 0.002064 | 0.001752 | 0.001958 |
| set.test | RUN | 0.319738 | 0.736189 | 0.939442 | 0.001643 | 0.001654 | 0.001926 |
| single_view.test | RUN | 0.293341 | 0.704335 | 0.856693 | 0.001764 | 0.001617 | 0.001663 |
| std_pair.test | RUN | 0.369889 | 0.869296 | 1.068844 | 0.001848 | 0.001802 | 0.001455 |
| boost_array.test | RUN | 0.318043 | 0.630200 | 0.833152 | 0.001677 | 0.001830 | 0.001708 |
| array.test | RUN | 0.318465 | 0.605315 | 0.806524 | 0.001620 | 0.001762 | 0.001746 |
| std_array.test | RUN | 0.317158 | 0.606257 | 0.812005 | 0.001764 | 0.001534 | 0.001661 |
| tuple_comparison.test | RUN | 0.277621 | 0.567575 | 0.760106 | 0.001859 | 0.001909 | 0.001630 |
| tuple_construction.test | RUN | 0.283028 | 0.572047 | 0.773534 | 0.001832 | 0.001896 | 0.001702 |
| tuple_conversion.test | RUN | 0.597170 | 1.750315 | 2.463678 | 0.001604 | 0.001688 | 0.001760 |
| tuple_copy.test | RUN | 0.402178 | 1.005339 | 1.333746 | 0.001808 | 0.001845 | 0.001806 |
| tuple_element.test | RUN | 0.259785 | 0.525681 | 0.683921 | 0.001458 | 0.001852 | 0.001626 |
| tuple_make.test | RUN | 0.285485 | 0.588746 | 0.755066 | 0.001755 | 0.001775 | 0.001559 |
| tuple_misc.test | RUN | 0.353880 | 0.716195 | 0.960369 | 0.001726 | 0.001777 | 0.001784 |
| tuple_mutate.test | RUN | 0.256631 | 0.491968 | 0.704150 | 0.001820 | 0.001932 | 0.001521 |
| tuple_nest.test | RUN | 0.967597 | 3.535155 | 4.586198 | 0.001816 | 0.001835 | 0.001885 |
| tuple_hash.test | RUN | 0.370735 | 0.995624 | 0.996172 | 0.001832 | 0.001820 | 0.001390 |
| tuple_tie.test | RUN | 0.271949 | 0.554560 | 0.740597 | 0.001719 | 0.001888 | 0.001648 |
| tuple_traits__maybe_variadic.test | RUN | 0.296351 | 0.652157 | 0.806702 | 0.001698 | 0.002094 | 0.001937 |
| tuple_traits__no_variadic.test | RUN | 0.332091 | 0.653562 | 0.829852 | 0.001848 | 0.001932 | 0.001844 |
| transform_view.test | RUN | 0.411501 | 1.114091 | 1.312237 | 0.001941 | 0.001961 | 0.001982 |
| identity_view.test | RUN | 0.384803 | 1.010102 | 1.232292 | 0.001722 | 0.001628 | 0.001832 |
| vector_comparison.test | RUN | 0.264646 | 0.566493 | 0.681392 | 0.001593 | 0.001615 | 0.001879 |
| vector_construction.test | RUN | 0.266847 | 0.515993 | 0.750316 | 0.001814 | 0.001880 | 0.001712 |
| vector_conversion.test | RUN | 0.700938 | 2.283863 | 3.123944 | 0.001865 | 0.001791 | 0.001743 |
| vector_copy.test | RUN | 0.385474 | 0.989056 | 1.280787 | 0.001706 | 0.001922 | 0.001964 |
| vector_iterator.test | RUN | 0.254850 | 0.548017 | 0.695047 | 0.001684 | 0.001619 | 0.001592 |
| vector_make.test | RUN | 0.270179 | 0.571539 | 0.724895 | 0.001459 | 0.001882 | 0.001847 |
| vector_misc.test | RUN | 0.332262 | 0.683456 | 0.925698 | 0.001672 | 0.001820 | 0.001677 |
| vector_move.test | RUN | 0.266881 | 0.578316 | 0.694607 | 0.001630 | 0.001824 | 0.001662 |
| vector_mutate.test | RUN | 0.238940 | 0.485256 | 0.620817 | 0.001517 | 0.001616 | 0.001938 |
| vector_n.test | RUN | 0.286469 | 0.587217 | 0.835967 | 0.001939 | 0.002001 | 0.001848 |
| vector_nest.test | RUN | 0.967441 | 3.759055 | 4.644934 | 0.001860 | 0.001835 | 0.001854 |
| vector_hash.test | RUN | 0.344658 | 0.968582 | 0.955404 | 0.001593 | 0.001794 | 0.001656 |
| vector_tie.test | RUN | 0.255362 | 0.513520 | 0.687781 | 0.001754 | 0.001655 | 0.001778 |
| vector_traits__maybe_variadic.test | RUN | 0.272492 | 0.627809 | 0.740264 | 0.001790 | 0.001855 | 0.001964 |
| vector_traits__no_variadic.test | RUN | 0.309164 | 0.636239 | 0.748010 | 0.001530 | 0.001860 | 0.001449 |
| vector_value_at.test | RUN | 0.243927 | 0.472430 | 0.652478 | 0.001654 | 0.001904 | 0.001702 |
| zip_view.test | RUN | 0.488118 | 1.150529 | 1.677233 | 0.001678 | 0.001834 | 0.001837 |
| zip_view2.test | RUN | 0.395049 | 0.922605 | 1.229326 | 0.001635 | 0.001646 | 0.001733 |
| zip_view_ignore.test | RUN | 0.373256 | 0.830003 | 1.111987 | 0.001820 | 0.001755 | 0.001576 |
| repetitive_view.test | RUN | 0.642544 | 1.495766 | 2.213497 | 0.001727 | 0.001830 | 0.001878 |
| deduce_sequence.test | RUN | 0.310905 | 0.661832 | 0.835480 | 0.001530 | 0.001939 | 0.001826 |
| adapt_adt_named.test | RUN | 0.363621 | 0.879629 | 1.034094 | 0.001675 | 0.001958 | 0.001977 |
| adapt_adt_named_empty.test | RUN | 0.327176 | 0.707463 | 0.885927 | 0.001836 | 0.001910 | 0.001902 |
| adapt_adt.test | RUN | 0.393158 | 1.172778 | 1.247314 | 0.001894 | 0.001949 | 0.001883 |
| adapt_adt_empty.test | RUN | 0.315772 | 0.705988 | 0.855742 | 0.001949 | 0.001888 | 0.001702 |
| adapt_assoc_adt_named.test | RUN | 0.407501 | 0.924591 | 1.143339 | 0.001664 | 0.001906 | 0.001632 |
| adapt_assoc_adt_named_empty.test | RUN | 0.325395 | 0.721122 | 0.862267 | 0.001872 | 0.001703 | 0.001776 |
| adapt_assoc_adt.test | RUN | 0.416376 | 0.962297 | 1.228910 | 0.001733 | 0.001691 | 0.001704 |
| adapt_assoc_adt_empty.test | RUN | 0.321622 | 0.699607 | 0.852878 | 0.001783 | 0.001924 | 0.001854 |
| adapt_assoc_struct_named.test | RUN | 0.406969 | 0.911969 | 1.122799 | 0.001472 | 0.001861 | 0.001570 |
| adapt_assoc_struct_named_empty.test | RUN | 0.325911 | 0.697860 | 0.870838 | 0.001854 | 0.001943 | 0.001532 |
| adapt_assoc_struct.test | RUN | 0.355550 | 0.842821 | 1.072632 | 0.001539 | 0.001747 | 0.001612 |
| adapt_assoc_struct_empty.test | RUN | 0.312795 | 0.687786 | 0.867541 | 0.001880 | 0.001734 | 0.001514 |
| adapt_assoc_tpl_adt.test | RUN | 0.420413 | 0.986473 | 1.223448 | 0.001641 | 0.002047 | 0.001707 |
| adapt_assoc_tpl_adt_empty.test | RUN | 0.319196 | 0.714231 | 0.883085 | 0.001786 | 0.001896 | 0.001868 |
| adapt_assoc_tpl_struct.test | RUN | 0.362707 | 0.806390 | 1.034659 | 0.001673 | 0.001890 | 0.001918 |
| adapt_assoc_tpl_struct_empty.test | RUN | 0.319016 | 0.695503 | 0.899137 | 0.001808 | 0.001389 | 0.001549 |
| adapt_struct_named.test | RUN | 0.362756 | 0.865409 | 1.021337 | 0.001928 | 0.001941 | 0.001764 |
| adapt_struct_named_empty.test | RUN | 0.321039 | 0.700369 | 0.878387 | 0.001634 | 0.001902 | 0.001788 |
| adapt_struct.test | RUN | 0.384539 | 0.976494 | 1.171426 | 0.001710 | 0.001814 | 0.001854 |
| adapt_struct_empty.test | RUN | 0.316953 | 0.712470 | 0.873539 | 0.001792 | 0.001961 | 0.001793 |
| adapt_tpl_adt.test | RUN | 0.404344 | 1.146474 | 1.221067 | 0.001609 | 0.001593 | 0.001601 |
| adapt_tpl_adt_empty.test | RUN | 0.329092 | 0.724009 | 0.885063 | 0.001623 | 0.001908 | 0.001948 |
| adapt_tpl_struct.test | RUN | 0.338793 | 0.789792 | 0.969063 | 0.001666 | 0.001708 | 0.001641 |
| adapt_tpl_struct_empty.test | RUN | 0.319183 | 0.736951 | 0.886275 | 0.001601 | 0.001856 | 0.001876 |
| adt_attribute_proxy.test | RUN | 0.371729 | 0.692237 | 0.916608 | 0.001720 | 0.001934 | 0.001734 |
| define_struct.test | RUN | 0.439849 | 0.976365 | 1.223753 | 0.001703 | 0.001872 | 0.001862 |
| define_struct_empty.test | RUN | 0.391320 | 0.788275 | 1.028658 | 0.001792 | 0.001677 | 0.001863 |
| define_struct_move.test | RUN | 0.259207 | 0.459810 | 0.615036 | 0.001674 | 0.001979 | 0.001466 |
| define_struct_inline.test | RUN | 0.455824 | 1.159740 | 1.382825 | 0.001774 | 0.001773 | 0.001840 |
| define_struct_inline_empty.test | RUN | 0.373979 | 0.800340 | 1.027064 | 0.001654 | 0.001758 | 0.001723 |
| define_struct_inline_move.test | RUN | 0.257888 | 0.468624 | 0.617601 | 0.001499 | 0.001522 | 0.001602 |
| define_assoc_struct.test | RUN | 0.418945 | 0.919735 | 1.164291 | 0.001788 | 0.001978 | 0.001892 |
| define_assoc_struct_empty.test | RUN | 0.376542 | 0.807897 | 1.037879 | 0.001809 | 0.001835 | 0.001817 |
| define_assoc_struct_move.test | RUN | 0.249060 | 0.443452 | 0.577414 | 0.001680 | 0.001800 | 0.001862 |
| define_tpl_struct.test | RUN | 0.408368 | 0.934213 | 1.163571 | 0.001694 | 0.001810 | 0.001605 |
| define_tpl_struct_empty.test | RUN | 0.381220 | 0.805039 | 0.989116 | 0.001889 | 0.001805 | 0.001831 |
| define_tpl_struct_move.test | RUN | 0.261139 | 0.473763 | 0.619779 | 0.001544 | 0.001669 | 0.001712 |
| define_tpl_struct_inline.test | RUN | 0.429276 | 1.103074 | 1.282812 | 0.001743 | 0.001613 | 0.001876 |
| define_tpl_struct_inline_empty.test | RUN | 0.384038 | 0.836554 | 1.011714 | 0.001555 | 0.001928 | 0.001916 |
| define_tpl_struct_inline_move.test | RUN | 0.258863 | 0.445858 | 0.623844 | 0.001900 | 0.001871 | 0.001832 |
| define_assoc_tpl_struct.test | RUN | 0.415253 | 0.935135 | 1.189447 | 0.001796 | 0.001949 | 0.001811 |
| define_assoc_tpl_struct_empty.test | RUN | 0.374259 | 0.817129 | 1.018128 | 0.001617 | 0.001761 | 0.001886 |
| define_assoc_tpl_struct_move.test | RUN | 0.261368 | 0.456296 | 0.600640 | 0.001683 | 0.001802 | 0.001815 |
| std_tuple.test | RUN | 0.256609 | 0.520529 | 0.687567 | 0.001573 | 0.001895 | 0.001597 |
| std_tuple_iterator.test | RUN | 0.255766 | 0.544063 | 0.642224 | 0.001847 | 0.001932 | 0.001760 |
| ref_vector.test | RUN | 0.292566 | 0.553780 | 0.727362 | 0.001669 | 0.001972 | 0.001888 |
| flatten_view.test | RUN | 0.307855 | 0.677767 | 0.876767 | 0.001872 | 0.001775 | 0.001700 |
| github-159.test | COMPILE | 0.144642 | 0.257309 | 0.302800 | — | — | — |
| github-176.test | RUN | 0.290902 | 0.602307 | 0.721061 | 0.001926 | 0.001920 | 0.001787 |
| size.test | COMPILE | 0.245423 | 0.435192 | 0.588103 | — | — | — |
| fused.test | RUN | 0.350321 | 0.656564 | 0.880933 | 0.001644 | 0.001920 | 0.001852 |
| fused_function_object.test | RUN | 0.258043 | 0.493767 | 0.652865 | 0.001635 | 0.001513 | 0.001904 |
| fused_procedure.test | RUN | 0.335694 | 0.621109 | 0.815583 | 0.001637 | 0.001814 | 0.001606 |
| unfused.test | RUN | 0.263529 | 0.501738 | 0.670923 | 0.001796 | 0.001842 | 0.001654 |
| unfused_typed.test | RUN | 0.262985 | 0.546094 | 0.689841 | 0.001615 | 0.001607 | 0.001920 |
| make_fused.test | RUN | 0.346824 | 0.639999 | 0.860259 | 0.001763 | 0.001367 | 0.001776 |
| make_fused_function_object.test | RUN | 0.260517 | 0.494993 | 0.668433 | 0.001632 | 0.001882 | 0.001344 |
| make_fused_procedure.test | RUN | 0.334266 | 0.613440 | 0.824262 | 0.001827 | 0.001886 | 0.001804 |
| make_unfused.test | RUN | 0.261939 | 0.507202 | 0.662814 | 0.001817 | 0.001908 | 0.001736 |
| invoke.test | RUN | 0.633807 | 1.791109 | 2.265000 | 0.001730 | 0.001703 | 0.001855 |
| invoke_function_object.test | RUN | 0.322383 | 0.623960 | 0.869487 | 0.001900 | 0.001890 | 0.001890 |
| invoke_procedure.test | RUN | 0.517450 | 1.310136 | 1.686140 | 0.001742 | 0.001877 | 0.001838 |
| swap.test | RUN | 0.396617 | 0.845013 | 1.111864 | 0.001578 | 0.001697 | 0.001780 |
| is_sequence.test | COMPILE | 0.075674 | 0.120417 | 0.138664 | — | — | — |
| is_view.test | COMPILE | 0.075676 | 0.121820 | 0.141233 | — | — | — |
| pair_deque.test | COMPILE | 0.147310 | 0.274342 | 0.339627 | — | — | — |
| pair_list.test | COMPILE | 0.147744 | 0.259055 | 0.332897 | — | — | — |
| pair_map.test | COMPILE | 0.153601 | 0.268541 | 0.320475 | — | — | — |
| pair_set.test | COMPILE | 0.154776 | 0.267634 | 0.330484 | — | — | — |
| pair_vector.test | COMPILE | 0.152893 | 0.269682 | 0.319156 | — | — | — |
| pair_nest.test | COMPILE | 0.060123 | 0.107509 | 0.117510 | — | — | — |
| index_sequence.test | COMPILE | 0.057766 | 0.078610 | 0.093566 | — | — | — |
| and.test | COMPILE | 0.045071 | 0.059071 | 0.070330 | — | — | — |
| tag_of.test | COMPILE | 0.074987 | 0.114707 | 0.122980 | — | — | — |
| unused.test | COMPILE | 0.035734 | 0.074964 | 0.078309 | — | — | — |
| tag_of_fallback.test | COMPILE | 0.074998 | 0.090151 | 0.105425 | — | — | — |
| make_pair_r-value.test | COMPILE | 0.053295 | 0.098740 | 0.113220 | — | — | — |
| quick.test | RUN | 0.190475 | 0.372744 | 0.438918 | 0.000562 | 0.001296 | 0.001245 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.004× | 1.004×, 1.003× | 0.995× |
| clang | 1.002× | 1.003×, 1.000× | 0.998× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adapt_adt.test | 10 | 0.001067s | 0.001075s | 0.001073s | byte equality observed |
| adapt_adt_empty.test | 10 | 0.001052s | 0.001037s | 0.001061s | byte equality observed |
| adapt_adt_named.test | 10 | 0.001063s | 0.001076s | 0.001049s | byte equality observed |
| adapt_adt_named_empty.test | 10 | 0.001052s | 0.001051s | 0.001054s | byte equality observed |
| adapt_assoc_adt.test | 10 | 0.001051s | 0.001063s | 0.001059s | byte equality observed |
| adapt_assoc_adt_empty.test | 10 | 0.001063s | 0.001049s | 0.001032s | byte equality observed |
| adapt_assoc_adt_named.test | 10 | 0.001061s | 0.001045s | 0.001046s | byte equality observed |
| adapt_assoc_adt_named_empty.test | 10 | 0.001057s | 0.001034s | 0.001056s | byte equality observed |
| adapt_assoc_struct.test | 10 | 0.001062s | 0.001045s | 0.001064s | byte equality observed |
| adapt_assoc_struct_empty.test | 10 | 0.001052s | 0.001079s | 0.001027s | byte equality observed |
| adapt_assoc_struct_named.test | 10 | 0.001047s | 0.001082s | 0.001076s | byte equality observed |
| adapt_assoc_struct_named_empty.test | 10 | 0.001063s | 0.001073s | 0.001054s | byte equality observed |
| adapt_assoc_tpl_adt.test | 10 | 0.001054s | 0.001083s | 0.001040s | byte equality observed |
| adapt_assoc_tpl_adt_empty.test | 10 | 0.001045s | 0.001064s | 0.001096s | byte equality observed |
| adapt_assoc_tpl_struct.test | 10 | 0.001088s | 0.001052s | 0.001069s | byte equality observed |
| adapt_assoc_tpl_struct_empty.test | 10 | 0.001051s | 0.001036s | 0.001053s | byte equality observed |
| adapt_struct.test | 10 | 0.001053s | 0.001042s | 0.001077s | byte equality observed |
| adapt_struct_empty.test | 10 | 0.001055s | 0.001019s | 0.001050s | byte equality observed |
| adapt_struct_named.test | 10 | 0.001040s | 0.001038s | 0.001063s | byte equality observed |
| adapt_struct_named_empty.test | 10 | 0.001041s | 0.001066s | 0.001053s | byte equality observed |
| adapt_tpl_adt.test | 10 | 0.001078s | 0.001098s | 0.001061s | byte equality observed |
| adapt_tpl_adt_empty.test | 10 | 0.001062s | 0.001057s | 0.001041s | byte equality observed |
| adapt_tpl_struct.test | 10 | 0.001063s | 0.001048s | 0.001051s | byte equality observed |
| adapt_tpl_struct_empty.test | 10 | 0.001053s | 0.001055s | 0.001036s | byte equality observed |
| adt_attribute_proxy.test | 10 | 0.001039s | 0.001038s | 0.001022s | byte equality observed |
| all.test | 10 | 0.001022s | 0.001033s | 0.001037s | byte equality observed |
| any.test | 10 | 0.001017s | 0.001012s | 0.001013s | byte equality observed |
| array.test | 10 | 0.001023s | 0.001043s | 0.001024s | byte equality observed |
| as_deque.test | 10 | 0.001017s | 0.001019s | 0.001027s | byte equality observed |
| as_list.test | 10 | 0.001071s | 0.001087s | 0.001080s | byte equality observed |
| as_map.test | 10 | 0.001064s | 0.001083s | 0.001042s | byte equality observed |
| as_map_assoc.test | 10 | 0.001052s | 0.001041s | 0.001042s | byte equality observed |
| as_set.test | 10 | 0.001072s | 0.001071s | 0.001058s | byte equality observed |
| as_vector.test | 10 | 0.001054s | 0.001074s | 0.001054s | byte equality observed |
| back_extended_deque.test | 10 | 0.001032s | 0.001018s | 0.001045s | byte equality observed |
| boost_array.test | 10 | 0.001022s | 0.001036s | 0.001056s | byte equality observed |
| boost_tuple.test | 10 | 0.001072s | 0.001076s | 0.001062s | byte equality observed |
| boost_tuple_iterator.test | 10 | 0.001021s | 0.001022s | 0.001033s | byte equality observed |
| clear.test | 10 | 0.001075s | 0.001084s | 0.001037s | byte equality observed |
| cons.test | 10 | 0.001066s | 0.001095s | 0.001084s | byte equality observed |
| convert_boost_tuple.test | 10 | 0.001044s | 0.001046s | 0.001040s | byte equality observed |
| convert_deque.test | 10 | 0.001070s | 0.001050s | 0.001030s | byte equality observed |
| convert_list.test | 10 | 0.001046s | 0.001037s | 0.001031s | byte equality observed |
| convert_std_pair.test | 10 | 0.001041s | 0.001047s | 0.001004s | byte equality observed |
| convert_std_tuple.test | 10 | 0.001052s | 0.001051s | 0.001018s | byte equality observed |
| convert_vector.test | 10 | 0.001052s | 0.001043s | 0.001038s | byte equality observed |
| copy.test | 10 | 0.001033s | 0.001018s | 0.001043s | byte equality observed |
| count.test | 10 | 0.001033s | 0.001040s | 0.001078s | byte equality observed |
| count_if.test | 10 | 0.001042s | 0.001071s | 0.001044s | byte equality observed |
| deduce_sequence.test | 10 | 0.001031s | 0.001035s | 0.001019s | byte equality observed |
| define_assoc_struct.test | 10 | 0.001059s | 0.001070s | 0.001056s | byte equality observed |
| define_assoc_struct_empty.test | 10 | 0.001045s | 0.001043s | 0.001019s | byte equality observed |
| define_assoc_struct_move.test | 10 | 0.001030s | 0.001026s | 0.001030s | byte equality observed |
| define_assoc_tpl_struct.test | 10 | 0.001042s | 0.001064s | 0.001043s | byte equality observed |
| define_assoc_tpl_struct_empty.test | 10 | 0.001058s | 0.001075s | 0.001039s | byte equality observed |
| define_assoc_tpl_struct_move.test | 10 | 0.001026s | 0.001027s | 0.001040s | byte equality observed |
| define_struct.test | 10 | 0.001054s | 0.001063s | 0.001039s | byte equality observed |
| define_struct_empty.test | 10 | 0.001051s | 0.001072s | 0.001045s | byte equality observed |
| define_struct_inline.test | 10 | 0.001054s | 0.001071s | 0.001042s | byte equality observed |
| define_struct_inline_empty.test | 10 | 0.001047s | 0.001055s | 0.001056s | byte equality observed |
| define_struct_inline_move.test | 10 | 0.001021s | 0.001041s | 0.001029s | byte equality observed |
| define_struct_move.test | 10 | 0.001021s | 0.001073s | 0.000994s | byte equality observed |
| define_tpl_struct.test | 10 | 0.001056s | 0.001051s | 0.001048s | byte equality observed |
| define_tpl_struct_empty.test | 10 | 0.001057s | 0.001064s | 0.001055s | byte equality observed |
| define_tpl_struct_inline.test | 10 | 0.001075s | 0.001041s | 0.001052s | byte equality observed |
| define_tpl_struct_inline_empty.test | 10 | 0.001051s | 0.001040s | 0.001045s | byte equality observed |
| define_tpl_struct_inline_move.test | 10 | 0.001021s | 0.001045s | 0.001020s | byte equality observed |
| define_tpl_struct_move.test | 10 | 0.001035s | 0.001038s | 0.001042s | byte equality observed |
| deque_comparison.test | 10 | 0.001029s | 0.001018s | 0.001018s | byte equality observed |
| deque_construction.test | 10 | 0.001030s | 0.001026s | 0.001032s | byte equality observed |
| deque_copy.test | 10 | 0.001036s | 0.001071s | 0.001017s | byte equality observed |
| deque_hash.test | 10 | 0.001036s | 0.001057s | 0.001024s | byte equality observed |
| deque_iterator.test | 10 | 0.001022s | 0.001032s | 0.001019s | byte equality observed |
| deque_make.test | 10 | 0.001051s | 0.001071s | 0.001038s | byte equality observed |
| deque_misc.test | 10 | 0.001027s | 0.001064s | 0.001029s | byte equality observed |
| deque_move.test | 10 | 0.001055s | 0.001043s | 0.001047s | byte equality observed |
| deque_mutate.test | 10 | 0.001027s | 0.001031s | 0.001034s | byte equality observed |
| deque_nest.test | 10 | 0.001068s | 0.001027s | 0.001029s | byte equality observed |
| deque_tie.test | 10 | 0.001018s | 0.001020s | 0.001001s | byte equality observed |
| deque_value_at.test | 9 | 0.001033s | 0.001045s | 0.001027s | byte equality observed |
| erase.test | 10 | 0.001060s | 0.001059s | 0.001050s | byte equality observed |
| erase_key.test | 10 | 0.001103s | 0.001078s | 0.001046s | byte equality observed |
| filter.test | 10 | 0.001055s | 0.001039s | 0.001029s | byte equality observed |
| filter_if.test | 10 | 0.001076s | 0.001047s | 0.001084s | byte equality observed |
| filter_view.test | 10 | 0.001051s | 0.001045s | 0.001089s | byte equality observed |
| find.test | 10 | 0.001064s | 0.001093s | 0.001094s | byte equality observed |
| find_if.test | 10 | 0.001057s | 0.001053s | 0.001070s | byte equality observed |
| flatten.test | 10 | 0.001050s | 0.001046s | 0.001044s | byte equality observed |
| flatten_view.test | 10 | 0.001046s | 0.001058s | 0.001011s | byte equality observed |
| fold.test | 10 | 0.001055s | 0.001062s | 0.001034s | byte equality observed |
| for_each.test | 10 | 0.001078s | 0.001079s | 0.001055s | byte equality observed |
| front_extended_deque.test | 10 | 0.001021s | 0.001009s | 0.001018s | byte equality observed |
| fused.test | 10 | 0.001022s | 0.001026s | 0.001023s | byte equality observed |
| fused_function_object.test | 10 | 0.001017s | 0.001024s | 0.001034s | byte equality observed |
| fused_procedure.test | 10 | 0.001024s | 0.001052s | 0.001044s | byte equality observed |
| github-176.test | 10 | 0.001027s | 0.001017s | 0.001017s | byte equality observed |
| hash.test | 10 | 0.001029s | 0.001040s | 0.001025s | byte equality observed |
| identity_view.test | 10 | 0.001059s | 0.001068s | 0.001109s | byte equality observed |
| insert.test | 10 | 0.001074s | 0.001073s | 0.001122s | byte equality observed |
| insert_range.test | 10 | 0.001087s | 0.001103s | 0.001073s | byte equality observed |
| invoke.test | 10 | 0.001028s | 0.001044s | 0.001093s | byte equality observed |
| invoke_function_object.test | 10 | 0.001026s | 0.001047s | 0.001081s | byte equality observed |
| invoke_procedure.test | 10 | 0.001039s | 0.001041s | 0.001040s | byte equality observed |
| io.test | 10 | 0.001168s | 0.001168s | 0.001163s | byte equality observed |
| iter_fold.test | 10 | 0.001070s | 0.001095s | 0.001066s | byte equality observed |
| iterator_range.test | 10 | 0.001069s | 0.001068s | 0.001087s | byte equality observed |
| join.test | 10 | 0.001025s | 0.001031s | 0.001032s | byte equality observed |
| joint_view.test | 10 | 0.001097s | 0.001102s | 0.001097s | byte equality observed |
| list_comparison.test | 10 | 0.001019s | 0.001024s | 0.001034s | byte equality observed |
| list_construction.test | 10 | 0.001040s | 0.001031s | 0.001045s | byte equality observed |
| list_copy.test | 10 | 0.001039s | 0.001036s | 0.001017s | byte equality observed |
| list_hash.test | 10 | 0.001059s | 0.001055s | 0.001017s | byte equality observed |
| list_iterator.test | 10 | 0.001031s | 0.001036s | 0.001033s | byte equality observed |
| list_make.test | 10 | 0.001040s | 0.001052s | 0.001021s | byte equality observed |
| list_misc.test | 10 | 0.001047s | 0.001074s | 0.001048s | byte equality observed |
| list_mutate.test | 10 | 0.001041s | 0.001020s | 0.001047s | byte equality observed |
| list_nest.test | 10 | 0.001034s | 0.001041s | 0.001067s | byte equality observed |
| list_tie.test | 10 | 0.001064s | 0.001024s | 0.001027s | byte equality observed |
| list_value_at.test | 10 | 0.001039s | 0.001028s | 0.001060s | byte equality observed |
| make_fused.test | 10 | 0.001030s | 0.001027s | 0.001075s | byte equality observed |
| make_fused_function_object.test | 10 | 0.001061s | 0.001025s | 0.001066s | byte equality observed |
| make_fused_procedure.test | 10 | 0.001052s | 0.001035s | 0.001045s | byte equality observed |
| make_list.test | 10 | 0.001043s | 0.001030s | 0.001035s | byte equality observed |
| make_unfused.test | 10 | 0.001018s | 0.001034s | 0.001052s | byte equality observed |
| make_vector.test | 10 | 0.001035s | 0.001067s | 0.001036s | byte equality observed |
| map.test | 10 | 0.001072s | 0.001082s | 0.001073s | byte equality observed |
| map_comparison.test | 10 | 0.001031s | 0.001031s | 0.001052s | byte equality observed |
| map_construction.test | 10 | 0.001035s | 0.001042s | 0.001028s | byte equality observed |
| map_copy.test | 10 | 0.001037s | 0.001024s | 0.001043s | byte equality observed |
| map_misc.test | 10 | 0.001035s | 0.001040s | 0.001053s | byte equality observed |
| map_move.test | 10 | 0.001057s | 0.001060s | 0.001048s | byte equality observed |
| map_mutate.test | 10 | 0.001024s | 0.001055s | 0.001033s | byte equality observed |
| map_tie.test | 10 | 0.001044s | 0.001005s | 0.001029s | byte equality observed |
| move.test | 10 | 0.001038s | 0.001044s | 0.001074s | byte equality observed |
| nil.test | 10 | 0.001037s | 0.001056s | 0.001056s | byte equality observed |
| none.test | 10 | 0.001017s | 0.001044s | 0.001057s | byte equality observed |
| nview.test | 10 | 0.001043s | 0.001063s | 0.001027s | byte equality observed |
| pop_back.test | 10 | 0.001091s | 0.001074s | 0.001061s | byte equality observed |
| pop_front.test | 10 | 0.001064s | 0.001081s | 0.001056s | byte equality observed |
| push_back.test | 10 | 0.001066s | 0.001099s | 0.001075s | byte equality observed |
| push_front.test | 10 | 0.001112s | 0.001094s | 0.001069s | byte equality observed |
| quick.test | 10 | 0.000480s | 0.001031s | 0.001025s | byte equality observed |
| ref_vector.test | 10 | 0.001050s | 0.001041s | 0.001032s | byte equality observed |
| remove.test | 10 | 0.001075s | 0.001067s | 0.001072s | byte equality observed |
| remove_if.test | 10 | 0.001074s | 0.001063s | 0.001076s | byte equality observed |
| repetitive_view.test | 10 | 0.001039s | 0.001029s | 0.001013s | byte equality observed |
| replace.test | 10 | 0.001058s | 0.001066s | 0.001049s | byte equality observed |
| replace_if.test | 10 | 0.001076s | 0.001094s | 0.001087s | byte equality observed |
| reverse.test | 10 | 0.001088s | 0.001093s | 0.001072s | byte equality observed |
| reverse_fold.test | 10 | 0.001067s | 0.001032s | 0.001092s | byte equality observed |
| reverse_iter_fold.test | 10 | 0.001059s | 0.001079s | 0.001100s | byte equality observed |
| reverse_view.test | 10 | 0.001072s | 0.001058s | 0.001053s | byte equality observed |
| segmented_find.test | 10 | 0.001038s | 0.001025s | 0.001032s | byte equality observed |
| segmented_find_if.test | 10 | 0.001046s | 0.001040s | 0.001033s | byte equality observed |
| segmented_fold.test | 10 | 0.001067s | 0.001057s | 0.001084s | byte equality observed |
| segmented_for_each.test | 10 | 0.001086s | 0.001068s | 0.001060s | byte equality observed |
| segmented_iterator_range.test | 10 | 0.001073s | 0.001120s | 0.001079s | byte equality observed |
| set.test | 10 | 0.001064s | 0.001048s | 0.001055s | byte equality observed |
| single_view.test | 10 | 0.001063s | 0.001064s | 0.001053s | byte equality observed |
| std_array.test | 10 | 0.001027s | 0.001029s | 0.001062s | byte equality observed |
| std_pair.test | 10 | 0.001071s | 0.001064s | 0.001074s | byte equality observed |
| std_tuple.test | 10 | 0.001033s | 0.001036s | 0.001019s | byte equality observed |
| std_tuple_iterator.test | 10 | 0.001024s | 0.000994s | 0.001055s | byte equality observed |
| swap.test | 10 | 0.001056s | 0.001012s | 0.001077s | byte equality observed |
| transform.test | 10 | 0.001064s | 0.001051s | 0.001083s | byte equality observed |
| transform_view.test | 10 | 0.001075s | 0.001102s | 0.001090s | byte equality observed |
| tuple_comparison.test | 10 | 0.001003s | 0.001008s | 0.001027s | byte equality observed |
| tuple_construction.test | 10 | 0.001006s | 0.001022s | 0.001035s | byte equality observed |
| tuple_conversion.test | 10 | 0.001016s | 0.001024s | 0.001044s | byte equality observed |
| tuple_copy.test | 10 | 0.001024s | 0.001054s | 0.001044s | byte equality observed |
| tuple_element.test | 10 | 0.001028s | 0.001008s | 0.001046s | byte equality observed |
| tuple_hash.test | 10 | 0.001030s | 0.001021s | 0.001072s | byte equality observed |
| tuple_make.test | 10 | 0.001042s | 0.001004s | 0.001038s | byte equality observed |
| tuple_misc.test | 10 | 0.001033s | 0.001038s | 0.001035s | byte equality observed |
| tuple_mutate.test | 10 | 0.001016s | 0.001068s | 0.001016s | byte equality observed |
| tuple_nest.test | 10 | 0.001024s | 0.001019s | 0.001031s | byte equality observed |
| tuple_tie.test | 10 | 0.001044s | 0.001020s | 0.001017s | byte equality observed |
| tuple_traits__maybe_variadic.test | 10 | 0.001033s | 0.001042s | 0.001017s | byte equality observed |
| tuple_traits__no_variadic.test | 10 | 0.001033s | 0.001008s | 0.000993s | byte equality observed |
| unfused.test | 10 | 0.001079s | 0.001043s | 0.000999s | byte equality observed |
| unfused_typed.test | 10 | 0.001033s | 0.001024s | 0.001042s | byte equality observed |
| vector_comparison.test | 10 | 0.001034s | 0.001011s | 0.001059s | byte equality observed |
| vector_construction.test | 10 | 0.001030s | 0.001020s | 0.001051s | byte equality observed |
| vector_conversion.test | 10 | 0.001038s | 0.001023s | 0.001030s | byte equality observed |
| vector_copy.test | 10 | 0.001072s | 0.001038s | 0.001047s | byte equality observed |
| vector_hash.test | 10 | 0.001033s | 0.001045s | 0.001060s | byte equality observed |
| vector_iterator.test | 10 | 0.001048s | 0.001030s | 0.001051s | byte equality observed |
| vector_make.test | 10 | 0.001038s | 0.001077s | 0.001047s | byte equality observed |
| vector_misc.test | 10 | 0.001052s | 0.001023s | 0.001041s | byte equality observed |
| vector_move.test | 10 | 0.001059s | 0.001058s | 0.001071s | byte equality observed |
| vector_mutate.test | 10 | 0.001034s | 0.000999s | 0.001041s | byte equality observed |
| vector_n.test | 10 | 0.001063s | 0.001064s | 0.001065s | byte equality observed |
| vector_nest.test | 10 | 0.001031s | 0.001054s | 0.001037s | byte equality observed |
| vector_tie.test | 10 | 0.001049s | 0.001043s | 0.001037s | byte equality observed |
| vector_traits__maybe_variadic.test | 10 | 0.001040s | 0.001037s | 0.001025s | byte equality observed |
| vector_traits__no_variadic.test | 10 | 0.001034s | 0.001056s | 0.001041s | byte equality observed |
| vector_value_at.test | 10 | 0.001055s | 0.001055s | 0.001030s | byte equality observed |
| zip.test | 10 | 0.001057s | 0.001055s | 0.001004s | byte equality observed |
| zip2.test | 10 | 0.001044s | 0.001032s | 0.001007s | byte equality observed |
| zip_ignore.test | 10 | 0.001039s | 0.001003s | 0.001010s | byte equality observed |
| zip_view.test | 10 | 0.001056s | 0.001020s | 0.001055s | byte equality observed |
| zip_view2.test | 10 | 0.001027s | 0.001021s | 0.001050s | byte equality observed |
| zip_view_ignore.test | 10 | 0.001036s | 0.001038s | 0.001033s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
