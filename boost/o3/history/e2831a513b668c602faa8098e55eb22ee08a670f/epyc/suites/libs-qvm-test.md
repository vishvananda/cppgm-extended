# libs/qvm/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| math_test.test | RUN | 0.356809 | 0.887963 | 1.218024 | 0.002702 | 0.002714 | 0.002682 |
| mat_traits_array_test.test | RUN | 0.284580 | 0.907015 | 0.947265 | 0.001851 | 0.001936 | 0.001682 |
| mat_traits_std_array_test.test | RUN | 0.283069 | 0.692135 | 0.964861 | 0.001770 | 0.001795 | 0.001798 |
| vec_traits_array_test.test | RUN | 0.242133 | 0.683171 | 0.854817 | 0.001802 | 0.002024 | 0.001764 |
| vec_traits_gnuc_test.test | RUN | 0.426400 | 0.881015 | 1.673019 | 0.001794 | 0.001958 | 0.002396 |
| vec_traits_std_array_test.test | RUN | 0.228205 | 0.535909 | 0.842706 | 0.002044 | 0.001896 | 0.001984 |
| quat_traits_array_test.test | RUN | 0.223625 | 0.734735 | 0.831276 | 0.001862 | 0.001852 | 0.001805 |
| quat_traits_std_array_test.test | RUN | 0.224284 | 0.553540 | 0.864485 | 0.001950 | 0.001897 | 0.002016 |
| access_m_test.test | RUN | 0.369576 | 1.367856 | 1.846167 | 0.001932 | 0.002232 | 0.002173 |
| access_m_fail1.test | COMPILE_FAIL | 0.036424 | 0.058000 | 0.132720 | — | — | — |
| access_m_fail2.test | COMPILE_FAIL | 0.036990 | 0.060335 | 0.130137 | — | — | — |
| access_q_test.test | RUN | 0.304360 | 0.692786 | 1.115741 | 0.001796 | 0.002018 | 0.001926 |
| access_q_fail.test | COMPILE_FAIL | 0.027189 | 0.050080 | 0.088676 | — | — | — |
| access_v_test.test | RUN | 0.301895 | 0.724579 | 1.082400 | 0.001808 | 0.002051 | 0.002337 |
| access_v_fail1.test | COMPILE_FAIL | 0.024349 | 0.043829 | 0.088777 | — | — | — |
| access_v_fail2.test | COMPILE_FAIL | 0.026244 | 0.045962 | 0.085713 | — | — | — |
| assign_test.test | RUN | 0.465342 | 1.078303 | 1.946384 | 0.002108 | 0.002158 | 0.001868 |
| to_string_test.test | RUN | 0.530182 | 1.779731 | 10.264856 | 0.001922 | 0.002086 | 0.001970 |
| cmp_vv_test.test | RUN | 0.375902 | 0.921953 | 1.325603 | 0.001837 | 0.002019 | 0.002160 |
| cross_test.test | RUN | 0.363937 | 0.808803 | 1.326412 | 0.001772 | 0.002250 | 0.001918 |
| div_eq_vs_test.test | RUN | 0.348390 | 0.774218 | 1.275556 | 0.001866 | 0.002124 | 0.002307 |
| div_vs_test.test | RUN | 0.362928 | 0.897078 | 1.365534 | 0.001852 | 0.002205 | 0.002060 |
| dot_vv_test.test | RUN | 0.341742 | 0.842094 | 1.217202 | 0.001687 | 0.002120 | 0.001920 |
| eq_vv_test.test | RUN | 0.383014 | 0.962537 | 1.289031 | 0.001972 | 0.001848 | 0.001940 |
| mag_v_test.test | RUN | 0.362039 | 0.777827 | 1.253089 | 0.001850 | 0.001992 | 0.001796 |
| mag_sqr_v_test.test | RUN | 0.364332 | 0.826700 | 1.263307 | 0.001770 | 0.002039 | 0.001856 |
| minus_v_test.test | RUN | 0.380813 | 0.921524 | 1.257336 | 0.002043 | 0.002211 | 0.002088 |
| minus_eq_vv_test.test | RUN | 0.360698 | 0.856869 | 1.286605 | 0.001701 | 0.002123 | 0.002182 |
| minus_vv_test.test | RUN | 0.364350 | 0.951041 | 1.391300 | 0.002113 | 0.002057 | 0.002147 |
| mul_eq_vs_test.test | RUN | 0.346338 | 0.783631 | 1.201606 | 0.001591 | 0.002230 | 0.002110 |
| mul_vs_test.test | RUN | 0.366176 | 0.909150 | 1.378054 | 0.001812 | 0.002165 | 0.001950 |
| mul_sv_test.test | RUN | 0.367482 | 0.934603 | 1.396785 | 0.001820 | 0.002087 | 0.002167 |
| mul_vm_test.test | RUN | 0.517273 | 1.645592 | 2.055452 | 0.001714 | 0.002077 | 0.001816 |
| normalize_v_test.test | RUN | 0.370070 | 1.171001 | 1.298278 | 0.001967 | 0.002065 | 0.001926 |
| plus_eq_vv_test.test | RUN | 0.345498 | 0.886427 | 1.336399 | 0.001814 | 0.002054 | 0.002028 |
| plus_vv_test.test | RUN | 0.363480 | 0.932220 | 1.347920 | 0.001909 | 0.002172 | 0.001736 |
| scalar_cast_v_test.test | RUN | 0.332792 | 0.705906 | 1.201230 | 0.001684 | 0.001877 | 0.002114 |
| vec_index_test.test | RUN | 0.287876 | 0.699945 | 1.026055 | 0.002291 | 0.001924 | 0.001714 |
| vec_register_test.test | RUN | 0.195125 | 0.653292 | 0.777222 | 0.001926 | 0.002040 | 0.001979 |
| cmp_mm_test.test | RUN | 0.562911 | 2.025951 | 2.574752 | 0.002168 | 0.002196 | 0.002204 |
| determinant_test.test | RUN | 0.398353 | 1.072306 | 1.516159 | 0.001944 | 0.002093 | 0.002171 |
| div_eq_ms_test.test | RUN | 0.416882 | 0.977161 | 1.680235 | 0.001758 | 0.001986 | 0.002759 |
| div_ms_test.test | RUN | 0.472732 | 1.533685 | 1.994939 | 0.001854 | 0.001982 | 0.002624 |
| eq_mm_test.test | RUN | 0.548967 | 1.883607 | 2.461043 | 0.002169 | 0.001865 | 0.002098 |
| minus_m_test.test | RUN | 0.485049 | 1.512157 | 1.931432 | 0.002084 | 0.001916 | 0.001934 |
| minus_eq_mm_test.test | RUN | 0.449025 | 1.422573 | 1.783846 | 0.002193 | 0.002094 | 0.001986 |
| minus_mm_test.test | RUN | 0.497209 | 1.771098 | 2.112042 | 0.001934 | 0.002266 | 0.002103 |
| mul_eq_mm_test.test | RUN | 0.431219 | 1.311096 | 1.615313 | 0.001938 | 0.001635 | 0.002107 |
| mul_eq_ms_test.test | RUN | 0.411779 | 0.990993 | 1.646616 | 0.002032 | 0.001477 | 0.002286 |
| mul_mm_test.test | RUN | 0.575552 | 2.899581 | 2.599849 | 0.001992 | 0.002159 | 0.001770 |
| mul_ms_test.test | RUN | 0.490328 | 1.557435 | 2.057863 | 0.001992 | 0.002172 | 0.001869 |
| mul_sm_test.test | RUN | 0.501446 | 1.496980 | 2.046173 | 0.002749 | 0.002516 | 0.002124 |
| mul_mv_test.test | RUN | 0.547668 | 1.616691 | 2.083290 | 0.002721 | 0.001895 | 0.002068 |
| inverse_m_test.test | RUN | 0.645473 | 2.967722 | 2.820388 | 0.002167 | 0.002243 | 0.001962 |
| plus_eq_mm_test.test | RUN | 0.457351 | 1.407312 | 1.864112 | 0.001841 | 0.002262 | 0.002083 |
| plus_mm_test.test | RUN | 0.496897 | 1.776770 | 2.116325 | 0.001834 | 0.002184 | 0.001996 |
| scalar_cast_m_test.test | RUN | 0.398859 | 0.815341 | 1.370606 | 0.001962 | 0.002230 | 0.001932 |
| mat_index_test.test | RUN | 0.307403 | 0.774760 | 1.144851 | 0.001770 | 0.002036 | 0.002499 |
| cmp_qq_test.test | RUN | 0.341707 | 0.714524 | 1.179680 | 0.002113 | 0.001799 | 0.001990 |
| conjugate_test.test | RUN | 0.342739 | 0.734074 | 1.190642 | 0.002162 | 0.002224 | 0.001622 |
| normalize_q_test.test | RUN | 0.343125 | 0.810869 | 1.180790 | 0.001965 | 0.002157 | 0.002216 |
| div_eq_qs_test.test | RUN | 0.340623 | 0.697122 | 1.187992 | 0.001848 | 0.001980 | 0.002080 |
| div_qs_test.test | RUN | 0.345326 | 0.758500 | 1.217348 | 0.002030 | 0.001900 | 0.002008 |
| dot_qq_test.test | RUN | 0.346402 | 0.730375 | 1.146750 | 0.002504 | 0.002164 | 0.001907 |
| eq_qq_test.test | RUN | 0.344305 | 0.765457 | 1.188105 | 0.002686 | 0.002352 | 0.002131 |
| inverse_q_test.test | RUN | 0.423624 | 0.997697 | 1.470746 | 0.002656 | 0.001941 | 0.002284 |
| mag_q_test.test | RUN | 0.361026 | 0.753026 | 1.116119 | 0.001994 | 0.001945 | 0.002096 |
| mag_sqr_q_test.test | RUN | 0.347155 | 0.731400 | 1.140655 | 0.002074 | 0.002177 | 0.001972 |
| minus_q_test.test | RUN | 0.343038 | 0.740022 | 1.181123 | 0.001944 | 0.001878 | 0.002086 |
| minus_eq_qq_test.test | RUN | 0.339364 | 0.736572 | 1.136107 | 0.001980 | 0.001872 | 0.002008 |
| minus_qq_test.test | RUN | 0.340800 | 0.771222 | 1.158062 | 0.001935 | 0.001967 | 0.001890 |
| mul_eq_qs_test.test | RUN | 0.344129 | 0.697383 | 1.180587 | 0.002404 | 0.002266 | 0.002013 |
| mul_qs_test.test | RUN | 0.339842 | 0.741641 | 1.149214 | 0.001953 | 0.002254 | 0.002220 |
| mul_qv_test.test | RUN | 0.423166 | 1.010527 | 1.497711 | 0.002178 | 0.002094 | 0.002080 |
| mul_qq_test.test | RUN | 0.421333 | 1.035962 | 1.523519 | 0.002280 | 0.002016 | 0.002020 |
| mul_eq_qq_test.test | RUN | 0.419907 | 1.035811 | 1.512791 | 0.002025 | 0.001892 | 0.001812 |
| plus_eq_qq_test.test | RUN | 0.336122 | 0.755956 | 1.154469 | 0.002478 | 0.001981 | 0.001770 |
| plus_qq_test.test | RUN | 0.344994 | 0.749475 | 1.199028 | 0.002066 | 0.001938 | 0.002099 |
| scalar_cast_q_test.test | RUN | 0.359200 | 0.726516 | 1.125032 | 0.002125 | 0.002144 | 0.002330 |
| slerp_test.test | RUN | 0.360229 | 1.414095 | 1.420780 | 0.030774 | 0.019333 | 0.021504 |
| convert_to_test.test | RUN | 0.501140 | 1.322750 | 2.204578 | 0.001868 | 0.001760 | 0.002558 |
| rot_mat_test.test | RUN | 0.761987 | 5.743104 | 6.040514 | 0.125096 | 0.053906 | 0.051129 |
| rot_quat_test.test | RUN | 0.414898 | 1.212242 | 1.504651 | 0.002186 | 0.002731 | 0.002215 |
| rotx_mat_test.test | RUN | 0.473475 | 1.581356 | 1.959477 | 0.001996 | 0.002083 | 0.002140 |
| rotx_quat_test.test | RUN | 0.422553 | 1.039421 | 1.412745 | 0.002080 | 0.002108 | 0.002337 |
| roty_mat_test.test | RUN | 0.474647 | 1.543789 | 1.920793 | 0.002205 | 0.002107 | 0.002370 |
| roty_quat_test.test | RUN | 0.416784 | 1.045930 | 1.429213 | 0.002179 | 0.002252 | 0.001799 |
| rotz_mat_test.test | RUN | 0.476324 | 1.736496 | 1.889660 | 0.001822 | 0.001928 | 0.002180 |
| rotz_quat_test.test | RUN | 0.411355 | 1.041851 | 1.419394 | 0.002510 | 0.001950 | 0.002020 |
| zero_vec_test.test | RUN | 0.351797 | 0.893241 | 1.155372 | 0.002116 | 0.001834 | 0.001768 |
| zero_mat_test.test | RUN | 0.444800 | 1.455803 | 1.742614 | 0.002191 | 0.002020 | 0.002173 |
| zero_quat_test.test | RUN | 0.336493 | 0.721734 | 1.136939 | 0.001886 | 0.001618 | 0.002331 |
| identity_mat_test.test | RUN | 0.404484 | 1.116664 | 1.473148 | 0.002080 | 0.001966 | 0.002148 |
| identity_quat_test.test | RUN | 0.346322 | 0.717689 | 1.168814 | 0.002072 | 0.002147 | 0.001888 |
| col_test.test | RUN | 0.645418 | 2.264915 | 3.244387 | 0.001904 | 0.002116 | 0.002236 |
| col_mat_test.test | RUN | 0.435237 | 1.012309 | 1.626365 | 0.002523 | 0.002140 | 0.003357 |
| neg_col_test.test | RUN | 0.484171 | 1.430850 | 2.047131 | 0.002094 | 0.002796 | 0.002028 |
| neg_row_test.test | RUN | 0.524548 | 1.443296 | 2.023357 | 0.001911 | 0.002064 | 0.002122 |
| del_col_test.test | RUN | 0.547508 | 1.583585 | 2.066277 | 0.001836 | 0.002154 | 0.002188 |
| del_row_test.test | RUN | 0.524377 | 1.557531 | 1.998422 | 0.001922 | 0.002302 | 0.002098 |
| del_row_col_test.test | RUN | 0.718597 | 3.015699 | 3.872646 | 0.002413 | 0.002291 | 0.001734 |
| swap_cols_test.test | RUN | 0.980718 | 3.973689 | 4.664974 | 0.001964 | 0.001988 | 0.002119 |
| swap_rows_test.test | RUN | 1.058328 | 4.206690 | 4.625928 | 0.002166 | 0.001800 | 0.002125 |
| diag_test.test | RUN | 0.425755 | 1.072230 | 1.720316 | 0.001936 | 0.002434 | 0.001856 |
| diag_mat_test.test | RUN | 0.455053 | 1.052106 | 1.660366 | 0.002032 | 0.002030 | 0.002278 |
| row_test.test | RUN | 0.653247 | 2.199182 | 3.204992 | 0.002008 | 0.002175 | 0.002054 |
| row_mat_test.test | RUN | 0.432723 | 0.992820 | 1.542831 | 0.001818 | 0.001790 | 0.002252 |
| transpose_test.test | RUN | 0.503160 | 1.522775 | 2.094823 | 0.001945 | 0.002062 | 0.002548 |
| translation_test.test | RUN | 0.407545 | 0.957064 | 1.511708 | 0.001908 | 0.001946 | 0.002524 |
| translation_mat_test.test | RUN | 0.447860 | 0.993463 | 1.693774 | 0.002107 | 0.001926 | 0.001998 |
| swizzle_const_fail.test | COMPILE_FAIL | 0.149772 | 0.302702 | 0.474843 | — | — | — |
| swizzle2_test2.test | RUN | 0.182077 | 0.418016 | 0.648761 | 0.002204 | 0.002406 | 0.001830 |
| swizzle2_test3.test | RUN | 0.174533 | 0.423886 | 0.645079 | 0.001906 | 0.002367 | 0.002901 |
| swizzle2_test4.test | RUN | 0.182384 | 0.423132 | 0.663126 | 0.001991 | 0.002932 | 0.002242 |
| swizzle2_test.test | RUN | 0.587704 | 1.199373 | 2.155186 | 0.002012 | 0.002088 | 0.002326 |
| swizzle3_test2.test | RUN | 0.203503 | 0.455212 | 0.709278 | 0.001967 | 0.002268 | 0.001904 |
| swizzle3_test3.test | RUN | 0.204023 | 0.453707 | 0.757671 | 0.001813 | 0.001788 | 0.002098 |
| swizzle3_test4.test | RUN | 0.201909 | 0.441488 | 0.707094 | 0.001963 | 0.002036 | 0.002030 |
| swizzle3_test.test | RUN | 0.571978 | 1.216824 | 2.132556 | 0.002061 | 0.001968 | 0.002372 |
| swizzle4_test2.test | RUN | 0.347500 | 0.655669 | 1.255143 | 0.001808 | 0.002087 | 0.002074 |
| swizzle4_test3.test | RUN | 0.348055 | 0.651816 | 1.303164 | 0.001936 | 0.002021 | 0.002120 |
| swizzle4_test4.test | RUN | 0.353320 | 0.690836 | 1.280873 | 0.001938 | 0.002290 | 0.002222 |
| swizzle4_test.test | RUN | 0.598320 | 1.315116 | 2.304937 | 0.002144 | 0.002088 | 0.002108 |
| deduce_scalar_test.test | COMPILE | 0.029552 | 0.064698 | 0.108695 | — | — | — |
| deduce_scalar_mq_test.test | RUN | 0.231332 | 0.486782 | 0.745642 | 0.001914 | 0.001958 | 0.001941 |
| deduce_scalar_mv_test.test | RUN | 0.273478 | 0.575610 | 0.921884 | 0.001965 | 0.002682 | 0.002166 |
| deduce_scalar_fail.test | COMPILE_FAIL | 0.027368 | 0.042251 | 0.083840 | — | — | — |
| deduce_mat_test.test | COMPILE | 0.026502 | 0.055939 | 0.089629 | — | — | — |
| deduce_quat_test.test | COMPILE | 0.023650 | 0.054654 | 0.089406 | — | — | — |
| deduce_vec_test.test | COMPILE | 0.027315 | 0.058130 | 0.082846 | — | — | — |
| interop_test.test | RUN | 0.284585 | 0.581249 | 1.406921 | 0.000812 | 0.000626 | 0.002072 |
| transform_test.test | RUN | 0.507255 | 1.011529 | 1.811853 | 0.001560 | 0.001403 | 0.001520 |
| projection_test.test | RUN | 0.316403 | 0.736431 | 1.313051 | 0.001548 | 0.001594 | 0.001491 |
| scalar_traits_test.test | COMPILE | 0.029430 | 0.058694 | 0.096197 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.705× | 0.717×, 0.693× | 0.977× |
| clang | 0.781× | 0.773×, 0.788× | 0.999× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898266).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| access_m_test.test | 10 | 0.001286s | 0.001313s | 0.001324s | byte equality observed |
| access_q_test.test | 10 | 0.001252s | 0.001274s | 0.001294s | byte equality observed |
| access_v_test.test | 10 | 0.001268s | 0.001272s | 0.001263s | byte equality observed |
| assign_test.test | 10 | 0.001291s | 0.001316s | 0.001324s | byte equality observed |
| cmp_mm_test.test | 10 | 0.001321s | 0.001317s | 0.001337s | byte equality observed |
| cmp_qq_test.test | 10 | 0.001325s | 0.001329s | 0.001269s | byte equality observed |
| cmp_vv_test.test | 10 | 0.001318s | 0.001333s | 0.001281s | byte equality observed |
| col_mat_test.test | 10 | 0.001274s | 0.001310s | 0.001281s | byte equality observed |
| col_test.test | 10 | 0.001272s | 0.001318s | 0.001300s | byte equality observed |
| conjugate_test.test | 10 | 0.001339s | 0.001298s | 0.001269s | byte equality observed |
| convert_to_test.test | 10 | 0.001288s | 0.001262s | 0.001277s | byte equality observed |
| cross_test.test | 10 | 0.001315s | 0.001307s | 0.001303s | byte equality observed |
| deduce_scalar_mq_test.test | 10 | 0.001316s | 0.001337s | 0.001316s | byte equality observed |
| deduce_scalar_mv_test.test | 10 | 0.001335s | 0.001289s | 0.001294s | byte equality observed |
| del_col_test.test | 10 | 0.001305s | 0.001311s | 0.001279s | byte equality observed |
| del_row_col_test.test | 10 | 0.001315s | 0.001336s | 0.001337s | byte equality observed |
| del_row_test.test | 10 | 0.001257s | 0.001318s | 0.001342s | byte equality observed |
| determinant_test.test | 10 | 0.001322s | 0.001336s | 0.001329s | byte equality observed |
| diag_mat_test.test | 10 | 0.001313s | 0.001333s | 0.001303s | byte equality observed |
| diag_test.test | 10 | 0.001289s | 0.001300s | 0.001322s | byte equality observed |
| div_eq_ms_test.test | 10 | 0.001299s | 0.001272s | 0.001336s | byte equality observed |
| div_eq_qs_test.test | 10 | 0.001319s | 0.001288s | 0.001271s | byte equality observed |
| div_eq_vs_test.test | 10 | 0.001265s | 0.001339s | 0.001280s | byte equality observed |
| div_ms_test.test | 10 | 0.001330s | 0.001309s | 0.001263s | byte equality observed |
| div_qs_test.test | 10 | 0.001268s | 0.001289s | 0.001304s | byte equality observed |
| div_vs_test.test | 10 | 0.001312s | 0.001270s | 0.001298s | byte equality observed |
| dot_qq_test.test | 10 | 0.001331s | 0.001250s | 0.001288s | byte equality observed |
| dot_vv_test.test | 10 | 0.001312s | 0.001289s | 0.001311s | byte equality observed |
| eq_mm_test.test | 10 | 0.001285s | 0.001269s | 0.001271s | byte equality observed |
| eq_qq_test.test | 10 | 0.001323s | 0.001263s | 0.001349s | byte equality observed |
| eq_vv_test.test | 10 | 0.001314s | 0.001330s | 0.001282s | byte equality observed |
| identity_mat_test.test | 10 | 0.001291s | 0.001288s | 0.001295s | byte equality observed |
| identity_quat_test.test | 10 | 0.001297s | 0.001308s | 0.001236s | byte equality observed |
| interop_test.test | 10 | 0.000605s | 0.000592s | 0.001259s | byte equality observed |
| inverse_m_test.test | 10 | 0.001321s | 0.001356s | 0.001321s | byte equality observed |
| inverse_q_test.test | 10 | 0.001274s | 0.001293s | 0.001279s | byte equality observed |
| mag_q_test.test | 10 | 0.001260s | 0.001245s | 0.001288s | byte equality observed |
| mag_sqr_q_test.test | 10 | 0.001245s | 0.001249s | 0.001307s | byte equality observed |
| mag_sqr_v_test.test | 10 | 0.001318s | 0.001278s | 0.001323s | byte equality observed |
| mag_v_test.test | 10 | 0.001298s | 0.001309s | 0.001309s | byte equality observed |
| mat_index_test.test | 10 | 0.001307s | 0.001299s | 0.001307s | byte equality observed |
| mat_traits_array_test.test | 10 | 0.001325s | 0.001345s | 0.001274s | byte equality observed |
| mat_traits_std_array_test.test | 10 | 0.001228s | 0.001244s | 0.001238s | byte equality observed |
| math_test.test | 10 | 0.001938s | 0.001880s | 0.001703s | byte equality observed |
| minus_eq_mm_test.test | 10 | 0.001249s | 0.001297s | 0.001281s | byte equality observed |
| minus_eq_qq_test.test | 10 | 0.001260s | 0.001288s | 0.001278s | byte equality observed |
| minus_eq_vv_test.test | 10 | 0.001262s | 0.001295s | 0.001304s | byte equality observed |
| minus_m_test.test | 10 | 0.001277s | 0.001279s | 0.001269s | byte equality observed |
| minus_mm_test.test | 10 | 0.001293s | 0.001354s | 0.001275s | byte equality observed |
| minus_q_test.test | 10 | 0.001353s | 0.001337s | 0.001348s | byte equality observed |
| minus_qq_test.test | 10 | 0.001305s | 0.001282s | 0.001338s | byte equality observed |
| minus_v_test.test | 10 | 0.001297s | 0.001369s | 0.001264s | byte equality observed |
| minus_vv_test.test | 10 | 0.001314s | 0.001312s | 0.001332s | byte equality observed |
| mul_eq_mm_test.test | 10 | 0.001299s | 0.001346s | 0.001300s | byte equality observed |
| mul_eq_ms_test.test | 10 | 0.001283s | 0.001326s | 0.001290s | byte equality observed |
| mul_eq_qq_test.test | 10 | 0.001349s | 0.001294s | 0.001318s | byte equality observed |
| mul_eq_qs_test.test | 10 | 0.001279s | 0.001298s | 0.001299s | byte equality observed |
| mul_eq_vs_test.test | 10 | 0.001281s | 0.001320s | 0.001289s | byte equality observed |
| mul_mm_test.test | 10 | 0.001312s | 0.001324s | 0.001279s | byte equality observed |
| mul_ms_test.test | 10 | 0.001315s | 0.001283s | 0.001306s | byte equality observed |
| mul_mv_test.test | 10 | 0.001304s | 0.001279s | 0.001294s | byte equality observed |
| mul_qq_test.test | 10 | 0.001269s | 0.001330s | 0.001295s | byte equality observed |
| mul_qs_test.test | 10 | 0.001275s | 0.001307s | 0.001316s | byte equality observed |
| mul_qv_test.test | 10 | 0.001287s | 0.001288s | 0.001311s | byte equality observed |
| mul_sm_test.test | 10 | 0.001299s | 0.001257s | 0.001298s | byte equality observed |
| mul_sv_test.test | 10 | 0.001317s | 0.001274s | 0.001276s | byte equality observed |
| mul_vm_test.test | 10 | 0.001286s | 0.001284s | 0.001304s | byte equality observed |
| mul_vs_test.test | 9 | 0.001286s | 0.001246s | 0.001294s | byte equality observed |
| neg_col_test.test | 10 | 0.001305s | 0.001330s | 0.001288s | byte equality observed |
| neg_row_test.test | 10 | 0.001266s | 0.001340s | 0.001307s | byte equality observed |
| normalize_q_test.test | 10 | 0.001312s | 0.001319s | 0.001331s | byte equality observed |
| normalize_v_test.test | 10 | 0.001274s | 0.001277s | 0.001299s | byte equality observed |
| plus_eq_mm_test.test | 10 | 0.001278s | 0.001284s | 0.001256s | byte equality observed |
| plus_eq_qq_test.test | 10 | 0.001312s | 0.001274s | 0.001370s | byte equality observed |
| plus_eq_vv_test.test | 10 | 0.001305s | 0.001308s | 0.001331s | byte equality observed |
| plus_mm_test.test | 10 | 0.001303s | 0.001306s | 0.001382s | byte equality observed |
| plus_qq_test.test | 10 | 0.001383s | 0.001331s | 0.001396s | byte equality observed |
| plus_vv_test.test | 10 | 0.001416s | 0.001333s | 0.001437s | byte equality observed |
| projection_test.test | 10 | 0.001464s | 0.001365s | 0.001435s | byte equality observed |
| quat_traits_array_test.test | 10 | 0.001442s | 0.001383s | 0.001487s | byte equality observed |
| quat_traits_std_array_test.test | 10 | 0.001461s | 0.001222s | 0.001420s | byte equality observed |
| rot_mat_test.test | 1 | 0.116401s | 0.034560s | 0.044939s | byte equality observed |
| rot_quat_test.test | 10 | 0.001481s | 0.001194s | 0.001505s | byte equality observed |
| rotx_mat_test.test | 10 | 0.001498s | 0.001336s | 0.001498s | byte equality observed |
| rotx_quat_test.test | 10 | 0.001498s | 0.001463s | 0.001471s | byte equality observed |
| roty_mat_test.test | 10 | 0.001550s | 0.001415s | 0.001524s | byte equality observed |
| roty_quat_test.test | 10 | 0.001510s | 0.001477s | 0.001535s | byte equality observed |
| rotz_mat_test.test | 10 | 0.001543s | 0.001576s | 0.001530s | byte equality observed |
| rotz_quat_test.test | 10 | 0.001490s | 0.001544s | 0.001536s | byte equality observed |
| row_mat_test.test | 10 | 0.001470s | 0.001539s | 0.001514s | byte equality observed |
| row_test.test | 10 | 0.001456s | 0.001517s | 0.001557s | byte equality observed |
| scalar_cast_m_test.test | 10 | 0.001479s | 0.001484s | 0.001534s | byte equality observed |
| scalar_cast_q_test.test | 10 | 0.001467s | 0.001512s | 0.001533s | byte equality observed |
| scalar_cast_v_test.test | 10 | 0.001462s | 0.001476s | 0.001525s | byte equality observed |
| slerp_test.test | 2 | 0.027822s | 0.018578s | 0.020067s | byte equality observed |
| swap_cols_test.test | 10 | 0.001481s | 0.001519s | 0.001447s | byte equality observed |
| swap_rows_test.test | 10 | 0.001530s | 0.001497s | 0.001509s | byte equality observed |
| swizzle2_test.test | 10 | 0.001462s | 0.001478s | 0.001494s | byte equality observed |
| swizzle2_test2.test | 10 | 0.001478s | 0.001531s | 0.001481s | byte equality observed |
| swizzle2_test3.test | 10 | 0.001473s | 0.001503s | 0.001499s | byte equality observed |
| swizzle2_test4.test | 10 | 0.001513s | 0.001526s | 0.001519s | byte equality observed |
| swizzle3_test.test | 10 | 0.001557s | 0.001467s | 0.001533s | byte equality observed |
| swizzle3_test2.test | 10 | 0.001519s | 0.001511s | 0.001523s | byte equality observed |
| swizzle3_test3.test | 10 | 0.001524s | 0.001524s | 0.001562s | byte equality observed |
| swizzle3_test4.test | 10 | 0.001533s | 0.001556s | 0.001559s | byte equality observed |
| swizzle4_test.test | 10 | 0.001528s | 0.001550s | 0.001532s | byte equality observed |
| swizzle4_test2.test | 10 | 0.001528s | 0.001545s | 0.001544s | byte equality observed |
| swizzle4_test3.test | 10 | 0.001533s | 0.001559s | 0.001554s | byte equality observed |
| swizzle4_test4.test | 10 | 0.001546s | 0.001567s | 0.001539s | byte equality observed |
| to_string_test.test | 10 | 0.001590s | 0.001589s | 0.001571s | byte equality observed |
| transform_test.test | 10 | 0.001537s | 0.001537s | 0.001558s | byte equality observed |
| translation_mat_test.test | 10 | 0.001542s | 0.001556s | 0.001526s | byte equality observed |
| translation_test.test | 10 | 0.001529s | 0.001555s | 0.001560s | byte equality observed |
| transpose_test.test | 10 | 0.001519s | 0.001571s | 0.001508s | byte equality observed |
| vec_index_test.test | 10 | 0.001534s | 0.001541s | 0.001545s | byte equality observed |
| vec_register_test.test | 9 | 0.001550s | 0.001553s | 0.001540s | byte equality observed |
| vec_traits_array_test.test | 10 | 0.001499s | 0.001533s | 0.001533s | byte equality observed |
| vec_traits_gnuc_test.test | 10 | 0.001529s | 0.001579s | 0.001532s | byte equality observed |
| vec_traits_std_array_test.test | 10 | 0.001517s | 0.001528s | 0.001555s | byte equality observed |
| zero_mat_test.test | 10 | 0.001548s | 0.001536s | 0.001531s | byte equality observed |
| zero_quat_test.test | 10 | 0.001510s | 0.001568s | 0.001543s | byte equality observed |
| zero_vec_test.test | 10 | 0.001522s | 0.001499s | 0.001555s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898266).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
