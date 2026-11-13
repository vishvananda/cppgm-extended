# libs/mp11/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| mp11.test | COMPILE | 0.026971 | 0.050803 | 0.072034 | — | — | — |
| complex_h.test | COMPILE | 0.108497 | 0.221117 | 0.294580 | — | — | — |
| mp_size.test | RUN | 0.124997 | 0.682379 | 0.379357 | 0.001377 | 0.001527 | 0.001496 |
| mp_size_2.test | RUN | 0.011416 | 0.026627 | 0.035865 | 0.000609 | 0.000601 | 0.001427 |
| mp_empty.test | RUN | 0.123838 | 0.434580 | 0.375208 | 0.001362 | 0.001450 | 0.001345 |
| mp_empty_2.test | RUN | 0.011894 | 0.026191 | 0.040574 | 0.000501 | 0.000615 | 0.001291 |
| mp_assign.test | RUN | 0.133847 | 0.924344 | 0.485263 | 0.001344 | 0.001531 | 0.001340 |
| mp_assign_2.test | RUN | 0.011767 | 0.028292 | 0.037730 | 0.000610 | 0.000641 | 0.001334 |
| mp_clear.test | RUN | 0.118389 | 0.393614 | 0.373260 | 0.001435 | 0.001443 | 0.001477 |
| mp_clear_2.test | RUN | 0.012293 | 0.028225 | 0.040877 | 0.000509 | 0.000599 | 0.001379 |
| mp_front.test | RUN | 0.125305 | 0.628999 | 0.433640 | 0.001397 | 0.001457 | 0.001259 |
| mp_front_2.test | RUN | 0.012118 | 0.027874 | 0.040547 | 0.000527 | 0.000630 | 0.001334 |
| mp_pop_front.test | RUN | 0.127541 | 0.659861 | 0.435227 | 0.001296 | 0.001324 | 0.001232 |
| mp_pop_front_2.test | RUN | 0.011870 | 0.027966 | 0.039250 | 0.000523 | 0.000639 | 0.001181 |
| mp_second.test | RUN | 0.125595 | 0.415564 | 0.412805 | 0.001394 | 0.001215 | 0.001456 |
| mp_second_2.test | RUN | 0.011914 | 0.027871 | 0.038257 | 0.000511 | 0.000662 | 0.001446 |
| mp_third.test | RUN | 0.119682 | 0.393283 | 0.363547 | 0.001275 | 0.001477 | 0.001290 |
| mp_third_2.test | RUN | 0.011989 | 0.027726 | 0.041408 | 0.000531 | 0.000549 | 0.001185 |
| mp_push_front.test | RUN | 0.156295 | 1.407952 | 0.708038 | 0.001393 | 0.001578 | 0.001380 |
| mp_push_front_2.test | RUN | 0.012186 | 0.026280 | 0.037875 | 0.000537 | 0.000559 | 0.001223 |
| mp_push_back.test | RUN | 0.157366 | 1.386970 | 0.709393 | 0.001371 | 0.001444 | 0.001520 |
| mp_push_back_2.test | RUN | 0.012235 | 0.027698 | 0.038669 | 0.000488 | 0.000679 | 0.001403 |
| mp_rename.test | RUN | 0.151148 | 1.288221 | 0.564305 | 0.001239 | 0.001562 | 0.001331 |
| mp_rename_2.test | RUN | 0.011470 | 0.028196 | 0.040531 | 0.000578 | 0.000760 | 0.001355 |
| mp_append.test | RUN | 0.156429 | 1.454926 | 0.645548 | 0.001539 | 0.001545 | 0.001236 |
| mp_append_2.test | RUN | 0.311244 | 3.265526 | 0.870723 | 0.001464 | 0.001546 | 0.001315 |
| mp_append_3.test | RUN | 0.011636 | 0.026788 | 0.040122 | 0.000574 | 0.000466 | 0.001441 |
| mp_append_sf.test | RUN | 0.119666 | 0.354086 | 0.352622 | 0.001442 | 0.001446 | 0.001405 |
| mp_replace_front.test | RUN | 0.148425 | 1.046709 | 0.566869 | 0.001396 | 0.001430 | 0.001196 |
| mp_replace_front_2.test | RUN | 0.012044 | 0.027733 | 0.038646 | 0.000664 | 0.000633 | 0.001449 |
| mp_replace_second.test | RUN | 0.139504 | 0.667630 | 0.485319 | 0.001530 | 0.001364 | 0.001359 |
| mp_replace_second_2.test | RUN | 0.012299 | 0.027893 | 0.038959 | 0.000529 | 0.000653 | 0.001285 |
| mp_replace_third.test | RUN | 0.129634 | 0.555583 | 0.435196 | 0.001294 | 0.001567 | 0.001448 |
| mp_replace_third_2.test | RUN | 0.011700 | 0.026876 | 0.039756 | 0.000646 | 0.000550 | 0.001331 |
| mp_apply_q.test | RUN | 0.151317 | 1.274178 | 0.578504 | 0.001419 | 0.001439 | 0.001243 |
| mp_apply_q_sf.test | RUN | 0.123222 | 0.375740 | 0.351069 | 0.001453 | 0.001495 | 0.001395 |
| mp_is_list.test | RUN | 0.124509 | 0.437784 | 0.378920 | 0.001373 | 0.001530 | 0.001433 |
| mp_list_c.test | RUN | 0.127569 | 0.536363 | 0.410457 | 0.001236 | 0.001372 | 0.001404 |
| mp_transform_front.test | RUN | 0.146370 | 1.032167 | 0.510880 | 0.001406 | 0.001342 | 0.001354 |
| mp_transform_front_2.test | RUN | 0.012022 | 0.026554 | 0.040023 | 0.000485 | 0.000533 | 0.001421 |
| mp_transform_second.test | RUN | 0.135074 | 0.902411 | 0.452963 | 0.001368 | 0.001528 | 0.001290 |
| mp_transform_second_2.test | RUN | 0.012082 | 0.027403 | 0.037504 | 0.000445 | 0.000695 | 0.001298 |
| mp_transform_third.test | RUN | 0.127191 | 0.695327 | 0.421508 | 0.001241 | 0.001306 | 0.001391 |
| mp_transform_third_2.test | RUN | 0.011904 | 0.026893 | 0.040120 | 0.000517 | 0.000545 | 0.001200 |
| mp_list_v.test | RUN | 0.012472 | 0.026356 | 0.038141 | 0.000465 | 0.000508 | 0.001463 |
| mp_rename_v.test | RUN | 0.012305 | 0.026061 | 0.041977 | 0.000577 | 0.000546 | 0.001179 |
| mp_is_value_list.test | RUN | 0.119242 | 0.270786 | 0.336357 | 0.001429 | 0.001595 | 0.001418 |
| mp_transform.test | RUN | 0.267362 | 2.187476 | 1.081920 | 0.001630 | 0.001474 | 0.001468 |
| mp_transform_2.test | RUN | 0.108140 | 0.240104 | 0.323479 | 0.001206 | 0.001551 | 0.001433 |
| mp_transform_q.test | RUN | 0.259896 | 2.075334 | 1.098152 | 0.001601 | 0.001617 | 0.001461 |
| mp_transform_q_2.test | RUN | 0.287784 | 0.667257 | 1.393150 | 0.001528 | 0.001433 | 0.001181 |
| mp_transform_sf.test | RUN | 0.125935 | 0.374631 | 0.391417 | 0.001392 | 0.001384 | 0.001544 |
| mp_transform_if.test | RUN | 0.148941 | 1.026176 | 0.563106 | 0.001294 | 0.001432 | 0.001326 |
| mp_transform_if_q.test | RUN | 0.152471 | 1.044902 | 0.596077 | 0.001133 | 0.001355 | 0.001364 |
| mp_filter.test | RUN | 0.138518 | 0.615213 | 0.433347 | 0.001387 | 0.001547 | 0.001511 |
| mp_fill.test | RUN | 0.129106 | 0.464647 | 0.420475 | 0.001374 | 0.001541 | 0.001427 |
| mp_fill_2.test | RUN | 0.020068 | 0.040041 | 0.059214 | 0.000533 | 0.000659 | 0.001377 |
| mp_count.test | RUN | 0.254058 | 1.118545 | 0.647215 | 0.001276 | 0.001443 | 0.001475 |
| mp_count_if.test | RUN | 0.265827 | 1.129937 | 0.730255 | 0.001263 | 0.001481 | 0.001343 |
| mp_count_if_q.test | RUN | 0.137568 | 0.737756 | 0.399130 | 0.001437 | 0.001353 | 0.001397 |
| mp_contains.test | RUN | 0.132294 | 0.445987 | 0.388752 | 0.001175 | 0.001370 | 0.001482 |
| mp_starts_with.test | RUN | 0.127602 | 0.450361 | 0.399573 | 0.001272 | 0.001342 | 0.001148 |
| mp_starts_with_sf.test | RUN | 0.122683 | 0.370851 | 0.377424 | 0.001370 | 0.001522 | 0.001177 |
| mp_repeat.test | RUN | 0.176949 | 1.850952 | 0.727041 | 0.001368 | 0.001366 | 0.001314 |
| mp_repeat_2.test | RUN | 0.020237 | 0.041461 | 0.063381 | 0.000521 | 0.000647 | 0.001205 |
| mp_repeat_3.test | RUN | 0.153565 | 0.241928 | 0.359049 | 0.001469 | 0.001576 | 0.001359 |
| mp_product.test | RUN | 0.146486 | 1.091870 | 0.550772 | 0.001415 | 0.001462 | 0.001394 |
| mp_drop.test | RUN | 0.156747 | 1.392550 | 0.592097 | 0.000992 | 0.001560 | 0.001248 |
| mp_drop_2.test | RUN | 0.020938 | 0.042214 | 0.061863 | 0.000559 | 0.000602 | 0.001327 |
| mp_iota.test | RUN | 0.150001 | 1.168109 | 0.544332 | 0.001377 | 0.001514 | 0.001447 |
| mp_iota_2.test | RUN | 0.158637 | 1.402791 | 0.620380 | 0.001189 | 0.001550 | 0.001285 |
| mp_at.test | RUN | 0.133261 | 0.616696 | 0.421951 | 0.001157 | 0.001416 | 0.001426 |
| mp_at_2.test | RUN | 0.020722 | 0.042829 | 0.061747 | 0.000501 | 0.000618 | 0.001368 |
| mp_at_sf.test | RUN | 0.125132 | 0.380098 | 0.385462 | 0.001375 | 0.001593 | 0.001331 |
| mp_take.test | RUN | 0.204233 | 1.850217 | 0.730545 | 0.001362 | 0.001364 | 0.001432 |
| mp_take_2.test | RUN | 0.020433 | 0.042030 | 0.054633 | 0.000656 | 0.000623 | 0.001283 |
| mp_slice.test | RUN | 0.231982 | 1.885336 | 0.972135 | 0.001439 | 0.001535 | 0.001457 |
| mp_slice_2.test | RUN | 0.019897 | 0.041847 | 0.060472 | 0.000536 | 0.000584 | 0.001418 |
| mp_replace.test | RUN | 0.156009 | 0.938197 | 0.613181 | 0.001466 | 0.001443 | 0.001363 |
| mp_replace_if.test | RUN | 0.146836 | 0.826388 | 0.539218 | 0.001388 | 0.001271 | 0.001275 |
| mp_replace_if_q.test | RUN | 0.144525 | 0.834425 | 0.552039 | 0.001389 | 0.001552 | 0.001393 |
| mp_copy_if.test | RUN | 0.151180 | 1.136428 | 0.510593 | 0.001458 | 0.001396 | 0.001441 |
| mp_copy_if_q.test | RUN | 0.152013 | 1.139025 | 0.547881 | 0.001405 | 0.001290 | 0.001407 |
| mp_remove.test | RUN | 0.145037 | 0.790694 | 0.548583 | 0.001481 | 0.001556 | 0.001202 |
| mp_remove_if.test | RUN | 0.152294 | 1.216141 | 0.530912 | 0.001321 | 0.001544 | 0.001441 |
| mp_remove_if_q.test | RUN | 0.155059 | 1.191409 | 0.566430 | 0.001277 | 0.001421 | 0.001531 |
| mp_partition.test | RUN | 0.154673 | 1.132673 | 0.604824 | 0.001314 | 0.001550 | 0.001298 |
| mp_partition_q.test | RUN | 0.154469 | 1.147683 | 0.636272 | 0.001480 | 0.001582 | 0.001402 |
| mp_sort.test | RUN | 0.141822 | 0.830678 | 0.503968 | 0.001351 | 0.001512 | 0.001349 |
| mp_sort_q.test | RUN | 0.149939 | 0.822845 | 0.534649 | 0.001413 | 0.001420 | 0.001433 |
| mp_find.test | RUN | 0.147377 | 0.886988 | 0.477312 | 0.001470 | 0.001471 | 0.001449 |
| mp_find_if.test | RUN | 0.136606 | 0.817144 | 0.438153 | 0.001482 | 0.001570 | 0.001232 |
| mp_find_if_q.test | RUN | 0.149510 | 0.918794 | 0.510250 | 0.001353 | 0.001524 | 0.001426 |
| mp_reverse.test | RUN | 0.285477 | 2.417059 | 1.066125 | 0.001449 | 0.001497 | 0.001538 |
| mp_fold.test | RUN | 0.480926 | 1.914804 | 2.738397 | 0.001445 | 0.001534 | 0.001451 |
| mp_fold_q.test | RUN | 0.142708 | 0.824437 | 0.502987 | 0.001363 | 0.001481 | 0.001409 |
| mp_fold_q_sf.test | RUN | 0.123981 | 0.398902 | 0.386458 | 0.001152 | 0.001568 | 0.001145 |
| mp_reverse_fold.test | RUN | 0.230495 | 1.821102 | 0.741127 | 0.001282 | 0.001322 | 0.001286 |
| mp_reverse_fold_q.test | RUN | 0.146594 | 1.000801 | 0.524091 | 0.001432 | 0.001419 | 0.001448 |
| mp_unique.test | RUN | 0.140553 | 0.909259 | 0.529760 | 0.001272 | 0.001521 | 0.001463 |
| mp_unique_if.test | RUN | 0.249015 | 1.868482 | 0.870414 | 0.001386 | 0.001590 | 0.001252 |
| mp_unique_if_q.test | RUN | 0.217362 | 1.846377 | 0.658044 | 0.001276 | 0.001400 | 0.001430 |
| mp_all_of.test | RUN | 0.364818 | 1.012611 | 0.873695 | 0.001572 | 0.001528 | 0.001464 |
| mp_all_of_q.test | RUN | 0.127529 | 0.434971 | 0.382800 | 0.001583 | 0.001581 | 0.001485 |
| mp_any_of.test | RUN | 0.366892 | 1.073229 | 0.872114 | 0.001309 | 0.001383 | 0.001427 |
| mp_any_of_q.test | RUN | 0.125568 | 0.454299 | 0.398340 | 0.001554 | 0.001565 | 0.001534 |
| mp_none_of.test | RUN | 0.342471 | 1.048532 | 0.854902 | 0.001315 | 0.001468 | 0.001392 |
| mp_none_of_q.test | RUN | 0.135032 | 0.457182 | 0.397884 | 0.001468 | 0.001516 | 0.001429 |
| mp_replace_at.test | RUN | 0.151733 | 0.909984 | 0.589253 | 0.001398 | 0.001629 | 0.001519 |
| mp_replace_at_c.test | RUN | 0.150134 | 0.914650 | 0.579742 | 0.001301 | 0.001404 | 0.001348 |
| mp_for_each.test | RUN | 0.144554 | 0.597737 | 0.605207 | 0.001358 | 0.001541 | 0.001336 |
| mp_insert.test | RUN | 0.218035 | 1.866332 | 0.852260 | 0.001426 | 0.001494 | 0.001260 |
| mp_insert_2.test | RUN | 0.020806 | 0.040248 | 0.062992 | 0.000590 | 0.000527 | 0.001258 |
| mp_erase.test | RUN | 0.238218 | 1.958417 | 1.014959 | 0.001404 | 0.001514 | 0.001433 |
| mp_erase_2.test | RUN | 0.020724 | 0.040629 | 0.063809 | 0.000478 | 0.000684 | 0.001400 |
| mp_with_index.test | RUN | 0.299897 | 3.953078 | 2.122653 | 0.001852 | 0.002016 | 0.001702 |
| mp_with_index_cx.test | RUN | 0.002877 | 0.014286 | 0.021712 | 0.000550 | 0.000660 | 0.001290 |
| mp_from_sequence.test | RUN | 0.147463 | 1.033974 | 0.572925 | 0.001326 | 0.001522 | 0.001355 |
| mp_from_sequence_2.test | RUN | 0.157243 | 1.208297 | 0.624199 | 0.001355 | 0.001313 | 0.001479 |
| mp_from_sequence_3.test | RUN | 0.137139 | 0.548773 | 0.449989 | 0.001361 | 0.001600 | 0.001368 |
| mp_min_element.test | RUN | 0.136936 | 0.487565 | 0.394811 | 0.001296 | 0.001533 | 0.001430 |
| mp_min_element_2.test | RUN | 0.020807 | 0.041648 | 0.061510 | 0.000510 | 0.000637 | 0.001298 |
| mp_min_element_q.test | RUN | 0.130202 | 0.503989 | 0.389575 | 0.001228 | 0.001570 | 0.001488 |
| mp_min_element_q_2.test | RUN | 0.020901 | 0.038612 | 0.062745 | 0.000616 | 0.000540 | 0.001431 |
| mp_max_element.test | RUN | 0.131952 | 0.487281 | 0.411585 | 0.001456 | 0.001520 | 0.001486 |
| mp_max_element_2.test | RUN | 0.021203 | 0.039273 | 0.058340 | 0.000624 | 0.000671 | 0.001228 |
| mp_max_element_q.test | RUN | 0.130277 | 0.492742 | 0.400274 | 0.001336 | 0.001574 | 0.001405 |
| mp_max_element_q_2.test | RUN | 0.021256 | 0.042147 | 0.058190 | 0.000542 | 0.000603 | 0.001359 |
| mp_nth_element.test | RUN | 0.145452 | 0.689845 | 0.492525 | 0.001481 | 0.001400 | 0.001178 |
| mp_nth_element_q.test | RUN | 0.141144 | 0.683323 | 0.478688 | 0.001408 | 0.001450 | 0.001324 |
| mp_back.test | RUN | 0.140230 | 0.569546 | 0.468055 | 0.001370 | 0.001448 | 0.001284 |
| mp_back_2.test | RUN | 0.020149 | 0.041295 | 0.063636 | 0.000468 | 0.000547 | 0.001237 |
| mp_pop_back.test | RUN | 0.346710 | 3.717491 | 1.018421 | 0.001536 | 0.001520 | 0.001270 |
| mp_pop_back_2.test | RUN | 0.020202 | 0.039706 | 0.061921 | 0.000581 | 0.000576 | 0.001359 |
| mp_flatten.test | RUN | 0.164871 | 1.306926 | 0.678579 | 0.001500 | 0.001582 | 0.001150 |
| mp_rotate_left.test | RUN | 0.160823 | 1.413344 | 0.662631 | 0.001294 | 0.001639 | 0.001214 |
| mp_rotate_right.test | RUN | 0.166203 | 1.430007 | 0.674818 | 0.001417 | 0.001379 | 0.001216 |
| mp_power_set.test | RUN | 0.158741 | 1.339341 | 0.681018 | 0.001636 | 0.001508 | 0.001236 |
| mp_partial_sum.test | RUN | 0.203434 | 1.824925 | 0.766221 | 0.001330 | 0.001442 | 0.001351 |
| mp_iterate.test | RUN | 0.146578 | 0.550736 | 0.522021 | 0.001394 | 0.001503 | 0.001362 |
| mp_pairwise_fold.test | RUN | 0.158883 | 1.259639 | 0.669675 | 0.001295 | 0.001540 | 0.001403 |
| mp_pairwise_fold_q.test | RUN | 0.126696 | 0.372311 | 0.372924 | 0.001566 | 0.001536 | 0.001428 |
| mp_sliding_fold.test | RUN | 0.282937 | 2.416213 | 1.283664 | 0.001525 | 0.001630 | 0.001459 |
| mp_sliding_fold_q.test | RUN | 0.281657 | 2.408886 | 1.279301 | 0.001310 | 0.001448 | 0.001175 |
| mp_intersperse.test | RUN | 0.153286 | 0.908860 | 0.571090 | 0.001444 | 0.001525 | 0.001409 |
| mp_split.test | RUN | 0.210018 | 1.730737 | 0.806725 | 0.001539 | 0.001582 | 0.001093 |
| mp_join.test | RUN | 0.170524 | 1.454237 | 0.709066 | 0.001383 | 0.001578 | 0.001500 |
| integral.test | RUN | 0.128087 | 0.650226 | 0.452395 | 0.001286 | 0.001574 | 0.001369 |
| mp_value.test | RUN | 0.009507 | 0.023772 | 0.034244 | 0.000532 | 0.000572 | 0.001384 |
| mp_identity.test | RUN | 0.124792 | 0.598176 | 0.425415 | 0.001547 | 0.001435 | 0.001319 |
| mp_inherit.test | RUN | 0.121147 | 0.424157 | 0.361156 | 0.001451 | 0.001572 | 0.001484 |
| mp_if.test | RUN | 0.125198 | 0.486593 | 0.400078 | 0.001446 | 0.001314 | 0.001497 |
| mp_if_sf.test | RUN | 0.118295 | 0.359646 | 0.365279 | 0.001159 | 0.001301 | 0.001506 |
| mp_eval_if.test | RUN | 0.125184 | 0.505775 | 0.388564 | 0.001584 | 0.001412 | 0.001201 |
| mp_eval_if_sf.test | RUN | 0.117041 | 0.370913 | 0.334092 | 0.001371 | 0.001452 | 0.001441 |
| mp_valid.test | RUN | 0.118686 | 0.371065 | 0.351407 | 0.001406 | 0.001513 | 0.001378 |
| mp_defer.test | RUN | 0.129719 | 0.622576 | 0.441909 | 0.001280 | 0.001580 | 0.001526 |
| mp_quote.test | RUN | 0.133023 | 0.635485 | 0.461027 | 0.001554 | 0.001499 | 0.001370 |
| mp_invoke_q.test | RUN | 0.137732 | 0.816886 | 0.485098 | 0.001222 | 0.001597 | 0.001322 |
| mp_invoke_q_sf.test | RUN | 0.117388 | 0.362403 | 0.349193 | 0.001374 | 0.001572 | 0.001412 |
| mp_quote_trait.test | RUN | 0.125013 | 0.702275 | 0.435833 | 0.001147 | 0.001287 | 0.001160 |
| mp_cond.test | RUN | 0.121935 | 0.426507 | 0.388285 | 0.001532 | 0.001486 | 0.001228 |
| mp_cond_sf.test | RUN | 0.116495 | 0.360585 | 0.350182 | 0.001376 | 0.001379 | 0.001259 |
| mp_not_fn.test | RUN | 0.117284 | 0.371042 | 0.353895 | 0.001362 | 0.001606 | 0.001453 |
| mp_eval_if_not.test | RUN | 0.122644 | 0.519715 | 0.403900 | 0.001334 | 0.001530 | 0.001323 |
| mp_eval_or.test | RUN | 0.120441 | 0.277512 | 0.361175 | 0.001282 | 0.001504 | 0.001366 |
| mp_compose.test | RUN | 0.132457 | 0.851776 | 0.480234 | 0.001444 | 0.001314 | 0.001210 |
| mp_valid_and_true.test | RUN | 0.122822 | 0.440231 | 0.371312 | 0.001370 | 0.001592 | 0.001177 |
| mp_compose_sf.test | COMPILE | 0.028607 | 0.053292 | 0.083799 | — | — | — |
| integer_sequence.test | RUN | 0.143503 | 0.849604 | 0.528408 | 0.001393 | 0.001591 | 0.001172 |
| tuple_for_each.test | RUN | 0.123170 | 0.313206 | 0.388125 | 0.001350 | 0.001550 | 0.001310 |
| tuple_for_each_cx.test | COMPILE | 0.023786 | 0.047789 | 0.068289 | — | — | — |
| tuple_apply.test | RUN | 0.120462 | 0.308120 | 0.398509 | 0.001435 | 0.001519 | 0.001278 |
| tuple_apply_cx.test | COMPILE | 0.033128 | 0.061663 | 0.093262 | — | — | — |
| construct_from_tuple.test | RUN | 0.121631 | 0.332254 | 0.376247 | 0.001308 | 0.001483 | 0.001450 |
| construct_from_tuple_cx.test | COMPILE | 0.034320 | 0.064963 | 0.089339 | — | — | — |
| tuple_transform.test | RUN | 0.191162 | 0.662106 | 0.750665 | 0.001490 | 0.001645 | 0.001337 |
| tuple_transform_2.test | RUN | 0.168179 | 0.483820 | 0.657060 | 0.001282 | 0.001342 | 0.001241 |
| tuple_transform_cx.test | COMPILE | 0.029055 | 0.061765 | 0.092942 | — | — | — |
| mp_set_contains.test | RUN | 0.124279 | 0.438857 | 0.390437 | 0.001488 | 0.001568 | 0.001396 |
| mp_set_push_back.test | RUN | 0.159726 | 1.364766 | 0.661650 | 0.001544 | 0.001542 | 0.001417 |
| mp_set_push_front.test | RUN | 0.202516 | 1.628442 | 0.755793 | 0.001380 | 0.001503 | 0.001379 |
| mp_is_set.test | RUN | 0.124606 | 0.440484 | 0.399968 | 0.001504 | 0.001492 | 0.001407 |
| mp_set_union.test | RUN | 0.154861 | 1.161851 | 0.640244 | 0.001431 | 0.001463 | 0.001259 |
| mp_set_union_sf.test | RUN | 0.119894 | 0.357363 | 0.366201 | 0.001383 | 0.001494 | 0.001458 |
| mp_set_difference.test | RUN | 0.141389 | 0.915451 | 0.536533 | 0.001443 | 0.001563 | 0.001118 |
| mp_set_difference_sf.test | RUN | 0.118874 | 0.358582 | 0.360680 | 0.001274 | 0.001549 | 0.001280 |
| mp_set_intersection.test | RUN | 0.140674 | 0.911021 | 0.518942 | 0.001492 | 0.001538 | 0.001400 |
| mp_set_intersection_sf.test | RUN | 0.121348 | 0.355378 | 0.353060 | 0.001260 | 0.001386 | 0.001287 |
| mp_all.test | RUN | 0.122189 | 0.441532 | 0.387428 | 0.001594 | 0.001588 | 0.001470 |
| mp_all_2.test | RUN | 0.347342 | 0.936663 | 0.890175 | 0.001558 | 0.001634 | 0.001547 |
| mp_and.test | RUN | 0.120521 | 0.452430 | 0.390610 | 0.001327 | 0.001596 | 0.001344 |
| mp_any.test | RUN | 0.124636 | 0.457651 | 0.401011 | 0.001332 | 0.001684 | 0.001508 |
| mp_any_2.test | RUN | 0.366133 | 0.928593 | 0.880118 | 0.001366 | 0.001593 | 0.001193 |
| mp_or.test | RUN | 0.125844 | 0.445716 | 0.374984 | 0.001402 | 0.001650 | 0.001572 |
| mp_same.test | RUN | 0.243712 | 0.738394 | 0.585778 | 0.001084 | 0.001580 | 0.001347 |
| mp_plus.test | RUN | 0.133839 | 0.528460 | 0.442802 | 0.001337 | 0.001310 | 0.001356 |
| mp_less.test | RUN | 0.125597 | 0.452414 | 0.400308 | 0.001430 | 0.001534 | 0.001438 |
| mp_min.test | RUN | 0.120867 | 0.396467 | 0.376394 | 0.001311 | 0.001494 | 0.001098 |
| mp_max.test | RUN | 0.125585 | 0.451133 | 0.367263 | 0.001018 | 0.001548 | 0.001394 |
| mp_similar.test | RUN | 0.121838 | 0.442735 | 0.380002 | 0.001296 | 0.001308 | 0.001484 |
| mp_map_find.test | RUN | 0.142633 | 0.799242 | 0.500965 | 0.001391 | 0.001505 | 0.001412 |
| mp_map_find_2.test | RUN | 0.132122 | 0.322288 | 0.421045 | 0.001365 | 0.001489 | 0.001271 |
| mp_map_find_3.test | RUN | 0.133772 | 0.316085 | 0.430464 | 0.001216 | 0.001312 | 0.001384 |
| mp_map_find_4.test | RUN | 0.126677 | 0.289957 | 0.404924 | 0.001247 | 0.001244 | 0.001472 |
| mp_map_contains.test | RUN | 0.129416 | 0.456843 | 0.414298 | 0.001500 | 0.001618 | 0.001464 |
| mp_map_insert.test | RUN | 0.167057 | 1.521607 | 0.727708 | 0.001339 | 0.001597 | 0.001317 |
| mp_map_replace.test | RUN | 0.211029 | 1.725045 | 0.836853 | 0.001352 | 0.001567 | 0.001395 |
| mp_map_erase.test | RUN | 0.164042 | 1.292864 | 0.703141 | 0.001300 | 0.001542 | 0.001183 |
| mp_map_update.test | RUN | 0.146810 | 0.828392 | 0.548218 | 0.001515 | 0.001530 | 0.001296 |
| mp_map_update_q.test | RUN | 0.142309 | 0.827037 | 0.525731 | 0.001261 | 0.001379 | 0.001303 |
| mp_map_keys.test | RUN | 0.141632 | 0.747003 | 0.512687 | 0.001346 | 0.001484 | 0.001471 |
| mp_is_map.test | RUN | 0.132083 | 0.452318 | 0.406432 | 0.001409 | 0.001307 | 0.001129 |
| mp_bind.test | RUN | 0.162837 | 1.346915 | 0.628650 | 0.001382 | 0.001323 | 0.001481 |
| mp_bind_q.test | RUN | 0.140184 | 0.830717 | 0.486227 | 0.001379 | 0.001552 | 0.001158 |
| mp_bind_front.test | RUN | 0.137605 | 0.905947 | 0.484397 | 0.001266 | 0.001504 | 0.001306 |
| mp_bind_back.test | RUN | 0.139434 | 0.892921 | 0.490790 | 0.001293 | 0.001330 | 0.001428 |
| mp_lambda.test | RUN | 0.295015 | 2.824245 | 1.307339 | 0.001361 | 0.001117 | 0.001220 |
| mpl.test | RUN | 0.274613 | 0.734982 | 1.008419 | 0.001279 | 0.001584 | 0.001296 |
| mpl_list.test | RUN | 0.159961 | 0.537707 | 0.464138 | 0.001330 | 0.001503 | 0.001487 |
| mpl_tuple.test | RUN | 0.161405 | 0.541290 | 0.483976 | 0.001236 | 0.001313 | 0.001463 |
| version.test | RUN | 0.079890 | 0.195483 | 0.250740 | 0.001413 | 0.001482 | 0.001241 |
| mp11_version.test | RUN | 0.097710 | 0.235385 | 0.282557 | 0.001448 | 0.001402 | 0.001341 |
| list_version.test | RUN | 0.082776 | 0.200702 | 0.248542 | 0.001276 | 0.001442 | 0.001402 |
| algorithm_version.test | RUN | 0.092660 | 0.251304 | 0.268556 | 0.001441 | 0.001500 | 0.001450 |
| integral_version.test | RUN | 0.080803 | 0.222987 | 0.241374 | 0.001594 | 0.001451 | 0.001395 |
| utility_version.test | RUN | 0.081044 | 0.239524 | 0.249223 | 0.001277 | 0.001542 | 0.001378 |
| function_version.test | RUN | 0.083286 | 0.205265 | 0.254129 | 0.001270 | 0.001495 | 0.001329 |
| map_version.test | RUN | 0.094620 | 0.223372 | 0.267378 | 0.001277 | 0.001579 | 0.001371 |
| set_version.test | RUN | 0.084668 | 0.207689 | 0.256114 | 0.001398 | 0.001562 | 0.001366 |
| bind_version.test | RUN | 0.091664 | 0.212821 | 0.272101 | 0.001134 | 0.001472 | 0.001368 |
| integer_sequence_version.test | RUN | 0.079505 | 0.203894 | 0.236565 | 0.001195 | 0.001448 | 0.001366 |
| tuple_version.test | RUN | 0.088297 | 0.214640 | 0.198752 | 0.001173 | 0.001557 | 0.001074 |
| mpl_version.test | RUN | 0.073763 | 0.170373 | 0.243009 | 0.001102 | 0.001134 | 0.001009 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.000× | 1.002×, 0.997× | 1.001× |
| clang | 1.087× | 1.089×, 1.084× | 1.002× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409407).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| algorithm_version.test | 10 | 0.000813s | 0.000815s | 0.000844s | byte equality observed |
| bind_version.test | 10 | 0.000815s | 0.000814s | 0.000859s | byte equality observed |
| construct_from_tuple.test | 10 | 0.000835s | 0.000835s | 0.000844s | byte equality observed |
| function_version.test | 10 | 0.000820s | 0.000805s | 0.000851s | byte equality observed |
| integer_sequence.test | 10 | 0.000825s | 0.000831s | 0.000850s | byte equality observed |
| integer_sequence_version.test | 10 | 0.000820s | 0.000806s | 0.000839s | byte equality observed |
| integral.test | 10 | 0.000839s | 0.000826s | 0.000838s | byte equality observed |
| integral_version.test | 10 | 0.000818s | 0.000814s | 0.000847s | byte equality observed |
| list_version.test | 10 | 0.000822s | 0.000832s | 0.000833s | byte equality observed |
| map_version.test | 10 | 0.000829s | 0.000862s | 0.000844s | byte equality observed |
| mp11_version.test | 10 | 0.000829s | 0.000836s | 0.000852s | byte equality observed |
| mp_all.test | 10 | 0.000847s | 0.000851s | 0.000847s | byte equality observed |
| mp_all_2.test | 10 | 0.000826s | 0.000862s | 0.000841s | byte equality observed |
| mp_all_of.test | 10 | 0.000845s | 0.000847s | 0.000862s | byte equality observed |
| mp_all_of_q.test | 10 | 0.000843s | 0.000868s | 0.000848s | byte equality observed |
| mp_and.test | 10 | 0.000843s | 0.000832s | 0.000853s | byte equality observed |
| mp_any.test | 10 | 0.000847s | 0.000862s | 0.000843s | byte equality observed |
| mp_any_2.test | 10 | 0.000840s | 0.000847s | 0.000849s | byte equality observed |
| mp_any_of.test | 10 | 0.000861s | 0.000845s | 0.000858s | byte equality observed |
| mp_any_of_q.test | 10 | 0.000850s | 0.000846s | 0.000858s | byte equality observed |
| mp_append.test | 10 | 0.000875s | 0.000854s | 0.000842s | byte equality observed |
| mp_append_2.test | 10 | 0.000876s | 0.000849s | 0.000852s | byte equality observed |
| mp_append_3.test | 10 | 0.000390s | 0.000389s | 0.000828s | byte equality observed |
| mp_append_sf.test | 10 | 0.000848s | 0.000868s | 0.000849s | byte equality observed |
| mp_apply_q.test | 10 | 0.000850s | 0.000857s | 0.000849s | byte equality observed |
| mp_apply_q_sf.test | 10 | 0.000865s | 0.000843s | 0.000857s | byte equality observed |
| mp_assign.test | 10 | 0.000848s | 0.000864s | 0.000862s | byte equality observed |
| mp_assign_2.test | 10 | 0.000392s | 0.000390s | 0.000837s | byte equality observed |
| mp_at.test | 10 | 0.000869s | 0.000880s | 0.000834s | byte equality observed |
| mp_at_2.test | 10 | 0.000388s | 0.000385s | 0.000840s | byte equality observed |
| mp_at_sf.test | 10 | 0.000857s | 0.000855s | 0.000843s | byte equality observed |
| mp_back.test | 10 | 0.000861s | 0.000865s | 0.000853s | byte equality observed |
| mp_back_2.test | 10 | 0.000394s | 0.000389s | 0.000834s | byte equality observed |
| mp_bind.test | 10 | 0.000860s | 0.000862s | 0.000852s | byte equality observed |
| mp_bind_back.test | 10 | 0.000843s | 0.000855s | 0.000833s | byte equality observed |
| mp_bind_front.test | 10 | 0.000832s | 0.000848s | 0.000839s | byte equality observed |
| mp_bind_q.test | 10 | 0.000848s | 0.000855s | 0.000855s | byte equality observed |
| mp_clear.test | 10 | 0.000850s | 0.000860s | 0.000849s | byte equality observed |
| mp_clear_2.test | 10 | 0.000388s | 0.000380s | 0.000829s | byte equality observed |
| mp_compose.test | 10 | 0.000842s | 0.000860s | 0.000848s | byte equality observed |
| mp_cond.test | 10 | 0.000847s | 0.000849s | 0.000861s | byte equality observed |
| mp_cond_sf.test | 10 | 0.000853s | 0.000846s | 0.000853s | byte equality observed |
| mp_contains.test | 10 | 0.000848s | 0.000841s | 0.000856s | byte equality observed |
| mp_copy_if.test | 10 | 0.000841s | 0.000850s | 0.000853s | byte equality observed |
| mp_copy_if_q.test | 10 | 0.000861s | 0.000867s | 0.000844s | byte equality observed |
| mp_count.test | 10 | 0.000843s | 0.000851s | 0.000840s | byte equality observed |
| mp_count_if.test | 10 | 0.000844s | 0.000854s | 0.000845s | byte equality observed |
| mp_count_if_q.test | 10 | 0.000852s | 0.000836s | 0.000845s | byte equality observed |
| mp_defer.test | 10 | 0.000851s | 0.000848s | 0.000848s | byte equality observed |
| mp_drop.test | 10 | 0.000856s | 0.000854s | 0.000853s | byte equality observed |
| mp_drop_2.test | 10 | 0.000393s | 0.000386s | 0.000831s | byte equality observed |
| mp_empty.test | 10 | 0.000857s | 0.000868s | 0.000852s | byte equality observed |
| mp_empty_2.test | 10 | 0.000385s | 0.000386s | 0.000822s | byte equality observed |
| mp_erase.test | 10 | 0.000871s | 0.000869s | 0.000837s | byte equality observed |
| mp_erase_2.test | 10 | 0.000392s | 0.000385s | 0.000824s | byte equality observed |
| mp_eval_if.test | 10 | 0.000848s | 0.000857s | 0.000845s | byte equality observed |
| mp_eval_if_not.test | 10 | 0.000842s | 0.000864s | 0.000856s | byte equality observed |
| mp_eval_if_sf.test | 10 | 0.000851s | 0.000852s | 0.000856s | byte equality observed |
| mp_eval_or.test | 10 | 0.000848s | 0.000840s | 0.000854s | byte equality observed |
| mp_fill.test | 10 | 0.000849s | 0.000855s | 0.000865s | byte equality observed |
| mp_fill_2.test | 10 | 0.000386s | 0.000391s | 0.000843s | byte equality observed |
| mp_filter.test | 10 | 0.000837s | 0.000870s | 0.000851s | byte equality observed |
| mp_find.test | 10 | 0.000851s | 0.000866s | 0.000847s | byte equality observed |
| mp_find_if.test | 10 | 0.000851s | 0.000842s | 0.000853s | byte equality observed |
| mp_find_if_q.test | 10 | 0.000851s | 0.000852s | 0.000845s | byte equality observed |
| mp_flatten.test | 10 | 0.000842s | 0.000875s | 0.000850s | byte equality observed |
| mp_fold.test | 10 | 0.000872s | 0.000855s | 0.000832s | byte equality observed |
| mp_fold_q.test | 10 | 0.000843s | 0.000859s | 0.000828s | byte equality observed |
| mp_fold_q_sf.test | 10 | 0.000847s | 0.000868s | 0.000840s | byte equality observed |
| mp_for_each.test | 10 | 0.000839s | 0.000861s | 0.000835s | byte equality observed |
| mp_from_sequence.test | 10 | 0.000841s | 0.000864s | 0.000845s | byte equality observed |
| mp_from_sequence_2.test | 10 | 0.000854s | 0.000858s | 0.000829s | byte equality observed |
| mp_from_sequence_3.test | 10 | 0.000842s | 0.000870s | 0.000846s | byte equality observed |
| mp_front.test | 10 | 0.000864s | 0.000859s | 0.000852s | byte equality observed |
| mp_front_2.test | 10 | 0.000388s | 0.000398s | 0.000820s | byte equality observed |
| mp_identity.test | 10 | 0.000861s | 0.000861s | 0.000843s | byte equality observed |
| mp_if.test | 10 | 0.000837s | 0.000852s | 0.000849s | byte equality observed |
| mp_if_sf.test | 10 | 0.000856s | 0.000834s | 0.000840s | byte equality observed |
| mp_inherit.test | 10 | 0.000853s | 0.000836s | 0.000876s | byte equality observed |
| mp_insert.test | 10 | 0.000875s | 0.000859s | 0.000855s | byte equality observed |
| mp_insert_2.test | 10 | 0.000389s | 0.000386s | 0.000838s | byte equality observed |
| mp_intersperse.test | 10 | 0.000839s | 0.000865s | 0.000849s | byte equality observed |
| mp_invoke_q.test | 10 | 0.000838s | 0.000848s | 0.000851s | byte equality observed |
| mp_invoke_q_sf.test | 10 | 0.000842s | 0.000835s | 0.000847s | byte equality observed |
| mp_iota.test | 10 | 0.000870s | 0.000855s | 0.000870s | byte equality observed |
| mp_iota_2.test | 10 | 0.000841s | 0.000868s | 0.000855s | byte equality observed |
| mp_is_list.test | 10 | 0.000851s | 0.000852s | 0.000835s | byte equality observed |
| mp_is_map.test | 10 | 0.000839s | 0.000855s | 0.000852s | byte equality observed |
| mp_is_set.test | 10 | 0.000860s | 0.000847s | 0.000839s | byte equality observed |
| mp_is_value_list.test | 10 | 0.000851s | 0.000869s | 0.000843s | byte equality observed |
| mp_iterate.test | 10 | 0.000844s | 0.000860s | 0.000833s | byte equality observed |
| mp_join.test | 10 | 0.000847s | 0.000876s | 0.000864s | byte equality observed |
| mp_lambda.test | 10 | 0.000861s | 0.000879s | 0.000849s | byte equality observed |
| mp_less.test | 10 | 0.000837s | 0.000857s | 0.000844s | byte equality observed |
| mp_list_c.test | 10 | 0.000843s | 0.000854s | 0.000860s | byte equality observed |
| mp_list_v.test | 10 | 0.000392s | 0.000388s | 0.000825s | byte equality observed |
| mp_map_contains.test | 10 | 0.000852s | 0.000879s | 0.000841s | byte equality observed |
| mp_map_erase.test | 10 | 0.000840s | 0.000834s | 0.000836s | byte equality observed |
| mp_map_find.test | 10 | 0.000850s | 0.000840s | 0.000853s | byte equality observed |
| mp_map_find_2.test | 10 | 0.000836s | 0.000843s | 0.000852s | byte equality observed |
| mp_map_find_3.test | 10 | 0.000848s | 0.000843s | 0.000842s | byte equality observed |
| mp_map_find_4.test | 10 | 0.000842s | 0.000842s | 0.000865s | byte equality observed |
| mp_map_insert.test | 10 | 0.000847s | 0.000855s | 0.000853s | byte equality observed |
| mp_map_keys.test | 10 | 0.000844s | 0.000851s | 0.000844s | byte equality observed |
| mp_map_replace.test | 10 | 0.000873s | 0.000862s | 0.000844s | byte equality observed |
| mp_map_update.test | 10 | 0.000835s | 0.000855s | 0.000836s | byte equality observed |
| mp_map_update_q.test | 10 | 0.000857s | 0.000859s | 0.000869s | byte equality observed |
| mp_max.test | 10 | 0.000856s | 0.000861s | 0.000838s | byte equality observed |
| mp_max_element.test | 10 | 0.000849s | 0.000847s | 0.000868s | byte equality observed |
| mp_max_element_2.test | 10 | 0.000389s | 0.000391s | 0.000832s | byte equality observed |
| mp_max_element_q.test | 10 | 0.000845s | 0.000866s | 0.000847s | byte equality observed |
| mp_max_element_q_2.test | 10 | 0.000384s | 0.000396s | 0.000839s | byte equality observed |
| mp_min.test | 10 | 0.000865s | 0.000845s | 0.000840s | byte equality observed |
| mp_min_element.test | 10 | 0.000847s | 0.000866s | 0.000862s | byte equality observed |
| mp_min_element_2.test | 10 | 0.000389s | 0.000385s | 0.000828s | byte equality observed |
| mp_min_element_q.test | 10 | 0.000863s | 0.000862s | 0.000851s | byte equality observed |
| mp_min_element_q_2.test | 10 | 0.000385s | 0.000385s | 0.000837s | byte equality observed |
| mp_none_of.test | 10 | 0.000844s | 0.000865s | 0.000838s | byte equality observed |
| mp_none_of_q.test | 10 | 0.000847s | 0.000865s | 0.000831s | byte equality observed |
| mp_not_fn.test | 10 | 0.000836s | 0.000847s | 0.000836s | byte equality observed |
| mp_nth_element.test | 10 | 0.000853s | 0.000851s | 0.000844s | byte equality observed |
| mp_nth_element_q.test | 10 | 0.000846s | 0.000854s | 0.000852s | byte equality observed |
| mp_or.test | 10 | 0.000850s | 0.000850s | 0.000850s | byte equality observed |
| mp_pairwise_fold.test | 10 | 0.000856s | 0.000860s | 0.000827s | byte equality observed |
| mp_pairwise_fold_q.test | 10 | 0.000861s | 0.000848s | 0.000847s | byte equality observed |
| mp_partial_sum.test | 10 | 0.000861s | 0.000865s | 0.000843s | byte equality observed |
| mp_partition.test | 10 | 0.000852s | 0.000858s | 0.000837s | byte equality observed |
| mp_partition_q.test | 10 | 0.000842s | 0.000849s | 0.000831s | byte equality observed |
| mp_plus.test | 10 | 0.000855s | 0.000842s | 0.000846s | byte equality observed |
| mp_pop_back.test | 10 | 0.000878s | 0.000873s | 0.000832s | byte equality observed |
| mp_pop_back_2.test | 10 | 0.000394s | 0.000382s | 0.000829s | byte equality observed |
| mp_pop_front.test | 10 | 0.000867s | 0.000866s | 0.000836s | byte equality observed |
| mp_pop_front_2.test | 10 | 0.000388s | 0.000384s | 0.000825s | byte equality observed |
| mp_power_set.test | 10 | 0.000845s | 0.000851s | 0.000853s | byte equality observed |
| mp_product.test | 10 | 0.000848s | 0.000855s | 0.000858s | byte equality observed |
| mp_push_back.test | 10 | 0.000850s | 0.000862s | 0.000852s | byte equality observed |
| mp_push_back_2.test | 10 | 0.000387s | 0.000388s | 0.000830s | byte equality observed |
| mp_push_front.test | 10 | 0.000858s | 0.000857s | 0.000856s | byte equality observed |
| mp_push_front_2.test | 10 | 0.000390s | 0.000387s | 0.000845s | byte equality observed |
| mp_quote.test | 10 | 0.000849s | 0.000865s | 0.000835s | byte equality observed |
| mp_quote_trait.test | 10 | 0.000854s | 0.000864s | 0.000854s | byte equality observed |
| mp_remove.test | 10 | 0.000851s | 0.000846s | 0.000863s | byte equality observed |
| mp_remove_if.test | 10 | 0.000851s | 0.000872s | 0.000848s | byte equality observed |
| mp_remove_if_q.test | 10 | 0.000855s | 0.000855s | 0.000844s | byte equality observed |
| mp_rename.test | 10 | 0.000852s | 0.000860s | 0.000860s | byte equality observed |
| mp_rename_2.test | 10 | 0.000389s | 0.000396s | 0.000820s | byte equality observed |
| mp_rename_v.test | 10 | 0.000388s | 0.000388s | 0.000831s | byte equality observed |
| mp_repeat.test | 10 | 0.000876s | 0.000877s | 0.000851s | byte equality observed |
| mp_repeat_2.test | 10 | 0.000389s | 0.000394s | 0.000832s | byte equality observed |
| mp_repeat_3.test | 10 | 0.000854s | 0.000857s | 0.000841s | byte equality observed |
| mp_replace.test | 10 | 0.000849s | 0.000850s | 0.000845s | byte equality observed |
| mp_replace_at.test | 10 | 0.000860s | 0.000847s | 0.000830s | byte equality observed |
| mp_replace_at_c.test | 10 | 0.000856s | 0.000863s | 0.000835s | byte equality observed |
| mp_replace_front.test | 10 | 0.000854s | 0.000862s | 0.000837s | byte equality observed |
| mp_replace_front_2.test | 10 | 0.000387s | 0.000389s | 0.000829s | byte equality observed |
| mp_replace_if.test | 10 | 0.000856s | 0.000862s | 0.000862s | byte equality observed |
| mp_replace_if_q.test | 10 | 0.000840s | 0.000851s | 0.000848s | byte equality observed |
| mp_replace_second.test | 10 | 0.000849s | 0.000863s | 0.000847s | byte equality observed |
| mp_replace_second_2.test | 10 | 0.000385s | 0.000390s | 0.000812s | byte equality observed |
| mp_replace_third.test | 10 | 0.000856s | 0.000846s | 0.000847s | byte equality observed |
| mp_replace_third_2.test | 10 | 0.000394s | 0.000387s | 0.000834s | byte equality observed |
| mp_reverse.test | 10 | 0.000870s | 0.000846s | 0.000864s | byte equality observed |
| mp_reverse_fold.test | 10 | 0.000864s | 0.000867s | 0.000845s | byte equality observed |
| mp_reverse_fold_q.test | 10 | 0.000839s | 0.000850s | 0.000871s | byte equality observed |
| mp_rotate_left.test | 10 | 0.000841s | 0.000852s | 0.000841s | byte equality observed |
| mp_rotate_right.test | 10 | 0.000859s | 0.000859s | 0.000841s | byte equality observed |
| mp_same.test | 10 | 0.000832s | 0.000856s | 0.000862s | byte equality observed |
| mp_second.test | 10 | 0.000865s | 0.000860s | 0.000829s | byte equality observed |
| mp_second_2.test | 10 | 0.000393s | 0.000383s | 0.000828s | byte equality observed |
| mp_set_contains.test | 10 | 0.000879s | 0.000857s | 0.000835s | byte equality observed |
| mp_set_difference.test | 10 | 0.000833s | 0.000843s | 0.000843s | byte equality observed |
| mp_set_difference_sf.test | 10 | 0.000839s | 0.000862s | 0.000839s | byte equality observed |
| mp_set_intersection.test | 10 | 0.000833s | 0.000846s | 0.000852s | byte equality observed |
| mp_set_intersection_sf.test | 10 | 0.000850s | 0.000837s | 0.000844s | byte equality observed |
| mp_set_push_back.test | 10 | 0.000853s | 0.000866s | 0.000852s | byte equality observed |
| mp_set_push_front.test | 10 | 0.000858s | 0.000854s | 0.000850s | byte equality observed |
| mp_set_union.test | 10 | 0.000858s | 0.000865s | 0.000852s | byte equality observed |
| mp_set_union_sf.test | 10 | 0.000846s | 0.000855s | 0.000850s | byte equality observed |
| mp_similar.test | 10 | 0.000855s | 0.000857s | 0.000853s | byte equality observed |
| mp_size.test | 10 | 0.000855s | 0.000856s | 0.000855s | byte equality observed |
| mp_size_2.test | 10 | 0.000386s | 0.000392s | 0.000835s | byte equality observed |
| mp_slice.test | 10 | 0.000885s | 0.000877s | 0.000849s | byte equality observed |
| mp_slice_2.test | 10 | 0.000392s | 0.000399s | 0.000832s | byte equality observed |
| mp_sliding_fold.test | 10 | 0.000875s | 0.000857s | 0.000852s | byte equality observed |
| mp_sliding_fold_q.test | 10 | 0.000873s | 0.000868s | 0.000849s | byte equality observed |
| mp_sort.test | 10 | 0.000854s | 0.000853s | 0.000837s | byte equality observed |
| mp_sort_q.test | 10 | 0.000846s | 0.000853s | 0.000842s | byte equality observed |
| mp_split.test | 10 | 0.000865s | 0.000859s | 0.000867s | byte equality observed |
| mp_starts_with.test | 10 | 0.000842s | 0.000861s | 0.000841s | byte equality observed |
| mp_starts_with_sf.test | 10 | 0.000846s | 0.000842s | 0.000833s | byte equality observed |
| mp_take.test | 10 | 0.000865s | 0.000869s | 0.000854s | byte equality observed |
| mp_take_2.test | 10 | 0.000384s | 0.000395s | 0.000840s | byte equality observed |
| mp_third.test | 10 | 0.000860s | 0.000857s | 0.000843s | byte equality observed |
| mp_third_2.test | 10 | 0.000387s | 0.000388s | 0.000824s | byte equality observed |
| mp_transform.test | 10 | 0.000874s | 0.000870s | 0.000851s | byte equality observed |
| mp_transform_2.test | 10 | 0.000843s | 0.000826s | 0.000831s | byte equality observed |
| mp_transform_front.test | 10 | 0.000857s | 0.000869s | 0.000846s | byte equality observed |
| mp_transform_front_2.test | 10 | 0.000389s | 0.000388s | 0.000826s | byte equality observed |
| mp_transform_if.test | 10 | 0.000845s | 0.000856s | 0.000853s | byte equality observed |
| mp_transform_if_q.test | 10 | 0.000852s | 0.000848s | 0.000835s | byte equality observed |
| mp_transform_q.test | 10 | 0.000869s | 0.000884s | 0.000847s | byte equality observed |
| mp_transform_q_2.test | 10 | 0.000852s | 0.000835s | 0.000853s | byte equality observed |
| mp_transform_second.test | 10 | 0.000854s | 0.000862s | 0.000844s | byte equality observed |
| mp_transform_second_2.test | 10 | 0.000393s | 0.000389s | 0.000824s | byte equality observed |
| mp_transform_sf.test | 10 | 0.000846s | 0.000871s | 0.000828s | byte equality observed |
| mp_transform_third.test | 10 | 0.000845s | 0.000857s | 0.000858s | byte equality observed |
| mp_transform_third_2.test | 10 | 0.000388s | 0.000393s | 0.000836s | byte equality observed |
| mp_unique.test | 10 | 0.000857s | 0.000857s | 0.000844s | byte equality observed |
| mp_unique_if.test | 10 | 0.000876s | 0.000859s | 0.000844s | byte equality observed |
| mp_unique_if_q.test | 10 | 0.000857s | 0.000839s | 0.000834s | byte equality observed |
| mp_valid.test | 10 | 0.000847s | 0.000833s | 0.000836s | byte equality observed |
| mp_valid_and_true.test | 10 | 0.000849s | 0.000859s | 0.000837s | byte equality observed |
| mp_value.test | 10 | 0.000391s | 0.000389s | 0.000820s | byte equality observed |
| mp_with_index.test | 10 | 0.001229s | 0.001194s | 0.001145s | byte equality observed |
| mp_with_index_cx.test | 10 | 0.000382s | 0.000391s | 0.000834s | byte equality observed |
| mpl.test | 10 | 0.000871s | 0.000847s | 0.000837s | byte equality observed |
| mpl_list.test | 10 | 0.000861s | 0.000853s | 0.000857s | byte equality observed |
| mpl_tuple.test | 10 | 0.000853s | 0.000850s | 0.000847s | byte equality observed |
| mpl_version.test | 10 | 0.000862s | 0.000845s | 0.000862s | byte equality observed |
| set_version.test | 10 | 0.000850s | 0.000856s | 0.000857s | byte equality observed |
| tuple_apply.test | 10 | 0.000848s | 0.000852s | 0.000831s | byte equality observed |
| tuple_for_each.test | 10 | 0.000861s | 0.000850s | 0.000844s | byte equality observed |
| tuple_transform.test | 10 | 0.000850s | 0.000853s | 0.000855s | byte equality observed |
| tuple_transform_2.test | 10 | 0.000852s | 0.000847s | 0.000837s | byte equality observed |
| tuple_version.test | 10 | 0.000853s | 0.000844s | 0.000838s | byte equality observed |
| utility_version.test | 10 | 0.000847s | 0.000849s | 0.000841s | byte equality observed |
| version.test | 10 | 0.000858s | 0.000838s | 0.000841s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409407).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
