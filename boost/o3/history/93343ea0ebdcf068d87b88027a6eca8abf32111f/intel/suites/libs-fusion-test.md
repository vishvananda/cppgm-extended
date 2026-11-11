# libs/fusion/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| all.test | RUN | 0.450726 | 0.972531 | 1.331144 | 0.001482 | 0.002006 | 0.001552 |
| any.test | RUN | 0.466767 | 0.922598 | 1.252440 | 0.001434 | 0.002289 | 0.001980 |
| clear.test | RUN | 0.320207 | 0.834426 | 1.023240 | 0.001716 | 0.001478 | 0.001780 |
| copy.test | RUN | 0.282168 | 0.641771 | 0.841892 | 0.001794 | 0.002302 | 0.002152 |
| count.test | RUN | 0.305154 | 0.698523 | 1.056823 | 0.001822 | 0.001794 | 0.001602 |
| count_if.test | RUN | 0.294436 | 0.683047 | 0.843067 | 0.001786 | 0.001678 | 0.001682 |
| erase.test | RUN | 0.412224 | 1.140475 | 1.478967 | 0.001689 | 0.002548 | 0.002231 |
| erase_key.test | RUN | 0.406835 | 1.159070 | 1.318279 | 0.001740 | 0.002094 | 0.001565 |
| filter.test | RUN | 0.357751 | 0.955853 | 1.209485 | 0.001812 | 0.001994 | 0.001777 |
| filter_if.test | RUN | 0.404135 | 1.189632 | 1.342153 | 0.001500 | 0.001934 | 0.001644 |
| find.test | RUN | 0.341171 | 0.792612 | 1.030022 | 0.001506 | 0.001822 | 0.001590 |
| find_if.test | RUN | 0.330557 | 0.710675 | 0.940965 | 0.001776 | 0.002182 | 0.002311 |
| fold.test | RUN | 0.365044 | 0.980972 | 1.215951 | 0.001597 | 0.001992 | 0.002320 |
| for_each.test | RUN | 0.327666 | 0.834472 | 1.054578 | 0.001772 | 0.002087 | 0.001659 |
| insert.test | RUN | 0.510272 | 1.490413 | 1.677580 | 0.001668 | 0.002150 | 0.003175 |
| insert_range.test | RUN | 0.491684 | 1.512825 | 1.692447 | 0.001728 | 0.002164 | 0.001982 |
| iter_fold.test | RUN | 0.573262 | 1.352971 | 1.944489 | 0.001469 | 0.002028 | 0.001478 |
| move.test | RUN | 0.293487 | 0.648701 | 0.868188 | 0.001732 | 0.002541 | 0.003027 |
| none.test | RUN | 0.446559 | 0.918278 | 1.288650 | 0.001622 | 0.002190 | 0.001894 |
| pop_back.test | RUN | 0.444147 | 1.241302 | 1.515930 | 0.001942 | 0.002263 | 0.002076 |
| pop_front.test | RUN | 0.366235 | 1.016002 | 1.184577 | 0.001603 | 0.002050 | 0.002709 |
| push_back.test | RUN | 0.469384 | 1.440354 | 1.633836 | 0.001718 | 0.002385 | 0.002200 |
| push_front.test | RUN | 0.431366 | 1.152726 | 1.393190 | 0.001648 | 0.002009 | 0.002068 |
| remove.test | RUN | 0.459906 | 1.448376 | 1.678139 | 0.001728 | 0.001736 | 0.003012 |
| remove_if.test | RUN | 0.399731 | 1.148325 | 1.339906 | 0.001677 | 0.001680 | 0.001748 |
| replace.test | RUN | 0.393371 | 1.180525 | 1.287719 | 0.001696 | 0.001872 | 0.001671 |
| replace_if.test | RUN | 0.395207 | 1.097101 | 1.245531 | 0.001928 | 0.001994 | 0.001500 |
| reverse_fold.test | RUN | 0.626966 | 1.517655 | 2.238495 | 0.002366 | 0.002196 | 0.001796 |
| reverse_iter_fold.test | RUN | 0.649929 | 1.785782 | 2.305660 | 0.001912 | 0.002025 | 0.002030 |
| reverse.test | RUN | 0.393730 | 1.164352 | 1.288018 | 0.001746 | 0.001824 | 0.001720 |
| segmented_for_each.test | RUN | 0.691617 | 2.204456 | 2.608553 | 0.002081 | 0.001790 | 0.001526 |
| segmented_find.test | RUN | 0.435139 | 1.156847 | 1.443706 | 0.001758 | 0.001986 | 0.002986 |
| segmented_find_if.test | RUN | 0.435318 | 1.110639 | 1.549062 | 0.001902 | 0.002148 | 0.002244 |
| segmented_fold.test | RUN | 0.532295 | 1.527690 | 1.892635 | 0.001692 | 0.001780 | 0.001739 |
| transform.test | RUN | 0.413497 | 1.265999 | 1.395595 | 0.001801 | 0.002148 | 0.001814 |
| join.test | RUN | 0.308266 | 0.730324 | 0.942031 | 0.002070 | 0.001704 | 0.002252 |
| zip.test | RUN | 0.538237 | 1.446363 | 1.805826 | 0.001469 | 0.001631 | 0.002008 |
| zip2.test | RUN | 0.526262 | 1.295426 | 1.746432 | 0.001509 | 0.002096 | 0.001942 |
| zip_ignore.test | RUN | 0.418764 | 0.870111 | 1.193595 | 0.001928 | 0.002434 | 0.002574 |
| flatten.test | RUN | 0.383911 | 1.001915 | 1.204522 | 0.001792 | 0.001954 | 0.001482 |
| ticket-5490.test | COMPILE | 0.211238 | 0.418449 | 0.510373 | — | — | — |
| as_deque.test | RUN | 0.377273 | 0.957839 | 1.244916 | 0.002032 | 0.002016 | 0.001870 |
| as_list.test | RUN | 0.435181 | 1.183967 | 1.358263 | 0.001850 | 0.002212 | 0.001962 |
| as_map.test | RUN | 0.436789 | 1.295009 | 1.350049 | 0.001671 | 0.001958 | 0.001758 |
| as_map_assoc.test | RUN | 0.419870 | 1.047773 | 1.265462 | 0.001687 | 0.001972 | 0.001891 |
| as_set.test | RUN | 0.471846 | 1.351494 | 1.666828 | 0.001767 | 0.002040 | 0.001746 |
| as_vector.test | RUN | 0.440180 | 1.205800 | 1.375277 | 0.001938 | 0.002686 | 0.001912 |
| boost_tuple.test | RUN | 0.440043 | 1.167848 | 1.394786 | 0.001712 | 0.001962 | 0.001949 |
| boost_tuple_iterator.test | RUN | 0.307394 | 0.669400 | 0.836717 | 0.002187 | 0.001780 | 0.002232 |
| cons.test | RUN | 0.566558 | 1.311204 | 1.678127 | 0.001710 | 0.002887 | 0.002025 |
| convert_boost_tuple.test | RUN | 0.351640 | 1.018242 | 1.096023 | 0.002995 | 0.002498 | 0.001594 |
| convert_deque.test | RUN | 0.356901 | 1.017229 | 1.108524 | 0.001606 | 0.002071 | 0.002314 |
| convert_list.test | RUN | 0.363065 | 1.097240 | 1.115660 | 0.001786 | 0.001846 | 0.001686 |
| convert_std_pair.test | RUN | 0.375292 | 1.026260 | 1.132961 | 0.001932 | 0.002023 | 0.001845 |
| convert_std_tuple.test | RUN | 0.358134 | 0.964736 | 1.086158 | 0.001685 | 0.001433 | 0.001688 |
| convert_vector.test | RUN | 0.360209 | 0.991302 | 1.056618 | 0.001786 | 0.002052 | 0.001539 |
| filter_view.test | RUN | 0.403050 | 1.048882 | 1.245463 | 0.001977 | 0.002014 | 0.002224 |
| hash.test | RUN | 0.405204 | 1.138851 | 1.218350 | 0.001587 | 0.001882 | 0.001935 |
| io.test | RUN | 0.432155 | 1.335825 | 1.494760 | 0.001946 | 0.003172 | 0.002609 |
| iterator_range.test | RUN | 0.406260 | 0.993008 | 1.205710 | 0.001749 | 0.001766 | 0.001784 |
| joint_view.test | RUN | 0.518694 | 1.822928 | 1.775067 | 0.001730 | 0.002128 | 0.001559 |
| list_comparison.test | RUN | 0.301770 | 0.703078 | 0.930161 | 0.002016 | 0.001832 | 0.001914 |
| list_construction.test | RUN | 0.295749 | 0.656411 | 0.874924 | 0.001701 | 0.001708 | 0.003330 |
| list_copy.test | RUN | 0.461948 | 1.270879 | 1.441912 | 0.001750 | 0.001772 | 0.001541 |
| list_iterator.test | RUN | 0.299979 | 0.676133 | 0.831181 | 0.002018 | 0.001689 | 0.002449 |
| list_hash.test | RUN | 0.415215 | 1.671092 | 1.269290 | 0.001744 | 0.001828 | 0.001955 |
| list_make.test | RUN | 0.315406 | 0.774265 | 0.941368 | 0.001664 | 0.001866 | 0.001734 |
| list_misc.test | RUN | 0.390908 | 0.895465 | 1.172432 | 0.001832 | 0.001681 | 0.001598 |
| list_mutate.test | RUN | 0.276946 | 0.616453 | 0.809956 | 0.001772 | 0.002394 | 0.001850 |
| list_nest.test | RUN | 0.585016 | 1.823912 | 2.162530 | 0.001712 | 0.002192 | 0.001753 |
| list_tie.test | RUN | 0.304523 | 0.701266 | 0.871838 | 0.001695 | 0.001712 | 0.001570 |
| list_value_at.test | RUN | 0.281700 | 0.657472 | 0.806459 | 0.001666 | 0.002040 | 0.001898 |
| deque_comparison.test | RUN | 0.301137 | 0.740599 | 0.914980 | 0.001512 | 0.002032 | 0.001927 |
| deque_construction.test | RUN | 0.296364 | 0.776967 | 0.862042 | 0.001608 | 0.002077 | 0.002845 |
| deque_copy.test | RUN | 0.454925 | 1.182509 | 1.383263 | 0.001660 | 0.001694 | 0.002913 |
| deque_iterator.test | RUN | 0.306272 | 0.764435 | 0.923757 | 0.001973 | 0.002225 | 0.002281 |
| deque_hash.test | RUN | 0.399346 | 1.655551 | 1.270145 | 0.001606 | 0.001683 | 0.001692 |
| deque_is_constructible.test | COMPILE | 0.168582 | 0.354364 | 0.409136 | — | — | — |
| deque_make.test | RUN | 0.311380 | 0.730930 | 0.897183 | 0.002077 | 0.002202 | 0.001547 |
| deque_misc.test | RUN | 0.390988 | 0.923236 | 1.111215 | 0.001706 | 0.002032 | 0.002364 |
| deque_move.test | RUN | 0.312367 | 0.774586 | 0.887272 | 0.001872 | 0.002064 | 0.001644 |
| deque_mutate.test | RUN | 0.280096 | 0.595548 | 0.800033 | 0.001906 | 0.001625 | 0.002278 |
| deque_nest.test | RUN | 0.853577 | 2.826091 | 3.352260 | 0.002028 | 0.001738 | 0.001454 |
| deque_tie.test | RUN | 0.286930 | 0.655784 | 0.857926 | 0.002330 | 0.002244 | 0.001642 |
| deque_value_at.test | RUN | 0.292686 | 0.624738 | 0.820405 | 0.001659 | 0.001728 | 0.001858 |
| front_extended_deque.test | RUN | 0.396620 | 0.966392 | 1.181840 | 0.002053 | 0.002078 | 0.001784 |
| back_extended_deque.test | RUN | 0.407308 | 0.952999 | 1.170929 | 0.001704 | 0.002012 | 0.001876 |
| make_list.test | RUN | 0.313231 | 0.758730 | 0.933378 | 0.001671 | 0.001950 | 0.002337 |
| make_vector.test | RUN | 0.316894 | 0.816513 | 0.932540 | 0.001434 | 0.001402 | 0.001966 |
| map.test | RUN | 0.397988 | 1.221057 | 1.328519 | 0.001900 | 0.001640 | 0.001716 |
| map_comparison.test | RUN | 0.311175 | 0.718312 | 0.921605 | 0.001694 | 0.001948 | 0.001990 |
| map_construction.test | RUN | 0.332908 | 0.694301 | 0.870557 | 0.001717 | 0.003368 | 0.002038 |
| map_copy.test | RUN | 0.361062 | 0.902317 | 1.133920 | 0.001977 | 0.001765 | 0.001890 |
| map_misc.test | RUN | 0.385899 | 0.882918 | 1.086486 | 0.001636 | 0.001396 | 0.002659 |
| map_move.test | RUN | 0.315188 | 0.747903 | 0.884746 | 0.001419 | 0.001948 | 0.001710 |
| map_mutate.test | RUN | 0.279716 | 0.618105 | 0.804838 | 0.001876 | 0.002541 | 0.002952 |
| map_tie.test | RUN | 0.294622 | 0.645554 | 0.808183 | 0.001698 | 0.001786 | 0.001399 |
| nil.test | RUN | 0.205480 | 0.472646 | 0.571225 | 0.001603 | 0.002004 | 0.001811 |
| nview.test | RUN | 0.382856 | 0.898569 | 1.129495 | 0.002300 | 0.003007 | 0.002540 |
| reverse_view.test | RUN | 0.484455 | 1.469038 | 1.676577 | 0.001850 | 0.001770 | 0.001874 |
| segmented_iterator_range.test | RUN | 0.576414 | 1.841407 | 2.100896 | 0.001699 | 0.002389 | 0.001800 |
| set.test | RUN | 0.382464 | 1.086471 | 1.184272 | 0.001877 | 0.002372 | 0.001788 |
| single_view.test | RUN | 0.333208 | 0.884547 | 1.054954 | 0.001888 | 0.002027 | 0.002562 |
| std_pair.test | RUN | 0.438352 | 1.268940 | 1.320624 | 0.001842 | 0.001782 | 0.001906 |
| boost_array.test | RUN | 0.371476 | 0.790506 | 0.996003 | 0.002072 | 0.001740 | 0.001938 |
| array.test | RUN | 0.364476 | 0.793554 | 1.018693 | 0.001750 | 0.001742 | 0.001834 |
| std_array.test | RUN | 0.364889 | 0.792855 | 0.986552 | 0.001769 | 0.002373 | 0.001676 |
| tuple_comparison.test | RUN | 0.322673 | 0.755281 | 0.985227 | 0.001459 | 0.002309 | 0.002144 |
| tuple_construction.test | RUN | 0.335199 | 0.772895 | 0.989004 | 0.001432 | 0.002078 | 0.001698 |
| tuple_conversion.test | RUN | 0.774331 | 2.574760 | 3.114662 | 0.001966 | 0.001728 | 0.001716 |
| tuple_copy.test | RUN | 0.485793 | 1.478769 | 1.630547 | 0.002157 | 0.002029 | 0.002162 |
| tuple_element.test | RUN | 0.317091 | 0.659440 | 0.860626 | 0.001767 | 0.001896 | 0.001882 |
| tuple_make.test | RUN | 0.332682 | 0.785933 | 0.947946 | 0.001770 | 0.001685 | 0.002486 |
| tuple_misc.test | RUN | 0.428355 | 0.950860 | 1.218382 | 0.001928 | 0.001864 | 0.001892 |
| tuple_mutate.test | RUN | 0.299264 | 0.638789 | 0.865723 | 0.001774 | 0.001714 | 0.002234 |
| tuple_nest.test | RUN | 1.307254 | 4.881762 | 5.612911 | 0.001984 | 0.002097 | 0.002244 |
| tuple_hash.test | RUN | 0.426266 | 1.668589 | 1.275604 | 0.001723 | 0.001726 | 0.001632 |
| tuple_tie.test | RUN | 0.344130 | 0.698170 | 0.917854 | 0.001856 | 0.001645 | 0.002206 |
| tuple_traits__maybe_variadic.test | RUN | 0.342940 | 0.870847 | 0.988220 | 0.001830 | 0.001714 | 0.002407 |
| tuple_traits__no_variadic.test | RUN | 0.376600 | 0.947751 | 1.034743 | 0.001384 | 0.003015 | 0.001526 |
| transform_view.test | RUN | 0.515211 | 1.489065 | 1.668692 | 0.001479 | 0.001829 | 0.002569 |
| identity_view.test | RUN | 0.479256 | 1.341673 | 1.599579 | 0.001844 | 0.002052 | 0.001922 |
| vector_comparison.test | RUN | 0.309064 | 0.734326 | 0.925563 | 0.001345 | 0.001814 | 0.001836 |
| vector_construction.test | RUN | 0.310757 | 0.676696 | 0.852229 | 0.001411 | 0.001728 | 0.002372 |
| vector_conversion.test | RUN | 0.922254 | 3.131091 | 3.977716 | 0.001667 | 0.001766 | 0.001722 |
| vector_copy.test | RUN | 0.489567 | 1.397560 | 1.658141 | 0.001606 | 0.001690 | 0.002082 |
| vector_iterator.test | RUN | 0.304821 | 0.719407 | 0.852165 | 0.001434 | 0.001982 | 0.002222 |
| vector_make.test | RUN | 0.324788 | 0.784650 | 0.895130 | 0.001774 | 0.001698 | 0.001908 |
| vector_misc.test | RUN | 0.403186 | 0.930084 | 1.114645 | 0.002395 | 0.001712 | 0.001954 |
| vector_move.test | RUN | 0.317158 | 0.757800 | 0.878618 | 0.001678 | 0.001657 | 0.002220 |
| vector_mutate.test | RUN | 0.284167 | 0.598767 | 0.765674 | 0.001742 | 0.001638 | 0.001659 |
| vector_n.test | RUN | 0.346719 | 0.774231 | 1.054684 | 0.002040 | 0.002620 | 0.001768 |
| vector_nest.test | RUN | 1.307060 | 5.149304 | 5.663321 | 0.001784 | 0.001886 | 0.001504 |
| vector_hash.test | RUN | 0.389161 | 1.711620 | 1.205107 | 0.001669 | 0.001819 | 0.002242 |
| vector_tie.test | RUN | 0.292652 | 0.656812 | 0.893229 | 0.001794 | 0.001790 | 0.002253 |
| vector_traits__maybe_variadic.test | RUN | 0.303072 | 0.775144 | 0.908636 | 0.001890 | 0.001915 | 0.001639 |
| vector_traits__no_variadic.test | RUN | 0.343774 | 0.813880 | 0.915871 | 0.001847 | 0.001732 | 0.001657 |
| vector_value_at.test | RUN | 0.295234 | 0.605694 | 0.807421 | 0.001602 | 0.001920 | 0.003030 |
| zip_view.test | RUN | 0.587762 | 1.537570 | 2.046098 | 0.002067 | 0.001782 | 0.002075 |
| zip_view2.test | RUN | 0.479187 | 1.169354 | 1.512024 | 0.002052 | 0.002293 | 0.001840 |
| zip_view_ignore.test | RUN | 0.443015 | 1.027560 | 1.352377 | 0.001860 | 0.001936 | 0.002131 |
| repetitive_view.test | RUN | 0.767065 | 2.006607 | 2.653203 | 0.002036 | 0.002009 | 0.002226 |
| deduce_sequence.test | RUN | 0.394501 | 0.817017 | 1.032710 | 0.001942 | 0.001913 | 0.002054 |
| adapt_adt_named.test | RUN | 0.438308 | 1.130187 | 1.283867 | 0.001811 | 0.002077 | 0.001684 |
| adapt_adt_named_empty.test | RUN | 0.389427 | 0.927766 | 1.112287 | 0.001940 | 0.001917 | 0.001441 |
| adapt_adt.test | RUN | 0.478084 | 1.554725 | 1.620576 | 0.002054 | 0.002230 | 0.001524 |
| adapt_adt_empty.test | RUN | 0.366444 | 0.918804 | 1.100478 | 0.002033 | 0.002281 | 0.001968 |
| adapt_assoc_adt_named.test | RUN | 0.476923 | 1.320849 | 1.453796 | 0.001734 | 0.001760 | 0.001923 |
| adapt_assoc_adt_named_empty.test | RUN | 0.425829 | 1.033010 | 1.080593 | 0.001768 | 0.002226 | 0.002212 |
| adapt_assoc_adt.test | RUN | 0.488259 | 1.234751 | 1.468532 | 0.001926 | 0.001749 | 0.002612 |
| adapt_assoc_adt_empty.test | RUN | 0.367490 | 0.927557 | 1.144640 | 0.001760 | 0.001667 | 0.001928 |
| adapt_assoc_struct_named.test | RUN | 0.462835 | 1.182802 | 1.443137 | 0.001695 | 0.001895 | 0.001509 |
| adapt_assoc_struct_named_empty.test | RUN | 0.372253 | 1.099968 | 1.083809 | 0.001740 | 0.002044 | 0.001814 |
| adapt_assoc_struct.test | RUN | 0.497683 | 1.133132 | 1.291846 | 0.001732 | 0.001680 | 0.001910 |
| adapt_assoc_struct_empty.test | RUN | 0.484507 | 0.916206 | 1.106028 | 0.001895 | 0.002049 | 0.001915 |
| adapt_assoc_tpl_adt.test | RUN | 0.498450 | 1.272826 | 1.486773 | 0.001706 | 0.002023 | 0.001726 |
| adapt_assoc_tpl_adt_empty.test | RUN | 0.371740 | 0.916689 | 1.073700 | 0.002096 | 0.001960 | 0.001816 |
| adapt_assoc_tpl_struct.test | RUN | 0.401986 | 1.143186 | 1.253899 | 0.001462 | 0.002068 | 0.001669 |
| adapt_assoc_tpl_struct_empty.test | RUN | 0.370089 | 0.919847 | 1.083382 | 0.001676 | 0.002096 | 0.001870 |
| adapt_struct_named.test | RUN | 0.412795 | 1.173234 | 1.306056 | 0.001873 | 0.002168 | 0.002116 |
| adapt_struct_named_empty.test | RUN | 0.423898 | 0.918589 | 1.069889 | 0.001822 | 0.001934 | 0.001832 |
| adapt_struct.test | RUN | 0.460244 | 1.282122 | 1.476643 | 0.001752 | 0.002122 | 0.002098 |
| adapt_struct_empty.test | RUN | 0.364204 | 0.948928 | 1.095201 | 0.001742 | 0.001968 | 0.001691 |
| adapt_tpl_adt.test | RUN | 0.470845 | 1.501277 | 1.491006 | 0.001838 | 0.001970 | 0.002116 |
| adapt_tpl_adt_empty.test | RUN | 0.371380 | 0.923086 | 1.103499 | 0.001992 | 0.001988 | 0.002052 |
| adapt_tpl_struct.test | RUN | 0.392945 | 1.037149 | 1.178019 | 0.001749 | 0.001727 | 0.001796 |
| adapt_tpl_struct_empty.test | RUN | 0.376376 | 0.937016 | 1.129697 | 0.001734 | 0.001712 | 0.001896 |
| adt_attribute_proxy.test | RUN | 0.423880 | 0.897165 | 1.127530 | 0.001998 | 0.002210 | 0.001488 |
| define_struct.test | RUN | 0.498374 | 1.276064 | 1.502597 | 0.001792 | 0.001776 | 0.002142 |
| define_struct_empty.test | RUN | 0.486063 | 1.035926 | 1.271491 | 0.001660 | 0.001742 | 0.001908 |
| define_struct_move.test | RUN | 0.294340 | 0.598375 | 0.752014 | 0.001480 | 0.001554 | 0.001830 |
| define_struct_inline.test | RUN | 0.530110 | 1.570219 | 1.671844 | 0.001906 | 0.001986 | 0.002126 |
| define_struct_inline_empty.test | RUN | 0.443786 | 1.049662 | 1.241425 | 0.001798 | 0.002124 | 0.001852 |
| define_struct_inline_move.test | RUN | 0.303515 | 0.585758 | 0.746614 | 0.001633 | 0.002445 | 0.002102 |
| define_assoc_struct.test | RUN | 0.541527 | 1.217213 | 1.472210 | 0.001814 | 0.001744 | 0.001330 |
| define_assoc_struct_empty.test | RUN | 0.444416 | 1.071110 | 1.244905 | 0.001792 | 0.002080 | 0.002048 |
| define_assoc_struct_move.test | RUN | 0.292701 | 0.606761 | 0.748154 | 0.001760 | 0.001737 | 0.001738 |
| define_tpl_struct.test | RUN | 0.482359 | 1.211505 | 1.458114 | 0.001537 | 0.001986 | 0.001618 |
| define_tpl_struct_empty.test | RUN | 0.447857 | 1.065082 | 1.262576 | 0.001773 | 0.002092 | 0.001488 |
| define_tpl_struct_move.test | RUN | 0.297358 | 0.587506 | 0.766390 | 0.001796 | 0.001764 | 0.002260 |
| define_tpl_struct_inline.test | RUN | 0.514560 | 1.433069 | 1.563440 | 0.001728 | 0.001764 | 0.001787 |
| define_tpl_struct_inline_empty.test | RUN | 0.455654 | 1.077010 | 1.236253 | 0.001808 | 0.001618 | 0.001948 |
| define_tpl_struct_inline_move.test | RUN | 0.303047 | 0.783345 | 0.759020 | 0.001746 | 0.002235 | 0.001738 |
| define_assoc_tpl_struct.test | RUN | 0.491446 | 1.250474 | 1.489822 | 0.001742 | 0.002072 | 0.001546 |
| define_assoc_tpl_struct_empty.test | RUN | 0.439258 | 1.077270 | 1.282830 | 0.002022 | 0.002352 | 0.001692 |
| define_assoc_tpl_struct_move.test | RUN | 0.293145 | 0.606804 | 0.760857 | 0.002062 | 0.002092 | 0.001870 |
| std_tuple.test | RUN | 0.296821 | 0.673867 | 0.851163 | 0.001890 | 0.002412 | 0.002625 |
| std_tuple_iterator.test | RUN | 0.301874 | 0.725740 | 0.922832 | 0.001705 | 0.001689 | 0.002104 |
| ref_vector.test | RUN | 0.340759 | 0.736521 | 0.931286 | 0.001780 | 0.001809 | 0.001802 |
| flatten_view.test | RUN | 0.367778 | 0.999380 | 1.129840 | 0.001707 | 0.002000 | 0.001631 |
| github-159.test | COMPILE | 0.168997 | 0.326103 | 0.400013 | — | — | — |
| github-176.test | RUN | 0.415828 | 0.815202 | 0.922275 | 0.002164 | 0.002220 | 0.001652 |
| size.test | COMPILE | 0.283832 | 0.567190 | 0.738371 | — | — | — |
| fused.test | RUN | 0.407431 | 0.841490 | 1.126405 | 0.002080 | 0.001696 | 0.001635 |
| fused_function_object.test | RUN | 0.302306 | 0.634088 | 0.824524 | 0.002054 | 0.001732 | 0.001680 |
| fused_procedure.test | RUN | 0.375674 | 0.785371 | 1.032182 | 0.001868 | 0.001791 | 0.001583 |
| unfused.test | RUN | 0.295948 | 0.658742 | 0.903684 | 0.001720 | 0.001447 | 0.002080 |
| unfused_typed.test | RUN | 0.307914 | 0.771385 | 0.907320 | 0.001762 | 0.001689 | 0.001854 |
| make_fused.test | RUN | 0.388929 | 0.829415 | 1.105756 | 0.001807 | 0.001680 | 0.001877 |
| make_fused_function_object.test | RUN | 0.299030 | 0.686267 | 0.838028 | 0.001573 | 0.001646 | 0.001726 |
| make_fused_procedure.test | RUN | 0.386331 | 0.825518 | 1.072111 | 0.001757 | 0.001646 | 0.001606 |
| make_unfused.test | RUN | 0.303259 | 0.644563 | 0.841574 | 0.001653 | 0.001761 | 0.001754 |
| invoke.test | RUN | 0.759856 | 2.389983 | 2.751804 | 0.002087 | 0.001848 | 0.002139 |
| invoke_function_object.test | RUN | 0.387264 | 0.846761 | 1.117728 | 0.002104 | 0.002144 | 0.002068 |
| invoke_procedure.test | RUN | 0.608114 | 1.735207 | 2.120965 | 0.002164 | 0.002236 | 0.002581 |
| swap.test | RUN | 0.464196 | 1.057612 | 1.331473 | 0.002728 | 0.002193 | 0.002058 |
| is_sequence.test | COMPILE | 0.083170 | 0.164351 | 0.163517 | — | — | — |
| is_view.test | COMPILE | 0.084009 | 0.139897 | 0.165590 | — | — | — |
| pair_deque.test | COMPILE | 0.173105 | 0.345773 | 0.419077 | — | — | — |
| pair_list.test | COMPILE | 0.172307 | 0.328207 | 0.413103 | — | — | — |
| pair_map.test | COMPILE | 0.177098 | 0.354418 | 0.434683 | — | — | — |
| pair_set.test | COMPILE | 0.183477 | 0.338591 | 0.427582 | — | — | — |
| pair_vector.test | COMPILE | 0.173602 | 0.343405 | 0.405035 | — | — | — |
| pair_nest.test | COMPILE | 0.069472 | 0.147573 | 0.178580 | — | — | — |
| index_sequence.test | COMPILE | 0.073994 | 0.096678 | 0.120715 | — | — | — |
| and.test | COMPILE | 0.050066 | 0.076753 | 0.087778 | — | — | — |
| tag_of.test | COMPILE | 0.083388 | 0.152939 | 0.163694 | — | — | — |
| unused.test | COMPILE | 0.040162 | 0.093436 | 0.107586 | — | — | — |
| tag_of_fallback.test | COMPILE | 0.085017 | 0.142839 | 0.169251 | — | — | — |
| make_pair_r-value.test | COMPILE | 0.071709 | 0.149041 | 0.173491 | — | — | — |
| quick.test | RUN | 0.321795 | 0.670544 | 0.872294 | 0.000781 | 0.002169 | 0.002938 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.015× | 1.023×, 1.007× | 0.957× |
| clang | 0.994× | 0.991×, 0.998× | 0.998× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1353 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adapt_adt.test | 7 | 0.001743s | 0.001763s | 0.001691s | byte equality observed |
| adapt_adt_empty.test | 6 | 0.001638s | 0.001786s | 0.001633s | byte equality observed |
| adapt_adt_named.test | 7 | 0.001774s | 0.001741s | 0.001658s | byte equality observed |
| adapt_adt_named_empty.test | 7 | 0.001607s | 0.001678s | 0.001679s | byte equality observed |
| adapt_assoc_adt.test | 8 | 0.001736s | 0.001692s | 0.001742s | byte equality observed |
| adapt_assoc_adt_empty.test | 9 | 0.001618s | 0.002014s | 0.001662s | byte equality observed |
| adapt_assoc_adt_named.test | 8 | 0.001656s | 0.001753s | 0.001747s | byte equality observed |
| adapt_assoc_adt_named_empty.test | 9 | 0.001640s | 0.001740s | 0.001675s | byte equality observed |
| adapt_assoc_struct.test | 8 | 0.001728s | 0.001728s | 0.001678s | byte equality observed |
| adapt_assoc_struct_empty.test | 8 | 0.001649s | 0.001722s | 0.001697s | byte equality observed |
| adapt_assoc_struct_named.test | 7 | 0.001618s | 0.001810s | 0.001647s | byte equality observed |
| adapt_assoc_struct_named_empty.test | 8 | 0.001671s | 0.001672s | 0.001738s | byte equality observed |
| adapt_assoc_tpl_adt.test | 8 | 0.001803s | 0.001747s | 0.001654s | byte equality observed |
| adapt_assoc_tpl_adt_empty.test | 8 | 0.001676s | 0.001678s | 0.001664s | byte equality observed |
| adapt_assoc_tpl_struct.test | 8 | 0.001742s | 0.001739s | 0.001751s | byte equality observed |
| adapt_assoc_tpl_struct_empty.test | 8 | 0.001670s | 0.001683s | 0.001720s | byte equality observed |
| adapt_struct.test | 8 | 0.001757s | 0.001694s | 0.001693s | byte equality observed |
| adapt_struct_empty.test | 5 | 0.001702s | 0.001689s | 0.001691s | byte equality observed |
| adapt_struct_named.test | 8 | 0.001640s | 0.001640s | 0.001678s | byte equality observed |
| adapt_struct_named_empty.test | 8 | 0.001664s | 0.001583s | 0.001664s | byte equality observed |
| adapt_tpl_adt.test | 8 | 0.001640s | 0.001739s | 0.001637s | byte equality observed |
| adapt_tpl_adt_empty.test | 8 | 0.001739s | 0.001659s | 0.001615s | byte equality observed |
| adapt_tpl_struct.test | 8 | 0.001670s | 0.001597s | 0.001700s | byte equality observed |
| adapt_tpl_struct_empty.test | 9 | 0.001615s | 0.001652s | 0.001675s | byte equality observed |
| adt_attribute_proxy.test | 7 | 0.001576s | 0.001558s | 0.001666s | byte equality observed |
| all.test | 7 | 0.001633s | 0.001607s | 0.001587s | byte equality observed |
| any.test | 7 | 0.001782s | 0.001578s | 0.001603s | byte equality observed |
| array.test | 9 | 0.001593s | 0.001615s | 0.001580s | byte equality observed |
| as_deque.test | 8 | 0.001583s | 0.001663s | 0.001622s | byte equality observed |
| as_list.test | 7 | 0.001684s | 0.001696s | 0.001669s | byte equality observed |
| as_map.test | 7 | 0.001638s | 0.001648s | 0.001635s | byte equality observed |
| as_map_assoc.test | 7 | 0.001687s | 0.001695s | 0.001636s | byte equality observed |
| as_set.test | 8 | 0.001646s | 0.001820s | 0.001601s | byte equality observed |
| as_vector.test | 7 | 0.001722s | 0.001782s | 0.001704s | byte equality observed |
| back_extended_deque.test | 8 | 0.001615s | 0.001656s | 0.001641s | byte equality observed |
| boost_array.test | 7 | 0.001608s | 0.001627s | 0.001621s | byte equality observed |
| boost_tuple.test | 7 | 0.001587s | 0.001632s | 0.001638s | byte equality observed |
| boost_tuple_iterator.test | 6 | 0.001606s | 0.001669s | 0.001637s | byte equality observed |
| clear.test | 7 | 0.001629s | 0.001757s | 0.001642s | byte equality observed |
| cons.test | 7 | 0.001658s | 0.001625s | 0.001697s | byte equality observed |
| convert_boost_tuple.test | 8 | 0.001613s | 0.001648s | 0.001583s | byte equality observed |
| convert_deque.test | 7 | 0.001679s | 0.001630s | 0.001666s | byte equality observed |
| convert_list.test | 7 | 0.001652s | 0.001569s | 0.001700s | byte equality observed |
| convert_std_pair.test | 8 | 0.001720s | 0.001607s | 0.001606s | byte equality observed |
| convert_std_tuple.test | 8 | 0.001630s | 0.001731s | 0.001660s | byte equality observed |
| convert_vector.test | 7 | 0.001645s | 0.001683s | 0.001618s | byte equality observed |
| copy.test | 7 | 0.001570s | 0.001609s | 0.001737s | byte equality observed |
| count.test | 8 | 0.001601s | 0.001630s | 0.001678s | byte equality observed |
| count_if.test | 8 | 0.001628s | 0.001593s | 0.001602s | byte equality observed |
| deduce_sequence.test | 8 | 0.001648s | 0.001606s | 0.001656s | byte equality observed |
| define_assoc_struct.test | 7 | 0.001718s | 0.001606s | 0.001708s | byte equality observed |
| define_assoc_struct_empty.test | 9 | 0.001579s | 0.001809s | 0.001684s | byte equality observed |
| define_assoc_struct_move.test | 8 | 0.001604s | 0.001670s | 0.001574s | byte equality observed |
| define_assoc_tpl_struct.test | 9 | 0.001661s | 0.001691s | 0.001704s | byte equality observed |
| define_assoc_tpl_struct_empty.test | 8 | 0.001626s | 0.001617s | 0.001630s | byte equality observed |
| define_assoc_tpl_struct_move.test | 8 | 0.001606s | 0.001618s | 0.001612s | byte equality observed |
| define_struct.test | 8 | 0.001775s | 0.001700s | 0.001669s | byte equality observed |
| define_struct_empty.test | 9 | 0.001621s | 0.001689s | 0.001692s | byte equality observed |
| define_struct_inline.test | 8 | 0.001695s | 0.001813s | 0.001710s | byte equality observed |
| define_struct_inline_empty.test | 7 | 0.001671s | 0.001730s | 0.001720s | byte equality observed |
| define_struct_inline_move.test | 6 | 0.001579s | 0.001658s | 0.001633s | byte equality observed |
| define_struct_move.test | 8 | 0.001604s | 0.001591s | 0.001598s | byte equality observed |
| define_tpl_struct.test | 8 | 0.001822s | 0.001675s | 0.001832s | byte equality observed |
| define_tpl_struct_empty.test | 8 | 0.001717s | 0.001721s | 0.001645s | byte equality observed |
| define_tpl_struct_inline.test | 4 | 0.001670s | 0.001699s | 0.001698s | byte equality observed |
| define_tpl_struct_inline_empty.test | 6 | 0.001688s | 0.001656s | 0.001628s | byte equality observed |
| define_tpl_struct_inline_move.test | 7 | 0.001632s | 0.001791s | 0.001736s | byte equality observed |
| define_tpl_struct_move.test | 7 | 0.001633s | 0.001635s | 0.001704s | byte equality observed |
| deque_comparison.test | 8 | 0.001584s | 0.001617s | 0.001687s | byte equality observed |
| deque_construction.test | 8 | 0.001696s | 0.001618s | 0.001673s | byte equality observed |
| deque_copy.test | 7 | 0.001550s | 0.001671s | 0.001717s | byte equality observed |
| deque_hash.test | 7 | 0.001576s | 0.001655s | 0.001678s | byte equality observed |
| deque_iterator.test | 7 | 0.001615s | 0.001676s | 0.001625s | byte equality observed |
| deque_make.test | 8 | 0.001576s | 0.001650s | 0.001658s | byte equality observed |
| deque_misc.test | 8 | 0.001576s | 0.001666s | 0.001638s | byte equality observed |
| deque_move.test | 7 | 0.001676s | 0.001655s | 0.001606s | byte equality observed |
| deque_mutate.test | 8 | 0.001592s | 0.001695s | 0.001633s | byte equality observed |
| deque_nest.test | 7 | 0.001562s | 0.001672s | 0.001649s | byte equality observed |
| deque_tie.test | 7 | 0.001559s | 0.001797s | 0.001713s | byte equality observed |
| deque_value_at.test | 7 | 0.001587s | 0.001755s | 0.001663s | byte equality observed |
| erase.test | 8 | 0.001615s | 0.001760s | 0.001676s | byte equality observed |
| erase_key.test | 7 | 0.001775s | 0.001722s | 0.001738s | byte equality observed |
| filter.test | 8 | 0.001685s | 0.001621s | 0.001634s | byte equality observed |
| filter_if.test | 8 | 0.001767s | 0.001689s | 0.001685s | byte equality observed |
| filter_view.test | 6 | 0.001676s | 0.001717s | 0.001778s | byte equality observed |
| find.test | 7 | 0.001707s | 0.001685s | 0.001709s | byte equality observed |
| find_if.test | 8 | 0.001737s | 0.001641s | 0.001631s | byte equality observed |
| flatten.test | 7 | 0.001684s | 0.001631s | 0.001672s | byte equality observed |
| flatten_view.test | 5 | 0.001684s | 0.001634s | 0.001773s | byte equality observed |
| fold.test | 7 | 0.001681s | 0.001829s | 0.001729s | byte equality observed |
| for_each.test | 8 | 0.001671s | 0.001617s | 0.001697s | byte equality observed |
| front_extended_deque.test | 4 | 0.001679s | 0.001601s | 0.001615s | byte equality observed |
| fused.test | 8 | 0.001595s | 0.001635s | 0.001687s | byte equality observed |
| fused_function_object.test | 6 | 0.001587s | 0.001598s | 0.001763s | byte equality observed |
| fused_procedure.test | 5 | 0.001591s | 0.001625s | 0.001727s | byte equality observed |
| github-176.test | 4 | 0.001585s | 0.001714s | 0.001658s | byte equality observed |
| hash.test | 6 | 0.001715s | 0.001579s | 0.001690s | byte equality observed |
| identity_view.test | 6 | 0.001658s | 0.001677s | 0.001680s | byte equality observed |
| insert.test | 4 | 0.001621s | 0.001783s | 0.001719s | byte equality observed |
| insert_range.test | 4 | 0.001727s | 0.001715s | 0.001722s | byte equality observed |
| invoke.test | 7 | 0.001602s | 0.001747s | 0.001690s | byte equality observed |
| invoke_function_object.test | 4 | 0.001637s | 0.001594s | 0.001612s | byte equality observed |
| invoke_procedure.test | 5 | 0.001691s | 0.001617s | 0.001597s | byte equality observed |
| io.test | 6 | 0.001886s | 0.001830s | 0.001839s | byte equality observed |
| iter_fold.test | 6 | 0.001644s | 0.001766s | 0.001660s | byte equality observed |
| iterator_range.test | 6 | 0.001666s | 0.001741s | 0.001678s | byte equality observed |
| join.test | 7 | 0.001582s | 0.001567s | 0.001660s | byte equality observed |
| joint_view.test | 6 | 0.001758s | 0.001856s | 0.001729s | byte equality observed |
| list_comparison.test | 7 | 0.001691s | 0.001611s | 0.001597s | byte equality observed |
| list_construction.test | 7 | 0.001641s | 0.001602s | 0.001658s | byte equality observed |
| list_copy.test | 5 | 0.001566s | 0.001715s | 0.001603s | byte equality observed |
| list_hash.test | 7 | 0.001696s | 0.001671s | 0.001619s | byte equality observed |
| list_iterator.test | 6 | 0.001670s | 0.001600s | 0.001638s | byte equality observed |
| list_make.test | 8 | 0.001626s | 0.001734s | 0.001654s | byte equality observed |
| list_misc.test | 6 | 0.001634s | 0.001623s | 0.001579s | byte equality observed |
| list_mutate.test | 6 | 0.001649s | 0.001577s | 0.001602s | byte equality observed |
| list_nest.test | 8 | 0.001594s | 0.001592s | 0.001599s | byte equality observed |
| list_tie.test | 6 | 0.001586s | 0.001600s | 0.001617s | byte equality observed |
| list_value_at.test | 5 | 0.001553s | 0.001608s | 0.001624s | byte equality observed |
| make_fused.test | 7 | 0.001635s | 0.001718s | 0.001646s | byte equality observed |
| make_fused_function_object.test | 6 | 0.001592s | 0.001620s | 0.001632s | byte equality observed |
| make_fused_procedure.test | 7 | 0.001553s | 0.001808s | 0.001614s | byte equality observed |
| make_list.test | 7 | 0.001583s | 0.001627s | 0.001640s | byte equality observed |
| make_unfused.test | 5 | 0.001581s | 0.001649s | 0.001704s | byte equality observed |
| make_vector.test | 4 | 0.001554s | 0.001633s | 0.001749s | byte equality observed |
| map.test | 6 | 0.001690s | 0.001669s | 0.001627s | byte equality observed |
| map_comparison.test | 6 | 0.001590s | 0.001575s | 0.001615s | byte equality observed |
| map_construction.test | 4 | 0.001540s | 0.001667s | 0.001698s | byte equality observed |
| map_copy.test | 3 | 0.001642s | 0.001563s | 0.001706s | byte equality observed |
| map_misc.test | 3 | 0.001574s | 0.001639s | 0.001645s | byte equality observed |
| map_move.test | 7 | 0.001603s | 0.001762s | 0.001667s | byte equality observed |
| map_mutate.test | 8 | 0.001593s | 0.001641s | 0.001749s | byte equality observed |
| map_tie.test | 7 | 0.001626s | 0.001638s | 0.001621s | byte equality observed |
| move.test | 7 | 0.001637s | 0.001666s | 0.001668s | byte equality observed |
| nil.test | 6 | 0.001530s | 0.001677s | 0.001603s | byte equality observed |
| none.test | 7 | 0.001631s | 0.001563s | 0.001688s | byte equality observed |
| nview.test | 6 | 0.001620s | 0.001671s | 0.001668s | byte equality observed |
| pop_back.test | 6 | 0.001640s | 0.001778s | 0.001687s | byte equality observed |
| pop_front.test | 7 | 0.001630s | 0.001877s | 0.001697s | byte equality observed |
| push_back.test | 7 | 0.001646s | 0.001781s | 0.001680s | byte equality observed |
| push_front.test | 6 | 0.001798s | 0.001709s | 0.001671s | byte equality observed |
| quick.test | 8 | 0.000831s | 0.001626s | 0.001662s | byte equality observed |
| ref_vector.test | 6 | 0.001552s | 0.001652s | 0.001680s | byte equality observed |
| remove.test | 6 | 0.001692s | 0.001875s | 0.001658s | byte equality observed |
| remove_if.test | 5 | 0.001634s | 0.001633s | 0.001750s | byte equality observed |
| repetitive_view.test | 5 | 0.001593s | 0.001640s | 0.001701s | byte equality observed |
| replace.test | 5 | 0.001584s | 0.001776s | 0.001718s | byte equality observed |
| replace_if.test | 6 | 0.001820s | 0.001752s | 0.001735s | byte equality observed |
| reverse.test | 5 | 0.001590s | 0.001755s | 0.001779s | byte equality observed |
| reverse_fold.test | 5 | 0.001622s | 0.001660s | 0.001650s | byte equality observed |
| reverse_iter_fold.test | 5 | 0.001729s | 0.001644s | 0.001678s | byte equality observed |
| reverse_view.test | 5 | 0.001641s | 0.001709s | 0.001644s | byte equality observed |
| segmented_find.test | 8 | 0.001648s | 0.001850s | 0.001615s | byte equality observed |
| segmented_find_if.test | 5 | 0.001642s | 0.001597s | 0.001671s | byte equality observed |
| segmented_fold.test | 6 | 0.001660s | 0.001672s | 0.001624s | byte equality observed |
| segmented_for_each.test | 5 | 0.001712s | 0.001631s | 0.001692s | byte equality observed |
| segmented_iterator_range.test | 4 | 0.001814s | 0.001656s | 0.001667s | byte equality observed |
| set.test | 6 | 0.001639s | 0.001702s | 0.001655s | byte equality observed |
| single_view.test | 6 | 0.001758s | 0.001708s | 0.001699s | byte equality observed |
| std_array.test | 6 | 0.001642s | 0.001633s | 0.001656s | byte equality observed |
| std_pair.test | 4 | 0.001625s | 0.001702s | 0.001718s | byte equality observed |
| std_tuple.test | 3 | 0.001569s | 0.001578s | 0.001734s | byte equality observed |
| std_tuple_iterator.test | 5 | 0.001653s | 0.001692s | 0.001647s | byte equality observed |
| swap.test | 5 | 0.001699s | 0.001653s | 0.001655s | byte equality observed |
| transform.test | 5 | 0.001716s | 0.001724s | 0.001665s | byte equality observed |
| transform_view.test | 3 | 0.001697s | 0.001746s | 0.001708s | byte equality observed |
| tuple_comparison.test | 5 | 0.001749s | 0.001609s | 0.001644s | byte equality observed |
| tuple_construction.test | 3 | 0.001686s | 0.001644s | 0.001648s | byte equality observed |
| tuple_conversion.test | 6 | 0.001690s | 0.001569s | 0.001621s | byte equality observed |
| tuple_copy.test | 5 | 0.001784s | 0.001598s | 0.001609s | byte equality observed |
| tuple_element.test | 5 | 0.001668s | 0.001664s | 0.001663s | byte equality observed |
| tuple_hash.test | 3 | 0.001663s | 0.001848s | 0.001671s | byte equality observed |
| tuple_make.test | 3 | 0.001701s | 0.001658s | 0.001592s | byte equality observed |
| tuple_misc.test | 6 | 0.001613s | 0.001656s | 0.001693s | byte equality observed |
| tuple_mutate.test | 6 | 0.001613s | 0.001671s | 0.001643s | byte equality observed |
| tuple_nest.test | 7 | 0.001699s | 0.001640s | 0.001565s | byte equality observed |
| tuple_tie.test | 3 | 0.001539s | 0.001647s | 0.001652s | byte equality observed |
| tuple_traits__maybe_variadic.test | 6 | 0.001601s | 0.001775s | 0.001619s | byte equality observed |
| tuple_traits__no_variadic.test | 5 | 0.001571s | 0.001740s | 0.001600s | byte equality observed |
| unfused.test | 7 | 0.001627s | 0.001617s | 0.001608s | byte equality observed |
| unfused_typed.test | 4 | 0.001594s | 0.001752s | 0.001630s | byte equality observed |
| vector_comparison.test | 6 | 0.001716s | 0.001615s | 0.001635s | byte equality observed |
| vector_construction.test | 5 | 0.001611s | 0.001663s | 0.001615s | byte equality observed |
| vector_conversion.test | 4 | 0.001635s | 0.001586s | 0.001631s | byte equality observed |
| vector_copy.test | 4 | 0.001631s | 0.001579s | 0.001618s | byte equality observed |
| vector_hash.test | 4 | 0.001717s | 0.001821s | 0.001596s | byte equality observed |
| vector_iterator.test | 5 | 0.001636s | 0.001588s | 0.001654s | byte equality observed |
| vector_make.test | 5 | 0.001631s | 0.001760s | 0.001598s | byte equality observed |
| vector_misc.test | 3 | 0.001605s | 0.001668s | 0.001545s | byte equality observed |
| vector_move.test | 5 | 0.001656s | 0.001641s | 0.001661s | byte equality observed |
| vector_mutate.test | 2 | 0.001652s | 0.001587s | 0.001705s | byte equality observed |
| vector_n.test | 4 | 0.001765s | 0.001783s | 0.001612s | byte equality observed |
| vector_nest.test | 2 | 0.001532s | 0.001602s | 0.001643s | byte equality observed |
| vector_tie.test | 6 | 0.001679s | 0.001673s | 0.001625s | byte equality observed |
| vector_traits__maybe_variadic.test | 7 | 0.001633s | 0.001635s | 0.001602s | byte equality observed |
| vector_traits__no_variadic.test | 6 | 0.001693s | 0.001765s | 0.001696s | byte equality observed |
| vector_value_at.test | 7 | 0.001699s | 0.001574s | 0.001593s | byte equality observed |
| zip.test | 7 | 0.001688s | 0.001635s | 0.001684s | byte equality observed |
| zip2.test | 6 | 0.001647s | 0.001742s | 0.001544s | byte equality observed |
| zip_ignore.test | 7 | 0.001830s | 0.001615s | 0.001658s | byte equality observed |
| zip_view.test | 8 | 0.001649s | 0.001631s | 0.001670s | byte equality observed |
| zip_view2.test | 7 | 0.001652s | 0.001686s | 0.001658s | byte equality observed |
| zip_view_ignore.test | 7 | 0.001697s | 0.001661s | 0.001618s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
