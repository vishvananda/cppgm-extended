# libs/qvm/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| math_test.test | RUN | 0.412719 | 0.888433 | 1.246416 | 0.002469 | 0.002680 | 0.002835 |
| mat_traits_array_test.test | RUN | 0.283059 | 0.905531 | 0.987794 | 0.001912 | 0.001679 | 0.002158 |
| mat_traits_std_array_test.test | RUN | 0.295855 | 0.690815 | 1.046852 | 0.001847 | 0.001932 | 0.001798 |
| vec_traits_array_test.test | RUN | 0.256891 | 0.690585 | 0.855841 | 0.001876 | 0.002028 | 0.001847 |
| vec_traits_gnuc_test.test | RUN | 0.458625 | 0.872320 | 1.694038 | 0.001827 | 0.001954 | 0.001946 |
| vec_traits_std_array_test.test | RUN | 0.247681 | 0.538104 | 0.854102 | 0.001872 | 0.002038 | 0.002014 |
| quat_traits_array_test.test | RUN | 0.242207 | 0.721511 | 0.848216 | 0.001924 | 0.001868 | 0.001668 |
| quat_traits_std_array_test.test | RUN | 0.243416 | 0.556275 | 0.868318 | 0.001654 | 0.001835 | 0.002186 |
| access_m_test.test | RUN | 0.395080 | 1.362545 | 1.842958 | 0.001970 | 0.002232 | 0.001788 |
| access_m_fail1.test | COMPILE_FAIL | 0.041680 | 0.058755 | 0.133805 | — | — | — |
| access_m_fail2.test | COMPILE_FAIL | 0.038727 | 0.062602 | 0.130932 | — | — | — |
| access_q_test.test | RUN | 0.332477 | 0.697522 | 1.132839 | 0.002137 | 0.001802 | 0.002035 |
| access_q_fail.test | COMPILE_FAIL | 0.029885 | 0.044239 | 0.088233 | — | — | — |
| access_v_test.test | RUN | 0.335485 | 0.687399 | 1.085604 | 0.001857 | 0.001933 | 0.001733 |
| access_v_fail1.test | COMPILE_FAIL | 0.029040 | 0.039256 | 0.092481 | — | — | — |
| access_v_fail2.test | COMPILE_FAIL | 0.028298 | 0.044789 | 0.087232 | — | — | — |
| assign_test.test | RUN | 0.498447 | 1.097969 | 2.001303 | 0.002051 | 0.002166 | 0.002010 |
| to_string_test.test | RUN | 0.573475 | 1.753321 | 10.410171 | 0.001751 | 0.002131 | 0.002424 |
| cmp_vv_test.test | RUN | 0.426164 | 0.909564 | 1.366864 | 0.001880 | 0.002182 | 0.002062 |
| cross_test.test | RUN | 0.389677 | 0.833669 | 1.374859 | 0.001874 | 0.002161 | 0.002334 |
| div_eq_vs_test.test | RUN | 0.382743 | 0.768051 | 1.273671 | 0.001882 | 0.002151 | 0.002110 |
| div_vs_test.test | RUN | 0.387934 | 0.906735 | 1.332685 | 0.002041 | 0.002149 | 0.002028 |
| dot_vv_test.test | RUN | 0.386882 | 0.823257 | 1.258442 | 0.001842 | 0.002248 | 0.002293 |
| eq_vv_test.test | RUN | 0.407901 | 0.938326 | 1.402248 | 0.001892 | 0.001798 | 0.001878 |
| mag_v_test.test | RUN | 0.376247 | 0.777377 | 1.177999 | 0.001864 | 0.001808 | 0.001770 |
| mag_sqr_v_test.test | RUN | 0.370134 | 0.783020 | 1.264860 | 0.001764 | 0.002073 | 0.002334 |
| minus_v_test.test | RUN | 0.383107 | 0.895722 | 1.368520 | 0.001878 | 0.002284 | 0.002127 |
| minus_eq_vv_test.test | RUN | 0.382987 | 0.872938 | 1.210709 | 0.001735 | 0.002061 | 0.002136 |
| minus_vv_test.test | RUN | 0.399094 | 0.949073 | 1.408827 | 0.001906 | 0.001875 | 0.002217 |
| mul_eq_vs_test.test | RUN | 0.385391 | 0.767646 | 1.234029 | 0.001937 | 0.001960 | 0.001982 |
| mul_vs_test.test | RUN | 0.398854 | 0.938423 | 1.371455 | 0.001922 | 0.002122 | 0.002340 |
| mul_sv_test.test | RUN | 0.402994 | 0.944012 | 1.406924 | 0.001922 | 0.002300 | 0.002379 |
| mul_vm_test.test | RUN | 0.554861 | 1.650251 | 1.942005 | 0.001918 | 0.001884 | 0.001829 |
| normalize_v_test.test | RUN | 0.396025 | 1.159497 | 1.409909 | 0.001938 | 0.002059 | 0.001820 |
| plus_eq_vv_test.test | RUN | 0.382552 | 0.855034 | 1.331792 | 0.001802 | 0.001947 | 0.002041 |
| plus_vv_test.test | RUN | 0.395013 | 0.945031 | 1.298557 | 0.001838 | 0.002252 | 0.001751 |
| scalar_cast_v_test.test | RUN | 0.370578 | 0.708532 | 1.225313 | 0.001593 | 0.001858 | 0.001728 |
| vec_index_test.test | RUN | 0.329257 | 0.705641 | 1.101493 | 0.001544 | 0.001870 | 0.001936 |
| vec_register_test.test | RUN | 0.208817 | 0.648717 | 0.789297 | 0.001963 | 0.002025 | 0.002065 |
| cmp_mm_test.test | RUN | 0.596964 | 2.011858 | 2.583906 | 0.002098 | 0.002060 | 0.002017 |
| determinant_test.test | RUN | 0.435417 | 1.046997 | 1.530231 | 0.002006 | 0.002086 | 0.002132 |
| div_eq_ms_test.test | RUN | 0.450353 | 0.990968 | 1.732695 | 0.001722 | 0.002250 | 0.001854 |
| div_ms_test.test | RUN | 0.504623 | 1.540099 | 2.078927 | 0.001944 | 0.001930 | 0.002179 |
| eq_mm_test.test | RUN | 0.593468 | 1.898863 | 2.471284 | 0.002174 | 0.002013 | 0.002084 |
| minus_m_test.test | RUN | 0.499795 | 1.495744 | 1.902823 | 0.001966 | 0.002075 | 0.002144 |
| minus_eq_mm_test.test | RUN | 0.481129 | 1.405231 | 1.803535 | 0.002178 | 0.001999 | 0.001878 |
| minus_mm_test.test | RUN | 0.520491 | 1.792013 | 2.088998 | 0.002142 | 0.002248 | 0.002250 |
| mul_eq_mm_test.test | RUN | 0.458861 | 1.342820 | 1.645175 | 0.002464 | 0.002096 | 0.002020 |
| mul_eq_ms_test.test | RUN | 0.469536 | 0.946002 | 1.660985 | 0.002076 | 0.002253 | 0.002090 |
| mul_mm_test.test | RUN | 0.637346 | 2.913736 | 2.625646 | 0.002224 | 0.002007 | 0.002238 |
| mul_ms_test.test | RUN | 0.530763 | 1.535234 | 2.150242 | 0.002009 | 0.002127 | 0.001975 |
| mul_sm_test.test | RUN | 0.526303 | 1.528458 | 2.050609 | 0.002131 | 0.002170 | 0.002056 |
| mul_mv_test.test | RUN | 0.572664 | 1.663772 | 2.079517 | 0.002205 | 0.002030 | 0.002204 |
| inverse_m_test.test | RUN | 0.691133 | 2.948068 | 2.831867 | 0.002133 | 0.002100 | 0.002170 |
| plus_eq_mm_test.test | RUN | 0.472811 | 1.403597 | 1.790479 | 0.001978 | 0.002150 | 0.002159 |
| plus_mm_test.test | RUN | 0.536725 | 1.802494 | 2.168355 | 0.002220 | 0.002219 | 0.002159 |
| scalar_cast_m_test.test | RUN | 0.428649 | 0.815411 | 1.366029 | 0.002038 | 0.002013 | 0.002092 |
| mat_index_test.test | RUN | 0.342783 | 0.762991 | 1.113202 | 0.002120 | 0.002070 | 0.002048 |
| cmp_qq_test.test | RUN | 0.365830 | 0.718189 | 1.121001 | 0.002268 | 0.002026 | 0.002085 |
| conjugate_test.test | RUN | 0.381478 | 0.741506 | 1.125875 | 0.002145 | 0.002023 | 0.002278 |
| normalize_q_test.test | RUN | 0.380387 | 0.825811 | 1.224861 | 0.002140 | 0.001904 | 0.001927 |
| div_eq_qs_test.test | RUN | 0.373739 | 0.701951 | 1.194736 | 0.001941 | 0.001894 | 0.001899 |
| div_qs_test.test | RUN | 0.376038 | 0.756704 | 1.173993 | 0.002022 | 0.001888 | 0.002228 |
| dot_qq_test.test | RUN | 0.372177 | 0.717874 | 1.194056 | 0.001961 | 0.002227 | 0.002011 |
| eq_qq_test.test | RUN | 0.382258 | 0.755795 | 1.201925 | 0.002082 | 0.001976 | 0.002149 |
| inverse_q_test.test | RUN | 0.454369 | 0.996728 | 1.481124 | 0.001963 | 0.002264 | 0.002098 |
| mag_q_test.test | RUN | 0.370750 | 0.747918 | 1.169098 | 0.002136 | 0.002262 | 0.002197 |
| mag_sqr_q_test.test | RUN | 0.376604 | 0.726474 | 1.172130 | 0.001905 | 0.001646 | 0.002233 |
| minus_q_test.test | RUN | 0.376275 | 0.739407 | 1.217957 | 0.001910 | 0.001821 | 0.002188 |
| minus_eq_qq_test.test | RUN | 0.383166 | 0.755359 | 1.114311 | 0.001936 | 0.001842 | 0.002208 |
| minus_qq_test.test | RUN | 0.385954 | 0.752596 | 1.183346 | 0.002144 | 0.002280 | 0.002076 |
| mul_eq_qs_test.test | RUN | 0.387650 | 0.693574 | 1.180921 | 0.002143 | 0.002289 | 0.001946 |
| mul_qs_test.test | RUN | 0.380890 | 0.739605 | 1.207765 | 0.002128 | 0.002214 | 0.002061 |
| mul_qv_test.test | RUN | 0.468440 | 1.039496 | 1.554927 | 0.002061 | 0.002134 | 0.002220 |
| mul_qq_test.test | RUN | 0.447498 | 1.009722 | 1.503002 | 0.002066 | 0.002096 | 0.002231 |
| mul_eq_qq_test.test | RUN | 0.450202 | 1.030298 | 1.450252 | 0.002222 | 0.002025 | 0.001980 |
| plus_eq_qq_test.test | RUN | 0.373639 | 0.727998 | 1.178324 | 0.002012 | 0.001975 | 0.002202 |
| plus_qq_test.test | RUN | 0.375937 | 0.758668 | 1.199750 | 0.001910 | 0.002132 | 0.002196 |
| scalar_cast_q_test.test | RUN | 0.369930 | 0.694617 | 1.170436 | 0.002328 | 0.002158 | 0.002146 |
| slerp_test.test | RUN | 0.408634 | 1.401336 | 1.395428 | 0.029086 | 0.020384 | 0.021435 |
| convert_to_test.test | RUN | 0.523438 | 1.318666 | 2.098342 | 0.002150 | 0.001978 | 0.002295 |
| rot_mat_test.test | RUN | 0.874845 | 5.826905 | 6.090091 | 0.098186 | 0.055075 | 0.050959 |
| rot_quat_test.test | RUN | 0.441002 | 1.177534 | 1.588118 | 0.002182 | 0.002275 | 0.002288 |
| rotx_mat_test.test | RUN | 0.511225 | 1.576971 | 1.980061 | 0.002280 | 0.002026 | 0.002257 |
| rotx_quat_test.test | RUN | 0.432143 | 1.018732 | 1.446558 | 0.001926 | 0.002054 | 0.002100 |
| roty_mat_test.test | RUN | 0.521939 | 1.592805 | 1.738268 | 0.002334 | 0.002215 | 0.002130 |
| roty_quat_test.test | RUN | 0.451571 | 1.011915 | 1.411059 | 0.002041 | 0.002291 | 0.002098 |
| rotz_mat_test.test | RUN | 0.521320 | 1.742997 | 2.086235 | 0.001760 | 0.002038 | 0.001974 |
| rotz_quat_test.test | RUN | 0.441564 | 1.015073 | 1.329133 | 0.001821 | 0.002052 | 0.002072 |
| zero_vec_test.test | RUN | 0.408134 | 0.886721 | 1.198696 | 0.002096 | 0.002072 | 0.002238 |
| zero_mat_test.test | RUN | 0.550503 | 1.450874 | 1.706468 | 0.002143 | 0.001774 | 0.001746 |
| zero_quat_test.test | RUN | 0.370640 | 0.724280 | 1.166158 | 0.001930 | 0.002026 | 0.002168 |
| identity_mat_test.test | RUN | 0.462604 | 1.108341 | 1.525712 | 0.001889 | 0.002025 | 0.002376 |
| identity_quat_test.test | RUN | 0.378149 | 0.712760 | 1.194694 | 0.002047 | 0.002218 | 0.001701 |
| col_test.test | RUN | 0.719281 | 2.303612 | 3.235782 | 0.001944 | 0.001978 | 0.002184 |
| col_mat_test.test | RUN | 0.494580 | 0.994804 | 1.598250 | 0.002218 | 0.002167 | 0.002254 |
| neg_col_test.test | RUN | 0.536918 | 1.394572 | 2.040897 | 0.001832 | 0.002146 | 0.001647 |
| neg_row_test.test | RUN | 0.557860 | 1.489882 | 2.001196 | 0.002011 | 0.001810 | 0.002240 |
| del_col_test.test | RUN | 0.542128 | 1.598163 | 2.057803 | 0.002289 | 0.002268 | 0.002234 |
| del_row_test.test | RUN | 0.553630 | 1.562954 | 2.039501 | 0.001872 | 0.002233 | 0.002116 |
| del_row_col_test.test | RUN | 0.736676 | 2.940507 | 3.939193 | 0.002244 | 0.002148 | 0.001950 |
| swap_cols_test.test | RUN | 1.042764 | 3.925963 | 4.654378 | 0.002290 | 0.002250 | 0.002238 |
| swap_rows_test.test | RUN | 1.126557 | 4.153373 | 4.690499 | 0.002421 | 0.002084 | 0.002212 |
| diag_test.test | RUN | 0.476763 | 1.065898 | 1.737113 | 0.002064 | 0.002214 | 0.002224 |
| diag_mat_test.test | RUN | 0.512062 | 1.073150 | 1.514250 | 0.002115 | 0.002113 | 0.002102 |
| row_test.test | RUN | 0.707234 | 2.276081 | 3.254334 | 0.002090 | 0.002192 | 0.002146 |
| row_mat_test.test | RUN | 0.467908 | 0.983425 | 1.527530 | 0.002130 | 0.001965 | 0.002112 |
| transpose_test.test | RUN | 0.541223 | 1.573894 | 2.030141 | 0.002197 | 0.002134 | 0.002031 |
| translation_test.test | RUN | 0.449485 | 0.958601 | 1.557923 | 0.002016 | 0.001826 | 0.001773 |
| translation_mat_test.test | RUN | 0.495796 | 0.983255 | 1.629916 | 0.002095 | 0.001870 | 0.001837 |
| swizzle_const_fail.test | COMPILE_FAIL | 0.169129 | 0.304755 | 0.514448 | — | — | — |
| swizzle2_test2.test | RUN | 0.207632 | 0.420265 | 0.626752 | 0.001775 | 0.001930 | 0.001791 |
| swizzle2_test3.test | RUN | 0.205871 | 0.426506 | 0.630028 | 0.002063 | 0.002071 | 0.001746 |
| swizzle2_test4.test | RUN | 0.201110 | 0.404381 | 0.635262 | 0.001763 | 0.002204 | 0.002056 |
| swizzle2_test.test | RUN | 0.650755 | 1.212158 | 2.222148 | 0.002160 | 0.002240 | 0.001778 |
| swizzle3_test2.test | RUN | 0.225850 | 0.443526 | 0.718652 | 0.002534 | 0.002128 | 0.002010 |
| swizzle3_test3.test | RUN | 0.229228 | 0.461781 | 0.756570 | 0.002070 | 0.002034 | 0.001691 |
| swizzle3_test4.test | RUN | 0.231013 | 0.454050 | 0.723048 | 0.002030 | 0.001981 | 0.002085 |
| swizzle3_test.test | RUN | 0.642407 | 1.218362 | 2.140430 | 0.001890 | 0.001856 | 0.002146 |
| swizzle4_test2.test | RUN | 0.379371 | 0.655854 | 1.263656 | 0.002006 | 0.002204 | 0.002078 |
| swizzle4_test3.test | RUN | 0.374117 | 0.682338 | 1.305630 | 0.001927 | 0.001998 | 0.002135 |
| swizzle4_test4.test | RUN | 0.403263 | 0.701934 | 1.313271 | 0.002162 | 0.001904 | 0.002072 |
| swizzle4_test.test | RUN | 0.652024 | 1.318758 | 2.247027 | 0.002002 | 0.002127 | 0.002054 |
| deduce_scalar_test.test | COMPILE | 0.032144 | 0.061460 | 0.110768 | — | — | — |
| deduce_scalar_mq_test.test | RUN | 0.244092 | 0.503114 | 0.732409 | 0.002087 | 0.002130 | 0.001787 |
| deduce_scalar_mv_test.test | RUN | 0.300416 | 0.559354 | 0.890717 | 0.001920 | 0.002004 | 0.002132 |
| deduce_scalar_fail.test | COMPILE_FAIL | 0.027513 | 0.037621 | 0.084770 | — | — | — |
| deduce_mat_test.test | COMPILE | 0.030784 | 0.056553 | 0.091526 | — | — | — |
| deduce_quat_test.test | COMPILE | 0.028530 | 0.056277 | 0.091870 | — | — | — |
| deduce_vec_test.test | COMPILE | 0.029979 | 0.055196 | 0.081611 | — | — | — |
| interop_test.test | RUN | 0.303376 | 0.721879 | 1.380532 | 0.000885 | 0.000974 | 0.001791 |
| transform_test.test | RUN | 0.550733 | 0.970637 | 1.686635 | 0.001680 | 0.001396 | 0.001497 |
| projection_test.test | RUN | 0.358037 | 0.786960 | 1.328332 | 0.001652 | 0.001460 | 0.001462 |
| scalar_traits_test.test | COMPILE | 0.029895 | 0.057743 | 0.096318 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.751× | 0.737×, 0.764× | 1.025× |
| clang | 0.829× | 0.830×, 0.828× | 0.998× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| access_m_test.test | 10 | 0.001283s | 0.001294s | 0.001254s | byte equality observed |
| access_q_test.test | 10 | 0.001280s | 0.001272s | 0.001259s | byte equality observed |
| access_v_test.test | 10 | 0.001295s | 0.001242s | 0.001301s | byte equality observed |
| assign_test.test | 10 | 0.001295s | 0.001304s | 0.001320s | byte equality observed |
| cmp_mm_test.test | 10 | 0.001299s | 0.001299s | 0.001272s | byte equality observed |
| cmp_qq_test.test | 10 | 0.001295s | 0.001302s | 0.001284s | byte equality observed |
| cmp_vv_test.test | 10 | 0.001296s | 0.001333s | 0.001286s | byte equality observed |
| col_mat_test.test | 10 | 0.001305s | 0.001279s | 0.001282s | byte equality observed |
| col_test.test | 10 | 0.001294s | 0.001260s | 0.001285s | byte equality observed |
| conjugate_test.test | 10 | 0.001276s | 0.001284s | 0.001284s | byte equality observed |
| convert_to_test.test | 10 | 0.001300s | 0.001283s | 0.001280s | byte equality observed |
| cross_test.test | 10 | 0.001287s | 0.001265s | 0.001290s | byte equality observed |
| deduce_scalar_mq_test.test | 10 | 0.001344s | 0.001314s | 0.001261s | byte equality observed |
| deduce_scalar_mv_test.test | 10 | 0.001308s | 0.001257s | 0.001275s | byte equality observed |
| del_col_test.test | 10 | 0.001290s | 0.001233s | 0.001285s | byte equality observed |
| del_row_col_test.test | 10 | 0.001272s | 0.001230s | 0.001348s | byte equality observed |
| del_row_test.test | 10 | 0.001262s | 0.001309s | 0.001280s | byte equality observed |
| determinant_test.test | 10 | 0.001320s | 0.001306s | 0.001281s | byte equality observed |
| diag_mat_test.test | 10 | 0.001290s | 0.001280s | 0.001338s | byte equality observed |
| diag_test.test | 10 | 0.001266s | 0.001341s | 0.001317s | byte equality observed |
| div_eq_ms_test.test | 10 | 0.001262s | 0.001282s | 0.001294s | byte equality observed |
| div_eq_qs_test.test | 10 | 0.001256s | 0.001328s | 0.001277s | byte equality observed |
| div_eq_vs_test.test | 10 | 0.001277s | 0.001353s | 0.001282s | byte equality observed |
| div_ms_test.test | 10 | 0.001269s | 0.001270s | 0.001264s | byte equality observed |
| div_qs_test.test | 10 | 0.001342s | 0.001278s | 0.001237s | byte equality observed |
| div_vs_test.test | 10 | 0.001290s | 0.001282s | 0.001224s | byte equality observed |
| dot_qq_test.test | 10 | 0.001324s | 0.001277s | 0.001274s | byte equality observed |
| dot_vv_test.test | 10 | 0.001316s | 0.001273s | 0.001253s | byte equality observed |
| eq_mm_test.test | 10 | 0.001333s | 0.001305s | 0.001272s | byte equality observed |
| eq_qq_test.test | 10 | 0.001308s | 0.001302s | 0.001316s | byte equality observed |
| eq_vv_test.test | 10 | 0.001298s | 0.001331s | 0.001287s | byte equality observed |
| identity_mat_test.test | 10 | 0.001321s | 0.001329s | 0.001298s | byte equality observed |
| identity_quat_test.test | 10 | 0.001306s | 0.001280s | 0.001238s | byte equality observed |
| interop_test.test | 10 | 0.000608s | 0.000597s | 0.001354s | byte equality observed |
| inverse_m_test.test | 10 | 0.001408s | 0.001337s | 0.001280s | byte equality observed |
| inverse_q_test.test | 10 | 0.001293s | 0.001292s | 0.001320s | byte equality observed |
| mag_q_test.test | 10 | 0.001320s | 0.001289s | 0.001433s | byte equality observed |
| mag_sqr_q_test.test | 10 | 0.001304s | 0.001249s | 0.001403s | byte equality observed |
| mag_sqr_v_test.test | 10 | 0.001326s | 0.001277s | 0.001426s | byte equality observed |
| mag_v_test.test | 10 | 0.001335s | 0.001286s | 0.001461s | byte equality observed |
| mat_index_test.test | 10 | 0.001334s | 0.001269s | 0.001455s | byte equality observed |
| mat_traits_array_test.test | 10 | 0.001280s | 0.001274s | 0.001494s | byte equality observed |
| mat_traits_std_array_test.test | 10 | 0.001294s | 0.001242s | 0.001534s | byte equality observed |
| math_test.test | 8 | 0.001948s | 0.001840s | 0.001958s | byte equality observed |
| minus_eq_mm_test.test | 10 | 0.001287s | 0.001254s | 0.001502s | byte equality observed |
| minus_eq_qq_test.test | 10 | 0.001356s | 0.001233s | 0.001517s | byte equality observed |
| minus_eq_vv_test.test | 10 | 0.001470s | 0.001263s | 0.001505s | byte equality observed |
| minus_m_test.test | 10 | 0.001470s | 0.001246s | 0.001492s | byte equality observed |
| minus_mm_test.test | 8 | 0.001482s | 0.001293s | 0.001467s | byte equality observed |
| minus_q_test.test | 10 | 0.001488s | 0.001273s | 0.001535s | byte equality observed |
| minus_qq_test.test | 10 | 0.001495s | 0.001305s | 0.001475s | byte equality observed |
| minus_v_test.test | 10 | 0.001490s | 0.001321s | 0.001520s | byte equality observed |
| minus_vv_test.test | 10 | 0.001516s | 0.001275s | 0.001506s | byte equality observed |
| mul_eq_mm_test.test | 10 | 0.001507s | 0.001300s | 0.001516s | byte equality observed |
| mul_eq_ms_test.test | 10 | 0.001514s | 0.001288s | 0.001526s | byte equality observed |
| mul_eq_qq_test.test | 10 | 0.001504s | 0.001295s | 0.001571s | byte equality observed |
| mul_eq_qs_test.test | 10 | 0.001508s | 0.001268s | 0.001527s | byte equality observed |
| mul_eq_vs_test.test | 10 | 0.001497s | 0.001283s | 0.001526s | byte equality observed |
| mul_mm_test.test | 10 | 0.001540s | 0.001327s | 0.001552s | byte equality observed |
| mul_ms_test.test | 10 | 0.001503s | 0.001278s | 0.001533s | byte equality observed |
| mul_mv_test.test | 10 | 0.001484s | 0.001291s | 0.001524s | byte equality observed |
| mul_qq_test.test | 10 | 0.001507s | 0.001300s | 0.001576s | byte equality observed |
| mul_qs_test.test | 9 | 0.001536s | 0.001302s | 0.001530s | byte equality observed |
| mul_qv_test.test | 10 | 0.001536s | 0.001306s | 0.001517s | byte equality observed |
| mul_sm_test.test | 10 | 0.001557s | 0.001339s | 0.001553s | byte equality observed |
| mul_sv_test.test | 10 | 0.001509s | 0.001321s | 0.001530s | byte equality observed |
| mul_vm_test.test | 10 | 0.001546s | 0.001279s | 0.001515s | byte equality observed |
| mul_vs_test.test | 10 | 0.001535s | 0.001282s | 0.001562s | byte equality observed |
| neg_col_test.test | 10 | 0.001558s | 0.001302s | 0.001567s | byte equality observed |
| neg_row_test.test | 10 | 0.001539s | 0.001353s | 0.001570s | byte equality observed |
| normalize_q_test.test | 10 | 0.001523s | 0.001262s | 0.001510s | byte equality observed |
| normalize_v_test.test | 10 | 0.001509s | 0.001253s | 0.001516s | byte equality observed |
| plus_eq_mm_test.test | 10 | 0.001548s | 0.001278s | 0.001546s | byte equality observed |
| plus_eq_qq_test.test | 10 | 0.001527s | 0.001324s | 0.001544s | byte equality observed |
| plus_eq_vv_test.test | 10 | 0.001526s | 0.001273s | 0.001472s | byte equality observed |
| plus_mm_test.test | 10 | 0.001550s | 0.001314s | 0.001535s | byte equality observed |
| plus_qq_test.test | 10 | 0.001490s | 0.001285s | 0.001495s | byte equality observed |
| plus_vv_test.test | 10 | 0.001497s | 0.001300s | 0.001508s | byte equality observed |
| projection_test.test | 10 | 0.001553s | 0.001243s | 0.001513s | byte equality observed |
| quat_traits_array_test.test | 10 | 0.001488s | 0.001247s | 0.001493s | byte equality observed |
| quat_traits_std_array_test.test | 10 | 0.001503s | 0.001210s | 0.001472s | byte equality observed |
| rot_mat_test.test | 1 | 0.089990s | 0.034979s | 0.045637s | byte equality observed |
| rot_quat_test.test | 10 | 0.001502s | 0.001211s | 0.001490s | byte equality observed |
| rotx_mat_test.test | 10 | 0.001511s | 0.001395s | 0.001494s | byte equality observed |
| rotx_quat_test.test | 10 | 0.001496s | 0.001411s | 0.001509s | byte equality observed |
| roty_mat_test.test | 10 | 0.001520s | 0.001505s | 0.001567s | byte equality observed |
| roty_quat_test.test | 10 | 0.001486s | 0.001498s | 0.001546s | byte equality observed |
| rotz_mat_test.test | 10 | 0.001512s | 0.001546s | 0.001575s | byte equality observed |
| rotz_quat_test.test | 10 | 0.001494s | 0.001538s | 0.001547s | byte equality observed |
| row_mat_test.test | 10 | 0.001474s | 0.001571s | 0.001563s | byte equality observed |
| row_test.test | 10 | 0.001494s | 0.001523s | 0.001525s | byte equality observed |
| scalar_cast_m_test.test | 10 | 0.001495s | 0.001509s | 0.001507s | byte equality observed |
| scalar_cast_q_test.test | 10 | 0.001491s | 0.001551s | 0.001514s | byte equality observed |
| scalar_cast_v_test.test | 10 | 0.001496s | 0.001498s | 0.001466s | byte equality observed |
| slerp_test.test | 2 | 0.026210s | 0.018383s | 0.019357s | byte equality observed |
| swap_cols_test.test | 10 | 0.001495s | 0.001505s | 0.001444s | byte equality observed |
| swap_rows_test.test | 10 | 0.001504s | 0.001458s | 0.001469s | byte equality observed |
| swizzle2_test.test | 10 | 0.001489s | 0.001488s | 0.001528s | byte equality observed |
| swizzle2_test2.test | 10 | 0.001506s | 0.001477s | 0.001516s | byte equality observed |
| swizzle2_test3.test | 10 | 0.001490s | 0.001502s | 0.001512s | byte equality observed |
| swizzle2_test4.test | 10 | 0.001507s | 0.001525s | 0.001540s | byte equality observed |
| swizzle3_test.test | 10 | 0.001518s | 0.001556s | 0.001534s | byte equality observed |
| swizzle3_test2.test | 10 | 0.001539s | 0.001533s | 0.001552s | byte equality observed |
| swizzle3_test3.test | 10 | 0.001534s | 0.001481s | 0.001527s | byte equality observed |
| swizzle3_test4.test | 10 | 0.001532s | 0.001536s | 0.001530s | byte equality observed |
| swizzle4_test.test | 10 | 0.001562s | 0.001512s | 0.001556s | byte equality observed |
| swizzle4_test2.test | 10 | 0.001543s | 0.001535s | 0.001555s | byte equality observed |
| swizzle4_test3.test | 10 | 0.001530s | 0.001513s | 0.001541s | byte equality observed |
| swizzle4_test4.test | 10 | 0.001539s | 0.001561s | 0.001551s | byte equality observed |
| to_string_test.test | 10 | 0.001589s | 0.001566s | 0.001602s | byte equality observed |
| transform_test.test | 10 | 0.001571s | 0.001526s | 0.001549s | byte equality observed |
| translation_mat_test.test | 10 | 0.001540s | 0.001514s | 0.001544s | byte equality observed |
| translation_test.test | 10 | 0.001552s | 0.001531s | 0.001541s | byte equality observed |
| transpose_test.test | 10 | 0.001533s | 0.001544s | 0.001534s | byte equality observed |
| vec_index_test.test | 10 | 0.001520s | 0.001547s | 0.001522s | byte equality observed |
| vec_register_test.test | 10 | 0.001523s | 0.001526s | 0.001525s | byte equality observed |
| vec_traits_array_test.test | 10 | 0.001499s | 0.001569s | 0.001563s | byte equality observed |
| vec_traits_gnuc_test.test | 10 | 0.001517s | 0.001544s | 0.001505s | byte equality observed |
| vec_traits_std_array_test.test | 10 | 0.001558s | 0.001571s | 0.001540s | byte equality observed |
| zero_mat_test.test | 10 | 0.001531s | 0.001535s | 0.001560s | byte equality observed |
| zero_quat_test.test | 10 | 0.001517s | 0.001539s | 0.001488s | byte equality observed |
| zero_vec_test.test | 10 | 0.001560s | 0.001520s | 0.001523s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
