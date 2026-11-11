# libs/fusion/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| all.test | RUN | 0.461840 | 0.900153 | 1.105981 | 0.001494 | 0.001564 | 0.001491 |
| any.test | RUN | 0.446302 | 0.848678 | 1.053344 | 0.001744 | 0.001702 | 0.001746 |
| clear.test | RUN | 0.324647 | 0.789820 | 0.866151 | 0.001534 | 0.001640 | 0.001768 |
| copy.test | RUN | 0.278235 | 0.581052 | 0.700538 | 0.001649 | 0.001820 | 0.001545 |
| count.test | RUN | 0.305284 | 0.649878 | 0.780333 | 0.001669 | 0.001773 | 0.001605 |
| count_if.test | RUN | 0.287927 | 0.611870 | 0.721876 | 0.001624 | 0.001776 | 0.001768 |
| erase.test | RUN | 0.407702 | 1.052222 | 1.184948 | 0.001604 | 0.001934 | 0.001572 |
| erase_key.test | RUN | 0.396062 | 1.053928 | 1.111731 | 0.001782 | 0.001706 | 0.001732 |
| filter.test | RUN | 0.359942 | 0.903249 | 0.999618 | 0.001788 | 0.001790 | 0.001875 |
| filter_if.test | RUN | 0.392600 | 1.040558 | 1.199175 | 0.001725 | 0.001734 | 0.001792 |
| find.test | RUN | 0.342114 | 0.723177 | 0.908380 | 0.001627 | 0.001754 | 0.001662 |
| find_if.test | RUN | 0.327819 | 0.656031 | 0.820586 | 0.001672 | 0.001760 | 0.001733 |
| fold.test | RUN | 0.361996 | 0.813430 | 1.012278 | 0.001811 | 0.001925 | 0.001758 |
| for_each.test | RUN | 0.326980 | 0.745073 | 0.880141 | 0.001642 | 0.001905 | 0.001770 |
| insert.test | RUN | 0.479905 | 1.325333 | 1.427412 | 0.001802 | 0.001595 | 0.001689 |
| insert_range.test | RUN | 0.478977 | 1.323717 | 1.421824 | 0.001548 | 0.001533 | 0.001781 |
| iter_fold.test | RUN | 0.547119 | 1.251601 | 1.655711 | 0.001882 | 0.001819 | 0.001780 |
| move.test | RUN | 0.287514 | 0.595760 | 0.699075 | 0.001912 | 0.001550 | 0.001650 |
| none.test | RUN | 0.444797 | 0.849175 | 1.088597 | 0.001613 | 0.001782 | 0.001587 |
| pop_back.test | RUN | 0.439221 | 1.146949 | 1.266785 | 0.001784 | 0.001688 | 0.001762 |
| pop_front.test | RUN | 0.365446 | 0.937223 | 1.020040 | 0.001754 | 0.001774 | 0.001662 |
| push_back.test | RUN | 0.453502 | 1.315216 | 1.425055 | 0.001795 | 0.001890 | 0.001865 |
| push_front.test | RUN | 0.405761 | 1.093796 | 1.187805 | 0.001637 | 0.001825 | 0.001762 |
| remove.test | RUN | 0.439551 | 1.243553 | 1.385424 | 0.001571 | 0.001697 | 0.001473 |
| remove_if.test | RUN | 0.393930 | 1.058117 | 1.185021 | 0.001805 | 0.001738 | 0.001824 |
| replace.test | RUN | 0.385741 | 1.023011 | 1.126172 | 0.001768 | 0.001908 | 0.001602 |
| replace_if.test | RUN | 0.389995 | 1.011119 | 1.110161 | 0.001940 | 0.001898 | 0.001886 |
| reverse_fold.test | RUN | 0.589613 | 1.407094 | 1.848137 | 0.001693 | 0.001612 | 0.001700 |
| reverse_iter_fold.test | RUN | 0.608559 | 1.452305 | 1.932458 | 0.001776 | 0.001842 | 0.001575 |
| reverse.test | RUN | 0.386853 | 1.046723 | 1.126682 | 0.001696 | 0.001742 | 0.001583 |
| segmented_for_each.test | RUN | 0.647400 | 1.920032 | 2.279881 | 0.001499 | 0.001732 | 0.001543 |
| segmented_find.test | RUN | 0.414091 | 1.030571 | 1.285551 | 0.001448 | 0.001596 | 0.001666 |
| segmented_find_if.test | RUN | 0.409881 | 1.029759 | 1.265863 | 0.001832 | 0.001560 | 0.001723 |
| segmented_fold.test | RUN | 0.507982 | 1.352350 | 1.605885 | 0.001844 | 0.001887 | 0.001712 |
| transform.test | RUN | 0.404964 | 1.096431 | 1.180034 | 0.001646 | 0.001729 | 0.001744 |
| join.test | RUN | 0.305012 | 0.669029 | 0.818496 | 0.001768 | 0.001826 | 0.001800 |
| zip.test | RUN | 0.516779 | 1.268024 | 1.545559 | 0.001686 | 0.001824 | 0.001726 |
| zip2.test | RUN | 0.505953 | 1.221220 | 1.484297 | 0.001698 | 0.001651 | 0.001664 |
| zip_ignore.test | RUN | 0.395358 | 0.827515 | 0.996473 | 0.001528 | 0.001460 | 0.001510 |
| flatten.test | RUN | 0.379525 | 0.909834 | 1.027994 | 0.001577 | 0.001718 | 0.001786 |
| ticket-5490.test | COMPILE | 0.202103 | 0.405739 | 0.449013 | — | — | — |
| as_deque.test | RUN | 0.359839 | 0.868557 | 1.098198 | 0.001789 | 0.001844 | 0.001719 |
| as_list.test | RUN | 0.410151 | 1.086005 | 1.190979 | 0.001626 | 0.001838 | 0.001770 |
| as_map.test | RUN | 0.418406 | 1.058525 | 1.195631 | 0.001515 | 0.001606 | 0.001787 |
| as_map_assoc.test | RUN | 0.396627 | 1.000538 | 1.133086 | 0.001575 | 0.001770 | 0.001650 |
| as_set.test | RUN | 0.451483 | 1.239657 | 1.378031 | 0.001686 | 0.001727 | 0.001610 |
| as_vector.test | RUN | 0.427170 | 1.110436 | 1.223355 | 0.001681 | 0.001580 | 0.001705 |
| boost_tuple.test | RUN | 0.407376 | 1.074992 | 1.196012 | 0.001641 | 0.001531 | 0.001580 |
| boost_tuple_iterator.test | RUN | 0.281384 | 0.622379 | 0.727347 | 0.001978 | 0.001585 | 0.001486 |
| cons.test | RUN | 0.514902 | 1.225036 | 1.420297 | 0.001720 | 0.001566 | 0.001758 |
| convert_boost_tuple.test | RUN | 0.339365 | 0.930557 | 0.934355 | 0.001940 | 0.001718 | 0.001437 |
| convert_deque.test | RUN | 0.340482 | 0.928347 | 0.936763 | 0.001342 | 0.001482 | 0.001610 |
| convert_list.test | RUN | 0.336272 | 0.952504 | 0.935150 | 0.001486 | 0.001482 | 0.001635 |
| convert_std_pair.test | RUN | 0.353692 | 0.939439 | 0.954412 | 0.001614 | 0.001570 | 0.001520 |
| convert_std_tuple.test | RUN | 0.335688 | 0.923139 | 0.926159 | 0.001584 | 0.001652 | 0.001555 |
| convert_vector.test | RUN | 0.338390 | 0.927551 | 0.936383 | 0.001355 | 0.001727 | 0.001650 |
| filter_view.test | RUN | 0.386026 | 0.977585 | 1.115815 | 0.001706 | 0.001621 | 0.001712 |
| hash.test | RUN | 0.392395 | 0.883689 | 1.056161 | 0.001611 | 0.001687 | 0.001765 |
| io.test | RUN | 0.419141 | 1.250873 | 1.295633 | 0.001694 | 0.002042 | 0.002074 |
| iterator_range.test | RUN | 0.375614 | 0.955963 | 1.066355 | 0.001694 | 0.001790 | 0.001634 |
| joint_view.test | RUN | 0.498311 | 1.555889 | 1.614237 | 0.001814 | 0.002058 | 0.001675 |
| list_comparison.test | RUN | 0.292761 | 0.657905 | 0.794224 | 0.001755 | 0.001567 | 0.001652 |
| list_construction.test | RUN | 0.286255 | 0.613590 | 0.738345 | 0.001554 | 0.001605 | 0.001205 |
| list_copy.test | RUN | 0.442739 | 1.146268 | 1.302005 | 0.001582 | 0.001602 | 0.001680 |
| list_iterator.test | RUN | 0.292283 | 0.646177 | 0.722909 | 0.001594 | 0.001820 | 0.001638 |
| list_hash.test | RUN | 0.395126 | 1.262442 | 1.066774 | 0.001618 | 0.001767 | 0.001701 |
| list_make.test | RUN | 0.306047 | 0.688938 | 0.808228 | 0.001770 | 0.001642 | 0.001749 |
| list_misc.test | RUN | 0.387864 | 0.823489 | 1.003952 | 0.001582 | 0.001664 | 0.001669 |
| list_mutate.test | RUN | 0.273522 | 0.550482 | 0.660608 | 0.001683 | 0.001731 | 0.001696 |
| list_nest.test | RUN | 0.550695 | 1.541391 | 1.823592 | 0.001602 | 0.001657 | 0.001569 |
| list_tie.test | RUN | 0.291375 | 0.633991 | 0.721488 | 0.001776 | 0.001780 | 0.001728 |
| list_value_at.test | RUN | 0.278704 | 0.569409 | 0.730924 | 0.001656 | 0.001735 | 0.001776 |
| deque_comparison.test | RUN | 0.301434 | 0.656543 | 0.753942 | 0.001585 | 0.001617 | 0.001713 |
| deque_construction.test | RUN | 0.291165 | 0.611518 | 0.756366 | 0.001790 | 0.001926 | 0.001723 |
| deque_copy.test | RUN | 0.407829 | 1.068638 | 1.236627 | 0.001617 | 0.001688 | 0.001826 |
| deque_iterator.test | RUN | 0.286806 | 0.656250 | 0.734099 | 0.001612 | 0.001469 | 0.001796 |
| deque_hash.test | RUN | 0.395923 | 1.236180 | 1.089042 | 0.001512 | 0.001790 | 0.001748 |
| deque_is_constructible.test | COMPILE | 0.162537 | 0.285931 | 0.345938 | — | — | — |
| deque_make.test | RUN | 0.298338 | 0.667280 | 0.780945 | 0.001890 | 0.001648 | 0.001785 |
| deque_misc.test | RUN | 0.375805 | 0.796543 | 0.975225 | 0.001450 | 0.001721 | 0.001668 |
| deque_move.test | RUN | 0.301408 | 0.690489 | 0.769379 | 0.001737 | 0.001674 | 0.001801 |
| deque_mutate.test | RUN | 0.267614 | 0.553301 | 0.676486 | 0.001547 | 0.001754 | 0.001813 |
| deque_nest.test | RUN | 0.811157 | 2.479710 | 2.976925 | 0.001700 | 0.001838 | 0.001776 |
| deque_tie.test | RUN | 0.275521 | 0.598546 | 0.728517 | 0.001788 | 0.001555 | 0.001734 |
| deque_value_at.test | RUN | 0.279769 | 0.561464 | 0.696973 | 0.001645 | 0.001844 | 0.001722 |
| front_extended_deque.test | RUN | 0.386886 | 0.852461 | 1.031406 | 0.001666 | 0.001744 | 0.001622 |
| back_extended_deque.test | RUN | 0.401768 | 0.834345 | 1.038291 | 0.001595 | 0.001699 | 0.001718 |
| make_list.test | RUN | 0.302731 | 0.682209 | 0.815258 | 0.001674 | 0.001744 | 0.001870 |
| make_vector.test | RUN | 0.308911 | 0.689070 | 0.840508 | 0.001539 | 0.001680 | 0.001289 |
| map.test | RUN | 0.384851 | 1.050175 | 1.156749 | 0.001642 | 0.001742 | 0.001490 |
| map_comparison.test | RUN | 0.297322 | 0.664128 | 0.786282 | 0.001617 | 0.001821 | 0.001764 |
| map_construction.test | RUN | 0.293732 | 0.660535 | 0.758305 | 0.001546 | 0.001818 | 0.001675 |
| map_copy.test | RUN | 0.351352 | 0.815718 | 0.963297 | 0.001667 | 0.001773 | 0.001692 |
| map_misc.test | RUN | 0.377347 | 0.818727 | 0.955446 | 0.001538 | 0.001711 | 0.001483 |
| map_move.test | RUN | 0.303842 | 0.697808 | 0.790823 | 0.001295 | 0.001785 | 0.001584 |
| map_mutate.test | RUN | 0.269377 | 0.562021 | 0.671956 | 0.001571 | 0.001623 | 0.001512 |
| map_tie.test | RUN | 0.282252 | 0.552865 | 0.697287 | 0.001510 | 0.001708 | 0.001762 |
| nil.test | RUN | 0.188118 | 0.390136 | 0.475005 | 0.001728 | 0.001636 | 0.001777 |
| nview.test | RUN | 0.373226 | 0.820455 | 0.992106 | 0.001688 | 0.001850 | 0.001747 |
| reverse_view.test | RUN | 0.454690 | 1.327561 | 1.472968 | 0.001792 | 0.001752 | 0.001842 |
| segmented_iterator_range.test | RUN | 0.550325 | 1.573577 | 1.858712 | 0.001780 | 0.001847 | 0.001623 |
| set.test | RUN | 0.363265 | 0.914900 | 1.025213 | 0.001881 | 0.001639 | 0.001785 |
| single_view.test | RUN | 0.338129 | 0.825422 | 0.933831 | 0.001609 | 0.001840 | 0.001673 |
| std_pair.test | RUN | 0.413662 | 1.063739 | 1.216686 | 0.001810 | 0.001710 | 0.001746 |
| boost_array.test | RUN | 0.352281 | 0.733938 | 0.893725 | 0.001674 | 0.001795 | 0.001618 |
| array.test | RUN | 0.352516 | 0.747083 | 0.889542 | 0.001759 | 0.001842 | 0.001739 |
| std_array.test | RUN | 0.352733 | 0.734830 | 0.867233 | 0.001731 | 0.001766 | 0.001613 |
| tuple_comparison.test | RUN | 0.324977 | 0.698835 | 0.826966 | 0.001701 | 0.001794 | 0.001645 |
| tuple_construction.test | RUN | 0.317014 | 0.690920 | 0.832911 | 0.001781 | 0.001817 | 0.001766 |
| tuple_conversion.test | RUN | 0.713908 | 2.120312 | 2.723841 | 0.001520 | 0.001562 | 0.001745 |
| tuple_copy.test | RUN | 0.454301 | 1.227452 | 1.440853 | 0.001741 | 0.001719 | 0.001513 |
| tuple_element.test | RUN | 0.298150 | 0.631905 | 0.770543 | 0.001727 | 0.001307 | 0.001738 |
| tuple_make.test | RUN | 0.321739 | 0.729170 | 0.851926 | 0.001640 | 0.001738 | 0.001662 |
| tuple_misc.test | RUN | 0.404123 | 0.884379 | 1.086973 | 0.001510 | 0.001863 | 0.001552 |
| tuple_mutate.test | RUN | 0.291861 | 0.598303 | 0.716950 | 0.001802 | 0.001491 | 0.001758 |
| tuple_nest.test | RUN | 1.214539 | 4.265903 | 5.030612 | 0.001834 | 0.001728 | 0.001772 |
| tuple_hash.test | RUN | 0.410503 | 1.273845 | 1.067213 | 0.001790 | 0.001794 | 0.001480 |
| tuple_tie.test | RUN | 0.306803 | 0.649619 | 0.782113 | 0.001690 | 0.001654 | 0.001655 |
| tuple_traits__maybe_variadic.test | RUN | 0.321739 | 0.816873 | 0.842578 | 0.001566 | 0.001876 | 0.001682 |
| tuple_traits__no_variadic.test | RUN | 0.364138 | 0.811147 | 0.889960 | 0.001688 | 0.001764 | 0.001738 |
| transform_view.test | RUN | 0.477171 | 1.358327 | 1.461678 | 0.001905 | 0.001723 | 0.001871 |
| identity_view.test | RUN | 0.457443 | 1.241732 | 1.350419 | 0.001600 | 0.001663 | 0.001634 |
| vector_comparison.test | RUN | 0.300398 | 0.675536 | 0.808525 | 0.001583 | 0.001613 | 0.001924 |
| vector_construction.test | RUN | 0.297876 | 0.624935 | 0.757945 | 0.001731 | 0.001812 | 0.001847 |
| vector_conversion.test | RUN | 0.857014 | 2.779391 | 3.452490 | 0.001736 | 0.001748 | 0.001690 |
| vector_copy.test | RUN | 0.454592 | 1.198724 | 1.418664 | 0.001371 | 0.001814 | 0.001771 |
| vector_iterator.test | RUN | 0.287737 | 0.668397 | 0.760025 | 0.001492 | 0.001835 | 0.001675 |
| vector_make.test | RUN | 0.310344 | 0.683507 | 0.792375 | 0.001653 | 0.001646 | 0.001719 |
| vector_misc.test | RUN | 0.398650 | 0.833273 | 1.021458 | 0.001734 | 0.001918 | 0.001538 |
| vector_move.test | RUN | 0.302468 | 0.712385 | 0.768150 | 0.001684 | 0.001606 | 0.001810 |
| vector_mutate.test | RUN | 0.271125 | 0.561763 | 0.667710 | 0.001480 | 0.001791 | 0.001728 |
| vector_n.test | RUN | 0.325709 | 0.728128 | 0.903895 | 0.001918 | 0.001852 | 0.001793 |
| vector_nest.test | RUN | 1.216996 | 4.532770 | 5.136265 | 0.001734 | 0.001672 | 0.001662 |
| vector_hash.test | RUN | 0.387186 | 1.256560 | 1.035628 | 0.001872 | 0.001850 | 0.001800 |
| vector_tie.test | RUN | 0.285161 | 0.622977 | 0.748858 | 0.001912 | 0.001696 | 0.001556 |
| vector_traits__maybe_variadic.test | RUN | 0.302688 | 0.750454 | 0.756513 | 0.001916 | 0.001824 | 0.001752 |
| vector_traits__no_variadic.test | RUN | 0.335040 | 0.761990 | 0.797701 | 0.001672 | 0.001806 | 0.001620 |
| vector_value_at.test | RUN | 0.283211 | 0.584240 | 0.720987 | 0.001531 | 0.001744 | 0.001732 |
| zip_view.test | RUN | 0.546995 | 1.397204 | 1.759885 | 0.001799 | 0.001834 | 0.001685 |
| zip_view2.test | RUN | 0.451258 | 1.077110 | 1.360219 | 0.001742 | 0.001608 | 0.001670 |
| zip_view_ignore.test | RUN | 0.438983 | 0.985674 | 1.210792 | 0.001756 | 0.001762 | 0.001758 |
| repetitive_view.test | RUN | 0.719218 | 1.793492 | 2.380182 | 0.001246 | 0.001798 | 0.001730 |
| deduce_sequence.test | RUN | 0.352579 | 0.770793 | 0.915683 | 0.001648 | 0.001895 | 0.001825 |
| adapt_adt_named.test | RUN | 0.422714 | 1.073671 | 1.154175 | 0.001580 | 0.001762 | 0.001758 |
| adapt_adt_named_empty.test | RUN | 0.362431 | 0.865668 | 0.973491 | 0.001605 | 0.001772 | 0.001788 |
| adapt_adt.test | RUN | 0.463879 | 1.419596 | 1.444994 | 0.001780 | 0.001834 | 0.001755 |
| adapt_adt_empty.test | RUN | 0.360403 | 0.876653 | 0.966080 | 0.001615 | 0.001773 | 0.001798 |
| adapt_assoc_adt_named.test | RUN | 0.465117 | 1.135581 | 1.276704 | 0.001640 | 0.001881 | 0.001866 |
| adapt_assoc_adt_named_empty.test | RUN | 0.364852 | 0.880692 | 0.976405 | 0.001586 | 0.001660 | 0.001825 |
| adapt_assoc_adt.test | RUN | 0.474696 | 1.178962 | 1.366418 | 0.001707 | 0.001878 | 0.001535 |
| adapt_assoc_adt_empty.test | RUN | 0.366643 | 0.863434 | 0.962968 | 0.001700 | 0.001950 | 0.001762 |
| adapt_assoc_struct_named.test | RUN | 0.458423 | 1.116444 | 1.271399 | 0.001516 | 0.001794 | 0.001810 |
| adapt_assoc_struct_named_empty.test | RUN | 0.362202 | 0.871060 | 0.962161 | 0.001729 | 0.001835 | 0.001744 |
| adapt_assoc_struct.test | RUN | 0.418713 | 1.052118 | 1.191363 | 0.001683 | 0.001812 | 0.001715 |
| adapt_assoc_struct_empty.test | RUN | 0.356701 | 0.866670 | 0.971712 | 0.001746 | 0.002034 | 0.001664 |
| adapt_assoc_tpl_adt.test | RUN | 0.485121 | 1.221337 | 1.369721 | 0.001696 | 0.001793 | 0.001721 |
| adapt_assoc_tpl_adt_empty.test | RUN | 0.361699 | 0.872275 | 0.995480 | 0.001582 | 0.001904 | 0.001852 |
| adapt_assoc_tpl_struct.test | RUN | 0.390910 | 1.038880 | 1.133543 | 0.001535 | 0.001835 | 0.001735 |
| adapt_assoc_tpl_struct_empty.test | RUN | 0.356900 | 0.868370 | 0.991442 | 0.001741 | 0.001832 | 0.001606 |
| adapt_struct_named.test | RUN | 0.404547 | 1.067870 | 1.129388 | 0.001703 | 0.001843 | 0.001781 |
| adapt_struct_named_empty.test | RUN | 0.357717 | 0.860504 | 0.966745 | 0.001570 | 0.001694 | 0.001594 |
| adapt_struct.test | RUN | 0.448390 | 1.197566 | 1.307917 | 0.001742 | 0.001846 | 0.001768 |
| adapt_struct_empty.test | RUN | 0.356801 | 0.875996 | 0.961316 | 0.001747 | 0.001848 | 0.001798 |
| adapt_tpl_adt.test | RUN | 0.456406 | 1.386960 | 1.368532 | 0.001552 | 0.001794 | 0.001698 |
| adapt_tpl_adt_empty.test | RUN | 0.369633 | 0.878525 | 0.969604 | 0.001755 | 0.001821 | 0.001782 |
| adapt_tpl_struct.test | RUN | 0.374118 | 0.973595 | 1.061911 | 0.001876 | 0.001544 | 0.001702 |
| adapt_tpl_struct_empty.test | RUN | 0.358846 | 0.885057 | 0.985192 | 0.001660 | 0.001750 | 0.001716 |
| adt_attribute_proxy.test | RUN | 0.402848 | 0.851671 | 1.027956 | 0.001516 | 0.001813 | 0.001646 |
| define_struct.test | RUN | 0.482337 | 1.186116 | 1.386521 | 0.001764 | 0.001822 | 0.001744 |
| define_struct_empty.test | RUN | 0.425042 | 0.971524 | 1.121945 | 0.001842 | 0.001742 | 0.001698 |
| define_struct_move.test | RUN | 0.282750 | 0.539793 | 0.674592 | 0.001665 | 0.001724 | 0.001768 |
| define_struct_inline.test | RUN | 0.525576 | 1.425019 | 1.557949 | 0.001573 | 0.001682 | 0.001735 |
| define_struct_inline_empty.test | RUN | 0.432086 | 0.977272 | 1.124631 | 0.001642 | 0.001755 | 0.001640 |
| define_struct_inline_move.test | RUN | 0.289873 | 0.562100 | 0.689738 | 0.001733 | 0.001675 | 0.001554 |
| define_assoc_struct.test | RUN | 0.472378 | 1.149500 | 1.303389 | 0.001751 | 0.001584 | 0.001844 |
| define_assoc_struct_empty.test | RUN | 0.444120 | 0.989142 | 1.112739 | 0.001737 | 0.001894 | 0.001722 |
| define_assoc_struct_move.test | RUN | 0.283387 | 0.555647 | 0.659235 | 0.001698 | 0.001709 | 0.001626 |
| define_tpl_struct.test | RUN | 0.483600 | 1.165357 | 1.292123 | 0.001826 | 0.001664 | 0.001863 |
| define_tpl_struct_empty.test | RUN | 0.444788 | 0.989642 | 1.125031 | 0.001566 | 0.001747 | 0.001784 |
| define_tpl_struct_move.test | RUN | 0.287469 | 0.563070 | 0.660928 | 0.001747 | 0.001660 | 0.001545 |
| define_tpl_struct_inline.test | RUN | 0.494160 | 1.333405 | 1.414611 | 0.001751 | 0.001897 | 0.001842 |
| define_tpl_struct_inline_empty.test | RUN | 0.421868 | 0.986560 | 1.123288 | 0.001890 | 0.001586 | 0.001710 |
| define_tpl_struct_inline_move.test | RUN | 0.291521 | 0.558501 | 0.673664 | 0.001548 | 0.001800 | 0.001753 |
| define_assoc_tpl_struct.test | RUN | 0.473776 | 1.163827 | 1.308471 | 0.001722 | 0.001825 | 0.001775 |
| define_assoc_tpl_struct_empty.test | RUN | 0.436938 | 1.000573 | 1.128317 | 0.001473 | 0.001853 | 0.001670 |
| define_assoc_tpl_struct_move.test | RUN | 0.282611 | 0.561879 | 0.667422 | 0.001735 | 0.001571 | 0.001806 |
| std_tuple.test | RUN | 0.292667 | 0.622599 | 0.734498 | 0.001588 | 0.001685 | 0.001738 |
| std_tuple_iterator.test | RUN | 0.289464 | 0.660754 | 0.754180 | 0.001740 | 0.001776 | 0.001546 |
| ref_vector.test | RUN | 0.324445 | 0.676818 | 0.793610 | 0.001876 | 0.001808 | 0.001772 |
| flatten_view.test | RUN | 0.349406 | 0.826347 | 0.958123 | 0.001612 | 0.001730 | 0.001458 |
| github-159.test | COMPILE | 0.162991 | 0.308520 | 0.350438 | — | — | — |
| github-176.test | RUN | 0.325211 | 0.742098 | 0.796922 | 0.001692 | 0.001686 | 0.001725 |
| size.test | COMPILE | 0.272266 | 0.524147 | 0.633466 | — | — | — |
| fused.test | RUN | 0.380272 | 0.783000 | 0.950843 | 0.001552 | 0.001832 | 0.001752 |
| fused_function_object.test | RUN | 0.283829 | 0.605554 | 0.723952 | 0.001528 | 0.001845 | 0.001814 |
| fused_procedure.test | RUN | 0.369909 | 0.739907 | 0.916517 | 0.001658 | 0.001615 | 0.001672 |
| unfused.test | RUN | 0.297119 | 0.611426 | 0.723422 | 0.001689 | 0.001754 | 0.001756 |
| unfused_typed.test | RUN | 0.298868 | 0.670827 | 0.756231 | 0.001558 | 0.002724 | 0.001679 |
| make_fused.test | RUN | 0.380929 | 0.782017 | 0.924493 | 0.001593 | 0.001824 | 0.001612 |
| make_fused_function_object.test | RUN | 0.290061 | 0.624802 | 0.707076 | 0.001527 | 0.001682 | 0.001608 |
| make_fused_procedure.test | RUN | 0.375399 | 0.771249 | 0.897869 | 0.001771 | 0.001769 | 0.001737 |
| make_unfused.test | RUN | 0.293597 | 0.603904 | 0.720000 | 0.001704 | 0.001790 | 0.001700 |
| invoke.test | RUN | 0.721758 | 2.185215 | 2.460961 | 0.001713 | 0.001540 | 0.001794 |
| invoke_function_object.test | RUN | 0.382549 | 0.773803 | 0.917269 | 0.001681 | 0.001885 | 0.001627 |
| invoke_procedure.test | RUN | 0.579489 | 1.622996 | 1.866663 | 0.001547 | 0.001697 | 0.001701 |
| swap.test | RUN | 0.439252 | 0.996903 | 1.147380 | 0.001736 | 0.001830 | 0.001631 |
| is_sequence.test | COMPILE | 0.080195 | 0.141713 | 0.146287 | — | — | — |
| is_view.test | COMPILE | 0.080609 | 0.138178 | 0.146837 | — | — | — |
| pair_deque.test | COMPILE | 0.164067 | 0.319528 | 0.349207 | — | — | — |
| pair_list.test | COMPILE | 0.163340 | 0.300903 | 0.348619 | — | — | — |
| pair_map.test | COMPILE | 0.166310 | 0.322894 | 0.365982 | — | — | — |
| pair_set.test | COMPILE | 0.170430 | 0.316648 | 0.358817 | — | — | — |
| pair_vector.test | COMPILE | 0.165903 | 0.312495 | 0.350661 | — | — | — |
| pair_nest.test | COMPILE | 0.065938 | 0.130752 | 0.139240 | — | — | — |
| index_sequence.test | COMPILE | 0.061862 | 0.086341 | 0.101293 | — | — | — |
| and.test | COMPILE | 0.047642 | 0.067010 | 0.070105 | — | — | — |
| tag_of.test | COMPILE | 0.080153 | 0.134522 | 0.142100 | — | — | — |
| unused.test | COMPILE | 0.037299 | 0.082170 | 0.081351 | — | — | — |
| tag_of_fallback.test | COMPILE | 0.062395 | 0.133168 | 0.139946 | — | — | — |
| make_pair_r-value.test | COMPILE | 0.049957 | 0.081995 | 0.132709 | — | — | — |
| quick.test | RUN | 0.208307 | 0.430663 | 0.445121 | 0.000589 | 0.001190 | 0.001253 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.993× | 0.987×, 1.000× | 1.005× |
| clang | 1.000× | 0.992×, 1.009× | 0.991× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adapt_adt.test | 10 | 0.001084s | 0.001070s | 0.001099s | byte equality observed |
| adapt_adt_empty.test | 10 | 0.001053s | 0.001067s | 0.001055s | byte equality observed |
| adapt_adt_named.test | 10 | 0.001068s | 0.001075s | 0.001055s | byte equality observed |
| adapt_adt_named_empty.test | 10 | 0.001062s | 0.001074s | 0.001062s | byte equality observed |
| adapt_assoc_adt.test | 10 | 0.001048s | 0.001070s | 0.001060s | byte equality observed |
| adapt_assoc_adt_empty.test | 10 | 0.001075s | 0.001074s | 0.001101s | byte equality observed |
| adapt_assoc_adt_named.test | 8 | 0.001074s | 0.001064s | 0.001068s | byte equality observed |
| adapt_assoc_adt_named_empty.test | 8 | 0.001067s | 0.001101s | 0.001083s | byte equality observed |
| adapt_assoc_struct.test | 10 | 0.001082s | 0.001053s | 0.001063s | byte equality observed |
| adapt_assoc_struct_empty.test | 10 | 0.001066s | 0.001057s | 0.001078s | byte equality observed |
| adapt_assoc_struct_named.test | 10 | 0.001075s | 0.001084s | 0.001084s | byte equality observed |
| adapt_assoc_struct_named_empty.test | 10 | 0.001072s | 0.001065s | 0.001080s | byte equality observed |
| adapt_assoc_tpl_adt.test | 10 | 0.001062s | 0.001091s | 0.001059s | byte equality observed |
| adapt_assoc_tpl_adt_empty.test | 10 | 0.001063s | 0.001080s | 0.001056s | byte equality observed |
| adapt_assoc_tpl_struct.test | 10 | 0.001088s | 0.001092s | 0.001061s | byte equality observed |
| adapt_assoc_tpl_struct_empty.test | 10 | 0.001039s | 0.001052s | 0.001045s | byte equality observed |
| adapt_struct.test | 10 | 0.001077s | 0.001085s | 0.001065s | byte equality observed |
| adapt_struct_empty.test | 10 | 0.001050s | 0.001041s | 0.001065s | byte equality observed |
| adapt_struct_named.test | 10 | 0.001053s | 0.001085s | 0.001060s | byte equality observed |
| adapt_struct_named_empty.test | 10 | 0.001062s | 0.001069s | 0.001048s | byte equality observed |
| adapt_tpl_adt.test | 10 | 0.001074s | 0.001112s | 0.001101s | byte equality observed |
| adapt_tpl_adt_empty.test | 9 | 0.001069s | 0.001068s | 0.001042s | byte equality observed |
| adapt_tpl_struct.test | 10 | 0.001078s | 0.001069s | 0.001036s | byte equality observed |
| adapt_tpl_struct_empty.test | 10 | 0.001068s | 0.001038s | 0.001055s | byte equality observed |
| adt_attribute_proxy.test | 10 | 0.001066s | 0.001056s | 0.001053s | byte equality observed |
| all.test | 10 | 0.001058s | 0.001093s | 0.001038s | byte equality observed |
| any.test | 10 | 0.001044s | 0.001048s | 0.001056s | byte equality observed |
| array.test | 10 | 0.001053s | 0.001082s | 0.001060s | byte equality observed |
| as_deque.test | 10 | 0.001067s | 0.001057s | 0.001070s | byte equality observed |
| as_list.test | 10 | 0.001111s | 0.001102s | 0.001089s | byte equality observed |
| as_map.test | 10 | 0.001068s | 0.001084s | 0.001077s | byte equality observed |
| as_map_assoc.test | 10 | 0.001066s | 0.001061s | 0.001057s | byte equality observed |
| as_set.test | 10 | 0.001072s | 0.001095s | 0.001065s | byte equality observed |
| as_vector.test | 10 | 0.001064s | 0.001078s | 0.001084s | byte equality observed |
| back_extended_deque.test | 10 | 0.001035s | 0.001039s | 0.001048s | byte equality observed |
| boost_array.test | 10 | 0.001032s | 0.001084s | 0.001046s | byte equality observed |
| boost_tuple.test | 10 | 0.001092s | 0.001094s | 0.001067s | byte equality observed |
| boost_tuple_iterator.test | 10 | 0.001041s | 0.001048s | 0.001035s | byte equality observed |
| clear.test | 10 | 0.001077s | 0.001081s | 0.001034s | byte equality observed |
| cons.test | 10 | 0.001083s | 0.001088s | 0.001106s | byte equality observed |
| convert_boost_tuple.test | 10 | 0.001054s | 0.001053s | 0.001036s | byte equality observed |
| convert_deque.test | 10 | 0.001056s | 0.001036s | 0.001047s | byte equality observed |
| convert_list.test | 10 | 0.001059s | 0.001045s | 0.001081s | byte equality observed |
| convert_std_pair.test | 10 | 0.001066s | 0.001037s | 0.001064s | byte equality observed |
| convert_std_tuple.test | 10 | 0.001072s | 0.001044s | 0.001066s | byte equality observed |
| convert_vector.test | 10 | 0.001053s | 0.001081s | 0.001079s | byte equality observed |
| copy.test | 10 | 0.001069s | 0.001042s | 0.001092s | byte equality observed |
| count.test | 10 | 0.001065s | 0.001082s | 0.001045s | byte equality observed |
| count_if.test | 9 | 0.001063s | 0.001091s | 0.001025s | byte equality observed |
| deduce_sequence.test | 10 | 0.001047s | 0.001035s | 0.001060s | byte equality observed |
| define_assoc_struct.test | 10 | 0.001061s | 0.001069s | 0.001057s | byte equality observed |
| define_assoc_struct_empty.test | 10 | 0.001061s | 0.001067s | 0.001045s | byte equality observed |
| define_assoc_struct_move.test | 10 | 0.001047s | 0.001038s | 0.001031s | byte equality observed |
| define_assoc_tpl_struct.test | 10 | 0.001072s | 0.001110s | 0.001076s | byte equality observed |
| define_assoc_tpl_struct_empty.test | 10 | 0.001074s | 0.001093s | 0.001063s | byte equality observed |
| define_assoc_tpl_struct_move.test | 10 | 0.001039s | 0.001032s | 0.001020s | byte equality observed |
| define_struct.test | 10 | 0.001070s | 0.001085s | 0.001062s | byte equality observed |
| define_struct_empty.test | 10 | 0.001048s | 0.001064s | 0.001055s | byte equality observed |
| define_struct_inline.test | 10 | 0.001050s | 0.001061s | 0.001055s | byte equality observed |
| define_struct_inline_empty.test | 10 | 0.001057s | 0.001073s | 0.001042s | byte equality observed |
| define_struct_inline_move.test | 10 | 0.001060s | 0.001075s | 0.001054s | byte equality observed |
| define_struct_move.test | 10 | 0.001067s | 0.001097s | 0.001054s | byte equality observed |
| define_tpl_struct.test | 10 | 0.001069s | 0.001067s | 0.001055s | byte equality observed |
| define_tpl_struct_empty.test | 10 | 0.001070s | 0.001131s | 0.001085s | byte equality observed |
| define_tpl_struct_inline.test | 10 | 0.001059s | 0.001082s | 0.001046s | byte equality observed |
| define_tpl_struct_inline_empty.test | 10 | 0.001063s | 0.001061s | 0.001057s | byte equality observed |
| define_tpl_struct_inline_move.test | 10 | 0.001038s | 0.001022s | 0.001035s | byte equality observed |
| define_tpl_struct_move.test | 10 | 0.001056s | 0.001066s | 0.001083s | byte equality observed |
| deque_comparison.test | 10 | 0.001047s | 0.001023s | 0.001020s | byte equality observed |
| deque_construction.test | 10 | 0.001062s | 0.001058s | 0.001013s | byte equality observed |
| deque_copy.test | 10 | 0.001050s | 0.001051s | 0.001033s | byte equality observed |
| deque_hash.test | 10 | 0.001054s | 0.001070s | 0.001021s | byte equality observed |
| deque_iterator.test | 10 | 0.001038s | 0.001057s | 0.001066s | byte equality observed |
| deque_make.test | 10 | 0.001070s | 0.001032s | 0.001051s | byte equality observed |
| deque_misc.test | 10 | 0.001039s | 0.001025s | 0.001057s | byte equality observed |
| deque_move.test | 10 | 0.001065s | 0.001063s | 0.001083s | byte equality observed |
| deque_mutate.test | 10 | 0.001060s | 0.001023s | 0.001053s | byte equality observed |
| deque_nest.test | 10 | 0.001088s | 0.001058s | 0.001059s | byte equality observed |
| deque_tie.test | 10 | 0.001025s | 0.001037s | 0.001046s | byte equality observed |
| deque_value_at.test | 10 | 0.001053s | 0.001047s | 0.001079s | byte equality observed |
| erase.test | 10 | 0.001074s | 0.001062s | 0.001040s | byte equality observed |
| erase_key.test | 10 | 0.001090s | 0.001118s | 0.001083s | byte equality observed |
| filter.test | 10 | 0.001057s | 0.001070s | 0.001041s | byte equality observed |
| filter_if.test | 10 | 0.001083s | 0.001074s | 0.001067s | byte equality observed |
| filter_view.test | 10 | 0.001115s | 0.001057s | 0.001065s | byte equality observed |
| find.test | 10 | 0.001069s | 0.001076s | 0.001079s | byte equality observed |
| find_if.test | 10 | 0.001103s | 0.001091s | 0.001059s | byte equality observed |
| flatten.test | 10 | 0.001065s | 0.001053s | 0.001066s | byte equality observed |
| flatten_view.test | 10 | 0.001058s | 0.001033s | 0.001049s | byte equality observed |
| fold.test | 10 | 0.001069s | 0.001083s | 0.001054s | byte equality observed |
| for_each.test | 10 | 0.001065s | 0.001083s | 0.001061s | byte equality observed |
| front_extended_deque.test | 9 | 0.001059s | 0.001037s | 0.001047s | byte equality observed |
| fused.test | 10 | 0.001073s | 0.001016s | 0.001039s | byte equality observed |
| fused_function_object.test | 10 | 0.001036s | 0.001048s | 0.001059s | byte equality observed |
| fused_procedure.test | 10 | 0.001064s | 0.001064s | 0.001029s | byte equality observed |
| github-176.test | 10 | 0.001049s | 0.001031s | 0.001067s | byte equality observed |
| hash.test | 10 | 0.001060s | 0.001059s | 0.001048s | byte equality observed |
| identity_view.test | 10 | 0.001081s | 0.001083s | 0.001095s | byte equality observed |
| insert.test | 10 | 0.001106s | 0.001093s | 0.001095s | byte equality observed |
| insert_range.test | 10 | 0.001096s | 0.001059s | 0.001082s | byte equality observed |
| invoke.test | 10 | 0.001043s | 0.001060s | 0.001061s | byte equality observed |
| invoke_function_object.test | 10 | 0.001072s | 0.001021s | 0.001048s | byte equality observed |
| invoke_procedure.test | 10 | 0.001037s | 0.001034s | 0.001037s | byte equality observed |
| io.test | 4 | 0.001253s | 0.001211s | 0.001207s | byte equality observed |
| iter_fold.test | 10 | 0.001043s | 0.001044s | 0.001067s | byte equality observed |
| iterator_range.test | 10 | 0.001099s | 0.001077s | 0.001069s | byte equality observed |
| join.test | 10 | 0.001049s | 0.001034s | 0.001055s | byte equality observed |
| joint_view.test | 10 | 0.001107s | 0.001119s | 0.001084s | byte equality observed |
| list_comparison.test | 10 | 0.001061s | 0.001028s | 0.001051s | byte equality observed |
| list_construction.test | 10 | 0.001036s | 0.001036s | 0.001040s | byte equality observed |
| list_copy.test | 10 | 0.001092s | 0.001047s | 0.001035s | byte equality observed |
| list_hash.test | 10 | 0.001064s | 0.001038s | 0.001069s | byte equality observed |
| list_iterator.test | 10 | 0.001070s | 0.001079s | 0.001071s | byte equality observed |
| list_make.test | 10 | 0.001079s | 0.001048s | 0.001049s | byte equality observed |
| list_misc.test | 10 | 0.001073s | 0.001053s | 0.001071s | byte equality observed |
| list_mutate.test | 10 | 0.001056s | 0.001039s | 0.001069s | byte equality observed |
| list_nest.test | 10 | 0.001041s | 0.001039s | 0.001050s | byte equality observed |
| list_tie.test | 10 | 0.001076s | 0.001069s | 0.001029s | byte equality observed |
| list_value_at.test | 10 | 0.001044s | 0.001046s | 0.001034s | byte equality observed |
| make_fused.test | 8 | 0.001087s | 0.001018s | 0.001034s | byte equality observed |
| make_fused_function_object.test | 10 | 0.001053s | 0.001066s | 0.001036s | byte equality observed |
| make_fused_procedure.test | 10 | 0.001062s | 0.001044s | 0.001045s | byte equality observed |
| make_list.test | 10 | 0.001074s | 0.001032s | 0.001052s | byte equality observed |
| make_unfused.test | 10 | 0.001042s | 0.001059s | 0.001055s | byte equality observed |
| make_vector.test | 10 | 0.001048s | 0.001028s | 0.001015s | byte equality observed |
| map.test | 10 | 0.001092s | 0.001068s | 0.001079s | byte equality observed |
| map_comparison.test | 10 | 0.001035s | 0.001010s | 0.001029s | byte equality observed |
| map_construction.test | 10 | 0.001063s | 0.001073s | 0.001074s | byte equality observed |
| map_copy.test | 10 | 0.001050s | 0.001052s | 0.001025s | byte equality observed |
| map_misc.test | 8 | 0.001086s | 0.001084s | 0.001036s | byte equality observed |
| map_move.test | 10 | 0.001068s | 0.001074s | 0.001041s | byte equality observed |
| map_mutate.test | 10 | 0.001038s | 0.001028s | 0.001014s | byte equality observed |
| map_tie.test | 10 | 0.001025s | 0.001042s | 0.001020s | byte equality observed |
| move.test | 10 | 0.001068s | 0.001025s | 0.001030s | byte equality observed |
| nil.test | 10 | 0.001055s | 0.001032s | 0.001041s | byte equality observed |
| none.test | 10 | 0.001077s | 0.001057s | 0.001059s | byte equality observed |
| nview.test | 10 | 0.001068s | 0.001055s | 0.001036s | byte equality observed |
| pop_back.test | 10 | 0.001092s | 0.001113s | 0.001079s | byte equality observed |
| pop_front.test | 10 | 0.001089s | 0.001079s | 0.001030s | byte equality observed |
| push_back.test | 10 | 0.001098s | 0.001083s | 0.001087s | byte equality observed |
| push_front.test | 10 | 0.001116s | 0.001076s | 0.001067s | byte equality observed |
| quick.test | 10 | 0.000473s | 0.001043s | 0.001023s | byte equality observed |
| ref_vector.test | 10 | 0.001059s | 0.001049s | 0.001041s | byte equality observed |
| remove.test | 10 | 0.001091s | 0.001090s | 0.001082s | byte equality observed |
| remove_if.test | 10 | 0.001073s | 0.001070s | 0.001068s | byte equality observed |
| repetitive_view.test | 10 | 0.001038s | 0.001045s | 0.001037s | byte equality observed |
| replace.test | 10 | 0.001086s | 0.001056s | 0.001047s | byte equality observed |
| replace_if.test | 10 | 0.001080s | 0.001104s | 0.001056s | byte equality observed |
| reverse.test | 10 | 0.001080s | 0.001081s | 0.001064s | byte equality observed |
| reverse_fold.test | 10 | 0.001052s | 0.001070s | 0.001038s | byte equality observed |
| reverse_iter_fold.test | 10 | 0.001060s | 0.001058s | 0.001064s | byte equality observed |
| reverse_view.test | 10 | 0.001080s | 0.001078s | 0.001054s | byte equality observed |
| segmented_find.test | 10 | 0.001053s | 0.001038s | 0.001015s | byte equality observed |
| segmented_find_if.test | 10 | 0.001098s | 0.001042s | 0.001031s | byte equality observed |
| segmented_fold.test | 10 | 0.001106s | 0.001090s | 0.001063s | byte equality observed |
| segmented_for_each.test | 10 | 0.001092s | 0.001103s | 0.001086s | byte equality observed |
| segmented_iterator_range.test | 10 | 0.001102s | 0.001135s | 0.001089s | byte equality observed |
| set.test | 10 | 0.001080s | 0.001084s | 0.001084s | byte equality observed |
| single_view.test | 10 | 0.001075s | 0.001071s | 0.001042s | byte equality observed |
| std_array.test | 10 | 0.001041s | 0.001043s | 0.001050s | byte equality observed |
| std_pair.test | 10 | 0.001079s | 0.001114s | 0.001041s | byte equality observed |
| std_tuple.test | 10 | 0.001057s | 0.001063s | 0.001044s | byte equality observed |
| std_tuple_iterator.test | 10 | 0.001046s | 0.001056s | 0.001008s | byte equality observed |
| swap.test | 10 | 0.001075s | 0.001068s | 0.001031s | byte equality observed |
| transform.test | 10 | 0.001068s | 0.001077s | 0.001044s | byte equality observed |
| transform_view.test | 10 | 0.001076s | 0.001076s | 0.001083s | byte equality observed |
| tuple_comparison.test | 10 | 0.001055s | 0.001052s | 0.001023s | byte equality observed |
| tuple_construction.test | 10 | 0.001071s | 0.001090s | 0.001099s | byte equality observed |
| tuple_conversion.test | 10 | 0.001075s | 0.001115s | 0.001073s | byte equality observed |
| tuple_copy.test | 10 | 0.001075s | 0.001085s | 0.001033s | byte equality observed |
| tuple_element.test | 10 | 0.001052s | 0.001036s | 0.001036s | byte equality observed |
| tuple_hash.test | 10 | 0.001052s | 0.001060s | 0.001047s | byte equality observed |
| tuple_make.test | 10 | 0.001086s | 0.001039s | 0.001044s | byte equality observed |
| tuple_misc.test | 10 | 0.001061s | 0.001055s | 0.001059s | byte equality observed |
| tuple_mutate.test | 10 | 0.001063s | 0.001061s | 0.001035s | byte equality observed |
| tuple_nest.test | 10 | 0.001061s | 0.001066s | 0.001051s | byte equality observed |
| tuple_tie.test | 10 | 0.001073s | 0.001055s | 0.001050s | byte equality observed |
| tuple_traits__maybe_variadic.test | 10 | 0.001036s | 0.001033s | 0.001045s | byte equality observed |
| tuple_traits__no_variadic.test | 10 | 0.001067s | 0.001051s | 0.001055s | byte equality observed |
| unfused.test | 10 | 0.001039s | 0.001083s | 0.001062s | byte equality observed |
| unfused_typed.test | 10 | 0.001064s | 0.001052s | 0.001067s | byte equality observed |
| vector_comparison.test | 10 | 0.001055s | 0.001054s | 0.001086s | byte equality observed |
| vector_construction.test | 10 | 0.001037s | 0.001015s | 0.001027s | byte equality observed |
| vector_conversion.test | 10 | 0.001068s | 0.001036s | 0.001042s | byte equality observed |
| vector_copy.test | 9 | 0.001069s | 0.001077s | 0.001091s | byte equality observed |
| vector_hash.test | 10 | 0.001070s | 0.001094s | 0.001028s | byte equality observed |
| vector_iterator.test | 9 | 0.001035s | 0.001078s | 0.001086s | byte equality observed |
| vector_make.test | 10 | 0.001084s | 0.001044s | 0.001103s | byte equality observed |
| vector_misc.test | 10 | 0.001065s | 0.001077s | 0.001064s | byte equality observed |
| vector_move.test | 10 | 0.001080s | 0.001079s | 0.001082s | byte equality observed |
| vector_mutate.test | 10 | 0.001058s | 0.001054s | 0.001035s | byte equality observed |
| vector_n.test | 10 | 0.001068s | 0.001076s | 0.001100s | byte equality observed |
| vector_nest.test | 10 | 0.001051s | 0.001049s | 0.001032s | byte equality observed |
| vector_tie.test | 10 | 0.001066s | 0.001062s | 0.001065s | byte equality observed |
| vector_traits__maybe_variadic.test | 10 | 0.001068s | 0.001094s | 0.001104s | byte equality observed |
| vector_traits__no_variadic.test | 10 | 0.001043s | 0.001076s | 0.001070s | byte equality observed |
| vector_value_at.test | 10 | 0.001070s | 0.001032s | 0.001075s | byte equality observed |
| zip.test | 10 | 0.001078s | 0.001058s | 0.001027s | byte equality observed |
| zip2.test | 10 | 0.001090s | 0.001057s | 0.001051s | byte equality observed |
| zip_ignore.test | 10 | 0.001061s | 0.001050s | 0.001038s | byte equality observed |
| zip_view.test | 10 | 0.001029s | 0.001041s | 0.001080s | byte equality observed |
| zip_view2.test | 10 | 0.001045s | 0.001058s | 0.001073s | byte equality observed |
| zip_view_ignore.test | 10 | 0.001039s | 0.001021s | 0.001045s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
