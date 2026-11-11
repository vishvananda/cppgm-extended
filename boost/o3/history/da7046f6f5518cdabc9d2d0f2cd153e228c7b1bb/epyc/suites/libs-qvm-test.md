# libs/qvm/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| math_test.test | RUN | 0.223392 | 0.467057 | 0.541373 | 0.001836 | 0.001888 | 0.001858 |
| mat_traits_array_test.test | RUN | 0.151000 | 0.454466 | 0.424040 | 0.001334 | 0.001342 | 0.001312 |
| mat_traits_std_array_test.test | RUN | 0.152361 | 0.350610 | 0.426557 | 0.001416 | 0.001438 | 0.001278 |
| vec_traits_array_test.test | RUN | 0.138785 | 0.350842 | 0.388805 | 0.001394 | 0.001536 | 0.001522 |
| vec_traits_gnuc_test.test | RUN | 0.243496 | 0.429597 | 0.645136 | 0.001217 | 0.001332 | 0.001561 |
| vec_traits_std_array_test.test | RUN | 0.132324 | 0.276063 | 0.337613 | 0.001475 | 0.001483 | 0.001535 |
| quat_traits_array_test.test | RUN | 0.132330 | 0.366640 | 0.334861 | 0.001444 | 0.001424 | 0.001629 |
| quat_traits_std_array_test.test | RUN | 0.133737 | 0.282882 | 0.325389 | 0.001294 | 0.001551 | 0.001545 |
| access_m_test.test | RUN | 0.207859 | 0.653917 | 0.761794 | 0.001393 | 0.001603 | 0.001522 |
| access_m_fail1.test | COMPILE_FAIL | 0.021057 | 0.030377 | 0.053457 | — | — | — |
| access_m_fail2.test | COMPILE_FAIL | 0.020718 | 0.031501 | 0.053936 | — | — | — |
| access_q_test.test | RUN | 0.183772 | 0.342432 | 0.463012 | 0.001351 | 0.001436 | 0.001392 |
| access_q_fail.test | COMPILE_FAIL | 0.016525 | 0.025524 | 0.048591 | — | — | — |
| access_v_test.test | RUN | 0.183055 | 0.361176 | 0.456176 | 0.001379 | 0.001385 | 0.001482 |
| access_v_fail1.test | COMPILE_FAIL | 0.015102 | 0.022977 | 0.042275 | — | — | — |
| access_v_fail2.test | COMPILE_FAIL | 0.014956 | 0.023256 | 0.042615 | — | — | — |
| assign_test.test | RUN | 0.256090 | 0.555291 | 0.815478 | 0.001410 | 0.001624 | 0.001456 |
| to_string_test.test | RUN | 0.301501 | 0.898936 | 4.751408 | 0.001345 | 0.001548 | 0.001502 |
| cmp_vv_test.test | RUN | 0.226890 | 0.462538 | 0.551361 | 0.001453 | 0.001513 | 0.001614 |
| cross_test.test | RUN | 0.223190 | 0.432262 | 0.547258 | 0.001323 | 0.001567 | 0.001384 |
| div_eq_vs_test.test | RUN | 0.201395 | 0.384888 | 0.544104 | 0.001478 | 0.001438 | 0.001178 |
| div_vs_test.test | RUN | 0.212626 | 0.482947 | 0.559849 | 0.001342 | 0.001660 | 0.001481 |
| dot_vv_test.test | RUN | 0.204062 | 0.422433 | 0.530960 | 0.001489 | 0.001431 | 0.001638 |
| eq_vv_test.test | RUN | 0.220911 | 0.483838 | 0.599020 | 0.001368 | 0.001501 | 0.001409 |
| mag_v_test.test | RUN | 0.200665 | 0.400445 | 0.528869 | 0.001454 | 0.001603 | 0.001178 |
| mag_sqr_v_test.test | RUN | 0.200074 | 0.398708 | 0.494821 | 0.001426 | 0.001471 | 0.001368 |
| minus_v_test.test | RUN | 0.205365 | 0.459883 | 0.534250 | 0.001378 | 0.001532 | 0.001546 |
| minus_eq_vv_test.test | RUN | 0.212162 | 0.453000 | 0.531337 | 0.001339 | 0.001514 | 0.001374 |
| minus_vv_test.test | RUN | 0.209748 | 0.489092 | 0.558123 | 0.001504 | 0.001438 | 0.001310 |
| mul_eq_vs_test.test | RUN | 0.200142 | 0.388400 | 0.542135 | 0.001364 | 0.001550 | 0.001630 |
| mul_vs_test.test | RUN | 0.215018 | 0.481594 | 0.578199 | 0.001371 | 0.001610 | 0.001397 |
| mul_sv_test.test | RUN | 0.219112 | 0.486220 | 0.553354 | 0.001271 | 0.001604 | 0.001394 |
| mul_vm_test.test | RUN | 0.295185 | 0.837016 | 0.903693 | 0.001304 | 0.001439 | 0.001723 |
| normalize_v_test.test | RUN | 0.215204 | 0.599028 | 0.574636 | 0.001422 | 0.001400 | 0.001484 |
| plus_eq_vv_test.test | RUN | 0.205874 | 0.440888 | 0.547775 | 0.001341 | 0.001378 | 0.001515 |
| plus_vv_test.test | RUN | 0.218504 | 0.492919 | 0.577864 | 0.001268 | 0.001605 | 0.001561 |
| scalar_cast_v_test.test | RUN | 0.197779 | 0.360351 | 0.481598 | 0.001197 | 0.001508 | 0.001667 |
| vec_index_test.test | RUN | 0.179556 | 0.367355 | 0.459979 | 0.001432 | 0.001509 | 0.001580 |
| vec_register_test.test | RUN | 0.111816 | 0.326782 | 0.323828 | 0.001624 | 0.001507 | 0.001672 |
| cmp_mm_test.test | RUN | 0.323833 | 1.036764 | 1.161890 | 0.001622 | 0.001501 | 0.001434 |
| determinant_test.test | RUN | 0.230393 | 0.534212 | 0.648779 | 0.001428 | 0.001501 | 0.001703 |
| div_eq_ms_test.test | RUN | 0.245452 | 0.507347 | 0.736444 | 0.001378 | 0.001322 | 0.001600 |
| div_ms_test.test | RUN | 0.264925 | 0.783541 | 0.883814 | 0.001447 | 0.001190 | 0.001488 |
| eq_mm_test.test | RUN | 0.314297 | 0.948016 | 1.113220 | 0.001437 | 0.001650 | 0.001634 |
| minus_m_test.test | RUN | 0.263232 | 0.759427 | 0.825386 | 0.001292 | 0.001550 | 0.001557 |
| minus_eq_mm_test.test | RUN | 0.252587 | 0.707315 | 0.782232 | 0.001456 | 0.001431 | 0.001553 |
| minus_mm_test.test | RUN | 0.280880 | 0.900380 | 0.916362 | 0.001347 | 0.001629 | 0.001574 |
| mul_eq_mm_test.test | RUN | 0.242005 | 0.679688 | 0.683023 | 0.001436 | 0.001321 | 0.001590 |
| mul_eq_ms_test.test | RUN | 0.244158 | 0.490264 | 0.725711 | 0.001471 | 0.001514 | 0.001562 |
| mul_mm_test.test | RUN | 0.320099 | 1.505536 | 1.167178 | 0.001451 | 0.001308 | 0.001624 |
| mul_ms_test.test | RUN | 0.277639 | 0.802323 | 0.876356 | 0.001337 | 0.001555 | 0.001542 |
| mul_sm_test.test | RUN | 0.274967 | 0.796897 | 0.892915 | 0.001674 | 0.001505 | 0.001630 |
| mul_mv_test.test | RUN | 0.297542 | 0.862187 | 0.916978 | 0.001517 | 0.001463 | 0.001504 |
| inverse_m_test.test | RUN | 0.379737 | 1.559673 | 1.279963 | 0.001639 | 0.001632 | 0.001527 |
| plus_eq_mm_test.test | RUN | 0.254859 | 0.744144 | 0.778942 | 0.001456 | 0.001627 | 0.001583 |
| plus_mm_test.test | RUN | 0.283549 | 0.903664 | 0.932193 | 0.001526 | 0.001428 | 0.001462 |
| scalar_cast_m_test.test | RUN | 0.230185 | 0.408399 | 0.575699 | 0.001416 | 0.001535 | 0.001273 |
| mat_index_test.test | RUN | 0.187849 | 0.399295 | 0.507479 | 0.001506 | 0.001424 | 0.001508 |
| cmp_qq_test.test | RUN | 0.199652 | 0.374203 | 0.493137 | 0.001555 | 0.001496 | 0.001434 |
| conjugate_test.test | RUN | 0.201903 | 0.382950 | 0.500974 | 0.001441 | 0.001449 | 0.001280 |
| normalize_q_test.test | RUN | 0.211186 | 0.429331 | 0.519922 | 0.001452 | 0.001453 | 0.001544 |
| div_eq_qs_test.test | RUN | 0.199688 | 0.355549 | 0.496948 | 0.001198 | 0.001375 | 0.001569 |
| div_qs_test.test | RUN | 0.191625 | 0.389585 | 0.516673 | 0.001436 | 0.001614 | 0.001540 |
| dot_qq_test.test | RUN | 0.199078 | 0.371473 | 0.511272 | 0.001515 | 0.001596 | 0.001243 |
| eq_qq_test.test | RUN | 0.198834 | 0.389701 | 0.506945 | 0.001346 | 0.001628 | 0.001567 |
| inverse_q_test.test | RUN | 0.238404 | 0.515179 | 0.634694 | 0.001401 | 0.001618 | 0.001456 |
| mag_q_test.test | RUN | 0.203501 | 0.377360 | 0.484157 | 0.001481 | 0.001406 | 0.001524 |
| mag_sqr_q_test.test | RUN | 0.197128 | 0.379262 | 0.482384 | 0.001434 | 0.001604 | 0.001551 |
| minus_q_test.test | RUN | 0.199253 | 0.379696 | 0.471410 | 0.001340 | 0.001561 | 0.001504 |
| minus_eq_qq_test.test | RUN | 0.199486 | 0.379232 | 0.494773 | 0.001295 | 0.001665 | 0.001574 |
| minus_qq_test.test | RUN | 0.205495 | 0.392442 | 0.487866 | 0.001259 | 0.001454 | 0.001535 |
| mul_eq_qs_test.test | RUN | 0.207623 | 0.367498 | 0.479687 | 0.001426 | 0.001569 | 0.001529 |
| mul_qs_test.test | RUN | 0.192132 | 0.379725 | 0.498741 | 0.001458 | 0.001597 | 0.001600 |
| mul_qv_test.test | RUN | 0.245131 | 0.520308 | 0.637581 | 0.001434 | 0.001634 | 0.001640 |
| mul_qq_test.test | RUN | 0.244075 | 0.515467 | 0.609001 | 0.001395 | 0.001482 | 0.001291 |
| mul_eq_qq_test.test | RUN | 0.237533 | 0.529373 | 0.613476 | 0.001303 | 0.001218 | 0.001583 |
| plus_eq_qq_test.test | RUN | 0.197428 | 0.379465 | 0.487301 | 0.001668 | 0.001490 | 0.001600 |
| plus_qq_test.test | RUN | 0.205787 | 0.393619 | 0.487888 | 0.001451 | 0.001483 | 0.001437 |
| scalar_cast_q_test.test | RUN | 0.200312 | 0.368944 | 0.488066 | 0.001387 | 0.001242 | 0.001493 |
| slerp_test.test | RUN | 0.221446 | 0.717532 | 0.595362 | 0.014494 | 0.011193 | 0.012660 |
| convert_to_test.test | RUN | 0.291425 | 0.671005 | 0.901639 | 0.001450 | 0.001582 | 0.001618 |
| rot_mat_test.test | RUN | 0.474666 | 3.029923 | 2.831654 | 0.046380 | 0.028446 | 0.029775 |
| rot_quat_test.test | RUN | 0.245882 | 0.619146 | 0.640000 | 0.001518 | 0.001564 | 0.001613 |
| rotx_mat_test.test | RUN | 0.269552 | 0.805642 | 0.847311 | 0.001446 | 0.001598 | 0.001586 |
| rotx_quat_test.test | RUN | 0.241557 | 0.532469 | 0.619641 | 0.001279 | 0.001526 | 0.001601 |
| roty_mat_test.test | RUN | 0.277271 | 0.815068 | 0.817875 | 0.001647 | 0.001499 | 0.001543 |
| roty_quat_test.test | RUN | 0.236983 | 0.521337 | 0.613028 | 0.001474 | 0.001634 | 0.001480 |
| rotz_mat_test.test | RUN | 0.281457 | 0.926230 | 0.918203 | 0.001508 | 0.001524 | 0.001730 |
| rotz_quat_test.test | RUN | 0.238690 | 0.526726 | 0.595590 | 0.001349 | 0.001557 | 0.001487 |
| zero_vec_test.test | RUN | 0.217719 | 0.451394 | 0.498141 | 0.001564 | 0.001232 | 0.001417 |
| zero_mat_test.test | RUN | 0.281618 | 0.733692 | 0.743858 | 0.001349 | 0.001360 | 0.001473 |
| zero_quat_test.test | RUN | 0.196308 | 0.379468 | 0.490040 | 0.001403 | 0.001490 | 0.001570 |
| identity_mat_test.test | RUN | 0.245216 | 0.563356 | 0.619038 | 0.001427 | 0.001579 | 0.001217 |
| identity_quat_test.test | RUN | 0.205200 | 0.368728 | 0.494619 | 0.001438 | 0.001625 | 0.001622 |
| col_test.test | RUN | 0.362286 | 1.145266 | 1.387171 | 0.001380 | 0.001508 | 0.001550 |
| col_mat_test.test | RUN | 0.246161 | 0.518177 | 0.654474 | 0.001511 | 0.001588 | 0.001616 |
| neg_col_test.test | RUN | 0.279146 | 0.722736 | 0.829264 | 0.001316 | 0.001635 | 0.001586 |
| neg_row_test.test | RUN | 0.298498 | 0.750860 | 0.842448 | 0.001381 | 0.001551 | 0.001603 |
| del_col_test.test | RUN | 0.283118 | 0.808020 | 0.862204 | 0.001550 | 0.001544 | 0.001618 |
| del_row_test.test | RUN | 0.284073 | 0.796521 | 0.873577 | 0.001498 | 0.001596 | 0.001601 |
| del_row_col_test.test | RUN | 0.386283 | 1.475809 | 1.605529 | 0.001478 | 0.001664 | 0.001534 |
| swap_cols_test.test | RUN | 0.530431 | 1.961990 | 1.996125 | 0.001548 | 0.001550 | 0.001649 |
| swap_rows_test.test | RUN | 0.570346 | 2.069981 | 1.950972 | 0.001713 | 0.001596 | 0.001571 |
| diag_test.test | RUN | 0.249835 | 0.546821 | 0.756582 | 0.001584 | 0.001537 | 0.001662 |
| diag_mat_test.test | RUN | 0.263920 | 0.538866 | 0.685007 | 0.001516 | 0.001580 | 0.001614 |
| row_test.test | RUN | 0.361821 | 1.134542 | 1.368001 | 0.001441 | 0.001540 | 0.001545 |
| row_mat_test.test | RUN | 0.248842 | 0.513168 | 0.667821 | 0.001680 | 0.001485 | 0.001433 |
| transpose_test.test | RUN | 0.285524 | 0.808655 | 0.884935 | 0.001445 | 0.001592 | 0.001292 |
| translation_test.test | RUN | 0.238438 | 0.502925 | 0.663091 | 0.001354 | 0.001492 | 0.001228 |
| translation_mat_test.test | RUN | 0.254532 | 0.508317 | 0.716240 | 0.001455 | 0.001199 | 0.001268 |
| swizzle_const_fail.test | COMPILE_FAIL | 0.087529 | 0.141904 | 0.200513 | — | — | — |
| swizzle2_test2.test | RUN | 0.108207 | 0.227577 | 0.273185 | 0.001382 | 0.001465 | 0.001394 |
| swizzle2_test3.test | RUN | 0.106352 | 0.221719 | 0.282225 | 0.001371 | 0.001472 | 0.001407 |
| swizzle2_test4.test | RUN | 0.109579 | 0.215708 | 0.270042 | 0.001113 | 0.001588 | 0.001098 |
| swizzle2_test.test | RUN | 0.337370 | 0.589287 | 0.851258 | 0.001458 | 0.001346 | 0.001603 |
| swizzle3_test2.test | RUN | 0.118925 | 0.229490 | 0.300547 | 0.001470 | 0.001508 | 0.001423 |
| swizzle3_test3.test | RUN | 0.118280 | 0.236958 | 0.301046 | 0.001382 | 0.001556 | 0.001216 |
| swizzle3_test4.test | RUN | 0.121150 | 0.238403 | 0.295925 | 0.001293 | 0.001391 | 0.001348 |
| swizzle3_test.test | RUN | 0.326659 | 0.598426 | 0.876213 | 0.001518 | 0.001328 | 0.001453 |
| swizzle4_test2.test | RUN | 0.195312 | 0.339497 | 0.505978 | 0.001590 | 0.001589 | 0.001492 |
| swizzle4_test3.test | RUN | 0.196899 | 0.328145 | 0.496390 | 0.001604 | 0.001347 | 0.001415 |
| swizzle4_test4.test | RUN | 0.204433 | 0.340783 | 0.476270 | 0.001340 | 0.001346 | 0.001562 |
| swizzle4_test.test | RUN | 0.337653 | 0.665814 | 0.902576 | 0.001363 | 0.001564 | 0.001541 |
| deduce_scalar_test.test | COMPILE | 0.017832 | 0.033762 | 0.052914 | — | — | — |
| deduce_scalar_mq_test.test | RUN | 0.128029 | 0.263859 | 0.306628 | 0.001426 | 0.001529 | 0.001425 |
| deduce_scalar_mv_test.test | RUN | 0.152795 | 0.293094 | 0.394811 | 0.001335 | 0.001404 | 0.001598 |
| deduce_scalar_fail.test | COMPILE_FAIL | 0.015027 | 0.021139 | 0.035012 | — | — | — |
| deduce_mat_test.test | COMPILE | 0.015895 | 0.030624 | 0.044371 | — | — | — |
| deduce_quat_test.test | COMPILE | 0.016055 | 0.031503 | 0.043385 | — | — | — |
| deduce_vec_test.test | COMPILE | 0.016214 | 0.030684 | 0.043995 | — | — | — |
| interop_test.test | RUN | 0.141254 | 0.359642 | 0.541734 | 0.000475 | 0.000665 | 0.001520 |
| transform_test.test | RUN | 0.303811 | 0.533342 | 0.769279 | 0.001115 | 0.001148 | 0.001140 |
| projection_test.test | RUN | 0.205737 | 0.389316 | 0.452277 | 0.001175 | 0.001122 | 0.001193 |
| scalar_traits_test.test | COMPILE | 0.017023 | 0.031780 | 0.047647 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.896×† | 0.934×, 0.858× | 0.987× |
| clang | 0.920× | 0.927×, 0.913× | 0.994× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| access_m_test.test | 10 | 0.000973s | 0.000955s | 0.001033s | byte equality observed |
| access_q_test.test | 10 | 0.000969s | 0.000955s | 0.001026s | byte equality observed |
| access_v_test.test | 10 | 0.000976s | 0.000931s | 0.000998s | byte equality observed |
| assign_test.test | 10 | 0.000960s | 0.000941s | 0.001000s | byte equality observed |
| cmp_mm_test.test | 10 | 0.000989s | 0.000952s | 0.001015s | byte equality observed |
| cmp_qq_test.test | 10 | 0.000974s | 0.000939s | 0.001019s | byte equality observed |
| cmp_vv_test.test | 10 | 0.000975s | 0.000948s | 0.001019s | byte equality observed |
| col_mat_test.test | 10 | 0.000975s | 0.000950s | 0.001052s | byte equality observed |
| col_test.test | 10 | 0.000961s | 0.000954s | 0.001028s | byte equality observed |
| conjugate_test.test | 10 | 0.000976s | 0.000928s | 0.001008s | byte equality observed |
| convert_to_test.test | 10 | 0.000976s | 0.000937s | 0.001010s | byte equality observed |
| cross_test.test | 10 | 0.000961s | 0.000964s | 0.001011s | byte equality observed |
| deduce_scalar_mq_test.test | 10 | 0.000999s | 0.000946s | 0.001009s | byte equality observed |
| deduce_scalar_mv_test.test | 10 | 0.001000s | 0.000952s | 0.001003s | byte equality observed |
| del_col_test.test | 10 | 0.000978s | 0.000971s | 0.001026s | byte equality observed |
| del_row_col_test.test | 10 | 0.000995s | 0.000961s | 0.001040s | byte equality observed |
| del_row_test.test | 10 | 0.000983s | 0.000942s | 0.001005s | byte equality observed |
| determinant_test.test | 10 | 0.000998s | 0.000955s | 0.001009s | byte equality observed |
| diag_mat_test.test | 10 | 0.000976s | 0.000954s | 0.001018s | byte equality observed |
| diag_test.test | 10 | 0.000971s | 0.000934s | 0.001051s | byte equality observed |
| div_eq_ms_test.test | 10 | 0.000965s | 0.000927s | 0.001038s | byte equality observed |
| div_eq_qs_test.test | 10 | 0.000984s | 0.000927s | 0.001030s | byte equality observed |
| div_eq_vs_test.test | 10 | 0.000965s | 0.000923s | 0.001028s | byte equality observed |
| div_ms_test.test | 10 | 0.000976s | 0.000921s | 0.000994s | byte equality observed |
| div_qs_test.test | 10 | 0.000963s | 0.000919s | 0.001000s | byte equality observed |
| div_vs_test.test | 10 | 0.000952s | 0.000921s | 0.000997s | byte equality observed |
| dot_qq_test.test | 10 | 0.000959s | 0.000911s | 0.001005s | byte equality observed |
| dot_vv_test.test | 10 | 0.000980s | 0.000931s | 0.001031s | byte equality observed |
| eq_mm_test.test | 10 | 0.000962s | 0.000927s | 0.001030s | byte equality observed |
| eq_qq_test.test | 10 | 0.000965s | 0.000922s | 0.000996s | byte equality observed |
| eq_vv_test.test | 10 | 0.000955s | 0.000929s | 0.001001s | byte equality observed |
| identity_mat_test.test | 10 | 0.000972s | 0.000929s | 0.001018s | byte equality observed |
| identity_quat_test.test | 10 | 0.000950s | 0.000965s | 0.001005s | byte equality observed |
| interop_test.test | 10 | 0.000448s | 0.000436s | 0.000983s | byte equality observed |
| inverse_m_test.test | 10 | 0.001032s | 0.001005s | 0.001057s | byte equality observed |
| inverse_q_test.test | 10 | 0.000963s | 0.000974s | 0.000979s | byte equality observed |
| mag_q_test.test | 10 | 0.000956s | 0.000958s | 0.000998s | byte equality observed |
| mag_sqr_q_test.test | 10 | 0.000954s | 0.000936s | 0.001007s | byte equality observed |
| mag_sqr_v_test.test | 10 | 0.000934s | 0.000941s | 0.001000s | byte equality observed |
| mag_v_test.test | 10 | 0.000949s | 0.000938s | 0.001010s | byte equality observed |
| mat_index_test.test | 10 | 0.000955s | 0.000930s | 0.000995s | byte equality observed |
| mat_traits_array_test.test | 10 | 0.000943s | 0.000947s | 0.001042s | byte equality observed |
| mat_traits_std_array_test.test | 10 | 0.000951s | 0.000953s | 0.001011s | byte equality observed |
| math_test.test | 10 | 0.001442s | 0.001356s | 0.001316s | byte equality observed |
| minus_eq_mm_test.test | 10 | 0.000968s | 0.000954s | 0.001011s | byte equality observed |
| minus_eq_qq_test.test | 10 | 0.000954s | 0.000965s | 0.001000s | byte equality observed |
| minus_eq_vv_test.test | 10 | 0.000943s | 0.000947s | 0.001029s | byte equality observed |
| minus_m_test.test | 10 | 0.000973s | 0.000953s | 0.001020s | byte equality observed |
| minus_mm_test.test | 10 | 0.000970s | 0.000936s | 0.001044s | byte equality observed |
| minus_q_test.test | 10 | 0.000971s | 0.000929s | 0.001008s | byte equality observed |
| minus_qq_test.test | 10 | 0.000975s | 0.000928s | 0.001004s | byte equality observed |
| minus_v_test.test | 10 | 0.000957s | 0.000940s | 0.000999s | byte equality observed |
| minus_vv_test.test | 10 | 0.000962s | 0.000943s | 0.001009s | byte equality observed |
| mul_eq_mm_test.test | 10 | 0.000982s | 0.000949s | 0.001008s | byte equality observed |
| mul_eq_ms_test.test | 10 | 0.000963s | 0.000951s | 0.001026s | byte equality observed |
| mul_eq_qq_test.test | 10 | 0.000991s | 0.000959s | 0.001011s | byte equality observed |
| mul_eq_qs_test.test | 10 | 0.000985s | 0.000944s | 0.001006s | byte equality observed |
| mul_eq_vs_test.test | 10 | 0.000950s | 0.000934s | 0.000998s | byte equality observed |
| mul_mm_test.test | 10 | 0.000958s | 0.000951s | 0.001006s | byte equality observed |
| mul_ms_test.test | 10 | 0.000984s | 0.000939s | 0.000990s | byte equality observed |
| mul_mv_test.test | 10 | 0.000972s | 0.000962s | 0.000994s | byte equality observed |
| mul_qq_test.test | 10 | 0.000967s | 0.000952s | 0.001016s | byte equality observed |
| mul_qs_test.test | 10 | 0.000969s | 0.000960s | 0.000996s | byte equality observed |
| mul_qv_test.test | 10 | 0.001001s | 0.000954s | 0.001005s | byte equality observed |
| mul_sm_test.test | 10 | 0.000942s | 0.000963s | 0.000991s | byte equality observed |
| mul_sv_test.test | 10 | 0.000950s | 0.000971s | 0.000996s | byte equality observed |
| mul_vm_test.test | 10 | 0.000968s | 0.000953s | 0.001006s | byte equality observed |
| mul_vs_test.test | 10 | 0.000944s | 0.000939s | 0.000997s | byte equality observed |
| neg_col_test.test | 10 | 0.000964s | 0.000991s | 0.001024s | byte equality observed |
| neg_row_test.test | 10 | 0.000972s | 0.000948s | 0.001023s | byte equality observed |
| normalize_q_test.test | 10 | 0.000991s | 0.000961s | 0.001042s | byte equality observed |
| normalize_v_test.test | 10 | 0.000962s | 0.000952s | 0.001022s | byte equality observed |
| plus_eq_mm_test.test | 10 | 0.000947s | 0.000958s | 0.001028s | byte equality observed |
| plus_eq_qq_test.test | 10 | 0.000952s | 0.000936s | 0.001043s | byte equality observed |
| plus_eq_vv_test.test | 10 | 0.000939s | 0.000941s | 0.001017s | byte equality observed |
| plus_mm_test.test | 10 | 0.000956s | 0.000955s | 0.000997s | byte equality observed |
| plus_qq_test.test | 10 | 0.000970s | 0.000949s | 0.000979s | byte equality observed |
| plus_vv_test.test | 10 | 0.000982s | 0.000961s | 0.000991s | byte equality observed |
| projection_test.test | 10 | 0.000965s | 0.000935s | 0.000996s | byte equality observed |
| quat_traits_array_test.test | 10 | 0.000960s | 0.000981s | 0.000999s | byte equality observed |
| quat_traits_std_array_test.test | 10 | 0.000942s | 0.000941s | 0.000977s | byte equality observed |
| rot_mat_test.test | 1 | 0.032698s | 0.021478s | 0.019883s | byte equality observed |
| rot_quat_test.test | 10 | 0.000918s | 0.000930s | 0.000961s | byte equality observed |
| rotx_mat_test.test | 10 | 0.000940s | 0.000956s | 0.001012s | byte equality observed |
| rotx_quat_test.test | 10 | 0.000958s | 0.000969s | 0.001029s | byte equality observed |
| roty_mat_test.test | 10 | 0.000996s | 0.000971s | 0.001027s | byte equality observed |
| roty_quat_test.test | 10 | 0.000980s | 0.000954s | 0.001028s | byte equality observed |
| rotz_mat_test.test | 10 | 0.001023s | 0.000965s | 0.001047s | byte equality observed |
| rotz_quat_test.test | 10 | 0.000971s | 0.000952s | 0.001031s | byte equality observed |
| row_mat_test.test | 10 | 0.000944s | 0.000930s | 0.001036s | byte equality observed |
| row_test.test | 10 | 0.000956s | 0.000937s | 0.001006s | byte equality observed |
| scalar_cast_m_test.test | 10 | 0.000951s | 0.000934s | 0.001023s | byte equality observed |
| scalar_cast_q_test.test | 10 | 0.000952s | 0.000941s | 0.001009s | byte equality observed |
| scalar_cast_v_test.test | 10 | 0.000951s | 0.000945s | 0.000993s | byte equality observed |
| slerp_test.test | 3 | 0.010610s | 0.008236s | 0.009387s | byte equality observed |
| swap_cols_test.test | 10 | 0.000932s | 0.000913s | 0.000971s | byte equality observed |
| swap_rows_test.test | 10 | 0.001000s | 0.000950s | 0.001003s | byte equality observed |
| swizzle2_test.test | 10 | 0.000981s | 0.000933s | 0.001005s | byte equality observed |
| swizzle2_test2.test | 10 | 0.000965s | 0.000954s | 0.001003s | byte equality observed |
| swizzle2_test3.test | 10 | 0.000935s | 0.000950s | 0.001002s | byte equality observed |
| swizzle2_test4.test | 10 | 0.000964s | 0.000937s | 0.000986s | byte equality observed |
| swizzle3_test.test | 10 | 0.000957s | 0.000949s | 0.001005s | byte equality observed |
| swizzle3_test2.test | 10 | 0.000975s | 0.000924s | 0.000986s | byte equality observed |
| swizzle3_test3.test | 10 | 0.001007s | 0.000930s | 0.000973s | byte equality observed |
| swizzle3_test4.test | 10 | 0.001004s | 0.000949s | 0.000990s | byte equality observed |
| swizzle4_test.test | 10 | 0.000991s | 0.000956s | 0.000995s | byte equality observed |
| swizzle4_test2.test | 10 | 0.000984s | 0.000961s | 0.001009s | byte equality observed |
| swizzle4_test3.test | 10 | 0.000968s | 0.000964s | 0.001025s | byte equality observed |
| swizzle4_test4.test | 10 | 0.000997s | 0.000960s | 0.001014s | byte equality observed |
| to_string_test.test | 10 | 0.001023s | 0.000978s | 0.001036s | byte equality observed |
| transform_test.test | 10 | 0.000982s | 0.000960s | 0.001000s | byte equality observed |
| translation_mat_test.test | 10 | 0.000970s | 0.000965s | 0.000976s | byte equality observed |
| translation_test.test | 10 | 0.000973s | 0.000959s | 0.001011s | byte equality observed |
| transpose_test.test | 10 | 0.000972s | 0.000956s | 0.000996s | byte equality observed |
| vec_index_test.test | 10 | 0.000956s | 0.000968s | 0.001002s | byte equality observed |
| vec_register_test.test | 10 | 0.000974s | 0.000959s | 0.000997s | byte equality observed |
| vec_traits_array_test.test | 10 | 0.000993s | 0.000971s | 0.000991s | byte equality observed |
| vec_traits_gnuc_test.test | 10 | 0.000975s | 0.000958s | 0.000999s | byte equality observed |
| vec_traits_std_array_test.test | 10 | 0.000980s | 0.000964s | 0.001014s | byte equality observed |
| zero_mat_test.test | 10 | 0.000966s | 0.000952s | 0.001020s | byte equality observed |
| zero_quat_test.test | 10 | 0.000971s | 0.000963s | 0.001013s | byte equality observed |
| zero_vec_test.test | 10 | 0.000987s | 0.000937s | 0.001001s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
