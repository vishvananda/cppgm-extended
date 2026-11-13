# libs/mp11/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| mp11.test | COMPILE | 0.036020 | 0.069021 | 0.114995 | — | — | — |
| complex_h.test | COMPILE | 0.146804 | 0.301701 | 0.504893 | — | — | — |
| mp_size.test | RUN | 0.169174 | 0.932647 | 0.617486 | 0.001678 | 0.001748 | 0.001408 |
| mp_size_2.test | RUN | 0.015211 | 0.035014 | 0.053513 | 0.000682 | 0.000742 | 0.001400 |
| mp_empty.test | RUN | 0.161095 | 0.595034 | 0.530359 | 0.001501 | 0.001712 | 0.001545 |
| mp_empty_2.test | RUN | 0.015885 | 0.037601 | 0.059302 | 0.000558 | 0.000696 | 0.001684 |
| mp_assign.test | RUN | 0.183133 | 1.217891 | 0.775963 | 0.001550 | 0.001788 | 0.001788 |
| mp_assign_2.test | RUN | 0.015905 | 0.037016 | 0.056515 | 0.000629 | 0.000751 | 0.001473 |
| mp_clear.test | RUN | 0.163632 | 0.539164 | 0.579375 | 0.001608 | 0.001681 | 0.001644 |
| mp_clear_2.test | RUN | 0.016261 | 0.037457 | 0.051897 | 0.000676 | 0.000587 | 0.001741 |
| mp_front.test | RUN | 0.176358 | 0.837819 | 0.640078 | 0.001624 | 0.001744 | 0.001676 |
| mp_front_2.test | RUN | 0.014814 | 0.036698 | 0.058446 | 0.000520 | 0.000649 | 0.001542 |
| mp_pop_front.test | RUN | 0.170741 | 0.873637 | 0.622377 | 0.001506 | 0.001755 | 0.001632 |
| mp_pop_front_2.test | RUN | 0.016344 | 0.037106 | 0.060006 | 0.000578 | 0.000654 | 0.001644 |
| mp_second.test | RUN | 0.163440 | 0.564862 | 0.564797 | 0.001443 | 0.001435 | 0.001595 |
| mp_second_2.test | RUN | 0.015442 | 0.037117 | 0.059625 | 0.000566 | 0.000781 | 0.001705 |
| mp_third.test | RUN | 0.159652 | 0.518737 | 0.562834 | 0.001556 | 0.001840 | 0.001749 |
| mp_third_2.test | RUN | 0.015742 | 0.033745 | 0.057667 | 0.000665 | 0.000667 | 0.001706 |
| mp_push_front.test | RUN | 0.207120 | 1.860167 | 0.938128 | 0.001620 | 0.001588 | 0.001713 |
| mp_push_front_2.test | RUN | 0.015112 | 0.034553 | 0.056141 | 0.000628 | 0.000697 | 0.001385 |
| mp_push_back.test | RUN | 0.215515 | 1.868259 | 1.017489 | 0.001694 | 0.001670 | 0.001749 |
| mp_push_back_2.test | RUN | 0.016094 | 0.037802 | 0.058963 | 0.000593 | 0.000803 | 0.001656 |
| mp_rename.test | RUN | 0.196972 | 1.717083 | 0.918988 | 0.001618 | 0.001826 | 0.001655 |
| mp_rename_2.test | RUN | 0.015661 | 0.036826 | 0.056358 | 0.000652 | 0.000763 | 0.001631 |
| mp_append.test | RUN | 0.207366 | 1.930847 | 0.917296 | 0.001481 | 0.001702 | 0.001753 |
| mp_append_2.test | RUN | 0.434878 | 4.315973 | 1.222420 | 0.001568 | 0.001813 | 0.001437 |
| mp_append_3.test | RUN | 0.016078 | 0.035712 | 0.059709 | 0.000651 | 0.000723 | 0.001586 |
| mp_append_sf.test | RUN | 0.161153 | 0.488827 | 0.572465 | 0.001674 | 0.001778 | 0.001565 |
| mp_replace_front.test | RUN | 0.196764 | 1.398291 | 0.747568 | 0.001279 | 0.001507 | 0.001443 |
| mp_replace_front_2.test | RUN | 0.015073 | 0.038187 | 0.062906 | 0.000694 | 0.000762 | 0.001467 |
| mp_replace_second.test | RUN | 0.185182 | 0.893423 | 0.739218 | 0.001669 | 0.001850 | 0.001734 |
| mp_replace_second_2.test | RUN | 0.015992 | 0.036632 | 0.058385 | 0.000590 | 0.000701 | 0.001410 |
| mp_replace_third.test | RUN | 0.180138 | 0.745291 | 0.665743 | 0.001805 | 0.001629 | 0.001746 |
| mp_replace_third_2.test | RUN | 0.015163 | 0.035596 | 0.050538 | 0.000779 | 0.000724 | 0.001668 |
| mp_apply_q.test | RUN | 0.203539 | 1.694806 | 0.859580 | 0.001594 | 0.001663 | 0.001437 |
| mp_apply_q_sf.test | RUN | 0.159905 | 0.512098 | 0.581493 | 0.001716 | 0.001707 | 0.001602 |
| mp_is_list.test | RUN | 0.164873 | 0.590335 | 0.575248 | 0.001524 | 0.001796 | 0.001448 |
| mp_list_c.test | RUN | 0.176084 | 0.713761 | 0.635082 | 0.001730 | 0.001749 | 0.001732 |
| mp_transform_front.test | RUN | 0.189093 | 1.375423 | 0.718979 | 0.001584 | 0.001572 | 0.001794 |
| mp_transform_front_2.test | RUN | 0.016373 | 0.037082 | 0.059548 | 0.000577 | 0.000761 | 0.001575 |
| mp_transform_second.test | RUN | 0.184322 | 1.186571 | 0.664866 | 0.001547 | 0.001385 | 0.001409 |
| mp_transform_second_2.test | RUN | 0.015926 | 0.035920 | 0.058740 | 0.000607 | 0.000780 | 0.001544 |
| mp_transform_third.test | RUN | 0.173108 | 0.898430 | 0.595908 | 0.001573 | 0.001728 | 0.001756 |
| mp_transform_third_2.test | RUN | 0.015283 | 0.036769 | 0.059937 | 0.000623 | 0.000674 | 0.001539 |
| mp_list_v.test | RUN | 0.015523 | 0.036073 | 0.056644 | 0.000665 | 0.000632 | 0.001563 |
| mp_rename_v.test | RUN | 0.015193 | 0.036715 | 0.059229 | 0.000619 | 0.000760 | 0.001268 |
| mp_is_value_list.test | RUN | 0.158046 | 0.374987 | 0.559032 | 0.001544 | 0.001532 | 0.001530 |
| mp_transform.test | RUN | 0.347200 | 2.895965 | 1.564678 | 0.001646 | 0.001604 | 0.001800 |
| mp_transform_2.test | RUN | 0.150583 | 0.336019 | 0.494657 | 0.001664 | 0.001832 | 0.001633 |
| mp_transform_q.test | RUN | 0.344691 | 2.761814 | 1.537900 | 0.001637 | 0.001756 | 0.001652 |
| mp_transform_q_2.test | RUN | 0.387565 | 0.891126 | 1.976349 | 0.001681 | 0.001557 | 0.001536 |
| mp_transform_sf.test | RUN | 0.171041 | 0.521075 | 0.582061 | 0.001704 | 0.001683 | 0.001728 |
| mp_transform_if.test | RUN | 0.209523 | 1.392250 | 0.860240 | 0.001682 | 0.001732 | 0.001704 |
| mp_transform_if_q.test | RUN | 0.207474 | 1.403236 | 0.872647 | 0.001542 | 0.001611 | 0.001665 |
| mp_filter.test | RUN | 0.189548 | 0.821119 | 0.673375 | 0.001652 | 0.001718 | 0.001509 |
| mp_fill.test | RUN | 0.182572 | 0.625796 | 0.632752 | 0.001484 | 0.001767 | 0.001751 |
| mp_fill_2.test | RUN | 0.027738 | 0.057698 | 0.084708 | 0.000704 | 0.000743 | 0.001768 |
| mp_count.test | RUN | 0.330687 | 1.510929 | 0.989159 | 0.001733 | 0.001642 | 0.001563 |
| mp_count_if.test | RUN | 0.345349 | 1.531814 | 1.108756 | 0.001604 | 0.001620 | 0.001672 |
| mp_count_if_q.test | RUN | 0.177173 | 0.982559 | 0.633678 | 0.001475 | 0.001754 | 0.001412 |
| mp_contains.test | RUN | 0.165902 | 0.608363 | 0.570825 | 0.001469 | 0.001692 | 0.001535 |
| mp_starts_with.test | RUN | 0.170125 | 0.594283 | 0.630573 | 0.001550 | 0.001859 | 0.001597 |
| mp_starts_with_sf.test | RUN | 0.164372 | 0.518759 | 0.599691 | 0.001629 | 0.001633 | 0.001616 |
| mp_repeat.test | RUN | 0.242259 | 2.470831 | 1.050777 | 0.001729 | 0.001884 | 0.001498 |
| mp_repeat_2.test | RUN | 0.027526 | 0.058320 | 0.097264 | 0.000627 | 0.000720 | 0.001740 |
| mp_repeat_3.test | RUN | 0.212285 | 0.333129 | 0.585189 | 0.001605 | 0.001752 | 0.001649 |
| mp_product.test | RUN | 0.197883 | 1.484276 | 0.843540 | 0.001331 | 0.001739 | 0.001674 |
| mp_drop.test | RUN | 0.209123 | 1.853710 | 0.935936 | 0.001611 | 0.001704 | 0.001755 |
| mp_drop_2.test | RUN | 0.028272 | 0.058955 | 0.086037 | 0.000574 | 0.000738 | 0.001618 |
| mp_iota.test | RUN | 0.205915 | 1.566703 | 0.815473 | 0.001497 | 0.001714 | 0.001637 |
| mp_iota_2.test | RUN | 0.216958 | 1.905849 | 0.925173 | 0.001555 | 0.001786 | 0.001730 |
| mp_at.test | RUN | 0.181861 | 0.839173 | 0.626119 | 0.001538 | 0.001816 | 0.001782 |
| mp_at_2.test | RUN | 0.027291 | 0.054629 | 0.097554 | 0.000617 | 0.000572 | 0.001678 |
| mp_at_sf.test | RUN | 0.168653 | 0.517294 | 0.552131 | 0.001627 | 0.001750 | 0.001308 |
| mp_take.test | RUN | 0.273521 | 2.436463 | 1.077173 | 0.001569 | 0.001680 | 0.001742 |
| mp_take_2.test | RUN | 0.027198 | 0.055027 | 0.098947 | 0.000712 | 0.000718 | 0.001560 |
| mp_slice.test | RUN | 0.316779 | 2.476334 | 1.374568 | 0.001748 | 0.001806 | 0.001414 |
| mp_slice_2.test | RUN | 0.027737 | 0.057821 | 0.098390 | 0.000643 | 0.000793 | 0.001605 |
| mp_replace.test | RUN | 0.206251 | 1.269659 | 0.907579 | 0.001691 | 0.001548 | 0.001666 |
| mp_replace_if.test | RUN | 0.205035 | 1.104321 | 0.841751 | 0.001663 | 0.001813 | 0.001436 |
| mp_replace_if_q.test | RUN | 0.201104 | 1.114587 | 0.851979 | 0.001363 | 0.001737 | 0.001726 |
| mp_copy_if.test | RUN | 0.209927 | 1.562781 | 0.812217 | 0.001738 | 0.001703 | 0.001433 |
| mp_copy_if_q.test | RUN | 0.203290 | 1.560349 | 0.767102 | 0.001756 | 0.001714 | 0.001486 |
| mp_remove.test | RUN | 0.199031 | 1.056556 | 0.837202 | 0.001774 | 0.001775 | 0.001748 |
| mp_remove_if.test | RUN | 0.208766 | 1.653986 | 0.870286 | 0.001517 | 0.001796 | 0.001686 |
| mp_remove_if_q.test | RUN | 0.210504 | 1.636007 | 0.839012 | 0.001763 | 0.001762 | 0.001828 |
| mp_partition.test | RUN | 0.206374 | 1.548061 | 0.906996 | 0.001784 | 0.001677 | 0.001682 |
| mp_partition_q.test | RUN | 0.214185 | 1.523102 | 0.925619 | 0.001713 | 0.001732 | 0.001788 |
| mp_sort.test | RUN | 0.191823 | 1.110849 | 0.739444 | 0.001521 | 0.001828 | 0.001710 |
| mp_sort_q.test | RUN | 0.200163 | 1.099186 | 0.787065 | 0.001758 | 0.001587 | 0.001785 |
| mp_find.test | RUN | 0.190660 | 1.191455 | 0.664853 | 0.001627 | 0.001672 | 0.001431 |
| mp_find_if.test | RUN | 0.186261 | 1.108531 | 0.658113 | 0.001629 | 0.001603 | 0.001596 |
| mp_find_if_q.test | RUN | 0.199942 | 1.236022 | 0.797969 | 0.001599 | 0.001787 | 0.001756 |
| mp_reverse.test | RUN | 0.393595 | 3.216460 | 1.487533 | 0.001686 | 0.001715 | 0.001636 |
| mp_fold.test | RUN | 0.658041 | 2.531957 | 3.769416 | 0.001411 | 0.001767 | 0.001734 |
| mp_fold_q.test | RUN | 0.190804 | 1.115047 | 0.771858 | 0.001317 | 0.001800 | 0.001655 |
| mp_fold_q_sf.test | RUN | 0.171209 | 0.519482 | 0.604943 | 0.001427 | 0.001781 | 0.001722 |
| mp_reverse_fold.test | RUN | 0.310966 | 2.403845 | 1.108261 | 0.001712 | 0.001683 | 0.001567 |
| mp_reverse_fold_q.test | RUN | 0.194418 | 1.332073 | 0.823464 | 0.001319 | 0.001620 | 0.001646 |
| mp_unique.test | RUN | 0.193830 | 1.205151 | 0.777941 | 0.001297 | 0.001669 | 0.001747 |
| mp_unique_if.test | RUN | 0.347441 | 2.532095 | 1.219586 | 0.001793 | 0.001846 | 0.001636 |
| mp_unique_if_q.test | RUN | 0.297800 | 2.504373 | 0.981161 | 0.001568 | 0.001706 | 0.001681 |
| mp_all_of.test | RUN | 0.462209 | 1.371731 | 1.325463 | 0.001902 | 0.001752 | 0.001750 |
| mp_all_of_q.test | RUN | 0.165582 | 0.577339 | 0.544961 | 0.001464 | 0.001738 | 0.001747 |
| mp_any_of.test | RUN | 0.450276 | 1.439495 | 1.341800 | 0.001706 | 0.001880 | 0.001747 |
| mp_any_of_q.test | RUN | 0.170345 | 0.577646 | 0.602002 | 0.001814 | 0.001781 | 0.001648 |
| mp_none_of.test | RUN | 0.452250 | 1.432499 | 1.316986 | 0.001546 | 0.001732 | 0.001589 |
| mp_none_of_q.test | RUN | 0.172491 | 0.594589 | 0.591180 | 0.001656 | 0.001842 | 0.001654 |
| mp_replace_at.test | RUN | 0.204010 | 1.194048 | 0.839419 | 0.001573 | 0.001831 | 0.001721 |
| mp_replace_at_c.test | RUN | 0.204759 | 1.215881 | 0.855127 | 0.001760 | 0.001612 | 0.001726 |
| mp_for_each.test | RUN | 0.197844 | 0.909975 | 0.929374 | 0.001642 | 0.001537 | 0.001634 |
| mp_insert.test | RUN | 0.300842 | 2.426354 | 1.301837 | 0.001636 | 0.001714 | 0.001680 |
| mp_insert_2.test | RUN | 0.028353 | 0.058148 | 0.100261 | 0.000574 | 0.000646 | 0.001763 |
| mp_erase.test | RUN | 0.327448 | 2.593771 | 1.467580 | 0.001656 | 0.001743 | 0.001596 |
| mp_erase_2.test | RUN | 0.026504 | 0.059308 | 0.084694 | 0.000617 | 0.000760 | 0.001297 |
| mp_with_index.test | RUN | 0.419548 | 5.315632 | 3.214893 | 0.002175 | 0.002214 | 0.002116 |
| mp_with_index_cx.test | RUN | 0.003428 | 0.017716 | 0.022997 | 0.000670 | 0.000726 | 0.001326 |
| mp_from_sequence.test | RUN | 0.205221 | 1.390106 | 0.894727 | 0.001664 | 0.001521 | 0.001686 |
| mp_from_sequence_2.test | RUN | 0.217209 | 1.627773 | 0.893778 | 0.001836 | 0.001629 | 0.001788 |
| mp_from_sequence_3.test | RUN | 0.179483 | 0.742559 | 0.643272 | 0.001543 | 0.001764 | 0.001706 |
| mp_min_element.test | RUN | 0.175880 | 0.664601 | 0.630370 | 0.001730 | 0.001826 | 0.001549 |
| mp_min_element_2.test | RUN | 0.028194 | 0.057433 | 0.095240 | 0.000665 | 0.000726 | 0.001587 |
| mp_min_element_q.test | RUN | 0.175865 | 0.658384 | 0.609060 | 0.001528 | 0.001854 | 0.001792 |
| mp_min_element_q_2.test | RUN | 0.026505 | 0.056750 | 0.085338 | 0.000627 | 0.000777 | 0.001712 |
| mp_max_element.test | RUN | 0.173599 | 0.654059 | 0.573602 | 0.001428 | 0.001781 | 0.001600 |
| mp_max_element_2.test | RUN | 0.027624 | 0.056726 | 0.096889 | 0.000599 | 0.000655 | 0.001727 |
| mp_max_element_q.test | RUN | 0.178512 | 0.670980 | 0.620799 | 0.001706 | 0.001770 | 0.001744 |
| mp_max_element_q_2.test | RUN | 0.028766 | 0.056413 | 0.090448 | 0.000696 | 0.000706 | 0.001538 |
| mp_nth_element.test | RUN | 0.194890 | 0.936079 | 0.755048 | 0.001572 | 0.001598 | 0.001647 |
| mp_nth_element_q.test | RUN | 0.192393 | 0.924009 | 0.761898 | 0.001302 | 0.001518 | 0.001676 |
| mp_back.test | RUN | 0.190490 | 0.746390 | 0.721486 | 0.001650 | 0.001696 | 0.001524 |
| mp_back_2.test | RUN | 0.027276 | 0.057677 | 0.091381 | 0.000635 | 0.000709 | 0.001368 |
| mp_pop_back.test | RUN | 0.481822 | 4.952635 | 1.416934 | 0.001460 | 0.001622 | 0.001570 |
| mp_pop_back_2.test | RUN | 0.027557 | 0.058275 | 0.093841 | 0.000712 | 0.000744 | 0.001531 |
| mp_flatten.test | RUN | 0.219264 | 1.768362 | 0.997133 | 0.001692 | 0.001738 | 0.001507 |
| mp_rotate_left.test | RUN | 0.225043 | 1.914704 | 0.998299 | 0.001746 | 0.001894 | 0.001614 |
| mp_rotate_right.test | RUN | 0.217514 | 1.912263 | 0.980589 | 0.001408 | 0.001719 | 0.001504 |
| mp_power_set.test | RUN | 0.214424 | 1.821306 | 0.985263 | 0.001700 | 0.001768 | 0.001510 |
| mp_partial_sum.test | RUN | 0.279657 | 2.433859 | 1.119997 | 0.001289 | 0.001548 | 0.001620 |
| mp_iterate.test | RUN | 0.190849 | 0.736933 | 0.797445 | 0.001558 | 0.001726 | 0.001575 |
| mp_pairwise_fold.test | RUN | 0.212592 | 1.681607 | 0.987011 | 0.001579 | 0.001798 | 0.001848 |
| mp_pairwise_fold_q.test | RUN | 0.170227 | 0.506550 | 0.577037 | 0.001804 | 0.001676 | 0.001717 |
| mp_sliding_fold.test | RUN | 0.404498 | 3.188484 | 1.812635 | 0.001802 | 0.001878 | 0.001532 |
| mp_sliding_fold_q.test | RUN | 0.397348 | 3.221086 | 1.834548 | 0.001664 | 0.001812 | 0.001626 |
| mp_intersperse.test | RUN | 0.207395 | 1.218482 | 0.860639 | 0.001823 | 0.001759 | 0.001875 |
| mp_split.test | RUN | 0.282540 | 2.311427 | 1.203221 | 0.001454 | 0.001797 | 0.001747 |
| mp_join.test | RUN | 0.231544 | 1.939476 | 1.032662 | 0.001552 | 0.001809 | 0.001721 |
| integral.test | RUN | 0.176884 | 0.866782 | 0.635875 | 0.001635 | 0.001754 | 0.001702 |
| mp_value.test | RUN | 0.011444 | 0.032265 | 0.045781 | 0.000752 | 0.000627 | 0.001607 |
| mp_identity.test | RUN | 0.174866 | 0.807829 | 0.616144 | 0.001483 | 0.001789 | 0.001717 |
| mp_inherit.test | RUN | 0.165054 | 0.568193 | 0.585588 | 0.001778 | 0.001817 | 0.001819 |
| mp_if.test | RUN | 0.165878 | 0.658343 | 0.619682 | 0.001420 | 0.001550 | 0.001837 |
| mp_if_sf.test | RUN | 0.159396 | 0.477858 | 0.521545 | 0.001663 | 0.001870 | 0.001798 |
| mp_eval_if.test | RUN | 0.164893 | 0.703402 | 0.590813 | 0.001571 | 0.001684 | 0.001693 |
| mp_eval_if_sf.test | RUN | 0.157119 | 0.503526 | 0.572579 | 0.001724 | 0.001563 | 0.001748 |
| mp_valid.test | RUN | 0.163419 | 0.503699 | 0.520762 | 0.001532 | 0.001560 | 0.001688 |
| mp_defer.test | RUN | 0.173280 | 0.853994 | 0.652028 | 0.001674 | 0.001801 | 0.001769 |
| mp_quote.test | RUN | 0.174842 | 0.855870 | 0.666508 | 0.001496 | 0.001827 | 0.001407 |
| mp_invoke_q.test | RUN | 0.187302 | 1.104879 | 0.722436 | 0.001379 | 0.001849 | 0.001642 |
| mp_invoke_q_sf.test | RUN | 0.159417 | 0.490490 | 0.536819 | 0.001710 | 0.001824 | 0.001833 |
| mp_quote_trait.test | RUN | 0.172252 | 0.924730 | 0.643635 | 0.001696 | 0.001789 | 0.001624 |
| mp_cond.test | RUN | 0.167974 | 0.586602 | 0.571433 | 0.001616 | 0.001630 | 0.001724 |
| mp_cond_sf.test | RUN | 0.156424 | 0.492788 | 0.543400 | 0.001732 | 0.001611 | 0.001850 |
| mp_not_fn.test | RUN | 0.154082 | 0.493732 | 0.520839 | 0.001282 | 0.001853 | 0.001518 |
| mp_eval_if_not.test | RUN | 0.170385 | 0.705697 | 0.600344 | 0.001924 | 0.001574 | 0.001687 |
| mp_eval_or.test | RUN | 0.161973 | 0.374198 | 0.559130 | 0.001584 | 0.001833 | 0.001515 |
| mp_compose.test | RUN | 0.181086 | 1.141765 | 0.712972 | 0.001550 | 0.001538 | 0.001710 |
| mp_valid_and_true.test | RUN | 0.163001 | 0.589505 | 0.550399 | 0.001486 | 0.001820 | 0.001439 |
| mp_compose_sf.test | COMPILE | 0.039170 | 0.076356 | 0.130983 | — | — | — |
| integer_sequence.test | RUN | 0.194565 | 1.124887 | 0.799865 | 0.001440 | 0.001803 | 0.001454 |
| tuple_for_each.test | RUN | 0.166302 | 0.437659 | 0.583102 | 0.001747 | 0.001542 | 0.001635 |
| tuple_for_each_cx.test | COMPILE | 0.031005 | 0.061846 | 0.109396 | — | — | — |
| tuple_apply.test | RUN | 0.167433 | 0.438722 | 0.608638 | 0.001732 | 0.001680 | 0.001616 |
| tuple_apply_cx.test | COMPILE | 0.044108 | 0.085278 | 0.150192 | — | — | — |
| construct_from_tuple.test | RUN | 0.167363 | 0.454504 | 0.555857 | 0.001592 | 0.001409 | 0.001621 |
| construct_from_tuple_cx.test | COMPILE | 0.045126 | 0.091066 | 0.130115 | — | — | — |
| tuple_transform.test | RUN | 0.265968 | 0.930470 | 1.174929 | 0.001652 | 0.001555 | 0.001539 |
| tuple_transform_2.test | RUN | 0.235180 | 0.686334 | 1.029688 | 0.001503 | 0.001710 | 0.001729 |
| tuple_transform_cx.test | COMPILE | 0.038067 | 0.086853 | 0.145317 | — | — | — |
| mp_set_contains.test | RUN | 0.169420 | 0.610535 | 0.601737 | 0.001622 | 0.001562 | 0.001738 |
| mp_set_push_back.test | RUN | 0.221368 | 1.817887 | 0.997340 | 0.001692 | 0.001688 | 0.001763 |
| mp_set_push_front.test | RUN | 0.273734 | 2.184292 | 1.113818 | 0.001586 | 0.001719 | 0.001400 |
| mp_is_set.test | RUN | 0.171224 | 0.607347 | 0.620790 | 0.001516 | 0.001696 | 0.001631 |
| mp_set_union.test | RUN | 0.208775 | 1.538622 | 0.917378 | 0.001706 | 0.001882 | 0.001699 |
| mp_set_union_sf.test | RUN | 0.163617 | 0.484765 | 0.569633 | 0.001780 | 0.001722 | 0.001534 |
| mp_set_difference.test | RUN | 0.191174 | 1.223794 | 0.799003 | 0.001559 | 0.001800 | 0.001776 |
| mp_set_difference_sf.test | RUN | 0.159594 | 0.476572 | 0.554820 | 0.001213 | 0.001786 | 0.001428 |
| mp_set_intersection.test | RUN | 0.189372 | 1.198810 | 0.781819 | 0.001670 | 0.001533 | 0.001563 |
| mp_set_intersection_sf.test | RUN | 0.161955 | 0.484061 | 0.561718 | 0.001560 | 0.001328 | 0.001448 |
| mp_all.test | RUN | 0.170245 | 0.599543 | 0.608475 | 0.001653 | 0.001605 | 0.001577 |
| mp_all_2.test | RUN | 0.464505 | 1.280158 | 1.304177 | 0.001754 | 0.001692 | 0.001820 |
| mp_and.test | RUN | 0.167354 | 0.583334 | 0.600308 | 0.001674 | 0.001754 | 0.001799 |
| mp_any.test | RUN | 0.169486 | 0.597288 | 0.564836 | 0.001557 | 0.001740 | 0.001639 |
| mp_any_2.test | RUN | 0.487268 | 1.289894 | 1.337074 | 0.001382 | 0.001825 | 0.001404 |
| mp_or.test | RUN | 0.167678 | 0.590326 | 0.596290 | 0.001765 | 0.001738 | 0.001670 |
| mp_same.test | RUN | 0.314936 | 0.986292 | 0.918248 | 0.001595 | 0.001731 | 0.001534 |
| mp_plus.test | RUN | 0.180667 | 0.694676 | 0.609869 | 0.001430 | 0.001755 | 0.001379 |
| mp_less.test | RUN | 0.168527 | 0.602272 | 0.544744 | 0.001760 | 0.001546 | 0.001506 |
| mp_min.test | RUN | 0.160761 | 0.557067 | 0.578332 | 0.001589 | 0.001851 | 0.001642 |
| mp_max.test | RUN | 0.165556 | 0.614830 | 0.594626 | 0.001574 | 0.001668 | 0.001537 |
| mp_similar.test | RUN | 0.162327 | 0.600621 | 0.591230 | 0.001496 | 0.001691 | 0.001431 |
| mp_map_find.test | RUN | 0.191937 | 1.094154 | 0.785338 | 0.001730 | 0.001104 | 0.001726 |
| mp_map_find_2.test | RUN | 0.175816 | 0.441293 | 0.648893 | 0.001585 | 0.001510 | 0.001462 |
| mp_map_find_3.test | RUN | 0.178201 | 0.428185 | 0.673298 | 0.001519 | 0.001734 | 0.001456 |
| mp_map_find_4.test | RUN | 0.167812 | 0.401927 | 0.627219 | 0.001269 | 0.001718 | 0.001403 |
| mp_map_contains.test | RUN | 0.171340 | 0.610370 | 0.597627 | 0.001758 | 0.001869 | 0.001696 |
| mp_map_insert.test | RUN | 0.227166 | 2.016803 | 1.057507 | 0.001786 | 0.001842 | 0.001464 |
| mp_map_replace.test | RUN | 0.286958 | 2.304859 | 1.226345 | 0.001537 | 0.001834 | 0.001710 |
| mp_map_erase.test | RUN | 0.216541 | 1.719845 | 1.037146 | 0.001295 | 0.001752 | 0.001506 |
| mp_map_update.test | RUN | 0.198186 | 1.123563 | 0.791293 | 0.001624 | 0.001772 | 0.001433 |
| mp_map_update_q.test | RUN | 0.191579 | 1.142318 | 0.779287 | 0.001356 | 0.001882 | 0.001518 |
| mp_map_keys.test | RUN | 0.186957 | 1.000108 | 0.785339 | 0.001724 | 0.001825 | 0.001534 |
| mp_is_map.test | RUN | 0.178637 | 0.608731 | 0.652243 | 0.001652 | 0.001547 | 0.001335 |
| mp_bind.test | RUN | 0.221363 | 1.794576 | 0.921249 | 0.001762 | 0.001715 | 0.001721 |
| mp_bind_q.test | RUN | 0.189656 | 1.113856 | 0.736052 | 0.001766 | 0.001683 | 0.001376 |
| mp_bind_front.test | RUN | 0.187593 | 1.215201 | 0.737203 | 0.001338 | 0.001808 | 0.001530 |
| mp_bind_back.test | RUN | 0.188449 | 1.216947 | 0.747737 | 0.001641 | 0.001710 | 0.001762 |
| mp_lambda.test | RUN | 0.405253 | 3.659686 | 1.875339 | 0.001488 | 0.001331 | 0.001702 |
| mpl.test | RUN | 0.379349 | 0.985274 | 1.515333 | 0.001798 | 0.001678 | 0.001667 |
| mpl_list.test | RUN | 0.215745 | 0.735747 | 0.735713 | 0.001593 | 0.001804 | 0.001570 |
| mpl_tuple.test | RUN | 0.218918 | 0.740507 | 0.733520 | 0.001666 | 0.001666 | 0.001675 |
| version.test | RUN | 0.108242 | 0.271855 | 0.368518 | 0.001579 | 0.001603 | 0.001241 |
| mp11_version.test | RUN | 0.130000 | 0.309110 | 0.446520 | 0.001576 | 0.001762 | 0.001668 |
| list_version.test | RUN | 0.114462 | 0.283593 | 0.368430 | 0.001634 | 0.001547 | 0.001670 |
| algorithm_version.test | RUN | 0.124432 | 0.296734 | 0.395352 | 0.001782 | 0.001756 | 0.001527 |
| integral_version.test | RUN | 0.108032 | 0.271146 | 0.357318 | 0.001591 | 0.001736 | 0.001654 |
| utility_version.test | RUN | 0.113259 | 0.277790 | 0.368688 | 0.001489 | 0.001543 | 0.001380 |
| function_version.test | RUN | 0.114670 | 0.284139 | 0.385288 | 0.001400 | 0.001638 | 0.001890 |
| map_version.test | RUN | 0.122314 | 0.295982 | 0.417737 | 0.001593 | 0.001709 | 0.001378 |
| set_version.test | RUN | 0.113893 | 0.285325 | 0.387021 | 0.001728 | 0.001604 | 0.001721 |
| bind_version.test | RUN | 0.124313 | 0.295736 | 0.416896 | 0.001586 | 0.001684 | 0.001564 |
| integer_sequence_version.test | RUN | 0.109797 | 0.274503 | 0.342813 | 0.001385 | 0.001685 | 0.001270 |
| tuple_version.test | RUN | 0.124157 | 0.295343 | 0.285515 | 0.001289 | 0.001926 | 0.001482 |
| mpl_version.test | RUN | 0.090129 | 0.211820 | 0.346758 | 0.001275 | 0.001312 | 0.001222 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.005× | 1.006×, 1.005× | 0.980× |
| clang | 1.082× | 1.086×, 1.077× | 0.998× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224918).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| algorithm_version.test | 10 | 0.000955s | 0.000937s | 0.000967s | byte equality observed |
| bind_version.test | 10 | 0.000957s | 0.000938s | 0.000956s | byte equality observed |
| construct_from_tuple.test | 10 | 0.000963s | 0.000942s | 0.000962s | byte equality observed |
| function_version.test | 10 | 0.000968s | 0.000940s | 0.000991s | byte equality observed |
| integer_sequence.test | 10 | 0.000960s | 0.000948s | 0.000969s | byte equality observed |
| integer_sequence_version.test | 10 | 0.000959s | 0.000937s | 0.000986s | byte equality observed |
| integral.test | 10 | 0.000968s | 0.000955s | 0.000972s | byte equality observed |
| integral_version.test | 10 | 0.000948s | 0.000943s | 0.001000s | byte equality observed |
| list_version.test | 10 | 0.000960s | 0.000952s | 0.000993s | byte equality observed |
| map_version.test | 10 | 0.000959s | 0.000957s | 0.001011s | byte equality observed |
| mp11_version.test | 10 | 0.000988s | 0.000996s | 0.000970s | byte equality observed |
| mp_all.test | 10 | 0.001003s | 0.000967s | 0.000983s | byte equality observed |
| mp_all_2.test | 10 | 0.000986s | 0.000968s | 0.001022s | byte equality observed |
| mp_all_of.test | 10 | 0.001009s | 0.000988s | 0.001034s | byte equality observed |
| mp_all_of_q.test | 10 | 0.001000s | 0.000985s | 0.000984s | byte equality observed |
| mp_and.test | 10 | 0.000990s | 0.000981s | 0.000979s | byte equality observed |
| mp_any.test | 10 | 0.001019s | 0.000957s | 0.001016s | byte equality observed |
| mp_any_2.test | 10 | 0.000975s | 0.000990s | 0.001014s | byte equality observed |
| mp_any_of.test | 10 | 0.001002s | 0.000990s | 0.000991s | byte equality observed |
| mp_any_of_q.test | 10 | 0.000995s | 0.001006s | 0.001024s | byte equality observed |
| mp_append.test | 10 | 0.000997s | 0.001004s | 0.001006s | byte equality observed |
| mp_append_2.test | 10 | 0.001037s | 0.001041s | 0.000991s | byte equality observed |
| mp_append_3.test | 10 | 0.000460s | 0.000462s | 0.000990s | byte equality observed |
| mp_append_sf.test | 10 | 0.001019s | 0.001028s | 0.000988s | byte equality observed |
| mp_apply_q.test | 10 | 0.001026s | 0.001040s | 0.000961s | byte equality observed |
| mp_apply_q_sf.test | 10 | 0.001013s | 0.001007s | 0.000975s | byte equality observed |
| mp_assign.test | 10 | 0.001033s | 0.001021s | 0.000992s | byte equality observed |
| mp_assign_2.test | 10 | 0.000461s | 0.000466s | 0.000978s | byte equality observed |
| mp_at.test | 10 | 0.001030s | 0.001008s | 0.000993s | byte equality observed |
| mp_at_2.test | 10 | 0.000456s | 0.000464s | 0.001003s | byte equality observed |
| mp_at_sf.test | 10 | 0.001007s | 0.001020s | 0.000990s | byte equality observed |
| mp_back.test | 10 | 0.000984s | 0.001017s | 0.000969s | byte equality observed |
| mp_back_2.test | 10 | 0.000455s | 0.000468s | 0.000962s | byte equality observed |
| mp_bind.test | 10 | 0.000994s | 0.001029s | 0.001027s | byte equality observed |
| mp_bind_back.test | 10 | 0.000994s | 0.001005s | 0.000982s | byte equality observed |
| mp_bind_front.test | 10 | 0.001001s | 0.001002s | 0.000981s | byte equality observed |
| mp_bind_q.test | 10 | 0.000984s | 0.001028s | 0.000980s | byte equality observed |
| mp_clear.test | 10 | 0.000999s | 0.001004s | 0.000973s | byte equality observed |
| mp_clear_2.test | 10 | 0.000469s | 0.000463s | 0.000987s | byte equality observed |
| mp_compose.test | 10 | 0.000986s | 0.001000s | 0.000976s | byte equality observed |
| mp_cond.test | 10 | 0.000989s | 0.001052s | 0.000989s | byte equality observed |
| mp_cond_sf.test | 10 | 0.000975s | 0.001024s | 0.000999s | byte equality observed |
| mp_contains.test | 10 | 0.000987s | 0.000992s | 0.000979s | byte equality observed |
| mp_copy_if.test | 10 | 0.000979s | 0.001020s | 0.001003s | byte equality observed |
| mp_copy_if_q.test | 10 | 0.000990s | 0.001024s | 0.000990s | byte equality observed |
| mp_count.test | 10 | 0.000998s | 0.001020s | 0.001014s | byte equality observed |
| mp_count_if.test | 10 | 0.001000s | 0.001023s | 0.000987s | byte equality observed |
| mp_count_if_q.test | 10 | 0.001004s | 0.000980s | 0.001023s | byte equality observed |
| mp_defer.test | 10 | 0.001014s | 0.001003s | 0.000987s | byte equality observed |
| mp_drop.test | 10 | 0.000998s | 0.001019s | 0.000960s | byte equality observed |
| mp_drop_2.test | 10 | 0.000464s | 0.000465s | 0.000983s | byte equality observed |
| mp_empty.test | 10 | 0.001004s | 0.001002s | 0.000998s | byte equality observed |
| mp_empty_2.test | 10 | 0.000459s | 0.000465s | 0.000998s | byte equality observed |
| mp_erase.test | 10 | 0.001015s | 0.001026s | 0.001007s | byte equality observed |
| mp_erase_2.test | 10 | 0.000462s | 0.000464s | 0.000970s | byte equality observed |
| mp_eval_if.test | 10 | 0.000994s | 0.001005s | 0.000992s | byte equality observed |
| mp_eval_if_not.test | 10 | 0.001022s | 0.001002s | 0.001003s | byte equality observed |
| mp_eval_if_sf.test | 10 | 0.001025s | 0.001014s | 0.001037s | byte equality observed |
| mp_eval_or.test | 10 | 0.000992s | 0.001015s | 0.000994s | byte equality observed |
| mp_fill.test | 10 | 0.001012s | 0.001028s | 0.000967s | byte equality observed |
| mp_fill_2.test | 10 | 0.000456s | 0.000457s | 0.000970s | byte equality observed |
| mp_filter.test | 10 | 0.001003s | 0.001024s | 0.001005s | byte equality observed |
| mp_find.test | 10 | 0.001001s | 0.001026s | 0.001010s | byte equality observed |
| mp_find_if.test | 10 | 0.000986s | 0.001011s | 0.001000s | byte equality observed |
| mp_find_if_q.test | 10 | 0.000990s | 0.001006s | 0.001008s | byte equality observed |
| mp_flatten.test | 10 | 0.000986s | 0.000999s | 0.000995s | byte equality observed |
| mp_fold.test | 10 | 0.001025s | 0.001009s | 0.001010s | byte equality observed |
| mp_fold_q.test | 10 | 0.001002s | 0.001011s | 0.000992s | byte equality observed |
| mp_fold_q_sf.test | 10 | 0.000995s | 0.001007s | 0.000996s | byte equality observed |
| mp_for_each.test | 10 | 0.001007s | 0.000994s | 0.001026s | byte equality observed |
| mp_from_sequence.test | 10 | 0.001013s | 0.001018s | 0.001003s | byte equality observed |
| mp_from_sequence_2.test | 10 | 0.000982s | 0.001008s | 0.001001s | byte equality observed |
| mp_from_sequence_3.test | 10 | 0.000984s | 0.001004s | 0.001007s | byte equality observed |
| mp_front.test | 10 | 0.001006s | 0.001025s | 0.001003s | byte equality observed |
| mp_front_2.test | 10 | 0.000463s | 0.000468s | 0.000996s | byte equality observed |
| mp_identity.test | 10 | 0.000992s | 0.000998s | 0.001007s | byte equality observed |
| mp_if.test | 10 | 0.000988s | 0.000994s | 0.001009s | byte equality observed |
| mp_if_sf.test | 10 | 0.000988s | 0.000969s | 0.001012s | byte equality observed |
| mp_inherit.test | 10 | 0.001007s | 0.000966s | 0.001019s | byte equality observed |
| mp_insert.test | 10 | 0.001026s | 0.001012s | 0.000994s | byte equality observed |
| mp_insert_2.test | 10 | 0.000460s | 0.000459s | 0.000983s | byte equality observed |
| mp_intersperse.test | 10 | 0.000997s | 0.000979s | 0.001016s | byte equality observed |
| mp_invoke_q.test | 10 | 0.000986s | 0.001003s | 0.001007s | byte equality observed |
| mp_invoke_q_sf.test | 10 | 0.001033s | 0.001015s | 0.000998s | byte equality observed |
| mp_iota.test | 10 | 0.001014s | 0.001024s | 0.000988s | byte equality observed |
| mp_iota_2.test | 10 | 0.001025s | 0.001004s | 0.000984s | byte equality observed |
| mp_is_list.test | 10 | 0.000980s | 0.001016s | 0.001035s | byte equality observed |
| mp_is_map.test | 10 | 0.000987s | 0.000995s | 0.000999s | byte equality observed |
| mp_is_set.test | 10 | 0.001000s | 0.000993s | 0.000990s | byte equality observed |
| mp_is_value_list.test | 10 | 0.000987s | 0.000991s | 0.001016s | byte equality observed |
| mp_iterate.test | 10 | 0.001006s | 0.000985s | 0.001015s | byte equality observed |
| mp_join.test | 10 | 0.001009s | 0.001021s | 0.000985s | byte equality observed |
| mp_lambda.test | 10 | 0.001052s | 0.000985s | 0.001014s | byte equality observed |
| mp_less.test | 10 | 0.001008s | 0.001014s | 0.000994s | byte equality observed |
| mp_list_c.test | 10 | 0.001011s | 0.000989s | 0.001010s | byte equality observed |
| mp_list_v.test | 10 | 0.000459s | 0.000463s | 0.000951s | byte equality observed |
| mp_map_contains.test | 10 | 0.001003s | 0.001013s | 0.000992s | byte equality observed |
| mp_map_erase.test | 10 | 0.001006s | 0.001007s | 0.000986s | byte equality observed |
| mp_map_find.test | 10 | 0.000995s | 0.001004s | 0.000998s | byte equality observed |
| mp_map_find_2.test | 10 | 0.000987s | 0.000995s | 0.001017s | byte equality observed |
| mp_map_find_3.test | 10 | 0.000995s | 0.001041s | 0.001005s | byte equality observed |
| mp_map_find_4.test | 10 | 0.000992s | 0.001005s | 0.000996s | byte equality observed |
| mp_map_insert.test | 10 | 0.000980s | 0.000999s | 0.001030s | byte equality observed |
| mp_map_keys.test | 10 | 0.000974s | 0.001025s | 0.001042s | byte equality observed |
| mp_map_replace.test | 10 | 0.000999s | 0.001031s | 0.001026s | byte equality observed |
| mp_map_update.test | 10 | 0.000997s | 0.000990s | 0.000983s | byte equality observed |
| mp_map_update_q.test | 10 | 0.000989s | 0.000981s | 0.000975s | byte equality observed |
| mp_max.test | 10 | 0.000995s | 0.000995s | 0.001041s | byte equality observed |
| mp_max_element.test | 10 | 0.001008s | 0.001031s | 0.000975s | byte equality observed |
| mp_max_element_2.test | 10 | 0.000461s | 0.000463s | 0.000984s | byte equality observed |
| mp_max_element_q.test | 10 | 0.001007s | 0.001024s | 0.000992s | byte equality observed |
| mp_max_element_q_2.test | 10 | 0.000464s | 0.000461s | 0.000949s | byte equality observed |
| mp_min.test | 10 | 0.000993s | 0.001000s | 0.001009s | byte equality observed |
| mp_min_element.test | 10 | 0.000995s | 0.001010s | 0.000995s | byte equality observed |
| mp_min_element_2.test | 10 | 0.000463s | 0.000464s | 0.000990s | byte equality observed |
| mp_min_element_q.test | 10 | 0.001035s | 0.001042s | 0.001013s | byte equality observed |
| mp_min_element_q_2.test | 10 | 0.000453s | 0.000461s | 0.000977s | byte equality observed |
| mp_none_of.test | 10 | 0.001007s | 0.001011s | 0.001001s | byte equality observed |
| mp_none_of_q.test | 10 | 0.001024s | 0.001009s | 0.001000s | byte equality observed |
| mp_not_fn.test | 10 | 0.001008s | 0.000992s | 0.001010s | byte equality observed |
| mp_nth_element.test | 10 | 0.001012s | 0.001020s | 0.000970s | byte equality observed |
| mp_nth_element_q.test | 10 | 0.001002s | 0.000993s | 0.000995s | byte equality observed |
| mp_or.test | 10 | 0.001000s | 0.001002s | 0.000976s | byte equality observed |
| mp_pairwise_fold.test | 10 | 0.000997s | 0.001009s | 0.000981s | byte equality observed |
| mp_pairwise_fold_q.test | 10 | 0.001005s | 0.000997s | 0.000994s | byte equality observed |
| mp_partial_sum.test | 10 | 0.001006s | 0.001016s | 0.001002s | byte equality observed |
| mp_partition.test | 10 | 0.000980s | 0.001011s | 0.000997s | byte equality observed |
| mp_partition_q.test | 10 | 0.000970s | 0.000984s | 0.000979s | byte equality observed |
| mp_plus.test | 10 | 0.000970s | 0.000990s | 0.000979s | byte equality observed |
| mp_pop_back.test | 10 | 0.001011s | 0.001015s | 0.000992s | byte equality observed |
| mp_pop_back_2.test | 10 | 0.000467s | 0.000463s | 0.000976s | byte equality observed |
| mp_pop_front.test | 10 | 0.001005s | 0.001048s | 0.000990s | byte equality observed |
| mp_pop_front_2.test | 10 | 0.000458s | 0.000461s | 0.000971s | byte equality observed |
| mp_power_set.test | 10 | 0.000994s | 0.001010s | 0.000992s | byte equality observed |
| mp_product.test | 10 | 0.001007s | 0.001022s | 0.000997s | byte equality observed |
| mp_push_back.test | 10 | 0.001015s | 0.001019s | 0.000991s | byte equality observed |
| mp_push_back_2.test | 10 | 0.000462s | 0.000463s | 0.000978s | byte equality observed |
| mp_push_front.test | 10 | 0.001011s | 0.001015s | 0.000996s | byte equality observed |
| mp_push_front_2.test | 10 | 0.000456s | 0.000460s | 0.000952s | byte equality observed |
| mp_quote.test | 10 | 0.001007s | 0.001015s | 0.000986s | byte equality observed |
| mp_quote_trait.test | 10 | 0.001011s | 0.001011s | 0.001014s | byte equality observed |
| mp_remove.test | 10 | 0.001020s | 0.001009s | 0.000989s | byte equality observed |
| mp_remove_if.test | 10 | 0.001027s | 0.001020s | 0.001005s | byte equality observed |
| mp_remove_if_q.test | 10 | 0.000999s | 0.001020s | 0.000989s | byte equality observed |
| mp_rename.test | 10 | 0.001003s | 0.001026s | 0.001000s | byte equality observed |
| mp_rename_2.test | 10 | 0.000465s | 0.000469s | 0.000969s | byte equality observed |
| mp_rename_v.test | 10 | 0.000464s | 0.000474s | 0.000962s | byte equality observed |
| mp_repeat.test | 10 | 0.001037s | 0.001040s | 0.000973s | byte equality observed |
| mp_repeat_2.test | 10 | 0.000457s | 0.000465s | 0.001013s | byte equality observed |
| mp_repeat_3.test | 10 | 0.000985s | 0.001016s | 0.000979s | byte equality observed |
| mp_replace.test | 10 | 0.000988s | 0.001007s | 0.000999s | byte equality observed |
| mp_replace_at.test | 10 | 0.000975s | 0.001014s | 0.001008s | byte equality observed |
| mp_replace_at_c.test | 10 | 0.000981s | 0.001007s | 0.001006s | byte equality observed |
| mp_replace_front.test | 10 | 0.001007s | 0.001014s | 0.001013s | byte equality observed |
| mp_replace_front_2.test | 10 | 0.000463s | 0.000456s | 0.000966s | byte equality observed |
| mp_replace_if.test | 10 | 0.000986s | 0.001036s | 0.001013s | byte equality observed |
| mp_replace_if_q.test | 10 | 0.000978s | 0.001015s | 0.000995s | byte equality observed |
| mp_replace_second.test | 10 | 0.001004s | 0.001027s | 0.000975s | byte equality observed |
| mp_replace_second_2.test | 10 | 0.000463s | 0.000466s | 0.000974s | byte equality observed |
| mp_replace_third.test | 10 | 0.001019s | 0.001011s | 0.000979s | byte equality observed |
| mp_replace_third_2.test | 10 | 0.000461s | 0.000458s | 0.000953s | byte equality observed |
| mp_reverse.test | 10 | 0.001017s | 0.001031s | 0.000996s | byte equality observed |
| mp_reverse_fold.test | 10 | 0.001033s | 0.001015s | 0.000964s | byte equality observed |
| mp_reverse_fold_q.test | 10 | 0.001040s | 0.001027s | 0.000964s | byte equality observed |
| mp_rotate_left.test | 10 | 0.000998s | 0.001012s | 0.000976s | byte equality observed |
| mp_rotate_right.test | 10 | 0.001003s | 0.001019s | 0.000968s | byte equality observed |
| mp_same.test | 10 | 0.000990s | 0.001016s | 0.000982s | byte equality observed |
| mp_second.test | 10 | 0.001027s | 0.001015s | 0.000966s | byte equality observed |
| mp_second_2.test | 10 | 0.000461s | 0.000461s | 0.000978s | byte equality observed |
| mp_set_contains.test | 10 | 0.001009s | 0.000978s | 0.001016s | byte equality observed |
| mp_set_difference.test | 10 | 0.001004s | 0.000991s | 0.000989s | byte equality observed |
| mp_set_difference_sf.test | 10 | 0.001018s | 0.000995s | 0.001000s | byte equality observed |
| mp_set_intersection.test | 10 | 0.001008s | 0.001009s | 0.001049s | byte equality observed |
| mp_set_intersection_sf.test | 10 | 0.001017s | 0.001015s | 0.001001s | byte equality observed |
| mp_set_push_back.test | 10 | 0.000999s | 0.001016s | 0.000993s | byte equality observed |
| mp_set_push_front.test | 10 | 0.001013s | 0.001054s | 0.001013s | byte equality observed |
| mp_set_union.test | 10 | 0.001002s | 0.001067s | 0.001013s | byte equality observed |
| mp_set_union_sf.test | 10 | 0.001006s | 0.001008s | 0.000995s | byte equality observed |
| mp_similar.test | 10 | 0.001005s | 0.000982s | 0.001007s | byte equality observed |
| mp_size.test | 10 | 0.001015s | 0.001028s | 0.000979s | byte equality observed |
| mp_size_2.test | 10 | 0.000465s | 0.000455s | 0.000963s | byte equality observed |
| mp_slice.test | 10 | 0.001036s | 0.001052s | 0.000982s | byte equality observed |
| mp_slice_2.test | 10 | 0.000468s | 0.000463s | 0.000965s | byte equality observed |
| mp_sliding_fold.test | 10 | 0.001031s | 0.001038s | 0.000995s | byte equality observed |
| mp_sliding_fold_q.test | 10 | 0.001035s | 0.001030s | 0.000992s | byte equality observed |
| mp_sort.test | 10 | 0.000997s | 0.001015s | 0.001020s | byte equality observed |
| mp_sort_q.test | 10 | 0.000996s | 0.001016s | 0.000975s | byte equality observed |
| mp_split.test | 10 | 0.001006s | 0.001043s | 0.000990s | byte equality observed |
| mp_starts_with.test | 10 | 0.000989s | 0.000991s | 0.000977s | byte equality observed |
| mp_starts_with_sf.test | 10 | 0.001003s | 0.000988s | 0.000973s | byte equality observed |
| mp_take.test | 10 | 0.001039s | 0.001017s | 0.000995s | byte equality observed |
| mp_take_2.test | 10 | 0.000461s | 0.000458s | 0.000979s | byte equality observed |
| mp_third.test | 10 | 0.001014s | 0.001019s | 0.000982s | byte equality observed |
| mp_third_2.test | 10 | 0.000465s | 0.000450s | 0.000981s | byte equality observed |
| mp_transform.test | 10 | 0.001025s | 0.000997s | 0.000981s | byte equality observed |
| mp_transform_2.test | 10 | 0.000964s | 0.000993s | 0.000981s | byte equality observed |
| mp_transform_front.test | 10 | 0.001003s | 0.001017s | 0.000979s | byte equality observed |
| mp_transform_front_2.test | 10 | 0.000464s | 0.000451s | 0.001001s | byte equality observed |
| mp_transform_if.test | 10 | 0.000983s | 0.001009s | 0.001004s | byte equality observed |
| mp_transform_if_q.test | 10 | 0.000999s | 0.000999s | 0.000980s | byte equality observed |
| mp_transform_q.test | 10 | 0.000999s | 0.001020s | 0.000953s | byte equality observed |
| mp_transform_q_2.test | 10 | 0.001014s | 0.000993s | 0.000958s | byte equality observed |
| mp_transform_second.test | 10 | 0.001013s | 0.001007s | 0.000995s | byte equality observed |
| mp_transform_second_2.test | 10 | 0.000464s | 0.000458s | 0.000982s | byte equality observed |
| mp_transform_sf.test | 10 | 0.001015s | 0.001023s | 0.000980s | byte equality observed |
| mp_transform_third.test | 10 | 0.001011s | 0.001029s | 0.000991s | byte equality observed |
| mp_transform_third_2.test | 10 | 0.000462s | 0.000466s | 0.001002s | byte equality observed |
| mp_unique.test | 10 | 0.000999s | 0.001007s | 0.000978s | byte equality observed |
| mp_unique_if.test | 10 | 0.000995s | 0.001027s | 0.001056s | byte equality observed |
| mp_unique_if_q.test | 10 | 0.001013s | 0.001014s | 0.000998s | byte equality observed |
| mp_valid.test | 10 | 0.000991s | 0.000995s | 0.000994s | byte equality observed |
| mp_valid_and_true.test | 10 | 0.001013s | 0.001014s | 0.001032s | byte equality observed |
| mp_value.test | 10 | 0.000460s | 0.000459s | 0.001000s | byte equality observed |
| mp_with_index.test | 10 | 0.001355s | 0.001314s | 0.001261s | byte equality observed |
| mp_with_index_cx.test | 10 | 0.000455s | 0.000458s | 0.000979s | byte equality observed |
| mpl.test | 10 | 0.001014s | 0.000989s | 0.000985s | byte equality observed |
| mpl_list.test | 10 | 0.000988s | 0.000998s | 0.001007s | byte equality observed |
| mpl_tuple.test | 10 | 0.000995s | 0.000995s | 0.000993s | byte equality observed |
| mpl_version.test | 10 | 0.001001s | 0.000981s | 0.001007s | byte equality observed |
| set_version.test | 10 | 0.000997s | 0.000973s | 0.001002s | byte equality observed |
| tuple_apply.test | 10 | 0.001027s | 0.000961s | 0.000996s | byte equality observed |
| tuple_for_each.test | 10 | 0.000993s | 0.001006s | 0.001008s | byte equality observed |
| tuple_transform.test | 10 | 0.000989s | 0.001022s | 0.001013s | byte equality observed |
| tuple_transform_2.test | 10 | 0.001009s | 0.001000s | 0.000986s | byte equality observed |
| tuple_version.test | 10 | 0.000991s | 0.001016s | 0.001002s | byte equality observed |
| utility_version.test | 10 | 0.001019s | 0.000997s | 0.001004s | byte equality observed |
| version.test | 10 | 0.001019s | 0.001009s | 0.001015s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224918).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
