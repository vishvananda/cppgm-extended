# libs/fusion/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| all.test | RUN | 0.356835 | 0.739529 | 1.053687 | 0.001798 | 0.001603 | 0.001524 |
| any.test | RUN | 0.354183 | 0.690333 | 0.990363 | 0.001433 | 0.001620 | 0.001870 |
| clear.test | RUN | 0.247811 | 0.638002 | 0.806656 | 0.001724 | 0.001755 | 0.001858 |
| copy.test | RUN | 0.212690 | 0.485828 | 0.656056 | 0.001754 | 0.001462 | 0.001548 |
| count.test | RUN | 0.240969 | 0.550122 | 0.716140 | 0.001788 | 0.002027 | 0.001715 |
| count_if.test | RUN | 0.232246 | 0.507354 | 0.672630 | 0.001862 | 0.001911 | 0.001751 |
| erase.test | RUN | 0.336534 | 0.907188 | 1.076659 | 0.001632 | 0.001894 | 0.001756 |
| erase_key.test | RUN | 0.317298 | 0.890067 | 1.024542 | 0.001754 | 0.001730 | 0.001694 |
| filter.test | RUN | 0.292444 | 0.729255 | 0.925632 | 0.001724 | 0.002364 | 0.001856 |
| filter_if.test | RUN | 0.337334 | 0.878446 | 1.103015 | 0.001628 | 0.001858 | 0.001902 |
| find.test | RUN | 0.279823 | 0.617410 | 0.837317 | 0.001812 | 0.001836 | 0.001668 |
| find_if.test | RUN | 0.274189 | 0.558740 | 0.768770 | 0.001547 | 0.001800 | 0.001918 |
| fold.test | RUN | 0.296689 | 0.709852 | 0.937331 | 0.001985 | 0.001939 | 0.001836 |
| for_each.test | RUN | 0.263992 | 0.634571 | 0.815138 | 0.001809 | 0.002008 | 0.001855 |
| insert.test | RUN | 0.383150 | 1.125197 | 1.321043 | 0.001988 | 0.001766 | 0.001666 |
| insert_range.test | RUN | 0.381447 | 1.146545 | 1.331426 | 0.001632 | 0.001764 | 0.001716 |
| iter_fold.test | RUN | 0.450010 | 1.094855 | 1.610630 | 0.001870 | 0.001878 | 0.001890 |
| move.test | RUN | 0.218791 | 0.523549 | 0.684364 | 0.001957 | 0.001527 | 0.001462 |
| none.test | RUN | 0.361626 | 0.719736 | 1.026446 | 0.001664 | 0.001670 | 0.001884 |
| pop_back.test | RUN | 0.348037 | 0.991004 | 1.185482 | 0.001647 | 0.002051 | 0.001942 |
| pop_front.test | RUN | 0.304277 | 0.789724 | 0.963870 | 0.001930 | 0.001954 | 0.001584 |
| push_back.test | RUN | 0.373917 | 1.126230 | 1.261595 | 0.002229 | 0.001928 | 0.001881 |
| push_front.test | RUN | 0.320690 | 0.893985 | 1.096818 | 0.001852 | 0.001847 | 0.001954 |
| remove.test | RUN | 0.357914 | 1.062047 | 1.304257 | 0.001640 | 0.002148 | 0.001661 |
| remove_if.test | RUN | 0.331222 | 0.899878 | 1.109511 | 0.001950 | 0.001708 | 0.001934 |
| replace.test | RUN | 0.317901 | 0.826987 | 1.012778 | 0.002040 | 0.001879 | 0.001924 |
| replace_if.test | RUN | 0.315149 | 0.811546 | 0.995565 | 0.001974 | 0.001967 | 0.001925 |
| reverse_fold.test | RUN | 0.505244 | 1.229342 | 1.744126 | 0.001700 | 0.001774 | 0.001794 |
| reverse_iter_fold.test | RUN | 0.516838 | 1.256449 | 1.851459 | 0.001665 | 0.001574 | 0.001509 |
| reverse.test | RUN | 0.315175 | 0.891445 | 1.032068 | 0.001689 | 0.001814 | 0.001761 |
| segmented_for_each.test | RUN | 0.548461 | 1.669291 | 2.133369 | 0.001756 | 0.001559 | 0.001547 |
| segmented_find.test | RUN | 0.337898 | 0.900540 | 1.196704 | 0.001693 | 0.001636 | 0.001552 |
| segmented_find_if.test | RUN | 0.345919 | 0.875446 | 1.202356 | 0.001405 | 0.001788 | 0.001965 |
| segmented_fold.test | RUN | 0.424086 | 1.171428 | 1.488811 | 0.001848 | 0.002006 | 0.001849 |
| transform.test | RUN | 0.322325 | 0.908972 | 1.105116 | 0.001624 | 0.001742 | 0.001965 |
| join.test | RUN | 0.245535 | 0.568207 | 0.795631 | 0.001760 | 0.001862 | 0.001868 |
| zip.test | RUN | 0.445018 | 1.045811 | 1.465795 | 0.001780 | 0.001793 | 0.001878 |
| zip2.test | RUN | 0.430343 | 1.033823 | 1.401676 | 0.001724 | 0.001528 | 0.001634 |
| zip_ignore.test | RUN | 0.318423 | 0.703161 | 0.943413 | 0.001874 | 0.001644 | 0.001601 |
| flatten.test | RUN | 0.298949 | 0.781459 | 0.952661 | 0.001619 | 0.002037 | 0.001818 |
| ticket-5490.test | COMPILE | 0.160754 | 0.335335 | 0.417444 | — | — | — |
| as_deque.test | RUN | 0.304649 | 0.736720 | 1.023671 | 0.001590 | 0.002037 | 0.001461 |
| as_list.test | RUN | 0.331607 | 0.922364 | 1.055488 | 0.001779 | 0.001794 | 0.001933 |
| as_map.test | RUN | 0.321879 | 0.898708 | 1.107279 | 0.001592 | 0.001633 | 0.001619 |
| as_map_assoc.test | RUN | 0.319355 | 0.829847 | 1.049300 | 0.001485 | 0.001992 | 0.001649 |
| as_set.test | RUN | 0.361881 | 1.045561 | 1.293070 | 0.002004 | 0.001863 | 0.001537 |
| as_vector.test | RUN | 0.325822 | 0.912763 | 1.139642 | 0.001686 | 0.001676 | 0.001740 |
| boost_tuple.test | RUN | 0.322572 | 0.931806 | 1.068035 | 0.001498 | 0.001441 | 0.001460 |
| boost_tuple_iterator.test | RUN | 0.229909 | 0.545505 | 0.681108 | 0.001601 | 0.001551 | 0.001777 |
| cons.test | RUN | 0.464459 | 1.052300 | 1.304745 | 0.001852 | 0.001834 | 0.001888 |
| convert_boost_tuple.test | RUN | 0.297879 | 0.782287 | 0.862545 | 0.001788 | 0.001572 | 0.001846 |
| convert_deque.test | RUN | 0.306442 | 0.783240 | 0.852072 | 0.001499 | 0.001215 | 0.001668 |
| convert_list.test | RUN | 0.306566 | 0.788704 | 0.861947 | 0.001417 | 0.001607 | 0.001666 |
| convert_std_pair.test | RUN | 0.273487 | 0.792911 | 0.843701 | 0.001792 | 0.001578 | 0.001500 |
| convert_std_tuple.test | RUN | 0.256834 | 0.772334 | 0.843694 | 0.001568 | 0.001692 | 0.001370 |
| convert_vector.test | RUN | 0.256902 | 0.764708 | 0.861129 | 0.001412 | 0.001934 | 0.001659 |
| filter_view.test | RUN | 0.316020 | 0.839314 | 1.024885 | 0.001530 | 0.001840 | 0.001710 |
| hash.test | RUN | 0.319067 | 0.735294 | 0.965260 | 0.001504 | 0.001787 | 0.001823 |
| io.test | RUN | 0.337250 | 1.021671 | 1.160544 | 0.001713 | 0.002019 | 0.001872 |
| iterator_range.test | RUN | 0.302082 | 0.772804 | 0.990307 | 0.001596 | 0.001737 | 0.001878 |
| joint_view.test | RUN | 0.415355 | 1.309496 | 1.428427 | 0.001870 | 0.001786 | 0.001687 |
| list_comparison.test | RUN | 0.232087 | 0.566558 | 0.699906 | 0.001552 | 0.001620 | 0.001529 |
| list_construction.test | RUN | 0.228126 | 0.507710 | 0.692194 | 0.001317 | 0.002017 | 0.001807 |
| list_copy.test | RUN | 0.336874 | 0.960647 | 1.190331 | 0.001707 | 0.001970 | 0.001824 |
| list_iterator.test | RUN | 0.226667 | 0.541304 | 0.671425 | 0.001660 | 0.001678 | 0.002121 |
| list_hash.test | RUN | 0.312533 | 1.033079 | 1.021879 | 0.001922 | 0.001910 | 0.001764 |
| list_make.test | RUN | 0.240660 | 0.589752 | 0.815885 | 0.001826 | 0.001766 | 0.001721 |
| list_misc.test | RUN | 0.308493 | 0.661812 | 0.942235 | 0.001576 | 0.001892 | 0.001399 |
| list_mutate.test | RUN | 0.214899 | 0.463578 | 0.641440 | 0.001838 | 0.001893 | 0.001721 |
| list_nest.test | RUN | 0.437859 | 1.319450 | 1.639367 | 0.001586 | 0.001609 | 0.001570 |
| list_tie.test | RUN | 0.238823 | 0.556447 | 0.705848 | 0.001624 | 0.001752 | 0.001822 |
| list_value_at.test | RUN | 0.221030 | 0.494940 | 0.656604 | 0.001357 | 0.001826 | 0.001771 |
| deque_comparison.test | RUN | 0.227753 | 0.566850 | 0.719212 | 0.001525 | 0.001859 | 0.001740 |
| deque_construction.test | RUN | 0.234860 | 0.529358 | 0.716571 | 0.001712 | 0.001983 | 0.001718 |
| deque_copy.test | RUN | 0.329741 | 0.905867 | 1.111462 | 0.001624 | 0.001695 | 0.001816 |
| deque_iterator.test | RUN | 0.226750 | 0.573681 | 0.698719 | 0.001691 | 0.001643 | 0.002004 |
| deque_hash.test | RUN | 0.313453 | 1.022098 | 0.985982 | 0.001642 | 0.002094 | 0.001822 |
| deque_is_constructible.test | COMPILE | 0.121772 | 0.240070 | 0.324551 | — | — | — |
| deque_make.test | RUN | 0.243458 | 0.586359 | 0.721802 | 0.001832 | 0.001681 | 0.001860 |
| deque_misc.test | RUN | 0.306259 | 0.720290 | 0.872138 | 0.001459 | 0.001794 | 0.001768 |
| deque_move.test | RUN | 0.231972 | 0.583644 | 0.702223 | 0.001383 | 0.001680 | 0.001948 |
| deque_mutate.test | RUN | 0.209996 | 0.483854 | 0.607514 | 0.001678 | 0.001868 | 0.001887 |
| deque_nest.test | RUN | 0.652375 | 2.140714 | 2.711019 | 0.002012 | 0.001870 | 0.001686 |
| deque_tie.test | RUN | 0.215219 | 0.505341 | 0.653176 | 0.001820 | 0.001755 | 0.001621 |
| deque_value_at.test | RUN | 0.211945 | 0.505462 | 0.640796 | 0.001530 | 0.001883 | 0.001847 |
| front_extended_deque.test | RUN | 0.328025 | 0.717936 | 1.000761 | 0.001886 | 0.001763 | 0.001795 |
| back_extended_deque.test | RUN | 0.310253 | 0.729563 | 0.950856 | 0.001584 | 0.001766 | 0.001726 |
| make_list.test | RUN | 0.259427 | 0.605901 | 0.758820 | 0.001688 | 0.001684 | 0.001766 |
| make_vector.test | RUN | 0.241121 | 0.619462 | 0.795183 | 0.001798 | 0.001749 | 0.001748 |
| map.test | RUN | 0.311787 | 0.891572 | 1.042599 | 0.002372 | 0.001854 | 0.001891 |
| map_comparison.test | RUN | 0.248511 | 0.578139 | 0.749156 | 0.001732 | 0.001937 | 0.001764 |
| map_construction.test | RUN | 0.239249 | 0.565531 | 0.721022 | 0.001714 | 0.001716 | 0.001796 |
| map_copy.test | RUN | 0.288034 | 0.682378 | 0.895129 | 0.001895 | 0.001906 | 0.001755 |
| map_misc.test | RUN | 0.312728 | 0.694320 | 0.869487 | 0.001596 | 0.001744 | 0.001848 |
| map_move.test | RUN | 0.243022 | 0.572053 | 0.701002 | 0.001836 | 0.001761 | 0.001602 |
| map_mutate.test | RUN | 0.220339 | 0.483536 | 0.642727 | 0.002466 | 0.001569 | 0.001858 |
| map_tie.test | RUN | 0.222643 | 0.468273 | 0.674112 | 0.001862 | 0.001738 | 0.001824 |
| nil.test | RUN | 0.150826 | 0.327873 | 0.424012 | 0.001781 | 0.002054 | 0.001863 |
| nview.test | RUN | 0.298732 | 0.685942 | 0.920039 | 0.001654 | 0.001916 | 0.001890 |
| reverse_view.test | RUN | 0.388155 | 1.133244 | 1.378675 | 0.001739 | 0.001801 | 0.001934 |
| segmented_iterator_range.test | RUN | 0.470559 | 1.368566 | 1.694131 | 0.002133 | 0.001876 | 0.001747 |
| set.test | RUN | 0.291986 | 0.777764 | 0.938694 | 0.001656 | 0.001808 | 0.001605 |
| single_view.test | RUN | 0.264768 | 0.726492 | 0.855982 | 0.001861 | 0.001706 | 0.001808 |
| std_pair.test | RUN | 0.345533 | 0.873471 | 1.073218 | 0.001814 | 0.001790 | 0.001822 |
| boost_array.test | RUN | 0.281070 | 0.617734 | 0.821289 | 0.001844 | 0.001817 | 0.001837 |
| array.test | RUN | 0.280231 | 0.626536 | 0.814300 | 0.001687 | 0.001758 | 0.001712 |
| std_array.test | RUN | 0.279382 | 0.632479 | 0.820890 | 0.001808 | 0.001607 | 0.001712 |
| tuple_comparison.test | RUN | 0.257670 | 0.632163 | 0.757108 | 0.001892 | 0.002163 | 0.001883 |
| tuple_construction.test | RUN | 0.256122 | 0.658116 | 0.766194 | 0.002028 | 0.002160 | 0.001779 |
| tuple_conversion.test | RUN | 0.568101 | 1.895718 | 2.475813 | 0.001558 | 0.001631 | 0.001486 |
| tuple_copy.test | RUN | 0.373851 | 1.149291 | 1.322638 | 0.001651 | 0.001668 | 0.001445 |
| tuple_element.test | RUN | 0.234150 | 0.552820 | 0.672055 | 0.001982 | 0.001826 | 0.001542 |
| tuple_make.test | RUN | 0.255904 | 0.621380 | 0.741317 | 0.001862 | 0.001821 | 0.001738 |
| tuple_misc.test | RUN | 0.320297 | 0.734965 | 1.006980 | 0.001654 | 0.001752 | 0.001239 |
| tuple_mutate.test | RUN | 0.225653 | 0.507496 | 0.663799 | 0.001740 | 0.001527 | 0.001834 |
| tuple_nest.test | RUN | 0.909266 | 3.697677 | 4.650803 | 0.001827 | 0.001708 | 0.001832 |
| tuple_hash.test | RUN | 0.330103 | 1.011154 | 1.044127 | 0.001892 | 0.001850 | 0.001622 |
| tuple_tie.test | RUN | 0.239923 | 0.532609 | 0.733072 | 0.001581 | 0.001851 | 0.001634 |
| tuple_traits__maybe_variadic.test | RUN | 0.259165 | 0.674982 | 0.776477 | 0.001533 | 0.001676 | 0.001812 |
| tuple_traits__no_variadic.test | RUN | 0.299160 | 0.654769 | 0.846899 | 0.001904 | 0.002068 | 0.001824 |
| transform_view.test | RUN | 0.381364 | 1.161319 | 1.309547 | 0.001860 | 0.002060 | 0.001628 |
| identity_view.test | RUN | 0.360226 | 1.058565 | 1.239214 | 0.001695 | 0.001586 | 0.001630 |
| vector_comparison.test | RUN | 0.233021 | 0.563510 | 0.767011 | 0.001533 | 0.001635 | 0.001777 |
| vector_construction.test | RUN | 0.235721 | 0.563303 | 0.705440 | 0.001759 | 0.001936 | 0.001915 |
| vector_conversion.test | RUN | 0.637431 | 2.367272 | 3.163368 | 0.001746 | 0.001808 | 0.001572 |
| vector_copy.test | RUN | 0.356020 | 1.048331 | 1.284661 | 0.001468 | 0.001849 | 0.001830 |
| vector_iterator.test | RUN | 0.232777 | 0.556557 | 0.707265 | 0.001400 | 0.001686 | 0.001738 |
| vector_make.test | RUN | 0.242498 | 0.587566 | 0.780251 | 0.001569 | 0.001805 | 0.001753 |
| vector_misc.test | RUN | 0.295508 | 0.748809 | 0.918176 | 0.001538 | 0.001838 | 0.001581 |
| vector_move.test | RUN | 0.231313 | 0.571422 | 0.724216 | 0.001787 | 0.001563 | 0.001878 |
| vector_mutate.test | RUN | 0.205963 | 0.476086 | 0.661232 | 0.001515 | 0.001229 | 0.001771 |
| vector_n.test | RUN | 0.258511 | 0.607662 | 0.807113 | 0.001720 | 0.002022 | 0.001775 |
| vector_nest.test | RUN | 0.910407 | 3.879215 | 4.571246 | 0.001845 | 0.001659 | 0.001630 |
| vector_hash.test | RUN | 0.297032 | 0.978333 | 0.961127 | 0.001631 | 0.001904 | 0.001749 |
| vector_tie.test | RUN | 0.219266 | 0.523649 | 0.697965 | 0.001961 | 0.001873 | 0.001738 |
| vector_traits__maybe_variadic.test | RUN | 0.245308 | 0.622295 | 0.655180 | 0.001772 | 0.001904 | 0.001809 |
| vector_traits__no_variadic.test | RUN | 0.261467 | 0.664273 | 0.778619 | 0.001491 | 0.001732 | 0.001411 |
| vector_value_at.test | RUN | 0.217755 | 0.490048 | 0.626228 | 0.001747 | 0.001630 | 0.001703 |
| zip_view.test | RUN | 0.462158 | 1.199134 | 1.606632 | 0.001748 | 0.001534 | 0.001648 |
| zip_view2.test | RUN | 0.370825 | 0.901182 | 1.242451 | 0.001860 | 0.001828 | 0.001612 |
| zip_view_ignore.test | RUN | 0.339473 | 0.824475 | 1.106307 | 0.001621 | 0.001755 | 0.001767 |
| repetitive_view.test | RUN | 0.587518 | 1.571862 | 2.088635 | 0.001881 | 0.001916 | 0.001600 |
| deduce_sequence.test | RUN | 0.270995 | 0.660005 | 0.882556 | 0.002286 | 0.001785 | 0.001431 |
| adapt_adt_named.test | RUN | 0.348605 | 0.907036 | 1.048966 | 0.001540 | 0.001802 | 0.001744 |
| adapt_adt_named_empty.test | RUN | 0.313344 | 0.758630 | 0.877167 | 0.001761 | 0.001904 | 0.001930 |
| adapt_adt.test | RUN | 0.395641 | 1.144912 | 1.269494 | 0.001581 | 0.001920 | 0.001778 |
| adapt_adt_empty.test | RUN | 0.314607 | 0.724443 | 0.867704 | 0.001705 | 0.001920 | 0.001833 |
| adapt_assoc_adt_named.test | RUN | 0.398308 | 0.963713 | 1.144620 | 0.001616 | 0.001914 | 0.001689 |
| adapt_assoc_adt_named_empty.test | RUN | 0.315178 | 0.741755 | 0.851004 | 0.001828 | 0.001873 | 0.001393 |
| adapt_assoc_adt.test | RUN | 0.371535 | 0.965907 | 1.197290 | 0.001568 | 0.002444 | 0.001732 |
| adapt_assoc_adt_empty.test | RUN | 0.279810 | 0.757171 | 0.869940 | 0.001796 | 0.001937 | 0.001846 |
| adapt_assoc_struct_named.test | RUN | 0.369125 | 0.938130 | 1.146618 | 0.001578 | 0.001892 | 0.001873 |
| adapt_assoc_struct_named_empty.test | RUN | 0.287163 | 0.725251 | 0.854768 | 0.001797 | 0.001926 | 0.001434 |
| adapt_assoc_struct.test | RUN | 0.331525 | 0.891467 | 1.044195 | 0.001470 | 0.001823 | 0.001762 |
| adapt_assoc_struct_empty.test | RUN | 0.283713 | 0.735796 | 0.856935 | 0.001486 | 0.001850 | 0.001604 |
| adapt_assoc_tpl_adt.test | RUN | 0.377212 | 1.033313 | 1.224936 | 0.001759 | 0.001908 | 0.001589 |
| adapt_assoc_tpl_adt_empty.test | RUN | 0.280383 | 0.740361 | 0.864592 | 0.001601 | 0.001757 | 0.001777 |
| adapt_assoc_tpl_struct.test | RUN | 0.314498 | 0.848378 | 1.010864 | 0.001725 | 0.001640 | 0.001842 |
| adapt_assoc_tpl_struct_empty.test | RUN | 0.276541 | 0.740996 | 0.879825 | 0.001748 | 0.001619 | 0.001477 |
| adapt_struct_named.test | RUN | 0.315391 | 0.922474 | 1.064684 | 0.001873 | 0.001450 | 0.001570 |
| adapt_struct_named_empty.test | RUN | 0.284718 | 0.759959 | 0.915680 | 0.001559 | 0.001753 | 0.001524 |
| adapt_struct.test | RUN | 0.345867 | 1.013702 | 1.215033 | 0.001554 | 0.001837 | 0.001628 |
| adapt_struct_empty.test | RUN | 0.279450 | 0.738189 | 0.922385 | 0.001686 | 0.001874 | 0.001448 |
| adapt_tpl_adt.test | RUN | 0.353025 | 1.152891 | 1.210910 | 0.001634 | 0.001765 | 0.001702 |
| adapt_tpl_adt_empty.test | RUN | 0.290692 | 0.729237 | 0.888619 | 0.001784 | 0.001944 | 0.001918 |
| adapt_tpl_struct.test | RUN | 0.290591 | 0.791294 | 0.976131 | 0.001618 | 0.001733 | 0.001658 |
| adapt_tpl_struct_empty.test | RUN | 0.282374 | 0.722784 | 0.909252 | 0.001589 | 0.001688 | 0.001603 |
| adt_attribute_proxy.test | RUN | 0.323480 | 0.710939 | 0.928141 | 0.001817 | 0.001985 | 0.001666 |
| define_struct.test | RUN | 0.375233 | 0.996898 | 1.232845 | 0.001654 | 0.001817 | 0.001833 |
| define_struct_empty.test | RUN | 0.327187 | 0.827213 | 1.021690 | 0.001595 | 0.001847 | 0.001810 |
| define_struct_move.test | RUN | 0.216834 | 0.462612 | 0.611029 | 0.001574 | 0.001795 | 0.001754 |
| define_struct_inline.test | RUN | 0.397062 | 1.207139 | 1.376266 | 0.001550 | 0.001870 | 0.001897 |
| define_struct_inline_empty.test | RUN | 0.328630 | 0.800744 | 1.040853 | 0.001787 | 0.001790 | 0.001650 |
| define_struct_inline_move.test | RUN | 0.218118 | 0.488366 | 0.634102 | 0.001532 | 0.001630 | 0.001782 |
| define_assoc_struct.test | RUN | 0.363202 | 0.959620 | 1.178555 | 0.001725 | 0.001995 | 0.001818 |
| define_assoc_struct_empty.test | RUN | 0.333502 | 0.820703 | 1.008150 | 0.003218 | 0.001936 | 0.001806 |
| define_assoc_struct_move.test | RUN | 0.208928 | 0.462675 | 0.612095 | 0.001910 | 0.001904 | 0.001849 |
| define_tpl_struct.test | RUN | 0.373789 | 0.962351 | 1.165603 | 0.002024 | 0.001874 | 0.001793 |
| define_tpl_struct_empty.test | RUN | 0.360572 | 0.823397 | 0.993067 | 0.001711 | 0.001885 | 0.001734 |
| define_tpl_struct_move.test | RUN | 0.232629 | 0.490762 | 0.619907 | 0.001405 | 0.001616 | 0.001622 |
| define_tpl_struct_inline.test | RUN | 0.382335 | 1.124892 | 1.286595 | 0.001656 | 0.001734 | 0.001693 |
| define_tpl_struct_inline_empty.test | RUN | 0.327716 | 0.822347 | 1.025153 | 0.001480 | 0.001600 | 0.001874 |
| define_tpl_struct_inline_move.test | RUN | 0.211291 | 0.476089 | 0.622933 | 0.001703 | 0.001986 | 0.001546 |
| define_assoc_tpl_struct.test | RUN | 0.364082 | 0.971726 | 1.175971 | 0.001814 | 0.001866 | 0.001849 |
| define_assoc_tpl_struct_empty.test | RUN | 0.326308 | 0.827023 | 0.986353 | 0.001496 | 0.001523 | 0.001533 |
| define_assoc_tpl_struct_move.test | RUN | 0.210910 | 0.463418 | 0.620970 | 0.001710 | 0.001862 | 0.001908 |
| std_tuple.test | RUN | 0.225711 | 0.547764 | 0.668200 | 0.001600 | 0.001868 | 0.001544 |
| std_tuple_iterator.test | RUN | 0.223204 | 0.553977 | 0.716825 | 0.001712 | 0.001551 | 0.001834 |
| ref_vector.test | RUN | 0.253065 | 0.573575 | 0.732842 | 0.001710 | 0.002058 | 0.001778 |
| flatten_view.test | RUN | 0.259795 | 0.717186 | 0.888621 | 0.001632 | 0.001592 | 0.001477 |
| github-159.test | COMPILE | 0.120919 | 0.252781 | 0.340876 | — | — | — |
| github-176.test | RUN | 0.244323 | 0.625720 | 0.724574 | 0.001727 | 0.001889 | 0.001894 |
| size.test | COMPILE | 0.212823 | 0.446165 | 0.585470 | — | — | — |
| fused.test | RUN | 0.296715 | 0.645820 | 0.863265 | 0.001629 | 0.001806 | 0.001681 |
| fused_function_object.test | RUN | 0.215708 | 0.488521 | 0.668989 | 0.001547 | 0.001662 | 0.001952 |
| fused_procedure.test | RUN | 0.284482 | 0.647524 | 0.843953 | 0.001763 | 0.001663 | 0.001918 |
| unfused.test | RUN | 0.223008 | 0.521195 | 0.663088 | 0.001724 | 0.001871 | 0.001590 |
| unfused_typed.test | RUN | 0.228748 | 0.555841 | 0.739712 | 0.001643 | 0.001704 | 0.001801 |
| make_fused.test | RUN | 0.290511 | 0.670949 | 0.923777 | 0.001808 | 0.001886 | 0.001468 |
| make_fused_function_object.test | RUN | 0.221085 | 0.485815 | 0.661972 | 0.001601 | 0.001686 | 0.001550 |
| make_fused_procedure.test | RUN | 0.288581 | 0.642708 | 0.823181 | 0.001710 | 0.001764 | 0.001829 |
| make_unfused.test | RUN | 0.219049 | 0.502992 | 0.695650 | 0.001888 | 0.001919 | 0.001978 |
| invoke.test | RUN | 0.575819 | 1.843130 | 2.373534 | 0.001338 | 0.001638 | 0.001757 |
| invoke_function_object.test | RUN | 0.264628 | 0.661815 | 0.843302 | 0.001776 | 0.001930 | 0.001705 |
| invoke_procedure.test | RUN | 0.455269 | 1.387190 | 1.749066 | 0.001498 | 0.001698 | 0.001751 |
| swap.test | RUN | 0.332635 | 0.809901 | 1.038195 | 0.001330 | 0.001652 | 0.001895 |
| is_sequence.test | COMPILE | 0.057444 | 0.124070 | 0.143231 | — | — | — |
| is_view.test | COMPILE | 0.058396 | 0.120980 | 0.143225 | — | — | — |
| pair_deque.test | COMPILE | 0.124869 | 0.257980 | 0.322684 | — | — | — |
| pair_list.test | COMPILE | 0.120025 | 0.265213 | 0.356367 | — | — | — |
| pair_map.test | COMPILE | 0.124396 | 0.277288 | 0.319990 | — | — | — |
| pair_set.test | COMPILE | 0.129439 | 0.267468 | 0.337738 | — | — | — |
| pair_vector.test | COMPILE | 0.124244 | 0.267804 | 0.313591 | — | — | — |
| pair_nest.test | COMPILE | 0.051670 | 0.114722 | 0.140455 | — | — | — |
| index_sequence.test | COMPILE | 0.044694 | 0.076229 | 0.093494 | — | — | — |
| and.test | COMPILE | 0.035242 | 0.059937 | 0.069328 | — | — | — |
| tag_of.test | COMPILE | 0.057981 | 0.116654 | 0.128903 | — | — | — |
| unused.test | COMPILE | 0.031383 | 0.076630 | 0.082163 | — | — | — |
| tag_of_fallback.test | COMPILE | 0.046320 | 0.110082 | 0.129228 | — | — | — |
| make_pair_r-value.test | COMPILE | 0.047660 | 0.076951 | 0.110888 | — | — | — |
| quick.test | RUN | 0.164211 | 0.379768 | 0.409564 | 0.000493 | 0.001316 | 0.001285 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.002× | 1.008×, 0.996× | 0.988× |
| clang | 1.007× | 1.013×, 1.001× | 1.010× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737952).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adapt_adt.test | 10 | 0.001043s | 0.001069s | 0.001058s | byte equality observed |
| adapt_adt_empty.test | 10 | 0.001044s | 0.001016s | 0.001064s | byte equality observed |
| adapt_adt_named.test | 10 | 0.001042s | 0.001049s | 0.001056s | byte equality observed |
| adapt_adt_named_empty.test | 10 | 0.001046s | 0.001059s | 0.001036s | byte equality observed |
| adapt_assoc_adt.test | 10 | 0.001042s | 0.001021s | 0.001037s | byte equality observed |
| adapt_assoc_adt_empty.test | 10 | 0.001023s | 0.001055s | 0.001050s | byte equality observed |
| adapt_assoc_adt_named.test | 10 | 0.001018s | 0.001044s | 0.001063s | byte equality observed |
| adapt_assoc_adt_named_empty.test | 10 | 0.001013s | 0.001047s | 0.001009s | byte equality observed |
| adapt_assoc_struct.test | 10 | 0.001042s | 0.001050s | 0.001038s | byte equality observed |
| adapt_assoc_struct_empty.test | 10 | 0.001030s | 0.001045s | 0.001038s | byte equality observed |
| adapt_assoc_struct_named.test | 10 | 0.001041s | 0.001030s | 0.001058s | byte equality observed |
| adapt_assoc_struct_named_empty.test | 10 | 0.001024s | 0.001036s | 0.001040s | byte equality observed |
| adapt_assoc_tpl_adt.test | 10 | 0.001020s | 0.001036s | 0.001048s | byte equality observed |
| adapt_assoc_tpl_adt_empty.test | 10 | 0.001028s | 0.001043s | 0.001013s | byte equality observed |
| adapt_assoc_tpl_struct.test | 10 | 0.001060s | 0.001064s | 0.001055s | byte equality observed |
| adapt_assoc_tpl_struct_empty.test | 10 | 0.001040s | 0.001021s | 0.001029s | byte equality observed |
| adapt_struct.test | 10 | 0.001042s | 0.001053s | 0.001076s | byte equality observed |
| adapt_struct_empty.test | 10 | 0.001045s | 0.001036s | 0.001032s | byte equality observed |
| adapt_struct_named.test | 10 | 0.001047s | 0.001060s | 0.001048s | byte equality observed |
| adapt_struct_named_empty.test | 10 | 0.001040s | 0.001058s | 0.001078s | byte equality observed |
| adapt_tpl_adt.test | 10 | 0.001056s | 0.001057s | 0.001047s | byte equality observed |
| adapt_tpl_adt_empty.test | 10 | 0.001040s | 0.001032s | 0.001072s | byte equality observed |
| adapt_tpl_struct.test | 10 | 0.001029s | 0.001080s | 0.001069s | byte equality observed |
| adapt_tpl_struct_empty.test | 10 | 0.001022s | 0.001042s | 0.001050s | byte equality observed |
| adt_attribute_proxy.test | 10 | 0.001026s | 0.001018s | 0.001041s | byte equality observed |
| all.test | 10 | 0.000995s | 0.001004s | 0.001031s | byte equality observed |
| any.test | 10 | 0.001004s | 0.001027s | 0.001013s | byte equality observed |
| array.test | 10 | 0.001004s | 0.001000s | 0.001022s | byte equality observed |
| as_deque.test | 10 | 0.001002s | 0.001000s | 0.001032s | byte equality observed |
| as_list.test | 10 | 0.001063s | 0.001071s | 0.001076s | byte equality observed |
| as_map.test | 10 | 0.001080s | 0.001072s | 0.001054s | byte equality observed |
| as_map_assoc.test | 10 | 0.001046s | 0.001084s | 0.001087s | byte equality observed |
| as_set.test | 10 | 0.001053s | 0.001057s | 0.001063s | byte equality observed |
| as_vector.test | 10 | 0.001037s | 0.001036s | 0.001065s | byte equality observed |
| back_extended_deque.test | 10 | 0.001003s | 0.001022s | 0.001003s | byte equality observed |
| boost_array.test | 10 | 0.001018s | 0.001032s | 0.001028s | byte equality observed |
| boost_tuple.test | 10 | 0.001048s | 0.001033s | 0.001035s | byte equality observed |
| boost_tuple_iterator.test | 10 | 0.001042s | 0.000990s | 0.000993s | byte equality observed |
| clear.test | 10 | 0.001034s | 0.001030s | 0.001031s | byte equality observed |
| cons.test | 10 | 0.001070s | 0.001042s | 0.001074s | byte equality observed |
| convert_boost_tuple.test | 10 | 0.001045s | 0.000994s | 0.001011s | byte equality observed |
| convert_deque.test | 10 | 0.001002s | 0.001011s | 0.000995s | byte equality observed |
| convert_list.test | 10 | 0.001012s | 0.001022s | 0.001017s | byte equality observed |
| convert_std_pair.test | 10 | 0.001030s | 0.000994s | 0.001007s | byte equality observed |
| convert_std_tuple.test | 10 | 0.000991s | 0.001004s | 0.001002s | byte equality observed |
| convert_vector.test | 10 | 0.001033s | 0.001013s | 0.000981s | byte equality observed |
| copy.test | 10 | 0.001007s | 0.000995s | 0.001043s | byte equality observed |
| count.test | 10 | 0.001012s | 0.001049s | 0.001047s | byte equality observed |
| count_if.test | 10 | 0.001001s | 0.000978s | 0.001034s | byte equality observed |
| deduce_sequence.test | 10 | 0.000998s | 0.001005s | 0.001033s | byte equality observed |
| define_assoc_struct.test | 10 | 0.001049s | 0.001040s | 0.001068s | byte equality observed |
| define_assoc_struct_empty.test | 10 | 0.001040s | 0.001027s | 0.001022s | byte equality observed |
| define_assoc_struct_move.test | 10 | 0.001005s | 0.001003s | 0.001015s | byte equality observed |
| define_assoc_tpl_struct.test | 10 | 0.001028s | 0.001040s | 0.001066s | byte equality observed |
| define_assoc_tpl_struct_empty.test | 10 | 0.001025s | 0.001014s | 0.001080s | byte equality observed |
| define_assoc_tpl_struct_move.test | 10 | 0.001012s | 0.001027s | 0.001065s | byte equality observed |
| define_struct.test | 10 | 0.001051s | 0.001030s | 0.001051s | byte equality observed |
| define_struct_empty.test | 10 | 0.001052s | 0.001020s | 0.001041s | byte equality observed |
| define_struct_inline.test | 10 | 0.001040s | 0.001041s | 0.001074s | byte equality observed |
| define_struct_inline_empty.test | 10 | 0.001029s | 0.001033s | 0.001048s | byte equality observed |
| define_struct_inline_move.test | 10 | 0.001026s | 0.001004s | 0.001032s | byte equality observed |
| define_struct_move.test | 10 | 0.001018s | 0.001014s | 0.001048s | byte equality observed |
| define_tpl_struct.test | 10 | 0.001061s | 0.001053s | 0.001053s | byte equality observed |
| define_tpl_struct_empty.test | 10 | 0.001027s | 0.001045s | 0.001047s | byte equality observed |
| define_tpl_struct_inline.test | 10 | 0.001041s | 0.001029s | 0.001030s | byte equality observed |
| define_tpl_struct_inline_empty.test | 10 | 0.001030s | 0.001039s | 0.001044s | byte equality observed |
| define_tpl_struct_inline_move.test | 10 | 0.001029s | 0.001013s | 0.001018s | byte equality observed |
| define_tpl_struct_move.test | 10 | 0.001040s | 0.000998s | 0.001000s | byte equality observed |
| deque_comparison.test | 10 | 0.001020s | 0.001010s | 0.001004s | byte equality observed |
| deque_construction.test | 10 | 0.001038s | 0.000989s | 0.001020s | byte equality observed |
| deque_copy.test | 10 | 0.001034s | 0.001021s | 0.001049s | byte equality observed |
| deque_hash.test | 10 | 0.001044s | 0.001004s | 0.001018s | byte equality observed |
| deque_iterator.test | 10 | 0.001009s | 0.000999s | 0.000996s | byte equality observed |
| deque_make.test | 10 | 0.001027s | 0.001037s | 0.001036s | byte equality observed |
| deque_misc.test | 10 | 0.001054s | 0.000990s | 0.001056s | byte equality observed |
| deque_move.test | 10 | 0.001041s | 0.001037s | 0.001054s | byte equality observed |
| deque_mutate.test | 10 | 0.001007s | 0.001021s | 0.000994s | byte equality observed |
| deque_nest.test | 10 | 0.001026s | 0.000962s | 0.001025s | byte equality observed |
| deque_tie.test | 10 | 0.001009s | 0.001005s | 0.001027s | byte equality observed |
| deque_value_at.test | 10 | 0.001004s | 0.001006s | 0.001041s | byte equality observed |
| erase.test | 10 | 0.001039s | 0.001026s | 0.001032s | byte equality observed |
| erase_key.test | 9 | 0.001054s | 0.001062s | 0.001037s | byte equality observed |
| filter.test | 10 | 0.001041s | 0.001049s | 0.001030s | byte equality observed |
| filter_if.test | 10 | 0.001054s | 0.001042s | 0.001037s | byte equality observed |
| filter_view.test | 10 | 0.001041s | 0.001035s | 0.001060s | byte equality observed |
| find.test | 10 | 0.001053s | 0.001043s | 0.001030s | byte equality observed |
| find_if.test | 10 | 0.001042s | 0.001021s | 0.001047s | byte equality observed |
| flatten.test | 10 | 0.001031s | 0.001011s | 0.001059s | byte equality observed |
| flatten_view.test | 10 | 0.001027s | 0.001020s | 0.001036s | byte equality observed |
| fold.test | 10 | 0.001029s | 0.001058s | 0.001056s | byte equality observed |
| for_each.test | 10 | 0.001037s | 0.001050s | 0.001044s | byte equality observed |
| front_extended_deque.test | 10 | 0.001007s | 0.001008s | 0.001012s | byte equality observed |
| fused.test | 10 | 0.001025s | 0.001012s | 0.000990s | byte equality observed |
| fused_function_object.test | 10 | 0.001014s | 0.001008s | 0.001047s | byte equality observed |
| fused_procedure.test | 10 | 0.001007s | 0.000993s | 0.001017s | byte equality observed |
| github-176.test | 10 | 0.001004s | 0.001034s | 0.001014s | byte equality observed |
| hash.test | 10 | 0.001027s | 0.001016s | 0.001018s | byte equality observed |
| identity_view.test | 10 | 0.001050s | 0.001057s | 0.001071s | byte equality observed |
| insert.test | 10 | 0.001068s | 0.001043s | 0.001095s | byte equality observed |
| insert_range.test | 10 | 0.001069s | 0.001065s | 0.001057s | byte equality observed |
| invoke.test | 10 | 0.001010s | 0.001004s | 0.001004s | byte equality observed |
| invoke_function_object.test | 10 | 0.001011s | 0.001022s | 0.000996s | byte equality observed |
| invoke_procedure.test | 10 | 0.001002s | 0.001006s | 0.001007s | byte equality observed |
| io.test | 10 | 0.001173s | 0.001158s | 0.001183s | byte equality observed |
| iter_fold.test | 10 | 0.001038s | 0.001061s | 0.001032s | byte equality observed |
| iterator_range.test | 10 | 0.001056s | 0.001086s | 0.001047s | byte equality observed |
| join.test | 10 | 0.001021s | 0.001027s | 0.001016s | byte equality observed |
| joint_view.test | 10 | 0.001079s | 0.001054s | 0.001045s | byte equality observed |
| list_comparison.test | 10 | 0.001031s | 0.001024s | 0.001019s | byte equality observed |
| list_construction.test | 10 | 0.001047s | 0.001042s | 0.001028s | byte equality observed |
| list_copy.test | 10 | 0.001025s | 0.001004s | 0.001019s | byte equality observed |
| list_hash.test | 10 | 0.001021s | 0.001029s | 0.001036s | byte equality observed |
| list_iterator.test | 10 | 0.001009s | 0.001002s | 0.000989s | byte equality observed |
| list_make.test | 10 | 0.001025s | 0.001028s | 0.001006s | byte equality observed |
| list_misc.test | 10 | 0.001010s | 0.000992s | 0.001023s | byte equality observed |
| list_mutate.test | 10 | 0.001034s | 0.001004s | 0.000994s | byte equality observed |
| list_nest.test | 10 | 0.000992s | 0.001012s | 0.001030s | byte equality observed |
| list_tie.test | 10 | 0.001021s | 0.001020s | 0.001029s | byte equality observed |
| list_value_at.test | 10 | 0.001010s | 0.001002s | 0.001039s | byte equality observed |
| make_fused.test | 10 | 0.001009s | 0.001015s | 0.001033s | byte equality observed |
| make_fused_function_object.test | 10 | 0.001004s | 0.001081s | 0.001029s | byte equality observed |
| make_fused_procedure.test | 10 | 0.001038s | 0.001044s | 0.001014s | byte equality observed |
| make_list.test | 10 | 0.001028s | 0.001042s | 0.001023s | byte equality observed |
| make_unfused.test | 10 | 0.001014s | 0.001029s | 0.001018s | byte equality observed |
| make_vector.test | 10 | 0.001034s | 0.001024s | 0.001027s | byte equality observed |
| map.test | 10 | 0.001044s | 0.001048s | 0.001074s | byte equality observed |
| map_comparison.test | 9 | 0.000999s | 0.001012s | 0.001028s | byte equality observed |
| map_construction.test | 10 | 0.001006s | 0.001025s | 0.001055s | byte equality observed |
| map_copy.test | 10 | 0.001026s | 0.001026s | 0.001017s | byte equality observed |
| map_misc.test | 10 | 0.001006s | 0.001022s | 0.001014s | byte equality observed |
| map_move.test | 10 | 0.001025s | 0.001037s | 0.001031s | byte equality observed |
| map_mutate.test | 10 | 0.000994s | 0.001020s | 0.001026s | byte equality observed |
| map_tie.test | 10 | 0.000995s | 0.001011s | 0.001048s | byte equality observed |
| move.test | 10 | 0.001017s | 0.001004s | 0.001016s | byte equality observed |
| nil.test | 10 | 0.001004s | 0.001034s | 0.001012s | byte equality observed |
| none.test | 10 | 0.001024s | 0.001004s | 0.001015s | byte equality observed |
| nview.test | 10 | 0.001017s | 0.001025s | 0.000996s | byte equality observed |
| pop_back.test | 10 | 0.001058s | 0.001034s | 0.001071s | byte equality observed |
| pop_front.test | 10 | 0.001033s | 0.001027s | 0.001051s | byte equality observed |
| push_back.test | 10 | 0.001036s | 0.001039s | 0.001065s | byte equality observed |
| push_front.test | 10 | 0.001055s | 0.001055s | 0.001058s | byte equality observed |
| quick.test | 10 | 0.000468s | 0.000984s | 0.000983s | byte equality observed |
| ref_vector.test | 10 | 0.001030s | 0.000993s | 0.001020s | byte equality observed |
| remove.test | 10 | 0.001033s | 0.001050s | 0.001063s | byte equality observed |
| remove_if.test | 10 | 0.001038s | 0.001065s | 0.001064s | byte equality observed |
| repetitive_view.test | 10 | 0.000985s | 0.000994s | 0.001034s | byte equality observed |
| replace.test | 10 | 0.001040s | 0.001034s | 0.001057s | byte equality observed |
| replace_if.test | 10 | 0.001069s | 0.001082s | 0.001074s | byte equality observed |
| reverse.test | 10 | 0.001041s | 0.001056s | 0.001064s | byte equality observed |
| reverse_fold.test | 10 | 0.001051s | 0.001031s | 0.001070s | byte equality observed |
| reverse_iter_fold.test | 10 | 0.001035s | 0.001050s | 0.001069s | byte equality observed |
| reverse_view.test | 10 | 0.001039s | 0.001054s | 0.001061s | byte equality observed |
| segmented_find.test | 10 | 0.001001s | 0.001028s | 0.001045s | byte equality observed |
| segmented_find_if.test | 10 | 0.001018s | 0.001012s | 0.001064s | byte equality observed |
| segmented_fold.test | 10 | 0.001036s | 0.001030s | 0.001042s | byte equality observed |
| segmented_for_each.test | 10 | 0.001051s | 0.001032s | 0.001061s | byte equality observed |
| segmented_iterator_range.test | 10 | 0.001069s | 0.001057s | 0.001066s | byte equality observed |
| set.test | 10 | 0.001040s | 0.001050s | 0.001035s | byte equality observed |
| single_view.test | 10 | 0.001038s | 0.001040s | 0.001010s | byte equality observed |
| std_array.test | 10 | 0.001001s | 0.001001s | 0.001026s | byte equality observed |
| std_pair.test | 10 | 0.001041s | 0.001031s | 0.001077s | byte equality observed |
| std_tuple.test | 10 | 0.001013s | 0.001006s | 0.001029s | byte equality observed |
| std_tuple_iterator.test | 10 | 0.001026s | 0.001042s | 0.001018s | byte equality observed |
| swap.test | 10 | 0.001011s | 0.001036s | 0.001023s | byte equality observed |
| transform.test | 10 | 0.001039s | 0.001073s | 0.001042s | byte equality observed |
| transform_view.test | 10 | 0.001058s | 0.001067s | 0.001061s | byte equality observed |
| tuple_comparison.test | 10 | 0.001000s | 0.001054s | 0.001004s | byte equality observed |
| tuple_construction.test | 10 | 0.001004s | 0.001037s | 0.001002s | byte equality observed |
| tuple_conversion.test | 10 | 0.001012s | 0.001032s | 0.001019s | byte equality observed |
| tuple_copy.test | 10 | 0.001004s | 0.001033s | 0.001030s | byte equality observed |
| tuple_element.test | 10 | 0.001003s | 0.000999s | 0.001034s | byte equality observed |
| tuple_hash.test | 10 | 0.001005s | 0.001036s | 0.001026s | byte equality observed |
| tuple_make.test | 10 | 0.001013s | 0.001028s | 0.001034s | byte equality observed |
| tuple_misc.test | 10 | 0.001016s | 0.001038s | 0.001014s | byte equality observed |
| tuple_mutate.test | 10 | 0.001018s | 0.000995s | 0.001001s | byte equality observed |
| tuple_nest.test | 10 | 0.001029s | 0.001009s | 0.001009s | byte equality observed |
| tuple_tie.test | 10 | 0.001047s | 0.001030s | 0.001004s | byte equality observed |
| tuple_traits__maybe_variadic.test | 10 | 0.001038s | 0.001013s | 0.001025s | byte equality observed |
| tuple_traits__no_variadic.test | 10 | 0.001016s | 0.000970s | 0.001004s | byte equality observed |
| unfused.test | 10 | 0.001018s | 0.001030s | 0.001010s | byte equality observed |
| unfused_typed.test | 10 | 0.001008s | 0.001010s | 0.001021s | byte equality observed |
| vector_comparison.test | 10 | 0.001001s | 0.001003s | 0.001017s | byte equality observed |
| vector_construction.test | 10 | 0.001004s | 0.001016s | 0.001003s | byte equality observed |
| vector_conversion.test | 10 | 0.001009s | 0.001012s | 0.000980s | byte equality observed |
| vector_copy.test | 10 | 0.001027s | 0.001010s | 0.000996s | byte equality observed |
| vector_hash.test | 10 | 0.001027s | 0.000997s | 0.001021s | byte equality observed |
| vector_iterator.test | 10 | 0.001037s | 0.001027s | 0.001012s | byte equality observed |
| vector_make.test | 10 | 0.001025s | 0.001028s | 0.001014s | byte equality observed |
| vector_misc.test | 10 | 0.001022s | 0.001010s | 0.000996s | byte equality observed |
| vector_move.test | 10 | 0.001047s | 0.001030s | 0.001020s | byte equality observed |
| vector_mutate.test | 10 | 0.001019s | 0.000995s | 0.000993s | byte equality observed |
| vector_n.test | 10 | 0.001038s | 0.001068s | 0.001053s | byte equality observed |
| vector_nest.test | 10 | 0.001036s | 0.001006s | 0.001007s | byte equality observed |
| vector_tie.test | 10 | 0.000995s | 0.001016s | 0.001025s | byte equality observed |
| vector_traits__maybe_variadic.test | 10 | 0.001008s | 0.001013s | 0.001013s | byte equality observed |
| vector_traits__no_variadic.test | 10 | 0.000998s | 0.000997s | 0.001019s | byte equality observed |
| vector_value_at.test | 10 | 0.001024s | 0.001015s | 0.001005s | byte equality observed |
| zip.test | 10 | 0.001010s | 0.001015s | 0.001017s | byte equality observed |
| zip2.test | 10 | 0.001012s | 0.001003s | 0.000996s | byte equality observed |
| zip_ignore.test | 10 | 0.000982s | 0.000998s | 0.000989s | byte equality observed |
| zip_view.test | 10 | 0.000996s | 0.001032s | 0.000995s | byte equality observed |
| zip_view2.test | 10 | 0.001008s | 0.001013s | 0.000995s | byte equality observed |
| zip_view_ignore.test | 10 | 0.000998s | 0.001039s | 0.001026s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737952).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
