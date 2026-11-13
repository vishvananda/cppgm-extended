# libs/qvm/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| math_test.test | RUN | 0.174798 | 0.457498 | 0.481447 | 0.001876 | 0.001666 | 0.001683 |
| mat_traits_array_test.test | RUN | 0.137721 | 0.446185 | 0.368879 | 0.001341 | 0.001378 | 0.001222 |
| mat_traits_std_array_test.test | RUN | 0.137943 | 0.346791 | 0.367819 | 0.001416 | 0.001413 | 0.001390 |
| vec_traits_array_test.test | RUN | 0.125284 | 0.358161 | 0.323265 | 0.001112 | 0.001275 | 0.001161 |
| vec_traits_gnuc_test.test | RUN | 0.222011 | 0.433525 | 0.600678 | 0.001227 | 0.001294 | 0.001400 |
| vec_traits_std_array_test.test | RUN | 0.118859 | 0.278227 | 0.315533 | 0.001264 | 0.001341 | 0.001411 |
| quat_traits_array_test.test | RUN | 0.118096 | 0.368122 | 0.315328 | 0.001365 | 0.001354 | 0.001188 |
| quat_traits_std_array_test.test | RUN | 0.119274 | 0.284366 | 0.314807 | 0.001266 | 0.001224 | 0.001325 |
| access_m_test.test | RUN | 0.186689 | 0.646819 | 0.728598 | 0.001292 | 0.001519 | 0.001310 |
| access_m_fail1.test | COMPILE_FAIL | 0.019618 | 0.029695 | 0.047735 | — | — | — |
| access_m_fail2.test | COMPILE_FAIL | 0.018668 | 0.029802 | 0.048028 | — | — | — |
| access_q_test.test | RUN | 0.160066 | 0.344838 | 0.441249 | 0.001200 | 0.001534 | 0.001502 |
| access_q_fail.test | COMPILE_FAIL | 0.015117 | 0.023883 | 0.042485 | — | — | — |
| access_v_test.test | RUN | 0.158445 | 0.365389 | 0.430455 | 0.001250 | 0.001376 | 0.001381 |
| access_v_fail1.test | COMPILE_FAIL | 0.013165 | 0.022919 | 0.039310 | — | — | — |
| access_v_fail2.test | COMPILE_FAIL | 0.013421 | 0.023171 | 0.039134 | — | — | — |
| assign_test.test | RUN | 0.238407 | 0.568417 | 0.760265 | 0.001438 | 0.001579 | 0.001369 |
| to_string_test.test | RUN | 0.262902 | 0.919386 | 4.491900 | 0.001346 | 0.001324 | 0.001480 |
| cmp_vv_test.test | RUN | 0.195206 | 0.453582 | 0.521839 | 0.001101 | 0.001480 | 0.001178 |
| cross_test.test | RUN | 0.195954 | 0.423070 | 0.511379 | 0.001234 | 0.001495 | 0.001431 |
| div_eq_vs_test.test | RUN | 0.177986 | 0.392901 | 0.505419 | 0.001448 | 0.001410 | 0.001456 |
| div_vs_test.test | RUN | 0.187431 | 0.481114 | 0.529082 | 0.001258 | 0.001523 | 0.001254 |
| dot_vv_test.test | RUN | 0.179030 | 0.440013 | 0.493238 | 0.001343 | 0.001259 | 0.001430 |
| eq_vv_test.test | RUN | 0.201807 | 0.478647 | 0.551408 | 0.001200 | 0.001389 | 0.001367 |
| mag_v_test.test | RUN | 0.177411 | 0.403444 | 0.467669 | 0.001363 | 0.001586 | 0.001358 |
| mag_sqr_v_test.test | RUN | 0.177686 | 0.395307 | 0.478324 | 0.001325 | 0.001356 | 0.001272 |
| minus_v_test.test | RUN | 0.181625 | 0.461086 | 0.516344 | 0.001235 | 0.001495 | 0.001293 |
| minus_eq_vv_test.test | RUN | 0.184561 | 0.450704 | 0.489710 | 0.001107 | 0.001276 | 0.001134 |
| minus_vv_test.test | RUN | 0.187381 | 0.494031 | 0.538241 | 0.001278 | 0.001418 | 0.001367 |
| mul_eq_vs_test.test | RUN | 0.177898 | 0.387708 | 0.499700 | 0.001275 | 0.001562 | 0.001536 |
| mul_vs_test.test | RUN | 0.192358 | 0.489524 | 0.522564 | 0.001267 | 0.001581 | 0.001356 |
| mul_sv_test.test | RUN | 0.198741 | 0.480634 | 0.536481 | 0.001178 | 0.001508 | 0.001407 |
| mul_vm_test.test | RUN | 0.267384 | 0.844427 | 0.838438 | 0.001310 | 0.001506 | 0.001367 |
| normalize_v_test.test | RUN | 0.191713 | 0.603331 | 0.536540 | 0.001444 | 0.001588 | 0.001365 |
| plus_eq_vv_test.test | RUN | 0.180041 | 0.456905 | 0.508747 | 0.001402 | 0.001302 | 0.001275 |
| plus_vv_test.test | RUN | 0.193223 | 0.488449 | 0.543537 | 0.001325 | 0.001578 | 0.001464 |
| scalar_cast_v_test.test | RUN | 0.173690 | 0.362272 | 0.472217 | 0.001157 | 0.001369 | 0.001178 |
| vec_index_test.test | RUN | 0.153282 | 0.364027 | 0.412334 | 0.001413 | 0.001421 | 0.001145 |
| vec_register_test.test | RUN | 0.101705 | 0.322589 | 0.307594 | 0.001186 | 0.001463 | 0.001271 |
| cmp_mm_test.test | RUN | 0.293585 | 1.026430 | 1.080770 | 0.001562 | 0.001633 | 0.001429 |
| determinant_test.test | RUN | 0.207755 | 0.548528 | 0.591750 | 0.001550 | 0.001398 | 0.001451 |
| div_eq_ms_test.test | RUN | 0.214426 | 0.490357 | 0.672444 | 0.001303 | 0.001370 | 0.001487 |
| div_ms_test.test | RUN | 0.249194 | 0.788828 | 0.798510 | 0.001376 | 0.001442 | 0.001483 |
| eq_mm_test.test | RUN | 0.291743 | 0.967311 | 1.029665 | 0.001590 | 0.001389 | 0.001267 |
| minus_m_test.test | RUN | 0.246150 | 0.778985 | 0.789806 | 0.001374 | 0.001499 | 0.001394 |
| minus_eq_mm_test.test | RUN | 0.237326 | 0.731364 | 0.725769 | 0.001236 | 0.001394 | 0.001388 |
| minus_mm_test.test | RUN | 0.264981 | 0.927030 | 0.882541 | 0.001231 | 0.001575 | 0.001385 |
| mul_eq_mm_test.test | RUN | 0.219898 | 0.679490 | 0.657925 | 0.001588 | 0.001316 | 0.001430 |
| mul_eq_ms_test.test | RUN | 0.220040 | 0.492950 | 0.677504 | 0.001374 | 0.001448 | 0.001241 |
| mul_mm_test.test | RUN | 0.305655 | 1.502251 | 1.095047 | 0.001366 | 0.001367 | 0.001420 |
| mul_ms_test.test | RUN | 0.257733 | 0.802157 | 0.835544 | 0.001372 | 0.001454 | 0.001319 |
| mul_sm_test.test | RUN | 0.258829 | 0.784066 | 0.832839 | 0.001508 | 0.001523 | 0.001329 |
| mul_mv_test.test | RUN | 0.269977 | 0.845098 | 0.837180 | 0.001501 | 0.001597 | 0.001196 |
| inverse_m_test.test | RUN | 0.340579 | 1.545722 | 1.188339 | 0.001514 | 0.001606 | 0.001558 |
| plus_eq_mm_test.test | RUN | 0.233346 | 0.720455 | 0.746717 | 0.001438 | 0.001522 | 0.001509 |
| plus_mm_test.test | RUN | 0.263314 | 0.932812 | 0.849768 | 0.001488 | 0.001476 | 0.001384 |
| scalar_cast_m_test.test | RUN | 0.202918 | 0.428142 | 0.535416 | 0.001432 | 0.001520 | 0.001334 |
| mat_index_test.test | RUN | 0.162744 | 0.398894 | 0.471438 | 0.001375 | 0.001460 | 0.001457 |
| cmp_qq_test.test | RUN | 0.184458 | 0.377726 | 0.438842 | 0.001383 | 0.001407 | 0.001370 |
| conjugate_test.test | RUN | 0.179949 | 0.386384 | 0.462811 | 0.001564 | 0.001566 | 0.001450 |
| normalize_q_test.test | RUN | 0.188175 | 0.428080 | 0.461040 | 0.001493 | 0.001504 | 0.001432 |
| div_eq_qs_test.test | RUN | 0.178217 | 0.359611 | 0.468466 | 0.001237 | 0.001374 | 0.001467 |
| div_qs_test.test | RUN | 0.182436 | 0.391121 | 0.461950 | 0.001228 | 0.001508 | 0.001381 |
| dot_qq_test.test | RUN | 0.184071 | 0.370499 | 0.465449 | 0.001526 | 0.001617 | 0.001714 |
| eq_qq_test.test | RUN | 0.180242 | 0.389524 | 0.475421 | 0.001577 | 0.001516 | 0.001427 |
| inverse_q_test.test | RUN | 0.213567 | 0.518644 | 0.585396 | 0.001391 | 0.001366 | 0.001505 |
| mag_q_test.test | RUN | 0.178856 | 0.380931 | 0.456251 | 0.001315 | 0.001614 | 0.001432 |
| mag_sqr_q_test.test | RUN | 0.178465 | 0.376123 | 0.468308 | 0.001494 | 0.001572 | 0.001455 |
| minus_q_test.test | RUN | 0.179135 | 0.382092 | 0.436763 | 0.001532 | 0.001608 | 0.001453 |
| minus_eq_qq_test.test | RUN | 0.178006 | 0.378762 | 0.461135 | 0.001335 | 0.001519 | 0.001512 |
| minus_qq_test.test | RUN | 0.182103 | 0.394572 | 0.471184 | 0.001448 | 0.001482 | 0.001366 |
| mul_eq_qs_test.test | RUN | 0.177204 | 0.355581 | 0.451403 | 0.001466 | 0.001572 | 0.001456 |
| mul_qs_test.test | RUN | 0.176848 | 0.383135 | 0.456756 | 0.001530 | 0.001576 | 0.001455 |
| mul_qv_test.test | RUN | 0.217240 | 0.509852 | 0.590908 | 0.001386 | 0.001556 | 0.001487 |
| mul_qq_test.test | RUN | 0.215529 | 0.532991 | 0.581209 | 0.001241 | 0.001565 | 0.001359 |
| mul_eq_qq_test.test | RUN | 0.211757 | 0.531875 | 0.585409 | 0.001477 | 0.001396 | 0.001447 |
| plus_eq_qq_test.test | RUN | 0.180639 | 0.375903 | 0.469033 | 0.001393 | 0.001268 | 0.001301 |
| plus_qq_test.test | RUN | 0.181348 | 0.398104 | 0.471776 | 0.001353 | 0.001532 | 0.001340 |
| scalar_cast_q_test.test | RUN | 0.183565 | 0.370645 | 0.454710 | 0.001450 | 0.001566 | 0.001543 |
| slerp_test.test | RUN | 0.195365 | 0.714044 | 0.563339 | 0.015942 | 0.011462 | 0.012015 |
| convert_to_test.test | RUN | 0.261609 | 0.666658 | 0.860612 | 0.001519 | 0.001586 | 0.001433 |
| rot_mat_test.test | RUN | 0.402554 | 3.016119 | 2.698988 | 0.055432 | 0.029934 | 0.026580 |
| rot_quat_test.test | RUN | 0.223018 | 0.623310 | 0.609267 | 0.001385 | 0.001582 | 0.001507 |
| rotx_mat_test.test | RUN | 0.248554 | 0.817415 | 0.785048 | 0.001374 | 0.001528 | 0.001370 |
| rotx_quat_test.test | RUN | 0.213956 | 0.539416 | 0.569271 | 0.001289 | 0.001548 | 0.001432 |
| roty_mat_test.test | RUN | 0.244137 | 0.811456 | 0.788733 | 0.001475 | 0.001606 | 0.001486 |
| roty_quat_test.test | RUN | 0.208167 | 0.532986 | 0.556074 | 0.001359 | 0.001515 | 0.001423 |
| rotz_mat_test.test | RUN | 0.259362 | 0.923665 | 0.831957 | 0.001331 | 0.001524 | 0.001563 |
| rotz_quat_test.test | RUN | 0.216181 | 0.533753 | 0.549392 | 0.001473 | 0.001385 | 0.001312 |
| zero_vec_test.test | RUN | 0.185790 | 0.445380 | 0.488526 | 0.001466 | 0.001150 | 0.001229 |
| zero_mat_test.test | RUN | 0.228804 | 0.728883 | 0.676828 | 0.001358 | 0.001521 | 0.001335 |
| zero_quat_test.test | RUN | 0.175707 | 0.379323 | 0.456942 | 0.001267 | 0.001321 | 0.001517 |
| identity_mat_test.test | RUN | 0.222088 | 0.540895 | 0.580033 | 0.001483 | 0.001486 | 0.001236 |
| identity_quat_test.test | RUN | 0.179565 | 0.366290 | 0.443914 | 0.001538 | 0.001621 | 0.001346 |
| col_test.test | RUN | 0.338353 | 1.142206 | 1.295803 | 0.001259 | 0.001451 | 0.001484 |
| col_mat_test.test | RUN | 0.222434 | 0.522167 | 0.598362 | 0.001439 | 0.001551 | 0.001406 |
| neg_col_test.test | RUN | 0.259183 | 0.727134 | 0.779029 | 0.001323 | 0.001395 | 0.001515 |
| neg_row_test.test | RUN | 0.268300 | 0.737637 | 0.787644 | 0.001527 | 0.001564 | 0.001422 |
| del_col_test.test | RUN | 0.263476 | 0.819722 | 0.816803 | 0.001423 | 0.001533 | 0.001450 |
| del_row_test.test | RUN | 0.267617 | 0.786311 | 0.815634 | 0.001494 | 0.001581 | 0.001426 |
| del_row_col_test.test | RUN | 0.367240 | 1.473390 | 1.490960 | 0.001572 | 0.001460 | 0.001463 |
| swap_cols_test.test | RUN | 0.495403 | 1.970769 | 1.869660 | 0.001370 | 0.001439 | 0.001534 |
| swap_rows_test.test | RUN | 0.535940 | 2.085945 | 1.838237 | 0.001724 | 0.001632 | 0.001424 |
| diag_test.test | RUN | 0.226624 | 0.543282 | 0.700411 | 0.001380 | 0.001325 | 0.001531 |
| diag_mat_test.test | RUN | 0.237836 | 0.529485 | 0.629365 | 0.001373 | 0.001562 | 0.001437 |
| row_test.test | RUN | 0.336499 | 1.140386 | 1.287266 | 0.001372 | 0.001193 | 0.001362 |
| row_mat_test.test | RUN | 0.220028 | 0.523955 | 0.599919 | 0.001513 | 0.001439 | 0.001486 |
| transpose_test.test | RUN | 0.264571 | 0.782576 | 0.833987 | 0.001488 | 0.001574 | 0.001462 |
| translation_test.test | RUN | 0.211273 | 0.502422 | 0.607907 | 0.001299 | 0.001546 | 0.001295 |
| translation_mat_test.test | RUN | 0.243764 | 0.500875 | 0.670657 | 0.001439 | 0.001594 | 0.001207 |
| swizzle_const_fail.test | COMPILE_FAIL | 0.081096 | 0.139897 | 0.184968 | — | — | — |
| swizzle2_test2.test | RUN | 0.096066 | 0.222998 | 0.259606 | 0.001408 | 0.001376 | 0.001326 |
| swizzle2_test3.test | RUN | 0.096989 | 0.213171 | 0.254236 | 0.001199 | 0.001369 | 0.001533 |
| swizzle2_test4.test | RUN | 0.097027 | 0.219838 | 0.263731 | 0.001303 | 0.001575 | 0.001167 |
| swizzle2_test.test | RUN | 0.309270 | 0.597900 | 0.816409 | 0.001422 | 0.001418 | 0.001398 |
| swizzle3_test2.test | RUN | 0.108836 | 0.231819 | 0.283655 | 0.001423 | 0.001539 | 0.001479 |
| swizzle3_test3.test | RUN | 0.107601 | 0.231362 | 0.280891 | 0.001279 | 0.001418 | 0.001354 |
| swizzle3_test4.test | RUN | 0.108111 | 0.231104 | 0.284175 | 0.001198 | 0.001570 | 0.001310 |
| swizzle3_test.test | RUN | 0.299749 | 0.603181 | 0.807919 | 0.001392 | 0.001238 | 0.001347 |
| swizzle4_test2.test | RUN | 0.181349 | 0.329069 | 0.443073 | 0.001514 | 0.001560 | 0.001391 |
| swizzle4_test3.test | RUN | 0.183525 | 0.345266 | 0.436746 | 0.001395 | 0.001446 | 0.001338 |
| swizzle4_test4.test | RUN | 0.185284 | 0.323037 | 0.442108 | 0.001226 | 0.001346 | 0.001426 |
| swizzle4_test.test | RUN | 0.308431 | 0.659222 | 0.828854 | 0.001271 | 0.001523 | 0.001408 |
| deduce_scalar_test.test | COMPILE | 0.016727 | 0.034464 | 0.048057 | — | — | — |
| deduce_scalar_mq_test.test | RUN | 0.118501 | 0.259322 | 0.298781 | 0.001374 | 0.001393 | 0.001438 |
| deduce_scalar_mv_test.test | RUN | 0.143525 | 0.296438 | 0.367469 | 0.001401 | 0.001290 | 0.001305 |
| deduce_scalar_fail.test | COMPILE_FAIL | 0.013100 | 0.022144 | 0.036704 | — | — | — |
| deduce_mat_test.test | COMPILE | 0.014611 | 0.030488 | 0.042336 | — | — | — |
| deduce_quat_test.test | COMPILE | 0.014328 | 0.029209 | 0.041922 | — | — | — |
| deduce_vec_test.test | COMPILE | 0.015459 | 0.029631 | 0.039923 | — | — | — |
| interop_test.test | RUN | 0.136494 | 0.359476 | 0.508467 | 0.000795 | 0.000444 | 0.001314 |
| transform_test.test | RUN | 0.249302 | 0.544195 | 0.721126 | 0.001120 | 0.001063 | 0.001005 |
| projection_test.test | RUN | 0.184420 | 0.363359 | 0.409955 | 0.001136 | 0.001182 | 0.001080 |
| scalar_traits_test.test | COMPILE | 0.015222 | 0.030721 | 0.045215 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.866× | 0.875×, 0.857× | 1.009× |
| clang | 0.872× | 0.871×, 0.873× | 1.002× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409407).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| access_m_test.test | 10 | 0.000854s | 0.000840s | 0.000856s | byte equality observed |
| access_q_test.test | 10 | 0.000856s | 0.000847s | 0.000850s | byte equality observed |
| access_v_test.test | 10 | 0.000841s | 0.000831s | 0.000886s | byte equality observed |
| assign_test.test | 10 | 0.000844s | 0.000837s | 0.000852s | byte equality observed |
| cmp_mm_test.test | 10 | 0.000865s | 0.000866s | 0.000852s | byte equality observed |
| cmp_qq_test.test | 10 | 0.000857s | 0.000837s | 0.000856s | byte equality observed |
| cmp_vv_test.test | 10 | 0.000845s | 0.000854s | 0.000848s | byte equality observed |
| col_mat_test.test | 10 | 0.000846s | 0.000853s | 0.000852s | byte equality observed |
| col_test.test | 10 | 0.000852s | 0.000848s | 0.000831s | byte equality observed |
| conjugate_test.test | 10 | 0.000853s | 0.000842s | 0.000843s | byte equality observed |
| convert_to_test.test | 10 | 0.000864s | 0.000837s | 0.000859s | byte equality observed |
| cross_test.test | 10 | 0.000848s | 0.000839s | 0.000848s | byte equality observed |
| deduce_scalar_mq_test.test | 10 | 0.000859s | 0.000840s | 0.000841s | byte equality observed |
| deduce_scalar_mv_test.test | 10 | 0.000871s | 0.000859s | 0.000853s | byte equality observed |
| del_col_test.test | 10 | 0.000852s | 0.000834s | 0.000836s | byte equality observed |
| del_row_col_test.test | 10 | 0.000854s | 0.000831s | 0.000835s | byte equality observed |
| del_row_test.test | 10 | 0.000857s | 0.000848s | 0.000845s | byte equality observed |
| determinant_test.test | 10 | 0.000867s | 0.000887s | 0.000873s | byte equality observed |
| diag_mat_test.test | 10 | 0.000844s | 0.000861s | 0.000848s | byte equality observed |
| diag_test.test | 10 | 0.000850s | 0.000853s | 0.000853s | byte equality observed |
| div_eq_ms_test.test | 10 | 0.000846s | 0.000871s | 0.000854s | byte equality observed |
| div_eq_qs_test.test | 10 | 0.000865s | 0.000839s | 0.000848s | byte equality observed |
| div_eq_vs_test.test | 10 | 0.000835s | 0.000869s | 0.000850s | byte equality observed |
| div_ms_test.test | 10 | 0.000855s | 0.000864s | 0.000833s | byte equality observed |
| div_qs_test.test | 10 | 0.000860s | 0.000859s | 0.000835s | byte equality observed |
| div_vs_test.test | 10 | 0.000848s | 0.000853s | 0.000844s | byte equality observed |
| dot_qq_test.test | 10 | 0.000847s | 0.000839s | 0.000850s | byte equality observed |
| dot_vv_test.test | 10 | 0.000846s | 0.000839s | 0.000867s | byte equality observed |
| eq_mm_test.test | 10 | 0.000863s | 0.000859s | 0.000840s | byte equality observed |
| eq_qq_test.test | 10 | 0.000846s | 0.000839s | 0.000846s | byte equality observed |
| eq_vv_test.test | 10 | 0.000844s | 0.000835s | 0.000860s | byte equality observed |
| identity_mat_test.test | 10 | 0.000856s | 0.000847s | 0.000864s | byte equality observed |
| identity_quat_test.test | 10 | 0.000858s | 0.000848s | 0.000861s | byte equality observed |
| interop_test.test | 10 | 0.000388s | 0.000392s | 0.000824s | byte equality observed |
| inverse_m_test.test | 10 | 0.000897s | 0.000911s | 0.000879s | byte equality observed |
| inverse_q_test.test | 10 | 0.000877s | 0.000836s | 0.000846s | byte equality observed |
| mag_q_test.test | 10 | 0.000852s | 0.000846s | 0.000852s | byte equality observed |
| mag_sqr_q_test.test | 10 | 0.000844s | 0.000848s | 0.000854s | byte equality observed |
| mag_sqr_v_test.test | 10 | 0.000843s | 0.000843s | 0.000845s | byte equality observed |
| mag_v_test.test | 10 | 0.000872s | 0.000846s | 0.000848s | byte equality observed |
| mat_index_test.test | 10 | 0.000844s | 0.000846s | 0.000855s | byte equality observed |
| mat_traits_array_test.test | 10 | 0.000863s | 0.000850s | 0.000840s | byte equality observed |
| mat_traits_std_array_test.test | 10 | 0.000860s | 0.000844s | 0.000837s | byte equality observed |
| math_test.test | 10 | 0.001274s | 0.001218s | 0.001103s | byte equality observed |
| minus_eq_mm_test.test | 10 | 0.000861s | 0.000854s | 0.000856s | byte equality observed |
| minus_eq_qq_test.test | 10 | 0.000867s | 0.000868s | 0.000850s | byte equality observed |
| minus_eq_vv_test.test | 10 | 0.000859s | 0.000841s | 0.000853s | byte equality observed |
| minus_m_test.test | 10 | 0.000853s | 0.000860s | 0.000853s | byte equality observed |
| minus_mm_test.test | 10 | 0.000851s | 0.000840s | 0.000848s | byte equality observed |
| minus_q_test.test | 10 | 0.000854s | 0.000858s | 0.000848s | byte equality observed |
| minus_qq_test.test | 10 | 0.000846s | 0.000846s | 0.000842s | byte equality observed |
| minus_v_test.test | 10 | 0.000860s | 0.000850s | 0.000865s | byte equality observed |
| minus_vv_test.test | 10 | 0.000857s | 0.000856s | 0.000851s | byte equality observed |
| mul_eq_mm_test.test | 10 | 0.000851s | 0.000852s | 0.000857s | byte equality observed |
| mul_eq_ms_test.test | 10 | 0.000845s | 0.000857s | 0.000857s | byte equality observed |
| mul_eq_qq_test.test | 10 | 0.000870s | 0.000855s | 0.000873s | byte equality observed |
| mul_eq_qs_test.test | 10 | 0.000852s | 0.000854s | 0.000842s | byte equality observed |
| mul_eq_vs_test.test | 10 | 0.000860s | 0.000851s | 0.000821s | byte equality observed |
| mul_mm_test.test | 10 | 0.000866s | 0.000853s | 0.000831s | byte equality observed |
| mul_ms_test.test | 10 | 0.000875s | 0.000870s | 0.000845s | byte equality observed |
| mul_mv_test.test | 10 | 0.000849s | 0.000852s | 0.000846s | byte equality observed |
| mul_qq_test.test | 10 | 0.000854s | 0.000850s | 0.000857s | byte equality observed |
| mul_qs_test.test | 10 | 0.000854s | 0.000847s | 0.000861s | byte equality observed |
| mul_qv_test.test | 10 | 0.000856s | 0.000849s | 0.000846s | byte equality observed |
| mul_sm_test.test | 10 | 0.000861s | 0.000851s | 0.000855s | byte equality observed |
| mul_sv_test.test | 10 | 0.000853s | 0.000841s | 0.000844s | byte equality observed |
| mul_vm_test.test | 10 | 0.000857s | 0.000839s | 0.000851s | byte equality observed |
| mul_vs_test.test | 10 | 0.000851s | 0.000848s | 0.000836s | byte equality observed |
| neg_col_test.test | 10 | 0.000850s | 0.000849s | 0.000843s | byte equality observed |
| neg_row_test.test | 10 | 0.000853s | 0.000826s | 0.000858s | byte equality observed |
| normalize_q_test.test | 10 | 0.000857s | 0.000854s | 0.000855s | byte equality observed |
| normalize_v_test.test | 10 | 0.000848s | 0.000866s | 0.000854s | byte equality observed |
| plus_eq_mm_test.test | 10 | 0.000842s | 0.000846s | 0.000835s | byte equality observed |
| plus_eq_qq_test.test | 10 | 0.000841s | 0.000840s | 0.000861s | byte equality observed |
| plus_eq_vv_test.test | 10 | 0.000847s | 0.000847s | 0.000848s | byte equality observed |
| plus_mm_test.test | 10 | 0.000854s | 0.000838s | 0.000861s | byte equality observed |
| plus_qq_test.test | 10 | 0.000860s | 0.000842s | 0.000847s | byte equality observed |
| plus_vv_test.test | 10 | 0.000854s | 0.000829s | 0.000859s | byte equality observed |
| projection_test.test | 10 | 0.000857s | 0.000843s | 0.000849s | byte equality observed |
| quat_traits_array_test.test | 10 | 0.000853s | 0.000842s | 0.000857s | byte equality observed |
| quat_traits_std_array_test.test | 10 | 0.000836s | 0.000860s | 0.000830s | byte equality observed |
| rot_mat_test.test | 1 | 0.035249s | 0.019401s | 0.017999s | byte equality observed |
| rot_quat_test.test | 10 | 0.000831s | 0.000827s | 0.000831s | byte equality observed |
| rotx_mat_test.test | 10 | 0.000829s | 0.000854s | 0.000864s | byte equality observed |
| rotx_quat_test.test | 10 | 0.000856s | 0.000860s | 0.000860s | byte equality observed |
| roty_mat_test.test | 10 | 0.000884s | 0.000853s | 0.000865s | byte equality observed |
| roty_quat_test.test | 10 | 0.000865s | 0.000858s | 0.000846s | byte equality observed |
| rotz_mat_test.test | 10 | 0.000873s | 0.000873s | 0.000876s | byte equality observed |
| rotz_quat_test.test | 10 | 0.000865s | 0.000849s | 0.000861s | byte equality observed |
| row_mat_test.test | 10 | 0.000841s | 0.000855s | 0.000834s | byte equality observed |
| row_test.test | 10 | 0.000881s | 0.000860s | 0.000833s | byte equality observed |
| scalar_cast_m_test.test | 10 | 0.000839s | 0.000863s | 0.000855s | byte equality observed |
| scalar_cast_q_test.test | 10 | 0.000848s | 0.000849s | 0.000844s | byte equality observed |
| scalar_cast_v_test.test | 10 | 0.000849s | 0.000852s | 0.000825s | byte equality observed |
| slerp_test.test | 2 | 0.010498s | 0.007704s | 0.008347s | byte equality observed |
| swap_cols_test.test | 10 | 0.000820s | 0.000831s | 0.000829s | byte equality observed |
| swap_rows_test.test | 10 | 0.000870s | 0.000868s | 0.000855s | byte equality observed |
| swizzle2_test.test | 10 | 0.000853s | 0.000846s | 0.000866s | byte equality observed |
| swizzle2_test2.test | 10 | 0.000839s | 0.000837s | 0.000841s | byte equality observed |
| swizzle2_test3.test | 10 | 0.000862s | 0.000837s | 0.000863s | byte equality observed |
| swizzle2_test4.test | 10 | 0.000846s | 0.000842s | 0.000874s | byte equality observed |
| swizzle3_test.test | 10 | 0.000842s | 0.000849s | 0.000838s | byte equality observed |
| swizzle3_test2.test | 10 | 0.000851s | 0.000852s | 0.000840s | byte equality observed |
| swizzle3_test3.test | 10 | 0.000855s | 0.000865s | 0.000829s | byte equality observed |
| swizzle3_test4.test | 10 | 0.000848s | 0.000856s | 0.000849s | byte equality observed |
| swizzle4_test.test | 10 | 0.000881s | 0.000843s | 0.000845s | byte equality observed |
| swizzle4_test2.test | 10 | 0.000856s | 0.000840s | 0.000836s | byte equality observed |
| swizzle4_test3.test | 10 | 0.000852s | 0.000840s | 0.000848s | byte equality observed |
| swizzle4_test4.test | 10 | 0.000861s | 0.000844s | 0.000850s | byte equality observed |
| to_string_test.test | 10 | 0.000880s | 0.000857s | 0.000866s | byte equality observed |
| transform_test.test | 10 | 0.000862s | 0.000863s | 0.000852s | byte equality observed |
| translation_mat_test.test | 10 | 0.000842s | 0.000842s | 0.000850s | byte equality observed |
| translation_test.test | 10 | 0.000850s | 0.000850s | 0.000850s | byte equality observed |
| transpose_test.test | 10 | 0.000842s | 0.000850s | 0.000838s | byte equality observed |
| vec_index_test.test | 10 | 0.000866s | 0.000857s | 0.000864s | byte equality observed |
| vec_register_test.test | 10 | 0.000860s | 0.000857s | 0.000871s | byte equality observed |
| vec_traits_array_test.test | 10 | 0.000850s | 0.000879s | 0.000852s | byte equality observed |
| vec_traits_gnuc_test.test | 10 | 0.000880s | 0.000844s | 0.000843s | byte equality observed |
| vec_traits_std_array_test.test | 10 | 0.000844s | 0.000836s | 0.000844s | byte equality observed |
| zero_mat_test.test | 10 | 0.000858s | 0.000849s | 0.000861s | byte equality observed |
| zero_quat_test.test | 10 | 0.000850s | 0.000830s | 0.000836s | byte equality observed |
| zero_vec_test.test | 10 | 0.000854s | 0.000856s | 0.000866s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409407).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
