# libs/qvm/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| math_test.test | RUN | 0.192196 | 0.550718 | 0.597590 | 0.001538 | 0.001489 | 0.001596 |
| mat_traits_array_test.test | RUN | 0.147693 | 0.548014 | 0.466933 | 0.001321 | 0.001128 | 0.001054 |
| mat_traits_std_array_test.test | RUN | 0.148081 | 0.418153 | 0.465138 | 0.001179 | 0.001408 | 0.000948 |
| vec_traits_array_test.test | RUN | 0.126989 | 0.424318 | 0.395703 | 0.001178 | 0.001111 | 0.001156 |
| vec_traits_gnuc_test.test | RUN | 0.238009 | 0.502215 | 0.762255 | 0.001041 | 0.001104 | 0.001214 |
| vec_traits_std_array_test.test | RUN | 0.126353 | 0.320144 | 0.389575 | 0.001246 | 0.001115 | 0.001116 |
| quat_traits_array_test.test | RUN | 0.123150 | 0.445759 | 0.390503 | 0.001195 | 0.001076 | 0.001255 |
| quat_traits_std_array_test.test | RUN | 0.124053 | 0.335897 | 0.387624 | 0.001045 | 0.001189 | 0.001224 |
| access_m_test.test | RUN | 0.207528 | 0.778791 | 0.884331 | 0.001126 | 0.001280 | 0.001202 |
| access_m_fail1.test | COMPILE_FAIL | 0.019418 | 0.030865 | 0.057478 | — | — | — |
| access_m_fail2.test | COMPILE_FAIL | 0.019768 | 0.030045 | 0.058999 | — | — | — |
| access_q_test.test | RUN | 0.168843 | 0.405601 | 0.525126 | 0.001119 | 0.001092 | 0.001166 |
| access_q_fail.test | COMPILE_FAIL | 0.014520 | 0.027006 | 0.048404 | — | — | — |
| access_v_test.test | RUN | 0.170101 | 0.408689 | 0.529372 | 0.001044 | 0.001044 | 0.001113 |
| access_v_fail1.test | COMPILE_FAIL | 0.013286 | 0.022907 | 0.043066 | — | — | — |
| access_v_fail2.test | COMPILE_FAIL | 0.012893 | 0.023488 | 0.045393 | — | — | — |
| assign_test.test | RUN | 0.256913 | 0.665741 | 0.973596 | 0.001106 | 0.001216 | 0.001141 |
| to_string_test.test | RUN | 0.299956 | 1.148497 | 5.668681 | 0.001029 | 0.001292 | 0.001457 |
| cmp_vv_test.test | RUN | 0.212124 | 0.530517 | 0.632008 | 0.001128 | 0.001114 | 0.001290 |
| cross_test.test | RUN | 0.198223 | 0.495540 | 0.636138 | 0.001178 | 0.001186 | 0.001393 |
| div_eq_vs_test.test | RUN | 0.191142 | 0.455397 | 0.637155 | 0.001195 | 0.001123 | 0.001424 |
| div_vs_test.test | RUN | 0.203457 | 0.558199 | 0.680533 | 0.001172 | 0.001119 | 0.001283 |
| dot_vv_test.test | RUN | 0.189054 | 0.505973 | 0.605211 | 0.001103 | 0.001172 | 0.001133 |
| eq_vv_test.test | RUN | 0.220431 | 0.571024 | 0.694454 | 0.001179 | 0.001206 | 0.001111 |
| mag_v_test.test | RUN | 0.196069 | 0.472995 | 0.578211 | 0.001176 | 0.001119 | 0.001233 |
| mag_sqr_v_test.test | RUN | 0.202417 | 0.452288 | 0.609728 | 0.001239 | 0.001150 | 0.001366 |
| minus_v_test.test | RUN | 0.203744 | 0.537712 | 0.628721 | 0.001077 | 0.001213 | 0.001349 |
| minus_eq_vv_test.test | RUN | 0.201465 | 0.524331 | 0.636728 | 0.001050 | 0.001158 | 0.001215 |
| minus_vv_test.test | RUN | 0.203499 | 0.572160 | 0.678331 | 0.000886 | 0.001232 | 0.001284 |
| mul_eq_vs_test.test | RUN | 0.189363 | 0.445910 | 0.633576 | 0.001075 | 0.001293 | 0.001558 |
| mul_vs_test.test | RUN | 0.206554 | 0.565517 | 0.675441 | 0.001292 | 0.001242 | 0.001319 |
| mul_sv_test.test | RUN | 0.204047 | 0.563737 | 0.667982 | 0.001105 | 0.001183 | 0.001189 |
| mul_vm_test.test | RUN | 0.286480 | 1.013803 | 1.053313 | 0.000985 | 0.001255 | 0.001421 |
| normalize_v_test.test | RUN | 0.211840 | 0.693862 | 0.696372 | 0.001240 | 0.001167 | 0.001362 |
| plus_eq_vv_test.test | RUN | 0.194818 | 0.514580 | 0.617119 | 0.001237 | 0.001171 | 0.001152 |
| plus_vv_test.test | RUN | 0.204450 | 0.563146 | 0.650035 | 0.001082 | 0.001196 | 0.001360 |
| scalar_cast_v_test.test | RUN | 0.184552 | 0.418442 | 0.569023 | 0.001054 | 0.001064 | 0.001242 |
| vec_index_test.test | RUN | 0.161412 | 0.431412 | 0.514118 | 0.001084 | 0.001155 | 0.001167 |
| vec_register_test.test | RUN | 0.106687 | 0.392649 | 0.370893 | 0.001209 | 0.001153 | 0.001241 |
| cmp_mm_test.test | RUN | 0.331626 | 1.249554 | 1.347014 | 0.001217 | 0.001252 | 0.001128 |
| determinant_test.test | RUN | 0.224849 | 0.632539 | 0.717320 | 0.001103 | 0.001192 | 0.001380 |
| div_eq_ms_test.test | RUN | 0.231481 | 0.579032 | 0.827131 | 0.001107 | 0.001139 | 0.001432 |
| div_ms_test.test | RUN | 0.271782 | 0.946216 | 1.004866 | 0.001072 | 0.001154 | 0.001200 |
| eq_mm_test.test | RUN | 0.316553 | 1.159598 | 1.294781 | 0.001197 | 0.001170 | 0.001075 |
| minus_m_test.test | RUN | 0.267091 | 0.920907 | 0.981540 | 0.001088 | 0.001169 | 0.001290 |
| minus_eq_mm_test.test | RUN | 0.260595 | 0.852832 | 0.913598 | 0.001116 | 0.001168 | 0.001261 |
| minus_mm_test.test | RUN | 0.293220 | 1.093208 | 1.062416 | 0.001080 | 0.001249 | 0.001249 |
| mul_eq_mm_test.test | RUN | 0.239556 | 0.789009 | 0.805747 | 0.001170 | 0.001257 | 0.001137 |
| mul_eq_ms_test.test | RUN | 0.229807 | 0.574546 | 0.826131 | 0.001020 | 0.001238 | 0.001239 |
| mul_mm_test.test | RUN | 0.332539 | 1.788731 | 1.326657 | 0.001083 | 0.001182 | 0.001273 |
| mul_ms_test.test | RUN | 0.277588 | 0.949558 | 1.024864 | 0.000979 | 0.001201 | 0.001245 |
| mul_sm_test.test | RUN | 0.271718 | 0.944389 | 1.029925 | 0.001142 | 0.001270 | 0.001398 |
| mul_mv_test.test | RUN | 0.283853 | 1.003893 | 1.038447 | 0.001095 | 0.001292 | 0.001270 |
| inverse_m_test.test | RUN | 0.377942 | 1.867219 | 1.461158 | 0.001168 | 0.001407 | 0.001378 |
| plus_eq_mm_test.test | RUN | 0.254372 | 0.862925 | 0.909327 | 0.001113 | 0.001137 | 0.001518 |
| plus_mm_test.test | RUN | 0.287240 | 1.083522 | 1.072687 | 0.001118 | 0.001131 | 0.001347 |
| scalar_cast_m_test.test | RUN | 0.217480 | 0.489517 | 0.663743 | 0.000983 | 0.001176 | 0.001256 |
| mat_index_test.test | RUN | 0.168612 | 0.461688 | 0.567073 | 0.001070 | 0.001287 | 0.001316 |
| cmp_qq_test.test | RUN | 0.188437 | 0.427075 | 0.548462 | 0.001089 | 0.001186 | 0.001292 |
| conjugate_test.test | RUN | 0.183120 | 0.440855 | 0.572619 | 0.001079 | 0.001194 | 0.001529 |
| normalize_q_test.test | RUN | 0.190617 | 0.492153 | 0.603117 | 0.000987 | 0.001234 | 0.001577 |
| div_eq_qs_test.test | RUN | 0.183634 | 0.408896 | 0.569680 | 0.001084 | 0.001290 | 0.001308 |
| div_qs_test.test | RUN | 0.191433 | 0.443139 | 0.590722 | 0.001076 | 0.001165 | 0.001225 |
| dot_qq_test.test | RUN | 0.185592 | 0.426353 | 0.580822 | 0.001051 | 0.001145 | 0.001323 |
| eq_qq_test.test | RUN | 0.191756 | 0.449086 | 0.596678 | 0.001039 | 0.001162 | 0.001495 |
| inverse_q_test.test | RUN | 0.229954 | 0.598428 | 0.742544 | 0.001085 | 0.001394 | 0.001316 |
| mag_q_test.test | RUN | 0.187724 | 0.442401 | 0.582521 | 0.001007 | 0.001172 | 0.001292 |
| mag_sqr_q_test.test | RUN | 0.182666 | 0.429705 | 0.573928 | 0.001116 | 0.001143 | 0.001271 |
| minus_q_test.test | RUN | 0.183214 | 0.450195 | 0.571024 | 0.001068 | 0.001161 | 0.001260 |
| minus_eq_qq_test.test | RUN | 0.176810 | 0.437096 | 0.566673 | 0.001040 | 0.001188 | 0.001304 |
| minus_qq_test.test | RUN | 0.182407 | 0.454275 | 0.585975 | 0.001051 | 0.001125 | 0.001162 |
| mul_eq_qs_test.test | RUN | 0.186050 | 0.414194 | 0.569332 | 0.001114 | 0.001385 | 0.001192 |
| mul_qs_test.test | RUN | 0.185291 | 0.446231 | 0.570072 | 0.001073 | 0.001320 | 0.001228 |
| mul_qv_test.test | RUN | 0.226505 | 0.610223 | 0.737333 | 0.001087 | 0.001138 | 0.001331 |
| mul_qq_test.test | RUN | 0.223341 | 0.617807 | 0.725762 | 0.000987 | 0.001096 | 0.001438 |
| mul_eq_qq_test.test | RUN | 0.224973 | 0.612744 | 0.720602 | 0.001013 | 0.001108 | 0.001220 |
| plus_eq_qq_test.test | RUN | 0.183069 | 0.439006 | 0.549847 | 0.001071 | 0.001065 | 0.001275 |
| plus_qq_test.test | RUN | 0.183136 | 0.451202 | 0.577877 | 0.001016 | 0.001309 | 0.001247 |
| scalar_cast_q_test.test | RUN | 0.183591 | 0.418104 | 0.559699 | 0.001069 | 0.001200 | 0.001328 |
| slerp_test.test | RUN | 0.200121 | 0.879353 | 0.684443 | 0.015141 | 0.013352 | 0.013127 |
| convert_to_test.test | RUN | 0.275657 | 0.789344 | 1.079472 | 0.001092 | 0.001131 | 0.001478 |
| rot_mat_test.test | RUN | 0.447205 | 3.670361 | 3.399901 | 0.034187 | 0.031698 | 0.033864 |
| rot_quat_test.test | RUN | 0.230880 | 0.739164 | 0.753484 | 0.001079 | 0.001354 | 0.001406 |
| rotx_mat_test.test | RUN | 0.254416 | 0.960265 | 0.988772 | 0.000977 | 0.001188 | 0.001272 |
| rotx_quat_test.test | RUN | 0.220572 | 0.613507 | 0.688802 | 0.001058 | 0.001258 | 0.001406 |
| roty_mat_test.test | RUN | 0.263999 | 0.958040 | 0.994324 | 0.001111 | 0.001138 | 0.001519 |
| roty_quat_test.test | RUN | 0.222737 | 0.615644 | 0.712145 | 0.001088 | 0.001156 | 0.001460 |
| rotz_mat_test.test | RUN | 0.285007 | 1.073327 | 0.983800 | 0.001064 | 0.001156 | 0.001376 |
| rotz_quat_test.test | RUN | 0.223779 | 0.612742 | 0.720002 | 0.001040 | 0.001229 | 0.001286 |
| zero_vec_test.test | RUN | 0.191093 | 0.536910 | 0.624957 | 0.001173 | 0.000853 | 0.001380 |
| zero_mat_test.test | RUN | 0.236600 | 0.871554 | 0.831400 | 0.001123 | 0.001211 | 0.001312 |
| zero_quat_test.test | RUN | 0.183088 | 0.434248 | 0.570902 | 0.001145 | 0.001245 | 0.001481 |
| identity_mat_test.test | RUN | 0.220428 | 0.636484 | 0.738412 | 0.001100 | 0.001217 | 0.001509 |
| identity_quat_test.test | RUN | 0.185445 | 0.431414 | 0.571825 | 0.001114 | 0.001311 | 0.001319 |
| col_test.test | RUN | 0.368359 | 1.394921 | 1.655593 | 0.001036 | 0.001221 | 0.001366 |
| col_mat_test.test | RUN | 0.235764 | 0.618630 | 0.770348 | 0.001220 | 0.001207 | 0.001284 |
| neg_col_test.test | RUN | 0.280449 | 0.858444 | 0.998729 | 0.001021 | 0.001218 | 0.001504 |
| neg_row_test.test | RUN | 0.283527 | 0.864217 | 0.992916 | 0.001022 | 0.001314 | 0.001460 |
| del_col_test.test | RUN | 0.288105 | 0.946808 | 1.033497 | 0.001091 | 0.001247 | 0.001391 |
| del_row_test.test | RUN | 0.288915 | 0.945105 | 1.044667 | 0.001043 | 0.001150 | 0.001404 |
| del_row_col_test.test | RUN | 0.402045 | 1.773257 | 1.930420 | 0.001100 | 0.001329 | 0.001230 |
| swap_cols_test.test | RUN | 0.534003 | 2.387801 | 2.385435 | 0.001209 | 0.001142 | 0.001368 |
| swap_rows_test.test | RUN | 0.669960 | 2.491552 | 2.348007 | 0.001206 | 0.001204 | 0.001364 |
| diag_test.test | RUN | 0.244969 | 0.628730 | 0.875756 | 0.001151 | 0.001194 | 0.001338 |
| diag_mat_test.test | RUN | 0.246158 | 0.637101 | 0.786752 | 0.001032 | 0.001198 | 0.001398 |
| row_test.test | RUN | 0.361597 | 1.376737 | 1.632542 | 0.001050 | 0.001265 | 0.001258 |
| row_mat_test.test | RUN | 0.231878 | 0.609387 | 0.770088 | 0.001020 | 0.001202 | 0.001204 |
| transpose_test.test | RUN | 0.281617 | 0.918357 | 1.042387 | 0.001022 | 0.001180 | 0.001395 |
| translation_test.test | RUN | 0.230451 | 0.588086 | 0.782367 | 0.001061 | 0.001349 | 0.001417 |
| translation_mat_test.test | RUN | 0.245501 | 0.589508 | 0.842669 | 0.001026 | 0.001176 | 0.001240 |
| swizzle_const_fail.test | COMPILE_FAIL | 0.076729 | 0.168862 | 0.228361 | — | — | — |
| swizzle2_test2.test | RUN | 0.098829 | 0.253333 | 0.311778 | 0.001038 | 0.001201 | 0.001494 |
| swizzle2_test3.test | RUN | 0.096270 | 0.252575 | 0.325151 | 0.001013 | 0.001137 | 0.001672 |
| swizzle2_test4.test | RUN | 0.096636 | 0.246498 | 0.311890 | 0.001111 | 0.001190 | 0.001286 |
| swizzle2_test.test | RUN | 0.325196 | 0.689007 | 1.015513 | 0.000950 | 0.001162 | 0.001374 |
| swizzle3_test2.test | RUN | 0.110373 | 0.271870 | 0.351911 | 0.001065 | 0.001264 | 0.001489 |
| swizzle3_test3.test | RUN | 0.112515 | 0.273940 | 0.348473 | 0.001053 | 0.001108 | 0.001258 |
| swizzle3_test4.test | RUN | 0.113785 | 0.273653 | 0.353664 | 0.001020 | 0.001348 | 0.001165 |
| swizzle3_test.test | RUN | 0.300169 | 0.712365 | 1.011806 | 0.001267 | 0.001233 | 0.001564 |
| swizzle4_test2.test | RUN | 0.186765 | 0.397080 | 0.563488 | 0.001241 | 0.001322 | 0.001170 |
| swizzle4_test3.test | RUN | 0.189114 | 0.388662 | 0.570028 | 0.001086 | 0.001375 | 0.001316 |
| swizzle4_test4.test | RUN | 0.185370 | 0.388378 | 0.570913 | 0.001034 | 0.001212 | 0.001345 |
| swizzle4_test.test | RUN | 0.318166 | 0.773215 | 1.041299 | 0.001050 | 0.001241 | 0.001542 |
| deduce_scalar_test.test | COMPILE | 0.015300 | 0.037343 | 0.056887 | — | — | — |
| deduce_scalar_mq_test.test | RUN | 0.120166 | 0.299745 | 0.381166 | 0.000984 | 0.001206 | 0.001205 |
| deduce_scalar_mv_test.test | RUN | 0.138950 | 0.335164 | 0.447359 | 0.001121 | 0.001225 | 0.001569 |
| deduce_scalar_fail.test | COMPILE_FAIL | 0.012105 | 0.024046 | 0.044440 | — | — | — |
| deduce_mat_test.test | COMPILE | 0.013473 | 0.033104 | 0.050689 | — | — | — |
| deduce_quat_test.test | COMPILE | 0.013815 | 0.031774 | 0.049234 | — | — | — |
| deduce_vec_test.test | COMPILE | 0.014266 | 0.032156 | 0.050963 | — | — | — |
| interop_test.test | RUN | 0.148194 | 0.433701 | 0.659292 | 0.000479 | 0.000508 | 0.001373 |
| transform_test.test | RUN | 0.274216 | 0.575999 | 0.906904 | 0.000823 | 0.000795 | 0.001162 |
| projection_test.test | RUN | 0.189651 | 0.501294 | 0.530504 | 0.000861 | 0.000866 | 0.001070 |
| scalar_traits_test.test | COMPILE | 0.014764 | 0.033125 | 0.053544 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.992× | 0.987×, 0.997× | 1.001× |
| clang | 1.029× | 1.027×, 1.030× | 1.004× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814672).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| access_m_test.test | 10 | 0.000817s | 0.000810s | 0.000829s | byte equality observed |
| access_q_test.test | 10 | 0.000815s | 0.000811s | 0.000826s | byte equality observed |
| access_v_test.test | 10 | 0.000810s | 0.000818s | 0.000814s | byte equality observed |
| assign_test.test | 10 | 0.000786s | 0.000829s | 0.000826s | byte equality observed |
| cmp_mm_test.test | 10 | 0.000808s | 0.000852s | 0.000807s | byte equality observed |
| cmp_qq_test.test | 10 | 0.000819s | 0.000794s | 0.000820s | byte equality observed |
| cmp_vv_test.test | 10 | 0.000799s | 0.000789s | 0.000804s | byte equality observed |
| col_mat_test.test | 10 | 0.000815s | 0.000812s | 0.000831s | byte equality observed |
| col_test.test | 10 | 0.000807s | 0.000820s | 0.000806s | byte equality observed |
| conjugate_test.test | 10 | 0.000805s | 0.000822s | 0.000804s | byte equality observed |
| convert_to_test.test | 10 | 0.000798s | 0.000826s | 0.000801s | byte equality observed |
| cross_test.test | 10 | 0.000799s | 0.000785s | 0.000824s | byte equality observed |
| deduce_scalar_mq_test.test | 10 | 0.000818s | 0.000794s | 0.000837s | byte equality observed |
| deduce_scalar_mv_test.test | 10 | 0.000839s | 0.000808s | 0.000838s | byte equality observed |
| del_col_test.test | 10 | 0.000833s | 0.000797s | 0.000821s | byte equality observed |
| del_row_col_test.test | 10 | 0.000815s | 0.000824s | 0.000833s | byte equality observed |
| del_row_test.test | 10 | 0.000815s | 0.000803s | 0.000811s | byte equality observed |
| determinant_test.test | 10 | 0.000822s | 0.000822s | 0.000827s | byte equality observed |
| diag_mat_test.test | 10 | 0.000807s | 0.000816s | 0.000795s | byte equality observed |
| diag_test.test | 10 | 0.000801s | 0.000813s | 0.000799s | byte equality observed |
| div_eq_ms_test.test | 10 | 0.000790s | 0.000801s | 0.000830s | byte equality observed |
| div_eq_qs_test.test | 10 | 0.000833s | 0.000805s | 0.000841s | byte equality observed |
| div_eq_vs_test.test | 10 | 0.000814s | 0.000813s | 0.000799s | byte equality observed |
| div_ms_test.test | 10 | 0.000810s | 0.000835s | 0.000804s | byte equality observed |
| div_qs_test.test | 10 | 0.000817s | 0.000797s | 0.000811s | byte equality observed |
| div_vs_test.test | 10 | 0.000820s | 0.000828s | 0.000793s | byte equality observed |
| dot_qq_test.test | 10 | 0.000804s | 0.000796s | 0.000810s | byte equality observed |
| dot_vv_test.test | 10 | 0.000802s | 0.000806s | 0.000823s | byte equality observed |
| eq_mm_test.test | 10 | 0.000805s | 0.000844s | 0.000830s | byte equality observed |
| eq_qq_test.test | 10 | 0.000829s | 0.000845s | 0.000837s | byte equality observed |
| eq_vv_test.test | 10 | 0.000834s | 0.000817s | 0.000786s | byte equality observed |
| identity_mat_test.test | 10 | 0.000801s | 0.000828s | 0.000804s | byte equality observed |
| identity_quat_test.test | 10 | 0.000801s | 0.000822s | 0.000781s | byte equality observed |
| interop_test.test | 10 | 0.000363s | 0.000364s | 0.000788s | byte equality observed |
| inverse_m_test.test | 10 | 0.000857s | 0.000844s | 0.000840s | byte equality observed |
| inverse_q_test.test | 10 | 0.000818s | 0.000831s | 0.000840s | byte equality observed |
| mag_q_test.test | 10 | 0.000801s | 0.000815s | 0.000828s | byte equality observed |
| mag_sqr_q_test.test | 10 | 0.000802s | 0.000817s | 0.000811s | byte equality observed |
| mag_sqr_v_test.test | 10 | 0.000782s | 0.000789s | 0.000821s | byte equality observed |
| mag_v_test.test | 10 | 0.000795s | 0.000791s | 0.000835s | byte equality observed |
| mat_index_test.test | 10 | 0.000816s | 0.000791s | 0.000815s | byte equality observed |
| mat_traits_array_test.test | 10 | 0.000806s | 0.000805s | 0.000815s | byte equality observed |
| mat_traits_std_array_test.test | 10 | 0.000794s | 0.000806s | 0.000803s | byte equality observed |
| math_test.test | 10 | 0.001216s | 0.001188s | 0.001049s | byte equality observed |
| minus_eq_mm_test.test | 10 | 0.000788s | 0.000791s | 0.000827s | byte equality observed |
| minus_eq_qq_test.test | 10 | 0.000797s | 0.000782s | 0.000828s | byte equality observed |
| minus_eq_vv_test.test | 10 | 0.000818s | 0.000793s | 0.000841s | byte equality observed |
| minus_m_test.test | 10 | 0.000819s | 0.000789s | 0.000807s | byte equality observed |
| minus_mm_test.test | 10 | 0.000824s | 0.000791s | 0.000803s | byte equality observed |
| minus_q_test.test | 10 | 0.000803s | 0.000815s | 0.000832s | byte equality observed |
| minus_qq_test.test | 10 | 0.000799s | 0.000823s | 0.000826s | byte equality observed |
| minus_v_test.test | 10 | 0.000794s | 0.000830s | 0.000807s | byte equality observed |
| minus_vv_test.test | 10 | 0.000812s | 0.000779s | 0.000808s | byte equality observed |
| mul_eq_mm_test.test | 10 | 0.000802s | 0.000786s | 0.000803s | byte equality observed |
| mul_eq_ms_test.test | 10 | 0.000803s | 0.000799s | 0.000790s | byte equality observed |
| mul_eq_qq_test.test | 10 | 0.000825s | 0.000801s | 0.000818s | byte equality observed |
| mul_eq_qs_test.test | 10 | 0.000819s | 0.000796s | 0.000826s | byte equality observed |
| mul_eq_vs_test.test | 10 | 0.000815s | 0.000789s | 0.000827s | byte equality observed |
| mul_mm_test.test | 10 | 0.000814s | 0.000781s | 0.000839s | byte equality observed |
| mul_ms_test.test | 10 | 0.000822s | 0.000804s | 0.000834s | byte equality observed |
| mul_mv_test.test | 10 | 0.000827s | 0.000785s | 0.000802s | byte equality observed |
| mul_qq_test.test | 10 | 0.000829s | 0.000817s | 0.000792s | byte equality observed |
| mul_qs_test.test | 10 | 0.000812s | 0.000794s | 0.000800s | byte equality observed |
| mul_qv_test.test | 10 | 0.000804s | 0.000843s | 0.000812s | byte equality observed |
| mul_sm_test.test | 10 | 0.000803s | 0.000797s | 0.000822s | byte equality observed |
| mul_sv_test.test | 10 | 0.000814s | 0.000811s | 0.000809s | byte equality observed |
| mul_vm_test.test | 10 | 0.000820s | 0.000814s | 0.000794s | byte equality observed |
| mul_vs_test.test | 10 | 0.000798s | 0.000806s | 0.000795s | byte equality observed |
| neg_col_test.test | 10 | 0.000826s | 0.000819s | 0.000845s | byte equality observed |
| neg_row_test.test | 10 | 0.000807s | 0.000825s | 0.000825s | byte equality observed |
| normalize_q_test.test | 10 | 0.000792s | 0.000798s | 0.000823s | byte equality observed |
| normalize_v_test.test | 10 | 0.000818s | 0.000804s | 0.000820s | byte equality observed |
| plus_eq_mm_test.test | 10 | 0.000834s | 0.000821s | 0.000820s | byte equality observed |
| plus_eq_qq_test.test | 10 | 0.000824s | 0.000813s | 0.000815s | byte equality observed |
| plus_eq_vv_test.test | 10 | 0.000809s | 0.000834s | 0.000805s | byte equality observed |
| plus_mm_test.test | 10 | 0.000802s | 0.000787s | 0.000829s | byte equality observed |
| plus_qq_test.test | 10 | 0.000798s | 0.000804s | 0.000824s | byte equality observed |
| plus_vv_test.test | 10 | 0.000812s | 0.000802s | 0.000818s | byte equality observed |
| projection_test.test | 10 | 0.000833s | 0.000821s | 0.000808s | byte equality observed |
| quat_traits_array_test.test | 10 | 0.000821s | 0.000806s | 0.000858s | byte equality observed |
| quat_traits_std_array_test.test | 10 | 0.000785s | 0.000771s | 0.000794s | byte equality observed |
| rot_mat_test.test | 1 | 0.022109s | 0.022644s | 0.026561s | byte equality observed |
| rot_quat_test.test | 10 | 0.000758s | 0.000722s | 0.000749s | byte equality observed |
| rotx_mat_test.test | 10 | 0.000783s | 0.000747s | 0.000754s | byte equality observed |
| rotx_quat_test.test | 10 | 0.000804s | 0.000825s | 0.000791s | byte equality observed |
| roty_mat_test.test | 10 | 0.000821s | 0.000837s | 0.000817s | byte equality observed |
| roty_quat_test.test | 10 | 0.000823s | 0.000842s | 0.000810s | byte equality observed |
| rotz_mat_test.test | 10 | 0.000838s | 0.000837s | 0.000812s | byte equality observed |
| rotz_quat_test.test | 10 | 0.000830s | 0.000824s | 0.000807s | byte equality observed |
| row_mat_test.test | 10 | 0.000804s | 0.000808s | 0.000794s | byte equality observed |
| row_test.test | 10 | 0.000787s | 0.000809s | 0.000806s | byte equality observed |
| scalar_cast_m_test.test | 10 | 0.000809s | 0.000783s | 0.000806s | byte equality observed |
| scalar_cast_q_test.test | 10 | 0.000801s | 0.000804s | 0.000836s | byte equality observed |
| scalar_cast_v_test.test | 10 | 0.000799s | 0.000798s | 0.000812s | byte equality observed |
| slerp_test.test | 2 | 0.010988s | 0.009472s | 0.010026s | byte equality observed |
| swap_cols_test.test | 10 | 0.000758s | 0.000759s | 0.000730s | byte equality observed |
| swap_rows_test.test | 10 | 0.000775s | 0.000750s | 0.000738s | byte equality observed |
| swizzle2_test.test | 10 | 0.000812s | 0.000833s | 0.000803s | byte equality observed |
| swizzle2_test2.test | 10 | 0.000834s | 0.000835s | 0.000778s | byte equality observed |
| swizzle2_test3.test | 10 | 0.000830s | 0.000828s | 0.000806s | byte equality observed |
| swizzle2_test4.test | 10 | 0.000830s | 0.000818s | 0.000818s | byte equality observed |
| swizzle3_test.test | 10 | 0.000832s | 0.000817s | 0.000798s | byte equality observed |
| swizzle3_test2.test | 10 | 0.000811s | 0.000814s | 0.000814s | byte equality observed |
| swizzle3_test3.test | 10 | 0.000834s | 0.000814s | 0.000790s | byte equality observed |
| swizzle3_test4.test | 10 | 0.000840s | 0.000806s | 0.000794s | byte equality observed |
| swizzle4_test.test | 10 | 0.000814s | 0.000828s | 0.000831s | byte equality observed |
| swizzle4_test2.test | 10 | 0.000818s | 0.000811s | 0.000803s | byte equality observed |
| swizzle4_test3.test | 10 | 0.000826s | 0.000840s | 0.000833s | byte equality observed |
| swizzle4_test4.test | 10 | 0.000819s | 0.000847s | 0.000821s | byte equality observed |
| to_string_test.test | 10 | 0.000831s | 0.000853s | 0.000810s | byte equality observed |
| transform_test.test | 10 | 0.000846s | 0.000828s | 0.000810s | byte equality observed |
| translation_mat_test.test | 10 | 0.000804s | 0.000793s | 0.000810s | byte equality observed |
| translation_test.test | 10 | 0.000814s | 0.000811s | 0.000814s | byte equality observed |
| transpose_test.test | 10 | 0.000802s | 0.000824s | 0.000819s | byte equality observed |
| vec_index_test.test | 10 | 0.000787s | 0.000823s | 0.000817s | byte equality observed |
| vec_register_test.test | 10 | 0.000807s | 0.000814s | 0.000813s | byte equality observed |
| vec_traits_array_test.test | 10 | 0.000793s | 0.000806s | 0.000814s | byte equality observed |
| vec_traits_gnuc_test.test | 10 | 0.000791s | 0.000825s | 0.000826s | byte equality observed |
| vec_traits_std_array_test.test | 10 | 0.000802s | 0.000833s | 0.000820s | byte equality observed |
| zero_mat_test.test | 10 | 0.000817s | 0.000811s | 0.000813s | byte equality observed |
| zero_quat_test.test | 10 | 0.000806s | 0.000810s | 0.000826s | byte equality observed |
| zero_vec_test.test | 10 | 0.000789s | 0.000819s | 0.000799s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814672).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
