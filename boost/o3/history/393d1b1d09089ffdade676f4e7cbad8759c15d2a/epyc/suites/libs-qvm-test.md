# libs/qvm/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| math_test.test | RUN | 0.244294 | 0.632852 | 0.785783 | 0.002062 | 0.002054 | 0.002178 |
| mat_traits_array_test.test | RUN | 0.188417 | 0.621698 | 0.605632 | 0.001216 | 0.001526 | 0.001603 |
| mat_traits_std_array_test.test | RUN | 0.191793 | 0.488210 | 0.619800 | 0.001584 | 0.001574 | 0.001474 |
| vec_traits_array_test.test | RUN | 0.165782 | 0.496301 | 0.523836 | 0.001620 | 0.001540 | 0.001497 |
| vec_traits_gnuc_test.test | RUN | 0.300735 | 0.611999 | 1.012768 | 0.001690 | 0.001548 | 0.001605 |
| vec_traits_std_array_test.test | RUN | 0.161803 | 0.391928 | 0.521757 | 0.001695 | 0.001712 | 0.001417 |
| quat_traits_array_test.test | RUN | 0.159732 | 0.512781 | 0.516426 | 0.001584 | 0.001540 | 0.001552 |
| quat_traits_std_array_test.test | RUN | 0.157870 | 0.395799 | 0.505165 | 0.001349 | 0.001539 | 0.001712 |
| access_m_test.test | RUN | 0.259182 | 0.937298 | 1.110577 | 0.001586 | 0.001801 | 0.001767 |
| access_m_fail1.test | COMPILE_FAIL | 0.026552 | 0.041683 | 0.089563 | — | — | — |
| access_m_fail2.test | COMPILE_FAIL | 0.024828 | 0.042941 | 0.085222 | — | — | — |
| access_q_test.test | RUN | 0.216475 | 0.490869 | 0.682325 | 0.001330 | 0.001747 | 0.001696 |
| access_q_fail.test | COMPILE_FAIL | 0.019216 | 0.030067 | 0.068160 | — | — | — |
| access_v_test.test | RUN | 0.216252 | 0.488722 | 0.672935 | 0.001486 | 0.001804 | 0.001569 |
| access_v_fail1.test | COMPILE_FAIL | 0.017602 | 0.028963 | 0.060551 | — | — | — |
| access_v_fail2.test | COMPILE_FAIL | 0.017989 | 0.030005 | 0.057298 | — | — | — |
| assign_test.test | RUN | 0.323234 | 0.788113 | 1.161156 | 0.001565 | 0.001824 | 0.001765 |
| to_string_test.test | RUN | 0.374380 | 1.260603 | 6.650782 | 0.001434 | 0.001794 | 0.001802 |
| cmp_vv_test.test | RUN | 0.271078 | 0.624594 | 0.840127 | 0.001549 | 0.001790 | 0.001517 |
| cross_test.test | RUN | 0.266592 | 0.598278 | 0.799903 | 0.001488 | 0.001806 | 0.001726 |
| div_eq_vs_test.test | RUN | 0.247326 | 0.530591 | 0.762789 | 0.001507 | 0.001684 | 0.001651 |
| div_vs_test.test | RUN | 0.254818 | 0.645439 | 0.865721 | 0.001440 | 0.001802 | 0.001514 |
| dot_vv_test.test | RUN | 0.245815 | 0.589319 | 0.807558 | 0.001411 | 0.001782 | 0.001681 |
| eq_vv_test.test | RUN | 0.268420 | 0.663146 | 0.841325 | 0.001496 | 0.001628 | 0.001728 |
| mag_v_test.test | RUN | 0.243428 | 0.539819 | 0.793093 | 0.001601 | 0.001768 | 0.001692 |
| mag_sqr_v_test.test | RUN | 0.246535 | 0.542454 | 0.796780 | 0.001447 | 0.001839 | 0.001646 |
| minus_v_test.test | RUN | 0.250040 | 0.626583 | 0.799276 | 0.001291 | 0.001888 | 0.001685 |
| minus_eq_vv_test.test | RUN | 0.247630 | 0.615429 | 0.832148 | 0.001508 | 0.001871 | 0.001553 |
| minus_vv_test.test | RUN | 0.258541 | 0.657470 | 0.876127 | 0.001646 | 0.001731 | 0.001626 |
| mul_eq_vs_test.test | RUN | 0.242217 | 0.544415 | 0.770035 | 0.001746 | 0.001720 | 0.001760 |
| mul_vs_test.test | RUN | 0.252960 | 0.659709 | 0.877502 | 0.001419 | 0.001751 | 0.001584 |
| mul_sv_test.test | RUN | 0.266070 | 0.664941 | 0.882266 | 0.001542 | 0.001837 | 0.001762 |
| mul_vm_test.test | RUN | 0.369521 | 1.165154 | 1.232840 | 0.001527 | 0.001610 | 0.001565 |
| normalize_v_test.test | RUN | 0.263803 | 0.823593 | 0.878421 | 0.001468 | 0.001612 | 0.001460 |
| plus_eq_vv_test.test | RUN | 0.245271 | 0.605554 | 0.829952 | 0.001657 | 0.001688 | 0.001616 |
| plus_vv_test.test | RUN | 0.258974 | 0.661723 | 0.858052 | 0.001319 | 0.002078 | 0.001465 |
| scalar_cast_v_test.test | RUN | 0.237308 | 0.495542 | 0.723872 | 0.001334 | 0.001736 | 0.001552 |
| vec_index_test.test | RUN | 0.206262 | 0.515830 | 0.692551 | 0.001624 | 0.001784 | 0.001549 |
| vec_register_test.test | RUN | 0.134862 | 0.457907 | 0.499727 | 0.001870 | 0.001322 | 0.001421 |
| cmp_mm_test.test | RUN | 0.407639 | 1.424695 | 1.663763 | 0.001749 | 0.001581 | 0.001775 |
| determinant_test.test | RUN | 0.282257 | 0.736453 | 0.952974 | 0.001313 | 0.001754 | 0.001453 |
| div_eq_ms_test.test | RUN | 0.293007 | 0.695518 | 1.062431 | 0.001523 | 0.001580 | 0.001679 |
| div_ms_test.test | RUN | 0.339162 | 1.080943 | 1.304109 | 0.001411 | 0.001820 | 0.001682 |
| eq_mm_test.test | RUN | 0.387727 | 1.345257 | 1.590386 | 0.001844 | 0.001672 | 0.001690 |
| minus_m_test.test | RUN | 0.345018 | 1.074103 | 1.251071 | 0.001753 | 0.001784 | 0.001624 |
| minus_eq_mm_test.test | RUN | 0.316829 | 1.011948 | 1.137880 | 0.001371 | 0.001695 | 0.001477 |
| minus_mm_test.test | RUN | 0.369059 | 1.283740 | 1.353643 | 0.001793 | 0.001867 | 0.001694 |
| mul_eq_mm_test.test | RUN | 0.299810 | 0.928782 | 1.029232 | 0.001794 | 0.001483 | 0.001749 |
| mul_eq_ms_test.test | RUN | 0.297033 | 0.693646 | 1.062217 | 0.001596 | 0.001746 | 0.001710 |
| mul_mm_test.test | RUN | 0.420820 | 2.061892 | 1.664226 | 0.001745 | 0.001694 | 0.001601 |
| mul_ms_test.test | RUN | 0.355983 | 1.085540 | 1.301689 | 0.001523 | 0.001768 | 0.001727 |
| mul_sm_test.test | RUN | 0.356794 | 1.087443 | 1.327318 | 0.001748 | 0.001737 | 0.001727 |
| mul_mv_test.test | RUN | 0.381392 | 1.171540 | 1.335761 | 0.001647 | 0.001871 | 0.001728 |
| inverse_m_test.test | RUN | 0.462231 | 2.112827 | 1.792604 | 0.001784 | 0.001669 | 0.001731 |
| plus_eq_mm_test.test | RUN | 0.329768 | 0.981126 | 1.179048 | 0.001802 | 0.001830 | 0.001726 |
| plus_mm_test.test | RUN | 0.369683 | 1.271206 | 1.336269 | 0.001536 | 0.001693 | 0.001327 |
| scalar_cast_m_test.test | RUN | 0.276434 | 0.585601 | 0.884995 | 0.001794 | 0.001757 | 0.001708 |
| mat_index_test.test | RUN | 0.221683 | 0.540768 | 0.719241 | 0.001540 | 0.001871 | 0.001760 |
| cmp_qq_test.test | RUN | 0.242539 | 0.518892 | 0.705157 | 0.001702 | 0.001678 | 0.001578 |
| conjugate_test.test | RUN | 0.241262 | 0.522036 | 0.733767 | 0.001622 | 0.001720 | 0.001534 |
| normalize_q_test.test | RUN | 0.245166 | 0.580914 | 0.770002 | 0.001707 | 0.001649 | 0.001745 |
| div_eq_qs_test.test | RUN | 0.245530 | 0.498908 | 0.729656 | 0.001615 | 0.001723 | 0.001739 |
| div_qs_test.test | RUN | 0.241632 | 0.538032 | 0.741959 | 0.001740 | 0.001744 | 0.001436 |
| dot_qq_test.test | RUN | 0.242456 | 0.515766 | 0.742158 | 0.001671 | 0.001775 | 0.001716 |
| eq_qq_test.test | RUN | 0.249386 | 0.538669 | 0.756980 | 0.001632 | 0.001779 | 0.001714 |
| inverse_q_test.test | RUN | 0.296395 | 0.700351 | 0.917138 | 0.001713 | 0.001498 | 0.001844 |
| mag_q_test.test | RUN | 0.244735 | 0.533301 | 0.721890 | 0.001527 | 0.001516 | 0.001552 |
| mag_sqr_q_test.test | RUN | 0.240381 | 0.518255 | 0.724320 | 0.001772 | 0.001762 | 0.001726 |
| minus_q_test.test | RUN | 0.242660 | 0.529334 | 0.737801 | 0.001639 | 0.001865 | 0.001713 |
| minus_eq_qq_test.test | RUN | 0.240883 | 0.528297 | 0.719793 | 0.001728 | 0.001726 | 0.001710 |
| minus_qq_test.test | RUN | 0.242516 | 0.540693 | 0.745737 | 0.001514 | 0.001525 | 0.001618 |
| mul_eq_qs_test.test | RUN | 0.240504 | 0.495152 | 0.722988 | 0.001658 | 0.001853 | 0.001666 |
| mul_qs_test.test | RUN | 0.239391 | 0.532477 | 0.743099 | 0.001800 | 0.001792 | 0.001718 |
| mul_qv_test.test | RUN | 0.300480 | 0.719979 | 0.963009 | 0.001763 | 0.001770 | 0.001669 |
| mul_qq_test.test | RUN | 0.289300 | 0.731801 | 0.920673 | 0.001573 | 0.001738 | 0.001723 |
| mul_eq_qq_test.test | RUN | 0.293215 | 0.731049 | 0.934040 | 0.001750 | 0.001764 | 0.001643 |
| plus_eq_qq_test.test | RUN | 0.243687 | 0.539634 | 0.751289 | 0.001537 | 0.001628 | 0.001633 |
| plus_qq_test.test | RUN | 0.248659 | 0.541686 | 0.745365 | 0.001692 | 0.001616 | 0.001688 |
| scalar_cast_q_test.test | RUN | 0.243713 | 0.508328 | 0.731225 | 0.001561 | 0.001524 | 0.001728 |
| slerp_test.test | RUN | 0.263537 | 0.996945 | 0.898917 | 0.020113 | 0.014623 | 0.015249 |
| convert_to_test.test | RUN | 0.360842 | 0.933573 | 1.352394 | 0.001738 | 0.001718 | 0.001793 |
| rot_mat_test.test | RUN | 0.546577 | 4.128035 | 4.013983 | 0.089166 | 0.040378 | 0.036181 |
| rot_quat_test.test | RUN | 0.295987 | 0.842001 | 0.987032 | 0.001758 | 0.001847 | 0.001686 |
| rotx_mat_test.test | RUN | 0.337729 | 1.119271 | 1.211798 | 0.001656 | 0.001872 | 0.001722 |
| rotx_quat_test.test | RUN | 0.295900 | 0.731782 | 0.904157 | 0.001547 | 0.001808 | 0.001745 |
| roty_mat_test.test | RUN | 0.326664 | 1.111288 | 1.192521 | 0.001858 | 0.001741 | 0.001627 |
| roty_quat_test.test | RUN | 0.286205 | 0.726360 | 0.914704 | 0.001680 | 0.001801 | 0.001789 |
| rotz_mat_test.test | RUN | 0.351465 | 1.266223 | 1.322696 | 0.001407 | 0.001724 | 0.001798 |
| rotz_quat_test.test | RUN | 0.286340 | 0.716555 | 0.849977 | 0.001434 | 0.001725 | 0.001680 |
| zero_vec_test.test | RUN | 0.248425 | 0.628569 | 0.729899 | 0.001733 | 0.001685 | 0.001699 |
| zero_mat_test.test | RUN | 0.308703 | 1.022830 | 1.059906 | 0.001807 | 0.001602 | 0.001784 |
| zero_quat_test.test | RUN | 0.240146 | 0.539695 | 0.717301 | 0.001465 | 0.001379 | 0.001658 |
| identity_mat_test.test | RUN | 0.287657 | 0.747475 | 0.994942 | 0.001679 | 0.001537 | 0.001630 |
| identity_quat_test.test | RUN | 0.246527 | 0.513466 | 0.736285 | 0.001619 | 0.001785 | 0.001778 |
| col_test.test | RUN | 0.467971 | 1.578277 | 2.037225 | 0.001687 | 0.001812 | 0.001779 |
| col_mat_test.test | RUN | 0.302093 | 0.716625 | 0.988625 | 0.001760 | 0.001813 | 0.001678 |
| neg_col_test.test | RUN | 0.356671 | 1.009457 | 1.281776 | 0.004000 | 0.001817 | 0.001642 |
| neg_row_test.test | RUN | 0.406094 | 1.046778 | 1.273803 | 0.001729 | 0.001843 | 0.001529 |
| del_col_test.test | RUN | 0.399226 | 1.145430 | 1.283377 | 0.001772 | 0.001884 | 0.001724 |
| del_row_test.test | RUN | 0.364585 | 1.088536 | 1.329687 | 0.001770 | 0.001844 | 0.001713 |
| del_row_col_test.test | RUN | 0.512820 | 2.127376 | 2.465087 | 0.001540 | 0.001804 | 0.001766 |
| swap_cols_test.test | RUN | 0.695510 | 2.812564 | 2.979029 | 0.001764 | 0.001796 | 0.001631 |
| swap_rows_test.test | RUN | 0.749123 | 2.956032 | 2.900360 | 0.001765 | 0.001510 | 0.001644 |
| diag_test.test | RUN | 0.310285 | 0.770435 | 1.110335 | 0.001802 | 0.001756 | 0.001720 |
| diag_mat_test.test | RUN | 0.325107 | 0.745202 | 0.951198 | 0.001610 | 0.001852 | 0.001846 |
| row_test.test | RUN | 0.472517 | 1.588400 | 2.005650 | 0.001746 | 0.001834 | 0.001670 |
| row_mat_test.test | RUN | 0.299408 | 0.698253 | 0.972113 | 0.001765 | 0.001627 | 0.001690 |
| transpose_test.test | RUN | 0.368003 | 1.098813 | 1.307719 | 0.001763 | 0.001763 | 0.001437 |
| translation_test.test | RUN | 0.289006 | 0.675531 | 0.962685 | 0.001530 | 0.001853 | 0.001690 |
| translation_mat_test.test | RUN | 0.320708 | 0.700441 | 1.072130 | 0.001636 | 0.001531 | 0.001543 |
| swizzle_const_fail.test | COMPILE_FAIL | 0.106580 | 0.209787 | 0.271682 | — | — | — |
| swizzle2_test2.test | RUN | 0.130255 | 0.295824 | 0.419962 | 0.001581 | 0.001876 | 0.001468 |
| swizzle2_test3.test | RUN | 0.125575 | 0.295822 | 0.403624 | 0.001461 | 0.001703 | 0.001201 |
| swizzle2_test4.test | RUN | 0.131029 | 0.294590 | 0.407181 | 0.001394 | 0.001782 | 0.001361 |
| swizzle2_test.test | RUN | 0.414231 | 0.830630 | 1.352117 | 0.001656 | 0.001500 | 0.001439 |
| swizzle3_test2.test | RUN | 0.145344 | 0.323831 | 0.459841 | 0.001683 | 0.001827 | 0.001414 |
| swizzle3_test3.test | RUN | 0.142620 | 0.310288 | 0.459516 | 0.001392 | 0.001681 | 0.001542 |
| swizzle3_test4.test | RUN | 0.144868 | 0.315659 | 0.473274 | 0.001430 | 0.001700 | 0.001568 |
| swizzle3_test.test | RUN | 0.411010 | 0.850799 | 1.344947 | 0.001586 | 0.001726 | 0.001764 |
| swizzle4_test2.test | RUN | 0.240463 | 0.469035 | 0.796079 | 0.001798 | 0.001813 | 0.001731 |
| swizzle4_test3.test | RUN | 0.238427 | 0.460907 | 0.820082 | 0.001356 | 0.001576 | 0.001608 |
| swizzle4_test4.test | RUN | 0.251039 | 0.464396 | 0.795258 | 0.001540 | 0.001558 | 0.001396 |
| swizzle4_test.test | RUN | 0.420047 | 0.926339 | 1.454545 | 0.001344 | 0.001840 | 0.001829 |
| deduce_scalar_test.test | COMPILE | 0.021761 | 0.045964 | 0.075378 | — | — | — |
| deduce_scalar_mq_test.test | RUN | 0.159108 | 0.349018 | 0.447121 | 0.001598 | 0.001573 | 0.001421 |
| deduce_scalar_mv_test.test | RUN | 0.185497 | 0.410536 | 0.618226 | 0.001628 | 0.001718 | 0.001754 |
| deduce_scalar_fail.test | COMPILE_FAIL | 0.017334 | 0.027685 | 0.050231 | — | — | — |
| deduce_mat_test.test | COMPILE | 0.018331 | 0.041476 | 0.056716 | — | — | — |
| deduce_quat_test.test | COMPILE | 0.018256 | 0.038778 | 0.056412 | — | — | — |
| deduce_vec_test.test | COMPILE | 0.018655 | 0.037750 | 0.055991 | — | — | — |
| interop_test.test | RUN | 0.191887 | 0.472133 | 0.862833 | 0.000741 | 0.000592 | 0.001755 |
| transform_test.test | RUN | 0.354050 | 0.751244 | 1.118407 | 0.001286 | 0.001301 | 0.001193 |
| projection_test.test | RUN | 0.251628 | 0.559250 | 0.773579 | 0.001303 | 0.001308 | 0.001274 |
| scalar_traits_test.test | COMPILE | 0.019886 | 0.041188 | 0.064671 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.814× | 0.811×, 0.817× | 0.956× |
| clang | 0.858×† | 0.828×, 0.888× | 1.009× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224918).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| access_m_test.test | 10 | 0.000999s | 0.000987s | 0.000984s | byte equality observed |
| access_q_test.test | 10 | 0.001012s | 0.001019s | 0.001003s | byte equality observed |
| access_v_test.test | 10 | 0.001035s | 0.000988s | 0.001023s | byte equality observed |
| assign_test.test | 10 | 0.000997s | 0.001030s | 0.000990s | byte equality observed |
| cmp_mm_test.test | 10 | 0.001012s | 0.001010s | 0.001015s | byte equality observed |
| cmp_qq_test.test | 10 | 0.001008s | 0.001029s | 0.000968s | byte equality observed |
| cmp_vv_test.test | 10 | 0.000980s | 0.001013s | 0.000989s | byte equality observed |
| col_mat_test.test | 10 | 0.000994s | 0.001004s | 0.000998s | byte equality observed |
| col_test.test | 10 | 0.000990s | 0.001021s | 0.001012s | byte equality observed |
| conjugate_test.test | 10 | 0.001000s | 0.000998s | 0.001024s | byte equality observed |
| convert_to_test.test | 10 | 0.000996s | 0.001010s | 0.001015s | byte equality observed |
| cross_test.test | 10 | 0.001000s | 0.001011s | 0.001089s | byte equality observed |
| deduce_scalar_mq_test.test | 10 | 0.001002s | 0.001016s | 0.000998s | byte equality observed |
| deduce_scalar_mv_test.test | 10 | 0.001014s | 0.000990s | 0.001008s | byte equality observed |
| del_col_test.test | 10 | 0.000990s | 0.000985s | 0.000988s | byte equality observed |
| del_row_col_test.test | 10 | 0.000995s | 0.000988s | 0.000998s | byte equality observed |
| del_row_test.test | 10 | 0.000999s | 0.001009s | 0.001020s | byte equality observed |
| determinant_test.test | 10 | 0.001002s | 0.001009s | 0.001040s | byte equality observed |
| diag_mat_test.test | 10 | 0.001004s | 0.001003s | 0.000985s | byte equality observed |
| diag_test.test | 10 | 0.000989s | 0.000988s | 0.001007s | byte equality observed |
| div_eq_ms_test.test | 10 | 0.000996s | 0.001001s | 0.001000s | byte equality observed |
| div_eq_qs_test.test | 10 | 0.000990s | 0.000997s | 0.000984s | byte equality observed |
| div_eq_vs_test.test | 10 | 0.000986s | 0.000970s | 0.000989s | byte equality observed |
| div_ms_test.test | 10 | 0.001002s | 0.000982s | 0.000985s | byte equality observed |
| div_qs_test.test | 10 | 0.001012s | 0.000990s | 0.001001s | byte equality observed |
| div_vs_test.test | 10 | 0.000997s | 0.000993s | 0.000991s | byte equality observed |
| dot_qq_test.test | 10 | 0.001000s | 0.000985s | 0.000991s | byte equality observed |
| dot_vv_test.test | 10 | 0.000988s | 0.000989s | 0.001008s | byte equality observed |
| eq_mm_test.test | 10 | 0.000979s | 0.001039s | 0.001042s | byte equality observed |
| eq_qq_test.test | 10 | 0.001022s | 0.000993s | 0.001015s | byte equality observed |
| eq_vv_test.test | 10 | 0.001007s | 0.001009s | 0.001008s | byte equality observed |
| identity_mat_test.test | 10 | 0.001008s | 0.001026s | 0.001021s | byte equality observed |
| identity_quat_test.test | 10 | 0.000997s | 0.000985s | 0.001008s | byte equality observed |
| interop_test.test | 10 | 0.000462s | 0.000470s | 0.000978s | byte equality observed |
| inverse_m_test.test | 10 | 0.001070s | 0.001052s | 0.001035s | byte equality observed |
| inverse_q_test.test | 10 | 0.001002s | 0.001011s | 0.000995s | byte equality observed |
| mag_q_test.test | 10 | 0.001007s | 0.000980s | 0.000992s | byte equality observed |
| mag_sqr_q_test.test | 10 | 0.001000s | 0.000996s | 0.001059s | byte equality observed |
| mag_sqr_v_test.test | 10 | 0.001006s | 0.000975s | 0.001033s | byte equality observed |
| mag_v_test.test | 10 | 0.001014s | 0.000996s | 0.000995s | byte equality observed |
| mat_index_test.test | 10 | 0.001012s | 0.000992s | 0.001025s | byte equality observed |
| mat_traits_array_test.test | 10 | 0.001007s | 0.000975s | 0.001023s | byte equality observed |
| mat_traits_std_array_test.test | 10 | 0.000997s | 0.000978s | 0.000989s | byte equality observed |
| math_test.test | 10 | 0.001533s | 0.001472s | 0.001338s | byte equality observed |
| minus_eq_mm_test.test | 10 | 0.001022s | 0.001001s | 0.001009s | byte equality observed |
| minus_eq_qq_test.test | 10 | 0.000983s | 0.001030s | 0.000985s | byte equality observed |
| minus_eq_vv_test.test | 10 | 0.000994s | 0.000984s | 0.000987s | byte equality observed |
| minus_m_test.test | 10 | 0.001000s | 0.001010s | 0.001038s | byte equality observed |
| minus_mm_test.test | 10 | 0.000993s | 0.001006s | 0.001003s | byte equality observed |
| minus_q_test.test | 10 | 0.000998s | 0.001029s | 0.000999s | byte equality observed |
| minus_qq_test.test | 10 | 0.000992s | 0.000999s | 0.001043s | byte equality observed |
| minus_v_test.test | 10 | 0.001007s | 0.001015s | 0.001059s | byte equality observed |
| minus_vv_test.test | 10 | 0.001021s | 0.000993s | 0.001035s | byte equality observed |
| mul_eq_mm_test.test | 10 | 0.000996s | 0.000984s | 0.000990s | byte equality observed |
| mul_eq_ms_test.test | 10 | 0.000996s | 0.001015s | 0.001055s | byte equality observed |
| mul_eq_qq_test.test | 10 | 0.001023s | 0.000999s | 0.001020s | byte equality observed |
| mul_eq_qs_test.test | 10 | 0.001011s | 0.000974s | 0.000990s | byte equality observed |
| mul_eq_vs_test.test | 10 | 0.000995s | 0.001005s | 0.000988s | byte equality observed |
| mul_mm_test.test | 10 | 0.001033s | 0.001012s | 0.001014s | byte equality observed |
| mul_ms_test.test | 10 | 0.001013s | 0.001038s | 0.001058s | byte equality observed |
| mul_mv_test.test | 10 | 0.000994s | 0.000998s | 0.001000s | byte equality observed |
| mul_qq_test.test | 10 | 0.001038s | 0.001026s | 0.001014s | byte equality observed |
| mul_qs_test.test | 10 | 0.000993s | 0.000994s | 0.001038s | byte equality observed |
| mul_qv_test.test | 10 | 0.001000s | 0.001008s | 0.001048s | byte equality observed |
| mul_sm_test.test | 10 | 0.001008s | 0.001053s | 0.001030s | byte equality observed |
| mul_sv_test.test | 10 | 0.000987s | 0.001024s | 0.001018s | byte equality observed |
| mul_vm_test.test | 10 | 0.001006s | 0.001007s | 0.001043s | byte equality observed |
| mul_vs_test.test | 10 | 0.001020s | 0.000988s | 0.000987s | byte equality observed |
| neg_col_test.test | 10 | 0.000989s | 0.001020s | 0.001003s | byte equality observed |
| neg_row_test.test | 10 | 0.001015s | 0.000991s | 0.001005s | byte equality observed |
| normalize_q_test.test | 10 | 0.001026s | 0.001011s | 0.001012s | byte equality observed |
| normalize_v_test.test | 10 | 0.001009s | 0.000987s | 0.001055s | byte equality observed |
| plus_eq_mm_test.test | 10 | 0.001001s | 0.000996s | 0.001034s | byte equality observed |
| plus_eq_qq_test.test | 10 | 0.000998s | 0.000984s | 0.000996s | byte equality observed |
| plus_eq_vv_test.test | 10 | 0.001012s | 0.000985s | 0.001001s | byte equality observed |
| plus_mm_test.test | 10 | 0.000995s | 0.000986s | 0.001018s | byte equality observed |
| plus_qq_test.test | 10 | 0.000998s | 0.000988s | 0.001042s | byte equality observed |
| plus_vv_test.test | 10 | 0.000992s | 0.001002s | 0.000972s | byte equality observed |
| projection_test.test | 10 | 0.001005s | 0.000986s | 0.001017s | byte equality observed |
| quat_traits_array_test.test | 10 | 0.001000s | 0.001033s | 0.001019s | byte equality observed |
| quat_traits_std_array_test.test | 10 | 0.000970s | 0.000991s | 0.000992s | byte equality observed |
| rot_mat_test.test | 1 | 0.056450s | 0.026071s | 0.025806s | byte equality observed |
| rot_quat_test.test | 10 | 0.000973s | 0.000942s | 0.000964s | byte equality observed |
| rotx_mat_test.test | 10 | 0.000981s | 0.000975s | 0.000984s | byte equality observed |
| rotx_quat_test.test | 10 | 0.000968s | 0.000980s | 0.001056s | byte equality observed |
| roty_mat_test.test | 10 | 0.001010s | 0.001029s | 0.000995s | byte equality observed |
| roty_quat_test.test | 10 | 0.001008s | 0.000981s | 0.001007s | byte equality observed |
| rotz_mat_test.test | 10 | 0.001061s | 0.001036s | 0.001026s | byte equality observed |
| rotz_quat_test.test | 10 | 0.001002s | 0.000987s | 0.001008s | byte equality observed |
| row_mat_test.test | 10 | 0.000985s | 0.000989s | 0.000990s | byte equality observed |
| row_test.test | 10 | 0.000983s | 0.001020s | 0.001007s | byte equality observed |
| scalar_cast_m_test.test | 10 | 0.000997s | 0.000984s | 0.001010s | byte equality observed |
| scalar_cast_q_test.test | 10 | 0.001019s | 0.000980s | 0.001011s | byte equality observed |
| scalar_cast_v_test.test | 10 | 0.000983s | 0.000975s | 0.000961s | byte equality observed |
| slerp_test.test | 2 | 0.014539s | 0.010861s | 0.011820s | byte equality observed |
| swap_cols_test.test | 10 | 0.000966s | 0.000972s | 0.000952s | byte equality observed |
| swap_rows_test.test | 10 | 0.000975s | 0.001014s | 0.000979s | byte equality observed |
| swizzle2_test.test | 10 | 0.000990s | 0.000997s | 0.001027s | byte equality observed |
| swizzle2_test2.test | 10 | 0.001000s | 0.000990s | 0.001003s | byte equality observed |
| swizzle2_test3.test | 10 | 0.001002s | 0.001011s | 0.000979s | byte equality observed |
| swizzle2_test4.test | 10 | 0.001011s | 0.001008s | 0.001038s | byte equality observed |
| swizzle3_test.test | 10 | 0.001005s | 0.001011s | 0.000988s | byte equality observed |
| swizzle3_test2.test | 10 | 0.001033s | 0.000986s | 0.000996s | byte equality observed |
| swizzle3_test3.test | 10 | 0.000983s | 0.001004s | 0.000978s | byte equality observed |
| swizzle3_test4.test | 10 | 0.000993s | 0.000981s | 0.001036s | byte equality observed |
| swizzle4_test.test | 10 | 0.001019s | 0.000991s | 0.001002s | byte equality observed |
| swizzle4_test2.test | 10 | 0.000989s | 0.000971s | 0.001015s | byte equality observed |
| swizzle4_test3.test | 10 | 0.001013s | 0.001024s | 0.001017s | byte equality observed |
| swizzle4_test4.test | 10 | 0.001001s | 0.001005s | 0.001012s | byte equality observed |
| to_string_test.test | 10 | 0.001033s | 0.001013s | 0.001038s | byte equality observed |
| transform_test.test | 10 | 0.001003s | 0.000993s | 0.001009s | byte equality observed |
| translation_mat_test.test | 10 | 0.000988s | 0.000976s | 0.001025s | byte equality observed |
| translation_test.test | 10 | 0.001004s | 0.000986s | 0.001025s | byte equality observed |
| transpose_test.test | 10 | 0.001004s | 0.001016s | 0.001018s | byte equality observed |
| vec_index_test.test | 10 | 0.000998s | 0.001020s | 0.001019s | byte equality observed |
| vec_register_test.test | 10 | 0.001017s | 0.000999s | 0.001049s | byte equality observed |
| vec_traits_array_test.test | 10 | 0.001012s | 0.000989s | 0.001036s | byte equality observed |
| vec_traits_gnuc_test.test | 10 | 0.001022s | 0.001043s | 0.001016s | byte equality observed |
| vec_traits_std_array_test.test | 10 | 0.001012s | 0.000975s | 0.000993s | byte equality observed |
| zero_mat_test.test | 10 | 0.000982s | 0.000988s | 0.001035s | byte equality observed |
| zero_quat_test.test | 10 | 0.001002s | 0.000980s | 0.001011s | byte equality observed |
| zero_vec_test.test | 10 | 0.000995s | 0.001000s | 0.001021s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224918).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
