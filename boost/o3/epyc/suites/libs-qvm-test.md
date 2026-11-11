# libs/qvm/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| math_test.test | RUN | 0.246702 | 0.527201 | 0.602718 | 0.002224 | 0.001928 | 0.002417 |
| mat_traits_array_test.test | RUN | 0.183082 | 0.521334 | 0.453532 | 0.001490 | 0.001658 | 0.001656 |
| mat_traits_std_array_test.test | RUN | 0.189310 | 0.409642 | 0.454307 | 0.001732 | 0.001549 | 0.001508 |
| vec_traits_array_test.test | RUN | 0.166870 | 0.416009 | 0.403373 | 0.001496 | 0.001540 | 0.001696 |
| vec_traits_gnuc_test.test | RUN | 0.295557 | 0.506090 | 0.732079 | 0.001964 | 0.001826 | 0.001674 |
| vec_traits_std_array_test.test | RUN | 0.164997 | 0.309488 | 0.387471 | 0.001578 | 0.001596 | 0.001972 |
| quat_traits_array_test.test | RUN | 0.161935 | 0.434415 | 0.382662 | 0.001603 | 0.001542 | 0.001608 |
| quat_traits_std_array_test.test | RUN | 0.163508 | 0.309071 | 0.379553 | 0.001510 | 0.001294 | 0.001564 |
| access_m_test.test | RUN | 0.254844 | 0.810288 | 0.869791 | 0.001578 | 0.001927 | 0.001730 |
| access_m_fail1.test | COMPILE_FAIL | 0.024458 | 0.035008 | 0.062865 | — | — | — |
| access_m_fail2.test | COMPILE_FAIL | 0.024255 | 0.034812 | 0.062393 | — | — | — |
| access_q_test.test | RUN | 0.220401 | 0.441860 | 0.535783 | 0.001697 | 0.001902 | 0.002413 |
| access_q_fail.test | COMPILE_FAIL | 0.019127 | 0.026942 | 0.053201 | — | — | — |
| access_v_test.test | RUN | 0.220770 | 0.468146 | 0.534140 | 0.001626 | 0.001844 | 0.001983 |
| access_v_fail1.test | COMPILE_FAIL | 0.018759 | 0.025382 | 0.050418 | — | — | — |
| access_v_fail2.test | COMPILE_FAIL | 0.018488 | 0.034965 | 0.050582 | — | — | — |
| assign_test.test | RUN | 0.328657 | 0.685006 | 0.961373 | 0.001724 | 0.001798 | 0.001927 |
| to_string_test.test | RUN | 0.363467 | 1.072289 | 5.839986 | 0.001511 | 0.001692 | 0.001936 |
| cmp_vv_test.test | RUN | 0.279652 | 0.518315 | 0.632496 | 0.001552 | 0.001846 | 0.001788 |
| cross_test.test | RUN | 0.263917 | 0.499321 | 0.619903 | 0.001648 | 0.001788 | 0.001755 |
| div_eq_vs_test.test | RUN | 0.254298 | 0.451925 | 0.609424 | 0.001676 | 0.001887 | 0.001800 |
| div_vs_test.test | RUN | 0.258018 | 0.556993 | 0.656894 | 0.001610 | 0.001784 | 0.001513 |
| dot_vv_test.test | RUN | 0.255652 | 0.493695 | 0.599414 | 0.001562 | 0.001554 | 0.001869 |
| eq_vv_test.test | RUN | 0.275944 | 0.560205 | 0.667357 | 0.001343 | 0.001553 | 0.001969 |
| mag_v_test.test | RUN | 0.240735 | 0.460813 | 0.590189 | 0.001927 | 0.001918 | 0.001656 |
| mag_sqr_v_test.test | RUN | 0.243740 | 0.461555 | 0.595185 | 0.001754 | 0.001888 | 0.001514 |
| minus_v_test.test | RUN | 0.254595 | 0.543130 | 0.620709 | 0.001486 | 0.001940 | 0.001467 |
| minus_eq_vv_test.test | RUN | 0.250984 | 0.533939 | 0.581314 | 0.001347 | 0.005206 | 0.001692 |
| minus_vv_test.test | RUN | 0.256524 | 0.593892 | 0.644021 | 0.001545 | 0.002968 | 0.001644 |
| mul_eq_vs_test.test | RUN | 0.244259 | 0.589003 | 0.586224 | 0.001505 | 0.001967 | 0.001626 |
| mul_vs_test.test | RUN | 0.260259 | 0.699720 | 0.658407 | 0.001476 | 0.001918 | 0.001698 |
| mul_sv_test.test | RUN | 0.262562 | 0.647551 | 0.665107 | 0.001530 | 0.001964 | 0.001797 |
| mul_vm_test.test | RUN | 0.364353 | 1.033412 | 0.977495 | 0.001713 | 0.001867 | 0.002086 |
| normalize_v_test.test | RUN | 0.264215 | 0.705661 | 0.694167 | 0.001552 | 0.001764 | 0.001956 |
| plus_eq_vv_test.test | RUN | 0.257020 | 0.574865 | 0.639781 | 0.001538 | 0.001844 | 0.001704 |
| plus_vv_test.test | RUN | 0.263298 | 0.602964 | 0.676524 | 0.001248 | 0.001829 | 0.001498 |
| scalar_cast_v_test.test | RUN | 0.240212 | 0.420071 | 0.579392 | 0.001619 | 0.001768 | 0.001514 |
| vec_index_test.test | RUN | 0.221277 | 0.421209 | 0.500310 | 0.001793 | 0.001607 | 0.001692 |
| vec_register_test.test | RUN | 0.136495 | 0.382345 | 0.364029 | 0.001846 | 0.001990 | 0.001883 |
| cmp_mm_test.test | RUN | 0.389063 | 1.225620 | 1.370153 | 0.001956 | 0.001663 | 0.001850 |
| determinant_test.test | RUN | 0.286368 | 0.628706 | 0.724601 | 0.001821 | 0.001908 | 0.001824 |
| div_eq_ms_test.test | RUN | 0.299723 | 0.586642 | 0.817305 | 0.001788 | 0.001567 | 0.001868 |
| div_ms_test.test | RUN | 0.338053 | 0.935518 | 1.028857 | 0.001960 | 0.001998 | 0.001860 |
| eq_mm_test.test | RUN | 0.386184 | 1.143933 | 1.268696 | 0.002110 | 0.001722 | 0.001843 |
| minus_m_test.test | RUN | 0.325044 | 0.896855 | 0.941732 | 0.001760 | 0.001998 | 0.001853 |
| minus_eq_mm_test.test | RUN | 0.312914 | 0.862223 | 0.886707 | 0.001604 | 0.001537 | 0.001891 |
| minus_mm_test.test | RUN | 0.340329 | 1.084151 | 1.015720 | 0.001876 | 0.001932 | 0.001948 |
| mul_eq_mm_test.test | RUN | 0.291237 | 0.834754 | 0.807289 | 0.001739 | 0.001749 | 0.001898 |
| mul_eq_ms_test.test | RUN | 0.308105 | 0.648825 | 0.807506 | 0.001547 | 0.001848 | 0.001877 |
| mul_mm_test.test | RUN | 0.393347 | 1.795133 | 1.357130 | 0.001937 | 0.001733 | 0.001924 |
| mul_ms_test.test | RUN | 0.333932 | 0.995170 | 1.059356 | 0.001631 | 0.001918 | 0.001548 |
| mul_sm_test.test | RUN | 0.336003 | 0.961063 | 1.024377 | 0.001845 | 0.001646 | 0.002076 |
| mul_mv_test.test | RUN | 0.366830 | 1.045335 | 1.068252 | 0.001866 | 0.001698 | 0.001926 |
| inverse_m_test.test | RUN | 0.433983 | 1.836344 | 1.479231 | 0.001827 | 0.001820 | 0.001802 |
| plus_eq_mm_test.test | RUN | 0.314090 | 0.849775 | 0.895772 | 0.001886 | 0.001913 | 0.001862 |
| plus_mm_test.test | RUN | 0.361129 | 1.138811 | 1.065365 | 0.001828 | 0.001732 | 0.001776 |
| scalar_cast_m_test.test | RUN | 0.284420 | 0.507630 | 0.668991 | 0.001686 | 0.002129 | 0.001631 |
| mat_index_test.test | RUN | 0.225493 | 0.469186 | 0.579306 | 0.001455 | 0.001752 | 0.001830 |
| cmp_qq_test.test | RUN | 0.246002 | 0.457060 | 0.547685 | 0.001652 | 0.001794 | 0.001794 |
| conjugate_test.test | RUN | 0.244633 | 0.461855 | 0.553464 | 0.001774 | 0.002084 | 0.001782 |
| normalize_q_test.test | RUN | 0.252837 | 0.499359 | 0.567323 | 0.001486 | 0.001433 | 0.001682 |
| div_eq_qs_test.test | RUN | 0.243595 | 0.427688 | 0.553154 | 0.001678 | 0.001934 | 0.001782 |
| div_qs_test.test | RUN | 0.250651 | 0.458955 | 0.563838 | 0.001773 | 0.002035 | 0.001912 |
| dot_qq_test.test | RUN | 0.256840 | 0.474018 | 0.567701 | 0.001836 | 0.001841 | 0.001884 |
| eq_qq_test.test | RUN | 0.248100 | 0.498603 | 0.573898 | 0.001628 | 0.001706 | 0.001662 |
| inverse_q_test.test | RUN | 0.290244 | 0.637615 | 0.711304 | 0.001628 | 0.001706 | 0.001495 |
| mag_q_test.test | RUN | 0.250906 | 0.496211 | 0.544078 | 0.001553 | 0.002018 | 0.001932 |
| mag_sqr_q_test.test | RUN | 0.244905 | 0.446812 | 0.544686 | 0.001788 | 0.001844 | 0.001791 |
| minus_q_test.test | RUN | 0.252241 | 0.454359 | 0.528973 | 0.001805 | 0.001744 | 0.001649 |
| minus_eq_qq_test.test | RUN | 0.251783 | 0.449015 | 0.552820 | 0.001638 | 0.002137 | 0.001800 |
| minus_qq_test.test | RUN | 0.252762 | 0.466914 | 0.553861 | 0.001823 | 0.002200 | 0.001886 |
| mul_eq_qs_test.test | RUN | 0.246516 | 0.442485 | 0.517102 | 0.001886 | 0.001782 | 0.001644 |
| mul_qs_test.test | RUN | 0.264017 | 0.472029 | 0.557743 | 0.001814 | 0.001819 | 0.001732 |
| mul_qv_test.test | RUN | 0.324823 | 0.632364 | 0.724855 | 0.001852 | 0.002022 | 0.001787 |
| mul_qq_test.test | RUN | 0.308197 | 0.652915 | 0.683673 | 0.001820 | 0.003621 | 0.002065 |
| mul_eq_qq_test.test | RUN | 0.305087 | 0.669853 | 0.693665 | 0.001716 | 0.001622 | 0.001754 |
| plus_eq_qq_test.test | RUN | 0.242553 | 0.496343 | 0.558921 | 0.001806 | 0.001878 | 0.001836 |
| plus_qq_test.test | RUN | 0.264478 | 0.450929 | 0.578788 | 0.001380 | 0.001763 | 0.002032 |
| scalar_cast_q_test.test | RUN | 0.265668 | 0.408955 | 0.563214 | 0.001730 | 0.001703 | 0.001819 |
| slerp_test.test | RUN | 0.286257 | 0.880028 | 0.685797 | 0.016446 | 0.012499 | 0.013912 |
| convert_to_test.test | RUN | 0.359240 | 0.784174 | 1.088792 | 0.001954 | 0.001917 | 0.001850 |
| rot_mat_test.test | RUN | 0.511601 | 3.502424 | 3.453869 | 0.058315 | 0.032746 | 0.029821 |
| rot_quat_test.test | RUN | 0.307848 | 0.728910 | 0.706884 | 0.001984 | 0.001864 | 0.001907 |
| rotx_mat_test.test | RUN | 0.340298 | 0.967696 | 0.935577 | 0.001589 | 0.001886 | 0.001931 |
| rotx_quat_test.test | RUN | 0.299915 | 0.644483 | 0.683711 | 0.001819 | 0.001976 | 0.001752 |
| roty_mat_test.test | RUN | 0.325556 | 0.980371 | 0.928434 | 0.001795 | 0.002046 | 0.001969 |
| roty_quat_test.test | RUN | 0.292904 | 0.633164 | 0.653761 | 0.001988 | 0.001820 | 0.001944 |
| rotz_mat_test.test | RUN | 0.333502 | 1.083309 | 1.017737 | 0.001908 | 0.001709 | 0.001776 |
| rotz_quat_test.test | RUN | 0.291914 | 0.649164 | 0.671614 | 0.002021 | 0.001722 | 0.001843 |
| zero_vec_test.test | RUN | 0.263923 | 0.565266 | 0.544754 | 0.001815 | 0.001888 | 0.001810 |
| zero_mat_test.test | RUN | 0.312372 | 0.836752 | 0.814948 | 0.001359 | 0.001650 | 0.001690 |
| zero_quat_test.test | RUN | 0.249530 | 0.445872 | 0.555703 | 0.001710 | 0.001779 | 0.001836 |
| identity_mat_test.test | RUN | 0.306153 | 0.647925 | 0.655840 | 0.002006 | 0.001618 | 0.001947 |
| identity_quat_test.test | RUN | 0.249505 | 0.433409 | 0.533257 | 0.001893 | 0.001964 | 0.001854 |
| col_test.test | RUN | 0.437658 | 1.415373 | 1.602439 | 0.001884 | 0.001414 | 0.001828 |
| col_mat_test.test | RUN | 0.301892 | 0.630827 | 0.734781 | 0.001832 | 0.002087 | 0.002215 |
| neg_col_test.test | RUN | 0.341137 | 0.912818 | 0.941533 | 0.001595 | 0.001834 | 0.001692 |
| neg_row_test.test | RUN | 0.358865 | 0.883705 | 0.956120 | 0.001792 | 0.001832 | 0.001692 |
| del_col_test.test | RUN | 0.348057 | 0.954036 | 0.979240 | 0.001938 | 0.002066 | 0.001802 |
| del_row_test.test | RUN | 0.344116 | 0.957809 | 0.946784 | 0.001792 | 0.001949 | 0.001740 |
| del_row_col_test.test | RUN | 0.466361 | 1.766642 | 1.829173 | 0.001840 | 0.001571 | 0.001551 |
| swap_cols_test.test | RUN | 0.636921 | 2.362812 | 2.319612 | 0.002197 | 0.001736 | 0.002108 |
| swap_rows_test.test | RUN | 0.682887 | 2.428753 | 2.326884 | 0.001542 | 0.001570 | 0.001338 |
| diag_test.test | RUN | 0.311734 | 0.621928 | 0.855866 | 0.001530 | 0.001810 | 0.001672 |
| diag_mat_test.test | RUN | 0.328397 | 0.654992 | 0.804059 | 0.001737 | 0.001812 | 0.001612 |
| row_test.test | RUN | 0.436583 | 1.405712 | 1.594023 | 0.001679 | 0.001609 | 0.001817 |
| row_mat_test.test | RUN | 0.297408 | 0.630670 | 0.703388 | 0.001871 | 0.001756 | 0.001747 |
| transpose_test.test | RUN | 0.331511 | 0.945359 | 0.955228 | 0.001625 | 0.001748 | 0.001696 |
| translation_test.test | RUN | 0.280941 | 0.576218 | 0.723504 | 0.001927 | 0.001635 | 0.001762 |
| translation_mat_test.test | RUN | 0.308208 | 0.595763 | 0.779660 | 0.001563 | 0.001804 | 0.001093 |
| swizzle_const_fail.test | COMPILE_FAIL | 0.106467 | 0.170613 | 0.201916 | — | — | — |
| swizzle2_test2.test | RUN | 0.124831 | 0.257621 | 0.312638 | 0.001769 | 0.001700 | 0.001468 |
| swizzle2_test3.test | RUN | 0.127601 | 0.262389 | 0.299422 | 0.001489 | 0.001760 | 0.001620 |
| swizzle2_test4.test | RUN | 0.135300 | 0.250722 | 0.311149 | 0.001781 | 0.001936 | 0.003715 |
| swizzle2_test.test | RUN | 0.406042 | 0.702562 | 0.937138 | 0.001926 | 0.001690 | 0.002028 |
| swizzle3_test2.test | RUN | 0.147936 | 0.281871 | 0.351421 | 0.001525 | 0.001588 | 0.001850 |
| swizzle3_test3.test | RUN | 0.143936 | 0.291340 | 0.346522 | 0.001545 | 0.001556 | 0.001771 |
| swizzle3_test4.test | RUN | 0.148554 | 0.279521 | 0.329176 | 0.001946 | 0.001451 | 0.001693 |
| swizzle3_test.test | RUN | 0.407971 | 0.710905 | 0.916614 | 0.001616 | 0.001410 | 0.001600 |
| swizzle4_test2.test | RUN | 0.239711 | 0.393813 | 0.526572 | 0.001757 | 0.001788 | 0.001604 |
| swizzle4_test3.test | RUN | 0.245631 | 0.376661 | 0.516754 | 0.001794 | 0.001725 | 0.001423 |
| swizzle4_test4.test | RUN | 0.241019 | 0.392015 | 0.535278 | 0.001847 | 0.001446 | 0.002014 |
| swizzle4_test.test | RUN | 0.422866 | 0.796397 | 0.950102 | 0.001594 | 0.001847 | 0.002054 |
| deduce_scalar_test.test | COMPILE | 0.022162 | 0.071068 | 0.058333 | — | — | — |
| deduce_scalar_mq_test.test | RUN | 0.164820 | 0.333914 | 0.348959 | 0.001784 | 0.001552 | 0.001496 |
| deduce_scalar_mv_test.test | RUN | 0.200273 | 0.353090 | 0.432508 | 0.001540 | 0.001666 | 0.001801 |
| deduce_scalar_fail.test | COMPILE_FAIL | 0.018391 | 0.025276 | 0.042782 | — | — | — |
| deduce_mat_test.test | COMPILE | 0.017745 | 0.034350 | 0.051073 | — | — | — |
| deduce_quat_test.test | COMPILE | 0.019622 | 0.036702 | 0.052517 | — | — | — |
| deduce_vec_test.test | COMPILE | 0.020878 | 0.034645 | 0.049144 | — | — | — |
| interop_test.test | RUN | 0.186388 | 0.366972 | 0.638309 | 0.000672 | 0.000668 | 0.001578 |
| transform_test.test | RUN | 0.369037 | 0.645910 | 0.917926 | 0.001495 | 0.001473 | 0.001418 |
| projection_test.test | RUN | 0.251198 | 0.504888 | 0.504413 | 0.001395 | 0.001432 | 0.001548 |
| scalar_traits_test.test | COMPILE | 0.022063 | 0.036415 | 0.060523 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.869× | 0.853×, 0.886× | 1.011× |
| clang | 0.897× | 0.887×, 0.906× | 0.992× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| access_m_test.test | 10 | 0.000967s | 0.000969s | 0.001015s | byte equality observed |
| access_q_test.test | 10 | 0.000958s | 0.000942s | 0.000962s | byte equality observed |
| access_v_test.test | 10 | 0.000960s | 0.000942s | 0.000985s | byte equality observed |
| assign_test.test | 10 | 0.000957s | 0.000943s | 0.000966s | byte equality observed |
| cmp_mm_test.test | 10 | 0.000979s | 0.000977s | 0.000981s | byte equality observed |
| cmp_qq_test.test | 10 | 0.000972s | 0.000955s | 0.000952s | byte equality observed |
| cmp_vv_test.test | 10 | 0.000975s | 0.000929s | 0.000998s | byte equality observed |
| col_mat_test.test | 10 | 0.000961s | 0.000954s | 0.000987s | byte equality observed |
| col_test.test | 10 | 0.000974s | 0.000985s | 0.000980s | byte equality observed |
| conjugate_test.test | 10 | 0.000973s | 0.000981s | 0.000955s | byte equality observed |
| convert_to_test.test | 10 | 0.000978s | 0.000948s | 0.000975s | byte equality observed |
| cross_test.test | 10 | 0.000975s | 0.000957s | 0.000965s | byte equality observed |
| deduce_scalar_mq_test.test | 10 | 0.000982s | 0.000936s | 0.000963s | byte equality observed |
| deduce_scalar_mv_test.test | 10 | 0.000992s | 0.000948s | 0.000984s | byte equality observed |
| del_col_test.test | 10 | 0.000965s | 0.000953s | 0.000974s | byte equality observed |
| del_row_col_test.test | 10 | 0.000991s | 0.000938s | 0.000982s | byte equality observed |
| del_row_test.test | 10 | 0.000953s | 0.000970s | 0.000984s | byte equality observed |
| determinant_test.test | 10 | 0.001002s | 0.000947s | 0.001035s | byte equality observed |
| diag_mat_test.test | 10 | 0.000959s | 0.000926s | 0.000976s | byte equality observed |
| diag_test.test | 10 | 0.000965s | 0.000945s | 0.000994s | byte equality observed |
| div_eq_ms_test.test | 10 | 0.000964s | 0.000950s | 0.000957s | byte equality observed |
| div_eq_qs_test.test | 10 | 0.001000s | 0.000932s | 0.000957s | byte equality observed |
| div_eq_vs_test.test | 10 | 0.000997s | 0.000948s | 0.000994s | byte equality observed |
| div_ms_test.test | 10 | 0.000994s | 0.000938s | 0.000964s | byte equality observed |
| div_qs_test.test | 10 | 0.000978s | 0.000941s | 0.000989s | byte equality observed |
| div_vs_test.test | 10 | 0.000963s | 0.000935s | 0.000956s | byte equality observed |
| dot_qq_test.test | 10 | 0.000969s | 0.000921s | 0.001006s | byte equality observed |
| dot_vv_test.test | 10 | 0.000979s | 0.000916s | 0.000989s | byte equality observed |
| eq_mm_test.test | 10 | 0.000985s | 0.000961s | 0.001012s | byte equality observed |
| eq_qq_test.test | 10 | 0.000970s | 0.000939s | 0.000976s | byte equality observed |
| eq_vv_test.test | 10 | 0.000982s | 0.000963s | 0.000989s | byte equality observed |
| identity_mat_test.test | 10 | 0.000980s | 0.000917s | 0.000970s | byte equality observed |
| identity_quat_test.test | 10 | 0.000974s | 0.000963s | 0.001020s | byte equality observed |
| interop_test.test | 10 | 0.000455s | 0.000450s | 0.001037s | byte equality observed |
| inverse_m_test.test | 10 | 0.001047s | 0.000998s | 0.001042s | byte equality observed |
| inverse_q_test.test | 10 | 0.001001s | 0.000981s | 0.001011s | byte equality observed |
| mag_q_test.test | 9 | 0.000963s | 0.000939s | 0.000973s | byte equality observed |
| mag_sqr_q_test.test | 10 | 0.000960s | 0.000933s | 0.000980s | byte equality observed |
| mag_sqr_v_test.test | 10 | 0.000981s | 0.000919s | 0.000963s | byte equality observed |
| mag_v_test.test | 10 | 0.000991s | 0.000940s | 0.000974s | byte equality observed |
| mat_index_test.test | 10 | 0.001013s | 0.000932s | 0.000972s | byte equality observed |
| mat_traits_array_test.test | 10 | 0.000987s | 0.000922s | 0.000965s | byte equality observed |
| mat_traits_std_array_test.test | 10 | 0.000977s | 0.000932s | 0.000954s | byte equality observed |
| math_test.test | 10 | 0.001469s | 0.001354s | 0.001319s | byte equality observed |
| minus_eq_mm_test.test | 10 | 0.000951s | 0.000926s | 0.000996s | byte equality observed |
| minus_eq_qq_test.test | 10 | 0.000951s | 0.000926s | 0.000982s | byte equality observed |
| minus_eq_vv_test.test | 10 | 0.000951s | 0.000930s | 0.001008s | byte equality observed |
| minus_m_test.test | 10 | 0.000968s | 0.000938s | 0.001032s | byte equality observed |
| minus_mm_test.test | 10 | 0.000994s | 0.000929s | 0.000977s | byte equality observed |
| minus_q_test.test | 10 | 0.000974s | 0.000959s | 0.000968s | byte equality observed |
| minus_qq_test.test | 10 | 0.000947s | 0.000944s | 0.000954s | byte equality observed |
| minus_v_test.test | 10 | 0.000941s | 0.000939s | 0.000966s | byte equality observed |
| minus_vv_test.test | 10 | 0.000982s | 0.000937s | 0.000968s | byte equality observed |
| mul_eq_mm_test.test | 10 | 0.000975s | 0.000936s | 0.000955s | byte equality observed |
| mul_eq_ms_test.test | 10 | 0.000975s | 0.000926s | 0.000991s | byte equality observed |
| mul_eq_qq_test.test | 10 | 0.000987s | 0.000974s | 0.001005s | byte equality observed |
| mul_eq_qs_test.test | 10 | 0.000974s | 0.000944s | 0.000978s | byte equality observed |
| mul_eq_vs_test.test | 10 | 0.000966s | 0.000941s | 0.000950s | byte equality observed |
| mul_mm_test.test | 10 | 0.000971s | 0.000939s | 0.000982s | byte equality observed |
| mul_ms_test.test | 10 | 0.000975s | 0.000944s | 0.000968s | byte equality observed |
| mul_mv_test.test | 10 | 0.000963s | 0.000959s | 0.000978s | byte equality observed |
| mul_qq_test.test | 10 | 0.000971s | 0.000951s | 0.000990s | byte equality observed |
| mul_qs_test.test | 10 | 0.000983s | 0.000936s | 0.000977s | byte equality observed |
| mul_qv_test.test | 10 | 0.000973s | 0.000970s | 0.000975s | byte equality observed |
| mul_sm_test.test | 10 | 0.000970s | 0.000963s | 0.000978s | byte equality observed |
| mul_sv_test.test | 10 | 0.000958s | 0.000943s | 0.001023s | byte equality observed |
| mul_vm_test.test | 10 | 0.000964s | 0.000977s | 0.001006s | byte equality observed |
| mul_vs_test.test | 10 | 0.000975s | 0.000953s | 0.001001s | byte equality observed |
| neg_col_test.test | 10 | 0.000967s | 0.000950s | 0.000992s | byte equality observed |
| neg_row_test.test | 10 | 0.000974s | 0.000950s | 0.000990s | byte equality observed |
| normalize_q_test.test | 10 | 0.000981s | 0.000967s | 0.000984s | byte equality observed |
| normalize_v_test.test | 10 | 0.000992s | 0.000950s | 0.000976s | byte equality observed |
| plus_eq_mm_test.test | 10 | 0.000989s | 0.000977s | 0.001003s | byte equality observed |
| plus_eq_qq_test.test | 10 | 0.000977s | 0.000964s | 0.000985s | byte equality observed |
| plus_eq_vv_test.test | 10 | 0.000971s | 0.000951s | 0.000983s | byte equality observed |
| plus_mm_test.test | 10 | 0.000956s | 0.000984s | 0.000990s | byte equality observed |
| plus_qq_test.test | 10 | 0.000953s | 0.000958s | 0.001022s | byte equality observed |
| plus_vv_test.test | 10 | 0.000952s | 0.000954s | 0.000996s | byte equality observed |
| projection_test.test | 10 | 0.000981s | 0.000976s | 0.001018s | byte equality observed |
| quat_traits_array_test.test | 10 | 0.000963s | 0.000958s | 0.000992s | byte equality observed |
| quat_traits_std_array_test.test | 10 | 0.000953s | 0.000972s | 0.000958s | byte equality observed |
| rot_mat_test.test | 1 | 0.035488s | 0.021585s | 0.019999s | byte equality observed |
| rot_quat_test.test | 10 | 0.000954s | 0.000914s | 0.000929s | byte equality observed |
| rotx_mat_test.test | 10 | 0.000958s | 0.000938s | 0.000981s | byte equality observed |
| rotx_quat_test.test | 10 | 0.000971s | 0.000963s | 0.001017s | byte equality observed |
| roty_mat_test.test | 10 | 0.001011s | 0.000958s | 0.001025s | byte equality observed |
| roty_quat_test.test | 10 | 0.000984s | 0.000946s | 0.000998s | byte equality observed |
| rotz_mat_test.test | 10 | 0.001029s | 0.000961s | 0.000989s | byte equality observed |
| rotz_quat_test.test | 10 | 0.000982s | 0.000954s | 0.000999s | byte equality observed |
| row_mat_test.test | 10 | 0.000962s | 0.000973s | 0.000937s | byte equality observed |
| row_test.test | 10 | 0.000992s | 0.000958s | 0.000952s | byte equality observed |
| scalar_cast_m_test.test | 10 | 0.000977s | 0.000941s | 0.001011s | byte equality observed |
| scalar_cast_q_test.test | 10 | 0.000964s | 0.000939s | 0.000974s | byte equality observed |
| scalar_cast_v_test.test | 10 | 0.000927s | 0.000896s | 0.000942s | byte equality observed |
| slerp_test.test | 2 | 0.010675s | 0.008341s | 0.008857s | byte equality observed |
| swap_cols_test.test | 10 | 0.000941s | 0.000943s | 0.000948s | byte equality observed |
| swap_rows_test.test | 10 | 0.001002s | 0.000959s | 0.000992s | byte equality observed |
| swizzle2_test.test | 10 | 0.000970s | 0.000966s | 0.000982s | byte equality observed |
| swizzle2_test2.test | 10 | 0.000968s | 0.000933s | 0.001001s | byte equality observed |
| swizzle2_test3.test | 10 | 0.000978s | 0.000951s | 0.000985s | byte equality observed |
| swizzle2_test4.test | 10 | 0.001001s | 0.000945s | 0.000997s | byte equality observed |
| swizzle3_test.test | 10 | 0.000985s | 0.000951s | 0.000960s | byte equality observed |
| swizzle3_test2.test | 10 | 0.000963s | 0.000948s | 0.000993s | byte equality observed |
| swizzle3_test3.test | 10 | 0.000968s | 0.000958s | 0.000949s | byte equality observed |
| swizzle3_test4.test | 10 | 0.000984s | 0.000937s | 0.000949s | byte equality observed |
| swizzle4_test.test | 10 | 0.000961s | 0.000930s | 0.000992s | byte equality observed |
| swizzle4_test2.test | 10 | 0.000960s | 0.000929s | 0.000986s | byte equality observed |
| swizzle4_test3.test | 10 | 0.000951s | 0.000926s | 0.000985s | byte equality observed |
| swizzle4_test4.test | 10 | 0.000981s | 0.000947s | 0.000971s | byte equality observed |
| to_string_test.test | 10 | 0.001011s | 0.000990s | 0.001001s | byte equality observed |
| transform_test.test | 10 | 0.000981s | 0.000950s | 0.000971s | byte equality observed |
| translation_mat_test.test | 10 | 0.000975s | 0.000944s | 0.000980s | byte equality observed |
| translation_test.test | 10 | 0.000960s | 0.000930s | 0.000960s | byte equality observed |
| transpose_test.test | 10 | 0.000997s | 0.000934s | 0.000977s | byte equality observed |
| vec_index_test.test | 10 | 0.000969s | 0.000956s | 0.000998s | byte equality observed |
| vec_register_test.test | 10 | 0.000962s | 0.000953s | 0.000986s | byte equality observed |
| vec_traits_array_test.test | 10 | 0.000953s | 0.000940s | 0.000962s | byte equality observed |
| vec_traits_gnuc_test.test | 10 | 0.000948s | 0.000940s | 0.000946s | byte equality observed |
| vec_traits_std_array_test.test | 10 | 0.000954s | 0.000936s | 0.000946s | byte equality observed |
| zero_mat_test.test | 10 | 0.000973s | 0.000945s | 0.000948s | byte equality observed |
| zero_quat_test.test | 10 | 0.000942s | 0.000938s | 0.000941s | byte equality observed |
| zero_vec_test.test | 10 | 0.000965s | 0.000949s | 0.000953s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
