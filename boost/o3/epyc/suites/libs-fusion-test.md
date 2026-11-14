# libs/fusion/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| all.test | RUN | 0.281400 | 0.601458 | 0.757424 | 0.001736 | 0.001502 | 0.001489 |
| any.test | RUN | 0.276814 | 0.573852 | 0.730338 | 0.001330 | 0.001516 | 0.001846 |
| clear.test | RUN | 0.195923 | 0.531826 | 0.615758 | 0.001486 | 0.001771 | 0.001938 |
| copy.test | RUN | 0.166634 | 0.389098 | 0.484091 | 0.001538 | 0.001704 | 0.001790 |
| count.test | RUN | 0.207076 | 0.436395 | 0.566966 | 0.001704 | 0.001741 | 0.001714 |
| count_if.test | RUN | 0.197700 | 0.404692 | 0.510392 | 0.001572 | 0.001804 | 0.001635 |
| erase.test | RUN | 0.271050 | 0.723487 | 0.849760 | 0.001426 | 0.001710 | 0.001610 |
| erase_key.test | RUN | 0.266359 | 0.716038 | 0.817377 | 0.001738 | 0.001467 | 0.001962 |
| filter.test | RUN | 0.231891 | 0.611146 | 0.762028 | 0.001902 | 0.001814 | 0.001759 |
| filter_if.test | RUN | 0.256039 | 0.716184 | 0.879185 | 0.001578 | 0.001727 | 0.001861 |
| find.test | RUN | 0.231188 | 0.481272 | 0.635458 | 0.001335 | 0.001464 | 0.001654 |
| find_if.test | RUN | 0.215137 | 0.443448 | 0.590497 | 0.001753 | 0.001731 | 0.001560 |
| fold.test | RUN | 0.227287 | 0.549735 | 0.680807 | 0.001463 | 0.001802 | 0.001716 |
| for_each.test | RUN | 0.201291 | 0.511421 | 0.610695 | 0.001538 | 0.001749 | 0.001806 |
| insert.test | RUN | 0.294979 | 0.871975 | 0.992537 | 0.001326 | 0.001512 | 0.001846 |
| insert_range.test | RUN | 0.292628 | 0.875352 | 1.034965 | 0.001697 | 0.001613 | 0.001929 |
| iter_fold.test | RUN | 0.342972 | 0.809532 | 1.205366 | 0.001764 | 0.001817 | 0.001778 |
| move.test | RUN | 0.174545 | 0.397818 | 0.527163 | 0.001577 | 0.001702 | 0.001845 |
| none.test | RUN | 0.285908 | 0.569730 | 0.780167 | 0.001724 | 0.001980 | 0.001641 |
| pop_back.test | RUN | 0.279652 | 0.759224 | 0.930423 | 0.001887 | 0.001700 | 0.001916 |
| pop_front.test | RUN | 0.243719 | 0.622000 | 0.749516 | 0.001928 | 0.001698 | 0.001956 |
| push_back.test | RUN | 0.294022 | 0.952666 | 1.004291 | 0.001577 | 0.001983 | 0.001853 |
| push_front.test | RUN | 0.253249 | 0.826746 | 0.860465 | 0.001614 | 0.001690 | 0.001883 |
| remove.test | RUN | 0.281815 | 0.907075 | 1.026936 | 0.001403 | 0.001823 | 0.001790 |
| remove_if.test | RUN | 0.249581 | 0.743054 | 0.846194 | 0.001838 | 0.001752 | 0.001792 |
| replace.test | RUN | 0.241429 | 0.682380 | 0.772531 | 0.001590 | 0.001850 | 0.001833 |
| replace_if.test | RUN | 0.235927 | 0.691678 | 0.759760 | 0.001829 | 0.001881 | 0.001900 |
| reverse_fold.test | RUN | 0.388832 | 0.928016 | 1.323931 | 0.001835 | 0.001803 | 0.001623 |
| reverse_iter_fold.test | RUN | 0.405090 | 0.979352 | 1.363012 | 0.001558 | 0.001734 | 0.001472 |
| reverse.test | RUN | 0.242405 | 0.687483 | 0.776548 | 0.001673 | 0.001604 | 0.001504 |
| segmented_for_each.test | RUN | 0.437163 | 1.251785 | 1.603538 | 0.001659 | 0.001538 | 0.001549 |
| segmented_find.test | RUN | 0.276643 | 0.671272 | 0.899574 | 0.001579 | 0.001785 | 0.001566 |
| segmented_find_if.test | RUN | 0.274234 | 0.679325 | 0.918003 | 0.001656 | 0.001793 | 0.001450 |
| segmented_fold.test | RUN | 0.340270 | 0.886346 | 1.151076 | 0.001834 | 0.001724 | 0.001726 |
| transform.test | RUN | 0.260158 | 0.726139 | 0.802259 | 0.001752 | 0.001485 | 0.005313 |
| join.test | RUN | 0.193037 | 0.459731 | 0.570612 | 0.001690 | 0.001756 | 0.001707 |
| zip.test | RUN | 0.353553 | 0.814634 | 1.136334 | 0.001746 | 0.001815 | 0.001390 |
| zip2.test | RUN | 0.329989 | 0.800825 | 1.073837 | 0.001477 | 0.001672 | 0.001841 |
| zip_ignore.test | RUN | 0.251204 | 0.550809 | 0.723006 | 0.001387 | 0.001410 | 0.001780 |
| flatten.test | RUN | 0.237156 | 0.604823 | 0.705037 | 0.001485 | 0.001635 | 0.001902 |
| ticket-5490.test | COMPILE | 0.119581 | 0.254583 | 0.314391 | — | — | — |
| as_deque.test | RUN | 0.239165 | 0.567893 | 0.773524 | 0.001607 | 0.001828 | 0.001855 |
| as_list.test | RUN | 0.265744 | 0.718349 | 0.825932 | 0.001925 | 0.001570 | 0.001704 |
| as_map.test | RUN | 0.265491 | 0.699485 | 0.840284 | 0.001534 | 0.001658 | 0.001940 |
| as_map_assoc.test | RUN | 0.254663 | 0.666761 | 0.798728 | 0.001427 | 0.001577 | 0.001853 |
| as_set.test | RUN | 0.303636 | 0.823709 | 0.971772 | 0.001846 | 0.001883 | 0.001826 |
| as_vector.test | RUN | 0.268870 | 0.744584 | 0.867134 | 0.001683 | 0.001512 | 0.001638 |
| boost_tuple.test | RUN | 0.283345 | 0.704982 | 0.864843 | 0.001557 | 0.001622 | 0.001847 |
| boost_tuple_iterator.test | RUN | 0.184944 | 0.415606 | 0.502766 | 0.001512 | 0.001666 | 0.001779 |
| cons.test | RUN | 0.345410 | 0.806147 | 0.989952 | 0.001355 | 0.001710 | 0.001463 |
| convert_boost_tuple.test | RUN | 0.214107 | 0.617182 | 0.655058 | 0.001537 | 0.001605 | 0.001353 |
| convert_deque.test | RUN | 0.218686 | 0.614681 | 0.651975 | 0.001530 | 0.001500 | 0.001176 |
| convert_list.test | RUN | 0.219281 | 0.640802 | 0.652252 | 0.001518 | 0.001472 | 0.001628 |
| convert_std_pair.test | RUN | 0.216373 | 0.618325 | 0.656009 | 0.001486 | 0.001636 | 0.001454 |
| convert_std_tuple.test | RUN | 0.208652 | 0.604001 | 0.639448 | 0.001486 | 0.001599 | 0.002006 |
| convert_vector.test | RUN | 0.212009 | 0.603423 | 0.646404 | 0.001534 | 0.001445 | 0.001647 |
| filter_view.test | RUN | 0.260919 | 0.644744 | 0.767061 | 0.001554 | 0.001604 | 0.001789 |
| hash.test | RUN | 0.248743 | 0.597182 | 0.721601 | 0.001370 | 0.001760 | 0.001570 |
| io.test | RUN | 0.254640 | 0.864062 | 0.865961 | 0.001618 | 0.002038 | 0.001703 |
| iterator_range.test | RUN | 0.231563 | 0.648530 | 0.718985 | 0.001528 | 0.001901 | 0.001680 |
| joint_view.test | RUN | 0.315477 | 1.051333 | 1.102937 | 0.001819 | 0.001776 | 0.001624 |
| list_comparison.test | RUN | 0.182078 | 0.448779 | 0.524207 | 0.001540 | 0.001423 | 0.001714 |
| list_construction.test | RUN | 0.179694 | 0.425834 | 0.506635 | 0.001406 | 0.001654 | 0.001690 |
| list_copy.test | RUN | 0.268965 | 0.750963 | 0.843972 | 0.001727 | 0.001515 | 0.001664 |
| list_iterator.test | RUN | 0.178136 | 0.432007 | 0.465420 | 0.001474 | 0.001704 | 0.001642 |
| list_hash.test | RUN | 0.251162 | 0.803337 | 0.686550 | 0.001407 | 0.001722 | 0.001763 |
| list_make.test | RUN | 0.186334 | 0.461143 | 0.522495 | 0.001861 | 0.001725 | 0.001732 |
| list_misc.test | RUN | 0.246179 | 0.545491 | 0.642181 | 0.001356 | 0.001774 | 0.001515 |
| list_mutate.test | RUN | 0.178968 | 0.366004 | 0.454243 | 0.001489 | 0.001511 | 0.001892 |
| list_nest.test | RUN | 0.325864 | 1.033533 | 1.289790 | 0.001544 | 0.001548 | 0.001746 |
| list_tie.test | RUN | 0.181292 | 0.405855 | 0.512254 | 0.001499 | 0.001790 | 0.001817 |
| list_value_at.test | RUN | 0.172052 | 0.390705 | 0.471635 | 0.001302 | 0.001663 | 0.001792 |
| deque_comparison.test | RUN | 0.180733 | 0.451032 | 0.550752 | 0.001288 | 0.001582 | 0.001842 |
| deque_construction.test | RUN | 0.189037 | 0.408726 | 0.532757 | 0.001754 | 0.001776 | 0.001640 |
| deque_copy.test | RUN | 0.266849 | 0.704233 | 0.837612 | 0.001840 | 0.001598 | 0.001724 |
| deque_iterator.test | RUN | 0.186255 | 0.435775 | 0.522737 | 0.001838 | 0.001531 | 0.001847 |
| deque_hash.test | RUN | 0.255973 | 0.785723 | 0.731819 | 0.001675 | 0.001740 | 0.001711 |
| deque_is_constructible.test | COMPILE | 0.098161 | 0.201336 | 0.241415 | — | — | — |
| deque_make.test | RUN | 0.187788 | 0.444000 | 0.532123 | 0.001818 | 0.001765 | 0.001690 |
| deque_misc.test | RUN | 0.245286 | 0.563845 | 0.660720 | 0.001438 | 0.001440 | 0.001649 |
| deque_move.test | RUN | 0.196421 | 0.455585 | 0.505837 | 0.001491 | 0.001512 | 0.001731 |
| deque_mutate.test | RUN | 0.173517 | 0.387607 | 0.454274 | 0.001481 | 0.001858 | 0.001732 |
| deque_nest.test | RUN | 0.511477 | 1.681496 | 2.080460 | 0.001727 | 0.001553 | 0.001707 |
| deque_tie.test | RUN | 0.182484 | 0.385101 | 0.493255 | 0.001555 | 0.001616 | 0.001408 |
| deque_value_at.test | RUN | 0.175444 | 0.374430 | 0.461550 | 0.001438 | 0.001382 | 0.001474 |
| front_extended_deque.test | RUN | 0.251883 | 0.566190 | 0.695067 | 0.001677 | 0.001840 | 0.001746 |
| back_extended_deque.test | RUN | 0.244675 | 0.558846 | 0.709306 | 0.001525 | 0.001733 | 0.001844 |
| make_list.test | RUN | 0.191306 | 0.452853 | 0.550382 | 0.001746 | 0.001746 | 0.001576 |
| make_vector.test | RUN | 0.193235 | 0.474020 | 0.529696 | 0.001606 | 0.001716 | 0.002194 |
| map.test | RUN | 0.252115 | 0.693163 | 0.837768 | 0.001697 | 0.001660 | 0.001573 |
| map_comparison.test | RUN | 0.192168 | 0.436947 | 0.554341 | 0.001770 | 0.001776 | 0.001734 |
| map_construction.test | RUN | 0.196665 | 0.421399 | 0.522442 | 0.001527 | 0.001707 | 0.001780 |
| map_copy.test | RUN | 0.233464 | 0.558272 | 0.662330 | 0.001878 | 0.001725 | 0.001779 |
| map_misc.test | RUN | 0.251338 | 0.547326 | 0.685120 | 0.001837 | 0.001551 | 0.001532 |
| map_move.test | RUN | 0.196839 | 0.463272 | 0.560757 | 0.001476 | 0.001588 | 0.001536 |
| map_mutate.test | RUN | 0.180391 | 0.358637 | 0.484836 | 0.001492 | 0.001518 | 0.001446 |
| map_tie.test | RUN | 0.184762 | 0.374200 | 0.481379 | 0.001698 | 0.001824 | 0.001726 |
| nil.test | RUN | 0.118464 | 0.273232 | 0.347083 | 0.001790 | 0.001659 | 0.001832 |
| nview.test | RUN | 0.240534 | 0.544140 | 0.706588 | 0.001763 | 0.001704 | 0.001498 |
| reverse_view.test | RUN | 0.312297 | 0.884078 | 1.052658 | 0.001805 | 0.001711 | 0.003480 |
| segmented_iterator_range.test | RUN | 0.386942 | 1.039701 | 1.398030 | 0.001780 | 0.001738 | 0.001625 |
| set.test | RUN | 0.235004 | 0.606483 | 0.792959 | 0.001520 | 0.001759 | 0.001872 |
| single_view.test | RUN | 0.217127 | 0.541727 | 0.695886 | 0.001787 | 0.001846 | 0.001806 |
| std_pair.test | RUN | 0.275676 | 0.713158 | 0.870970 | 0.001616 | 0.001780 | 0.001813 |
| boost_array.test | RUN | 0.214276 | 0.488693 | 0.630403 | 0.001718 | 0.001772 | 0.001732 |
| array.test | RUN | 0.219783 | 0.498049 | 0.602673 | 0.001497 | 0.001518 | 0.001856 |
| std_array.test | RUN | 0.220302 | 0.498269 | 0.613806 | 0.001570 | 0.001718 | 0.001716 |
| tuple_comparison.test | RUN | 0.195567 | 0.450455 | 0.588418 | 0.001595 | 0.001679 | 0.001799 |
| tuple_construction.test | RUN | 0.196188 | 0.448470 | 0.603573 | 0.001723 | 0.001824 | 0.001907 |
| tuple_conversion.test | RUN | 0.428222 | 1.410031 | 2.004960 | 0.001433 | 0.001494 | 0.001628 |
| tuple_copy.test | RUN | 0.285065 | 0.804103 | 1.006182 | 0.001706 | 0.001477 | 0.001673 |
| tuple_element.test | RUN | 0.186997 | 0.409857 | 0.538033 | 0.001362 | 0.001782 | 0.001500 |
| tuple_make.test | RUN | 0.205935 | 0.476051 | 0.581850 | 0.001782 | 0.001836 | 0.001652 |
| tuple_misc.test | RUN | 0.253739 | 0.607447 | 0.720072 | 0.001538 | 0.001728 | 0.001543 |
| tuple_mutate.test | RUN | 0.182292 | 0.418773 | 0.509584 | 0.001750 | 0.001710 | 0.001728 |
| tuple_nest.test | RUN | 0.748081 | 2.936576 | 3.530762 | 0.001640 | 0.001732 | 0.001681 |
| tuple_hash.test | RUN | 0.265061 | 0.802016 | 0.740106 | 0.001966 | 0.001612 | 0.001414 |
| tuple_tie.test | RUN | 0.193893 | 0.428655 | 0.549578 | 0.001761 | 0.001616 | 0.001700 |
| tuple_traits__maybe_variadic.test | RUN | 0.210076 | 0.520097 | 0.584879 | 0.001521 | 0.001525 | 0.001414 |
| tuple_traits__no_variadic.test | RUN | 0.229182 | 0.534727 | 0.590079 | 0.001386 | 0.001798 | 0.001631 |
| transform_view.test | RUN | 0.300827 | 0.892661 | 0.983937 | 0.001553 | 0.001684 | 0.001412 |
| identity_view.test | RUN | 0.290393 | 0.803518 | 0.929329 | 0.001431 | 0.001732 | 0.001656 |
| vector_comparison.test | RUN | 0.186709 | 0.446811 | 0.589747 | 0.001472 | 0.001453 | 0.001668 |
| vector_construction.test | RUN | 0.195578 | 0.406809 | 0.492625 | 0.001673 | 0.001850 | 0.001733 |
| vector_conversion.test | RUN | 0.508747 | 1.844679 | 2.321911 | 0.001719 | 0.001697 | 0.001824 |
| vector_copy.test | RUN | 0.278515 | 0.824159 | 0.935733 | 0.001311 | 0.001811 | 0.001557 |
| vector_iterator.test | RUN | 0.175486 | 0.431901 | 0.508099 | 0.001663 | 0.001837 | 0.001891 |
| vector_make.test | RUN | 0.195298 | 0.460683 | 0.541515 | 0.001438 | 0.001695 | 0.001770 |
| vector_misc.test | RUN | 0.242680 | 0.583316 | 0.648888 | 0.001586 | 0.001606 | 0.001384 |
| vector_move.test | RUN | 0.187581 | 0.449892 | 0.549196 | 0.001441 | 0.001495 | 0.001641 |
| vector_mutate.test | RUN | 0.168165 | 0.374411 | 0.440361 | 0.001441 | 0.001575 | 0.001384 |
| vector_n.test | RUN | 0.204949 | 0.464267 | 0.599229 | 0.001742 | 0.001772 | 0.001531 |
| vector_nest.test | RUN | 0.698715 | 3.083120 | 3.469692 | 0.001596 | 0.001834 | 0.001192 |
| vector_hash.test | RUN | 0.241594 | 0.784219 | 0.680332 | 0.001703 | 0.001814 | 0.001792 |
| vector_tie.test | RUN | 0.171078 | 0.412236 | 0.504375 | 0.001564 | 0.001796 | 0.001610 |
| vector_traits__maybe_variadic.test | RUN | 0.185203 | 0.473877 | 0.499437 | 0.001712 | 0.001674 | 0.001446 |
| vector_traits__no_variadic.test | RUN | 0.203747 | 0.501392 | 0.514439 | 0.001429 | 0.001658 | 0.001530 |
| vector_value_at.test | RUN | 0.173972 | 0.383507 | 0.469992 | 0.001539 | 0.001712 | 0.001726 |
| zip_view.test | RUN | 0.343656 | 0.921229 | 1.166828 | 0.001508 | 0.001484 | 0.001804 |
| zip_view2.test | RUN | 0.279391 | 0.698872 | 0.874107 | 0.001849 | 0.001583 | 0.001680 |
| zip_view_ignore.test | RUN | 0.259737 | 0.657275 | 0.788161 | 0.001579 | 0.001738 | 0.001682 |
| repetitive_view.test | RUN | 0.465021 | 1.181594 | 1.520132 | 0.001587 | 0.001764 | 0.001669 |
| deduce_sequence.test | RUN | 0.219667 | 0.493474 | 0.652264 | 0.001715 | 0.001605 | 0.001557 |
| adapt_adt_named.test | RUN | 0.261030 | 0.721216 | 0.771933 | 0.001604 | 0.001824 | 0.001786 |
| adapt_adt_named_empty.test | RUN | 0.223209 | 0.567965 | 0.652721 | 0.001522 | 0.001924 | 0.001544 |
| adapt_adt.test | RUN | 0.290962 | 0.943260 | 0.928743 | 0.001756 | 0.001770 | 0.001450 |
| adapt_adt_empty.test | RUN | 0.226701 | 0.585778 | 0.633469 | 0.001480 | 0.001769 | 0.001710 |
| adapt_assoc_adt_named.test | RUN | 0.296502 | 0.771987 | 0.837532 | 0.001748 | 0.003519 | 0.001710 |
| adapt_assoc_adt_named_empty.test | RUN | 0.223000 | 0.582171 | 0.635206 | 0.001335 | 0.001857 | 0.001706 |
| adapt_assoc_adt.test | RUN | 0.297824 | 0.802675 | 0.905905 | 0.001278 | 0.001512 | 0.001916 |
| adapt_assoc_adt_empty.test | RUN | 0.226232 | 0.616131 | 0.625335 | 0.001583 | 0.001716 | 0.001748 |
| adapt_assoc_struct_named.test | RUN | 0.288584 | 0.804237 | 0.874040 | 0.001554 | 0.001578 | 0.001774 |
| adapt_assoc_struct_named_empty.test | RUN | 0.232164 | 0.615444 | 0.663958 | 0.001534 | 0.001867 | 0.001662 |
| adapt_assoc_struct.test | RUN | 0.256333 | 0.703228 | 0.810279 | 0.001492 | 0.001721 | 0.001857 |
| adapt_assoc_struct_empty.test | RUN | 0.218459 | 0.588970 | 0.697142 | 0.001577 | 0.001716 | 0.001822 |
| adapt_assoc_tpl_adt.test | RUN | 0.306311 | 0.781046 | 0.944121 | 0.001554 | 0.001850 | 0.001910 |
| adapt_assoc_tpl_adt_empty.test | RUN | 0.227373 | 0.589750 | 0.669467 | 0.001795 | 0.001704 | 0.001860 |
| adapt_assoc_tpl_struct.test | RUN | 0.294219 | 0.684577 | 0.804775 | 0.001511 | 0.001603 | 0.001590 |
| adapt_assoc_tpl_struct_empty.test | RUN | 0.262883 | 0.583029 | 0.652743 | 0.001756 | 0.001555 | 0.001402 |
| adapt_struct_named.test | RUN | 0.276565 | 0.704906 | 0.800723 | 0.001722 | 0.001558 | 0.001742 |
| adapt_struct_named_empty.test | RUN | 0.230739 | 0.568386 | 0.668886 | 0.001461 | 0.001721 | 0.001436 |
| adapt_struct.test | RUN | 0.278550 | 0.805764 | 0.882818 | 0.001766 | 0.001768 | 0.001813 |
| adapt_struct_empty.test | RUN | 0.222039 | 0.586226 | 0.668232 | 0.001878 | 0.001876 | 0.001804 |
| adapt_tpl_adt.test | RUN | 0.297648 | 0.920149 | 0.927505 | 0.001537 | 0.001661 | 0.001696 |
| adapt_tpl_adt_empty.test | RUN | 0.228975 | 0.583529 | 0.673766 | 0.001676 | 0.001779 | 0.001756 |
| adapt_tpl_struct.test | RUN | 0.241186 | 0.639332 | 0.727161 | 0.001813 | 0.001493 | 0.001434 |
| adapt_tpl_struct_empty.test | RUN | 0.222505 | 0.594384 | 0.681856 | 0.001469 | 0.001832 | 0.001820 |
| adt_attribute_proxy.test | RUN | 0.254625 | 0.566574 | 0.712381 | 0.001506 | 0.001792 | 0.001484 |
| define_struct.test | RUN | 0.309195 | 0.811489 | 0.924510 | 0.001718 | 0.001764 | 0.001866 |
| define_struct_empty.test | RUN | 0.263558 | 0.661274 | 0.755717 | 0.001490 | 0.001546 | 0.001546 |
| define_struct_move.test | RUN | 0.171566 | 0.369150 | 0.442338 | 0.001698 | 0.001730 | 0.001684 |
| define_struct_inline.test | RUN | 0.328777 | 0.974098 | 1.061897 | 0.001653 | 0.001757 | 0.001562 |
| define_struct_inline_empty.test | RUN | 0.273394 | 0.709483 | 0.773070 | 0.001491 | 0.001425 | 0.001722 |
| define_struct_inline_move.test | RUN | 0.175926 | 0.385015 | 0.478997 | 0.001585 | 0.001774 | 0.001534 |
| define_assoc_struct.test | RUN | 0.300125 | 0.790247 | 0.927050 | 0.002670 | 0.001795 | 0.001712 |
| define_assoc_struct_empty.test | RUN | 0.277342 | 0.644687 | 0.784342 | 0.001472 | 0.001709 | 0.001761 |
| define_assoc_struct_move.test | RUN | 0.183063 | 0.368794 | 0.449311 | 0.001739 | 0.001447 | 0.001770 |
| define_tpl_struct.test | RUN | 0.305393 | 0.758129 | 0.905679 | 0.001732 | 0.001836 | 0.001806 |
| define_tpl_struct_empty.test | RUN | 0.279901 | 0.672815 | 0.785695 | 0.001835 | 0.001752 | 0.001424 |
| define_tpl_struct_move.test | RUN | 0.181420 | 0.365829 | 0.470463 | 0.001454 | 0.001774 | 0.001698 |
| define_tpl_struct_inline.test | RUN | 0.319449 | 0.886078 | 1.010801 | 0.001745 | 0.001653 | 0.001761 |
| define_tpl_struct_inline_empty.test | RUN | 0.273279 | 0.641843 | 0.908018 | 0.001674 | 0.001480 | 0.001982 |
| define_tpl_struct_inline_move.test | RUN | 0.179776 | 0.385208 | 0.485707 | 0.001445 | 0.001674 | 0.002717 |
| define_assoc_tpl_struct.test | RUN | 0.304703 | 0.789040 | 1.032171 | 0.001743 | 0.001681 | 0.002076 |
| define_assoc_tpl_struct_empty.test | RUN | 0.277723 | 0.663071 | 0.825763 | 0.001515 | 0.001715 | 0.001579 |
| define_assoc_tpl_struct_move.test | RUN | 0.183379 | 0.390144 | 0.501283 | 0.001515 | 0.001560 | 0.001866 |
| std_tuple.test | RUN | 0.186269 | 0.426158 | 0.523073 | 0.001498 | 0.001654 | 0.001819 |
| std_tuple_iterator.test | RUN | 0.181651 | 0.433366 | 0.528707 | 0.001647 | 0.001546 | 0.001945 |
| ref_vector.test | RUN | 0.202112 | 0.452362 | 0.547296 | 0.001663 | 0.001754 | 0.001821 |
| flatten_view.test | RUN | 0.215884 | 0.546342 | 0.685114 | 0.001525 | 0.001786 | 0.001788 |
| github-159.test | COMPILE | 0.096114 | 0.208198 | 0.253128 | — | — | — |
| github-176.test | RUN | 0.206647 | 0.471485 | 0.578543 | 0.001733 | 0.001861 | 0.001818 |
| size.test | COMPILE | 0.172216 | 0.338991 | 0.440308 | — | — | — |
| fused.test | RUN | 0.237017 | 0.531992 | 0.626066 | 0.001478 | 0.001684 | 0.001795 |
| fused_function_object.test | RUN | 0.170812 | 0.382302 | 0.522239 | 0.001573 | 0.001620 | 0.001624 |
| fused_procedure.test | RUN | 0.232480 | 0.503777 | 0.651485 | 0.001544 | 0.001621 | 0.002162 |
| unfused.test | RUN | 0.183538 | 0.410755 | 0.527340 | 0.001743 | 0.001784 | 0.001596 |
| unfused_typed.test | RUN | 0.189147 | 0.436103 | 0.518283 | 0.001642 | 0.001710 | 0.001900 |
| make_fused.test | RUN | 0.245347 | 0.518409 | 0.645751 | 0.001679 | 0.001543 | 0.001626 |
| make_fused_function_object.test | RUN | 0.186650 | 0.402939 | 0.515488 | 0.001522 | 0.001841 | 0.001758 |
| make_fused_procedure.test | RUN | 0.243545 | 0.503840 | 0.607932 | 0.001860 | 0.001702 | 0.001702 |
| make_unfused.test | RUN | 0.191017 | 0.401155 | 0.504822 | 0.001717 | 0.001849 | 0.001896 |
| invoke.test | RUN | 0.476830 | 1.432680 | 1.791828 | 0.001583 | 0.001468 | 0.001454 |
| invoke_function_object.test | RUN | 0.224572 | 0.505979 | 0.649107 | 0.001860 | 0.001825 | 0.001972 |
| invoke_procedure.test | RUN | 0.374385 | 1.051550 | 1.284641 | 0.001606 | 0.001713 | 0.001561 |
| swap.test | RUN | 0.288692 | 0.681389 | 0.854263 | 0.001433 | 0.001752 | 0.001758 |
| is_sequence.test | COMPILE | 0.049805 | 0.097176 | 0.112330 | — | — | — |
| is_view.test | COMPILE | 0.044671 | 0.096787 | 0.105759 | — | — | — |
| pair_deque.test | COMPILE | 0.101103 | 0.211781 | 0.232908 | — | — | — |
| pair_list.test | COMPILE | 0.098219 | 0.204261 | 0.246482 | — | — | — |
| pair_map.test | COMPILE | 0.102093 | 0.216716 | 0.262223 | — | — | — |
| pair_set.test | COMPILE | 0.101585 | 0.214353 | 0.249515 | — | — | — |
| pair_vector.test | COMPILE | 0.097954 | 0.213991 | 0.253355 | — | — | — |
| pair_nest.test | COMPILE | 0.041045 | 0.086292 | 0.104083 | — | — | — |
| index_sequence.test | COMPILE | 0.034787 | 0.062057 | 0.076747 | — | — | — |
| and.test | COMPILE | 0.026836 | 0.049112 | 0.058260 | — | — | — |
| tag_of.test | COMPILE | 0.045646 | 0.091243 | 0.102283 | — | — | — |
| unused.test | COMPILE | 0.026338 | 0.060818 | 0.064227 | — | — | — |
| tag_of_fallback.test | COMPILE | 0.044057 | 0.086892 | 0.100983 | — | — | — |
| make_pair_r-value.test | COMPILE | 0.040355 | 0.065679 | 0.095248 | — | — | — |
| quick.test | RUN | 0.131642 | 0.286268 | 0.321851 | 0.000531 | 0.001191 | 0.001313 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.992× | 0.980×, 1.003× | 0.985× |
| clang | 0.990× | 1.007×, 0.973× | 0.984× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051632).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adapt_adt.test | 10 | 0.000937s | 0.000947s | 0.000937s | byte equality observed |
| adapt_adt_empty.test | 10 | 0.000920s | 0.000941s | 0.000916s | byte equality observed |
| adapt_adt_named.test | 10 | 0.000944s | 0.000925s | 0.000923s | byte equality observed |
| adapt_adt_named_empty.test | 10 | 0.000922s | 0.000933s | 0.000909s | byte equality observed |
| adapt_assoc_adt.test | 10 | 0.000922s | 0.000910s | 0.000928s | byte equality observed |
| adapt_assoc_adt_empty.test | 10 | 0.000910s | 0.000909s | 0.000908s | byte equality observed |
| adapt_assoc_adt_named.test | 10 | 0.000919s | 0.000909s | 0.000928s | byte equality observed |
| adapt_assoc_adt_named_empty.test | 10 | 0.000920s | 0.000919s | 0.000909s | byte equality observed |
| adapt_assoc_struct.test | 10 | 0.000922s | 0.000917s | 0.000937s | byte equality observed |
| adapt_assoc_struct_empty.test | 10 | 0.000926s | 0.000918s | 0.000930s | byte equality observed |
| adapt_assoc_struct_named.test | 10 | 0.000932s | 0.000932s | 0.000931s | byte equality observed |
| adapt_assoc_struct_named_empty.test | 10 | 0.000912s | 0.000919s | 0.000919s | byte equality observed |
| adapt_assoc_tpl_adt.test | 10 | 0.000932s | 0.000940s | 0.000907s | byte equality observed |
| adapt_assoc_tpl_adt_empty.test | 10 | 0.000935s | 0.000920s | 0.000899s | byte equality observed |
| adapt_assoc_tpl_struct.test | 10 | 0.000950s | 0.000931s | 0.000945s | byte equality observed |
| adapt_assoc_tpl_struct_empty.test | 10 | 0.000924s | 0.000912s | 0.000922s | byte equality observed |
| adapt_struct.test | 10 | 0.000948s | 0.000926s | 0.000935s | byte equality observed |
| adapt_struct_empty.test | 10 | 0.000920s | 0.000910s | 0.000927s | byte equality observed |
| adapt_struct_named.test | 10 | 0.000938s | 0.000908s | 0.000941s | byte equality observed |
| adapt_struct_named_empty.test | 10 | 0.000915s | 0.000897s | 0.000930s | byte equality observed |
| adapt_tpl_adt.test | 10 | 0.000930s | 0.000930s | 0.000935s | byte equality observed |
| adapt_tpl_adt_empty.test | 10 | 0.000916s | 0.000912s | 0.000916s | byte equality observed |
| adapt_tpl_struct.test | 10 | 0.000944s | 0.000925s | 0.000933s | byte equality observed |
| adapt_tpl_struct_empty.test | 10 | 0.000925s | 0.000901s | 0.000921s | byte equality observed |
| adt_attribute_proxy.test | 10 | 0.000924s | 0.000896s | 0.000918s | byte equality observed |
| all.test | 10 | 0.000912s | 0.000899s | 0.000909s | byte equality observed |
| any.test | 10 | 0.000924s | 0.000912s | 0.000901s | byte equality observed |
| array.test | 10 | 0.000922s | 0.000873s | 0.000904s | byte equality observed |
| as_deque.test | 9 | 0.000911s | 0.000884s | 0.000917s | byte equality observed |
| as_list.test | 10 | 0.000932s | 0.000922s | 0.000927s | byte equality observed |
| as_map.test | 10 | 0.000922s | 0.000900s | 0.000925s | byte equality observed |
| as_map_assoc.test | 10 | 0.000928s | 0.000921s | 0.000899s | byte equality observed |
| as_set.test | 10 | 0.000934s | 0.000917s | 0.000913s | byte equality observed |
| as_vector.test | 10 | 0.000943s | 0.000921s | 0.000920s | byte equality observed |
| back_extended_deque.test | 10 | 0.000895s | 0.000890s | 0.000925s | byte equality observed |
| boost_array.test | 9 | 0.000908s | 0.000886s | 0.000903s | byte equality observed |
| boost_tuple.test | 10 | 0.000930s | 0.000916s | 0.000924s | byte equality observed |
| boost_tuple_iterator.test | 10 | 0.000903s | 0.000886s | 0.000899s | byte equality observed |
| clear.test | 10 | 0.000936s | 0.000897s | 0.000916s | byte equality observed |
| cons.test | 10 | 0.000958s | 0.000926s | 0.000918s | byte equality observed |
| convert_boost_tuple.test | 10 | 0.000906s | 0.000896s | 0.000898s | byte equality observed |
| convert_deque.test | 10 | 0.000922s | 0.000876s | 0.000884s | byte equality observed |
| convert_list.test | 10 | 0.000910s | 0.000888s | 0.000895s | byte equality observed |
| convert_std_pair.test | 10 | 0.000914s | 0.000894s | 0.000894s | byte equality observed |
| convert_std_tuple.test | 10 | 0.000907s | 0.000903s | 0.000912s | byte equality observed |
| convert_vector.test | 10 | 0.000900s | 0.000903s | 0.000901s | byte equality observed |
| copy.test | 10 | 0.000911s | 0.000907s | 0.000917s | byte equality observed |
| count.test | 10 | 0.000901s | 0.000888s | 0.000911s | byte equality observed |
| count_if.test | 10 | 0.000896s | 0.000878s | 0.000890s | byte equality observed |
| deduce_sequence.test | 10 | 0.000887s | 0.000883s | 0.000915s | byte equality observed |
| define_assoc_struct.test | 10 | 0.000941s | 0.000893s | 0.000903s | byte equality observed |
| define_assoc_struct_empty.test | 10 | 0.000939s | 0.000903s | 0.000927s | byte equality observed |
| define_assoc_struct_move.test | 10 | 0.000911s | 0.000886s | 0.000890s | byte equality observed |
| define_assoc_tpl_struct.test | 10 | 0.000914s | 0.000916s | 0.000940s | byte equality observed |
| define_assoc_tpl_struct_empty.test | 10 | 0.000910s | 0.000921s | 0.000913s | byte equality observed |
| define_assoc_tpl_struct_move.test | 10 | 0.000904s | 0.000885s | 0.000907s | byte equality observed |
| define_struct.test | 10 | 0.000935s | 0.000930s | 0.000921s | byte equality observed |
| define_struct_empty.test | 10 | 0.000907s | 0.000913s | 0.000925s | byte equality observed |
| define_struct_inline.test | 10 | 0.000934s | 0.000919s | 0.000919s | byte equality observed |
| define_struct_inline_empty.test | 10 | 0.000907s | 0.000915s | 0.000910s | byte equality observed |
| define_struct_inline_move.test | 10 | 0.000904s | 0.000884s | 0.000917s | byte equality observed |
| define_struct_move.test | 10 | 0.000925s | 0.000905s | 0.000941s | byte equality observed |
| define_tpl_struct.test | 10 | 0.000919s | 0.000914s | 0.000927s | byte equality observed |
| define_tpl_struct_empty.test | 10 | 0.000909s | 0.000936s | 0.000920s | byte equality observed |
| define_tpl_struct_inline.test | 10 | 0.000945s | 0.000927s | 0.000943s | byte equality observed |
| define_tpl_struct_inline_empty.test | 10 | 0.000927s | 0.000910s | 0.000918s | byte equality observed |
| define_tpl_struct_inline_move.test | 10 | 0.000913s | 0.000879s | 0.000915s | byte equality observed |
| define_tpl_struct_move.test | 10 | 0.000913s | 0.000918s | 0.000893s | byte equality observed |
| deque_comparison.test | 10 | 0.000911s | 0.000892s | 0.000891s | byte equality observed |
| deque_construction.test | 10 | 0.000913s | 0.000890s | 0.000883s | byte equality observed |
| deque_copy.test | 10 | 0.000903s | 0.000941s | 0.000922s | byte equality observed |
| deque_hash.test | 10 | 0.000926s | 0.000918s | 0.000905s | byte equality observed |
| deque_iterator.test | 10 | 0.000914s | 0.000902s | 0.000902s | byte equality observed |
| deque_make.test | 10 | 0.000908s | 0.000888s | 0.000909s | byte equality observed |
| deque_misc.test | 10 | 0.000909s | 0.000910s | 0.000893s | byte equality observed |
| deque_move.test | 10 | 0.000923s | 0.000902s | 0.000935s | byte equality observed |
| deque_mutate.test | 10 | 0.000897s | 0.000891s | 0.000931s | byte equality observed |
| deque_nest.test | 10 | 0.000903s | 0.000911s | 0.000914s | byte equality observed |
| deque_tie.test | 10 | 0.000892s | 0.000878s | 0.000932s | byte equality observed |
| deque_value_at.test | 10 | 0.000900s | 0.000901s | 0.000917s | byte equality observed |
| erase.test | 10 | 0.000944s | 0.000915s | 0.000949s | byte equality observed |
| erase_key.test | 10 | 0.000933s | 0.000921s | 0.000958s | byte equality observed |
| filter.test | 10 | 0.000933s | 0.000901s | 0.000940s | byte equality observed |
| filter_if.test | 10 | 0.000928s | 0.000913s | 0.000916s | byte equality observed |
| filter_view.test | 10 | 0.000946s | 0.000901s | 0.000926s | byte equality observed |
| find.test | 10 | 0.000941s | 0.000906s | 0.000907s | byte equality observed |
| find_if.test | 10 | 0.000929s | 0.000899s | 0.000906s | byte equality observed |
| flatten.test | 10 | 0.000933s | 0.000900s | 0.000912s | byte equality observed |
| flatten_view.test | 10 | 0.000931s | 0.000896s | 0.000893s | byte equality observed |
| fold.test | 10 | 0.000941s | 0.000921s | 0.000953s | byte equality observed |
| for_each.test | 10 | 0.000947s | 0.000922s | 0.000930s | byte equality observed |
| front_extended_deque.test | 10 | 0.000924s | 0.000889s | 0.000899s | byte equality observed |
| fused.test | 10 | 0.000919s | 0.000869s | 0.000893s | byte equality observed |
| fused_function_object.test | 10 | 0.000907s | 0.000880s | 0.000889s | byte equality observed |
| fused_procedure.test | 10 | 0.000917s | 0.000885s | 0.000897s | byte equality observed |
| github-176.test | 10 | 0.000917s | 0.000890s | 0.000908s | byte equality observed |
| hash.test | 10 | 0.000923s | 0.000880s | 0.000891s | byte equality observed |
| identity_view.test | 10 | 0.000935s | 0.000906s | 0.000928s | byte equality observed |
| insert.test | 10 | 0.000953s | 0.000922s | 0.000910s | byte equality observed |
| insert_range.test | 10 | 0.000960s | 0.000922s | 0.000929s | byte equality observed |
| invoke.test | 10 | 0.000907s | 0.000919s | 0.000902s | byte equality observed |
| invoke_function_object.test | 10 | 0.000926s | 0.000898s | 0.000876s | byte equality observed |
| invoke_procedure.test | 10 | 0.000899s | 0.000901s | 0.000899s | byte equality observed |
| io.test | 10 | 0.001052s | 0.001008s | 0.001029s | byte equality observed |
| iter_fold.test | 10 | 0.000929s | 0.000936s | 0.000930s | byte equality observed |
| iterator_range.test | 10 | 0.000952s | 0.000925s | 0.000946s | byte equality observed |
| join.test | 10 | 0.000918s | 0.000890s | 0.000909s | byte equality observed |
| joint_view.test | 10 | 0.000966s | 0.000947s | 0.000953s | byte equality observed |
| list_comparison.test | 10 | 0.000895s | 0.000899s | 0.000904s | byte equality observed |
| list_construction.test | 10 | 0.000896s | 0.000921s | 0.000897s | byte equality observed |
| list_copy.test | 10 | 0.000900s | 0.000924s | 0.000940s | byte equality observed |
| list_hash.test | 10 | 0.000917s | 0.000891s | 0.000910s | byte equality observed |
| list_iterator.test | 10 | 0.000927s | 0.000908s | 0.000920s | byte equality observed |
| list_make.test | 10 | 0.000903s | 0.000910s | 0.000929s | byte equality observed |
| list_misc.test | 10 | 0.000921s | 0.000896s | 0.000905s | byte equality observed |
| list_mutate.test | 10 | 0.000919s | 0.000912s | 0.000900s | byte equality observed |
| list_nest.test | 10 | 0.000894s | 0.000899s | 0.000900s | byte equality observed |
| list_tie.test | 10 | 0.000889s | 0.000872s | 0.000896s | byte equality observed |
| list_value_at.test | 10 | 0.000912s | 0.000896s | 0.000900s | byte equality observed |
| make_fused.test | 10 | 0.000886s | 0.000896s | 0.000905s | byte equality observed |
| make_fused_function_object.test | 6 | 0.000887s | 0.000870s | 0.000950s | byte equality observed |
| make_fused_procedure.test | 10 | 0.000890s | 0.000879s | 0.000910s | byte equality observed |
| make_list.test | 10 | 0.000906s | 0.000891s | 0.000918s | byte equality observed |
| make_unfused.test | 10 | 0.000871s | 0.000893s | 0.000894s | byte equality observed |
| make_vector.test | 10 | 0.000905s | 0.000888s | 0.000914s | byte equality observed |
| map.test | 10 | 0.000935s | 0.000915s | 0.000917s | byte equality observed |
| map_comparison.test | 10 | 0.000901s | 0.000879s | 0.000921s | byte equality observed |
| map_construction.test | 10 | 0.000906s | 0.000876s | 0.000904s | byte equality observed |
| map_copy.test | 10 | 0.000916s | 0.000895s | 0.000905s | byte equality observed |
| map_misc.test | 10 | 0.000904s | 0.000893s | 0.000909s | byte equality observed |
| map_move.test | 10 | 0.000929s | 0.000922s | 0.000911s | byte equality observed |
| map_mutate.test | 10 | 0.000909s | 0.000897s | 0.000914s | byte equality observed |
| map_tie.test | 10 | 0.000905s | 0.000872s | 0.000908s | byte equality observed |
| move.test | 10 | 0.000913s | 0.000894s | 0.000912s | byte equality observed |
| nil.test | 10 | 0.000902s | 0.000884s | 0.000883s | byte equality observed |
| none.test | 10 | 0.000888s | 0.000910s | 0.000886s | byte equality observed |
| nview.test | 10 | 0.000929s | 0.000895s | 0.000902s | byte equality observed |
| pop_back.test | 10 | 0.000944s | 0.000944s | 0.000928s | byte equality observed |
| pop_front.test | 10 | 0.000949s | 0.000947s | 0.000932s | byte equality observed |
| push_back.test | 10 | 0.000950s | 0.000952s | 0.000936s | byte equality observed |
| push_front.test | 10 | 0.000949s | 0.000940s | 0.000932s | byte equality observed |
| quick.test | 10 | 0.000417s | 0.000877s | 0.000881s | byte equality observed |
| ref_vector.test | 10 | 0.000923s | 0.000911s | 0.000928s | byte equality observed |
| remove.test | 10 | 0.000948s | 0.000938s | 0.000947s | byte equality observed |
| remove_if.test | 10 | 0.000932s | 0.000936s | 0.000950s | byte equality observed |
| repetitive_view.test | 10 | 0.000919s | 0.000896s | 0.000916s | byte equality observed |
| replace.test | 10 | 0.000939s | 0.000923s | 0.000920s | byte equality observed |
| replace_if.test | 10 | 0.000939s | 0.000931s | 0.000945s | byte equality observed |
| reverse.test | 10 | 0.000932s | 0.000930s | 0.000940s | byte equality observed |
| reverse_fold.test | 10 | 0.000924s | 0.000905s | 0.000904s | byte equality observed |
| reverse_iter_fold.test | 10 | 0.000906s | 0.000930s | 0.000899s | byte equality observed |
| reverse_view.test | 10 | 0.000945s | 0.000943s | 0.000937s | byte equality observed |
| segmented_find.test | 10 | 0.000907s | 0.000904s | 0.000884s | byte equality observed |
| segmented_find_if.test | 10 | 0.000907s | 0.000893s | 0.000887s | byte equality observed |
| segmented_fold.test | 10 | 0.000932s | 0.000931s | 0.000928s | byte equality observed |
| segmented_for_each.test | 10 | 0.000952s | 0.000923s | 0.000896s | byte equality observed |
| segmented_iterator_range.test | 10 | 0.000949s | 0.000944s | 0.000926s | byte equality observed |
| set.test | 10 | 0.000939s | 0.000939s | 0.000910s | byte equality observed |
| single_view.test | 10 | 0.000931s | 0.000934s | 0.000897s | byte equality observed |
| std_array.test | 10 | 0.000924s | 0.000918s | 0.000878s | byte equality observed |
| std_pair.test | 10 | 0.000943s | 0.000943s | 0.000913s | byte equality observed |
| std_tuple.test | 10 | 0.000912s | 0.000888s | 0.000866s | byte equality observed |
| std_tuple_iterator.test | 10 | 0.000919s | 0.000921s | 0.000867s | byte equality observed |
| swap.test | 10 | 0.000921s | 0.000897s | 0.000886s | byte equality observed |
| transform.test | 10 | 0.000950s | 0.000928s | 0.000916s | byte equality observed |
| transform_view.test | 10 | 0.000958s | 0.000950s | 0.000928s | byte equality observed |
| tuple_comparison.test | 10 | 0.000915s | 0.000899s | 0.000888s | byte equality observed |
| tuple_construction.test | 10 | 0.000901s | 0.000910s | 0.000877s | byte equality observed |
| tuple_conversion.test | 10 | 0.000912s | 0.000889s | 0.000888s | byte equality observed |
| tuple_copy.test | 10 | 0.000935s | 0.000916s | 0.000869s | byte equality observed |
| tuple_element.test | 10 | 0.000923s | 0.000885s | 0.000873s | byte equality observed |
| tuple_hash.test | 10 | 0.000911s | 0.000902s | 0.000893s | byte equality observed |
| tuple_make.test | 10 | 0.000899s | 0.000905s | 0.000897s | byte equality observed |
| tuple_misc.test | 10 | 0.000910s | 0.000895s | 0.000901s | byte equality observed |
| tuple_mutate.test | 10 | 0.000911s | 0.000894s | 0.000884s | byte equality observed |
| tuple_nest.test | 10 | 0.000900s | 0.000889s | 0.000896s | byte equality observed |
| tuple_tie.test | 10 | 0.000902s | 0.000919s | 0.000895s | byte equality observed |
| tuple_traits__maybe_variadic.test | 10 | 0.000894s | 0.000921s | 0.000916s | byte equality observed |
| tuple_traits__no_variadic.test | 10 | 0.000901s | 0.000908s | 0.000912s | byte equality observed |
| unfused.test | 10 | 0.000897s | 0.000897s | 0.000890s | byte equality observed |
| unfused_typed.test | 10 | 0.000912s | 0.000888s | 0.000906s | byte equality observed |
| vector_comparison.test | 10 | 0.000901s | 0.000889s | 0.000897s | byte equality observed |
| vector_construction.test | 10 | 0.000905s | 0.000885s | 0.000892s | byte equality observed |
| vector_conversion.test | 10 | 0.000903s | 0.000875s | 0.000922s | byte equality observed |
| vector_copy.test | 10 | 0.000919s | 0.000884s | 0.000915s | byte equality observed |
| vector_hash.test | 10 | 0.000908s | 0.000875s | 0.000893s | byte equality observed |
| vector_iterator.test | 10 | 0.000886s | 0.000897s | 0.000896s | byte equality observed |
| vector_make.test | 10 | 0.000897s | 0.000899s | 0.000885s | byte equality observed |
| vector_misc.test | 10 | 0.000911s | 0.000882s | 0.000906s | byte equality observed |
| vector_move.test | 10 | 0.000929s | 0.000900s | 0.000905s | byte equality observed |
| vector_mutate.test | 10 | 0.000917s | 0.000883s | 0.000871s | byte equality observed |
| vector_n.test | 10 | 0.000932s | 0.000907s | 0.000920s | byte equality observed |
| vector_nest.test | 10 | 0.000915s | 0.000893s | 0.000874s | byte equality observed |
| vector_tie.test | 10 | 0.000913s | 0.000897s | 0.000882s | byte equality observed |
| vector_traits__maybe_variadic.test | 10 | 0.000942s | 0.000893s | 0.000879s | byte equality observed |
| vector_traits__no_variadic.test | 10 | 0.000909s | 0.000890s | 0.000877s | byte equality observed |
| vector_value_at.test | 10 | 0.000922s | 0.000882s | 0.000898s | byte equality observed |
| zip.test | 10 | 0.000904s | 0.000896s | 0.000899s | byte equality observed |
| zip2.test | 10 | 0.000904s | 0.000903s | 0.000901s | byte equality observed |
| zip_ignore.test | 10 | 0.000914s | 0.000884s | 0.000891s | byte equality observed |
| zip_view.test | 10 | 0.000910s | 0.000886s | 0.000895s | byte equality observed |
| zip_view2.test | 10 | 0.000919s | 0.000879s | 0.000920s | byte equality observed |
| zip_view_ignore.test | 10 | 0.000898s | 0.000894s | 0.000893s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051632).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
