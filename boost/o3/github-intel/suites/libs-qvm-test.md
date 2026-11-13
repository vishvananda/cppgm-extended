# libs/qvm/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| math_test.test | RUN | 0.236776 | 0.654263 | 0.675681 | 0.001788 | 0.001664 | 0.001575 |
| mat_traits_array_test.test | RUN | 0.187043 | 0.648514 | 0.528998 | 0.001270 | 0.001299 | 0.001258 |
| mat_traits_std_array_test.test | RUN | 0.185537 | 0.501447 | 0.538183 | 0.001353 | 0.001253 | 0.001262 |
| vec_traits_array_test.test | RUN | 0.162266 | 0.505956 | 0.451738 | 0.001239 | 0.001353 | 0.001156 |
| vec_traits_gnuc_test.test | RUN | 0.289882 | 0.610540 | 0.841403 | 0.001321 | 0.001310 | 0.001100 |
| vec_traits_std_array_test.test | RUN | 0.156487 | 0.400094 | 0.452130 | 0.001348 | 0.001245 | 0.001145 |
| quat_traits_array_test.test | RUN | 0.154135 | 0.530277 | 0.439048 | 0.001039 | 0.001245 | 0.001215 |
| quat_traits_std_array_test.test | RUN | 0.153548 | 0.395350 | 0.434992 | 0.001219 | 0.001275 | 0.001137 |
| access_m_test.test | RUN | 0.250229 | 0.953371 | 1.026913 | 0.001297 | 0.001448 | 0.001220 |
| access_m_fail1.test | COMPILE_FAIL | 0.024327 | 0.041252 | 0.064141 | — | — | — |
| access_m_fail2.test | COMPILE_FAIL | 0.024648 | 0.041230 | 0.059707 | — | — | — |
| access_q_test.test | RUN | 0.207707 | 0.495095 | 0.606896 | 0.001303 | 0.001415 | 0.001205 |
| access_q_fail.test | COMPILE_FAIL | 0.018818 | 0.031857 | 0.050745 | — | — | — |
| access_v_test.test | RUN | 0.207630 | 0.499690 | 0.603698 | 0.001313 | 0.001321 | 0.001255 |
| access_v_fail1.test | COMPILE_FAIL | 0.016403 | 0.028717 | 0.046597 | — | — | — |
| access_v_fail2.test | COMPILE_FAIL | 0.016320 | 0.029741 | 0.047464 | — | — | — |
| assign_test.test | RUN | 0.319609 | 0.810711 | 1.091384 | 0.001312 | 0.001431 | 0.001232 |
| to_string_test.test | RUN | 0.373623 | 1.351674 | 6.387492 | 0.001375 | 0.001337 | 0.001268 |
| cmp_vv_test.test | RUN | 0.258923 | 0.655213 | 0.715885 | 0.001272 | 0.001420 | 0.001207 |
| cross_test.test | RUN | 0.247270 | 0.602148 | 0.709230 | 0.001346 | 0.001394 | 0.001203 |
| div_eq_vs_test.test | RUN | 0.243368 | 0.549479 | 0.700938 | 0.001371 | 0.001271 | 0.001307 |
| div_vs_test.test | RUN | 0.251200 | 0.679245 | 0.748001 | 0.001212 | 0.001488 | 0.001133 |
| dot_vv_test.test | RUN | 0.239889 | 0.612276 | 0.676507 | 0.001161 | 0.001308 | 0.001203 |
| eq_vv_test.test | RUN | 0.270528 | 0.680695 | 0.772327 | 0.001260 | 0.001522 | 0.001239 |
| mag_v_test.test | RUN | 0.236255 | 0.566539 | 0.670337 | 0.001590 | 0.001218 | 0.001038 |
| mag_sqr_v_test.test | RUN | 0.232327 | 0.556286 | 0.637214 | 0.001385 | 0.001371 | 0.001230 |
| minus_v_test.test | RUN | 0.239490 | 0.659372 | 0.712730 | 0.001385 | 0.001444 | 0.001237 |
| minus_eq_vv_test.test | RUN | 0.242483 | 0.629417 | 0.719803 | 0.001057 | 0.001401 | 0.001145 |
| minus_vv_test.test | RUN | 0.251822 | 0.688772 | 0.752421 | 0.001479 | 0.001504 | 0.001254 |
| mul_eq_vs_test.test | RUN | 0.238884 | 0.540724 | 0.714193 | 0.001298 | 0.001466 | 0.001203 |
| mul_vs_test.test | RUN | 0.251510 | 0.682158 | 0.765658 | 0.001332 | 0.001433 | 0.001230 |
| mul_sv_test.test | RUN | 0.253762 | 0.685687 | 0.731598 | 0.001340 | 0.001422 | 0.001217 |
| mul_vm_test.test | RUN | 0.364631 | 1.225833 | 1.177858 | 0.001277 | 0.001347 | 0.001340 |
| normalize_v_test.test | RUN | 0.254876 | 0.832025 | 0.773363 | 0.001299 | 0.001463 | 0.001321 |
| plus_eq_vv_test.test | RUN | 0.237856 | 0.633999 | 0.659106 | 0.001336 | 0.001342 | 0.001207 |
| plus_vv_test.test | RUN | 0.249540 | 0.679838 | 0.720047 | 0.000929 | 0.001449 | 0.001194 |
| scalar_cast_v_test.test | RUN | 0.230257 | 0.496429 | 0.640942 | 0.001192 | 0.001447 | 0.001152 |
| vec_index_test.test | RUN | 0.202231 | 0.512022 | 0.586770 | 0.001394 | 0.001322 | 0.001181 |
| vec_register_test.test | RUN | 0.135524 | 0.468448 | 0.431295 | 0.001353 | 0.001422 | 0.001262 |
| cmp_mm_test.test | RUN | 0.405225 | 1.504946 | 1.543461 | 0.001475 | 0.001413 | 0.001290 |
| determinant_test.test | RUN | 0.283969 | 0.763700 | 0.821203 | 0.001383 | 0.001412 | 0.001253 |
| div_eq_ms_test.test | RUN | 0.291580 | 0.708892 | 0.950750 | 0.001397 | 0.001470 | 0.001260 |
| div_ms_test.test | RUN | 0.340512 | 1.125543 | 1.132777 | 0.001254 | 0.001390 | 0.001242 |
| eq_mm_test.test | RUN | 0.390554 | 1.387896 | 1.471412 | 0.001331 | 0.001394 | 0.001170 |
| minus_m_test.test | RUN | 0.338920 | 1.108020 | 1.109917 | 0.001368 | 0.001316 | 0.001350 |
| minus_eq_mm_test.test | RUN | 0.316906 | 1.033604 | 1.051529 | 0.001356 | 0.001410 | 0.001317 |
| minus_mm_test.test | RUN | 0.354398 | 1.316032 | 1.237290 | 0.001359 | 0.001381 | 0.001305 |
| mul_eq_mm_test.test | RUN | 0.291980 | 0.966541 | 0.924896 | 0.001284 | 0.001459 | 0.001232 |
| mul_eq_ms_test.test | RUN | 0.287831 | 0.692853 | 0.944720 | 0.001309 | 0.001411 | 0.001212 |
| mul_mm_test.test | RUN | 0.412042 | 2.161179 | 1.528865 | 0.001336 | 0.001458 | 0.001102 |
| mul_ms_test.test | RUN | 0.341289 | 1.137149 | 1.177676 | 0.001305 | 0.001294 | 0.001260 |
| mul_sm_test.test | RUN | 0.352541 | 1.126048 | 1.167590 | 0.001376 | 0.001342 | 0.001189 |
| mul_mv_test.test | RUN | 0.373978 | 1.226798 | 1.171316 | 0.001279 | 0.001450 | 0.001140 |
| inverse_m_test.test | RUN | 0.472481 | 2.262070 | 1.681137 | 0.001635 | 0.001512 | 0.001258 |
| plus_eq_mm_test.test | RUN | 0.320439 | 1.051190 | 1.051304 | 0.001306 | 0.001424 | 0.001283 |
| plus_mm_test.test | RUN | 0.351607 | 1.342395 | 1.218966 | 0.001373 | 0.001371 | 0.001069 |
| scalar_cast_m_test.test | RUN | 0.275465 | 0.587652 | 0.768671 | 0.001442 | 0.001756 | 0.001356 |
| mat_index_test.test | RUN | 0.211564 | 0.563064 | 0.649975 | 0.001284 | 0.001362 | 0.001245 |
| cmp_qq_test.test | RUN | 0.230201 | 0.524533 | 0.621916 | 0.001360 | 0.001305 | 0.001233 |
| conjugate_test.test | RUN | 0.234308 | 0.531374 | 0.643770 | 0.001173 | 0.001489 | 0.001314 |
| normalize_q_test.test | RUN | 0.240416 | 0.596549 | 0.667612 | 0.001340 | 0.001265 | 0.001414 |
| div_eq_qs_test.test | RUN | 0.230601 | 0.512196 | 0.642407 | 0.001294 | 0.001212 | 0.001288 |
| div_qs_test.test | RUN | 0.231768 | 0.534259 | 0.673289 | 0.001421 | 0.001495 | 0.001204 |
| dot_qq_test.test | RUN | 0.235212 | 0.518390 | 0.671537 | 0.001709 | 0.001470 | 0.001169 |
| eq_qq_test.test | RUN | 0.243889 | 0.551689 | 0.661559 | 0.001412 | 0.001366 | 0.001134 |
| inverse_q_test.test | RUN | 0.292333 | 0.719888 | 0.811234 | 0.001378 | 0.001324 | 0.001216 |
| mag_q_test.test | RUN | 0.242883 | 0.526339 | 0.637515 | 0.001320 | 0.001525 | 0.001218 |
| mag_sqr_q_test.test | RUN | 0.228707 | 0.530207 | 0.634366 | 0.001486 | 0.001360 | 0.001194 |
| minus_q_test.test | RUN | 0.238599 | 0.540264 | 0.629052 | 0.001309 | 0.001416 | 0.001203 |
| minus_eq_qq_test.test | RUN | 0.231208 | 0.535279 | 0.630941 | 0.001354 | 0.001655 | 0.001236 |
| minus_qq_test.test | RUN | 0.234314 | 0.548681 | 0.638197 | 0.001334 | 0.001350 | 0.001257 |
| mul_eq_qs_test.test | RUN | 0.231965 | 0.500213 | 0.648073 | 0.001318 | 0.001450 | 0.001206 |
| mul_qs_test.test | RUN | 0.236881 | 0.543106 | 0.648308 | 0.001367 | 0.001582 | 0.001159 |
| mul_qv_test.test | RUN | 0.290853 | 0.740626 | 0.834550 | 0.001389 | 0.001612 | 0.001276 |
| mul_qq_test.test | RUN | 0.282282 | 0.751964 | 0.820843 | 0.001371 | 0.001339 | 0.001288 |
| mul_eq_qq_test.test | RUN | 0.289418 | 0.753703 | 0.812539 | 0.001445 | 0.001265 | 0.001221 |
| plus_eq_qq_test.test | RUN | 0.236218 | 0.548153 | 0.628025 | 0.001359 | 0.001251 | 0.001263 |
| plus_qq_test.test | RUN | 0.236632 | 0.546842 | 0.643236 | 0.001390 | 0.001334 | 0.001226 |
| scalar_cast_q_test.test | RUN | 0.238400 | 0.506405 | 0.642976 | 0.001295 | 0.001392 | 0.001164 |
| slerp_test.test | RUN | 0.255253 | 1.035258 | 0.781486 | 0.020281 | 0.015001 | 0.015788 |
| convert_to_test.test | RUN | 0.351181 | 0.954781 | 1.226537 | 0.001383 | 0.001459 | 0.001251 |
| rot_mat_test.test | RUN | 0.588822 | 4.426488 | 3.881611 | 0.078409 | 0.038220 | 0.040458 |
| rot_quat_test.test | RUN | 0.293587 | 0.879920 | 0.846591 | 0.001451 | 0.001381 | 0.001227 |
| rotx_mat_test.test | RUN | 0.328394 | 1.158442 | 1.145618 | 0.001568 | 0.001512 | 0.001381 |
| rotx_quat_test.test | RUN | 0.288001 | 0.748629 | 0.789362 | 0.001329 | 0.001390 | 0.001378 |
| roty_mat_test.test | RUN | 0.336237 | 1.152338 | 1.085169 | 0.001508 | 0.001365 | 0.001221 |
| roty_quat_test.test | RUN | 0.288310 | 0.747264 | 0.805594 | 0.001406 | 0.001444 | 0.001286 |
| rotz_mat_test.test | RUN | 0.346865 | 1.320289 | 1.200436 | 0.001400 | 0.001446 | 0.001266 |
| rotz_quat_test.test | RUN | 0.287553 | 0.732294 | 0.755170 | 0.001133 | 0.001332 | 0.001246 |
| zero_vec_test.test | RUN | 0.239657 | 0.639252 | 0.675173 | 0.001341 | 0.001343 | 0.001235 |
| zero_mat_test.test | RUN | 0.296753 | 1.061333 | 0.983338 | 0.001360 | 0.001498 | 0.001264 |
| zero_quat_test.test | RUN | 0.228422 | 0.540747 | 0.611987 | 0.001326 | 0.001311 | 0.001232 |
| identity_mat_test.test | RUN | 0.279974 | 0.773196 | 0.831227 | 0.001309 | 0.001373 | 0.001194 |
| identity_quat_test.test | RUN | 0.231606 | 0.506045 | 0.639563 | 0.001274 | 0.001474 | 0.001234 |
| col_test.test | RUN | 0.454199 | 1.661420 | 1.863333 | 0.001392 | 0.001336 | 0.001137 |
| col_mat_test.test | RUN | 0.290571 | 0.736950 | 0.865769 | 0.001360 | 0.001418 | 0.001264 |
| neg_col_test.test | RUN | 0.344782 | 1.045470 | 1.118657 | 0.001318 | 0.001510 | 0.001324 |
| neg_row_test.test | RUN | 0.360287 | 1.055637 | 1.131304 | 0.001314 | 0.001328 | 0.001220 |
| del_col_test.test | RUN | 0.362968 | 1.174667 | 1.178453 | 0.001394 | 0.001466 | 0.001227 |
| del_row_test.test | RUN | 0.363253 | 1.134947 | 1.172112 | 0.001368 | 0.001369 | 0.001278 |
| del_row_col_test.test | RUN | 0.491033 | 2.125327 | 2.123298 | 0.001413 | 0.001434 | 0.001217 |
| swap_cols_test.test | RUN | 0.698925 | 2.895765 | 2.615462 | 0.001515 | 0.001390 | 0.001230 |
| swap_rows_test.test | RUN | 0.774304 | 3.035164 | 2.640118 | 0.001570 | 0.001376 | 0.001249 |
| diag_test.test | RUN | 0.310970 | 0.764366 | 1.006623 | 0.001391 | 0.001307 | 0.001175 |
| diag_mat_test.test | RUN | 0.307775 | 0.749722 | 0.875345 | 0.001487 | 0.001454 | 0.001261 |
| row_test.test | RUN | 0.464034 | 1.657327 | 1.831316 | 0.001278 | 0.001511 | 0.001204 |
| row_mat_test.test | RUN | 0.295537 | 0.744672 | 0.873601 | 0.001420 | 0.001340 | 0.001311 |
| transpose_test.test | RUN | 0.362758 | 1.107842 | 1.179114 | 0.001380 | 0.001348 | 0.001176 |
| translation_test.test | RUN | 0.283320 | 0.695064 | 0.868873 | 0.001325 | 0.001444 | 0.001260 |
| translation_mat_test.test | RUN | 0.314991 | 0.707800 | 0.942745 | 0.001498 | 0.001343 | 0.001242 |
| swizzle_const_fail.test | COMPILE_FAIL | 0.102363 | 0.205938 | 0.247291 | — | — | — |
| swizzle2_test2.test | RUN | 0.125139 | 0.298643 | 0.354829 | 0.001300 | 0.001356 | 0.001137 |
| swizzle2_test3.test | RUN | 0.125956 | 0.296626 | 0.357925 | 0.001317 | 0.001410 | 0.001164 |
| swizzle2_test4.test | RUN | 0.127190 | 0.301613 | 0.343657 | 0.001270 | 0.001432 | 0.001217 |
| swizzle2_test.test | RUN | 0.405559 | 0.845318 | 1.106982 | 0.001359 | 0.001307 | 0.001294 |
| swizzle3_test2.test | RUN | 0.141591 | 0.319648 | 0.388418 | 0.001055 | 0.001405 | 0.001223 |
| swizzle3_test3.test | RUN | 0.142834 | 0.331171 | 0.389174 | 0.001562 | 0.001300 | 0.001229 |
| swizzle3_test4.test | RUN | 0.139775 | 0.321858 | 0.388977 | 0.001254 | 0.001353 | 0.001212 |
| swizzle3_test.test | RUN | 0.398740 | 0.854889 | 1.142243 | 0.001214 | 0.001309 | 0.001167 |
| swizzle4_test2.test | RUN | 0.238399 | 0.461498 | 0.619926 | 0.001265 | 0.001435 | 0.001230 |
| swizzle4_test3.test | RUN | 0.247090 | 0.470407 | 0.652817 | 0.001334 | 0.001582 | 0.001276 |
| swizzle4_test4.test | RUN | 0.242594 | 0.471626 | 0.609159 | 0.001249 | 0.001297 | 0.001257 |
| swizzle4_test.test | RUN | 0.399702 | 0.928344 | 1.152485 | 0.001130 | 0.001432 | 0.001262 |
| deduce_scalar_test.test | COMPILE | 0.020417 | 0.044662 | 0.059033 | — | — | — |
| deduce_scalar_mq_test.test | RUN | 0.149748 | 0.352629 | 0.404729 | 0.001331 | 0.001266 | 0.001108 |
| deduce_scalar_mv_test.test | RUN | 0.183788 | 0.410831 | 0.516116 | 0.001252 | 0.001373 | 0.001241 |
| deduce_scalar_fail.test | COMPILE_FAIL | 0.016274 | 0.029126 | 0.045357 | — | — | — |
| deduce_mat_test.test | COMPILE | 0.017643 | 0.039309 | 0.050313 | — | — | — |
| deduce_quat_test.test | COMPILE | 0.017464 | 0.038543 | 0.049973 | — | — | — |
| deduce_vec_test.test | COMPILE | 0.017902 | 0.039554 | 0.051336 | — | — | — |
| interop_test.test | RUN | 0.191354 | 0.523740 | 0.719672 | 0.000618 | 0.000634 | 0.001284 |
| transform_test.test | RUN | 0.328804 | 0.690401 | 0.964409 | 0.001080 | 0.001039 | 0.000925 |
| projection_test.test | RUN | 0.224078 | 0.583678 | 0.590896 | 0.001042 | 0.000929 | 0.000972 |
| scalar_traits_test.test | COMPILE | 0.018363 | 0.041214 | 0.056515 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.814× | 0.820×, 0.808× | 1.013× |
| clang | 0.832× | 0.842×, 0.821× | 0.988× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345325).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| access_m_test.test | 10 | 0.000888s | 0.000884s | 0.000894s | byte equality observed |
| access_q_test.test | 10 | 0.000860s | 0.000888s | 0.000875s | byte equality observed |
| access_v_test.test | 10 | 0.000882s | 0.000904s | 0.000908s | byte equality observed |
| assign_test.test | 10 | 0.000870s | 0.000890s | 0.000875s | byte equality observed |
| cmp_mm_test.test | 10 | 0.000898s | 0.000902s | 0.000905s | byte equality observed |
| cmp_qq_test.test | 10 | 0.000893s | 0.000926s | 0.000921s | byte equality observed |
| cmp_vv_test.test | 10 | 0.000898s | 0.000892s | 0.000871s | byte equality observed |
| col_mat_test.test | 10 | 0.000904s | 0.000885s | 0.000862s | byte equality observed |
| col_test.test | 10 | 0.000911s | 0.000908s | 0.000858s | byte equality observed |
| conjugate_test.test | 10 | 0.000875s | 0.000882s | 0.000897s | byte equality observed |
| convert_to_test.test | 10 | 0.000871s | 0.000874s | 0.000906s | byte equality observed |
| cross_test.test | 10 | 0.000886s | 0.000912s | 0.000863s | byte equality observed |
| deduce_scalar_mq_test.test | 10 | 0.000889s | 0.000904s | 0.000902s | byte equality observed |
| deduce_scalar_mv_test.test | 10 | 0.000879s | 0.000912s | 0.000892s | byte equality observed |
| del_col_test.test | 10 | 0.000877s | 0.000888s | 0.000887s | byte equality observed |
| del_row_col_test.test | 10 | 0.000877s | 0.000899s | 0.000881s | byte equality observed |
| del_row_test.test | 10 | 0.000877s | 0.000919s | 0.000875s | byte equality observed |
| determinant_test.test | 10 | 0.000890s | 0.000909s | 0.000866s | byte equality observed |
| diag_mat_test.test | 10 | 0.000884s | 0.000923s | 0.000889s | byte equality observed |
| diag_test.test | 10 | 0.000891s | 0.000871s | 0.000893s | byte equality observed |
| div_eq_ms_test.test | 10 | 0.000879s | 0.000898s | 0.000858s | byte equality observed |
| div_eq_qs_test.test | 10 | 0.000866s | 0.000892s | 0.000879s | byte equality observed |
| div_eq_vs_test.test | 10 | 0.000875s | 0.000903s | 0.000892s | byte equality observed |
| div_ms_test.test | 10 | 0.000889s | 0.000900s | 0.000858s | byte equality observed |
| div_qs_test.test | 10 | 0.000889s | 0.000908s | 0.000903s | byte equality observed |
| div_vs_test.test | 10 | 0.000874s | 0.000881s | 0.000858s | byte equality observed |
| dot_qq_test.test | 10 | 0.000898s | 0.000879s | 0.000866s | byte equality observed |
| dot_vv_test.test | 10 | 0.000917s | 0.000899s | 0.000859s | byte equality observed |
| eq_mm_test.test | 10 | 0.000901s | 0.000895s | 0.000892s | byte equality observed |
| eq_qq_test.test | 10 | 0.000878s | 0.000915s | 0.000875s | byte equality observed |
| eq_vv_test.test | 10 | 0.000890s | 0.000910s | 0.000869s | byte equality observed |
| identity_mat_test.test | 10 | 0.000898s | 0.000864s | 0.000882s | byte equality observed |
| identity_quat_test.test | 10 | 0.000878s | 0.000896s | 0.000901s | byte equality observed |
| interop_test.test | 10 | 0.000395s | 0.000407s | 0.000871s | byte equality observed |
| inverse_m_test.test | 10 | 0.000938s | 0.000947s | 0.000928s | byte equality observed |
| inverse_q_test.test | 10 | 0.000887s | 0.000897s | 0.000897s | byte equality observed |
| mag_q_test.test | 10 | 0.000871s | 0.000884s | 0.000893s | byte equality observed |
| mag_sqr_q_test.test | 10 | 0.000883s | 0.000874s | 0.000897s | byte equality observed |
| mag_sqr_v_test.test | 10 | 0.000874s | 0.000896s | 0.000893s | byte equality observed |
| mag_v_test.test | 10 | 0.000872s | 0.000877s | 0.000932s | byte equality observed |
| mat_index_test.test | 10 | 0.000893s | 0.000878s | 0.000919s | byte equality observed |
| mat_traits_array_test.test | 10 | 0.000896s | 0.000868s | 0.000891s | byte equality observed |
| mat_traits_std_array_test.test | 10 | 0.000897s | 0.000892s | 0.000871s | byte equality observed |
| math_test.test | 10 | 0.001326s | 0.001284s | 0.001182s | byte equality observed |
| minus_eq_mm_test.test | 10 | 0.000874s | 0.000886s | 0.000868s | byte equality observed |
| minus_eq_qq_test.test | 10 | 0.000886s | 0.000881s | 0.000874s | byte equality observed |
| minus_eq_vv_test.test | 10 | 0.000866s | 0.000871s | 0.000893s | byte equality observed |
| minus_m_test.test | 10 | 0.000900s | 0.000880s | 0.000869s | byte equality observed |
| minus_mm_test.test | 10 | 0.000903s | 0.000876s | 0.000900s | byte equality observed |
| minus_q_test.test | 10 | 0.000884s | 0.000911s | 0.000869s | byte equality observed |
| minus_qq_test.test | 10 | 0.000891s | 0.000890s | 0.000873s | byte equality observed |
| minus_v_test.test | 10 | 0.000885s | 0.000892s | 0.000883s | byte equality observed |
| minus_vv_test.test | 10 | 0.000877s | 0.000897s | 0.000886s | byte equality observed |
| mul_eq_mm_test.test | 10 | 0.000893s | 0.000874s | 0.000878s | byte equality observed |
| mul_eq_ms_test.test | 10 | 0.000884s | 0.000873s | 0.000902s | byte equality observed |
| mul_eq_qq_test.test | 10 | 0.000895s | 0.000880s | 0.000882s | byte equality observed |
| mul_eq_qs_test.test | 10 | 0.000876s | 0.000881s | 0.000892s | byte equality observed |
| mul_eq_vs_test.test | 10 | 0.000890s | 0.000868s | 0.000880s | byte equality observed |
| mul_mm_test.test | 10 | 0.000889s | 0.000897s | 0.000885s | byte equality observed |
| mul_ms_test.test | 10 | 0.000902s | 0.000895s | 0.000923s | byte equality observed |
| mul_mv_test.test | 10 | 0.000891s | 0.000909s | 0.000895s | byte equality observed |
| mul_qq_test.test | 10 | 0.000898s | 0.000889s | 0.000888s | byte equality observed |
| mul_qs_test.test | 10 | 0.000901s | 0.000898s | 0.000896s | byte equality observed |
| mul_qv_test.test | 10 | 0.000898s | 0.000915s | 0.000890s | byte equality observed |
| mul_sm_test.test | 10 | 0.000894s | 0.000892s | 0.000883s | byte equality observed |
| mul_sv_test.test | 9 | 0.000871s | 0.000891s | 0.000894s | byte equality observed |
| mul_vm_test.test | 10 | 0.000882s | 0.000907s | 0.000860s | byte equality observed |
| mul_vs_test.test | 10 | 0.000885s | 0.000882s | 0.000911s | byte equality observed |
| neg_col_test.test | 10 | 0.000898s | 0.000869s | 0.000892s | byte equality observed |
| neg_row_test.test | 10 | 0.000894s | 0.000894s | 0.000901s | byte equality observed |
| normalize_q_test.test | 10 | 0.000923s | 0.000879s | 0.000907s | byte equality observed |
| normalize_v_test.test | 10 | 0.000890s | 0.000879s | 0.000887s | byte equality observed |
| plus_eq_mm_test.test | 10 | 0.000874s | 0.000902s | 0.000901s | byte equality observed |
| plus_eq_qq_test.test | 10 | 0.000896s | 0.000907s | 0.000887s | byte equality observed |
| plus_eq_vv_test.test | 10 | 0.000910s | 0.000926s | 0.000878s | byte equality observed |
| plus_mm_test.test | 10 | 0.000896s | 0.000871s | 0.000900s | byte equality observed |
| plus_qq_test.test | 10 | 0.000858s | 0.000859s | 0.000874s | byte equality observed |
| plus_vv_test.test | 10 | 0.000880s | 0.000908s | 0.000876s | byte equality observed |
| projection_test.test | 10 | 0.000885s | 0.000903s | 0.000849s | byte equality observed |
| quat_traits_array_test.test | 10 | 0.000878s | 0.000910s | 0.000881s | byte equality observed |
| quat_traits_std_array_test.test | 10 | 0.000868s | 0.000860s | 0.000870s | byte equality observed |
| rot_mat_test.test | 1 | 0.053972s | 0.024827s | 0.028147s | byte equality observed |
| rot_quat_test.test | 10 | 0.000846s | 0.000842s | 0.000853s | byte equality observed |
| rotx_mat_test.test | 10 | 0.000849s | 0.000861s | 0.000898s | byte equality observed |
| rotx_quat_test.test | 10 | 0.000843s | 0.000935s | 0.000910s | byte equality observed |
| roty_mat_test.test | 10 | 0.000881s | 0.000935s | 0.000905s | byte equality observed |
| roty_quat_test.test | 10 | 0.000940s | 0.000879s | 0.000916s | byte equality observed |
| rotz_mat_test.test | 10 | 0.000942s | 0.000912s | 0.000890s | byte equality observed |
| rotz_quat_test.test | 10 | 0.000922s | 0.000892s | 0.000927s | byte equality observed |
| row_mat_test.test | 10 | 0.000907s | 0.000883s | 0.000903s | byte equality observed |
| row_test.test | 10 | 0.000874s | 0.000877s | 0.000934s | byte equality observed |
| scalar_cast_m_test.test | 10 | 0.000874s | 0.000880s | 0.000920s | byte equality observed |
| scalar_cast_q_test.test | 10 | 0.000884s | 0.000873s | 0.000888s | byte equality observed |
| scalar_cast_v_test.test | 10 | 0.000878s | 0.000843s | 0.000862s | byte equality observed |
| slerp_test.test | 2 | 0.014461s | 0.010790s | 0.011314s | byte equality observed |
| swap_cols_test.test | 10 | 0.000809s | 0.000811s | 0.000816s | byte equality observed |
| swap_rows_test.test | 10 | 0.000844s | 0.000910s | 0.000850s | byte equality observed |
| swizzle2_test.test | 10 | 0.000860s | 0.000879s | 0.000895s | byte equality observed |
| swizzle2_test2.test | 10 | 0.000872s | 0.000874s | 0.000903s | byte equality observed |
| swizzle2_test3.test | 10 | 0.000881s | 0.000888s | 0.000883s | byte equality observed |
| swizzle2_test4.test | 10 | 0.000890s | 0.000882s | 0.000888s | byte equality observed |
| swizzle3_test.test | 10 | 0.000902s | 0.000888s | 0.000895s | byte equality observed |
| swizzle3_test2.test | 10 | 0.000901s | 0.000886s | 0.000884s | byte equality observed |
| swizzle3_test3.test | 10 | 0.000901s | 0.000913s | 0.000914s | byte equality observed |
| swizzle3_test4.test | 10 | 0.000911s | 0.000880s | 0.000922s | byte equality observed |
| swizzle4_test.test | 10 | 0.000876s | 0.000887s | 0.000897s | byte equality observed |
| swizzle4_test2.test | 10 | 0.000863s | 0.000863s | 0.000889s | byte equality observed |
| swizzle4_test3.test | 10 | 0.000890s | 0.000892s | 0.000870s | byte equality observed |
| swizzle4_test4.test | 10 | 0.000887s | 0.000881s | 0.000891s | byte equality observed |
| to_string_test.test | 10 | 0.000938s | 0.000917s | 0.000934s | byte equality observed |
| transform_test.test | 10 | 0.000889s | 0.000886s | 0.000879s | byte equality observed |
| translation_mat_test.test | 10 | 0.000892s | 0.000883s | 0.000875s | byte equality observed |
| translation_test.test | 10 | 0.000902s | 0.000864s | 0.000887s | byte equality observed |
| transpose_test.test | 10 | 0.000909s | 0.000889s | 0.000889s | byte equality observed |
| vec_index_test.test | 10 | 0.000886s | 0.000863s | 0.000879s | byte equality observed |
| vec_register_test.test | 10 | 0.000907s | 0.000901s | 0.000869s | byte equality observed |
| vec_traits_array_test.test | 10 | 0.000891s | 0.000898s | 0.000875s | byte equality observed |
| vec_traits_gnuc_test.test | 10 | 0.000895s | 0.000879s | 0.000882s | byte equality observed |
| vec_traits_std_array_test.test | 10 | 0.000906s | 0.000902s | 0.000861s | byte equality observed |
| zero_mat_test.test | 10 | 0.000880s | 0.000892s | 0.000871s | byte equality observed |
| zero_quat_test.test | 10 | 0.000878s | 0.000923s | 0.000888s | byte equality observed |
| zero_vec_test.test | 10 | 0.000917s | 0.000869s | 0.000896s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345325).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
