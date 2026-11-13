# libs/qvm/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| math_test.test | RUN | 0.252826 | 0.658462 | 0.778838 | 0.002234 | 0.002158 | 0.002248 |
| mat_traits_array_test.test | RUN | 0.194001 | 0.641166 | 0.622536 | 0.001861 | 0.001603 | 0.001608 |
| mat_traits_std_array_test.test | RUN | 0.200926 | 0.504979 | 0.634741 | 0.001566 | 0.001733 | 0.001420 |
| vec_traits_array_test.test | RUN | 0.180730 | 0.521579 | 0.537782 | 0.001574 | 0.001786 | 0.001272 |
| vec_traits_gnuc_test.test | RUN | 0.312456 | 0.638641 | 1.050349 | 0.001447 | 0.001552 | 0.001467 |
| vec_traits_std_array_test.test | RUN | 0.168832 | 0.409972 | 0.528814 | 0.001626 | 0.001649 | 0.001584 |
| quat_traits_array_test.test | RUN | 0.176659 | 0.533623 | 0.524432 | 0.001699 | 0.001664 | 0.001347 |
| quat_traits_std_array_test.test | RUN | 0.175970 | 0.405814 | 0.535230 | 0.001544 | 0.001688 | 0.001812 |
| access_m_test.test | RUN | 0.269891 | 0.986510 | 1.180839 | 0.001636 | 0.001929 | 0.001427 |
| access_m_fail1.test | COMPILE_FAIL | 0.027222 | 0.043577 | 0.086446 | — | — | — |
| access_m_fail2.test | COMPILE_FAIL | 0.026329 | 0.043280 | 0.080620 | — | — | — |
| access_q_test.test | RUN | 0.232142 | 0.497864 | 0.718148 | 0.001869 | 0.002009 | 0.001688 |
| access_q_fail.test | COMPILE_FAIL | 0.020483 | 0.032796 | 0.057530 | — | — | — |
| access_v_test.test | RUN | 0.232032 | 0.529822 | 0.694399 | 0.001886 | 0.001935 | 0.001846 |
| access_v_fail1.test | COMPILE_FAIL | 0.017182 | 0.032975 | 0.061763 | — | — | — |
| access_v_fail2.test | COMPILE_FAIL | 0.017743 | 0.033509 | 0.061527 | — | — | — |
| assign_test.test | RUN | 0.334729 | 0.807961 | 1.258936 | 0.001822 | 0.001906 | 0.001952 |
| to_string_test.test | RUN | 0.392106 | 1.320040 | 6.944905 | 0.001618 | 0.001820 | 0.001876 |
| cmp_vv_test.test | RUN | 0.276569 | 0.662604 | 0.835239 | 0.001728 | 0.001949 | 0.001805 |
| cross_test.test | RUN | 0.284851 | 0.606906 | 0.869341 | 0.001611 | 0.001935 | 0.001688 |
| div_eq_vs_test.test | RUN | 0.265047 | 0.551872 | 0.816430 | 0.001728 | 0.001790 | 0.001770 |
| div_vs_test.test | RUN | 0.273406 | 0.703983 | 0.863017 | 0.001375 | 0.001829 | 0.001783 |
| dot_vv_test.test | RUN | 0.262426 | 0.617226 | 0.776160 | 0.001417 | 0.001879 | 0.001912 |
| eq_vv_test.test | RUN | 0.288066 | 0.697127 | 0.894892 | 0.001806 | 0.001682 | 0.001577 |
| mag_v_test.test | RUN | 0.264649 | 0.593065 | 0.742683 | 0.001794 | 0.001932 | 0.001279 |
| mag_sqr_v_test.test | RUN | 0.245243 | 0.574832 | 0.786933 | 0.001571 | 0.001666 | 0.001854 |
| minus_v_test.test | RUN | 0.252239 | 0.680081 | 0.800571 | 0.001540 | 0.001872 | 0.001835 |
| minus_eq_vv_test.test | RUN | 0.274187 | 0.653486 | 0.829210 | 0.001808 | 0.001837 | 0.001552 |
| minus_vv_test.test | RUN | 0.261713 | 0.717286 | 0.881027 | 0.001692 | 0.001651 | 0.001893 |
| mul_eq_vs_test.test | RUN | 0.255119 | 0.563956 | 0.777759 | 0.001577 | 0.001790 | 0.001637 |
| mul_vs_test.test | RUN | 0.274877 | 0.678002 | 0.878518 | 0.001687 | 0.001906 | 0.001870 |
| mul_sv_test.test | RUN | 0.280492 | 0.684390 | 0.882777 | 0.001744 | 0.001828 | 0.001894 |
| mul_vm_test.test | RUN | 0.390047 | 1.200681 | 1.243349 | 0.001638 | 0.001610 | 0.001718 |
| normalize_v_test.test | RUN | 0.280189 | 0.813804 | 0.885905 | 0.001826 | 0.001697 | 0.001675 |
| plus_eq_vv_test.test | RUN | 0.269840 | 0.634993 | 0.835217 | 0.001407 | 0.001980 | 0.001872 |
| plus_vv_test.test | RUN | 0.282349 | 0.677349 | 0.875952 | 0.001695 | 0.001917 | 0.001628 |
| scalar_cast_v_test.test | RUN | 0.253225 | 0.499079 | 0.776194 | 0.001583 | 0.001740 | 0.001414 |
| vec_index_test.test | RUN | 0.227616 | 0.526085 | 0.659581 | 0.001591 | 0.001526 | 0.001737 |
| vec_register_test.test | RUN | 0.143754 | 0.468989 | 0.504736 | 0.001749 | 0.001820 | 0.001603 |
| cmp_mm_test.test | RUN | 0.417309 | 1.463574 | 1.743966 | 0.002128 | 0.001917 | 0.001826 |
| determinant_test.test | RUN | 0.303088 | 0.775542 | 0.971783 | 0.001865 | 0.001899 | 0.001855 |
| div_eq_ms_test.test | RUN | 0.315994 | 0.713458 | 1.114434 | 0.001602 | 0.001645 | 0.002005 |
| div_ms_test.test | RUN | 0.368631 | 1.119474 | 1.349356 | 0.001759 | 0.001664 | 0.001836 |
| eq_mm_test.test | RUN | 0.400637 | 1.360287 | 1.665611 | 0.001759 | 0.001924 | 0.001827 |
| minus_m_test.test | RUN | 0.356328 | 1.093883 | 1.273282 | 0.001592 | 0.001858 | 0.001723 |
| minus_eq_mm_test.test | RUN | 0.340255 | 1.018444 | 1.137961 | 0.001521 | 0.001854 | 0.001534 |
| minus_mm_test.test | RUN | 0.379715 | 1.286963 | 1.402742 | 0.001727 | 0.001896 | 0.001856 |
| mul_eq_mm_test.test | RUN | 0.320509 | 0.953661 | 1.034769 | 0.001841 | 0.001654 | 0.001723 |
| mul_eq_ms_test.test | RUN | 0.314205 | 0.718389 | 1.090786 | 0.001863 | 0.001704 | 0.001734 |
| mul_mm_test.test | RUN | 0.446823 | 2.094004 | 1.731534 | 0.001820 | 0.001790 | 0.001631 |
| mul_ms_test.test | RUN | 0.364488 | 1.154855 | 1.338456 | 0.001571 | 0.001817 | 0.001691 |
| mul_sm_test.test | RUN | 0.372304 | 1.118697 | 1.366667 | 0.001961 | 0.001808 | 0.001924 |
| mul_mv_test.test | RUN | 0.400142 | 1.200147 | 1.332007 | 0.001858 | 0.001602 | 0.001649 |
| inverse_m_test.test | RUN | 0.478930 | 2.142821 | 1.867795 | 0.001988 | 0.001866 | 0.001817 |
| plus_eq_mm_test.test | RUN | 0.346486 | 1.032998 | 1.172584 | 0.001695 | 0.001860 | 0.001794 |
| plus_mm_test.test | RUN | 0.391064 | 1.282648 | 1.377570 | 0.001796 | 0.001900 | 0.001790 |
| scalar_cast_m_test.test | RUN | 0.287620 | 0.589229 | 0.849474 | 0.001570 | 0.001876 | 0.001869 |
| mat_index_test.test | RUN | 0.235097 | 0.566250 | 0.753496 | 0.001688 | 0.001918 | 0.001912 |
| cmp_qq_test.test | RUN | 0.254558 | 0.531411 | 0.747899 | 0.001821 | 0.001999 | 0.001809 |
| conjugate_test.test | RUN | 0.259737 | 0.528336 | 0.723027 | 0.001763 | 0.001925 | 0.001896 |
| normalize_q_test.test | RUN | 0.264569 | 0.595448 | 0.747274 | 0.001919 | 0.001833 | 0.001802 |
| div_eq_qs_test.test | RUN | 0.255493 | 0.536441 | 0.772803 | 0.001756 | 0.001906 | 0.001869 |
| div_qs_test.test | RUN | 0.261958 | 0.543019 | 0.795304 | 0.001865 | 0.001916 | 0.001629 |
| dot_qq_test.test | RUN | 0.261154 | 0.513265 | 0.759763 | 0.001886 | 0.002010 | 0.001860 |
| eq_qq_test.test | RUN | 0.265522 | 0.558336 | 0.735721 | 0.001658 | 0.001908 | 0.001872 |
| inverse_q_test.test | RUN | 0.319135 | 0.739133 | 0.963685 | 0.001827 | 0.001810 | 0.001630 |
| mag_q_test.test | RUN | 0.265738 | 0.530877 | 0.738957 | 0.001736 | 0.001866 | 0.001752 |
| mag_sqr_q_test.test | RUN | 0.265002 | 0.544013 | 0.735358 | 0.001801 | 0.001928 | 0.001846 |
| minus_q_test.test | RUN | 0.259431 | 0.532665 | 0.747791 | 0.001750 | 0.001883 | 0.001452 |
| minus_eq_qq_test.test | RUN | 0.260640 | 0.532633 | 0.732540 | 0.001816 | 0.001936 | 0.001896 |
| minus_qq_test.test | RUN | 0.262493 | 0.550064 | 0.754175 | 0.001424 | 0.001800 | 0.001795 |
| mul_eq_qs_test.test | RUN | 0.249415 | 0.521226 | 0.728116 | 0.001790 | 0.001938 | 0.002070 |
| mul_qs_test.test | RUN | 0.253958 | 0.548068 | 0.812013 | 0.001830 | 0.001940 | 0.001871 |
| mul_qv_test.test | RUN | 0.311200 | 0.737543 | 0.963689 | 0.001880 | 0.001685 | 0.001928 |
| mul_qq_test.test | RUN | 0.306879 | 0.759118 | 1.017532 | 0.002128 | 0.001920 | 0.001728 |
| mul_eq_qq_test.test | RUN | 0.307579 | 0.746845 | 0.991184 | 0.001901 | 0.001610 | 0.001738 |
| plus_eq_qq_test.test | RUN | 0.255433 | 0.534589 | 0.745722 | 0.001627 | 0.001729 | 0.001796 |
| plus_qq_test.test | RUN | 0.259499 | 0.542223 | 0.754093 | 0.001869 | 0.001776 | 0.001876 |
| scalar_cast_q_test.test | RUN | 0.254286 | 0.515871 | 0.749877 | 0.001713 | 0.001729 | 0.001874 |
| slerp_test.test | RUN | 0.273911 | 1.007166 | 0.888241 | 0.022132 | 0.014667 | 0.015652 |
| convert_to_test.test | RUN | 0.374431 | 0.974388 | 1.397112 | 0.001841 | 0.001676 | 0.001856 |
| rot_mat_test.test | RUN | 0.582320 | 4.176961 | 4.206031 | 0.078599 | 0.039440 | 0.035765 |
| rot_quat_test.test | RUN | 0.310637 | 0.852303 | 0.964253 | 0.001814 | 0.001922 | 0.001896 |
| rotx_mat_test.test | RUN | 0.349232 | 1.146605 | 1.277137 | 0.001705 | 0.001963 | 0.001960 |
| rotx_quat_test.test | RUN | 0.312396 | 0.763530 | 0.907397 | 0.001559 | 0.002014 | 0.001918 |
| roty_mat_test.test | RUN | 0.350250 | 1.144857 | 1.215055 | 0.001900 | 0.001916 | 0.001926 |
| roty_quat_test.test | RUN | 0.295501 | 0.764554 | 0.833422 | 0.001998 | 0.001830 | 0.001783 |
| rotz_mat_test.test | RUN | 0.352006 | 1.324215 | 1.324872 | 0.001497 | 0.001464 | 0.001884 |
| rotz_quat_test.test | RUN | 0.303511 | 0.754803 | 0.915228 | 0.001771 | 0.001624 | 0.001828 |
| zero_vec_test.test | RUN | 0.268857 | 0.660416 | 0.736576 | 0.001805 | 0.001799 | 0.001644 |
| zero_mat_test.test | RUN | 0.327121 | 1.067422 | 1.167819 | 0.001562 | 0.001924 | 0.001782 |
| zero_quat_test.test | RUN | 0.252943 | 0.563371 | 0.750525 | 0.001794 | 0.001634 | 0.001891 |
| identity_mat_test.test | RUN | 0.306244 | 0.772605 | 1.003697 | 0.001568 | 0.001711 | 0.001652 |
| identity_quat_test.test | RUN | 0.253856 | 0.521409 | 0.739568 | 0.001828 | 0.001944 | 0.001869 |
| col_test.test | RUN | 0.509723 | 1.623303 | 2.113999 | 0.001602 | 0.001764 | 0.001836 |
| col_mat_test.test | RUN | 0.312866 | 0.733148 | 1.022387 | 0.002005 | 0.001910 | 0.001802 |
| neg_col_test.test | RUN | 0.382558 | 1.061056 | 1.325797 | 0.001834 | 0.001709 | 0.001874 |
| neg_row_test.test | RUN | 0.428881 | 1.039504 | 1.288109 | 0.003717 | 0.001874 | 0.001628 |
| del_col_test.test | RUN | 0.419183 | 1.158212 | 1.315525 | 0.001816 | 0.001928 | 0.001842 |
| del_row_test.test | RUN | 0.450941 | 1.156840 | 1.290360 | 0.001882 | 0.001860 | 0.001846 |
| del_row_col_test.test | RUN | 0.583914 | 2.168907 | 2.538727 | 0.002414 | 0.001936 | 0.001828 |
| swap_cols_test.test | RUN | 0.751390 | 2.944821 | 3.025661 | 0.001746 | 0.001612 | 0.001949 |
| swap_rows_test.test | RUN | 0.791996 | 3.064822 | 3.082796 | 0.001499 | 0.001906 | 0.001811 |
| diag_test.test | RUN | 0.337872 | 0.768410 | 1.186899 | 0.001294 | 0.001873 | 0.001777 |
| diag_mat_test.test | RUN | 0.342800 | 0.796496 | 1.026414 | 0.001748 | 0.001805 | 0.001912 |
| row_test.test | RUN | 0.484822 | 1.712627 | 2.032001 | 0.001870 | 0.001690 | 0.001832 |
| row_mat_test.test | RUN | 0.322198 | 0.774007 | 0.971503 | 0.001889 | 0.001954 | 0.001653 |
| transpose_test.test | RUN | 0.379542 | 1.121844 | 1.300563 | 0.001872 | 0.001832 | 0.001600 |
| translation_test.test | RUN | 0.308023 | 0.720112 | 0.975159 | 0.001478 | 0.001936 | 0.001442 |
| translation_mat_test.test | RUN | 0.336878 | 0.746653 | 1.087227 | 0.001732 | 0.001714 | 0.001469 |
| swizzle_const_fail.test | COMPILE_FAIL | 0.120197 | 0.207090 | 0.284418 | — | — | — |
| swizzle2_test2.test | RUN | 0.139997 | 0.322649 | 0.418719 | 0.001451 | 0.001716 | 0.001542 |
| swizzle2_test3.test | RUN | 0.132548 | 0.303949 | 0.417758 | 0.001759 | 0.001605 | 0.001168 |
| swizzle2_test4.test | RUN | 0.140410 | 0.309474 | 0.420992 | 0.001552 | 0.001926 | 0.001422 |
| swizzle2_test.test | RUN | 0.433306 | 0.868444 | 1.377772 | 0.001920 | 0.001873 | 0.001810 |
| swizzle3_test2.test | RUN | 0.152034 | 0.337181 | 0.477102 | 0.001594 | 0.001908 | 0.001678 |
| swizzle3_test3.test | RUN | 0.148658 | 0.335286 | 0.467108 | 0.001526 | 0.001950 | 0.001516 |
| swizzle3_test4.test | RUN | 0.151625 | 0.336618 | 0.470274 | 0.001568 | 0.001955 | 0.001519 |
| swizzle3_test.test | RUN | 0.434477 | 0.884173 | 1.510109 | 0.001653 | 0.001588 | 0.002226 |
| swizzle4_test2.test | RUN | 0.255606 | 0.492574 | 0.836496 | 0.001779 | 0.001930 | 0.001666 |
| swizzle4_test3.test | RUN | 0.258347 | 0.480553 | 0.808862 | 0.001624 | 0.001944 | 0.001641 |
| swizzle4_test4.test | RUN | 0.261896 | 0.470770 | 0.892789 | 0.001657 | 0.001916 | 0.001692 |
| swizzle4_test.test | RUN | 0.450649 | 0.906083 | 1.487007 | 0.001659 | 0.001879 | 0.001601 |
| deduce_scalar_test.test | COMPILE | 0.023547 | 0.047573 | 0.075991 | — | — | — |
| deduce_scalar_mq_test.test | RUN | 0.167893 | 0.375591 | 0.552743 | 0.001594 | 0.001573 | 0.001593 |
| deduce_scalar_mv_test.test | RUN | 0.203949 | 0.412839 | 0.630042 | 0.001644 | 0.001660 | 0.001824 |
| deduce_scalar_fail.test | COMPILE_FAIL | 0.017920 | 0.032212 | 0.052670 | — | — | — |
| deduce_mat_test.test | COMPILE | 0.019253 | 0.043623 | 0.060819 | — | — | — |
| deduce_quat_test.test | COMPILE | 0.018791 | 0.037108 | 0.059106 | — | — | — |
| deduce_vec_test.test | COMPILE | 0.020916 | 0.040819 | 0.060498 | — | — | — |
| interop_test.test | RUN | 0.208456 | 0.529648 | 0.914209 | 0.000800 | 0.000899 | 0.001808 |
| transform_test.test | RUN | 0.392608 | 0.691161 | 1.142992 | 0.001427 | 0.001428 | 0.001328 |
| projection_test.test | RUN | 0.251807 | 0.615948 | 0.751982 | 0.001478 | 0.001459 | 0.001359 |
| scalar_traits_test.test | COMPILE | 0.020123 | 0.042145 | 0.071858 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.847×† | 0.755×, 0.939× | 0.997× |
| clang | 0.872×† | 0.890×, 0.854× | 1.055× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737952).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| access_m_test.test | 10 | 0.001060s | 0.001093s | 0.001120s | byte equality observed |
| access_q_test.test | 10 | 0.001038s | 0.001177s | 0.001089s | byte equality observed |
| access_v_test.test | 10 | 0.001015s | 0.001133s | 0.001094s | byte equality observed |
| assign_test.test | 10 | 0.001027s | 0.001135s | 0.001061s | byte equality observed |
| cmp_mm_test.test | 10 | 0.001020s | 0.001162s | 0.001058s | byte equality observed |
| cmp_qq_test.test | 10 | 0.001019s | 0.001194s | 0.001090s | byte equality observed |
| cmp_vv_test.test | 10 | 0.001034s | 0.001141s | 0.001090s | byte equality observed |
| col_mat_test.test | 10 | 0.001032s | 0.001113s | 0.001098s | byte equality observed |
| col_test.test | 10 | 0.001017s | 0.001180s | 0.001074s | byte equality observed |
| conjugate_test.test | 10 | 0.001059s | 0.001144s | 0.001057s | byte equality observed |
| convert_to_test.test | 10 | 0.001020s | 0.001098s | 0.001069s | byte equality observed |
| cross_test.test | 10 | 0.001052s | 0.001143s | 0.001082s | byte equality observed |
| deduce_scalar_mq_test.test | 10 | 0.001044s | 0.001151s | 0.001085s | byte equality observed |
| deduce_scalar_mv_test.test | 10 | 0.001069s | 0.001152s | 0.001098s | byte equality observed |
| del_col_test.test | 10 | 0.001054s | 0.001118s | 0.001062s | byte equality observed |
| del_row_col_test.test | 10 | 0.001077s | 0.001111s | 0.001076s | byte equality observed |
| del_row_test.test | 10 | 0.001027s | 0.001158s | 0.001058s | byte equality observed |
| determinant_test.test | 10 | 0.001039s | 0.001183s | 0.001109s | byte equality observed |
| diag_mat_test.test | 10 | 0.001042s | 0.001149s | 0.001090s | byte equality observed |
| diag_test.test | 10 | 0.001084s | 0.001121s | 0.001090s | byte equality observed |
| div_eq_ms_test.test | 10 | 0.001047s | 0.001121s | 0.001088s | byte equality observed |
| div_eq_qs_test.test | 10 | 0.001051s | 0.001116s | 0.001109s | byte equality observed |
| div_eq_vs_test.test | 10 | 0.001060s | 0.001123s | 0.001094s | byte equality observed |
| div_ms_test.test | 10 | 0.001057s | 0.001079s | 0.001116s | byte equality observed |
| div_qs_test.test | 10 | 0.001042s | 0.001103s | 0.001112s | byte equality observed |
| div_vs_test.test | 10 | 0.001062s | 0.001114s | 0.001130s | byte equality observed |
| dot_qq_test.test | 10 | 0.001027s | 0.001175s | 0.001113s | byte equality observed |
| dot_vv_test.test | 10 | 0.001050s | 0.001172s | 0.001116s | byte equality observed |
| eq_mm_test.test | 10 | 0.001055s | 0.001182s | 0.001107s | byte equality observed |
| eq_qq_test.test | 10 | 0.001022s | 0.001165s | 0.001180s | byte equality observed |
| eq_vv_test.test | 10 | 0.001064s | 0.001101s | 0.001148s | byte equality observed |
| identity_mat_test.test | 10 | 0.001111s | 0.001190s | 0.001176s | byte equality observed |
| identity_quat_test.test | 10 | 0.001095s | 0.001127s | 0.001194s | byte equality observed |
| interop_test.test | 10 | 0.000503s | 0.000519s | 0.001142s | byte equality observed |
| inverse_m_test.test | 10 | 0.001144s | 0.001216s | 0.001215s | byte equality observed |
| inverse_q_test.test | 10 | 0.001063s | 0.001150s | 0.001161s | byte equality observed |
| mag_q_test.test | 10 | 0.001047s | 0.001127s | 0.001185s | byte equality observed |
| mag_sqr_q_test.test | 10 | 0.001106s | 0.001153s | 0.001134s | byte equality observed |
| mag_sqr_v_test.test | 10 | 0.001119s | 0.001121s | 0.001181s | byte equality observed |
| mag_v_test.test | 10 | 0.001092s | 0.001164s | 0.001123s | byte equality observed |
| mat_index_test.test | 10 | 0.001136s | 0.001107s | 0.001143s | byte equality observed |
| mat_traits_array_test.test | 10 | 0.001027s | 0.001170s | 0.001161s | byte equality observed |
| mat_traits_std_array_test.test | 10 | 0.001091s | 0.001084s | 0.001184s | byte equality observed |
| math_test.test | 10 | 0.001563s | 0.001656s | 0.001565s | byte equality observed |
| minus_eq_mm_test.test | 10 | 0.001034s | 0.001148s | 0.001184s | byte equality observed |
| minus_eq_qq_test.test | 10 | 0.001066s | 0.001122s | 0.001151s | byte equality observed |
| minus_eq_vv_test.test | 10 | 0.001100s | 0.001065s | 0.001174s | byte equality observed |
| minus_m_test.test | 10 | 0.001058s | 0.001150s | 0.001162s | byte equality observed |
| minus_mm_test.test | 10 | 0.001020s | 0.001143s | 0.001180s | byte equality observed |
| minus_q_test.test | 10 | 0.001062s | 0.001148s | 0.001152s | byte equality observed |
| minus_qq_test.test | 10 | 0.001077s | 0.001190s | 0.001175s | byte equality observed |
| minus_v_test.test | 10 | 0.001040s | 0.001140s | 0.001160s | byte equality observed |
| minus_vv_test.test | 10 | 0.001105s | 0.001156s | 0.001158s | byte equality observed |
| mul_eq_mm_test.test | 10 | 0.001100s | 0.001141s | 0.001143s | byte equality observed |
| mul_eq_ms_test.test | 10 | 0.001035s | 0.001138s | 0.001129s | byte equality observed |
| mul_eq_qq_test.test | 10 | 0.001108s | 0.001112s | 0.001162s | byte equality observed |
| mul_eq_qs_test.test | 10 | 0.001024s | 0.001171s | 0.001178s | byte equality observed |
| mul_eq_vs_test.test | 10 | 0.001088s | 0.001158s | 0.001133s | byte equality observed |
| mul_mm_test.test | 10 | 0.001109s | 0.001061s | 0.001179s | byte equality observed |
| mul_ms_test.test | 10 | 0.001095s | 0.001132s | 0.001220s | byte equality observed |
| mul_mv_test.test | 10 | 0.001090s | 0.001144s | 0.001091s | byte equality observed |
| mul_qq_test.test | 10 | 0.001122s | 0.001173s | 0.001098s | byte equality observed |
| mul_qs_test.test | 10 | 0.001044s | 0.001144s | 0.001120s | byte equality observed |
| mul_qv_test.test | 10 | 0.001166s | 0.001167s | 0.001166s | byte equality observed |
| mul_sm_test.test | 10 | 0.001119s | 0.001164s | 0.001156s | byte equality observed |
| mul_sv_test.test | 10 | 0.001130s | 0.001188s | 0.001165s | byte equality observed |
| mul_vm_test.test | 10 | 0.001155s | 0.001176s | 0.001190s | byte equality observed |
| mul_vs_test.test | 10 | 0.001151s | 0.001160s | 0.001197s | byte equality observed |
| neg_col_test.test | 10 | 0.001154s | 0.001147s | 0.001172s | byte equality observed |
| neg_row_test.test | 10 | 0.001125s | 0.001156s | 0.001127s | byte equality observed |
| normalize_q_test.test | 10 | 0.001146s | 0.001177s | 0.001154s | byte equality observed |
| normalize_v_test.test | 10 | 0.001080s | 0.001128s | 0.001149s | byte equality observed |
| plus_eq_mm_test.test | 10 | 0.001099s | 0.001155s | 0.001117s | byte equality observed |
| plus_eq_qq_test.test | 10 | 0.001118s | 0.001131s | 0.001135s | byte equality observed |
| plus_eq_vv_test.test | 10 | 0.001134s | 0.001189s | 0.001115s | byte equality observed |
| plus_mm_test.test | 10 | 0.001138s | 0.001170s | 0.001102s | byte equality observed |
| plus_qq_test.test | 10 | 0.001137s | 0.001083s | 0.001098s | byte equality observed |
| plus_vv_test.test | 10 | 0.001092s | 0.001106s | 0.001173s | byte equality observed |
| projection_test.test | 10 | 0.001158s | 0.001116s | 0.001162s | byte equality observed |
| quat_traits_array_test.test | 10 | 0.001144s | 0.001147s | 0.001122s | byte equality observed |
| quat_traits_std_array_test.test | 8 | 0.001116s | 0.001182s | 0.001160s | byte equality observed |
| rot_mat_test.test | 1 | 0.060464s | 0.031920s | 0.032552s | byte equality observed |
| rot_quat_test.test | 10 | 0.001057s | 0.001157s | 0.001176s | byte equality observed |
| rotx_mat_test.test | 10 | 0.001148s | 0.001125s | 0.001134s | byte equality observed |
| rotx_quat_test.test | 10 | 0.001130s | 0.001139s | 0.001155s | byte equality observed |
| roty_mat_test.test | 10 | 0.001160s | 0.001123s | 0.001154s | byte equality observed |
| roty_quat_test.test | 10 | 0.001136s | 0.001054s | 0.001155s | byte equality observed |
| rotz_mat_test.test | 10 | 0.001192s | 0.001121s | 0.001164s | byte equality observed |
| rotz_quat_test.test | 10 | 0.001158s | 0.001106s | 0.001162s | byte equality observed |
| row_mat_test.test | 10 | 0.001110s | 0.001177s | 0.001101s | byte equality observed |
| row_test.test | 10 | 0.001098s | 0.001160s | 0.001137s | byte equality observed |
| scalar_cast_m_test.test | 10 | 0.001148s | 0.001183s | 0.001182s | byte equality observed |
| scalar_cast_q_test.test | 10 | 0.001094s | 0.001131s | 0.001159s | byte equality observed |
| scalar_cast_v_test.test | 10 | 0.001098s | 0.001140s | 0.001088s | byte equality observed |
| slerp_test.test | 2 | 0.018126s | 0.013357s | 0.013189s | byte equality observed |
| swap_cols_test.test | 10 | 0.001122s | 0.001151s | 0.001165s | byte equality observed |
| swap_rows_test.test | 10 | 0.001134s | 0.001158s | 0.001161s | byte equality observed |
| swizzle2_test.test | 10 | 0.001112s | 0.001143s | 0.001158s | byte equality observed |
| swizzle2_test2.test | 10 | 0.001092s | 0.001137s | 0.001178s | byte equality observed |
| swizzle2_test3.test | 10 | 0.001126s | 0.001105s | 0.001182s | byte equality observed |
| swizzle2_test4.test | 10 | 0.001106s | 0.001130s | 0.001192s | byte equality observed |
| swizzle3_test.test | 10 | 0.001102s | 0.001089s | 0.001170s | byte equality observed |
| swizzle3_test2.test | 10 | 0.001111s | 0.001116s | 0.001169s | byte equality observed |
| swizzle3_test3.test | 10 | 0.001118s | 0.001173s | 0.001151s | byte equality observed |
| swizzle3_test4.test | 10 | 0.001127s | 0.001166s | 0.001178s | byte equality observed |
| swizzle4_test.test | 10 | 0.001175s | 0.001150s | 0.001126s | byte equality observed |
| swizzle4_test2.test | 10 | 0.001146s | 0.001118s | 0.001133s | byte equality observed |
| swizzle4_test3.test | 10 | 0.001149s | 0.001074s | 0.001155s | byte equality observed |
| swizzle4_test4.test | 10 | 0.001140s | 0.001144s | 0.001146s | byte equality observed |
| to_string_test.test | 10 | 0.001170s | 0.001187s | 0.001218s | byte equality observed |
| transform_test.test | 10 | 0.001092s | 0.001153s | 0.001177s | byte equality observed |
| translation_mat_test.test | 10 | 0.001143s | 0.001167s | 0.001183s | byte equality observed |
| translation_test.test | 10 | 0.001138s | 0.001161s | 0.001199s | byte equality observed |
| transpose_test.test | 10 | 0.001171s | 0.001144s | 0.001190s | byte equality observed |
| vec_index_test.test | 10 | 0.001131s | 0.001215s | 0.001165s | byte equality observed |
| vec_register_test.test | 10 | 0.001156s | 0.001161s | 0.001214s | byte equality observed |
| vec_traits_array_test.test | 10 | 0.001155s | 0.001171s | 0.001198s | byte equality observed |
| vec_traits_gnuc_test.test | 10 | 0.001123s | 0.001147s | 0.001164s | byte equality observed |
| vec_traits_std_array_test.test | 10 | 0.001159s | 0.001080s | 0.001199s | byte equality observed |
| zero_mat_test.test | 10 | 0.001119s | 0.001078s | 0.001163s | byte equality observed |
| zero_quat_test.test | 10 | 0.001145s | 0.001112s | 0.001179s | byte equality observed |
| zero_vec_test.test | 10 | 0.001077s | 0.001174s | 0.001119s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737952).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
