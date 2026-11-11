# libs/qvm/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| math_test.test | RUN | 0.253915 | 0.538257 | 0.594452 | 0.002040 | 0.001966 | 0.002270 |
| mat_traits_array_test.test | RUN | 0.169411 | 0.530232 | 0.457577 | 0.001756 | 0.001406 | 0.001318 |
| mat_traits_std_array_test.test | RUN | 0.169374 | 0.404805 | 0.462279 | 0.001500 | 0.001689 | 0.001340 |
| vec_traits_array_test.test | RUN | 0.152366 | 0.406726 | 0.395300 | 0.001392 | 0.001684 | 0.001550 |
| vec_traits_gnuc_test.test | RUN | 0.277856 | 0.500578 | 0.717583 | 0.001579 | 0.001701 | 0.001740 |
| vec_traits_std_array_test.test | RUN | 0.150940 | 0.324837 | 0.377652 | 0.001679 | 0.001772 | 0.001677 |
| quat_traits_array_test.test | RUN | 0.148758 | 0.429608 | 0.379421 | 0.001513 | 0.001558 | 0.001624 |
| quat_traits_std_array_test.test | RUN | 0.152383 | 0.333559 | 0.375434 | 0.001529 | 0.001672 | 0.003135 |
| access_m_test.test | RUN | 0.247951 | 0.736422 | 0.872353 | 0.001642 | 0.001812 | 0.001871 |
| access_m_fail1.test | COMPILE_FAIL | 0.024242 | 0.034732 | 0.068030 | — | — | — |
| access_m_fail2.test | COMPILE_FAIL | 0.023736 | 0.033655 | 0.061862 | — | — | — |
| access_q_test.test | RUN | 0.217393 | 0.400523 | 0.543548 | 0.001561 | 0.001770 | 0.001930 |
| access_q_fail.test | COMPILE_FAIL | 0.019020 | 0.027523 | 0.056297 | — | — | — |
| access_v_test.test | RUN | 0.211744 | 0.404097 | 0.531649 | 0.001488 | 0.001667 | 0.001694 |
| access_v_fail1.test | COMPILE_FAIL | 0.016896 | 0.024608 | 0.049764 | — | — | — |
| access_v_fail2.test | COMPILE_FAIL | 0.017358 | 0.025790 | 0.049310 | — | — | — |
| assign_test.test | RUN | 0.298417 | 0.645225 | 0.930234 | 0.001708 | 0.001893 | 0.001672 |
| to_string_test.test | RUN | 0.336440 | 1.034277 | 5.980971 | 0.001654 | 0.001660 | 0.002020 |
| cmp_vv_test.test | RUN | 0.259054 | 0.515702 | 0.622648 | 0.001809 | 0.001666 | 0.001702 |
| cross_test.test | RUN | 0.245552 | 0.484040 | 0.612433 | 0.001430 | 0.001820 | 0.001972 |
| div_eq_vs_test.test | RUN | 0.228850 | 0.450327 | 0.623507 | 0.001572 | 0.001916 | 0.002418 |
| div_vs_test.test | RUN | 0.236765 | 0.556534 | 0.654638 | 0.001476 | 0.001840 | 0.001968 |
| dot_vv_test.test | RUN | 0.226455 | 0.488307 | 0.584811 | 0.001600 | 0.001659 | 0.003933 |
| eq_vv_test.test | RUN | 0.247442 | 0.548446 | 0.713362 | 0.001496 | 0.001610 | 0.001579 |
| mag_v_test.test | RUN | 0.232378 | 0.461606 | 0.550931 | 0.001654 | 0.001825 | 0.001717 |
| mag_sqr_v_test.test | RUN | 0.227820 | 0.456767 | 0.603956 | 0.001396 | 0.001776 | 0.001610 |
| minus_v_test.test | RUN | 0.232398 | 0.533526 | 0.656749 | 0.001387 | 0.001975 | 0.001460 |
| minus_eq_vv_test.test | RUN | 0.237163 | 0.518570 | 0.603678 | 0.001441 | 0.001705 | 0.001435 |
| minus_vv_test.test | RUN | 0.241013 | 0.561809 | 0.685787 | 0.001731 | 0.001792 | 0.001932 |
| mul_eq_vs_test.test | RUN | 0.233522 | 0.459024 | 0.627516 | 0.001432 | 0.001773 | 0.001882 |
| mul_vs_test.test | RUN | 0.247132 | 0.551536 | 0.674034 | 0.001620 | 0.001822 | 0.001646 |
| mul_sv_test.test | RUN | 0.248581 | 0.553643 | 0.662159 | 0.001560 | 0.001964 | 0.001932 |
| mul_vm_test.test | RUN | 0.353702 | 0.947453 | 0.975668 | 0.001483 | 0.001768 | 0.001868 |
| normalize_v_test.test | RUN | 0.256980 | 0.666806 | 0.670587 | 0.001555 | 0.002666 | 0.001587 |
| plus_eq_vv_test.test | RUN | 0.240620 | 0.494222 | 0.620830 | 0.001252 | 0.001619 | 0.001742 |
| plus_vv_test.test | RUN | 0.253719 | 0.586093 | 0.657352 | 0.001459 | 0.001880 | 0.001589 |
| scalar_cast_v_test.test | RUN | 0.235070 | 0.425909 | 0.569824 | 0.001628 | 0.002060 | 0.001535 |
| vec_index_test.test | RUN | 0.210921 | 0.421795 | 0.482553 | 0.001684 | 0.001550 | 0.002014 |
| vec_register_test.test | RUN | 0.132728 | 0.391883 | 0.362367 | 0.001961 | 0.001551 | 0.001910 |
| cmp_mm_test.test | RUN | 0.376399 | 1.232381 | 1.314811 | 0.001824 | 0.001644 | 0.001794 |
| determinant_test.test | RUN | 0.270344 | 0.603302 | 0.702911 | 0.001927 | 0.001485 | 0.001930 |
| div_eq_ms_test.test | RUN | 0.277785 | 0.564822 | 0.780392 | 0.001693 | 0.001776 | 0.001812 |
| div_ms_test.test | RUN | 0.309517 | 0.924303 | 0.976839 | 0.001475 | 0.001586 | 0.001644 |
| eq_mm_test.test | RUN | 0.354278 | 1.115871 | 1.224073 | 0.001770 | 0.001906 | 0.001937 |
| minus_m_test.test | RUN | 0.310700 | 0.907107 | 0.930978 | 0.002040 | 0.001924 | 0.001775 |
| minus_eq_mm_test.test | RUN | 0.294560 | 0.859453 | 0.871299 | 0.001733 | 0.001899 | 0.001876 |
| minus_mm_test.test | RUN | 0.379535 | 1.079931 | 1.003493 | 0.001935 | 0.001742 | 0.002059 |
| mul_eq_mm_test.test | RUN | 0.357817 | 0.805328 | 0.767762 | 0.001797 | 0.001510 | 0.001876 |
| mul_eq_ms_test.test | RUN | 0.283420 | 0.589318 | 0.805080 | 0.001810 | 0.001482 | 0.001758 |
| mul_mm_test.test | RUN | 0.367958 | 1.687864 | 1.303040 | 0.001678 | 0.001676 | 0.001677 |
| mul_ms_test.test | RUN | 0.320656 | 0.929372 | 1.057015 | 0.001478 | 0.001883 | 0.001845 |
| mul_sm_test.test | RUN | 0.320528 | 0.920367 | 1.009790 | 0.001982 | 0.001866 | 0.001858 |
| mul_mv_test.test | RUN | 0.358256 | 0.973026 | 1.024728 | 0.001777 | 0.001686 | 0.001910 |
| inverse_m_test.test | RUN | 0.427045 | 1.826438 | 1.482009 | 0.001862 | 0.001825 | 0.001686 |
| plus_eq_mm_test.test | RUN | 0.291885 | 0.821120 | 0.880411 | 0.001648 | 0.001950 | 0.001918 |
| plus_mm_test.test | RUN | 0.334346 | 1.068625 | 1.044580 | 0.001814 | 0.001996 | 0.001788 |
| scalar_cast_m_test.test | RUN | 0.268848 | 0.489578 | 0.663179 | 0.001570 | 0.001916 | 0.001730 |
| mat_index_test.test | RUN | 0.210449 | 0.485631 | 0.566843 | 0.001543 | 0.002064 | 0.001942 |
| cmp_qq_test.test | RUN | 0.225035 | 0.469955 | 0.532210 | 0.001641 | 0.001752 | 0.001929 |
| conjugate_test.test | RUN | 0.233339 | 0.452098 | 0.548941 | 0.001808 | 0.001900 | 0.001866 |
| normalize_q_test.test | RUN | 0.233730 | 0.502940 | 0.573338 | 0.001543 | 0.001413 | 0.001781 |
| div_eq_qs_test.test | RUN | 0.224100 | 0.420176 | 0.526281 | 0.001562 | 0.001684 | 0.001986 |
| div_qs_test.test | RUN | 0.225954 | 0.456051 | 0.551075 | 0.001605 | 0.002041 | 0.001764 |
| dot_qq_test.test | RUN | 0.229343 | 0.426470 | 0.550096 | 0.001758 | 0.001893 | 0.001517 |
| eq_qq_test.test | RUN | 0.224275 | 0.456155 | 0.559514 | 0.001869 | 0.001535 | 0.001784 |
| inverse_q_test.test | RUN | 0.266506 | 0.587679 | 0.699052 | 0.001515 | 0.001774 | 0.001501 |
| mag_q_test.test | RUN | 0.228585 | 0.441727 | 0.549099 | 0.001516 | 0.001491 | 0.001931 |
| mag_sqr_q_test.test | RUN | 0.227772 | 0.439643 | 0.552400 | 0.001739 | 0.001868 | 0.001848 |
| minus_q_test.test | RUN | 0.228716 | 0.439881 | 0.511957 | 0.001784 | 0.001513 | 0.001570 |
| minus_eq_qq_test.test | RUN | 0.227322 | 0.435649 | 0.529281 | 0.001566 | 0.001711 | 0.001906 |
| minus_qq_test.test | RUN | 0.233456 | 0.456406 | 0.537213 | 0.001712 | 0.001670 | 0.001970 |
| mul_eq_qs_test.test | RUN | 0.231303 | 0.428027 | 0.524919 | 0.001738 | 0.001958 | 0.001891 |
| mul_qs_test.test | RUN | 0.236237 | 0.449840 | 0.537993 | 0.001568 | 0.001902 | 0.001624 |
| mul_qv_test.test | RUN | 0.278230 | 0.608817 | 0.715472 | 0.001662 | 0.001972 | 0.002038 |
| mul_qq_test.test | RUN | 0.279274 | 0.605480 | 0.710100 | 0.001789 | 0.001646 | 0.001727 |
| mul_eq_qq_test.test | RUN | 0.279020 | 0.594468 | 0.696531 | 0.001674 | 0.001890 | 0.001892 |
| plus_eq_qq_test.test | RUN | 0.228860 | 0.448238 | 0.569113 | 0.001667 | 0.001888 | 0.001638 |
| plus_qq_test.test | RUN | 0.229870 | 0.457954 | 0.537154 | 0.001764 | 0.001711 | 0.001858 |
| scalar_cast_q_test.test | RUN | 0.229752 | 0.419541 | 0.540193 | 0.001686 | 0.001609 | 0.001818 |
| slerp_test.test | RUN | 0.249848 | 0.822132 | 0.634875 | 0.016623 | 0.012380 | 0.013582 |
| convert_to_test.test | RUN | 0.328381 | 0.751100 | 1.062647 | 0.001784 | 0.001615 | 0.001870 |
| rot_mat_test.test | RUN | 0.541816 | 3.470251 | 3.472847 | 0.052022 | 0.032007 | 0.029668 |
| rot_quat_test.test | RUN | 0.290355 | 0.701521 | 0.713718 | 0.001798 | 0.002020 | 0.001906 |
| rotx_mat_test.test | RUN | 0.326336 | 0.909977 | 0.954181 | 0.001989 | 0.001975 | 0.002042 |
| rotx_quat_test.test | RUN | 0.293058 | 0.593502 | 0.673415 | 0.001830 | 0.001852 | 0.004015 |
| roty_mat_test.test | RUN | 0.326943 | 0.918367 | 0.982061 | 0.001724 | 0.001916 | 0.001813 |
| roty_quat_test.test | RUN | 0.288039 | 0.619132 | 0.672295 | 0.001857 | 0.001882 | 0.001927 |
| rotz_mat_test.test | RUN | 0.327716 | 1.066210 | 1.000646 | 0.001878 | 0.001929 | 0.001798 |
| rotz_quat_test.test | RUN | 0.282750 | 0.642084 | 0.672031 | 0.001779 | 0.001896 | 0.001864 |
| zero_vec_test.test | RUN | 0.255659 | 0.562420 | 0.603451 | 0.001736 | 0.001670 | 0.001640 |
| zero_mat_test.test | RUN | 0.323660 | 0.874412 | 0.779352 | 0.001560 | 0.001699 | 0.001659 |
| zero_quat_test.test | RUN | 0.227830 | 0.446276 | 0.548945 | 0.001590 | 0.001609 | 0.001750 |
| identity_mat_test.test | RUN | 0.281208 | 0.666840 | 0.706006 | 0.001698 | 0.001772 | 0.002289 |
| identity_quat_test.test | RUN | 0.229164 | 0.447985 | 0.557292 | 0.001581 | 0.001910 | 0.001835 |
| col_test.test | RUN | 0.400735 | 1.382520 | 1.541827 | 0.001568 | 0.001887 | 0.001864 |
| col_mat_test.test | RUN | 0.280086 | 0.607141 | 0.737929 | 0.001706 | 0.001870 | 0.001869 |
| neg_col_test.test | RUN | 0.313648 | 0.842350 | 0.934379 | 0.001474 | 0.002028 | 0.001765 |
| neg_row_test.test | RUN | 0.322870 | 0.888682 | 0.940234 | 0.001791 | 0.001930 | 0.001842 |
| del_col_test.test | RUN | 0.321667 | 0.943450 | 0.981041 | 0.001626 | 0.001893 | 0.001832 |
| del_row_test.test | RUN | 0.323079 | 0.901809 | 0.975768 | 0.001770 | 0.001870 | 0.001873 |
| del_row_col_test.test | RUN | 0.434972 | 1.668691 | 1.852540 | 0.001948 | 0.001762 | 0.001838 |
| swap_cols_test.test | RUN | 0.624790 | 2.209452 | 2.291211 | 0.001715 | 0.001586 | 0.001936 |
| swap_rows_test.test | RUN | 0.688002 | 2.322503 | 2.327124 | 0.001684 | 0.001838 | 0.002014 |
| diag_test.test | RUN | 0.331481 | 0.618081 | 0.879337 | 0.001762 | 0.001606 | 0.001966 |
| diag_mat_test.test | RUN | 0.305571 | 0.590563 | 0.720271 | 0.001688 | 0.001733 | 0.001827 |
| row_test.test | RUN | 0.402003 | 1.277541 | 1.602931 | 0.001750 | 0.001607 | 0.002015 |
| row_mat_test.test | RUN | 0.278336 | 0.595471 | 0.714308 | 0.001565 | 0.001706 | 0.001860 |
| transpose_test.test | RUN | 0.319269 | 0.869057 | 1.044764 | 0.001947 | 0.001574 | 0.001834 |
| translation_test.test | RUN | 0.265164 | 0.562029 | 0.770048 | 0.001820 | 0.001669 | 0.002024 |
| translation_mat_test.test | RUN | 0.293412 | 0.585027 | 0.876592 | 0.001613 | 0.001546 | 0.001808 |
| swizzle_const_fail.test | COMPILE_FAIL | 0.101609 | 0.163200 | 0.206669 | — | — | — |
| swizzle2_test2.test | RUN | 0.121227 | 0.244534 | 0.353342 | 0.001615 | 0.001699 | 0.001810 |
| swizzle2_test3.test | RUN | 0.121126 | 0.247020 | 0.349046 | 0.001523 | 0.001489 | 0.001632 |
| swizzle2_test4.test | RUN | 0.122234 | 0.246606 | 0.348936 | 0.001457 | 0.001892 | 0.001680 |
| swizzle2_test.test | RUN | 0.387606 | 0.668735 | 0.947425 | 0.001889 | 0.001549 | 0.001858 |
| swizzle3_test2.test | RUN | 0.136310 | 0.270405 | 0.340927 | 0.001865 | 0.001652 | 0.001767 |
| swizzle3_test3.test | RUN | 0.133482 | 0.273030 | 0.332234 | 0.001524 | 0.001517 | 0.001610 |
| swizzle3_test4.test | RUN | 0.137056 | 0.264010 | 0.326394 | 0.001754 | 0.001808 | 0.001540 |
| swizzle3_test.test | RUN | 0.382797 | 0.669639 | 0.922194 | 0.001638 | 0.001824 | 0.001591 |
| swizzle4_test2.test | RUN | 0.228498 | 0.365811 | 0.519853 | 0.001744 | 0.001808 | 0.001600 |
| swizzle4_test3.test | RUN | 0.229029 | 0.357245 | 0.512799 | 0.001797 | 0.001516 | 0.001598 |
| swizzle4_test4.test | RUN | 0.233711 | 0.374460 | 0.534542 | 0.001406 | 0.001680 | 0.001880 |
| swizzle4_test.test | RUN | 0.400892 | 0.725581 | 0.957935 | 0.001625 | 0.001521 | 0.001886 |
| deduce_scalar_test.test | COMPILE | 0.019780 | 0.037239 | 0.057090 | — | — | — |
| deduce_scalar_mq_test.test | RUN | 0.154137 | 0.280352 | 0.371251 | 0.001657 | 0.001543 | 0.001935 |
| deduce_scalar_mv_test.test | RUN | 0.186218 | 0.319091 | 0.431879 | 0.001527 | 0.001723 | 0.001640 |
| deduce_scalar_fail.test | COMPILE_FAIL | 0.018446 | 0.024207 | 0.047579 | — | — | — |
| deduce_mat_test.test | COMPILE | 0.019348 | 0.033226 | 0.052544 | — | — | — |
| deduce_quat_test.test | COMPILE | 0.019723 | 0.032361 | 0.051006 | — | — | — |
| deduce_vec_test.test | COMPILE | 0.019225 | 0.033585 | 0.050187 | — | — | — |
| interop_test.test | RUN | 0.170327 | 0.398170 | 0.620342 | 0.000663 | 0.000861 | 0.001479 |
| transform_test.test | RUN | 0.369401 | 0.527303 | 0.871820 | 0.001436 | 0.001271 | 0.001416 |
| projection_test.test | RUN | 0.251631 | 0.474895 | 0.483368 | 0.001370 | 0.001158 | 0.001439 |
| scalar_traits_test.test | COMPILE | 0.019963 | 0.032639 | 0.054091 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.913× | 0.910×, 0.916× | 0.973× |
| clang | 0.897× | 0.899×, 0.894× | 1.018× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| access_m_test.test | 10 | 0.000962s | 0.000969s | 0.000978s | byte equality observed |
| access_q_test.test | 10 | 0.000960s | 0.000974s | 0.000996s | byte equality observed |
| access_v_test.test | 10 | 0.000945s | 0.000984s | 0.000966s | byte equality observed |
| assign_test.test | 10 | 0.000975s | 0.000968s | 0.000953s | byte equality observed |
| cmp_mm_test.test | 10 | 0.000985s | 0.000988s | 0.000973s | byte equality observed |
| cmp_qq_test.test | 10 | 0.000952s | 0.000966s | 0.000973s | byte equality observed |
| cmp_vv_test.test | 10 | 0.000940s | 0.000969s | 0.000948s | byte equality observed |
| col_mat_test.test | 10 | 0.000949s | 0.000938s | 0.000950s | byte equality observed |
| col_test.test | 10 | 0.000956s | 0.000958s | 0.000981s | byte equality observed |
| conjugate_test.test | 10 | 0.000960s | 0.000947s | 0.000961s | byte equality observed |
| convert_to_test.test | 10 | 0.000949s | 0.000963s | 0.000959s | byte equality observed |
| cross_test.test | 10 | 0.000953s | 0.000974s | 0.000944s | byte equality observed |
| deduce_scalar_mq_test.test | 10 | 0.000954s | 0.000948s | 0.000949s | byte equality observed |
| deduce_scalar_mv_test.test | 10 | 0.000959s | 0.000967s | 0.000964s | byte equality observed |
| del_col_test.test | 10 | 0.000980s | 0.000991s | 0.000949s | byte equality observed |
| del_row_col_test.test | 10 | 0.000970s | 0.001010s | 0.000942s | byte equality observed |
| del_row_test.test | 10 | 0.000954s | 0.000976s | 0.000949s | byte equality observed |
| determinant_test.test | 10 | 0.000968s | 0.000980s | 0.000982s | byte equality observed |
| diag_mat_test.test | 10 | 0.000966s | 0.000974s | 0.000967s | byte equality observed |
| diag_test.test | 10 | 0.000941s | 0.000956s | 0.000948s | byte equality observed |
| div_eq_ms_test.test | 10 | 0.000958s | 0.000951s | 0.000947s | byte equality observed |
| div_eq_qs_test.test | 10 | 0.000948s | 0.000987s | 0.000959s | byte equality observed |
| div_eq_vs_test.test | 10 | 0.000962s | 0.000980s | 0.000963s | byte equality observed |
| div_ms_test.test | 10 | 0.000957s | 0.000964s | 0.000972s | byte equality observed |
| div_qs_test.test | 10 | 0.000975s | 0.000955s | 0.000968s | byte equality observed |
| div_vs_test.test | 10 | 0.000961s | 0.000939s | 0.000952s | byte equality observed |
| dot_qq_test.test | 10 | 0.000946s | 0.000942s | 0.000953s | byte equality observed |
| dot_vv_test.test | 10 | 0.000963s | 0.000973s | 0.000943s | byte equality observed |
| eq_mm_test.test | 10 | 0.000964s | 0.000968s | 0.000965s | byte equality observed |
| eq_qq_test.test | 10 | 0.000952s | 0.000972s | 0.000954s | byte equality observed |
| eq_vv_test.test | 10 | 0.000945s | 0.000951s | 0.000965s | byte equality observed |
| identity_mat_test.test | 10 | 0.000961s | 0.001013s | 0.000974s | byte equality observed |
| identity_quat_test.test | 10 | 0.000966s | 0.000983s | 0.000951s | byte equality observed |
| interop_test.test | 10 | 0.000448s | 0.000458s | 0.000918s | byte equality observed |
| inverse_m_test.test | 10 | 0.001049s | 0.001050s | 0.001005s | byte equality observed |
| inverse_q_test.test | 10 | 0.000986s | 0.000999s | 0.000942s | byte equality observed |
| mag_q_test.test | 10 | 0.000955s | 0.000988s | 0.000965s | byte equality observed |
| mag_sqr_q_test.test | 10 | 0.000946s | 0.000984s | 0.000954s | byte equality observed |
| mag_sqr_v_test.test | 10 | 0.000946s | 0.000999s | 0.000964s | byte equality observed |
| mag_v_test.test | 10 | 0.000979s | 0.000969s | 0.000946s | byte equality observed |
| mat_index_test.test | 10 | 0.000962s | 0.000961s | 0.000948s | byte equality observed |
| mat_traits_array_test.test | 10 | 0.000964s | 0.000978s | 0.000960s | byte equality observed |
| mat_traits_std_array_test.test | 10 | 0.000961s | 0.000972s | 0.000948s | byte equality observed |
| math_test.test | 10 | 0.001420s | 0.001407s | 0.001248s | byte equality observed |
| minus_eq_mm_test.test | 10 | 0.000942s | 0.000978s | 0.000951s | byte equality observed |
| minus_eq_qq_test.test | 10 | 0.000959s | 0.000984s | 0.000946s | byte equality observed |
| minus_eq_vv_test.test | 10 | 0.000959s | 0.000990s | 0.000945s | byte equality observed |
| minus_m_test.test | 10 | 0.000957s | 0.000978s | 0.000948s | byte equality observed |
| minus_mm_test.test | 10 | 0.000967s | 0.000974s | 0.000977s | byte equality observed |
| minus_q_test.test | 10 | 0.000973s | 0.000991s | 0.000955s | byte equality observed |
| minus_qq_test.test | 10 | 0.000961s | 0.000975s | 0.000949s | byte equality observed |
| minus_v_test.test | 10 | 0.000959s | 0.000979s | 0.000951s | byte equality observed |
| minus_vv_test.test | 10 | 0.000993s | 0.000991s | 0.000963s | byte equality observed |
| mul_eq_mm_test.test | 10 | 0.000957s | 0.000988s | 0.000983s | byte equality observed |
| mul_eq_ms_test.test | 10 | 0.000951s | 0.000947s | 0.000956s | byte equality observed |
| mul_eq_qq_test.test | 10 | 0.000985s | 0.000979s | 0.000977s | byte equality observed |
| mul_eq_qs_test.test | 10 | 0.000964s | 0.000967s | 0.000944s | byte equality observed |
| mul_eq_vs_test.test | 10 | 0.000945s | 0.000984s | 0.000945s | byte equality observed |
| mul_mm_test.test | 10 | 0.000976s | 0.000986s | 0.000972s | byte equality observed |
| mul_ms_test.test | 10 | 0.000951s | 0.000979s | 0.000973s | byte equality observed |
| mul_mv_test.test | 10 | 0.000967s | 0.000983s | 0.000954s | byte equality observed |
| mul_qq_test.test | 10 | 0.000979s | 0.001012s | 0.000963s | byte equality observed |
| mul_qs_test.test | 10 | 0.000956s | 0.000942s | 0.000973s | byte equality observed |
| mul_qv_test.test | 10 | 0.000981s | 0.000985s | 0.000972s | byte equality observed |
| mul_sm_test.test | 10 | 0.000974s | 0.000965s | 0.000953s | byte equality observed |
| mul_sv_test.test | 10 | 0.000972s | 0.000959s | 0.000939s | byte equality observed |
| mul_vm_test.test | 10 | 0.000950s | 0.000973s | 0.000956s | byte equality observed |
| mul_vs_test.test | 10 | 0.000954s | 0.000959s | 0.000958s | byte equality observed |
| neg_col_test.test | 10 | 0.000966s | 0.000959s | 0.000938s | byte equality observed |
| neg_row_test.test | 10 | 0.000978s | 0.000966s | 0.000954s | byte equality observed |
| normalize_q_test.test | 10 | 0.000959s | 0.000949s | 0.000958s | byte equality observed |
| normalize_v_test.test | 10 | 0.000992s | 0.000959s | 0.000947s | byte equality observed |
| plus_eq_mm_test.test | 10 | 0.000951s | 0.000956s | 0.000956s | byte equality observed |
| plus_eq_qq_test.test | 10 | 0.000962s | 0.000958s | 0.000960s | byte equality observed |
| plus_eq_vv_test.test | 10 | 0.000967s | 0.000968s | 0.000954s | byte equality observed |
| plus_mm_test.test | 10 | 0.000961s | 0.000987s | 0.000968s | byte equality observed |
| plus_qq_test.test | 10 | 0.000962s | 0.000967s | 0.000973s | byte equality observed |
| plus_vv_test.test | 10 | 0.000969s | 0.000968s | 0.000956s | byte equality observed |
| projection_test.test | 10 | 0.000983s | 0.000961s | 0.000963s | byte equality observed |
| quat_traits_array_test.test | 10 | 0.000950s | 0.000976s | 0.000955s | byte equality observed |
| quat_traits_std_array_test.test | 10 | 0.000938s | 0.000957s | 0.000953s | byte equality observed |
| rot_mat_test.test | 1 | 0.034057s | 0.021067s | 0.019603s | byte equality observed |
| rot_quat_test.test | 10 | 0.000932s | 0.000979s | 0.000937s | byte equality observed |
| rotx_mat_test.test | 10 | 0.000939s | 0.000975s | 0.000952s | byte equality observed |
| rotx_quat_test.test | 10 | 0.000968s | 0.000970s | 0.000978s | byte equality observed |
| roty_mat_test.test | 10 | 0.000989s | 0.000974s | 0.000973s | byte equality observed |
| roty_quat_test.test | 10 | 0.000961s | 0.000957s | 0.000978s | byte equality observed |
| rotz_mat_test.test | 10 | 0.000992s | 0.000981s | 0.000981s | byte equality observed |
| rotz_quat_test.test | 10 | 0.000966s | 0.000953s | 0.000973s | byte equality observed |
| row_mat_test.test | 10 | 0.000935s | 0.000986s | 0.000957s | byte equality observed |
| row_test.test | 10 | 0.000951s | 0.000985s | 0.000947s | byte equality observed |
| scalar_cast_m_test.test | 10 | 0.000954s | 0.000941s | 0.000972s | byte equality observed |
| scalar_cast_q_test.test | 10 | 0.000945s | 0.000958s | 0.000955s | byte equality observed |
| scalar_cast_v_test.test | 10 | 0.000941s | 0.000932s | 0.000949s | byte equality observed |
| slerp_test.test | 3 | 0.010546s | 0.008522s | 0.008918s | byte equality observed |
| swap_cols_test.test | 10 | 0.000930s | 0.000925s | 0.000945s | byte equality observed |
| swap_rows_test.test | 10 | 0.000953s | 0.000966s | 0.000956s | byte equality observed |
| swizzle2_test.test | 10 | 0.000962s | 0.000955s | 0.000954s | byte equality observed |
| swizzle2_test2.test | 10 | 0.000967s | 0.000955s | 0.000940s | byte equality observed |
| swizzle2_test3.test | 10 | 0.000950s | 0.000958s | 0.000972s | byte equality observed |
| swizzle2_test4.test | 10 | 0.000954s | 0.000964s | 0.000957s | byte equality observed |
| swizzle3_test.test | 10 | 0.000948s | 0.000978s | 0.000960s | byte equality observed |
| swizzle3_test2.test | 10 | 0.000937s | 0.000968s | 0.000946s | byte equality observed |
| swizzle3_test3.test | 10 | 0.000945s | 0.000962s | 0.000944s | byte equality observed |
| swizzle3_test4.test | 10 | 0.000949s | 0.000960s | 0.000954s | byte equality observed |
| swizzle4_test.test | 10 | 0.000948s | 0.000945s | 0.000954s | byte equality observed |
| swizzle4_test2.test | 10 | 0.000945s | 0.000953s | 0.000968s | byte equality observed |
| swizzle4_test3.test | 10 | 0.000945s | 0.000962s | 0.000968s | byte equality observed |
| swizzle4_test4.test | 10 | 0.000954s | 0.000957s | 0.000954s | byte equality observed |
| to_string_test.test | 10 | 0.000984s | 0.001012s | 0.001008s | byte equality observed |
| transform_test.test | 10 | 0.000972s | 0.000953s | 0.000954s | byte equality observed |
| translation_mat_test.test | 10 | 0.000947s | 0.000976s | 0.000986s | byte equality observed |
| translation_test.test | 10 | 0.000950s | 0.000983s | 0.000971s | byte equality observed |
| transpose_test.test | 10 | 0.000974s | 0.000992s | 0.000972s | byte equality observed |
| vec_index_test.test | 10 | 0.000946s | 0.000959s | 0.000974s | byte equality observed |
| vec_register_test.test | 10 | 0.000945s | 0.000943s | 0.000976s | byte equality observed |
| vec_traits_array_test.test | 10 | 0.000947s | 0.000946s | 0.000958s | byte equality observed |
| vec_traits_gnuc_test.test | 10 | 0.000961s | 0.000929s | 0.000954s | byte equality observed |
| vec_traits_std_array_test.test | 10 | 0.000949s | 0.000992s | 0.000983s | byte equality observed |
| zero_mat_test.test | 10 | 0.000954s | 0.000955s | 0.000989s | byte equality observed |
| zero_quat_test.test | 10 | 0.000942s | 0.000979s | 0.000965s | byte equality observed |
| zero_vec_test.test | 10 | 0.000943s | 0.000962s | 0.000955s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
