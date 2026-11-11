# libs/qvm/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| math_test.test | RUN | 0.310314 | 0.642427 | 0.774667 | 0.002200 | 0.001998 | 0.002234 |
| mat_traits_array_test.test | RUN | 0.211290 | 0.646269 | 0.620400 | 0.001512 | 0.001669 | 0.001214 |
| mat_traits_std_array_test.test | RUN | 0.208733 | 0.496299 | 0.611910 | 0.001644 | 0.001631 | 0.001641 |
| vec_traits_array_test.test | RUN | 0.181876 | 0.493082 | 0.520269 | 0.001582 | 0.001798 | 0.001458 |
| vec_traits_gnuc_test.test | RUN | 0.330619 | 0.620616 | 1.064284 | 0.001536 | 0.001614 | 0.001662 |
| vec_traits_std_array_test.test | RUN | 0.186271 | 0.396693 | 0.515206 | 0.001634 | 0.001836 | 0.001501 |
| quat_traits_array_test.test | RUN | 0.177988 | 0.511800 | 0.513747 | 0.001624 | 0.001561 | 0.001643 |
| quat_traits_std_array_test.test | RUN | 0.182978 | 0.395214 | 0.528489 | 0.001547 | 0.001694 | 0.001469 |
| access_m_test.test | RUN | 0.291601 | 0.945353 | 1.128813 | 0.001692 | 0.001933 | 0.001598 |
| access_m_fail1.test | COMPILE_FAIL | 0.028925 | 0.040595 | 0.065695 | — | — | — |
| access_m_fail2.test | COMPILE_FAIL | 0.028434 | 0.045156 | 0.076382 | — | — | — |
| access_q_test.test | RUN | 0.251095 | 0.471904 | 0.682249 | 0.001720 | 0.001875 | 0.001780 |
| access_q_fail.test | COMPILE_FAIL | 0.022528 | 0.032983 | 0.066972 | — | — | — |
| access_v_test.test | RUN | 0.250594 | 0.508278 | 0.670625 | 0.001746 | 0.001794 | 0.001490 |
| access_v_fail1.test | COMPILE_FAIL | 0.020848 | 0.031106 | 0.061204 | — | — | — |
| access_v_fail2.test | COMPILE_FAIL | 0.019556 | 0.031251 | 0.055715 | — | — | — |
| assign_test.test | RUN | 0.359590 | 0.781822 | 1.199188 | 0.001698 | 0.001844 | 0.001698 |
| to_string_test.test | RUN | 0.417137 | 1.267769 | 6.603047 | 0.001735 | 0.001711 | 0.001815 |
| cmp_vv_test.test | RUN | 0.312238 | 0.621273 | 0.843016 | 0.001725 | 0.001741 | 0.001540 |
| cross_test.test | RUN | 0.292399 | 0.600881 | 0.845240 | 0.001535 | 0.001780 | 0.001419 |
| div_eq_vs_test.test | RUN | 0.279913 | 0.532334 | 0.813951 | 0.001604 | 0.001916 | 0.001449 |
| div_vs_test.test | RUN | 0.295692 | 0.644661 | 0.820353 | 0.001616 | 0.001792 | 0.001693 |
| dot_vv_test.test | RUN | 0.281446 | 0.588412 | 0.811873 | 0.001517 | 0.001742 | 0.001680 |
| eq_vv_test.test | RUN | 0.299995 | 0.658358 | 0.884734 | 0.001529 | 0.001776 | 0.001366 |
| mag_v_test.test | RUN | 0.282595 | 0.551858 | 0.717100 | 0.001702 | 0.001954 | 0.001753 |
| mag_sqr_v_test.test | RUN | 0.275457 | 0.540410 | 0.785506 | 0.001646 | 0.001887 | 0.001552 |
| minus_v_test.test | RUN | 0.283772 | 0.637128 | 0.776501 | 0.001533 | 0.001950 | 0.001678 |
| minus_eq_vv_test.test | RUN | 0.284004 | 0.616577 | 0.822697 | 0.001517 | 0.001846 | 0.001469 |
| minus_vv_test.test | RUN | 0.295734 | 0.677625 | 0.868791 | 0.001601 | 0.001860 | 0.001666 |
| mul_eq_vs_test.test | RUN | 0.284544 | 0.541456 | 0.811720 | 0.001728 | 0.001747 | 0.001649 |
| mul_vs_test.test | RUN | 0.294467 | 0.674046 | 0.888735 | 0.001506 | 0.001975 | 0.001725 |
| mul_sv_test.test | RUN | 0.294356 | 0.673895 | 0.826244 | 0.001542 | 0.001918 | 0.001562 |
| mul_vm_test.test | RUN | 0.402709 | 1.166797 | 1.330561 | 0.001689 | 0.001611 | 0.001647 |
| normalize_v_test.test | RUN | 0.301817 | 0.829881 | 0.808698 | 0.001501 | 0.001703 | 0.001747 |
| plus_eq_vv_test.test | RUN | 0.280496 | 0.607505 | 0.838150 | 0.001498 | 0.001948 | 0.001765 |
| plus_vv_test.test | RUN | 0.296877 | 0.671588 | 0.829517 | 0.001564 | 0.001936 | 0.001554 |
| scalar_cast_v_test.test | RUN | 0.278147 | 0.496825 | 0.706291 | 0.001679 | 0.001614 | 0.001678 |
| vec_index_test.test | RUN | 0.244383 | 0.501652 | 0.672339 | 0.001604 | 0.001460 | 0.001644 |
| vec_register_test.test | RUN | 0.150765 | 0.457061 | 0.483979 | 0.001697 | 0.001754 | 0.001644 |
| cmp_mm_test.test | RUN | 0.444549 | 1.453598 | 1.657503 | 0.001780 | 0.001796 | 0.001746 |
| determinant_test.test | RUN | 0.317411 | 0.733012 | 0.953258 | 0.001786 | 0.001866 | 0.001558 |
| div_eq_ms_test.test | RUN | 0.325402 | 0.690569 | 1.054855 | 0.001798 | 0.001739 | 0.001780 |
| div_ms_test.test | RUN | 0.368195 | 1.092384 | 1.284046 | 0.001500 | 0.001458 | 0.001768 |
| eq_mm_test.test | RUN | 0.432490 | 1.319920 | 1.606917 | 0.001805 | 0.001691 | 0.001377 |
| minus_m_test.test | RUN | 0.361609 | 1.068477 | 1.264107 | 0.001842 | 0.001789 | 0.001592 |
| minus_eq_mm_test.test | RUN | 0.342862 | 0.999227 | 1.120326 | 0.001555 | 0.001890 | 0.001561 |
| minus_mm_test.test | RUN | 0.387254 | 1.260610 | 1.339962 | 0.001592 | 0.001894 | 0.001724 |
| mul_eq_mm_test.test | RUN | 0.359925 | 0.943641 | 1.020189 | 0.001836 | 0.001774 | 0.001717 |
| mul_eq_ms_test.test | RUN | 0.361044 | 0.691550 | 1.083574 | 0.001822 | 0.001706 | 0.001783 |
| mul_mm_test.test | RUN | 0.454335 | 2.023481 | 1.743186 | 0.001854 | 0.001520 | 0.001531 |
| mul_ms_test.test | RUN | 0.379538 | 1.105612 | 1.305490 | 0.001572 | 0.001768 | 0.001688 |
| mul_sm_test.test | RUN | 0.378961 | 1.073288 | 1.273253 | 0.001775 | 0.001879 | 0.001846 |
| mul_mv_test.test | RUN | 0.419103 | 1.187505 | 1.327857 | 0.001590 | 0.001798 | 0.001692 |
| inverse_m_test.test | RUN | 0.520272 | 2.109303 | 1.815560 | 0.001842 | 0.001347 | 0.001724 |
| plus_eq_mm_test.test | RUN | 0.348173 | 1.008374 | 1.137297 | 0.001709 | 0.001792 | 0.001708 |
| plus_mm_test.test | RUN | 0.384786 | 1.256570 | 1.368413 | 0.002811 | 0.001674 | 0.001663 |
| scalar_cast_m_test.test | RUN | 0.323984 | 0.562475 | 0.842659 | 0.001880 | 0.001798 | 0.001856 |
| mat_index_test.test | RUN | 0.273222 | 0.550854 | 0.707029 | 0.001762 | 0.001558 | 0.001638 |
| cmp_qq_test.test | RUN | 0.288908 | 0.519050 | 0.740723 | 0.001723 | 0.001585 | 0.001426 |
| conjugate_test.test | RUN | 0.273533 | 0.529404 | 0.761129 | 0.001811 | 0.001770 | 0.001465 |
| normalize_q_test.test | RUN | 0.284785 | 0.596118 | 0.780651 | 0.001666 | 0.001831 | 0.001696 |
| div_eq_qs_test.test | RUN | 0.268164 | 0.494501 | 0.757252 | 0.001526 | 0.001753 | 0.001514 |
| div_qs_test.test | RUN | 0.277023 | 0.529967 | 0.757063 | 0.001760 | 0.001908 | 0.001500 |
| dot_qq_test.test | RUN | 0.282749 | 0.506409 | 0.768224 | 0.001782 | 0.001637 | 0.001695 |
| eq_qq_test.test | RUN | 0.271654 | 0.541762 | 0.774452 | 0.001682 | 0.001583 | 0.001770 |
| inverse_q_test.test | RUN | 0.324399 | 0.716638 | 0.947639 | 0.001554 | 0.001872 | 0.001712 |
| mag_q_test.test | RUN | 0.273402 | 0.506592 | 0.744834 | 0.001624 | 0.001635 | 0.001365 |
| mag_sqr_q_test.test | RUN | 0.273271 | 0.524148 | 0.746444 | 0.001829 | 0.001794 | 0.001746 |
| minus_q_test.test | RUN | 0.272806 | 0.514137 | 0.726875 | 0.001777 | 0.001816 | 0.001743 |
| minus_eq_qq_test.test | RUN | 0.273622 | 0.528579 | 0.730896 | 0.001623 | 0.001592 | 0.001727 |
| minus_qq_test.test | RUN | 0.280141 | 0.531565 | 0.761057 | 0.001647 | 0.001725 | 0.001623 |
| mul_eq_qs_test.test | RUN | 0.268765 | 0.502716 | 0.753614 | 0.001562 | 0.001785 | 0.001592 |
| mul_qs_test.test | RUN | 0.278731 | 0.536835 | 0.712864 | 0.001734 | 0.001777 | 0.001731 |
| mul_qv_test.test | RUN | 0.339963 | 0.716537 | 0.943544 | 0.001690 | 0.001698 | 0.001723 |
| mul_qq_test.test | RUN | 0.334935 | 0.739667 | 0.960635 | 0.001632 | 0.001749 | 0.001650 |
| mul_eq_qq_test.test | RUN | 0.337076 | 0.733029 | 0.946795 | 0.001742 | 0.001784 | 0.001566 |
| plus_eq_qq_test.test | RUN | 0.286364 | 0.546740 | 0.729051 | 0.001648 | 0.001596 | 0.001544 |
| plus_qq_test.test | RUN | 0.276024 | 0.536856 | 0.734420 | 0.001796 | 0.001390 | 0.001757 |
| scalar_cast_q_test.test | RUN | 0.272781 | 0.509320 | 0.758338 | 0.001720 | 0.001882 | 0.001650 |
| slerp_test.test | RUN | 0.294279 | 1.019049 | 0.886145 | 0.020439 | 0.014499 | 0.016399 |
| convert_to_test.test | RUN | 0.390923 | 0.941630 | 1.353434 | 0.001820 | 0.001793 | 0.001727 |
| rot_mat_test.test | RUN | 0.652229 | 4.136315 | 3.990299 | 0.063123 | 0.037569 | 0.034154 |
| rot_quat_test.test | RUN | 0.336041 | 0.867413 | 0.966271 | 0.001825 | 0.001795 | 0.001684 |
| rotx_mat_test.test | RUN | 0.372831 | 1.116481 | 1.243506 | 0.001478 | 0.001790 | 0.001654 |
| rotx_quat_test.test | RUN | 0.321031 | 0.749488 | 0.878204 | 0.001744 | 0.001824 | 0.001812 |
| roty_mat_test.test | RUN | 0.378885 | 1.119421 | 1.209799 | 0.001918 | 0.001763 | 0.001740 |
| roty_quat_test.test | RUN | 0.324669 | 0.714471 | 0.798903 | 0.001758 | 0.001771 | 0.001658 |
| rotz_mat_test.test | RUN | 0.389835 | 1.257115 | 1.295816 | 0.001768 | 0.001380 | 0.001778 |
| rotz_quat_test.test | RUN | 0.334610 | 0.721694 | 0.872102 | 0.001381 | 0.001815 | 0.001545 |
| zero_vec_test.test | RUN | 0.296784 | 0.618992 | 0.730024 | 0.001816 | 0.001697 | 0.001455 |
| zero_mat_test.test | RUN | 0.387745 | 1.013547 | 1.111013 | 0.001612 | 0.001883 | 0.001330 |
| zero_quat_test.test | RUN | 0.271861 | 0.531543 | 0.756507 | 0.001734 | 0.001746 | 0.001498 |
| identity_mat_test.test | RUN | 0.333553 | 0.748527 | 0.968318 | 0.001573 | 0.001877 | 0.001775 |
| identity_quat_test.test | RUN | 0.274039 | 0.502742 | 0.758709 | 0.001688 | 0.001850 | 0.001539 |
| col_test.test | RUN | 0.500869 | 1.590690 | 2.044718 | 0.001699 | 0.001749 | 0.001764 |
| col_mat_test.test | RUN | 0.340348 | 0.706288 | 0.957879 | 0.001698 | 0.001790 | 0.001739 |
| neg_col_test.test | RUN | 0.380883 | 1.045165 | 1.296672 | 0.001555 | 0.001744 | 0.001740 |
| neg_row_test.test | RUN | 0.409411 | 1.007719 | 1.248908 | 0.001734 | 0.001815 | 0.001767 |
| del_col_test.test | RUN | 0.400573 | 1.122169 | 1.286029 | 0.001783 | 0.001873 | 0.003072 |
| del_row_test.test | RUN | 0.404944 | 1.110817 | 1.277007 | 0.002018 | 0.001814 | 0.001802 |
| del_row_col_test.test | RUN | 0.542341 | 2.109695 | 2.479515 | 0.001793 | 0.001664 | 0.001752 |
| swap_cols_test.test | RUN | 0.748762 | 2.801372 | 2.987500 | 0.001828 | 0.001642 | 0.001817 |
| swap_rows_test.test | RUN | 0.808248 | 2.941905 | 2.928785 | 0.001844 | 0.001770 | 0.001781 |
| diag_test.test | RUN | 0.341156 | 0.747092 | 1.142737 | 0.001471 | 0.001552 | 0.001398 |
| diag_mat_test.test | RUN | 0.359432 | 0.747414 | 0.986004 | 0.001654 | 0.001657 | 0.001534 |
| row_test.test | RUN | 0.508013 | 1.617081 | 2.026127 | 0.001586 | 0.001848 | 0.001482 |
| row_mat_test.test | RUN | 0.341081 | 0.683467 | 1.001229 | 0.001785 | 0.001586 | 0.001592 |
| transpose_test.test | RUN | 0.390243 | 1.086210 | 1.287334 | 0.001631 | 0.001605 | 0.001694 |
| translation_test.test | RUN | 0.330268 | 0.683268 | 0.997118 | 0.001722 | 0.001643 | 0.001632 |
| translation_mat_test.test | RUN | 0.356504 | 0.692853 | 1.066657 | 0.001781 | 0.001754 | 0.001762 |
| swizzle_const_fail.test | COMPILE_FAIL | 0.125272 | 0.199103 | 0.319212 | — | — | — |
| swizzle2_test2.test | RUN | 0.146421 | 0.297043 | 0.395873 | 0.001614 | 0.001348 | 0.001429 |
| swizzle2_test3.test | RUN | 0.147671 | 0.302823 | 0.415967 | 0.001572 | 0.001537 | 0.001627 |
| swizzle2_test4.test | RUN | 0.147054 | 0.296384 | 0.395066 | 0.001582 | 0.001514 | 0.001584 |
| swizzle2_test.test | RUN | 0.453266 | 0.827515 | 1.349994 | 0.001742 | 0.001941 | 0.001742 |
| swizzle3_test2.test | RUN | 0.162470 | 0.317679 | 0.427461 | 0.001645 | 0.001738 | 0.001801 |
| swizzle3_test3.test | RUN | 0.160788 | 0.316355 | 0.473369 | 0.001553 | 0.001681 | 0.001572 |
| swizzle3_test4.test | RUN | 0.160767 | 0.308516 | 0.433535 | 0.001328 | 0.001816 | 0.001736 |
| swizzle3_test.test | RUN | 0.449413 | 0.839351 | 1.325224 | 0.001565 | 0.001737 | 0.001737 |
| swizzle4_test2.test | RUN | 0.265744 | 0.458853 | 0.778398 | 0.001596 | 0.001908 | 0.001810 |
| swizzle4_test3.test | RUN | 0.274457 | 0.453637 | 0.812929 | 0.001565 | 0.001560 | 0.001712 |
| swizzle4_test4.test | RUN | 0.277228 | 0.470279 | 0.776909 | 0.001540 | 0.001684 | 0.001729 |
| swizzle4_test.test | RUN | 0.460447 | 0.935401 | 1.387036 | 0.001687 | 0.001806 | 0.001952 |
| deduce_scalar_test.test | COMPILE | 0.023042 | 0.046637 | 0.065029 | — | — | — |
| deduce_scalar_mq_test.test | RUN | 0.177902 | 0.349733 | 0.479131 | 0.001636 | 0.001570 | 0.001724 |
| deduce_scalar_mv_test.test | RUN | 0.209603 | 0.404899 | 0.577182 | 0.001523 | 0.001668 | 0.001646 |
| deduce_scalar_fail.test | COMPILE_FAIL | 0.020460 | 0.031132 | 0.057887 | — | — | — |
| deduce_mat_test.test | COMPILE | 0.021141 | 0.042129 | 0.065447 | — | — | — |
| deduce_quat_test.test | COMPILE | 0.021534 | 0.041306 | 0.058537 | — | — | — |
| deduce_vec_test.test | COMPILE | 0.021320 | 0.037919 | 0.063100 | — | — | — |
| interop_test.test | RUN | 0.213943 | 0.434857 | 0.875201 | 0.000795 | 0.000839 | 0.001308 |
| transform_test.test | RUN | 0.409863 | 0.735966 | 1.209071 | 0.001327 | 0.001335 | 0.001241 |
| projection_test.test | RUN | 0.281707 | 0.536966 | 0.661373 | 0.001422 | 0.001307 | 0.001690 |
| scalar_traits_test.test | COMPILE | 0.022783 | 0.042471 | 0.065919 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.934×† | 0.884×, 0.985× | 1.038× |
| clang | 0.929×† | 0.870×, 0.989× | 0.986× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| access_m_test.test | 10 | 0.001075s | 0.001086s | 0.001045s | byte equality observed |
| access_q_test.test | 10 | 0.001036s | 0.001215s | 0.001100s | byte equality observed |
| access_v_test.test | 10 | 0.001053s | 0.001128s | 0.001116s | byte equality observed |
| assign_test.test | 10 | 0.001088s | 0.001134s | 0.001125s | byte equality observed |
| cmp_mm_test.test | 10 | 0.001058s | 0.001123s | 0.001141s | byte equality observed |
| cmp_qq_test.test | 10 | 0.001030s | 0.001118s | 0.001127s | byte equality observed |
| cmp_vv_test.test | 10 | 0.001044s | 0.001106s | 0.001090s | byte equality observed |
| col_mat_test.test | 10 | 0.001059s | 0.001113s | 0.001133s | byte equality observed |
| col_test.test | 10 | 0.001033s | 0.001225s | 0.001103s | byte equality observed |
| conjugate_test.test | 10 | 0.001039s | 0.001177s | 0.001097s | byte equality observed |
| convert_to_test.test | 10 | 0.001084s | 0.001149s | 0.001117s | byte equality observed |
| cross_test.test | 10 | 0.001057s | 0.001172s | 0.001074s | byte equality observed |
| deduce_scalar_mq_test.test | 10 | 0.001061s | 0.001193s | 0.001109s | byte equality observed |
| deduce_scalar_mv_test.test | 10 | 0.001103s | 0.001185s | 0.001128s | byte equality observed |
| del_col_test.test | 10 | 0.001037s | 0.001198s | 0.001040s | byte equality observed |
| del_row_col_test.test | 10 | 0.001035s | 0.001183s | 0.001109s | byte equality observed |
| del_row_test.test | 10 | 0.001078s | 0.001206s | 0.001120s | byte equality observed |
| determinant_test.test | 10 | 0.001051s | 0.001218s | 0.001146s | byte equality observed |
| diag_mat_test.test | 10 | 0.001049s | 0.001181s | 0.001114s | byte equality observed |
| diag_test.test | 10 | 0.001026s | 0.001135s | 0.001133s | byte equality observed |
| div_eq_ms_test.test | 10 | 0.001046s | 0.001141s | 0.001137s | byte equality observed |
| div_eq_qs_test.test | 10 | 0.001049s | 0.001187s | 0.001102s | byte equality observed |
| div_eq_vs_test.test | 10 | 0.001055s | 0.001170s | 0.001084s | byte equality observed |
| div_ms_test.test | 10 | 0.001062s | 0.001203s | 0.001091s | byte equality observed |
| div_qs_test.test | 10 | 0.001073s | 0.001128s | 0.001120s | byte equality observed |
| div_vs_test.test | 10 | 0.001064s | 0.001180s | 0.001126s | byte equality observed |
| dot_qq_test.test | 10 | 0.001042s | 0.001181s | 0.001159s | byte equality observed |
| dot_vv_test.test | 10 | 0.001056s | 0.001225s | 0.001125s | byte equality observed |
| eq_mm_test.test | 10 | 0.001107s | 0.001115s | 0.001113s | byte equality observed |
| eq_qq_test.test | 10 | 0.001094s | 0.001107s | 0.001036s | byte equality observed |
| eq_vv_test.test | 10 | 0.001086s | 0.001137s | 0.001124s | byte equality observed |
| identity_mat_test.test | 10 | 0.001030s | 0.001161s | 0.001095s | byte equality observed |
| identity_quat_test.test | 10 | 0.001076s | 0.001206s | 0.001070s | byte equality observed |
| interop_test.test | 10 | 0.000493s | 0.000537s | 0.001096s | byte equality observed |
| inverse_m_test.test | 10 | 0.001107s | 0.001258s | 0.001172s | byte equality observed |
| inverse_q_test.test | 10 | 0.001071s | 0.001175s | 0.001121s | byte equality observed |
| mag_q_test.test | 10 | 0.001066s | 0.001218s | 0.001096s | byte equality observed |
| mag_sqr_q_test.test | 10 | 0.001055s | 0.001180s | 0.001128s | byte equality observed |
| mag_sqr_v_test.test | 10 | 0.001040s | 0.001181s | 0.001144s | byte equality observed |
| mag_v_test.test | 10 | 0.001079s | 0.001197s | 0.001116s | byte equality observed |
| mat_index_test.test | 10 | 0.001068s | 0.001190s | 0.001124s | byte equality observed |
| mat_traits_array_test.test | 10 | 0.001047s | 0.001141s | 0.001144s | byte equality observed |
| mat_traits_std_array_test.test | 10 | 0.001053s | 0.001187s | 0.001156s | byte equality observed |
| math_test.test | 10 | 0.001564s | 0.001717s | 0.001560s | byte equality observed |
| minus_eq_mm_test.test | 10 | 0.001037s | 0.001167s | 0.001179s | byte equality observed |
| minus_eq_qq_test.test | 10 | 0.001030s | 0.001175s | 0.001173s | byte equality observed |
| minus_eq_vv_test.test | 10 | 0.001026s | 0.001172s | 0.001165s | byte equality observed |
| minus_m_test.test | 10 | 0.001057s | 0.001199s | 0.001190s | byte equality observed |
| minus_mm_test.test | 10 | 0.001071s | 0.001186s | 0.001155s | byte equality observed |
| minus_q_test.test | 10 | 0.001118s | 0.001176s | 0.001112s | byte equality observed |
| minus_qq_test.test | 10 | 0.001084s | 0.001186s | 0.001168s | byte equality observed |
| minus_v_test.test | 10 | 0.001097s | 0.001170s | 0.001207s | byte equality observed |
| minus_vv_test.test | 10 | 0.001038s | 0.001159s | 0.001195s | byte equality observed |
| mul_eq_mm_test.test | 10 | 0.001045s | 0.001137s | 0.001165s | byte equality observed |
| mul_eq_ms_test.test | 10 | 0.001078s | 0.001146s | 0.001193s | byte equality observed |
| mul_eq_qq_test.test | 10 | 0.001049s | 0.001211s | 0.001134s | byte equality observed |
| mul_eq_qs_test.test | 10 | 0.001051s | 0.001179s | 0.001137s | byte equality observed |
| mul_eq_vs_test.test | 10 | 0.001067s | 0.001175s | 0.001149s | byte equality observed |
| mul_mm_test.test | 10 | 0.001045s | 0.001155s | 0.001192s | byte equality observed |
| mul_ms_test.test | 10 | 0.001035s | 0.001121s | 0.001106s | byte equality observed |
| mul_mv_test.test | 10 | 0.001085s | 0.001202s | 0.001127s | byte equality observed |
| mul_qq_test.test | 10 | 0.001068s | 0.001188s | 0.001228s | byte equality observed |
| mul_qs_test.test | 10 | 0.001063s | 0.001149s | 0.001191s | byte equality observed |
| mul_qv_test.test | 10 | 0.001065s | 0.001183s | 0.001222s | byte equality observed |
| mul_sm_test.test | 10 | 0.001080s | 0.001202s | 0.001177s | byte equality observed |
| mul_sv_test.test | 10 | 0.001038s | 0.001174s | 0.001117s | byte equality observed |
| mul_vm_test.test | 10 | 0.001068s | 0.001190s | 0.001163s | byte equality observed |
| mul_vs_test.test | 10 | 0.001044s | 0.001128s | 0.001157s | byte equality observed |
| neg_col_test.test | 10 | 0.001048s | 0.001162s | 0.001156s | byte equality observed |
| neg_row_test.test | 10 | 0.001061s | 0.001091s | 0.001174s | byte equality observed |
| normalize_q_test.test | 10 | 0.001046s | 0.001181s | 0.001141s | byte equality observed |
| normalize_v_test.test | 10 | 0.001046s | 0.001155s | 0.001197s | byte equality observed |
| plus_eq_mm_test.test | 10 | 0.001054s | 0.001206s | 0.001198s | byte equality observed |
| plus_eq_qq_test.test | 10 | 0.001048s | 0.001190s | 0.001128s | byte equality observed |
| plus_eq_vv_test.test | 10 | 0.001042s | 0.001198s | 0.001179s | byte equality observed |
| plus_mm_test.test | 10 | 0.001087s | 0.001185s | 0.001171s | byte equality observed |
| plus_qq_test.test | 10 | 0.001052s | 0.001132s | 0.001185s | byte equality observed |
| plus_vv_test.test | 10 | 0.001047s | 0.001208s | 0.001185s | byte equality observed |
| projection_test.test | 10 | 0.001080s | 0.001120s | 0.001221s | byte equality observed |
| quat_traits_array_test.test | 10 | 0.001084s | 0.001150s | 0.001160s | byte equality observed |
| quat_traits_std_array_test.test | 10 | 0.001004s | 0.001168s | 0.001192s | byte equality observed |
| rot_mat_test.test | 1 | 0.047203s | 0.027308s | 0.032281s | byte equality observed |
| rot_quat_test.test | 10 | 0.001016s | 0.001130s | 0.001194s | byte equality observed |
| rotx_mat_test.test | 10 | 0.001013s | 0.001190s | 0.001199s | byte equality observed |
| rotx_quat_test.test | 10 | 0.001063s | 0.001127s | 0.001148s | byte equality observed |
| roty_mat_test.test | 10 | 0.001084s | 0.001218s | 0.001183s | byte equality observed |
| roty_quat_test.test | 10 | 0.001057s | 0.001163s | 0.001199s | byte equality observed |
| rotz_mat_test.test | 10 | 0.001062s | 0.001212s | 0.001155s | byte equality observed |
| rotz_quat_test.test | 10 | 0.001096s | 0.001210s | 0.001106s | byte equality observed |
| row_mat_test.test | 10 | 0.001077s | 0.001194s | 0.001140s | byte equality observed |
| row_test.test | 10 | 0.001073s | 0.001184s | 0.001193s | byte equality observed |
| scalar_cast_m_test.test | 10 | 0.001047s | 0.001165s | 0.001197s | byte equality observed |
| scalar_cast_q_test.test | 10 | 0.001033s | 0.001159s | 0.001182s | byte equality observed |
| scalar_cast_v_test.test | 10 | 0.001037s | 0.001146s | 0.001152s | byte equality observed |
| slerp_test.test | 2 | 0.014980s | 0.013358s | 0.014403s | byte equality observed |
| swap_cols_test.test | 10 | 0.001028s | 0.001177s | 0.001171s | byte equality observed |
| swap_rows_test.test | 10 | 0.001055s | 0.001206s | 0.001178s | byte equality observed |
| swizzle2_test.test | 10 | 0.001119s | 0.001167s | 0.001193s | byte equality observed |
| swizzle2_test2.test | 10 | 0.001055s | 0.001209s | 0.001187s | byte equality observed |
| swizzle2_test3.test | 10 | 0.001050s | 0.001184s | 0.001180s | byte equality observed |
| swizzle2_test4.test | 10 | 0.001068s | 0.001175s | 0.001124s | byte equality observed |
| swizzle3_test.test | 10 | 0.001045s | 0.001200s | 0.001184s | byte equality observed |
| swizzle3_test2.test | 10 | 0.001038s | 0.001171s | 0.001173s | byte equality observed |
| swizzle3_test3.test | 10 | 0.001054s | 0.001187s | 0.001192s | byte equality observed |
| swizzle3_test4.test | 10 | 0.001042s | 0.001177s | 0.001183s | byte equality observed |
| swizzle4_test.test | 10 | 0.001076s | 0.001188s | 0.001190s | byte equality observed |
| swizzle4_test2.test | 10 | 0.001024s | 0.001164s | 0.001171s | byte equality observed |
| swizzle4_test3.test | 10 | 0.001033s | 0.001205s | 0.001192s | byte equality observed |
| swizzle4_test4.test | 10 | 0.001014s | 0.001167s | 0.001193s | byte equality observed |
| to_string_test.test | 10 | 0.001068s | 0.001185s | 0.001225s | byte equality observed |
| transform_test.test | 10 | 0.001051s | 0.001164s | 0.001186s | byte equality observed |
| translation_mat_test.test | 10 | 0.001041s | 0.001206s | 0.001177s | byte equality observed |
| translation_test.test | 10 | 0.001098s | 0.001179s | 0.001187s | byte equality observed |
| transpose_test.test | 10 | 0.001084s | 0.001203s | 0.001187s | byte equality observed |
| vec_index_test.test | 10 | 0.001079s | 0.001122s | 0.001153s | byte equality observed |
| vec_register_test.test | 10 | 0.001047s | 0.001105s | 0.001170s | byte equality observed |
| vec_traits_array_test.test | 10 | 0.001039s | 0.001137s | 0.001175s | byte equality observed |
| vec_traits_gnuc_test.test | 10 | 0.001077s | 0.001128s | 0.001194s | byte equality observed |
| vec_traits_std_array_test.test | 10 | 0.001045s | 0.001185s | 0.001204s | byte equality observed |
| zero_mat_test.test | 10 | 0.001068s | 0.001168s | 0.001204s | byte equality observed |
| zero_quat_test.test | 10 | 0.001017s | 0.001165s | 0.001206s | byte equality observed |
| zero_vec_test.test | 10 | 0.001043s | 0.001190s | 0.001212s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
