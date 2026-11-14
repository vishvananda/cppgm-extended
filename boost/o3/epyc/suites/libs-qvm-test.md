# libs/qvm/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| math_test.test | RUN | 0.204059 | 0.505422 | 0.593844 | 0.001996 | 0.001842 | 0.002240 |
| mat_traits_array_test.test | RUN | 0.160027 | 0.495348 | 0.453247 | 0.001684 | 0.001400 | 0.001554 |
| mat_traits_std_array_test.test | RUN | 0.165042 | 0.373808 | 0.452278 | 0.001471 | 0.001410 | 0.001151 |
| vec_traits_array_test.test | RUN | 0.140252 | 0.377671 | 0.396336 | 0.001470 | 0.001418 | 0.001839 |
| vec_traits_gnuc_test.test | RUN | 0.259475 | 0.454305 | 0.738840 | 0.001489 | 0.001414 | 0.001485 |
| vec_traits_std_array_test.test | RUN | 0.139421 | 0.291571 | 0.383782 | 0.001508 | 0.001494 | 0.001815 |
| quat_traits_array_test.test | RUN | 0.136810 | 0.387230 | 0.382221 | 0.001559 | 0.001394 | 0.001615 |
| quat_traits_std_array_test.test | RUN | 0.137522 | 0.295698 | 0.389534 | 0.001434 | 0.001334 | 0.001721 |
| access_m_test.test | RUN | 0.223408 | 0.687497 | 0.885171 | 0.001668 | 0.001542 | 0.001760 |
| access_m_fail1.test | COMPILE_FAIL | 0.021633 | 0.031588 | 0.064183 | — | — | — |
| access_m_fail2.test | COMPILE_FAIL | 0.021547 | 0.031170 | 0.058079 | — | — | — |
| access_q_test.test | RUN | 0.184910 | 0.371157 | 0.527819 | 0.001672 | 0.001589 | 0.001685 |
| access_q_fail.test | COMPILE_FAIL | 0.017424 | 0.026097 | 0.051975 | — | — | — |
| access_v_test.test | RUN | 0.188768 | 0.373737 | 0.519084 | 0.001457 | 0.001600 | 0.001613 |
| access_v_fail1.test | COMPILE_FAIL | 0.015286 | 0.023037 | 0.049998 | — | — | — |
| access_v_fail2.test | COMPILE_FAIL | 0.015947 | 0.024362 | 0.050848 | — | — | — |
| assign_test.test | RUN | 0.274690 | 0.605677 | 0.939912 | 0.001803 | 0.001680 | 0.001967 |
| to_string_test.test | RUN | 0.305516 | 0.980431 | 5.666828 | 0.001494 | 0.001498 | 0.001801 |
| cmp_vv_test.test | RUN | 0.228055 | 0.468270 | 0.628457 | 0.001596 | 0.001482 | 0.001866 |
| cross_test.test | RUN | 0.223400 | 0.458965 | 0.613776 | 0.001605 | 0.001778 | 0.001830 |
| div_eq_vs_test.test | RUN | 0.214398 | 0.410346 | 0.614177 | 0.001799 | 0.001582 | 0.001816 |
| div_vs_test.test | RUN | 0.220083 | 0.505428 | 0.618939 | 0.001353 | 0.001652 | 0.001581 |
| dot_vv_test.test | RUN | 0.210170 | 0.452004 | 0.592703 | 0.001617 | 0.001541 | 0.001325 |
| eq_vv_test.test | RUN | 0.228474 | 0.494904 | 0.675551 | 0.001388 | 0.001568 | 0.001675 |
| mag_v_test.test | RUN | 0.214825 | 0.418542 | 0.540278 | 0.001736 | 0.001680 | 0.001896 |
| mag_sqr_v_test.test | RUN | 0.212101 | 0.410712 | 0.584924 | 0.001538 | 0.001577 | 0.001696 |
| minus_v_test.test | RUN | 0.220931 | 0.474003 | 0.653726 | 0.001410 | 0.001494 | 0.001860 |
| minus_eq_vv_test.test | RUN | 0.215624 | 0.464053 | 0.604692 | 0.001361 | 0.001445 | 0.001896 |
| minus_vv_test.test | RUN | 0.220208 | 0.504700 | 0.675637 | 0.001666 | 0.001550 | 0.001867 |
| mul_eq_vs_test.test | RUN | 0.207421 | 0.392278 | 0.629145 | 0.001423 | 0.001609 | 0.001413 |
| mul_vs_test.test | RUN | 0.216623 | 0.500222 | 0.631832 | 0.001154 | 0.001712 | 0.001622 |
| mul_sv_test.test | RUN | 0.217407 | 0.505727 | 0.679981 | 0.001479 | 0.001597 | 0.001882 |
| mul_vm_test.test | RUN | 0.297341 | 0.908870 | 0.990113 | 0.001376 | 0.001689 | 0.001843 |
| normalize_v_test.test | RUN | 0.218328 | 0.613325 | 0.603568 | 0.001416 | 0.001480 | 0.001473 |
| plus_eq_vv_test.test | RUN | 0.199471 | 0.475132 | 0.603891 | 0.001630 | 0.001509 | 0.001824 |
| plus_vv_test.test | RUN | 0.208191 | 0.519798 | 0.627341 | 0.001596 | 0.001688 | 0.001752 |
| scalar_cast_v_test.test | RUN | 0.206112 | 0.381025 | 0.501665 | 0.001431 | 0.001460 | 0.001623 |
| vec_index_test.test | RUN | 0.177608 | 0.394167 | 0.488204 | 0.001431 | 0.001624 | 0.001378 |
| vec_register_test.test | RUN | 0.117626 | 0.339392 | 0.346341 | 0.001772 | 0.001630 | 0.001812 |
| cmp_mm_test.test | RUN | 0.336418 | 1.089690 | 1.277887 | 0.001735 | 0.001631 | 0.001480 |
| determinant_test.test | RUN | 0.238060 | 0.553454 | 0.661447 | 0.001715 | 0.001529 | 0.001796 |
| div_eq_ms_test.test | RUN | 0.244950 | 0.516167 | 0.755960 | 0.001488 | 0.001580 | 0.001786 |
| div_ms_test.test | RUN | 0.285575 | 0.826200 | 0.957097 | 0.001465 | 0.001478 | 0.001634 |
| eq_mm_test.test | RUN | 0.326057 | 1.046857 | 1.237328 | 0.001778 | 0.001539 | 0.001632 |
| minus_m_test.test | RUN | 0.288346 | 0.841957 | 0.931291 | 0.001592 | 0.001288 | 0.001447 |
| minus_eq_mm_test.test | RUN | 0.267745 | 0.797411 | 0.868239 | 0.001484 | 0.001596 | 0.001734 |
| minus_mm_test.test | RUN | 0.301790 | 1.004821 | 1.013673 | 0.001652 | 0.001712 | 0.001775 |
| mul_eq_mm_test.test | RUN | 0.261075 | 0.710091 | 0.743857 | 0.001838 | 0.001594 | 0.001644 |
| mul_eq_ms_test.test | RUN | 0.249929 | 0.530525 | 0.792400 | 0.001766 | 0.001494 | 0.001718 |
| mul_mm_test.test | RUN | 0.357078 | 1.586551 | 1.262070 | 0.001722 | 0.001522 | 0.001814 |
| mul_ms_test.test | RUN | 0.301316 | 0.867317 | 0.954655 | 0.001424 | 0.001555 | 0.001741 |
| mul_sm_test.test | RUN | 0.305385 | 0.834734 | 0.956479 | 0.001978 | 0.001652 | 0.001580 |
| mul_mv_test.test | RUN | 0.308084 | 0.912052 | 0.951404 | 0.001801 | 0.001567 | 0.001740 |
| inverse_m_test.test | RUN | 0.402570 | 1.645555 | 1.372224 | 0.001820 | 0.001409 | 0.001805 |
| plus_eq_mm_test.test | RUN | 0.268739 | 0.754058 | 0.840704 | 0.001748 | 0.001494 | 0.001640 |
| plus_mm_test.test | RUN | 0.307909 | 0.997917 | 1.002102 | 0.001757 | 0.001578 | 0.001676 |
| scalar_cast_m_test.test | RUN | 0.238658 | 0.451459 | 0.640052 | 0.001830 | 0.001499 | 0.001724 |
| mat_index_test.test | RUN | 0.192869 | 0.416426 | 0.544485 | 0.001440 | 0.001355 | 0.001719 |
| cmp_qq_test.test | RUN | 0.209926 | 0.396589 | 0.513278 | 0.001734 | 0.001491 | 0.001591 |
| conjugate_test.test | RUN | 0.211718 | 0.403719 | 0.520031 | 0.001720 | 0.001617 | 0.001627 |
| normalize_q_test.test | RUN | 0.216979 | 0.459909 | 0.529287 | 0.001703 | 0.001566 | 0.001674 |
| div_eq_qs_test.test | RUN | 0.211720 | 0.386016 | 0.503572 | 0.001714 | 0.001439 | 0.001628 |
| div_qs_test.test | RUN | 0.211341 | 0.417750 | 0.522080 | 0.001809 | 0.001391 | 0.001447 |
| dot_qq_test.test | RUN | 0.207741 | 0.393005 | 0.528571 | 0.001805 | 0.001863 | 0.001631 |
| eq_qq_test.test | RUN | 0.212799 | 0.416122 | 0.526886 | 0.001612 | 0.001669 | 0.001675 |
| inverse_q_test.test | RUN | 0.250981 | 0.546330 | 0.654121 | 0.001748 | 0.001483 | 0.001664 |
| mag_q_test.test | RUN | 0.207914 | 0.404817 | 0.527605 | 0.001629 | 0.001467 | 0.001419 |
| mag_sqr_q_test.test | RUN | 0.203830 | 0.401595 | 0.525792 | 0.001776 | 0.001686 | 0.001413 |
| minus_q_test.test | RUN | 0.205546 | 0.416641 | 0.512115 | 0.001559 | 0.001698 | 0.001670 |
| minus_eq_qq_test.test | RUN | 0.206542 | 0.397773 | 0.501224 | 0.001487 | 0.001341 | 0.001674 |
| minus_qq_test.test | RUN | 0.217143 | 0.425189 | 0.536713 | 0.001719 | 0.001756 | 0.001659 |
| mul_eq_qs_test.test | RUN | 0.206696 | 0.386152 | 0.527706 | 0.001779 | 0.001660 | 0.001706 |
| mul_qs_test.test | RUN | 0.210325 | 0.411295 | 0.518120 | 0.001734 | 0.001639 | 0.001739 |
| mul_qv_test.test | RUN | 0.260972 | 0.559875 | 0.667623 | 0.001647 | 0.001647 | 0.001708 |
| mul_qq_test.test | RUN | 0.248323 | 0.556763 | 0.639420 | 0.001606 | 0.001668 | 0.001612 |
| mul_eq_qq_test.test | RUN | 0.245080 | 0.566060 | 0.633588 | 0.001524 | 0.001652 | 0.001531 |
| plus_eq_qq_test.test | RUN | 0.207442 | 0.412760 | 0.479080 | 0.001506 | 0.001292 | 0.001412 |
| plus_qq_test.test | RUN | 0.207984 | 0.413110 | 0.496810 | 0.001769 | 0.001498 | 0.001866 |
| scalar_cast_q_test.test | RUN | 0.207087 | 0.392382 | 0.495270 | 0.001773 | 0.001554 | 0.001724 |
| slerp_test.test | RUN | 0.225331 | 0.777347 | 0.590761 | 0.015035 | 0.012367 | 0.012977 |
| convert_to_test.test | RUN | 0.299497 | 0.714012 | 0.944460 | 0.001748 | 0.001723 | 0.001656 |
| rot_mat_test.test | RUN | 0.436267 | 3.240116 | 3.015306 | 0.031901 | 0.030374 | 0.029560 |
| rot_quat_test.test | RUN | 0.255052 | 0.667341 | 0.669642 | 0.001846 | 0.001700 | 0.001684 |
| rotx_mat_test.test | RUN | 0.288302 | 0.865120 | 0.903983 | 0.001558 | 0.001726 | 0.001633 |
| rotx_quat_test.test | RUN | 0.242248 | 0.578424 | 0.645834 | 0.001459 | 0.001550 | 0.001706 |
| roty_mat_test.test | RUN | 0.277896 | 0.882907 | 0.832962 | 0.001735 | 0.001755 | 0.001664 |
| roty_quat_test.test | RUN | 0.249643 | 0.574112 | 0.630023 | 0.001508 | 0.001768 | 0.001704 |
| rotz_mat_test.test | RUN | 0.295104 | 0.998687 | 0.947802 | 0.001525 | 0.001679 | 0.001729 |
| rotz_quat_test.test | RUN | 0.247726 | 0.576213 | 0.631520 | 0.001646 | 0.001718 | 0.001556 |
| zero_vec_test.test | RUN | 0.208797 | 0.490595 | 0.557464 | 0.001592 | 0.001540 | 0.001670 |
| zero_mat_test.test | RUN | 0.256566 | 0.791863 | 0.731100 | 0.001715 | 0.001520 | 0.001319 |
| zero_quat_test.test | RUN | 0.206883 | 0.396846 | 0.508293 | 0.001652 | 0.001459 | 0.001465 |
| identity_mat_test.test | RUN | 0.250769 | 0.598058 | 0.660598 | 0.001575 | 0.001385 | 0.001543 |
| identity_quat_test.test | RUN | 0.206973 | 0.401849 | 0.510698 | 0.001709 | 0.001756 | 0.001412 |
| col_test.test | RUN | 0.383270 | 1.274239 | 1.402933 | 0.001493 | 0.001530 | 0.001652 |
| col_mat_test.test | RUN | 0.257096 | 0.588132 | 0.650063 | 0.001571 | 0.001653 | 0.001666 |
| neg_col_test.test | RUN | 0.294285 | 0.795603 | 0.854125 | 0.001672 | 0.001680 | 0.001500 |
| neg_row_test.test | RUN | 0.302051 | 0.786723 | 0.851349 | 0.001757 | 0.001602 | 0.001403 |
| del_col_test.test | RUN | 0.301548 | 0.854921 | 0.891666 | 0.001751 | 0.001673 | 0.001635 |
| del_row_test.test | RUN | 0.304454 | 0.838300 | 0.873289 | 0.001548 | 0.001799 | 0.001687 |
| del_row_col_test.test | RUN | 0.417780 | 1.587185 | 1.627172 | 0.001842 | 0.001597 | 0.001653 |
| swap_cols_test.test | RUN | 0.552014 | 2.089090 | 2.021933 | 0.001796 | 0.001506 | 0.001657 |
| swap_rows_test.test | RUN | 0.644675 | 2.185322 | 1.954712 | 0.001816 | 0.001526 | 0.001409 |
| diag_test.test | RUN | 0.262573 | 0.595757 | 0.740154 | 0.001438 | 0.001642 | 0.001670 |
| diag_mat_test.test | RUN | 0.274000 | 0.574852 | 0.679246 | 0.001706 | 0.001685 | 0.001510 |
| row_test.test | RUN | 0.374990 | 1.202501 | 1.462206 | 0.001738 | 0.001704 | 0.001611 |
| row_mat_test.test | RUN | 0.255207 | 0.541312 | 0.679846 | 0.001725 | 0.001999 | 0.001712 |
| transpose_test.test | RUN | 0.302559 | 0.846775 | 0.940708 | 0.001649 | 0.001492 | 0.001660 |
| translation_test.test | RUN | 0.251806 | 0.525209 | 0.714097 | 0.001595 | 0.001555 | 0.001319 |
| translation_mat_test.test | RUN | 0.266859 | 0.548304 | 0.739636 | 0.001516 | 0.001550 | 0.001360 |
| swizzle_const_fail.test | COMPILE_FAIL | 0.091366 | 0.152260 | 0.197136 | — | — | — |
| swizzle2_test2.test | RUN | 0.109985 | 0.228098 | 0.278053 | 0.001638 | 0.001280 | 0.001654 |
| swizzle2_test3.test | RUN | 0.110645 | 0.227743 | 0.278921 | 0.001551 | 0.001298 | 0.001405 |
| swizzle2_test4.test | RUN | 0.112566 | 0.228545 | 0.265532 | 0.001708 | 0.001720 | 0.001479 |
| swizzle2_test.test | RUN | 0.353228 | 0.624249 | 0.856940 | 0.001723 | 0.001781 | 0.001696 |
| swizzle3_test2.test | RUN | 0.122969 | 0.235202 | 0.314705 | 0.001533 | 0.001647 | 0.001433 |
| swizzle3_test3.test | RUN | 0.126121 | 0.240012 | 0.323853 | 0.001530 | 0.001462 | 0.001758 |
| swizzle3_test4.test | RUN | 0.126540 | 0.238591 | 0.315898 | 0.001605 | 0.001532 | 0.001510 |
| swizzle3_test.test | RUN | 0.350252 | 0.637940 | 0.854331 | 0.001587 | 0.001291 | 0.001487 |
| swizzle4_test2.test | RUN | 0.201354 | 0.347757 | 0.494628 | 0.001438 | 0.001586 | 0.001763 |
| swizzle4_test3.test | RUN | 0.202736 | 0.353449 | 0.488826 | 0.001643 | 0.001664 | 0.001482 |
| swizzle4_test4.test | RUN | 0.202165 | 0.335658 | 0.498638 | 0.001663 | 0.001602 | 0.001786 |
| swizzle4_test.test | RUN | 0.351770 | 0.687754 | 0.883621 | 0.001405 | 0.001386 | 0.001756 |
| deduce_scalar_test.test | COMPILE | 0.017450 | 0.034535 | 0.052319 | — | — | — |
| deduce_scalar_mq_test.test | RUN | 0.131773 | 0.266632 | 0.333784 | 0.001464 | 0.001348 | 0.001429 |
| deduce_scalar_mv_test.test | RUN | 0.161318 | 0.301763 | 0.393406 | 0.001570 | 0.001465 | 0.001732 |
| deduce_scalar_fail.test | COMPILE_FAIL | 0.014835 | 0.022828 | 0.041326 | — | — | — |
| deduce_mat_test.test | COMPILE | 0.015966 | 0.031509 | 0.047695 | — | — | — |
| deduce_quat_test.test | COMPILE | 0.015780 | 0.030757 | 0.048128 | — | — | — |
| deduce_vec_test.test | COMPILE | 0.016350 | 0.030014 | 0.040806 | — | — | — |
| interop_test.test | RUN | 0.160294 | 0.308696 | 0.590765 | 0.000785 | 0.000485 | 0.001320 |
| transform_test.test | RUN | 0.304091 | 0.538929 | 0.763450 | 0.001320 | 0.001104 | 0.001269 |
| projection_test.test | RUN | 0.192929 | 0.406576 | 0.554289 | 0.001423 | 0.001191 | 0.001362 |
| scalar_traits_test.test | COMPILE | 0.016777 | 0.032491 | 0.050786 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.986×† | 0.964×, 1.009× | 1.063× |
| clang | 1.004× | 1.022×, 0.986× | 1.002× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051632).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| access_m_test.test | 10 | 0.000909s | 0.000923s | 0.000953s | byte equality observed |
| access_q_test.test | 10 | 0.000900s | 0.000915s | 0.000932s | byte equality observed |
| access_v_test.test | 10 | 0.000901s | 0.000898s | 0.000912s | byte equality observed |
| assign_test.test | 10 | 0.000896s | 0.000933s | 0.000937s | byte equality observed |
| cmp_mm_test.test | 10 | 0.000906s | 0.000928s | 0.000926s | byte equality observed |
| cmp_qq_test.test | 10 | 0.000902s | 0.000908s | 0.000920s | byte equality observed |
| cmp_vv_test.test | 10 | 0.000887s | 0.000894s | 0.000923s | byte equality observed |
| col_mat_test.test | 10 | 0.000906s | 0.000893s | 0.000920s | byte equality observed |
| col_test.test | 10 | 0.000907s | 0.000887s | 0.000919s | byte equality observed |
| conjugate_test.test | 10 | 0.000888s | 0.000905s | 0.000905s | byte equality observed |
| convert_to_test.test | 10 | 0.000908s | 0.000896s | 0.000908s | byte equality observed |
| cross_test.test | 10 | 0.000890s | 0.000911s | 0.000930s | byte equality observed |
| deduce_scalar_mq_test.test | 10 | 0.000903s | 0.000908s | 0.000906s | byte equality observed |
| deduce_scalar_mv_test.test | 10 | 0.000906s | 0.000930s | 0.000907s | byte equality observed |
| del_col_test.test | 10 | 0.000902s | 0.000943s | 0.000904s | byte equality observed |
| del_row_col_test.test | 10 | 0.000904s | 0.000903s | 0.000912s | byte equality observed |
| del_row_test.test | 10 | 0.000884s | 0.000903s | 0.000909s | byte equality observed |
| determinant_test.test | 10 | 0.000904s | 0.000918s | 0.000894s | byte equality observed |
| diag_mat_test.test | 10 | 0.000881s | 0.000878s | 0.000906s | byte equality observed |
| diag_test.test | 10 | 0.000907s | 0.000879s | 0.000929s | byte equality observed |
| div_eq_ms_test.test | 10 | 0.000892s | 0.000894s | 0.000922s | byte equality observed |
| div_eq_qs_test.test | 10 | 0.000889s | 0.000902s | 0.000917s | byte equality observed |
| div_eq_vs_test.test | 10 | 0.000894s | 0.000927s | 0.000927s | byte equality observed |
| div_ms_test.test | 10 | 0.000896s | 0.000915s | 0.000910s | byte equality observed |
| div_qs_test.test | 10 | 0.000895s | 0.000885s | 0.000940s | byte equality observed |
| div_vs_test.test | 10 | 0.000890s | 0.000919s | 0.000911s | byte equality observed |
| dot_qq_test.test | 10 | 0.000873s | 0.000902s | 0.000923s | byte equality observed |
| dot_vv_test.test | 10 | 0.000880s | 0.000901s | 0.000961s | byte equality observed |
| eq_mm_test.test | 10 | 0.000906s | 0.000900s | 0.000929s | byte equality observed |
| eq_qq_test.test | 10 | 0.000872s | 0.000896s | 0.000904s | byte equality observed |
| eq_vv_test.test | 10 | 0.000893s | 0.000910s | 0.000926s | byte equality observed |
| identity_mat_test.test | 10 | 0.000900s | 0.000911s | 0.000907s | byte equality observed |
| identity_quat_test.test | 10 | 0.000883s | 0.000920s | 0.000911s | byte equality observed |
| interop_test.test | 10 | 0.000410s | 0.000411s | 0.000884s | byte equality observed |
| inverse_m_test.test | 10 | 0.000918s | 0.000955s | 0.000961s | byte equality observed |
| inverse_q_test.test | 10 | 0.000897s | 0.000921s | 0.000910s | byte equality observed |
| mag_q_test.test | 10 | 0.000888s | 0.000916s | 0.000911s | byte equality observed |
| mag_sqr_q_test.test | 10 | 0.000877s | 0.000897s | 0.000904s | byte equality observed |
| mag_sqr_v_test.test | 10 | 0.000884s | 0.000887s | 0.000901s | byte equality observed |
| mag_v_test.test | 10 | 0.000906s | 0.000892s | 0.000914s | byte equality observed |
| mat_index_test.test | 10 | 0.000917s | 0.000890s | 0.000907s | byte equality observed |
| mat_traits_array_test.test | 10 | 0.000890s | 0.000894s | 0.000930s | byte equality observed |
| mat_traits_std_array_test.test | 10 | 0.000886s | 0.000895s | 0.000900s | byte equality observed |
| math_test.test | 10 | 0.001347s | 0.001298s | 0.001184s | byte equality observed |
| minus_eq_mm_test.test | 10 | 0.000909s | 0.000888s | 0.000920s | byte equality observed |
| minus_eq_qq_test.test | 10 | 0.000897s | 0.000927s | 0.000900s | byte equality observed |
| minus_eq_vv_test.test | 10 | 0.000899s | 0.000882s | 0.000917s | byte equality observed |
| minus_m_test.test | 10 | 0.000910s | 0.000886s | 0.000897s | byte equality observed |
| minus_mm_test.test | 10 | 0.000907s | 0.000888s | 0.000926s | byte equality observed |
| minus_q_test.test | 10 | 0.000901s | 0.000899s | 0.000926s | byte equality observed |
| minus_qq_test.test | 10 | 0.000907s | 0.000893s | 0.000911s | byte equality observed |
| minus_v_test.test | 10 | 0.000900s | 0.000921s | 0.000915s | byte equality observed |
| minus_vv_test.test | 10 | 0.000920s | 0.000906s | 0.000920s | byte equality observed |
| mul_eq_mm_test.test | 10 | 0.000907s | 0.000934s | 0.000920s | byte equality observed |
| mul_eq_ms_test.test | 10 | 0.000899s | 0.000901s | 0.000935s | byte equality observed |
| mul_eq_qq_test.test | 10 | 0.000895s | 0.000931s | 0.000938s | byte equality observed |
| mul_eq_qs_test.test | 10 | 0.000912s | 0.000895s | 0.000917s | byte equality observed |
| mul_eq_vs_test.test | 10 | 0.000884s | 0.000893s | 0.000918s | byte equality observed |
| mul_mm_test.test | 10 | 0.000886s | 0.000910s | 0.000928s | byte equality observed |
| mul_ms_test.test | 10 | 0.000873s | 0.000898s | 0.000899s | byte equality observed |
| mul_mv_test.test | 10 | 0.000896s | 0.000898s | 0.000889s | byte equality observed |
| mul_qq_test.test | 10 | 0.000912s | 0.000926s | 0.000938s | byte equality observed |
| mul_qs_test.test | 10 | 0.000895s | 0.000902s | 0.000907s | byte equality observed |
| mul_qv_test.test | 10 | 0.000925s | 0.000913s | 0.000922s | byte equality observed |
| mul_sm_test.test | 10 | 0.000908s | 0.000904s | 0.000901s | byte equality observed |
| mul_sv_test.test | 10 | 0.000922s | 0.000895s | 0.000921s | byte equality observed |
| mul_vm_test.test | 10 | 0.000901s | 0.000899s | 0.000905s | byte equality observed |
| mul_vs_test.test | 10 | 0.000899s | 0.000891s | 0.000904s | byte equality observed |
| neg_col_test.test | 10 | 0.000894s | 0.000880s | 0.000934s | byte equality observed |
| neg_row_test.test | 10 | 0.000901s | 0.000918s | 0.000907s | byte equality observed |
| normalize_q_test.test | 10 | 0.000911s | 0.000902s | 0.000923s | byte equality observed |
| normalize_v_test.test | 10 | 0.000916s | 0.000896s | 0.000893s | byte equality observed |
| plus_eq_mm_test.test | 10 | 0.000892s | 0.000878s | 0.000924s | byte equality observed |
| plus_eq_qq_test.test | 10 | 0.000929s | 0.000889s | 0.000927s | byte equality observed |
| plus_eq_vv_test.test | 10 | 0.000893s | 0.000891s | 0.000917s | byte equality observed |
| plus_mm_test.test | 10 | 0.000912s | 0.000871s | 0.000937s | byte equality observed |
| plus_qq_test.test | 10 | 0.000909s | 0.000868s | 0.000913s | byte equality observed |
| plus_vv_test.test | 10 | 0.000882s | 0.000884s | 0.000895s | byte equality observed |
| projection_test.test | 10 | 0.000911s | 0.000882s | 0.000902s | byte equality observed |
| quat_traits_array_test.test | 10 | 0.000902s | 0.000865s | 0.000897s | byte equality observed |
| quat_traits_std_array_test.test | 10 | 0.000869s | 0.000917s | 0.000879s | byte equality observed |
| rot_mat_test.test | 1 | 0.020709s | 0.020337s | 0.019557s | byte equality observed |
| rot_quat_test.test | 10 | 0.000853s | 0.000900s | 0.000871s | byte equality observed |
| rotx_mat_test.test | 10 | 0.000898s | 0.000927s | 0.000920s | byte equality observed |
| rotx_quat_test.test | 10 | 0.000920s | 0.000948s | 0.000954s | byte equality observed |
| roty_mat_test.test | 10 | 0.000928s | 0.000944s | 0.000945s | byte equality observed |
| roty_quat_test.test | 10 | 0.000905s | 0.000924s | 0.000909s | byte equality observed |
| rotz_mat_test.test | 10 | 0.000923s | 0.000972s | 0.000932s | byte equality observed |
| rotz_quat_test.test | 10 | 0.000909s | 0.000944s | 0.000916s | byte equality observed |
| row_mat_test.test | 10 | 0.000903s | 0.000927s | 0.000927s | byte equality observed |
| row_test.test | 10 | 0.000891s | 0.000936s | 0.000909s | byte equality observed |
| scalar_cast_m_test.test | 10 | 0.000882s | 0.000907s | 0.000911s | byte equality observed |
| scalar_cast_q_test.test | 10 | 0.000896s | 0.000906s | 0.000890s | byte equality observed |
| scalar_cast_v_test.test | 10 | 0.000860s | 0.000897s | 0.000880s | byte equality observed |
| slerp_test.test | 2 | 0.009812s | 0.008221s | 0.008840s | byte equality observed |
| swap_cols_test.test | 10 | 0.000893s | 0.000876s | 0.000895s | byte equality observed |
| swap_rows_test.test | 10 | 0.000923s | 0.000941s | 0.000905s | byte equality observed |
| swizzle2_test.test | 10 | 0.000896s | 0.000910s | 0.000896s | byte equality observed |
| swizzle2_test2.test | 10 | 0.000874s | 0.000912s | 0.000932s | byte equality observed |
| swizzle2_test3.test | 10 | 0.000888s | 0.000898s | 0.000909s | byte equality observed |
| swizzle2_test4.test | 10 | 0.000894s | 0.000904s | 0.000940s | byte equality observed |
| swizzle3_test.test | 10 | 0.000891s | 0.000903s | 0.000919s | byte equality observed |
| swizzle3_test2.test | 10 | 0.000885s | 0.000904s | 0.000918s | byte equality observed |
| swizzle3_test3.test | 10 | 0.000916s | 0.000920s | 0.000919s | byte equality observed |
| swizzle3_test4.test | 10 | 0.000888s | 0.000916s | 0.000923s | byte equality observed |
| swizzle4_test.test | 10 | 0.000905s | 0.000921s | 0.000923s | byte equality observed |
| swizzle4_test2.test | 10 | 0.000901s | 0.000928s | 0.000913s | byte equality observed |
| swizzle4_test3.test | 10 | 0.000894s | 0.000919s | 0.000941s | byte equality observed |
| swizzle4_test4.test | 10 | 0.000910s | 0.000939s | 0.000910s | byte equality observed |
| to_string_test.test | 10 | 0.000928s | 0.000947s | 0.000963s | byte equality observed |
| transform_test.test | 10 | 0.000894s | 0.000906s | 0.000891s | byte equality observed |
| translation_mat_test.test | 10 | 0.000901s | 0.000908s | 0.000889s | byte equality observed |
| translation_test.test | 10 | 0.000908s | 0.000931s | 0.000926s | byte equality observed |
| transpose_test.test | 10 | 0.000924s | 0.000908s | 0.000903s | byte equality observed |
| vec_index_test.test | 10 | 0.000916s | 0.000890s | 0.000938s | byte equality observed |
| vec_register_test.test | 10 | 0.000925s | 0.000925s | 0.000912s | byte equality observed |
| vec_traits_array_test.test | 10 | 0.000912s | 0.000889s | 0.000901s | byte equality observed |
| vec_traits_gnuc_test.test | 10 | 0.000892s | 0.000912s | 0.000943s | byte equality observed |
| vec_traits_std_array_test.test | 10 | 0.000912s | 0.000893s | 0.000925s | byte equality observed |
| zero_mat_test.test | 10 | 0.000905s | 0.000896s | 0.000905s | byte equality observed |
| zero_quat_test.test | 10 | 0.000907s | 0.000913s | 0.000904s | byte equality observed |
| zero_vec_test.test | 10 | 0.000892s | 0.000885s | 0.000908s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051632).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
