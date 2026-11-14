# libs/mp11/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| mp11.test | COMPILE | 0.028541 | 0.058807 | 0.089295 | — | — | — |
| complex_h.test | COMPILE | 0.119292 | 0.248265 | 0.341093 | — | — | — |
| mp_size.test | RUN | 0.139003 | 0.824827 | 0.484521 | 0.001141 | 0.001153 | 0.001216 |
| mp_size_2.test | RUN | 0.012020 | 0.029613 | 0.043856 | 0.000462 | 0.000484 | 0.001110 |
| mp_empty.test | RUN | 0.133245 | 0.521082 | 0.451737 | 0.001051 | 0.001202 | 0.001108 |
| mp_empty_2.test | RUN | 0.011677 | 0.029877 | 0.043244 | 0.000461 | 0.000492 | 0.001235 |
| mp_assign.test | RUN | 0.153324 | 1.104320 | 0.587484 | 0.001139 | 0.001118 | 0.001172 |
| mp_assign_2.test | RUN | 0.011404 | 0.029448 | 0.043103 | 0.000443 | 0.000492 | 0.001187 |
| mp_clear.test | RUN | 0.123204 | 0.467747 | 0.446446 | 0.001008 | 0.001356 | 0.001041 |
| mp_clear_2.test | RUN | 0.011337 | 0.028631 | 0.043043 | 0.000483 | 0.000496 | 0.001224 |
| mp_front.test | RUN | 0.128931 | 0.748685 | 0.516424 | 0.001027 | 0.001255 | 0.001212 |
| mp_front_2.test | RUN | 0.010988 | 0.028772 | 0.044855 | 0.000440 | 0.000473 | 0.001092 |
| mp_pop_front.test | RUN | 0.131350 | 0.792673 | 0.516989 | 0.001003 | 0.001207 | 0.001148 |
| mp_pop_front_2.test | RUN | 0.010982 | 0.028956 | 0.042087 | 0.000452 | 0.000487 | 0.001102 |
| mp_second.test | RUN | 0.134967 | 0.491534 | 0.478758 | 0.001067 | 0.001178 | 0.001129 |
| mp_second_2.test | RUN | 0.011572 | 0.029705 | 0.045259 | 0.000484 | 0.000490 | 0.001180 |
| mp_third.test | RUN | 0.125753 | 0.459483 | 0.455419 | 0.000996 | 0.001354 | 0.001256 |
| mp_third_2.test | RUN | 0.010930 | 0.029432 | 0.044970 | 0.000486 | 0.000477 | 0.001237 |
| mp_push_front.test | RUN | 0.180701 | 1.707298 | 0.847715 | 0.000998 | 0.001302 | 0.001054 |
| mp_push_front_2.test | RUN | 0.010865 | 0.027319 | 0.043493 | 0.000447 | 0.000492 | 0.001037 |
| mp_push_back.test | RUN | 0.183845 | 1.721126 | 0.850251 | 0.001049 | 0.001338 | 0.000842 |
| mp_push_back_2.test | RUN | 0.012460 | 0.030198 | 0.044253 | 0.000475 | 0.000495 | 0.001210 |
| mp_rename.test | RUN | 0.168410 | 1.577698 | 0.694434 | 0.001002 | 0.001220 | 0.001057 |
| mp_rename_2.test | RUN | 0.010987 | 0.030421 | 0.043724 | 0.000462 | 0.000476 | 0.001243 |
| mp_append.test | RUN | 0.172095 | 1.774409 | 0.790547 | 0.001057 | 0.001188 | 0.001143 |
| mp_append_2.test | RUN | 0.391241 | 3.993236 | 1.082800 | 0.001032 | 0.001269 | 0.001181 |
| mp_append_3.test | RUN | 0.011324 | 0.029695 | 0.042459 | 0.000501 | 0.000453 | 0.001126 |
| mp_append_sf.test | RUN | 0.125131 | 0.422259 | 0.417818 | 0.001114 | 0.001107 | 0.001146 |
| mp_replace_front.test | RUN | 0.153600 | 1.255685 | 0.635614 | 0.001070 | 0.001225 | 0.001286 |
| mp_replace_front_2.test | RUN | 0.011179 | 0.029319 | 0.044035 | 0.000421 | 0.000408 | 0.001145 |
| mp_replace_second.test | RUN | 0.151350 | 0.796005 | 0.595153 | 0.001245 | 0.001236 | 0.001224 |
| mp_replace_second_2.test | RUN | 0.011084 | 0.028250 | 0.042266 | 0.000479 | 0.000482 | 0.001071 |
| mp_replace_third.test | RUN | 0.135613 | 0.655102 | 0.543575 | 0.001031 | 0.000893 | 0.001347 |
| mp_replace_third_2.test | RUN | 0.011088 | 0.027825 | 0.042773 | 0.000469 | 0.000413 | 0.001247 |
| mp_apply_q.test | RUN | 0.158952 | 1.551457 | 0.714910 | 0.001062 | 0.001177 | 0.001113 |
| mp_apply_q_sf.test | RUN | 0.130211 | 0.431511 | 0.435353 | 0.001040 | 0.001379 | 0.001062 |
| mp_is_list.test | RUN | 0.133144 | 0.516883 | 0.454767 | 0.001088 | 0.001102 | 0.001128 |
| mp_list_c.test | RUN | 0.132880 | 0.629234 | 0.506818 | 0.001016 | 0.001282 | 0.001155 |
| mp_transform_front.test | RUN | 0.155137 | 1.252896 | 0.608838 | 0.000975 | 0.001346 | 0.001304 |
| mp_transform_front_2.test | RUN | 0.011226 | 0.029385 | 0.043689 | 0.000438 | 0.000493 | 0.001143 |
| mp_transform_second.test | RUN | 0.136022 | 1.070272 | 0.578396 | 0.001026 | 0.001148 | 0.001100 |
| mp_transform_second_2.test | RUN | 0.011301 | 0.029331 | 0.043098 | 0.000466 | 0.000453 | 0.001122 |
| mp_transform_third.test | RUN | 0.132776 | 0.810307 | 0.516859 | 0.001036 | 0.001275 | 0.001188 |
| mp_transform_third_2.test | RUN | 0.011251 | 0.029587 | 0.041872 | 0.000473 | 0.000488 | 0.001111 |
| mp_list_v.test | RUN | 0.011033 | 0.027818 | 0.041528 | 0.000485 | 0.000497 | 0.001065 |
| mp_rename_v.test | RUN | 0.011323 | 0.028538 | 0.042169 | 0.000454 | 0.000473 | 0.001087 |
| mp_is_value_list.test | RUN | 0.116216 | 0.316086 | 0.406549 | 0.001099 | 0.001272 | 0.001145 |
| mp_transform.test | RUN | 0.314846 | 2.685451 | 1.352878 | 0.001149 | 0.001170 | 0.001187 |
| mp_transform_2.test | RUN | 0.117350 | 0.280724 | 0.393240 | 0.001089 | 0.001122 | 0.001263 |
| mp_transform_q.test | RUN | 0.310257 | 2.528698 | 1.294174 | 0.001099 | 0.001242 | 0.001280 |
| mp_transform_q_2.test | RUN | 0.323409 | 0.791135 | 1.669915 | 0.001078 | 0.001116 | 0.001131 |
| mp_transform_sf.test | RUN | 0.136175 | 0.440040 | 0.463713 | 0.001091 | 0.001306 | 0.001194 |
| mp_transform_if.test | RUN | 0.160564 | 1.272826 | 0.693721 | 0.000996 | 0.001297 | 0.001214 |
| mp_transform_if_q.test | RUN | 0.167861 | 1.260040 | 0.707312 | 0.001059 | 0.000928 | 0.001018 |
| mp_filter.test | RUN | 0.143319 | 0.738625 | 0.511877 | 0.000975 | 0.001174 | 0.001101 |
| mp_fill.test | RUN | 0.138255 | 0.553574 | 0.493299 | 0.001020 | 0.001194 | 0.001109 |
| mp_fill_2.test | RUN | 0.021410 | 0.042026 | 0.068453 | 0.000484 | 0.000376 | 0.001155 |
| mp_count.test | RUN | 0.271435 | 1.325004 | 0.765552 | 0.001179 | 0.001342 | 0.001134 |
| mp_count_if.test | RUN | 0.278074 | 1.339652 | 0.839035 | 0.001012 | 0.001116 | 0.001190 |
| mp_count_if_q.test | RUN | 0.142307 | 0.897977 | 0.486321 | 0.001357 | 0.001262 | 0.001096 |
| mp_contains.test | RUN | 0.137870 | 0.535560 | 0.447851 | 0.001189 | 0.001271 | 0.001162 |
| mp_starts_with.test | RUN | 0.137699 | 0.531810 | 0.464083 | 0.001169 | 0.001068 | 0.001347 |
| mp_starts_with_sf.test | RUN | 0.132852 | 0.450220 | 0.460826 | 0.001077 | 0.001242 | 0.001288 |
| mp_repeat.test | RUN | 0.202966 | 2.235600 | 0.892460 | 0.000996 | 0.001230 | 0.001266 |
| mp_repeat_2.test | RUN | 0.020726 | 0.047446 | 0.069824 | 0.000448 | 0.000469 | 0.001370 |
| mp_repeat_3.test | RUN | 0.172633 | 0.280775 | 0.442059 | 0.001170 | 0.001143 | 0.001278 |
| mp_product.test | RUN | 0.155124 | 1.367309 | 0.676517 | 0.001066 | 0.001330 | 0.001224 |
| mp_drop.test | RUN | 0.174150 | 1.687557 | 0.763481 | 0.001033 | 0.001132 | 0.001191 |
| mp_drop_2.test | RUN | 0.020512 | 0.045755 | 0.068062 | 0.000454 | 0.000490 | 0.001246 |
| mp_iota.test | RUN | 0.164838 | 1.445216 | 0.674458 | 0.001259 | 0.001148 | 0.001170 |
| mp_iota_2.test | RUN | 0.182526 | 1.716668 | 0.772976 | 0.001001 | 0.001173 | 0.001104 |
| mp_at.test | RUN | 0.143366 | 0.738099 | 0.513282 | 0.001133 | 0.001270 | 0.001150 |
| mp_at_2.test | RUN | 0.020547 | 0.045103 | 0.070839 | 0.000450 | 0.000503 | 0.001106 |
| mp_at_sf.test | RUN | 0.137986 | 0.448599 | 0.452503 | 0.001155 | 0.001256 | 0.001272 |
| mp_take.test | RUN | 0.234409 | 2.209723 | 0.891569 | 0.001113 | 0.001153 | 0.001200 |
| mp_take_2.test | RUN | 0.021428 | 0.044234 | 0.070541 | 0.000488 | 0.000479 | 0.001115 |
| mp_slice.test | RUN | 0.280677 | 2.259620 | 1.159923 | 0.001080 | 0.001189 | 0.001098 |
| mp_slice_2.test | RUN | 0.019968 | 0.044065 | 0.069740 | 0.000451 | 0.000458 | 0.001143 |
| mp_replace.test | RUN | 0.170177 | 1.140662 | 0.725625 | 0.001048 | 0.001315 | 0.001194 |
| mp_replace_if.test | RUN | 0.165756 | 1.004503 | 0.676732 | 0.001075 | 0.001199 | 0.001081 |
| mp_replace_if_q.test | RUN | 0.162416 | 1.000515 | 0.671150 | 0.001003 | 0.001207 | 0.001234 |
| mp_copy_if.test | RUN | 0.161424 | 1.399669 | 0.644729 | 0.001007 | 0.001249 | 0.001163 |
| mp_copy_if_q.test | RUN | 0.168854 | 1.419474 | 0.641529 | 0.001002 | 0.001173 | 0.001107 |
| mp_remove.test | RUN | 0.157524 | 0.938136 | 0.666600 | 0.001038 | 0.001154 | 0.001133 |
| mp_remove_if.test | RUN | 0.169071 | 1.500386 | 0.686617 | 0.001181 | 0.001271 | 0.001170 |
| mp_remove_if_q.test | RUN | 0.168437 | 1.494974 | 0.687886 | 0.000835 | 0.001231 | 0.001176 |
| mp_partition.test | RUN | 0.164825 | 1.404366 | 0.730681 | 0.001026 | 0.001256 | 0.001115 |
| mp_partition_q.test | RUN | 0.169008 | 1.394011 | 0.743719 | 0.001134 | 0.001203 | 0.001201 |
| mp_sort.test | RUN | 0.161336 | 1.001733 | 0.641630 | 0.001060 | 0.001320 | 0.001238 |
| mp_sort_q.test | RUN | 0.157241 | 1.001595 | 0.642003 | 0.000937 | 0.001227 | 0.001109 |
| mp_find.test | RUN | 0.155014 | 1.075460 | 0.546699 | 0.001010 | 0.001265 | 0.000999 |
| mp_find_if.test | RUN | 0.146498 | 0.979628 | 0.527578 | 0.000983 | 0.001198 | 0.001152 |
| mp_find_if_q.test | RUN | 0.156936 | 1.101349 | 0.644235 | 0.001004 | 0.001324 | 0.001394 |
| mp_reverse.test | RUN | 0.351177 | 2.973135 | 1.279769 | 0.001178 | 0.001212 | 0.001397 |
| mp_fold.test | RUN | 0.607041 | 2.319184 | 3.264270 | 0.001300 | 0.001320 | 0.001289 |
| mp_fold_q.test | RUN | 0.153499 | 0.983037 | 0.613579 | 0.001216 | 0.001314 | 0.001327 |
| mp_fold_q_sf.test | RUN | 0.131613 | 0.452095 | 0.468310 | 0.000990 | 0.001208 | 0.001331 |
| mp_reverse_fold.test | RUN | 0.277056 | 2.222650 | 0.907423 | 0.001030 | 0.001322 | 0.001334 |
| mp_reverse_fold_q.test | RUN | 0.161497 | 1.220708 | 0.662738 | 0.001300 | 0.001240 | 0.001127 |
| mp_unique.test | RUN | 0.159904 | 1.097146 | 0.643651 | 0.001128 | 0.001322 | 0.001185 |
| mp_unique_if.test | RUN | 0.281223 | 2.327669 | 1.013301 | 0.001129 | 0.001341 | 0.001278 |
| mp_unique_if_q.test | RUN | 0.253396 | 2.261019 | 0.829624 | 0.001161 | 0.001291 | 0.001236 |
| mp_all_of.test | RUN | 0.377732 | 1.202612 | 1.061732 | 0.001002 | 0.001368 | 0.001189 |
| mp_all_of_q.test | RUN | 0.135909 | 0.527157 | 0.460531 | 0.001399 | 0.001212 | 0.001216 |
| mp_any_of.test | RUN | 0.387550 | 1.290209 | 1.046438 | 0.001274 | 0.001317 | 0.001265 |
| mp_any_of_q.test | RUN | 0.138985 | 0.521411 | 0.483942 | 0.001252 | 0.001286 | 0.001238 |
| mp_none_of.test | RUN | 0.378680 | 1.265723 | 1.060459 | 0.001055 | 0.001263 | 0.001225 |
| mp_none_of_q.test | RUN | 0.140091 | 0.514635 | 0.448369 | 0.001140 | 0.001252 | 0.001408 |
| mp_replace_at.test | RUN | 0.172751 | 1.114607 | 0.687828 | 0.001179 | 0.001269 | 0.001303 |
| mp_replace_at_c.test | RUN | 0.172321 | 1.108283 | 0.727351 | 0.001089 | 0.001300 | 0.001328 |
| mp_for_each.test | RUN | 0.154885 | 0.697825 | 0.714755 | 0.001072 | 0.001255 | 0.001076 |
| mp_insert.test | RUN | 0.265906 | 2.257640 | 1.067556 | 0.001095 | 0.001218 | 0.001187 |
| mp_insert_2.test | RUN | 0.021620 | 0.045588 | 0.072326 | 0.000436 | 0.000474 | 0.001051 |
| mp_erase.test | RUN | 0.284789 | 2.411851 | 1.231044 | 0.001032 | 0.001255 | 0.001075 |
| mp_erase_2.test | RUN | 0.020011 | 0.045707 | 0.068483 | 0.000497 | 0.000501 | 0.001319 |
| mp_with_index.test | RUN | 0.355857 | 4.819996 | 2.667300 | 0.001394 | 0.001401 | 0.001641 |
| mp_with_index_cx.test | RUN | 0.002318 | 0.014308 | 0.021984 | 0.000432 | 0.000459 | 0.001177 |
| mp_from_sequence.test | RUN | 0.166296 | 1.249739 | 0.690032 | 0.001183 | 0.001175 | 0.001252 |
| mp_from_sequence_2.test | RUN | 0.181170 | 1.490963 | 0.750911 | 0.001078 | 0.001363 | 0.001120 |
| mp_from_sequence_3.test | RUN | 0.145270 | 0.650757 | 0.514751 | 0.001013 | 0.001333 | 0.001090 |
| mp_min_element.test | RUN | 0.142226 | 0.580895 | 0.487359 | 0.001042 | 0.001196 | 0.001196 |
| mp_min_element_2.test | RUN | 0.020417 | 0.045992 | 0.071832 | 0.000382 | 0.000509 | 0.001044 |
| mp_min_element_q.test | RUN | 0.140816 | 0.585918 | 0.488032 | 0.001150 | 0.001077 | 0.001188 |
| mp_min_element_q_2.test | RUN | 0.020393 | 0.045074 | 0.070634 | 0.000382 | 0.000474 | 0.001190 |
| mp_max_element.test | RUN | 0.140720 | 0.583058 | 0.480020 | 0.001003 | 0.001228 | 0.001180 |
| mp_max_element_2.test | RUN | 0.020137 | 0.045575 | 0.072212 | 0.000451 | 0.000443 | 0.001145 |
| mp_max_element_q.test | RUN | 0.140365 | 0.586609 | 0.489797 | 0.001133 | 0.001255 | 0.001247 |
| mp_max_element_q_2.test | RUN | 0.019740 | 0.046463 | 0.070707 | 0.000472 | 0.000516 | 0.001168 |
| mp_nth_element.test | RUN | 0.156341 | 0.832393 | 0.608237 | 0.001114 | 0.001301 | 0.001159 |
| mp_nth_element_q.test | RUN | 0.154068 | 0.819345 | 0.609392 | 0.000964 | 0.001221 | 0.001198 |
| mp_back.test | RUN | 0.151122 | 0.676162 | 0.564464 | 0.000994 | 0.001168 | 0.001346 |
| mp_back_2.test | RUN | 0.020806 | 0.046620 | 0.074435 | 0.000402 | 0.000472 | 0.001113 |
| mp_pop_back.test | RUN | 0.445268 | 4.611214 | 1.235809 | 0.001082 | 0.001407 | 0.001257 |
| mp_pop_back_2.test | RUN | 0.020214 | 0.046030 | 0.071313 | 0.000494 | 0.000442 | 0.001336 |
| mp_flatten.test | RUN | 0.183061 | 1.623055 | 0.808811 | 0.001148 | 0.001220 | 0.001234 |
| mp_rotate_left.test | RUN | 0.183132 | 1.744010 | 0.795018 | 0.001056 | 0.001260 | 0.001269 |
| mp_rotate_right.test | RUN | 0.188090 | 1.744220 | 0.798730 | 0.001067 | 0.001210 | 0.001218 |
| mp_power_set.test | RUN | 0.174596 | 1.671145 | 0.809165 | 0.001063 | 0.001332 | 0.001164 |
| mp_partial_sum.test | RUN | 0.238069 | 2.247359 | 0.921965 | 0.001157 | 0.001252 | 0.001118 |
| mp_iterate.test | RUN | 0.157637 | 0.659355 | 0.627319 | 0.001043 | 0.001251 | 0.001218 |
| mp_pairwise_fold.test | RUN | 0.183280 | 1.540684 | 0.836566 | 0.001014 | 0.001304 | 0.001169 |
| mp_pairwise_fold_q.test | RUN | 0.133082 | 0.436229 | 0.461856 | 0.001037 | 0.001403 | 0.001199 |
| mp_sliding_fold.test | RUN | 0.353173 | 2.979331 | 1.554074 | 0.001205 | 0.001211 | 0.001292 |
| mp_sliding_fold_q.test | RUN | 0.355192 | 2.986791 | 1.542000 | 0.001058 | 0.001297 | 0.001229 |
| mp_intersperse.test | RUN | 0.163983 | 1.103605 | 0.713918 | 0.001183 | 0.001239 | 0.001123 |
| mp_split.test | RUN | 0.252847 | 2.123218 | 0.994904 | 0.001011 | 0.001238 | 0.001302 |
| mp_join.test | RUN | 0.188020 | 1.783155 | 0.911832 | 0.001143 | 0.001283 | 0.001148 |
| integral.test | RUN | 0.142620 | 0.787663 | 0.543641 | 0.001030 | 0.001185 | 0.001193 |
| mp_value.test | RUN | 0.009084 | 0.022937 | 0.036452 | 0.000466 | 0.000414 | 0.001157 |
| mp_identity.test | RUN | 0.127443 | 0.718020 | 0.486608 | 0.001059 | 0.001250 | 0.001115 |
| mp_inherit.test | RUN | 0.133949 | 0.506004 | 0.469159 | 0.001100 | 0.001355 | 0.001229 |
| mp_if.test | RUN | 0.133699 | 0.570565 | 0.467682 | 0.001067 | 0.001170 | 0.001180 |
| mp_if_sf.test | RUN | 0.122856 | 0.427082 | 0.424611 | 0.000999 | 0.001152 | 0.001129 |
| mp_eval_if.test | RUN | 0.132796 | 0.622947 | 0.476241 | 0.001064 | 0.001312 | 0.001183 |
| mp_eval_if_sf.test | RUN | 0.129523 | 0.437651 | 0.424938 | 0.001332 | 0.001241 | 0.001127 |
| mp_valid.test | RUN | 0.129731 | 0.436300 | 0.432764 | 0.001125 | 0.001149 | 0.001158 |
| mp_defer.test | RUN | 0.140889 | 0.749465 | 0.536921 | 0.001015 | 0.001174 | 0.001268 |
| mp_quote.test | RUN | 0.144845 | 0.749924 | 0.552817 | 0.001050 | 0.001335 | 0.001175 |
| mp_invoke_q.test | RUN | 0.151425 | 1.001911 | 0.596105 | 0.001182 | 0.001243 | 0.001181 |
| mp_invoke_q_sf.test | RUN | 0.122423 | 0.437453 | 0.425442 | 0.001141 | 0.001275 | 0.001085 |
| mp_quote_trait.test | RUN | 0.134878 | 0.834536 | 0.514978 | 0.001078 | 0.001319 | 0.001292 |
| mp_cond.test | RUN | 0.131953 | 0.497464 | 0.455145 | 0.001042 | 0.001174 | 0.001147 |
| mp_cond_sf.test | RUN | 0.123909 | 0.428678 | 0.436598 | 0.001067 | 0.001212 | 0.001127 |
| mp_not_fn.test | RUN | 0.124026 | 0.434089 | 0.424484 | 0.001099 | 0.001300 | 0.001104 |
| mp_eval_if_not.test | RUN | 0.137511 | 0.623518 | 0.483336 | 0.001105 | 0.001083 | 0.001155 |
| mp_eval_or.test | RUN | 0.130887 | 0.323219 | 0.436536 | 0.001041 | 0.001222 | 0.001178 |
| mp_compose.test | RUN | 0.147010 | 1.035332 | 0.584473 | 0.001106 | 0.001149 | 0.001203 |
| mp_valid_and_true.test | RUN | 0.127280 | 0.523547 | 0.450446 | 0.001027 | 0.001367 | 0.001203 |
| mp_compose_sf.test | COMPILE | 0.030057 | 0.058532 | 0.102714 | — | — | — |
| integer_sequence.test | RUN | 0.162062 | 1.019601 | 0.650400 | 0.001059 | 0.001263 | 0.001168 |
| tuple_for_each.test | RUN | 0.126946 | 0.367640 | 0.471393 | 0.001097 | 0.001137 | 0.001134 |
| tuple_for_each_cx.test | COMPILE | 0.023603 | 0.050600 | 0.081373 | — | — | — |
| tuple_apply.test | RUN | 0.131961 | 0.362826 | 0.479669 | 0.001137 | 0.001203 | 0.001214 |
| tuple_apply_cx.test | COMPILE | 0.034736 | 0.074649 | 0.112351 | — | — | — |
| construct_from_tuple.test | RUN | 0.134365 | 0.392467 | 0.465940 | 0.001137 | 0.001225 | 0.001238 |
| construct_from_tuple_cx.test | COMPILE | 0.033411 | 0.075345 | 0.106017 | — | — | — |
| tuple_transform.test | RUN | 0.206616 | 0.798022 | 0.915808 | 0.000991 | 0.001171 | 0.001122 |
| tuple_transform_2.test | RUN | 0.183969 | 0.573250 | 0.816623 | 0.001103 | 0.001193 | 0.001111 |
| tuple_transform_cx.test | COMPILE | 0.031330 | 0.071464 | 0.107949 | — | — | — |
| mp_set_contains.test | RUN | 0.136800 | 0.535902 | 0.468307 | 0.001214 | 0.001260 | 0.001247 |
| mp_set_push_back.test | RUN | 0.181941 | 1.661511 | 0.819059 | 0.001038 | 0.001266 | 0.001168 |
| mp_set_push_front.test | RUN | 0.237327 | 1.988920 | 0.934369 | 0.001031 | 0.001309 | 0.001074 |
| mp_is_set.test | RUN | 0.141267 | 0.529487 | 0.465876 | 0.001077 | 0.001292 | 0.001204 |
| mp_set_union.test | RUN | 0.174129 | 1.419172 | 0.769800 | 0.001172 | 0.001297 | 0.001100 |
| mp_set_union_sf.test | RUN | 0.126843 | 0.421361 | 0.424114 | 0.001045 | 0.001175 | 0.001145 |
| mp_set_difference.test | RUN | 0.156018 | 1.105156 | 0.656952 | 0.001141 | 0.001173 | 0.001297 |
| mp_set_difference_sf.test | RUN | 0.131276 | 0.423227 | 0.442528 | 0.001033 | 0.001214 | 0.001214 |
| mp_set_intersection.test | RUN | 0.163070 | 1.088342 | 0.639527 | 0.001106 | 0.001191 | 0.001311 |
| mp_set_intersection_sf.test | RUN | 0.124976 | 0.422729 | 0.438343 | 0.001111 | 0.001193 | 0.001221 |
| mp_all.test | RUN | 0.131875 | 0.522772 | 0.470298 | 0.001114 | 0.001173 | 0.001175 |
| mp_all_2.test | RUN | 0.388168 | 1.143155 | 1.057895 | 0.001048 | 0.001160 | 0.001029 |
| mp_and.test | RUN | 0.137961 | 0.522718 | 0.460145 | 0.001115 | 0.001302 | 0.001215 |
| mp_any.test | RUN | 0.131712 | 0.530681 | 0.480460 | 0.001042 | 0.001212 | 0.001273 |
| mp_any_2.test | RUN | 0.385883 | 1.116766 | 1.044213 | 0.001174 | 0.001264 | 0.001262 |
| mp_or.test | RUN | 0.135750 | 0.524026 | 0.456455 | 0.001053 | 0.001273 | 0.001104 |
| mp_same.test | RUN | 0.265311 | 0.879439 | 0.699660 | 0.001158 | 0.001297 | 0.001385 |
| mp_plus.test | RUN | 0.138387 | 0.633074 | 0.544456 | 0.001105 | 0.001245 | 0.001246 |
| mp_less.test | RUN | 0.132768 | 0.526399 | 0.475385 | 0.001069 | 0.001144 | 0.001252 |
| mp_min.test | RUN | 0.129338 | 0.473252 | 0.457413 | 0.001086 | 0.001339 | 0.001234 |
| mp_max.test | RUN | 0.130092 | 0.537741 | 0.471290 | 0.001024 | 0.001183 | 0.001186 |
| mp_similar.test | RUN | 0.128050 | 0.526536 | 0.466179 | 0.001021 | 0.001226 | 0.001290 |
| mp_map_find.test | RUN | 0.157891 | 0.978014 | 0.623351 | 0.001260 | 0.001332 | 0.001251 |
| mp_map_find_2.test | RUN | 0.143731 | 0.368116 | 0.530494 | 0.001104 | 0.001078 | 0.001297 |
| mp_map_find_3.test | RUN | 0.147778 | 0.369935 | 0.529740 | 0.001165 | 0.001233 | 0.001140 |
| mp_map_find_4.test | RUN | 0.136360 | 0.347028 | 0.481605 | 0.001161 | 0.001133 | 0.001178 |
| mp_map_contains.test | RUN | 0.126780 | 0.531212 | 0.483926 | 0.001162 | 0.001278 | 0.001239 |
| mp_map_insert.test | RUN | 0.174215 | 1.882733 | 0.887468 | 0.001063 | 0.001224 | 0.001210 |
| mp_map_replace.test | RUN | 0.247302 | 2.111663 | 1.013857 | 0.001054 | 0.001268 | 0.001065 |
| mp_map_erase.test | RUN | 0.183390 | 1.578036 | 0.841871 | 0.001164 | 0.001241 | 0.001101 |
| mp_map_update.test | RUN | 0.160548 | 1.024993 | 0.654694 | 0.001078 | 0.001323 | 0.000976 |
| mp_map_update_q.test | RUN | 0.158992 | 1.018935 | 0.643818 | 0.001069 | 0.001212 | 0.001246 |
| mp_map_keys.test | RUN | 0.153004 | 0.898400 | 0.623571 | 0.001054 | 0.001200 | 0.001104 |
| mp_is_map.test | RUN | 0.144662 | 0.546429 | 0.498089 | 0.001086 | 0.001190 | 0.001185 |
| mp_bind.test | RUN | 0.182721 | 1.639362 | 0.745995 | 0.001031 | 0.001165 | 0.001217 |
| mp_bind_q.test | RUN | 0.158432 | 1.022070 | 0.615065 | 0.001086 | 0.001318 | 0.001231 |
| mp_bind_front.test | RUN | 0.153639 | 1.114936 | 0.592824 | 0.001052 | 0.001511 | 0.001070 |
| mp_bind_back.test | RUN | 0.150155 | 1.096715 | 0.585984 | 0.000998 | 0.001198 | 0.001131 |
| mp_lambda.test | RUN | 0.357197 | 3.346535 | 1.596528 | 0.001228 | 0.000926 | 0.001106 |
| mpl.test | RUN | 0.312816 | 0.859031 | 1.239000 | 0.001017 | 0.001209 | 0.001089 |
| mpl_list.test | RUN | 0.172432 | 0.647411 | 0.582278 | 0.001265 | 0.001123 | 0.001101 |
| mpl_tuple.test | RUN | 0.172820 | 0.652397 | 0.567989 | 0.000882 | 0.001155 | 0.001102 |
| version.test | RUN | 0.081320 | 0.229796 | 0.292694 | 0.001050 | 0.001300 | 0.001012 |
| mp11_version.test | RUN | 0.105187 | 0.268858 | 0.334198 | 0.001114 | 0.001195 | 0.001115 |
| list_version.test | RUN | 0.088229 | 0.236665 | 0.291964 | 0.001043 | 0.001222 | 0.001134 |
| algorithm_version.test | RUN | 0.095882 | 0.250550 | 0.315649 | 0.000743 | 0.001103 | 0.001216 |
| integral_version.test | RUN | 0.078008 | 0.233334 | 0.286932 | 0.001055 | 0.001140 | 0.001133 |
| utility_version.test | RUN | 0.083730 | 0.231619 | 0.287479 | 0.001073 | 0.001184 | 0.001005 |
| function_version.test | RUN | 0.090087 | 0.232386 | 0.304444 | 0.001048 | 0.001164 | 0.001115 |
| map_version.test | RUN | 0.097804 | 0.248474 | 0.318173 | 0.001031 | 0.001292 | 0.001026 |
| set_version.test | RUN | 0.089260 | 0.237280 | 0.297137 | 0.001014 | 0.001181 | 0.001124 |
| bind_version.test | RUN | 0.096308 | 0.250890 | 0.315380 | 0.001158 | 0.001176 | 0.001107 |
| integer_sequence_version.test | RUN | 0.081935 | 0.229510 | 0.288745 | 0.001033 | 0.001417 | 0.000817 |
| tuple_version.test | RUN | 0.093703 | 0.249138 | 0.206259 | 0.000778 | 0.000898 | 0.001254 |
| mpl_version.test | RUN | 0.085668 | 0.196569 | 0.298256 | 0.000891 | 0.000886 | 0.000883 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.007× | 1.006×, 1.009× | 0.995× |
| clang | 1.090× | 1.090×, 1.090× | 1.003× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814672).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| algorithm_version.test | 10 | 0.000733s | 0.000738s | 0.000746s | byte equality observed |
| bind_version.test | 10 | 0.000729s | 0.000753s | 0.000747s | byte equality observed |
| construct_from_tuple.test | 10 | 0.000737s | 0.000752s | 0.000736s | byte equality observed |
| function_version.test | 10 | 0.000740s | 0.000758s | 0.000736s | byte equality observed |
| integer_sequence.test | 10 | 0.000744s | 0.000748s | 0.000737s | byte equality observed |
| integer_sequence_version.test | 10 | 0.000751s | 0.000756s | 0.000727s | byte equality observed |
| integral.test | 10 | 0.000749s | 0.000741s | 0.000719s | byte equality observed |
| integral_version.test | 10 | 0.000729s | 0.000745s | 0.000715s | byte equality observed |
| list_version.test | 10 | 0.000735s | 0.000759s | 0.000723s | byte equality observed |
| map_version.test | 10 | 0.000730s | 0.000739s | 0.000726s | byte equality observed |
| mp11_version.test | 10 | 0.000747s | 0.000745s | 0.000795s | byte equality observed |
| mp_all.test | 10 | 0.000739s | 0.000744s | 0.000800s | byte equality observed |
| mp_all_2.test | 10 | 0.000742s | 0.000753s | 0.000795s | byte equality observed |
| mp_all_of.test | 10 | 0.000732s | 0.000746s | 0.000822s | byte equality observed |
| mp_all_of_q.test | 10 | 0.000751s | 0.000752s | 0.000825s | byte equality observed |
| mp_and.test | 10 | 0.000746s | 0.000757s | 0.000816s | byte equality observed |
| mp_any.test | 10 | 0.000730s | 0.000744s | 0.000842s | byte equality observed |
| mp_any_2.test | 10 | 0.000742s | 0.000728s | 0.000805s | byte equality observed |
| mp_any_of.test | 10 | 0.000727s | 0.000727s | 0.000813s | byte equality observed |
| mp_any_of_q.test | 10 | 0.000753s | 0.000744s | 0.000816s | byte equality observed |
| mp_append.test | 10 | 0.000752s | 0.000745s | 0.000821s | byte equality observed |
| mp_append_2.test | 10 | 0.000762s | 0.000745s | 0.000809s | byte equality observed |
| mp_append_3.test | 10 | 0.000344s | 0.000345s | 0.000797s | byte equality observed |
| mp_append_sf.test | 10 | 0.000748s | 0.000745s | 0.000796s | byte equality observed |
| mp_apply_q.test | 10 | 0.000749s | 0.000744s | 0.000811s | byte equality observed |
| mp_apply_q_sf.test | 10 | 0.000748s | 0.000730s | 0.000782s | byte equality observed |
| mp_assign.test | 10 | 0.000752s | 0.000738s | 0.000779s | byte equality observed |
| mp_assign_2.test | 10 | 0.000347s | 0.000338s | 0.000795s | byte equality observed |
| mp_at.test | 10 | 0.000758s | 0.000732s | 0.000801s | byte equality observed |
| mp_at_2.test | 10 | 0.000360s | 0.000346s | 0.000766s | byte equality observed |
| mp_at_sf.test | 10 | 0.000798s | 0.000756s | 0.000806s | byte equality observed |
| mp_back.test | 10 | 0.000784s | 0.000788s | 0.000784s | byte equality observed |
| mp_back_2.test | 10 | 0.000362s | 0.000354s | 0.000771s | byte equality observed |
| mp_bind.test | 10 | 0.000794s | 0.000848s | 0.000780s | byte equality observed |
| mp_bind_back.test | 10 | 0.000794s | 0.000837s | 0.000780s | byte equality observed |
| mp_bind_front.test | 10 | 0.000811s | 0.000814s | 0.000787s | byte equality observed |
| mp_bind_q.test | 10 | 0.000826s | 0.000820s | 0.000803s | byte equality observed |
| mp_clear.test | 10 | 0.000803s | 0.000826s | 0.000830s | byte equality observed |
| mp_clear_2.test | 10 | 0.000369s | 0.000363s | 0.000770s | byte equality observed |
| mp_compose.test | 10 | 0.000800s | 0.000808s | 0.000788s | byte equality observed |
| mp_cond.test | 10 | 0.000811s | 0.000850s | 0.000796s | byte equality observed |
| mp_cond_sf.test | 10 | 0.000802s | 0.000799s | 0.000791s | byte equality observed |
| mp_contains.test | 10 | 0.000778s | 0.000817s | 0.000778s | byte equality observed |
| mp_copy_if.test | 10 | 0.000800s | 0.000815s | 0.000820s | byte equality observed |
| mp_copy_if_q.test | 10 | 0.000796s | 0.000823s | 0.000828s | byte equality observed |
| mp_count.test | 10 | 0.000809s | 0.000830s | 0.000802s | byte equality observed |
| mp_count_if.test | 10 | 0.000805s | 0.000816s | 0.000806s | byte equality observed |
| mp_count_if_q.test | 10 | 0.000820s | 0.000836s | 0.000820s | byte equality observed |
| mp_defer.test | 10 | 0.000821s | 0.000841s | 0.000816s | byte equality observed |
| mp_drop.test | 10 | 0.000808s | 0.000822s | 0.000796s | byte equality observed |
| mp_drop_2.test | 10 | 0.000362s | 0.000361s | 0.000774s | byte equality observed |
| mp_empty.test | 10 | 0.000801s | 0.000797s | 0.000795s | byte equality observed |
| mp_empty_2.test | 10 | 0.000360s | 0.000365s | 0.000805s | byte equality observed |
| mp_erase.test | 10 | 0.000829s | 0.000843s | 0.000785s | byte equality observed |
| mp_erase_2.test | 10 | 0.000363s | 0.000365s | 0.000781s | byte equality observed |
| mp_eval_if.test | 10 | 0.000799s | 0.000836s | 0.000783s | byte equality observed |
| mp_eval_if_not.test | 10 | 0.000803s | 0.000818s | 0.000814s | byte equality observed |
| mp_eval_if_sf.test | 10 | 0.000798s | 0.000799s | 0.000783s | byte equality observed |
| mp_eval_or.test | 10 | 0.000814s | 0.000799s | 0.000795s | byte equality observed |
| mp_fill.test | 10 | 0.000811s | 0.000812s | 0.000843s | byte equality observed |
| mp_fill_2.test | 10 | 0.000369s | 0.000363s | 0.000806s | byte equality observed |
| mp_filter.test | 10 | 0.000802s | 0.000809s | 0.000811s | byte equality observed |
| mp_find.test | 10 | 0.000838s | 0.000822s | 0.000809s | byte equality observed |
| mp_find_if.test | 10 | 0.000825s | 0.000820s | 0.000810s | byte equality observed |
| mp_find_if_q.test | 10 | 0.000802s | 0.000786s | 0.000824s | byte equality observed |
| mp_flatten.test | 10 | 0.000819s | 0.000815s | 0.000805s | byte equality observed |
| mp_fold.test | 10 | 0.000826s | 0.000828s | 0.000818s | byte equality observed |
| mp_fold_q.test | 10 | 0.000807s | 0.000834s | 0.000787s | byte equality observed |
| mp_fold_q_sf.test | 10 | 0.000810s | 0.000839s | 0.000821s | byte equality observed |
| mp_for_each.test | 10 | 0.000804s | 0.000829s | 0.000834s | byte equality observed |
| mp_from_sequence.test | 10 | 0.000795s | 0.000841s | 0.000823s | byte equality observed |
| mp_from_sequence_2.test | 10 | 0.000806s | 0.000830s | 0.000803s | byte equality observed |
| mp_from_sequence_3.test | 10 | 0.000802s | 0.000822s | 0.000835s | byte equality observed |
| mp_front.test | 10 | 0.000813s | 0.000809s | 0.000837s | byte equality observed |
| mp_front_2.test | 10 | 0.000366s | 0.000356s | 0.000766s | byte equality observed |
| mp_identity.test | 10 | 0.000816s | 0.000797s | 0.000800s | byte equality observed |
| mp_if.test | 10 | 0.000800s | 0.000815s | 0.000793s | byte equality observed |
| mp_if_sf.test | 10 | 0.000801s | 0.000792s | 0.000782s | byte equality observed |
| mp_inherit.test | 10 | 0.000821s | 0.000835s | 0.000796s | byte equality observed |
| mp_insert.test | 10 | 0.000807s | 0.000825s | 0.000799s | byte equality observed |
| mp_insert_2.test | 10 | 0.000361s | 0.000369s | 0.000778s | byte equality observed |
| mp_intersperse.test | 10 | 0.000793s | 0.000819s | 0.000787s | byte equality observed |
| mp_invoke_q.test | 10 | 0.000818s | 0.000813s | 0.000819s | byte equality observed |
| mp_invoke_q_sf.test | 10 | 0.000804s | 0.000813s | 0.000825s | byte equality observed |
| mp_iota.test | 10 | 0.000810s | 0.000808s | 0.000820s | byte equality observed |
| mp_iota_2.test | 10 | 0.000822s | 0.000827s | 0.000806s | byte equality observed |
| mp_is_list.test | 10 | 0.000819s | 0.000820s | 0.000798s | byte equality observed |
| mp_is_map.test | 10 | 0.000822s | 0.000847s | 0.000790s | byte equality observed |
| mp_is_set.test | 10 | 0.000795s | 0.000828s | 0.000803s | byte equality observed |
| mp_is_value_list.test | 10 | 0.000804s | 0.000783s | 0.000816s | byte equality observed |
| mp_iterate.test | 10 | 0.000802s | 0.000826s | 0.000834s | byte equality observed |
| mp_join.test | 10 | 0.000829s | 0.000841s | 0.000825s | byte equality observed |
| mp_lambda.test | 10 | 0.000824s | 0.000841s | 0.000810s | byte equality observed |
| mp_less.test | 10 | 0.000815s | 0.000812s | 0.000799s | byte equality observed |
| mp_list_c.test | 10 | 0.000808s | 0.000817s | 0.000779s | byte equality observed |
| mp_list_v.test | 10 | 0.000364s | 0.000364s | 0.000791s | byte equality observed |
| mp_map_contains.test | 10 | 0.000795s | 0.000801s | 0.000789s | byte equality observed |
| mp_map_erase.test | 10 | 0.000829s | 0.000832s | 0.000785s | byte equality observed |
| mp_map_find.test | 10 | 0.000812s | 0.000826s | 0.000789s | byte equality observed |
| mp_map_find_2.test | 10 | 0.000793s | 0.000796s | 0.000792s | byte equality observed |
| mp_map_find_3.test | 10 | 0.000797s | 0.000787s | 0.000797s | byte equality observed |
| mp_map_find_4.test | 10 | 0.000813s | 0.000808s | 0.000777s | byte equality observed |
| mp_map_insert.test | 10 | 0.000790s | 0.000814s | 0.000799s | byte equality observed |
| mp_map_keys.test | 10 | 0.000828s | 0.000809s | 0.000794s | byte equality observed |
| mp_map_replace.test | 10 | 0.000800s | 0.000842s | 0.000814s | byte equality observed |
| mp_map_update.test | 10 | 0.000802s | 0.000842s | 0.000798s | byte equality observed |
| mp_map_update_q.test | 10 | 0.000799s | 0.000807s | 0.000798s | byte equality observed |
| mp_max.test | 10 | 0.000828s | 0.000824s | 0.000781s | byte equality observed |
| mp_max_element.test | 10 | 0.000804s | 0.000822s | 0.000819s | byte equality observed |
| mp_max_element_2.test | 10 | 0.000362s | 0.000367s | 0.000817s | byte equality observed |
| mp_max_element_q.test | 10 | 0.000793s | 0.000799s | 0.000786s | byte equality observed |
| mp_max_element_q_2.test | 10 | 0.000366s | 0.000358s | 0.000791s | byte equality observed |
| mp_min.test | 10 | 0.000810s | 0.000809s | 0.000795s | byte equality observed |
| mp_min_element.test | 10 | 0.000806s | 0.000816s | 0.000815s | byte equality observed |
| mp_min_element_2.test | 10 | 0.000366s | 0.000369s | 0.000767s | byte equality observed |
| mp_min_element_q.test | 10 | 0.000794s | 0.000806s | 0.000792s | byte equality observed |
| mp_min_element_q_2.test | 10 | 0.000359s | 0.000359s | 0.000768s | byte equality observed |
| mp_none_of.test | 10 | 0.000821s | 0.000801s | 0.000801s | byte equality observed |
| mp_none_of_q.test | 10 | 0.000819s | 0.000798s | 0.000787s | byte equality observed |
| mp_not_fn.test | 10 | 0.000799s | 0.000831s | 0.000788s | byte equality observed |
| mp_nth_element.test | 10 | 0.000798s | 0.000813s | 0.000795s | byte equality observed |
| mp_nth_element_q.test | 10 | 0.000805s | 0.000803s | 0.000784s | byte equality observed |
| mp_or.test | 10 | 0.000790s | 0.000803s | 0.000814s | byte equality observed |
| mp_pairwise_fold.test | 10 | 0.000810s | 0.000814s | 0.000785s | byte equality observed |
| mp_pairwise_fold_q.test | 10 | 0.000824s | 0.000811s | 0.000799s | byte equality observed |
| mp_partial_sum.test | 10 | 0.000827s | 0.000804s | 0.000800s | byte equality observed |
| mp_partition.test | 10 | 0.000811s | 0.000798s | 0.000806s | byte equality observed |
| mp_partition_q.test | 10 | 0.000808s | 0.000829s | 0.000805s | byte equality observed |
| mp_plus.test | 10 | 0.000801s | 0.000836s | 0.000801s | byte equality observed |
| mp_pop_back.test | 10 | 0.000816s | 0.000820s | 0.000834s | byte equality observed |
| mp_pop_back_2.test | 10 | 0.000364s | 0.000365s | 0.000777s | byte equality observed |
| mp_pop_front.test | 10 | 0.000805s | 0.000790s | 0.000786s | byte equality observed |
| mp_pop_front_2.test | 10 | 0.000361s | 0.000364s | 0.000813s | byte equality observed |
| mp_power_set.test | 10 | 0.000802s | 0.000833s | 0.000816s | byte equality observed |
| mp_product.test | 10 | 0.000812s | 0.000800s | 0.000820s | byte equality observed |
| mp_push_back.test | 10 | 0.000806s | 0.000811s | 0.000790s | byte equality observed |
| mp_push_back_2.test | 10 | 0.000358s | 0.000358s | 0.000790s | byte equality observed |
| mp_push_front.test | 10 | 0.000786s | 0.000807s | 0.000790s | byte equality observed |
| mp_push_front_2.test | 10 | 0.000366s | 0.000361s | 0.000777s | byte equality observed |
| mp_quote.test | 10 | 0.000817s | 0.000841s | 0.000788s | byte equality observed |
| mp_quote_trait.test | 10 | 0.000817s | 0.000845s | 0.000800s | byte equality observed |
| mp_remove.test | 10 | 0.000813s | 0.000849s | 0.000811s | byte equality observed |
| mp_remove_if.test | 10 | 0.000797s | 0.000840s | 0.000807s | byte equality observed |
| mp_remove_if_q.test | 10 | 0.000804s | 0.000815s | 0.000800s | byte equality observed |
| mp_rename.test | 10 | 0.000803s | 0.000820s | 0.000786s | byte equality observed |
| mp_rename_2.test | 10 | 0.000375s | 0.000364s | 0.000753s | byte equality observed |
| mp_rename_v.test | 10 | 0.000359s | 0.000355s | 0.000760s | byte equality observed |
| mp_repeat.test | 10 | 0.000814s | 0.000806s | 0.000797s | byte equality observed |
| mp_repeat_2.test | 10 | 0.000366s | 0.000360s | 0.000767s | byte equality observed |
| mp_repeat_3.test | 10 | 0.000819s | 0.000823s | 0.000815s | byte equality observed |
| mp_replace.test | 10 | 0.000799s | 0.000840s | 0.000831s | byte equality observed |
| mp_replace_at.test | 10 | 0.000816s | 0.000790s | 0.000841s | byte equality observed |
| mp_replace_at_c.test | 10 | 0.000808s | 0.000809s | 0.000816s | byte equality observed |
| mp_replace_front.test | 10 | 0.000806s | 0.000794s | 0.000816s | byte equality observed |
| mp_replace_front_2.test | 10 | 0.000363s | 0.000359s | 0.000777s | byte equality observed |
| mp_replace_if.test | 10 | 0.000807s | 0.000805s | 0.000815s | byte equality observed |
| mp_replace_if_q.test | 10 | 0.000796s | 0.000831s | 0.000825s | byte equality observed |
| mp_replace_second.test | 10 | 0.000790s | 0.000811s | 0.000787s | byte equality observed |
| mp_replace_second_2.test | 10 | 0.000365s | 0.000361s | 0.000770s | byte equality observed |
| mp_replace_third.test | 6 | 0.000818s | 0.000799s | 0.000788s | byte equality observed |
| mp_replace_third_2.test | 10 | 0.000359s | 0.000368s | 0.000789s | byte equality observed |
| mp_reverse.test | 10 | 0.000833s | 0.000805s | 0.000810s | byte equality observed |
| mp_reverse_fold.test | 10 | 0.000840s | 0.000806s | 0.000803s | byte equality observed |
| mp_reverse_fold_q.test | 10 | 0.000799s | 0.000806s | 0.000824s | byte equality observed |
| mp_rotate_left.test | 10 | 0.000794s | 0.000824s | 0.000828s | byte equality observed |
| mp_rotate_right.test | 10 | 0.000795s | 0.000823s | 0.000826s | byte equality observed |
| mp_same.test | 10 | 0.000823s | 0.000831s | 0.000798s | byte equality observed |
| mp_second.test | 10 | 0.000812s | 0.000816s | 0.000796s | byte equality observed |
| mp_second_2.test | 10 | 0.000362s | 0.000365s | 0.000763s | byte equality observed |
| mp_set_contains.test | 10 | 0.000803s | 0.000813s | 0.000800s | byte equality observed |
| mp_set_difference.test | 10 | 0.000823s | 0.000806s | 0.000809s | byte equality observed |
| mp_set_difference_sf.test | 10 | 0.000818s | 0.000789s | 0.000828s | byte equality observed |
| mp_set_intersection.test | 10 | 0.000823s | 0.000805s | 0.000831s | byte equality observed |
| mp_set_intersection_sf.test | 10 | 0.000807s | 0.000831s | 0.000780s | byte equality observed |
| mp_set_push_back.test | 10 | 0.000804s | 0.000808s | 0.000817s | byte equality observed |
| mp_set_push_front.test | 10 | 0.000817s | 0.000848s | 0.000793s | byte equality observed |
| mp_set_union.test | 10 | 0.000803s | 0.000821s | 0.000820s | byte equality observed |
| mp_set_union_sf.test | 10 | 0.000805s | 0.000821s | 0.000799s | byte equality observed |
| mp_similar.test | 10 | 0.000817s | 0.000827s | 0.000796s | byte equality observed |
| mp_size.test | 10 | 0.000813s | 0.000824s | 0.000791s | byte equality observed |
| mp_size_2.test | 10 | 0.000364s | 0.000370s | 0.000783s | byte equality observed |
| mp_slice.test | 10 | 0.000817s | 0.000818s | 0.000804s | byte equality observed |
| mp_slice_2.test | 10 | 0.000363s | 0.000360s | 0.000799s | byte equality observed |
| mp_sliding_fold.test | 10 | 0.000821s | 0.000826s | 0.000809s | byte equality observed |
| mp_sliding_fold_q.test | 10 | 0.000815s | 0.000814s | 0.000780s | byte equality observed |
| mp_sort.test | 10 | 0.000804s | 0.000814s | 0.000796s | byte equality observed |
| mp_sort_q.test | 10 | 0.000813s | 0.000815s | 0.000827s | byte equality observed |
| mp_split.test | 10 | 0.000823s | 0.000826s | 0.000830s | byte equality observed |
| mp_starts_with.test | 10 | 0.000809s | 0.000819s | 0.000813s | byte equality observed |
| mp_starts_with_sf.test | 10 | 0.000825s | 0.000818s | 0.000814s | byte equality observed |
| mp_take.test | 10 | 0.000829s | 0.000822s | 0.000815s | byte equality observed |
| mp_take_2.test | 10 | 0.000355s | 0.000362s | 0.000819s | byte equality observed |
| mp_third.test | 10 | 0.000790s | 0.000825s | 0.000786s | byte equality observed |
| mp_third_2.test | 10 | 0.000360s | 0.000367s | 0.000795s | byte equality observed |
| mp_transform.test | 10 | 0.000815s | 0.000812s | 0.000791s | byte equality observed |
| mp_transform_2.test | 10 | 0.000797s | 0.000780s | 0.000792s | byte equality observed |
| mp_transform_front.test | 10 | 0.000793s | 0.000801s | 0.000796s | byte equality observed |
| mp_transform_front_2.test | 10 | 0.000368s | 0.000359s | 0.000783s | byte equality observed |
| mp_transform_if.test | 10 | 0.000827s | 0.000830s | 0.000789s | byte equality observed |
| mp_transform_if_q.test | 10 | 0.000831s | 0.000806s | 0.000777s | byte equality observed |
| mp_transform_q.test | 10 | 0.000837s | 0.000809s | 0.000815s | byte equality observed |
| mp_transform_q_2.test | 10 | 0.000814s | 0.000792s | 0.000799s | byte equality observed |
| mp_transform_second.test | 10 | 0.000796s | 0.000805s | 0.000805s | byte equality observed |
| mp_transform_second_2.test | 10 | 0.000363s | 0.000370s | 0.000787s | byte equality observed |
| mp_transform_sf.test | 10 | 0.000803s | 0.000793s | 0.000794s | byte equality observed |
| mp_transform_third.test | 10 | 0.000808s | 0.000800s | 0.000796s | byte equality observed |
| mp_transform_third_2.test | 10 | 0.000360s | 0.000361s | 0.000787s | byte equality observed |
| mp_unique.test | 10 | 0.000792s | 0.000802s | 0.000797s | byte equality observed |
| mp_unique_if.test | 10 | 0.000825s | 0.000791s | 0.000800s | byte equality observed |
| mp_unique_if_q.test | 10 | 0.000826s | 0.000797s | 0.000816s | byte equality observed |
| mp_valid.test | 10 | 0.000817s | 0.000797s | 0.000829s | byte equality observed |
| mp_valid_and_true.test | 10 | 0.000805s | 0.000794s | 0.000799s | byte equality observed |
| mp_value.test | 10 | 0.000363s | 0.000360s | 0.000795s | byte equality observed |
| mp_with_index.test | 10 | 0.001103s | 0.001077s | 0.001007s | byte equality observed |
| mp_with_index_cx.test | 10 | 0.000360s | 0.000375s | 0.000817s | byte equality observed |
| mpl.test | 10 | 0.000820s | 0.000786s | 0.000805s | byte equality observed |
| mpl_list.test | 10 | 0.000788s | 0.000815s | 0.000792s | byte equality observed |
| mpl_tuple.test | 10 | 0.000810s | 0.000827s | 0.000804s | byte equality observed |
| mpl_version.test | 10 | 0.000812s | 0.000809s | 0.000803s | byte equality observed |
| set_version.test | 10 | 0.000802s | 0.000800s | 0.000828s | byte equality observed |
| tuple_apply.test | 10 | 0.000796s | 0.000814s | 0.000790s | byte equality observed |
| tuple_for_each.test | 10 | 0.000805s | 0.000800s | 0.000810s | byte equality observed |
| tuple_transform.test | 10 | 0.000814s | 0.000803s | 0.000803s | byte equality observed |
| tuple_transform_2.test | 10 | 0.000810s | 0.000799s | 0.000819s | byte equality observed |
| tuple_version.test | 10 | 0.000805s | 0.000838s | 0.000810s | byte equality observed |
| utility_version.test | 10 | 0.000797s | 0.000799s | 0.000804s | byte equality observed |
| version.test | 10 | 0.000805s | 0.000824s | 0.000805s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814672).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
