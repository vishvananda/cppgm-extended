# libs/qvm/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| math_test.test | RUN | 0.329714 | 0.912384 | 1.234081 | 0.002691 | 0.002785 | 0.002508 |
| mat_traits_array_test.test | RUN | 0.250351 | 0.925199 | 0.987649 | 0.001739 | 0.001878 | 0.001651 |
| mat_traits_std_array_test.test | RUN | 0.252090 | 0.709722 | 1.041525 | 0.001868 | 0.001893 | 0.001798 |
| vec_traits_array_test.test | RUN | 0.225148 | 0.696322 | 0.835124 | 0.001726 | 0.002109 | 0.002160 |
| vec_traits_gnuc_test.test | RUN | 0.412243 | 0.892435 | 1.665157 | 0.001716 | 0.001937 | 0.002105 |
| vec_traits_std_array_test.test | RUN | 0.223719 | 0.556141 | 0.850685 | 0.002038 | 0.001981 | 0.002147 |
| quat_traits_array_test.test | RUN | 0.216372 | 0.724463 | 0.802577 | 0.001834 | 0.001834 | 0.001676 |
| quat_traits_std_array_test.test | RUN | 0.218640 | 0.558701 | 0.806107 | 0.001860 | 0.001957 | 0.002074 |
| access_m_test.test | RUN | 0.354859 | 1.358731 | 1.819050 | 0.001898 | 0.002299 | 0.002022 |
| access_m_fail1.test | COMPILE_FAIL | 0.037264 | 0.062483 | 0.113119 | — | — | — |
| access_m_fail2.test | COMPILE_FAIL | 0.034854 | 0.061749 | 0.125116 | — | — | — |
| access_q_test.test | RUN | 0.289371 | 0.702433 | 1.049195 | 0.001832 | 0.002058 | 0.002175 |
| access_q_fail.test | COMPILE_FAIL | 0.026706 | 0.050625 | 0.099177 | — | — | — |
| access_v_test.test | RUN | 0.286939 | 0.700257 | 1.061762 | 0.001710 | 0.002239 | 0.001802 |
| access_v_fail1.test | COMPILE_FAIL | 0.023977 | 0.039434 | 0.089481 | — | — | — |
| access_v_fail2.test | COMPILE_FAIL | 0.023269 | 0.044559 | 0.089281 | — | — | — |
| assign_test.test | RUN | 0.440149 | 1.139382 | 1.873200 | 0.001789 | 0.002218 | 0.002221 |
| to_string_test.test | RUN | 0.512923 | 1.775857 | 10.458437 | 0.001737 | 0.002218 | 0.002134 |
| cmp_vv_test.test | RUN | 0.366671 | 0.893783 | 1.342050 | 0.001744 | 0.002154 | 0.002046 |
| cross_test.test | RUN | 0.356359 | 0.864129 | 1.322504 | 0.001680 | 0.002106 | 0.002290 |
| div_eq_vs_test.test | RUN | 0.335309 | 0.759024 | 1.278477 | 0.001885 | 0.002104 | 0.002510 |
| div_vs_test.test | RUN | 0.351019 | 0.951344 | 1.333162 | 0.001539 | 0.002074 | 0.002188 |
| dot_vv_test.test | RUN | 0.330640 | 0.825683 | 1.268625 | 0.001851 | 0.001982 | 0.002041 |
| eq_vv_test.test | RUN | 0.362794 | 0.965089 | 1.394694 | 0.001781 | 0.001935 | 0.002052 |
| mag_v_test.test | RUN | 0.337484 | 0.814013 | 1.180868 | 0.001755 | 0.002144 | 0.001766 |
| mag_sqr_v_test.test | RUN | 0.333895 | 0.747691 | 1.253888 | 0.001736 | 0.002225 | 0.002258 |
| minus_v_test.test | RUN | 0.335284 | 0.888435 | 1.375064 | 0.001720 | 0.001926 | 0.002042 |
| minus_eq_vv_test.test | RUN | 0.342149 | 0.869593 | 1.344172 | 0.001771 | 0.002244 | 0.001783 |
| minus_vv_test.test | RUN | 0.351413 | 0.954321 | 1.310451 | 0.001802 | 0.001736 | 0.002064 |
| mul_eq_vs_test.test | RUN | 0.336793 | 0.737185 | 1.341108 | 0.001884 | 0.001921 | 0.001921 |
| mul_vs_test.test | RUN | 0.347424 | 0.944218 | 1.422864 | 0.001727 | 0.002316 | 0.001915 |
| mul_sv_test.test | RUN | 0.354204 | 0.935999 | 1.301765 | 0.001902 | 0.002248 | 0.001904 |
| mul_vm_test.test | RUN | 0.486918 | 1.679886 | 2.165711 | 0.001870 | 0.002302 | 0.002360 |
| normalize_v_test.test | RUN | 0.350241 | 1.161600 | 1.457276 | 0.001872 | 0.001798 | 0.002254 |
| plus_eq_vv_test.test | RUN | 0.329691 | 0.850521 | 1.198324 | 0.002098 | 0.001912 | 0.001866 |
| plus_vv_test.test | RUN | 0.356700 | 0.965473 | 1.308510 | 0.001750 | 0.001914 | 0.001890 |
| scalar_cast_v_test.test | RUN | 0.323093 | 0.699913 | 1.216309 | 0.001193 | 0.002077 | 0.001638 |
| vec_index_test.test | RUN | 0.290842 | 0.729539 | 1.075448 | 0.001864 | 0.001830 | 0.002232 |
| vec_register_test.test | RUN | 0.190283 | 0.654618 | 0.759877 | 0.001938 | 0.002190 | 0.002226 |
| cmp_mm_test.test | RUN | 0.549450 | 2.053979 | 2.632419 | 0.002080 | 0.002080 | 0.002064 |
| determinant_test.test | RUN | 0.388775 | 1.051944 | 1.476937 | 0.002080 | 0.001915 | 0.002442 |
| div_eq_ms_test.test | RUN | 0.406720 | 1.002710 | 1.695120 | 0.002093 | 0.001958 | 0.002046 |
| div_ms_test.test | RUN | 0.467009 | 1.559379 | 2.061881 | 0.001840 | 0.002190 | 0.002166 |
| eq_mm_test.test | RUN | 0.545396 | 1.913814 | 2.528971 | 0.002192 | 0.002089 | 0.001811 |
| minus_m_test.test | RUN | 0.476525 | 1.541405 | 1.913825 | 0.001888 | 0.002154 | 0.002496 |
| minus_eq_mm_test.test | RUN | 0.429925 | 1.424476 | 1.823673 | 0.001820 | 0.002202 | 0.002162 |
| minus_mm_test.test | RUN | 0.506242 | 1.788686 | 2.200581 | 0.002100 | 0.002240 | 0.002201 |
| mul_eq_mm_test.test | RUN | 0.414657 | 1.337295 | 1.649126 | 0.002164 | 0.002278 | 0.002418 |
| mul_eq_ms_test.test | RUN | 0.407328 | 0.969530 | 1.741720 | 0.002049 | 0.002020 | 0.001864 |
| mul_mm_test.test | RUN | 0.556380 | 2.971674 | 2.722844 | 0.001949 | 0.002212 | 0.002066 |
| mul_ms_test.test | RUN | 0.481475 | 1.577080 | 2.140529 | 0.001786 | 0.002173 | 0.002025 |
| mul_sm_test.test | RUN | 0.483607 | 1.563918 | 2.144010 | 0.002366 | 0.002139 | 0.002386 |
| mul_mv_test.test | RUN | 0.502233 | 1.686212 | 2.129421 | 0.001947 | 0.002011 | 0.002226 |
| inverse_m_test.test | RUN | 0.639028 | 2.941662 | 2.881852 | 0.002026 | 0.001858 | 0.002438 |
| plus_eq_mm_test.test | RUN | 0.429673 | 1.415344 | 1.849931 | 0.002028 | 0.001816 | 0.002230 |
| plus_mm_test.test | RUN | 0.493446 | 1.789038 | 2.169113 | 0.002036 | 0.002095 | 0.002334 |
| scalar_cast_m_test.test | RUN | 0.375468 | 0.845407 | 1.366030 | 0.001850 | 0.001864 | 0.002271 |
| mat_index_test.test | RUN | 0.300464 | 0.770711 | 1.146667 | 0.001843 | 0.001980 | 0.002172 |
| cmp_qq_test.test | RUN | 0.338778 | 0.732079 | 1.130083 | 0.002045 | 0.001990 | 0.002265 |
| conjugate_test.test | RUN | 0.329240 | 0.738808 | 1.206707 | 0.002092 | 0.002239 | 0.002177 |
| normalize_q_test.test | RUN | 0.345707 | 0.828491 | 1.213261 | 0.001996 | 0.001394 | 0.002012 |
| div_eq_qs_test.test | RUN | 0.335477 | 0.712773 | 1.174268 | 0.001853 | 0.001676 | 0.002330 |
| div_qs_test.test | RUN | 0.336432 | 0.764020 | 1.186021 | 0.002065 | 0.001958 | 0.002272 |
| dot_qq_test.test | RUN | 0.340535 | 0.725043 | 1.214046 | 0.001855 | 0.002257 | 0.002246 |
| eq_qq_test.test | RUN | 0.338336 | 0.770006 | 1.230887 | 0.001767 | 0.001781 | 0.002206 |
| inverse_q_test.test | RUN | 0.395760 | 1.011434 | 1.514061 | 0.002076 | 0.002111 | 0.002260 |
| mag_q_test.test | RUN | 0.335346 | 0.733437 | 1.251576 | 0.002024 | 0.002142 | 0.002326 |
| mag_sqr_q_test.test | RUN | 0.331070 | 0.732407 | 1.193322 | 0.001840 | 0.002210 | 0.002104 |
| minus_q_test.test | RUN | 0.327529 | 0.746472 | 1.136212 | 0.001796 | 0.001622 | 0.002250 |
| minus_eq_qq_test.test | RUN | 0.331832 | 0.742522 | 1.133623 | 0.001708 | 0.002031 | 0.002274 |
| minus_qq_test.test | RUN | 0.329355 | 0.795044 | 1.242175 | 0.001759 | 0.002261 | 0.002174 |
| mul_eq_qs_test.test | RUN | 0.327559 | 0.717655 | 1.199368 | 0.001822 | 0.002266 | 0.001635 |
| mul_qs_test.test | RUN | 0.325345 | 0.746643 | 1.147977 | 0.001805 | 0.002284 | 0.002373 |
| mul_qv_test.test | RUN | 0.409744 | 1.014243 | 1.595062 | 0.001906 | 0.002025 | 0.002286 |
| mul_qq_test.test | RUN | 0.398411 | 1.089249 | 1.477347 | 0.001877 | 0.001848 | 0.002220 |
| mul_eq_qq_test.test | RUN | 0.391729 | 1.033769 | 1.512522 | 0.001911 | 0.001905 | 0.002010 |
| plus_eq_qq_test.test | RUN | 0.323481 | 0.744068 | 1.214708 | 0.001760 | 0.002060 | 0.002024 |
| plus_qq_test.test | RUN | 0.330766 | 0.790679 | 1.189880 | 0.001814 | 0.002188 | 0.002256 |
| scalar_cast_q_test.test | RUN | 0.324965 | 0.690170 | 1.153061 | 0.001752 | 0.002172 | 0.002288 |
| slerp_test.test | RUN | 0.352470 | 1.403862 | 1.440778 | 0.023645 | 0.020203 | 0.021664 |
| convert_to_test.test | RUN | 0.480713 | 1.347969 | 2.216680 | 0.002005 | 0.002080 | 0.002216 |
| rot_mat_test.test | RUN | 0.709963 | 5.870967 | 6.170750 | 0.060385 | 0.054322 | 0.050475 |
| rot_quat_test.test | RUN | 0.402065 | 1.197242 | 1.560739 | 0.002002 | 0.002037 | 0.002042 |
| rotx_mat_test.test | RUN | 0.454199 | 1.560067 | 2.057292 | 0.001908 | 0.002102 | 0.001887 |
| rotx_quat_test.test | RUN | 0.397855 | 1.042003 | 1.483910 | 0.001830 | 0.002100 | 0.002030 |
| roty_mat_test.test | RUN | 0.443029 | 1.565041 | 1.921136 | 0.002192 | 0.002096 | 0.002186 |
| roty_quat_test.test | RUN | 0.389751 | 1.022419 | 1.467390 | 0.001874 | 0.002226 | 0.002337 |
| rotz_mat_test.test | RUN | 0.475069 | 1.814542 | 1.842314 | 0.001765 | 0.001826 | 0.002072 |
| rotz_quat_test.test | RUN | 0.394905 | 1.014898 | 1.502250 | 0.001877 | 0.001899 | 0.001832 |
| zero_vec_test.test | RUN | 0.341761 | 0.902342 | 1.271603 | 0.001953 | 0.001970 | 0.002132 |
| zero_mat_test.test | RUN | 0.420169 | 1.432166 | 1.615192 | 0.001908 | 0.002185 | 0.002168 |
| zero_quat_test.test | RUN | 0.319286 | 0.736466 | 1.229763 | 0.001562 | 0.002098 | 0.002258 |
| identity_mat_test.test | RUN | 0.390138 | 1.078624 | 1.528802 | 0.001692 | 0.002160 | 0.001997 |
| identity_quat_test.test | RUN | 0.319233 | 0.706685 | 1.169803 | 0.001947 | 0.002230 | 0.002095 |
| col_test.test | RUN | 0.648295 | 2.284795 | 3.294361 | 0.001884 | 0.002276 | 0.002206 |
| col_mat_test.test | RUN | 0.412468 | 1.020402 | 1.589711 | 0.001954 | 0.002302 | 0.002274 |
| neg_col_test.test | RUN | 0.491516 | 1.465341 | 2.049247 | 0.001940 | 0.002098 | 0.002216 |
| neg_row_test.test | RUN | 0.487688 | 1.454557 | 2.027736 | 0.001964 | 0.002132 | 0.002215 |
| del_col_test.test | RUN | 0.512484 | 1.586740 | 2.066141 | 0.001956 | 0.002070 | 0.002255 |
| del_row_test.test | RUN | 0.507280 | 1.580543 | 2.020480 | 0.001977 | 0.002164 | 0.002212 |
| del_row_col_test.test | RUN | 0.713961 | 3.003393 | 3.948355 | 0.001959 | 0.002208 | 0.002068 |
| swap_cols_test.test | RUN | 0.924613 | 4.005938 | 4.760019 | 0.002035 | 0.002202 | 0.002346 |
| swap_rows_test.test | RUN | 1.011604 | 4.204499 | 4.677631 | 0.002027 | 0.001797 | 0.002210 |
| diag_test.test | RUN | 0.432608 | 1.058570 | 1.775795 | 0.001856 | 0.002054 | 0.002008 |
| diag_mat_test.test | RUN | 0.429427 | 1.065217 | 1.619732 | 0.001887 | 0.002159 | 0.001978 |
| row_test.test | RUN | 0.642265 | 2.323271 | 3.242212 | 0.001900 | 0.002102 | 0.002262 |
| row_mat_test.test | RUN | 0.422320 | 0.999802 | 1.640316 | 0.002122 | 0.001808 | 0.002286 |
| transpose_test.test | RUN | 0.496358 | 1.561211 | 2.068801 | 0.001828 | 0.001698 | 0.002182 |
| translation_test.test | RUN | 0.394897 | 0.984200 | 1.627176 | 0.001838 | 0.001918 | 0.002099 |
| translation_mat_test.test | RUN | 0.427681 | 1.028936 | 1.603835 | 0.002038 | 0.001903 | 0.002170 |
| swizzle_const_fail.test | COMPILE_FAIL | 0.144575 | 0.283374 | 0.491959 | — | — | — |
| swizzle2_test2.test | RUN | 0.174746 | 0.424143 | 0.644066 | 0.001708 | 0.001908 | 0.001725 |
| swizzle2_test3.test | RUN | 0.175219 | 0.431500 | 0.644655 | 0.001908 | 0.001786 | 0.001890 |
| swizzle2_test4.test | RUN | 0.174172 | 0.405681 | 0.620974 | 0.001702 | 0.002282 | 0.001933 |
| swizzle2_test.test | RUN | 0.566444 | 1.215743 | 2.201143 | 0.001989 | 0.001855 | 0.002166 |
| swizzle3_test2.test | RUN | 0.198269 | 0.452912 | 0.733858 | 0.001812 | 0.002040 | 0.001951 |
| swizzle3_test3.test | RUN | 0.195933 | 0.468509 | 0.757251 | 0.001834 | 0.001943 | 0.002010 |
| swizzle3_test4.test | RUN | 0.199327 | 0.472174 | 0.723209 | 0.001692 | 0.001943 | 0.002193 |
| swizzle3_test.test | RUN | 0.561442 | 1.218499 | 2.212132 | 0.001735 | 0.001905 | 0.002278 |
| swizzle4_test2.test | RUN | 0.331793 | 0.665734 | 1.318314 | 0.001761 | 0.002272 | 0.002479 |
| swizzle4_test3.test | RUN | 0.342707 | 0.656034 | 1.285400 | 0.001883 | 0.001980 | 0.002058 |
| swizzle4_test4.test | RUN | 0.338651 | 0.683238 | 1.264976 | 0.001789 | 0.002054 | 0.002048 |
| swizzle4_test.test | RUN | 0.566519 | 1.338810 | 2.313245 | 0.001742 | 0.002182 | 0.002235 |
| deduce_scalar_test.test | COMPILE | 0.028311 | 0.064648 | 0.109488 | — | — | — |
| deduce_scalar_mq_test.test | RUN | 0.216248 | 0.501443 | 0.798429 | 0.001862 | 0.002056 | 0.002260 |
| deduce_scalar_mv_test.test | RUN | 0.258441 | 0.572420 | 1.014612 | 0.001834 | 0.002110 | 0.002402 |
| deduce_scalar_fail.test | COMPILE_FAIL | 0.024463 | 0.041225 | 0.075443 | — | — | — |
| deduce_mat_test.test | COMPILE | 0.026775 | 0.057434 | 0.090209 | — | — | — |
| deduce_quat_test.test | COMPILE | 0.025212 | 0.057151 | 0.088353 | — | — | — |
| deduce_vec_test.test | COMPILE | 0.023848 | 0.054923 | 0.090417 | — | — | — |
| interop_test.test | RUN | 0.269529 | 0.650819 | 1.429571 | 0.000674 | 0.000687 | 0.002156 |
| transform_test.test | RUN | 0.509595 | 1.035002 | 1.917091 | 0.001475 | 0.001696 | 0.001528 |
| projection_test.test | RUN | 0.287382 | 0.778263 | 1.112691 | 0.001565 | 0.001615 | 0.001568 |
| scalar_traits_test.test | COMPILE | 0.027599 | 0.058508 | 0.098068 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.858× | 0.855×, 0.862× | 1.004× |
| clang | 0.939× | 0.942×, 0.935× | 1.021× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274616).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| access_m_test.test | 10 | 0.001239s | 0.001283s | 0.001234s | byte equality observed |
| access_q_test.test | 8 | 0.001282s | 0.001266s | 0.001250s | byte equality observed |
| access_v_test.test | 10 | 0.001254s | 0.001233s | 0.001240s | byte equality observed |
| assign_test.test | 10 | 0.001255s | 0.001259s | 0.001279s | byte equality observed |
| cmp_mm_test.test | 10 | 0.001282s | 0.001282s | 0.001283s | byte equality observed |
| cmp_qq_test.test | 10 | 0.001288s | 0.001210s | 0.001278s | byte equality observed |
| cmp_vv_test.test | 10 | 0.001276s | 0.001239s | 0.001257s | byte equality observed |
| col_mat_test.test | 10 | 0.001223s | 0.001283s | 0.001250s | byte equality observed |
| col_test.test | 10 | 0.001250s | 0.001241s | 0.001228s | byte equality observed |
| conjugate_test.test | 10 | 0.001268s | 0.001257s | 0.001217s | byte equality observed |
| convert_to_test.test | 10 | 0.001248s | 0.001238s | 0.001234s | byte equality observed |
| cross_test.test | 10 | 0.001286s | 0.001237s | 0.001255s | byte equality observed |
| deduce_scalar_mq_test.test | 10 | 0.001251s | 0.001276s | 0.001269s | byte equality observed |
| deduce_scalar_mv_test.test | 10 | 0.001219s | 0.001244s | 0.001257s | byte equality observed |
| del_col_test.test | 10 | 0.001224s | 0.001248s | 0.001284s | byte equality observed |
| del_row_col_test.test | 10 | 0.001237s | 0.001196s | 0.001268s | byte equality observed |
| del_row_test.test | 10 | 0.001244s | 0.001281s | 0.001229s | byte equality observed |
| determinant_test.test | 10 | 0.001286s | 0.001257s | 0.001298s | byte equality observed |
| diag_mat_test.test | 10 | 0.001266s | 0.001275s | 0.001311s | byte equality observed |
| diag_test.test | 10 | 0.001284s | 0.001274s | 0.001238s | byte equality observed |
| div_eq_ms_test.test | 10 | 0.001262s | 0.001250s | 0.001294s | byte equality observed |
| div_eq_qs_test.test | 10 | 0.001256s | 0.001242s | 0.001234s | byte equality observed |
| div_eq_vs_test.test | 10 | 0.001271s | 0.001261s | 0.001270s | byte equality observed |
| div_ms_test.test | 10 | 0.001266s | 0.001306s | 0.001237s | byte equality observed |
| div_qs_test.test | 10 | 0.001261s | 0.001287s | 0.001255s | byte equality observed |
| div_vs_test.test | 10 | 0.001275s | 0.001234s | 0.001267s | byte equality observed |
| dot_qq_test.test | 10 | 0.001275s | 0.001272s | 0.001256s | byte equality observed |
| dot_vv_test.test | 10 | 0.001255s | 0.001301s | 0.001250s | byte equality observed |
| eq_mm_test.test | 10 | 0.001248s | 0.001250s | 0.001234s | byte equality observed |
| eq_qq_test.test | 10 | 0.001216s | 0.001298s | 0.001253s | byte equality observed |
| eq_vv_test.test | 10 | 0.001247s | 0.001296s | 0.001262s | byte equality observed |
| identity_mat_test.test | 10 | 0.001278s | 0.001285s | 0.001241s | byte equality observed |
| identity_quat_test.test | 10 | 0.001236s | 0.001267s | 0.001235s | byte equality observed |
| interop_test.test | 10 | 0.000579s | 0.000578s | 0.001221s | byte equality observed |
| inverse_m_test.test | 10 | 0.001329s | 0.001337s | 0.001270s | byte equality observed |
| inverse_q_test.test | 10 | 0.001271s | 0.001305s | 0.001211s | byte equality observed |
| mag_q_test.test | 10 | 0.001237s | 0.001247s | 0.001227s | byte equality observed |
| mag_sqr_q_test.test | 10 | 0.001244s | 0.001243s | 0.001247s | byte equality observed |
| mag_sqr_v_test.test | 10 | 0.001247s | 0.001228s | 0.001246s | byte equality observed |
| mag_v_test.test | 10 | 0.001252s | 0.001273s | 0.001246s | byte equality observed |
| mat_index_test.test | 10 | 0.001280s | 0.001209s | 0.001231s | byte equality observed |
| mat_traits_array_test.test | 10 | 0.001222s | 0.001226s | 0.001231s | byte equality observed |
| mat_traits_std_array_test.test | 10 | 0.001213s | 0.001233s | 0.001226s | byte equality observed |
| math_test.test | 10 | 0.001879s | 0.001785s | 0.001628s | byte equality observed |
| minus_eq_mm_test.test | 10 | 0.001220s | 0.001231s | 0.001221s | byte equality observed |
| minus_eq_qq_test.test | 10 | 0.001251s | 0.001272s | 0.001217s | byte equality observed |
| minus_eq_vv_test.test | 10 | 0.001235s | 0.001266s | 0.001244s | byte equality observed |
| minus_m_test.test | 10 | 0.001265s | 0.001272s | 0.001208s | byte equality observed |
| minus_mm_test.test | 10 | 0.001250s | 0.001275s | 0.001239s | byte equality observed |
| minus_q_test.test | 10 | 0.001273s | 0.001257s | 0.001236s | byte equality observed |
| minus_qq_test.test | 10 | 0.001249s | 0.001269s | 0.001261s | byte equality observed |
| minus_v_test.test | 10 | 0.001251s | 0.001301s | 0.001225s | byte equality observed |
| minus_vv_test.test | 10 | 0.001249s | 0.001262s | 0.001244s | byte equality observed |
| mul_eq_mm_test.test | 10 | 0.001240s | 0.001248s | 0.001274s | byte equality observed |
| mul_eq_ms_test.test | 10 | 0.001247s | 0.001244s | 0.001247s | byte equality observed |
| mul_eq_qq_test.test | 10 | 0.001270s | 0.001263s | 0.001254s | byte equality observed |
| mul_eq_qs_test.test | 10 | 0.001248s | 0.001238s | 0.001245s | byte equality observed |
| mul_eq_vs_test.test | 10 | 0.001239s | 0.001238s | 0.001231s | byte equality observed |
| mul_mm_test.test | 10 | 0.001237s | 0.001264s | 0.001259s | byte equality observed |
| mul_ms_test.test | 10 | 0.001252s | 0.001235s | 0.001237s | byte equality observed |
| mul_mv_test.test | 10 | 0.001263s | 0.001259s | 0.001258s | byte equality observed |
| mul_qq_test.test | 10 | 0.001268s | 0.001289s | 0.001223s | byte equality observed |
| mul_qs_test.test | 10 | 0.001249s | 0.001243s | 0.001251s | byte equality observed |
| mul_qv_test.test | 10 | 0.001236s | 0.001234s | 0.001318s | byte equality observed |
| mul_sm_test.test | 10 | 0.001240s | 0.001346s | 0.001249s | byte equality observed |
| mul_sv_test.test | 10 | 0.001190s | 0.001239s | 0.001213s | byte equality observed |
| mul_vm_test.test | 10 | 0.001282s | 0.001284s | 0.001266s | byte equality observed |
| mul_vs_test.test | 7 | 0.001234s | 0.001245s | 0.001211s | byte equality observed |
| neg_col_test.test | 9 | 0.001294s | 0.001235s | 0.001267s | byte equality observed |
| neg_row_test.test | 10 | 0.001295s | 0.001272s | 0.001297s | byte equality observed |
| normalize_q_test.test | 10 | 0.001253s | 0.001234s | 0.001292s | byte equality observed |
| normalize_v_test.test | 10 | 0.001276s | 0.001245s | 0.001356s | byte equality observed |
| plus_eq_mm_test.test | 10 | 0.001283s | 0.001262s | 0.001332s | byte equality observed |
| plus_eq_qq_test.test | 10 | 0.001362s | 0.001223s | 0.001380s | byte equality observed |
| plus_eq_vv_test.test | 10 | 0.001340s | 0.001275s | 0.001466s | byte equality observed |
| plus_mm_test.test | 10 | 0.001404s | 0.001239s | 0.001483s | byte equality observed |
| plus_qq_test.test | 10 | 0.001503s | 0.001295s | 0.001426s | byte equality observed |
| plus_vv_test.test | 10 | 0.001431s | 0.001335s | 0.001482s | byte equality observed |
| projection_test.test | 10 | 0.001448s | 0.001223s | 0.001478s | byte equality observed |
| quat_traits_array_test.test | 10 | 0.001444s | 0.001233s | 0.001428s | byte equality observed |
| quat_traits_std_array_test.test | 10 | 0.001424s | 0.001180s | 0.001469s | byte equality observed |
| rot_mat_test.test | 1 | 0.056199s | 0.033472s | 0.046758s | byte equality observed |
| rot_quat_test.test | 10 | 0.001430s | 0.001164s | 0.001460s | byte equality observed |
| rotx_mat_test.test | 10 | 0.001443s | 0.001191s | 0.001467s | byte equality observed |
| rotx_quat_test.test | 10 | 0.001459s | 0.001239s | 0.001449s | byte equality observed |
| roty_mat_test.test | 10 | 0.001502s | 0.001260s | 0.001480s | byte equality observed |
| roty_quat_test.test | 10 | 0.001485s | 0.001251s | 0.001509s | byte equality observed |
| rotz_mat_test.test | 10 | 0.001510s | 0.001257s | 0.001499s | byte equality observed |
| rotz_quat_test.test | 10 | 0.001519s | 0.001272s | 0.001501s | byte equality observed |
| row_mat_test.test | 10 | 0.001480s | 0.001238s | 0.001488s | byte equality observed |
| row_test.test | 10 | 0.001495s | 0.001265s | 0.001469s | byte equality observed |
| scalar_cast_m_test.test | 10 | 0.001472s | 0.001288s | 0.001480s | byte equality observed |
| scalar_cast_q_test.test | 10 | 0.001462s | 0.001245s | 0.001488s | byte equality observed |
| scalar_cast_v_test.test | 10 | 0.001421s | 0.001332s | 0.001454s | byte equality observed |
| slerp_test.test | 2 | 0.023506s | 0.015405s | 0.019217s | byte equality observed |
| swap_cols_test.test | 10 | 0.001445s | 0.001260s | 0.001472s | byte equality observed |
| swap_rows_test.test | 10 | 0.001444s | 0.001463s | 0.001461s | byte equality observed |
| swizzle2_test.test | 10 | 0.001414s | 0.001432s | 0.001478s | byte equality observed |
| swizzle2_test2.test | 10 | 0.001450s | 0.001445s | 0.001485s | byte equality observed |
| swizzle2_test3.test | 10 | 0.001447s | 0.001491s | 0.001478s | byte equality observed |
| swizzle2_test4.test | 10 | 0.001473s | 0.001464s | 0.001480s | byte equality observed |
| swizzle3_test.test | 10 | 0.001488s | 0.001454s | 0.001468s | byte equality observed |
| swizzle3_test2.test | 10 | 0.001483s | 0.001476s | 0.001483s | byte equality observed |
| swizzle3_test3.test | 10 | 0.001486s | 0.001502s | 0.001471s | byte equality observed |
| swizzle3_test4.test | 10 | 0.001499s | 0.001504s | 0.001482s | byte equality observed |
| swizzle4_test.test | 10 | 0.001498s | 0.001498s | 0.001482s | byte equality observed |
| swizzle4_test2.test | 10 | 0.001467s | 0.001501s | 0.001503s | byte equality observed |
| swizzle4_test3.test | 10 | 0.001509s | 0.001473s | 0.001513s | byte equality observed |
| swizzle4_test4.test | 10 | 0.001507s | 0.001439s | 0.001498s | byte equality observed |
| to_string_test.test | 10 | 0.001548s | 0.001539s | 0.001535s | byte equality observed |
| transform_test.test | 10 | 0.001500s | 0.001509s | 0.001495s | byte equality observed |
| translation_mat_test.test | 10 | 0.001512s | 0.001501s | 0.001499s | byte equality observed |
| translation_test.test | 10 | 0.001489s | 0.001503s | 0.001472s | byte equality observed |
| transpose_test.test | 10 | 0.001490s | 0.001520s | 0.001514s | byte equality observed |
| vec_index_test.test | 10 | 0.001506s | 0.001472s | 0.001490s | byte equality observed |
| vec_register_test.test | 10 | 0.001514s | 0.001472s | 0.001508s | byte equality observed |
| vec_traits_array_test.test | 10 | 0.001499s | 0.001514s | 0.001478s | byte equality observed |
| vec_traits_gnuc_test.test | 10 | 0.001500s | 0.001496s | 0.001524s | byte equality observed |
| vec_traits_std_array_test.test | 10 | 0.001492s | 0.001483s | 0.001456s | byte equality observed |
| zero_mat_test.test | 10 | 0.001525s | 0.001510s | 0.001524s | byte equality observed |
| zero_quat_test.test | 10 | 0.001479s | 0.001490s | 0.001493s | byte equality observed |
| zero_vec_test.test | 10 | 0.001503s | 0.001489s | 0.001484s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274616).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
