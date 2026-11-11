# libs/qvm/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| math_test.test | RUN | 0.388709 | 0.843481 | 0.919429 | 0.002456 | 0.002269 | 0.002070 |
| mat_traits_array_test.test | RUN | 0.260635 | 0.847055 | 0.724345 | 0.001422 | 0.001624 | 0.001361 |
| mat_traits_std_array_test.test | RUN | 0.255827 | 0.654159 | 0.742206 | 0.001660 | 0.001548 | 0.001383 |
| vec_traits_array_test.test | RUN | 0.224571 | 0.647485 | 0.615264 | 0.001620 | 0.001714 | 0.001436 |
| vec_traits_gnuc_test.test | RUN | 0.416916 | 0.806855 | 1.238253 | 0.001581 | 0.001708 | 0.001627 |
| vec_traits_std_array_test.test | RUN | 0.226232 | 0.499703 | 0.609053 | 0.001472 | 0.001741 | 0.001507 |
| quat_traits_array_test.test | RUN | 0.225190 | 0.684975 | 0.593783 | 0.001668 | 0.001510 | 0.001480 |
| quat_traits_std_array_test.test | RUN | 0.226126 | 0.514272 | 0.603090 | 0.001564 | 0.001569 | 0.001612 |
| access_m_test.test | RUN | 0.353446 | 1.275856 | 1.411549 | 0.001716 | 0.001724 | 0.001619 |
| access_m_fail1.test | COMPILE_FAIL | 0.035117 | 0.054427 | 0.090952 | — | — | — |
| access_m_fail2.test | COMPILE_FAIL | 0.034675 | 0.053715 | 0.092945 | — | — | — |
| access_q_test.test | RUN | 0.303613 | 0.618896 | 0.803719 | 0.001675 | 0.001666 | 0.001658 |
| access_q_fail.test | COMPILE_FAIL | 0.027271 | 0.041732 | 0.071233 | — | — | — |
| access_v_test.test | RUN | 0.299027 | 0.653601 | 0.808716 | 0.001716 | 0.001753 | 0.001692 |
| access_v_fail1.test | COMPILE_FAIL | 0.023854 | 0.038643 | 0.064851 | — | — | — |
| access_v_fail2.test | COMPILE_FAIL | 0.023765 | 0.038107 | 0.064135 | — | — | — |
| assign_test.test | RUN | 0.449371 | 1.048273 | 1.454655 | 0.001486 | 0.001711 | 0.001657 |
| to_string_test.test | RUN | 0.528891 | 1.714479 | 8.560914 | 0.001568 | 0.001662 | 0.001523 |
| cmp_vv_test.test | RUN | 0.400138 | 0.815730 | 0.980598 | 0.001715 | 0.001690 | 0.001680 |
| cross_test.test | RUN | 0.362845 | 0.758233 | 0.977631 | 0.001507 | 0.001728 | 0.001631 |
| div_eq_vs_test.test | RUN | 0.344860 | 0.696896 | 0.966895 | 0.001840 | 0.001526 | 0.001691 |
| div_vs_test.test | RUN | 0.368323 | 0.865433 | 1.018942 | 0.001750 | 0.001742 | 0.001539 |
| dot_vv_test.test | RUN | 0.345332 | 0.784297 | 0.949711 | 0.001749 | 0.001679 | 0.001385 |
| eq_vv_test.test | RUN | 0.380837 | 0.878389 | 1.045048 | 0.001706 | 0.001802 | 0.001687 |
| mag_v_test.test | RUN | 0.333690 | 0.731995 | 0.916077 | 0.001707 | 0.001478 | 0.001712 |
| mag_sqr_v_test.test | RUN | 0.342168 | 0.704358 | 0.911741 | 0.001801 | 0.001676 | 0.001442 |
| minus_v_test.test | RUN | 0.354834 | 0.832263 | 0.985768 | 0.001552 | 0.001688 | 0.001547 |
| minus_eq_vv_test.test | RUN | 0.348412 | 0.808567 | 0.960643 | 0.001476 | 0.001640 | 0.001488 |
| minus_vv_test.test | RUN | 0.357081 | 0.884460 | 1.012270 | 0.001474 | 0.001567 | 0.001426 |
| mul_eq_vs_test.test | RUN | 0.339511 | 0.695511 | 0.948684 | 0.001686 | 0.001740 | 0.001628 |
| mul_vs_test.test | RUN | 0.359336 | 0.870651 | 1.015130 | 0.001467 | 0.001761 | 0.001628 |
| mul_sv_test.test | RUN | 0.359280 | 0.871637 | 1.023766 | 0.001458 | 0.001706 | 0.001435 |
| mul_vm_test.test | RUN | 0.500577 | 1.571649 | 1.592293 | 0.001460 | 0.001690 | 0.001680 |
| normalize_v_test.test | RUN | 0.365083 | 1.056536 | 0.980811 | 0.001525 | 0.001699 | 0.001608 |
| plus_eq_vv_test.test | RUN | 0.342594 | 0.812238 | 0.961436 | 0.001576 | 0.001594 | 0.001436 |
| plus_vv_test.test | RUN | 0.356246 | 0.873256 | 0.974431 | 0.001488 | 0.001704 | 0.001457 |
| scalar_cast_v_test.test | RUN | 0.328801 | 0.646611 | 0.869521 | 0.001546 | 0.001714 | 0.001405 |
| vec_index_test.test | RUN | 0.297746 | 0.667777 | 0.785840 | 0.001707 | 0.001635 | 0.001624 |
| vec_register_test.test | RUN | 0.189212 | 0.597088 | 0.583056 | 0.001794 | 0.001708 | 0.001631 |
| cmp_mm_test.test | RUN | 0.557277 | 1.927679 | 2.044970 | 0.001708 | 0.001706 | 0.001620 |
| determinant_test.test | RUN | 0.388410 | 0.968285 | 1.114409 | 0.001663 | 0.001639 | 0.001681 |
| div_eq_ms_test.test | RUN | 0.407378 | 0.914844 | 1.268918 | 0.001652 | 0.001674 | 0.001620 |
| div_ms_test.test | RUN | 0.456782 | 1.433853 | 1.555069 | 0.001502 | 0.001540 | 0.001683 |
| eq_mm_test.test | RUN | 0.524150 | 1.786193 | 1.991125 | 0.001734 | 0.001657 | 0.001679 |
| minus_m_test.test | RUN | 0.449185 | 1.410403 | 1.468677 | 0.001673 | 0.001754 | 0.001618 |
| minus_eq_mm_test.test | RUN | 0.428223 | 1.310229 | 1.382657 | 0.001903 | 0.001704 | 0.001679 |
| minus_mm_test.test | RUN | 0.474273 | 1.691505 | 1.632887 | 0.001302 | 0.001690 | 0.001610 |
| mul_eq_mm_test.test | RUN | 0.414248 | 1.238796 | 1.229250 | 0.001715 | 0.001709 | 0.001635 |
| mul_eq_ms_test.test | RUN | 0.423145 | 0.902698 | 1.281445 | 0.001762 | 0.001791 | 0.001660 |
| mul_mm_test.test | RUN | 0.557897 | 2.745924 | 2.070157 | 0.001501 | 0.001699 | 0.001651 |
| mul_ms_test.test | RUN | 0.456150 | 1.443233 | 1.613319 | 0.001713 | 0.001688 | 0.001526 |
| mul_sm_test.test | RUN | 0.470689 | 1.438125 | 1.582251 | 0.001526 | 0.001657 | 0.001594 |
| mul_mv_test.test | RUN | 0.531949 | 1.565223 | 1.636283 | 0.001694 | 0.001660 | 0.001564 |
| inverse_m_test.test | RUN | 0.636485 | 2.819807 | 2.244273 | 0.001881 | 0.001831 | 0.001584 |
| plus_eq_mm_test.test | RUN | 0.427341 | 1.309986 | 1.370317 | 0.001751 | 0.001620 | 0.001645 |
| plus_mm_test.test | RUN | 0.477833 | 1.695443 | 1.659803 | 0.001715 | 0.001702 | 0.001595 |
| scalar_cast_m_test.test | RUN | 0.380164 | 0.755511 | 1.008932 | 0.001573 | 0.001620 | 0.001675 |
| mat_index_test.test | RUN | 0.308297 | 0.714800 | 0.884283 | 0.001632 | 0.001796 | 0.001658 |
| cmp_qq_test.test | RUN | 0.328846 | 0.678670 | 0.856055 | 0.001646 | 0.001736 | 0.001626 |
| conjugate_test.test | RUN | 0.332952 | 0.688569 | 0.893666 | 0.001746 | 0.001765 | 0.001468 |
| normalize_q_test.test | RUN | 0.342794 | 0.771725 | 0.930053 | 0.001708 | 0.001511 | 0.001501 |
| div_eq_qs_test.test | RUN | 0.329012 | 0.667556 | 0.868921 | 0.001528 | 0.001580 | 0.001646 |
| div_qs_test.test | RUN | 0.334123 | 0.697415 | 0.896111 | 0.001711 | 0.001692 | 0.001665 |
| dot_qq_test.test | RUN | 0.335256 | 0.672009 | 0.892682 | 0.001692 | 0.001721 | 0.001591 |
| eq_qq_test.test | RUN | 0.340555 | 0.709781 | 0.899525 | 0.001518 | 0.001565 | 0.001590 |
| inverse_q_test.test | RUN | 0.403733 | 0.904476 | 1.117305 | 0.001683 | 0.001678 | 0.001578 |
| mag_q_test.test | RUN | 0.334301 | 0.683906 | 0.880882 | 0.001716 | 0.001859 | 0.001638 |
| mag_sqr_q_test.test | RUN | 0.332523 | 0.681143 | 0.892926 | 0.001576 | 0.001765 | 0.001546 |
| minus_q_test.test | RUN | 0.337882 | 0.683744 | 0.874031 | 0.001143 | 0.001608 | 0.001646 |
| minus_eq_qq_test.test | RUN | 0.332980 | 0.685069 | 0.874976 | 0.001649 | 0.001694 | 0.001609 |
| minus_qq_test.test | RUN | 0.324952 | 0.701761 | 0.891243 | 0.001736 | 0.001704 | 0.001646 |
| mul_eq_qs_test.test | RUN | 0.323557 | 0.646873 | 0.878283 | 0.001702 | 0.001688 | 0.001496 |
| mul_qs_test.test | RUN | 0.329568 | 0.692742 | 0.895918 | 0.001550 | 0.001723 | 0.001658 |
| mul_qv_test.test | RUN | 0.412564 | 0.940706 | 1.138238 | 0.001692 | 0.001775 | 0.001629 |
| mul_qq_test.test | RUN | 0.392797 | 0.954037 | 1.117471 | 0.001744 | 0.001462 | 0.001673 |
| mul_eq_qq_test.test | RUN | 0.396040 | 0.953844 | 1.113906 | 0.001548 | 0.001832 | 0.001434 |
| plus_eq_qq_test.test | RUN | 0.329246 | 0.692500 | 0.890346 | 0.001468 | 0.001450 | 0.001602 |
| plus_qq_test.test | RUN | 0.331975 | 0.699792 | 0.881502 | 0.001719 | 0.001560 | 0.001646 |
| scalar_cast_q_test.test | RUN | 0.324239 | 0.654513 | 0.870673 | 0.001552 | 0.001426 | 0.001580 |
| slerp_test.test | RUN | 0.358045 | 1.322187 | 1.063972 | 0.025588 | 0.020266 | 0.021878 |
| convert_to_test.test | RUN | 0.479924 | 1.223119 | 1.630099 | 0.001666 | 0.001849 | 0.001614 |
| rot_mat_test.test | RUN | 0.829585 | 5.631950 | 5.168258 | 0.090973 | 0.050010 | 0.053347 |
| rot_quat_test.test | RUN | 0.408573 | 1.118591 | 1.145762 | 0.001776 | 0.001626 | 0.001673 |
| rotx_mat_test.test | RUN | 0.460542 | 1.498617 | 1.515848 | 0.001575 | 0.001806 | 0.001584 |
| rotx_quat_test.test | RUN | 0.394228 | 0.974323 | 1.098520 | 0.001504 | 0.001758 | 0.001651 |
| roty_mat_test.test | RUN | 0.459721 | 1.500045 | 1.517088 | 0.001754 | 0.001784 | 0.001669 |
| roty_quat_test.test | RUN | 0.402167 | 0.971594 | 1.074815 | 0.001867 | 0.001734 | 0.001667 |
| rotz_mat_test.test | RUN | 0.484984 | 1.717459 | 1.528306 | 0.001512 | 0.001672 | 0.001628 |
| rotz_quat_test.test | RUN | 0.400958 | 0.973727 | 1.089337 | 0.001494 | 0.001772 | 0.001594 |
| zero_vec_test.test | RUN | 0.350580 | 0.852170 | 0.947064 | 0.001608 | 0.001736 | 0.001634 |
| zero_mat_test.test | RUN | 0.484631 | 1.391503 | 1.318304 | 0.001752 | 0.001754 | 0.001655 |
| zero_quat_test.test | RUN | 0.333992 | 0.696070 | 0.835865 | 0.001635 | 0.001694 | 0.001676 |
| identity_mat_test.test | RUN | 0.417611 | 1.029232 | 1.129707 | 0.001424 | 0.001703 | 0.001555 |
| identity_quat_test.test | RUN | 0.328589 | 0.706521 | 0.871969 | 0.001682 | 0.001739 | 0.001575 |
| col_test.test | RUN | 0.617405 | 2.163709 | 2.506251 | 0.001555 | 0.001696 | 0.001585 |
| col_mat_test.test | RUN | 0.420416 | 0.958280 | 1.139358 | 0.001685 | 0.001794 | 0.001624 |
| neg_col_test.test | RUN | 0.476139 | 1.368963 | 1.527822 | 0.001783 | 0.001735 | 0.001613 |
| neg_row_test.test | RUN | 0.492607 | 1.373296 | 1.516326 | 0.001556 | 0.001775 | 0.001640 |
| del_col_test.test | RUN | 0.488188 | 1.515659 | 1.549086 | 0.001685 | 0.001817 | 0.001613 |
| del_row_test.test | RUN | 0.495444 | 1.485668 | 1.567275 | 0.001740 | 0.001889 | 0.001610 |
| del_row_col_test.test | RUN | 0.668128 | 2.816202 | 3.019535 | 0.001736 | 0.001779 | 0.001624 |
| swap_cols_test.test | RUN | 0.934496 | 3.829348 | 3.641457 | 0.001814 | 0.001734 | 0.001665 |
| swap_rows_test.test | RUN | 1.036487 | 4.015768 | 3.657420 | 0.001759 | 0.001712 | 0.001491 |
| diag_test.test | RUN | 0.430444 | 0.989998 | 1.326233 | 0.001704 | 0.001648 | 0.001516 |
| diag_mat_test.test | RUN | 0.442523 | 0.971961 | 1.223067 | 0.001696 | 0.001665 | 0.001614 |
| row_test.test | RUN | 0.625088 | 2.120619 | 2.504648 | 0.001574 | 0.001742 | 0.001520 |
| row_mat_test.test | RUN | 0.411904 | 0.928346 | 1.169023 | 0.001660 | 0.001755 | 0.001542 |
| transpose_test.test | RUN | 0.467736 | 1.439482 | 1.571918 | 0.001838 | 0.001524 | 0.001658 |
| translation_test.test | RUN | 0.381234 | 0.894509 | 1.196729 | 0.001694 | 0.001724 | 0.001623 |
| translation_mat_test.test | RUN | 0.430593 | 0.927407 | 1.297040 | 0.001670 | 0.001691 | 0.001561 |
| swizzle_const_fail.test | COMPILE_FAIL | 0.141508 | 0.264593 | 0.357004 | — | — | — |
| swizzle2_test2.test | RUN | 0.176584 | 0.387460 | 0.486617 | 0.001736 | 0.001715 | 0.001633 |
| swizzle2_test3.test | RUN | 0.176127 | 0.386099 | 0.470551 | 0.001596 | 0.001416 | 0.001532 |
| swizzle2_test4.test | RUN | 0.175685 | 0.393027 | 0.476522 | 0.001735 | 0.001756 | 0.001595 |
| swizzle2_test.test | RUN | 0.548958 | 1.100509 | 1.582690 | 0.001664 | 0.001676 | 0.001656 |
| swizzle3_test2.test | RUN | 0.197161 | 0.417812 | 0.522451 | 0.001442 | 0.001732 | 0.001626 |
| swizzle3_test3.test | RUN | 0.195258 | 0.419456 | 0.538430 | 0.001647 | 0.001422 | 0.001603 |
| swizzle3_test4.test | RUN | 0.190020 | 0.420210 | 0.527275 | 0.001560 | 0.001627 | 0.001560 |
| swizzle3_test.test | RUN | 0.545746 | 1.120780 | 1.588833 | 0.001715 | 0.001498 | 0.001572 |
| swizzle4_test2.test | RUN | 0.331434 | 0.619133 | 0.917745 | 0.001704 | 0.001567 | 0.001603 |
| swizzle4_test3.test | RUN | 0.327516 | 0.609928 | 0.923672 | 0.001488 | 0.001748 | 0.001656 |
| swizzle4_test4.test | RUN | 0.328467 | 0.613456 | 0.916364 | 0.001460 | 0.001635 | 0.001682 |
| swizzle4_test.test | RUN | 0.549175 | 1.225281 | 1.659544 | 0.001698 | 0.001597 | 0.001619 |
| deduce_scalar_test.test | COMPILE | 0.028263 | 0.058057 | 0.080830 | — | — | — |
| deduce_scalar_mq_test.test | RUN | 0.209201 | 0.448917 | 0.577039 | 0.001540 | 0.001808 | 0.001641 |
| deduce_scalar_mv_test.test | RUN | 0.248764 | 0.522022 | 0.701562 | 0.001479 | 0.001684 | 0.001626 |
| deduce_scalar_fail.test | COMPILE_FAIL | 0.023443 | 0.036527 | 0.059242 | — | — | — |
| deduce_mat_test.test | COMPILE | 0.025489 | 0.049335 | 0.069176 | — | — | — |
| deduce_quat_test.test | COMPILE | 0.024808 | 0.047664 | 0.068296 | — | — | — |
| deduce_vec_test.test | COMPILE | 0.025291 | 0.048595 | 0.067439 | — | — | — |
| interop_test.test | RUN | 0.259023 | 0.670421 | 1.030517 | 0.000803 | 0.000809 | 0.001069 |
| transform_test.test | RUN | 0.419899 | 0.987349 | 1.435710 | 0.001225 | 0.001228 | 0.001073 |
| projection_test.test | RUN | 0.329888 | 0.651344 | 0.826495 | 0.001151 | 0.001197 | 0.001117 |
| scalar_traits_test.test | COMPILE | 0.026727 | 0.051073 | 0.071766 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.836×† | 0.857×, 0.814× | 0.985× |
| clang | 0.860× | 0.874×, 0.846× | 1.008× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| access_m_test.test | 10 | 0.000988s | 0.000999s | 0.000958s | byte equality observed |
| access_q_test.test | 10 | 0.000979s | 0.000993s | 0.001014s | byte equality observed |
| access_v_test.test | 10 | 0.000990s | 0.001010s | 0.001006s | byte equality observed |
| assign_test.test | 10 | 0.001002s | 0.001006s | 0.001018s | byte equality observed |
| cmp_mm_test.test | 10 | 0.001008s | 0.000981s | 0.001007s | byte equality observed |
| cmp_qq_test.test | 10 | 0.001006s | 0.001011s | 0.001022s | byte equality observed |
| cmp_vv_test.test | 10 | 0.000979s | 0.000966s | 0.001008s | byte equality observed |
| col_mat_test.test | 10 | 0.001033s | 0.000993s | 0.000988s | byte equality observed |
| col_test.test | 10 | 0.000991s | 0.000980s | 0.001027s | byte equality observed |
| conjugate_test.test | 10 | 0.001000s | 0.000992s | 0.000978s | byte equality observed |
| convert_to_test.test | 10 | 0.001023s | 0.000990s | 0.000998s | byte equality observed |
| cross_test.test | 10 | 0.000996s | 0.000975s | 0.000989s | byte equality observed |
| deduce_scalar_mq_test.test | 10 | 0.000993s | 0.001017s | 0.001010s | byte equality observed |
| deduce_scalar_mv_test.test | 10 | 0.001010s | 0.000989s | 0.001005s | byte equality observed |
| del_col_test.test | 10 | 0.001027s | 0.000964s | 0.000983s | byte equality observed |
| del_row_col_test.test | 10 | 0.001011s | 0.001025s | 0.001021s | byte equality observed |
| del_row_test.test | 10 | 0.000985s | 0.000992s | 0.001013s | byte equality observed |
| determinant_test.test | 10 | 0.001010s | 0.001004s | 0.001017s | byte equality observed |
| diag_mat_test.test | 10 | 0.000985s | 0.000994s | 0.000996s | byte equality observed |
| diag_test.test | 10 | 0.000987s | 0.000984s | 0.000968s | byte equality observed |
| div_eq_ms_test.test | 10 | 0.000965s | 0.001023s | 0.001003s | byte equality observed |
| div_eq_qs_test.test | 10 | 0.000963s | 0.001031s | 0.000998s | byte equality observed |
| div_eq_vs_test.test | 10 | 0.001025s | 0.001016s | 0.001004s | byte equality observed |
| div_ms_test.test | 10 | 0.000992s | 0.000982s | 0.000991s | byte equality observed |
| div_qs_test.test | 10 | 0.000990s | 0.000998s | 0.001018s | byte equality observed |
| div_vs_test.test | 10 | 0.000996s | 0.000984s | 0.001015s | byte equality observed |
| dot_qq_test.test | 10 | 0.000991s | 0.000991s | 0.000998s | byte equality observed |
| dot_vv_test.test | 10 | 0.001015s | 0.000979s | 0.000998s | byte equality observed |
| eq_mm_test.test | 10 | 0.001011s | 0.000997s | 0.001028s | byte equality observed |
| eq_qq_test.test | 10 | 0.001010s | 0.000998s | 0.001000s | byte equality observed |
| eq_vv_test.test | 10 | 0.000960s | 0.000977s | 0.000972s | byte equality observed |
| identity_mat_test.test | 10 | 0.000987s | 0.000971s | 0.001028s | byte equality observed |
| identity_quat_test.test | 10 | 0.000998s | 0.001001s | 0.000968s | byte equality observed |
| interop_test.test | 10 | 0.000456s | 0.000445s | 0.001000s | byte equality observed |
| inverse_m_test.test | 10 | 0.001091s | 0.001012s | 0.001033s | byte equality observed |
| inverse_q_test.test | 10 | 0.001016s | 0.001066s | 0.000998s | byte equality observed |
| mag_q_test.test | 10 | 0.001015s | 0.000981s | 0.001004s | byte equality observed |
| mag_sqr_q_test.test | 10 | 0.001005s | 0.001027s | 0.001015s | byte equality observed |
| mag_sqr_v_test.test | 10 | 0.000992s | 0.001003s | 0.001013s | byte equality observed |
| mag_v_test.test | 10 | 0.001032s | 0.001020s | 0.001006s | byte equality observed |
| mat_index_test.test | 10 | 0.000996s | 0.000972s | 0.000974s | byte equality observed |
| mat_traits_array_test.test | 10 | 0.000996s | 0.000980s | 0.001008s | byte equality observed |
| mat_traits_std_array_test.test | 10 | 0.000994s | 0.000988s | 0.000986s | byte equality observed |
| math_test.test | 10 | 0.001500s | 0.001449s | 0.001317s | byte equality observed |
| minus_eq_mm_test.test | 10 | 0.000986s | 0.000989s | 0.000981s | byte equality observed |
| minus_eq_qq_test.test | 10 | 0.000989s | 0.000968s | 0.000986s | byte equality observed |
| minus_eq_vv_test.test | 10 | 0.000979s | 0.000993s | 0.000997s | byte equality observed |
| minus_m_test.test | 10 | 0.000988s | 0.001016s | 0.001008s | byte equality observed |
| minus_mm_test.test | 10 | 0.001010s | 0.001036s | 0.000997s | byte equality observed |
| minus_q_test.test | 10 | 0.000991s | 0.000981s | 0.000990s | byte equality observed |
| minus_qq_test.test | 10 | 0.000954s | 0.000974s | 0.001023s | byte equality observed |
| minus_v_test.test | 10 | 0.000992s | 0.001039s | 0.001015s | byte equality observed |
| minus_vv_test.test | 10 | 0.000997s | 0.001013s | 0.000981s | byte equality observed |
| mul_eq_mm_test.test | 10 | 0.000985s | 0.001001s | 0.000997s | byte equality observed |
| mul_eq_ms_test.test | 10 | 0.000998s | 0.000970s | 0.001001s | byte equality observed |
| mul_eq_qq_test.test | 10 | 0.000985s | 0.001033s | 0.000992s | byte equality observed |
| mul_eq_qs_test.test | 10 | 0.001019s | 0.001013s | 0.001022s | byte equality observed |
| mul_eq_vs_test.test | 10 | 0.000991s | 0.001014s | 0.000975s | byte equality observed |
| mul_mm_test.test | 10 | 0.001026s | 0.000999s | 0.000979s | byte equality observed |
| mul_ms_test.test | 10 | 0.001021s | 0.001004s | 0.001000s | byte equality observed |
| mul_mv_test.test | 10 | 0.000989s | 0.000959s | 0.000974s | byte equality observed |
| mul_qq_test.test | 10 | 0.000997s | 0.000998s | 0.001019s | byte equality observed |
| mul_qs_test.test | 10 | 0.001003s | 0.000964s | 0.000974s | byte equality observed |
| mul_qv_test.test | 10 | 0.001011s | 0.000986s | 0.000993s | byte equality observed |
| mul_sm_test.test | 10 | 0.000999s | 0.000968s | 0.000969s | byte equality observed |
| mul_sv_test.test | 10 | 0.000978s | 0.001001s | 0.000982s | byte equality observed |
| mul_vm_test.test | 10 | 0.000999s | 0.000982s | 0.001004s | byte equality observed |
| mul_vs_test.test | 10 | 0.000984s | 0.001013s | 0.001040s | byte equality observed |
| neg_col_test.test | 10 | 0.000988s | 0.000981s | 0.001013s | byte equality observed |
| neg_row_test.test | 10 | 0.001001s | 0.001024s | 0.000973s | byte equality observed |
| normalize_q_test.test | 10 | 0.000981s | 0.001000s | 0.001007s | byte equality observed |
| normalize_v_test.test | 10 | 0.000988s | 0.001001s | 0.001012s | byte equality observed |
| plus_eq_mm_test.test | 10 | 0.000984s | 0.001029s | 0.000973s | byte equality observed |
| plus_eq_qq_test.test | 10 | 0.000979s | 0.000986s | 0.000979s | byte equality observed |
| plus_eq_vv_test.test | 10 | 0.000978s | 0.000996s | 0.001013s | byte equality observed |
| plus_mm_test.test | 10 | 0.001000s | 0.001003s | 0.000976s | byte equality observed |
| plus_qq_test.test | 10 | 0.001022s | 0.001012s | 0.000965s | byte equality observed |
| plus_vv_test.test | 10 | 0.000978s | 0.001011s | 0.001003s | byte equality observed |
| projection_test.test | 10 | 0.000998s | 0.001008s | 0.001012s | byte equality observed |
| quat_traits_array_test.test | 10 | 0.001000s | 0.000994s | 0.001014s | byte equality observed |
| quat_traits_std_array_test.test | 10 | 0.000943s | 0.000948s | 0.000979s | byte equality observed |
| rot_mat_test.test | 1 | 0.062001s | 0.029199s | 0.033813s | byte equality observed |
| rot_quat_test.test | 10 | 0.000904s | 0.000928s | 0.000941s | byte equality observed |
| rotx_mat_test.test | 10 | 0.000923s | 0.000959s | 0.000935s | byte equality observed |
| rotx_quat_test.test | 10 | 0.000900s | 0.001043s | 0.000978s | byte equality observed |
| roty_mat_test.test | 10 | 0.000926s | 0.001013s | 0.001041s | byte equality observed |
| roty_quat_test.test | 10 | 0.000995s | 0.001012s | 0.001052s | byte equality observed |
| rotz_mat_test.test | 10 | 0.001062s | 0.001036s | 0.001027s | byte equality observed |
| rotz_quat_test.test | 10 | 0.001011s | 0.001008s | 0.000987s | byte equality observed |
| row_mat_test.test | 10 | 0.000978s | 0.001041s | 0.001024s | byte equality observed |
| row_test.test | 10 | 0.001004s | 0.001019s | 0.001028s | byte equality observed |
| scalar_cast_m_test.test | 10 | 0.001023s | 0.000987s | 0.000998s | byte equality observed |
| scalar_cast_q_test.test | 10 | 0.001002s | 0.001006s | 0.001012s | byte equality observed |
| scalar_cast_v_test.test | 10 | 0.000969s | 0.001017s | 0.000967s | byte equality observed |
| slerp_test.test | 2 | 0.018464s | 0.014165s | 0.015805s | byte equality observed |
| swap_cols_test.test | 10 | 0.000898s | 0.000934s | 0.000916s | byte equality observed |
| swap_rows_test.test | 10 | 0.000949s | 0.000937s | 0.000913s | byte equality observed |
| swizzle2_test.test | 10 | 0.000958s | 0.001039s | 0.001010s | byte equality observed |
| swizzle2_test2.test | 10 | 0.001010s | 0.000978s | 0.001027s | byte equality observed |
| swizzle2_test3.test | 10 | 0.000985s | 0.000987s | 0.000994s | byte equality observed |
| swizzle2_test4.test | 10 | 0.000986s | 0.001006s | 0.001008s | byte equality observed |
| swizzle3_test.test | 10 | 0.001012s | 0.001002s | 0.001012s | byte equality observed |
| swizzle3_test2.test | 10 | 0.000981s | 0.000988s | 0.001016s | byte equality observed |
| swizzle3_test3.test | 10 | 0.001011s | 0.000990s | 0.001005s | byte equality observed |
| swizzle3_test4.test | 10 | 0.001016s | 0.000975s | 0.001012s | byte equality observed |
| swizzle4_test.test | 10 | 0.000994s | 0.001004s | 0.000996s | byte equality observed |
| swizzle4_test2.test | 10 | 0.000993s | 0.000991s | 0.001024s | byte equality observed |
| swizzle4_test3.test | 10 | 0.000996s | 0.000988s | 0.000994s | byte equality observed |
| swizzle4_test4.test | 10 | 0.000992s | 0.000984s | 0.000996s | byte equality observed |
| to_string_test.test | 10 | 0.001031s | 0.001007s | 0.001032s | byte equality observed |
| transform_test.test | 10 | 0.001037s | 0.001028s | 0.001037s | byte equality observed |
| translation_mat_test.test | 10 | 0.000984s | 0.001024s | 0.000997s | byte equality observed |
| translation_test.test | 10 | 0.000979s | 0.001017s | 0.001005s | byte equality observed |
| transpose_test.test | 10 | 0.001015s | 0.001031s | 0.000995s | byte equality observed |
| vec_index_test.test | 10 | 0.001015s | 0.001009s | 0.001012s | byte equality observed |
| vec_register_test.test | 10 | 0.000987s | 0.000960s | 0.000974s | byte equality observed |
| vec_traits_array_test.test | 10 | 0.000982s | 0.000985s | 0.001030s | byte equality observed |
| vec_traits_gnuc_test.test | 10 | 0.000990s | 0.000957s | 0.001004s | byte equality observed |
| vec_traits_std_array_test.test | 10 | 0.001032s | 0.001004s | 0.001015s | byte equality observed |
| zero_mat_test.test | 10 | 0.000988s | 0.001026s | 0.001002s | byte equality observed |
| zero_quat_test.test | 10 | 0.000978s | 0.000974s | 0.001042s | byte equality observed |
| zero_vec_test.test | 10 | 0.001016s | 0.001012s | 0.000977s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
