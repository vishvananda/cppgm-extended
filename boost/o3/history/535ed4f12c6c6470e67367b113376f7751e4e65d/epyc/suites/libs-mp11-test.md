# libs/mp11/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| mp11.test | COMPILE | 0.040546 | 0.069945 | 0.119312 | — | — | — |
| complex_h.test | COMPILE | 0.164988 | 0.301115 | 0.475688 | — | — | — |
| mp_size.test | RUN | 0.193467 | 0.944069 | 0.609196 | 0.001828 | 0.001770 | 0.001817 |
| mp_size_2.test | RUN | 0.017385 | 0.035631 | 0.054491 | 0.000770 | 0.000761 | 0.001522 |
| mp_empty.test | RUN | 0.181256 | 0.597155 | 0.549961 | 0.001730 | 0.001868 | 0.001715 |
| mp_empty_2.test | RUN | 0.017954 | 0.037933 | 0.059824 | 0.000632 | 0.000806 | 0.001703 |
| mp_assign.test | RUN | 0.209754 | 1.231983 | 0.731346 | 0.001541 | 0.001910 | 0.001438 |
| mp_assign_2.test | RUN | 0.019058 | 0.037621 | 0.053520 | 0.000630 | 0.000793 | 0.001444 |
| mp_clear.test | RUN | 0.187244 | 0.546043 | 0.589876 | 0.001534 | 0.001710 | 0.001547 |
| mp_clear_2.test | RUN | 0.018404 | 0.034449 | 0.060513 | 0.000610 | 0.000627 | 0.001728 |
| mp_front.test | RUN | 0.189173 | 0.851269 | 0.685999 | 0.001715 | 0.001779 | 0.001908 |
| mp_front_2.test | RUN | 0.015625 | 0.035468 | 0.057337 | 0.000649 | 0.000793 | 0.001612 |
| mp_pop_front.test | RUN | 0.185541 | 0.886965 | 0.648839 | 0.001632 | 0.001683 | 0.001632 |
| mp_pop_front_2.test | RUN | 0.018001 | 0.037254 | 0.049956 | 0.000523 | 0.000717 | 0.001797 |
| mp_second.test | RUN | 0.196657 | 0.570082 | 0.567395 | 0.001698 | 0.001332 | 0.001558 |
| mp_second_2.test | RUN | 0.018846 | 0.038503 | 0.060752 | 0.000737 | 0.000781 | 0.001829 |
| mp_third.test | RUN | 0.185707 | 0.532547 | 0.566588 | 0.001525 | 0.001828 | 0.001462 |
| mp_third_2.test | RUN | 0.018698 | 0.037911 | 0.061857 | 0.000595 | 0.000782 | 0.001550 |
| mp_push_front.test | RUN | 0.269825 | 1.870831 | 1.048035 | 0.001822 | 0.001831 | 0.001596 |
| mp_push_front_2.test | RUN | 0.017834 | 0.035433 | 0.058520 | 0.000660 | 0.000726 | 0.001706 |
| mp_push_back.test | RUN | 0.272615 | 1.873102 | 1.044059 | 0.001682 | 0.001709 | 0.001560 |
| mp_push_back_2.test | RUN | 0.018571 | 0.037643 | 0.055137 | 0.000755 | 0.000842 | 0.001776 |
| mp_rename.test | RUN | 0.223496 | 1.711453 | 0.840977 | 0.001756 | 0.001762 | 0.001634 |
| mp_rename_2.test | RUN | 0.018685 | 0.038434 | 0.058688 | 0.000640 | 0.000787 | 0.001714 |
| mp_append.test | RUN | 0.233954 | 1.956334 | 0.971126 | 0.001607 | 0.001640 | 0.001739 |
| mp_append_2.test | RUN | 0.436114 | 4.331771 | 1.255466 | 0.001874 | 0.001906 | 0.001245 |
| mp_append_3.test | RUN | 0.018414 | 0.037619 | 0.059588 | 0.000599 | 0.000808 | 0.001781 |
| mp_append_sf.test | RUN | 0.184537 | 0.469114 | 0.560012 | 0.001536 | 0.001843 | 0.001616 |
| mp_replace_front.test | RUN | 0.217000 | 1.399245 | 0.832969 | 0.001268 | 0.002052 | 0.001746 |
| mp_replace_front_2.test | RUN | 0.018142 | 0.035575 | 0.053390 | 0.000643 | 0.000791 | 0.001724 |
| mp_replace_second.test | RUN | 0.206947 | 0.898008 | 0.696384 | 0.001572 | 0.001836 | 0.001551 |
| mp_replace_second_2.test | RUN | 0.017735 | 0.037618 | 0.057819 | 0.000696 | 0.000797 | 0.001348 |
| mp_replace_third.test | RUN | 0.202228 | 0.739528 | 0.658379 | 0.001752 | 0.001659 | 0.001888 |
| mp_replace_third_2.test | RUN | 0.017257 | 0.036489 | 0.055710 | 0.000777 | 0.000641 | 0.001686 |
| mp_apply_q.test | RUN | 0.219748 | 1.720483 | 0.877545 | 0.001439 | 0.001950 | 0.001533 |
| mp_apply_q_sf.test | RUN | 0.182338 | 0.506211 | 0.548567 | 0.001764 | 0.001883 | 0.001847 |
| mp_is_list.test | RUN | 0.189041 | 0.599110 | 0.585624 | 0.001621 | 0.001844 | 0.001728 |
| mp_list_c.test | RUN | 0.189127 | 0.721125 | 0.622596 | 0.001558 | 0.001824 | 0.001468 |
| mp_transform_front.test | RUN | 0.204411 | 1.405905 | 0.816967 | 0.001746 | 0.001784 | 0.001330 |
| mp_transform_front_2.test | RUN | 0.018386 | 0.037806 | 0.052354 | 0.000668 | 0.000846 | 0.001851 |
| mp_transform_second.test | RUN | 0.198025 | 1.203407 | 0.718279 | 0.001384 | 0.001821 | 0.001466 |
| mp_transform_second_2.test | RUN | 0.017953 | 0.035735 | 0.058433 | 0.000757 | 0.000607 | 0.001578 |
| mp_transform_third.test | RUN | 0.187681 | 0.911671 | 0.618215 | 0.001613 | 0.001612 | 0.001931 |
| mp_transform_third_2.test | RUN | 0.017880 | 0.036966 | 0.058244 | 0.000603 | 0.000610 | 0.001725 |
| mp_list_v.test | RUN | 0.017466 | 0.037582 | 0.058249 | 0.000578 | 0.000773 | 0.001566 |
| mp_rename_v.test | RUN | 0.017650 | 0.037053 | 0.052792 | 0.000650 | 0.000740 | 0.001715 |
| mp_is_value_list.test | RUN | 0.174362 | 0.384381 | 0.529888 | 0.001832 | 0.001802 | 0.001850 |
| mp_transform.test | RUN | 0.364741 | 2.941200 | 1.578132 | 0.001800 | 0.001802 | 0.001714 |
| mp_transform_2.test | RUN | 0.177554 | 0.355790 | 0.520747 | 0.001686 | 0.001782 | 0.001844 |
| mp_transform_q.test | RUN | 0.353399 | 2.808021 | 1.535353 | 0.001718 | 0.001783 | 0.001817 |
| mp_transform_q_2.test | RUN | 0.406985 | 0.927861 | 2.016128 | 0.001670 | 0.001641 | 0.001657 |
| mp_transform_sf.test | RUN | 0.191528 | 0.521606 | 0.597780 | 0.001793 | 0.001783 | 0.001616 |
| mp_transform_if.test | RUN | 0.228637 | 1.404627 | 0.850425 | 0.001888 | 0.001763 | 0.001435 |
| mp_transform_if_q.test | RUN | 0.229022 | 1.404658 | 0.876573 | 0.001580 | 0.001788 | 0.001383 |
| mp_filter.test | RUN | 0.212581 | 0.844501 | 0.680554 | 0.001638 | 0.001787 | 0.001511 |
| mp_fill.test | RUN | 0.197135 | 0.629028 | 0.636265 | 0.001624 | 0.001739 | 0.001730 |
| mp_fill_2.test | RUN | 0.032317 | 0.058481 | 0.089407 | 0.000657 | 0.000816 | 0.001738 |
| mp_count.test | RUN | 0.360639 | 1.510056 | 1.000227 | 0.001808 | 0.001716 | 0.001594 |
| mp_count_if.test | RUN | 0.369224 | 1.542658 | 1.069418 | 0.001512 | 0.001654 | 0.001809 |
| mp_count_if_q.test | RUN | 0.196202 | 0.994585 | 0.638244 | 0.001480 | 0.001871 | 0.001771 |
| mp_contains.test | RUN | 0.191165 | 0.608033 | 0.600008 | 0.001516 | 0.001594 | 0.001822 |
| mp_starts_with.test | RUN | 0.190680 | 0.616932 | 0.625409 | 0.001634 | 0.001719 | 0.001461 |
| mp_starts_with_sf.test | RUN | 0.188449 | 0.546139 | 0.561817 | 0.001286 | 0.001705 | 0.001666 |
| mp_repeat.test | RUN | 0.282698 | 2.483063 | 1.122137 | 0.001853 | 0.001602 | 0.001762 |
| mp_repeat_2.test | RUN | 0.033546 | 0.054596 | 0.099231 | 0.000773 | 0.000774 | 0.001690 |
| mp_repeat_3.test | RUN | 0.231125 | 0.333811 | 0.570658 | 0.001585 | 0.001918 | 0.001756 |
| mp_product.test | RUN | 0.214931 | 1.485292 | 0.833656 | 0.001583 | 0.001818 | 0.001608 |
| mp_drop.test | RUN | 0.228112 | 1.877488 | 0.921458 | 0.001408 | 0.001672 | 0.001534 |
| mp_drop_2.test | RUN | 0.029985 | 0.058496 | 0.088257 | 0.000633 | 0.000797 | 0.001812 |
| mp_iota.test | RUN | 0.225717 | 1.574543 | 0.827554 | 0.001746 | 0.001835 | 0.001656 |
| mp_iota_2.test | RUN | 0.238523 | 1.907097 | 0.926007 | 0.001548 | 0.001883 | 0.001636 |
| mp_at.test | RUN | 0.206003 | 0.839661 | 0.635581 | 0.001649 | 0.001870 | 0.001720 |
| mp_at_2.test | RUN | 0.031635 | 0.055044 | 0.099763 | 0.000816 | 0.000826 | 0.001815 |
| mp_at_sf.test | RUN | 0.199093 | 0.517232 | 0.597802 | 0.001548 | 0.001809 | 0.001861 |
| mp_take.test | RUN | 0.289544 | 2.452197 | 1.102343 | 0.001656 | 0.001635 | 0.001854 |
| mp_take_2.test | RUN | 0.030965 | 0.057501 | 0.082217 | 0.000645 | 0.000778 | 0.001548 |
| mp_slice.test | RUN | 0.325294 | 2.487976 | 1.396161 | 0.001622 | 0.001914 | 0.001736 |
| mp_slice_2.test | RUN | 0.032359 | 0.056684 | 0.101938 | 0.000655 | 0.000779 | 0.001836 |
| mp_replace.test | RUN | 0.234914 | 1.277498 | 0.878454 | 0.001903 | 0.001806 | 0.001814 |
| mp_replace_if.test | RUN | 0.223083 | 1.113057 | 0.860557 | 0.001775 | 0.001495 | 0.001789 |
| mp_replace_if_q.test | RUN | 0.220767 | 1.121248 | 0.819174 | 0.001758 | 0.001886 | 0.001864 |
| mp_copy_if.test | RUN | 0.225354 | 1.568232 | 0.796573 | 0.001629 | 0.001850 | 0.001523 |
| mp_copy_if_q.test | RUN | 0.230977 | 1.589288 | 0.792524 | 0.001856 | 0.001905 | 0.001336 |
| mp_remove.test | RUN | 0.217168 | 1.045249 | 0.829079 | 0.001834 | 0.001840 | 0.001810 |
| mp_remove_if.test | RUN | 0.231113 | 1.655161 | 0.882652 | 0.001732 | 0.001811 | 0.001822 |
| mp_remove_if_q.test | RUN | 0.239081 | 1.659649 | 0.877923 | 0.001554 | 0.001564 | 0.001668 |
| mp_partition.test | RUN | 0.229620 | 1.567435 | 0.887172 | 0.001853 | 0.001898 | 0.001738 |
| mp_partition_q.test | RUN | 0.232143 | 1.539107 | 0.907759 | 0.001785 | 0.001877 | 0.001775 |
| mp_sort.test | RUN | 0.210854 | 1.128445 | 0.802092 | 0.001707 | 0.001892 | 0.001878 |
| mp_sort_q.test | RUN | 0.217711 | 1.114663 | 0.757003 | 0.001676 | 0.001516 | 0.001553 |
| mp_find.test | RUN | 0.217164 | 1.202884 | 0.725419 | 0.001818 | 0.001870 | 0.001808 |
| mp_find_if.test | RUN | 0.211134 | 1.105958 | 0.688218 | 0.001816 | 0.001613 | 0.001840 |
| mp_find_if_q.test | RUN | 0.228127 | 1.246031 | 0.790265 | 0.001828 | 0.001959 | 0.001824 |
| mp_reverse.test | RUN | 0.393851 | 3.264203 | 1.552281 | 0.001646 | 0.001893 | 0.001792 |
| mp_fold.test | RUN | 0.665550 | 2.543220 | 3.852639 | 0.001673 | 0.001882 | 0.001616 |
| mp_fold_q.test | RUN | 0.215743 | 1.113875 | 0.745609 | 0.001444 | 0.001830 | 0.001843 |
| mp_fold_q_sf.test | RUN | 0.189458 | 0.545861 | 0.580620 | 0.001733 | 0.001858 | 0.001412 |
| mp_reverse_fold.test | RUN | 0.328324 | 2.446385 | 1.099645 | 0.001611 | 0.001843 | 0.001699 |
| mp_reverse_fold_q.test | RUN | 0.219040 | 1.330225 | 0.834369 | 0.001792 | 0.001970 | 0.001292 |
| mp_unique.test | RUN | 0.215451 | 1.220966 | 0.801157 | 0.001522 | 0.001804 | 0.001792 |
| mp_unique_if.test | RUN | 0.348828 | 2.515703 | 1.249927 | 0.001289 | 0.001635 | 0.001924 |
| mp_unique_if_q.test | RUN | 0.299804 | 2.512533 | 1.048741 | 0.001731 | 0.001793 | 0.001352 |
| mp_all_of.test | RUN | 0.504439 | 1.415132 | 1.321529 | 0.001719 | 0.001924 | 0.001822 |
| mp_all_of_q.test | RUN | 0.191188 | 0.620576 | 0.629907 | 0.001830 | 0.001890 | 0.001742 |
| mp_any_of.test | RUN | 0.497104 | 1.467921 | 1.368775 | 0.001622 | 0.001710 | 0.001664 |
| mp_any_of_q.test | RUN | 0.200001 | 0.618355 | 0.615813 | 0.001816 | 0.001792 | 0.001716 |
| mp_none_of.test | RUN | 0.519428 | 1.437038 | 1.361719 | 0.001761 | 0.001849 | 0.001780 |
| mp_none_of_q.test | RUN | 0.189327 | 0.610955 | 0.556903 | 0.001522 | 0.001899 | 0.001727 |
| mp_replace_at.test | RUN | 0.223959 | 1.234222 | 0.808993 | 0.001590 | 0.001824 | 0.001758 |
| mp_replace_at_c.test | RUN | 0.222465 | 1.201190 | 0.913498 | 0.001455 | 0.001762 | 0.001778 |
| mp_for_each.test | RUN | 0.214059 | 0.890429 | 1.038035 | 0.001860 | 0.001762 | 0.001783 |
| mp_insert.test | RUN | 0.315702 | 2.459810 | 1.187343 | 0.001701 | 0.001524 | 0.001892 |
| mp_insert_2.test | RUN | 0.031275 | 0.054153 | 0.098532 | 0.000805 | 0.000714 | 0.001761 |
| mp_erase.test | RUN | 0.340909 | 2.633810 | 1.516454 | 0.001770 | 0.001922 | 0.001894 |
| mp_erase_2.test | RUN | 0.031080 | 0.058210 | 0.096247 | 0.000609 | 0.000791 | 0.001732 |
| mp_with_index.test | RUN | 0.474341 | 5.379023 | 3.268962 | 0.002171 | 0.002022 | 0.002302 |
| mp_with_index_cx.test | RUN | 0.003457 | 0.018208 | 0.026148 | 0.000751 | 0.000814 | 0.001636 |
| mp_from_sequence.test | RUN | 0.223351 | 1.406566 | 0.857796 | 0.001820 | 0.001959 | 0.001801 |
| mp_from_sequence_2.test | RUN | 0.239452 | 1.642492 | 0.922392 | 0.001711 | 0.001894 | 0.001812 |
| mp_from_sequence_3.test | RUN | 0.205334 | 0.722308 | 0.695546 | 0.001810 | 0.001916 | 0.001765 |
| mp_min_element.test | RUN | 0.198611 | 0.679052 | 0.574016 | 0.001860 | 0.001873 | 0.001635 |
| mp_min_element_2.test | RUN | 0.032279 | 0.055933 | 0.095772 | 0.000605 | 0.000790 | 0.001834 |
| mp_min_element_q.test | RUN | 0.197134 | 0.662761 | 0.620646 | 0.001541 | 0.001752 | 0.001521 |
| mp_min_element_q_2.test | RUN | 0.031973 | 0.057585 | 0.092297 | 0.000702 | 0.000738 | 0.001721 |
| mp_max_element.test | RUN | 0.196531 | 0.677246 | 0.619891 | 0.001398 | 0.001865 | 0.001689 |
| mp_max_element_2.test | RUN | 0.031387 | 0.053531 | 0.091821 | 0.000640 | 0.000686 | 0.001739 |
| mp_max_element_q.test | RUN | 0.197653 | 0.668413 | 0.596857 | 0.001878 | 0.001898 | 0.001892 |
| mp_max_element_q_2.test | RUN | 0.031462 | 0.056213 | 0.082294 | 0.000798 | 0.000793 | 0.001685 |
| mp_nth_element.test | RUN | 0.221268 | 0.929836 | 0.794501 | 0.001804 | 0.001620 | 0.001680 |
| mp_nth_element_q.test | RUN | 0.219825 | 0.923628 | 0.786881 | 0.001686 | 0.001629 | 0.001800 |
| mp_back.test | RUN | 0.214286 | 0.776060 | 0.658200 | 0.001687 | 0.001621 | 0.001471 |
| mp_back_2.test | RUN | 0.032392 | 0.054563 | 0.100096 | 0.000592 | 0.000828 | 0.001809 |
| mp_pop_back.test | RUN | 0.482375 | 5.023263 | 1.435490 | 0.001894 | 0.001750 | 0.001646 |
| mp_pop_back_2.test | RUN | 0.031106 | 0.055608 | 0.083258 | 0.000792 | 0.000624 | 0.001506 |
| mp_flatten.test | RUN | 0.236318 | 1.788859 | 1.021648 | 0.001785 | 0.001923 | 0.001696 |
| mp_rotate_left.test | RUN | 0.242344 | 1.928369 | 0.988427 | 0.001702 | 0.001879 | 0.001686 |
| mp_rotate_right.test | RUN | 0.238455 | 1.924431 | 1.000902 | 0.001364 | 0.001552 | 0.001849 |
| mp_power_set.test | RUN | 0.237223 | 1.856280 | 0.979255 | 0.001820 | 0.001854 | 0.001766 |
| mp_partial_sum.test | RUN | 0.294941 | 2.455080 | 1.119929 | 0.001467 | 0.001740 | 0.001660 |
| mp_iterate.test | RUN | 0.210120 | 0.741222 | 0.795364 | 0.001644 | 0.001808 | 0.001602 |
| mp_pairwise_fold.test | RUN | 0.240617 | 1.686926 | 1.003191 | 0.001613 | 0.001930 | 0.001296 |
| mp_pairwise_fold_q.test | RUN | 0.195241 | 0.502870 | 0.618586 | 0.001793 | 0.001713 | 0.001917 |
| mp_sliding_fold.test | RUN | 0.405233 | 3.251671 | 1.853234 | 0.001611 | 0.001676 | 0.001458 |
| mp_sliding_fold_q.test | RUN | 0.404004 | 3.206608 | 1.872139 | 0.001627 | 0.001864 | 0.001640 |
| mp_intersperse.test | RUN | 0.225952 | 1.227839 | 0.904591 | 0.001923 | 0.001915 | 0.001764 |
| mp_split.test | RUN | 0.296642 | 2.340986 | 1.183683 | 0.001703 | 0.001877 | 0.001914 |
| mp_join.test | RUN | 0.284336 | 1.967152 | 1.086133 | 0.001812 | 0.001890 | 0.001848 |
| integral.test | RUN | 0.198142 | 0.880934 | 0.634771 | 0.001613 | 0.001933 | 0.001837 |
| mp_value.test | RUN | 0.013801 | 0.030390 | 0.047404 | 0.000760 | 0.000656 | 0.001876 |
| mp_identity.test | RUN | 0.189459 | 0.814459 | 0.623119 | 0.001528 | 0.001824 | 0.001830 |
| mp_inherit.test | RUN | 0.185879 | 0.569570 | 0.577646 | 0.001820 | 0.001750 | 0.001822 |
| mp_if.test | RUN | 0.185555 | 0.663908 | 0.564762 | 0.001840 | 0.001868 | 0.001843 |
| mp_if_sf.test | RUN | 0.181238 | 0.481873 | 0.561379 | 0.001736 | 0.001909 | 0.001854 |
| mp_eval_if.test | RUN | 0.188111 | 0.709220 | 0.625741 | 0.001621 | 0.001660 | 0.001850 |
| mp_eval_if_sf.test | RUN | 0.180054 | 0.499826 | 0.510351 | 0.001406 | 0.001724 | 0.001774 |
| mp_valid.test | RUN | 0.179684 | 0.510394 | 0.541871 | 0.001692 | 0.001631 | 0.001853 |
| mp_defer.test | RUN | 0.198558 | 0.843160 | 0.666448 | 0.001749 | 0.001551 | 0.001843 |
| mp_quote.test | RUN | 0.208012 | 0.852663 | 0.675411 | 0.001557 | 0.001882 | 0.001682 |
| mp_invoke_q.test | RUN | 0.219980 | 1.119117 | 0.746310 | 0.001794 | 0.001908 | 0.001614 |
| mp_invoke_q_sf.test | RUN | 0.177391 | 0.510702 | 0.565983 | 0.001572 | 0.001863 | 0.001774 |
| mp_quote_trait.test | RUN | 0.198317 | 0.933700 | 0.605101 | 0.001621 | 0.001488 | 0.001807 |
| mp_cond.test | RUN | 0.187225 | 0.578218 | 0.594690 | 0.001635 | 0.001666 | 0.001463 |
| mp_cond_sf.test | RUN | 0.185171 | 0.510099 | 0.565152 | 0.001738 | 0.001921 | 0.001846 |
| mp_not_fn.test | RUN | 0.179002 | 0.501861 | 0.528880 | 0.001595 | 0.001698 | 0.001786 |
| mp_eval_if_not.test | RUN | 0.189205 | 0.707731 | 0.607352 | 0.001792 | 0.001896 | 0.001944 |
| mp_eval_or.test | RUN | 0.185356 | 0.382010 | 0.578463 | 0.001744 | 0.001715 | 0.001850 |
| mp_compose.test | RUN | 0.204964 | 1.168065 | 0.726971 | 0.001684 | 0.001756 | 0.001694 |
| mp_valid_and_true.test | RUN | 0.181779 | 0.589987 | 0.559361 | 0.001534 | 0.001851 | 0.001922 |
| mp_compose_sf.test | COMPILE | 0.044216 | 0.075661 | 0.148196 | — | — | — |
| integer_sequence.test | RUN | 0.210058 | 1.133457 | 0.779190 | 0.001414 | 0.001576 | 0.001856 |
| tuple_for_each.test | RUN | 0.186271 | 0.428790 | 0.580562 | 0.001652 | 0.001752 | 0.001802 |
| tuple_for_each_cx.test | COMPILE | 0.035762 | 0.062897 | 0.109424 | — | — | — |
| tuple_apply.test | RUN | 0.193246 | 0.427596 | 0.615500 | 0.001628 | 0.001826 | 0.001766 |
| tuple_apply_cx.test | COMPILE | 0.051258 | 0.090661 | 0.153551 | — | — | — |
| construct_from_tuple.test | RUN | 0.190187 | 0.459819 | 0.603232 | 0.001778 | 0.001945 | 0.001860 |
| construct_from_tuple_cx.test | COMPILE | 0.052292 | 0.092581 | 0.127246 | — | — | — |
| tuple_transform.test | RUN | 0.292396 | 0.935609 | 1.218217 | 0.001743 | 0.001701 | 0.001825 |
| tuple_transform_2.test | RUN | 0.260180 | 0.697014 | 1.069156 | 0.001798 | 0.001830 | 0.001552 |
| tuple_transform_cx.test | COMPILE | 0.044172 | 0.084198 | 0.141972 | — | — | — |
| mp_set_contains.test | RUN | 0.187879 | 0.612180 | 0.630644 | 0.001773 | 0.001575 | 0.001604 |
| mp_set_push_back.test | RUN | 0.239379 | 1.833888 | 1.024661 | 0.001798 | 0.001847 | 0.001863 |
| mp_set_push_front.test | RUN | 0.279470 | 2.216224 | 1.167061 | 0.001459 | 0.001750 | 0.001530 |
| mp_is_set.test | RUN | 0.187073 | 0.608927 | 0.568697 | 0.001700 | 0.001840 | 0.001920 |
| mp_set_union.test | RUN | 0.230344 | 1.565178 | 0.973063 | 0.001848 | 0.001853 | 0.001602 |
| mp_set_union_sf.test | RUN | 0.181053 | 0.480690 | 0.541818 | 0.001827 | 0.001897 | 0.001714 |
| mp_set_difference.test | RUN | 0.213340 | 1.219829 | 0.797596 | 0.001680 | 0.001938 | 0.001868 |
| mp_set_difference_sf.test | RUN | 0.182381 | 0.483902 | 0.574759 | 0.001636 | 0.001948 | 0.001798 |
| mp_set_intersection.test | RUN | 0.210239 | 1.219914 | 0.799544 | 0.001766 | 0.001714 | 0.001781 |
| mp_set_intersection_sf.test | RUN | 0.187671 | 0.490258 | 0.532783 | 0.001832 | 0.001866 | 0.001652 |
| mp_all.test | RUN | 0.185797 | 0.596282 | 0.592456 | 0.001658 | 0.001846 | 0.001856 |
| mp_all_2.test | RUN | 0.511067 | 1.314563 | 1.335309 | 0.001791 | 0.001874 | 0.001656 |
| mp_and.test | RUN | 0.182482 | 0.606852 | 0.567519 | 0.001822 | 0.001910 | 0.001854 |
| mp_any.test | RUN | 0.185994 | 0.578939 | 0.596220 | 0.001605 | 0.001863 | 0.001915 |
| mp_any_2.test | RUN | 0.491897 | 1.303097 | 1.288927 | 0.001774 | 0.001912 | 0.001653 |
| mp_or.test | RUN | 0.189817 | 0.578880 | 0.613288 | 0.001825 | 0.001841 | 0.001478 |
| mp_same.test | RUN | 0.341303 | 1.017141 | 0.964104 | 0.001307 | 0.001579 | 0.001811 |
| mp_plus.test | RUN | 0.197362 | 0.690758 | 0.653605 | 0.001646 | 0.001657 | 0.001668 |
| mp_less.test | RUN | 0.186656 | 0.603001 | 0.551522 | 0.001829 | 0.001720 | 0.001476 |
| mp_min.test | RUN | 0.188730 | 0.558363 | 0.579921 | 0.001831 | 0.001807 | 0.001832 |
| mp_max.test | RUN | 0.192042 | 0.618252 | 0.593523 | 0.001527 | 0.001630 | 0.001780 |
| mp_similar.test | RUN | 0.184625 | 0.606278 | 0.614053 | 0.001670 | 0.001611 | 0.001718 |
| mp_map_find.test | RUN | 0.213803 | 1.105782 | 0.780851 | 0.001679 | 0.001498 | 0.001716 |
| mp_map_find_2.test | RUN | 0.201146 | 0.447924 | 0.680130 | 0.001776 | 0.001912 | 0.001814 |
| mp_map_find_3.test | RUN | 0.199274 | 0.447076 | 0.673436 | 0.001518 | 0.001513 | 0.001773 |
| mp_map_find_4.test | RUN | 0.193010 | 0.422818 | 0.609072 | 0.001622 | 0.001555 | 0.001750 |
| mp_map_contains.test | RUN | 0.198391 | 0.636127 | 0.618397 | 0.001871 | 0.001931 | 0.001962 |
| mp_map_insert.test | RUN | 0.284097 | 2.051627 | 1.078605 | 0.001422 | 0.001896 | 0.001840 |
| mp_map_replace.test | RUN | 0.299006 | 2.316293 | 1.243715 | 0.001880 | 0.001914 | 0.001830 |
| mp_map_erase.test | RUN | 0.244657 | 1.723401 | 1.020417 | 0.001461 | 0.001920 | 0.001246 |
| mp_map_update.test | RUN | 0.219551 | 1.134238 | 0.819620 | 0.001937 | 0.001936 | 0.001673 |
| mp_map_update_q.test | RUN | 0.215633 | 1.144984 | 0.791632 | 0.001802 | 0.001728 | 0.001445 |
| mp_map_keys.test | RUN | 0.214088 | 1.032290 | 0.793510 | 0.001345 | 0.001852 | 0.001801 |
| mp_is_map.test | RUN | 0.199039 | 0.614447 | 0.651845 | 0.001661 | 0.001708 | 0.001520 |
| mp_bind.test | RUN | 0.234911 | 1.813656 | 0.932633 | 0.001865 | 0.001633 | 0.001812 |
| mp_bind_q.test | RUN | 0.213726 | 1.133955 | 0.750088 | 0.001763 | 0.001709 | 0.001520 |
| mp_bind_front.test | RUN | 0.213572 | 1.222038 | 0.750362 | 0.001406 | 0.001571 | 0.001454 |
| mp_bind_back.test | RUN | 0.212790 | 1.247898 | 0.753518 | 0.001575 | 0.001963 | 0.001606 |
| mp_lambda.test | RUN | 0.413323 | 3.693838 | 1.898773 | 0.001322 | 0.001396 | 0.001752 |
| mpl.test | RUN | 0.398663 | 0.999635 | 1.566468 | 0.001716 | 0.001816 | 0.001788 |
| mpl_list.test | RUN | 0.255084 | 0.744863 | 0.731159 | 0.001682 | 0.001708 | 0.001401 |
| mpl_tuple.test | RUN | 0.247754 | 0.738345 | 0.733552 | 0.001597 | 0.001691 | 0.001815 |
| version.test | RUN | 0.123098 | 0.272002 | 0.366969 | 0.001824 | 0.001658 | 0.001609 |
| mp11_version.test | RUN | 0.152028 | 0.315861 | 0.444007 | 0.001655 | 0.001811 | 0.001723 |
| list_version.test | RUN | 0.128216 | 0.283596 | 0.370689 | 0.001332 | 0.001738 | 0.001666 |
| algorithm_version.test | RUN | 0.143149 | 0.295254 | 0.414234 | 0.001774 | 0.001820 | 0.001454 |
| integral_version.test | RUN | 0.124378 | 0.275883 | 0.381466 | 0.001786 | 0.001890 | 0.001518 |
| utility_version.test | RUN | 0.125391 | 0.273978 | 0.361910 | 0.001612 | 0.001743 | 0.001574 |
| function_version.test | RUN | 0.129342 | 0.276817 | 0.391406 | 0.001382 | 0.001766 | 0.001602 |
| map_version.test | RUN | 0.142620 | 0.301641 | 0.422611 | 0.001793 | 0.001785 | 0.001751 |
| set_version.test | RUN | 0.130283 | 0.281106 | 0.397255 | 0.001844 | 0.001632 | 0.001616 |
| bind_version.test | RUN | 0.144374 | 0.294614 | 0.427461 | 0.001694 | 0.001869 | 0.001760 |
| integer_sequence_version.test | RUN | 0.124064 | 0.276049 | 0.296467 | 0.001676 | 0.001848 | 0.001840 |
| tuple_version.test | RUN | 0.139350 | 0.297598 | 0.353467 | 0.001468 | 0.001894 | 0.001303 |
| mpl_version.test | RUN | 0.104965 | 0.227522 | 0.341427 | 0.001522 | 0.001384 | 0.001282 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.999× | 0.992×, 1.006× | 1.007× |
| clang | 1.073× | 1.079×, 1.068× | 1.005× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| algorithm_version.test | 10 | 0.000987s | 0.000996s | 0.001028s | byte equality observed |
| bind_version.test | 10 | 0.000985s | 0.000968s | 0.001020s | byte equality observed |
| construct_from_tuple.test | 10 | 0.000997s | 0.000981s | 0.001045s | byte equality observed |
| function_version.test | 10 | 0.001006s | 0.000979s | 0.001034s | byte equality observed |
| integer_sequence.test | 10 | 0.001006s | 0.001000s | 0.001024s | byte equality observed |
| integer_sequence_version.test | 10 | 0.000990s | 0.000995s | 0.001025s | byte equality observed |
| integral.test | 10 | 0.001008s | 0.000991s | 0.001039s | byte equality observed |
| integral_version.test | 10 | 0.001020s | 0.000970s | 0.001019s | byte equality observed |
| list_version.test | 10 | 0.001042s | 0.000989s | 0.001021s | byte equality observed |
| map_version.test | 10 | 0.001034s | 0.000974s | 0.001027s | byte equality observed |
| mp11_version.test | 10 | 0.001045s | 0.000978s | 0.001047s | byte equality observed |
| mp_all.test | 10 | 0.001028s | 0.000990s | 0.001035s | byte equality observed |
| mp_all_2.test | 10 | 0.001057s | 0.000988s | 0.001019s | byte equality observed |
| mp_all_of.test | 10 | 0.001038s | 0.001011s | 0.001045s | byte equality observed |
| mp_all_of_q.test | 10 | 0.001017s | 0.001014s | 0.001025s | byte equality observed |
| mp_and.test | 10 | 0.001024s | 0.001011s | 0.001031s | byte equality observed |
| mp_any.test | 10 | 0.001025s | 0.001007s | 0.001003s | byte equality observed |
| mp_any_2.test | 10 | 0.001003s | 0.001047s | 0.001030s | byte equality observed |
| mp_any_of.test | 10 | 0.001022s | 0.001018s | 0.001016s | byte equality observed |
| mp_any_of_q.test | 10 | 0.001018s | 0.001044s | 0.001003s | byte equality observed |
| mp_append.test | 10 | 0.001007s | 0.001066s | 0.001015s | byte equality observed |
| mp_append_2.test | 10 | 0.001065s | 0.001050s | 0.001037s | byte equality observed |
| mp_append_3.test | 10 | 0.000487s | 0.000480s | 0.001015s | byte equality observed |
| mp_append_sf.test | 10 | 0.001038s | 0.001048s | 0.001018s | byte equality observed |
| mp_apply_q.test | 10 | 0.001027s | 0.001052s | 0.001030s | byte equality observed |
| mp_apply_q_sf.test | 10 | 0.001009s | 0.001020s | 0.001020s | byte equality observed |
| mp_assign.test | 10 | 0.001038s | 0.001038s | 0.001030s | byte equality observed |
| mp_assign_2.test | 10 | 0.000485s | 0.000473s | 0.001009s | byte equality observed |
| mp_at.test | 10 | 0.001049s | 0.001068s | 0.001037s | byte equality observed |
| mp_at_2.test | 10 | 0.000483s | 0.000484s | 0.000987s | byte equality observed |
| mp_at_sf.test | 10 | 0.001056s | 0.001029s | 0.001026s | byte equality observed |
| mp_back.test | 10 | 0.001053s | 0.001032s | 0.001020s | byte equality observed |
| mp_back_2.test | 10 | 0.000485s | 0.000488s | 0.000997s | byte equality observed |
| mp_bind.test | 10 | 0.001038s | 0.001054s | 0.001024s | byte equality observed |
| mp_bind_back.test | 10 | 0.001027s | 0.001070s | 0.001013s | byte equality observed |
| mp_bind_front.test | 10 | 0.001067s | 0.001036s | 0.001029s | byte equality observed |
| mp_bind_q.test | 10 | 0.001055s | 0.001063s | 0.001038s | byte equality observed |
| mp_clear.test | 10 | 0.001056s | 0.001031s | 0.001044s | byte equality observed |
| mp_clear_2.test | 10 | 0.000491s | 0.000487s | 0.001020s | byte equality observed |
| mp_compose.test | 10 | 0.001018s | 0.001047s | 0.001016s | byte equality observed |
| mp_cond.test | 10 | 0.001031s | 0.001056s | 0.001025s | byte equality observed |
| mp_cond_sf.test | 10 | 0.001020s | 0.001021s | 0.001027s | byte equality observed |
| mp_contains.test | 10 | 0.001012s | 0.001046s | 0.001022s | byte equality observed |
| mp_copy_if.test | 10 | 0.001036s | 0.001032s | 0.001012s | byte equality observed |
| mp_copy_if_q.test | 10 | 0.001038s | 0.001036s | 0.001030s | byte equality observed |
| mp_count.test | 10 | 0.001042s | 0.001033s | 0.001040s | byte equality observed |
| mp_count_if.test | 10 | 0.001032s | 0.001049s | 0.001016s | byte equality observed |
| mp_count_if_q.test | 10 | 0.001034s | 0.001065s | 0.001037s | byte equality observed |
| mp_defer.test | 10 | 0.001073s | 0.001038s | 0.001029s | byte equality observed |
| mp_drop.test | 10 | 0.001061s | 0.001061s | 0.001045s | byte equality observed |
| mp_drop_2.test | 10 | 0.000485s | 0.000486s | 0.001026s | byte equality observed |
| mp_empty.test | 10 | 0.001063s | 0.001053s | 0.001009s | byte equality observed |
| mp_empty_2.test | 10 | 0.000486s | 0.000488s | 0.001010s | byte equality observed |
| mp_erase.test | 10 | 0.001087s | 0.001082s | 0.001024s | byte equality observed |
| mp_erase_2.test | 10 | 0.000493s | 0.000481s | 0.001024s | byte equality observed |
| mp_eval_if.test | 10 | 0.001056s | 0.001057s | 0.001027s | byte equality observed |
| mp_eval_if_not.test | 10 | 0.001032s | 0.001051s | 0.001013s | byte equality observed |
| mp_eval_if_sf.test | 10 | 0.001041s | 0.001047s | 0.001014s | byte equality observed |
| mp_eval_or.test | 10 | 0.001033s | 0.001026s | 0.001017s | byte equality observed |
| mp_fill.test | 10 | 0.001045s | 0.001064s | 0.001025s | byte equality observed |
| mp_fill_2.test | 10 | 0.000481s | 0.000476s | 0.001020s | byte equality observed |
| mp_filter.test | 10 | 0.001066s | 0.001050s | 0.001033s | byte equality observed |
| mp_find.test | 10 | 0.001065s | 0.001033s | 0.001005s | byte equality observed |
| mp_find_if.test | 10 | 0.001034s | 0.001032s | 0.001024s | byte equality observed |
| mp_find_if_q.test | 10 | 0.001032s | 0.001022s | 0.001045s | byte equality observed |
| mp_flatten.test | 10 | 0.001029s | 0.001019s | 0.001028s | byte equality observed |
| mp_fold.test | 10 | 0.001062s | 0.001022s | 0.001074s | byte equality observed |
| mp_fold_q.test | 10 | 0.001034s | 0.001043s | 0.001043s | byte equality observed |
| mp_fold_q_sf.test | 10 | 0.001034s | 0.001025s | 0.001019s | byte equality observed |
| mp_for_each.test | 10 | 0.001050s | 0.001013s | 0.001058s | byte equality observed |
| mp_from_sequence.test | 10 | 0.001045s | 0.001020s | 0.001028s | byte equality observed |
| mp_from_sequence_2.test | 10 | 0.001022s | 0.001035s | 0.001007s | byte equality observed |
| mp_from_sequence_3.test | 10 | 0.001063s | 0.001030s | 0.001020s | byte equality observed |
| mp_front.test | 10 | 0.001041s | 0.001038s | 0.001022s | byte equality observed |
| mp_front_2.test | 10 | 0.000490s | 0.000490s | 0.001012s | byte equality observed |
| mp_identity.test | 10 | 0.001033s | 0.001022s | 0.001054s | byte equality observed |
| mp_if.test | 10 | 0.001025s | 0.001029s | 0.001041s | byte equality observed |
| mp_if_sf.test | 10 | 0.001024s | 0.001041s | 0.001014s | byte equality observed |
| mp_inherit.test | 10 | 0.001030s | 0.001036s | 0.001073s | byte equality observed |
| mp_insert.test | 10 | 0.001052s | 0.001091s | 0.001037s | byte equality observed |
| mp_insert_2.test | 10 | 0.000488s | 0.000483s | 0.001039s | byte equality observed |
| mp_intersperse.test | 10 | 0.001031s | 0.001046s | 0.001013s | byte equality observed |
| mp_invoke_q.test | 10 | 0.001033s | 0.001060s | 0.001052s | byte equality observed |
| mp_invoke_q_sf.test | 10 | 0.001018s | 0.001060s | 0.001051s | byte equality observed |
| mp_iota.test | 10 | 0.001032s | 0.001045s | 0.001034s | byte equality observed |
| mp_iota_2.test | 10 | 0.001048s | 0.001036s | 0.001025s | byte equality observed |
| mp_is_list.test | 10 | 0.001041s | 0.001038s | 0.001055s | byte equality observed |
| mp_is_map.test | 10 | 0.001043s | 0.001040s | 0.001035s | byte equality observed |
| mp_is_set.test | 10 | 0.001028s | 0.001033s | 0.001034s | byte equality observed |
| mp_is_value_list.test | 10 | 0.001027s | 0.001043s | 0.001026s | byte equality observed |
| mp_iterate.test | 10 | 0.001025s | 0.001063s | 0.001005s | byte equality observed |
| mp_join.test | 10 | 0.001058s | 0.001059s | 0.001056s | byte equality observed |
| mp_lambda.test | 10 | 0.001054s | 0.001041s | 0.001058s | byte equality observed |
| mp_less.test | 10 | 0.001028s | 0.001042s | 0.001055s | byte equality observed |
| mp_list_c.test | 10 | 0.001038s | 0.001043s | 0.001035s | byte equality observed |
| mp_list_v.test | 10 | 0.000485s | 0.000483s | 0.001052s | byte equality observed |
| mp_map_contains.test | 10 | 0.001053s | 0.001043s | 0.001018s | byte equality observed |
| mp_map_erase.test | 10 | 0.001038s | 0.001074s | 0.001056s | byte equality observed |
| mp_map_find.test | 10 | 0.001035s | 0.001059s | 0.001023s | byte equality observed |
| mp_map_find_2.test | 10 | 0.001082s | 0.001018s | 0.001048s | byte equality observed |
| mp_map_find_3.test | 10 | 0.001040s | 0.001018s | 0.001066s | byte equality observed |
| mp_map_find_4.test | 10 | 0.001042s | 0.001021s | 0.001030s | byte equality observed |
| mp_map_insert.test | 10 | 0.001049s | 0.001041s | 0.001014s | byte equality observed |
| mp_map_keys.test | 10 | 0.001041s | 0.001033s | 0.001010s | byte equality observed |
| mp_map_replace.test | 10 | 0.001070s | 0.001041s | 0.001037s | byte equality observed |
| mp_map_update.test | 10 | 0.001003s | 0.001041s | 0.001029s | byte equality observed |
| mp_map_update_q.test | 10 | 0.001045s | 0.001038s | 0.001016s | byte equality observed |
| mp_max.test | 10 | 0.001035s | 0.001067s | 0.001074s | byte equality observed |
| mp_max_element.test | 10 | 0.001041s | 0.001065s | 0.001030s | byte equality observed |
| mp_max_element_2.test | 10 | 0.000477s | 0.000489s | 0.001009s | byte equality observed |
| mp_max_element_q.test | 10 | 0.001046s | 0.001055s | 0.001042s | byte equality observed |
| mp_max_element_q_2.test | 10 | 0.000478s | 0.000482s | 0.001033s | byte equality observed |
| mp_min.test | 10 | 0.001037s | 0.001038s | 0.001012s | byte equality observed |
| mp_min_element.test | 10 | 0.001040s | 0.001086s | 0.001069s | byte equality observed |
| mp_min_element_2.test | 10 | 0.000486s | 0.000490s | 0.000998s | byte equality observed |
| mp_min_element_q.test | 10 | 0.001056s | 0.001066s | 0.001022s | byte equality observed |
| mp_min_element_q_2.test | 10 | 0.000483s | 0.000475s | 0.001021s | byte equality observed |
| mp_none_of.test | 10 | 0.001020s | 0.001046s | 0.001055s | byte equality observed |
| mp_none_of_q.test | 5 | 0.001017s | 0.001022s | 0.001033s | byte equality observed |
| mp_not_fn.test | 10 | 0.001027s | 0.001022s | 0.001018s | byte equality observed |
| mp_nth_element.test | 10 | 0.001019s | 0.001040s | 0.001038s | byte equality observed |
| mp_nth_element_q.test | 10 | 0.001037s | 0.001027s | 0.001035s | byte equality observed |
| mp_or.test | 10 | 0.001033s | 0.001044s | 0.001029s | byte equality observed |
| mp_pairwise_fold.test | 10 | 0.001036s | 0.001062s | 0.001061s | byte equality observed |
| mp_pairwise_fold_q.test | 10 | 0.001024s | 0.001075s | 0.001049s | byte equality observed |
| mp_partial_sum.test | 10 | 0.001043s | 0.001077s | 0.001046s | byte equality observed |
| mp_partition.test | 10 | 0.001053s | 0.001048s | 0.001039s | byte equality observed |
| mp_partition_q.test | 10 | 0.001036s | 0.001072s | 0.001033s | byte equality observed |
| mp_plus.test | 10 | 0.001054s | 0.001029s | 0.001051s | byte equality observed |
| mp_pop_back.test | 10 | 0.001055s | 0.001086s | 0.001024s | byte equality observed |
| mp_pop_back_2.test | 10 | 0.000492s | 0.000482s | 0.001034s | byte equality observed |
| mp_pop_front.test | 10 | 0.001048s | 0.001039s | 0.001011s | byte equality observed |
| mp_pop_front_2.test | 10 | 0.000482s | 0.000479s | 0.001008s | byte equality observed |
| mp_power_set.test | 10 | 0.001028s | 0.001038s | 0.001029s | byte equality observed |
| mp_product.test | 10 | 0.001053s | 0.001041s | 0.001030s | byte equality observed |
| mp_push_back.test | 10 | 0.001056s | 0.001057s | 0.001033s | byte equality observed |
| mp_push_back_2.test | 10 | 0.000490s | 0.000484s | 0.001023s | byte equality observed |
| mp_push_front.test | 10 | 0.001068s | 0.001068s | 0.001019s | byte equality observed |
| mp_push_front_2.test | 10 | 0.000486s | 0.000486s | 0.001033s | byte equality observed |
| mp_quote.test | 10 | 0.001064s | 0.001043s | 0.001034s | byte equality observed |
| mp_quote_trait.test | 10 | 0.001037s | 0.001078s | 0.001028s | byte equality observed |
| mp_remove.test | 10 | 0.001038s | 0.001056s | 0.001050s | byte equality observed |
| mp_remove_if.test | 10 | 0.001070s | 0.001068s | 0.001056s | byte equality observed |
| mp_remove_if_q.test | 10 | 0.001027s | 0.001045s | 0.001027s | byte equality observed |
| mp_rename.test | 10 | 0.001055s | 0.001042s | 0.001030s | byte equality observed |
| mp_rename_2.test | 10 | 0.000489s | 0.000492s | 0.001006s | byte equality observed |
| mp_rename_v.test | 10 | 0.000496s | 0.000487s | 0.001030s | byte equality observed |
| mp_repeat.test | 10 | 0.001087s | 0.001069s | 0.001039s | byte equality observed |
| mp_repeat_2.test | 10 | 0.000482s | 0.000487s | 0.001020s | byte equality observed |
| mp_repeat_3.test | 10 | 0.001058s | 0.001035s | 0.001044s | byte equality observed |
| mp_replace.test | 10 | 0.001025s | 0.001045s | 0.001024s | byte equality observed |
| mp_replace_at.test | 10 | 0.001043s | 0.001037s | 0.001015s | byte equality observed |
| mp_replace_at_c.test | 10 | 0.001049s | 0.001085s | 0.001016s | byte equality observed |
| mp_replace_front.test | 10 | 0.001044s | 0.001064s | 0.001042s | byte equality observed |
| mp_replace_front_2.test | 10 | 0.000489s | 0.000497s | 0.001014s | byte equality observed |
| mp_replace_if.test | 10 | 0.001019s | 0.001054s | 0.001020s | byte equality observed |
| mp_replace_if_q.test | 10 | 0.001035s | 0.001050s | 0.001033s | byte equality observed |
| mp_replace_second.test | 10 | 0.001049s | 0.001055s | 0.001030s | byte equality observed |
| mp_replace_second_2.test | 10 | 0.000489s | 0.000482s | 0.001000s | byte equality observed |
| mp_replace_third.test | 10 | 0.001053s | 0.001046s | 0.001025s | byte equality observed |
| mp_replace_third_2.test | 10 | 0.000483s | 0.000476s | 0.001013s | byte equality observed |
| mp_reverse.test | 10 | 0.001049s | 0.001047s | 0.001054s | byte equality observed |
| mp_reverse_fold.test | 10 | 0.001078s | 0.001030s | 0.001038s | byte equality observed |
| mp_reverse_fold_q.test | 10 | 0.001029s | 0.001049s | 0.001033s | byte equality observed |
| mp_rotate_left.test | 10 | 0.001043s | 0.001061s | 0.001033s | byte equality observed |
| mp_rotate_right.test | 10 | 0.001052s | 0.001043s | 0.001027s | byte equality observed |
| mp_same.test | 10 | 0.001050s | 0.001040s | 0.001031s | byte equality observed |
| mp_second.test | 10 | 0.001034s | 0.001062s | 0.001038s | byte equality observed |
| mp_second_2.test | 10 | 0.000486s | 0.000489s | 0.001026s | byte equality observed |
| mp_set_contains.test | 10 | 0.001048s | 0.001063s | 0.001026s | byte equality observed |
| mp_set_difference.test | 10 | 0.001011s | 0.001060s | 0.001003s | byte equality observed |
| mp_set_difference_sf.test | 10 | 0.001029s | 0.001078s | 0.001028s | byte equality observed |
| mp_set_intersection.test | 10 | 0.001018s | 0.001039s | 0.001051s | byte equality observed |
| mp_set_intersection_sf.test | 10 | 0.001035s | 0.001013s | 0.001035s | byte equality observed |
| mp_set_push_back.test | 10 | 0.001032s | 0.001040s | 0.001052s | byte equality observed |
| mp_set_push_front.test | 10 | 0.001052s | 0.001026s | 0.001021s | byte equality observed |
| mp_set_union.test | 10 | 0.001055s | 0.001071s | 0.001041s | byte equality observed |
| mp_set_union_sf.test | 10 | 0.001020s | 0.001020s | 0.000994s | byte equality observed |
| mp_similar.test | 10 | 0.001008s | 0.001069s | 0.001000s | byte equality observed |
| mp_size.test | 10 | 0.001039s | 0.001058s | 0.001035s | byte equality observed |
| mp_size_2.test | 10 | 0.000479s | 0.000493s | 0.001019s | byte equality observed |
| mp_slice.test | 10 | 0.001066s | 0.001065s | 0.001049s | byte equality observed |
| mp_slice_2.test | 10 | 0.000478s | 0.000495s | 0.001003s | byte equality observed |
| mp_sliding_fold.test | 10 | 0.001070s | 0.001062s | 0.001068s | byte equality observed |
| mp_sliding_fold_q.test | 10 | 0.001077s | 0.001063s | 0.001021s | byte equality observed |
| mp_sort.test | 10 | 0.001060s | 0.001036s | 0.001044s | byte equality observed |
| mp_sort_q.test | 10 | 0.001036s | 0.001069s | 0.001036s | byte equality observed |
| mp_split.test | 10 | 0.001043s | 0.001055s | 0.001037s | byte equality observed |
| mp_starts_with.test | 10 | 0.001008s | 0.001032s | 0.001014s | byte equality observed |
| mp_starts_with_sf.test | 10 | 0.001031s | 0.001012s | 0.001005s | byte equality observed |
| mp_take.test | 10 | 0.001070s | 0.001059s | 0.001001s | byte equality observed |
| mp_take_2.test | 10 | 0.000489s | 0.000489s | 0.001029s | byte equality observed |
| mp_third.test | 10 | 0.001053s | 0.001040s | 0.001014s | byte equality observed |
| mp_third_2.test | 10 | 0.000489s | 0.000494s | 0.001029s | byte equality observed |
| mp_transform.test | 10 | 0.001052s | 0.001062s | 0.001017s | byte equality observed |
| mp_transform_2.test | 10 | 0.001054s | 0.000998s | 0.001033s | byte equality observed |
| mp_transform_front.test | 10 | 0.001045s | 0.001060s | 0.001023s | byte equality observed |
| mp_transform_front_2.test | 10 | 0.000487s | 0.000489s | 0.001024s | byte equality observed |
| mp_transform_if.test | 10 | 0.001048s | 0.001072s | 0.001019s | byte equality observed |
| mp_transform_if_q.test | 10 | 0.001039s | 0.001031s | 0.001037s | byte equality observed |
| mp_transform_q.test | 10 | 0.001059s | 0.001028s | 0.001021s | byte equality observed |
| mp_transform_q_2.test | 10 | 0.001000s | 0.001002s | 0.001019s | byte equality observed |
| mp_transform_second.test | 10 | 0.001040s | 0.001054s | 0.001057s | byte equality observed |
| mp_transform_second_2.test | 10 | 0.000485s | 0.000484s | 0.001037s | byte equality observed |
| mp_transform_sf.test | 10 | 0.001066s | 0.001023s | 0.001016s | byte equality observed |
| mp_transform_third.test | 10 | 0.001022s | 0.001067s | 0.001024s | byte equality observed |
| mp_transform_third_2.test | 10 | 0.000479s | 0.000486s | 0.000988s | byte equality observed |
| mp_unique.test | 10 | 0.001059s | 0.001033s | 0.001048s | byte equality observed |
| mp_unique_if.test | 10 | 0.001069s | 0.001036s | 0.001037s | byte equality observed |
| mp_unique_if_q.test | 10 | 0.001068s | 0.001026s | 0.001043s | byte equality observed |
| mp_valid.test | 10 | 0.001066s | 0.001035s | 0.001083s | byte equality observed |
| mp_valid_and_true.test | 10 | 0.001040s | 0.001055s | 0.001014s | byte equality observed |
| mp_value.test | 10 | 0.000481s | 0.000483s | 0.001026s | byte equality observed |
| mp_with_index.test | 10 | 0.001390s | 0.001379s | 0.001296s | byte equality observed |
| mp_with_index_cx.test | 10 | 0.000482s | 0.000478s | 0.001009s | byte equality observed |
| mpl.test | 10 | 0.001037s | 0.001061s | 0.001041s | byte equality observed |
| mpl_list.test | 10 | 0.001023s | 0.001051s | 0.001027s | byte equality observed |
| mpl_tuple.test | 10 | 0.001026s | 0.001090s | 0.001038s | byte equality observed |
| mpl_version.test | 10 | 0.001034s | 0.001066s | 0.001055s | byte equality observed |
| set_version.test | 10 | 0.001040s | 0.001071s | 0.001040s | byte equality observed |
| tuple_apply.test | 10 | 0.001062s | 0.001055s | 0.001022s | byte equality observed |
| tuple_for_each.test | 10 | 0.001054s | 0.001032s | 0.001044s | byte equality observed |
| tuple_transform.test | 10 | 0.001057s | 0.001051s | 0.001052s | byte equality observed |
| tuple_transform_2.test | 10 | 0.001026s | 0.001037s | 0.001056s | byte equality observed |
| tuple_version.test | 10 | 0.001056s | 0.001018s | 0.001060s | byte equality observed |
| utility_version.test | 10 | 0.001059s | 0.001026s | 0.001030s | byte equality observed |
| version.test | 10 | 0.001045s | 0.001036s | 0.001055s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
