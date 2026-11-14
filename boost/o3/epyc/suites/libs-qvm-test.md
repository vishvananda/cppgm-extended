# libs/qvm/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| math_test.test | RUN | 0.252187 | 0.649759 | 0.785305 | 0.002239 | 0.002209 | 0.002190 |
| mat_traits_array_test.test | RUN | 0.199231 | 0.643525 | 0.624059 | 0.001397 | 0.001537 | 0.001570 |
| mat_traits_std_array_test.test | RUN | 0.196790 | 0.491235 | 0.620076 | 0.001696 | 0.001844 | 0.001462 |
| vec_traits_array_test.test | RUN | 0.170468 | 0.475848 | 0.525430 | 0.001517 | 0.001701 | 0.001134 |
| vec_traits_gnuc_test.test | RUN | 0.306440 | 0.649428 | 1.048445 | 0.001403 | 0.001816 | 0.001590 |
| vec_traits_std_array_test.test | RUN | 0.171129 | 0.417063 | 0.529784 | 0.001557 | 0.001824 | 0.001422 |
| quat_traits_array_test.test | RUN | 0.162089 | 0.549130 | 0.498617 | 0.001581 | 0.001529 | 0.002008 |
| quat_traits_std_array_test.test | RUN | 0.165539 | 0.437559 | 0.503237 | 0.001500 | 0.001695 | 0.002859 |
| access_m_test.test | RUN | 0.266976 | 0.948123 | 1.130520 | 0.001572 | 0.001682 | 0.001691 |
| access_m_fail1.test | COMPILE_FAIL | 0.026173 | 0.041295 | 0.103455 | — | — | — |
| access_m_fail2.test | COMPILE_FAIL | 0.025924 | 0.044444 | 0.085975 | — | — | — |
| access_q_test.test | RUN | 0.227748 | 0.485388 | 0.685636 | 0.001478 | 0.001525 | 0.001808 |
| access_q_fail.test | COMPILE_FAIL | 0.020073 | 0.032190 | 0.064190 | — | — | — |
| access_v_test.test | RUN | 0.223417 | 0.507408 | 0.676568 | 0.001443 | 0.001727 | 0.001812 |
| access_v_fail1.test | COMPILE_FAIL | 0.017893 | 0.031728 | 0.059979 | — | — | — |
| access_v_fail2.test | COMPILE_FAIL | 0.018715 | 0.030946 | 0.055021 | — | — | — |
| assign_test.test | RUN | 0.328970 | 0.769739 | 1.169593 | 0.001579 | 0.001880 | 0.001707 |
| to_string_test.test | RUN | 0.375838 | 1.266273 | 6.664037 | 0.001402 | 0.001749 | 0.001556 |
| cmp_vv_test.test | RUN | 0.277031 | 0.651299 | 0.810019 | 0.001507 | 0.001678 | 0.001698 |
| cross_test.test | RUN | 0.264662 | 0.573938 | 0.850695 | 0.001596 | 0.001804 | 0.001666 |
| div_eq_vs_test.test | RUN | 0.255573 | 0.542967 | 0.775597 | 0.001535 | 0.001886 | 0.001730 |
| div_vs_test.test | RUN | 0.262476 | 0.654285 | 0.828933 | 0.001484 | 0.001759 | 0.001724 |
| dot_vv_test.test | RUN | 0.250708 | 0.598317 | 0.788522 | 0.001602 | 0.001639 | 0.001720 |
| eq_vv_test.test | RUN | 0.277351 | 0.674939 | 0.876939 | 0.001572 | 0.001730 | 0.001651 |
| mag_v_test.test | RUN | 0.247447 | 0.547986 | 0.766999 | 0.001535 | 0.001902 | 0.001741 |
| mag_sqr_v_test.test | RUN | 0.247199 | 0.550907 | 0.731645 | 0.001403 | 0.001738 | 0.001548 |
| minus_v_test.test | RUN | 0.258652 | 0.639602 | 0.837770 | 0.001359 | 0.001903 | 0.001588 |
| minus_eq_vv_test.test | RUN | 0.253258 | 0.611659 | 0.818229 | 0.001398 | 0.001880 | 0.001791 |
| minus_vv_test.test | RUN | 0.264775 | 0.675881 | 0.810112 | 0.001506 | 0.001816 | 0.001613 |
| mul_eq_vs_test.test | RUN | 0.253182 | 0.558182 | 0.796700 | 0.001521 | 0.001731 | 0.001899 |
| mul_vs_test.test | RUN | 0.266833 | 0.654523 | 0.827975 | 0.001436 | 0.001868 | 0.001697 |
| mul_sv_test.test | RUN | 0.265897 | 0.674773 | 0.850477 | 0.001616 | 0.001854 | 0.001616 |
| mul_vm_test.test | RUN | 0.361400 | 1.205954 | 1.339734 | 0.001626 | 0.001867 | 0.001690 |
| normalize_v_test.test | RUN | 0.270603 | 0.801495 | 0.905814 | 0.001496 | 0.001583 | 0.001777 |
| plus_eq_vv_test.test | RUN | 0.253505 | 0.615131 | 0.761953 | 0.001446 | 0.001733 | 0.001821 |
| plus_vv_test.test | RUN | 0.262672 | 0.684858 | 0.831973 | 0.001413 | 0.001856 | 0.001444 |
| scalar_cast_v_test.test | RUN | 0.245676 | 0.519651 | 0.729396 | 0.001448 | 0.001690 | 0.001519 |
| vec_index_test.test | RUN | 0.211301 | 0.517987 | 0.667383 | 0.001468 | 0.001716 | 0.001741 |
| vec_register_test.test | RUN | 0.139787 | 0.464258 | 0.501562 | 0.001676 | 0.001578 | 0.001453 |
| cmp_mm_test.test | RUN | 0.396167 | 1.447302 | 1.664934 | 0.001792 | 0.001849 | 0.001777 |
| determinant_test.test | RUN | 0.295431 | 0.743781 | 0.956771 | 0.001765 | 0.001784 | 0.001621 |
| div_eq_ms_test.test | RUN | 0.301453 | 0.708497 | 1.069178 | 0.001604 | 0.001736 | 0.001772 |
| div_ms_test.test | RUN | 0.344002 | 1.094740 | 1.300379 | 0.001717 | 0.001688 | 0.001622 |
| eq_mm_test.test | RUN | 0.405698 | 1.353979 | 1.601076 | 0.001824 | 0.001681 | 0.001664 |
| minus_m_test.test | RUN | 0.343627 | 1.082739 | 1.262162 | 0.001649 | 0.001702 | 0.001731 |
| minus_eq_mm_test.test | RUN | 0.331535 | 1.012571 | 1.158320 | 0.001593 | 0.001793 | 0.001469 |
| minus_mm_test.test | RUN | 0.354169 | 1.287987 | 1.363444 | 0.001758 | 0.001812 | 0.001838 |
| mul_eq_mm_test.test | RUN | 0.313989 | 0.947356 | 1.040424 | 0.001754 | 0.001816 | 0.001726 |
| mul_eq_ms_test.test | RUN | 0.302380 | 0.707076 | 1.066254 | 0.001753 | 0.001653 | 0.001751 |
| mul_mm_test.test | RUN | 0.413877 | 2.102495 | 1.672685 | 0.001712 | 0.001540 | 0.001789 |
| mul_ms_test.test | RUN | 0.354930 | 1.107316 | 1.337586 | 0.001648 | 0.001800 | 0.001776 |
| mul_sm_test.test | RUN | 0.360473 | 1.105481 | 1.304496 | 0.001783 | 0.001854 | 0.001840 |
| mul_mv_test.test | RUN | 0.364767 | 1.196783 | 1.376697 | 0.001740 | 0.001664 | 0.001866 |
| inverse_m_test.test | RUN | 0.466306 | 2.143672 | 1.838992 | 0.001856 | 0.001844 | 0.001853 |
| plus_eq_mm_test.test | RUN | 0.326016 | 1.024755 | 1.141080 | 0.002131 | 0.001888 | 0.001808 |
| plus_mm_test.test | RUN | 0.359330 | 1.246358 | 1.362716 | 0.001766 | 0.001732 | 0.001818 |
| scalar_cast_m_test.test | RUN | 0.284362 | 0.556049 | 0.841558 | 0.001764 | 0.001804 | 0.001806 |
| mat_index_test.test | RUN | 0.229218 | 0.547073 | 0.739717 | 0.001553 | 0.001863 | 0.001781 |
| cmp_qq_test.test | RUN | 0.247619 | 0.528187 | 0.708101 | 0.001770 | 0.001864 | 0.001769 |
| conjugate_test.test | RUN | 0.249197 | 0.533737 | 0.755302 | 0.001744 | 0.001815 | 0.001797 |
| normalize_q_test.test | RUN | 0.252871 | 0.583927 | 0.780954 | 0.001742 | 0.001770 | 0.001759 |
| div_eq_qs_test.test | RUN | 0.243910 | 0.512952 | 0.697188 | 0.001573 | 0.001790 | 0.001539 |
| div_qs_test.test | RUN | 0.245318 | 0.550514 | 0.723885 | 0.001562 | 0.001909 | 0.001648 |
| dot_qq_test.test | RUN | 0.248269 | 0.522337 | 0.757045 | 0.001758 | 0.002178 | 0.001852 |
| eq_qq_test.test | RUN | 0.249721 | 0.545927 | 0.750488 | 0.001566 | 0.001851 | 0.001764 |
| inverse_q_test.test | RUN | 0.300309 | 0.725327 | 0.928608 | 0.001748 | 0.001705 | 0.001805 |
| mag_q_test.test | RUN | 0.247589 | 0.539016 | 0.727266 | 0.001633 | 0.001803 | 0.001764 |
| mag_sqr_q_test.test | RUN | 0.246070 | 0.530276 | 0.733535 | 0.001787 | 0.001842 | 0.001768 |
| minus_q_test.test | RUN | 0.248333 | 0.533136 | 0.726406 | 0.001825 | 0.001806 | 0.001779 |
| minus_eq_qq_test.test | RUN | 0.243648 | 0.542954 | 0.765257 | 0.001635 | 0.001695 | 0.001766 |
| minus_qq_test.test | RUN | 0.248655 | 0.531972 | 0.746199 | 0.001409 | 0.001630 | 0.001734 |
| mul_eq_qs_test.test | RUN | 0.243923 | 0.509154 | 0.736456 | 0.001274 | 0.001830 | 0.001780 |
| mul_qs_test.test | RUN | 0.249767 | 0.526207 | 0.745806 | 0.001792 | 0.001900 | 0.001807 |
| mul_qv_test.test | RUN | 0.309139 | 0.719732 | 0.964280 | 0.001694 | 0.001915 | 0.001787 |
| mul_qq_test.test | RUN | 0.299769 | 0.744493 | 0.936597 | 0.001832 | 0.001811 | 0.001649 |
| mul_eq_qq_test.test | RUN | 0.291591 | 0.736820 | 0.939236 | 0.001682 | 0.001678 | 0.001754 |
| plus_eq_qq_test.test | RUN | 0.244974 | 0.534700 | 0.771207 | 0.001768 | 0.001836 | 0.001611 |
| plus_qq_test.test | RUN | 0.246814 | 0.539220 | 0.710515 | 0.001896 | 0.001753 | 0.001754 |
| scalar_cast_q_test.test | RUN | 0.244405 | 0.499961 | 0.733536 | 0.001730 | 0.001698 | 0.001775 |
| slerp_test.test | RUN | 0.272218 | 1.011973 | 0.873263 | 0.018053 | 0.014931 | 0.015610 |
| convert_to_test.test | RUN | 0.363438 | 0.943668 | 1.376068 | 0.001820 | 0.001865 | 0.001554 |
| rot_mat_test.test | RUN | 0.544745 | 4.146012 | 4.101016 | 0.041267 | 0.039980 | 0.035704 |
| rot_quat_test.test | RUN | 0.296884 | 0.850521 | 0.955179 | 0.001598 | 0.001894 | 0.001854 |
| rotx_mat_test.test | RUN | 0.332433 | 1.125866 | 1.240190 | 0.001753 | 0.001868 | 0.001779 |
| rotx_quat_test.test | RUN | 0.299665 | 0.726570 | 0.888189 | 0.001884 | 0.001883 | 0.001832 |
| roty_mat_test.test | RUN | 0.347690 | 1.133571 | 1.210986 | 0.001672 | 0.001786 | 0.001867 |
| roty_quat_test.test | RUN | 0.307999 | 0.730898 | 0.820331 | 0.001796 | 0.001940 | 0.001681 |
| rotz_mat_test.test | RUN | 0.357228 | 1.265805 | 1.302838 | 0.001658 | 0.001780 | 0.001796 |
| rotz_quat_test.test | RUN | 0.298608 | 0.729236 | 0.893547 | 0.001542 | 0.001806 | 0.001636 |
| zero_vec_test.test | RUN | 0.254171 | 0.634518 | 0.734113 | 0.001875 | 0.001600 | 0.001547 |
| zero_mat_test.test | RUN | 0.320730 | 1.055519 | 1.059245 | 0.001652 | 0.001806 | 0.001631 |
| zero_quat_test.test | RUN | 0.239487 | 0.523910 | 0.764732 | 0.001534 | 0.002941 | 0.001561 |
| identity_mat_test.test | RUN | 0.291476 | 0.812965 | 0.962934 | 0.001413 | 0.001794 | 0.001451 |
| identity_quat_test.test | RUN | 0.238751 | 0.504044 | 0.724537 | 0.001770 | 0.001815 | 0.001512 |
| col_test.test | RUN | 0.472170 | 1.589141 | 2.108675 | 0.001413 | 0.001594 | 0.001846 |
| col_mat_test.test | RUN | 0.313598 | 0.740043 | 1.020499 | 0.001522 | 0.001827 | 0.001796 |
| neg_col_test.test | RUN | 0.352772 | 1.028445 | 1.297200 | 0.001747 | 0.001651 | 0.001662 |
| neg_row_test.test | RUN | 0.357409 | 1.048752 | 1.287050 | 0.001751 | 0.001673 | 0.001455 |
| del_col_test.test | RUN | 0.370866 | 1.139517 | 1.290410 | 0.001751 | 0.001763 | 0.001814 |
| del_row_test.test | RUN | 0.362831 | 1.109286 | 1.336970 | 0.001720 | 0.001877 | 0.001782 |
| del_row_col_test.test | RUN | 0.507968 | 2.113287 | 2.469436 | 0.001818 | 0.001858 | 0.001870 |
| swap_cols_test.test | RUN | 0.669750 | 2.840822 | 3.073087 | 0.001847 | 0.001575 | 0.001464 |
| swap_rows_test.test | RUN | 0.725165 | 2.976343 | 3.063351 | 0.001909 | 0.001593 | 0.001926 |
| diag_test.test | RUN | 0.305435 | 0.771838 | 1.066172 | 0.001777 | 0.001826 | 0.001803 |
| diag_mat_test.test | RUN | 0.326172 | 0.760675 | 0.962449 | 0.001616 | 0.001780 | 0.001494 |
| row_test.test | RUN | 0.458692 | 1.605262 | 2.043379 | 0.001737 | 0.001870 | 0.001521 |
| row_mat_test.test | RUN | 0.298034 | 0.703936 | 1.011601 | 0.001647 | 0.001634 | 0.001811 |
| transpose_test.test | RUN | 0.360832 | 1.100184 | 1.313505 | 0.001706 | 0.001828 | 0.001720 |
| translation_test.test | RUN | 0.285055 | 0.703183 | 0.967585 | 0.001511 | 0.001558 | 0.001771 |
| translation_mat_test.test | RUN | 0.313827 | 0.704768 | 1.081075 | 0.001788 | 0.001753 | 0.001752 |
| swizzle_const_fail.test | COMPILE_FAIL | 0.106872 | 0.216307 | 0.324247 | — | — | — |
| swizzle2_test2.test | RUN | 0.129197 | 0.301796 | 0.400888 | 0.001545 | 0.001575 | 0.001698 |
| swizzle2_test3.test | RUN | 0.131497 | 0.303990 | 0.395564 | 0.001325 | 0.001614 | 0.001813 |
| swizzle2_test4.test | RUN | 0.127552 | 0.299986 | 0.421220 | 0.001502 | 0.001576 | 0.001553 |
| swizzle2_test.test | RUN | 0.415269 | 0.845675 | 1.366220 | 0.001677 | 0.001694 | 0.001838 |
| swizzle3_test2.test | RUN | 0.145297 | 0.320059 | 0.481555 | 0.001769 | 0.001678 | 0.001684 |
| swizzle3_test3.test | RUN | 0.145611 | 0.320110 | 0.458613 | 0.001557 | 0.001408 | 0.001565 |
| swizzle3_test4.test | RUN | 0.142538 | 0.331586 | 0.451407 | 0.001502 | 0.001724 | 0.001784 |
| swizzle3_test.test | RUN | 0.406641 | 0.857449 | 1.352856 | 0.001584 | 0.001731 | 0.001823 |
| swizzle4_test2.test | RUN | 0.243229 | 0.468496 | 0.790271 | 0.001330 | 0.001880 | 0.001870 |
| swizzle4_test3.test | RUN | 0.237692 | 0.464352 | 0.826485 | 0.001678 | 0.001481 | 0.001961 |
| swizzle4_test4.test | RUN | 0.251044 | 0.476947 | 0.779936 | 0.001725 | 0.001609 | 0.001812 |
| swizzle4_test.test | RUN | 0.417555 | 0.914944 | 1.442902 | 0.001602 | 0.001645 | 0.001818 |
| deduce_scalar_test.test | COMPILE | 0.021902 | 0.048139 | 0.073776 | — | — | — |
| deduce_scalar_mq_test.test | RUN | 0.159296 | 0.353502 | 0.442664 | 0.001628 | 0.002076 | 0.001443 |
| deduce_scalar_mv_test.test | RUN | 0.191045 | 0.426155 | 0.621541 | 0.001662 | 0.001566 | 0.001784 |
| deduce_scalar_fail.test | COMPILE_FAIL | 0.017813 | 0.031303 | 0.058361 | — | — | — |
| deduce_mat_test.test | COMPILE | 0.019463 | 0.042933 | 0.054812 | — | — | — |
| deduce_quat_test.test | COMPILE | 0.019455 | 0.040950 | 0.057045 | — | — | — |
| deduce_vec_test.test | COMPILE | 0.019171 | 0.039870 | 0.057818 | — | — | — |
| interop_test.test | RUN | 0.198271 | 0.506620 | 0.843248 | 0.000812 | 0.000748 | 0.001336 |
| transform_test.test | RUN | 0.356121 | 0.613461 | 1.238917 | 0.001281 | 0.001282 | 0.001278 |
| projection_test.test | RUN | 0.261693 | 0.580088 | 0.717319 | 0.001339 | 0.001082 | 0.001400 |
| scalar_traits_test.test | COMPILE | 0.020558 | 0.038971 | 0.067265 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.937×† | 0.898×, 0.976× | 1.005× |
| clang | 0.932×† | 0.878×, 0.986× | 0.924× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267198).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| access_m_test.test | 10 | 0.001025s | 0.000995s | 0.001002s | byte equality observed |
| access_q_test.test | 10 | 0.001049s | 0.001019s | 0.001011s | byte equality observed |
| access_v_test.test | 10 | 0.001006s | 0.001019s | 0.000978s | byte equality observed |
| assign_test.test | 10 | 0.001028s | 0.001022s | 0.001044s | byte equality observed |
| cmp_mm_test.test | 10 | 0.001056s | 0.001024s | 0.001028s | byte equality observed |
| cmp_qq_test.test | 10 | 0.001042s | 0.001045s | 0.001009s | byte equality observed |
| cmp_vv_test.test | 10 | 0.001043s | 0.001030s | 0.001018s | byte equality observed |
| col_mat_test.test | 10 | 0.001053s | 0.000994s | 0.000992s | byte equality observed |
| col_test.test | 10 | 0.001026s | 0.001039s | 0.001005s | byte equality observed |
| conjugate_test.test | 10 | 0.001051s | 0.001006s | 0.001017s | byte equality observed |
| convert_to_test.test | 10 | 0.001020s | 0.001018s | 0.001028s | byte equality observed |
| cross_test.test | 10 | 0.001021s | 0.001024s | 0.001032s | byte equality observed |
| deduce_scalar_mq_test.test | 10 | 0.001015s | 0.001002s | 0.001031s | byte equality observed |
| deduce_scalar_mv_test.test | 10 | 0.001009s | 0.001015s | 0.001006s | byte equality observed |
| del_col_test.test | 10 | 0.001005s | 0.000992s | 0.001029s | byte equality observed |
| del_row_col_test.test | 10 | 0.001041s | 0.001023s | 0.001009s | byte equality observed |
| del_row_test.test | 10 | 0.001016s | 0.001038s | 0.001013s | byte equality observed |
| determinant_test.test | 10 | 0.001053s | 0.001021s | 0.001046s | byte equality observed |
| diag_mat_test.test | 10 | 0.000999s | 0.001025s | 0.001024s | byte equality observed |
| diag_test.test | 10 | 0.001015s | 0.001018s | 0.001015s | byte equality observed |
| div_eq_ms_test.test | 10 | 0.001069s | 0.001000s | 0.001056s | byte equality observed |
| div_eq_qs_test.test | 10 | 0.001032s | 0.001015s | 0.001039s | byte equality observed |
| div_eq_vs_test.test | 10 | 0.001037s | 0.001016s | 0.001008s | byte equality observed |
| div_ms_test.test | 10 | 0.001051s | 0.001005s | 0.001018s | byte equality observed |
| div_qs_test.test | 10 | 0.001027s | 0.000995s | 0.001060s | byte equality observed |
| div_vs_test.test | 10 | 0.001038s | 0.000989s | 0.001036s | byte equality observed |
| dot_qq_test.test | 10 | 0.001047s | 0.001004s | 0.001064s | byte equality observed |
| dot_vv_test.test | 10 | 0.001022s | 0.001013s | 0.000985s | byte equality observed |
| eq_mm_test.test | 10 | 0.001019s | 0.001004s | 0.001031s | byte equality observed |
| eq_qq_test.test | 10 | 0.001022s | 0.000999s | 0.001016s | byte equality observed |
| eq_vv_test.test | 10 | 0.001021s | 0.001016s | 0.000999s | byte equality observed |
| identity_mat_test.test | 10 | 0.001006s | 0.000996s | 0.001006s | byte equality observed |
| identity_quat_test.test | 10 | 0.001024s | 0.001031s | 0.000994s | byte equality observed |
| interop_test.test | 10 | 0.000471s | 0.000455s | 0.000998s | byte equality observed |
| inverse_m_test.test | 10 | 0.001098s | 0.001064s | 0.001082s | byte equality observed |
| inverse_q_test.test | 10 | 0.001061s | 0.001019s | 0.001010s | byte equality observed |
| mag_q_test.test | 10 | 0.001040s | 0.000991s | 0.000991s | byte equality observed |
| mag_sqr_q_test.test | 10 | 0.001006s | 0.000969s | 0.001000s | byte equality observed |
| mag_sqr_v_test.test | 10 | 0.001016s | 0.000989s | 0.001000s | byte equality observed |
| mag_v_test.test | 10 | 0.001042s | 0.000991s | 0.000995s | byte equality observed |
| mat_index_test.test | 10 | 0.001026s | 0.000984s | 0.001007s | byte equality observed |
| mat_traits_array_test.test | 10 | 0.001004s | 0.001004s | 0.001016s | byte equality observed |
| mat_traits_std_array_test.test | 10 | 0.001031s | 0.001006s | 0.001032s | byte equality observed |
| math_test.test | 10 | 0.001546s | 0.001485s | 0.001331s | byte equality observed |
| minus_eq_mm_test.test | 10 | 0.001040s | 0.001012s | 0.001013s | byte equality observed |
| minus_eq_qq_test.test | 10 | 0.001027s | 0.001028s | 0.001020s | byte equality observed |
| minus_eq_vv_test.test | 10 | 0.001023s | 0.001010s | 0.000991s | byte equality observed |
| minus_m_test.test | 10 | 0.001034s | 0.001010s | 0.001002s | byte equality observed |
| minus_mm_test.test | 10 | 0.001023s | 0.001063s | 0.001002s | byte equality observed |
| minus_q_test.test | 10 | 0.001033s | 0.001045s | 0.000995s | byte equality observed |
| minus_qq_test.test | 10 | 0.001078s | 0.001017s | 0.000980s | byte equality observed |
| minus_v_test.test | 10 | 0.001038s | 0.001031s | 0.000995s | byte equality observed |
| minus_vv_test.test | 10 | 0.001025s | 0.001010s | 0.001040s | byte equality observed |
| mul_eq_mm_test.test | 10 | 0.001028s | 0.001025s | 0.000991s | byte equality observed |
| mul_eq_ms_test.test | 10 | 0.001073s | 0.001003s | 0.001016s | byte equality observed |
| mul_eq_qq_test.test | 10 | 0.001068s | 0.001036s | 0.001058s | byte equality observed |
| mul_eq_qs_test.test | 10 | 0.001052s | 0.001006s | 0.001009s | byte equality observed |
| mul_eq_vs_test.test | 10 | 0.001038s | 0.001007s | 0.001023s | byte equality observed |
| mul_mm_test.test | 10 | 0.001058s | 0.001032s | 0.001007s | byte equality observed |
| mul_ms_test.test | 10 | 0.001031s | 0.001045s | 0.001009s | byte equality observed |
| mul_mv_test.test | 10 | 0.001059s | 0.001035s | 0.001009s | byte equality observed |
| mul_qq_test.test | 10 | 0.001074s | 0.001005s | 0.001035s | byte equality observed |
| mul_qs_test.test | 10 | 0.001046s | 0.001024s | 0.001019s | byte equality observed |
| mul_qv_test.test | 10 | 0.001036s | 0.001029s | 0.000998s | byte equality observed |
| mul_sm_test.test | 10 | 0.001048s | 0.001022s | 0.001009s | byte equality observed |
| mul_sv_test.test | 10 | 0.001014s | 0.001014s | 0.000998s | byte equality observed |
| mul_vm_test.test | 10 | 0.001007s | 0.001040s | 0.001050s | byte equality observed |
| mul_vs_test.test | 10 | 0.001026s | 0.001032s | 0.001015s | byte equality observed |
| neg_col_test.test | 10 | 0.001031s | 0.001006s | 0.001010s | byte equality observed |
| neg_row_test.test | 10 | 0.001035s | 0.001027s | 0.000993s | byte equality observed |
| normalize_q_test.test | 10 | 0.001062s | 0.001007s | 0.001008s | byte equality observed |
| normalize_v_test.test | 10 | 0.001051s | 0.001005s | 0.001007s | byte equality observed |
| plus_eq_mm_test.test | 10 | 0.001012s | 0.001041s | 0.001007s | byte equality observed |
| plus_eq_qq_test.test | 10 | 0.001010s | 0.000993s | 0.001019s | byte equality observed |
| plus_eq_vv_test.test | 10 | 0.001048s | 0.001039s | 0.001001s | byte equality observed |
| plus_mm_test.test | 10 | 0.001038s | 0.001003s | 0.000986s | byte equality observed |
| plus_qq_test.test | 10 | 0.001016s | 0.001049s | 0.001023s | byte equality observed |
| plus_vv_test.test | 10 | 0.001022s | 0.000993s | 0.000992s | byte equality observed |
| projection_test.test | 10 | 0.001043s | 0.000999s | 0.000988s | byte equality observed |
| quat_traits_array_test.test | 10 | 0.001010s | 0.001007s | 0.000987s | byte equality observed |
| quat_traits_std_array_test.test | 10 | 0.001009s | 0.001014s | 0.000962s | byte equality observed |
| rot_mat_test.test | 1 | 0.028713s | 0.025605s | 0.025568s | byte equality observed |
| rot_quat_test.test | 10 | 0.000962s | 0.000980s | 0.000967s | byte equality observed |
| rotx_mat_test.test | 10 | 0.001005s | 0.001027s | 0.001004s | byte equality observed |
| rotx_quat_test.test | 10 | 0.001082s | 0.001053s | 0.000994s | byte equality observed |
| roty_mat_test.test | 10 | 0.001054s | 0.001037s | 0.001019s | byte equality observed |
| roty_quat_test.test | 10 | 0.001020s | 0.001028s | 0.000998s | byte equality observed |
| rotz_mat_test.test | 10 | 0.001026s | 0.001024s | 0.001007s | byte equality observed |
| rotz_quat_test.test | 10 | 0.001051s | 0.001060s | 0.001027s | byte equality observed |
| row_mat_test.test | 10 | 0.001044s | 0.001015s | 0.001012s | byte equality observed |
| row_test.test | 10 | 0.001021s | 0.001028s | 0.001019s | byte equality observed |
| scalar_cast_m_test.test | 10 | 0.001042s | 0.001027s | 0.000992s | byte equality observed |
| scalar_cast_q_test.test | 10 | 0.001048s | 0.001016s | 0.001003s | byte equality observed |
| scalar_cast_v_test.test | 10 | 0.001017s | 0.000957s | 0.000995s | byte equality observed |
| slerp_test.test | 2 | 0.013277s | 0.011161s | 0.011680s | byte equality observed |
| swap_cols_test.test | 10 | 0.000963s | 0.000979s | 0.000987s | byte equality observed |
| swap_rows_test.test | 10 | 0.001018s | 0.000994s | 0.001013s | byte equality observed |
| swizzle2_test.test | 10 | 0.001013s | 0.000979s | 0.001010s | byte equality observed |
| swizzle2_test2.test | 10 | 0.001008s | 0.000999s | 0.000982s | byte equality observed |
| swizzle2_test3.test | 10 | 0.001013s | 0.000988s | 0.000973s | byte equality observed |
| swizzle2_test4.test | 10 | 0.000997s | 0.001003s | 0.000994s | byte equality observed |
| swizzle3_test.test | 10 | 0.001024s | 0.001006s | 0.001004s | byte equality observed |
| swizzle3_test2.test | 10 | 0.001038s | 0.000991s | 0.001001s | byte equality observed |
| swizzle3_test3.test | 10 | 0.001024s | 0.000980s | 0.001021s | byte equality observed |
| swizzle3_test4.test | 10 | 0.001035s | 0.001011s | 0.001002s | byte equality observed |
| swizzle4_test.test | 10 | 0.001038s | 0.000999s | 0.001045s | byte equality observed |
| swizzle4_test2.test | 10 | 0.000992s | 0.001023s | 0.001023s | byte equality observed |
| swizzle4_test3.test | 10 | 0.001022s | 0.001018s | 0.001030s | byte equality observed |
| swizzle4_test4.test | 10 | 0.001012s | 0.001042s | 0.001013s | byte equality observed |
| to_string_test.test | 10 | 0.001065s | 0.001037s | 0.001033s | byte equality observed |
| transform_test.test | 10 | 0.001073s | 0.001033s | 0.000990s | byte equality observed |
| translation_mat_test.test | 10 | 0.001023s | 0.000998s | 0.001010s | byte equality observed |
| translation_test.test | 10 | 0.001037s | 0.001007s | 0.001012s | byte equality observed |
| transpose_test.test | 10 | 0.001014s | 0.001035s | 0.001009s | byte equality observed |
| vec_index_test.test | 10 | 0.001018s | 0.001018s | 0.000998s | byte equality observed |
| vec_register_test.test | 10 | 0.001013s | 0.001003s | 0.001028s | byte equality observed |
| vec_traits_array_test.test | 10 | 0.001005s | 0.001019s | 0.000993s | byte equality observed |
| vec_traits_gnuc_test.test | 10 | 0.001004s | 0.000988s | 0.001007s | byte equality observed |
| vec_traits_std_array_test.test | 10 | 0.001059s | 0.001006s | 0.001020s | byte equality observed |
| zero_mat_test.test | 10 | 0.001067s | 0.000988s | 0.000998s | byte equality observed |
| zero_quat_test.test | 10 | 0.001029s | 0.001032s | 0.001003s | byte equality observed |
| zero_vec_test.test | 10 | 0.001007s | 0.001013s | 0.001007s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267198).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
