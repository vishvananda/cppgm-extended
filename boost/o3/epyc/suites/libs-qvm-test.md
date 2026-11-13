# libs/qvm/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| math_test.test | RUN | 0.317474 | 0.884485 | 1.221130 | 0.002515 | 0.002724 | 0.002729 |
| mat_traits_array_test.test | RUN | 0.251393 | 0.895651 | 0.957432 | 0.001916 | 0.001731 | 0.001757 |
| mat_traits_std_array_test.test | RUN | 0.256152 | 0.689169 | 0.993394 | 0.001721 | 0.002080 | 0.001670 |
| vec_traits_array_test.test | RUN | 0.218293 | 0.683237 | 0.822100 | 0.001711 | 0.001856 | 0.001624 |
| vec_traits_gnuc_test.test | RUN | 0.408111 | 0.871107 | 1.557556 | 0.001928 | 0.001723 | 0.002139 |
| vec_traits_std_array_test.test | RUN | 0.220835 | 0.530237 | 0.814209 | 0.001698 | 0.001904 | 0.001804 |
| quat_traits_array_test.test | RUN | 0.212861 | 0.718500 | 0.809942 | 0.001768 | 0.001938 | 0.001978 |
| quat_traits_std_array_test.test | RUN | 0.216399 | 0.557693 | 0.807918 | 0.001754 | 0.001753 | 0.002044 |
| access_m_test.test | RUN | 0.346606 | 1.336607 | 1.761343 | 0.001806 | 0.002092 | 0.001783 |
| access_m_fail1.test | COMPILE_FAIL | 0.034869 | 0.059221 | 0.129737 | — | — | — |
| access_m_fail2.test | COMPILE_FAIL | 0.034385 | 0.062867 | 0.132439 | — | — | — |
| access_q_test.test | RUN | 0.282798 | 0.649612 | 1.037173 | 0.001751 | 0.002048 | 0.001939 |
| access_q_fail.test | COMPILE_FAIL | 0.026939 | 0.043471 | 0.098885 | — | — | — |
| access_v_test.test | RUN | 0.283278 | 0.710614 | 1.036977 | 0.001800 | 0.001912 | 0.001338 |
| access_v_fail1.test | COMPILE_FAIL | 0.023750 | 0.043880 | 0.083492 | — | — | — |
| access_v_fail2.test | COMPILE_FAIL | 0.023186 | 0.043774 | 0.085665 | — | — | — |
| assign_test.test | RUN | 0.435932 | 1.090206 | 1.893086 | 0.001836 | 0.002063 | 0.002100 |
| to_string_test.test | RUN | 0.506995 | 1.766197 | 10.034944 | 0.001794 | 0.002148 | 0.001962 |
| cmp_vv_test.test | RUN | 0.362981 | 0.897133 | 1.325109 | 0.001770 | 0.001949 | 0.001838 |
| cross_test.test | RUN | 0.348169 | 0.791692 | 1.297435 | 0.001738 | 0.002054 | 0.001634 |
| div_eq_vs_test.test | RUN | 0.338253 | 0.745641 | 1.221579 | 0.001732 | 0.001984 | 0.001896 |
| div_vs_test.test | RUN | 0.350567 | 0.915772 | 1.357851 | 0.001560 | 0.002044 | 0.001702 |
| dot_vv_test.test | RUN | 0.329141 | 0.802991 | 1.277960 | 0.001874 | 0.001813 | 0.001834 |
| eq_vv_test.test | RUN | 0.368201 | 0.938123 | 1.284202 | 0.001761 | 0.002019 | 0.001918 |
| mag_v_test.test | RUN | 0.324617 | 0.783868 | 1.223575 | 0.001808 | 0.001991 | 0.001964 |
| mag_sqr_v_test.test | RUN | 0.325996 | 0.738091 | 1.211820 | 0.001956 | 0.001822 | 0.001676 |
| minus_v_test.test | RUN | 0.335855 | 0.853465 | 1.227919 | 0.001756 | 0.001942 | 0.002058 |
| minus_eq_vv_test.test | RUN | 0.338305 | 0.849895 | 1.298520 | 0.001824 | 0.001917 | 0.001680 |
| minus_vv_test.test | RUN | 0.358202 | 0.920315 | 1.354606 | 0.002242 | 0.001780 | 0.001733 |
| mul_eq_vs_test.test | RUN | 0.332287 | 0.749527 | 1.173394 | 0.001792 | 0.002129 | 0.001911 |
| mul_vs_test.test | RUN | 0.350263 | 0.924907 | 1.359490 | 0.001784 | 0.002065 | 0.001724 |
| mul_sv_test.test | RUN | 0.353059 | 0.934508 | 1.386274 | 0.001664 | 0.001941 | 0.001804 |
| mul_vm_test.test | RUN | 0.505747 | 1.689893 | 2.088865 | 0.001861 | 0.001966 | 0.001732 |
| normalize_v_test.test | RUN | 0.354003 | 1.086848 | 1.396450 | 0.001911 | 0.001984 | 0.001795 |
| plus_eq_vv_test.test | RUN | 0.331295 | 0.899096 | 1.181713 | 0.001540 | 0.001981 | 0.001705 |
| plus_vv_test.test | RUN | 0.352123 | 0.918307 | 1.345048 | 0.001832 | 0.001966 | 0.001698 |
| scalar_cast_v_test.test | RUN | 0.325324 | 0.686024 | 1.199758 | 0.001836 | 0.002028 | 0.001530 |
| vec_index_test.test | RUN | 0.276991 | 0.730518 | 1.021501 | 0.001750 | 0.001956 | 0.001865 |
| vec_register_test.test | RUN | 0.186571 | 0.636794 | 0.762558 | 0.001982 | 0.001908 | 0.001757 |
| cmp_mm_test.test | RUN | 0.535424 | 1.997166 | 2.610023 | 0.002130 | 0.002209 | 0.002023 |
| determinant_test.test | RUN | 0.387737 | 1.060454 | 1.489753 | 0.001934 | 0.002043 | 0.002008 |
| div_eq_ms_test.test | RUN | 0.400740 | 0.965673 | 1.652250 | 0.001445 | 0.001782 | 0.002221 |
| div_ms_test.test | RUN | 0.465107 | 1.541401 | 2.007155 | 0.002014 | 0.001709 | 0.001549 |
| eq_mm_test.test | RUN | 0.518562 | 1.853080 | 2.373765 | 0.001900 | 0.002094 | 0.001933 |
| minus_m_test.test | RUN | 0.457134 | 1.453878 | 1.854858 | 0.001788 | 0.002064 | 0.001824 |
| minus_eq_mm_test.test | RUN | 0.429043 | 1.386800 | 1.725033 | 0.001890 | 0.002040 | 0.001858 |
| minus_mm_test.test | RUN | 0.485741 | 1.757469 | 2.067618 | 0.002380 | 0.002107 | 0.001994 |
| mul_eq_mm_test.test | RUN | 0.412690 | 1.312458 | 1.554311 | 0.001999 | 0.001838 | 0.002088 |
| mul_eq_ms_test.test | RUN | 0.406454 | 0.983766 | 1.676149 | 0.002074 | 0.001741 | 0.002060 |
| mul_mm_test.test | RUN | 0.555943 | 2.899500 | 2.631548 | 0.002020 | 0.002127 | 0.002132 |
| mul_ms_test.test | RUN | 0.491474 | 1.552556 | 2.035053 | 0.001974 | 0.002050 | 0.002052 |
| mul_sm_test.test | RUN | 0.481387 | 1.519470 | 2.095324 | 0.001808 | 0.001748 | 0.002100 |
| mul_mv_test.test | RUN | 0.524529 | 1.645069 | 1.986272 | 0.002066 | 0.002073 | 0.002100 |
| inverse_m_test.test | RUN | 0.632889 | 2.958189 | 2.776614 | 0.001994 | 0.002032 | 0.002036 |
| plus_eq_mm_test.test | RUN | 0.430483 | 1.392761 | 1.796484 | 0.001964 | 0.001960 | 0.002010 |
| plus_mm_test.test | RUN | 0.491974 | 1.766159 | 2.084100 | 0.001973 | 0.002158 | 0.001882 |
| scalar_cast_m_test.test | RUN | 0.370584 | 0.810007 | 1.331349 | 0.001832 | 0.001920 | 0.002100 |
| mat_index_test.test | RUN | 0.294044 | 0.767784 | 1.054206 | 0.001550 | 0.001836 | 0.002014 |
| cmp_qq_test.test | RUN | 0.329805 | 0.699793 | 1.123952 | 0.001769 | 0.001766 | 0.001703 |
| conjugate_test.test | RUN | 0.326355 | 0.724513 | 1.173001 | 0.001806 | 0.002114 | 0.001931 |
| normalize_q_test.test | RUN | 0.339519 | 0.802912 | 1.250011 | 0.002004 | 0.001830 | 0.001936 |
| div_eq_qs_test.test | RUN | 0.328847 | 0.697634 | 1.149662 | 0.001868 | 0.002040 | 0.002052 |
| div_qs_test.test | RUN | 0.324701 | 0.754602 | 1.162817 | 0.002068 | 0.001907 | 0.001826 |
| dot_qq_test.test | RUN | 0.332130 | 0.707998 | 1.180538 | 0.001882 | 0.002112 | 0.002068 |
| eq_qq_test.test | RUN | 0.332423 | 0.757540 | 1.184049 | 0.001964 | 0.002096 | 0.002104 |
| inverse_q_test.test | RUN | 0.393627 | 0.982570 | 1.444855 | 0.001972 | 0.002139 | 0.002030 |
| mag_q_test.test | RUN | 0.327119 | 0.751968 | 1.143060 | 0.001728 | 0.002357 | 0.002148 |
| mag_sqr_q_test.test | RUN | 0.320630 | 0.729986 | 1.204967 | 0.001908 | 0.002155 | 0.001968 |
| minus_q_test.test | RUN | 0.322830 | 0.719037 | 1.105275 | 0.001941 | 0.001750 | 0.002142 |
| minus_eq_qq_test.test | RUN | 0.324392 | 0.755576 | 1.155552 | 0.001932 | 0.001865 | 0.002122 |
| minus_qq_test.test | RUN | 0.331940 | 0.755319 | 1.145412 | 0.001880 | 0.002036 | 0.002069 |
| mul_eq_qs_test.test | RUN | 0.325387 | 0.686868 | 1.130815 | 0.001652 | 0.002080 | 0.002067 |
| mul_qs_test.test | RUN | 0.329154 | 0.730029 | 1.168291 | 0.002344 | 0.002084 | 0.002312 |
| mul_qv_test.test | RUN | 0.406124 | 1.015539 | 1.511799 | 0.001950 | 0.002076 | 0.002006 |
| mul_qq_test.test | RUN | 0.395165 | 1.017396 | 1.455178 | 0.001756 | 0.001993 | 0.002162 |
| mul_eq_qq_test.test | RUN | 0.394326 | 1.034354 | 1.412092 | 0.002002 | 0.002145 | 0.001885 |
| plus_eq_qq_test.test | RUN | 0.324677 | 0.725267 | 1.136189 | 0.002078 | 0.002124 | 0.002152 |
| plus_qq_test.test | RUN | 0.335307 | 0.731529 | 1.135275 | 0.002014 | 0.002100 | 0.002075 |
| scalar_cast_q_test.test | RUN | 0.318666 | 0.696812 | 1.105486 | 0.002114 | 0.001922 | 0.001965 |
| slerp_test.test | RUN | 0.354031 | 1.405856 | 1.383733 | 0.028613 | 0.020545 | 0.021273 |
| convert_to_test.test | RUN | 0.478173 | 1.296439 | 2.076115 | 0.002136 | 0.001861 | 0.002146 |
| rot_mat_test.test | RUN | 0.749152 | 5.828025 | 6.004566 | 0.125385 | 0.053590 | 0.050717 |
| rot_quat_test.test | RUN | 0.393916 | 1.156426 | 1.516200 | 0.002006 | 0.002246 | 0.002166 |
| rotx_mat_test.test | RUN | 0.444005 | 1.569644 | 1.891094 | 0.001798 | 0.002033 | 0.002011 |
| rotx_quat_test.test | RUN | 0.374998 | 1.013445 | 1.429302 | 0.001907 | 0.002048 | 0.002123 |
| roty_mat_test.test | RUN | 0.444876 | 1.561304 | 1.954379 | 0.002018 | 0.002072 | 0.002048 |
| roty_quat_test.test | RUN | 0.389288 | 1.015548 | 1.245062 | 0.001773 | 0.002034 | 0.002144 |
| rotz_mat_test.test | RUN | 0.472404 | 1.749139 | 1.982079 | 0.001964 | 0.001763 | 0.002147 |
| rotz_quat_test.test | RUN | 0.384853 | 1.033469 | 1.295104 | 0.002108 | 0.001943 | 0.002056 |
| zero_vec_test.test | RUN | 0.334609 | 0.868671 | 1.155880 | 0.002012 | 0.001914 | 0.001712 |
| zero_mat_test.test | RUN | 0.424589 | 1.415059 | 1.731383 | 0.001916 | 0.001974 | 0.001790 |
| zero_quat_test.test | RUN | 0.324931 | 0.730424 | 1.087153 | 0.001782 | 0.002094 | 0.002123 |
| identity_mat_test.test | RUN | 0.386435 | 1.072555 | 1.524430 | 0.001915 | 0.002271 | 0.002106 |
| identity_quat_test.test | RUN | 0.322512 | 0.698833 | 1.109640 | 0.001746 | 0.002130 | 0.001819 |
| col_test.test | RUN | 0.628330 | 2.204333 | 3.142626 | 0.001938 | 0.001925 | 0.001885 |
| col_mat_test.test | RUN | 0.401576 | 1.033414 | 1.568748 | 0.002030 | 0.002146 | 0.002098 |
| neg_col_test.test | RUN | 0.480112 | 1.406453 | 2.023311 | 0.001506 | 0.002146 | 0.001817 |
| neg_row_test.test | RUN | 0.494244 | 1.454195 | 1.987902 | 0.001971 | 0.002092 | 0.002196 |
| del_col_test.test | RUN | 0.500219 | 1.592618 | 2.004430 | 0.002022 | 0.002119 | 0.002048 |
| del_row_test.test | RUN | 0.497331 | 1.563437 | 1.991277 | 0.001967 | 0.001892 | 0.002055 |
| del_row_col_test.test | RUN | 0.697050 | 2.990432 | 3.885663 | 0.002052 | 0.002120 | 0.002066 |
| swap_cols_test.test | RUN | 0.939466 | 3.971695 | 4.585584 | 0.002026 | 0.002124 | 0.001836 |
| swap_rows_test.test | RUN | 1.023600 | 4.169474 | 4.498914 | 0.002095 | 0.001890 | 0.002040 |
| diag_test.test | RUN | 0.410393 | 1.054690 | 1.658805 | 0.001856 | 0.002082 | 0.001916 |
| diag_mat_test.test | RUN | 0.436147 | 1.028978 | 1.654922 | 0.001996 | 0.002180 | 0.002124 |
| row_test.test | RUN | 0.630302 | 2.214227 | 3.206341 | 0.001904 | 0.002056 | 0.002081 |
| row_mat_test.test | RUN | 0.406719 | 1.009486 | 1.481224 | 0.001856 | 0.002046 | 0.002026 |
| transpose_test.test | RUN | 0.488136 | 1.517533 | 2.026642 | 0.002041 | 0.002170 | 0.001769 |
| translation_test.test | RUN | 0.387751 | 0.963474 | 1.564693 | 0.001756 | 0.002124 | 0.001764 |
| translation_mat_test.test | RUN | 0.446568 | 0.982122 | 1.628927 | 0.001912 | 0.002130 | 0.001822 |
| swizzle_const_fail.test | COMPILE_FAIL | 0.150968 | 0.301814 | 0.499841 | — | — | — |
| swizzle2_test2.test | RUN | 0.176617 | 0.422844 | 0.629041 | 0.001814 | 0.001801 | 0.001522 |
| swizzle2_test3.test | RUN | 0.177393 | 0.409630 | 0.629204 | 0.001705 | 0.002088 | 0.001688 |
| swizzle2_test4.test | RUN | 0.174262 | 0.404726 | 0.620240 | 0.001632 | 0.002097 | 0.001792 |
| swizzle2_test.test | RUN | 0.568505 | 1.201103 | 2.169889 | 0.001863 | 0.002142 | 0.001539 |
| swizzle3_test2.test | RUN | 0.199059 | 0.442278 | 0.717267 | 0.001974 | 0.002115 | 0.001810 |
| swizzle3_test3.test | RUN | 0.199519 | 0.448198 | 0.712433 | 0.001816 | 0.002048 | 0.001658 |
| swizzle3_test4.test | RUN | 0.194098 | 0.438916 | 0.745615 | 0.001836 | 0.001909 | 0.002034 |
| swizzle3_test.test | RUN | 0.554607 | 1.198147 | 2.177612 | 0.001734 | 0.001998 | 0.002070 |
| swizzle4_test2.test | RUN | 0.324456 | 0.647071 | 1.284450 | 0.001909 | 0.002162 | 0.002028 |
| swizzle4_test3.test | RUN | 0.321547 | 0.684596 | 1.259187 | 0.001749 | 0.002065 | 0.001900 |
| swizzle4_test4.test | RUN | 0.353135 | 0.658643 | 1.220027 | 0.001892 | 0.001780 | 0.001771 |
| swizzle4_test.test | RUN | 0.555529 | 1.340411 | 2.213531 | 0.001742 | 0.002116 | 0.002052 |
| deduce_scalar_test.test | COMPILE | 0.028330 | 0.060454 | 0.107491 | — | — | — |
| deduce_scalar_mq_test.test | RUN | 0.211543 | 0.482034 | 0.793729 | 0.001965 | 0.001708 | 0.001658 |
| deduce_scalar_mv_test.test | RUN | 0.247899 | 0.580705 | 0.940969 | 0.001884 | 0.001858 | 0.002058 |
| deduce_scalar_fail.test | COMPILE_FAIL | 0.023870 | 0.041700 | 0.072489 | — | — | — |
| deduce_mat_test.test | COMPILE | 0.025034 | 0.056251 | 0.084799 | — | — | — |
| deduce_quat_test.test | COMPILE | 0.024919 | 0.050215 | 0.081680 | — | — | — |
| deduce_vec_test.test | COMPILE | 0.025521 | 0.052650 | 0.081583 | — | — | — |
| interop_test.test | RUN | 0.235717 | 0.680281 | 1.153314 | 0.000672 | 0.001099 | 0.001415 |
| transform_test.test | RUN | 0.501904 | 1.069224 | 1.861482 | 0.001554 | 0.001404 | 0.001470 |
| projection_test.test | RUN | 0.335638 | 0.789406 | 1.150298 | 0.001421 | 0.001370 | 0.001490 |
| scalar_traits_test.test | COMPILE | 0.027888 | 0.057976 | 0.093520 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.677× | 0.683×, 0.671× | 1.003× |
| clang | 0.748×† | 0.769×, 0.727× | 1.003× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824563).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| access_m_test.test | 10 | 0.001264s | 0.001254s | 0.001209s | byte equality observed |
| access_q_test.test | 10 | 0.001259s | 0.001216s | 0.001203s | byte equality observed |
| access_v_test.test | 9 | 0.001279s | 0.001260s | 0.001217s | byte equality observed |
| assign_test.test | 10 | 0.001244s | 0.001206s | 0.001248s | byte equality observed |
| cmp_mm_test.test | 10 | 0.001277s | 0.001252s | 0.001218s | byte equality observed |
| cmp_qq_test.test | 10 | 0.001253s | 0.001229s | 0.001221s | byte equality observed |
| cmp_vv_test.test | 10 | 0.001269s | 0.001207s | 0.001199s | byte equality observed |
| col_mat_test.test | 10 | 0.001259s | 0.001272s | 0.001221s | byte equality observed |
| col_test.test | 10 | 0.001288s | 0.001207s | 0.001228s | byte equality observed |
| conjugate_test.test | 10 | 0.001303s | 0.001244s | 0.001248s | byte equality observed |
| convert_to_test.test | 10 | 0.001269s | 0.001243s | 0.001212s | byte equality observed |
| cross_test.test | 10 | 0.001230s | 0.001245s | 0.001199s | byte equality observed |
| deduce_scalar_mq_test.test | 10 | 0.001258s | 0.001257s | 0.001233s | byte equality observed |
| deduce_scalar_mv_test.test | 10 | 0.001226s | 0.001243s | 0.001264s | byte equality observed |
| del_col_test.test | 10 | 0.001235s | 0.001201s | 0.001229s | byte equality observed |
| del_row_col_test.test | 10 | 0.001248s | 0.001239s | 0.001197s | byte equality observed |
| del_row_test.test | 10 | 0.001230s | 0.001245s | 0.001262s | byte equality observed |
| determinant_test.test | 10 | 0.001278s | 0.001263s | 0.001260s | byte equality observed |
| diag_mat_test.test | 10 | 0.001250s | 0.001293s | 0.001252s | byte equality observed |
| diag_test.test | 10 | 0.001236s | 0.001208s | 0.001252s | byte equality observed |
| div_eq_ms_test.test | 10 | 0.001229s | 0.001233s | 0.001291s | byte equality observed |
| div_eq_qs_test.test | 10 | 0.001240s | 0.001306s | 0.001249s | byte equality observed |
| div_eq_vs_test.test | 10 | 0.001223s | 0.001263s | 0.001273s | byte equality observed |
| div_ms_test.test | 10 | 0.001247s | 0.001258s | 0.001207s | byte equality observed |
| div_qs_test.test | 10 | 0.001240s | 0.001219s | 0.001251s | byte equality observed |
| div_vs_test.test | 10 | 0.001238s | 0.001246s | 0.001234s | byte equality observed |
| dot_qq_test.test | 10 | 0.001229s | 0.001242s | 0.001211s | byte equality observed |
| dot_vv_test.test | 10 | 0.001278s | 0.001211s | 0.001225s | byte equality observed |
| eq_mm_test.test | 10 | 0.001273s | 0.001277s | 0.001231s | byte equality observed |
| eq_qq_test.test | 10 | 0.001242s | 0.001238s | 0.001206s | byte equality observed |
| eq_vv_test.test | 10 | 0.001264s | 0.001212s | 0.001312s | byte equality observed |
| identity_mat_test.test | 10 | 0.001259s | 0.001218s | 0.001245s | byte equality observed |
| identity_quat_test.test | 8 | 0.001232s | 0.001292s | 0.001183s | byte equality observed |
| interop_test.test | 10 | 0.000581s | 0.000578s | 0.001267s | byte equality observed |
| inverse_m_test.test | 10 | 0.001268s | 0.001316s | 0.001261s | byte equality observed |
| inverse_q_test.test | 10 | 0.001212s | 0.001257s | 0.001325s | byte equality observed |
| mag_q_test.test | 10 | 0.001191s | 0.001212s | 0.001288s | byte equality observed |
| mag_sqr_q_test.test | 10 | 0.001187s | 0.001267s | 0.001218s | byte equality observed |
| mag_sqr_v_test.test | 10 | 0.001217s | 0.001248s | 0.001200s | byte equality observed |
| mag_v_test.test | 10 | 0.001261s | 0.001243s | 0.001204s | byte equality observed |
| mat_index_test.test | 10 | 0.001296s | 0.001254s | 0.001279s | byte equality observed |
| mat_traits_array_test.test | 10 | 0.001218s | 0.001263s | 0.001285s | byte equality observed |
| mat_traits_std_array_test.test | 10 | 0.001231s | 0.001200s | 0.001247s | byte equality observed |
| math_test.test | 10 | 0.001858s | 0.001776s | 0.001624s | byte equality observed |
| minus_eq_mm_test.test | 10 | 0.001241s | 0.001220s | 0.001302s | byte equality observed |
| minus_eq_qq_test.test | 10 | 0.001232s | 0.001281s | 0.001283s | byte equality observed |
| minus_eq_vv_test.test | 10 | 0.001258s | 0.001268s | 0.001231s | byte equality observed |
| minus_m_test.test | 10 | 0.001263s | 0.001252s | 0.001227s | byte equality observed |
| minus_mm_test.test | 10 | 0.001242s | 0.001252s | 0.001242s | byte equality observed |
| minus_q_test.test | 10 | 0.001260s | 0.001260s | 0.001267s | byte equality observed |
| minus_qq_test.test | 10 | 0.001234s | 0.001315s | 0.001233s | byte equality observed |
| minus_v_test.test | 10 | 0.001214s | 0.001233s | 0.001225s | byte equality observed |
| minus_vv_test.test | 10 | 0.001283s | 0.001291s | 0.001186s | byte equality observed |
| mul_eq_mm_test.test | 10 | 0.001303s | 0.001292s | 0.001251s | byte equality observed |
| mul_eq_ms_test.test | 10 | 0.001244s | 0.001259s | 0.001264s | byte equality observed |
| mul_eq_qq_test.test | 10 | 0.001261s | 0.001237s | 0.001221s | byte equality observed |
| mul_eq_qs_test.test | 10 | 0.001250s | 0.001279s | 0.001250s | byte equality observed |
| mul_eq_vs_test.test | 10 | 0.001267s | 0.001243s | 0.001244s | byte equality observed |
| mul_mm_test.test | 10 | 0.001246s | 0.001280s | 0.001246s | byte equality observed |
| mul_ms_test.test | 10 | 0.001221s | 0.001235s | 0.001203s | byte equality observed |
| mul_mv_test.test | 10 | 0.001211s | 0.001280s | 0.001268s | byte equality observed |
| mul_qq_test.test | 10 | 0.001261s | 0.001291s | 0.001236s | byte equality observed |
| mul_qs_test.test | 10 | 0.001273s | 0.001275s | 0.001252s | byte equality observed |
| mul_qv_test.test | 10 | 0.001270s | 0.001251s | 0.001252s | byte equality observed |
| mul_sm_test.test | 10 | 0.001262s | 0.001243s | 0.001344s | byte equality observed |
| mul_sv_test.test | 10 | 0.001276s | 0.001273s | 0.001219s | byte equality observed |
| mul_vm_test.test | 10 | 0.001279s | 0.001230s | 0.001250s | byte equality observed |
| mul_vs_test.test | 10 | 0.001253s | 0.001203s | 0.001291s | byte equality observed |
| neg_col_test.test | 10 | 0.001259s | 0.001248s | 0.001243s | byte equality observed |
| neg_row_test.test | 10 | 0.001320s | 0.001223s | 0.001263s | byte equality observed |
| normalize_q_test.test | 10 | 0.001309s | 0.001255s | 0.001285s | byte equality observed |
| normalize_v_test.test | 10 | 0.001397s | 0.001269s | 0.001321s | byte equality observed |
| plus_eq_mm_test.test | 10 | 0.001430s | 0.001220s | 0.001276s | byte equality observed |
| plus_eq_qq_test.test | 10 | 0.001388s | 0.001238s | 0.001319s | byte equality observed |
| plus_eq_vv_test.test | 10 | 0.001478s | 0.001268s | 0.001359s | byte equality observed |
| plus_mm_test.test | 10 | 0.001478s | 0.001222s | 0.001448s | byte equality observed |
| plus_qq_test.test | 10 | 0.001462s | 0.001261s | 0.001416s | byte equality observed |
| plus_vv_test.test | 10 | 0.001459s | 0.001213s | 0.001522s | byte equality observed |
| projection_test.test | 10 | 0.001467s | 0.001207s | 0.001494s | byte equality observed |
| quat_traits_array_test.test | 10 | 0.001467s | 0.001249s | 0.001488s | byte equality observed |
| quat_traits_std_array_test.test | 10 | 0.001409s | 0.001252s | 0.001443s | byte equality observed |
| rot_mat_test.test | 1 | 0.115114s | 0.033499s | 0.046322s | byte equality observed |
| rot_quat_test.test | 10 | 0.001451s | 0.001143s | 0.001449s | byte equality observed |
| rotx_mat_test.test | 10 | 0.001470s | 0.001196s | 0.001480s | byte equality observed |
| rotx_quat_test.test | 10 | 0.001434s | 0.001284s | 0.001471s | byte equality observed |
| roty_mat_test.test | 10 | 0.001481s | 0.001261s | 0.001488s | byte equality observed |
| roty_quat_test.test | 10 | 0.001429s | 0.001241s | 0.001485s | byte equality observed |
| rotz_mat_test.test | 10 | 0.001448s | 0.001320s | 0.001505s | byte equality observed |
| rotz_quat_test.test | 10 | 0.001430s | 0.001245s | 0.001496s | byte equality observed |
| row_mat_test.test | 10 | 0.001431s | 0.001266s | 0.001478s | byte equality observed |
| row_test.test | 10 | 0.001406s | 0.001293s | 0.001449s | byte equality observed |
| scalar_cast_m_test.test | 10 | 0.001403s | 0.001305s | 0.001498s | byte equality observed |
| scalar_cast_q_test.test | 10 | 0.001412s | 0.001292s | 0.001430s | byte equality observed |
| scalar_cast_v_test.test | 10 | 0.001403s | 0.001401s | 0.001461s | byte equality observed |
| slerp_test.test | 1 | 0.027166s | 0.014727s | 0.018552s | byte equality observed |
| swap_cols_test.test | 10 | 0.001437s | 0.001316s | 0.001435s | byte equality observed |
| swap_rows_test.test | 10 | 0.001464s | 0.001370s | 0.001496s | byte equality observed |
| swizzle2_test.test | 10 | 0.001457s | 0.001408s | 0.001472s | byte equality observed |
| swizzle2_test2.test | 10 | 0.001471s | 0.001406s | 0.001494s | byte equality observed |
| swizzle2_test3.test | 10 | 0.001484s | 0.001471s | 0.001518s | byte equality observed |
| swizzle2_test4.test | 10 | 0.001459s | 0.001479s | 0.001496s | byte equality observed |
| swizzle3_test.test | 10 | 0.001494s | 0.001449s | 0.001511s | byte equality observed |
| swizzle3_test2.test | 10 | 0.001500s | 0.001455s | 0.001466s | byte equality observed |
| swizzle3_test3.test | 10 | 0.001498s | 0.001456s | 0.001483s | byte equality observed |
| swizzle3_test4.test | 10 | 0.001481s | 0.001459s | 0.001433s | byte equality observed |
| swizzle4_test.test | 10 | 0.001498s | 0.001479s | 0.001543s | byte equality observed |
| swizzle4_test2.test | 10 | 0.001456s | 0.001499s | 0.001532s | byte equality observed |
| swizzle4_test3.test | 9 | 0.001471s | 0.001491s | 0.001488s | byte equality observed |
| swizzle4_test4.test | 10 | 0.001509s | 0.001459s | 0.001508s | byte equality observed |
| to_string_test.test | 10 | 0.001542s | 0.001526s | 0.001534s | byte equality observed |
| transform_test.test | 10 | 0.001510s | 0.001474s | 0.001519s | byte equality observed |
| translation_mat_test.test | 10 | 0.001462s | 0.001491s | 0.001501s | byte equality observed |
| translation_test.test | 10 | 0.001535s | 0.001487s | 0.001458s | byte equality observed |
| transpose_test.test | 10 | 0.001466s | 0.001502s | 0.001470s | byte equality observed |
| vec_index_test.test | 10 | 0.001494s | 0.001483s | 0.001448s | byte equality observed |
| vec_register_test.test | 10 | 0.001516s | 0.001487s | 0.001456s | byte equality observed |
| vec_traits_array_test.test | 10 | 0.001483s | 0.001502s | 0.001510s | byte equality observed |
| vec_traits_gnuc_test.test | 10 | 0.001496s | 0.001508s | 0.001495s | byte equality observed |
| vec_traits_std_array_test.test | 10 | 0.001473s | 0.001490s | 0.001484s | byte equality observed |
| zero_mat_test.test | 8 | 0.001481s | 0.001489s | 0.001490s | byte equality observed |
| zero_quat_test.test | 10 | 0.001478s | 0.001493s | 0.001482s | byte equality observed |
| zero_vec_test.test | 10 | 0.001477s | 0.001464s | 0.001482s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824563).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
