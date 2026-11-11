# libs/mp11/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| mp11.test | COMPILE | 0.041519 | 0.071652 | 0.121623 | — | — | — |
| complex_h.test | COMPILE | 0.169471 | 0.307994 | 0.448128 | — | — | — |
| mp_size.test | RUN | 0.193846 | 0.961151 | 0.596017 | 0.001684 | 0.001827 | 0.001660 |
| mp_size_2.test | RUN | 0.018286 | 0.035861 | 0.053301 | 0.000870 | 0.000811 | 0.001439 |
| mp_empty.test | RUN | 0.188129 | 0.618010 | 0.535784 | 0.001594 | 0.001944 | 0.001742 |
| mp_empty_2.test | RUN | 0.019210 | 0.037646 | 0.059305 | 0.000581 | 0.000678 | 0.001609 |
| mp_assign.test | RUN | 0.207676 | 1.253455 | 0.710708 | 0.001591 | 0.001844 | 0.001593 |
| mp_assign_2.test | RUN | 0.017849 | 0.037994 | 0.059060 | 0.000735 | 0.000798 | 0.001692 |
| mp_clear.test | RUN | 0.186277 | 0.565661 | 0.577097 | 0.001657 | 0.001687 | 0.001733 |
| mp_clear_2.test | RUN | 0.018858 | 0.034547 | 0.058130 | 0.000745 | 0.000609 | 0.001716 |
| mp_front.test | RUN | 0.195968 | 0.874583 | 0.649280 | 0.001660 | 0.001959 | 0.001683 |
| mp_front_2.test | RUN | 0.018147 | 0.035362 | 0.052596 | 0.000614 | 0.000680 | 0.001411 |
| mp_pop_front.test | RUN | 0.199360 | 0.921409 | 0.602636 | 0.001887 | 0.001574 | 0.001618 |
| mp_pop_front_2.test | RUN | 0.019141 | 0.038763 | 0.056646 | 0.000629 | 0.000826 | 0.001555 |
| mp_second.test | RUN | 0.184422 | 0.583781 | 0.585698 | 0.001491 | 0.001722 | 0.001724 |
| mp_second_2.test | RUN | 0.019034 | 0.038250 | 0.057606 | 0.000600 | 0.000864 | 0.001741 |
| mp_third.test | RUN | 0.191110 | 0.541701 | 0.578336 | 0.001619 | 0.001843 | 0.001574 |
| mp_third_2.test | RUN | 0.019000 | 0.038544 | 0.050561 | 0.000600 | 0.000719 | 0.001468 |
| mp_push_front.test | RUN | 0.269173 | 1.919465 | 1.006141 | 0.001880 | 0.001832 | 0.001706 |
| mp_push_front_2.test | RUN | 0.018575 | 0.037667 | 0.060533 | 0.000613 | 0.000843 | 0.001593 |
| mp_push_back.test | RUN | 0.273393 | 1.929206 | 0.946099 | 0.001722 | 0.001522 | 0.001852 |
| mp_push_back_2.test | RUN | 0.018762 | 0.038687 | 0.059201 | 0.000725 | 0.000816 | 0.001833 |
| mp_rename.test | RUN | 0.229485 | 1.747134 | 0.913842 | 0.001564 | 0.001942 | 0.001583 |
| mp_rename_2.test | RUN | 0.019118 | 0.037734 | 0.054998 | 0.000793 | 0.000857 | 0.001692 |
| mp_append.test | RUN | 0.229192 | 1.994774 | 0.930421 | 0.001574 | 0.002035 | 0.001785 |
| mp_append_2.test | RUN | 0.433642 | 4.471560 | 1.249947 | 0.001679 | 0.001826 | 0.001520 |
| mp_append_3.test | RUN | 0.017252 | 0.038177 | 0.057139 | 0.000738 | 0.000788 | 0.001751 |
| mp_append_sf.test | RUN | 0.175951 | 0.506326 | 0.576944 | 0.001684 | 0.001874 | 0.001754 |
| mp_replace_front.test | RUN | 0.210317 | 1.465304 | 0.755964 | 0.001708 | 0.001779 | 0.001646 |
| mp_replace_front_2.test | RUN | 0.018465 | 0.035963 | 0.061505 | 0.000687 | 0.000830 | 0.001520 |
| mp_replace_second.test | RUN | 0.208905 | 0.959885 | 0.682333 | 0.001588 | 0.001672 | 0.001742 |
| mp_replace_second_2.test | RUN | 0.019306 | 0.039976 | 0.054412 | 0.000680 | 0.000831 | 0.001521 |
| mp_replace_third.test | RUN | 0.200070 | 0.796851 | 0.657588 | 0.001682 | 0.001991 | 0.001829 |
| mp_replace_third_2.test | RUN | 0.018047 | 0.035894 | 0.058048 | 0.000625 | 0.000686 | 0.001782 |
| mp_apply_q.test | RUN | 0.222728 | 1.759525 | 0.903423 | 0.001618 | 0.001949 | 0.001775 |
| mp_apply_q_sf.test | RUN | 0.182999 | 0.519641 | 0.572326 | 0.001701 | 0.001984 | 0.001855 |
| mp_is_list.test | RUN | 0.183276 | 0.623714 | 0.525512 | 0.001365 | 0.001848 | 0.001653 |
| mp_list_c.test | RUN | 0.188327 | 0.749720 | 0.606002 | 0.001818 | 0.001826 | 0.001883 |
| mp_transform_front.test | RUN | 0.208054 | 1.450748 | 0.722784 | 0.001640 | 0.001806 | 0.001833 |
| mp_transform_front_2.test | RUN | 0.019457 | 0.037707 | 0.059716 | 0.000510 | 0.000844 | 0.001610 |
| mp_transform_second.test | RUN | 0.206765 | 1.229695 | 0.668533 | 0.001582 | 0.001790 | 0.001530 |
| mp_transform_second_2.test | RUN | 0.017618 | 0.036067 | 0.058410 | 0.000739 | 0.000786 | 0.001663 |
| mp_transform_third.test | RUN | 0.196928 | 0.936135 | 0.602630 | 0.001700 | 0.001864 | 0.001686 |
| mp_transform_third_2.test | RUN | 0.018708 | 0.038470 | 0.059263 | 0.000638 | 0.000790 | 0.001398 |
| mp_list_v.test | RUN | 0.017507 | 0.038972 | 0.050390 | 0.000651 | 0.000636 | 0.001585 |
| mp_rename_v.test | RUN | 0.018835 | 0.038698 | 0.057075 | 0.000611 | 0.000702 | 0.001526 |
| mp_is_value_list.test | RUN | 0.178471 | 0.380831 | 0.542137 | 0.001691 | 0.001614 | 0.001885 |
| mp_transform.test | RUN | 0.365641 | 2.959168 | 1.597584 | 0.001716 | 0.001803 | 0.001444 |
| mp_transform_2.test | RUN | 0.183577 | 0.340608 | 0.497155 | 0.001384 | 0.001874 | 0.001630 |
| mp_transform_q.test | RUN | 0.356219 | 2.825672 | 1.552826 | 0.001683 | 0.001725 | 0.001850 |
| mp_transform_q_2.test | RUN | 0.413154 | 0.898436 | 1.990294 | 0.001851 | 0.001803 | 0.001431 |
| mp_transform_sf.test | RUN | 0.192957 | 0.531883 | 0.598716 | 0.001810 | 0.001884 | 0.001772 |
| mp_transform_if.test | RUN | 0.228798 | 1.413108 | 0.841438 | 0.001621 | 0.001862 | 0.001850 |
| mp_transform_if_q.test | RUN | 0.232817 | 1.423245 | 0.875433 | 0.001676 | 0.001803 | 0.001512 |
| mp_filter.test | RUN | 0.207757 | 0.845342 | 0.656795 | 0.001550 | 0.001595 | 0.001815 |
| mp_fill.test | RUN | 0.197696 | 0.633188 | 0.594862 | 0.001771 | 0.001900 | 0.001786 |
| mp_fill_2.test | RUN | 0.033353 | 0.058570 | 0.097202 | 0.000642 | 0.000810 | 0.001730 |
| mp_count.test | RUN | 0.379967 | 1.536316 | 0.975244 | 0.001666 | 0.001775 | 0.001631 |
| mp_count_if.test | RUN | 0.368790 | 1.567388 | 1.086659 | 0.001586 | 0.001619 | 0.001366 |
| mp_count_if_q.test | RUN | 0.211820 | 1.009097 | 0.634885 | 0.001510 | 0.001790 | 0.001277 |
| mp_contains.test | RUN | 0.203802 | 0.624954 | 0.604476 | 0.001561 | 0.001879 | 0.001823 |
| mp_starts_with.test | RUN | 0.194227 | 0.628708 | 0.567958 | 0.001606 | 0.001824 | 0.001717 |
| mp_starts_with_sf.test | RUN | 0.191217 | 0.537891 | 0.580752 | 0.001674 | 0.001904 | 0.001648 |
| mp_repeat.test | RUN | 0.291204 | 2.505767 | 1.113049 | 0.001867 | 0.001782 | 0.001891 |
| mp_repeat_2.test | RUN | 0.031758 | 0.054596 | 0.097262 | 0.000644 | 0.000829 | 0.001778 |
| mp_repeat_3.test | RUN | 0.232012 | 0.375140 | 0.546818 | 0.001853 | 0.001908 | 0.001846 |
| mp_product.test | RUN | 0.221548 | 1.539634 | 0.770652 | 0.001561 | 0.001931 | 0.001796 |
| mp_drop.test | RUN | 0.227319 | 1.898608 | 0.886305 | 0.001555 | 0.001444 | 0.001826 |
| mp_drop_2.test | RUN | 0.032341 | 0.058174 | 0.101791 | 0.000617 | 0.000757 | 0.001740 |
| mp_iota.test | RUN | 0.228291 | 1.586615 | 0.770823 | 0.001365 | 0.001868 | 0.001788 |
| mp_iota_2.test | RUN | 0.243066 | 1.934687 | 0.938330 | 0.001770 | 0.001922 | 0.001812 |
| mp_at.test | RUN | 0.203703 | 0.850018 | 0.698256 | 0.001591 | 0.001701 | 0.001743 |
| mp_at_2.test | RUN | 0.030931 | 0.054318 | 0.099558 | 0.000771 | 0.000782 | 0.001745 |
| mp_at_sf.test | RUN | 0.198417 | 0.518722 | 0.532502 | 0.001616 | 0.001769 | 0.001720 |
| mp_take.test | RUN | 0.288854 | 2.462801 | 1.097692 | 0.001820 | 0.001585 | 0.001675 |
| mp_take_2.test | RUN | 0.032876 | 0.055312 | 0.097834 | 0.000690 | 0.000833 | 0.001477 |
| mp_slice.test | RUN | 0.323937 | 2.557082 | 1.407172 | 0.001832 | 0.001924 | 0.001716 |
| mp_slice_2.test | RUN | 0.032107 | 0.058678 | 0.088528 | 0.000694 | 0.000857 | 0.001801 |
| mp_replace.test | RUN | 0.235445 | 1.305927 | 0.889791 | 0.001792 | 0.002550 | 0.001742 |
| mp_replace_if.test | RUN | 0.214505 | 1.130350 | 0.843919 | 0.001786 | 0.001676 | 0.001832 |
| mp_replace_if_q.test | RUN | 0.222557 | 1.121378 | 0.812809 | 0.001696 | 0.001886 | 0.001844 |
| mp_copy_if.test | RUN | 0.231137 | 1.569918 | 0.800740 | 0.001538 | 0.001830 | 0.001897 |
| mp_copy_if_q.test | RUN | 0.230980 | 1.571161 | 0.807613 | 0.001752 | 0.001574 | 0.001725 |
| mp_remove.test | RUN | 0.219076 | 1.070292 | 0.807023 | 0.001660 | 0.001915 | 0.001432 |
| mp_remove_if.test | RUN | 0.230003 | 1.723229 | 0.871395 | 0.001554 | 0.002130 | 0.001762 |
| mp_remove_if_q.test | RUN | 0.229290 | 1.687542 | 0.798618 | 0.001565 | 0.001585 | 0.001867 |
| mp_partition.test | RUN | 0.229062 | 1.581339 | 0.914990 | 0.001705 | 0.001757 | 0.001716 |
| mp_partition_q.test | RUN | 0.228598 | 1.586719 | 0.925196 | 0.001738 | 0.001918 | 0.001507 |
| mp_sort.test | RUN | 0.217729 | 1.134441 | 0.784809 | 0.001349 | 0.001878 | 0.001722 |
| mp_sort_q.test | RUN | 0.219027 | 1.115333 | 0.791579 | 0.001554 | 0.001400 | 0.001927 |
| mp_find.test | RUN | 0.215985 | 1.206714 | 0.715608 | 0.001600 | 0.001760 | 0.001710 |
| mp_find_if.test | RUN | 0.210340 | 1.131300 | 0.687875 | 0.001757 | 0.001796 | 0.001660 |
| mp_find_if_q.test | RUN | 0.226668 | 1.263654 | 0.779625 | 0.001828 | 0.001796 | 0.001891 |
| mp_reverse.test | RUN | 0.390895 | 3.269059 | 1.484381 | 0.001718 | 0.001781 | 0.001782 |
| mp_fold.test | RUN | 0.655531 | 2.596936 | 3.738646 | 0.001869 | 0.001809 | 0.001942 |
| mp_fold_q.test | RUN | 0.215124 | 1.114948 | 0.783949 | 0.001809 | 0.001782 | 0.001938 |
| mp_fold_q_sf.test | RUN | 0.191840 | 0.513043 | 0.607621 | 0.001879 | 0.001809 | 0.001702 |
| mp_reverse_fold.test | RUN | 0.327533 | 2.448172 | 1.059773 | 0.001696 | 0.001834 | 0.001529 |
| mp_reverse_fold_q.test | RUN | 0.219902 | 1.374008 | 0.810171 | 0.001760 | 0.001615 | 0.001634 |
| mp_unique.test | RUN | 0.217507 | 1.210008 | 0.799216 | 0.001566 | 0.001843 | 0.001920 |
| mp_unique_if.test | RUN | 0.347517 | 2.475046 | 1.215249 | 0.001710 | 0.001599 | 0.001601 |
| mp_unique_if_q.test | RUN | 0.302424 | 2.436052 | 1.013270 | 0.001654 | 0.001737 | 0.001805 |
| mp_all_of.test | RUN | 0.510481 | 1.375398 | 1.341045 | 0.001751 | 0.001872 | 0.001828 |
| mp_all_of_q.test | RUN | 0.194358 | 0.605146 | 0.588174 | 0.001647 | 0.001857 | 0.001751 |
| mp_any_of.test | RUN | 0.497075 | 1.430455 | 1.334344 | 0.001574 | 0.001554 | 0.001917 |
| mp_any_of_q.test | RUN | 0.194670 | 0.614662 | 0.657597 | 0.001790 | 0.001881 | 0.001726 |
| mp_none_of.test | RUN | 0.520893 | 1.401439 | 1.368488 | 0.001777 | 0.001537 | 0.001922 |
| mp_none_of_q.test | RUN | 0.193792 | 0.574126 | 0.555610 | 0.001640 | 0.001678 | 0.001794 |
| mp_replace_at.test | RUN | 0.226053 | 1.235386 | 0.830423 | 0.001622 | 0.001727 | 0.001802 |
| mp_replace_at_c.test | RUN | 0.229484 | 1.238478 | 0.922318 | 0.001630 | 0.001813 | 0.001820 |
| mp_for_each.test | RUN | 0.214304 | 0.851272 | 1.038759 | 0.001294 | 0.001866 | 0.001775 |
| mp_insert.test | RUN | 0.305949 | 2.424172 | 1.309002 | 0.001867 | 0.001977 | 0.001825 |
| mp_insert_2.test | RUN | 0.033117 | 0.058727 | 0.102028 | 0.000682 | 0.000677 | 0.001753 |
| mp_erase.test | RUN | 0.347356 | 2.587492 | 1.533070 | 0.001894 | 0.002270 | 0.001863 |
| mp_erase_2.test | RUN | 0.032224 | 0.057820 | 0.088166 | 0.000656 | 0.000775 | 0.001810 |
| mp_with_index.test | RUN | 0.482391 | 5.328664 | 3.361082 | 0.002143 | 0.002140 | 0.002271 |
| mp_with_index_cx.test | RUN | 0.003548 | 0.018009 | 0.028863 | 0.000648 | 0.000751 | 0.001875 |
| mp_from_sequence.test | RUN | 0.224562 | 1.391876 | 0.925893 | 0.001790 | 0.001622 | 0.001842 |
| mp_from_sequence_2.test | RUN | 0.238960 | 1.647852 | 0.918156 | 0.001603 | 0.001562 | 0.001876 |
| mp_from_sequence_3.test | RUN | 0.201015 | 0.723448 | 0.647089 | 0.001678 | 0.001837 | 0.001792 |
| mp_min_element.test | RUN | 0.196267 | 0.681541 | 0.637403 | 0.001655 | 0.001760 | 0.001708 |
| mp_min_element_2.test | RUN | 0.032420 | 0.055966 | 0.102080 | 0.000650 | 0.000767 | 0.001754 |
| mp_min_element_q.test | RUN | 0.204331 | 0.658567 | 0.605648 | 0.001654 | 0.001722 | 0.001504 |
| mp_min_element_q_2.test | RUN | 0.032125 | 0.057144 | 0.101214 | 0.000598 | 0.000616 | 0.001875 |
| mp_max_element.test | RUN | 0.200374 | 0.664162 | 0.628802 | 0.001780 | 0.001793 | 0.001747 |
| mp_max_element_2.test | RUN | 0.032268 | 0.053999 | 0.099935 | 0.000736 | 0.000780 | 0.001664 |
| mp_max_element_q.test | RUN | 0.204429 | 0.653700 | 0.631494 | 0.001568 | 0.001823 | 0.001822 |
| mp_max_element_q_2.test | RUN | 0.032257 | 0.056964 | 0.100009 | 0.000534 | 0.000769 | 0.001626 |
| mp_nth_element.test | RUN | 0.221353 | 0.944699 | 0.748645 | 0.001689 | 0.001814 | 0.001613 |
| mp_nth_element_q.test | RUN | 0.220775 | 0.934674 | 0.749876 | 0.001852 | 0.001786 | 0.001698 |
| mp_back.test | RUN | 0.207709 | 0.766892 | 0.720773 | 0.001702 | 0.001864 | 0.001807 |
| mp_back_2.test | RUN | 0.031882 | 0.058484 | 0.101324 | 0.000628 | 0.000775 | 0.001681 |
| mp_pop_back.test | RUN | 0.487951 | 5.034453 | 1.435134 | 0.001486 | 0.001748 | 0.001452 |
| mp_pop_back_2.test | RUN | 0.032549 | 0.056506 | 0.102410 | 0.000789 | 0.000617 | 0.001611 |
| mp_flatten.test | RUN | 0.237785 | 1.803418 | 0.885295 | 0.001852 | 0.001893 | 0.001864 |
| mp_rotate_left.test | RUN | 0.237572 | 1.932222 | 1.005216 | 0.001715 | 0.001714 | 0.001778 |
| mp_rotate_right.test | RUN | 0.238503 | 1.935393 | 0.984234 | 0.001670 | 0.001589 | 0.001872 |
| mp_power_set.test | RUN | 0.245676 | 1.854191 | 0.987080 | 0.001816 | 0.001821 | 0.001833 |
| mp_partial_sum.test | RUN | 0.293466 | 2.480874 | 1.170719 | 0.001735 | 0.001604 | 0.001680 |
| mp_iterate.test | RUN | 0.218713 | 0.750867 | 0.816037 | 0.001505 | 0.001952 | 0.001809 |
| mp_pairwise_fold.test | RUN | 0.235193 | 1.700062 | 0.958711 | 0.001600 | 0.001932 | 0.001887 |
| mp_pairwise_fold_q.test | RUN | 0.194212 | 0.501655 | 0.585325 | 0.001968 | 0.001722 | 0.001908 |
| mp_sliding_fold.test | RUN | 0.417087 | 3.242396 | 1.877384 | 0.001668 | 0.001762 | 0.001630 |
| mp_sliding_fold_q.test | RUN | 0.422845 | 3.261559 | 1.870346 | 0.001606 | 0.001715 | 0.001484 |
| mp_intersperse.test | RUN | 0.247339 | 1.233674 | 0.874345 | 0.001742 | 0.002086 | 0.001802 |
| mp_split.test | RUN | 0.299791 | 2.348803 | 1.217062 | 0.001698 | 0.001876 | 0.001966 |
| mp_join.test | RUN | 0.291785 | 1.973687 | 1.088652 | 0.001805 | 0.001800 | 0.001764 |
| integral.test | RUN | 0.202408 | 0.885913 | 0.669926 | 0.001602 | 0.001987 | 0.001837 |
| mp_value.test | RUN | 0.014634 | 0.029665 | 0.045037 | 0.000773 | 0.000756 | 0.001721 |
| mp_identity.test | RUN | 0.187248 | 0.805090 | 0.655455 | 0.001705 | 0.001629 | 0.001503 |
| mp_inherit.test | RUN | 0.187072 | 0.582852 | 0.617233 | 0.001422 | 0.001742 | 0.001554 |
| mp_if.test | RUN | 0.189042 | 0.669771 | 0.601224 | 0.001668 | 0.001810 | 0.001609 |
| mp_if_sf.test | RUN | 0.184485 | 0.496459 | 0.547799 | 0.001581 | 0.001596 | 0.001820 |
| mp_eval_if.test | RUN | 0.188264 | 0.697537 | 0.624771 | 0.001753 | 0.001711 | 0.001470 |
| mp_eval_if_sf.test | RUN | 0.179107 | 0.516128 | 0.578083 | 0.001804 | 0.001675 | 0.001615 |
| mp_valid.test | RUN | 0.181575 | 0.510081 | 0.574769 | 0.001776 | 0.001610 | 0.001828 |
| mp_defer.test | RUN | 0.195330 | 0.852081 | 0.662766 | 0.001520 | 0.001866 | 0.001812 |
| mp_quote.test | RUN | 0.195967 | 0.877958 | 0.705317 | 0.001744 | 0.001896 | 0.001712 |
| mp_invoke_q.test | RUN | 0.211852 | 1.140278 | 0.763621 | 0.001554 | 0.001848 | 0.001860 |
| mp_invoke_q_sf.test | RUN | 0.180849 | 0.515203 | 0.542898 | 0.001826 | 0.001977 | 0.001848 |
| mp_quote_trait.test | RUN | 0.194868 | 0.978838 | 0.682915 | 0.001730 | 0.001696 | 0.001928 |
| mp_cond.test | RUN | 0.185458 | 0.572845 | 0.577531 | 0.001728 | 0.001924 | 0.001807 |
| mp_cond_sf.test | RUN | 0.183742 | 0.509017 | 0.542630 | 0.001482 | 0.001871 | 0.001552 |
| mp_not_fn.test | RUN | 0.180727 | 0.500295 | 0.565051 | 0.001844 | 0.001788 | 0.001836 |
| mp_eval_if_not.test | RUN | 0.184663 | 0.711962 | 0.603358 | 0.001695 | 0.001522 | 0.001897 |
| mp_eval_or.test | RUN | 0.183092 | 0.383286 | 0.584482 | 0.001633 | 0.001770 | 0.001500 |
| mp_compose.test | RUN | 0.202589 | 1.171477 | 0.739925 | 0.001604 | 0.002947 | 0.001625 |
| mp_valid_and_true.test | RUN | 0.188525 | 0.599262 | 0.542297 | 0.001884 | 0.001969 | 0.001887 |
| mp_compose_sf.test | COMPILE | 0.044968 | 0.075265 | 0.139186 | — | — | — |
| integer_sequence.test | RUN | 0.217851 | 1.179416 | 0.792592 | 0.001588 | 0.001674 | 0.001754 |
| tuple_for_each.test | RUN | 0.186433 | 0.446749 | 0.598302 | 0.001528 | 0.003568 | 0.001976 |
| tuple_for_each_cx.test | COMPILE | 0.035500 | 0.065783 | 0.106637 | — | — | — |
| tuple_apply.test | RUN | 0.184148 | 0.483393 | 0.606791 | 0.001692 | 0.001819 | 0.001642 |
| tuple_apply_cx.test | COMPILE | 0.053547 | 0.086235 | 0.154922 | — | — | — |
| construct_from_tuple.test | RUN | 0.194385 | 0.466251 | 0.590517 | 0.001665 | 0.001937 | 0.001831 |
| construct_from_tuple_cx.test | COMPILE | 0.055135 | 0.091697 | 0.129481 | — | — | — |
| tuple_transform.test | RUN | 0.307147 | 0.948365 | 1.203623 | 0.001750 | 0.001636 | 0.001773 |
| tuple_transform_2.test | RUN | 0.267889 | 0.720366 | 1.111951 | 0.001780 | 0.001580 | 0.001786 |
| tuple_transform_cx.test | COMPILE | 0.048552 | 0.084549 | 0.136935 | — | — | — |
| mp_set_contains.test | RUN | 0.192221 | 0.603298 | 0.593052 | 0.001826 | 0.001595 | 0.001758 |
| mp_set_push_back.test | RUN | 0.237811 | 1.856928 | 0.978076 | 0.001846 | 0.001884 | 0.001844 |
| mp_set_push_front.test | RUN | 0.286270 | 2.221605 | 1.146184 | 0.001690 | 0.001830 | 0.001408 |
| mp_is_set.test | RUN | 0.185963 | 0.624632 | 0.592120 | 0.001794 | 0.001908 | 0.001808 |
| mp_set_union.test | RUN | 0.233808 | 1.581499 | 0.966356 | 0.001736 | 0.001934 | 0.001742 |
| mp_set_union_sf.test | RUN | 0.178479 | 0.504265 | 0.543228 | 0.001616 | 0.001858 | 0.001626 |
| mp_set_difference.test | RUN | 0.215515 | 1.234612 | 0.821286 | 0.001775 | 0.002037 | 0.001736 |
| mp_set_difference_sf.test | RUN | 0.187196 | 0.491309 | 0.528837 | 0.001678 | 0.001937 | 0.001759 |
| mp_set_intersection.test | RUN | 0.213069 | 1.234673 | 0.785725 | 0.001628 | 0.001705 | 0.001663 |
| mp_set_intersection_sf.test | RUN | 0.181231 | 0.517047 | 0.535901 | 0.001646 | 0.001865 | 0.001866 |
| mp_all.test | RUN | 0.190560 | 0.596941 | 0.577357 | 0.001918 | 0.001780 | 0.001826 |
| mp_all_2.test | RUN | 0.519326 | 1.319736 | 1.321531 | 0.001846 | 0.001860 | 0.001783 |
| mp_and.test | RUN | 0.190851 | 0.611393 | 0.575744 | 0.001804 | 0.001733 | 0.001774 |
| mp_any.test | RUN | 0.190100 | 0.575334 | 0.572927 | 0.001484 | 0.001887 | 0.001493 |
| mp_any_2.test | RUN | 0.516534 | 1.319078 | 1.282709 | 0.001430 | 0.001759 | 0.001652 |
| mp_or.test | RUN | 0.188945 | 0.597611 | 0.556436 | 0.001768 | 0.002014 | 0.001821 |
| mp_same.test | RUN | 0.347159 | 1.002775 | 0.874693 | 0.001481 | 0.001842 | 0.001712 |
| mp_plus.test | RUN | 0.191406 | 0.713581 | 0.658566 | 0.001802 | 0.001684 | 0.001786 |
| mp_less.test | RUN | 0.193101 | 0.596766 | 0.615849 | 0.001794 | 0.001680 | 0.001936 |
| mp_min.test | RUN | 0.184333 | 0.552572 | 0.541871 | 0.001724 | 0.001901 | 0.001673 |
| mp_max.test | RUN | 0.186723 | 0.561654 | 0.588500 | 0.001448 | 0.001788 | 0.001813 |
| mp_similar.test | RUN | 0.184273 | 0.552218 | 0.593146 | 0.001577 | 0.001908 | 0.001690 |
| mp_map_find.test | RUN | 0.211425 | 1.072467 | 0.767146 | 0.001691 | 0.002003 | 0.001758 |
| mp_map_find_2.test | RUN | 0.202575 | 0.416426 | 0.672807 | 0.001484 | 0.001952 | 0.001754 |
| mp_map_find_3.test | RUN | 0.202159 | 0.424670 | 0.673392 | 0.001834 | 0.002016 | 0.001800 |
| mp_map_find_4.test | RUN | 0.193802 | 0.418430 | 0.583435 | 0.001366 | 0.001824 | 0.001829 |
| mp_map_contains.test | RUN | 0.199975 | 0.621408 | 0.632407 | 0.001780 | 0.002080 | 0.001824 |
| mp_map_insert.test | RUN | 0.283166 | 2.070674 | 1.044535 | 0.001674 | 0.001750 | 0.001857 |
| mp_map_replace.test | RUN | 0.296136 | 2.330887 | 1.210239 | 0.001691 | 0.001914 | 0.001441 |
| mp_map_erase.test | RUN | 0.242110 | 1.763048 | 1.017962 | 0.001821 | 0.001737 | 0.001640 |
| mp_map_update.test | RUN | 0.214998 | 1.142318 | 0.807648 | 0.001813 | 0.001864 | 0.001719 |
| mp_map_update_q.test | RUN | 0.214001 | 1.146457 | 0.789902 | 0.001830 | 0.001776 | 0.001484 |
| mp_map_keys.test | RUN | 0.211047 | 1.011090 | 0.793064 | 0.001338 | 0.001921 | 0.001526 |
| mp_is_map.test | RUN | 0.197782 | 0.611911 | 0.656720 | 0.001532 | 0.001878 | 0.001576 |
| mp_bind.test | RUN | 0.237291 | 1.825527 | 0.919478 | 0.001840 | 0.001750 | 0.001708 |
| mp_bind_q.test | RUN | 0.210228 | 1.146612 | 0.740916 | 0.001737 | 0.001898 | 0.001650 |
| mp_bind_front.test | RUN | 0.209130 | 1.243171 | 0.743454 | 0.001326 | 0.001598 | 0.002386 |
| mp_bind_back.test | RUN | 0.211053 | 1.231869 | 0.750641 | 0.001530 | 0.001826 | 0.001658 |
| mp_lambda.test | RUN | 0.410986 | 3.777284 | 1.860805 | 0.001814 | 0.001424 | 0.001538 |
| mpl.test | RUN | 0.409101 | 0.987998 | 1.505504 | 0.001739 | 0.004357 | 0.001541 |
| mpl_list.test | RUN | 0.253993 | 0.769280 | 0.740364 | 0.001688 | 0.001914 | 0.001529 |
| mpl_tuple.test | RUN | 0.254743 | 0.757910 | 0.736982 | 0.001553 | 0.001998 | 0.001614 |
| version.test | RUN | 0.128041 | 0.304848 | 0.367477 | 0.001756 | 0.001948 | 0.001519 |
| mp11_version.test | RUN | 0.149344 | 0.315060 | 0.427402 | 0.001563 | 0.001777 | 0.001691 |
| list_version.test | RUN | 0.124517 | 0.287065 | 0.394126 | 0.001359 | 0.001552 | 0.001647 |
| algorithm_version.test | RUN | 0.150138 | 0.311831 | 0.425891 | 0.001813 | 0.001891 | 0.001439 |
| integral_version.test | RUN | 0.128035 | 0.273910 | 0.361521 | 0.001414 | 0.001792 | 0.001373 |
| utility_version.test | RUN | 0.128643 | 0.279921 | 0.369395 | 0.001799 | 0.001636 | 0.001743 |
| function_version.test | RUN | 0.132974 | 0.290726 | 0.401206 | 0.001547 | 0.001766 | 0.001600 |
| map_version.test | RUN | 0.142979 | 0.296729 | 0.422047 | 0.001731 | 0.001775 | 0.001465 |
| set_version.test | RUN | 0.130012 | 0.286737 | 0.380697 | 0.001697 | 0.001688 | 0.001419 |
| bind_version.test | RUN | 0.138918 | 0.301506 | 0.410964 | 0.001656 | 0.001914 | 0.001668 |
| integer_sequence_version.test | RUN | 0.121820 | 0.279450 | 0.381131 | 0.001598 | 0.001908 | 0.001164 |
| tuple_version.test | RUN | 0.141169 | 0.297039 | 0.272568 | 0.001391 | 0.001896 | 0.001694 |
| mpl_version.test | RUN | 0.107147 | 0.242021 | 0.411371 | 0.001435 | 0.001453 | 0.001225 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.020×† | 1.026×, 1.014× | 1.082× |
| clang | 1.073× | 1.075×, 1.070× | 1.024× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| algorithm_version.test | 10 | 0.001003s | 0.000994s | 0.000991s | byte equality observed |
| bind_version.test | 10 | 0.000997s | 0.000999s | 0.001033s | byte equality observed |
| construct_from_tuple.test | 10 | 0.000994s | 0.001010s | 0.000992s | byte equality observed |
| function_version.test | 10 | 0.001022s | 0.001016s | 0.001008s | byte equality observed |
| integer_sequence.test | 10 | 0.001017s | 0.001046s | 0.001014s | byte equality observed |
| integer_sequence_version.test | 10 | 0.001035s | 0.001017s | 0.001093s | byte equality observed |
| integral.test | 10 | 0.001074s | 0.001030s | 0.001001s | byte equality observed |
| integral_version.test | 10 | 0.001048s | 0.001019s | 0.001011s | byte equality observed |
| list_version.test | 10 | 0.001022s | 0.001026s | 0.001040s | byte equality observed |
| map_version.test | 10 | 0.001032s | 0.001026s | 0.001023s | byte equality observed |
| mp11_version.test | 10 | 0.001035s | 0.001049s | 0.001021s | byte equality observed |
| mp_all.test | 10 | 0.001043s | 0.001026s | 0.001017s | byte equality observed |
| mp_all_2.test | 10 | 0.001049s | 0.001044s | 0.001031s | byte equality observed |
| mp_all_of.test | 10 | 0.001046s | 0.001069s | 0.001010s | byte equality observed |
| mp_all_of_q.test | 10 | 0.001065s | 0.001031s | 0.001000s | byte equality observed |
| mp_and.test | 10 | 0.001055s | 0.001049s | 0.001023s | byte equality observed |
| mp_any.test | 10 | 0.001047s | 0.001089s | 0.001010s | byte equality observed |
| mp_any_2.test | 10 | 0.001030s | 0.001038s | 0.001018s | byte equality observed |
| mp_any_of.test | 10 | 0.001045s | 0.001063s | 0.001005s | byte equality observed |
| mp_any_of_q.test | 10 | 0.001032s | 0.001020s | 0.001011s | byte equality observed |
| mp_append.test | 10 | 0.001025s | 0.001030s | 0.001035s | byte equality observed |
| mp_append_2.test | 10 | 0.001086s | 0.001063s | 0.001026s | byte equality observed |
| mp_append_3.test | 10 | 0.000487s | 0.000462s | 0.001017s | byte equality observed |
| mp_append_sf.test | 10 | 0.001029s | 0.001067s | 0.000999s | byte equality observed |
| mp_apply_q.test | 10 | 0.001053s | 0.001069s | 0.001027s | byte equality observed |
| mp_apply_q_sf.test | 10 | 0.001019s | 0.001021s | 0.001021s | byte equality observed |
| mp_assign.test | 10 | 0.001044s | 0.001044s | 0.001020s | byte equality observed |
| mp_assign_2.test | 10 | 0.000484s | 0.000479s | 0.000991s | byte equality observed |
| mp_at.test | 10 | 0.001075s | 0.001048s | 0.001013s | byte equality observed |
| mp_at_2.test | 10 | 0.000476s | 0.000484s | 0.001036s | byte equality observed |
| mp_at_sf.test | 10 | 0.001050s | 0.001045s | 0.001032s | byte equality observed |
| mp_back.test | 10 | 0.001053s | 0.001033s | 0.001049s | byte equality observed |
| mp_back_2.test | 10 | 0.000482s | 0.000477s | 0.001004s | byte equality observed |
| mp_bind.test | 10 | 0.001038s | 0.001071s | 0.001015s | byte equality observed |
| mp_bind_back.test | 10 | 0.001028s | 0.001053s | 0.000996s | byte equality observed |
| mp_bind_front.test | 10 | 0.001057s | 0.001020s | 0.001002s | byte equality observed |
| mp_bind_q.test | 10 | 0.001029s | 0.001080s | 0.001017s | byte equality observed |
| mp_clear.test | 10 | 0.001052s | 0.001068s | 0.001015s | byte equality observed |
| mp_clear_2.test | 10 | 0.000487s | 0.000481s | 0.001010s | byte equality observed |
| mp_compose.test | 10 | 0.001044s | 0.001043s | 0.001015s | byte equality observed |
| mp_cond.test | 10 | 0.001049s | 0.001051s | 0.001031s | byte equality observed |
| mp_cond_sf.test | 10 | 0.001020s | 0.001037s | 0.001031s | byte equality observed |
| mp_contains.test | 10 | 0.001040s | 0.001043s | 0.001004s | byte equality observed |
| mp_copy_if.test | 10 | 0.001072s | 0.001057s | 0.001021s | byte equality observed |
| mp_copy_if_q.test | 10 | 0.001029s | 0.001042s | 0.001003s | byte equality observed |
| mp_count.test | 10 | 0.001025s | 0.001050s | 0.001021s | byte equality observed |
| mp_count_if.test | 10 | 0.001031s | 0.001054s | 0.001028s | byte equality observed |
| mp_count_if_q.test | 10 | 0.001033s | 0.001017s | 0.001018s | byte equality observed |
| mp_defer.test | 10 | 0.001051s | 0.001033s | 0.001021s | byte equality observed |
| mp_drop.test | 10 | 0.001048s | 0.001075s | 0.001027s | byte equality observed |
| mp_drop_2.test | 10 | 0.000476s | 0.000473s | 0.001046s | byte equality observed |
| mp_empty.test | 10 | 0.001059s | 0.001072s | 0.001021s | byte equality observed |
| mp_empty_2.test | 10 | 0.000482s | 0.000481s | 0.001019s | byte equality observed |
| mp_erase.test | 10 | 0.001076s | 0.001109s | 0.001031s | byte equality observed |
| mp_erase_2.test | 10 | 0.000483s | 0.000496s | 0.000995s | byte equality observed |
| mp_eval_if.test | 10 | 0.001030s | 0.001054s | 0.001039s | byte equality observed |
| mp_eval_if_not.test | 10 | 0.001032s | 0.001049s | 0.000998s | byte equality observed |
| mp_eval_if_sf.test | 10 | 0.001068s | 0.001054s | 0.000999s | byte equality observed |
| mp_eval_or.test | 10 | 0.001043s | 0.001065s | 0.001030s | byte equality observed |
| mp_fill.test | 10 | 0.001058s | 0.001077s | 0.001040s | byte equality observed |
| mp_fill_2.test | 10 | 0.000486s | 0.000486s | 0.001023s | byte equality observed |
| mp_filter.test | 10 | 0.001043s | 0.001028s | 0.001063s | byte equality observed |
| mp_find.test | 10 | 0.001048s | 0.001066s | 0.001046s | byte equality observed |
| mp_find_if.test | 10 | 0.001033s | 0.001029s | 0.001065s | byte equality observed |
| mp_find_if_q.test | 10 | 0.001035s | 0.001015s | 0.001043s | byte equality observed |
| mp_flatten.test | 10 | 0.001054s | 0.001033s | 0.001030s | byte equality observed |
| mp_fold.test | 10 | 0.001102s | 0.001049s | 0.001050s | byte equality observed |
| mp_fold_q.test | 10 | 0.001031s | 0.001027s | 0.001010s | byte equality observed |
| mp_fold_q_sf.test | 10 | 0.001040s | 0.001034s | 0.001022s | byte equality observed |
| mp_for_each.test | 10 | 0.001054s | 0.001023s | 0.001008s | byte equality observed |
| mp_from_sequence.test | 10 | 0.001049s | 0.001010s | 0.001013s | byte equality observed |
| mp_from_sequence_2.test | 10 | 0.001024s | 0.001021s | 0.001030s | byte equality observed |
| mp_from_sequence_3.test | 10 | 0.001027s | 0.001068s | 0.001036s | byte equality observed |
| mp_front.test | 10 | 0.001054s | 0.001041s | 0.001018s | byte equality observed |
| mp_front_2.test | 10 | 0.000489s | 0.000486s | 0.001041s | byte equality observed |
| mp_identity.test | 10 | 0.001063s | 0.001064s | 0.001036s | byte equality observed |
| mp_if.test | 10 | 0.001032s | 0.001042s | 0.001061s | byte equality observed |
| mp_if_sf.test | 10 | 0.001054s | 0.001026s | 0.001015s | byte equality observed |
| mp_inherit.test | 10 | 0.001032s | 0.001041s | 0.001022s | byte equality observed |
| mp_insert.test | 10 | 0.001067s | 0.001056s | 0.001062s | byte equality observed |
| mp_insert_2.test | 10 | 0.000490s | 0.000478s | 0.001001s | byte equality observed |
| mp_intersperse.test | 10 | 0.001072s | 0.001032s | 0.001040s | byte equality observed |
| mp_invoke_q.test | 10 | 0.001070s | 0.001061s | 0.001053s | byte equality observed |
| mp_invoke_q_sf.test | 10 | 0.001029s | 0.001051s | 0.001025s | byte equality observed |
| mp_iota.test | 10 | 0.001031s | 0.001036s | 0.001016s | byte equality observed |
| mp_iota_2.test | 10 | 0.001039s | 0.001083s | 0.001000s | byte equality observed |
| mp_is_list.test | 10 | 0.001050s | 0.001039s | 0.001021s | byte equality observed |
| mp_is_map.test | 10 | 0.001058s | 0.001028s | 0.001031s | byte equality observed |
| mp_is_set.test | 10 | 0.001028s | 0.001018s | 0.001032s | byte equality observed |
| mp_is_value_list.test | 10 | 0.001052s | 0.001056s | 0.001055s | byte equality observed |
| mp_iterate.test | 10 | 0.001043s | 0.001053s | 0.001048s | byte equality observed |
| mp_join.test | 10 | 0.001080s | 0.001053s | 0.001098s | byte equality observed |
| mp_lambda.test | 10 | 0.001055s | 0.001090s | 0.001052s | byte equality observed |
| mp_less.test | 10 | 0.001068s | 0.001049s | 0.001037s | byte equality observed |
| mp_list_c.test | 10 | 0.001042s | 0.001065s | 0.001019s | byte equality observed |
| mp_list_v.test | 10 | 0.000488s | 0.000487s | 0.001011s | byte equality observed |
| mp_map_contains.test | 10 | 0.001029s | 0.001063s | 0.001011s | byte equality observed |
| mp_map_erase.test | 10 | 0.001027s | 0.001051s | 0.001016s | byte equality observed |
| mp_map_find.test | 10 | 0.001040s | 0.001092s | 0.001048s | byte equality observed |
| mp_map_find_2.test | 10 | 0.001047s | 0.001050s | 0.001040s | byte equality observed |
| mp_map_find_3.test | 10 | 0.001029s | 0.001040s | 0.001034s | byte equality observed |
| mp_map_find_4.test | 10 | 0.001031s | 0.001060s | 0.001033s | byte equality observed |
| mp_map_insert.test | 10 | 0.001039s | 0.001058s | 0.001051s | byte equality observed |
| mp_map_keys.test | 10 | 0.001020s | 0.001083s | 0.001018s | byte equality observed |
| mp_map_replace.test | 10 | 0.001047s | 0.001076s | 0.001062s | byte equality observed |
| mp_map_update.test | 10 | 0.001047s | 0.001033s | 0.001059s | byte equality observed |
| mp_map_update_q.test | 10 | 0.001042s | 0.001049s | 0.001078s | byte equality observed |
| mp_max.test | 10 | 0.001037s | 0.001057s | 0.001076s | byte equality observed |
| mp_max_element.test | 10 | 0.001044s | 0.001081s | 0.001030s | byte equality observed |
| mp_max_element_2.test | 10 | 0.000486s | 0.000482s | 0.001049s | byte equality observed |
| mp_max_element_q.test | 10 | 0.001047s | 0.001098s | 0.001023s | byte equality observed |
| mp_max_element_q_2.test | 10 | 0.000483s | 0.000484s | 0.001038s | byte equality observed |
| mp_min.test | 10 | 0.001029s | 0.001047s | 0.001082s | byte equality observed |
| mp_min_element.test | 10 | 0.001052s | 0.001063s | 0.001048s | byte equality observed |
| mp_min_element_2.test | 10 | 0.000481s | 0.000489s | 0.001053s | byte equality observed |
| mp_min_element_q.test | 10 | 0.001069s | 0.001091s | 0.001015s | byte equality observed |
| mp_min_element_q_2.test | 10 | 0.000485s | 0.000482s | 0.001035s | byte equality observed |
| mp_none_of.test | 10 | 0.001036s | 0.001062s | 0.001040s | byte equality observed |
| mp_none_of_q.test | 10 | 0.001034s | 0.001049s | 0.001041s | byte equality observed |
| mp_not_fn.test | 10 | 0.001034s | 0.001023s | 0.001043s | byte equality observed |
| mp_nth_element.test | 10 | 0.001033s | 0.001037s | 0.001019s | byte equality observed |
| mp_nth_element_q.test | 10 | 0.001035s | 0.001049s | 0.001040s | byte equality observed |
| mp_or.test | 10 | 0.001066s | 0.001073s | 0.001074s | byte equality observed |
| mp_pairwise_fold.test | 10 | 0.001041s | 0.001046s | 0.001044s | byte equality observed |
| mp_pairwise_fold_q.test | 10 | 0.001046s | 0.001044s | 0.001024s | byte equality observed |
| mp_partial_sum.test | 10 | 0.001049s | 0.001057s | 0.001082s | byte equality observed |
| mp_partition.test | 10 | 0.001029s | 0.001072s | 0.001030s | byte equality observed |
| mp_partition_q.test | 10 | 0.001051s | 0.001057s | 0.001017s | byte equality observed |
| mp_plus.test | 10 | 0.001074s | 0.001035s | 0.001069s | byte equality observed |
| mp_pop_back.test | 10 | 0.001074s | 0.001072s | 0.001044s | byte equality observed |
| mp_pop_back_2.test | 10 | 0.000474s | 0.000486s | 0.001057s | byte equality observed |
| mp_pop_front.test | 10 | 0.001039s | 0.001073s | 0.001026s | byte equality observed |
| mp_pop_front_2.test | 10 | 0.000479s | 0.000497s | 0.001035s | byte equality observed |
| mp_power_set.test | 10 | 0.001047s | 0.001066s | 0.001047s | byte equality observed |
| mp_product.test | 10 | 0.001033s | 0.001070s | 0.001034s | byte equality observed |
| mp_push_back.test | 10 | 0.001083s | 0.001078s | 0.001038s | byte equality observed |
| mp_push_back_2.test | 10 | 0.000480s | 0.000485s | 0.001040s | byte equality observed |
| mp_push_front.test | 10 | 0.001078s | 0.001078s | 0.001036s | byte equality observed |
| mp_push_front_2.test | 10 | 0.000484s | 0.000489s | 0.001048s | byte equality observed |
| mp_quote.test | 10 | 0.001045s | 0.001039s | 0.001050s | byte equality observed |
| mp_quote_trait.test | 10 | 0.001033s | 0.001052s | 0.001086s | byte equality observed |
| mp_remove.test | 10 | 0.001059s | 0.001030s | 0.001054s | byte equality observed |
| mp_remove_if.test | 10 | 0.001024s | 0.001073s | 0.001047s | byte equality observed |
| mp_remove_if_q.test | 10 | 0.001056s | 0.001060s | 0.001075s | byte equality observed |
| mp_rename.test | 10 | 0.001052s | 0.001073s | 0.001075s | byte equality observed |
| mp_rename_2.test | 10 | 0.000490s | 0.000493s | 0.001031s | byte equality observed |
| mp_rename_v.test | 10 | 0.000484s | 0.000492s | 0.001018s | byte equality observed |
| mp_repeat.test | 10 | 0.001101s | 0.001094s | 0.001031s | byte equality observed |
| mp_repeat_2.test | 10 | 0.000485s | 0.000489s | 0.001025s | byte equality observed |
| mp_repeat_3.test | 10 | 0.001046s | 0.001046s | 0.001023s | byte equality observed |
| mp_replace.test | 10 | 0.001070s | 0.001075s | 0.001072s | byte equality observed |
| mp_replace_at.test | 10 | 0.001034s | 0.001060s | 0.001116s | byte equality observed |
| mp_replace_at_c.test | 10 | 0.001038s | 0.001069s | 0.001066s | byte equality observed |
| mp_replace_front.test | 10 | 0.001042s | 0.001071s | 0.001029s | byte equality observed |
| mp_replace_front_2.test | 10 | 0.000482s | 0.000485s | 0.001048s | byte equality observed |
| mp_replace_if.test | 10 | 0.001022s | 0.001058s | 0.001046s | byte equality observed |
| mp_replace_if_q.test | 10 | 0.001022s | 0.001069s | 0.001047s | byte equality observed |
| mp_replace_second.test | 10 | 0.001049s | 0.001116s | 0.001061s | byte equality observed |
| mp_replace_second_2.test | 10 | 0.000484s | 0.000476s | 0.001003s | byte equality observed |
| mp_replace_third.test | 10 | 0.001065s | 0.001074s | 0.001040s | byte equality observed |
| mp_replace_third_2.test | 10 | 0.000484s | 0.000497s | 0.001046s | byte equality observed |
| mp_reverse.test | 10 | 0.001063s | 0.001075s | 0.001031s | byte equality observed |
| mp_reverse_fold.test | 10 | 0.001038s | 0.001090s | 0.001061s | byte equality observed |
| mp_reverse_fold_q.test | 10 | 0.001037s | 0.001086s | 0.001062s | byte equality observed |
| mp_rotate_left.test | 10 | 0.001021s | 0.001079s | 0.001069s | byte equality observed |
| mp_rotate_right.test | 10 | 0.001049s | 0.001109s | 0.001064s | byte equality observed |
| mp_same.test | 10 | 0.001035s | 0.001151s | 0.001048s | byte equality observed |
| mp_second.test | 10 | 0.001054s | 0.001086s | 0.001072s | byte equality observed |
| mp_second_2.test | 10 | 0.000484s | 0.000488s | 0.001028s | byte equality observed |
| mp_set_contains.test | 10 | 0.001036s | 0.001075s | 0.001048s | byte equality observed |
| mp_set_difference.test | 10 | 0.001032s | 0.001033s | 0.001086s | byte equality observed |
| mp_set_difference_sf.test | 10 | 0.001037s | 0.001046s | 0.001024s | byte equality observed |
| mp_set_intersection.test | 10 | 0.001035s | 0.001024s | 0.001040s | byte equality observed |
| mp_set_intersection_sf.test | 10 | 0.001023s | 0.001087s | 0.001086s | byte equality observed |
| mp_set_push_back.test | 10 | 0.001028s | 0.001068s | 0.001051s | byte equality observed |
| mp_set_push_front.test | 10 | 0.001069s | 0.001047s | 0.001043s | byte equality observed |
| mp_set_union.test | 10 | 0.001030s | 0.001081s | 0.001026s | byte equality observed |
| mp_set_union_sf.test | 10 | 0.001025s | 0.001030s | 0.001058s | byte equality observed |
| mp_similar.test | 10 | 0.001042s | 0.001036s | 0.001002s | byte equality observed |
| mp_size.test | 10 | 0.001029s | 0.001055s | 0.001039s | byte equality observed |
| mp_size_2.test | 10 | 0.000481s | 0.000494s | 0.001045s | byte equality observed |
| mp_slice.test | 10 | 0.001073s | 0.001115s | 0.001019s | byte equality observed |
| mp_slice_2.test | 10 | 0.000484s | 0.000495s | 0.001021s | byte equality observed |
| mp_sliding_fold.test | 10 | 0.001052s | 0.001090s | 0.001035s | byte equality observed |
| mp_sliding_fold_q.test | 10 | 0.001064s | 0.001119s | 0.001025s | byte equality observed |
| mp_sort.test | 10 | 0.001024s | 0.001053s | 0.001026s | byte equality observed |
| mp_sort_q.test | 10 | 0.001032s | 0.001042s | 0.001008s | byte equality observed |
| mp_split.test | 10 | 0.001067s | 0.001057s | 0.001051s | byte equality observed |
| mp_starts_with.test | 10 | 0.001026s | 0.001021s | 0.001048s | byte equality observed |
| mp_starts_with_sf.test | 10 | 0.001034s | 0.001028s | 0.001013s | byte equality observed |
| mp_take.test | 10 | 0.001069s | 0.001077s | 0.001070s | byte equality observed |
| mp_take_2.test | 10 | 0.000492s | 0.000486s | 0.001023s | byte equality observed |
| mp_third.test | 10 | 0.001066s | 0.001141s | 0.001072s | byte equality observed |
| mp_third_2.test | 10 | 0.000471s | 0.000494s | 0.001035s | byte equality observed |
| mp_transform.test | 10 | 0.001047s | 0.001066s | 0.001046s | byte equality observed |
| mp_transform_2.test | 10 | 0.001027s | 0.001053s | 0.001045s | byte equality observed |
| mp_transform_front.test | 10 | 0.001052s | 0.001098s | 0.001045s | byte equality observed |
| mp_transform_front_2.test | 10 | 0.000487s | 0.000487s | 0.001051s | byte equality observed |
| mp_transform_if.test | 10 | 0.001042s | 0.001089s | 0.001052s | byte equality observed |
| mp_transform_if_q.test | 10 | 0.001047s | 0.001078s | 0.001042s | byte equality observed |
| mp_transform_q.test | 10 | 0.001057s | 0.001066s | 0.001075s | byte equality observed |
| mp_transform_q_2.test | 10 | 0.001034s | 0.001039s | 0.001032s | byte equality observed |
| mp_transform_second.test | 10 | 0.001047s | 0.001063s | 0.001054s | byte equality observed |
| mp_transform_second_2.test | 10 | 0.000482s | 0.000487s | 0.001029s | byte equality observed |
| mp_transform_sf.test | 10 | 0.001054s | 0.001022s | 0.001069s | byte equality observed |
| mp_transform_third.test | 10 | 0.001050s | 0.001052s | 0.001062s | byte equality observed |
| mp_transform_third_2.test | 10 | 0.000489s | 0.000492s | 0.001066s | byte equality observed |
| mp_unique.test | 10 | 0.001053s | 0.001056s | 0.001019s | byte equality observed |
| mp_unique_if.test | 10 | 0.001068s | 0.001060s | 0.001091s | byte equality observed |
| mp_unique_if_q.test | 10 | 0.001073s | 0.001055s | 0.001055s | byte equality observed |
| mp_valid.test | 10 | 0.001050s | 0.001066s | 0.001058s | byte equality observed |
| mp_valid_and_true.test | 10 | 0.001039s | 0.001069s | 0.001057s | byte equality observed |
| mp_value.test | 10 | 0.000481s | 0.000487s | 0.000999s | byte equality observed |
| mp_with_index.test | 10 | 0.001405s | 0.001379s | 0.001290s | byte equality observed |
| mp_with_index_cx.test | 10 | 0.000476s | 0.000477s | 0.001028s | byte equality observed |
| mpl.test | 10 | 0.001044s | 0.001071s | 0.001013s | byte equality observed |
| mpl_list.test | 10 | 0.001038s | 0.001032s | 0.001029s | byte equality observed |
| mpl_tuple.test | 10 | 0.001023s | 0.001041s | 0.001046s | byte equality observed |
| mpl_version.test | 10 | 0.001064s | 0.001021s | 0.001048s | byte equality observed |
| set_version.test | 10 | 0.001085s | 0.001018s | 0.001053s | byte equality observed |
| tuple_apply.test | 10 | 0.001056s | 0.001076s | 0.001045s | byte equality observed |
| tuple_for_each.test | 10 | 0.001045s | 0.001043s | 0.001041s | byte equality observed |
| tuple_transform.test | 10 | 0.001041s | 0.001052s | 0.001042s | byte equality observed |
| tuple_transform_2.test | 10 | 0.001029s | 0.001053s | 0.001035s | byte equality observed |
| tuple_version.test | 10 | 0.001012s | 0.001046s | 0.001050s | byte equality observed |
| utility_version.test | 10 | 0.001027s | 0.001078s | 0.001039s | byte equality observed |
| version.test | 10 | 0.001043s | 0.001056s | 0.001045s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
