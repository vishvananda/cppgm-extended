# libs/qvm/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| math_test.test | RUN | 0.305089 | 0.634637 | 0.790816 | 0.001991 | 0.002028 | 0.002350 |
| mat_traits_array_test.test | RUN | 0.207401 | 0.634722 | 0.626778 | 0.001618 | 0.001627 | 0.001412 |
| mat_traits_std_array_test.test | RUN | 0.212292 | 0.497023 | 0.619342 | 0.001662 | 0.001599 | 0.001744 |
| vec_traits_array_test.test | RUN | 0.185499 | 0.493204 | 0.529904 | 0.001619 | 0.001647 | 0.001288 |
| vec_traits_gnuc_test.test | RUN | 0.331744 | 0.607654 | 1.036551 | 0.001599 | 0.001653 | 0.001519 |
| vec_traits_std_array_test.test | RUN | 0.183132 | 0.391916 | 0.524155 | 0.001766 | 0.001716 | 0.001620 |
| quat_traits_array_test.test | RUN | 0.180337 | 0.508649 | 0.522701 | 0.001554 | 0.001585 | 0.001601 |
| quat_traits_std_array_test.test | RUN | 0.182552 | 0.400917 | 0.531611 | 0.001582 | 0.001582 | 0.001644 |
| access_m_test.test | RUN | 0.296129 | 0.952629 | 1.178725 | 0.001585 | 0.001798 | 0.001628 |
| access_m_fail1.test | COMPILE_FAIL | 0.028464 | 0.042488 | 0.083493 | — | — | — |
| access_m_fail2.test | COMPILE_FAIL | 0.028180 | 0.041854 | 0.082498 | — | — | — |
| access_q_test.test | RUN | 0.256573 | 0.478850 | 0.723387 | 0.001420 | 0.001831 | 0.001670 |
| access_q_fail.test | COMPILE_FAIL | 0.023540 | 0.032233 | 0.056492 | — | — | — |
| access_v_test.test | RUN | 0.252899 | 0.511500 | 0.686358 | 0.001604 | 0.001758 | 0.001645 |
| access_v_fail1.test | COMPILE_FAIL | 0.020182 | 0.031775 | 0.060918 | — | — | — |
| access_v_fail2.test | COMPILE_FAIL | 0.020982 | 0.031623 | 0.060765 | — | — | — |
| assign_test.test | RUN | 0.363144 | 0.809111 | 1.233147 | 0.001726 | 0.001830 | 0.001834 |
| to_string_test.test | RUN | 0.423857 | 1.282565 | 6.816973 | 0.001692 | 0.001644 | 0.001849 |
| cmp_vv_test.test | RUN | 0.315799 | 0.632127 | 0.823604 | 0.001559 | 0.001843 | 0.001870 |
| cross_test.test | RUN | 0.306554 | 0.594238 | 0.853480 | 0.001350 | 0.001785 | 0.001731 |
| div_eq_vs_test.test | RUN | 0.280048 | 0.534485 | 0.796479 | 0.001740 | 0.001806 | 0.001922 |
| div_vs_test.test | RUN | 0.295373 | 0.667666 | 0.841278 | 0.001695 | 0.001738 | 0.001186 |
| dot_vv_test.test | RUN | 0.277614 | 0.612574 | 0.789232 | 0.001454 | 0.001768 | 0.001842 |
| eq_vv_test.test | RUN | 0.294247 | 0.673242 | 0.875064 | 0.001479 | 0.001764 | 0.001670 |
| mag_v_test.test | RUN | 0.283336 | 0.564822 | 0.734925 | 0.001563 | 0.001793 | 0.001715 |
| mag_sqr_v_test.test | RUN | 0.280510 | 0.551182 | 0.775699 | 0.001786 | 0.001866 | 0.001631 |
| minus_v_test.test | RUN | 0.287114 | 0.651078 | 0.849140 | 0.001342 | 0.001950 | 0.001849 |
| minus_eq_vv_test.test | RUN | 0.284154 | 0.636740 | 0.779577 | 0.001452 | 0.001866 | 0.001557 |
| minus_vv_test.test | RUN | 0.298811 | 0.689321 | 0.871461 | 0.001716 | 0.001824 | 0.001784 |
| mul_eq_vs_test.test | RUN | 0.281070 | 0.544396 | 0.815005 | 0.001876 | 0.001902 | 0.001830 |
| mul_vs_test.test | RUN | 0.300076 | 0.680239 | 0.834641 | 0.001549 | 0.001890 | 0.001535 |
| mul_sv_test.test | RUN | 0.293470 | 0.671963 | 0.866495 | 0.001396 | 0.001930 | 0.001862 |
| mul_vm_test.test | RUN | 0.412010 | 1.186188 | 1.257339 | 0.001623 | 0.001562 | 0.001710 |
| normalize_v_test.test | RUN | 0.300311 | 0.817114 | 0.891520 | 0.001749 | 0.001930 | 0.001667 |
| plus_eq_vv_test.test | RUN | 0.283809 | 0.640323 | 0.836755 | 0.001511 | 0.001698 | 0.001538 |
| plus_vv_test.test | RUN | 0.298860 | 0.668373 | 0.885034 | 0.001417 | 0.001939 | 0.001518 |
| scalar_cast_v_test.test | RUN | 0.271637 | 0.505792 | 0.787815 | 0.001751 | 0.001846 | 0.001251 |
| vec_index_test.test | RUN | 0.242452 | 0.516539 | 0.659150 | 0.001612 | 0.001513 | 0.001489 |
| vec_register_test.test | RUN | 0.154065 | 0.447568 | 0.505169 | 0.001824 | 0.001774 | 0.001483 |
| cmp_mm_test.test | RUN | 0.444051 | 1.450803 | 1.682389 | 0.001890 | 0.001984 | 0.001882 |
| determinant_test.test | RUN | 0.318945 | 0.765028 | 0.968570 | 0.001726 | 0.001929 | 0.001836 |
| div_eq_ms_test.test | RUN | 0.330990 | 0.696825 | 1.090264 | 0.001710 | 0.001722 | 0.001746 |
| div_ms_test.test | RUN | 0.371917 | 1.088247 | 1.312131 | 0.001630 | 0.001738 | 0.001840 |
| eq_mm_test.test | RUN | 0.441592 | 1.348383 | 1.621851 | 0.001882 | 0.001669 | 0.001627 |
| minus_m_test.test | RUN | 0.368590 | 1.083099 | 1.214113 | 0.001349 | 0.001856 | 0.001893 |
| minus_eq_mm_test.test | RUN | 0.346016 | 1.005649 | 1.168343 | 0.001602 | 0.001750 | 0.001691 |
| minus_mm_test.test | RUN | 0.387486 | 1.281261 | 1.351148 | 0.001560 | 0.001890 | 0.001779 |
| mul_eq_mm_test.test | RUN | 0.343594 | 0.952980 | 0.999624 | 0.001836 | 0.001757 | 0.001842 |
| mul_eq_ms_test.test | RUN | 0.336758 | 0.684198 | 1.088949 | 0.001806 | 0.001740 | 0.001922 |
| mul_mm_test.test | RUN | 0.453726 | 2.095240 | 1.738242 | 0.001860 | 0.001507 | 0.001496 |
| mul_ms_test.test | RUN | 0.374658 | 1.106374 | 1.285175 | 0.001532 | 0.001848 | 0.001627 |
| mul_sm_test.test | RUN | 0.388457 | 1.098090 | 1.345265 | 0.002049 | 0.001913 | 0.001934 |
| mul_mv_test.test | RUN | 0.409698 | 1.192665 | 1.344514 | 0.001719 | 0.001854 | 0.001858 |
| inverse_m_test.test | RUN | 0.497286 | 2.134820 | 1.854734 | 0.001932 | 0.001683 | 0.001716 |
| plus_eq_mm_test.test | RUN | 0.355945 | 1.028245 | 1.201744 | 0.001731 | 0.001876 | 0.001880 |
| plus_mm_test.test | RUN | 0.395608 | 1.274852 | 1.338000 | 0.001720 | 0.001910 | 0.001882 |
| scalar_cast_m_test.test | RUN | 0.308212 | 0.572883 | 0.882932 | 0.001712 | 0.001842 | 0.001671 |
| mat_index_test.test | RUN | 0.252358 | 0.553072 | 0.739077 | 0.001561 | 0.001746 | 0.001898 |
| cmp_qq_test.test | RUN | 0.279400 | 0.528846 | 0.738422 | 0.001864 | 0.001922 | 0.001664 |
| conjugate_test.test | RUN | 0.278300 | 0.525625 | 0.761449 | 0.001680 | 0.001594 | 0.001472 |
| normalize_q_test.test | RUN | 0.279153 | 0.584006 | 0.737660 | 0.001768 | 0.001538 | 0.001478 |
| div_eq_qs_test.test | RUN | 0.274540 | 0.507967 | 0.742438 | 0.001675 | 0.001692 | 0.001865 |
| div_qs_test.test | RUN | 0.284530 | 0.543033 | 0.739837 | 0.001814 | 0.001966 | 0.001792 |
| dot_qq_test.test | RUN | 0.278570 | 0.517296 | 0.776899 | 0.001749 | 0.001755 | 0.001635 |
| eq_qq_test.test | RUN | 0.283242 | 0.542055 | 0.781003 | 0.001784 | 0.001968 | 0.001272 |
| inverse_q_test.test | RUN | 0.329132 | 0.716998 | 0.945950 | 0.001409 | 0.001518 | 0.001915 |
| mag_q_test.test | RUN | 0.282048 | 0.537133 | 0.728676 | 0.001784 | 0.001954 | 0.001643 |
| mag_sqr_q_test.test | RUN | 0.277881 | 0.527128 | 0.757709 | 0.001714 | 0.001671 | 0.001618 |
| minus_q_test.test | RUN | 0.274415 | 0.521981 | 0.775538 | 0.001734 | 0.001910 | 0.001578 |
| minus_eq_qq_test.test | RUN | 0.275357 | 0.528770 | 0.726798 | 0.001468 | 0.001813 | 0.001844 |
| minus_qq_test.test | RUN | 0.284364 | 0.543442 | 0.751932 | 0.001836 | 0.001571 | 0.001833 |
| mul_eq_qs_test.test | RUN | 0.281451 | 0.496967 | 0.727744 | 0.001782 | 0.001880 | 0.001640 |
| mul_qs_test.test | RUN | 0.276397 | 0.530429 | 0.748417 | 0.001720 | 0.001819 | 0.001726 |
| mul_qv_test.test | RUN | 0.340603 | 0.725705 | 0.952476 | 0.001797 | 0.001730 | 0.001808 |
| mul_qq_test.test | RUN | 0.323408 | 0.742384 | 0.967915 | 0.001796 | 0.001689 | 0.001778 |
| mul_eq_qq_test.test | RUN | 0.327527 | 0.723538 | 0.971065 | 0.001854 | 0.001661 | 0.001814 |
| plus_eq_qq_test.test | RUN | 0.279336 | 0.538660 | 0.728305 | 0.001595 | 0.001861 | 0.001736 |
| plus_qq_test.test | RUN | 0.278886 | 0.525368 | 0.725733 | 0.001663 | 0.001891 | 0.001947 |
| scalar_cast_q_test.test | RUN | 0.278449 | 0.492205 | 0.759366 | 0.001791 | 0.001819 | 0.001663 |
| slerp_test.test | RUN | 0.298917 | 1.015891 | 0.898755 | 0.019394 | 0.014638 | 0.015668 |
| convert_to_test.test | RUN | 0.397532 | 0.940045 | 1.380436 | 0.001883 | 0.001701 | 0.001884 |
| rot_mat_test.test | RUN | 0.628203 | 4.177810 | 4.086347 | 0.071170 | 0.037987 | 0.036147 |
| rot_quat_test.test | RUN | 0.346117 | 0.850296 | 0.959254 | 0.001798 | 0.001954 | 0.001884 |
| rotx_mat_test.test | RUN | 0.376448 | 1.118398 | 1.257021 | 0.001738 | 0.001867 | 0.001778 |
| rotx_quat_test.test | RUN | 0.330444 | 0.731470 | 0.897770 | 0.001910 | 0.001850 | 0.001896 |
| roty_mat_test.test | RUN | 0.377803 | 1.133349 | 1.212143 | 0.001857 | 0.001920 | 0.001922 |
| roty_quat_test.test | RUN | 0.330761 | 0.731875 | 0.820872 | 0.001681 | 0.001941 | 0.001708 |
| rotz_mat_test.test | RUN | 0.394562 | 1.257562 | 1.312840 | 0.001573 | 0.001695 | 0.001881 |
| rotz_quat_test.test | RUN | 0.332867 | 0.724563 | 0.882161 | 0.001515 | 0.001916 | 0.001683 |
| zero_vec_test.test | RUN | 0.296448 | 0.633688 | 0.727578 | 0.001835 | 0.001788 | 0.001826 |
| zero_mat_test.test | RUN | 0.393923 | 1.043415 | 1.112222 | 0.001516 | 0.001712 | 0.001812 |
| zero_quat_test.test | RUN | 0.271064 | 0.522689 | 0.769743 | 0.001758 | 0.001695 | 0.001722 |
| identity_mat_test.test | RUN | 0.335265 | 0.794290 | 0.954430 | 0.001576 | 0.001695 | 0.001432 |
| identity_quat_test.test | RUN | 0.281445 | 0.502370 | 0.766811 | 0.001718 | 0.001912 | 0.001333 |
| col_test.test | RUN | 0.504484 | 1.606595 | 2.116702 | 0.001684 | 0.002062 | 0.001906 |
| col_mat_test.test | RUN | 0.334976 | 0.743096 | 1.023884 | 0.001728 | 0.001924 | 0.001701 |
| neg_col_test.test | RUN | 0.379562 | 1.024057 | 1.299649 | 0.001873 | 0.001814 | 0.001686 |
| neg_row_test.test | RUN | 0.408774 | 1.043215 | 1.291067 | 0.001763 | 0.001740 | 0.001462 |
| del_col_test.test | RUN | 0.396829 | 1.133663 | 1.291751 | 0.001785 | 0.001936 | 0.001873 |
| del_row_test.test | RUN | 0.397350 | 1.118245 | 1.337363 | 0.001787 | 0.001833 | 0.001870 |
| del_row_col_test.test | RUN | 0.542810 | 2.147997 | 2.515688 | 0.001790 | 0.001827 | 0.001920 |
| swap_cols_test.test | RUN | 0.746867 | 2.837190 | 3.076464 | 0.001491 | 0.001776 | 0.001862 |
| swap_rows_test.test | RUN | 0.807281 | 2.992081 | 3.023793 | 0.001777 | 0.001882 | 0.001784 |
| diag_test.test | RUN | 0.341853 | 0.772299 | 1.083693 | 0.001853 | 0.001846 | 0.001866 |
| diag_mat_test.test | RUN | 0.361787 | 0.764110 | 1.076700 | 0.001717 | 0.001823 | 0.001888 |
| row_test.test | RUN | 0.501221 | 1.629736 | 2.163062 | 0.001851 | 0.001923 | 0.001822 |
| row_mat_test.test | RUN | 0.333452 | 0.737128 | 0.961617 | 0.001883 | 0.001671 | 0.001862 |
| transpose_test.test | RUN | 0.388668 | 1.082292 | 1.351068 | 0.001674 | 0.001943 | 0.001621 |
| translation_test.test | RUN | 0.321694 | 0.673451 | 0.947708 | 0.001601 | 0.001907 | 0.001747 |
| translation_mat_test.test | RUN | 0.362546 | 0.714736 | 1.066669 | 0.001790 | 0.001758 | 0.001822 |
| swizzle_const_fail.test | COMPILE_FAIL | 0.121788 | 0.214006 | 0.279868 | — | — | — |
| swizzle2_test2.test | RUN | 0.151448 | 0.296163 | 0.405793 | 0.001309 | 0.001804 | 0.001672 |
| swizzle2_test3.test | RUN | 0.148204 | 0.300837 | 0.433448 | 0.001775 | 0.001694 | 0.001257 |
| swizzle2_test4.test | RUN | 0.147585 | 0.300484 | 0.407459 | 0.001630 | 0.001889 | 0.001429 |
| swizzle2_test.test | RUN | 0.469335 | 0.834352 | 1.354963 | 0.001908 | 0.001714 | 0.001815 |
| swizzle3_test2.test | RUN | 0.164155 | 0.326019 | 0.469307 | 0.001808 | 0.001890 | 0.001577 |
| swizzle3_test3.test | RUN | 0.161968 | 0.315672 | 0.469964 | 0.001854 | 0.001884 | 0.001514 |
| swizzle3_test4.test | RUN | 0.161740 | 0.318767 | 0.483943 | 0.001560 | 0.001803 | 0.001808 |
| swizzle3_test.test | RUN | 0.456685 | 0.866907 | 1.391407 | 0.001762 | 0.001569 | 0.001883 |
| swizzle4_test2.test | RUN | 0.269812 | 0.473616 | 0.837483 | 0.001618 | 0.001958 | 0.001832 |
| swizzle4_test3.test | RUN | 0.282448 | 0.455000 | 0.782238 | 0.001597 | 0.001617 | 0.001766 |
| swizzle4_test4.test | RUN | 0.284636 | 0.480219 | 0.756331 | 0.001616 | 0.001566 | 0.001898 |
| swizzle4_test.test | RUN | 0.472581 | 0.925173 | 1.316920 | 0.001799 | 0.001916 | 0.001819 |
| deduce_scalar_test.test | COMPILE | 0.024716 | 0.047945 | 0.072967 | — | — | — |
| deduce_scalar_mq_test.test | RUN | 0.182106 | 0.356171 | 0.493659 | 0.001928 | 0.001624 | 0.001825 |
| deduce_scalar_mv_test.test | RUN | 0.207042 | 0.411731 | 0.562330 | 0.001307 | 0.001746 | 0.001850 |
| deduce_scalar_fail.test | COMPILE_FAIL | 0.019920 | 0.030185 | 0.057283 | — | — | — |
| deduce_mat_test.test | COMPILE | 0.021404 | 0.041453 | 0.064953 | — | — | — |
| deduce_quat_test.test | COMPILE | 0.021289 | 0.040640 | 0.061779 | — | — | — |
| deduce_vec_test.test | COMPILE | 0.021484 | 0.038311 | 0.063542 | — | — | — |
| interop_test.test | RUN | 0.216103 | 0.509342 | 0.888672 | 0.000663 | 0.000847 | 0.001328 |
| transform_test.test | RUN | 0.426656 | 0.694046 | 1.237341 | 0.001346 | 0.001436 | 0.001236 |
| projection_test.test | RUN | 0.297951 | 0.579500 | 0.627196 | 0.001430 | 0.001509 | 0.002002 |
| scalar_traits_test.test | COMPILE | 0.023061 | 0.042546 | 0.068550 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.895×† | 0.939×, 0.850× | 1.019× |
| clang | 0.893×† | 0.987×, 0.799× | 0.925× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| access_m_test.test | 10 | 0.001045s | 0.001068s | 0.001036s | byte equality observed |
| access_q_test.test | 10 | 0.001040s | 0.001047s | 0.001040s | byte equality observed |
| access_v_test.test | 10 | 0.001036s | 0.001034s | 0.001048s | byte equality observed |
| assign_test.test | 10 | 0.001036s | 0.001024s | 0.001030s | byte equality observed |
| cmp_mm_test.test | 10 | 0.001085s | 0.001037s | 0.001051s | byte equality observed |
| cmp_qq_test.test | 10 | 0.001032s | 0.001020s | 0.001017s | byte equality observed |
| cmp_vv_test.test | 10 | 0.001037s | 0.001067s | 0.001035s | byte equality observed |
| col_mat_test.test | 10 | 0.001026s | 0.001050s | 0.001034s | byte equality observed |
| col_test.test | 10 | 0.001054s | 0.001062s | 0.001049s | byte equality observed |
| conjugate_test.test | 10 | 0.001034s | 0.001039s | 0.001052s | byte equality observed |
| convert_to_test.test | 10 | 0.001039s | 0.001015s | 0.001046s | byte equality observed |
| cross_test.test | 10 | 0.001053s | 0.001025s | 0.001046s | byte equality observed |
| deduce_scalar_mq_test.test | 10 | 0.001055s | 0.001030s | 0.001055s | byte equality observed |
| deduce_scalar_mv_test.test | 10 | 0.001060s | 0.001037s | 0.001066s | byte equality observed |
| del_col_test.test | 10 | 0.001037s | 0.001031s | 0.001029s | byte equality observed |
| del_row_col_test.test | 10 | 0.001068s | 0.001082s | 0.001038s | byte equality observed |
| del_row_test.test | 10 | 0.001030s | 0.001052s | 0.001063s | byte equality observed |
| determinant_test.test | 10 | 0.001047s | 0.001051s | 0.001103s | byte equality observed |
| diag_mat_test.test | 10 | 0.001052s | 0.001057s | 0.001076s | byte equality observed |
| diag_test.test | 10 | 0.001034s | 0.001026s | 0.001036s | byte equality observed |
| div_eq_ms_test.test | 10 | 0.001025s | 0.001039s | 0.001058s | byte equality observed |
| div_eq_qs_test.test | 10 | 0.001019s | 0.001060s | 0.001054s | byte equality observed |
| div_eq_vs_test.test | 10 | 0.001038s | 0.001027s | 0.001064s | byte equality observed |
| div_ms_test.test | 10 | 0.001012s | 0.001047s | 0.001030s | byte equality observed |
| div_qs_test.test | 10 | 0.001010s | 0.001085s | 0.001064s | byte equality observed |
| div_vs_test.test | 10 | 0.001048s | 0.001025s | 0.001048s | byte equality observed |
| dot_qq_test.test | 10 | 0.001032s | 0.001009s | 0.001090s | byte equality observed |
| dot_vv_test.test | 10 | 0.001022s | 0.001036s | 0.001093s | byte equality observed |
| eq_mm_test.test | 10 | 0.001048s | 0.001020s | 0.001012s | byte equality observed |
| eq_qq_test.test | 10 | 0.001036s | 0.001007s | 0.001054s | byte equality observed |
| eq_vv_test.test | 10 | 0.001019s | 0.001054s | 0.001025s | byte equality observed |
| identity_mat_test.test | 10 | 0.001039s | 0.001023s | 0.001032s | byte equality observed |
| identity_quat_test.test | 10 | 0.001050s | 0.001074s | 0.001042s | byte equality observed |
| interop_test.test | 10 | 0.000490s | 0.000484s | 0.001087s | byte equality observed |
| inverse_m_test.test | 10 | 0.001101s | 0.001092s | 0.001087s | byte equality observed |
| inverse_q_test.test | 10 | 0.001041s | 0.001054s | 0.001111s | byte equality observed |
| mag_q_test.test | 10 | 0.001028s | 0.001025s | 0.001023s | byte equality observed |
| mag_sqr_q_test.test | 10 | 0.001032s | 0.001016s | 0.001063s | byte equality observed |
| mag_sqr_v_test.test | 10 | 0.001037s | 0.001035s | 0.001047s | byte equality observed |
| mag_v_test.test | 10 | 0.001044s | 0.001029s | 0.001043s | byte equality observed |
| mat_index_test.test | 10 | 0.001064s | 0.001020s | 0.001038s | byte equality observed |
| mat_traits_array_test.test | 10 | 0.001053s | 0.001077s | 0.001033s | byte equality observed |
| mat_traits_std_array_test.test | 10 | 0.001048s | 0.001024s | 0.001053s | byte equality observed |
| math_test.test | 10 | 0.001584s | 0.001513s | 0.001384s | byte equality observed |
| minus_eq_mm_test.test | 10 | 0.001030s | 0.001018s | 0.001035s | byte equality observed |
| minus_eq_qq_test.test | 10 | 0.001049s | 0.001029s | 0.001023s | byte equality observed |
| minus_eq_vv_test.test | 10 | 0.001033s | 0.001011s | 0.001017s | byte equality observed |
| minus_m_test.test | 10 | 0.001023s | 0.001039s | 0.001060s | byte equality observed |
| minus_mm_test.test | 10 | 0.001026s | 0.001035s | 0.001074s | byte equality observed |
| minus_q_test.test | 10 | 0.001027s | 0.001030s | 0.001066s | byte equality observed |
| minus_qq_test.test | 10 | 0.001036s | 0.001021s | 0.001046s | byte equality observed |
| minus_v_test.test | 10 | 0.001031s | 0.001031s | 0.001031s | byte equality observed |
| minus_vv_test.test | 10 | 0.001033s | 0.001051s | 0.001067s | byte equality observed |
| mul_eq_mm_test.test | 10 | 0.001036s | 0.001062s | 0.001068s | byte equality observed |
| mul_eq_ms_test.test | 10 | 0.001032s | 0.001059s | 0.001060s | byte equality observed |
| mul_eq_qq_test.test | 10 | 0.001053s | 0.001098s | 0.001132s | byte equality observed |
| mul_eq_qs_test.test | 10 | 0.001037s | 0.001121s | 0.001062s | byte equality observed |
| mul_eq_vs_test.test | 10 | 0.001067s | 0.001043s | 0.001088s | byte equality observed |
| mul_mm_test.test | 10 | 0.001067s | 0.001044s | 0.001105s | byte equality observed |
| mul_ms_test.test | 10 | 0.001038s | 0.001041s | 0.001058s | byte equality observed |
| mul_mv_test.test | 10 | 0.001025s | 0.001046s | 0.001048s | byte equality observed |
| mul_qq_test.test | 10 | 0.001049s | 0.001068s | 0.001105s | byte equality observed |
| mul_qs_test.test | 10 | 0.001025s | 0.001100s | 0.001034s | byte equality observed |
| mul_qv_test.test | 10 | 0.001047s | 0.001065s | 0.001073s | byte equality observed |
| mul_sm_test.test | 10 | 0.001029s | 0.001085s | 0.001066s | byte equality observed |
| mul_sv_test.test | 10 | 0.001052s | 0.001041s | 0.001055s | byte equality observed |
| mul_vm_test.test | 10 | 0.001057s | 0.001080s | 0.001065s | byte equality observed |
| mul_vs_test.test | 10 | 0.001033s | 0.001036s | 0.001025s | byte equality observed |
| neg_col_test.test | 10 | 0.001040s | 0.001051s | 0.001067s | byte equality observed |
| neg_row_test.test | 10 | 0.001070s | 0.001043s | 0.001017s | byte equality observed |
| normalize_q_test.test | 10 | 0.001062s | 0.001053s | 0.001047s | byte equality observed |
| normalize_v_test.test | 10 | 0.001046s | 0.001047s | 0.001043s | byte equality observed |
| plus_eq_mm_test.test | 10 | 0.001039s | 0.001065s | 0.001038s | byte equality observed |
| plus_eq_qq_test.test | 10 | 0.001038s | 0.001032s | 0.001042s | byte equality observed |
| plus_eq_vv_test.test | 10 | 0.001031s | 0.001080s | 0.001034s | byte equality observed |
| plus_mm_test.test | 10 | 0.001042s | 0.001011s | 0.001030s | byte equality observed |
| plus_qq_test.test | 10 | 0.001062s | 0.001017s | 0.001042s | byte equality observed |
| plus_vv_test.test | 10 | 0.001038s | 0.001060s | 0.001038s | byte equality observed |
| projection_test.test | 10 | 0.001025s | 0.001078s | 0.001001s | byte equality observed |
| quat_traits_array_test.test | 10 | 0.001040s | 0.001019s | 0.001022s | byte equality observed |
| quat_traits_std_array_test.test | 10 | 0.001000s | 0.001002s | 0.001020s | byte equality observed |
| rot_mat_test.test | 1 | 0.044933s | 0.024775s | 0.026818s | byte equality observed |
| rot_quat_test.test | 10 | 0.001003s | 0.001039s | 0.001016s | byte equality observed |
| rotx_mat_test.test | 10 | 0.001040s | 0.001047s | 0.001093s | byte equality observed |
| rotx_quat_test.test | 10 | 0.001049s | 0.001035s | 0.001142s | byte equality observed |
| roty_mat_test.test | 10 | 0.001094s | 0.001033s | 0.001130s | byte equality observed |
| roty_quat_test.test | 10 | 0.001047s | 0.001033s | 0.001116s | byte equality observed |
| rotz_mat_test.test | 10 | 0.001076s | 0.001041s | 0.001142s | byte equality observed |
| rotz_quat_test.test | 10 | 0.001064s | 0.001030s | 0.001164s | byte equality observed |
| row_mat_test.test | 10 | 0.001045s | 0.001068s | 0.001109s | byte equality observed |
| row_test.test | 10 | 0.001092s | 0.001015s | 0.001150s | byte equality observed |
| scalar_cast_m_test.test | 10 | 0.001019s | 0.001038s | 0.001080s | byte equality observed |
| scalar_cast_q_test.test | 10 | 0.001085s | 0.001095s | 0.001184s | byte equality observed |
| scalar_cast_v_test.test | 10 | 0.001045s | 0.001005s | 0.001120s | byte equality observed |
| slerp_test.test | 2 | 0.014495s | 0.011361s | 0.011762s | byte equality observed |
| swap_cols_test.test | 10 | 0.001007s | 0.000967s | 0.000999s | byte equality observed |
| swap_rows_test.test | 10 | 0.001053s | 0.001032s | 0.001065s | byte equality observed |
| swizzle2_test.test | 10 | 0.001077s | 0.001052s | 0.001092s | byte equality observed |
| swizzle2_test2.test | 10 | 0.001072s | 0.001036s | 0.001114s | byte equality observed |
| swizzle2_test3.test | 10 | 0.001027s | 0.001069s | 0.001109s | byte equality observed |
| swizzle2_test4.test | 10 | 0.001023s | 0.001033s | 0.001125s | byte equality observed |
| swizzle3_test.test | 10 | 0.001010s | 0.001064s | 0.001102s | byte equality observed |
| swizzle3_test2.test | 10 | 0.001028s | 0.001066s | 0.001096s | byte equality observed |
| swizzle3_test3.test | 10 | 0.001026s | 0.001040s | 0.001152s | byte equality observed |
| swizzle3_test4.test | 10 | 0.001055s | 0.001030s | 0.001122s | byte equality observed |
| swizzle4_test.test | 10 | 0.001053s | 0.001062s | 0.001110s | byte equality observed |
| swizzle4_test2.test | 10 | 0.001041s | 0.001033s | 0.001136s | byte equality observed |
| swizzle4_test3.test | 10 | 0.001039s | 0.001023s | 0.001145s | byte equality observed |
| swizzle4_test4.test | 10 | 0.001040s | 0.001075s | 0.001140s | byte equality observed |
| to_string_test.test | 10 | 0.001083s | 0.001098s | 0.001132s | byte equality observed |
| transform_test.test | 10 | 0.001061s | 0.001091s | 0.001149s | byte equality observed |
| translation_mat_test.test | 10 | 0.001059s | 0.001043s | 0.001101s | byte equality observed |
| translation_test.test | 10 | 0.001055s | 0.001029s | 0.001139s | byte equality observed |
| transpose_test.test | 10 | 0.001036s | 0.001055s | 0.001109s | byte equality observed |
| vec_index_test.test | 10 | 0.001057s | 0.001043s | 0.001123s | byte equality observed |
| vec_register_test.test | 10 | 0.001055s | 0.001028s | 0.001088s | byte equality observed |
| vec_traits_array_test.test | 10 | 0.001057s | 0.001027s | 0.001122s | byte equality observed |
| vec_traits_gnuc_test.test | 10 | 0.001050s | 0.001066s | 0.001121s | byte equality observed |
| vec_traits_std_array_test.test | 10 | 0.001065s | 0.001027s | 0.001132s | byte equality observed |
| zero_mat_test.test | 10 | 0.001045s | 0.001029s | 0.001117s | byte equality observed |
| zero_quat_test.test | 10 | 0.001077s | 0.001062s | 0.001110s | byte equality observed |
| zero_vec_test.test | 10 | 0.001075s | 0.001019s | 0.001141s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
