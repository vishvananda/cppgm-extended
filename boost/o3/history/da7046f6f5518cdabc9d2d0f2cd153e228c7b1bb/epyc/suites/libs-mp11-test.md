# libs/mp11/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| mp11.test | COMPILE | 0.030132 | 0.051653 | 0.083139 | — | — | — |
| complex_h.test | COMPILE | 0.120150 | 0.216957 | 0.363082 | — | — | — |
| mp_size.test | RUN | 0.140985 | 0.693376 | 0.480343 | 0.001369 | 0.001426 | 0.001990 |
| mp_size_2.test | RUN | 0.013125 | 0.026812 | 0.044359 | 0.000688 | 0.000571 | 0.001626 |
| mp_empty.test | RUN | 0.134671 | 0.429596 | 0.404885 | 0.001293 | 0.001559 | 0.001225 |
| mp_empty_2.test | RUN | 0.013608 | 0.027745 | 0.045787 | 0.000567 | 0.000669 | 0.001602 |
| mp_assign.test | RUN | 0.151538 | 0.929789 | 0.606904 | 0.001250 | 0.001584 | 0.001774 |
| mp_assign_2.test | RUN | 0.013589 | 0.028287 | 0.044342 | 0.000632 | 0.000627 | 0.001368 |
| mp_clear.test | RUN | 0.132082 | 0.399977 | 0.434266 | 0.001508 | 0.001436 | 0.001368 |
| mp_clear_2.test | RUN | 0.013430 | 0.026991 | 0.053199 | 0.000542 | 0.000534 | 0.001775 |
| mp_front.test | RUN | 0.141036 | 0.617134 | 0.491325 | 0.001411 | 0.001459 | 0.001390 |
| mp_front_2.test | RUN | 0.013984 | 0.027339 | 0.050017 | 0.000550 | 0.000686 | 0.001551 |
| mp_pop_front.test | RUN | 0.138314 | 0.664576 | 0.524458 | 0.001265 | 0.001407 | 0.001266 |
| mp_pop_front_2.test | RUN | 0.013785 | 0.028128 | 0.051749 | 0.000512 | 0.000681 | 0.001970 |
| mp_second.test | RUN | 0.140382 | 0.424297 | 0.446415 | 0.001465 | 0.001394 | 0.001349 |
| mp_second_2.test | RUN | 0.014006 | 0.027914 | 0.046212 | 0.000676 | 0.000549 | 0.001961 |
| mp_third.test | RUN | 0.135767 | 0.384632 | 0.477025 | 0.001336 | 0.001464 | 0.002002 |
| mp_third_2.test | RUN | 0.013061 | 0.028013 | 0.047425 | 0.000540 | 0.000594 | 0.001489 |
| mp_push_front.test | RUN | 0.203920 | 1.395027 | 0.800653 | 0.001425 | 0.001534 | 0.001382 |
| mp_push_front_2.test | RUN | 0.014170 | 0.026917 | 0.045102 | 0.000592 | 0.000582 | 0.001698 |
| mp_push_back.test | RUN | 0.199029 | 1.389134 | 0.740085 | 0.001448 | 0.001461 | 0.001529 |
| mp_push_back_2.test | RUN | 0.013561 | 0.028350 | 0.046568 | 0.000667 | 0.000681 | 0.001622 |
| mp_rename.test | RUN | 0.158721 | 1.279970 | 0.647244 | 0.001311 | 0.001596 | 0.001563 |
| mp_rename_2.test | RUN | 0.013753 | 0.028245 | 0.042919 | 0.000561 | 0.000595 | 0.001406 |
| mp_append.test | RUN | 0.167884 | 1.449339 | 0.655574 | 0.001548 | 0.001640 | 0.001536 |
| mp_append_2.test | RUN | 0.312133 | 3.261656 | 0.952663 | 0.001434 | 0.001555 | 0.001584 |
| mp_append_3.test | RUN | 0.014063 | 0.028469 | 0.040928 | 0.000579 | 0.000707 | 0.001794 |
| mp_append_sf.test | RUN | 0.132168 | 0.365058 | 0.401453 | 0.001124 | 0.001370 | 0.001566 |
| mp_replace_front.test | RUN | 0.159326 | 1.034699 | 0.534376 | 0.001404 | 0.001498 | 0.001453 |
| mp_replace_front_2.test | RUN | 0.013478 | 0.027027 | 0.044133 | 0.000513 | 0.000633 | 0.001531 |
| mp_replace_second.test | RUN | 0.150294 | 0.663416 | 0.489440 | 0.001410 | 0.001546 | 0.001245 |
| mp_replace_second_2.test | RUN | 0.013423 | 0.028050 | 0.038786 | 0.000553 | 0.000686 | 0.001507 |
| mp_replace_third.test | RUN | 0.146430 | 0.551703 | 0.462846 | 0.001401 | 0.001604 | 0.001594 |
| mp_replace_third_2.test | RUN | 0.013286 | 0.028188 | 0.043106 | 0.000642 | 0.000631 | 0.001445 |
| mp_apply_q.test | RUN | 0.162254 | 1.275257 | 0.622987 | 0.001389 | 0.001596 | 0.001579 |
| mp_apply_q_sf.test | RUN | 0.134375 | 0.373564 | 0.373404 | 0.001404 | 0.001486 | 0.001538 |
| mp_is_list.test | RUN | 0.139263 | 0.443825 | 0.396915 | 0.001356 | 0.001591 | 0.001492 |
| mp_list_c.test | RUN | 0.140402 | 0.534751 | 0.433287 | 0.001414 | 0.001530 | 0.001540 |
| mp_transform_front.test | RUN | 0.155274 | 1.058276 | 0.537546 | 0.001285 | 0.001441 | 0.001582 |
| mp_transform_front_2.test | RUN | 0.014631 | 0.027937 | 0.041580 | 0.000536 | 0.000675 | 0.001154 |
| mp_transform_second.test | RUN | 0.147922 | 0.899187 | 0.505341 | 0.001485 | 0.001557 | 0.001374 |
| mp_transform_second_2.test | RUN | 0.013889 | 0.028089 | 0.043270 | 0.000596 | 0.000580 | 0.001464 |
| mp_transform_third.test | RUN | 0.142555 | 0.670956 | 0.441758 | 0.001223 | 0.001462 | 0.001288 |
| mp_transform_third_2.test | RUN | 0.012684 | 0.028187 | 0.041811 | 0.000523 | 0.000749 | 0.001236 |
| mp_list_v.test | RUN | 0.013160 | 0.027972 | 0.042576 | 0.000572 | 0.000602 | 0.001446 |
| mp_rename_v.test | RUN | 0.013559 | 0.027664 | 0.038647 | 0.000496 | 0.000557 | 0.001138 |
| mp_is_value_list.test | RUN | 0.129842 | 0.275914 | 0.382324 | 0.001460 | 0.001542 | 0.001572 |
| mp_transform.test | RUN | 0.253974 | 2.183917 | 1.211005 | 0.001571 | 0.001649 | 0.001407 |
| mp_transform_2.test | RUN | 0.126873 | 0.238665 | 0.343978 | 0.001307 | 0.001592 | 0.001452 |
| mp_transform_q.test | RUN | 0.241496 | 2.079844 | 1.167734 | 0.001363 | 0.001437 | 0.001613 |
| mp_transform_q_2.test | RUN | 0.287826 | 0.657765 | 1.545125 | 0.001365 | 0.001482 | 0.001350 |
| mp_transform_sf.test | RUN | 0.132458 | 0.386008 | 0.431816 | 0.001470 | 0.001266 | 0.001728 |
| mp_transform_if.test | RUN | 0.162909 | 0.993065 | 0.623910 | 0.001408 | 0.001461 | 0.001774 |
| mp_transform_if_q.test | RUN | 0.168096 | 0.992062 | 0.637763 | 0.001409 | 0.001270 | 0.001524 |
| mp_filter.test | RUN | 0.144802 | 0.615788 | 0.485534 | 0.001479 | 0.001641 | 0.001640 |
| mp_fill.test | RUN | 0.145038 | 0.472679 | 0.437508 | 0.001337 | 0.001485 | 0.001762 |
| mp_fill_2.test | RUN | 0.023328 | 0.039786 | 0.067335 | 0.000555 | 0.000523 | 0.001609 |
| mp_count.test | RUN | 0.268823 | 1.100623 | 0.715153 | 0.001576 | 0.001438 | 0.001658 |
| mp_count_if.test | RUN | 0.276515 | 1.135290 | 0.779813 | 0.001502 | 0.001485 | 0.001796 |
| mp_count_if_q.test | RUN | 0.145953 | 0.733856 | 0.456357 | 0.001386 | 0.001505 | 0.001716 |
| mp_contains.test | RUN | 0.148370 | 0.450356 | 0.430891 | 0.001452 | 0.001655 | 0.001668 |
| mp_starts_with.test | RUN | 0.139641 | 0.448114 | 0.433093 | 0.001376 | 0.001362 | 0.001437 |
| mp_starts_with_sf.test | RUN | 0.141016 | 0.376079 | 0.420659 | 0.001301 | 0.001494 | 0.001659 |
| mp_repeat.test | RUN | 0.211492 | 1.912333 | 0.821095 | 0.001751 | 0.001606 | 0.001791 |
| mp_repeat_2.test | RUN | 0.022705 | 0.042843 | 0.072241 | 0.000533 | 0.000643 | 0.001590 |
| mp_repeat_3.test | RUN | 0.167414 | 0.243791 | 0.403911 | 0.001431 | 0.001608 | 0.001759 |
| mp_product.test | RUN | 0.160979 | 1.133248 | 0.580499 | 0.001197 | 0.001540 | 0.001694 |
| mp_drop.test | RUN | 0.167462 | 1.452922 | 0.672920 | 0.001238 | 0.001365 | 0.001700 |
| mp_drop_2.test | RUN | 0.022914 | 0.063477 | 0.074672 | 0.000599 | 0.000663 | 0.001516 |
| mp_iota.test | RUN | 0.163059 | 1.173750 | 0.603793 | 0.001237 | 0.001524 | 0.001574 |
| mp_iota_2.test | RUN | 0.174244 | 1.419044 | 0.652580 | 0.001290 | 0.001497 | 0.001570 |
| mp_at.test | RUN | 0.145102 | 0.617435 | 0.484949 | 0.001456 | 0.001319 | 0.001615 |
| mp_at_2.test | RUN | 0.022869 | 0.043800 | 0.064966 | 0.000669 | 0.000583 | 0.001544 |
| mp_at_sf.test | RUN | 0.141259 | 0.390929 | 0.398273 | 0.001328 | 0.001530 | 0.001486 |
| mp_take.test | RUN | 0.213559 | 1.838130 | 0.785826 | 0.001341 | 0.001556 | 0.001602 |
| mp_take_2.test | RUN | 0.023060 | 0.041104 | 0.066615 | 0.000680 | 0.000657 | 0.001606 |
| mp_slice.test | RUN | 0.240044 | 1.867199 | 1.027221 | 0.001424 | 0.001560 | 0.001458 |
| mp_slice_2.test | RUN | 0.022758 | 0.041811 | 0.068327 | 0.000628 | 0.000589 | 0.001708 |
| mp_replace.test | RUN | 0.167889 | 0.932103 | 0.641770 | 0.001409 | 0.001389 | 0.001710 |
| mp_replace_if.test | RUN | 0.159615 | 0.825613 | 0.598952 | 0.001481 | 0.001302 | 0.001614 |
| mp_replace_if_q.test | RUN | 0.159272 | 0.828787 | 0.593929 | 0.001423 | 0.001578 | 0.001551 |
| mp_copy_if.test | RUN | 0.164833 | 1.152833 | 0.567957 | 0.001142 | 0.001494 | 0.001564 |
| mp_copy_if_q.test | RUN | 0.166089 | 1.138809 | 0.568499 | 0.001546 | 0.001309 | 0.001527 |
| mp_remove.test | RUN | 0.162360 | 0.788879 | 0.580364 | 0.001486 | 0.001616 | 0.001434 |
| mp_remove_if.test | RUN | 0.169152 | 1.210011 | 0.606258 | 0.001402 | 0.001602 | 0.001606 |
| mp_remove_if_q.test | RUN | 0.172666 | 1.194970 | 0.599717 | 0.001451 | 0.001454 | 0.001580 |
| mp_partition.test | RUN | 0.170546 | 1.143285 | 0.668567 | 0.001469 | 0.001426 | 0.001307 |
| mp_partition_q.test | RUN | 0.167744 | 1.149194 | 0.648314 | 0.001352 | 0.001617 | 0.001518 |
| mp_sort.test | RUN | 0.158100 | 0.828056 | 0.545990 | 0.001333 | 0.001553 | 0.001608 |
| mp_sort_q.test | RUN | 0.159517 | 0.820551 | 0.543914 | 0.001309 | 0.001504 | 0.001544 |
| mp_find.test | RUN | 0.160850 | 0.881607 | 0.496603 | 0.001492 | 0.001307 | 0.001428 |
| mp_find_if.test | RUN | 0.148525 | 0.809932 | 0.472516 | 0.001629 | 0.001470 | 0.001479 |
| mp_find_if_q.test | RUN | 0.160738 | 0.917775 | 0.540291 | 0.001465 | 0.001514 | 0.001592 |
| mp_reverse.test | RUN | 0.288025 | 2.434640 | 1.142208 | 0.001525 | 0.001376 | 0.001549 |
| mp_fold.test | RUN | 0.486000 | 1.908435 | 2.933801 | 0.001631 | 0.001613 | 0.001619 |
| mp_fold_q.test | RUN | 0.159849 | 0.827736 | 0.518397 | 0.001537 | 0.001576 | 0.001367 |
| mp_fold_q_sf.test | RUN | 0.139441 | 0.386928 | 0.404802 | 0.001288 | 0.001560 | 0.001653 |
| mp_reverse_fold.test | RUN | 0.236141 | 1.812414 | 0.795346 | 0.001431 | 0.001469 | 0.001375 |
| mp_reverse_fold_q.test | RUN | 0.158211 | 1.006891 | 0.595482 | 0.001262 | 0.001597 | 0.001456 |
| mp_unique.test | RUN | 0.156423 | 0.904223 | 0.568527 | 0.001341 | 0.001507 | 0.001587 |
| mp_unique_if.test | RUN | 0.248661 | 1.896609 | 0.905064 | 0.001905 | 0.001532 | 0.001660 |
| mp_unique_if_q.test | RUN | 0.218792 | 1.862377 | 0.741139 | 0.001322 | 0.001516 | 0.001635 |
| mp_all_of.test | RUN | 0.380347 | 1.005601 | 0.954282 | 0.001480 | 0.001606 | 0.001396 |
| mp_all_of_q.test | RUN | 0.140028 | 0.431875 | 0.428015 | 0.001413 | 0.001561 | 0.001582 |
| mp_any_of.test | RUN | 0.387077 | 1.077940 | 0.945080 | 0.001398 | 0.001654 | 0.001451 |
| mp_any_of_q.test | RUN | 0.140279 | 0.441891 | 0.411995 | 0.001436 | 0.001512 | 0.001699 |
| mp_none_of.test | RUN | 0.382575 | 1.054358 | 0.925119 | 0.001499 | 0.001581 | 0.001661 |
| mp_none_of_q.test | RUN | 0.149091 | 0.432933 | 0.419736 | 0.001414 | 0.001602 | 0.001516 |
| mp_replace_at.test | RUN | 0.171245 | 0.919056 | 0.636336 | 0.001325 | 0.001428 | 0.001610 |
| mp_replace_at_c.test | RUN | 0.163733 | 0.919652 | 0.640511 | 0.001400 | 0.001506 | 0.001380 |
| mp_for_each.test | RUN | 0.155246 | 0.595352 | 0.670793 | 0.001375 | 0.001610 | 0.001272 |
| mp_insert.test | RUN | 0.227148 | 1.859909 | 0.931639 | 0.001462 | 0.001538 | 0.001731 |
| mp_insert_2.test | RUN | 0.023082 | 0.042342 | 0.064131 | 0.000665 | 0.000587 | 0.001514 |
| mp_erase.test | RUN | 0.241876 | 1.984019 | 1.105422 | 0.001372 | 0.001591 | 0.001554 |
| mp_erase_2.test | RUN | 0.023318 | 0.042918 | 0.068816 | 0.000564 | 0.000721 | 0.001624 |
| mp_with_index.test | RUN | 0.333491 | 3.970814 | 2.279776 | 0.001888 | 0.001996 | 0.001735 |
| mp_with_index_cx.test | RUN | 0.003031 | 0.014318 | 0.023071 | 0.000635 | 0.000669 | 0.001403 |
| mp_from_sequence.test | RUN | 0.169860 | 1.045702 | 0.607837 | 0.001459 | 0.001397 | 0.001252 |
| mp_from_sequence_2.test | RUN | 0.174702 | 1.212391 | 0.668876 | 0.001422 | 0.001513 | 0.001823 |
| mp_from_sequence_3.test | RUN | 0.145894 | 0.551643 | 0.447404 | 0.001484 | 0.001554 | 0.001586 |
| mp_min_element.test | RUN | 0.148708 | 0.497433 | 0.426593 | 0.001415 | 0.001451 | 0.001552 |
| mp_min_element_2.test | RUN | 0.023166 | 0.041893 | 0.066609 | 0.000545 | 0.000596 | 0.001490 |
| mp_min_element_q.test | RUN | 0.142466 | 0.487556 | 0.443249 | 0.001469 | 0.001612 | 0.001661 |
| mp_min_element_q_2.test | RUN | 0.023230 | 0.045244 | 0.065130 | 0.000545 | 0.000552 | 0.001616 |
| mp_max_element.test | RUN | 0.143944 | 0.497177 | 0.446338 | 0.001397 | 0.001455 | 0.001494 |
| mp_max_element_2.test | RUN | 0.023202 | 0.042412 | 0.067637 | 0.000556 | 0.000518 | 0.001288 |
| mp_max_element_q.test | RUN | 0.144405 | 0.466417 | 0.421494 | 0.001512 | 0.001645 | 0.001613 |
| mp_max_element_q_2.test | RUN | 0.022980 | 0.042345 | 0.068708 | 0.000655 | 0.000555 | 0.001532 |
| mp_nth_element.test | RUN | 0.160591 | 0.683536 | 0.545928 | 0.001467 | 0.001506 | 0.001527 |
| mp_nth_element_q.test | RUN | 0.159055 | 0.682728 | 0.545252 | 0.001321 | 0.001366 | 0.001200 |
| mp_back.test | RUN | 0.160765 | 0.570042 | 0.496957 | 0.001214 | 0.001486 | 0.001603 |
| mp_back_2.test | RUN | 0.023560 | 0.041727 | 0.069639 | 0.000504 | 0.000608 | 0.001419 |
| mp_pop_back.test | RUN | 0.342832 | 3.740608 | 1.100232 | 0.001329 | 0.001602 | 0.001522 |
| mp_pop_back_2.test | RUN | 0.023213 | 0.041802 | 0.064496 | 0.000632 | 0.000632 | 0.001514 |
| mp_flatten.test | RUN | 0.177000 | 1.324892 | 0.711582 | 0.001429 | 0.001517 | 0.001643 |
| mp_rotate_left.test | RUN | 0.170280 | 1.443562 | 0.711146 | 0.001563 | 0.001522 | 0.001640 |
| mp_rotate_right.test | RUN | 0.174000 | 1.429515 | 0.712274 | 0.001454 | 0.001508 | 0.001729 |
| mp_power_set.test | RUN | 0.170915 | 1.345138 | 0.720315 | 0.001591 | 0.001548 | 0.001293 |
| mp_partial_sum.test | RUN | 0.211586 | 1.834064 | 0.824427 | 0.001357 | 0.001490 | 0.001490 |
| mp_iterate.test | RUN | 0.160320 | 0.545588 | 0.559717 | 0.001401 | 0.001571 | 0.001719 |
| mp_pairwise_fold.test | RUN | 0.173548 | 1.266166 | 0.744429 | 0.001135 | 0.001698 | 0.001737 |
| mp_pairwise_fold_q.test | RUN | 0.142552 | 0.374364 | 0.454869 | 0.001610 | 0.001476 | 0.001671 |
| mp_sliding_fold.test | RUN | 0.277631 | 2.450173 | 1.483983 | 0.001389 | 0.001612 | 0.001565 |
| mp_sliding_fold_q.test | RUN | 0.278362 | 2.448483 | 1.475619 | 0.001379 | 0.001594 | 0.001775 |
| mp_intersperse.test | RUN | 0.166275 | 0.930663 | 0.708595 | 0.001432 | 0.001554 | 0.001800 |
| mp_split.test | RUN | 0.210919 | 1.765146 | 0.954112 | 0.001468 | 0.001530 | 0.001736 |
| mp_join.test | RUN | 0.209221 | 1.449268 | 0.837250 | 0.001602 | 0.001588 | 0.001622 |
| integral.test | RUN | 0.141795 | 0.641075 | 0.521564 | 0.001370 | 0.001533 | 0.001580 |
| mp_value.test | RUN | 0.010829 | 0.024585 | 0.039967 | 0.000590 | 0.000624 | 0.001538 |
| mp_identity.test | RUN | 0.143949 | 0.592325 | 0.500534 | 0.001538 | 0.001554 | 0.001573 |
| mp_inherit.test | RUN | 0.136198 | 0.432590 | 0.443290 | 0.001378 | 0.001574 | 0.001669 |
| mp_if.test | RUN | 0.136751 | 0.534713 | 0.464707 | 0.001435 | 0.001483 | 0.001474 |
| mp_if_sf.test | RUN | 0.132098 | 0.424341 | 0.411683 | 0.001325 | 0.001551 | 0.001760 |
| mp_eval_if.test | RUN | 0.138891 | 0.535521 | 0.476971 | 0.001598 | 0.001483 | 0.001710 |
| mp_eval_if_sf.test | RUN | 0.133359 | 0.366755 | 0.441585 | 0.001378 | 0.001587 | 0.001558 |
| mp_valid.test | RUN | 0.133108 | 0.374603 | 0.422938 | 0.001550 | 0.001384 | 0.001752 |
| mp_defer.test | RUN | 0.141794 | 0.621944 | 0.507988 | 0.001346 | 0.001598 | 0.001744 |
| mp_quote.test | RUN | 0.143106 | 0.630984 | 0.560004 | 0.001542 | 0.001616 | 0.001676 |
| mp_invoke_q.test | RUN | 0.153961 | 0.825257 | 0.592934 | 0.001462 | 0.001597 | 0.001794 |
| mp_invoke_q_sf.test | RUN | 0.131684 | 0.371966 | 0.388636 | 0.001497 | 0.001604 | 0.001606 |
| mp_quote_trait.test | RUN | 0.141630 | 0.686708 | 0.507050 | 0.001407 | 0.001513 | 0.001768 |
| mp_cond.test | RUN | 0.134451 | 0.425143 | 0.429102 | 0.001576 | 0.001588 | 0.001648 |
| mp_cond_sf.test | RUN | 0.131282 | 0.373720 | 0.437786 | 0.001444 | 0.001510 | 0.001536 |
| mp_not_fn.test | RUN | 0.132774 | 0.365133 | 0.425453 | 0.001429 | 0.001672 | 0.001582 |
| mp_eval_if_not.test | RUN | 0.137158 | 0.531438 | 0.457519 | 0.001414 | 0.001640 | 0.001710 |
| mp_eval_or.test | RUN | 0.135108 | 0.279100 | 0.433210 | 0.001317 | 0.001612 | 0.001706 |
| mp_compose.test | RUN | 0.150931 | 0.879965 | 0.560265 | 0.001284 | 0.001245 | 0.001615 |
| mp_valid_and_true.test | RUN | 0.137420 | 0.449615 | 0.440956 | 0.001350 | 0.001608 | 0.001453 |
| mp_compose_sf.test | COMPILE | 0.032875 | 0.055832 | 0.107047 | — | — | — |
| integer_sequence.test | RUN | 0.156981 | 0.855729 | 0.648945 | 0.001324 | 0.001386 | 0.001768 |
| tuple_for_each.test | RUN | 0.134293 | 0.306923 | 0.467752 | 0.001299 | 0.001227 | 0.001677 |
| tuple_for_each_cx.test | COMPILE | 0.026573 | 0.046027 | 0.076241 | — | — | — |
| tuple_apply.test | RUN | 0.137716 | 0.307518 | 0.465622 | 0.001436 | 0.001319 | 0.001474 |
| tuple_apply_cx.test | COMPILE | 0.039062 | 0.065640 | 0.108718 | — | — | — |
| construct_from_tuple.test | RUN | 0.136711 | 0.331847 | 0.447605 | 0.001265 | 0.001432 | 0.001455 |
| construct_from_tuple_cx.test | COMPILE | 0.038758 | 0.066354 | 0.095216 | — | — | — |
| tuple_transform.test | RUN | 0.212881 | 0.664485 | 0.777741 | 0.001544 | 0.001388 | 0.001450 |
| tuple_transform_2.test | RUN | 0.181407 | 0.479777 | 0.704018 | 0.001427 | 0.001553 | 0.001560 |
| tuple_transform_cx.test | COMPILE | 0.034471 | 0.061876 | 0.097966 | — | — | — |
| mp_set_contains.test | RUN | 0.140905 | 0.455808 | 0.411196 | 0.001528 | 0.001459 | 0.001354 |
| mp_set_push_back.test | RUN | 0.175362 | 1.367337 | 0.714922 | 0.001515 | 0.001502 | 0.001539 |
| mp_set_push_front.test | RUN | 0.206999 | 1.634341 | 0.821208 | 0.001473 | 0.001459 | 0.001423 |
| mp_is_set.test | RUN | 0.138818 | 0.442153 | 0.396848 | 0.001446 | 0.001506 | 0.001548 |
| mp_set_union.test | RUN | 0.171474 | 1.159528 | 0.683812 | 0.001411 | 0.001770 | 0.001286 |
| mp_set_union_sf.test | RUN | 0.133148 | 0.359525 | 0.370666 | 0.001399 | 0.001524 | 0.001390 |
| mp_set_difference.test | RUN | 0.155233 | 0.911955 | 0.563522 | 0.001574 | 0.001587 | 0.001661 |
| mp_set_difference_sf.test | RUN | 0.133931 | 0.363633 | 0.376230 | 0.001209 | 0.001485 | 0.001387 |
| mp_set_intersection.test | RUN | 0.154645 | 0.906136 | 0.559957 | 0.001433 | 0.001496 | 0.001524 |
| mp_set_intersection_sf.test | RUN | 0.132828 | 0.353797 | 0.376011 | 0.001388 | 0.001502 | 0.001474 |
| mp_all.test | RUN | 0.140851 | 0.444252 | 0.402310 | 0.001638 | 0.001542 | 0.001625 |
| mp_all_2.test | RUN | 0.377142 | 0.932531 | 0.940725 | 0.001503 | 0.001579 | 0.001634 |
| mp_and.test | RUN | 0.132987 | 0.447441 | 0.393771 | 0.001366 | 0.001682 | 0.001638 |
| mp_any.test | RUN | 0.139070 | 0.459208 | 0.392848 | 0.001342 | 0.001669 | 0.001635 |
| mp_any_2.test | RUN | 0.385471 | 0.927795 | 0.918988 | 0.001309 | 0.001566 | 0.001347 |
| mp_or.test | RUN | 0.138038 | 0.454005 | 0.415992 | 0.001455 | 0.001523 | 0.001610 |
| mp_same.test | RUN | 0.260789 | 0.744459 | 0.665197 | 0.001369 | 0.001618 | 0.001414 |
| mp_plus.test | RUN | 0.140125 | 0.526231 | 0.450232 | 0.001492 | 0.001406 | 0.001386 |
| mp_less.test | RUN | 0.138958 | 0.448773 | 0.395481 | 0.001422 | 0.001566 | 0.001535 |
| mp_min.test | RUN | 0.137685 | 0.400989 | 0.402409 | 0.001377 | 0.001365 | 0.001623 |
| mp_max.test | RUN | 0.138270 | 0.446299 | 0.405962 | 0.001374 | 0.001584 | 0.001396 |
| mp_similar.test | RUN | 0.134661 | 0.442960 | 0.404004 | 0.001365 | 0.001371 | 0.001538 |
| mp_map_find.test | RUN | 0.155726 | 0.800964 | 0.537739 | 0.001371 | 0.001415 | 0.001549 |
| mp_map_find_2.test | RUN | 0.149693 | 0.314225 | 0.448026 | 0.001201 | 0.001414 | 0.001444 |
| mp_map_find_3.test | RUN | 0.146233 | 0.307901 | 0.468043 | 0.001374 | 0.001393 | 0.001346 |
| mp_map_find_4.test | RUN | 0.139989 | 0.302640 | 0.428362 | 0.001207 | 0.001206 | 0.001447 |
| mp_map_contains.test | RUN | 0.141102 | 0.459492 | 0.413282 | 0.001542 | 0.001608 | 0.001547 |
| mp_map_insert.test | RUN | 0.206432 | 1.508429 | 0.777355 | 0.001350 | 0.001612 | 0.001510 |
| mp_map_replace.test | RUN | 0.219773 | 1.728258 | 0.878617 | 0.001306 | 0.001641 | 0.001396 |
| mp_map_erase.test | RUN | 0.178161 | 1.275083 | 0.749308 | 0.001455 | 0.001624 | 0.001452 |
| mp_map_update.test | RUN | 0.157912 | 0.834538 | 0.571701 | 0.001610 | 0.001548 | 0.001536 |
| mp_map_update_q.test | RUN | 0.159518 | 0.828889 | 0.569519 | 0.001404 | 0.001516 | 0.001344 |
| mp_map_keys.test | RUN | 0.153627 | 0.756611 | 0.559445 | 0.001420 | 0.001579 | 0.001314 |
| mp_is_map.test | RUN | 0.143000 | 0.460064 | 0.436201 | 0.001240 | 0.001497 | 0.001460 |
| mp_bind.test | RUN | 0.175296 | 1.361309 | 0.663711 | 0.001657 | 0.001494 | 0.001509 |
| mp_bind_q.test | RUN | 0.155771 | 0.836011 | 0.535372 | 0.001468 | 0.001652 | 0.001607 |
| mp_bind_front.test | RUN | 0.156552 | 0.908294 | 0.500483 | 0.001213 | 0.001554 | 0.001420 |
| mp_bind_back.test | RUN | 0.154737 | 0.898047 | 0.524960 | 0.001140 | 0.001290 | 0.001352 |
| mp_lambda.test | RUN | 0.293695 | 2.826124 | 1.390353 | 0.001187 | 0.001110 | 0.001408 |
| mpl.test | RUN | 0.287117 | 0.728212 | 1.113048 | 0.001527 | 0.001601 | 0.001524 |
| mpl_list.test | RUN | 0.182286 | 0.538207 | 0.503723 | 0.001198 | 0.001578 | 0.001600 |
| mpl_tuple.test | RUN | 0.176077 | 0.546606 | 0.503332 | 0.001460 | 0.001364 | 0.001343 |
| version.test | RUN | 0.091169 | 0.197477 | 0.261235 | 0.001476 | 0.001524 | 0.001277 |
| mp11_version.test | RUN | 0.112295 | 0.234422 | 0.301061 | 0.001296 | 0.001501 | 0.001433 |
| list_version.test | RUN | 0.093981 | 0.201689 | 0.267481 | 0.001348 | 0.001567 | 0.001442 |
| algorithm_version.test | RUN | 0.113260 | 0.213380 | 0.285480 | 0.001507 | 0.001570 | 0.001384 |
| integral_version.test | RUN | 0.089921 | 0.204551 | 0.261233 | 0.001184 | 0.001405 | 0.001523 |
| utility_version.test | RUN | 0.090082 | 0.199607 | 0.260578 | 0.001412 | 0.001429 | 0.001499 |
| function_version.test | RUN | 0.093611 | 0.210573 | 0.265811 | 0.001286 | 0.001551 | 0.001471 |
| map_version.test | RUN | 0.105470 | 0.213827 | 0.289461 | 0.001614 | 0.001401 | 0.001451 |
| set_version.test | RUN | 0.092918 | 0.210034 | 0.265003 | 0.001427 | 0.001311 | 0.001366 |
| bind_version.test | RUN | 0.102673 | 0.222282 | 0.282869 | 0.001320 | 0.001309 | 0.001403 |
| integer_sequence_version.test | RUN | 0.087606 | 0.194845 | 0.279820 | 0.001014 | 0.001391 | 0.001273 |
| tuple_version.test | RUN | 0.096798 | 0.216274 | 0.278495 | 0.001173 | 0.001581 | 0.001042 |
| mpl_version.test | RUN | 0.077140 | 0.157632 | 0.232368 | 0.001115 | 0.001173 | 0.001188 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.013× | 1.014×, 1.011× | 1.006× |
| clang | 1.101× | 1.118×, 1.085× | 0.953× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| algorithm_version.test | 10 | 0.000943s | 0.000908s | 0.001015s | byte equality observed |
| bind_version.test | 10 | 0.000919s | 0.000911s | 0.001003s | byte equality observed |
| construct_from_tuple.test | 10 | 0.000917s | 0.000937s | 0.001010s | byte equality observed |
| function_version.test | 10 | 0.000935s | 0.000907s | 0.000988s | byte equality observed |
| integer_sequence.test | 10 | 0.000907s | 0.000945s | 0.001025s | byte equality observed |
| integer_sequence_version.test | 10 | 0.000941s | 0.000906s | 0.001000s | byte equality observed |
| integral.test | 10 | 0.000921s | 0.000956s | 0.001002s | byte equality observed |
| integral_version.test | 10 | 0.000930s | 0.000948s | 0.001039s | byte equality observed |
| list_version.test | 10 | 0.000935s | 0.000955s | 0.001041s | byte equality observed |
| map_version.test | 10 | 0.000962s | 0.000939s | 0.000989s | byte equality observed |
| mp11_version.test | 10 | 0.000951s | 0.000940s | 0.001030s | byte equality observed |
| mp_all.test | 10 | 0.000954s | 0.000942s | 0.001002s | byte equality observed |
| mp_all_2.test | 10 | 0.000936s | 0.000940s | 0.000995s | byte equality observed |
| mp_all_of.test | 10 | 0.000950s | 0.000936s | 0.001007s | byte equality observed |
| mp_all_of_q.test | 10 | 0.000963s | 0.000927s | 0.001029s | byte equality observed |
| mp_and.test | 10 | 0.000962s | 0.000921s | 0.001015s | byte equality observed |
| mp_any.test | 10 | 0.000958s | 0.000960s | 0.000999s | byte equality observed |
| mp_any_2.test | 10 | 0.000966s | 0.000954s | 0.000994s | byte equality observed |
| mp_any_of.test | 10 | 0.000960s | 0.000944s | 0.000996s | byte equality observed |
| mp_any_of_q.test | 10 | 0.000950s | 0.000925s | 0.001012s | byte equality observed |
| mp_append.test | 10 | 0.000954s | 0.000934s | 0.001008s | byte equality observed |
| mp_append_2.test | 10 | 0.000996s | 0.000968s | 0.001014s | byte equality observed |
| mp_append_3.test | 10 | 0.000441s | 0.000432s | 0.000983s | byte equality observed |
| mp_append_sf.test | 10 | 0.000948s | 0.000942s | 0.001022s | byte equality observed |
| mp_apply_q.test | 10 | 0.000963s | 0.000948s | 0.001013s | byte equality observed |
| mp_apply_q_sf.test | 10 | 0.000970s | 0.000933s | 0.001052s | byte equality observed |
| mp_assign.test | 10 | 0.000960s | 0.000945s | 0.001012s | byte equality observed |
| mp_assign_2.test | 10 | 0.000444s | 0.000435s | 0.000991s | byte equality observed |
| mp_at.test | 10 | 0.000988s | 0.000942s | 0.000988s | byte equality observed |
| mp_at_2.test | 10 | 0.000449s | 0.000440s | 0.000978s | byte equality observed |
| mp_at_sf.test | 10 | 0.000955s | 0.000943s | 0.001014s | byte equality observed |
| mp_back.test | 10 | 0.000982s | 0.000958s | 0.001010s | byte equality observed |
| mp_back_2.test | 10 | 0.000450s | 0.000438s | 0.000993s | byte equality observed |
| mp_bind.test | 10 | 0.000960s | 0.000962s | 0.001006s | byte equality observed |
| mp_bind_back.test | 10 | 0.000955s | 0.000961s | 0.001010s | byte equality observed |
| mp_bind_front.test | 10 | 0.000982s | 0.000967s | 0.001018s | byte equality observed |
| mp_bind_q.test | 10 | 0.000970s | 0.000950s | 0.001013s | byte equality observed |
| mp_clear.test | 10 | 0.000973s | 0.000940s | 0.000999s | byte equality observed |
| mp_clear_2.test | 10 | 0.000457s | 0.000454s | 0.000990s | byte equality observed |
| mp_compose.test | 10 | 0.000980s | 0.000948s | 0.000991s | byte equality observed |
| mp_cond.test | 10 | 0.000955s | 0.000944s | 0.001011s | byte equality observed |
| mp_cond_sf.test | 10 | 0.000953s | 0.000935s | 0.001006s | byte equality observed |
| mp_contains.test | 10 | 0.000978s | 0.000945s | 0.001000s | byte equality observed |
| mp_copy_if.test | 10 | 0.000968s | 0.000973s | 0.001012s | byte equality observed |
| mp_copy_if_q.test | 10 | 0.000947s | 0.000945s | 0.001004s | byte equality observed |
| mp_count.test | 10 | 0.000962s | 0.000932s | 0.000998s | byte equality observed |
| mp_count_if.test | 10 | 0.000944s | 0.000950s | 0.001000s | byte equality observed |
| mp_count_if_q.test | 10 | 0.000956s | 0.000952s | 0.000991s | byte equality observed |
| mp_defer.test | 10 | 0.000983s | 0.000941s | 0.000999s | byte equality observed |
| mp_drop.test | 10 | 0.000950s | 0.000968s | 0.000994s | byte equality observed |
| mp_drop_2.test | 10 | 0.000441s | 0.000438s | 0.000976s | byte equality observed |
| mp_empty.test | 10 | 0.000962s | 0.000947s | 0.001023s | byte equality observed |
| mp_empty_2.test | 10 | 0.000442s | 0.000435s | 0.000961s | byte equality observed |
| mp_erase.test | 10 | 0.001003s | 0.000971s | 0.001001s | byte equality observed |
| mp_erase_2.test | 10 | 0.000443s | 0.000429s | 0.000977s | byte equality observed |
| mp_eval_if.test | 10 | 0.001000s | 0.000951s | 0.001005s | byte equality observed |
| mp_eval_if_not.test | 10 | 0.000965s | 0.000945s | 0.000991s | byte equality observed |
| mp_eval_if_sf.test | 10 | 0.001001s | 0.000936s | 0.001001s | byte equality observed |
| mp_eval_or.test | 10 | 0.000962s | 0.000939s | 0.001004s | byte equality observed |
| mp_fill.test | 10 | 0.000997s | 0.000986s | 0.001005s | byte equality observed |
| mp_fill_2.test | 10 | 0.000441s | 0.000446s | 0.000988s | byte equality observed |
| mp_filter.test | 10 | 0.000968s | 0.000961s | 0.001004s | byte equality observed |
| mp_find.test | 10 | 0.000969s | 0.000962s | 0.000986s | byte equality observed |
| mp_find_if.test | 10 | 0.000957s | 0.000930s | 0.001018s | byte equality observed |
| mp_find_if_q.test | 10 | 0.000957s | 0.000967s | 0.001001s | byte equality observed |
| mp_flatten.test | 10 | 0.000957s | 0.000953s | 0.001020s | byte equality observed |
| mp_fold.test | 10 | 0.001009s | 0.000939s | 0.001028s | byte equality observed |
| mp_fold_q.test | 10 | 0.000953s | 0.000927s | 0.001023s | byte equality observed |
| mp_fold_q_sf.test | 10 | 0.000941s | 0.000956s | 0.001011s | byte equality observed |
| mp_for_each.test | 10 | 0.000991s | 0.000928s | 0.001009s | byte equality observed |
| mp_from_sequence.test | 10 | 0.000969s | 0.000991s | 0.001011s | byte equality observed |
| mp_from_sequence_2.test | 10 | 0.000961s | 0.000962s | 0.001011s | byte equality observed |
| mp_from_sequence_3.test | 10 | 0.000949s | 0.000960s | 0.001013s | byte equality observed |
| mp_front.test | 10 | 0.000976s | 0.000958s | 0.000990s | byte equality observed |
| mp_front_2.test | 10 | 0.000447s | 0.000437s | 0.001001s | byte equality observed |
| mp_identity.test | 10 | 0.000942s | 0.000945s | 0.001021s | byte equality observed |
| mp_if.test | 10 | 0.000956s | 0.000957s | 0.001014s | byte equality observed |
| mp_if_sf.test | 10 | 0.000966s | 0.000938s | 0.001025s | byte equality observed |
| mp_inherit.test | 10 | 0.000940s | 0.000941s | 0.001025s | byte equality observed |
| mp_insert.test | 10 | 0.000997s | 0.000962s | 0.001013s | byte equality observed |
| mp_insert_2.test | 10 | 0.000441s | 0.000437s | 0.000985s | byte equality observed |
| mp_intersperse.test | 10 | 0.000980s | 0.000951s | 0.001055s | byte equality observed |
| mp_invoke_q.test | 10 | 0.000970s | 0.000968s | 0.001033s | byte equality observed |
| mp_invoke_q_sf.test | 10 | 0.000983s | 0.000955s | 0.001012s | byte equality observed |
| mp_iota.test | 10 | 0.000952s | 0.000951s | 0.001017s | byte equality observed |
| mp_iota_2.test | 10 | 0.000972s | 0.000972s | 0.001010s | byte equality observed |
| mp_is_list.test | 10 | 0.000953s | 0.000963s | 0.001029s | byte equality observed |
| mp_is_map.test | 10 | 0.000952s | 0.000953s | 0.001030s | byte equality observed |
| mp_is_set.test | 10 | 0.000966s | 0.000961s | 0.001060s | byte equality observed |
| mp_is_value_list.test | 10 | 0.000949s | 0.000968s | 0.001012s | byte equality observed |
| mp_iterate.test | 10 | 0.000960s | 0.000966s | 0.001025s | byte equality observed |
| mp_join.test | 10 | 0.000971s | 0.000969s | 0.001028s | byte equality observed |
| mp_lambda.test | 10 | 0.000973s | 0.000985s | 0.001027s | byte equality observed |
| mp_less.test | 10 | 0.000965s | 0.000961s | 0.001034s | byte equality observed |
| mp_list_c.test | 10 | 0.000930s | 0.000966s | 0.001022s | byte equality observed |
| mp_list_v.test | 10 | 0.000451s | 0.000444s | 0.001003s | byte equality observed |
| mp_map_contains.test | 10 | 0.000951s | 0.000953s | 0.001018s | byte equality observed |
| mp_map_erase.test | 10 | 0.000937s | 0.000962s | 0.001011s | byte equality observed |
| mp_map_find.test | 10 | 0.000955s | 0.000943s | 0.001034s | byte equality observed |
| mp_map_find_2.test | 10 | 0.000941s | 0.000935s | 0.001016s | byte equality observed |
| mp_map_find_3.test | 10 | 0.000919s | 0.000924s | 0.001007s | byte equality observed |
| mp_map_find_4.test | 10 | 0.000935s | 0.000941s | 0.001022s | byte equality observed |
| mp_map_insert.test | 10 | 0.000971s | 0.000975s | 0.001008s | byte equality observed |
| mp_map_keys.test | 10 | 0.000960s | 0.000939s | 0.001002s | byte equality observed |
| mp_map_replace.test | 10 | 0.000964s | 0.000945s | 0.001011s | byte equality observed |
| mp_map_update.test | 10 | 0.000967s | 0.000942s | 0.001029s | byte equality observed |
| mp_map_update_q.test | 10 | 0.000976s | 0.000933s | 0.001015s | byte equality observed |
| mp_max.test | 10 | 0.000995s | 0.000941s | 0.000979s | byte equality observed |
| mp_max_element.test | 10 | 0.000992s | 0.000949s | 0.001038s | byte equality observed |
| mp_max_element_2.test | 10 | 0.000452s | 0.000432s | 0.000982s | byte equality observed |
| mp_max_element_q.test | 10 | 0.000989s | 0.000957s | 0.001021s | byte equality observed |
| mp_max_element_q_2.test | 10 | 0.000456s | 0.000447s | 0.000974s | byte equality observed |
| mp_min.test | 10 | 0.000956s | 0.000941s | 0.001005s | byte equality observed |
| mp_min_element.test | 10 | 0.000960s | 0.000953s | 0.001020s | byte equality observed |
| mp_min_element_2.test | 10 | 0.000444s | 0.000444s | 0.000998s | byte equality observed |
| mp_min_element_q.test | 10 | 0.000986s | 0.000969s | 0.001001s | byte equality observed |
| mp_min_element_q_2.test | 10 | 0.000433s | 0.000446s | 0.000974s | byte equality observed |
| mp_none_of.test | 10 | 0.000933s | 0.000953s | 0.001001s | byte equality observed |
| mp_none_of_q.test | 10 | 0.000925s | 0.000958s | 0.000996s | byte equality observed |
| mp_not_fn.test | 10 | 0.000938s | 0.000969s | 0.001012s | byte equality observed |
| mp_nth_element.test | 10 | 0.000942s | 0.000962s | 0.000987s | byte equality observed |
| mp_nth_element_q.test | 10 | 0.000954s | 0.000970s | 0.001003s | byte equality observed |
| mp_or.test | 10 | 0.000944s | 0.000968s | 0.001028s | byte equality observed |
| mp_pairwise_fold.test | 10 | 0.000950s | 0.000961s | 0.001026s | byte equality observed |
| mp_pairwise_fold_q.test | 10 | 0.000947s | 0.000959s | 0.001019s | byte equality observed |
| mp_partial_sum.test | 10 | 0.000961s | 0.000987s | 0.001037s | byte equality observed |
| mp_partition.test | 10 | 0.000960s | 0.000943s | 0.001005s | byte equality observed |
| mp_partition_q.test | 10 | 0.000947s | 0.000956s | 0.001002s | byte equality observed |
| mp_plus.test | 10 | 0.000940s | 0.000951s | 0.001011s | byte equality observed |
| mp_pop_back.test | 10 | 0.000987s | 0.000989s | 0.001008s | byte equality observed |
| mp_pop_back_2.test | 10 | 0.000445s | 0.000445s | 0.001000s | byte equality observed |
| mp_pop_front.test | 10 | 0.000958s | 0.000978s | 0.001007s | byte equality observed |
| mp_pop_front_2.test | 10 | 0.000444s | 0.000445s | 0.000984s | byte equality observed |
| mp_power_set.test | 10 | 0.000927s | 0.000952s | 0.001046s | byte equality observed |
| mp_product.test | 10 | 0.000945s | 0.000956s | 0.001013s | byte equality observed |
| mp_push_back.test | 10 | 0.000979s | 0.000964s | 0.000999s | byte equality observed |
| mp_push_back_2.test | 10 | 0.000428s | 0.000440s | 0.000986s | byte equality observed |
| mp_push_front.test | 10 | 0.000987s | 0.000960s | 0.001018s | byte equality observed |
| mp_push_front_2.test | 10 | 0.000446s | 0.000455s | 0.000970s | byte equality observed |
| mp_quote.test | 10 | 0.000939s | 0.000962s | 0.001008s | byte equality observed |
| mp_quote_trait.test | 10 | 0.000920s | 0.000984s | 0.000999s | byte equality observed |
| mp_remove.test | 10 | 0.000938s | 0.000957s | 0.001002s | byte equality observed |
| mp_remove_if.test | 10 | 0.000951s | 0.000981s | 0.001001s | byte equality observed |
| mp_remove_if_q.test | 10 | 0.000926s | 0.000972s | 0.000983s | byte equality observed |
| mp_rename.test | 10 | 0.000948s | 0.000959s | 0.001011s | byte equality observed |
| mp_rename_2.test | 10 | 0.000439s | 0.000443s | 0.000999s | byte equality observed |
| mp_rename_v.test | 10 | 0.000438s | 0.000433s | 0.000982s | byte equality observed |
| mp_repeat.test | 10 | 0.000972s | 0.000969s | 0.001006s | byte equality observed |
| mp_repeat_2.test | 10 | 0.000442s | 0.000453s | 0.000980s | byte equality observed |
| mp_repeat_3.test | 10 | 0.000949s | 0.000948s | 0.001008s | byte equality observed |
| mp_replace.test | 10 | 0.000941s | 0.000963s | 0.001022s | byte equality observed |
| mp_replace_at.test | 10 | 0.000950s | 0.000981s | 0.001006s | byte equality observed |
| mp_replace_at_c.test | 10 | 0.000917s | 0.000963s | 0.001021s | byte equality observed |
| mp_replace_front.test | 10 | 0.000970s | 0.000981s | 0.001003s | byte equality observed |
| mp_replace_front_2.test | 10 | 0.000441s | 0.000434s | 0.000996s | byte equality observed |
| mp_replace_if.test | 10 | 0.000957s | 0.000962s | 0.001003s | byte equality observed |
| mp_replace_if_q.test | 10 | 0.000958s | 0.000953s | 0.001014s | byte equality observed |
| mp_replace_second.test | 10 | 0.000969s | 0.000968s | 0.000974s | byte equality observed |
| mp_replace_second_2.test | 10 | 0.000445s | 0.000436s | 0.000995s | byte equality observed |
| mp_replace_third.test | 10 | 0.000953s | 0.000966s | 0.001016s | byte equality observed |
| mp_replace_third_2.test | 10 | 0.000441s | 0.000452s | 0.000970s | byte equality observed |
| mp_reverse.test | 10 | 0.000973s | 0.000951s | 0.001002s | byte equality observed |
| mp_reverse_fold.test | 10 | 0.000997s | 0.000954s | 0.001003s | byte equality observed |
| mp_reverse_fold_q.test | 10 | 0.000925s | 0.000947s | 0.000987s | byte equality observed |
| mp_rotate_left.test | 10 | 0.000958s | 0.000955s | 0.000992s | byte equality observed |
| mp_rotate_right.test | 10 | 0.000964s | 0.000946s | 0.001024s | byte equality observed |
| mp_same.test | 10 | 0.000958s | 0.000931s | 0.000986s | byte equality observed |
| mp_second.test | 10 | 0.000949s | 0.000973s | 0.000993s | byte equality observed |
| mp_second_2.test | 10 | 0.000441s | 0.000434s | 0.000967s | byte equality observed |
| mp_set_contains.test | 10 | 0.000954s | 0.000940s | 0.000996s | byte equality observed |
| mp_set_difference.test | 10 | 0.000957s | 0.000989s | 0.000989s | byte equality observed |
| mp_set_difference_sf.test | 10 | 0.000945s | 0.000938s | 0.000985s | byte equality observed |
| mp_set_intersection.test | 10 | 0.000947s | 0.000969s | 0.000982s | byte equality observed |
| mp_set_intersection_sf.test | 10 | 0.000922s | 0.000929s | 0.000999s | byte equality observed |
| mp_set_push_back.test | 10 | 0.000964s | 0.000955s | 0.000992s | byte equality observed |
| mp_set_push_front.test | 10 | 0.001012s | 0.000982s | 0.001002s | byte equality observed |
| mp_set_union.test | 10 | 0.000939s | 0.000951s | 0.001003s | byte equality observed |
| mp_set_union_sf.test | 10 | 0.000954s | 0.000946s | 0.000994s | byte equality observed |
| mp_similar.test | 10 | 0.000967s | 0.000953s | 0.000991s | byte equality observed |
| mp_size.test | 10 | 0.000975s | 0.000961s | 0.000998s | byte equality observed |
| mp_size_2.test | 10 | 0.000440s | 0.000447s | 0.000975s | byte equality observed |
| mp_slice.test | 10 | 0.001007s | 0.000977s | 0.000989s | byte equality observed |
| mp_slice_2.test | 10 | 0.000453s | 0.000428s | 0.000982s | byte equality observed |
| mp_sliding_fold.test | 10 | 0.000968s | 0.000970s | 0.000983s | byte equality observed |
| mp_sliding_fold_q.test | 10 | 0.000987s | 0.000947s | 0.001005s | byte equality observed |
| mp_sort.test | 10 | 0.000925s | 0.000960s | 0.001009s | byte equality observed |
| mp_sort_q.test | 10 | 0.000933s | 0.000952s | 0.001000s | byte equality observed |
| mp_split.test | 10 | 0.000988s | 0.000974s | 0.001004s | byte equality observed |
| mp_starts_with.test | 10 | 0.000939s | 0.000976s | 0.001007s | byte equality observed |
| mp_starts_with_sf.test | 10 | 0.000964s | 0.000935s | 0.000986s | byte equality observed |
| mp_take.test | 10 | 0.000968s | 0.000994s | 0.000991s | byte equality observed |
| mp_take_2.test | 10 | 0.000439s | 0.000441s | 0.000988s | byte equality observed |
| mp_third.test | 10 | 0.000955s | 0.000977s | 0.000986s | byte equality observed |
| mp_third_2.test | 10 | 0.000448s | 0.000448s | 0.000971s | byte equality observed |
| mp_transform.test | 10 | 0.000961s | 0.000958s | 0.000987s | byte equality observed |
| mp_transform_2.test | 10 | 0.000929s | 0.000925s | 0.000970s | byte equality observed |
| mp_transform_front.test | 10 | 0.000953s | 0.000960s | 0.000994s | byte equality observed |
| mp_transform_front_2.test | 10 | 0.000434s | 0.000433s | 0.000951s | byte equality observed |
| mp_transform_if.test | 10 | 0.000926s | 0.000987s | 0.000990s | byte equality observed |
| mp_transform_if_q.test | 10 | 0.000953s | 0.000943s | 0.001011s | byte equality observed |
| mp_transform_q.test | 10 | 0.000968s | 0.000954s | 0.000978s | byte equality observed |
| mp_transform_q_2.test | 10 | 0.000950s | 0.000945s | 0.000979s | byte equality observed |
| mp_transform_second.test | 10 | 0.000960s | 0.000961s | 0.000993s | byte equality observed |
| mp_transform_second_2.test | 10 | 0.000430s | 0.000445s | 0.000980s | byte equality observed |
| mp_transform_sf.test | 10 | 0.000951s | 0.000979s | 0.000996s | byte equality observed |
| mp_transform_third.test | 10 | 0.000970s | 0.000942s | 0.000990s | byte equality observed |
| mp_transform_third_2.test | 10 | 0.000440s | 0.000452s | 0.000977s | byte equality observed |
| mp_unique.test | 10 | 0.000957s | 0.000956s | 0.001006s | byte equality observed |
| mp_unique_if.test | 10 | 0.000973s | 0.000955s | 0.001041s | byte equality observed |
| mp_unique_if_q.test | 10 | 0.000965s | 0.000953s | 0.001063s | byte equality observed |
| mp_valid.test | 10 | 0.000955s | 0.000935s | 0.001017s | byte equality observed |
| mp_valid_and_true.test | 10 | 0.000952s | 0.000968s | 0.001010s | byte equality observed |
| mp_value.test | 10 | 0.000443s | 0.000439s | 0.000978s | byte equality observed |
| mp_with_index.test | 10 | 0.001409s | 0.001344s | 0.001334s | byte equality observed |
| mp_with_index_cx.test | 10 | 0.000438s | 0.000430s | 0.001000s | byte equality observed |
| mpl.test | 10 | 0.000980s | 0.000961s | 0.001017s | byte equality observed |
| mpl_list.test | 10 | 0.000950s | 0.000946s | 0.001016s | byte equality observed |
| mpl_tuple.test | 10 | 0.000972s | 0.000977s | 0.001033s | byte equality observed |
| mpl_version.test | 10 | 0.000954s | 0.000954s | 0.001048s | byte equality observed |
| set_version.test | 10 | 0.000948s | 0.000951s | 0.000999s | byte equality observed |
| tuple_apply.test | 10 | 0.000963s | 0.000938s | 0.000991s | byte equality observed |
| tuple_for_each.test | 10 | 0.000950s | 0.000971s | 0.001042s | byte equality observed |
| tuple_transform.test | 10 | 0.000941s | 0.000974s | 0.001030s | byte equality observed |
| tuple_transform_2.test | 10 | 0.000943s | 0.000955s | 0.001018s | byte equality observed |
| tuple_version.test | 10 | 0.000956s | 0.000938s | 0.001006s | byte equality observed |
| utility_version.test | 10 | 0.000944s | 0.000951s | 0.001013s | byte equality observed |
| version.test | 10 | 0.000976s | 0.000943s | 0.001006s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
