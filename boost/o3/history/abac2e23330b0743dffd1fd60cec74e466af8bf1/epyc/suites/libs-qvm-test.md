# libs/qvm/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| math_test.test | RUN | 0.430515 | 0.924090 | 1.213462 | 0.002944 | 0.002969 | 0.002726 |
| mat_traits_array_test.test | RUN | 0.292282 | 0.931909 | 0.987791 | 0.001975 | 0.001772 | 0.001758 |
| mat_traits_std_array_test.test | RUN | 0.294364 | 0.711848 | 0.985289 | 0.001674 | 0.001934 | 0.001962 |
| vec_traits_array_test.test | RUN | 0.260413 | 0.701669 | 0.849913 | 0.001904 | 0.002062 | 0.001748 |
| vec_traits_gnuc_test.test | RUN | 0.467170 | 0.895755 | 1.690605 | 0.002171 | 0.002022 | 0.002219 |
| vec_traits_std_array_test.test | RUN | 0.256974 | 0.557072 | 0.838000 | 0.001868 | 0.002046 | 0.001678 |
| quat_traits_array_test.test | RUN | 0.251426 | 0.764509 | 0.819086 | 0.001958 | 0.002043 | 0.001822 |
| quat_traits_std_array_test.test | RUN | 0.250992 | 0.567175 | 0.864801 | 0.001914 | 0.001996 | 0.002080 |
| access_m_test.test | RUN | 0.414103 | 1.395824 | 1.863865 | 0.001846 | 0.002108 | 0.001786 |
| access_m_fail1.test | COMPILE_FAIL | 0.040834 | 0.061288 | 0.134298 | — | — | — |
| access_m_fail2.test | COMPILE_FAIL | 0.040423 | 0.065747 | 0.136679 | — | — | — |
| access_q_test.test | RUN | 0.356424 | 0.695559 | 1.127846 | 0.002032 | 0.002157 | 0.002262 |
| access_q_fail.test | COMPILE_FAIL | 0.031142 | 0.045858 | 0.089391 | — | — | — |
| access_v_test.test | RUN | 0.348331 | 0.733684 | 1.069110 | 0.001908 | 0.002150 | 0.001972 |
| access_v_fail1.test | COMPILE_FAIL | 0.027634 | 0.044404 | 0.088608 | — | — | — |
| access_v_fail2.test | COMPILE_FAIL | 0.027818 | 0.045374 | 0.086566 | — | — | — |
| assign_test.test | RUN | 0.508676 | 1.147254 | 1.999307 | 0.002130 | 0.002052 | 0.001934 |
| to_string_test.test | RUN | 0.585669 | 1.847224 | 10.533025 | 0.001758 | 0.002158 | 0.002288 |
| cmp_vv_test.test | RUN | 0.435123 | 0.891331 | 1.368665 | 0.002097 | 0.002343 | 0.001894 |
| cross_test.test | RUN | 0.409366 | 0.882462 | 1.347096 | 0.001892 | 0.002160 | 0.001582 |
| div_eq_vs_test.test | RUN | 0.392867 | 0.774240 | 1.258635 | 0.002122 | 0.002288 | 0.002324 |
| div_vs_test.test | RUN | 0.405153 | 0.980984 | 1.409658 | 0.002002 | 0.002302 | 0.001759 |
| dot_vv_test.test | RUN | 0.396985 | 0.842333 | 1.326413 | 0.002142 | 0.002046 | 0.001900 |
| eq_vv_test.test | RUN | 0.417512 | 0.961246 | 1.315782 | 0.001864 | 0.001904 | 0.002200 |
| mag_v_test.test | RUN | 0.386293 | 0.829877 | 1.274498 | 0.002226 | 0.002050 | 0.001894 |
| mag_sqr_v_test.test | RUN | 0.385938 | 0.795141 | 1.275342 | 0.002179 | 0.002292 | 0.001868 |
| minus_v_test.test | RUN | 0.400824 | 0.919060 | 1.321477 | 0.001579 | 0.002043 | 0.002192 |
| minus_eq_vv_test.test | RUN | 0.390951 | 0.908923 | 1.221409 | 0.001927 | 0.002074 | 0.001782 |
| minus_vv_test.test | RUN | 0.408313 | 0.975197 | 1.341291 | 0.002200 | 0.002092 | 0.002275 |
| mul_eq_vs_test.test | RUN | 0.381203 | 0.783020 | 1.233754 | 0.001711 | 0.002276 | 0.002285 |
| mul_vs_test.test | RUN | 0.415357 | 0.972732 | 1.369971 | 0.001871 | 0.002338 | 0.002296 |
| mul_sv_test.test | RUN | 0.399028 | 0.979107 | 1.390331 | 0.002138 | 0.002320 | 0.002150 |
| mul_vm_test.test | RUN | 0.580989 | 1.750906 | 2.096651 | 0.002150 | 0.002164 | 0.002206 |
| normalize_v_test.test | RUN | 0.432432 | 1.171423 | 1.293501 | 0.001759 | 0.002181 | 0.001920 |
| plus_eq_vv_test.test | RUN | 0.388545 | 0.918276 | 1.339412 | 0.001988 | 0.002079 | 0.002150 |
| plus_vv_test.test | RUN | 0.408547 | 0.973562 | 1.353453 | 0.001764 | 0.002301 | 0.001887 |
| scalar_cast_v_test.test | RUN | 0.383103 | 0.723014 | 1.120920 | 0.001933 | 0.002172 | 0.001701 |
| vec_index_test.test | RUN | 0.338634 | 0.739668 | 1.059923 | 0.001949 | 0.002076 | 0.002199 |
| vec_register_test.test | RUN | 0.213386 | 0.649378 | 0.770260 | 0.002092 | 0.002277 | 0.002236 |
| cmp_mm_test.test | RUN | 0.608847 | 2.105328 | 2.655649 | 0.002126 | 0.002082 | 0.002071 |
| determinant_test.test | RUN | 0.448252 | 1.074588 | 1.519353 | 0.002120 | 0.002066 | 0.002186 |
| div_eq_ms_test.test | RUN | 0.465145 | 1.018280 | 1.689956 | 0.002148 | 0.002242 | 0.002161 |
| div_ms_test.test | RUN | 0.518793 | 1.568217 | 2.086190 | 0.002258 | 0.001867 | 0.001980 |
| eq_mm_test.test | RUN | 0.615459 | 1.932289 | 2.500979 | 0.002098 | 0.002178 | 0.001921 |
| minus_m_test.test | RUN | 0.520153 | 1.561938 | 2.018006 | 0.002037 | 0.002082 | 0.001968 |
| minus_eq_mm_test.test | RUN | 0.481972 | 1.442617 | 1.897122 | 0.001976 | 0.002380 | 0.002274 |
| minus_mm_test.test | RUN | 0.555906 | 1.842658 | 2.127283 | 0.002206 | 0.002273 | 0.002203 |
| mul_eq_mm_test.test | RUN | 0.475039 | 1.353327 | 1.617472 | 0.002166 | 0.001836 | 0.002220 |
| mul_eq_ms_test.test | RUN | 0.469038 | 1.021645 | 1.702740 | 0.002068 | 0.001954 | 0.002246 |
| mul_mm_test.test | RUN | 0.632187 | 3.013165 | 2.667259 | 0.002142 | 0.002361 | 0.002054 |
| mul_ms_test.test | RUN | 0.527777 | 1.624669 | 2.136240 | 0.001937 | 0.002177 | 0.002166 |
| mul_sm_test.test | RUN | 0.534638 | 1.573474 | 2.078525 | 0.002193 | 0.001924 | 0.002248 |
| mul_mv_test.test | RUN | 0.588760 | 1.700942 | 2.194898 | 0.002046 | 0.002040 | 0.002201 |
| inverse_m_test.test | RUN | 0.694894 | 3.035100 | 2.911289 | 0.002314 | 0.001947 | 0.002200 |
| plus_eq_mm_test.test | RUN | 0.481313 | 1.447538 | 1.829975 | 0.002271 | 0.002172 | 0.002338 |
| plus_mm_test.test | RUN | 0.542104 | 1.833174 | 2.107301 | 0.002262 | 0.002392 | 0.001742 |
| scalar_cast_m_test.test | RUN | 0.445696 | 0.854837 | 1.432532 | 0.002216 | 0.002086 | 0.001924 |
| mat_index_test.test | RUN | 0.348641 | 0.803345 | 1.137145 | 0.001940 | 0.002022 | 0.002226 |
| cmp_qq_test.test | RUN | 0.391203 | 0.732727 | 1.194340 | 0.002204 | 0.002334 | 0.002007 |
| conjugate_test.test | RUN | 0.393035 | 0.767539 | 1.223615 | 0.002223 | 0.002324 | 0.002156 |
| normalize_q_test.test | RUN | 0.390471 | 0.845815 | 1.261165 | 0.002282 | 0.002156 | 0.002118 |
| div_eq_qs_test.test | RUN | 0.392112 | 0.734880 | 1.216804 | 0.001960 | 0.001974 | 0.002286 |
| div_qs_test.test | RUN | 0.387940 | 0.793070 | 1.221988 | 0.002084 | 0.002016 | 0.001828 |
| dot_qq_test.test | RUN | 0.387880 | 0.734656 | 1.228490 | 0.002146 | 0.002380 | 0.002125 |
| eq_qq_test.test | RUN | 0.384576 | 0.807951 | 1.249310 | 0.001934 | 0.002139 | 0.002214 |
| inverse_q_test.test | RUN | 0.453322 | 1.027558 | 1.498497 | 0.002047 | 0.002112 | 0.002185 |
| mag_q_test.test | RUN | 0.381430 | 0.787468 | 1.197211 | 0.002068 | 0.001891 | 0.001896 |
| mag_sqr_q_test.test | RUN | 0.391169 | 0.748753 | 1.201136 | 0.002138 | 0.002322 | 0.002210 |
| minus_q_test.test | RUN | 0.375912 | 0.756240 | 1.181268 | 0.002094 | 0.002196 | 0.002142 |
| minus_eq_qq_test.test | RUN | 0.382577 | 0.752494 | 1.218946 | 0.001881 | 0.002268 | 0.002233 |
| minus_qq_test.test | RUN | 0.383377 | 0.795487 | 1.185694 | 0.002120 | 0.002257 | 0.002247 |
| mul_eq_qs_test.test | RUN | 0.383294 | 0.720660 | 1.198405 | 0.001940 | 0.002140 | 0.002263 |
| mul_qs_test.test | RUN | 0.393618 | 0.765942 | 1.211923 | 0.002066 | 0.002210 | 0.002198 |
| mul_qv_test.test | RUN | 0.476197 | 1.061088 | 1.630063 | 0.002192 | 0.001924 | 0.002287 |
| mul_qq_test.test | RUN | 0.464708 | 1.050244 | 1.449720 | 0.002058 | 0.002122 | 0.002160 |
| mul_eq_qq_test.test | RUN | 0.460756 | 1.059639 | 1.496240 | 0.002136 | 0.002152 | 0.001722 |
| plus_eq_qq_test.test | RUN | 0.381087 | 0.756526 | 1.236885 | 0.002195 | 0.002094 | 0.001888 |
| plus_qq_test.test | RUN | 0.393587 | 0.799593 | 1.164021 | 0.001970 | 0.002245 | 0.002156 |
| scalar_cast_q_test.test | RUN | 0.388358 | 0.745486 | 1.193756 | 0.002110 | 0.002218 | 0.002225 |
| slerp_test.test | RUN | 0.418518 | 1.444693 | 1.422702 | 0.028485 | 0.020058 | 0.021201 |
| convert_to_test.test | RUN | 0.562002 | 1.363850 | 2.126736 | 0.002233 | 0.001947 | 0.002254 |
| rot_mat_test.test | RUN | 0.896406 | 5.951249 | 6.255999 | 0.096187 | 0.054194 | 0.049626 |
| rot_quat_test.test | RUN | 0.493217 | 1.236379 | 1.571755 | 0.002198 | 0.002171 | 0.002208 |
| rotx_mat_test.test | RUN | 0.526065 | 1.620094 | 1.943708 | 0.002186 | 0.002258 | 0.001800 |
| rotx_quat_test.test | RUN | 0.440897 | 1.058136 | 1.446821 | 0.001883 | 0.002167 | 0.002228 |
| roty_mat_test.test | RUN | 0.522013 | 1.646627 | 1.793132 | 0.002210 | 0.002317 | 0.002032 |
| roty_quat_test.test | RUN | 0.469062 | 1.045541 | 1.499593 | 0.002080 | 0.002228 | 0.002202 |
| rotz_mat_test.test | RUN | 0.535594 | 1.842504 | 2.092814 | 0.001897 | 0.001942 | 0.002299 |
| rotz_quat_test.test | RUN | 0.466614 | 1.076886 | 1.331867 | 0.002215 | 0.001920 | 0.001950 |
| zero_vec_test.test | RUN | 0.412137 | 0.900076 | 1.215086 | 0.002164 | 0.002148 | 0.001991 |
| zero_mat_test.test | RUN | 0.560198 | 1.484009 | 1.785867 | 0.001946 | 0.002297 | 0.002094 |
| zero_quat_test.test | RUN | 0.381036 | 0.765097 | 1.184342 | 0.001964 | 0.002140 | 0.002236 |
| identity_mat_test.test | RUN | 0.477797 | 1.089497 | 1.530257 | 0.001650 | 0.002376 | 0.002292 |
| identity_quat_test.test | RUN | 0.378245 | 0.730887 | 1.171944 | 0.002105 | 0.002301 | 0.002233 |
| col_test.test | RUN | 0.699163 | 2.329974 | 3.357169 | 0.001999 | 0.002349 | 0.002241 |
| col_mat_test.test | RUN | 0.474580 | 1.047037 | 1.546153 | 0.001934 | 0.002364 | 0.002242 |
| neg_col_test.test | RUN | 0.548219 | 1.520091 | 2.034254 | 0.001966 | 0.002119 | 0.002184 |
| neg_row_test.test | RUN | 0.575487 | 1.504749 | 2.080916 | 0.002084 | 0.002194 | 0.002205 |
| del_col_test.test | RUN | 0.563239 | 1.672937 | 1.980250 | 0.002248 | 0.002302 | 0.002248 |
| del_row_test.test | RUN | 0.560254 | 1.625571 | 2.076000 | 0.002188 | 0.002410 | 0.002166 |
| del_row_col_test.test | RUN | 0.750986 | 3.049635 | 3.926538 | 0.002072 | 0.002214 | 0.002067 |
| swap_cols_test.test | RUN | 1.066650 | 4.195420 | 4.659392 | 0.002145 | 0.002168 | 0.002274 |
| swap_rows_test.test | RUN | 1.136686 | 4.384728 | 4.809257 | 0.002275 | 0.002309 | 0.002204 |
| diag_test.test | RUN | 0.490539 | 1.082014 | 1.831625 | 0.002054 | 0.002155 | 0.002250 |
| diag_mat_test.test | RUN | 0.518432 | 1.076092 | 1.585819 | 0.002158 | 0.001895 | 0.002065 |
| row_test.test | RUN | 0.713688 | 2.369398 | 3.238667 | 0.002143 | 0.002070 | 0.002216 |
| row_mat_test.test | RUN | 0.492131 | 1.108484 | 1.600503 | 0.002215 | 0.002184 | 0.002169 |
| transpose_test.test | RUN | 0.532036 | 1.548746 | 2.137531 | 0.001773 | 0.002250 | 0.002278 |
| translation_test.test | RUN | 0.459388 | 0.995820 | 1.597248 | 0.001958 | 0.002134 | 0.002264 |
| translation_mat_test.test | RUN | 0.499326 | 1.044332 | 1.610904 | 0.001943 | 0.002085 | 0.001820 |
| swizzle_const_fail.test | COMPILE_FAIL | 0.163738 | 0.309435 | 0.499760 | — | — | — |
| swizzle2_test2.test | RUN | 0.201518 | 0.422593 | 0.659165 | 0.002234 | 0.002062 | 0.002156 |
| swizzle2_test3.test | RUN | 0.200015 | 0.439855 | 0.643442 | 0.002028 | 0.001980 | 0.002116 |
| swizzle2_test4.test | RUN | 0.201295 | 0.427780 | 0.642796 | 0.001946 | 0.002246 | 0.002226 |
| swizzle2_test.test | RUN | 0.627860 | 1.231623 | 2.193636 | 0.002018 | 0.002406 | 0.002298 |
| swizzle3_test2.test | RUN | 0.232810 | 0.459947 | 0.704161 | 0.001954 | 0.002324 | 0.002170 |
| swizzle3_test3.test | RUN | 0.222830 | 0.457159 | 0.761160 | 0.002073 | 0.002250 | 0.001922 |
| swizzle3_test4.test | RUN | 0.229972 | 0.442389 | 0.697791 | 0.001792 | 0.002293 | 0.001924 |
| swizzle3_test.test | RUN | 0.634199 | 1.225521 | 2.253516 | 0.001844 | 0.001880 | 0.002172 |
| swizzle4_test2.test | RUN | 0.368352 | 0.705738 | 1.305111 | 0.002174 | 0.002246 | 0.001877 |
| swizzle4_test3.test | RUN | 0.367680 | 0.707042 | 1.304398 | 0.001854 | 0.001919 | 0.001968 |
| swizzle4_test4.test | RUN | 0.394075 | 0.701510 | 1.283657 | 0.001826 | 0.002246 | 0.002270 |
| swizzle4_test.test | RUN | 0.643391 | 1.382540 | 2.249582 | 0.002229 | 0.002097 | 0.001636 |
| deduce_scalar_test.test | COMPILE | 0.033283 | 0.065768 | 0.113696 | — | — | — |
| deduce_scalar_mq_test.test | RUN | 0.245484 | 0.491631 | 0.843223 | 0.001626 | 0.002006 | 0.002030 |
| deduce_scalar_mv_test.test | RUN | 0.294971 | 0.612017 | 0.894209 | 0.001873 | 0.001868 | 0.002284 |
| deduce_scalar_fail.test | COMPILE_FAIL | 0.027725 | 0.042802 | 0.084680 | — | — | — |
| deduce_mat_test.test | COMPILE | 0.029165 | 0.058434 | 0.096343 | — | — | — |
| deduce_quat_test.test | COMPILE | 0.029306 | 0.052636 | 0.091921 | — | — | — |
| deduce_vec_test.test | COMPILE | 0.030573 | 0.055011 | 0.096885 | — | — | — |
| interop_test.test | RUN | 0.302618 | 0.752406 | 1.406429 | 0.000782 | 0.001039 | 0.001573 |
| transform_test.test | RUN | 0.556862 | 1.098870 | 1.908411 | 0.001816 | 0.001762 | 0.001667 |
| projection_test.test | RUN | 0.392245 | 0.698354 | 1.063335 | 0.001702 | 0.001701 | 0.002028 |
| scalar_traits_test.test | COMPILE | 0.030908 | 0.056113 | 0.096655 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.786× | 0.772×, 0.800× | 0.982× |
| clang | 0.809× | 0.800×, 0.819× | 0.995× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| access_m_test.test | 10 | 0.001316s | 0.001269s | 0.001296s | byte equality observed |
| access_q_test.test | 10 | 0.001294s | 0.001349s | 0.001319s | byte equality observed |
| access_v_test.test | 10 | 0.001335s | 0.001299s | 0.001344s | byte equality observed |
| assign_test.test | 10 | 0.001387s | 0.001353s | 0.001325s | byte equality observed |
| cmp_mm_test.test | 10 | 0.001347s | 0.001312s | 0.001292s | byte equality observed |
| cmp_qq_test.test | 10 | 0.001365s | 0.001270s | 0.001364s | byte equality observed |
| cmp_vv_test.test | 10 | 0.001330s | 0.001329s | 0.001264s | byte equality observed |
| col_mat_test.test | 10 | 0.001333s | 0.001301s | 0.001308s | byte equality observed |
| col_test.test | 10 | 0.001313s | 0.001292s | 0.001273s | byte equality observed |
| conjugate_test.test | 10 | 0.001348s | 0.001287s | 0.001281s | byte equality observed |
| convert_to_test.test | 10 | 0.001313s | 0.001292s | 0.001288s | byte equality observed |
| cross_test.test | 10 | 0.001300s | 0.001303s | 0.001359s | byte equality observed |
| deduce_scalar_mq_test.test | 10 | 0.001337s | 0.001290s | 0.001282s | byte equality observed |
| deduce_scalar_mv_test.test | 10 | 0.001312s | 0.001295s | 0.001290s | byte equality observed |
| del_col_test.test | 10 | 0.001302s | 0.001301s | 0.001285s | byte equality observed |
| del_row_col_test.test | 10 | 0.001309s | 0.001315s | 0.001305s | byte equality observed |
| del_row_test.test | 10 | 0.001297s | 0.001319s | 0.001306s | byte equality observed |
| determinant_test.test | 10 | 0.001336s | 0.001332s | 0.001299s | byte equality observed |
| diag_mat_test.test | 10 | 0.001326s | 0.001354s | 0.001301s | byte equality observed |
| diag_test.test | 10 | 0.001293s | 0.001339s | 0.001339s | byte equality observed |
| div_eq_ms_test.test | 10 | 0.001289s | 0.001361s | 0.001301s | byte equality observed |
| div_eq_qs_test.test | 10 | 0.001265s | 0.001288s | 0.001319s | byte equality observed |
| div_eq_vs_test.test | 10 | 0.001285s | 0.001327s | 0.001313s | byte equality observed |
| div_ms_test.test | 10 | 0.001355s | 0.001307s | 0.001339s | byte equality observed |
| div_qs_test.test | 10 | 0.001270s | 0.001345s | 0.001359s | byte equality observed |
| div_vs_test.test | 10 | 0.001279s | 0.001288s | 0.001285s | byte equality observed |
| dot_qq_test.test | 10 | 0.001292s | 0.001341s | 0.001284s | byte equality observed |
| dot_vv_test.test | 10 | 0.001289s | 0.001320s | 0.001273s | byte equality observed |
| eq_mm_test.test | 10 | 0.001295s | 0.001349s | 0.001260s | byte equality observed |
| eq_qq_test.test | 10 | 0.001281s | 0.001327s | 0.001300s | byte equality observed |
| eq_vv_test.test | 10 | 0.001333s | 0.001294s | 0.001306s | byte equality observed |
| identity_mat_test.test | 10 | 0.001319s | 0.001272s | 0.001349s | byte equality observed |
| identity_quat_test.test | 10 | 0.001319s | 0.001264s | 0.001277s | byte equality observed |
| interop_test.test | 10 | 0.000618s | 0.000626s | 0.001327s | byte equality observed |
| inverse_m_test.test | 10 | 0.001392s | 0.001353s | 0.001334s | byte equality observed |
| inverse_q_test.test | 10 | 0.001358s | 0.001304s | 0.001393s | byte equality observed |
| mag_q_test.test | 10 | 0.001308s | 0.001288s | 0.001323s | byte equality observed |
| mag_sqr_q_test.test | 10 | 0.001321s | 0.001309s | 0.001290s | byte equality observed |
| mag_sqr_v_test.test | 10 | 0.001291s | 0.001310s | 0.001287s | byte equality observed |
| mag_v_test.test | 10 | 0.001294s | 0.001292s | 0.001262s | byte equality observed |
| mat_index_test.test | 10 | 0.001285s | 0.001276s | 0.001263s | byte equality observed |
| mat_traits_array_test.test | 10 | 0.001329s | 0.001306s | 0.001285s | byte equality observed |
| mat_traits_std_array_test.test | 10 | 0.001297s | 0.001296s | 0.001269s | byte equality observed |
| math_test.test | 10 | 0.001976s | 0.001890s | 0.001698s | byte equality observed |
| minus_eq_mm_test.test | 10 | 0.001293s | 0.001291s | 0.001324s | byte equality observed |
| minus_eq_qq_test.test | 10 | 0.001371s | 0.001289s | 0.001318s | byte equality observed |
| minus_eq_vv_test.test | 10 | 0.001324s | 0.001306s | 0.001276s | byte equality observed |
| minus_m_test.test | 10 | 0.001330s | 0.001312s | 0.001285s | byte equality observed |
| minus_mm_test.test | 10 | 0.001345s | 0.001315s | 0.001311s | byte equality observed |
| minus_q_test.test | 10 | 0.001386s | 0.001320s | 0.001284s | byte equality observed |
| minus_qq_test.test | 10 | 0.001364s | 0.001308s | 0.001273s | byte equality observed |
| minus_v_test.test | 10 | 0.001349s | 0.001284s | 0.001325s | byte equality observed |
| minus_vv_test.test | 10 | 0.001415s | 0.001326s | 0.001315s | byte equality observed |
| mul_eq_mm_test.test | 10 | 0.001468s | 0.001328s | 0.001297s | byte equality observed |
| mul_eq_ms_test.test | 10 | 0.001459s | 0.001342s | 0.001392s | byte equality observed |
| mul_eq_qq_test.test | 10 | 0.001539s | 0.001344s | 0.001400s | byte equality observed |
| mul_eq_qs_test.test | 10 | 0.001513s | 0.001320s | 0.001425s | byte equality observed |
| mul_eq_vs_test.test | 10 | 0.001514s | 0.001262s | 0.001466s | byte equality observed |
| mul_mm_test.test | 10 | 0.001533s | 0.001293s | 0.001571s | byte equality observed |
| mul_ms_test.test | 10 | 0.001527s | 0.001252s | 0.001457s | byte equality observed |
| mul_mv_test.test | 10 | 0.001564s | 0.001264s | 0.001540s | byte equality observed |
| mul_qq_test.test | 10 | 0.001577s | 0.001282s | 0.001567s | byte equality observed |
| mul_qs_test.test | 10 | 0.001526s | 0.001348s | 0.001549s | byte equality observed |
| mul_qv_test.test | 10 | 0.001592s | 0.001354s | 0.001589s | byte equality observed |
| mul_sm_test.test | 10 | 0.001549s | 0.001296s | 0.001591s | byte equality observed |
| mul_sv_test.test | 10 | 0.001552s | 0.001360s | 0.001570s | byte equality observed |
| mul_vm_test.test | 10 | 0.001572s | 0.001315s | 0.001571s | byte equality observed |
| mul_vs_test.test | 10 | 0.001575s | 0.001326s | 0.001548s | byte equality observed |
| neg_col_test.test | 7 | 0.001568s | 0.001328s | 0.001580s | byte equality observed |
| neg_row_test.test | 8 | 0.001552s | 0.001337s | 0.001580s | byte equality observed |
| normalize_q_test.test | 10 | 0.001561s | 0.001289s | 0.001550s | byte equality observed |
| normalize_v_test.test | 10 | 0.001593s | 0.001322s | 0.001505s | byte equality observed |
| plus_eq_mm_test.test | 10 | 0.001531s | 0.001360s | 0.001580s | byte equality observed |
| plus_eq_qq_test.test | 10 | 0.001556s | 0.001292s | 0.001579s | byte equality observed |
| plus_eq_vv_test.test | 9 | 0.001546s | 0.001357s | 0.001555s | byte equality observed |
| plus_mm_test.test | 9 | 0.001545s | 0.001297s | 0.001569s | byte equality observed |
| plus_qq_test.test | 10 | 0.001564s | 0.001311s | 0.001538s | byte equality observed |
| plus_vv_test.test | 10 | 0.001533s | 0.001350s | 0.001521s | byte equality observed |
| projection_test.test | 10 | 0.001553s | 0.001251s | 0.001521s | byte equality observed |
| quat_traits_array_test.test | 10 | 0.001499s | 0.001367s | 0.001534s | byte equality observed |
| quat_traits_std_array_test.test | 10 | 0.001500s | 0.001455s | 0.001541s | byte equality observed |
| rot_mat_test.test | 1 | 0.095005s | 0.045789s | 0.046485s | byte equality observed |
| rot_quat_test.test | 10 | 0.001488s | 0.001471s | 0.001522s | byte equality observed |
| rotx_mat_test.test | 10 | 0.001555s | 0.001498s | 0.001499s | byte equality observed |
| rotx_quat_test.test | 10 | 0.001513s | 0.001528s | 0.001489s | byte equality observed |
| roty_mat_test.test | 10 | 0.001577s | 0.001546s | 0.001555s | byte equality observed |
| roty_quat_test.test | 10 | 0.001510s | 0.001545s | 0.001552s | byte equality observed |
| rotz_mat_test.test | 10 | 0.001561s | 0.001590s | 0.001588s | byte equality observed |
| rotz_quat_test.test | 10 | 0.001520s | 0.001601s | 0.001548s | byte equality observed |
| row_mat_test.test | 10 | 0.001497s | 0.001574s | 0.001566s | byte equality observed |
| row_test.test | 10 | 0.001541s | 0.001564s | 0.001566s | byte equality observed |
| scalar_cast_m_test.test | 10 | 0.001531s | 0.001568s | 0.001503s | byte equality observed |
| scalar_cast_q_test.test | 10 | 0.001517s | 0.001522s | 0.001531s | byte equality observed |
| scalar_cast_v_test.test | 10 | 0.001518s | 0.001488s | 0.001528s | byte equality observed |
| slerp_test.test | 2 | 0.026006s | 0.019376s | 0.019808s | byte equality observed |
| swap_cols_test.test | 10 | 0.001540s | 0.001507s | 0.001489s | byte equality observed |
| swap_rows_test.test | 10 | 0.001524s | 0.001488s | 0.001512s | byte equality observed |
| swizzle2_test.test | 10 | 0.001507s | 0.001477s | 0.001498s | byte equality observed |
| swizzle2_test2.test | 10 | 0.001501s | 0.001527s | 0.001488s | byte equality observed |
| swizzle2_test3.test | 10 | 0.001504s | 0.001561s | 0.001556s | byte equality observed |
| swizzle2_test4.test | 10 | 0.001530s | 0.001561s | 0.001561s | byte equality observed |
| swizzle3_test.test | 9 | 0.001578s | 0.001551s | 0.001547s | byte equality observed |
| swizzle3_test2.test | 10 | 0.001548s | 0.001539s | 0.001566s | byte equality observed |
| swizzle3_test3.test | 10 | 0.001554s | 0.001562s | 0.001567s | byte equality observed |
| swizzle3_test4.test | 10 | 0.001541s | 0.001527s | 0.001580s | byte equality observed |
| swizzle4_test.test | 10 | 0.001557s | 0.001567s | 0.001561s | byte equality observed |
| swizzle4_test2.test | 10 | 0.001552s | 0.001521s | 0.001556s | byte equality observed |
| swizzle4_test3.test | 8 | 0.001533s | 0.001580s | 0.001548s | byte equality observed |
| swizzle4_test4.test | 10 | 0.001563s | 0.001536s | 0.001587s | byte equality observed |
| to_string_test.test | 10 | 0.001606s | 0.001569s | 0.001603s | byte equality observed |
| transform_test.test | 10 | 0.001576s | 0.001569s | 0.001513s | byte equality observed |
| translation_mat_test.test | 10 | 0.001570s | 0.001541s | 0.001561s | byte equality observed |
| translation_test.test | 10 | 0.001574s | 0.001568s | 0.001547s | byte equality observed |
| transpose_test.test | 10 | 0.001539s | 0.001524s | 0.001534s | byte equality observed |
| vec_index_test.test | 10 | 0.001539s | 0.001580s | 0.001537s | byte equality observed |
| vec_register_test.test | 10 | 0.001567s | 0.001562s | 0.001569s | byte equality observed |
| vec_traits_array_test.test | 9 | 0.001579s | 0.001542s | 0.001517s | byte equality observed |
| vec_traits_gnuc_test.test | 10 | 0.001589s | 0.001569s | 0.001528s | byte equality observed |
| vec_traits_std_array_test.test | 10 | 0.001575s | 0.001557s | 0.001556s | byte equality observed |
| zero_mat_test.test | 10 | 0.001555s | 0.001560s | 0.001557s | byte equality observed |
| zero_quat_test.test | 10 | 0.001592s | 0.001525s | 0.001551s | byte equality observed |
| zero_vec_test.test | 10 | 0.001574s | 0.001535s | 0.001550s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
