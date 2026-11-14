# libs/qvm/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| math_test.test | RUN | 0.204074 | 0.464714 | 0.493697 | 0.001878 | 0.001798 | 0.001778 |
| mat_traits_array_test.test | RUN | 0.156692 | 0.458289 | 0.372664 | 0.001653 | 0.001245 | 0.001267 |
| mat_traits_std_array_test.test | RUN | 0.158599 | 0.348310 | 0.379964 | 0.001450 | 0.001442 | 0.001034 |
| vec_traits_array_test.test | RUN | 0.138812 | 0.359048 | 0.320467 | 0.001240 | 0.001469 | 0.001232 |
| vec_traits_gnuc_test.test | RUN | 0.240396 | 0.432236 | 0.597996 | 0.001400 | 0.001378 | 0.001260 |
| vec_traits_std_array_test.test | RUN | 0.129297 | 0.277738 | 0.315666 | 0.001544 | 0.001363 | 0.001353 |
| quat_traits_array_test.test | RUN | 0.130538 | 0.366645 | 0.317589 | 0.001337 | 0.001386 | 0.001194 |
| quat_traits_std_array_test.test | RUN | 0.128130 | 0.286124 | 0.314665 | 0.001302 | 0.001349 | 0.001460 |
| access_m_test.test | RUN | 0.192322 | 0.653743 | 0.720054 | 0.001426 | 0.001588 | 0.001308 |
| access_m_fail1.test | COMPILE_FAIL | 0.020623 | 0.030667 | 0.051955 | — | — | — |
| access_m_fail2.test | COMPILE_FAIL | 0.015246 | 0.029889 | 0.048093 | — | — | — |
| access_q_test.test | RUN | 0.166859 | 0.350371 | 0.442945 | 0.001346 | 0.001385 | 0.001346 |
| access_q_fail.test | COMPILE_FAIL | 0.015078 | 0.025329 | 0.042938 | — | — | — |
| access_v_test.test | RUN | 0.162103 | 0.366296 | 0.428125 | 0.001271 | 0.001412 | 0.001390 |
| access_v_fail1.test | COMPILE_FAIL | 0.013570 | 0.023648 | 0.039674 | — | — | — |
| access_v_fail2.test | COMPILE_FAIL | 0.013617 | 0.023132 | 0.039551 | — | — | — |
| assign_test.test | RUN | 0.243829 | 0.568255 | 0.776036 | 0.001686 | 0.001519 | 0.001283 |
| to_string_test.test | RUN | 0.277985 | 0.910717 | 4.953536 | 0.001303 | 0.001461 | 0.001568 |
| cmp_vv_test.test | RUN | 0.196655 | 0.464528 | 0.527954 | 0.001334 | 0.001471 | 0.001262 |
| cross_test.test | RUN | 0.196874 | 0.437843 | 0.517567 | 0.001210 | 0.001532 | 0.001560 |
| div_eq_vs_test.test | RUN | 0.187468 | 0.392201 | 0.526302 | 0.001584 | 0.001414 | 0.001338 |
| div_vs_test.test | RUN | 0.193854 | 0.490863 | 0.555998 | 0.001280 | 0.001671 | 0.001412 |
| dot_vv_test.test | RUN | 0.186970 | 0.426678 | 0.502888 | 0.001133 | 0.001422 | 0.001499 |
| eq_vv_test.test | RUN | 0.204747 | 0.485060 | 0.549088 | 0.001117 | 0.001560 | 0.001402 |
| mag_v_test.test | RUN | 0.195141 | 0.396780 | 0.475932 | 0.001591 | 0.001551 | 0.001226 |
| mag_sqr_v_test.test | RUN | 0.189959 | 0.395917 | 0.498236 | 0.001435 | 0.001446 | 0.001345 |
| minus_v_test.test | RUN | 0.195425 | 0.465133 | 0.504728 | 0.001418 | 0.001471 | 0.001104 |
| minus_eq_vv_test.test | RUN | 0.199578 | 0.457844 | 0.506322 | 0.001250 | 0.001275 | 0.001385 |
| minus_vv_test.test | RUN | 0.198442 | 0.498431 | 0.540305 | 0.001402 | 0.001461 | 0.001512 |
| mul_eq_vs_test.test | RUN | 0.188150 | 0.385103 | 0.489107 | 0.001358 | 0.001572 | 0.001412 |
| mul_vs_test.test | RUN | 0.197677 | 0.490098 | 0.559795 | 0.001345 | 0.001611 | 0.003905 |
| mul_sv_test.test | RUN | 0.200187 | 0.487133 | 0.558839 | 0.001326 | 0.001581 | 0.001332 |
| mul_vm_test.test | RUN | 0.279142 | 0.853823 | 1.144961 | 0.001401 | 0.001465 | 0.003030 |
| normalize_v_test.test | RUN | 0.207701 | 0.614222 | 0.852222 | 0.001489 | 0.001489 | 0.001612 |
| plus_eq_vv_test.test | RUN | 0.197761 | 0.466688 | 0.813528 | 0.001364 | 0.001472 | 0.003031 |
| plus_vv_test.test | RUN | 0.206840 | 0.493085 | 0.787792 | 0.001335 | 0.001590 | 0.001190 |
| scalar_cast_v_test.test | RUN | 0.190998 | 0.364211 | 0.672761 | 0.001329 | 0.001421 | 0.001478 |
| vec_index_test.test | RUN | 0.165739 | 0.373734 | 0.436474 | 0.001241 | 0.001610 | 0.001260 |
| vec_register_test.test | RUN | 0.109656 | 0.335165 | 0.318140 | 0.001724 | 0.001462 | 0.001245 |
| cmp_mm_test.test | RUN | 0.317145 | 1.041424 | 1.086262 | 0.001612 | 0.001540 | 0.001573 |
| determinant_test.test | RUN | 0.226754 | 0.536118 | 0.605710 | 0.001344 | 0.001592 | 0.001545 |
| div_eq_ms_test.test | RUN | 0.233640 | 0.506637 | 0.690432 | 0.001460 | 0.001410 | 0.001589 |
| div_ms_test.test | RUN | 0.268612 | 0.802450 | 0.818513 | 0.001453 | 0.001602 | 0.001573 |
| eq_mm_test.test | RUN | 0.313808 | 0.976619 | 1.044846 | 0.001735 | 0.001389 | 0.001621 |
| minus_m_test.test | RUN | 0.265935 | 0.783765 | 0.780812 | 0.001408 | 0.001510 | 0.001477 |
| minus_eq_mm_test.test | RUN | 0.249902 | 0.742556 | 0.755230 | 0.001340 | 0.001473 | 0.001412 |
| minus_mm_test.test | RUN | 0.287992 | 0.923395 | 0.867302 | 0.001659 | 0.001614 | 0.001512 |
| mul_eq_mm_test.test | RUN | 0.248917 | 0.688201 | 0.657467 | 0.001676 | 0.001290 | 0.001308 |
| mul_eq_ms_test.test | RUN | 0.241231 | 0.505424 | 0.669772 | 0.001578 | 0.001406 | 0.001502 |
| mul_mm_test.test | RUN | 0.326134 | 1.504765 | 1.080642 | 0.001725 | 0.001446 | 0.001393 |
| mul_ms_test.test | RUN | 0.289314 | 0.805668 | 0.844945 | 0.001410 | 0.001462 | 0.001147 |
| mul_sm_test.test | RUN | 0.282671 | 0.792325 | 0.839172 | 0.001626 | 0.001396 | 0.001333 |
| mul_mv_test.test | RUN | 0.291880 | 0.854338 | 0.849325 | 0.001646 | 0.001351 | 0.001508 |
| inverse_m_test.test | RUN | 0.377314 | 1.576737 | 1.193797 | 0.001450 | 0.001506 | 0.001454 |
| plus_eq_mm_test.test | RUN | 0.264933 | 0.720932 | 0.741887 | 0.001700 | 0.001580 | 0.001497 |
| plus_mm_test.test | RUN | 0.294053 | 0.931834 | 0.854958 | 0.001444 | 0.001470 | 0.001470 |
| scalar_cast_m_test.test | RUN | 0.217692 | 0.426041 | 0.548908 | 0.001590 | 0.001532 | 0.001518 |
| mat_index_test.test | RUN | 0.177182 | 0.400777 | 0.471325 | 0.001435 | 0.001479 | 0.001392 |
| cmp_qq_test.test | RUN | 0.192550 | 0.372174 | 0.451214 | 0.001335 | 0.001330 | 0.001504 |
| conjugate_test.test | RUN | 0.192130 | 0.380621 | 0.448235 | 0.001634 | 0.001613 | 0.001342 |
| normalize_q_test.test | RUN | 0.197159 | 0.427863 | 0.484417 | 0.001609 | 0.001440 | 0.001560 |
| div_eq_qs_test.test | RUN | 0.195123 | 0.364593 | 0.460510 | 0.001613 | 0.001532 | 0.001261 |
| div_qs_test.test | RUN | 0.197821 | 0.391617 | 0.456958 | 0.001487 | 0.001451 | 0.001504 |
| dot_qq_test.test | RUN | 0.201647 | 0.369102 | 0.464985 | 0.001670 | 0.001656 | 0.001337 |
| eq_qq_test.test | RUN | 0.209235 | 0.394490 | 0.463334 | 0.001614 | 0.001434 | 0.001519 |
| inverse_q_test.test | RUN | 0.249960 | 0.520237 | 0.591761 | 0.001697 | 0.001488 | 0.001473 |
| mag_q_test.test | RUN | 0.206036 | 0.381417 | 0.461281 | 0.001390 | 0.001698 | 0.001329 |
| mag_sqr_q_test.test | RUN | 0.202832 | 0.382381 | 0.444354 | 0.001664 | 0.001622 | 0.001409 |
| minus_q_test.test | RUN | 0.199011 | 0.383057 | 0.461269 | 0.001524 | 0.001470 | 0.001507 |
| minus_eq_qq_test.test | RUN | 0.200191 | 0.383183 | 0.462867 | 0.001612 | 0.001544 | 0.001470 |
| minus_qq_test.test | RUN | 0.201778 | 0.396588 | 0.458929 | 0.001452 | 0.001500 | 0.001298 |
| mul_eq_qs_test.test | RUN | 0.195307 | 0.365498 | 0.448733 | 0.001606 | 0.001564 | 0.001427 |
| mul_qs_test.test | RUN | 0.191621 | 0.379451 | 0.460296 | 0.001700 | 0.001632 | 0.001424 |
| mul_qv_test.test | RUN | 0.235045 | 0.530488 | 0.595298 | 0.001564 | 0.001541 | 0.001411 |
| mul_qq_test.test | RUN | 0.235707 | 0.523464 | 0.584951 | 0.001406 | 0.001540 | 0.001425 |
| mul_eq_qq_test.test | RUN | 0.234269 | 0.533450 | 0.568118 | 0.001482 | 0.001444 | 0.001091 |
| plus_eq_qq_test.test | RUN | 0.188385 | 0.383285 | 0.453930 | 0.001573 | 0.001311 | 0.001513 |
| plus_qq_test.test | RUN | 0.194192 | 0.393988 | 0.473041 | 0.001760 | 0.001491 | 0.001447 |
| scalar_cast_q_test.test | RUN | 0.184862 | 0.372297 | 0.459856 | 0.001437 | 0.001500 | 0.001263 |
| slerp_test.test | RUN | 0.208810 | 0.725686 | 0.544852 | 0.013801 | 0.011582 | 0.012233 |
| convert_to_test.test | RUN | 0.282110 | 0.665383 | 0.880896 | 0.001536 | 0.001622 | 0.001437 |
| rot_mat_test.test | RUN | 0.420601 | 3.082612 | 2.698499 | 0.029490 | 0.029449 | 0.027166 |
| rot_quat_test.test | RUN | 0.239923 | 0.635362 | 0.605532 | 0.001738 | 0.001621 | 0.001510 |
| rotx_mat_test.test | RUN | 0.266900 | 0.826688 | 0.796500 | 0.001496 | 0.001626 | 0.001572 |
| rotx_quat_test.test | RUN | 0.233883 | 0.527404 | 0.573020 | 0.001555 | 0.001567 | 0.001438 |
| roty_mat_test.test | RUN | 0.258550 | 0.819766 | 0.808483 | 0.001600 | 0.001582 | 0.001453 |
| roty_quat_test.test | RUN | 0.220921 | 0.536286 | 0.573227 | 0.001549 | 0.001587 | 0.001327 |
| rotz_mat_test.test | RUN | 0.261446 | 0.935746 | 0.800376 | 0.001380 | 0.001551 | 0.001519 |
| rotz_quat_test.test | RUN | 0.220350 | 0.528293 | 0.583645 | 0.001314 | 0.003100 | 0.001295 |
| zero_vec_test.test | RUN | 0.193507 | 0.472719 | 0.503228 | 0.001572 | 0.001231 | 0.001273 |
| zero_mat_test.test | RUN | 0.231335 | 0.743275 | 0.814896 | 0.001304 | 0.001477 | 0.001293 |
| zero_quat_test.test | RUN | 0.185084 | 0.379428 | 0.558952 | 0.001398 | 0.001326 | 0.001308 |
| identity_mat_test.test | RUN | 0.218308 | 0.569180 | 0.696455 | 0.001478 | 0.001313 | 0.001413 |
| identity_quat_test.test | RUN | 0.183196 | 0.374411 | 0.566382 | 0.001539 | 0.001654 | 0.001371 |
| col_test.test | RUN | 0.343906 | 1.157163 | 1.337127 | 0.001337 | 0.001407 | 0.001347 |
| col_mat_test.test | RUN | 0.229595 | 0.518817 | 0.618126 | 0.001402 | 0.001590 | 0.001486 |
| neg_col_test.test | RUN | 0.267119 | 0.729072 | 0.783178 | 0.001383 | 0.001590 | 0.001300 |
| neg_row_test.test | RUN | 0.268404 | 0.754444 | 0.797131 | 0.001470 | 0.001512 | 0.001259 |
| del_col_test.test | RUN | 0.272275 | 0.825776 | 0.842876 | 0.001493 | 0.001564 | 0.001510 |
| del_row_test.test | RUN | 0.270935 | 0.806550 | 0.831455 | 0.001575 | 0.001616 | 0.001452 |
| del_row_col_test.test | RUN | 0.372273 | 1.553819 | 1.521187 | 0.001589 | 0.001543 | 0.001532 |
| swap_cols_test.test | RUN | 0.489828 | 2.077956 | 1.868899 | 0.001474 | 0.001422 | 0.001522 |
| swap_rows_test.test | RUN | 0.577829 | 2.167160 | 1.842637 | 0.001630 | 0.001450 | 0.001462 |
| diag_test.test | RUN | 0.231592 | 0.621373 | 0.697711 | 0.001567 | 0.001561 | 0.001542 |
| diag_mat_test.test | RUN | 0.240578 | 0.536245 | 0.636208 | 0.001526 | 0.001319 | 0.001477 |
| row_test.test | RUN | 0.343073 | 1.148240 | 1.335873 | 0.001206 | 0.001348 | 0.001506 |
| row_mat_test.test | RUN | 0.231655 | 0.510454 | 0.624783 | 0.001527 | 0.001446 | 0.001552 |
| transpose_test.test | RUN | 0.269637 | 0.784945 | 0.852117 | 0.001403 | 0.001435 | 0.001361 |
| translation_test.test | RUN | 0.218679 | 0.497668 | 0.657263 | 0.001370 | 0.001488 | 0.001240 |
| translation_mat_test.test | RUN | 0.241042 | 0.507295 | 0.692274 | 0.001413 | 0.001375 | 0.001146 |
| swizzle_const_fail.test | COMPILE_FAIL | 0.081793 | 0.152836 | 0.188605 | — | — | — |
| swizzle2_test2.test | RUN | 0.094424 | 0.224256 | 0.265213 | 0.001111 | 0.001502 | 0.001338 |
| swizzle2_test3.test | RUN | 0.101150 | 0.216101 | 0.267336 | 0.001269 | 0.001304 | 0.001268 |
| swizzle2_test4.test | RUN | 0.101797 | 0.220343 | 0.270709 | 0.001259 | 0.001527 | 0.001376 |
| swizzle2_test.test | RUN | 0.316516 | 0.615822 | 0.798716 | 0.001530 | 0.001434 | 0.001396 |
| swizzle3_test2.test | RUN | 0.109718 | 0.238552 | 0.284629 | 0.001376 | 0.001492 | 0.001309 |
| swizzle3_test3.test | RUN | 0.109077 | 0.243148 | 0.279185 | 0.001315 | 0.001504 | 0.001320 |
| swizzle3_test4.test | RUN | 0.112725 | 0.233329 | 0.287630 | 0.001336 | 0.001631 | 0.001212 |
| swizzle3_test.test | RUN | 0.321520 | 0.606378 | 0.816325 | 0.001520 | 0.001474 | 0.001588 |
| swizzle4_test2.test | RUN | 0.186152 | 0.335381 | 0.442479 | 0.001500 | 0.001519 | 0.001257 |
| swizzle4_test3.test | RUN | 0.183648 | 0.329622 | 0.439826 | 0.001532 | 0.001508 | 0.001227 |
| swizzle4_test4.test | RUN | 0.192456 | 0.339706 | 0.461658 | 0.001429 | 0.001382 | 0.001601 |
| swizzle4_test.test | RUN | 0.315512 | 0.662485 | 0.846051 | 0.001175 | 0.001437 | 0.001481 |
| deduce_scalar_test.test | COMPILE | 0.016596 | 0.034379 | 0.048336 | — | — | — |
| deduce_scalar_mq_test.test | RUN | 0.117745 | 0.255389 | 0.315892 | 0.001415 | 0.001393 | 0.001504 |
| deduce_scalar_mv_test.test | RUN | 0.142650 | 0.295040 | 0.357741 | 0.001312 | 0.001419 | 0.001277 |
| deduce_scalar_fail.test | COMPILE_FAIL | 0.013634 | 0.022283 | 0.039507 | — | — | — |
| deduce_mat_test.test | COMPILE | 0.014296 | 0.031272 | 0.045996 | — | — | — |
| deduce_quat_test.test | COMPILE | 0.014740 | 0.030325 | 0.046393 | — | — | — |
| deduce_vec_test.test | COMPILE | 0.014901 | 0.028375 | 0.040338 | — | — | — |
| interop_test.test | RUN | 0.130865 | 0.298994 | 0.522316 | 0.000530 | 0.000445 | 0.001110 |
| transform_test.test | RUN | 0.291829 | 0.530137 | 0.661128 | 0.001124 | 0.001083 | 0.001016 |
| projection_test.test | RUN | 0.196167 | 0.407354 | 0.534511 | 0.001159 | 0.001112 | 0.001086 |
| scalar_traits_test.test | COMPILE | 0.015977 | 0.031644 | 0.046113 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.987× | 0.991×, 0.982× | 0.968× |
| clang | 0.982× | 0.983×, 0.982× | 0.992× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026855).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| access_m_test.test | 10 | 0.000875s | 0.000857s | 0.000862s | byte equality observed |
| access_q_test.test | 10 | 0.000872s | 0.000845s | 0.000855s | byte equality observed |
| access_v_test.test | 10 | 0.000859s | 0.000870s | 0.000853s | byte equality observed |
| assign_test.test | 10 | 0.000865s | 0.000880s | 0.000867s | byte equality observed |
| cmp_mm_test.test | 10 | 0.000869s | 0.000867s | 0.000867s | byte equality observed |
| cmp_qq_test.test | 10 | 0.000852s | 0.000862s | 0.000874s | byte equality observed |
| cmp_vv_test.test | 10 | 0.000856s | 0.000859s | 0.000860s | byte equality observed |
| col_mat_test.test | 10 | 0.000853s | 0.000853s | 0.000892s | byte equality observed |
| col_test.test | 10 | 0.000861s | 0.000856s | 0.000846s | byte equality observed |
| conjugate_test.test | 10 | 0.000888s | 0.000867s | 0.000854s | byte equality observed |
| convert_to_test.test | 10 | 0.000875s | 0.000860s | 0.000867s | byte equality observed |
| cross_test.test | 10 | 0.000861s | 0.000855s | 0.000850s | byte equality observed |
| deduce_scalar_mq_test.test | 10 | 0.000876s | 0.000854s | 0.000860s | byte equality observed |
| deduce_scalar_mv_test.test | 10 | 0.000869s | 0.000845s | 0.000846s | byte equality observed |
| del_col_test.test | 10 | 0.000857s | 0.000853s | 0.000841s | byte equality observed |
| del_row_col_test.test | 10 | 0.000872s | 0.000874s | 0.000857s | byte equality observed |
| del_row_test.test | 10 | 0.000864s | 0.000852s | 0.000868s | byte equality observed |
| determinant_test.test | 10 | 0.000876s | 0.000856s | 0.000863s | byte equality observed |
| diag_mat_test.test | 10 | 0.000858s | 0.000863s | 0.000864s | byte equality observed |
| diag_test.test | 10 | 0.000844s | 0.000854s | 0.000871s | byte equality observed |
| div_eq_ms_test.test | 10 | 0.000861s | 0.000859s | 0.000843s | byte equality observed |
| div_eq_qs_test.test | 10 | 0.000851s | 0.000859s | 0.000847s | byte equality observed |
| div_eq_vs_test.test | 10 | 0.000860s | 0.000842s | 0.000868s | byte equality observed |
| div_ms_test.test | 10 | 0.000868s | 0.000847s | 0.000870s | byte equality observed |
| div_qs_test.test | 10 | 0.000861s | 0.000848s | 0.000861s | byte equality observed |
| div_vs_test.test | 10 | 0.000858s | 0.000858s | 0.000864s | byte equality observed |
| dot_qq_test.test | 10 | 0.000854s | 0.000856s | 0.000859s | byte equality observed |
| dot_vv_test.test | 10 | 0.000850s | 0.000844s | 0.000861s | byte equality observed |
| eq_mm_test.test | 10 | 0.000852s | 0.000853s | 0.000882s | byte equality observed |
| eq_qq_test.test | 10 | 0.000859s | 0.000845s | 0.000877s | byte equality observed |
| eq_vv_test.test | 10 | 0.000855s | 0.000851s | 0.000861s | byte equality observed |
| identity_mat_test.test | 10 | 0.000850s | 0.000860s | 0.000862s | byte equality observed |
| identity_quat_test.test | 10 | 0.000865s | 0.000860s | 0.000857s | byte equality observed |
| interop_test.test | 10 | 0.000390s | 0.000399s | 0.000839s | byte equality observed |
| inverse_m_test.test | 10 | 0.000903s | 0.000903s | 0.000916s | byte equality observed |
| inverse_q_test.test | 10 | 0.000892s | 0.000850s | 0.000870s | byte equality observed |
| mag_q_test.test | 10 | 0.000853s | 0.000845s | 0.000835s | byte equality observed |
| mag_sqr_q_test.test | 10 | 0.000855s | 0.000848s | 0.000849s | byte equality observed |
| mag_sqr_v_test.test | 10 | 0.000847s | 0.000858s | 0.000848s | byte equality observed |
| mag_v_test.test | 10 | 0.000858s | 0.000892s | 0.000869s | byte equality observed |
| mat_index_test.test | 10 | 0.000860s | 0.000870s | 0.000859s | byte equality observed |
| mat_traits_array_test.test | 10 | 0.000865s | 0.000861s | 0.000850s | byte equality observed |
| mat_traits_std_array_test.test | 10 | 0.000851s | 0.000852s | 0.000854s | byte equality observed |
| math_test.test | 10 | 0.001275s | 0.001214s | 0.001122s | byte equality observed |
| minus_eq_mm_test.test | 10 | 0.000849s | 0.000858s | 0.000861s | byte equality observed |
| minus_eq_qq_test.test | 10 | 0.000843s | 0.000874s | 0.000852s | byte equality observed |
| minus_eq_vv_test.test | 10 | 0.000857s | 0.000851s | 0.000853s | byte equality observed |
| minus_m_test.test | 10 | 0.000860s | 0.000862s | 0.000866s | byte equality observed |
| minus_mm_test.test | 10 | 0.000870s | 0.000861s | 0.000869s | byte equality observed |
| minus_q_test.test | 10 | 0.000849s | 0.000849s | 0.000880s | byte equality observed |
| minus_qq_test.test | 10 | 0.000869s | 0.000863s | 0.000846s | byte equality observed |
| minus_v_test.test | 10 | 0.000859s | 0.000864s | 0.000867s | byte equality observed |
| minus_vv_test.test | 10 | 0.000872s | 0.000859s | 0.000845s | byte equality observed |
| mul_eq_mm_test.test | 10 | 0.000861s | 0.000850s | 0.000856s | byte equality observed |
| mul_eq_ms_test.test | 10 | 0.000856s | 0.000855s | 0.000873s | byte equality observed |
| mul_eq_qq_test.test | 10 | 0.000888s | 0.000865s | 0.000865s | byte equality observed |
| mul_eq_qs_test.test | 10 | 0.000854s | 0.000848s | 0.000880s | byte equality observed |
| mul_eq_vs_test.test | 10 | 0.000845s | 0.000843s | 0.000862s | byte equality observed |
| mul_mm_test.test | 10 | 0.000862s | 0.000870s | 0.000855s | byte equality observed |
| mul_ms_test.test | 10 | 0.000860s | 0.000849s | 0.000835s | byte equality observed |
| mul_mv_test.test | 10 | 0.000858s | 0.000863s | 0.000852s | byte equality observed |
| mul_qq_test.test | 10 | 0.000868s | 0.000873s | 0.000853s | byte equality observed |
| mul_qs_test.test | 10 | 0.000870s | 0.000876s | 0.000842s | byte equality observed |
| mul_qv_test.test | 10 | 0.000879s | 0.000869s | 0.000857s | byte equality observed |
| mul_sm_test.test | 10 | 0.000872s | 0.000865s | 0.000842s | byte equality observed |
| mul_sv_test.test | 10 | 0.000851s | 0.000846s | 0.000854s | byte equality observed |
| mul_vm_test.test | 10 | 0.000861s | 0.000857s | 0.000855s | byte equality observed |
| mul_vs_test.test | 10 | 0.000850s | 0.000847s | 0.000869s | byte equality observed |
| neg_col_test.test | 10 | 0.000852s | 0.000857s | 0.000884s | byte equality observed |
| neg_row_test.test | 10 | 0.000858s | 0.000865s | 0.000850s | byte equality observed |
| normalize_q_test.test | 10 | 0.000860s | 0.000873s | 0.000871s | byte equality observed |
| normalize_v_test.test | 10 | 0.000862s | 0.000872s | 0.000857s | byte equality observed |
| plus_eq_mm_test.test | 10 | 0.000860s | 0.000856s | 0.000862s | byte equality observed |
| plus_eq_qq_test.test | 10 | 0.000847s | 0.000843s | 0.000845s | byte equality observed |
| plus_eq_vv_test.test | 10 | 0.000853s | 0.000845s | 0.000860s | byte equality observed |
| plus_mm_test.test | 10 | 0.000853s | 0.000842s | 0.000843s | byte equality observed |
| plus_qq_test.test | 10 | 0.000839s | 0.000847s | 0.000868s | byte equality observed |
| plus_vv_test.test | 10 | 0.000881s | 0.000845s | 0.000861s | byte equality observed |
| projection_test.test | 10 | 0.000860s | 0.000854s | 0.000861s | byte equality observed |
| quat_traits_array_test.test | 10 | 0.000880s | 0.000843s | 0.000861s | byte equality observed |
| quat_traits_std_array_test.test | 10 | 0.000849s | 0.000829s | 0.000840s | byte equality observed |
| rot_mat_test.test | 2 | 0.019834s | 0.019637s | 0.017890s | byte equality observed |
| rot_quat_test.test | 10 | 0.000819s | 0.000832s | 0.000826s | byte equality observed |
| rotx_mat_test.test | 10 | 0.000826s | 0.000835s | 0.000847s | byte equality observed |
| rotx_quat_test.test | 10 | 0.000813s | 0.000859s | 0.000844s | byte equality observed |
| roty_mat_test.test | 10 | 0.000885s | 0.000876s | 0.000885s | byte equality observed |
| roty_quat_test.test | 10 | 0.000865s | 0.000865s | 0.000884s | byte equality observed |
| rotz_mat_test.test | 10 | 0.000881s | 0.000900s | 0.000881s | byte equality observed |
| rotz_quat_test.test | 10 | 0.000876s | 0.000870s | 0.000868s | byte equality observed |
| row_mat_test.test | 10 | 0.000858s | 0.000886s | 0.000855s | byte equality observed |
| row_test.test | 10 | 0.000857s | 0.000851s | 0.000875s | byte equality observed |
| scalar_cast_m_test.test | 10 | 0.000868s | 0.000875s | 0.000854s | byte equality observed |
| scalar_cast_q_test.test | 10 | 0.000856s | 0.000864s | 0.000854s | byte equality observed |
| scalar_cast_v_test.test | 10 | 0.000829s | 0.000828s | 0.000833s | byte equality observed |
| slerp_test.test | 3 | 0.009600s | 0.007949s | 0.008663s | byte equality observed |
| swap_cols_test.test | 10 | 0.000844s | 0.000817s | 0.000818s | byte equality observed |
| swap_rows_test.test | 10 | 0.000869s | 0.000863s | 0.000843s | byte equality observed |
| swizzle2_test.test | 10 | 0.000868s | 0.000851s | 0.000873s | byte equality observed |
| swizzle2_test2.test | 10 | 0.000858s | 0.000859s | 0.000855s | byte equality observed |
| swizzle2_test3.test | 10 | 0.000841s | 0.000858s | 0.000856s | byte equality observed |
| swizzle2_test4.test | 10 | 0.000859s | 0.000852s | 0.000872s | byte equality observed |
| swizzle3_test.test | 10 | 0.000854s | 0.000854s | 0.000862s | byte equality observed |
| swizzle3_test2.test | 10 | 0.000864s | 0.000863s | 0.000866s | byte equality observed |
| swizzle3_test3.test | 10 | 0.000845s | 0.000864s | 0.000867s | byte equality observed |
| swizzle3_test4.test | 10 | 0.000863s | 0.000859s | 0.000860s | byte equality observed |
| swizzle4_test.test | 10 | 0.000853s | 0.000859s | 0.000868s | byte equality observed |
| swizzle4_test2.test | 10 | 0.000863s | 0.000849s | 0.000857s | byte equality observed |
| swizzle4_test3.test | 10 | 0.000859s | 0.000848s | 0.000852s | byte equality observed |
| swizzle4_test4.test | 10 | 0.000857s | 0.000872s | 0.000850s | byte equality observed |
| to_string_test.test | 10 | 0.000888s | 0.000890s | 0.000909s | byte equality observed |
| transform_test.test | 10 | 0.000893s | 0.000871s | 0.000859s | byte equality observed |
| translation_mat_test.test | 10 | 0.000849s | 0.000847s | 0.000873s | byte equality observed |
| translation_test.test | 10 | 0.000880s | 0.000857s | 0.000859s | byte equality observed |
| transpose_test.test | 10 | 0.000849s | 0.000863s | 0.000845s | byte equality observed |
| vec_index_test.test | 10 | 0.000861s | 0.000840s | 0.000869s | byte equality observed |
| vec_register_test.test | 10 | 0.000853s | 0.000888s | 0.000850s | byte equality observed |
| vec_traits_array_test.test | 10 | 0.000862s | 0.000853s | 0.000878s | byte equality observed |
| vec_traits_gnuc_test.test | 10 | 0.000849s | 0.000863s | 0.000854s | byte equality observed |
| vec_traits_std_array_test.test | 10 | 0.000858s | 0.000877s | 0.000869s | byte equality observed |
| zero_mat_test.test | 10 | 0.000863s | 0.000864s | 0.000849s | byte equality observed |
| zero_quat_test.test | 10 | 0.000860s | 0.000872s | 0.000853s | byte equality observed |
| zero_vec_test.test | 10 | 0.000863s | 0.000861s | 0.000871s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026855).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
