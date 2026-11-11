# libs/fusion/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| all.test | RUN | 0.355620 | 0.657497 | 0.804238 | 0.001627 | 0.001615 | 0.001710 |
| any.test | RUN | 0.346892 | 0.635679 | 0.773287 | 0.001772 | 0.001766 | 0.001771 |
| clear.test | RUN | 0.251176 | 0.588687 | 0.644453 | 0.001579 | 0.001953 | 0.001842 |
| copy.test | RUN | 0.217559 | 0.430742 | 0.522412 | 0.001698 | 0.001946 | 0.001869 |
| count.test | RUN | 0.246206 | 0.495068 | 0.563517 | 0.001634 | 0.002072 | 0.001873 |
| count_if.test | RUN | 0.229634 | 0.450715 | 0.524334 | 0.001929 | 0.001716 | 0.001892 |
| erase.test | RUN | 0.312867 | 0.814336 | 0.845915 | 0.001924 | 0.001886 | 0.001643 |
| erase_key.test | RUN | 0.303934 | 0.795322 | 0.798490 | 0.001610 | 0.001938 | 0.001973 |
| filter.test | RUN | 0.268264 | 0.669917 | 0.741675 | 0.001860 | 0.001961 | 0.002975 |
| filter_if.test | RUN | 0.303014 | 0.791778 | 0.971362 | 0.001672 | 0.001844 | 0.002469 |
| find.test | RUN | 0.269341 | 0.554850 | 0.752237 | 0.001794 | 0.002228 | 0.002112 |
| find_if.test | RUN | 0.254250 | 0.510824 | 0.727577 | 0.001830 | 0.002064 | 0.001744 |
| fold.test | RUN | 0.290323 | 0.615563 | 0.765939 | 0.001886 | 0.002082 | 0.001750 |
| for_each.test | RUN | 0.256344 | 0.563361 | 0.635372 | 0.001443 | 0.001998 | 0.002112 |
| insert.test | RUN | 0.362197 | 1.024745 | 1.009074 | 0.002140 | 0.002028 | 0.001848 |
| insert_range.test | RUN | 0.364954 | 1.032169 | 1.028958 | 0.001705 | 0.002072 | 0.001720 |
| iter_fold.test | RUN | 0.425808 | 0.996708 | 1.246532 | 0.001951 | 0.002060 | 0.002104 |
| move.test | RUN | 0.219099 | 0.474789 | 0.520114 | 0.001918 | 0.001730 | 0.001928 |
| none.test | RUN | 0.351987 | 0.654770 | 0.779527 | 0.001738 | 0.001972 | 0.001907 |
| pop_back.test | RUN | 0.331700 | 0.878374 | 0.974140 | 0.001908 | 0.002072 | 0.001660 |
| pop_front.test | RUN | 0.286872 | 0.703519 | 0.788944 | 0.002006 | 0.002014 | 0.001638 |
| push_back.test | RUN | 0.357310 | 0.974967 | 1.029856 | 0.001813 | 0.001940 | 0.001765 |
| push_front.test | RUN | 0.308208 | 0.807935 | 0.864951 | 0.001842 | 0.002082 | 0.001697 |
| remove.test | RUN | 0.351662 | 0.918379 | 1.030569 | 0.001680 | 0.001815 | 0.002050 |
| remove_if.test | RUN | 0.315245 | 0.777143 | 0.867560 | 0.001927 | 0.001992 | 0.001874 |
| replace.test | RUN | 0.292965 | 0.777733 | 0.772556 | 0.002096 | 0.002106 | 0.001870 |
| replace_if.test | RUN | 0.301481 | 0.764019 | 0.790162 | 0.001939 | 0.002170 | 0.002082 |
| reverse_fold.test | RUN | 0.466146 | 1.112350 | 1.401335 | 0.001740 | 0.001910 | 0.001772 |
| reverse_iter_fold.test | RUN | 0.469387 | 1.164639 | 1.451255 | 0.001856 | 0.001664 | 0.002008 |
| reverse.test | RUN | 0.297567 | 0.805960 | 0.816294 | 0.001765 | 0.001998 | 0.001722 |
| segmented_for_each.test | RUN | 0.503114 | 1.548472 | 1.725387 | 0.002063 | 0.001862 | 0.001786 |
| segmented_find.test | RUN | 0.349427 | 0.790468 | 0.978923 | 0.001526 | 0.001830 | 0.001798 |
| segmented_find_if.test | RUN | 0.340036 | 0.800992 | 0.961361 | 0.002034 | 0.001602 | 0.001633 |
| segmented_fold.test | RUN | 0.409910 | 1.066020 | 1.221830 | 0.001846 | 0.002083 | 0.001992 |
| transform.test | RUN | 0.316949 | 0.827113 | 0.887585 | 0.001928 | 0.001842 | 0.001979 |
| join.test | RUN | 0.239584 | 0.506977 | 0.589203 | 0.002117 | 0.001842 | 0.001917 |
| zip.test | RUN | 0.416308 | 0.941477 | 1.164535 | 0.001594 | 0.001887 | 0.001846 |
| zip2.test | RUN | 0.402421 | 0.920246 | 1.130548 | 0.001704 | 0.001841 | 0.001860 |
| zip_ignore.test | RUN | 0.300753 | 0.584434 | 0.727662 | 0.001638 | 0.001615 | 0.001687 |
| flatten.test | RUN | 0.286155 | 0.691628 | 0.735549 | 0.001760 | 0.002015 | 0.001908 |
| ticket-5490.test | COMPILE | 0.152925 | 0.278567 | 0.323635 | — | — | — |
| as_deque.test | RUN | 0.287257 | 0.644847 | 0.832848 | 0.002118 | 0.001989 | 0.001742 |
| as_list.test | RUN | 0.309685 | 0.798926 | 0.880342 | 0.001848 | 0.001942 | 0.001912 |
| as_map.test | RUN | 0.307942 | 0.782856 | 0.858853 | 0.001692 | 0.001924 | 0.001570 |
| as_map_assoc.test | RUN | 0.309639 | 0.745500 | 0.828676 | 0.001513 | 0.001909 | 0.001823 |
| as_set.test | RUN | 0.339553 | 0.901532 | 1.024359 | 0.002168 | 0.001834 | 0.001849 |
| as_vector.test | RUN | 0.314484 | 0.796008 | 0.889507 | 0.001770 | 0.001686 | 0.001581 |
| boost_tuple.test | RUN | 0.307343 | 0.808293 | 0.841396 | 0.001932 | 0.001645 | 0.001888 |
| boost_tuple_iterator.test | RUN | 0.219145 | 0.466569 | 0.515289 | 0.001461 | 0.001505 | 0.001683 |
| cons.test | RUN | 0.388685 | 0.924809 | 1.023352 | 0.001468 | 0.001807 | 0.002058 |
| convert_boost_tuple.test | RUN | 0.249502 | 0.674882 | 0.678996 | 0.001878 | 0.003237 | 0.001932 |
| convert_deque.test | RUN | 0.252210 | 0.657934 | 0.672986 | 0.001562 | 0.002906 | 0.001464 |
| convert_list.test | RUN | 0.255222 | 0.683555 | 0.684845 | 0.001504 | 0.001918 | 0.002068 |
| convert_std_pair.test | RUN | 0.265660 | 0.685583 | 0.688380 | 0.001478 | 0.001773 | 0.001334 |
| convert_std_tuple.test | RUN | 0.252482 | 0.665531 | 0.676348 | 0.001586 | 0.001958 | 0.001902 |
| convert_vector.test | RUN | 0.253407 | 0.675391 | 0.666327 | 0.001465 | 0.001784 | 0.001726 |
| filter_view.test | RUN | 0.293726 | 0.737644 | 0.798451 | 0.001907 | 0.001720 | 0.001980 |
| hash.test | RUN | 0.305280 | 0.647137 | 0.774185 | 0.001695 | 0.003667 | 0.001636 |
| io.test | RUN | 0.307985 | 0.953287 | 0.915592 | 0.001632 | 0.002094 | 0.001789 |
| iterator_range.test | RUN | 0.288300 | 0.732621 | 0.770704 | 0.001606 | 0.001986 | 0.001889 |
| joint_view.test | RUN | 0.362657 | 1.191576 | 1.160051 | 0.001644 | 0.001935 | 0.001804 |
| list_comparison.test | RUN | 0.240864 | 0.484907 | 0.545867 | 0.001737 | 0.001642 | 0.001529 |
| list_construction.test | RUN | 0.234442 | 0.454061 | 0.538661 | 0.001837 | 0.001926 | 0.001974 |
| list_copy.test | RUN | 0.359586 | 0.835295 | 0.926322 | 0.001637 | 0.002008 | 0.001823 |
| list_iterator.test | RUN | 0.239213 | 0.455386 | 0.524858 | 0.001541 | 0.001895 | 0.001897 |
| list_hash.test | RUN | 0.322331 | 0.893376 | 0.773224 | 0.001645 | 0.002214 | 0.001487 |
| list_make.test | RUN | 0.249290 | 0.504153 | 0.588211 | 0.001776 | 0.002096 | 0.001829 |
| list_misc.test | RUN | 0.328405 | 0.622220 | 0.739391 | 0.001806 | 0.002055 | 0.001920 |
| list_mutate.test | RUN | 0.228662 | 0.419821 | 0.510118 | 0.001805 | 0.002022 | 0.001892 |
| list_nest.test | RUN | 0.408407 | 1.186122 | 1.348629 | 0.001465 | 0.001889 | 0.002076 |
| list_tie.test | RUN | 0.229597 | 0.465547 | 0.559616 | 0.001642 | 0.001851 | 0.001956 |
| list_value_at.test | RUN | 0.242729 | 0.463719 | 0.497288 | 0.001391 | 0.002011 | 0.001549 |
| deque_comparison.test | RUN | 0.231844 | 0.487507 | 0.527832 | 0.001577 | 0.001832 | 0.001584 |
| deque_construction.test | RUN | 0.228216 | 0.456336 | 0.581961 | 0.001894 | 0.001940 | 0.002026 |
| deque_copy.test | RUN | 0.315487 | 0.798113 | 0.863082 | 0.001624 | 0.001870 | 0.001850 |
| deque_iterator.test | RUN | 0.216874 | 0.488305 | 0.524774 | 0.001729 | 0.001909 | 0.001777 |
| deque_hash.test | RUN | 0.311103 | 0.867930 | 0.752385 | 0.001531 | 0.001892 | 0.001840 |
| deque_is_constructible.test | COMPILE | 0.121128 | 0.218057 | 0.234139 | — | — | — |
| deque_make.test | RUN | 0.236127 | 0.485680 | 0.556724 | 0.001925 | 0.001824 | 0.001728 |
| deque_misc.test | RUN | 0.291699 | 0.593552 | 0.667888 | 0.001612 | 0.001930 | 0.001754 |
| deque_move.test | RUN | 0.232369 | 0.504126 | 0.521974 | 0.001418 | 0.001946 | 0.001844 |
| deque_mutate.test | RUN | 0.211325 | 0.408783 | 0.457768 | 0.001758 | 0.001926 | 0.001815 |
| deque_nest.test | RUN | 0.600033 | 1.871461 | 2.113514 | 0.001856 | 0.001854 | 0.001439 |
| deque_tie.test | RUN | 0.215439 | 0.432550 | 0.514196 | 0.001520 | 0.002082 | 0.001821 |
| deque_value_at.test | RUN | 0.213821 | 0.423002 | 0.484339 | 0.001698 | 0.001868 | 0.001781 |
| front_extended_deque.test | RUN | 0.297418 | 0.634272 | 0.712818 | 0.001787 | 0.001965 | 0.001981 |
| back_extended_deque.test | RUN | 0.306502 | 0.638528 | 0.730880 | 0.001512 | 0.001901 | 0.001685 |
| make_list.test | RUN | 0.242011 | 0.571871 | 0.551008 | 0.002026 | 0.001394 | 0.001962 |
| make_vector.test | RUN | 0.238522 | 0.508343 | 0.540768 | 0.001694 | 0.001996 | 0.001712 |
| map.test | RUN | 0.297361 | 0.811089 | 0.838122 | 0.001738 | 0.002188 | 0.001902 |
| map_comparison.test | RUN | 0.230095 | 0.529740 | 0.555868 | 0.001684 | 0.002097 | 0.001866 |
| map_construction.test | RUN | 0.230557 | 0.472699 | 0.525599 | 0.001549 | 0.001879 | 0.001778 |
| map_copy.test | RUN | 0.277616 | 0.609060 | 0.662903 | 0.001864 | 0.001976 | 0.002150 |
| map_misc.test | RUN | 0.295567 | 0.596648 | 0.687273 | 0.001952 | 0.002112 | 0.001618 |
| map_move.test | RUN | 0.239678 | 0.510105 | 0.541278 | 0.001688 | 0.001736 | 0.001484 |
| map_mutate.test | RUN | 0.213154 | 0.406830 | 0.487873 | 0.001590 | 0.001527 | 0.001669 |
| map_tie.test | RUN | 0.225217 | 0.425434 | 0.492666 | 0.001868 | 0.002004 | 0.001852 |
| nil.test | RUN | 0.152490 | 0.305054 | 0.337023 | 0.002322 | 0.002024 | 0.001878 |
| nview.test | RUN | 0.312933 | 0.601308 | 0.702024 | 0.001841 | 0.001912 | 0.001870 |
| reverse_view.test | RUN | 0.369096 | 1.030364 | 1.046655 | 0.001550 | 0.002070 | 0.001880 |
| segmented_iterator_range.test | RUN | 0.447209 | 1.226153 | 1.348155 | 0.001563 | 0.001807 | 0.001776 |
| set.test | RUN | 0.274509 | 0.703430 | 0.725479 | 0.001808 | 0.001654 | 0.001850 |
| single_view.test | RUN | 0.252977 | 0.667591 | 0.685683 | 0.001860 | 0.001737 | 0.001682 |
| std_pair.test | RUN | 0.316316 | 0.789932 | 0.843537 | 0.002077 | 0.001764 | 0.002050 |
| boost_array.test | RUN | 0.281064 | 0.536230 | 0.622452 | 0.001990 | 0.001872 | 0.001657 |
| array.test | RUN | 0.274950 | 0.538090 | 0.613443 | 0.001494 | 0.001501 | 0.002008 |
| std_array.test | RUN | 0.285073 | 0.551593 | 0.592924 | 0.001872 | 0.001911 | 0.001832 |
| tuple_comparison.test | RUN | 0.249926 | 0.510563 | 0.565295 | 0.001706 | 0.001973 | 0.001821 |
| tuple_construction.test | RUN | 0.254236 | 0.515026 | 0.573077 | 0.002146 | 0.001931 | 0.001874 |
| tuple_conversion.test | RUN | 0.513334 | 1.603562 | 1.943107 | 0.001663 | 0.001974 | 0.001830 |
| tuple_copy.test | RUN | 0.338680 | 0.888487 | 0.987276 | 0.001508 | 0.001878 | 0.001718 |
| tuple_element.test | RUN | 0.234565 | 0.435531 | 0.541970 | 0.001943 | 0.002108 | 0.001561 |
| tuple_make.test | RUN | 0.251101 | 0.530911 | 0.589488 | 0.001906 | 0.001854 | 0.001798 |
| tuple_misc.test | RUN | 0.305157 | 0.639930 | 0.755949 | 0.001570 | 0.002044 | 0.001765 |
| tuple_mutate.test | RUN | 0.225310 | 0.466841 | 0.494079 | 0.001844 | 0.001648 | 0.001556 |
| tuple_nest.test | RUN | 0.849954 | 3.333154 | 3.737485 | 0.002008 | 0.001658 | 0.001896 |
| tuple_hash.test | RUN | 0.314291 | 0.914587 | 0.732629 | 0.001850 | 0.002056 | 0.001520 |
| tuple_tie.test | RUN | 0.234304 | 0.486010 | 0.558090 | 0.001938 | 0.001677 | 0.001618 |
| tuple_traits__maybe_variadic.test | RUN | 0.246450 | 0.579382 | 0.578310 | 0.001714 | 0.001756 | 0.001542 |
| tuple_traits__no_variadic.test | RUN | 0.290198 | 0.600233 | 0.654591 | 0.001791 | 0.002008 | 0.001882 |
| transform_view.test | RUN | 0.351553 | 1.002275 | 1.070601 | 0.001862 | 0.001978 | 0.001734 |
| identity_view.test | RUN | 0.337593 | 0.935086 | 1.010462 | 0.001471 | 0.001648 | 0.001554 |
| vector_comparison.test | RUN | 0.226003 | 0.508731 | 0.611536 | 0.001467 | 0.001962 | 0.001997 |
| vector_construction.test | RUN | 0.224061 | 0.508629 | 0.547484 | 0.001793 | 0.001883 | 0.001932 |
| vector_conversion.test | RUN | 0.598091 | 2.043789 | 2.561232 | 0.001829 | 0.001576 | 0.001875 |
| vector_copy.test | RUN | 0.326124 | 0.895056 | 1.015625 | 0.001794 | 0.001850 | 0.001416 |
| vector_iterator.test | RUN | 0.213414 | 0.481284 | 0.538188 | 0.001930 | 0.001760 | 0.001759 |
| vector_make.test | RUN | 0.234122 | 0.517161 | 0.586591 | 0.001661 | 0.002004 | 0.001820 |
| vector_misc.test | RUN | 0.290961 | 0.602026 | 0.705561 | 0.001724 | 0.001365 | 0.001492 |
| vector_move.test | RUN | 0.231753 | 0.526156 | 0.554364 | 0.001618 | 0.001916 | 0.001784 |
| vector_mutate.test | RUN | 0.203570 | 0.407342 | 0.510211 | 0.001551 | 0.001842 | 0.001666 |
| vector_n.test | RUN | 0.245161 | 0.517398 | 0.642346 | 0.002001 | 0.001839 | 0.002024 |
| vector_nest.test | RUN | 0.845694 | 3.596552 | 3.789448 | 0.001253 | 0.002101 | 0.001724 |
| vector_hash.test | RUN | 0.299732 | 0.880452 | 0.740029 | 0.001518 | 0.002109 | 0.001730 |
| vector_tie.test | RUN | 0.218191 | 0.457335 | 0.502175 | 0.001728 | 0.001742 | 0.001924 |
| vector_traits__maybe_variadic.test | RUN | 0.234957 | 0.539265 | 0.532207 | 0.002034 | 0.002022 | 0.001950 |
| vector_traits__no_variadic.test | RUN | 0.254525 | 0.563642 | 0.615537 | 0.001709 | 0.001823 | 0.001876 |
| vector_value_at.test | RUN | 0.213787 | 0.421492 | 0.494898 | 0.001472 | 0.001875 | 0.001960 |
| zip_view.test | RUN | 0.417740 | 1.018176 | 1.306157 | 0.001967 | 0.001496 | 0.001782 |
| zip_view2.test | RUN | 0.333213 | 0.806795 | 0.991923 | 0.001648 | 0.001810 | 0.001437 |
| zip_view_ignore.test | RUN | 0.316181 | 0.791970 | 0.877358 | 0.001670 | 0.004190 | 0.001811 |
| repetitive_view.test | RUN | 0.547783 | 1.440198 | 1.717988 | 0.001606 | 0.001938 | 0.001926 |
| deduce_sequence.test | RUN | 0.259998 | 0.670913 | 0.693827 | 0.001672 | 0.002080 | 0.001439 |
| adapt_adt_named.test | RUN | 0.313716 | 0.848596 | 0.854584 | 0.001507 | 0.002080 | 0.001846 |
| adapt_adt_named_empty.test | RUN | 0.273524 | 0.646969 | 0.719195 | 0.001724 | 0.002207 | 0.001504 |
| adapt_adt.test | RUN | 0.346326 | 1.091163 | 1.039023 | 0.001668 | 0.002012 | 0.001845 |
| adapt_adt_empty.test | RUN | 0.272566 | 0.609594 | 0.727775 | 0.001848 | 0.001888 | 0.002030 |
| adapt_assoc_adt_named.test | RUN | 0.350904 | 0.860478 | 0.914342 | 0.001865 | 0.001866 | 0.001790 |
| adapt_assoc_adt_named_empty.test | RUN | 0.274959 | 0.671521 | 0.685346 | 0.001712 | 0.001708 | 0.001868 |
| adapt_assoc_adt.test | RUN | 0.369274 | 0.973544 | 0.968729 | 0.001661 | 0.002026 | 0.001867 |
| adapt_assoc_adt_empty.test | RUN | 0.280071 | 0.718160 | 0.670846 | 0.001954 | 0.001758 | 0.001747 |
| adapt_assoc_struct_named.test | RUN | 0.360235 | 0.883178 | 0.864489 | 0.002142 | 0.001823 | 0.001549 |
| adapt_assoc_struct_named_empty.test | RUN | 0.285970 | 0.715346 | 0.672502 | 0.001350 | 0.002034 | 0.001776 |
| adapt_assoc_struct.test | RUN | 0.303582 | 0.778108 | 0.802122 | 0.001867 | 0.001804 | 0.001712 |
| adapt_assoc_struct_empty.test | RUN | 0.270458 | 0.623691 | 0.664969 | 0.002019 | 0.001900 | 0.001960 |
| adapt_assoc_tpl_adt.test | RUN | 0.372565 | 0.897772 | 0.982051 | 0.001813 | 0.001984 | 0.002078 |
| adapt_assoc_tpl_adt_empty.test | RUN | 0.282064 | 0.641153 | 0.708647 | 0.001687 | 0.001943 | 0.001870 |
| adapt_assoc_tpl_struct.test | RUN | 0.296972 | 0.757423 | 0.839163 | 0.001699 | 0.001944 | 0.004243 |
| adapt_assoc_tpl_struct_empty.test | RUN | 0.268687 | 0.623397 | 0.675242 | 0.001609 | 0.001914 | 0.001882 |
| adapt_struct_named.test | RUN | 0.312316 | 0.792860 | 0.896471 | 0.001832 | 0.001848 | 0.001990 |
| adapt_struct_named_empty.test | RUN | 0.273151 | 0.629659 | 0.720667 | 0.001594 | 0.001656 | 0.001781 |
| adapt_struct.test | RUN | 0.336131 | 0.908932 | 0.969576 | 0.001625 | 0.001842 | 0.001876 |
| adapt_struct_empty.test | RUN | 0.274084 | 0.632845 | 0.699429 | 0.001743 | 0.001874 | 0.001942 |
| adapt_tpl_adt.test | RUN | 0.344275 | 1.055805 | 0.991822 | 0.001735 | 0.002151 | 0.001762 |
| adapt_tpl_adt_empty.test | RUN | 0.273277 | 0.663275 | 0.692682 | 0.001722 | 0.001934 | 0.001816 |
| adapt_tpl_struct.test | RUN | 0.300510 | 0.725685 | 0.752612 | 0.001795 | 0.001896 | 0.001417 |
| adapt_tpl_struct_empty.test | RUN | 0.278082 | 0.653252 | 0.702226 | 0.001802 | 0.001922 | 0.002026 |
| adt_attribute_proxy.test | RUN | 0.318415 | 0.651598 | 0.729245 | 0.001824 | 0.001909 | 0.001448 |
| define_struct.test | RUN | 0.363816 | 0.875020 | 0.953033 | 0.001558 | 0.001848 | 0.002008 |
| define_struct_empty.test | RUN | 0.324716 | 0.717314 | 0.790579 | 0.002017 | 0.002060 | 0.001748 |
| define_struct_move.test | RUN | 0.223670 | 0.404552 | 0.472729 | 0.001836 | 0.001816 | 0.001462 |
| define_struct_inline.test | RUN | 0.415015 | 1.067721 | 1.093843 | 0.001995 | 0.001590 | 0.001998 |
| define_struct_inline_empty.test | RUN | 0.359020 | 0.707419 | 0.769672 | 0.001645 | 0.002014 | 0.002038 |
| define_struct_inline_move.test | RUN | 0.238421 | 0.421316 | 0.477444 | 0.001684 | 0.001872 | 0.001707 |
| define_assoc_struct.test | RUN | 0.388935 | 0.866210 | 0.929523 | 0.001920 | 0.001786 | 0.001927 |
| define_assoc_struct_empty.test | RUN | 0.335940 | 0.753572 | 0.803925 | 0.001901 | 0.002024 | 0.001870 |
| define_assoc_struct_move.test | RUN | 0.220487 | 0.397560 | 0.501350 | 0.001798 | 0.002210 | 0.001719 |
| define_tpl_struct.test | RUN | 0.363974 | 0.887844 | 0.896125 | 0.001894 | 0.001770 | 0.001940 |
| define_tpl_struct_empty.test | RUN | 0.333501 | 0.769590 | 0.784626 | 0.001804 | 0.002030 | 0.001719 |
| define_tpl_struct_move.test | RUN | 0.219869 | 0.412622 | 0.476427 | 0.001950 | 0.001543 | 0.001760 |
| define_tpl_struct_inline.test | RUN | 0.368371 | 0.942471 | 1.021785 | 0.001751 | 0.001986 | 0.001772 |
| define_tpl_struct_inline_empty.test | RUN | 0.334858 | 0.702309 | 0.827213 | 0.001712 | 0.001836 | 0.001892 |
| define_tpl_struct_inline_move.test | RUN | 0.222201 | 0.404815 | 0.492989 | 0.001878 | 0.001599 | 0.001840 |
| define_assoc_tpl_struct.test | RUN | 0.355569 | 0.855140 | 0.953172 | 0.001941 | 0.001980 | 0.001896 |
| define_assoc_tpl_struct_empty.test | RUN | 0.327954 | 0.705897 | 0.833666 | 0.001762 | 0.001869 | 0.002085 |
| define_assoc_tpl_struct_move.test | RUN | 0.217432 | 0.407462 | 0.483519 | 0.001677 | 0.001776 | 0.001999 |
| std_tuple.test | RUN | 0.226261 | 0.453290 | 0.558047 | 0.001700 | 0.001488 | 0.001942 |
| std_tuple_iterator.test | RUN | 0.216440 | 0.469598 | 0.558038 | 0.001472 | 0.001802 | 0.002004 |
| ref_vector.test | RUN | 0.253752 | 0.498234 | 0.567084 | 0.001840 | 0.001977 | 0.001911 |
| flatten_view.test | RUN | 0.259725 | 0.638903 | 0.712261 | 0.003156 | 0.003346 | 0.001603 |
| github-159.test | COMPILE | 0.122735 | 0.213125 | 0.247628 | — | — | — |
| github-176.test | RUN | 0.290050 | 0.588931 | 0.584108 | 0.001871 | 0.002148 | 0.001875 |
| size.test | COMPILE | 0.246465 | 0.431881 | 0.431119 | — | — | — |
| fused.test | RUN | 0.299981 | 0.644703 | 0.658306 | 0.001515 | 0.001888 | 0.001667 |
| fused_function_object.test | RUN | 0.220464 | 0.459554 | 0.493713 | 0.001900 | 0.001874 | 0.001796 |
| fused_procedure.test | RUN | 0.291087 | 0.569313 | 0.623401 | 0.001884 | 0.001880 | 0.001828 |
| unfused.test | RUN | 0.233647 | 0.444090 | 0.519733 | 0.002024 | 0.001846 | 0.001627 |
| unfused_typed.test | RUN | 0.278802 | 0.478316 | 0.528931 | 0.001883 | 0.001906 | 0.001842 |
| make_fused.test | RUN | 0.342779 | 0.564164 | 0.634642 | 0.001920 | 0.001863 | 0.001724 |
| make_fused_function_object.test | RUN | 0.232534 | 0.442862 | 0.487518 | 0.001870 | 0.002080 | 0.001768 |
| make_fused_procedure.test | RUN | 0.296855 | 0.561864 | 0.640850 | 0.001614 | 0.001860 | 0.001848 |
| make_unfused.test | RUN | 0.224283 | 0.456637 | 0.518664 | 0.001818 | 0.002068 | 0.001857 |
| invoke.test | RUN | 0.558256 | 1.621207 | 1.823895 | 0.001676 | 0.001741 | 0.002056 |
| invoke_function_object.test | RUN | 0.278004 | 0.590809 | 0.628480 | 0.001788 | 0.001842 | 0.002049 |
| invoke_procedure.test | RUN | 0.448265 | 1.226539 | 1.333697 | 0.001658 | 0.001796 | 0.001989 |
| swap.test | RUN | 0.339120 | 0.724482 | 0.793190 | 0.001486 | 0.001946 | 0.001874 |
| is_sequence.test | COMPILE | 0.066525 | 0.105216 | 0.107743 | — | — | — |
| is_view.test | COMPILE | 0.062343 | 0.104670 | 0.108651 | — | — | — |
| pair_deque.test | COMPILE | 0.125940 | 0.226145 | 0.246426 | — | — | — |
| pair_list.test | COMPILE | 0.124451 | 0.224189 | 0.262723 | — | — | — |
| pair_map.test | COMPILE | 0.130848 | 0.232796 | 0.277648 | — | — | — |
| pair_set.test | COMPILE | 0.136835 | 0.233701 | 0.268839 | — | — | — |
| pair_vector.test | COMPILE | 0.130202 | 0.229463 | 0.258613 | — | — | — |
| pair_nest.test | COMPILE | 0.052951 | 0.097069 | 0.106126 | — | — | — |
| index_sequence.test | COMPILE | 0.050346 | 0.067094 | 0.078535 | — | — | — |
| and.test | COMPILE | 0.038852 | 0.055237 | 0.062128 | — | — | — |
| tag_of.test | COMPILE | 0.062053 | 0.100144 | 0.104471 | — | — | — |
| unused.test | COMPILE | 0.029751 | 0.068469 | 0.066424 | — | — | — |
| tag_of_fallback.test | COMPILE | 0.056109 | 0.101047 | 0.092265 | — | — | — |
| make_pair_r-value.test | COMPILE | 0.045863 | 0.088045 | 0.088456 | — | — | — |
| quick.test | RUN | 0.169957 | 0.326227 | 0.341956 | 0.000601 | 0.001369 | 0.001391 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.971× | 0.951×, 0.990× | 1.006× |
| clang | 1.011× | 1.024×, 0.998× | 0.981× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adapt_adt.test | 10 | 0.000972s | 0.000967s | 0.001001s | byte equality observed |
| adapt_adt_empty.test | 10 | 0.000956s | 0.000978s | 0.000977s | byte equality observed |
| adapt_adt_named.test | 10 | 0.000971s | 0.000965s | 0.001001s | byte equality observed |
| adapt_adt_named_empty.test | 10 | 0.000982s | 0.001001s | 0.000993s | byte equality observed |
| adapt_assoc_adt.test | 10 | 0.000957s | 0.000980s | 0.000975s | byte equality observed |
| adapt_assoc_adt_empty.test | 10 | 0.000964s | 0.000982s | 0.000980s | byte equality observed |
| adapt_assoc_adt_named.test | 10 | 0.000965s | 0.000970s | 0.000993s | byte equality observed |
| adapt_assoc_adt_named_empty.test | 10 | 0.000945s | 0.000943s | 0.001001s | byte equality observed |
| adapt_assoc_struct.test | 10 | 0.000948s | 0.000970s | 0.000975s | byte equality observed |
| adapt_assoc_struct_empty.test | 10 | 0.000956s | 0.000959s | 0.000974s | byte equality observed |
| adapt_assoc_struct_named.test | 10 | 0.000966s | 0.000965s | 0.000977s | byte equality observed |
| adapt_assoc_struct_named_empty.test | 10 | 0.000957s | 0.000960s | 0.000985s | byte equality observed |
| adapt_assoc_tpl_adt.test | 10 | 0.000982s | 0.000953s | 0.000999s | byte equality observed |
| adapt_assoc_tpl_adt_empty.test | 10 | 0.000962s | 0.000964s | 0.000992s | byte equality observed |
| adapt_assoc_tpl_struct.test | 10 | 0.000996s | 0.001008s | 0.001013s | byte equality observed |
| adapt_assoc_tpl_struct_empty.test | 10 | 0.000965s | 0.000995s | 0.000986s | byte equality observed |
| adapt_struct.test | 10 | 0.000982s | 0.001014s | 0.001009s | byte equality observed |
| adapt_struct_empty.test | 10 | 0.000955s | 0.001018s | 0.000977s | byte equality observed |
| adapt_struct_named.test | 10 | 0.000959s | 0.001007s | 0.000981s | byte equality observed |
| adapt_struct_named_empty.test | 10 | 0.000951s | 0.000991s | 0.000981s | byte equality observed |
| adapt_tpl_adt.test | 10 | 0.000961s | 0.000986s | 0.001049s | byte equality observed |
| adapt_tpl_adt_empty.test | 10 | 0.000958s | 0.000953s | 0.001086s | byte equality observed |
| adapt_tpl_struct.test | 10 | 0.000958s | 0.000959s | 0.000972s | byte equality observed |
| adapt_tpl_struct_empty.test | 10 | 0.000963s | 0.000956s | 0.000998s | byte equality observed |
| adt_attribute_proxy.test | 10 | 0.000956s | 0.000971s | 0.000978s | byte equality observed |
| all.test | 10 | 0.000947s | 0.000975s | 0.000982s | byte equality observed |
| any.test | 10 | 0.000946s | 0.000962s | 0.000954s | byte equality observed |
| array.test | 10 | 0.000943s | 0.000952s | 0.000959s | byte equality observed |
| as_deque.test | 10 | 0.000961s | 0.000942s | 0.000959s | byte equality observed |
| as_list.test | 10 | 0.000983s | 0.001011s | 0.001008s | byte equality observed |
| as_map.test | 10 | 0.000974s | 0.000985s | 0.000978s | byte equality observed |
| as_map_assoc.test | 10 | 0.000965s | 0.000979s | 0.001041s | byte equality observed |
| as_set.test | 10 | 0.000982s | 0.000986s | 0.001016s | byte equality observed |
| as_vector.test | 10 | 0.001017s | 0.000998s | 0.001017s | byte equality observed |
| back_extended_deque.test | 10 | 0.000952s | 0.000956s | 0.001017s | byte equality observed |
| boost_array.test | 10 | 0.000949s | 0.000958s | 0.000971s | byte equality observed |
| boost_tuple.test | 10 | 0.000981s | 0.000972s | 0.000984s | byte equality observed |
| boost_tuple_iterator.test | 10 | 0.000953s | 0.000970s | 0.000963s | byte equality observed |
| clear.test | 10 | 0.000972s | 0.000983s | 0.001001s | byte equality observed |
| cons.test | 10 | 0.001012s | 0.001011s | 0.001070s | byte equality observed |
| convert_boost_tuple.test | 10 | 0.000986s | 0.000945s | 0.001012s | byte equality observed |
| convert_deque.test | 10 | 0.000981s | 0.000960s | 0.000976s | byte equality observed |
| convert_list.test | 10 | 0.000948s | 0.001005s | 0.000997s | byte equality observed |
| convert_std_pair.test | 10 | 0.000953s | 0.000983s | 0.000984s | byte equality observed |
| convert_std_tuple.test | 10 | 0.000956s | 0.000996s | 0.000956s | byte equality observed |
| convert_vector.test | 10 | 0.000971s | 0.000971s | 0.000957s | byte equality observed |
| copy.test | 10 | 0.000960s | 0.000938s | 0.000962s | byte equality observed |
| count.test | 10 | 0.000966s | 0.000965s | 0.000972s | byte equality observed |
| count_if.test | 10 | 0.000964s | 0.000941s | 0.000950s | byte equality observed |
| deduce_sequence.test | 10 | 0.000940s | 0.000950s | 0.000951s | byte equality observed |
| define_assoc_struct.test | 10 | 0.000963s | 0.000999s | 0.000985s | byte equality observed |
| define_assoc_struct_empty.test | 10 | 0.000982s | 0.000981s | 0.000972s | byte equality observed |
| define_assoc_struct_move.test | 10 | 0.000954s | 0.000975s | 0.000968s | byte equality observed |
| define_assoc_tpl_struct.test | 10 | 0.000980s | 0.001036s | 0.001021s | byte equality observed |
| define_assoc_tpl_struct_empty.test | 10 | 0.000987s | 0.000978s | 0.001019s | byte equality observed |
| define_assoc_tpl_struct_move.test | 10 | 0.000953s | 0.000961s | 0.000965s | byte equality observed |
| define_struct.test | 10 | 0.000973s | 0.001006s | 0.000986s | byte equality observed |
| define_struct_empty.test | 10 | 0.000969s | 0.000980s | 0.000950s | byte equality observed |
| define_struct_inline.test | 10 | 0.000993s | 0.000996s | 0.001006s | byte equality observed |
| define_struct_inline_empty.test | 10 | 0.000970s | 0.001007s | 0.000977s | byte equality observed |
| define_struct_inline_move.test | 10 | 0.000947s | 0.000970s | 0.000946s | byte equality observed |
| define_struct_move.test | 10 | 0.000957s | 0.000966s | 0.000971s | byte equality observed |
| define_tpl_struct.test | 10 | 0.000974s | 0.000994s | 0.000992s | byte equality observed |
| define_tpl_struct_empty.test | 10 | 0.000960s | 0.000969s | 0.000980s | byte equality observed |
| define_tpl_struct_inline.test | 10 | 0.000966s | 0.000986s | 0.001004s | byte equality observed |
| define_tpl_struct_inline_empty.test | 10 | 0.000972s | 0.000970s | 0.000988s | byte equality observed |
| define_tpl_struct_inline_move.test | 10 | 0.000945s | 0.000973s | 0.000999s | byte equality observed |
| define_tpl_struct_move.test | 10 | 0.000965s | 0.000958s | 0.001027s | byte equality observed |
| deque_comparison.test | 10 | 0.000946s | 0.000958s | 0.001011s | byte equality observed |
| deque_construction.test | 10 | 0.000964s | 0.000953s | 0.000981s | byte equality observed |
| deque_copy.test | 10 | 0.000959s | 0.000961s | 0.000987s | byte equality observed |
| deque_hash.test | 10 | 0.000976s | 0.000978s | 0.000974s | byte equality observed |
| deque_iterator.test | 10 | 0.000957s | 0.000965s | 0.000964s | byte equality observed |
| deque_make.test | 10 | 0.000957s | 0.000982s | 0.000992s | byte equality observed |
| deque_misc.test | 10 | 0.000940s | 0.000958s | 0.000968s | byte equality observed |
| deque_move.test | 10 | 0.000980s | 0.000987s | 0.000971s | byte equality observed |
| deque_mutate.test | 10 | 0.000953s | 0.000948s | 0.000949s | byte equality observed |
| deque_nest.test | 10 | 0.000960s | 0.000934s | 0.000962s | byte equality observed |
| deque_tie.test | 10 | 0.000952s | 0.000968s | 0.000962s | byte equality observed |
| deque_value_at.test | 10 | 0.000954s | 0.000944s | 0.000980s | byte equality observed |
| erase.test | 10 | 0.000995s | 0.000969s | 0.000984s | byte equality observed |
| erase_key.test | 10 | 0.001000s | 0.001000s | 0.000996s | byte equality observed |
| filter.test | 10 | 0.001004s | 0.000978s | 0.000945s | byte equality observed |
| filter_if.test | 10 | 0.000982s | 0.000999s | 0.000963s | byte equality observed |
| filter_view.test | 10 | 0.001001s | 0.000987s | 0.000988s | byte equality observed |
| find.test | 10 | 0.000988s | 0.001012s | 0.000974s | byte equality observed |
| find_if.test | 10 | 0.000974s | 0.001005s | 0.000982s | byte equality observed |
| flatten.test | 10 | 0.000963s | 0.000991s | 0.000965s | byte equality observed |
| flatten_view.test | 10 | 0.000965s | 0.000983s | 0.000942s | byte equality observed |
| fold.test | 10 | 0.000982s | 0.000989s | 0.000964s | byte equality observed |
| for_each.test | 10 | 0.000984s | 0.001009s | 0.000984s | byte equality observed |
| front_extended_deque.test | 10 | 0.000955s | 0.000983s | 0.000926s | byte equality observed |
| fused.test | 10 | 0.000966s | 0.001000s | 0.000949s | byte equality observed |
| fused_function_object.test | 10 | 0.000961s | 0.001005s | 0.000964s | byte equality observed |
| fused_procedure.test | 10 | 0.000949s | 0.001003s | 0.000951s | byte equality observed |
| github-176.test | 10 | 0.000956s | 0.000969s | 0.000968s | byte equality observed |
| hash.test | 10 | 0.000956s | 0.000980s | 0.000944s | byte equality observed |
| identity_view.test | 10 | 0.000986s | 0.001024s | 0.000968s | byte equality observed |
| insert.test | 10 | 0.000985s | 0.001018s | 0.000984s | byte equality observed |
| insert_range.test | 10 | 0.001015s | 0.001002s | 0.001015s | byte equality observed |
| invoke.test | 10 | 0.000969s | 0.000985s | 0.000978s | byte equality observed |
| invoke_function_object.test | 10 | 0.000942s | 0.000946s | 0.000960s | byte equality observed |
| invoke_procedure.test | 10 | 0.000959s | 0.000943s | 0.000991s | byte equality observed |
| io.test | 10 | 0.001105s | 0.001098s | 0.001098s | byte equality observed |
| iter_fold.test | 10 | 0.000983s | 0.000968s | 0.000981s | byte equality observed |
| iterator_range.test | 10 | 0.001000s | 0.000992s | 0.001042s | byte equality observed |
| join.test | 10 | 0.000940s | 0.000948s | 0.001027s | byte equality observed |
| joint_view.test | 10 | 0.000992s | 0.001016s | 0.001077s | byte equality observed |
| list_comparison.test | 10 | 0.000941s | 0.000960s | 0.000975s | byte equality observed |
| list_construction.test | 10 | 0.000950s | 0.000959s | 0.001011s | byte equality observed |
| list_copy.test | 10 | 0.000963s | 0.000974s | 0.000977s | byte equality observed |
| list_hash.test | 10 | 0.000967s | 0.000983s | 0.000965s | byte equality observed |
| list_iterator.test | 10 | 0.000964s | 0.000986s | 0.000967s | byte equality observed |
| list_make.test | 10 | 0.000958s | 0.000956s | 0.000963s | byte equality observed |
| list_misc.test | 10 | 0.000956s | 0.000975s | 0.000959s | byte equality observed |
| list_mutate.test | 10 | 0.000953s | 0.000966s | 0.000990s | byte equality observed |
| list_nest.test | 10 | 0.000962s | 0.000984s | 0.000975s | byte equality observed |
| list_tie.test | 10 | 0.000950s | 0.000969s | 0.000942s | byte equality observed |
| list_value_at.test | 10 | 0.000976s | 0.000989s | 0.000973s | byte equality observed |
| make_fused.test | 10 | 0.000968s | 0.000977s | 0.000953s | byte equality observed |
| make_fused_function_object.test | 10 | 0.000953s | 0.000956s | 0.000969s | byte equality observed |
| make_fused_procedure.test | 10 | 0.000957s | 0.000963s | 0.000942s | byte equality observed |
| make_list.test | 10 | 0.000967s | 0.000963s | 0.000989s | byte equality observed |
| make_unfused.test | 10 | 0.000974s | 0.000986s | 0.000976s | byte equality observed |
| make_vector.test | 10 | 0.000972s | 0.000978s | 0.001003s | byte equality observed |
| map.test | 10 | 0.000993s | 0.001000s | 0.000978s | byte equality observed |
| map_comparison.test | 10 | 0.000962s | 0.000955s | 0.000939s | byte equality observed |
| map_construction.test | 10 | 0.000965s | 0.000951s | 0.000960s | byte equality observed |
| map_copy.test | 10 | 0.000975s | 0.000968s | 0.000982s | byte equality observed |
| map_misc.test | 10 | 0.000953s | 0.000960s | 0.000955s | byte equality observed |
| map_move.test | 10 | 0.000962s | 0.000959s | 0.000986s | byte equality observed |
| map_mutate.test | 10 | 0.000963s | 0.000963s | 0.000916s | byte equality observed |
| map_tie.test | 10 | 0.000956s | 0.000951s | 0.000946s | byte equality observed |
| move.test | 10 | 0.000946s | 0.000981s | 0.000939s | byte equality observed |
| nil.test | 10 | 0.000950s | 0.000942s | 0.000962s | byte equality observed |
| none.test | 10 | 0.000967s | 0.000942s | 0.000919s | byte equality observed |
| nview.test | 10 | 0.000954s | 0.000988s | 0.000938s | byte equality observed |
| pop_back.test | 10 | 0.001003s | 0.000983s | 0.001004s | byte equality observed |
| pop_front.test | 10 | 0.000977s | 0.000991s | 0.000980s | byte equality observed |
| push_back.test | 10 | 0.000989s | 0.000980s | 0.000998s | byte equality observed |
| push_front.test | 10 | 0.000991s | 0.000994s | 0.001034s | byte equality observed |
| quick.test | 10 | 0.000443s | 0.000935s | 0.000964s | byte equality observed |
| ref_vector.test | 10 | 0.000973s | 0.000969s | 0.000976s | byte equality observed |
| remove.test | 10 | 0.000973s | 0.001004s | 0.000990s | byte equality observed |
| remove_if.test | 10 | 0.000991s | 0.000998s | 0.001014s | byte equality observed |
| repetitive_view.test | 10 | 0.000946s | 0.000951s | 0.000979s | byte equality observed |
| replace.test | 10 | 0.000964s | 0.000955s | 0.000953s | byte equality observed |
| replace_if.test | 10 | 0.001001s | 0.000987s | 0.000963s | byte equality observed |
| reverse.test | 10 | 0.000992s | 0.000981s | 0.000994s | byte equality observed |
| reverse_fold.test | 10 | 0.000968s | 0.000954s | 0.000978s | byte equality observed |
| reverse_iter_fold.test | 10 | 0.000976s | 0.000972s | 0.000969s | byte equality observed |
| reverse_view.test | 10 | 0.001007s | 0.000984s | 0.000973s | byte equality observed |
| segmented_find.test | 10 | 0.000974s | 0.000954s | 0.000940s | byte equality observed |
| segmented_find_if.test | 10 | 0.000974s | 0.000943s | 0.000983s | byte equality observed |
| segmented_fold.test | 10 | 0.001017s | 0.000981s | 0.000997s | byte equality observed |
| segmented_for_each.test | 10 | 0.001012s | 0.000979s | 0.000999s | byte equality observed |
| segmented_iterator_range.test | 10 | 0.001015s | 0.000985s | 0.000999s | byte equality observed |
| set.test | 10 | 0.000996s | 0.000958s | 0.000976s | byte equality observed |
| single_view.test | 10 | 0.001005s | 0.000988s | 0.001003s | byte equality observed |
| std_array.test | 10 | 0.000966s | 0.000972s | 0.000973s | byte equality observed |
| std_pair.test | 10 | 0.001019s | 0.000986s | 0.000989s | byte equality observed |
| std_tuple.test | 10 | 0.000972s | 0.000926s | 0.000963s | byte equality observed |
| std_tuple_iterator.test | 10 | 0.000951s | 0.000947s | 0.000948s | byte equality observed |
| swap.test | 10 | 0.000944s | 0.000939s | 0.000976s | byte equality observed |
| transform.test | 10 | 0.000983s | 0.000968s | 0.000981s | byte equality observed |
| transform_view.test | 10 | 0.001008s | 0.000980s | 0.000993s | byte equality observed |
| tuple_comparison.test | 10 | 0.000939s | 0.000951s | 0.000950s | byte equality observed |
| tuple_construction.test | 10 | 0.000963s | 0.000980s | 0.000946s | byte equality observed |
| tuple_conversion.test | 10 | 0.000961s | 0.000942s | 0.000944s | byte equality observed |
| tuple_copy.test | 10 | 0.000953s | 0.000949s | 0.000945s | byte equality observed |
| tuple_element.test | 10 | 0.000936s | 0.000929s | 0.000966s | byte equality observed |
| tuple_hash.test | 10 | 0.000955s | 0.000958s | 0.000964s | byte equality observed |
| tuple_make.test | 10 | 0.000963s | 0.000961s | 0.000957s | byte equality observed |
| tuple_misc.test | 10 | 0.000981s | 0.000986s | 0.000991s | byte equality observed |
| tuple_mutate.test | 10 | 0.000949s | 0.000979s | 0.000965s | byte equality observed |
| tuple_nest.test | 10 | 0.000953s | 0.000942s | 0.000961s | byte equality observed |
| tuple_tie.test | 10 | 0.000951s | 0.000935s | 0.001012s | byte equality observed |
| tuple_traits__maybe_variadic.test | 10 | 0.000943s | 0.000941s | 0.000984s | byte equality observed |
| tuple_traits__no_variadic.test | 10 | 0.000936s | 0.000918s | 0.000970s | byte equality observed |
| unfused.test | 10 | 0.000936s | 0.000940s | 0.000988s | byte equality observed |
| unfused_typed.test | 10 | 0.000950s | 0.000953s | 0.001002s | byte equality observed |
| vector_comparison.test | 10 | 0.000951s | 0.000926s | 0.000955s | byte equality observed |
| vector_construction.test | 10 | 0.000951s | 0.000917s | 0.000970s | byte equality observed |
| vector_conversion.test | 10 | 0.000938s | 0.000930s | 0.000967s | byte equality observed |
| vector_copy.test | 10 | 0.000969s | 0.000938s | 0.000954s | byte equality observed |
| vector_hash.test | 10 | 0.000970s | 0.000968s | 0.000953s | byte equality observed |
| vector_iterator.test | 10 | 0.000945s | 0.000938s | 0.000961s | byte equality observed |
| vector_make.test | 10 | 0.000969s | 0.000953s | 0.000978s | byte equality observed |
| vector_misc.test | 10 | 0.000971s | 0.000943s | 0.000975s | byte equality observed |
| vector_move.test | 10 | 0.000977s | 0.000995s | 0.000973s | byte equality observed |
| vector_mutate.test | 10 | 0.000987s | 0.000945s | 0.000935s | byte equality observed |
| vector_n.test | 10 | 0.000995s | 0.000975s | 0.000968s | byte equality observed |
| vector_nest.test | 10 | 0.000977s | 0.000926s | 0.000953s | byte equality observed |
| vector_tie.test | 10 | 0.000958s | 0.000962s | 0.000944s | byte equality observed |
| vector_traits__maybe_variadic.test | 10 | 0.000936s | 0.000940s | 0.000951s | byte equality observed |
| vector_traits__no_variadic.test | 10 | 0.000946s | 0.000972s | 0.000966s | byte equality observed |
| vector_value_at.test | 10 | 0.000941s | 0.000936s | 0.000922s | byte equality observed |
| zip.test | 10 | 0.000951s | 0.000972s | 0.000936s | byte equality observed |
| zip2.test | 10 | 0.000958s | 0.000980s | 0.000935s | byte equality observed |
| zip_ignore.test | 10 | 0.000974s | 0.001003s | 0.000920s | byte equality observed |
| zip_view.test | 10 | 0.000938s | 0.001013s | 0.000954s | byte equality observed |
| zip_view2.test | 10 | 0.000967s | 0.000946s | 0.000966s | byte equality observed |
| zip_view_ignore.test | 10 | 0.000979s | 0.000958s | 0.000943s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
