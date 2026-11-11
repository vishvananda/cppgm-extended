# libs/qvm/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| math_test.test | RUN | 0.369551 | 0.848510 | 1.028860 | 0.002313 | 0.002952 | 0.003388 |
| mat_traits_array_test.test | RUN | 0.236496 | 0.865273 | 0.822077 | 0.001557 | 0.002132 | 0.002243 |
| mat_traits_std_array_test.test | RUN | 0.237462 | 0.645686 | 0.775096 | 0.001595 | 0.002002 | 0.002037 |
| vec_traits_array_test.test | RUN | 0.210745 | 0.661242 | 0.634337 | 0.001800 | 0.001720 | 0.001477 |
| vec_traits_gnuc_test.test | RUN | 0.390417 | 0.822529 | 1.461645 | 0.001411 | 0.001934 | 0.001988 |
| vec_traits_std_array_test.test | RUN | 0.216379 | 0.509580 | 0.648212 | 0.001702 | 0.001736 | 0.002094 |
| quat_traits_array_test.test | RUN | 0.202099 | 0.693300 | 0.640540 | 0.001765 | 0.001729 | 0.002028 |
| quat_traits_std_array_test.test | RUN | 0.203227 | 0.509135 | 0.713170 | 0.001978 | 0.001416 | 0.002515 |
| access_m_test.test | RUN | 0.323577 | 1.257766 | 1.497383 | 0.001430 | 0.001740 | 0.001616 |
| access_m_fail1.test | COMPILE_FAIL | 0.031940 | 0.055273 | 0.099948 | — | — | — |
| access_m_fail2.test | COMPILE_FAIL | 0.035018 | 0.054069 | 0.101838 | — | — | — |
| access_q_test.test | RUN | 0.276811 | 0.632895 | 0.850276 | 0.001727 | 0.001968 | 0.001870 |
| access_q_fail.test | COMPILE_FAIL | 0.024894 | 0.043796 | 0.085759 | — | — | — |
| access_v_test.test | RUN | 0.271621 | 0.660080 | 0.915182 | 0.001411 | 0.001442 | 0.001485 |
| access_v_fail1.test | COMPILE_FAIL | 0.022226 | 0.037873 | 0.078015 | — | — | — |
| access_v_fail2.test | COMPILE_FAIL | 0.022771 | 0.038421 | 0.081605 | — | — | — |
| assign_test.test | RUN | 0.416291 | 1.058648 | 1.561816 | 0.001778 | 0.002388 | 0.002263 |
| to_string_test.test | RUN | 0.541839 | 1.732729 | 8.436113 | 0.001706 | 0.001968 | 0.002653 |
| cmp_vv_test.test | RUN | 0.370595 | 0.854796 | 1.029138 | 0.001597 | 0.001762 | 0.002139 |
| cross_test.test | RUN | 0.342811 | 0.779264 | 1.033012 | 0.002030 | 0.001642 | 0.002947 |
| div_eq_vs_test.test | RUN | 0.341841 | 0.738196 | 1.024559 | 0.001597 | 0.001688 | 0.001745 |
| div_vs_test.test | RUN | 0.338145 | 0.856537 | 1.171796 | 0.001722 | 0.001594 | 0.001836 |
| dot_vv_test.test | RUN | 0.319133 | 0.833027 | 1.073753 | 0.001832 | 0.001869 | 0.001628 |
| eq_vv_test.test | RUN | 0.344076 | 0.876520 | 1.125219 | 0.001978 | 0.001448 | 0.001946 |
| mag_v_test.test | RUN | 0.311487 | 0.708062 | 1.012601 | 0.001880 | 0.001441 | 0.001834 |
| mag_sqr_v_test.test | RUN | 0.309027 | 0.696936 | 0.977553 | 0.001613 | 0.002033 | 0.001405 |
| minus_v_test.test | RUN | 0.331203 | 0.814018 | 1.096358 | 0.001666 | 0.001828 | 0.001524 |
| minus_eq_vv_test.test | RUN | 0.328902 | 0.814171 | 1.042976 | 0.001592 | 0.001669 | 0.001519 |
| minus_vv_test.test | RUN | 0.348054 | 0.876001 | 1.124223 | 0.001618 | 0.002312 | 0.001584 |
| mul_eq_vs_test.test | RUN | 0.320016 | 0.710343 | 1.067827 | 0.001658 | 0.001890 | 0.002048 |
| mul_vs_test.test | RUN | 0.346163 | 0.853392 | 1.124554 | 0.001791 | 0.002046 | 0.001449 |
| mul_sv_test.test | RUN | 0.365125 | 0.882508 | 1.130664 | 0.001477 | 0.002186 | 0.002104 |
| mul_vm_test.test | RUN | 0.507288 | 1.521052 | 1.685982 | 0.001698 | 0.002651 | 0.001584 |
| normalize_v_test.test | RUN | 0.358089 | 1.070418 | 1.104596 | 0.001672 | 0.001972 | 0.001638 |
| plus_eq_vv_test.test | RUN | 0.331625 | 0.800332 | 1.046060 | 0.001658 | 0.002547 | 0.002148 |
| plus_vv_test.test | RUN | 0.362037 | 0.895146 | 1.078611 | 0.001668 | 0.001982 | 0.002268 |
| scalar_cast_v_test.test | RUN | 0.320905 | 0.663784 | 0.956563 | 0.001680 | 0.001896 | 0.002526 |
| vec_index_test.test | RUN | 0.292194 | 0.657662 | 0.838233 | 0.001912 | 0.001614 | 0.002400 |
| vec_register_test.test | RUN | 0.180439 | 0.579781 | 0.640515 | 0.001626 | 0.002114 | 0.001986 |
| cmp_mm_test.test | RUN | 0.528372 | 1.900823 | 2.069343 | 0.001760 | 0.001907 | 0.001924 |
| determinant_test.test | RUN | 0.369558 | 0.973636 | 1.189170 | 0.001588 | 0.001910 | 0.003012 |
| div_eq_ms_test.test | RUN | 0.472542 | 0.913859 | 1.335060 | 0.001667 | 0.002683 | 0.002223 |
| div_ms_test.test | RUN | 0.444000 | 1.485609 | 1.683818 | 0.002403 | 0.001641 | 0.002124 |
| eq_mm_test.test | RUN | 0.510427 | 1.724313 | 2.112005 | 0.001714 | 0.002272 | 0.001570 |
| minus_m_test.test | RUN | 0.419893 | 1.435109 | 1.630268 | 0.001572 | 0.001490 | 0.001982 |
| minus_eq_mm_test.test | RUN | 0.397474 | 1.304533 | 1.492671 | 0.001906 | 0.001934 | 0.002090 |
| minus_mm_test.test | RUN | 0.473539 | 1.647669 | 1.749325 | 0.002066 | 0.001934 | 0.002225 |
| mul_eq_mm_test.test | RUN | 0.394364 | 1.179598 | 1.338147 | 0.001682 | 0.001996 | 0.001668 |
| mul_eq_ms_test.test | RUN | 0.404140 | 0.889778 | 1.319354 | 0.001596 | 0.002722 | 0.001464 |
| mul_mm_test.test | RUN | 0.582028 | 2.596493 | 2.120255 | 0.001832 | 0.001799 | 0.001752 |
| mul_ms_test.test | RUN | 0.499040 | 1.501138 | 1.710484 | 0.001747 | 0.002034 | 0.002944 |
| mul_sm_test.test | RUN | 0.491847 | 1.393263 | 1.650791 | 0.001515 | 0.001912 | 0.001752 |
| mul_mv_test.test | RUN | 0.541549 | 1.492680 | 1.675572 | 0.001648 | 0.002119 | 0.002915 |
| inverse_m_test.test | RUN | 0.667998 | 2.754159 | 2.525660 | 0.001827 | 0.002246 | 0.001691 |
| plus_eq_mm_test.test | RUN | 0.415818 | 1.295343 | 1.476293 | 0.001617 | 0.002313 | 0.002196 |
| plus_mm_test.test | RUN | 0.456842 | 1.632215 | 1.700040 | 0.001595 | 0.001692 | 0.001868 |
| scalar_cast_m_test.test | RUN | 0.355937 | 0.761259 | 1.088736 | 0.001763 | 0.003058 | 0.002131 |
| mat_index_test.test | RUN | 0.288803 | 0.705340 | 1.146989 | 0.001804 | 0.002102 | 0.001815 |
| cmp_qq_test.test | RUN | 0.324518 | 0.694899 | 0.945342 | 0.001650 | 0.001767 | 0.002134 |
| conjugate_test.test | RUN | 0.319833 | 0.694041 | 0.960991 | 0.001563 | 0.001920 | 0.001997 |
| normalize_q_test.test | RUN | 0.363243 | 0.765134 | 0.976141 | 0.001590 | 0.001916 | 0.001848 |
| div_eq_qs_test.test | RUN | 0.308659 | 0.661128 | 1.018882 | 0.001602 | 0.002643 | 0.001566 |
| div_qs_test.test | RUN | 0.311258 | 0.699197 | 1.083202 | 0.001659 | 0.002289 | 0.001658 |
| dot_qq_test.test | RUN | 0.324480 | 0.698124 | 1.073770 | 0.001563 | 0.002200 | 0.001532 |
| eq_qq_test.test | RUN | 0.324006 | 0.707578 | 0.991628 | 0.001852 | 0.001670 | 0.001728 |
| inverse_q_test.test | RUN | 0.376095 | 0.960606 | 1.336475 | 0.001734 | 0.002042 | 0.001795 |
| mag_q_test.test | RUN | 0.312278 | 0.693729 | 1.062971 | 0.002572 | 0.002372 | 0.001735 |
| mag_sqr_q_test.test | RUN | 0.305033 | 0.704129 | 1.171888 | 0.001622 | 0.001780 | 0.001656 |
| minus_q_test.test | RUN | 0.310844 | 0.692703 | 1.059853 | 0.001602 | 0.001788 | 0.001730 |
| minus_eq_qq_test.test | RUN | 0.307452 | 0.680244 | 0.951436 | 0.001571 | 0.001874 | 0.001590 |
| minus_qq_test.test | RUN | 0.315356 | 0.692855 | 1.171575 | 0.001922 | 0.001736 | 0.001599 |
| mul_eq_qs_test.test | RUN | 0.312637 | 0.646532 | 1.028500 | 0.001543 | 0.001691 | 0.002044 |
| mul_qs_test.test | RUN | 0.314037 | 0.687981 | 1.099460 | 0.001772 | 0.002258 | 0.001518 |
| mul_qv_test.test | RUN | 0.383994 | 0.929683 | 1.301881 | 0.001716 | 0.002462 | 0.001613 |
| mul_qq_test.test | RUN | 0.379760 | 0.985511 | 1.264206 | 0.001474 | 0.001753 | 0.001428 |
| mul_eq_qq_test.test | RUN | 0.371396 | 0.955299 | 1.213617 | 0.001554 | 0.002568 | 0.001544 |
| plus_eq_qq_test.test | RUN | 0.307536 | 0.686963 | 1.010270 | 0.001786 | 0.001982 | 0.001826 |
| plus_qq_test.test | RUN | 0.313748 | 0.723637 | 0.990701 | 0.001464 | 0.001772 | 0.001409 |
| scalar_cast_q_test.test | RUN | 0.311582 | 0.666890 | 1.024320 | 0.001650 | 0.001716 | 0.001540 |
| slerp_test.test | RUN | 0.339082 | 1.355773 | 1.194500 | 0.025382 | 0.020120 | 0.019865 |
| convert_to_test.test | RUN | 0.451911 | 1.213753 | 1.754882 | 0.001598 | 0.002292 | 0.002010 |
| rot_mat_test.test | RUN | 0.828296 | 5.241912 | 4.969037 | 0.080862 | 0.045445 | 0.047398 |
| rot_quat_test.test | RUN | 0.442855 | 1.131560 | 1.222005 | 0.001922 | 0.001806 | 0.001654 |
| rotx_mat_test.test | RUN | 0.449112 | 1.468328 | 1.618945 | 0.001872 | 0.003054 | 0.001409 |
| rotx_quat_test.test | RUN | 0.376451 | 0.957298 | 1.198837 | 0.001851 | 0.002100 | 0.001991 |
| roty_mat_test.test | RUN | 0.438538 | 1.500346 | 1.590581 | 0.001982 | 0.001990 | 0.001715 |
| roty_quat_test.test | RUN | 0.363780 | 1.017962 | 1.213494 | 0.001930 | 0.001630 | 0.001982 |
| rotz_mat_test.test | RUN | 0.452788 | 1.682877 | 1.641574 | 0.002014 | 0.002452 | 0.001678 |
| rotz_quat_test.test | RUN | 0.372133 | 1.014073 | 1.132055 | 0.001905 | 0.001946 | 0.001696 |
| zero_vec_test.test | RUN | 0.351205 | 0.858906 | 1.037825 | 0.002136 | 0.001732 | 0.001710 |
| zero_mat_test.test | RUN | 0.483143 | 1.364480 | 1.419195 | 0.001516 | 0.001949 | 0.003075 |
| zero_quat_test.test | RUN | 0.311324 | 0.701799 | 0.953352 | 0.001763 | 0.001793 | 0.001894 |
| identity_mat_test.test | RUN | 0.405519 | 1.034481 | 1.237195 | 0.001992 | 0.001590 | 0.001956 |
| identity_quat_test.test | RUN | 0.317878 | 0.668784 | 1.114610 | 0.001761 | 0.001723 | 0.001802 |
| col_test.test | RUN | 0.600438 | 2.146755 | 2.624888 | 0.001791 | 0.001760 | 0.001781 |
| col_mat_test.test | RUN | 0.391889 | 0.977013 | 1.258009 | 0.001528 | 0.002283 | 0.002020 |
| neg_col_test.test | RUN | 0.459096 | 1.477544 | 1.652987 | 0.002118 | 0.001892 | 0.002178 |
| neg_row_test.test | RUN | 0.466329 | 1.357905 | 1.668459 | 0.001895 | 0.001728 | 0.001704 |
| del_col_test.test | RUN | 0.547672 | 1.444197 | 1.792238 | 0.001939 | 0.001637 | 0.001731 |
| del_row_test.test | RUN | 0.466896 | 1.435081 | 1.616621 | 0.001922 | 0.001692 | 0.001389 |
| del_row_col_test.test | RUN | 0.630005 | 2.693804 | 3.189639 | 0.001568 | 0.001896 | 0.001585 |
| swap_cols_test.test | RUN | 0.907975 | 3.600500 | 3.655922 | 0.001593 | 0.001500 | 0.002136 |
| swap_rows_test.test | RUN | 1.019472 | 3.844523 | 3.863051 | 0.001996 | 0.002458 | 0.001853 |
| diag_test.test | RUN | 0.399982 | 0.987451 | 1.441487 | 0.001812 | 0.002101 | 0.001782 |
| diag_mat_test.test | RUN | 0.397435 | 0.973258 | 1.546142 | 0.001511 | 0.002032 | 0.002366 |
| row_test.test | RUN | 0.594135 | 2.116841 | 2.633106 | 0.001715 | 0.001660 | 0.002034 |
| row_mat_test.test | RUN | 0.392536 | 0.928655 | 1.248148 | 0.001525 | 0.001884 | 0.001540 |
| transpose_test.test | RUN | 0.439905 | 1.448502 | 1.701833 | 0.001714 | 0.001392 | 0.002353 |
| translation_test.test | RUN | 0.363245 | 0.887187 | 1.470675 | 0.001670 | 0.001657 | 0.002724 |
| translation_mat_test.test | RUN | 0.416572 | 0.954827 | 1.476831 | 0.001662 | 0.001537 | 0.001780 |
| swizzle_const_fail.test | COMPILE_FAIL | 0.131751 | 0.311698 | 0.421149 | — | — | — |
| swizzle2_test2.test | RUN | 0.167967 | 0.390237 | 0.512415 | 0.001555 | 0.001766 | 0.002755 |
| swizzle2_test3.test | RUN | 0.176676 | 0.402371 | 0.525173 | 0.001600 | 0.001767 | 0.001850 |
| swizzle2_test4.test | RUN | 0.172754 | 0.397048 | 0.533474 | 0.001690 | 0.001981 | 0.001808 |
| swizzle2_test.test | RUN | 0.515346 | 1.092898 | 1.747969 | 0.001794 | 0.001550 | 0.001748 |
| swizzle3_test2.test | RUN | 0.186835 | 0.417644 | 0.576343 | 0.001607 | 0.001982 | 0.002404 |
| swizzle3_test3.test | RUN | 0.186390 | 0.418725 | 0.612686 | 0.001652 | 0.001459 | 0.001880 |
| swizzle3_test4.test | RUN | 0.215525 | 0.416663 | 0.583131 | 0.001474 | 0.001722 | 0.001918 |
| swizzle3_test.test | RUN | 0.502159 | 1.146901 | 1.775334 | 0.001624 | 0.002334 | 0.001687 |
| swizzle4_test2.test | RUN | 0.305963 | 0.615969 | 1.006648 | 0.001803 | 0.001684 | 0.002202 |
| swizzle4_test3.test | RUN | 0.317029 | 0.602075 | 1.011174 | 0.001633 | 0.002273 | 0.002054 |
| swizzle4_test4.test | RUN | 0.304967 | 0.594757 | 0.982639 | 0.001624 | 0.003766 | 0.002827 |
| swizzle4_test.test | RUN | 0.517787 | 1.216396 | 1.882429 | 0.002034 | 0.002081 | 0.002280 |
| deduce_scalar_test.test | COMPILE | 0.026977 | 0.056386 | 0.089504 | — | — | — |
| deduce_scalar_mq_test.test | RUN | 0.229859 | 0.460156 | 0.650800 | 0.001736 | 0.002028 | 0.002808 |
| deduce_scalar_mv_test.test | RUN | 0.237541 | 0.523831 | 0.745922 | 0.001710 | 0.002018 | 0.002034 |
| deduce_scalar_fail.test | COMPILE_FAIL | 0.022760 | 0.036259 | 0.072473 | — | — | — |
| deduce_mat_test.test | COMPILE | 0.024122 | 0.050432 | 0.087686 | — | — | — |
| deduce_quat_test.test | COMPILE | 0.024281 | 0.047051 | 0.084446 | — | — | — |
| deduce_vec_test.test | COMPILE | 0.024803 | 0.051158 | 0.084614 | — | — | — |
| interop_test.test | RUN | 0.244723 | 0.657119 | 1.053894 | 0.000713 | 0.000828 | 0.001998 |
| transform_test.test | RUN | 0.535056 | 1.102966 | 1.904343 | 0.002642 | 0.002068 | 0.002545 |
| projection_test.test | RUN | 0.347361 | 0.820244 | 1.143392 | 0.002014 | 0.002050 | 0.002130 |
| scalar_traits_test.test | COMPILE | 0.025208 | 0.051278 | 0.084151 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.853× | 0.857×, 0.850× | 1.015× |
| clang | 0.860× | 0.852×, 0.868× | 1.013× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1353 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| access_m_test.test | 8 | 0.001603s | 0.001648s | 0.001584s | byte equality observed |
| access_q_test.test | 8 | 0.001614s | 0.001579s | 0.001653s | byte equality observed |
| access_v_test.test | 8 | 0.001643s | 0.001621s | 0.001647s | byte equality observed |
| assign_test.test | 9 | 0.001616s | 0.001681s | 0.001616s | byte equality observed |
| cmp_mm_test.test | 7 | 0.001649s | 0.001642s | 0.001623s | byte equality observed |
| cmp_qq_test.test | 7 | 0.001572s | 0.001596s | 0.001584s | byte equality observed |
| cmp_vv_test.test | 7 | 0.001675s | 0.001615s | 0.001606s | byte equality observed |
| col_mat_test.test | 7 | 0.001626s | 0.001597s | 0.001582s | byte equality observed |
| col_test.test | 6 | 0.001657s | 0.001645s | 0.001623s | byte equality observed |
| conjugate_test.test | 7 | 0.001663s | 0.001567s | 0.001638s | byte equality observed |
| convert_to_test.test | 7 | 0.001654s | 0.001575s | 0.001681s | byte equality observed |
| cross_test.test | 7 | 0.001578s | 0.001654s | 0.001616s | byte equality observed |
| deduce_scalar_mq_test.test | 8 | 0.001601s | 0.001567s | 0.001589s | byte equality observed |
| deduce_scalar_mv_test.test | 6 | 0.001588s | 0.001699s | 0.001583s | byte equality observed |
| del_col_test.test | 6 | 0.001618s | 0.001621s | 0.001609s | byte equality observed |
| del_row_col_test.test | 8 | 0.001672s | 0.001636s | 0.001557s | byte equality observed |
| del_row_test.test | 6 | 0.001632s | 0.001633s | 0.001594s | byte equality observed |
| determinant_test.test | 8 | 0.001620s | 0.001622s | 0.001619s | byte equality observed |
| diag_mat_test.test | 8 | 0.001596s | 0.001551s | 0.001537s | byte equality observed |
| diag_test.test | 8 | 0.001620s | 0.001602s | 0.001606s | byte equality observed |
| div_eq_ms_test.test | 7 | 0.001629s | 0.001618s | 0.001641s | byte equality observed |
| div_eq_qs_test.test | 6 | 0.001586s | 0.001721s | 0.001715s | byte equality observed |
| div_eq_vs_test.test | 7 | 0.001632s | 0.001606s | 0.001616s | byte equality observed |
| div_ms_test.test | 7 | 0.001581s | 0.001621s | 0.001590s | byte equality observed |
| div_qs_test.test | 7 | 0.001610s | 0.001622s | 0.001663s | byte equality observed |
| div_vs_test.test | 8 | 0.001591s | 0.001580s | 0.001583s | byte equality observed |
| dot_qq_test.test | 7 | 0.001582s | 0.001606s | 0.001726s | byte equality observed |
| dot_vv_test.test | 6 | 0.001633s | 0.001684s | 0.001578s | byte equality observed |
| eq_mm_test.test | 7 | 0.001682s | 0.001670s | 0.001646s | byte equality observed |
| eq_qq_test.test | 7 | 0.001617s | 0.001748s | 0.001634s | byte equality observed |
| eq_vv_test.test | 8 | 0.001682s | 0.001627s | 0.001607s | byte equality observed |
| identity_mat_test.test | 6 | 0.001592s | 0.001687s | 0.001649s | byte equality observed |
| identity_quat_test.test | 7 | 0.001618s | 0.001707s | 0.001655s | byte equality observed |
| interop_test.test | 8 | 0.000804s | 0.000828s | 0.001553s | byte equality observed |
| inverse_m_test.test | 6 | 0.001705s | 0.001762s | 0.001708s | byte equality observed |
| inverse_q_test.test | 6 | 0.001683s | 0.001618s | 0.001665s | byte equality observed |
| mag_q_test.test | 6 | 0.001654s | 0.001659s | 0.001620s | byte equality observed |
| mag_sqr_q_test.test | 8 | 0.001594s | 0.001733s | 0.001604s | byte equality observed |
| mag_sqr_v_test.test | 6 | 0.001594s | 0.001816s | 0.001636s | byte equality observed |
| mag_v_test.test | 7 | 0.001618s | 0.001675s | 0.001603s | byte equality observed |
| mat_index_test.test | 6 | 0.001673s | 0.001683s | 0.001630s | byte equality observed |
| mat_traits_array_test.test | 6 | 0.001563s | 0.001672s | 0.001659s | byte equality observed |
| mat_traits_std_array_test.test | 6 | 0.001638s | 0.001668s | 0.001564s | byte equality observed |
| math_test.test | 6 | 0.002336s | 0.002200s | 0.002010s | byte equality observed |
| minus_eq_mm_test.test | 9 | 0.001648s | 0.001738s | 0.001650s | byte equality observed |
| minus_eq_qq_test.test | 8 | 0.001596s | 0.001716s | 0.001588s | byte equality observed |
| minus_eq_vv_test.test | 7 | 0.001585s | 0.001680s | 0.001603s | byte equality observed |
| minus_m_test.test | 7 | 0.001624s | 0.001611s | 0.001593s | byte equality observed |
| minus_mm_test.test | 8 | 0.001642s | 0.001594s | 0.001605s | byte equality observed |
| minus_q_test.test | 7 | 0.001628s | 0.001655s | 0.001638s | byte equality observed |
| minus_qq_test.test | 9 | 0.001659s | 0.001641s | 0.001657s | byte equality observed |
| minus_v_test.test | 8 | 0.001591s | 0.001613s | 0.001613s | byte equality observed |
| minus_vv_test.test | 5 | 0.001567s | 0.001600s | 0.001535s | byte equality observed |
| mul_eq_mm_test.test | 7 | 0.001596s | 0.001677s | 0.001581s | byte equality observed |
| mul_eq_ms_test.test | 7 | 0.001589s | 0.001651s | 0.001638s | byte equality observed |
| mul_eq_qq_test.test | 8 | 0.001663s | 0.001697s | 0.001593s | byte equality observed |
| mul_eq_qs_test.test | 9 | 0.001613s | 0.001632s | 0.001608s | byte equality observed |
| mul_eq_vs_test.test | 7 | 0.001600s | 0.001604s | 0.001668s | byte equality observed |
| mul_mm_test.test | 7 | 0.001657s | 0.001629s | 0.001596s | byte equality observed |
| mul_ms_test.test | 7 | 0.001654s | 0.001608s | 0.001598s | byte equality observed |
| mul_mv_test.test | 7 | 0.001643s | 0.001597s | 0.001698s | byte equality observed |
| mul_qq_test.test | 7 | 0.001677s | 0.001646s | 0.001683s | byte equality observed |
| mul_qs_test.test | 7 | 0.001696s | 0.001638s | 0.001592s | byte equality observed |
| mul_qv_test.test | 8 | 0.001680s | 0.001682s | 0.001698s | byte equality observed |
| mul_sm_test.test | 7 | 0.001698s | 0.001614s | 0.001626s | byte equality observed |
| mul_sv_test.test | 8 | 0.001582s | 0.001626s | 0.001578s | byte equality observed |
| mul_vm_test.test | 8 | 0.001634s | 0.001608s | 0.001611s | byte equality observed |
| mul_vs_test.test | 8 | 0.001648s | 0.001588s | 0.001568s | byte equality observed |
| neg_col_test.test | 9 | 0.001626s | 0.001595s | 0.001689s | byte equality observed |
| neg_row_test.test | 7 | 0.001673s | 0.001609s | 0.001626s | byte equality observed |
| normalize_q_test.test | 7 | 0.001568s | 0.001637s | 0.001655s | byte equality observed |
| normalize_v_test.test | 9 | 0.001651s | 0.001613s | 0.001589s | byte equality observed |
| plus_eq_mm_test.test | 7 | 0.001716s | 0.001724s | 0.001581s | byte equality observed |
| plus_eq_qq_test.test | 6 | 0.001603s | 0.001595s | 0.001636s | byte equality observed |
| plus_eq_vv_test.test | 7 | 0.001652s | 0.001609s | 0.001563s | byte equality observed |
| plus_mm_test.test | 8 | 0.001649s | 0.001760s | 0.001623s | byte equality observed |
| plus_qq_test.test | 8 | 0.001612s | 0.001615s | 0.001637s | byte equality observed |
| plus_vv_test.test | 7 | 0.001684s | 0.001610s | 0.001630s | byte equality observed |
| projection_test.test | 8 | 0.001653s | 0.001653s | 0.001693s | byte equality observed |
| quat_traits_array_test.test | 7 | 0.001670s | 0.001607s | 0.001616s | byte equality observed |
| quat_traits_std_array_test.test | 7 | 0.001617s | 0.001687s | 0.001625s | byte equality observed |
| rot_mat_test.test | 1 | 0.088063s | 0.047022s | 0.052699s | byte equality observed |
| rot_quat_test.test | 7 | 0.001703s | 0.001690s | 0.001651s | byte equality observed |
| rotx_mat_test.test | 8 | 0.001733s | 0.001745s | 0.001640s | byte equality observed |
| rotx_quat_test.test | 7 | 0.001718s | 0.001754s | 0.001713s | byte equality observed |
| roty_mat_test.test | 8 | 0.001731s | 0.001644s | 0.001680s | byte equality observed |
| roty_quat_test.test | 7 | 0.001735s | 0.001656s | 0.001679s | byte equality observed |
| rotz_mat_test.test | 8 | 0.001686s | 0.001674s | 0.001730s | byte equality observed |
| rotz_quat_test.test | 8 | 0.001745s | 0.001719s | 0.001663s | byte equality observed |
| row_mat_test.test | 9 | 0.001675s | 0.001653s | 0.001604s | byte equality observed |
| row_test.test | 7 | 0.001648s | 0.001679s | 0.001728s | byte equality observed |
| scalar_cast_m_test.test | 8 | 0.001615s | 0.001666s | 0.001633s | byte equality observed |
| scalar_cast_q_test.test | 8 | 0.001642s | 0.001587s | 0.001605s | byte equality observed |
| scalar_cast_v_test.test | 8 | 0.001652s | 0.001610s | 0.001603s | byte equality observed |
| slerp_test.test | 1 | 0.025226s | 0.019432s | 0.019639s | byte equality observed |
| swap_cols_test.test | 8 | 0.001662s | 0.001773s | 0.001624s | byte equality observed |
| swap_rows_test.test | 8 | 0.001659s | 0.001651s | 0.001629s | byte equality observed |
| swizzle2_test.test | 8 | 0.001699s | 0.001608s | 0.001639s | byte equality observed |
| swizzle2_test2.test | 9 | 0.001626s | 0.001637s | 0.001672s | byte equality observed |
| swizzle2_test3.test | 8 | 0.001647s | 0.001634s | 0.001648s | byte equality observed |
| swizzle2_test4.test | 7 | 0.001625s | 0.001643s | 0.001609s | byte equality observed |
| swizzle3_test.test | 8 | 0.001615s | 0.001566s | 0.001600s | byte equality observed |
| swizzle3_test2.test | 7 | 0.001601s | 0.001567s | 0.001600s | byte equality observed |
| swizzle3_test3.test | 7 | 0.001672s | 0.001639s | 0.001619s | byte equality observed |
| swizzle3_test4.test | 7 | 0.001602s | 0.001687s | 0.001683s | byte equality observed |
| swizzle4_test.test | 7 | 0.001665s | 0.001683s | 0.001581s | byte equality observed |
| swizzle4_test2.test | 7 | 0.001558s | 0.001605s | 0.001629s | byte equality observed |
| swizzle4_test3.test | 8 | 0.001598s | 0.001617s | 0.001636s | byte equality observed |
| swizzle4_test4.test | 6 | 0.001624s | 0.001694s | 0.001617s | byte equality observed |
| to_string_test.test | 7 | 0.001649s | 0.001687s | 0.001598s | byte equality observed |
| transform_test.test | 8 | 0.001662s | 0.001614s | 0.001573s | byte equality observed |
| translation_mat_test.test | 7 | 0.001642s | 0.001703s | 0.001577s | byte equality observed |
| translation_test.test | 9 | 0.001601s | 0.001641s | 0.001529s | byte equality observed |
| transpose_test.test | 8 | 0.001664s | 0.001635s | 0.001582s | byte equality observed |
| vec_index_test.test | 8 | 0.001591s | 0.001590s | 0.001697s | byte equality observed |
| vec_register_test.test | 7 | 0.001634s | 0.001721s | 0.001590s | byte equality observed |
| vec_traits_array_test.test | 9 | 0.001578s | 0.001620s | 0.001684s | byte equality observed |
| vec_traits_gnuc_test.test | 8 | 0.001671s | 0.001644s | 0.001584s | byte equality observed |
| vec_traits_std_array_test.test | 7 | 0.001608s | 0.001663s | 0.001580s | byte equality observed |
| zero_mat_test.test | 8 | 0.001694s | 0.001806s | 0.001652s | byte equality observed |
| zero_quat_test.test | 8 | 0.001681s | 0.001659s | 0.001596s | byte equality observed |
| zero_vec_test.test | 7 | 0.001639s | 0.001680s | 0.001612s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
