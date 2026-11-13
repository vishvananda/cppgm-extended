# libs/mp11/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| mp11.test | COMPILE | 0.034911 | 0.069729 | 0.105354 | — | — | — |
| complex_h.test | COMPILE | 0.141113 | 0.300673 | 0.420040 | — | — | — |
| mp_size.test | RUN | 0.166416 | 0.992009 | 0.539869 | 0.001338 | 0.001340 | 0.001362 |
| mp_size_2.test | RUN | 0.014355 | 0.034425 | 0.047368 | 0.000422 | 0.000563 | 0.001321 |
| mp_empty.test | RUN | 0.160916 | 0.602767 | 0.515806 | 0.001200 | 0.001369 | 0.001372 |
| mp_empty_2.test | RUN | 0.014686 | 0.035870 | 0.050205 | 0.000500 | 0.000532 | 0.001245 |
| mp_assign.test | RUN | 0.184630 | 1.330306 | 0.695369 | 0.001319 | 0.001434 | 0.001178 |
| mp_assign_2.test | RUN | 0.014317 | 0.035689 | 0.046166 | 0.000534 | 0.000536 | 0.001295 |
| mp_clear.test | RUN | 0.162313 | 0.547012 | 0.524105 | 0.001401 | 0.001620 | 0.001288 |
| mp_clear_2.test | RUN | 0.014763 | 0.035148 | 0.049182 | 0.000538 | 0.000553 | 0.001276 |
| mp_front.test | RUN | 0.166614 | 0.913876 | 0.606093 | 0.001294 | 0.001458 | 0.001244 |
| mp_front_2.test | RUN | 0.013942 | 0.036680 | 0.052548 | 0.000474 | 0.000626 | 0.001257 |
| mp_pop_front.test | RUN | 0.174505 | 0.943821 | 0.589691 | 0.001271 | 0.001292 | 0.001322 |
| mp_pop_front_2.test | RUN | 0.014154 | 0.041552 | 0.047732 | 0.000527 | 0.000568 | 0.001243 |
| mp_second.test | RUN | 0.162831 | 0.578947 | 0.545993 | 0.001259 | 0.001336 | 0.001184 |
| mp_second_2.test | RUN | 0.014317 | 0.036175 | 0.049776 | 0.000464 | 0.000539 | 0.001332 |
| mp_third.test | RUN | 0.160374 | 0.535499 | 0.523813 | 0.001244 | 0.001386 | 0.001134 |
| mp_third_2.test | RUN | 0.014432 | 0.035337 | 0.049346 | 0.000520 | 0.000473 | 0.001186 |
| mp_push_front.test | RUN | 0.214618 | 1.992786 | 0.983983 | 0.001245 | 0.001362 | 0.001177 |
| mp_push_front_2.test | RUN | 0.014092 | 0.033322 | 0.048992 | 0.000465 | 0.000545 | 0.001235 |
| mp_push_back.test | RUN | 0.222097 | 1.993059 | 0.985117 | 0.001430 | 0.001575 | 0.001293 |
| mp_push_back_2.test | RUN | 0.014579 | 0.034856 | 0.049037 | 0.000507 | 0.000509 | 0.001280 |
| mp_rename.test | RUN | 0.211968 | 1.830791 | 0.805576 | 0.001257 | 0.001396 | 0.001223 |
| mp_rename_2.test | RUN | 0.011469 | 0.035191 | 0.049814 | 0.000559 | 0.000542 | 0.001282 |
| mp_append.test | RUN | 0.213963 | 2.077446 | 0.915895 | 0.001274 | 0.001390 | 0.001294 |
| mp_append_2.test | RUN | 0.483236 | 4.647777 | 1.227680 | 0.001268 | 0.001311 | 0.001298 |
| mp_append_3.test | RUN | 0.014159 | 0.034696 | 0.054996 | 0.000539 | 0.000527 | 0.001252 |
| mp_append_sf.test | RUN | 0.157924 | 0.491316 | 0.497475 | 0.001470 | 0.001466 | 0.001316 |
| mp_replace_front.test | RUN | 0.193121 | 1.474611 | 0.722535 | 0.001363 | 0.001331 | 0.001403 |
| mp_replace_front_2.test | RUN | 0.015256 | 0.034207 | 0.050086 | 0.000421 | 0.000466 | 0.001221 |
| mp_replace_second.test | RUN | 0.189614 | 0.942335 | 0.695431 | 0.001347 | 0.001361 | 0.001303 |
| mp_replace_second_2.test | RUN | 0.014938 | 0.034249 | 0.048280 | 0.000473 | 0.000612 | 0.001262 |
| mp_replace_third.test | RUN | 0.179300 | 0.779947 | 0.614006 | 0.001263 | 0.001465 | 0.001318 |
| mp_replace_third_2.test | RUN | 0.014621 | 0.035492 | 0.047733 | 0.000515 | 0.000466 | 0.001295 |
| mp_apply_q.test | RUN | 0.207121 | 1.820532 | 0.821372 | 0.001283 | 0.001339 | 0.001319 |
| mp_apply_q_sf.test | RUN | 0.163152 | 0.513755 | 0.508825 | 0.001271 | 0.001628 | 0.001242 |
| mp_is_list.test | RUN | 0.160669 | 0.611753 | 0.517590 | 0.001270 | 0.001326 | 0.001302 |
| mp_list_c.test | RUN | 0.170426 | 0.744318 | 0.584787 | 0.001349 | 0.001332 | 0.001250 |
| mp_transform_front.test | RUN | 0.192029 | 1.465052 | 0.701035 | 0.001267 | 0.001427 | 0.001334 |
| mp_transform_front_2.test | RUN | 0.013936 | 0.035171 | 0.050263 | 0.000522 | 0.000520 | 0.001323 |
| mp_transform_second.test | RUN | 0.181799 | 1.254615 | 0.664923 | 0.001245 | 0.000963 | 0.001240 |
| mp_transform_second_2.test | RUN | 0.015121 | 0.035218 | 0.046953 | 0.000486 | 0.000578 | 0.001217 |
| mp_transform_third.test | RUN | 0.170252 | 0.950346 | 0.590087 | 0.001328 | 0.001464 | 0.001339 |
| mp_transform_third_2.test | RUN | 0.014442 | 0.034765 | 0.049211 | 0.000575 | 0.000501 | 0.001162 |
| mp_list_v.test | RUN | 0.013659 | 0.034691 | 0.048752 | 0.000542 | 0.000547 | 0.001390 |
| mp_rename_v.test | RUN | 0.014081 | 0.034914 | 0.046863 | 0.000473 | 0.000507 | 0.001209 |
| mp_is_value_list.test | RUN | 0.152908 | 0.377772 | 0.488052 | 0.001278 | 0.001626 | 0.001292 |
| mp_transform.test | RUN | 0.380973 | 3.102207 | 1.552716 | 0.001397 | 0.001336 | 0.001390 |
| mp_transform_2.test | RUN | 0.147111 | 0.335883 | 0.444475 | 0.001249 | 0.001302 | 0.001265 |
| mp_transform_q.test | RUN | 0.371731 | 2.961401 | 1.504217 | 0.001236 | 0.001414 | 0.001359 |
| mp_transform_q_2.test | RUN | 0.394902 | 0.935479 | 1.899171 | 0.001331 | 0.001270 | 0.001199 |
| mp_transform_sf.test | RUN | 0.166738 | 0.521771 | 0.528459 | 0.001272 | 0.001329 | 0.001360 |
| mp_transform_if.test | RUN | 0.207467 | 1.484596 | 0.812822 | 0.001246 | 0.001147 | 0.001343 |
| mp_transform_if_q.test | RUN | 0.206649 | 1.478212 | 0.806396 | 0.001235 | 0.001278 | 0.001131 |
| mp_filter.test | RUN | 0.181596 | 0.873390 | 0.611334 | 0.001301 | 0.001795 | 0.001241 |
| mp_fill.test | RUN | 0.172627 | 0.653930 | 0.560933 | 0.001301 | 0.001391 | 0.001364 |
| mp_fill_2.test | RUN | 0.026830 | 0.053926 | 0.082876 | 0.000487 | 0.000532 | 0.001287 |
| mp_count.test | RUN | 0.333130 | 1.605702 | 0.907213 | 0.001228 | 0.001313 | 0.001293 |
| mp_count_if.test | RUN | 0.338076 | 1.634655 | 0.978337 | 0.001142 | 0.001314 | 0.001336 |
| mp_count_if_q.test | RUN | 0.180296 | 1.076859 | 0.583861 | 0.001491 | 0.001322 | 0.001336 |
| mp_contains.test | RUN | 0.168255 | 0.629358 | 0.544560 | 0.001237 | 0.001415 | 0.001261 |
| mp_starts_with.test | RUN | 0.169740 | 0.633739 | 0.532155 | 0.001286 | 0.001410 | 0.001177 |
| mp_starts_with_sf.test | RUN | 0.166514 | 0.535070 | 0.524284 | 0.001204 | 0.001212 | 0.001255 |
| mp_repeat.test | RUN | 0.261660 | 2.628592 | 1.045812 | 0.001400 | 0.001293 | 0.001288 |
| mp_repeat_2.test | RUN | 0.025252 | 0.054425 | 0.081922 | 0.000550 | 0.000559 | 0.001317 |
| mp_repeat_3.test | RUN | 0.210774 | 0.342344 | 0.520693 | 0.001443 | 0.001433 | 0.001312 |
| mp_product.test | RUN | 0.205349 | 1.583208 | 0.777476 | 0.001335 | 0.001340 | 0.001369 |
| mp_drop.test | RUN | 0.217492 | 1.994324 | 0.887240 | 0.001177 | 0.001516 | 0.001330 |
| mp_drop_2.test | RUN | 0.029322 | 0.055633 | 0.075539 | 0.000512 | 0.000604 | 0.001264 |
| mp_iota.test | RUN | 0.212374 | 1.677240 | 0.765772 | 0.001392 | 0.001279 | 0.001280 |
| mp_iota_2.test | RUN | 0.235525 | 2.014199 | 0.859637 | 0.001371 | 0.001459 | 0.001354 |
| mp_at.test | RUN | 0.185454 | 0.876623 | 0.618380 | 0.001434 | 0.001303 | 0.001326 |
| mp_at_2.test | RUN | 0.027452 | 0.057883 | 0.083506 | 0.000529 | 0.000532 | 0.001278 |
| mp_at_sf.test | RUN | 0.175486 | 0.526528 | 0.515815 | 0.001289 | 0.001480 | 0.001290 |
| mp_take.test | RUN | 0.306009 | 2.602621 | 1.030075 | 0.001426 | 0.001393 | 0.001309 |
| mp_take_2.test | RUN | 0.026946 | 0.054298 | 0.083244 | 0.000520 | 0.000542 | 0.001279 |
| mp_slice.test | RUN | 0.354697 | 2.654453 | 1.333250 | 0.001522 | 0.001376 | 0.001214 |
| mp_slice_2.test | RUN | 0.027141 | 0.054996 | 0.083424 | 0.000597 | 0.000563 | 0.001303 |
| mp_replace.test | RUN | 0.219669 | 1.336227 | 0.845422 | 0.001413 | 0.001362 | 0.001367 |
| mp_replace_if.test | RUN | 0.210310 | 1.173654 | 0.771843 | 0.001431 | 0.001304 | 0.001350 |
| mp_replace_if_q.test | RUN | 0.210464 | 1.182149 | 0.783782 | 0.001428 | 0.001344 | 0.001370 |
| mp_copy_if.test | RUN | 0.217553 | 1.659120 | 0.741008 | 0.001418 | 0.001316 | 0.001236 |
| mp_copy_if_q.test | RUN | 0.213907 | 1.648364 | 0.748620 | 0.001296 | 0.001357 | 0.001341 |
| mp_remove.test | RUN | 0.210038 | 1.107795 | 0.755993 | 0.001502 | 0.001334 | 0.001259 |
| mp_remove_if.test | RUN | 0.223618 | 1.739252 | 0.797292 | 0.001254 | 0.001482 | 0.001420 |
| mp_remove_if_q.test | RUN | 0.225160 | 1.739816 | 0.795969 | 0.001390 | 0.001345 | 0.001344 |
| mp_partition.test | RUN | 0.222102 | 1.647032 | 0.858658 | 0.001467 | 0.001281 | 0.001403 |
| mp_partition_q.test | RUN | 0.224097 | 1.642176 | 0.844792 | 0.001464 | 0.001320 | 0.001252 |
| mp_sort.test | RUN | 0.203765 | 1.174840 | 0.739601 | 0.001293 | 0.001357 | 0.001334 |
| mp_sort_q.test | RUN | 0.208980 | 1.187275 | 0.742658 | 0.001346 | 0.001269 | 0.001409 |
| mp_find.test | RUN | 0.199850 | 1.290064 | 0.651037 | 0.001447 | 0.001278 | 0.001205 |
| mp_find_if.test | RUN | 0.191083 | 1.184326 | 0.594200 | 0.001365 | 0.001238 | 0.001379 |
| mp_find_if_q.test | RUN | 0.209016 | 1.309157 | 0.751635 | 0.001320 | 0.001344 | 0.001309 |
| mp_reverse.test | RUN | 0.453321 | 3.453923 | 1.470695 | 0.001332 | 0.001452 | 0.001304 |
| mp_fold.test | RUN | 0.763654 | 2.656825 | 3.701154 | 0.001442 | 0.001346 | 0.001256 |
| mp_fold_q.test | RUN | 0.198099 | 1.169983 | 0.704352 | 0.001221 | 0.001357 | 0.001372 |
| mp_fold_q_sf.test | RUN | 0.174905 | 0.527394 | 0.548784 | 0.001350 | 0.001327 | 0.001346 |
| mp_reverse_fold.test | RUN | 0.347965 | 2.582440 | 1.033325 | 0.001335 | 0.001319 | 0.001267 |
| mp_reverse_fold_q.test | RUN | 0.206250 | 1.409079 | 0.749890 | 0.001318 | 0.001357 | 0.001223 |
| mp_unique.test | RUN | 0.201417 | 1.280583 | 0.734955 | 0.001315 | 0.001266 | 0.001379 |
| mp_unique_if.test | RUN | 0.360444 | 2.698667 | 1.165323 | 0.001398 | 0.001471 | 0.001258 |
| mp_unique_if_q.test | RUN | 0.314562 | 2.657192 | 0.955432 | 0.001302 | 0.001373 | 0.001366 |
| mp_all_of.test | RUN | 0.449599 | 1.448355 | 1.242735 | 0.001301 | 0.001348 | 0.001308 |
| mp_all_of_q.test | RUN | 0.161614 | 0.613342 | 0.534852 | 0.001274 | 0.001413 | 0.001306 |
| mp_any_of.test | RUN | 0.465924 | 1.494471 | 1.239287 | 0.001277 | 0.001747 | 0.001195 |
| mp_any_of_q.test | RUN | 0.176894 | 0.610427 | 0.526342 | 0.001318 | 0.001351 | 0.001319 |
| mp_none_of.test | RUN | 0.488703 | 1.512696 | 1.243616 | 0.001135 | 0.001373 | 0.001315 |
| mp_none_of_q.test | RUN | 0.180625 | 0.654966 | 0.534093 | 0.001278 | 0.001378 | 0.001367 |
| mp_replace_at.test | RUN | 0.207032 | 1.316051 | 0.775706 | 0.001290 | 0.001400 | 0.001305 |
| mp_replace_at_c.test | RUN | 0.206546 | 1.305268 | 0.830292 | 0.001278 | 0.001364 | 0.001375 |
| mp_for_each.test | RUN | 0.193263 | 0.885153 | 0.845847 | 0.001202 | 0.001321 | 0.001253 |
| mp_insert.test | RUN | 0.313762 | 2.648432 | 1.213924 | 0.001351 | 0.001363 | 0.001392 |
| mp_insert_2.test | RUN | 0.026222 | 0.052711 | 0.082530 | 0.000535 | 0.000490 | 0.001316 |
| mp_erase.test | RUN | 0.372918 | 2.781944 | 1.422955 | 0.001329 | 0.001345 | 0.001340 |
| mp_erase_2.test | RUN | 0.026110 | 0.056432 | 0.078271 | 0.000433 | 0.000543 | 0.001317 |
| mp_with_index.test | RUN | 0.447274 | 5.637410 | 3.048565 | 0.001756 | 0.001626 | 0.001680 |
| mp_with_index_cx.test | RUN | 0.003399 | 0.016394 | 0.024270 | 0.000565 | 0.000558 | 0.001267 |
| mp_from_sequence.test | RUN | 0.216990 | 1.469155 | 0.794724 | 0.001256 | 0.001381 | 0.001286 |
| mp_from_sequence_2.test | RUN | 0.220494 | 1.743651 | 0.859730 | 0.001354 | 0.001292 | 0.001376 |
| mp_from_sequence_3.test | RUN | 0.179804 | 0.757314 | 0.599610 | 0.001272 | 0.001446 | 0.001116 |
| mp_min_element.test | RUN | 0.172631 | 0.702422 | 0.562583 | 0.001249 | 0.001405 | 0.001229 |
| mp_min_element_2.test | RUN | 0.025854 | 0.054611 | 0.082762 | 0.000523 | 0.000551 | 0.001273 |
| mp_min_element_q.test | RUN | 0.174142 | 0.694298 | 0.577051 | 0.001333 | 0.001435 | 0.001431 |
| mp_min_element_q_2.test | RUN | 0.026215 | 0.052721 | 0.080213 | 0.000531 | 0.000503 | 0.001341 |
| mp_max_element.test | RUN | 0.171956 | 0.688493 | 0.574102 | 0.001244 | 0.001410 | 0.001283 |
| mp_max_element_2.test | RUN | 0.026081 | 0.052772 | 0.079202 | 0.000511 | 0.000491 | 0.001293 |
| mp_max_element_q.test | RUN | 0.166452 | 0.684498 | 0.555016 | 0.001320 | 0.001395 | 0.001401 |
| mp_max_element_q_2.test | RUN | 0.026244 | 0.055481 | 0.079094 | 0.000521 | 0.000547 | 0.001325 |
| mp_nth_element.test | RUN | 0.191170 | 0.978318 | 0.702038 | 0.001307 | 0.001351 | 0.001317 |
| mp_nth_element_q.test | RUN | 0.189278 | 0.972439 | 0.674855 | 0.001308 | 0.001191 | 0.001306 |
| mp_back.test | RUN | 0.185005 | 0.799588 | 0.652556 | 0.001312 | 0.001368 | 0.001336 |
| mp_back_2.test | RUN | 0.026060 | 0.052526 | 0.080393 | 0.000498 | 0.000532 | 0.001230 |
| mp_pop_back.test | RUN | 0.535084 | 5.398610 | 1.402150 | 0.001217 | 0.001402 | 0.001301 |
| mp_pop_back_2.test | RUN | 0.025651 | 0.055473 | 0.074758 | 0.000556 | 0.000558 | 0.001212 |
| mp_flatten.test | RUN | 0.218235 | 1.892981 | 0.939624 | 0.001326 | 0.001470 | 0.001280 |
| mp_rotate_left.test | RUN | 0.223185 | 2.043528 | 0.927185 | 0.001309 | 0.001274 | 0.001248 |
| mp_rotate_right.test | RUN | 0.223036 | 2.032223 | 0.925468 | 0.001307 | 0.001374 | 0.001210 |
| mp_power_set.test | RUN | 0.219484 | 1.972625 | 0.927061 | 0.001395 | 0.001334 | 0.001276 |
| mp_partial_sum.test | RUN | 0.291924 | 2.634804 | 1.065567 | 0.001346 | 0.001308 | 0.001282 |
| mp_iterate.test | RUN | 0.193538 | 0.800185 | 0.723898 | 0.001242 | 0.001417 | 0.001222 |
| mp_pairwise_fold.test | RUN | 0.220341 | 1.779774 | 0.944503 | 0.001310 | 0.001325 | 0.001235 |
| mp_pairwise_fold_q.test | RUN | 0.171720 | 0.516487 | 0.526512 | 0.001567 | 0.001365 | 0.001324 |
| mp_sliding_fold.test | RUN | 0.434070 | 3.455256 | 1.778728 | 0.001378 | 0.001381 | 0.001441 |
| mp_sliding_fold_q.test | RUN | 0.436718 | 3.461686 | 1.764352 | 0.001506 | 0.001403 | 0.001276 |
| mp_intersperse.test | RUN | 0.214315 | 1.267629 | 0.823313 | 0.001336 | 0.001402 | 0.001396 |
| mp_split.test | RUN | 0.298053 | 2.490464 | 1.138513 | 0.001258 | 0.001375 | 0.001230 |
| mp_join.test | RUN | 0.233616 | 2.081681 | 1.019236 | 0.001349 | 0.001430 | 0.001305 |
| integral.test | RUN | 0.177324 | 0.921905 | 0.605655 | 0.001286 | 0.001336 | 0.001363 |
| mp_value.test | RUN | 0.010850 | 0.029485 | 0.038232 | 0.000542 | 0.000541 | 0.001233 |
| mp_identity.test | RUN | 0.168784 | 0.838667 | 0.582180 | 0.001257 | 0.001292 | 0.001189 |
| mp_inherit.test | RUN | 0.165138 | 0.589717 | 0.541590 | 0.001605 | 0.001322 | 0.001232 |
| mp_if.test | RUN | 0.166717 | 0.681455 | 0.533749 | 0.001283 | 0.001347 | 0.001335 |
| mp_if_sf.test | RUN | 0.158438 | 0.500610 | 0.481509 | 0.001373 | 0.001430 | 0.001295 |
| mp_eval_if.test | RUN | 0.171412 | 0.728821 | 0.556137 | 0.001244 | 0.001239 | 0.001290 |
| mp_eval_if_sf.test | RUN | 0.156678 | 0.526089 | 0.503743 | 0.001388 | 0.001423 | 0.001237 |
| mp_valid.test | RUN | 0.159020 | 0.520484 | 0.490325 | 0.001227 | 0.001321 | 0.001196 |
| mp_defer.test | RUN | 0.172845 | 0.883427 | 0.605265 | 0.001350 | 0.001367 | 0.001220 |
| mp_quote.test | RUN | 0.179825 | 0.890746 | 0.628096 | 0.001258 | 0.001437 | 0.001288 |
| mp_invoke_q.test | RUN | 0.191618 | 1.167911 | 0.692250 | 0.001368 | 0.001471 | 0.001217 |
| mp_invoke_q_sf.test | RUN | 0.160885 | 0.516166 | 0.496896 | 0.001206 | 0.001352 | 0.001278 |
| mp_quote_trait.test | RUN | 0.172667 | 0.976819 | 0.604088 | 0.001303 | 0.001293 | 0.001212 |
| mp_cond.test | RUN | 0.160298 | 0.593441 | 0.525404 | 0.001232 | 0.001350 | 0.001233 |
| mp_cond_sf.test | RUN | 0.155304 | 0.509344 | 0.498560 | 0.001234 | 0.001291 | 0.001175 |
| mp_not_fn.test | RUN | 0.155164 | 0.516645 | 0.493908 | 0.001205 | 0.001330 | 0.001282 |
| mp_eval_if_not.test | RUN | 0.168673 | 0.734529 | 0.555277 | 0.001461 | 0.001360 | 0.001277 |
| mp_eval_or.test | RUN | 0.158456 | 0.385197 | 0.511425 | 0.001524 | 0.001332 | 0.001207 |
| mp_compose.test | RUN | 0.178713 | 1.207275 | 0.668659 | 0.001499 | 0.001341 | 0.001143 |
| mp_valid_and_true.test | RUN | 0.166795 | 0.617973 | 0.519492 | 0.001265 | 0.001402 | 0.001195 |
| mp_compose_sf.test | COMPILE | 0.037532 | 0.073688 | 0.109707 | — | — | — |
| integer_sequence.test | RUN | 0.210067 | 1.190294 | 0.756276 | 0.001345 | 0.001410 | 0.001088 |
| tuple_for_each.test | RUN | 0.165731 | 0.432239 | 0.540113 | 0.001296 | 0.001332 | 0.001204 |
| tuple_for_each_cx.test | COMPILE | 0.029113 | 0.060073 | 0.084299 | — | — | — |
| tuple_apply.test | RUN | 0.164909 | 0.425988 | 0.536133 | 0.001199 | 0.001380 | 0.001193 |
| tuple_apply_cx.test | COMPILE | 0.043155 | 0.087993 | 0.126796 | — | — | — |
| construct_from_tuple.test | RUN | 0.165306 | 0.466137 | 0.536849 | 0.001301 | 0.001357 | 0.001168 |
| construct_from_tuple_cx.test | COMPILE | 0.043270 | 0.089575 | 0.122025 | — | — | — |
| tuple_transform.test | RUN | 0.256372 | 0.944385 | 1.057682 | 0.001392 | 0.001012 | 0.001244 |
| tuple_transform_2.test | RUN | 0.222752 | 0.690487 | 0.965163 | 0.001335 | 0.001205 | 0.001315 |
| tuple_transform_cx.test | COMPILE | 0.037507 | 0.082916 | 0.113795 | — | — | — |
| mp_set_contains.test | RUN | 0.163186 | 0.620499 | 0.552359 | 0.001282 | 0.001350 | 0.001306 |
| mp_set_push_back.test | RUN | 0.221344 | 1.930004 | 0.953458 | 0.001423 | 0.001447 | 0.001297 |
| mp_set_push_front.test | RUN | 0.285841 | 2.320074 | 1.067467 | 0.001244 | 0.001402 | 0.001316 |
| mp_is_set.test | RUN | 0.169226 | 0.622833 | 0.556356 | 0.001474 | 0.001350 | 0.001351 |
| mp_set_union.test | RUN | 0.216364 | 1.654804 | 0.894670 | 0.001278 | 0.001340 | 0.001215 |
| mp_set_union_sf.test | RUN | 0.159675 | 0.495949 | 0.503591 | 0.001202 | 0.001344 | 0.001335 |
| mp_set_difference.test | RUN | 0.192368 | 1.281444 | 0.757711 | 0.001343 | 0.001320 | 0.001259 |
| mp_set_difference_sf.test | RUN | 0.156716 | 0.505379 | 0.502956 | 0.001298 | 0.001361 | 0.001290 |
| mp_set_intersection.test | RUN | 0.191206 | 1.276104 | 0.728038 | 0.001269 | 0.001332 | 0.001282 |
| mp_set_intersection_sf.test | RUN | 0.159009 | 0.497838 | 0.490285 | 0.001220 | 0.001133 | 0.001305 |
| mp_all.test | RUN | 0.164456 | 0.618687 | 0.528489 | 0.001200 | 0.001410 | 0.001317 |
| mp_all_2.test | RUN | 0.459532 | 1.333113 | 1.259335 | 0.001257 | 0.001345 | 0.001372 |
| mp_and.test | RUN | 0.160549 | 0.610548 | 0.519714 | 0.001297 | 0.001381 | 0.001266 |
| mp_any.test | RUN | 0.170520 | 0.621692 | 0.540723 | 0.001258 | 0.001306 | 0.001345 |
| mp_any_2.test | RUN | 0.477809 | 1.352807 | 1.237904 | 0.001279 | 0.001356 | 0.001167 |
| mp_or.test | RUN | 0.161863 | 0.609308 | 0.532577 | 0.001428 | 0.001364 | 0.001318 |
| mp_same.test | RUN | 0.335193 | 1.027200 | 0.835448 | 0.001375 | 0.001331 | 0.001241 |
| mp_plus.test | RUN | 0.164136 | 0.736066 | 0.618378 | 0.001241 | 0.001194 | 0.001268 |
| mp_less.test | RUN | 0.165045 | 0.618956 | 0.539634 | 0.001302 | 0.001229 | 0.001186 |
| mp_min.test | RUN | 0.160825 | 0.560980 | 0.522753 | 0.001327 | 0.001350 | 0.001194 |
| mp_max.test | RUN | 0.165719 | 0.625901 | 0.537765 | 0.001409 | 0.001263 | 0.001114 |
| mp_similar.test | RUN | 0.164289 | 0.613333 | 0.532728 | 0.001143 | 0.001536 | 0.001234 |
| mp_map_find.test | RUN | 0.197320 | 1.136471 | 0.711590 | 0.001403 | 0.001458 | 0.001251 |
| mp_map_find_2.test | RUN | 0.177972 | 0.434263 | 0.604999 | 0.001288 | 0.001327 | 0.001260 |
| mp_map_find_3.test | RUN | 0.175415 | 0.434373 | 0.602714 | 0.001250 | 0.001254 | 0.001238 |
| mp_map_find_4.test | RUN | 0.165431 | 0.407403 | 0.543973 | 0.001356 | 0.001240 | 0.001243 |
| mp_map_contains.test | RUN | 0.173432 | 0.633264 | 0.553443 | 0.001376 | 0.001461 | 0.001281 |
| mp_map_insert.test | RUN | 0.237491 | 2.164737 | 1.015915 | 0.001611 | 0.001374 | 0.001132 |
| mp_map_replace.test | RUN | 0.311589 | 2.476509 | 1.175537 | 0.001331 | 0.001400 | 0.001183 |
| mp_map_erase.test | RUN | 0.228382 | 1.845280 | 0.976606 | 0.001214 | 0.001359 | 0.001170 |
| mp_map_update.test | RUN | 0.199353 | 1.179800 | 0.743315 | 0.001318 | 0.001350 | 0.000840 |
| mp_map_update_q.test | RUN | 0.198305 | 1.180755 | 0.730095 | 0.001457 | 0.001381 | 0.001235 |
| mp_map_keys.test | RUN | 0.195269 | 1.046957 | 0.710472 | 0.001248 | 0.001370 | 0.001242 |
| mp_is_map.test | RUN | 0.177813 | 0.643188 | 0.564488 | 0.001180 | 0.001316 | 0.001178 |
| mp_bind.test | RUN | 0.214681 | 1.920971 | 0.866955 | 0.001267 | 0.001365 | 0.001290 |
| mp_bind_q.test | RUN | 0.192109 | 1.183176 | 0.711349 | 0.001314 | 0.001463 | 0.001334 |
| mp_bind_front.test | RUN | 0.186593 | 1.294250 | 0.684171 | 0.001401 | 0.001315 | 0.001326 |
| mp_bind_back.test | RUN | 0.187355 | 1.303577 | 0.695177 | 0.001239 | 0.001300 | 0.001203 |
| mp_lambda.test | RUN | 0.437536 | 3.820645 | 1.878413 | 0.001283 | 0.001022 | 0.001169 |
| mpl.test | RUN | 0.381916 | 0.997012 | 1.493065 | 0.001225 | 0.001327 | 0.001225 |
| mpl_list.test | RUN | 0.209046 | 0.774360 | 0.678322 | 0.001365 | 0.001306 | 0.001299 |
| mpl_tuple.test | RUN | 0.208929 | 0.761051 | 0.679195 | 0.001256 | 0.001389 | 0.001235 |
| version.test | RUN | 0.105602 | 0.275843 | 0.342605 | 0.001430 | 0.001317 | 0.001222 |
| mp11_version.test | RUN | 0.128722 | 0.311559 | 0.393983 | 0.001263 | 0.001417 | 0.001134 |
| list_version.test | RUN | 0.106982 | 0.282970 | 0.349009 | 0.001229 | 0.001225 | 0.001183 |
| algorithm_version.test | RUN | 0.122991 | 0.298430 | 0.366468 | 0.001218 | 0.001368 | 0.001272 |
| integral_version.test | RUN | 0.107446 | 0.273628 | 0.328486 | 0.001376 | 0.001126 | 0.001165 |
| utility_version.test | RUN | 0.105652 | 0.277333 | 0.331772 | 0.001200 | 0.001215 | 0.001149 |
| function_version.test | RUN | 0.107156 | 0.279878 | 0.346462 | 0.001219 | 0.001529 | 0.001154 |
| map_version.test | RUN | 0.120889 | 0.295119 | 0.370635 | 0.001329 | 0.001234 | 0.001117 |
| set_version.test | RUN | 0.111196 | 0.281133 | 0.346040 | 0.001232 | 0.001187 | 0.000914 |
| bind_version.test | RUN | 0.118694 | 0.296938 | 0.355383 | 0.000946 | 0.001278 | 0.001254 |
| integer_sequence_version.test | RUN | 0.102467 | 0.271475 | 0.335410 | 0.001393 | 0.001413 | 0.000883 |
| tuple_version.test | RUN | 0.122485 | 0.295445 | 0.228563 | 0.001007 | 0.001284 | 0.001243 |
| mpl_version.test | RUN | 0.081522 | 0.192487 | 0.357028 | 0.000994 | 0.001024 | 0.000850 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.995× | 1.002×, 0.988× | 1.001× |
| clang | 1.104× | 1.108×, 1.101× | 0.985× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345325).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| algorithm_version.test | 10 | 0.000845s | 0.000813s | 0.000842s | byte equality observed |
| bind_version.test | 10 | 0.000842s | 0.000812s | 0.000856s | byte equality observed |
| construct_from_tuple.test | 10 | 0.000845s | 0.000858s | 0.000916s | byte equality observed |
| function_version.test | 10 | 0.000835s | 0.000839s | 0.000918s | byte equality observed |
| integer_sequence.test | 10 | 0.000844s | 0.000853s | 0.000924s | byte equality observed |
| integer_sequence_version.test | 10 | 0.000834s | 0.000853s | 0.000908s | byte equality observed |
| integral.test | 10 | 0.000837s | 0.000846s | 0.000913s | byte equality observed |
| integral_version.test | 10 | 0.000837s | 0.000834s | 0.000922s | byte equality observed |
| list_version.test | 10 | 0.000837s | 0.000834s | 0.000916s | byte equality observed |
| map_version.test | 10 | 0.000831s | 0.000811s | 0.000919s | byte equality observed |
| mp11_version.test | 10 | 0.000833s | 0.000810s | 0.000886s | byte equality observed |
| mp_all.test | 10 | 0.000837s | 0.000820s | 0.000902s | byte equality observed |
| mp_all_2.test | 10 | 0.000839s | 0.000821s | 0.000889s | byte equality observed |
| mp_all_of.test | 10 | 0.000845s | 0.000813s | 0.000895s | byte equality observed |
| mp_all_of_q.test | 10 | 0.000853s | 0.000833s | 0.000919s | byte equality observed |
| mp_and.test | 10 | 0.000841s | 0.000847s | 0.000895s | byte equality observed |
| mp_any.test | 10 | 0.000842s | 0.000847s | 0.000884s | byte equality observed |
| mp_any_2.test | 10 | 0.000835s | 0.000842s | 0.000880s | byte equality observed |
| mp_any_of.test | 10 | 0.000847s | 0.000851s | 0.000933s | byte equality observed |
| mp_any_of_q.test | 10 | 0.000848s | 0.000842s | 0.000892s | byte equality observed |
| mp_append.test | 10 | 0.000857s | 0.000857s | 0.000885s | byte equality observed |
| mp_append_2.test | 10 | 0.000855s | 0.000888s | 0.000924s | byte equality observed |
| mp_append_3.test | 10 | 0.000385s | 0.000407s | 0.000894s | byte equality observed |
| mp_append_sf.test | 10 | 0.000860s | 0.000910s | 0.000928s | byte equality observed |
| mp_apply_q.test | 10 | 0.000874s | 0.000901s | 0.000914s | byte equality observed |
| mp_apply_q_sf.test | 10 | 0.000872s | 0.000906s | 0.000900s | byte equality observed |
| mp_assign.test | 10 | 0.000882s | 0.000897s | 0.000910s | byte equality observed |
| mp_assign_2.test | 10 | 0.000406s | 0.000397s | 0.000916s | byte equality observed |
| mp_at.test | 10 | 0.000900s | 0.000899s | 0.000906s | byte equality observed |
| mp_at_2.test | 10 | 0.000403s | 0.000395s | 0.000861s | byte equality observed |
| mp_at_sf.test | 10 | 0.000908s | 0.000878s | 0.000901s | byte equality observed |
| mp_back.test | 10 | 0.000905s | 0.000915s | 0.000908s | byte equality observed |
| mp_back_2.test | 10 | 0.000396s | 0.000405s | 0.000878s | byte equality observed |
| mp_bind.test | 10 | 0.000883s | 0.000909s | 0.000881s | byte equality observed |
| mp_bind_back.test | 10 | 0.000890s | 0.000912s | 0.000876s | byte equality observed |
| mp_bind_front.test | 10 | 0.000905s | 0.000902s | 0.000887s | byte equality observed |
| mp_bind_q.test | 10 | 0.000903s | 0.000922s | 0.000885s | byte equality observed |
| mp_clear.test | 10 | 0.000918s | 0.000896s | 0.000902s | byte equality observed |
| mp_clear_2.test | 10 | 0.000407s | 0.000387s | 0.000890s | byte equality observed |
| mp_compose.test | 10 | 0.000918s | 0.000876s | 0.000877s | byte equality observed |
| mp_cond.test | 10 | 0.000892s | 0.000918s | 0.000927s | byte equality observed |
| mp_cond_sf.test | 10 | 0.000889s | 0.000894s | 0.000937s | byte equality observed |
| mp_contains.test | 10 | 0.000907s | 0.000879s | 0.000883s | byte equality observed |
| mp_copy_if.test | 10 | 0.000914s | 0.000895s | 0.000912s | byte equality observed |
| mp_copy_if_q.test | 10 | 0.000920s | 0.000889s | 0.000925s | byte equality observed |
| mp_count.test | 10 | 0.000896s | 0.000876s | 0.000955s | byte equality observed |
| mp_count_if.test | 10 | 0.000906s | 0.000927s | 0.000950s | byte equality observed |
| mp_count_if_q.test | 10 | 0.000894s | 0.000924s | 0.000902s | byte equality observed |
| mp_defer.test | 10 | 0.000887s | 0.000916s | 0.000878s | byte equality observed |
| mp_drop.test | 10 | 0.000899s | 0.000900s | 0.000907s | byte equality observed |
| mp_drop_2.test | 10 | 0.000398s | 0.000399s | 0.000912s | byte equality observed |
| mp_empty.test | 10 | 0.000907s | 0.000917s | 0.000892s | byte equality observed |
| mp_empty_2.test | 10 | 0.000390s | 0.000397s | 0.000900s | byte equality observed |
| mp_erase.test | 10 | 0.000932s | 0.000915s | 0.000891s | byte equality observed |
| mp_erase_2.test | 10 | 0.000394s | 0.000389s | 0.000908s | byte equality observed |
| mp_eval_if.test | 10 | 0.000890s | 0.000900s | 0.000903s | byte equality observed |
| mp_eval_if_not.test | 10 | 0.000886s | 0.000885s | 0.000877s | byte equality observed |
| mp_eval_if_sf.test | 10 | 0.000886s | 0.000891s | 0.000877s | byte equality observed |
| mp_eval_or.test | 10 | 0.000874s | 0.000865s | 0.000908s | byte equality observed |
| mp_fill.test | 10 | 0.000896s | 0.000890s | 0.000886s | byte equality observed |
| mp_fill_2.test | 10 | 0.000399s | 0.000384s | 0.000865s | byte equality observed |
| mp_filter.test | 10 | 0.000896s | 0.000921s | 0.000896s | byte equality observed |
| mp_find.test | 10 | 0.000903s | 0.000926s | 0.000875s | byte equality observed |
| mp_find_if.test | 10 | 0.000887s | 0.000891s | 0.000887s | byte equality observed |
| mp_find_if_q.test | 10 | 0.000893s | 0.000877s | 0.000896s | byte equality observed |
| mp_flatten.test | 10 | 0.000909s | 0.000867s | 0.000881s | byte equality observed |
| mp_fold.test | 10 | 0.000921s | 0.000882s | 0.000878s | byte equality observed |
| mp_fold_q.test | 10 | 0.000911s | 0.000905s | 0.000894s | byte equality observed |
| mp_fold_q_sf.test | 10 | 0.000894s | 0.000891s | 0.000910s | byte equality observed |
| mp_for_each.test | 10 | 0.000899s | 0.000885s | 0.000909s | byte equality observed |
| mp_from_sequence.test | 10 | 0.000881s | 0.000929s | 0.000881s | byte equality observed |
| mp_from_sequence_2.test | 10 | 0.000893s | 0.000877s | 0.000877s | byte equality observed |
| mp_from_sequence_3.test | 10 | 0.000899s | 0.000867s | 0.000880s | byte equality observed |
| mp_front.test | 10 | 0.000915s | 0.000875s | 0.000926s | byte equality observed |
| mp_front_2.test | 10 | 0.000409s | 0.000396s | 0.000868s | byte equality observed |
| mp_identity.test | 10 | 0.000913s | 0.000903s | 0.000878s | byte equality observed |
| mp_if.test | 10 | 0.000898s | 0.000867s | 0.000901s | byte equality observed |
| mp_if_sf.test | 10 | 0.000916s | 0.000893s | 0.000858s | byte equality observed |
| mp_inherit.test | 10 | 0.000909s | 0.000854s | 0.000921s | byte equality observed |
| mp_insert.test | 10 | 0.000922s | 0.000921s | 0.000877s | byte equality observed |
| mp_insert_2.test | 10 | 0.000400s | 0.000388s | 0.000876s | byte equality observed |
| mp_intersperse.test | 10 | 0.000890s | 0.000914s | 0.000881s | byte equality observed |
| mp_invoke_q.test | 10 | 0.000905s | 0.000886s | 0.000875s | byte equality observed |
| mp_invoke_q_sf.test | 10 | 0.000891s | 0.000914s | 0.000873s | byte equality observed |
| mp_iota.test | 10 | 0.000903s | 0.000907s | 0.000882s | byte equality observed |
| mp_iota_2.test | 10 | 0.000891s | 0.000913s | 0.000872s | byte equality observed |
| mp_is_list.test | 10 | 0.000893s | 0.000901s | 0.000867s | byte equality observed |
| mp_is_map.test | 10 | 0.000878s | 0.000877s | 0.000886s | byte equality observed |
| mp_is_set.test | 10 | 0.000908s | 0.000909s | 0.000870s | byte equality observed |
| mp_is_value_list.test | 10 | 0.000884s | 0.000872s | 0.000884s | byte equality observed |
| mp_iterate.test | 10 | 0.000887s | 0.000891s | 0.000902s | byte equality observed |
| mp_join.test | 10 | 0.000895s | 0.000902s | 0.000902s | byte equality observed |
| mp_lambda.test | 10 | 0.000921s | 0.000915s | 0.000868s | byte equality observed |
| mp_less.test | 10 | 0.000900s | 0.000886s | 0.000884s | byte equality observed |
| mp_list_c.test | 10 | 0.000889s | 0.000896s | 0.000864s | byte equality observed |
| mp_list_v.test | 10 | 0.000401s | 0.000400s | 0.000881s | byte equality observed |
| mp_map_contains.test | 10 | 0.000879s | 0.000872s | 0.000902s | byte equality observed |
| mp_map_erase.test | 10 | 0.000924s | 0.000901s | 0.000923s | byte equality observed |
| mp_map_find.test | 10 | 0.000882s | 0.000895s | 0.000899s | byte equality observed |
| mp_map_find_2.test | 10 | 0.000924s | 0.000895s | 0.000900s | byte equality observed |
| mp_map_find_3.test | 10 | 0.000906s | 0.000856s | 0.000890s | byte equality observed |
| mp_map_find_4.test | 10 | 0.000890s | 0.000891s | 0.000895s | byte equality observed |
| mp_map_insert.test | 10 | 0.000902s | 0.000883s | 0.000917s | byte equality observed |
| mp_map_keys.test | 10 | 0.000885s | 0.000887s | 0.000900s | byte equality observed |
| mp_map_replace.test | 10 | 0.000902s | 0.000892s | 0.000886s | byte equality observed |
| mp_map_update.test | 10 | 0.000893s | 0.000879s | 0.000888s | byte equality observed |
| mp_map_update_q.test | 10 | 0.000900s | 0.000885s | 0.000890s | byte equality observed |
| mp_max.test | 10 | 0.000903s | 0.000890s | 0.000869s | byte equality observed |
| mp_max_element.test | 10 | 0.000892s | 0.000896s | 0.000898s | byte equality observed |
| mp_max_element_2.test | 10 | 0.000409s | 0.000388s | 0.000906s | byte equality observed |
| mp_max_element_q.test | 10 | 0.000877s | 0.000900s | 0.000896s | byte equality observed |
| mp_max_element_q_2.test | 10 | 0.000400s | 0.000387s | 0.000899s | byte equality observed |
| mp_min.test | 10 | 0.000886s | 0.000896s | 0.000904s | byte equality observed |
| mp_min_element.test | 10 | 0.000889s | 0.000921s | 0.000894s | byte equality observed |
| mp_min_element_2.test | 10 | 0.000394s | 0.000387s | 0.000930s | byte equality observed |
| mp_min_element_q.test | 10 | 0.000911s | 0.000900s | 0.000891s | byte equality observed |
| mp_min_element_q_2.test | 10 | 0.000390s | 0.000390s | 0.000897s | byte equality observed |
| mp_none_of.test | 10 | 0.000897s | 0.000923s | 0.000899s | byte equality observed |
| mp_none_of_q.test | 10 | 0.000899s | 0.000882s | 0.000892s | byte equality observed |
| mp_not_fn.test | 10 | 0.000884s | 0.000904s | 0.000903s | byte equality observed |
| mp_nth_element.test | 10 | 0.000887s | 0.000896s | 0.000913s | byte equality observed |
| mp_nth_element_q.test | 10 | 0.000864s | 0.000890s | 0.000913s | byte equality observed |
| mp_or.test | 10 | 0.000900s | 0.000894s | 0.000926s | byte equality observed |
| mp_pairwise_fold.test | 10 | 0.000901s | 0.000870s | 0.000905s | byte equality observed |
| mp_pairwise_fold_q.test | 10 | 0.000902s | 0.000889s | 0.000879s | byte equality observed |
| mp_partial_sum.test | 10 | 0.000895s | 0.000873s | 0.000885s | byte equality observed |
| mp_partition.test | 10 | 0.000909s | 0.000895s | 0.000888s | byte equality observed |
| mp_partition_q.test | 10 | 0.000890s | 0.000865s | 0.000895s | byte equality observed |
| mp_plus.test | 10 | 0.000913s | 0.000897s | 0.000873s | byte equality observed |
| mp_pop_back.test | 10 | 0.000925s | 0.000897s | 0.000909s | byte equality observed |
| mp_pop_back_2.test | 10 | 0.000394s | 0.000392s | 0.000877s | byte equality observed |
| mp_pop_front.test | 10 | 0.000904s | 0.000888s | 0.000880s | byte equality observed |
| mp_pop_front_2.test | 10 | 0.000393s | 0.000390s | 0.000872s | byte equality observed |
| mp_power_set.test | 10 | 0.000896s | 0.000893s | 0.000887s | byte equality observed |
| mp_product.test | 10 | 0.000919s | 0.000908s | 0.000916s | byte equality observed |
| mp_push_back.test | 10 | 0.000884s | 0.000904s | 0.000884s | byte equality observed |
| mp_push_back_2.test | 10 | 0.000399s | 0.000391s | 0.000867s | byte equality observed |
| mp_push_front.test | 10 | 0.000913s | 0.000899s | 0.000898s | byte equality observed |
| mp_push_front_2.test | 10 | 0.000395s | 0.000375s | 0.000892s | byte equality observed |
| mp_quote.test | 10 | 0.000902s | 0.000877s | 0.000904s | byte equality observed |
| mp_quote_trait.test | 10 | 0.000879s | 0.000885s | 0.000910s | byte equality observed |
| mp_remove.test | 10 | 0.000895s | 0.000898s | 0.000896s | byte equality observed |
| mp_remove_if.test | 10 | 0.000907s | 0.000933s | 0.000871s | byte equality observed |
| mp_remove_if_q.test | 10 | 0.000885s | 0.000901s | 0.000899s | byte equality observed |
| mp_rename.test | 10 | 0.000877s | 0.000879s | 0.000875s | byte equality observed |
| mp_rename_2.test | 10 | 0.000399s | 0.000392s | 0.000877s | byte equality observed |
| mp_rename_v.test | 10 | 0.000396s | 0.000392s | 0.000850s | byte equality observed |
| mp_repeat.test | 10 | 0.000890s | 0.000915s | 0.000896s | byte equality observed |
| mp_repeat_2.test | 10 | 0.000390s | 0.000397s | 0.000881s | byte equality observed |
| mp_repeat_3.test | 10 | 0.000884s | 0.000908s | 0.000886s | byte equality observed |
| mp_replace.test | 10 | 0.000881s | 0.000889s | 0.000873s | byte equality observed |
| mp_replace_at.test | 10 | 0.000894s | 0.000884s | 0.000909s | byte equality observed |
| mp_replace_at_c.test | 10 | 0.000888s | 0.000901s | 0.000927s | byte equality observed |
| mp_replace_front.test | 10 | 0.000906s | 0.000908s | 0.000886s | byte equality observed |
| mp_replace_front_2.test | 10 | 0.000397s | 0.000392s | 0.000892s | byte equality observed |
| mp_replace_if.test | 10 | 0.000892s | 0.000897s | 0.000884s | byte equality observed |
| mp_replace_if_q.test | 10 | 0.000870s | 0.000882s | 0.000949s | byte equality observed |
| mp_replace_second.test | 10 | 0.000900s | 0.000884s | 0.000905s | byte equality observed |
| mp_replace_second_2.test | 10 | 0.000386s | 0.000389s | 0.000891s | byte equality observed |
| mp_replace_third.test | 10 | 0.000882s | 0.000894s | 0.000922s | byte equality observed |
| mp_replace_third_2.test | 10 | 0.000393s | 0.000396s | 0.000880s | byte equality observed |
| mp_reverse.test | 10 | 0.000897s | 0.000949s | 0.000919s | byte equality observed |
| mp_reverse_fold.test | 10 | 0.000900s | 0.000870s | 0.000893s | byte equality observed |
| mp_reverse_fold_q.test | 10 | 0.000871s | 0.000906s | 0.000873s | byte equality observed |
| mp_rotate_left.test | 10 | 0.000879s | 0.000899s | 0.000902s | byte equality observed |
| mp_rotate_right.test | 10 | 0.000883s | 0.000902s | 0.000891s | byte equality observed |
| mp_same.test | 10 | 0.000893s | 0.000906s | 0.000875s | byte equality observed |
| mp_second.test | 10 | 0.000905s | 0.000877s | 0.000887s | byte equality observed |
| mp_second_2.test | 10 | 0.000388s | 0.000393s | 0.000869s | byte equality observed |
| mp_set_contains.test | 10 | 0.000890s | 0.000900s | 0.000889s | byte equality observed |
| mp_set_difference.test | 10 | 0.000880s | 0.000891s | 0.000864s | byte equality observed |
| mp_set_difference_sf.test | 10 | 0.000878s | 0.000873s | 0.000909s | byte equality observed |
| mp_set_intersection.test | 10 | 0.000884s | 0.000888s | 0.000887s | byte equality observed |
| mp_set_intersection_sf.test | 10 | 0.000873s | 0.000874s | 0.000879s | byte equality observed |
| mp_set_push_back.test | 10 | 0.000879s | 0.000878s | 0.000904s | byte equality observed |
| mp_set_push_front.test | 10 | 0.000896s | 0.000914s | 0.000904s | byte equality observed |
| mp_set_union.test | 10 | 0.000883s | 0.000863s | 0.000907s | byte equality observed |
| mp_set_union_sf.test | 10 | 0.000889s | 0.000883s | 0.000866s | byte equality observed |
| mp_similar.test | 10 | 0.000894s | 0.000904s | 0.000897s | byte equality observed |
| mp_size.test | 10 | 0.000883s | 0.000877s | 0.000910s | byte equality observed |
| mp_size_2.test | 10 | 0.000396s | 0.000391s | 0.000867s | byte equality observed |
| mp_slice.test | 10 | 0.000917s | 0.000892s | 0.000878s | byte equality observed |
| mp_slice_2.test | 10 | 0.000392s | 0.000397s | 0.000886s | byte equality observed |
| mp_sliding_fold.test | 10 | 0.000892s | 0.000881s | 0.000910s | byte equality observed |
| mp_sliding_fold_q.test | 10 | 0.000917s | 0.000880s | 0.000905s | byte equality observed |
| mp_sort.test | 10 | 0.000867s | 0.000867s | 0.000856s | byte equality observed |
| mp_sort_q.test | 10 | 0.000870s | 0.000909s | 0.000891s | byte equality observed |
| mp_split.test | 10 | 0.000908s | 0.000916s | 0.000900s | byte equality observed |
| mp_starts_with.test | 10 | 0.000898s | 0.000891s | 0.000920s | byte equality observed |
| mp_starts_with_sf.test | 10 | 0.000875s | 0.000876s | 0.000908s | byte equality observed |
| mp_take.test | 10 | 0.000915s | 0.000877s | 0.000907s | byte equality observed |
| mp_take_2.test | 10 | 0.000400s | 0.000389s | 0.000871s | byte equality observed |
| mp_third.test | 10 | 0.000893s | 0.000908s | 0.000873s | byte equality observed |
| mp_third_2.test | 10 | 0.000393s | 0.000385s | 0.000862s | byte equality observed |
| mp_transform.test | 10 | 0.000903s | 0.000926s | 0.000902s | byte equality observed |
| mp_transform_2.test | 10 | 0.000872s | 0.000852s | 0.000858s | byte equality observed |
| mp_transform_front.test | 10 | 0.000874s | 0.000888s | 0.000899s | byte equality observed |
| mp_transform_front_2.test | 10 | 0.000398s | 0.000390s | 0.000871s | byte equality observed |
| mp_transform_if.test | 10 | 0.000877s | 0.000888s | 0.000945s | byte equality observed |
| mp_transform_if_q.test | 10 | 0.000885s | 0.000880s | 0.000881s | byte equality observed |
| mp_transform_q.test | 10 | 0.000900s | 0.000889s | 0.000901s | byte equality observed |
| mp_transform_q_2.test | 10 | 0.000878s | 0.000868s | 0.000900s | byte equality observed |
| mp_transform_second.test | 10 | 0.000889s | 0.000876s | 0.000883s | byte equality observed |
| mp_transform_second_2.test | 10 | 0.000387s | 0.000385s | 0.000862s | byte equality observed |
| mp_transform_sf.test | 10 | 0.000896s | 0.000882s | 0.000912s | byte equality observed |
| mp_transform_third.test | 10 | 0.000880s | 0.000880s | 0.000903s | byte equality observed |
| mp_transform_third_2.test | 10 | 0.000395s | 0.000379s | 0.000869s | byte equality observed |
| mp_unique.test | 10 | 0.000888s | 0.000875s | 0.000880s | byte equality observed |
| mp_unique_if.test | 10 | 0.000920s | 0.000923s | 0.000877s | byte equality observed |
| mp_unique_if_q.test | 10 | 0.000918s | 0.000906s | 0.000896s | byte equality observed |
| mp_valid.test | 10 | 0.000873s | 0.000861s | 0.000868s | byte equality observed |
| mp_valid_and_true.test | 10 | 0.000887s | 0.000908s | 0.000899s | byte equality observed |
| mp_value.test | 10 | 0.000386s | 0.000388s | 0.000870s | byte equality observed |
| mp_with_index.test | 10 | 0.001219s | 0.001157s | 0.001122s | byte equality observed |
| mp_with_index_cx.test | 10 | 0.000375s | 0.000382s | 0.000864s | byte equality observed |
| mpl.test | 10 | 0.000893s | 0.000870s | 0.000898s | byte equality observed |
| mpl_list.test | 10 | 0.000879s | 0.000873s | 0.000914s | byte equality observed |
| mpl_tuple.test | 10 | 0.000877s | 0.000876s | 0.000900s | byte equality observed |
| mpl_version.test | 10 | 0.000878s | 0.000910s | 0.000882s | byte equality observed |
| set_version.test | 10 | 0.000877s | 0.000906s | 0.000896s | byte equality observed |
| tuple_apply.test | 10 | 0.000880s | 0.000905s | 0.000915s | byte equality observed |
| tuple_for_each.test | 10 | 0.000897s | 0.000889s | 0.000884s | byte equality observed |
| tuple_transform.test | 10 | 0.000905s | 0.000880s | 0.000867s | byte equality observed |
| tuple_transform_2.test | 10 | 0.000898s | 0.000896s | 0.000899s | byte equality observed |
| tuple_version.test | 10 | 0.000887s | 0.000867s | 0.000880s | byte equality observed |
| utility_version.test | 10 | 0.000890s | 0.000907s | 0.000893s | byte equality observed |
| version.test | 10 | 0.000871s | 0.000870s | 0.000897s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345325).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
