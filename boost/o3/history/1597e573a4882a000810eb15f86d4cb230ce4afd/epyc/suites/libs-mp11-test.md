# libs/mp11/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| mp11.test | COMPILE | 0.030292 | 0.053331 | 0.070858 | — | — | — |
| complex_h.test | COMPILE | 0.124375 | 0.228258 | 0.305084 | — | — | — |
| mp_size.test | RUN | 0.142835 | 0.734272 | 0.399215 | 0.001298 | 0.001440 | 0.001332 |
| mp_size_2.test | RUN | 0.013451 | 0.027521 | 0.034965 | 0.000699 | 0.000706 | 0.001232 |
| mp_empty.test | RUN | 0.133207 | 0.453480 | 0.357991 | 0.001542 | 0.001610 | 0.001251 |
| mp_empty_2.test | RUN | 0.013202 | 0.028548 | 0.040931 | 0.000523 | 0.000745 | 0.001454 |
| mp_assign.test | RUN | 0.145483 | 0.968286 | 0.486596 | 0.001358 | 0.001345 | 0.001254 |
| mp_assign_2.test | RUN | 0.012237 | 0.031597 | 0.040750 | 0.000599 | 0.000713 | 0.001342 |
| mp_clear.test | RUN | 0.130566 | 0.409558 | 0.379366 | 0.001387 | 0.001584 | 0.001338 |
| mp_clear_2.test | RUN | 0.012732 | 0.027436 | 0.041113 | 0.000724 | 0.000439 | 0.001393 |
| mp_front.test | RUN | 0.140361 | 0.639958 | 0.434460 | 0.001520 | 0.001607 | 0.001558 |
| mp_front_2.test | RUN | 0.012204 | 0.027258 | 0.037345 | 0.000486 | 0.000685 | 0.001297 |
| mp_pop_front.test | RUN | 0.133669 | 0.688507 | 0.418809 | 0.001135 | 0.001478 | 0.001403 |
| mp_pop_front_2.test | RUN | 0.012181 | 0.027776 | 0.039113 | 0.000537 | 0.000653 | 0.001337 |
| mp_second.test | RUN | 0.133975 | 0.437608 | 0.394948 | 0.001159 | 0.001267 | 0.001200 |
| mp_second_2.test | RUN | 0.013232 | 0.028526 | 0.039603 | 0.000511 | 0.000637 | 0.001137 |
| mp_third.test | RUN | 0.126842 | 0.397036 | 0.388252 | 0.001307 | 0.001431 | 0.001484 |
| mp_third_2.test | RUN | 0.011981 | 0.028538 | 0.038801 | 0.000558 | 0.000577 | 0.001310 |
| mp_push_front.test | RUN | 0.166355 | 1.416618 | 0.679267 | 0.001349 | 0.001485 | 0.001396 |
| mp_push_front_2.test | RUN | 0.012223 | 0.027241 | 0.038149 | 0.000519 | 0.000698 | 0.001155 |
| mp_push_back.test | RUN | 0.171158 | 1.408495 | 0.711268 | 0.001620 | 0.001450 | 0.001509 |
| mp_push_back_2.test | RUN | 0.012309 | 0.028031 | 0.040963 | 0.000509 | 0.000636 | 0.001407 |
| mp_rename.test | RUN | 0.162029 | 1.300375 | 0.613023 | 0.001409 | 0.001538 | 0.001482 |
| mp_rename_2.test | RUN | 0.012997 | 0.028643 | 0.041044 | 0.000535 | 0.000638 | 0.001216 |
| mp_append.test | RUN | 0.169499 | 1.469622 | 0.648989 | 0.001419 | 0.001530 | 0.001574 |
| mp_append_2.test | RUN | 0.334590 | 3.328478 | 0.855942 | 0.001501 | 0.001490 | 0.001101 |
| mp_append_3.test | RUN | 0.013071 | 0.028034 | 0.040592 | 0.000642 | 0.000678 | 0.001410 |
| mp_append_sf.test | RUN | 0.131934 | 0.362460 | 0.358430 | 0.001578 | 0.001274 | 0.001372 |
| mp_replace_front.test | RUN | 0.150485 | 1.053257 | 0.554082 | 0.001512 | 0.001610 | 0.001552 |
| mp_replace_front_2.test | RUN | 0.012666 | 0.027689 | 0.040972 | 0.000506 | 0.000638 | 0.001263 |
| mp_replace_second.test | RUN | 0.145898 | 0.670269 | 0.491614 | 0.001859 | 0.001399 | 0.001366 |
| mp_replace_second_2.test | RUN | 0.013062 | 0.028236 | 0.039425 | 0.000641 | 0.000656 | 0.001368 |
| mp_replace_third.test | RUN | 0.140053 | 0.553403 | 0.446683 | 0.001585 | 0.001488 | 0.001470 |
| mp_replace_third_2.test | RUN | 0.011671 | 0.028243 | 0.040942 | 0.000506 | 0.000553 | 0.001243 |
| mp_apply_q.test | RUN | 0.157094 | 1.298884 | 0.597321 | 0.001652 | 0.001440 | 0.001439 |
| mp_apply_q_sf.test | RUN | 0.124716 | 0.380601 | 0.364938 | 0.001265 | 0.001560 | 0.001423 |
| mp_is_list.test | RUN | 0.126045 | 0.444674 | 0.374442 | 0.001661 | 0.001672 | 0.001254 |
| mp_list_c.test | RUN | 0.133660 | 0.550303 | 0.429753 | 0.001391 | 0.001591 | 0.001456 |
| mp_transform_front.test | RUN | 0.154611 | 1.125813 | 0.507606 | 0.001553 | 0.001419 | 0.001374 |
| mp_transform_front_2.test | RUN | 0.012610 | 0.028214 | 0.038244 | 0.000589 | 0.000651 | 0.001291 |
| mp_transform_second.test | RUN | 0.147105 | 0.974789 | 0.476064 | 0.001657 | 0.001503 | 0.001289 |
| mp_transform_second_2.test | RUN | 0.013380 | 0.028324 | 0.040248 | 0.000608 | 0.000518 | 0.001379 |
| mp_transform_third.test | RUN | 0.146550 | 0.681136 | 0.430221 | 0.001543 | 0.001617 | 0.001318 |
| mp_transform_third_2.test | RUN | 0.013862 | 0.027949 | 0.038437 | 0.000655 | 0.000591 | 0.001305 |
| mp_list_v.test | RUN | 0.014672 | 0.028233 | 0.039212 | 0.000556 | 0.000559 | 0.001332 |
| mp_rename_v.test | RUN | 0.013945 | 0.027391 | 0.038862 | 0.000547 | 0.000620 | 0.001309 |
| mp_is_value_list.test | RUN | 0.121540 | 0.276852 | 0.341485 | 0.001528 | 0.001389 | 0.001430 |
| mp_transform.test | RUN | 0.261303 | 2.187319 | 1.127725 | 0.001434 | 0.001544 | 0.001446 |
| mp_transform_2.test | RUN | 0.118272 | 0.240419 | 0.309628 | 0.001417 | 0.001535 | 0.001484 |
| mp_transform_q.test | RUN | 0.253391 | 2.093245 | 1.089709 | 0.001650 | 0.001328 | 0.001362 |
| mp_transform_q_2.test | RUN | 0.291343 | 0.673463 | 1.362929 | 0.001642 | 0.001332 | 0.001181 |
| mp_transform_sf.test | RUN | 0.131925 | 0.380452 | 0.382355 | 0.001673 | 0.001431 | 0.001471 |
| mp_transform_if.test | RUN | 0.161140 | 1.040459 | 0.597712 | 0.001439 | 0.001503 | 0.001297 |
| mp_transform_if_q.test | RUN | 0.157061 | 1.055862 | 0.584484 | 0.001171 | 0.001318 | 0.001422 |
| mp_filter.test | RUN | 0.144181 | 0.629401 | 0.443151 | 0.001464 | 0.001362 | 0.001238 |
| mp_fill.test | RUN | 0.134232 | 0.479560 | 0.399615 | 0.001601 | 0.001613 | 0.001525 |
| mp_fill_2.test | RUN | 0.021085 | 0.042507 | 0.059143 | 0.000497 | 0.000562 | 0.001434 |
| mp_count.test | RUN | 0.254066 | 1.103150 | 0.661178 | 0.001622 | 0.001421 | 0.001440 |
| mp_count_if.test | RUN | 0.270669 | 1.126788 | 0.711562 | 0.001282 | 0.001599 | 0.001374 |
| mp_count_if_q.test | RUN | 0.137129 | 0.722783 | 0.414945 | 0.001394 | 0.001323 | 0.001483 |
| mp_contains.test | RUN | 0.135844 | 0.432592 | 0.390226 | 0.001508 | 0.001417 | 0.001409 |
| mp_starts_with.test | RUN | 0.137455 | 0.460522 | 0.390661 | 0.001450 | 0.001414 | 0.001224 |
| mp_starts_with_sf.test | RUN | 0.134395 | 0.391340 | 0.383472 | 0.001304 | 0.001416 | 0.001379 |
| mp_repeat.test | RUN | 0.190995 | 1.857573 | 0.751937 | 0.001614 | 0.001596 | 0.001258 |
| mp_repeat_2.test | RUN | 0.021127 | 0.040888 | 0.065047 | 0.000523 | 0.000645 | 0.001277 |
| mp_repeat_3.test | RUN | 0.170969 | 0.253491 | 0.370719 | 0.001564 | 0.001552 | 0.001335 |
| mp_product.test | RUN | 0.155755 | 1.102878 | 0.530788 | 0.001237 | 0.001540 | 0.001180 |
| mp_drop.test | RUN | 0.170453 | 1.409425 | 0.629145 | 0.001411 | 0.001426 | 0.001277 |
| mp_drop_2.test | RUN | 0.022267 | 0.042462 | 0.065468 | 0.000523 | 0.000610 | 0.001279 |
| mp_iota.test | RUN | 0.156339 | 1.180802 | 0.549747 | 0.001462 | 0.001563 | 0.001393 |
| mp_iota_2.test | RUN | 0.166221 | 1.412438 | 0.647180 | 0.001260 | 0.001520 | 0.001115 |
| mp_at.test | RUN | 0.137151 | 0.617108 | 0.413963 | 0.001339 | 0.001261 | 0.001444 |
| mp_at_2.test | RUN | 0.021264 | 0.043054 | 0.062103 | 0.000556 | 0.000505 | 0.001408 |
| mp_at_sf.test | RUN | 0.128926 | 0.388473 | 0.383340 | 0.001395 | 0.001436 | 0.001365 |
| mp_take.test | RUN | 0.215408 | 1.844533 | 0.724725 | 0.001449 | 0.001333 | 0.001473 |
| mp_take_2.test | RUN | 0.020273 | 0.041170 | 0.063014 | 0.000662 | 0.000659 | 0.001398 |
| mp_slice.test | RUN | 0.239109 | 1.881548 | 0.961599 | 0.001386 | 0.001509 | 0.001453 |
| mp_slice_2.test | RUN | 0.021403 | 0.042509 | 0.062434 | 0.000676 | 0.000670 | 0.001452 |
| mp_replace.test | RUN | 0.163670 | 0.953826 | 0.611455 | 0.001710 | 0.001338 | 0.001444 |
| mp_replace_if.test | RUN | 0.150292 | 0.829702 | 0.548311 | 0.001506 | 0.001454 | 0.001549 |
| mp_replace_if_q.test | RUN | 0.147652 | 0.836845 | 0.549985 | 0.001396 | 0.001544 | 0.001479 |
| mp_copy_if.test | RUN | 0.159661 | 1.143876 | 0.532624 | 0.001378 | 0.001534 | 0.001438 |
| mp_copy_if_q.test | RUN | 0.159117 | 1.150611 | 0.537247 | 0.001609 | 0.001292 | 0.001477 |
| mp_remove.test | RUN | 0.150708 | 0.795460 | 0.537344 | 0.001407 | 0.001577 | 0.001528 |
| mp_remove_if.test | RUN | 0.160600 | 1.223992 | 0.578589 | 0.001455 | 0.001549 | 0.001481 |
| mp_remove_if_q.test | RUN | 0.160297 | 1.202979 | 0.572085 | 0.001358 | 0.001263 | 0.001433 |
| mp_partition.test | RUN | 0.162415 | 1.148917 | 0.608314 | 0.001643 | 0.001466 | 0.001455 |
| mp_partition_q.test | RUN | 0.162992 | 1.148147 | 0.608633 | 0.001419 | 0.001598 | 0.001472 |
| mp_sort.test | RUN | 0.151354 | 0.833111 | 0.529583 | 0.001487 | 0.001484 | 0.001444 |
| mp_sort_q.test | RUN | 0.157463 | 0.817888 | 0.542842 | 0.001284 | 0.001370 | 0.001250 |
| mp_find.test | RUN | 0.151181 | 0.878988 | 0.472110 | 0.001527 | 0.001450 | 0.001553 |
| mp_find_if.test | RUN | 0.146530 | 0.816512 | 0.434627 | 0.001534 | 0.001584 | 0.001470 |
| mp_find_if_q.test | RUN | 0.156459 | 0.927450 | 0.542767 | 0.001420 | 0.001488 | 0.001457 |
| mp_reverse.test | RUN | 0.287068 | 2.498055 | 1.068494 | 0.001526 | 0.001239 | 0.001458 |
| mp_fold.test | RUN | 0.495907 | 1.921338 | 2.760824 | 0.001408 | 0.001573 | 0.001392 |
| mp_fold_q.test | RUN | 0.149037 | 0.832647 | 0.515327 | 0.001395 | 0.001565 | 0.001469 |
| mp_fold_q_sf.test | RUN | 0.126753 | 0.388305 | 0.400033 | 0.001307 | 0.001589 | 0.001440 |
| mp_reverse_fold.test | RUN | 0.233130 | 1.860486 | 0.749107 | 0.001354 | 0.001550 | 0.001478 |
| mp_reverse_fold_q.test | RUN | 0.151309 | 1.039729 | 0.549418 | 0.001444 | 0.001478 | 0.001452 |
| mp_unique.test | RUN | 0.151034 | 0.917360 | 0.529849 | 0.001349 | 0.001406 | 0.001479 |
| mp_unique_if.test | RUN | 0.259916 | 1.895758 | 0.871077 | 0.001419 | 0.001403 | 0.001594 |
| mp_unique_if_q.test | RUN | 0.223755 | 1.863374 | 0.664537 | 0.001402 | 0.001534 | 0.001477 |
| mp_all_of.test | RUN | 0.368374 | 1.007889 | 0.877204 | 0.001578 | 0.001598 | 0.001596 |
| mp_all_of_q.test | RUN | 0.127858 | 0.433136 | 0.390556 | 0.001454 | 0.001562 | 0.001536 |
| mp_any_of.test | RUN | 0.351504 | 1.070902 | 0.887800 | 0.001508 | 0.001560 | 0.001490 |
| mp_any_of_q.test | RUN | 0.130554 | 0.443779 | 0.378003 | 0.001601 | 0.001420 | 0.001485 |
| mp_none_of.test | RUN | 0.358305 | 1.068998 | 0.888203 | 0.001590 | 0.001530 | 0.001438 |
| mp_none_of_q.test | RUN | 0.132879 | 0.458675 | 0.387445 | 0.001489 | 0.001549 | 0.001409 |
| mp_replace_at.test | RUN | 0.157683 | 0.932052 | 0.585692 | 0.001695 | 0.001505 | 0.001471 |
| mp_replace_at_c.test | RUN | 0.159413 | 0.930654 | 0.583012 | 0.001205 | 0.001500 | 0.001395 |
| mp_for_each.test | RUN | 0.155629 | 0.594255 | 0.621786 | 0.001222 | 0.001460 | 0.001440 |
| mp_insert.test | RUN | 0.226131 | 1.875557 | 0.838606 | 0.001544 | 0.001568 | 0.001500 |
| mp_insert_2.test | RUN | 0.021239 | 0.040717 | 0.063330 | 0.000566 | 0.000569 | 0.001313 |
| mp_erase.test | RUN | 0.257883 | 1.972436 | 1.030771 | 0.001698 | 0.001566 | 0.001574 |
| mp_erase_2.test | RUN | 0.021409 | 0.042170 | 0.064606 | 0.000586 | 0.000672 | 0.001426 |
| mp_with_index.test | RUN | 0.311216 | 3.981040 | 2.226288 | 0.002124 | 0.002018 | 0.001787 |
| mp_with_index_cx.test | RUN | 0.003052 | 0.014312 | 0.021734 | 0.000617 | 0.000641 | 0.001221 |
| mp_from_sequence.test | RUN | 0.154490 | 1.045211 | 0.567440 | 0.001421 | 0.001549 | 0.001378 |
| mp_from_sequence_2.test | RUN | 0.167444 | 1.218582 | 0.622553 | 0.001450 | 0.001412 | 0.001449 |
| mp_from_sequence_3.test | RUN | 0.134910 | 0.553225 | 0.420880 | 0.001470 | 0.001551 | 0.001522 |
| mp_min_element.test | RUN | 0.137105 | 0.489884 | 0.417266 | 0.001540 | 0.001505 | 0.001391 |
| mp_min_element_2.test | RUN | 0.020867 | 0.042654 | 0.062794 | 0.000513 | 0.000610 | 0.001427 |
| mp_min_element_q.test | RUN | 0.132610 | 0.500093 | 0.442161 | 0.001429 | 0.001558 | 0.001345 |
| mp_min_element_q_2.test | RUN | 0.021294 | 0.041810 | 0.102942 | 0.000536 | 0.000632 | 0.001486 |
| mp_max_element.test | RUN | 0.133789 | 0.489429 | 0.484591 | 0.001289 | 0.001393 | 0.001255 |
| mp_max_element_2.test | RUN | 0.021015 | 0.041774 | 0.058242 | 0.000547 | 0.000654 | 0.001461 |
| mp_max_element_q.test | RUN | 0.134793 | 0.477382 | 0.415339 | 0.001503 | 0.001623 | 0.001538 |
| mp_max_element_q_2.test | RUN | 0.021315 | 0.041973 | 0.064363 | 0.000585 | 0.000675 | 0.001406 |
| mp_nth_element.test | RUN | 0.144119 | 0.690132 | 0.518245 | 0.001413 | 0.001338 | 0.001493 |
| mp_nth_element_q.test | RUN | 0.146021 | 0.691121 | 0.511248 | 0.001273 | 0.001440 | 0.001305 |
| mp_back.test | RUN | 0.143266 | 0.580766 | 0.456103 | 0.001526 | 0.001461 | 0.001398 |
| mp_back_2.test | RUN | 0.020536 | 0.040863 | 0.067588 | 0.000532 | 0.000559 | 0.001400 |
| mp_pop_back.test | RUN | 0.344864 | 3.756874 | 1.040975 | 0.001310 | 0.001650 | 0.001380 |
| mp_pop_back_2.test | RUN | 0.020726 | 0.040386 | 0.059585 | 0.000540 | 0.000478 | 0.001383 |
| mp_flatten.test | RUN | 0.165574 | 1.329377 | 0.683787 | 0.001406 | 0.001510 | 0.001598 |
| mp_rotate_left.test | RUN | 0.168616 | 1.432088 | 0.662185 | 0.001416 | 0.001436 | 0.001462 |
| mp_rotate_right.test | RUN | 0.165317 | 1.424593 | 0.662378 | 0.001168 | 0.001316 | 0.001428 |
| mp_power_set.test | RUN | 0.164388 | 1.363072 | 0.663568 | 0.001450 | 0.001502 | 0.001405 |
| mp_partial_sum.test | RUN | 0.210609 | 1.822934 | 0.769062 | 0.001408 | 0.001470 | 0.001523 |
| mp_iterate.test | RUN | 0.145406 | 0.537712 | 0.531910 | 0.001484 | 0.001532 | 0.001259 |
| mp_pairwise_fold.test | RUN | 0.167402 | 1.266636 | 0.686831 | 0.001349 | 0.001452 | 0.001464 |
| mp_pairwise_fold_q.test | RUN | 0.128333 | 0.373620 | 0.377213 | 0.001630 | 0.001480 | 0.001520 |
| mp_sliding_fold.test | RUN | 0.304632 | 2.445292 | 1.302420 | 0.001316 | 0.001577 | 0.001458 |
| mp_sliding_fold_q.test | RUN | 0.304096 | 2.444098 | 1.296451 | 0.001338 | 0.001540 | 0.001333 |
| mp_intersperse.test | RUN | 0.153284 | 0.913146 | 0.594167 | 0.001437 | 0.001503 | 0.001531 |
| mp_split.test | RUN | 0.211771 | 1.740883 | 0.840903 | 0.001532 | 0.001476 | 0.001329 |
| mp_join.test | RUN | 0.176824 | 1.460619 | 0.763168 | 0.001387 | 0.001623 | 0.001538 |
| integral.test | RUN | 0.135179 | 0.652341 | 0.450599 | 0.001500 | 0.001421 | 0.001442 |
| mp_value.test | RUN | 0.009528 | 0.024308 | 0.034456 | 0.000665 | 0.000600 | 0.001489 |
| mp_identity.test | RUN | 0.131924 | 0.601677 | 0.419230 | 0.001459 | 0.001477 | 0.001479 |
| mp_inherit.test | RUN | 0.126051 | 0.426903 | 0.398318 | 0.001626 | 0.001574 | 0.001511 |
| mp_if.test | RUN | 0.132305 | 0.487470 | 0.393852 | 0.001401 | 0.001521 | 0.001376 |
| mp_if_sf.test | RUN | 0.124803 | 0.362870 | 0.355099 | 0.001550 | 0.001498 | 0.001483 |
| mp_eval_if.test | RUN | 0.132996 | 0.524327 | 0.410894 | 0.001537 | 0.001369 | 0.001442 |
| mp_eval_if_sf.test | RUN | 0.128167 | 0.367133 | 0.375651 | 0.001473 | 0.001489 | 0.001475 |
| mp_valid.test | RUN | 0.139362 | 0.379045 | 0.356926 | 0.001456 | 0.001377 | 0.001506 |
| mp_defer.test | RUN | 0.141355 | 0.625474 | 0.434588 | 0.001426 | 0.001579 | 0.001520 |
| mp_quote.test | RUN | 0.136061 | 0.636225 | 0.454775 | 0.001508 | 0.001508 | 0.001534 |
| mp_invoke_q.test | RUN | 0.143922 | 0.840388 | 0.507178 | 0.001400 | 0.001534 | 0.001497 |
| mp_invoke_q_sf.test | RUN | 0.122939 | 0.359815 | 0.366353 | 0.001329 | 0.001606 | 0.001400 |
| mp_quote_trait.test | RUN | 0.135654 | 0.694695 | 0.435569 | 0.001572 | 0.001479 | 0.001356 |
| mp_cond.test | RUN | 0.121825 | 0.425026 | 0.380172 | 0.001460 | 0.001500 | 0.001236 |
| mp_cond_sf.test | RUN | 0.119629 | 0.380123 | 0.349430 | 0.001408 | 0.001605 | 0.001293 |
| mp_not_fn.test | RUN | 0.121007 | 0.368569 | 0.364133 | 0.001350 | 0.001509 | 0.001573 |
| mp_eval_if_not.test | RUN | 0.129658 | 0.520755 | 0.400325 | 0.001526 | 0.001589 | 0.001442 |
| mp_eval_or.test | RUN | 0.120612 | 0.281452 | 0.385673 | 0.001385 | 0.001470 | 0.001466 |
| mp_compose.test | RUN | 0.135362 | 0.856858 | 0.487610 | 0.001319 | 0.001349 | 0.001367 |
| mp_valid_and_true.test | RUN | 0.123182 | 0.450704 | 0.374031 | 0.001330 | 0.001520 | 0.001500 |
| mp_compose_sf.test | COMPILE | 0.029774 | 0.050730 | 0.087820 | — | — | — |
| integer_sequence.test | RUN | 0.144923 | 0.846277 | 0.532128 | 0.001255 | 0.001464 | 0.001369 |
| tuple_for_each.test | RUN | 0.122554 | 0.322229 | 0.391552 | 0.001410 | 0.001339 | 0.001506 |
| tuple_for_each_cx.test | COMPILE | 0.023457 | 0.045954 | 0.070739 | — | — | — |
| tuple_apply.test | RUN | 0.126681 | 0.310918 | 0.390828 | 0.001559 | 0.001312 | 0.001483 |
| tuple_apply_cx.test | COMPILE | 0.033556 | 0.065980 | 0.096883 | — | — | — |
| construct_from_tuple.test | RUN | 0.120023 | 0.336008 | 0.391881 | 0.001426 | 0.001452 | 0.001476 |
| construct_from_tuple_cx.test | COMPILE | 0.033831 | 0.065979 | 0.087463 | — | — | — |
| tuple_transform.test | RUN | 0.198029 | 0.667439 | 0.769964 | 0.001402 | 0.001568 | 0.001448 |
| tuple_transform_2.test | RUN | 0.169810 | 0.485232 | 0.675981 | 0.001398 | 0.001250 | 0.001570 |
| tuple_transform_cx.test | COMPILE | 0.027932 | 0.061029 | 0.092868 | — | — | — |
| mp_set_contains.test | RUN | 0.126662 | 0.443719 | 0.395811 | 0.001411 | 0.001306 | 0.001459 |
| mp_set_push_back.test | RUN | 0.165814 | 1.382166 | 0.697995 | 0.001517 | 0.001478 | 0.001466 |
| mp_set_push_front.test | RUN | 0.208948 | 1.650822 | 0.796443 | 0.001430 | 0.001552 | 0.001147 |
| mp_is_set.test | RUN | 0.130358 | 0.445053 | 0.417518 | 0.001397 | 0.001468 | 0.001458 |
| mp_set_union.test | RUN | 0.155630 | 1.167369 | 0.649280 | 0.001265 | 0.001552 | 0.001381 |
| mp_set_union_sf.test | RUN | 0.122473 | 0.360473 | 0.385119 | 0.001203 | 0.001490 | 0.001156 |
| mp_set_difference.test | RUN | 0.146012 | 0.900704 | 0.537318 | 0.001638 | 0.001561 | 0.001364 |
| mp_set_difference_sf.test | RUN | 0.121123 | 0.368871 | 0.369246 | 0.001112 | 0.001470 | 0.001404 |
| mp_set_intersection.test | RUN | 0.147135 | 0.915248 | 0.519771 | 0.001558 | 0.001384 | 0.001477 |
| mp_set_intersection_sf.test | RUN | 0.129930 | 0.365552 | 0.362434 | 0.001454 | 0.001555 | 0.001242 |
| mp_all.test | RUN | 0.127698 | 0.436507 | 0.381338 | 0.001603 | 0.001606 | 0.001523 |
| mp_all_2.test | RUN | 0.359109 | 0.961991 | 0.897203 | 0.001570 | 0.001613 | 0.001379 |
| mp_and.test | RUN | 0.126151 | 0.450151 | 0.376636 | 0.001606 | 0.001606 | 0.001547 |
| mp_any.test | RUN | 0.134574 | 0.452020 | 0.382956 | 0.001489 | 0.001496 | 0.001533 |
| mp_any_2.test | RUN | 0.361749 | 0.947564 | 0.915484 | 0.001344 | 0.001496 | 0.001420 |
| mp_or.test | RUN | 0.134051 | 0.453791 | 0.400142 | 0.001604 | 0.001531 | 0.001550 |
| mp_same.test | RUN | 0.265481 | 0.734487 | 0.614151 | 0.001340 | 0.000976 | 0.001447 |
| mp_plus.test | RUN | 0.134637 | 0.533112 | 0.454798 | 0.001448 | 0.001473 | 0.001382 |
| mp_less.test | RUN | 0.131370 | 0.448318 | 0.398160 | 0.001628 | 0.001656 | 0.001320 |
| mp_min.test | RUN | 0.122232 | 0.404907 | 0.373796 | 0.001477 | 0.001451 | 0.001432 |
| mp_max.test | RUN | 0.124850 | 0.448465 | 0.384462 | 0.001271 | 0.001175 | 0.001282 |
| mp_similar.test | RUN | 0.127806 | 0.444199 | 0.368516 | 0.001425 | 0.001242 | 0.001479 |
| mp_map_find.test | RUN | 0.148184 | 0.815804 | 0.503971 | 0.001640 | 0.001404 | 0.001451 |
| mp_map_find_2.test | RUN | 0.135442 | 0.320036 | 0.437873 | 0.001297 | 0.001475 | 0.001436 |
| mp_map_find_3.test | RUN | 0.133498 | 0.310791 | 0.448061 | 0.001145 | 0.001448 | 0.001582 |
| mp_map_find_4.test | RUN | 0.127898 | 0.298990 | 0.397722 | 0.001322 | 0.001488 | 0.001411 |
| mp_map_contains.test | RUN | 0.127444 | 0.457021 | 0.388588 | 0.001648 | 0.001613 | 0.001440 |
| mp_map_insert.test | RUN | 0.171505 | 1.508817 | 0.735214 | 0.001542 | 0.001565 | 0.004262 |
| mp_map_replace.test | RUN | 0.217007 | 1.743976 | 0.892784 | 0.001311 | 0.001599 | 0.001503 |
| mp_map_erase.test | RUN | 0.167046 | 1.288675 | 0.749402 | 0.001217 | 0.001520 | 0.001196 |
| mp_map_update.test | RUN | 0.158789 | 0.836445 | 0.567259 | 0.001519 | 0.001485 | 0.001308 |
| mp_map_update_q.test | RUN | 0.187377 | 0.846639 | 0.536047 | 0.001314 | 0.001616 | 0.001170 |
| mp_map_keys.test | RUN | 0.184112 | 0.743895 | 0.512877 | 0.001324 | 0.001578 | 0.001348 |
| mp_is_map.test | RUN | 0.175426 | 0.466839 | 0.412231 | 0.001200 | 0.001385 | 0.001164 |
| mp_bind.test | RUN | 0.165210 | 1.353174 | 0.626482 | 0.001598 | 0.001285 | 0.001446 |
| mp_bind_q.test | RUN | 0.144549 | 0.842346 | 0.512793 | 0.001421 | 0.001479 | 0.001468 |
| mp_bind_front.test | RUN | 0.140911 | 0.907813 | 0.495054 | 0.001250 | 0.001192 | 0.001301 |
| mp_bind_back.test | RUN | 0.139628 | 0.910707 | 0.485531 | 0.001198 | 0.001596 | 0.001363 |
| mp_lambda.test | RUN | 0.302014 | 2.827951 | 1.305748 | 0.001517 | 0.001089 | 0.001173 |
| mpl.test | RUN | 0.275729 | 0.716273 | 1.024604 | 0.001546 | 0.001462 | 0.001306 |
| mpl_list.test | RUN | 0.155670 | 0.547293 | 0.460773 | 0.001223 | 0.001358 | 0.001376 |
| mpl_tuple.test | RUN | 0.154456 | 0.558053 | 0.487062 | 0.001154 | 0.001523 | 0.001330 |
| version.test | RUN | 0.081528 | 0.196879 | 0.246788 | 0.001286 | 0.001360 | 0.001261 |
| mp11_version.test | RUN | 0.088694 | 0.228028 | 0.281362 | 0.001453 | 0.001533 | 0.001344 |
| list_version.test | RUN | 0.082069 | 0.210561 | 0.247747 | 0.001162 | 0.001394 | 0.001307 |
| algorithm_version.test | RUN | 0.093031 | 0.213208 | 0.270519 | 0.001554 | 0.001461 | 0.001258 |
| integral_version.test | RUN | 0.080970 | 0.206318 | 0.243172 | 0.001468 | 0.001413 | 0.001332 |
| utility_version.test | RUN | 0.081402 | 0.201457 | 0.236903 | 0.001268 | 0.001331 | 0.001157 |
| function_version.test | RUN | 0.082771 | 0.211878 | 0.251447 | 0.001328 | 0.001493 | 0.001188 |
| map_version.test | RUN | 0.099014 | 0.214702 | 0.270938 | 0.001666 | 0.001451 | 0.001416 |
| set_version.test | RUN | 0.085270 | 0.207086 | 0.250762 | 0.001365 | 0.001417 | 0.001252 |
| bind_version.test | RUN | 0.094362 | 0.213323 | 0.263926 | 0.001186 | 0.001386 | 0.002994 |
| integer_sequence_version.test | RUN | 0.079900 | 0.205894 | 0.232901 | 0.001274 | 0.001517 | 0.001314 |
| tuple_version.test | RUN | 0.083137 | 0.215962 | 0.183765 | 0.001166 | 0.001203 | 0.001123 |
| mpl_version.test | RUN | 0.069076 | 0.189392 | 0.259542 | 0.001106 | 0.001147 | 0.001058 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.957×† | 1.009×, 0.905× | 0.955× |
| clang | 1.089× | 1.084×, 1.094× | 1.000× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026855).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| algorithm_version.test | 10 | 0.000824s | 0.000826s | 0.000852s | byte equality observed |
| bind_version.test | 10 | 0.000823s | 0.000810s | 0.000860s | byte equality observed |
| construct_from_tuple.test | 10 | 0.000838s | 0.000833s | 0.000851s | byte equality observed |
| function_version.test | 10 | 0.000827s | 0.000834s | 0.000870s | byte equality observed |
| integer_sequence.test | 10 | 0.000828s | 0.000842s | 0.000854s | byte equality observed |
| integer_sequence_version.test | 10 | 0.000831s | 0.000841s | 0.000850s | byte equality observed |
| integral.test | 10 | 0.000829s | 0.000836s | 0.000872s | byte equality observed |
| integral_version.test | 10 | 0.000829s | 0.000848s | 0.000847s | byte equality observed |
| list_version.test | 10 | 0.000829s | 0.000842s | 0.000851s | byte equality observed |
| map_version.test | 10 | 0.000839s | 0.000849s | 0.000849s | byte equality observed |
| mp11_version.test | 10 | 0.000844s | 0.000870s | 0.000853s | byte equality observed |
| mp_all.test | 10 | 0.000853s | 0.000857s | 0.000857s | byte equality observed |
| mp_all_2.test | 10 | 0.000857s | 0.000871s | 0.000853s | byte equality observed |
| mp_all_of.test | 10 | 0.000866s | 0.000873s | 0.000859s | byte equality observed |
| mp_all_of_q.test | 10 | 0.000850s | 0.000869s | 0.000862s | byte equality observed |
| mp_and.test | 10 | 0.000860s | 0.000862s | 0.000889s | byte equality observed |
| mp_any.test | 10 | 0.000867s | 0.000852s | 0.000845s | byte equality observed |
| mp_any_2.test | 10 | 0.000858s | 0.000867s | 0.000838s | byte equality observed |
| mp_any_of.test | 10 | 0.000852s | 0.000852s | 0.000848s | byte equality observed |
| mp_any_of_q.test | 10 | 0.000853s | 0.000881s | 0.000851s | byte equality observed |
| mp_append.test | 10 | 0.000854s | 0.000869s | 0.000858s | byte equality observed |
| mp_append_2.test | 10 | 0.000879s | 0.000859s | 0.000858s | byte equality observed |
| mp_append_3.test | 10 | 0.000392s | 0.000389s | 0.000837s | byte equality observed |
| mp_append_sf.test | 10 | 0.000870s | 0.000859s | 0.000860s | byte equality observed |
| mp_apply_q.test | 10 | 0.000849s | 0.000874s | 0.000857s | byte equality observed |
| mp_apply_q_sf.test | 10 | 0.000862s | 0.000860s | 0.000855s | byte equality observed |
| mp_assign.test | 10 | 0.000861s | 0.000867s | 0.000862s | byte equality observed |
| mp_assign_2.test | 10 | 0.000395s | 0.000393s | 0.000851s | byte equality observed |
| mp_at.test | 10 | 0.000885s | 0.000878s | 0.000861s | byte equality observed |
| mp_at_2.test | 10 | 0.000396s | 0.000389s | 0.000835s | byte equality observed |
| mp_at_sf.test | 10 | 0.000872s | 0.000860s | 0.000845s | byte equality observed |
| mp_back.test | 10 | 0.000869s | 0.000863s | 0.000856s | byte equality observed |
| mp_back_2.test | 10 | 0.000391s | 0.000394s | 0.000839s | byte equality observed |
| mp_bind.test | 10 | 0.000873s | 0.000870s | 0.000863s | byte equality observed |
| mp_bind_back.test | 10 | 0.000874s | 0.000878s | 0.000858s | byte equality observed |
| mp_bind_front.test | 10 | 0.000857s | 0.000867s | 0.000851s | byte equality observed |
| mp_bind_q.test | 10 | 0.000862s | 0.000861s | 0.000860s | byte equality observed |
| mp_clear.test | 10 | 0.000871s | 0.000865s | 0.000853s | byte equality observed |
| mp_clear_2.test | 10 | 0.000393s | 0.000397s | 0.000850s | byte equality observed |
| mp_compose.test | 10 | 0.000859s | 0.000846s | 0.000861s | byte equality observed |
| mp_cond.test | 10 | 0.000854s | 0.000866s | 0.000847s | byte equality observed |
| mp_cond_sf.test | 10 | 0.000859s | 0.000861s | 0.000857s | byte equality observed |
| mp_contains.test | 10 | 0.000863s | 0.000856s | 0.000853s | byte equality observed |
| mp_copy_if.test | 10 | 0.000855s | 0.000863s | 0.000853s | byte equality observed |
| mp_copy_if_q.test | 10 | 0.000863s | 0.000862s | 0.000853s | byte equality observed |
| mp_count.test | 10 | 0.000848s | 0.000876s | 0.000860s | byte equality observed |
| mp_count_if.test | 10 | 0.000868s | 0.000875s | 0.000847s | byte equality observed |
| mp_count_if_q.test | 10 | 0.000868s | 0.000855s | 0.000871s | byte equality observed |
| mp_defer.test | 10 | 0.000868s | 0.000860s | 0.000858s | byte equality observed |
| mp_drop.test | 10 | 0.000885s | 0.000863s | 0.000868s | byte equality observed |
| mp_drop_2.test | 10 | 0.000396s | 0.000399s | 0.000849s | byte equality observed |
| mp_empty.test | 10 | 0.000882s | 0.000879s | 0.000860s | byte equality observed |
| mp_empty_2.test | 10 | 0.000394s | 0.000390s | 0.000854s | byte equality observed |
| mp_erase.test | 10 | 0.000885s | 0.000872s | 0.000874s | byte equality observed |
| mp_erase_2.test | 10 | 0.000398s | 0.000395s | 0.000849s | byte equality observed |
| mp_eval_if.test | 10 | 0.000865s | 0.000860s | 0.000859s | byte equality observed |
| mp_eval_if_not.test | 10 | 0.000871s | 0.000845s | 0.000855s | byte equality observed |
| mp_eval_if_sf.test | 10 | 0.000858s | 0.000860s | 0.000873s | byte equality observed |
| mp_eval_or.test | 10 | 0.000869s | 0.000855s | 0.000857s | byte equality observed |
| mp_fill.test | 10 | 0.000860s | 0.000878s | 0.000863s | byte equality observed |
| mp_fill_2.test | 10 | 0.000391s | 0.000389s | 0.000832s | byte equality observed |
| mp_filter.test | 10 | 0.000848s | 0.000895s | 0.000865s | byte equality observed |
| mp_find.test | 10 | 0.000861s | 0.000849s | 0.000854s | byte equality observed |
| mp_find_if.test | 10 | 0.000870s | 0.000868s | 0.000864s | byte equality observed |
| mp_find_if_q.test | 10 | 0.000858s | 0.000884s | 0.000857s | byte equality observed |
| mp_flatten.test | 10 | 0.000861s | 0.000877s | 0.000889s | byte equality observed |
| mp_fold.test | 10 | 0.000896s | 0.000861s | 0.000848s | byte equality observed |
| mp_fold_q.test | 10 | 0.000865s | 0.000886s | 0.000848s | byte equality observed |
| mp_fold_q_sf.test | 10 | 0.000853s | 0.000857s | 0.000852s | byte equality observed |
| mp_for_each.test | 10 | 0.000867s | 0.000869s | 0.000848s | byte equality observed |
| mp_from_sequence.test | 10 | 0.000874s | 0.000855s | 0.000862s | byte equality observed |
| mp_from_sequence_2.test | 10 | 0.000860s | 0.000867s | 0.000857s | byte equality observed |
| mp_from_sequence_3.test | 10 | 0.000860s | 0.000865s | 0.000867s | byte equality observed |
| mp_front.test | 10 | 0.000872s | 0.000869s | 0.000858s | byte equality observed |
| mp_front_2.test | 10 | 0.000390s | 0.000395s | 0.000842s | byte equality observed |
| mp_identity.test | 10 | 0.000848s | 0.000869s | 0.000848s | byte equality observed |
| mp_if.test | 10 | 0.000865s | 0.000857s | 0.000840s | byte equality observed |
| mp_if_sf.test | 10 | 0.000857s | 0.000863s | 0.000859s | byte equality observed |
| mp_inherit.test | 10 | 0.000859s | 0.000851s | 0.000872s | byte equality observed |
| mp_insert.test | 10 | 0.000897s | 0.000896s | 0.000861s | byte equality observed |
| mp_insert_2.test | 10 | 0.000398s | 0.000391s | 0.000852s | byte equality observed |
| mp_intersperse.test | 10 | 0.000854s | 0.000851s | 0.000866s | byte equality observed |
| mp_invoke_q.test | 10 | 0.000862s | 0.000865s | 0.000878s | byte equality observed |
| mp_invoke_q_sf.test | 10 | 0.000868s | 0.000873s | 0.000854s | byte equality observed |
| mp_iota.test | 10 | 0.000856s | 0.000857s | 0.000854s | byte equality observed |
| mp_iota_2.test | 10 | 0.000863s | 0.000874s | 0.000852s | byte equality observed |
| mp_is_list.test | 10 | 0.000846s | 0.000862s | 0.000859s | byte equality observed |
| mp_is_map.test | 10 | 0.000860s | 0.000845s | 0.000845s | byte equality observed |
| mp_is_set.test | 10 | 0.000852s | 0.000866s | 0.000848s | byte equality observed |
| mp_is_value_list.test | 10 | 0.000862s | 0.000850s | 0.000853s | byte equality observed |
| mp_iterate.test | 10 | 0.000849s | 0.000856s | 0.000837s | byte equality observed |
| mp_join.test | 10 | 0.000867s | 0.000859s | 0.000855s | byte equality observed |
| mp_lambda.test | 10 | 0.000868s | 0.000881s | 0.000843s | byte equality observed |
| mp_less.test | 10 | 0.000859s | 0.000851s | 0.000854s | byte equality observed |
| mp_list_c.test | 10 | 0.000854s | 0.000876s | 0.000859s | byte equality observed |
| mp_list_v.test | 10 | 0.000395s | 0.000391s | 0.000837s | byte equality observed |
| mp_map_contains.test | 10 | 0.000858s | 0.000861s | 0.000842s | byte equality observed |
| mp_map_erase.test | 10 | 0.000870s | 0.000852s | 0.000865s | byte equality observed |
| mp_map_find.test | 10 | 0.000849s | 0.000878s | 0.000860s | byte equality observed |
| mp_map_find_2.test | 10 | 0.000856s | 0.000863s | 0.000861s | byte equality observed |
| mp_map_find_3.test | 10 | 0.000863s | 0.000854s | 0.000851s | byte equality observed |
| mp_map_find_4.test | 10 | 0.000841s | 0.000853s | 0.000858s | byte equality observed |
| mp_map_insert.test | 10 | 0.000856s | 0.000876s | 0.000861s | byte equality observed |
| mp_map_keys.test | 10 | 0.000858s | 0.000866s | 0.000867s | byte equality observed |
| mp_map_replace.test | 10 | 0.000877s | 0.000874s | 0.000854s | byte equality observed |
| mp_map_update.test | 10 | 0.000865s | 0.000854s | 0.000862s | byte equality observed |
| mp_map_update_q.test | 10 | 0.000861s | 0.000873s | 0.000855s | byte equality observed |
| mp_max.test | 10 | 0.000861s | 0.000855s | 0.000845s | byte equality observed |
| mp_max_element.test | 10 | 0.000861s | 0.000857s | 0.000864s | byte equality observed |
| mp_max_element_2.test | 10 | 0.000395s | 0.000387s | 0.000835s | byte equality observed |
| mp_max_element_q.test | 10 | 0.000866s | 0.000866s | 0.000850s | byte equality observed |
| mp_max_element_q_2.test | 10 | 0.000400s | 0.000399s | 0.000848s | byte equality observed |
| mp_min.test | 10 | 0.000858s | 0.000862s | 0.000858s | byte equality observed |
| mp_min_element.test | 10 | 0.000861s | 0.000867s | 0.000852s | byte equality observed |
| mp_min_element_2.test | 10 | 0.000398s | 0.000388s | 0.000835s | byte equality observed |
| mp_min_element_q.test | 10 | 0.000867s | 0.000872s | 0.000857s | byte equality observed |
| mp_min_element_q_2.test | 10 | 0.000397s | 0.000389s | 0.000846s | byte equality observed |
| mp_none_of.test | 10 | 0.000857s | 0.000855s | 0.000873s | byte equality observed |
| mp_none_of_q.test | 10 | 0.000859s | 0.000869s | 0.000847s | byte equality observed |
| mp_not_fn.test | 10 | 0.000857s | 0.000876s | 0.000862s | byte equality observed |
| mp_nth_element.test | 10 | 0.000862s | 0.000857s | 0.000863s | byte equality observed |
| mp_nth_element_q.test | 10 | 0.000859s | 0.000866s | 0.000863s | byte equality observed |
| mp_or.test | 10 | 0.000857s | 0.000872s | 0.000860s | byte equality observed |
| mp_pairwise_fold.test | 10 | 0.000851s | 0.000869s | 0.000856s | byte equality observed |
| mp_pairwise_fold_q.test | 10 | 0.000869s | 0.000848s | 0.000861s | byte equality observed |
| mp_partial_sum.test | 10 | 0.000870s | 0.000876s | 0.000873s | byte equality observed |
| mp_partition.test | 10 | 0.000865s | 0.000860s | 0.000857s | byte equality observed |
| mp_partition_q.test | 10 | 0.000850s | 0.000871s | 0.000865s | byte equality observed |
| mp_plus.test | 10 | 0.000863s | 0.000853s | 0.000863s | byte equality observed |
| mp_pop_back.test | 10 | 0.000875s | 0.000888s | 0.000871s | byte equality observed |
| mp_pop_back_2.test | 10 | 0.000395s | 0.000394s | 0.000861s | byte equality observed |
| mp_pop_front.test | 10 | 0.000871s | 0.000870s | 0.000856s | byte equality observed |
| mp_pop_front_2.test | 10 | 0.000389s | 0.000394s | 0.000837s | byte equality observed |
| mp_power_set.test | 10 | 0.000875s | 0.000867s | 0.000855s | byte equality observed |
| mp_product.test | 10 | 0.000863s | 0.000853s | 0.000864s | byte equality observed |
| mp_push_back.test | 10 | 0.000882s | 0.000884s | 0.000840s | byte equality observed |
| mp_push_back_2.test | 10 | 0.000395s | 0.000393s | 0.000843s | byte equality observed |
| mp_push_front.test | 10 | 0.000863s | 0.000880s | 0.000857s | byte equality observed |
| mp_push_front_2.test | 10 | 0.000392s | 0.000386s | 0.000843s | byte equality observed |
| mp_quote.test | 10 | 0.000872s | 0.000856s | 0.000867s | byte equality observed |
| mp_quote_trait.test | 10 | 0.000876s | 0.000855s | 0.000869s | byte equality observed |
| mp_remove.test | 10 | 0.000857s | 0.000845s | 0.000865s | byte equality observed |
| mp_remove_if.test | 10 | 0.000871s | 0.000892s | 0.000848s | byte equality observed |
| mp_remove_if_q.test | 10 | 0.000858s | 0.000870s | 0.000858s | byte equality observed |
| mp_rename.test | 10 | 0.000862s | 0.000859s | 0.000855s | byte equality observed |
| mp_rename_2.test | 10 | 0.000396s | 0.000399s | 0.000838s | byte equality observed |
| mp_rename_v.test | 10 | 0.000398s | 0.000398s | 0.000847s | byte equality observed |
| mp_repeat.test | 10 | 0.000878s | 0.000890s | 0.000865s | byte equality observed |
| mp_repeat_2.test | 10 | 0.000398s | 0.000391s | 0.000834s | byte equality observed |
| mp_repeat_3.test | 10 | 0.000865s | 0.000857s | 0.000853s | byte equality observed |
| mp_replace.test | 10 | 0.000884s | 0.000856s | 0.000879s | byte equality observed |
| mp_replace_at.test | 10 | 0.000871s | 0.000863s | 0.000862s | byte equality observed |
| mp_replace_at_c.test | 10 | 0.000864s | 0.000862s | 0.000842s | byte equality observed |
| mp_replace_front.test | 10 | 0.000871s | 0.000867s | 0.000864s | byte equality observed |
| mp_replace_front_2.test | 10 | 0.000398s | 0.000399s | 0.000839s | byte equality observed |
| mp_replace_if.test | 10 | 0.000861s | 0.000874s | 0.000873s | byte equality observed |
| mp_replace_if_q.test | 10 | 0.000875s | 0.000878s | 0.000858s | byte equality observed |
| mp_replace_second.test | 10 | 0.000871s | 0.000863s | 0.000873s | byte equality observed |
| mp_replace_second_2.test | 10 | 0.000398s | 0.000387s | 0.000845s | byte equality observed |
| mp_replace_third.test | 10 | 0.000870s | 0.000876s | 0.000857s | byte equality observed |
| mp_replace_third_2.test | 10 | 0.000397s | 0.000395s | 0.000850s | byte equality observed |
| mp_reverse.test | 10 | 0.000893s | 0.000884s | 0.000864s | byte equality observed |
| mp_reverse_fold.test | 10 | 0.000872s | 0.000866s | 0.000860s | byte equality observed |
| mp_reverse_fold_q.test | 10 | 0.000849s | 0.000879s | 0.000843s | byte equality observed |
| mp_rotate_left.test | 10 | 0.000867s | 0.000886s | 0.000868s | byte equality observed |
| mp_rotate_right.test | 10 | 0.000861s | 0.000861s | 0.000862s | byte equality observed |
| mp_same.test | 10 | 0.000860s | 0.000863s | 0.000837s | byte equality observed |
| mp_second.test | 10 | 0.000880s | 0.000861s | 0.000847s | byte equality observed |
| mp_second_2.test | 10 | 0.000394s | 0.000401s | 0.000835s | byte equality observed |
| mp_set_contains.test | 10 | 0.000869s | 0.000881s | 0.000850s | byte equality observed |
| mp_set_difference.test | 10 | 0.000890s | 0.000866s | 0.000859s | byte equality observed |
| mp_set_difference_sf.test | 10 | 0.000855s | 0.000874s | 0.000856s | byte equality observed |
| mp_set_intersection.test | 10 | 0.000859s | 0.000860s | 0.000849s | byte equality observed |
| mp_set_intersection_sf.test | 10 | 0.000862s | 0.000887s | 0.000865s | byte equality observed |
| mp_set_push_back.test | 10 | 0.000857s | 0.000872s | 0.000844s | byte equality observed |
| mp_set_push_front.test | 10 | 0.000866s | 0.000866s | 0.000853s | byte equality observed |
| mp_set_union.test | 10 | 0.000864s | 0.000868s | 0.000850s | byte equality observed |
| mp_set_union_sf.test | 10 | 0.000852s | 0.000849s | 0.000860s | byte equality observed |
| mp_similar.test | 10 | 0.000854s | 0.000875s | 0.000845s | byte equality observed |
| mp_size.test | 10 | 0.000872s | 0.000869s | 0.000876s | byte equality observed |
| mp_size_2.test | 10 | 0.000394s | 0.000397s | 0.000850s | byte equality observed |
| mp_slice.test | 10 | 0.000891s | 0.000890s | 0.000869s | byte equality observed |
| mp_slice_2.test | 10 | 0.000396s | 0.000395s | 0.000863s | byte equality observed |
| mp_sliding_fold.test | 10 | 0.000885s | 0.000881s | 0.000871s | byte equality observed |
| mp_sliding_fold_q.test | 10 | 0.000885s | 0.000893s | 0.000861s | byte equality observed |
| mp_sort.test | 10 | 0.000859s | 0.000864s | 0.000858s | byte equality observed |
| mp_sort_q.test | 10 | 0.000851s | 0.000869s | 0.000887s | byte equality observed |
| mp_split.test | 10 | 0.000876s | 0.000876s | 0.000888s | byte equality observed |
| mp_starts_with.test | 10 | 0.000861s | 0.000870s | 0.000864s | byte equality observed |
| mp_starts_with_sf.test | 10 | 0.000854s | 0.000871s | 0.000858s | byte equality observed |
| mp_take.test | 10 | 0.000878s | 0.000883s | 0.000887s | byte equality observed |
| mp_take_2.test | 10 | 0.000392s | 0.000392s | 0.000849s | byte equality observed |
| mp_third.test | 10 | 0.000875s | 0.000885s | 0.000858s | byte equality observed |
| mp_third_2.test | 10 | 0.000395s | 0.000393s | 0.000837s | byte equality observed |
| mp_transform.test | 10 | 0.000870s | 0.000864s | 0.000864s | byte equality observed |
| mp_transform_2.test | 10 | 0.000840s | 0.000840s | 0.000834s | byte equality observed |
| mp_transform_front.test | 10 | 0.000870s | 0.000886s | 0.000856s | byte equality observed |
| mp_transform_front_2.test | 10 | 0.000393s | 0.000393s | 0.000834s | byte equality observed |
| mp_transform_if.test | 10 | 0.000867s | 0.000876s | 0.000868s | byte equality observed |
| mp_transform_if_q.test | 10 | 0.000873s | 0.000871s | 0.000858s | byte equality observed |
| mp_transform_q.test | 10 | 0.000875s | 0.000857s | 0.000851s | byte equality observed |
| mp_transform_q_2.test | 10 | 0.000856s | 0.000851s | 0.000864s | byte equality observed |
| mp_transform_second.test | 10 | 0.000866s | 0.000866s | 0.000858s | byte equality observed |
| mp_transform_second_2.test | 10 | 0.000396s | 0.000397s | 0.000845s | byte equality observed |
| mp_transform_sf.test | 10 | 0.000855s | 0.000870s | 0.000845s | byte equality observed |
| mp_transform_third.test | 10 | 0.000868s | 0.000862s | 0.000840s | byte equality observed |
| mp_transform_third_2.test | 10 | 0.000393s | 0.000391s | 0.000851s | byte equality observed |
| mp_unique.test | 10 | 0.000860s | 0.000873s | 0.000881s | byte equality observed |
| mp_unique_if.test | 10 | 0.000880s | 0.000883s | 0.000861s | byte equality observed |
| mp_unique_if_q.test | 10 | 0.000879s | 0.000881s | 0.000871s | byte equality observed |
| mp_valid.test | 10 | 0.000859s | 0.000854s | 0.000885s | byte equality observed |
| mp_valid_and_true.test | 10 | 0.000871s | 0.000858s | 0.000868s | byte equality observed |
| mp_value.test | 10 | 0.000387s | 0.000384s | 0.000837s | byte equality observed |
| mp_with_index.test | 10 | 0.001238s | 0.001218s | 0.001163s | byte equality observed |
| mp_with_index_cx.test | 10 | 0.000398s | 0.000387s | 0.000846s | byte equality observed |
| mpl.test | 10 | 0.000866s | 0.000852s | 0.000854s | byte equality observed |
| mpl_list.test | 10 | 0.000876s | 0.000874s | 0.000864s | byte equality observed |
| mpl_tuple.test | 10 | 0.000853s | 0.000849s | 0.000872s | byte equality observed |
| mpl_version.test | 10 | 0.000869s | 0.000844s | 0.000864s | byte equality observed |
| set_version.test | 10 | 0.000857s | 0.000844s | 0.000859s | byte equality observed |
| tuple_apply.test | 10 | 0.000859s | 0.000847s | 0.000862s | byte equality observed |
| tuple_for_each.test | 10 | 0.000854s | 0.000851s | 0.000872s | byte equality observed |
| tuple_transform.test | 10 | 0.000856s | 0.000864s | 0.000872s | byte equality observed |
| tuple_transform_2.test | 10 | 0.000854s | 0.000879s | 0.000859s | byte equality observed |
| tuple_version.test | 10 | 0.000872s | 0.000872s | 0.000854s | byte equality observed |
| utility_version.test | 10 | 0.000862s | 0.000860s | 0.000854s | byte equality observed |
| version.test | 10 | 0.000855s | 0.000861s | 0.000871s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026855).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
