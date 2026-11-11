# libs/qvm/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| math_test.test | RUN | 0.380220 | 0.789007 | 0.833610 | 0.002538 | 0.002262 | 0.002206 |
| mat_traits_array_test.test | RUN | 0.247849 | 0.794546 | 0.652239 | 0.001476 | 0.001728 | 0.001554 |
| mat_traits_std_array_test.test | RUN | 0.244183 | 0.602962 | 0.665163 | 0.001704 | 0.001540 | 0.001460 |
| vec_traits_array_test.test | RUN | 0.212712 | 0.595809 | 0.561878 | 0.001546 | 0.001796 | 0.001535 |
| vec_traits_gnuc_test.test | RUN | 0.390895 | 0.726291 | 1.039906 | 0.001485 | 0.001567 | 0.001761 |
| vec_traits_std_array_test.test | RUN | 0.211975 | 0.466234 | 0.551103 | 0.001868 | 0.001703 | 0.001656 |
| quat_traits_array_test.test | RUN | 0.208485 | 0.632240 | 0.545963 | 0.001735 | 0.001646 | 0.001754 |
| quat_traits_std_array_test.test | RUN | 0.209871 | 0.473361 | 0.548746 | 0.001745 | 0.001606 | 0.001740 |
| access_m_test.test | RUN | 0.329378 | 1.125971 | 1.262019 | 0.001838 | 0.001867 | 0.001652 |
| access_m_fail1.test | COMPILE_FAIL | 0.032280 | 0.048197 | 0.078961 | — | — | — |
| access_m_fail2.test | COMPILE_FAIL | 0.032726 | 0.048065 | 0.076357 | — | — | — |
| access_q_test.test | RUN | 0.291374 | 0.568749 | 0.745082 | 0.001901 | 0.001650 | 0.001804 |
| access_q_fail.test | COMPILE_FAIL | 0.025187 | 0.037844 | 0.065088 | — | — | — |
| access_v_test.test | RUN | 0.284857 | 0.604989 | 0.743200 | 0.001717 | 0.001784 | 0.001702 |
| access_v_fail1.test | COMPILE_FAIL | 0.022587 | 0.035758 | 0.058855 | — | — | — |
| access_v_fail2.test | COMPILE_FAIL | 0.022519 | 0.035860 | 0.059854 | — | — | — |
| assign_test.test | RUN | 0.424413 | 0.956162 | 1.349957 | 0.001735 | 0.001772 | 0.001721 |
| to_string_test.test | RUN | 0.518698 | 1.588038 | 8.083678 | 0.001703 | 0.001850 | 0.001660 |
| cmp_vv_test.test | RUN | 0.371371 | 0.776433 | 0.880821 | 0.001838 | 0.001692 | 0.001615 |
| cross_test.test | RUN | 0.351925 | 0.715328 | 0.880242 | 0.001588 | 0.001875 | 0.001458 |
| div_eq_vs_test.test | RUN | 0.322757 | 0.649645 | 0.860401 | 0.001868 | 0.001736 | 0.001744 |
| div_vs_test.test | RUN | 0.346529 | 0.807007 | 0.933017 | 0.001766 | 0.001792 | 0.001487 |
| dot_vv_test.test | RUN | 0.331364 | 0.723683 | 0.866262 | 0.001768 | 0.001823 | 0.001730 |
| eq_vv_test.test | RUN | 0.358972 | 0.818904 | 0.937344 | 0.001739 | 0.001618 | 0.001709 |
| mag_v_test.test | RUN | 0.319790 | 0.663360 | 0.829127 | 0.001658 | 0.001797 | 0.001548 |
| mag_sqr_v_test.test | RUN | 0.329776 | 0.662894 | 0.823065 | 0.001524 | 0.001832 | 0.001486 |
| minus_v_test.test | RUN | 0.332002 | 0.769886 | 0.911865 | 0.001436 | 0.001812 | 0.001656 |
| minus_eq_vv_test.test | RUN | 0.331239 | 0.749989 | 0.873002 | 0.001801 | 0.001754 | 0.001805 |
| minus_vv_test.test | RUN | 0.349021 | 0.822582 | 0.937418 | 0.001746 | 0.001802 | 0.001472 |
| mul_eq_vs_test.test | RUN | 0.327868 | 0.665453 | 0.852151 | 0.001799 | 0.001795 | 0.001866 |
| mul_vs_test.test | RUN | 0.349610 | 0.805925 | 0.941916 | 0.001581 | 0.001792 | 0.001775 |
| mul_sv_test.test | RUN | 0.348238 | 0.819117 | 0.953605 | 0.001806 | 0.001771 | 0.001682 |
| mul_vm_test.test | RUN | 0.489147 | 1.447255 | 1.399660 | 0.001565 | 0.001770 | 0.001764 |
| normalize_v_test.test | RUN | 0.350699 | 0.999707 | 0.961358 | 0.001543 | 0.001836 | 0.001589 |
| plus_eq_vv_test.test | RUN | 0.322414 | 0.764068 | 0.892165 | 0.001583 | 0.001844 | 0.001507 |
| plus_vv_test.test | RUN | 0.342771 | 0.816332 | 0.934517 | 0.001717 | 0.001808 | 0.001733 |
| scalar_cast_v_test.test | RUN | 0.320205 | 0.607768 | 0.809809 | 0.001474 | 0.001676 | 0.001610 |
| vec_index_test.test | RUN | 0.285758 | 0.617060 | 0.694093 | 0.001775 | 0.001763 | 0.001679 |
| vec_register_test.test | RUN | 0.178378 | 0.559633 | 0.535349 | 0.001722 | 0.001691 | 0.001767 |
| cmp_mm_test.test | RUN | 0.536882 | 1.801797 | 1.931615 | 0.001805 | 0.001792 | 0.001720 |
| determinant_test.test | RUN | 0.372932 | 0.922828 | 1.008435 | 0.001676 | 0.001793 | 0.001776 |
| div_eq_ms_test.test | RUN | 0.386707 | 0.840620 | 1.143859 | 0.001830 | 0.001582 | 0.001704 |
| div_ms_test.test | RUN | 0.425935 | 1.352973 | 1.410119 | 0.001527 | 0.001643 | 0.001711 |
| eq_mm_test.test | RUN | 0.506827 | 1.655570 | 1.833472 | 0.001840 | 0.001746 | 0.001768 |
| minus_m_test.test | RUN | 0.430232 | 1.333119 | 1.389136 | 0.001734 | 0.001786 | 0.001724 |
| minus_eq_mm_test.test | RUN | 0.406402 | 1.231314 | 1.275216 | 0.001722 | 0.001755 | 0.001850 |
| minus_mm_test.test | RUN | 0.457139 | 1.570217 | 1.513492 | 0.001749 | 0.001736 | 0.001856 |
| mul_eq_mm_test.test | RUN | 0.392525 | 1.157580 | 1.123595 | 0.001826 | 0.001674 | 0.001741 |
| mul_eq_ms_test.test | RUN | 0.391141 | 0.834429 | 1.156360 | 0.001725 | 0.001664 | 0.001837 |
| mul_mm_test.test | RUN | 0.525496 | 2.597041 | 1.890845 | 0.001838 | 0.001775 | 0.001788 |
| mul_ms_test.test | RUN | 0.438436 | 1.363930 | 1.457992 | 0.001555 | 0.001804 | 0.001455 |
| mul_sm_test.test | RUN | 0.437621 | 1.358893 | 1.450286 | 0.001732 | 0.001674 | 0.001762 |
| mul_mv_test.test | RUN | 0.493024 | 1.445840 | 1.481780 | 0.001663 | 0.001704 | 0.001937 |
| inverse_m_test.test | RUN | 0.615799 | 2.679544 | 2.089153 | 0.001925 | 0.001736 | 0.001848 |
| plus_eq_mm_test.test | RUN | 0.404415 | 1.244033 | 1.283654 | 0.001771 | 0.001717 | 0.001777 |
| plus_mm_test.test | RUN | 0.462440 | 1.561096 | 1.516394 | 0.001798 | 0.001786 | 0.001602 |
| scalar_cast_m_test.test | RUN | 0.366313 | 0.699228 | 0.910616 | 0.001916 | 0.001817 | 0.001789 |
| mat_index_test.test | RUN | 0.295341 | 0.666640 | 0.813344 | 0.001842 | 0.001667 | 0.001734 |
| cmp_qq_test.test | RUN | 0.311155 | 0.607136 | 0.768588 | 0.001777 | 0.001705 | 0.001714 |
| conjugate_test.test | RUN | 0.320293 | 0.634885 | 0.814763 | 0.001672 | 0.001316 | 0.001770 |
| normalize_q_test.test | RUN | 0.318951 | 0.703225 | 0.832737 | 0.001890 | 0.001640 | 0.001797 |
| div_eq_qs_test.test | RUN | 0.309819 | 0.606298 | 0.761823 | 0.001615 | 0.001706 | 0.001791 |
| div_qs_test.test | RUN | 0.315071 | 0.638335 | 0.789221 | 0.001666 | 0.001611 | 0.001797 |
| dot_qq_test.test | RUN | 0.314453 | 0.614571 | 0.816244 | 0.001741 | 0.001797 | 0.001732 |
| eq_qq_test.test | RUN | 0.314110 | 0.656982 | 0.821960 | 0.001838 | 0.001685 | 0.001738 |
| inverse_q_test.test | RUN | 0.376922 | 0.841184 | 1.009723 | 0.001680 | 0.001808 | 0.001806 |
| mag_q_test.test | RUN | 0.309727 | 0.640121 | 0.790971 | 0.001737 | 0.001762 | 0.001748 |
| mag_sqr_q_test.test | RUN | 0.310197 | 0.634092 | 0.787031 | 0.001737 | 0.001770 | 0.001710 |
| minus_q_test.test | RUN | 0.313811 | 0.635281 | 0.795569 | 0.001726 | 0.001683 | 0.001649 |
| minus_eq_qq_test.test | RUN | 0.311503 | 0.626044 | 0.809983 | 0.001642 | 0.001589 | 0.001707 |
| minus_qq_test.test | RUN | 0.316134 | 0.653268 | 0.813470 | 0.001714 | 0.001717 | 0.001733 |
| mul_eq_qs_test.test | RUN | 0.305022 | 0.603106 | 0.792098 | 0.001844 | 0.001801 | 0.001566 |
| mul_qs_test.test | RUN | 0.308066 | 0.635114 | 0.813254 | 0.001818 | 0.001782 | 0.001719 |
| mul_qv_test.test | RUN | 0.381624 | 0.867894 | 1.006697 | 0.001716 | 0.001831 | 0.001770 |
| mul_qq_test.test | RUN | 0.379578 | 0.886145 | 1.012220 | 0.001726 | 0.001821 | 0.001748 |
| mul_eq_qq_test.test | RUN | 0.374140 | 0.894511 | 0.992374 | 0.001837 | 0.001660 | 0.001586 |
| plus_eq_qq_test.test | RUN | 0.306307 | 0.640104 | 0.786189 | 0.001608 | 0.001648 | 0.001573 |
| plus_qq_test.test | RUN | 0.315024 | 0.655611 | 0.796289 | 0.001739 | 0.001855 | 0.001954 |
| scalar_cast_q_test.test | RUN | 0.311211 | 0.611050 | 0.783948 | 0.001772 | 0.001680 | 0.001783 |
| slerp_test.test | RUN | 0.335937 | 1.242967 | 0.965191 | 0.023338 | 0.018306 | 0.018879 |
| convert_to_test.test | RUN | 0.457438 | 1.150605 | 1.494855 | 0.001738 | 0.001576 | 0.001758 |
| rot_mat_test.test | RUN | 0.822703 | 5.287925 | 4.863498 | 0.087286 | 0.045159 | 0.048250 |
| rot_quat_test.test | RUN | 0.385161 | 1.068374 | 1.050995 | 0.002007 | 0.001770 | 0.001848 |
| rotx_mat_test.test | RUN | 0.449790 | 1.382978 | 1.383285 | 0.001855 | 0.001871 | 0.001748 |
| rotx_quat_test.test | RUN | 0.375850 | 0.896858 | 0.986126 | 0.001507 | 0.001824 | 0.001726 |
| roty_mat_test.test | RUN | 0.439477 | 1.382561 | 1.350909 | 0.001852 | 0.001776 | 0.001846 |
| roty_quat_test.test | RUN | 0.375909 | 0.881322 | 0.995843 | 0.001636 | 0.001808 | 0.001766 |
| rotz_mat_test.test | RUN | 0.460454 | 1.571381 | 1.479356 | 0.001687 | 0.001906 | 0.001843 |
| rotz_quat_test.test | RUN | 0.377555 | 0.877535 | 0.944799 | 0.001652 | 0.001660 | 0.001802 |
| zero_vec_test.test | RUN | 0.337546 | 0.764552 | 0.815429 | 0.001838 | 0.001623 | 0.001787 |
| zero_mat_test.test | RUN | 0.465640 | 1.267649 | 1.135837 | 0.001672 | 0.001559 | 0.001769 |
| zero_quat_test.test | RUN | 0.313946 | 0.624736 | 0.778654 | 0.001728 | 0.001728 | 0.001693 |
| identity_mat_test.test | RUN | 0.392709 | 0.963138 | 1.020685 | 0.001549 | 0.001686 | 0.001720 |
| identity_quat_test.test | RUN | 0.316320 | 0.618049 | 0.762070 | 0.001767 | 0.001764 | 0.001723 |
| col_test.test | RUN | 0.596167 | 1.960241 | 2.239002 | 0.001476 | 0.001860 | 0.001426 |
| col_mat_test.test | RUN | 0.392446 | 0.875605 | 1.050323 | 0.001734 | 0.001849 | 0.001755 |
| neg_col_test.test | RUN | 0.454854 | 1.237492 | 1.373528 | 0.001716 | 0.001724 | 0.001750 |
| neg_row_test.test | RUN | 0.467980 | 1.242935 | 1.370842 | 0.001728 | 0.001814 | 0.001792 |
| del_col_test.test | RUN | 0.462743 | 1.364344 | 1.413409 | 0.001679 | 0.001834 | 0.001739 |
| del_row_test.test | RUN | 0.463650 | 1.352878 | 1.418103 | 0.001842 | 0.001804 | 0.001776 |
| del_row_col_test.test | RUN | 0.627257 | 2.556611 | 2.681430 | 0.001809 | 0.001805 | 0.001544 |
| swap_cols_test.test | RUN | 0.892955 | 3.485253 | 3.311815 | 0.001651 | 0.001824 | 0.001742 |
| swap_rows_test.test | RUN | 0.985226 | 3.632864 | 3.291337 | 0.001802 | 0.001709 | 0.001565 |
| diag_test.test | RUN | 0.404111 | 0.914464 | 1.191703 | 0.001500 | 0.001823 | 0.001797 |
| diag_mat_test.test | RUN | 0.416151 | 0.896346 | 1.056552 | 0.001790 | 0.001720 | 0.001578 |
| row_test.test | RUN | 0.598931 | 1.945090 | 2.256404 | 0.001702 | 0.001770 | 0.001761 |
| row_mat_test.test | RUN | 0.387062 | 0.845042 | 1.052878 | 0.001702 | 0.001682 | 0.001786 |
| transpose_test.test | RUN | 0.443796 | 1.334357 | 1.450658 | 0.001492 | 0.001496 | 0.001745 |
| translation_test.test | RUN | 0.371957 | 0.834979 | 1.079077 | 0.001553 | 0.001694 | 0.001761 |
| translation_mat_test.test | RUN | 0.413610 | 0.834013 | 1.172542 | 0.001654 | 0.001634 | 0.001695 |
| swizzle_const_fail.test | COMPILE_FAIL | 0.135950 | 0.246836 | 0.297512 | — | — | — |
| swizzle2_test2.test | RUN | 0.165835 | 0.361499 | 0.440711 | 0.001697 | 0.001679 | 0.001650 |
| swizzle2_test3.test | RUN | 0.169504 | 0.363849 | 0.422264 | 0.001704 | 0.001601 | 0.001593 |
| swizzle2_test4.test | RUN | 0.168145 | 0.360185 | 0.429891 | 0.001543 | 0.001811 | 0.001882 |
| swizzle2_test.test | RUN | 0.526850 | 0.993239 | 1.362649 | 0.001758 | 0.001560 | 0.001753 |
| swizzle3_test2.test | RUN | 0.190462 | 0.386598 | 0.487876 | 0.001714 | 0.001722 | 0.001844 |
| swizzle3_test3.test | RUN | 0.184376 | 0.394358 | 0.484246 | 0.001677 | 0.001670 | 0.001477 |
| swizzle3_test4.test | RUN | 0.187156 | 0.389072 | 0.481962 | 0.001600 | 0.001720 | 0.001765 |
| swizzle3_test.test | RUN | 0.515129 | 1.022078 | 1.366615 | 0.001405 | 0.001689 | 0.001752 |
| swizzle4_test2.test | RUN | 0.304069 | 0.566914 | 0.791416 | 0.001744 | 0.001688 | 0.001705 |
| swizzle4_test3.test | RUN | 0.305345 | 0.561840 | 0.767010 | 0.001404 | 0.001749 | 0.001643 |
| swizzle4_test4.test | RUN | 0.307081 | 0.558691 | 0.762485 | 0.001667 | 0.001565 | 0.001765 |
| swizzle4_test.test | RUN | 0.538400 | 1.121126 | 1.396979 | 0.001846 | 0.001782 | 0.001700 |
| deduce_scalar_test.test | COMPILE | 0.026537 | 0.052108 | 0.074407 | — | — | — |
| deduce_scalar_mq_test.test | RUN | 0.201107 | 0.419585 | 0.499372 | 0.001644 | 0.001916 | 0.001661 |
| deduce_scalar_mv_test.test | RUN | 0.244851 | 0.489164 | 0.616320 | 0.001494 | 0.001789 | 0.001688 |
| deduce_scalar_fail.test | COMPILE_FAIL | 0.022753 | 0.033529 | 0.056581 | — | — | — |
| deduce_mat_test.test | COMPILE | 0.023878 | 0.046955 | 0.064743 | — | — | — |
| deduce_quat_test.test | COMPILE | 0.023952 | 0.045765 | 0.064154 | — | — | — |
| deduce_vec_test.test | COMPILE | 0.024487 | 0.044290 | 0.066187 | — | — | — |
| interop_test.test | RUN | 0.245660 | 0.623489 | 0.901527 | 0.000831 | 0.000820 | 0.001698 |
| transform_test.test | RUN | 0.456948 | 0.915610 | 1.221755 | 0.001349 | 0.001343 | 0.001287 |
| projection_test.test | RUN | 0.330025 | 0.665930 | 0.696150 | 0.001322 | 0.001352 | 0.001289 |
| scalar_traits_test.test | COMPILE | 0.025034 | 0.049709 | 0.068666 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.837× | 0.836×, 0.838× | 1.006× |
| clang | 0.910×† | 0.870×, 0.950× | 1.031× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| access_m_test.test | 10 | 0.001060s | 0.001028s | 0.001095s | byte equality observed |
| access_q_test.test | 10 | 0.001065s | 0.001063s | 0.001059s | byte equality observed |
| access_v_test.test | 10 | 0.001053s | 0.001062s | 0.001053s | byte equality observed |
| assign_test.test | 10 | 0.001066s | 0.001053s | 0.001045s | byte equality observed |
| cmp_mm_test.test | 10 | 0.001076s | 0.001053s | 0.001059s | byte equality observed |
| cmp_qq_test.test | 10 | 0.001068s | 0.001095s | 0.001063s | byte equality observed |
| cmp_vv_test.test | 10 | 0.001060s | 0.001067s | 0.001036s | byte equality observed |
| col_mat_test.test | 10 | 0.001042s | 0.001050s | 0.001040s | byte equality observed |
| col_test.test | 10 | 0.001047s | 0.001107s | 0.001073s | byte equality observed |
| conjugate_test.test | 10 | 0.001041s | 0.001064s | 0.001052s | byte equality observed |
| convert_to_test.test | 10 | 0.001068s | 0.001080s | 0.001029s | byte equality observed |
| cross_test.test | 10 | 0.001056s | 0.001064s | 0.001062s | byte equality observed |
| deduce_scalar_mq_test.test | 10 | 0.001076s | 0.001056s | 0.001045s | byte equality observed |
| deduce_scalar_mv_test.test | 10 | 0.001087s | 0.001061s | 0.001061s | byte equality observed |
| del_col_test.test | 10 | 0.001054s | 0.001060s | 0.001042s | byte equality observed |
| del_row_col_test.test | 10 | 0.001022s | 0.001054s | 0.001049s | byte equality observed |
| del_row_test.test | 10 | 0.001057s | 0.001084s | 0.001044s | byte equality observed |
| determinant_test.test | 10 | 0.001067s | 0.001040s | 0.001039s | byte equality observed |
| diag_mat_test.test | 10 | 0.001045s | 0.001071s | 0.001054s | byte equality observed |
| diag_test.test | 10 | 0.001047s | 0.001040s | 0.001041s | byte equality observed |
| div_eq_ms_test.test | 10 | 0.001057s | 0.001057s | 0.001082s | byte equality observed |
| div_eq_qs_test.test | 10 | 0.001055s | 0.001032s | 0.001057s | byte equality observed |
| div_eq_vs_test.test | 10 | 0.001056s | 0.001024s | 0.001045s | byte equality observed |
| div_ms_test.test | 10 | 0.001084s | 0.001075s | 0.001069s | byte equality observed |
| div_qs_test.test | 10 | 0.001052s | 0.001061s | 0.001034s | byte equality observed |
| div_vs_test.test | 10 | 0.001055s | 0.001039s | 0.001036s | byte equality observed |
| dot_qq_test.test | 10 | 0.001032s | 0.001057s | 0.001054s | byte equality observed |
| dot_vv_test.test | 10 | 0.001044s | 0.001059s | 0.001040s | byte equality observed |
| eq_mm_test.test | 10 | 0.001063s | 0.001048s | 0.001074s | byte equality observed |
| eq_qq_test.test | 10 | 0.001063s | 0.001060s | 0.001050s | byte equality observed |
| eq_vv_test.test | 10 | 0.001022s | 0.001062s | 0.001011s | byte equality observed |
| identity_mat_test.test | 10 | 0.001052s | 0.001035s | 0.001082s | byte equality observed |
| identity_quat_test.test | 10 | 0.001027s | 0.001061s | 0.001031s | byte equality observed |
| interop_test.test | 10 | 0.000487s | 0.000474s | 0.001041s | byte equality observed |
| inverse_m_test.test | 10 | 0.001166s | 0.001111s | 0.001118s | byte equality observed |
| inverse_q_test.test | 10 | 0.001055s | 0.001059s | 0.001053s | byte equality observed |
| mag_q_test.test | 10 | 0.001052s | 0.001068s | 0.001042s | byte equality observed |
| mag_sqr_q_test.test | 10 | 0.001039s | 0.001016s | 0.001089s | byte equality observed |
| mag_sqr_v_test.test | 10 | 0.001039s | 0.001093s | 0.001042s | byte equality observed |
| mag_v_test.test | 10 | 0.001042s | 0.001081s | 0.001076s | byte equality observed |
| mat_index_test.test | 10 | 0.001049s | 0.001078s | 0.001043s | byte equality observed |
| mat_traits_array_test.test | 10 | 0.001036s | 0.001088s | 0.001050s | byte equality observed |
| mat_traits_std_array_test.test | 10 | 0.001022s | 0.001054s | 0.001051s | byte equality observed |
| math_test.test | 10 | 0.001587s | 0.001564s | 0.001384s | byte equality observed |
| minus_eq_mm_test.test | 10 | 0.001032s | 0.001021s | 0.001093s | byte equality observed |
| minus_eq_qq_test.test | 10 | 0.001053s | 0.001039s | 0.001028s | byte equality observed |
| minus_eq_vv_test.test | 10 | 0.001051s | 0.001033s | 0.001060s | byte equality observed |
| minus_m_test.test | 10 | 0.001049s | 0.001037s | 0.001099s | byte equality observed |
| minus_mm_test.test | 10 | 0.001072s | 0.001042s | 0.001058s | byte equality observed |
| minus_q_test.test | 10 | 0.001065s | 0.001053s | 0.001028s | byte equality observed |
| minus_qq_test.test | 10 | 0.001042s | 0.001077s | 0.001105s | byte equality observed |
| minus_v_test.test | 10 | 0.001044s | 0.001070s | 0.001070s | byte equality observed |
| minus_vv_test.test | 10 | 0.001066s | 0.001070s | 0.001049s | byte equality observed |
| mul_eq_mm_test.test | 10 | 0.001043s | 0.001084s | 0.001077s | byte equality observed |
| mul_eq_ms_test.test | 10 | 0.001037s | 0.001142s | 0.001069s | byte equality observed |
| mul_eq_qq_test.test | 10 | 0.001050s | 0.001055s | 0.001086s | byte equality observed |
| mul_eq_qs_test.test | 10 | 0.001061s | 0.001023s | 0.001068s | byte equality observed |
| mul_eq_vs_test.test | 10 | 0.001021s | 0.001057s | 0.001064s | byte equality observed |
| mul_mm_test.test | 10 | 0.001052s | 0.001043s | 0.001032s | byte equality observed |
| mul_ms_test.test | 10 | 0.001038s | 0.001052s | 0.001093s | byte equality observed |
| mul_mv_test.test | 10 | 0.001073s | 0.001081s | 0.001006s | byte equality observed |
| mul_qq_test.test | 10 | 0.001079s | 0.001068s | 0.001073s | byte equality observed |
| mul_qs_test.test | 10 | 0.001052s | 0.001077s | 0.001045s | byte equality observed |
| mul_qv_test.test | 10 | 0.001056s | 0.001085s | 0.001066s | byte equality observed |
| mul_sm_test.test | 10 | 0.001056s | 0.001073s | 0.001076s | byte equality observed |
| mul_sv_test.test | 10 | 0.001026s | 0.001074s | 0.001063s | byte equality observed |
| mul_vm_test.test | 10 | 0.001059s | 0.001076s | 0.001087s | byte equality observed |
| mul_vs_test.test | 10 | 0.001047s | 0.001072s | 0.001043s | byte equality observed |
| neg_col_test.test | 10 | 0.001063s | 0.001062s | 0.001040s | byte equality observed |
| neg_row_test.test | 10 | 0.001053s | 0.001021s | 0.001043s | byte equality observed |
| normalize_q_test.test | 10 | 0.001053s | 0.001052s | 0.001065s | byte equality observed |
| normalize_v_test.test | 10 | 0.001050s | 0.001056s | 0.001069s | byte equality observed |
| plus_eq_mm_test.test | 10 | 0.001060s | 0.001101s | 0.001044s | byte equality observed |
| plus_eq_qq_test.test | 10 | 0.001046s | 0.001093s | 0.001092s | byte equality observed |
| plus_eq_vv_test.test | 10 | 0.001059s | 0.001049s | 0.001062s | byte equality observed |
| plus_mm_test.test | 10 | 0.001046s | 0.001031s | 0.001071s | byte equality observed |
| plus_qq_test.test | 10 | 0.001028s | 0.001050s | 0.001053s | byte equality observed |
| plus_vv_test.test | 10 | 0.001068s | 0.001057s | 0.001066s | byte equality observed |
| projection_test.test | 10 | 0.001041s | 0.001069s | 0.001084s | byte equality observed |
| quat_traits_array_test.test | 10 | 0.001064s | 0.001045s | 0.001066s | byte equality observed |
| quat_traits_std_array_test.test | 10 | 0.001025s | 0.001022s | 0.001035s | byte equality observed |
| rot_mat_test.test | 1 | 0.056607s | 0.030011s | 0.033615s | byte equality observed |
| rot_quat_test.test | 10 | 0.000984s | 0.000967s | 0.000996s | byte equality observed |
| rotx_mat_test.test | 10 | 0.001000s | 0.001034s | 0.001029s | byte equality observed |
| rotx_quat_test.test | 10 | 0.000992s | 0.001105s | 0.001138s | byte equality observed |
| roty_mat_test.test | 10 | 0.001097s | 0.001075s | 0.001209s | byte equality observed |
| roty_quat_test.test | 10 | 0.001090s | 0.001070s | 0.001124s | byte equality observed |
| rotz_mat_test.test | 10 | 0.001117s | 0.001079s | 0.001173s | byte equality observed |
| rotz_quat_test.test | 10 | 0.001072s | 0.001065s | 0.001165s | byte equality observed |
| row_mat_test.test | 10 | 0.001085s | 0.001035s | 0.001170s | byte equality observed |
| row_test.test | 10 | 0.001062s | 0.001084s | 0.001129s | byte equality observed |
| scalar_cast_m_test.test | 10 | 0.001065s | 0.001094s | 0.001133s | byte equality observed |
| scalar_cast_q_test.test | 10 | 0.001060s | 0.001067s | 0.001159s | byte equality observed |
| scalar_cast_v_test.test | 10 | 0.001000s | 0.001024s | 0.001107s | byte equality observed |
| slerp_test.test | 2 | 0.016408s | 0.012596s | 0.014789s | byte equality observed |
| swap_cols_test.test | 10 | 0.000959s | 0.000984s | 0.001087s | byte equality observed |
| swap_rows_test.test | 10 | 0.001025s | 0.001035s | 0.001098s | byte equality observed |
| swizzle2_test.test | 10 | 0.001041s | 0.001072s | 0.001133s | byte equality observed |
| swizzle2_test2.test | 10 | 0.001051s | 0.001034s | 0.001137s | byte equality observed |
| swizzle2_test3.test | 10 | 0.001041s | 0.001056s | 0.001147s | byte equality observed |
| swizzle2_test4.test | 10 | 0.001042s | 0.001041s | 0.001099s | byte equality observed |
| swizzle3_test.test | 10 | 0.001041s | 0.001033s | 0.001111s | byte equality observed |
| swizzle3_test2.test | 9 | 0.001047s | 0.001080s | 0.001165s | byte equality observed |
| swizzle3_test3.test | 10 | 0.001029s | 0.001038s | 0.001167s | byte equality observed |
| swizzle3_test4.test | 10 | 0.001036s | 0.001069s | 0.001205s | byte equality observed |
| swizzle4_test.test | 10 | 0.001091s | 0.001099s | 0.001180s | byte equality observed |
| swizzle4_test2.test | 10 | 0.001059s | 0.001066s | 0.001178s | byte equality observed |
| swizzle4_test3.test | 10 | 0.001108s | 0.001044s | 0.001209s | byte equality observed |
| swizzle4_test4.test | 10 | 0.001126s | 0.001090s | 0.001173s | byte equality observed |
| to_string_test.test | 10 | 0.001157s | 0.001087s | 0.001214s | byte equality observed |
| transform_test.test | 10 | 0.001117s | 0.001077s | 0.001202s | byte equality observed |
| translation_mat_test.test | 10 | 0.001139s | 0.001088s | 0.001207s | byte equality observed |
| translation_test.test | 10 | 0.001128s | 0.001054s | 0.001210s | byte equality observed |
| transpose_test.test | 10 | 0.001180s | 0.001056s | 0.001191s | byte equality observed |
| vec_index_test.test | 10 | 0.001213s | 0.001071s | 0.001209s | byte equality observed |
| vec_register_test.test | 10 | 0.001182s | 0.001076s | 0.001220s | byte equality observed |
| vec_traits_array_test.test | 10 | 0.001218s | 0.001060s | 0.001211s | byte equality observed |
| vec_traits_gnuc_test.test | 10 | 0.001209s | 0.001060s | 0.001194s | byte equality observed |
| vec_traits_std_array_test.test | 10 | 0.001218s | 0.001062s | 0.001193s | byte equality observed |
| zero_mat_test.test | 10 | 0.001216s | 0.001049s | 0.001224s | byte equality observed |
| zero_quat_test.test | 10 | 0.001196s | 0.001048s | 0.001198s | byte equality observed |
| zero_vec_test.test | 10 | 0.001203s | 0.001093s | 0.001193s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
