# libs/fusion/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| all.test | RUN | 0.390226 | 0.732763 | 1.021916 | 0.001368 | 0.001548 | 0.001668 |
| any.test | RUN | 0.380808 | 0.703840 | 0.956106 | 0.001732 | 0.001610 | 0.001635 |
| clear.test | RUN | 0.273211 | 0.632642 | 0.789942 | 0.001578 | 0.001753 | 0.001765 |
| copy.test | RUN | 0.239275 | 0.474616 | 0.642165 | 0.001639 | 0.001810 | 0.001502 |
| count.test | RUN | 0.266111 | 0.527627 | 0.725339 | 0.001682 | 0.001761 | 0.001655 |
| count_if.test | RUN | 0.251645 | 0.494122 | 0.668273 | 0.001560 | 0.001794 | 0.001443 |
| erase.test | RUN | 0.339315 | 0.858161 | 1.069617 | 0.001305 | 0.001906 | 0.001567 |
| erase_key.test | RUN | 0.333958 | 0.846120 | 1.014170 | 0.001774 | 0.001483 | 0.001588 |
| filter.test | RUN | 0.304466 | 0.704963 | 0.909923 | 0.001870 | 0.001856 | 0.001803 |
| filter_if.test | RUN | 0.338812 | 0.844075 | 1.062451 | 0.001580 | 0.001972 | 0.001820 |
| find.test | RUN | 0.292029 | 0.587941 | 0.815276 | 0.001573 | 0.001631 | 0.001712 |
| find_if.test | RUN | 0.282519 | 0.552005 | 0.779634 | 0.001643 | 0.001791 | 0.001386 |
| fold.test | RUN | 0.305513 | 0.658451 | 0.892675 | 0.001845 | 0.001851 | 0.001655 |
| for_each.test | RUN | 0.283607 | 0.604678 | 0.802956 | 0.001711 | 0.001917 | 0.001786 |
| insert.test | RUN | 0.402808 | 1.077781 | 1.287393 | 0.001772 | 0.001490 | 0.001484 |
| insert_range.test | RUN | 0.401647 | 1.053905 | 1.278591 | 0.001406 | 0.001728 | 0.001649 |
| iter_fold.test | RUN | 0.465560 | 1.027110 | 1.556088 | 0.001843 | 0.001827 | 0.001755 |
| move.test | RUN | 0.253444 | 0.499329 | 0.673902 | 0.001770 | 0.001644 | 0.001722 |
| none.test | RUN | 0.391517 | 0.691178 | 1.020955 | 0.001398 | 0.001445 | 0.001538 |
| pop_back.test | RUN | 0.375382 | 0.948871 | 1.126857 | 0.001779 | 0.001984 | 0.001580 |
| pop_front.test | RUN | 0.314778 | 0.761048 | 0.940977 | 0.001906 | 0.001680 | 0.001404 |
| push_back.test | RUN | 0.393456 | 1.074424 | 1.252431 | 0.001804 | 0.001921 | 0.001843 |
| push_front.test | RUN | 0.346247 | 0.892583 | 1.069702 | 0.001870 | 0.001769 | 0.001465 |
| remove.test | RUN | 0.389436 | 1.034796 | 1.294846 | 0.001637 | 0.001640 | 0.001670 |
| remove_if.test | RUN | 0.352005 | 0.825909 | 1.053011 | 0.001784 | 0.001844 | 0.001744 |
| replace.test | RUN | 0.328912 | 0.827099 | 1.004837 | 0.001797 | 0.001888 | 0.001673 |
| replace_if.test | RUN | 0.330780 | 0.831469 | 0.989769 | 0.001856 | 0.001910 | 0.001759 |
| reverse_fold.test | RUN | 0.521538 | 1.150332 | 1.721199 | 0.001673 | 0.001690 | 0.001791 |
| reverse_iter_fold.test | RUN | 0.536419 | 1.221373 | 1.785723 | 0.001744 | 0.001506 | 0.001809 |
| reverse.test | RUN | 0.328760 | 0.863623 | 0.999842 | 0.001620 | 0.001670 | 0.001415 |
| segmented_for_each.test | RUN | 0.557923 | 1.307537 | 2.058057 | 0.001844 | 0.001725 | 0.001724 |
| segmented_find.test | RUN | 0.369197 | 0.837924 | 1.153020 | 0.001647 | 0.001527 | 0.001854 |
| segmented_find_if.test | RUN | 0.362547 | 0.856337 | 1.146047 | 0.001522 | 0.001890 | 0.001729 |
| segmented_fold.test | RUN | 0.444165 | 1.091243 | 1.463177 | 0.001908 | 0.001747 | 0.001743 |
| transform.test | RUN | 0.343549 | 0.893565 | 1.048729 | 0.001541 | 0.001478 | 0.001661 |
| join.test | RUN | 0.268975 | 0.556648 | 0.752907 | 0.001760 | 0.001823 | 0.001708 |
| zip.test | RUN | 0.458307 | 1.052426 | 1.442187 | 0.001774 | 0.001870 | 0.001783 |
| zip2.test | RUN | 0.445757 | 0.983016 | 1.351999 | 0.001637 | 0.001658 | 0.001703 |
| zip_ignore.test | RUN | 0.340861 | 0.702773 | 0.910253 | 0.001469 | 0.001342 | 0.001486 |
| flatten.test | RUN | 0.320050 | 0.733384 | 0.962586 | 0.001708 | 0.001663 | 0.001468 |
| ticket-5490.test | COMPILE | 0.178834 | 0.345903 | 0.420109 | — | — | — |
| as_deque.test | RUN | 0.317121 | 0.724643 | 1.007969 | 0.002290 | 0.001768 | 0.001632 |
| as_list.test | RUN | 0.360239 | 0.878399 | 1.071475 | 0.001364 | 0.001382 | 0.001699 |
| as_map.test | RUN | 0.343976 | 0.871502 | 1.080457 | 0.001699 | 0.001717 | 0.001426 |
| as_map_assoc.test | RUN | 0.340507 | 0.813225 | 1.028542 | 0.001376 | 0.001515 | 0.001480 |
| as_set.test | RUN | 0.383724 | 0.992380 | 1.249974 | 0.001862 | 0.001756 | 0.001371 |
| as_vector.test | RUN | 0.357927 | 0.891361 | 1.111208 | 0.001362 | 0.001565 | 0.001716 |
| boost_tuple.test | RUN | 0.353083 | 0.875559 | 1.058759 | 0.001548 | 0.001646 | 0.001723 |
| boost_tuple_iterator.test | RUN | 0.257224 | 0.542569 | 0.668775 | 0.001708 | 0.001545 | 0.001747 |
| cons.test | RUN | 0.440570 | 1.001604 | 1.280075 | 0.001532 | 0.001701 | 0.001474 |
| convert_boost_tuple.test | RUN | 0.288788 | 0.756517 | 0.846625 | 0.001582 | 0.001725 | 0.001419 |
| convert_deque.test | RUN | 0.291910 | 0.762607 | 0.841953 | 0.001373 | 0.001652 | 0.001302 |
| convert_list.test | RUN | 0.291237 | 0.789923 | 0.858569 | 0.001419 | 0.001712 | 0.001669 |
| convert_std_pair.test | RUN | 0.306432 | 0.770599 | 0.863054 | 0.001780 | 0.001594 | 0.001243 |
| convert_std_tuple.test | RUN | 0.290854 | 0.755584 | 0.843692 | 0.001522 | 0.001652 | 0.001639 |
| convert_vector.test | RUN | 0.285128 | 0.750520 | 0.829029 | 0.001172 | 0.001549 | 0.001766 |
| filter_view.test | RUN | 0.329428 | 0.789666 | 0.988157 | 0.001553 | 0.001711 | 0.001714 |
| hash.test | RUN | 0.340404 | 0.719104 | 0.991847 | 0.001427 | 0.001577 | 0.001296 |
| io.test | RUN | 0.344757 | 1.022479 | 1.150693 | 0.001683 | 0.002071 | 0.001744 |
| iterator_range.test | RUN | 0.317126 | 0.766464 | 0.937637 | 0.001518 | 0.001720 | 0.001831 |
| joint_view.test | RUN | 0.410863 | 1.274337 | 1.418799 | 0.001691 | 0.001705 | 0.001777 |
| list_comparison.test | RUN | 0.252419 | 0.535206 | 0.692421 | 0.001545 | 0.001620 | 0.001467 |
| list_construction.test | RUN | 0.251197 | 0.527640 | 0.671268 | 0.001555 | 0.001765 | 0.001661 |
| list_copy.test | RUN | 0.368770 | 0.934657 | 1.159763 | 0.001575 | 0.001620 | 0.001446 |
| list_iterator.test | RUN | 0.253660 | 0.514758 | 0.644227 | 0.001750 | 0.001971 | 0.001570 |
| list_hash.test | RUN | 0.344765 | 1.014764 | 0.940805 | 0.001629 | 0.001846 | 0.001728 |
| list_make.test | RUN | 0.267621 | 0.574407 | 0.731514 | 0.001854 | 0.001756 | 0.001695 |
| list_misc.test | RUN | 0.349080 | 0.675580 | 0.870987 | 0.001428 | 0.001813 | 0.001763 |
| list_mutate.test | RUN | 0.246466 | 0.446420 | 0.587511 | 0.001694 | 0.001760 | 0.001725 |
| list_nest.test | RUN | 0.461303 | 1.278248 | 1.638702 | 0.001310 | 0.001499 | 0.001333 |
| list_tie.test | RUN | 0.248556 | 0.511915 | 0.685731 | 0.001756 | 0.001839 | 0.001711 |
| list_value_at.test | RUN | 0.250465 | 0.492388 | 0.611182 | 0.001684 | 0.001910 | 0.001638 |
| deque_comparison.test | RUN | 0.257864 | 0.558605 | 0.736209 | 0.001468 | 0.001675 | 0.001781 |
| deque_construction.test | RUN | 0.251090 | 0.501241 | 0.672213 | 0.001824 | 0.001846 | 0.001599 |
| deque_copy.test | RUN | 0.350289 | 0.869121 | 1.084469 | 0.001635 | 0.001585 | 0.001508 |
| deque_iterator.test | RUN | 0.246103 | 0.549959 | 0.667612 | 0.001593 | 0.001754 | 0.001626 |
| deque_hash.test | RUN | 0.337706 | 0.958995 | 0.963906 | 0.001629 | 0.001866 | 0.001832 |
| deque_is_constructible.test | COMPILE | 0.148908 | 0.251410 | 0.325950 | — | — | — |
| deque_make.test | RUN | 0.252953 | 0.547441 | 0.698531 | 0.001774 | 0.001831 | 0.001760 |
| deque_misc.test | RUN | 0.326719 | 0.658642 | 0.863422 | 0.001591 | 0.001643 | 0.001470 |
| deque_move.test | RUN | 0.261522 | 0.574391 | 0.714391 | 0.001573 | 0.001606 | 0.001538 |
| deque_mutate.test | RUN | 0.237029 | 0.463037 | 0.593556 | 0.001760 | 0.001723 | 0.001760 |
| deque_nest.test | RUN | 0.678183 | 2.022471 | 2.618933 | 0.001785 | 0.001728 | 0.001390 |
| deque_tie.test | RUN | 0.238246 | 0.503164 | 0.644967 | 0.001616 | 0.001680 | 0.001537 |
| deque_value_at.test | RUN | 0.234070 | 0.457573 | 0.628466 | 0.001416 | 0.001660 | 0.001411 |
| front_extended_deque.test | RUN | 0.346005 | 0.686497 | 0.978084 | 0.001696 | 0.001862 | 0.001685 |
| back_extended_deque.test | RUN | 0.346648 | 0.704596 | 0.924671 | 0.001752 | 0.001797 | 0.001746 |
| make_list.test | RUN | 0.271346 | 0.589811 | 0.741529 | 0.001600 | 0.001858 | 0.001399 |
| make_vector.test | RUN | 0.267434 | 0.563806 | 0.772211 | 0.001565 | 0.001750 | 0.001528 |
| map.test | RUN | 0.333107 | 0.851582 | 1.010748 | 0.001680 | 0.001941 | 0.001772 |
| map_comparison.test | RUN | 0.263472 | 0.564881 | 0.722533 | 0.001578 | 0.001786 | 0.001608 |
| map_construction.test | RUN | 0.261027 | 0.531855 | 0.702206 | 0.001536 | 0.001664 | 0.001723 |
| map_copy.test | RUN | 0.317789 | 0.696209 | 0.856956 | 0.001670 | 0.001850 | 0.001737 |
| map_misc.test | RUN | 0.329073 | 0.676091 | 0.878794 | 0.001536 | 0.001782 | 0.001384 |
| map_move.test | RUN | 0.264591 | 0.557991 | 0.712225 | 0.001814 | 0.001626 | 0.001685 |
| map_mutate.test | RUN | 0.236588 | 0.471284 | 0.600878 | 0.001410 | 0.001501 | 0.001708 |
| map_tie.test | RUN | 0.246781 | 0.468181 | 0.640355 | 0.001691 | 0.001748 | 0.001582 |
| nil.test | RUN | 0.171443 | 0.318071 | 0.422949 | 0.001818 | 0.001811 | 0.001645 |
| nview.test | RUN | 0.327622 | 0.665748 | 0.893950 | 0.001720 | 0.001932 | 0.001735 |
| reverse_view.test | RUN | 0.387532 | 1.108290 | 1.298220 | 0.001722 | 0.001681 | 0.001646 |
| segmented_iterator_range.test | RUN | 0.473667 | 1.325247 | 1.600297 | 0.001754 | 0.001814 | 0.001478 |
| set.test | RUN | 0.311814 | 0.728967 | 0.936910 | 0.001710 | 0.001696 | 0.001862 |
| single_view.test | RUN | 0.288042 | 0.697935 | 0.824650 | 0.001813 | 0.001603 | 0.001614 |
| std_pair.test | RUN | 0.365553 | 0.857306 | 1.043591 | 0.001740 | 0.001753 | 0.001718 |
| boost_array.test | RUN | 0.313781 | 0.619595 | 0.812291 | 0.001711 | 0.001786 | 0.001442 |
| array.test | RUN | 0.313843 | 0.605670 | 0.793164 | 0.001769 | 0.001375 | 0.001618 |
| std_array.test | RUN | 0.307086 | 0.598371 | 0.804600 | 0.001694 | 0.001812 | 0.001533 |
| tuple_comparison.test | RUN | 0.276412 | 0.550171 | 0.716434 | 0.001735 | 0.001770 | 0.001778 |
| tuple_construction.test | RUN | 0.284636 | 0.592087 | 0.751088 | 0.001860 | 0.001931 | 0.001668 |
| tuple_conversion.test | RUN | 0.583771 | 1.749289 | 2.399105 | 0.001446 | 0.001387 | 0.001447 |
| tuple_copy.test | RUN | 0.398430 | 1.044871 | 1.252304 | 0.001538 | 0.001620 | 0.001796 |
| tuple_element.test | RUN | 0.261680 | 0.520921 | 0.708230 | 0.001687 | 0.001935 | 0.001371 |
| tuple_make.test | RUN | 0.290874 | 0.596945 | 0.759772 | 0.001670 | 0.001712 | 0.001744 |
| tuple_misc.test | RUN | 0.347476 | 0.741016 | 0.964923 | 0.001858 | 0.001837 | 0.001544 |
| tuple_mutate.test | RUN | 0.249821 | 0.480679 | 0.642357 | 0.001815 | 0.001785 | 0.001726 |
| tuple_nest.test | RUN | 1.007606 | 3.516079 | 4.460930 | 0.001660 | 0.001623 | 0.001714 |
| tuple_hash.test | RUN | 0.368880 | 0.990059 | 1.040919 | 0.001828 | 0.001858 | 0.001544 |
| tuple_tie.test | RUN | 0.265242 | 0.529058 | 0.710714 | 0.001582 | 0.001830 | 0.001373 |
| tuple_traits__maybe_variadic.test | RUN | 0.280982 | 0.665151 | 0.754526 | 0.001627 | 0.001766 | 0.001338 |
| tuple_traits__no_variadic.test | RUN | 0.324604 | 0.671722 | 0.818091 | 0.001563 | 0.001828 | 0.001789 |
| transform_view.test | RUN | 0.402922 | 1.109426 | 1.280105 | 0.001779 | 0.001908 | 0.001648 |
| identity_view.test | RUN | 0.383805 | 0.996429 | 1.239290 | 0.001812 | 0.001625 | 0.001665 |
| vector_comparison.test | RUN | 0.258850 | 0.564337 | 0.688527 | 0.001337 | 0.001791 | 0.001761 |
| vector_construction.test | RUN | 0.257702 | 0.503459 | 0.699645 | 0.001540 | 0.001840 | 0.001758 |
| vector_conversion.test | RUN | 0.700283 | 2.287587 | 3.040299 | 0.001754 | 0.001750 | 0.001663 |
| vector_copy.test | RUN | 0.375243 | 0.982973 | 1.278606 | 0.001420 | 0.001770 | 0.001549 |
| vector_iterator.test | RUN | 0.251777 | 0.547207 | 0.675825 | 0.001635 | 0.001602 | 0.001624 |
| vector_make.test | RUN | 0.269076 | 0.575193 | 0.708712 | 0.001532 | 0.001752 | 0.001646 |
| vector_misc.test | RUN | 0.333662 | 0.706568 | 0.889461 | 0.001599 | 0.001740 | 0.001762 |
| vector_move.test | RUN | 0.260244 | 0.585381 | 0.700253 | 0.001565 | 0.001589 | 0.001838 |
| vector_mutate.test | RUN | 0.233222 | 0.513040 | 0.661474 | 0.001576 | 0.001680 | 0.001649 |
| vector_n.test | RUN | 0.286351 | 0.584566 | 0.811223 | 0.001826 | 0.001843 | 0.001722 |
| vector_nest.test | RUN | 0.995592 | 3.665277 | 4.501365 | 0.001757 | 0.001564 | 0.001646 |
| vector_hash.test | RUN | 0.341764 | 0.956672 | 0.934473 | 0.001561 | 0.001865 | 0.001432 |
| vector_tie.test | RUN | 0.252060 | 0.528070 | 0.689973 | 0.001831 | 0.001694 | 0.001597 |
| vector_traits__maybe_variadic.test | RUN | 0.268078 | 0.583383 | 0.728677 | 0.001656 | 0.001852 | 0.001708 |
| vector_traits__no_variadic.test | RUN | 0.292731 | 0.620469 | 0.739151 | 0.001688 | 0.001476 | 0.001550 |
| vector_value_at.test | RUN | 0.247893 | 0.467365 | 0.648441 | 0.001475 | 0.001719 | 0.001701 |
| zip_view.test | RUN | 0.479368 | 1.142737 | 1.652064 | 0.001832 | 0.001842 | 0.001750 |
| zip_view2.test | RUN | 0.399583 | 0.924631 | 1.205913 | 0.001529 | 0.001603 | 0.001462 |
| zip_view_ignore.test | RUN | 0.365327 | 0.841423 | 1.095997 | 0.001534 | 0.001643 | 0.001715 |
| repetitive_view.test | RUN | 0.607541 | 1.481176 | 2.062588 | 0.001543 | 0.001713 | 0.001787 |
| deduce_sequence.test | RUN | 0.306569 | 0.639676 | 0.838191 | 0.001539 | 0.001601 | 0.001555 |
| adapt_adt_named.test | RUN | 0.351975 | 0.889714 | 1.073093 | 0.001462 | 0.001900 | 0.001658 |
| adapt_adt_named_empty.test | RUN | 0.314632 | 0.712099 | 0.855434 | 0.001665 | 0.001908 | 0.001654 |
| adapt_adt.test | RUN | 0.393542 | 1.146764 | 1.261644 | 0.001541 | 0.001739 | 0.001758 |
| adapt_adt_empty.test | RUN | 0.313276 | 0.701157 | 0.857401 | 0.001757 | 0.001838 | 0.001684 |
| adapt_assoc_adt_named.test | RUN | 0.392497 | 0.922046 | 1.118483 | 0.001650 | 0.001582 | 0.001790 |
| adapt_assoc_adt_named_empty.test | RUN | 0.312343 | 0.725551 | 0.867384 | 0.001679 | 0.001738 | 0.001670 |
| adapt_assoc_adt.test | RUN | 0.401348 | 0.941110 | 1.173988 | 0.001619 | 0.001716 | 0.001701 |
| adapt_assoc_adt_empty.test | RUN | 0.318869 | 0.717630 | 0.872158 | 0.001541 | 0.001616 | 0.001716 |
| adapt_assoc_struct_named.test | RUN | 0.399042 | 0.905521 | 1.108677 | 0.001458 | 0.001784 | 0.001716 |
| adapt_assoc_struct_named_empty.test | RUN | 0.320581 | 0.711982 | 0.864567 | 0.001692 | 0.001670 | 0.001673 |
| adapt_assoc_struct.test | RUN | 0.345867 | 0.841248 | 1.044352 | 0.001583 | 0.001579 | 0.001348 |
| adapt_assoc_struct_empty.test | RUN | 0.304544 | 0.711524 | 0.831362 | 0.001756 | 0.001640 | 0.001707 |
| adapt_assoc_tpl_adt.test | RUN | 0.420228 | 0.979720 | 1.224606 | 0.001602 | 0.001744 | 0.001718 |
| adapt_assoc_tpl_adt_empty.test | RUN | 0.317183 | 0.719568 | 0.862819 | 0.001656 | 0.001598 | 0.001748 |
| adapt_assoc_tpl_struct.test | RUN | 0.342776 | 0.825061 | 1.040531 | 0.001654 | 0.001744 | 0.001738 |
| adapt_assoc_tpl_struct_empty.test | RUN | 0.304684 | 0.715234 | 0.841064 | 0.001683 | 0.001754 | 0.001503 |
| adapt_struct_named.test | RUN | 0.349606 | 0.842883 | 1.050494 | 0.001750 | 0.001708 | 0.001708 |
| adapt_struct_named_empty.test | RUN | 0.323859 | 0.692592 | 0.874977 | 0.001753 | 0.001892 | 0.001727 |
| adapt_struct.test | RUN | 0.379842 | 0.983805 | 1.099380 | 0.001541 | 0.001808 | 0.001448 |
| adapt_struct_empty.test | RUN | 0.309280 | 0.707132 | 0.851179 | 0.001861 | 0.001700 | 0.001540 |
| adapt_tpl_adt.test | RUN | 0.393545 | 1.121902 | 1.194490 | 0.001760 | 0.001361 | 0.001764 |
| adapt_tpl_adt_empty.test | RUN | 0.325888 | 0.705698 | 0.879644 | 0.001403 | 0.001866 | 0.001684 |
| adapt_tpl_struct.test | RUN | 0.330168 | 0.813615 | 0.958132 | 0.001652 | 0.001312 | 0.001491 |
| adapt_tpl_struct_empty.test | RUN | 0.308102 | 0.699034 | 0.880090 | 0.001446 | 0.001900 | 0.001498 |
| adt_attribute_proxy.test | RUN | 0.357968 | 0.699775 | 0.917630 | 0.001720 | 0.001884 | 0.001472 |
| define_struct.test | RUN | 0.420541 | 0.972707 | 1.210572 | 0.001697 | 0.001718 | 0.001736 |
| define_struct_empty.test | RUN | 0.372904 | 0.802379 | 0.986196 | 0.001602 | 0.001802 | 0.001721 |
| define_struct_move.test | RUN | 0.254470 | 0.450561 | 0.616667 | 0.001662 | 0.002028 | 0.001699 |
| define_struct_inline.test | RUN | 0.439618 | 1.157157 | 1.333343 | 0.001800 | 0.001774 | 0.001742 |
| define_struct_inline_empty.test | RUN | 0.376531 | 0.831761 | 1.004328 | 0.001448 | 0.001786 | 0.001586 |
| define_struct_inline_move.test | RUN | 0.258194 | 0.436264 | 0.619144 | 0.001537 | 0.001808 | 0.001668 |
| define_assoc_struct.test | RUN | 0.413903 | 0.909450 | 1.139402 | 0.001762 | 0.001872 | 0.001643 |
| define_assoc_struct_empty.test | RUN | 0.363970 | 0.794382 | 0.990972 | 0.001661 | 0.001756 | 0.001699 |
| define_assoc_struct_move.test | RUN | 0.251442 | 0.441633 | 0.610407 | 0.001644 | 0.001790 | 0.001728 |
| define_tpl_struct.test | RUN | 0.400114 | 0.928678 | 1.166503 | 0.001732 | 0.001437 | 0.001601 |
| define_tpl_struct_empty.test | RUN | 0.372524 | 0.795757 | 1.000111 | 0.001754 | 0.001693 | 0.001766 |
| define_tpl_struct_move.test | RUN | 0.254878 | 0.463658 | 0.593839 | 0.001538 | 0.001636 | 0.001722 |
| define_tpl_struct_inline.test | RUN | 0.418531 | 1.087908 | 1.239486 | 0.001680 | 0.001772 | 0.001476 |
| define_tpl_struct_inline_empty.test | RUN | 0.367251 | 0.780990 | 0.998061 | 0.001472 | 0.001714 | 0.001703 |
| define_tpl_struct_inline_move.test | RUN | 0.254793 | 0.479704 | 0.628288 | 0.001780 | 0.001750 | 0.001660 |
| define_assoc_tpl_struct.test | RUN | 0.405868 | 0.939842 | 1.159696 | 0.001732 | 0.001901 | 0.001750 |
| define_assoc_tpl_struct_empty.test | RUN | 0.367250 | 0.808540 | 0.992397 | 0.001670 | 0.001793 | 0.001749 |
| define_assoc_tpl_struct_move.test | RUN | 0.253666 | 0.460452 | 0.624893 | 0.001614 | 0.001458 | 0.001694 |
| std_tuple.test | RUN | 0.257472 | 0.508105 | 0.665854 | 0.001604 | 0.001760 | 0.001408 |
| std_tuple_iterator.test | RUN | 0.250315 | 0.536922 | 0.682204 | 0.001720 | 0.001822 | 0.001604 |
| ref_vector.test | RUN | 0.283340 | 0.545177 | 0.697544 | 0.001637 | 0.001805 | 0.001675 |
| flatten_view.test | RUN | 0.307430 | 0.675738 | 0.862130 | 0.001802 | 0.001831 | 0.001513 |
| github-159.test | COMPILE | 0.143642 | 0.262882 | 0.316410 | — | — | — |
| github-176.test | RUN | 0.283603 | 0.602117 | 0.718169 | 0.001675 | 0.001852 | 0.001719 |
| size.test | COMPILE | 0.240919 | 0.420671 | 0.574948 | — | — | — |
| fused.test | RUN | 0.334928 | 0.636909 | 0.856217 | 0.001614 | 0.001738 | 0.001684 |
| fused_function_object.test | RUN | 0.249867 | 0.480523 | 0.654048 | 0.001735 | 0.001855 | 0.001671 |
| fused_procedure.test | RUN | 0.320332 | 0.618950 | 0.838997 | 0.001612 | 0.001770 | 0.001523 |
| unfused.test | RUN | 0.263136 | 0.498115 | 0.644142 | 0.001370 | 0.001870 | 0.001340 |
| unfused_typed.test | RUN | 0.261674 | 0.540619 | 0.681804 | 0.001723 | 0.001873 | 0.001740 |
| make_fused.test | RUN | 0.337351 | 0.649231 | 0.869241 | 0.001735 | 0.001839 | 0.001617 |
| make_fused_function_object.test | RUN | 0.251648 | 0.499876 | 0.628250 | 0.001584 | 0.001617 | 0.001831 |
| make_fused_procedure.test | RUN | 0.326850 | 0.607034 | 0.824984 | 0.001693 | 0.001668 | 0.001607 |
| make_unfused.test | RUN | 0.251722 | 0.492375 | 0.616260 | 0.001696 | 0.001840 | 0.001677 |
| invoke.test | RUN | 0.616469 | 1.756936 | 2.230305 | 0.001695 | 0.001738 | 0.001716 |
| invoke_function_object.test | RUN | 0.324696 | 0.630118 | 0.860598 | 0.001840 | 0.001890 | 0.001746 |
| invoke_procedure.test | RUN | 0.509813 | 1.311196 | 1.689413 | 0.001468 | 0.001617 | 0.001466 |
| swap.test | RUN | 0.377271 | 0.818928 | 1.058633 | 0.001424 | 0.001806 | 0.001821 |
| is_sequence.test | COMPILE | 0.075019 | 0.119653 | 0.138116 | — | — | — |
| is_view.test | COMPILE | 0.074677 | 0.117100 | 0.133710 | — | — | — |
| pair_deque.test | COMPILE | 0.146117 | 0.269836 | 0.315716 | — | — | — |
| pair_list.test | COMPILE | 0.146300 | 0.254688 | 0.314380 | — | — | — |
| pair_map.test | COMPILE | 0.145243 | 0.265130 | 0.326696 | — | — | — |
| pair_set.test | COMPILE | 0.148481 | 0.267399 | 0.327465 | — | — | — |
| pair_vector.test | COMPILE | 0.148676 | 0.258523 | 0.307048 | — | — | — |
| pair_nest.test | COMPILE | 0.059694 | 0.109794 | 0.126730 | — | — | — |
| index_sequence.test | COMPILE | 0.055363 | 0.077834 | 0.093771 | — | — | — |
| and.test | COMPILE | 0.043849 | 0.060292 | 0.067724 | — | — | — |
| tag_of.test | COMPILE | 0.072548 | 0.112551 | 0.129328 | — | — | — |
| unused.test | COMPILE | 0.034122 | 0.071888 | 0.073991 | — | — | — |
| tag_of_fallback.test | COMPILE | 0.071017 | 0.108111 | 0.113834 | — | — | — |
| make_pair_r-value.test | COMPILE | 0.058010 | 0.081270 | 0.108128 | — | — | — |
| quick.test | RUN | 0.190303 | 0.352826 | 0.402515 | 0.000561 | 0.001245 | 0.001248 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.011× | 1.011×, 1.010× | 0.999× |
| clang | 1.004× | 1.002×, 1.006× | 0.999× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adapt_adt.test | 10 | 0.001059s | 0.001046s | 0.001043s | byte equality observed |
| adapt_adt_empty.test | 10 | 0.001039s | 0.001031s | 0.001038s | byte equality observed |
| adapt_adt_named.test | 10 | 0.001039s | 0.001044s | 0.001050s | byte equality observed |
| adapt_adt_named_empty.test | 10 | 0.001026s | 0.001048s | 0.001041s | byte equality observed |
| adapt_assoc_adt.test | 10 | 0.001033s | 0.001042s | 0.001050s | byte equality observed |
| adapt_assoc_adt_empty.test | 10 | 0.001017s | 0.001054s | 0.001044s | byte equality observed |
| adapt_assoc_adt_named.test | 10 | 0.001037s | 0.001064s | 0.001044s | byte equality observed |
| adapt_assoc_adt_named_empty.test | 10 | 0.001031s | 0.001040s | 0.001042s | byte equality observed |
| adapt_assoc_struct.test | 10 | 0.001023s | 0.001075s | 0.001052s | byte equality observed |
| adapt_assoc_struct_empty.test | 10 | 0.001027s | 0.001050s | 0.001021s | byte equality observed |
| adapt_assoc_struct_named.test | 10 | 0.001023s | 0.001073s | 0.001049s | byte equality observed |
| adapt_assoc_struct_named_empty.test | 10 | 0.001027s | 0.001063s | 0.001026s | byte equality observed |
| adapt_assoc_tpl_adt.test | 10 | 0.001036s | 0.001062s | 0.001042s | byte equality observed |
| adapt_assoc_tpl_adt_empty.test | 10 | 0.001036s | 0.001057s | 0.001039s | byte equality observed |
| adapt_assoc_tpl_struct.test | 10 | 0.001056s | 0.001095s | 0.001063s | byte equality observed |
| adapt_assoc_tpl_struct_empty.test | 10 | 0.001042s | 0.001052s | 0.001030s | byte equality observed |
| adapt_struct.test | 10 | 0.001043s | 0.001051s | 0.001055s | byte equality observed |
| adapt_struct_empty.test | 10 | 0.001032s | 0.001051s | 0.001026s | byte equality observed |
| adapt_struct_named.test | 10 | 0.001026s | 0.001069s | 0.001056s | byte equality observed |
| adapt_struct_named_empty.test | 10 | 0.001028s | 0.001059s | 0.001056s | byte equality observed |
| adapt_tpl_adt.test | 10 | 0.001039s | 0.001036s | 0.001060s | byte equality observed |
| adapt_tpl_adt_empty.test | 10 | 0.001041s | 0.001015s | 0.001032s | byte equality observed |
| adapt_tpl_struct.test | 10 | 0.001044s | 0.001023s | 0.001046s | byte equality observed |
| adapt_tpl_struct_empty.test | 10 | 0.001042s | 0.001023s | 0.001031s | byte equality observed |
| adt_attribute_proxy.test | 10 | 0.001030s | 0.001035s | 0.001034s | byte equality observed |
| all.test | 10 | 0.001030s | 0.001037s | 0.001051s | byte equality observed |
| any.test | 10 | 0.001018s | 0.001021s | 0.001079s | byte equality observed |
| array.test | 10 | 0.001015s | 0.001027s | 0.001014s | byte equality observed |
| as_deque.test | 10 | 0.001032s | 0.001010s | 0.001025s | byte equality observed |
| as_list.test | 10 | 0.001074s | 0.001048s | 0.001066s | byte equality observed |
| as_map.test | 10 | 0.001031s | 0.001069s | 0.001036s | byte equality observed |
| as_map_assoc.test | 10 | 0.001039s | 0.001038s | 0.001027s | byte equality observed |
| as_set.test | 10 | 0.001063s | 0.001052s | 0.001057s | byte equality observed |
| as_vector.test | 10 | 0.001063s | 0.001058s | 0.001049s | byte equality observed |
| back_extended_deque.test | 10 | 0.001004s | 0.001040s | 0.001022s | byte equality observed |
| boost_array.test | 10 | 0.001007s | 0.001019s | 0.001061s | byte equality observed |
| boost_tuple.test | 10 | 0.001049s | 0.001054s | 0.001055s | byte equality observed |
| boost_tuple_iterator.test | 10 | 0.001012s | 0.001032s | 0.001043s | byte equality observed |
| clear.test | 10 | 0.001043s | 0.001047s | 0.001040s | byte equality observed |
| cons.test | 10 | 0.001092s | 0.001066s | 0.001093s | byte equality observed |
| convert_boost_tuple.test | 10 | 0.001031s | 0.001042s | 0.001031s | byte equality observed |
| convert_deque.test | 10 | 0.001034s | 0.001051s | 0.001021s | byte equality observed |
| convert_list.test | 10 | 0.001029s | 0.001035s | 0.001024s | byte equality observed |
| convert_std_pair.test | 10 | 0.001050s | 0.001017s | 0.001002s | byte equality observed |
| convert_std_tuple.test | 10 | 0.001026s | 0.001055s | 0.001031s | byte equality observed |
| convert_vector.test | 10 | 0.001060s | 0.001040s | 0.001011s | byte equality observed |
| copy.test | 10 | 0.001025s | 0.001054s | 0.001038s | byte equality observed |
| count.test | 10 | 0.001021s | 0.001034s | 0.001038s | byte equality observed |
| count_if.test | 10 | 0.001020s | 0.001030s | 0.001014s | byte equality observed |
| deduce_sequence.test | 10 | 0.001022s | 0.001058s | 0.001040s | byte equality observed |
| define_assoc_struct.test | 10 | 0.001051s | 0.001036s | 0.001047s | byte equality observed |
| define_assoc_struct_empty.test | 10 | 0.001043s | 0.001040s | 0.001068s | byte equality observed |
| define_assoc_struct_move.test | 10 | 0.001014s | 0.001020s | 0.001052s | byte equality observed |
| define_assoc_tpl_struct.test | 10 | 0.001056s | 0.001057s | 0.001048s | byte equality observed |
| define_assoc_tpl_struct_empty.test | 10 | 0.001046s | 0.001038s | 0.001038s | byte equality observed |
| define_assoc_tpl_struct_move.test | 10 | 0.001022s | 0.001023s | 0.001051s | byte equality observed |
| define_struct.test | 10 | 0.001054s | 0.001063s | 0.001063s | byte equality observed |
| define_struct_empty.test | 10 | 0.001035s | 0.001035s | 0.001073s | byte equality observed |
| define_struct_inline.test | 10 | 0.001057s | 0.001057s | 0.001063s | byte equality observed |
| define_struct_inline_empty.test | 10 | 0.001048s | 0.001058s | 0.001058s | byte equality observed |
| define_struct_inline_move.test | 10 | 0.001018s | 0.001038s | 0.001046s | byte equality observed |
| define_struct_move.test | 10 | 0.001026s | 0.001016s | 0.001019s | byte equality observed |
| define_tpl_struct.test | 10 | 0.001069s | 0.001063s | 0.001065s | byte equality observed |
| define_tpl_struct_empty.test | 10 | 0.001078s | 0.001040s | 0.001023s | byte equality observed |
| define_tpl_struct_inline.test | 10 | 0.001051s | 0.001066s | 0.001056s | byte equality observed |
| define_tpl_struct_inline_empty.test | 10 | 0.001041s | 0.001042s | 0.001022s | byte equality observed |
| define_tpl_struct_inline_move.test | 10 | 0.001018s | 0.001012s | 0.001036s | byte equality observed |
| define_tpl_struct_move.test | 10 | 0.001056s | 0.001052s | 0.001030s | byte equality observed |
| deque_comparison.test | 10 | 0.001013s | 0.001009s | 0.001039s | byte equality observed |
| deque_construction.test | 10 | 0.001023s | 0.001038s | 0.001038s | byte equality observed |
| deque_copy.test | 10 | 0.001024s | 0.001041s | 0.001059s | byte equality observed |
| deque_hash.test | 10 | 0.001024s | 0.001086s | 0.001033s | byte equality observed |
| deque_iterator.test | 10 | 0.001019s | 0.001038s | 0.001034s | byte equality observed |
| deque_make.test | 10 | 0.001027s | 0.001036s | 0.001055s | byte equality observed |
| deque_misc.test | 10 | 0.001007s | 0.001020s | 0.001038s | byte equality observed |
| deque_move.test | 10 | 0.001045s | 0.001080s | 0.001042s | byte equality observed |
| deque_mutate.test | 10 | 0.001020s | 0.001030s | 0.001043s | byte equality observed |
| deque_nest.test | 10 | 0.001028s | 0.001035s | 0.001012s | byte equality observed |
| deque_tie.test | 10 | 0.001045s | 0.001049s | 0.001016s | byte equality observed |
| deque_value_at.test | 10 | 0.001041s | 0.001038s | 0.001050s | byte equality observed |
| erase.test | 10 | 0.001060s | 0.001038s | 0.001037s | byte equality observed |
| erase_key.test | 10 | 0.001068s | 0.001072s | 0.001055s | byte equality observed |
| filter.test | 10 | 0.001042s | 0.001049s | 0.001037s | byte equality observed |
| filter_if.test | 10 | 0.001056s | 0.001106s | 0.001050s | byte equality observed |
| filter_view.test | 10 | 0.001073s | 0.001043s | 0.001050s | byte equality observed |
| find.test | 10 | 0.001050s | 0.001064s | 0.001055s | byte equality observed |
| find_if.test | 10 | 0.001083s | 0.001058s | 0.001041s | byte equality observed |
| flatten.test | 10 | 0.001065s | 0.001037s | 0.001032s | byte equality observed |
| flatten_view.test | 10 | 0.001036s | 0.001027s | 0.001049s | byte equality observed |
| fold.test | 10 | 0.001041s | 0.001073s | 0.001052s | byte equality observed |
| for_each.test | 10 | 0.001069s | 0.001083s | 0.001059s | byte equality observed |
| front_extended_deque.test | 10 | 0.001014s | 0.001022s | 0.001023s | byte equality observed |
| fused.test | 10 | 0.001031s | 0.001000s | 0.001016s | byte equality observed |
| fused_function_object.test | 10 | 0.001013s | 0.001023s | 0.001030s | byte equality observed |
| fused_procedure.test | 10 | 0.001018s | 0.001020s | 0.001033s | byte equality observed |
| github-176.test | 10 | 0.001010s | 0.001027s | 0.001068s | byte equality observed |
| hash.test | 10 | 0.001036s | 0.001048s | 0.001042s | byte equality observed |
| identity_view.test | 10 | 0.001058s | 0.001092s | 0.001056s | byte equality observed |
| insert.test | 10 | 0.001062s | 0.001095s | 0.001048s | byte equality observed |
| insert_range.test | 10 | 0.001074s | 0.001082s | 0.001065s | byte equality observed |
| invoke.test | 10 | 0.001026s | 0.001025s | 0.001018s | byte equality observed |
| invoke_function_object.test | 10 | 0.001021s | 0.001037s | 0.001018s | byte equality observed |
| invoke_procedure.test | 10 | 0.001021s | 0.001051s | 0.001039s | byte equality observed |
| io.test | 10 | 0.001174s | 0.001172s | 0.001174s | byte equality observed |
| iter_fold.test | 10 | 0.001059s | 0.001100s | 0.001051s | byte equality observed |
| iterator_range.test | 10 | 0.001083s | 0.001054s | 0.001059s | byte equality observed |
| join.test | 10 | 0.001014s | 0.001060s | 0.001013s | byte equality observed |
| joint_view.test | 10 | 0.001098s | 0.001098s | 0.001107s | byte equality observed |
| list_comparison.test | 10 | 0.001018s | 0.001026s | 0.001005s | byte equality observed |
| list_construction.test | 10 | 0.001027s | 0.001054s | 0.001035s | byte equality observed |
| list_copy.test | 10 | 0.001025s | 0.001060s | 0.001072s | byte equality observed |
| list_hash.test | 10 | 0.001028s | 0.001065s | 0.001034s | byte equality observed |
| list_iterator.test | 10 | 0.001004s | 0.001047s | 0.001033s | byte equality observed |
| list_make.test | 10 | 0.001025s | 0.001039s | 0.001023s | byte equality observed |
| list_misc.test | 10 | 0.001028s | 0.001013s | 0.001020s | byte equality observed |
| list_mutate.test | 10 | 0.001014s | 0.001038s | 0.001075s | byte equality observed |
| list_nest.test | 10 | 0.001010s | 0.001018s | 0.001011s | byte equality observed |
| list_tie.test | 10 | 0.001017s | 0.001034s | 0.001023s | byte equality observed |
| list_value_at.test | 10 | 0.001021s | 0.001028s | 0.001033s | byte equality observed |
| make_fused.test | 10 | 0.001024s | 0.001002s | 0.001033s | byte equality observed |
| make_fused_function_object.test | 10 | 0.001032s | 0.001063s | 0.001084s | byte equality observed |
| make_fused_procedure.test | 10 | 0.001007s | 0.001025s | 0.001037s | byte equality observed |
| make_list.test | 10 | 0.001030s | 0.001034s | 0.001035s | byte equality observed |
| make_unfused.test | 10 | 0.001020s | 0.001030s | 0.001028s | byte equality observed |
| make_vector.test | 10 | 0.001023s | 0.001031s | 0.001032s | byte equality observed |
| map.test | 10 | 0.001049s | 0.001047s | 0.001079s | byte equality observed |
| map_comparison.test | 10 | 0.001028s | 0.001035s | 0.001043s | byte equality observed |
| map_construction.test | 10 | 0.001018s | 0.001015s | 0.001032s | byte equality observed |
| map_copy.test | 10 | 0.001047s | 0.001026s | 0.001026s | byte equality observed |
| map_misc.test | 10 | 0.001005s | 0.001005s | 0.001048s | byte equality observed |
| map_move.test | 10 | 0.001049s | 0.001043s | 0.001058s | byte equality observed |
| map_mutate.test | 10 | 0.001013s | 0.001005s | 0.001057s | byte equality observed |
| map_tie.test | 10 | 0.001006s | 0.000995s | 0.001008s | byte equality observed |
| move.test | 10 | 0.001018s | 0.001008s | 0.001023s | byte equality observed |
| nil.test | 10 | 0.001004s | 0.001089s | 0.001050s | byte equality observed |
| none.test | 10 | 0.001043s | 0.001032s | 0.001021s | byte equality observed |
| nview.test | 10 | 0.001016s | 0.001068s | 0.001028s | byte equality observed |
| pop_back.test | 10 | 0.001064s | 0.001067s | 0.001057s | byte equality observed |
| pop_front.test | 10 | 0.001057s | 0.001086s | 0.001079s | byte equality observed |
| push_back.test | 10 | 0.001071s | 0.001091s | 0.001061s | byte equality observed |
| push_front.test | 10 | 0.001102s | 0.001058s | 0.001081s | byte equality observed |
| quick.test | 10 | 0.000475s | 0.001024s | 0.000997s | byte equality observed |
| ref_vector.test | 10 | 0.001042s | 0.001026s | 0.001062s | byte equality observed |
| remove.test | 10 | 0.001075s | 0.001063s | 0.001072s | byte equality observed |
| remove_if.test | 10 | 0.001064s | 0.001059s | 0.001060s | byte equality observed |
| repetitive_view.test | 10 | 0.001023s | 0.001027s | 0.001037s | byte equality observed |
| replace.test | 10 | 0.001031s | 0.001041s | 0.001067s | byte equality observed |
| replace_if.test | 10 | 0.001059s | 0.001085s | 0.001078s | byte equality observed |
| reverse.test | 10 | 0.001052s | 0.001044s | 0.001080s | byte equality observed |
| reverse_fold.test | 10 | 0.001037s | 0.001039s | 0.001083s | byte equality observed |
| reverse_iter_fold.test | 10 | 0.001054s | 0.001026s | 0.001068s | byte equality observed |
| reverse_view.test | 10 | 0.001064s | 0.001084s | 0.001072s | byte equality observed |
| segmented_find.test | 10 | 0.001035s | 0.001014s | 0.000998s | byte equality observed |
| segmented_find_if.test | 10 | 0.001025s | 0.001020s | 0.001019s | byte equality observed |
| segmented_fold.test | 10 | 0.001070s | 0.001042s | 0.001044s | byte equality observed |
| segmented_for_each.test | 10 | 0.001052s | 0.001040s | 0.001028s | byte equality observed |
| segmented_iterator_range.test | 10 | 0.001077s | 0.001042s | 0.001050s | byte equality observed |
| set.test | 10 | 0.001046s | 0.001062s | 0.001049s | byte equality observed |
| single_view.test | 10 | 0.001055s | 0.001060s | 0.001051s | byte equality observed |
| std_array.test | 10 | 0.001021s | 0.001011s | 0.001030s | byte equality observed |
| std_pair.test | 10 | 0.001050s | 0.001053s | 0.001039s | byte equality observed |
| std_tuple.test | 10 | 0.001017s | 0.001014s | 0.001012s | byte equality observed |
| std_tuple_iterator.test | 10 | 0.001034s | 0.001023s | 0.001024s | byte equality observed |
| swap.test | 10 | 0.001045s | 0.001021s | 0.001031s | byte equality observed |
| transform.test | 10 | 0.001054s | 0.001057s | 0.001064s | byte equality observed |
| transform_view.test | 10 | 0.001064s | 0.001086s | 0.001106s | byte equality observed |
| tuple_comparison.test | 10 | 0.001027s | 0.001006s | 0.001045s | byte equality observed |
| tuple_construction.test | 10 | 0.001005s | 0.000998s | 0.001032s | byte equality observed |
| tuple_conversion.test | 10 | 0.001023s | 0.001021s | 0.001020s | byte equality observed |
| tuple_copy.test | 10 | 0.001019s | 0.001004s | 0.001024s | byte equality observed |
| tuple_element.test | 10 | 0.001000s | 0.001012s | 0.001007s | byte equality observed |
| tuple_hash.test | 10 | 0.001047s | 0.001043s | 0.001031s | byte equality observed |
| tuple_make.test | 10 | 0.001023s | 0.001024s | 0.000996s | byte equality observed |
| tuple_misc.test | 10 | 0.001042s | 0.001023s | 0.001036s | byte equality observed |
| tuple_mutate.test | 10 | 0.001017s | 0.001011s | 0.001051s | byte equality observed |
| tuple_nest.test | 10 | 0.001026s | 0.001048s | 0.001040s | byte equality observed |
| tuple_tie.test | 10 | 0.001013s | 0.001023s | 0.001044s | byte equality observed |
| tuple_traits__maybe_variadic.test | 10 | 0.001015s | 0.001030s | 0.001019s | byte equality observed |
| tuple_traits__no_variadic.test | 10 | 0.001035s | 0.001032s | 0.001049s | byte equality observed |
| unfused.test | 10 | 0.001048s | 0.001040s | 0.001034s | byte equality observed |
| unfused_typed.test | 10 | 0.001028s | 0.001007s | 0.001071s | byte equality observed |
| vector_comparison.test | 10 | 0.001025s | 0.001033s | 0.001043s | byte equality observed |
| vector_construction.test | 10 | 0.001020s | 0.001016s | 0.001032s | byte equality observed |
| vector_conversion.test | 10 | 0.001034s | 0.001010s | 0.001019s | byte equality observed |
| vector_copy.test | 10 | 0.001018s | 0.001036s | 0.001037s | byte equality observed |
| vector_hash.test | 10 | 0.001052s | 0.001040s | 0.001081s | byte equality observed |
| vector_iterator.test | 10 | 0.000999s | 0.001041s | 0.001029s | byte equality observed |
| vector_make.test | 10 | 0.001051s | 0.001020s | 0.001025s | byte equality observed |
| vector_misc.test | 10 | 0.001040s | 0.001032s | 0.001028s | byte equality observed |
| vector_move.test | 10 | 0.001054s | 0.001039s | 0.001044s | byte equality observed |
| vector_mutate.test | 10 | 0.001019s | 0.001024s | 0.001008s | byte equality observed |
| vector_n.test | 10 | 0.001064s | 0.001040s | 0.001063s | byte equality observed |
| vector_nest.test | 10 | 0.001016s | 0.001014s | 0.001028s | byte equality observed |
| vector_tie.test | 10 | 0.001000s | 0.001041s | 0.001015s | byte equality observed |
| vector_traits__maybe_variadic.test | 10 | 0.001033s | 0.001025s | 0.001059s | byte equality observed |
| vector_traits__no_variadic.test | 10 | 0.001013s | 0.001059s | 0.001023s | byte equality observed |
| vector_value_at.test | 10 | 0.001028s | 0.001046s | 0.001035s | byte equality observed |
| zip.test | 10 | 0.001033s | 0.001046s | 0.001017s | byte equality observed |
| zip2.test | 10 | 0.001023s | 0.001039s | 0.001019s | byte equality observed |
| zip_ignore.test | 10 | 0.001027s | 0.001006s | 0.001028s | byte equality observed |
| zip_view.test | 10 | 0.001023s | 0.001015s | 0.001018s | byte equality observed |
| zip_view2.test | 10 | 0.001045s | 0.001010s | 0.001030s | byte equality observed |
| zip_view_ignore.test | 10 | 0.001011s | 0.001012s | 0.001015s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
