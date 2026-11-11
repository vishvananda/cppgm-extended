# libs/mp11/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| mp11.test | COMPILE | 0.051173 | 0.103880 | 0.139302 | — | — | — |
| complex_h.test | COMPILE | 0.205060 | 0.401724 | 0.576212 | — | — | — |
| mp_size.test | RUN | 0.236233 | 1.251509 | 0.715122 | 0.001687 | 0.001700 | 0.001530 |
| mp_size_2.test | RUN | 0.021253 | 0.043454 | 0.062454 | 0.000735 | 0.000728 | 0.001552 |
| mp_empty.test | RUN | 0.222326 | 0.789041 | 0.684010 | 0.001623 | 0.001758 | 0.001594 |
| mp_empty_2.test | RUN | 0.021192 | 0.045345 | 0.063036 | 0.000672 | 0.000643 | 0.001539 |
| mp_assign.test | RUN | 0.251617 | 1.618798 | 0.921707 | 0.001670 | 0.001859 | 0.001523 |
| mp_assign_2.test | RUN | 0.021463 | 0.044836 | 0.062214 | 0.000787 | 0.000780 | 0.001368 |
| mp_clear.test | RUN | 0.222391 | 0.720090 | 0.711010 | 0.001644 | 0.001590 | 0.001622 |
| mp_clear_2.test | RUN | 0.020748 | 0.044291 | 0.062333 | 0.000596 | 0.000557 | 0.001614 |
| mp_front.test | RUN | 0.235191 | 1.128532 | 0.769043 | 0.001591 | 0.001794 | 0.001463 |
| mp_front_2.test | RUN | 0.020877 | 0.045158 | 0.062591 | 0.000713 | 0.000765 | 0.001632 |
| mp_pop_front.test | RUN | 0.235681 | 1.199749 | 0.782746 | 0.001614 | 0.001646 | 0.001666 |
| mp_pop_front_2.test | RUN | 0.019453 | 0.045721 | 0.060130 | 0.000747 | 0.000802 | 0.001495 |
| mp_second.test | RUN | 0.226359 | 0.751295 | 0.732897 | 0.001584 | 0.001307 | 0.001732 |
| mp_second_2.test | RUN | 0.020802 | 0.045654 | 0.061796 | 0.000728 | 0.000773 | 0.001643 |
| mp_third.test | RUN | 0.231290 | 0.702295 | 0.689739 | 0.001692 | 0.001820 | 0.001530 |
| mp_third_2.test | RUN | 0.019817 | 0.043810 | 0.062320 | 0.000694 | 0.000776 | 0.001607 |
| mp_push_front.test | RUN | 0.326290 | 2.488612 | 1.222617 | 0.001655 | 0.001564 | 0.001670 |
| mp_push_front_2.test | RUN | 0.021279 | 0.045474 | 0.062640 | 0.000620 | 0.000681 | 0.001654 |
| mp_push_back.test | RUN | 0.337945 | 2.480533 | 1.250115 | 0.001557 | 0.001597 | 0.001754 |
| mp_push_back_2.test | RUN | 0.021146 | 0.045228 | 0.062981 | 0.000706 | 0.000776 | 0.001670 |
| mp_rename.test | RUN | 0.274250 | 2.258621 | 1.092563 | 0.001488 | 0.001571 | 0.001457 |
| mp_rename_2.test | RUN | 0.019370 | 0.044789 | 0.064105 | 0.000760 | 0.000805 | 0.001633 |
| mp_append.test | RUN | 0.280206 | 2.582434 | 1.172108 | 0.001657 | 0.001557 | 0.001602 |
| mp_append_2.test | RUN | 0.550413 | 5.754285 | 1.538271 | 0.001459 | 0.001768 | 0.001434 |
| mp_append_3.test | RUN | 0.021372 | 0.044642 | 0.061200 | 0.000646 | 0.000781 | 0.001627 |
| mp_append_sf.test | RUN | 0.216332 | 0.624043 | 0.645514 | 0.001576 | 0.001695 | 0.001617 |
| mp_replace_front.test | RUN | 0.264946 | 1.841963 | 0.963806 | 0.001598 | 0.001737 | 0.001541 |
| mp_replace_front_2.test | RUN | 0.021238 | 0.045538 | 0.064411 | 0.000579 | 0.000667 | 0.001618 |
| mp_replace_second.test | RUN | 0.244461 | 1.180450 | 0.907955 | 0.001565 | 0.001662 | 0.001658 |
| mp_replace_second_2.test | RUN | 0.021259 | 0.044394 | 0.064295 | 0.000567 | 0.000798 | 0.001375 |
| mp_replace_third.test | RUN | 0.250025 | 0.980911 | 0.798445 | 0.001762 | 0.001691 | 0.001709 |
| mp_replace_third_2.test | RUN | 0.021096 | 0.043044 | 0.062637 | 0.000767 | 0.000769 | 0.001675 |
| mp_apply_q.test | RUN | 0.265532 | 2.253403 | 1.075716 | 0.001789 | 0.001708 | 0.001671 |
| mp_apply_q_sf.test | RUN | 0.221837 | 0.656353 | 0.665291 | 0.001723 | 0.001668 | 0.001662 |
| mp_is_list.test | RUN | 0.219275 | 0.780384 | 0.701174 | 0.001476 | 0.001670 | 0.001706 |
| mp_list_c.test | RUN | 0.236627 | 0.938841 | 0.762667 | 0.001618 | 0.001736 | 0.001548 |
| mp_transform_front.test | RUN | 0.253855 | 1.847280 | 0.943277 | 0.001736 | 0.001706 | 0.001419 |
| mp_transform_front_2.test | RUN | 0.021571 | 0.045472 | 0.058758 | 0.000770 | 0.000631 | 0.001576 |
| mp_transform_second.test | RUN | 0.250079 | 1.569072 | 0.840623 | 0.001636 | 0.001525 | 0.001536 |
| mp_transform_second_2.test | RUN | 0.020568 | 0.044642 | 0.062457 | 0.000717 | 0.000800 | 0.001437 |
| mp_transform_third.test | RUN | 0.234882 | 1.201730 | 0.766771 | 0.001570 | 0.001502 | 0.001618 |
| mp_transform_third_2.test | RUN | 0.021360 | 0.044366 | 0.061746 | 0.000597 | 0.000774 | 0.001496 |
| mp_list_v.test | RUN | 0.021312 | 0.044460 | 0.061182 | 0.000641 | 0.000668 | 0.001405 |
| mp_rename_v.test | RUN | 0.021375 | 0.045038 | 0.062193 | 0.000588 | 0.000795 | 0.001597 |
| mp_is_value_list.test | RUN | 0.219561 | 0.490104 | 0.648376 | 0.001577 | 0.001680 | 0.001619 |
| mp_transform.test | RUN | 0.461375 | 3.839164 | 1.946225 | 0.001476 | 0.001587 | 0.001606 |
| mp_transform_2.test | RUN | 0.214767 | 0.455401 | 0.597190 | 0.001614 | 0.001699 | 0.001520 |
| mp_transform_q.test | RUN | 0.449142 | 3.677561 | 1.869636 | 0.001490 | 0.001581 | 0.001630 |
| mp_transform_q_2.test | RUN | 0.510585 | 1.182375 | 2.460969 | 0.001632 | 0.001577 | 0.001473 |
| mp_transform_sf.test | RUN | 0.234130 | 0.670020 | 0.713063 | 0.001665 | 0.001561 | 0.001656 |
| mp_transform_if.test | RUN | 0.287589 | 1.845582 | 1.044803 | 0.001676 | 0.001414 | 0.001684 |
| mp_transform_if_q.test | RUN | 0.278794 | 1.842097 | 1.041308 | 0.001537 | 0.001397 | 0.001616 |
| mp_filter.test | RUN | 0.254752 | 1.090038 | 0.808620 | 0.001596 | 0.001744 | 0.001544 |
| mp_fill.test | RUN | 0.239748 | 0.826161 | 0.748291 | 0.001747 | 0.001707 | 0.001686 |
| mp_fill_2.test | RUN | 0.036657 | 0.070553 | 0.104985 | 0.000702 | 0.000641 | 0.001608 |
| mp_count.test | RUN | 0.448388 | 2.010017 | 1.210152 | 0.001671 | 0.001874 | 0.001632 |
| mp_count_if.test | RUN | 0.448700 | 2.047775 | 1.362998 | 0.001699 | 0.001436 | 0.001567 |
| mp_count_if_q.test | RUN | 0.254455 | 1.323656 | 0.758670 | 0.001427 | 0.001514 | 0.001558 |
| mp_contains.test | RUN | 0.235388 | 0.798583 | 0.704205 | 0.001505 | 0.001592 | 0.001414 |
| mp_starts_with.test | RUN | 0.235272 | 0.792840 | 0.711461 | 0.001481 | 0.001749 | 0.001718 |
| mp_starts_with_sf.test | RUN | 0.229322 | 0.680755 | 0.690976 | 0.001530 | 0.001500 | 0.001173 |
| mp_repeat.test | RUN | 0.356699 | 3.269026 | 1.322969 | 0.001728 | 0.001672 | 0.001658 |
| mp_repeat_2.test | RUN | 0.036700 | 0.072163 | 0.105736 | 0.000645 | 0.000778 | 0.001623 |
| mp_repeat_3.test | RUN | 0.278444 | 0.435359 | 0.700972 | 0.001539 | 0.001757 | 0.001707 |
| mp_product.test | RUN | 0.269310 | 1.994037 | 0.998732 | 0.001363 | 0.001715 | 0.001689 |
| mp_drop.test | RUN | 0.286081 | 2.462919 | 1.149790 | 0.001573 | 0.001639 | 0.001642 |
| mp_drop_2.test | RUN | 0.035706 | 0.071483 | 0.136522 | 0.000605 | 0.000809 | 0.001469 |
| mp_iota.test | RUN | 0.275724 | 2.089310 | 0.978594 | 0.001554 | 0.001729 | 0.001630 |
| mp_iota_2.test | RUN | 0.300311 | 2.510859 | 1.114803 | 0.001459 | 0.001774 | 0.001690 |
| mp_at.test | RUN | 0.246480 | 1.086470 | 0.813513 | 0.001482 | 0.001576 | 0.001578 |
| mp_at_2.test | RUN | 0.037934 | 0.071184 | 0.108382 | 0.000652 | 0.000777 | 0.001662 |
| mp_at_sf.test | RUN | 0.230113 | 0.674328 | 0.683264 | 0.001535 | 0.001716 | 0.001660 |
| mp_take.test | RUN | 0.357302 | 3.215229 | 1.316845 | 0.001658 | 0.001757 | 0.001630 |
| mp_take_2.test | RUN | 0.037868 | 0.072505 | 0.109359 | 0.000689 | 0.000658 | 0.001578 |
| mp_slice.test | RUN | 0.397912 | 3.270402 | 1.700381 | 0.001668 | 0.001738 | 0.001654 |
| mp_slice_2.test | RUN | 0.037672 | 0.070755 | 0.108791 | 0.000717 | 0.000790 | 0.001562 |
| mp_replace.test | RUN | 0.280546 | 1.673089 | 1.090499 | 0.001706 | 0.001730 | 0.001651 |
| mp_replace_if.test | RUN | 0.265564 | 1.477394 | 1.005882 | 0.001683 | 0.001726 | 0.001486 |
| mp_replace_if_q.test | RUN | 0.269968 | 1.462341 | 1.024679 | 0.001579 | 0.001778 | 0.001675 |
| mp_copy_if.test | RUN | 0.275635 | 2.080067 | 0.968654 | 0.001607 | 0.001716 | 0.001645 |
| mp_copy_if_q.test | RUN | 0.278655 | 2.086316 | 0.969797 | 0.001755 | 0.001624 | 0.001673 |
| mp_remove.test | RUN | 0.264973 | 1.382429 | 0.996427 | 0.001641 | 0.001761 | 0.001627 |
| mp_remove_if.test | RUN | 0.280854 | 2.185145 | 1.009054 | 0.001500 | 0.001747 | 0.001639 |
| mp_remove_if_q.test | RUN | 0.283870 | 2.180193 | 1.030194 | 0.001972 | 0.001654 | 0.001592 |
| mp_partition.test | RUN | 0.279317 | 2.052638 | 1.098429 | 0.001744 | 0.001586 | 0.001492 |
| mp_partition_q.test | RUN | 0.281848 | 2.046847 | 1.111099 | 0.001698 | 0.001774 | 0.001647 |
| mp_sort.test | RUN | 0.264516 | 1.472732 | 0.963343 | 0.001559 | 0.001724 | 0.001683 |
| mp_sort_q.test | RUN | 0.265978 | 1.470950 | 0.973041 | 0.001675 | 0.001707 | 0.001689 |
| mp_find.test | RUN | 0.264465 | 1.591195 | 0.844137 | 0.001698 | 0.001709 | 0.001601 |
| mp_find_if.test | RUN | 0.254962 | 1.457205 | 0.810462 | 0.001678 | 0.001705 | 0.001611 |
| mp_find_if_q.test | RUN | 0.278812 | 1.625369 | 0.979612 | 0.001698 | 0.001784 | 0.001664 |
| mp_reverse.test | RUN | 0.493401 | 4.277934 | 1.857652 | 0.001617 | 0.001730 | 0.001715 |
| mp_fold.test | RUN | 0.844698 | 3.312249 | 4.716054 | 0.001600 | 0.001656 | 0.001735 |
| mp_fold_q.test | RUN | 0.262347 | 1.437642 | 0.909912 | 0.001653 | 0.001677 | 0.001642 |
| mp_fold_q_sf.test | RUN | 0.238296 | 0.688770 | 0.737308 | 0.001700 | 0.001631 | 0.001719 |
| mp_reverse_fold.test | RUN | 0.414195 | 3.194651 | 1.328341 | 0.001659 | 0.001729 | 0.001525 |
| mp_reverse_fold_q.test | RUN | 0.268231 | 1.753512 | 0.979989 | 0.001692 | 0.001648 | 0.001706 |
| mp_unique.test | RUN | 0.267969 | 1.602096 | 0.963746 | 0.001550 | 0.001770 | 0.001646 |
| mp_unique_if.test | RUN | 0.438764 | 3.338041 | 1.544651 | 0.001575 | 0.001577 | 0.001773 |
| mp_unique_if_q.test | RUN | 0.382253 | 3.338709 | 1.212038 | 0.001480 | 0.001685 | 0.001602 |
| mp_all_of.test | RUN | 0.571560 | 1.873988 | 1.691799 | 0.001695 | 0.001658 | 0.001736 |
| mp_all_of_q.test | RUN | 0.237212 | 0.804504 | 0.702009 | 0.001657 | 0.001722 | 0.001688 |
| mp_any_of.test | RUN | 0.595927 | 1.896866 | 1.682831 | 0.001496 | 0.001686 | 0.001654 |
| mp_any_of_q.test | RUN | 0.233589 | 0.771863 | 0.709872 | 0.001716 | 0.001730 | 0.001563 |
| mp_none_of.test | RUN | 0.569007 | 1.856183 | 1.676991 | 0.001527 | 0.001715 | 0.001604 |
| mp_none_of_q.test | RUN | 0.230599 | 0.782482 | 0.719741 | 0.001658 | 0.001744 | 0.001590 |
| mp_replace_at.test | RUN | 0.279984 | 1.614310 | 1.064731 | 0.001709 | 0.001738 | 0.001688 |
| mp_replace_at_c.test | RUN | 0.274005 | 1.636692 | 1.043263 | 0.001216 | 0.001646 | 0.001487 |
| mp_for_each.test | RUN | 0.263632 | 1.190675 | 1.211347 | 0.001527 | 0.001633 | 0.001564 |
| mp_insert.test | RUN | 0.388383 | 3.257135 | 1.552871 | 0.001574 | 0.001754 | 0.001708 |
| mp_insert_2.test | RUN | 0.037666 | 0.072561 | 0.109829 | 0.000573 | 0.000501 | 0.001690 |
| mp_erase.test | RUN | 0.424085 | 3.427980 | 1.785777 | 0.001684 | 0.001712 | 0.001717 |
| mp_erase_2.test | RUN | 0.037959 | 0.070898 | 0.107919 | 0.000639 | 0.000758 | 0.001466 |
| mp_with_index.test | RUN | 0.599208 | 7.132431 | 4.026024 | 0.002287 | 0.001856 | 0.002010 |
| mp_with_index_cx.test | RUN | 0.003524 | 0.020132 | 0.026300 | 0.000740 | 0.000765 | 0.001632 |
| mp_from_sequence.test | RUN | 0.279970 | 1.854423 | 1.037053 | 0.001588 | 0.001695 | 0.001594 |
| mp_from_sequence_2.test | RUN | 0.290167 | 2.183231 | 1.147376 | 0.001712 | 0.001678 | 0.001656 |
| mp_from_sequence_3.test | RUN | 0.243229 | 0.966387 | 0.832472 | 0.001671 | 0.001757 | 0.001525 |
| mp_min_element.test | RUN | 0.242380 | 0.892356 | 0.756373 | 0.001656 | 0.001536 | 0.001656 |
| mp_min_element_2.test | RUN | 0.037047 | 0.071662 | 0.109421 | 0.000588 | 0.000802 | 0.001322 |
| mp_min_element_q.test | RUN | 0.235390 | 0.876437 | 0.729462 | 0.001678 | 0.001704 | 0.001712 |
| mp_min_element_q_2.test | RUN | 0.036295 | 0.072535 | 0.107459 | 0.000603 | 0.000757 | 0.001639 |
| mp_max_element.test | RUN | 0.234109 | 0.867311 | 0.745641 | 0.001732 | 0.001712 | 0.001654 |
| mp_max_element_2.test | RUN | 0.037567 | 0.070156 | 0.108859 | 0.000621 | 0.000751 | 0.001681 |
| mp_max_element_q.test | RUN | 0.235150 | 0.870397 | 0.736140 | 0.001722 | 0.001736 | 0.001605 |
| mp_max_element_q_2.test | RUN | 0.037015 | 0.070692 | 0.109161 | 0.000709 | 0.000795 | 0.001613 |
| mp_nth_element.test | RUN | 0.260754 | 1.211853 | 0.914715 | 0.001734 | 0.001697 | 0.001593 |
| mp_nth_element_q.test | RUN | 0.262607 | 1.207952 | 0.904833 | 0.001607 | 0.001509 | 0.001563 |
| mp_back.test | RUN | 0.258793 | 1.018741 | 0.835224 | 0.001451 | 0.001648 | 0.001624 |
| mp_back_2.test | RUN | 0.036848 | 0.070595 | 0.110162 | 0.000720 | 0.000629 | 0.001634 |
| mp_pop_back.test | RUN | 0.616222 | 6.608721 | 1.748401 | 0.001682 | 0.001755 | 0.001493 |
| mp_pop_back_2.test | RUN | 0.036995 | 0.070108 | 0.107149 | 0.000713 | 0.000772 | 0.001468 |
| mp_flatten.test | RUN | 0.297334 | 2.340893 | 1.193768 | 0.001726 | 0.001674 | 0.001637 |
| mp_rotate_left.test | RUN | 0.292660 | 2.529692 | 1.182453 | 0.001619 | 0.001800 | 0.001468 |
| mp_rotate_right.test | RUN | 0.294919 | 2.523740 | 1.214271 | 0.001543 | 0.001772 | 0.001655 |
| mp_power_set.test | RUN | 0.292229 | 2.414260 | 1.184190 | 0.001571 | 0.001664 | 0.001628 |
| mp_partial_sum.test | RUN | 0.362832 | 3.235159 | 1.350795 | 0.001498 | 0.001738 | 0.001513 |
| mp_iterate.test | RUN | 0.266352 | 0.974340 | 0.940613 | 0.001502 | 0.001780 | 0.001756 |
| mp_pairwise_fold.test | RUN | 0.297704 | 2.211594 | 1.208167 | 0.001512 | 0.001772 | 0.001527 |
| mp_pairwise_fold_q.test | RUN | 0.231672 | 0.661470 | 0.697987 | 0.001744 | 0.001600 | 0.001622 |
| mp_sliding_fold.test | RUN | 0.514293 | 4.255419 | 2.218556 | 0.001713 | 0.001758 | 0.001490 |
| mp_sliding_fold_q.test | RUN | 0.506036 | 4.263052 | 2.220456 | 0.001515 | 0.001738 | 0.001644 |
| mp_intersperse.test | RUN | 0.279768 | 1.586118 | 1.052023 | 0.001543 | 0.001726 | 0.001644 |
| mp_split.test | RUN | 0.365998 | 3.064632 | 1.465159 | 0.001728 | 0.001734 | 0.001610 |
| mp_join.test | RUN | 0.353719 | 2.577051 | 1.325963 | 0.001595 | 0.001606 | 0.001673 |
| integral.test | RUN | 0.239948 | 1.158491 | 0.804669 | 0.001510 | 0.001751 | 0.001654 |
| mp_value.test | RUN | 0.016411 | 0.037893 | 0.050033 | 0.000650 | 0.000676 | 0.001560 |
| mp_identity.test | RUN | 0.232352 | 1.051316 | 0.764350 | 0.001570 | 0.001728 | 0.001577 |
| mp_inherit.test | RUN | 0.216133 | 0.747878 | 0.718467 | 0.001455 | 0.001741 | 0.001654 |
| mp_if.test | RUN | 0.224837 | 0.861467 | 0.706335 | 0.001629 | 0.001637 | 0.001680 |
| mp_if_sf.test | RUN | 0.214195 | 0.634090 | 0.642763 | 0.001470 | 0.001705 | 0.001741 |
| mp_eval_if.test | RUN | 0.226075 | 0.920706 | 0.738223 | 0.001696 | 0.001586 | 0.001662 |
| mp_eval_if_sf.test | RUN | 0.218536 | 0.655042 | 0.655346 | 0.001518 | 0.001548 | 0.001579 |
| mp_valid.test | RUN | 0.217961 | 0.665263 | 0.658895 | 0.001682 | 0.001606 | 0.001641 |
| mp_defer.test | RUN | 0.236026 | 1.104761 | 0.794809 | 0.001574 | 0.001741 | 0.001637 |
| mp_quote.test | RUN | 0.237817 | 1.127491 | 0.798453 | 0.001464 | 0.001763 | 0.001476 |
| mp_invoke_q.test | RUN | 0.256240 | 1.469842 | 0.907970 | 0.001484 | 0.001778 | 0.001460 |
| mp_invoke_q_sf.test | RUN | 0.218138 | 0.656407 | 0.650144 | 0.001714 | 0.001737 | 0.001645 |
| mp_quote_trait.test | RUN | 0.237104 | 1.239354 | 0.799070 | 0.001617 | 0.001679 | 0.001726 |
| mp_cond.test | RUN | 0.222448 | 0.763665 | 0.684607 | 0.001702 | 0.001742 | 0.001559 |
| mp_cond_sf.test | RUN | 0.218764 | 0.657265 | 0.660735 | 0.001574 | 0.001680 | 0.001365 |
| mp_not_fn.test | RUN | 0.215780 | 0.652435 | 0.646628 | 0.001674 | 0.001708 | 0.001529 |
| mp_eval_if_not.test | RUN | 0.227098 | 0.924255 | 0.733263 | 0.001739 | 0.001698 | 0.001621 |
| mp_eval_or.test | RUN | 0.218023 | 0.499986 | 0.681152 | 0.001639 | 0.001700 | 0.001595 |
| mp_compose.test | RUN | 0.246427 | 1.516013 | 0.867549 | 0.001494 | 0.001552 | 0.001633 |
| mp_valid_and_true.test | RUN | 0.222582 | 0.794046 | 0.686166 | 0.001521 | 0.001667 | 0.001454 |
| mp_compose_sf.test | COMPILE | 0.053739 | 0.089708 | 0.156308 | — | — | — |
| integer_sequence.test | RUN | 0.263093 | 1.493031 | 0.978250 | 0.001650 | 0.001642 | 0.001685 |
| tuple_for_each.test | RUN | 0.231549 | 0.578241 | 0.726885 | 0.001406 | 0.001571 | 0.001654 |
| tuple_for_each_cx.test | COMPILE | 0.042984 | 0.081631 | 0.116259 | — | — | — |
| tuple_apply.test | RUN | 0.233038 | 0.557116 | 0.725981 | 0.001442 | 0.001722 | 0.001577 |
| tuple_apply_cx.test | COMPILE | 0.064153 | 0.115158 | 0.172257 | — | — | — |
| construct_from_tuple.test | RUN | 0.232756 | 0.592491 | 0.721507 | 0.001507 | 0.001603 | 0.001655 |
| construct_from_tuple_cx.test | COMPILE | 0.063121 | 0.114520 | 0.161941 | — | — | — |
| tuple_transform.test | RUN | 0.355256 | 1.206299 | 1.408724 | 0.001692 | 0.001450 | 0.001430 |
| tuple_transform_2.test | RUN | 0.310591 | 0.907400 | 1.256177 | 0.001521 | 0.001759 | 0.001615 |
| tuple_transform_cx.test | COMPILE | 0.053941 | 0.105888 | 0.168621 | — | — | — |
| mp_set_contains.test | RUN | 0.228788 | 0.781496 | 0.705660 | 0.001660 | 0.001463 | 0.001657 |
| mp_set_push_back.test | RUN | 0.290034 | 2.415000 | 1.189689 | 0.001674 | 0.001616 | 0.001647 |
| mp_set_push_front.test | RUN | 0.358925 | 2.891426 | 1.360621 | 0.001470 | 0.001750 | 0.001580 |
| mp_is_set.test | RUN | 0.226870 | 0.794321 | 0.725765 | 0.001664 | 0.001701 | 0.001723 |
| mp_set_union.test | RUN | 0.288705 | 2.057479 | 1.134442 | 0.001695 | 0.001765 | 0.001512 |
| mp_set_union_sf.test | RUN | 0.223475 | 0.641575 | 0.667624 | 0.001521 | 0.001665 | 0.001675 |
| mp_set_difference.test | RUN | 0.260304 | 1.595046 | 0.962598 | 0.001684 | 0.001630 | 0.001530 |
| mp_set_difference_sf.test | RUN | 0.221349 | 0.638605 | 0.650286 | 0.001750 | 0.001678 | 0.001650 |
| mp_set_intersection.test | RUN | 0.260226 | 1.590776 | 0.965219 | 0.001703 | 0.001531 | 0.001588 |
| mp_set_intersection_sf.test | RUN | 0.225554 | 0.636990 | 0.652001 | 0.001533 | 0.001608 | 0.001636 |
| mp_all.test | RUN | 0.224528 | 0.786723 | 0.707329 | 0.001710 | 0.001657 | 0.001667 |
| mp_all_2.test | RUN | 0.581187 | 1.715465 | 1.675604 | 0.001646 | 0.001654 | 0.001594 |
| mp_and.test | RUN | 0.228030 | 0.771450 | 0.685936 | 0.001660 | 0.001664 | 0.001575 |
| mp_any.test | RUN | 0.225289 | 0.758148 | 0.725313 | 0.001512 | 0.001691 | 0.001649 |
| mp_any_2.test | RUN | 0.608544 | 1.703645 | 1.682044 | 0.001650 | 0.001709 | 0.001430 |
| mp_or.test | RUN | 0.230871 | 0.776979 | 0.699645 | 0.001732 | 0.001773 | 0.001614 |
| mp_same.test | RUN | 0.415737 | 1.316233 | 1.123306 | 0.001422 | 0.001654 | 0.001726 |
| mp_plus.test | RUN | 0.236705 | 0.916708 | 0.801560 | 0.001543 | 0.001569 | 0.001693 |
| mp_less.test | RUN | 0.226541 | 0.762676 | 0.716825 | 0.001562 | 0.001500 | 0.001396 |
| mp_min.test | RUN | 0.227117 | 0.715820 | 0.689166 | 0.001605 | 0.001722 | 0.001752 |
| mp_max.test | RUN | 0.231028 | 0.795471 | 0.711394 | 0.001532 | 0.001588 | 0.001633 |
| mp_similar.test | RUN | 0.228245 | 0.771270 | 0.702803 | 0.001475 | 0.001541 | 0.001614 |
| mp_map_find.test | RUN | 0.259985 | 1.424474 | 0.927533 | 0.001661 | 0.001649 | 0.001624 |
| mp_map_find_2.test | RUN | 0.246552 | 0.557028 | 0.794344 | 0.001697 | 0.001380 | 0.001744 |
| mp_map_find_3.test | RUN | 0.247004 | 0.556081 | 0.795963 | 0.001446 | 0.001496 | 0.001538 |
| mp_map_find_4.test | RUN | 0.234460 | 0.528815 | 0.719363 | 0.001474 | 0.001509 | 0.001674 |
| mp_map_contains.test | RUN | 0.238681 | 0.813503 | 0.741974 | 0.001748 | 0.001794 | 0.001623 |
| mp_map_insert.test | RUN | 0.360287 | 2.719864 | 1.298069 | 0.001544 | 0.001726 | 0.001529 |
| mp_map_replace.test | RUN | 0.387948 | 3.069278 | 1.489278 | 0.001694 | 0.001750 | 0.001531 |
| mp_map_erase.test | RUN | 0.301160 | 2.281807 | 1.242758 | 0.001692 | 0.001784 | 0.001538 |
| mp_map_update.test | RUN | 0.263229 | 1.486078 | 0.977364 | 0.001799 | 0.001716 | 0.001481 |
| mp_map_update_q.test | RUN | 0.267404 | 1.480911 | 0.956380 | 0.001771 | 0.001524 | 0.001501 |
| mp_map_keys.test | RUN | 0.265845 | 1.323159 | 0.929570 | 0.001524 | 0.001742 | 0.001633 |
| mp_is_map.test | RUN | 0.246920 | 0.800709 | 0.754124 | 0.001499 | 0.001744 | 0.001603 |
| mp_bind.test | RUN | 0.290146 | 2.367134 | 1.126225 | 0.001672 | 0.001803 | 0.001620 |
| mp_bind_q.test | RUN | 0.272172 | 1.475615 | 0.910144 | 0.001668 | 0.001695 | 0.001527 |
| mp_bind_front.test | RUN | 0.260671 | 1.629032 | 0.883021 | 0.001638 | 0.001634 | 0.001160 |
| mp_bind_back.test | RUN | 0.258421 | 1.621477 | 0.892665 | 0.001542 | 0.001692 | 0.001649 |
| mp_lambda.test | RUN | 0.518469 | 4.711365 | 2.332785 | 0.001691 | 0.001165 | 0.001489 |
| mpl.test | RUN | 0.507968 | 1.280214 | 1.859111 | 0.001776 | 0.001695 | 0.001405 |
| mpl_list.test | RUN | 0.314553 | 0.963038 | 0.872506 | 0.001722 | 0.001719 | 0.001631 |
| mpl_tuple.test | RUN | 0.305973 | 0.972756 | 0.896244 | 0.001574 | 0.001755 | 0.001686 |
| version.test | RUN | 0.150542 | 0.354142 | 0.452972 | 0.001642 | 0.001576 | 0.001559 |
| mp11_version.test | RUN | 0.188486 | 0.403778 | 0.539822 | 0.001576 | 0.001465 | 0.001520 |
| list_version.test | RUN | 0.160579 | 0.364139 | 0.463853 | 0.001462 | 0.001423 | 0.001696 |
| algorithm_version.test | RUN | 0.169612 | 0.384291 | 0.495992 | 0.001646 | 0.001534 | 0.001368 |
| integral_version.test | RUN | 0.152703 | 0.353691 | 0.447034 | 0.001548 | 0.001598 | 0.001406 |
| utility_version.test | RUN | 0.152615 | 0.359284 | 0.456777 | 0.001662 | 0.001409 | 0.001410 |
| function_version.test | RUN | 0.158753 | 0.361779 | 0.453869 | 0.001554 | 0.001553 | 0.001626 |
| map_version.test | RUN | 0.169353 | 0.381117 | 0.492484 | 0.001745 | 0.001590 | 0.001584 |
| set_version.test | RUN | 0.158927 | 0.362348 | 0.463875 | 0.001650 | 0.001459 | 0.001665 |
| bind_version.test | RUN | 0.173724 | 0.380632 | 0.491730 | 0.001324 | 0.001603 | 0.001572 |
| integer_sequence_version.test | RUN | 0.151881 | 0.353792 | 0.346355 | 0.001448 | 0.001771 | 0.001177 |
| tuple_version.test | RUN | 0.170869 | 0.381364 | 0.454567 | 0.001234 | 0.001175 | 0.001069 |
| mpl_version.test | RUN | 0.111335 | 0.275078 | 0.429840 | 0.001417 | 0.001171 | 0.001057 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.001× | 1.024×, 0.978× | 1.008× |
| clang | 1.100× | 1.102×, 1.097× | 1.004× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| algorithm_version.test | 10 | 0.000924s | 0.000934s | 0.000952s | byte equality observed |
| bind_version.test | 10 | 0.000927s | 0.000936s | 0.000937s | byte equality observed |
| construct_from_tuple.test | 10 | 0.000935s | 0.000946s | 0.000932s | byte equality observed |
| function_version.test | 10 | 0.000930s | 0.000926s | 0.000939s | byte equality observed |
| integer_sequence.test | 10 | 0.000933s | 0.000936s | 0.000933s | byte equality observed |
| integer_sequence_version.test | 10 | 0.000928s | 0.000932s | 0.000932s | byte equality observed |
| integral.test | 10 | 0.000929s | 0.000929s | 0.000927s | byte equality observed |
| integral_version.test | 10 | 0.000931s | 0.000931s | 0.000931s | byte equality observed |
| list_version.test | 10 | 0.000929s | 0.000952s | 0.000937s | byte equality observed |
| map_version.test | 10 | 0.000928s | 0.000923s | 0.000932s | byte equality observed |
| mp11_version.test | 10 | 0.000930s | 0.000925s | 0.000933s | byte equality observed |
| mp_all.test | 10 | 0.000926s | 0.000942s | 0.000941s | byte equality observed |
| mp_all_2.test | 10 | 0.000932s | 0.000932s | 0.000929s | byte equality observed |
| mp_all_of.test | 10 | 0.000935s | 0.000936s | 0.000972s | byte equality observed |
| mp_all_of_q.test | 10 | 0.000934s | 0.000931s | 0.000991s | byte equality observed |
| mp_and.test | 10 | 0.000934s | 0.000941s | 0.001034s | byte equality observed |
| mp_any.test | 10 | 0.000932s | 0.000941s | 0.000987s | byte equality observed |
| mp_any_2.test | 10 | 0.000940s | 0.000933s | 0.001043s | byte equality observed |
| mp_any_of.test | 10 | 0.000940s | 0.000935s | 0.000977s | byte equality observed |
| mp_any_of_q.test | 10 | 0.000946s | 0.000938s | 0.001016s | byte equality observed |
| mp_append.test | 10 | 0.000941s | 0.000943s | 0.001010s | byte equality observed |
| mp_append_2.test | 10 | 0.000956s | 0.000947s | 0.000971s | byte equality observed |
| mp_append_3.test | 10 | 0.000437s | 0.000432s | 0.000968s | byte equality observed |
| mp_append_sf.test | 10 | 0.000939s | 0.000956s | 0.000989s | byte equality observed |
| mp_apply_q.test | 10 | 0.000941s | 0.000937s | 0.001005s | byte equality observed |
| mp_apply_q_sf.test | 10 | 0.000932s | 0.000938s | 0.000982s | byte equality observed |
| mp_assign.test | 10 | 0.000930s | 0.000932s | 0.000976s | byte equality observed |
| mp_assign_2.test | 10 | 0.000435s | 0.000434s | 0.000992s | byte equality observed |
| mp_at.test | 10 | 0.000931s | 0.000937s | 0.000979s | byte equality observed |
| mp_at_2.test | 10 | 0.000436s | 0.000438s | 0.001025s | byte equality observed |
| mp_at_sf.test | 10 | 0.000955s | 0.000978s | 0.000976s | byte equality observed |
| mp_back.test | 10 | 0.001023s | 0.000934s | 0.000976s | byte equality observed |
| mp_back_2.test | 10 | 0.000447s | 0.000438s | 0.000993s | byte equality observed |
| mp_bind.test | 10 | 0.001007s | 0.000956s | 0.000970s | byte equality observed |
| mp_bind_back.test | 10 | 0.001006s | 0.000953s | 0.001010s | byte equality observed |
| mp_bind_front.test | 10 | 0.001018s | 0.000967s | 0.001027s | byte equality observed |
| mp_bind_q.test | 10 | 0.000997s | 0.001022s | 0.000982s | byte equality observed |
| mp_clear.test | 10 | 0.001022s | 0.000995s | 0.000973s | byte equality observed |
| mp_clear_2.test | 10 | 0.000451s | 0.000450s | 0.000984s | byte equality observed |
| mp_compose.test | 10 | 0.001014s | 0.001033s | 0.000992s | byte equality observed |
| mp_cond.test | 10 | 0.001000s | 0.001011s | 0.001011s | byte equality observed |
| mp_cond_sf.test | 10 | 0.001010s | 0.001022s | 0.001036s | byte equality observed |
| mp_contains.test | 10 | 0.000987s | 0.000985s | 0.001007s | byte equality observed |
| mp_copy_if.test | 10 | 0.000976s | 0.000988s | 0.001009s | byte equality observed |
| mp_copy_if_q.test | 10 | 0.001011s | 0.000983s | 0.000988s | byte equality observed |
| mp_count.test | 10 | 0.000988s | 0.000987s | 0.000992s | byte equality observed |
| mp_count_if.test | 10 | 0.001016s | 0.001010s | 0.001038s | byte equality observed |
| mp_count_if_q.test | 10 | 0.000988s | 0.001032s | 0.000985s | byte equality observed |
| mp_defer.test | 10 | 0.000996s | 0.000994s | 0.000987s | byte equality observed |
| mp_drop.test | 10 | 0.001028s | 0.001030s | 0.001001s | byte equality observed |
| mp_drop_2.test | 10 | 0.000451s | 0.000459s | 0.000970s | byte equality observed |
| mp_empty.test | 10 | 0.001020s | 0.001014s | 0.000978s | byte equality observed |
| mp_empty_2.test | 10 | 0.000447s | 0.000443s | 0.000980s | byte equality observed |
| mp_erase.test | 10 | 0.001031s | 0.001017s | 0.000989s | byte equality observed |
| mp_erase_2.test | 10 | 0.000449s | 0.000448s | 0.000972s | byte equality observed |
| mp_eval_if.test | 10 | 0.001015s | 0.001017s | 0.000978s | byte equality observed |
| mp_eval_if_not.test | 10 | 0.001029s | 0.001018s | 0.001041s | byte equality observed |
| mp_eval_if_sf.test | 10 | 0.001011s | 0.000981s | 0.001003s | byte equality observed |
| mp_eval_or.test | 10 | 0.000992s | 0.001041s | 0.001021s | byte equality observed |
| mp_fill.test | 10 | 0.001020s | 0.001014s | 0.000997s | byte equality observed |
| mp_fill_2.test | 10 | 0.000442s | 0.000436s | 0.000957s | byte equality observed |
| mp_filter.test | 10 | 0.000997s | 0.000993s | 0.000984s | byte equality observed |
| mp_find.test | 10 | 0.000995s | 0.001031s | 0.001006s | byte equality observed |
| mp_find_if.test | 10 | 0.000999s | 0.001012s | 0.000993s | byte equality observed |
| mp_find_if_q.test | 10 | 0.000993s | 0.000993s | 0.000989s | byte equality observed |
| mp_flatten.test | 10 | 0.001022s | 0.001037s | 0.000969s | byte equality observed |
| mp_fold.test | 10 | 0.001020s | 0.001003s | 0.000969s | byte equality observed |
| mp_fold_q.test | 10 | 0.000997s | 0.000992s | 0.000968s | byte equality observed |
| mp_fold_q_sf.test | 10 | 0.000964s | 0.001047s | 0.001017s | byte equality observed |
| mp_for_each.test | 10 | 0.000996s | 0.001033s | 0.001030s | byte equality observed |
| mp_from_sequence.test | 10 | 0.000972s | 0.000993s | 0.001037s | byte equality observed |
| mp_from_sequence_2.test | 10 | 0.000978s | 0.000982s | 0.001025s | byte equality observed |
| mp_from_sequence_3.test | 10 | 0.000992s | 0.001004s | 0.001002s | byte equality observed |
| mp_front.test | 10 | 0.001003s | 0.001000s | 0.000975s | byte equality observed |
| mp_front_2.test | 10 | 0.000437s | 0.000447s | 0.000974s | byte equality observed |
| mp_identity.test | 10 | 0.000974s | 0.001027s | 0.000950s | byte equality observed |
| mp_if.test | 10 | 0.001028s | 0.001044s | 0.001058s | byte equality observed |
| mp_if_sf.test | 10 | 0.001003s | 0.000995s | 0.000991s | byte equality observed |
| mp_inherit.test | 10 | 0.000984s | 0.001012s | 0.000987s | byte equality observed |
| mp_insert.test | 10 | 0.001021s | 0.001045s | 0.000963s | byte equality observed |
| mp_insert_2.test | 10 | 0.000444s | 0.000442s | 0.001028s | byte equality observed |
| mp_intersperse.test | 10 | 0.000971s | 0.001000s | 0.001009s | byte equality observed |
| mp_invoke_q.test | 10 | 0.001002s | 0.001012s | 0.001015s | byte equality observed |
| mp_invoke_q_sf.test | 10 | 0.000996s | 0.000991s | 0.001008s | byte equality observed |
| mp_iota.test | 10 | 0.000978s | 0.001025s | 0.001001s | byte equality observed |
| mp_iota_2.test | 10 | 0.000979s | 0.001009s | 0.001009s | byte equality observed |
| mp_is_list.test | 10 | 0.001021s | 0.001031s | 0.001019s | byte equality observed |
| mp_is_map.test | 10 | 0.000963s | 0.000980s | 0.000987s | byte equality observed |
| mp_is_set.test | 10 | 0.000969s | 0.000975s | 0.001030s | byte equality observed |
| mp_is_value_list.test | 10 | 0.001003s | 0.001023s | 0.000988s | byte equality observed |
| mp_iterate.test | 10 | 0.000998s | 0.001018s | 0.001008s | byte equality observed |
| mp_join.test | 10 | 0.000993s | 0.001030s | 0.001007s | byte equality observed |
| mp_lambda.test | 10 | 0.001001s | 0.001027s | 0.000968s | byte equality observed |
| mp_less.test | 10 | 0.000985s | 0.001034s | 0.001039s | byte equality observed |
| mp_list_c.test | 10 | 0.001032s | 0.001030s | 0.000994s | byte equality observed |
| mp_list_v.test | 10 | 0.000442s | 0.000436s | 0.001062s | byte equality observed |
| mp_map_contains.test | 10 | 0.000998s | 0.001030s | 0.000973s | byte equality observed |
| mp_map_erase.test | 10 | 0.000987s | 0.001003s | 0.001019s | byte equality observed |
| mp_map_find.test | 10 | 0.001002s | 0.001027s | 0.000978s | byte equality observed |
| mp_map_find_2.test | 10 | 0.001001s | 0.001021s | 0.000964s | byte equality observed |
| mp_map_find_3.test | 10 | 0.001020s | 0.000993s | 0.000985s | byte equality observed |
| mp_map_find_4.test | 10 | 0.000967s | 0.000976s | 0.000971s | byte equality observed |
| mp_map_insert.test | 10 | 0.000994s | 0.001007s | 0.000953s | byte equality observed |
| mp_map_keys.test | 10 | 0.001018s | 0.001018s | 0.000981s | byte equality observed |
| mp_map_replace.test | 10 | 0.000980s | 0.000989s | 0.000992s | byte equality observed |
| mp_map_update.test | 10 | 0.000996s | 0.001017s | 0.000970s | byte equality observed |
| mp_map_update_q.test | 10 | 0.001020s | 0.001003s | 0.000979s | byte equality observed |
| mp_max.test | 10 | 0.000982s | 0.000971s | 0.000989s | byte equality observed |
| mp_max_element.test | 10 | 0.000997s | 0.001042s | 0.001019s | byte equality observed |
| mp_max_element_2.test | 10 | 0.000446s | 0.000441s | 0.000971s | byte equality observed |
| mp_max_element_q.test | 10 | 0.001008s | 0.001020s | 0.000966s | byte equality observed |
| mp_max_element_q_2.test | 10 | 0.000445s | 0.000444s | 0.001033s | byte equality observed |
| mp_min.test | 10 | 0.001007s | 0.001016s | 0.000973s | byte equality observed |
| mp_min_element.test | 10 | 0.000972s | 0.001018s | 0.000970s | byte equality observed |
| mp_min_element_2.test | 10 | 0.000450s | 0.000439s | 0.001025s | byte equality observed |
| mp_min_element_q.test | 10 | 0.000997s | 0.001033s | 0.000967s | byte equality observed |
| mp_min_element_q_2.test | 6 | 0.000436s | 0.000465s | 0.000998s | byte equality observed |
| mp_none_of.test | 10 | 0.000997s | 0.001021s | 0.001031s | byte equality observed |
| mp_none_of_q.test | 10 | 0.000983s | 0.001025s | 0.001004s | byte equality observed |
| mp_not_fn.test | 10 | 0.000984s | 0.000986s | 0.000985s | byte equality observed |
| mp_nth_element.test | 10 | 0.000985s | 0.001036s | 0.001022s | byte equality observed |
| mp_nth_element_q.test | 10 | 0.001002s | 0.001048s | 0.000965s | byte equality observed |
| mp_or.test | 10 | 0.001006s | 0.001017s | 0.000994s | byte equality observed |
| mp_pairwise_fold.test | 10 | 0.001003s | 0.000999s | 0.001027s | byte equality observed |
| mp_pairwise_fold_q.test | 10 | 0.000989s | 0.001033s | 0.000991s | byte equality observed |
| mp_partial_sum.test | 10 | 0.001011s | 0.001034s | 0.001019s | byte equality observed |
| mp_partition.test | 10 | 0.001014s | 0.001021s | 0.001010s | byte equality observed |
| mp_partition_q.test | 10 | 0.000994s | 0.001031s | 0.000996s | byte equality observed |
| mp_plus.test | 10 | 0.000991s | 0.000998s | 0.000987s | byte equality observed |
| mp_pop_back.test | 10 | 0.001003s | 0.001011s | 0.000984s | byte equality observed |
| mp_pop_back_2.test | 10 | 0.000444s | 0.000454s | 0.000961s | byte equality observed |
| mp_pop_front.test | 10 | 0.000987s | 0.001025s | 0.000990s | byte equality observed |
| mp_pop_front_2.test | 10 | 0.000449s | 0.000446s | 0.000962s | byte equality observed |
| mp_power_set.test | 10 | 0.000970s | 0.001046s | 0.000975s | byte equality observed |
| mp_product.test | 10 | 0.000991s | 0.001011s | 0.000970s | byte equality observed |
| mp_push_back.test | 10 | 0.001021s | 0.001034s | 0.000964s | byte equality observed |
| mp_push_back_2.test | 10 | 0.000437s | 0.000448s | 0.001010s | byte equality observed |
| mp_push_front.test | 10 | 0.001051s | 0.001039s | 0.000946s | byte equality observed |
| mp_push_front_2.test | 10 | 0.000435s | 0.000455s | 0.001000s | byte equality observed |
| mp_quote.test | 10 | 0.000972s | 0.000979s | 0.000965s | byte equality observed |
| mp_quote_trait.test | 10 | 0.000991s | 0.000999s | 0.001035s | byte equality observed |
| mp_remove.test | 10 | 0.000983s | 0.001008s | 0.001011s | byte equality observed |
| mp_remove_if.test | 10 | 0.001027s | 0.001028s | 0.000964s | byte equality observed |
| mp_remove_if_q.test | 10 | 0.000986s | 0.000987s | 0.000992s | byte equality observed |
| mp_rename.test | 10 | 0.001017s | 0.001004s | 0.000991s | byte equality observed |
| mp_rename_2.test | 10 | 0.000449s | 0.000437s | 0.000956s | byte equality observed |
| mp_rename_v.test | 10 | 0.000447s | 0.000455s | 0.000946s | byte equality observed |
| mp_repeat.test | 10 | 0.001021s | 0.001044s | 0.000979s | byte equality observed |
| mp_repeat_2.test | 10 | 0.000437s | 0.000445s | 0.001032s | byte equality observed |
| mp_repeat_3.test | 10 | 0.000987s | 0.001010s | 0.001004s | byte equality observed |
| mp_replace.test | 10 | 0.000988s | 0.001013s | 0.001000s | byte equality observed |
| mp_replace_at.test | 10 | 0.000985s | 0.001026s | 0.000979s | byte equality observed |
| mp_replace_at_c.test | 10 | 0.001021s | 0.001048s | 0.001006s | byte equality observed |
| mp_replace_front.test | 10 | 0.001006s | 0.001010s | 0.000967s | byte equality observed |
| mp_replace_front_2.test | 10 | 0.000438s | 0.000439s | 0.001058s | byte equality observed |
| mp_replace_if.test | 10 | 0.000984s | 0.001014s | 0.000991s | byte equality observed |
| mp_replace_if_q.test | 10 | 0.001008s | 0.001011s | 0.000954s | byte equality observed |
| mp_replace_second.test | 10 | 0.001007s | 0.001036s | 0.000987s | byte equality observed |
| mp_replace_second_2.test | 10 | 0.000435s | 0.000443s | 0.000994s | byte equality observed |
| mp_replace_third.test | 10 | 0.000992s | 0.001012s | 0.000978s | byte equality observed |
| mp_replace_third_2.test | 10 | 0.000444s | 0.000445s | 0.000982s | byte equality observed |
| mp_reverse.test | 10 | 0.000992s | 0.001003s | 0.001015s | byte equality observed |
| mp_reverse_fold.test | 10 | 0.001026s | 0.000999s | 0.000952s | byte equality observed |
| mp_reverse_fold_q.test | 10 | 0.001009s | 0.000980s | 0.000988s | byte equality observed |
| mp_rotate_left.test | 10 | 0.000998s | 0.000980s | 0.001031s | byte equality observed |
| mp_rotate_right.test | 10 | 0.001018s | 0.000989s | 0.001000s | byte equality observed |
| mp_same.test | 10 | 0.000983s | 0.001052s | 0.001043s | byte equality observed |
| mp_second.test | 10 | 0.000980s | 0.001027s | 0.000960s | byte equality observed |
| mp_second_2.test | 10 | 0.000438s | 0.000437s | 0.001023s | byte equality observed |
| mp_set_contains.test | 10 | 0.000973s | 0.000971s | 0.000976s | byte equality observed |
| mp_set_difference.test | 10 | 0.001028s | 0.000982s | 0.001013s | byte equality observed |
| mp_set_difference_sf.test | 10 | 0.001028s | 0.000978s | 0.001004s | byte equality observed |
| mp_set_intersection.test | 10 | 0.000990s | 0.000964s | 0.000975s | byte equality observed |
| mp_set_intersection_sf.test | 10 | 0.001010s | 0.001019s | 0.000989s | byte equality observed |
| mp_set_push_back.test | 10 | 0.000974s | 0.000983s | 0.001016s | byte equality observed |
| mp_set_push_front.test | 10 | 0.001016s | 0.001014s | 0.000987s | byte equality observed |
| mp_set_union.test | 10 | 0.000989s | 0.000987s | 0.001007s | byte equality observed |
| mp_set_union_sf.test | 10 | 0.000985s | 0.000999s | 0.001048s | byte equality observed |
| mp_similar.test | 10 | 0.001007s | 0.000992s | 0.001006s | byte equality observed |
| mp_size.test | 10 | 0.000976s | 0.001037s | 0.001000s | byte equality observed |
| mp_size_2.test | 10 | 0.000448s | 0.000438s | 0.000970s | byte equality observed |
| mp_slice.test | 10 | 0.001023s | 0.001035s | 0.000982s | byte equality observed |
| mp_slice_2.test | 10 | 0.000442s | 0.000460s | 0.001050s | byte equality observed |
| mp_sliding_fold.test | 10 | 0.001043s | 0.001002s | 0.001002s | byte equality observed |
| mp_sliding_fold_q.test | 10 | 0.000994s | 0.000993s | 0.001001s | byte equality observed |
| mp_sort.test | 10 | 0.001005s | 0.000988s | 0.000991s | byte equality observed |
| mp_sort_q.test | 10 | 0.000987s | 0.001034s | 0.001037s | byte equality observed |
| mp_split.test | 10 | 0.001017s | 0.001019s | 0.001041s | byte equality observed |
| mp_starts_with.test | 10 | 0.001016s | 0.001008s | 0.001013s | byte equality observed |
| mp_starts_with_sf.test | 10 | 0.000983s | 0.001034s | 0.001018s | byte equality observed |
| mp_take.test | 10 | 0.001011s | 0.001001s | 0.000973s | byte equality observed |
| mp_take_2.test | 10 | 0.000446s | 0.000443s | 0.001032s | byte equality observed |
| mp_third.test | 10 | 0.001011s | 0.001004s | 0.000967s | byte equality observed |
| mp_third_2.test | 10 | 0.000438s | 0.000429s | 0.001006s | byte equality observed |
| mp_transform.test | 10 | 0.000993s | 0.001014s | 0.000972s | byte equality observed |
| mp_transform_2.test | 10 | 0.000983s | 0.000985s | 0.001018s | byte equality observed |
| mp_transform_front.test | 10 | 0.001018s | 0.001029s | 0.000956s | byte equality observed |
| mp_transform_front_2.test | 10 | 0.000435s | 0.000443s | 0.001042s | byte equality observed |
| mp_transform_if.test | 10 | 0.000985s | 0.000994s | 0.000956s | byte equality observed |
| mp_transform_if_q.test | 10 | 0.001001s | 0.000997s | 0.001007s | byte equality observed |
| mp_transform_q.test | 10 | 0.001024s | 0.001029s | 0.000955s | byte equality observed |
| mp_transform_q_2.test | 10 | 0.000963s | 0.000953s | 0.001025s | byte equality observed |
| mp_transform_second.test | 10 | 0.001001s | 0.001016s | 0.001014s | byte equality observed |
| mp_transform_second_2.test | 10 | 0.000441s | 0.000454s | 0.001034s | byte equality observed |
| mp_transform_sf.test | 10 | 0.000974s | 0.000985s | 0.000965s | byte equality observed |
| mp_transform_third.test | 10 | 0.000980s | 0.001058s | 0.001018s | byte equality observed |
| mp_transform_third_2.test | 10 | 0.000445s | 0.000436s | 0.000957s | byte equality observed |
| mp_unique.test | 10 | 0.000990s | 0.001019s | 0.000958s | byte equality observed |
| mp_unique_if.test | 10 | 0.001026s | 0.001012s | 0.000971s | byte equality observed |
| mp_unique_if_q.test | 10 | 0.000994s | 0.001038s | 0.001058s | byte equality observed |
| mp_valid.test | 10 | 0.000999s | 0.000995s | 0.000974s | byte equality observed |
| mp_valid_and_true.test | 10 | 0.000994s | 0.001017s | 0.000967s | byte equality observed |
| mp_value.test | 10 | 0.000451s | 0.000443s | 0.001002s | byte equality observed |
| mp_with_index.test | 10 | 0.001370s | 0.001267s | 0.001227s | byte equality observed |
| mp_with_index_cx.test | 10 | 0.000437s | 0.000444s | 0.000961s | byte equality observed |
| mpl.test | 10 | 0.000995s | 0.001004s | 0.000992s | byte equality observed |
| mpl_list.test | 10 | 0.000981s | 0.001021s | 0.000988s | byte equality observed |
| mpl_tuple.test | 10 | 0.001012s | 0.001005s | 0.000971s | byte equality observed |
| mpl_version.test | 10 | 0.000988s | 0.001002s | 0.001031s | byte equality observed |
| set_version.test | 10 | 0.000988s | 0.001001s | 0.000990s | byte equality observed |
| tuple_apply.test | 10 | 0.001004s | 0.001032s | 0.000985s | byte equality observed |
| tuple_for_each.test | 10 | 0.000997s | 0.001025s | 0.000997s | byte equality observed |
| tuple_transform.test | 10 | 0.001011s | 0.000970s | 0.000975s | byte equality observed |
| tuple_transform_2.test | 10 | 0.001025s | 0.001001s | 0.000988s | byte equality observed |
| tuple_version.test | 10 | 0.001022s | 0.001035s | 0.001017s | byte equality observed |
| utility_version.test | 10 | 0.001013s | 0.001003s | 0.001004s | byte equality observed |
| version.test | 10 | 0.001021s | 0.000959s | 0.000978s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
