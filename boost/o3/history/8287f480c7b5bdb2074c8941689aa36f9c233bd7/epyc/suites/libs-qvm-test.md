# libs/qvm/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| math_test.test | RUN | 0.309497 | 0.634336 | 0.780287 | 0.002220 | 0.002154 | 0.002196 |
| mat_traits_array_test.test | RUN | 0.203882 | 0.609508 | 0.607302 | 0.001773 | 0.001409 | 0.001399 |
| mat_traits_std_array_test.test | RUN | 0.202864 | 0.477316 | 0.618563 | 0.001391 | 0.001644 | 0.001652 |
| vec_traits_array_test.test | RUN | 0.184312 | 0.499012 | 0.519240 | 0.001664 | 0.001748 | 0.001171 |
| vec_traits_gnuc_test.test | RUN | 0.327624 | 0.611750 | 1.039211 | 0.001537 | 0.001550 | 0.001393 |
| vec_traits_std_array_test.test | RUN | 0.183552 | 0.379256 | 0.494166 | 0.001734 | 0.001645 | 0.001569 |
| quat_traits_array_test.test | RUN | 0.177510 | 0.501049 | 0.519110 | 0.001597 | 0.001418 | 0.001496 |
| quat_traits_std_array_test.test | RUN | 0.186504 | 0.401948 | 0.489452 | 0.001630 | 0.001707 | 0.001725 |
| access_m_test.test | RUN | 0.286594 | 0.929900 | 1.147339 | 0.001666 | 0.001920 | 0.001552 |
| access_m_fail1.test | COMPILE_FAIL | 0.028088 | 0.041186 | 0.087221 | — | — | — |
| access_m_fail2.test | COMPILE_FAIL | 0.028566 | 0.040775 | 0.078958 | — | — | — |
| access_q_test.test | RUN | 0.245760 | 0.495466 | 0.708605 | 0.001554 | 0.001765 | 0.001593 |
| access_q_fail.test | COMPILE_FAIL | 0.021912 | 0.032965 | 0.057531 | — | — | — |
| access_v_test.test | RUN | 0.246544 | 0.480132 | 0.668356 | 0.001638 | 0.001761 | 0.001719 |
| access_v_fail1.test | COMPILE_FAIL | 0.020669 | 0.028989 | 0.061315 | — | — | — |
| access_v_fail2.test | COMPILE_FAIL | 0.019650 | 0.031459 | 0.060128 | — | — | — |
| assign_test.test | RUN | 0.357938 | 0.839607 | 1.222195 | 0.001675 | 0.001814 | 0.001561 |
| to_string_test.test | RUN | 0.407203 | 1.308449 | 6.652227 | 0.001444 | 0.001812 | 0.001516 |
| cmp_vv_test.test | RUN | 0.311824 | 0.671677 | 0.823058 | 0.001724 | 0.001828 | 0.001747 |
| cross_test.test | RUN | 0.302626 | 0.601974 | 0.844003 | 0.001704 | 0.001848 | 0.001526 |
| div_eq_vs_test.test | RUN | 0.270313 | 0.527227 | 0.773227 | 0.001670 | 0.001864 | 0.001910 |
| div_vs_test.test | RUN | 0.288477 | 0.644803 | 0.876665 | 0.001329 | 0.001733 | 0.001729 |
| dot_vv_test.test | RUN | 0.280865 | 0.584038 | 0.823187 | 0.001608 | 0.001832 | 0.001337 |
| eq_vv_test.test | RUN | 0.305907 | 0.660215 | 0.864103 | 0.001603 | 0.001793 | 0.001608 |
| mag_v_test.test | RUN | 0.276147 | 0.539257 | 0.727831 | 0.001622 | 0.001940 | 0.001563 |
| mag_sqr_v_test.test | RUN | 0.278558 | 0.573139 | 0.760008 | 0.001679 | 0.001858 | 0.001611 |
| minus_v_test.test | RUN | 0.284799 | 0.657716 | 0.835170 | 0.001343 | 0.001848 | 0.001554 |
| minus_eq_vv_test.test | RUN | 0.280452 | 0.635585 | 0.761145 | 0.001486 | 0.001862 | 0.001436 |
| minus_vv_test.test | RUN | 0.293304 | 0.651905 | 0.859840 | 0.001682 | 0.001816 | 0.001733 |
| mul_eq_vs_test.test | RUN | 0.282669 | 0.548940 | 0.772928 | 0.001333 | 0.001732 | 0.001564 |
| mul_vs_test.test | RUN | 0.293860 | 0.667584 | 0.868886 | 0.001714 | 0.001879 | 0.001548 |
| mul_sv_test.test | RUN | 0.306283 | 0.672636 | 0.877178 | 0.001586 | 0.001812 | 0.001718 |
| mul_vm_test.test | RUN | 0.401623 | 1.164677 | 1.239600 | 0.001364 | 0.001592 | 0.001728 |
| normalize_v_test.test | RUN | 0.290204 | 0.818522 | 0.875031 | 0.001507 | 0.001642 | 0.001562 |
| plus_eq_vv_test.test | RUN | 0.273319 | 0.623191 | 0.825084 | 0.001679 | 0.001794 | 0.001442 |
| plus_vv_test.test | RUN | 0.294616 | 0.668608 | 0.870982 | 0.001592 | 0.001846 | 0.001388 |
| scalar_cast_v_test.test | RUN | 0.275274 | 0.502200 | 0.777576 | 0.001519 | 0.001504 | 0.001553 |
| vec_index_test.test | RUN | 0.241989 | 0.506919 | 0.653420 | 0.001942 | 0.001556 | 0.001382 |
| vec_register_test.test | RUN | 0.150415 | 0.460735 | 0.504981 | 0.001704 | 0.001662 | 0.001657 |
| cmp_mm_test.test | RUN | 0.443541 | 1.437386 | 1.656555 | 0.001764 | 0.001571 | 0.001763 |
| determinant_test.test | RUN | 0.314760 | 0.762130 | 0.950187 | 0.001729 | 0.001885 | 0.001454 |
| div_eq_ms_test.test | RUN | 0.327041 | 0.692787 | 1.050262 | 0.001527 | 0.001669 | 0.001705 |
| div_ms_test.test | RUN | 0.370035 | 1.078711 | 1.294884 | 0.001852 | 0.001789 | 0.001759 |
| eq_mm_test.test | RUN | 0.431314 | 1.341733 | 1.581695 | 0.001695 | 0.001579 | 0.001529 |
| minus_m_test.test | RUN | 0.370874 | 1.076484 | 1.253313 | 0.001747 | 0.001808 | 0.001638 |
| minus_eq_mm_test.test | RUN | 0.347760 | 1.005953 | 1.137503 | 0.001628 | 0.001693 | 0.001756 |
| minus_mm_test.test | RUN | 0.395622 | 1.266912 | 1.346968 | 0.001758 | 0.001804 | 0.001762 |
| mul_eq_mm_test.test | RUN | 0.342184 | 0.938752 | 1.024805 | 0.001839 | 0.001662 | 0.001758 |
| mul_eq_ms_test.test | RUN | 0.334523 | 0.690468 | 1.036230 | 0.001809 | 0.001881 | 0.001668 |
| mul_mm_test.test | RUN | 0.445619 | 2.062145 | 1.651632 | 0.001818 | 0.001746 | 0.001612 |
| mul_ms_test.test | RUN | 0.388491 | 1.089446 | 1.360311 | 0.001762 | 0.001790 | 0.001716 |
| mul_sm_test.test | RUN | 0.380664 | 1.086495 | 1.304498 | 0.001820 | 0.001672 | 0.001741 |
| mul_mv_test.test | RUN | 0.407304 | 1.171071 | 1.315955 | 0.001774 | 0.001730 | 0.001742 |
| inverse_m_test.test | RUN | 0.512617 | 2.101072 | 1.815147 | 0.001688 | 0.001685 | 0.001615 |
| plus_eq_mm_test.test | RUN | 0.354993 | 0.999877 | 1.135848 | 0.001666 | 0.001800 | 0.001753 |
| plus_mm_test.test | RUN | 0.392525 | 1.285119 | 1.349437 | 0.001699 | 0.001768 | 0.001620 |
| scalar_cast_m_test.test | RUN | 0.307434 | 0.577322 | 0.859521 | 0.001553 | 0.001754 | 0.001512 |
| mat_index_test.test | RUN | 0.261665 | 0.544289 | 0.725668 | 0.001732 | 0.001663 | 0.001751 |
| cmp_qq_test.test | RUN | 0.275390 | 0.511494 | 0.719253 | 0.001650 | 0.001812 | 0.001601 |
| conjugate_test.test | RUN | 0.272292 | 0.529294 | 0.774754 | 0.001767 | 0.001765 | 0.001735 |
| normalize_q_test.test | RUN | 0.274629 | 0.574310 | 0.757693 | 0.001526 | 0.001555 | 0.001450 |
| div_eq_qs_test.test | RUN | 0.270267 | 0.504772 | 0.681384 | 0.001646 | 0.001508 | 0.001706 |
| div_qs_test.test | RUN | 0.276850 | 0.539361 | 0.762162 | 0.001705 | 0.001701 | 0.001462 |
| dot_qq_test.test | RUN | 0.272692 | 0.510408 | 0.753174 | 0.001736 | 0.001785 | 0.001742 |
| eq_qq_test.test | RUN | 0.273356 | 0.541582 | 0.764830 | 0.001459 | 0.001812 | 0.001699 |
| inverse_q_test.test | RUN | 0.327237 | 0.708026 | 0.977663 | 0.001780 | 0.001759 | 0.001718 |
| mag_q_test.test | RUN | 0.273033 | 0.542668 | 0.712469 | 0.001605 | 0.001396 | 0.001656 |
| mag_sqr_q_test.test | RUN | 0.299094 | 0.507513 | 0.720739 | 0.001674 | 0.001868 | 0.001752 |
| minus_q_test.test | RUN | 0.305754 | 0.526766 | 0.715991 | 0.001532 | 0.001855 | 0.001703 |
| minus_eq_qq_test.test | RUN | 0.294842 | 0.524286 | 0.750316 | 0.001758 | 0.001712 | 0.001745 |
| minus_qq_test.test | RUN | 0.279794 | 0.526430 | 0.751251 | 0.001660 | 0.001523 | 0.001574 |
| mul_eq_qs_test.test | RUN | 0.266836 | 0.497083 | 0.705797 | 0.001603 | 0.001826 | 0.001703 |
| mul_qs_test.test | RUN | 0.268570 | 0.519960 | 0.732251 | 0.001416 | 0.001822 | 0.001727 |
| mul_qv_test.test | RUN | 0.331438 | 0.715241 | 0.951669 | 0.001864 | 0.001823 | 0.001840 |
| mul_qq_test.test | RUN | 0.321379 | 0.728333 | 0.924492 | 0.001758 | 0.001671 | 0.001496 |
| mul_eq_qq_test.test | RUN | 0.329689 | 0.731577 | 0.907321 | 0.001620 | 0.001722 | 0.001655 |
| plus_eq_qq_test.test | RUN | 0.275128 | 0.517133 | 0.748397 | 0.001772 | 0.001735 | 0.001672 |
| plus_qq_test.test | RUN | 0.276872 | 0.531290 | 0.737480 | 0.001688 | 0.001690 | 0.001726 |
| scalar_cast_q_test.test | RUN | 0.267107 | 0.489462 | 0.748286 | 0.001626 | 0.001831 | 0.001781 |
| slerp_test.test | RUN | 0.288624 | 0.993963 | 0.894082 | 0.019450 | 0.014538 | 0.015649 |
| convert_to_test.test | RUN | 0.383448 | 0.932985 | 1.283724 | 0.001752 | 0.001848 | 0.001667 |
| rot_mat_test.test | RUN | 0.647837 | 4.156684 | 3.962221 | 0.073071 | 0.039033 | 0.035600 |
| rot_quat_test.test | RUN | 0.320078 | 0.840733 | 0.991052 | 0.001740 | 0.001942 | 0.001729 |
| rotx_mat_test.test | RUN | 0.377668 | 1.119758 | 1.229598 | 0.001684 | 0.001794 | 0.001677 |
| rotx_quat_test.test | RUN | 0.333411 | 0.721905 | 0.908379 | 0.001764 | 0.001785 | 0.001748 |
| roty_mat_test.test | RUN | 0.364620 | 1.124250 | 1.148077 | 0.001762 | 0.001864 | 0.001675 |
| roty_quat_test.test | RUN | 0.315491 | 0.720812 | 0.931220 | 0.001642 | 0.001819 | 0.001702 |
| rotz_mat_test.test | RUN | 0.382790 | 1.256667 | 1.330619 | 0.001629 | 0.001670 | 0.001832 |
| rotz_quat_test.test | RUN | 0.323993 | 0.722920 | 0.841405 | 0.001948 | 0.001770 | 0.001722 |
| zero_vec_test.test | RUN | 0.307710 | 0.627834 | 0.747991 | 0.001701 | 0.001660 | 0.001708 |
| zero_mat_test.test | RUN | 0.384881 | 1.029133 | 1.102778 | 0.001703 | 0.001677 | 0.001718 |
| zero_quat_test.test | RUN | 0.268345 | 0.537692 | 0.695331 | 0.001484 | 0.001636 | 0.001699 |
| identity_mat_test.test | RUN | 0.336820 | 0.755320 | 0.932942 | 0.001723 | 0.001689 | 0.001449 |
| identity_quat_test.test | RUN | 0.274549 | 0.504028 | 0.728065 | 0.001669 | 0.001853 | 0.001738 |
| col_test.test | RUN | 0.486388 | 1.563958 | 2.032600 | 0.001727 | 0.001834 | 0.001761 |
| col_mat_test.test | RUN | 0.325911 | 0.728944 | 0.966698 | 0.001828 | 0.001806 | 0.001670 |
| neg_col_test.test | RUN | 0.379740 | 1.045133 | 1.241680 | 0.001468 | 0.001767 | 0.001831 |
| neg_row_test.test | RUN | 0.390404 | 1.047399 | 1.284536 | 0.001359 | 0.001840 | 0.001601 |
| del_col_test.test | RUN | 0.387016 | 1.126285 | 1.276878 | 0.001780 | 0.001808 | 0.001658 |
| del_row_test.test | RUN | 0.387068 | 1.096783 | 1.271283 | 0.001724 | 0.001875 | 0.001620 |
| del_row_col_test.test | RUN | 0.530815 | 2.150669 | 2.436099 | 0.001914 | 0.001943 | 0.001736 |
| swap_cols_test.test | RUN | 0.730954 | 2.837843 | 2.942893 | 0.001651 | 0.002008 | 0.001808 |
| swap_rows_test.test | RUN | 0.804936 | 2.926205 | 2.924440 | 0.001740 | 0.001691 | 0.001702 |
| diag_test.test | RUN | 0.350939 | 0.760934 | 1.146666 | 0.001838 | 0.001744 | 0.001679 |
| diag_mat_test.test | RUN | 0.360289 | 0.752838 | 0.979431 | 0.001590 | 0.001764 | 0.001720 |
| row_test.test | RUN | 0.513617 | 1.602824 | 2.004201 | 0.001514 | 0.001804 | 0.001740 |
| row_mat_test.test | RUN | 0.351654 | 0.701196 | 1.016566 | 0.001636 | 0.001659 | 0.001719 |
| transpose_test.test | RUN | 0.379392 | 1.119160 | 1.295575 | 0.001710 | 0.001867 | 0.001739 |
| translation_test.test | RUN | 0.320544 | 0.674578 | 1.005608 | 0.001598 | 0.001830 | 0.001552 |
| translation_mat_test.test | RUN | 0.343132 | 0.707912 | 1.017781 | 0.001490 | 0.001827 | 0.001723 |
| swizzle_const_fail.test | COMPILE_FAIL | 0.119110 | 0.208635 | 0.273663 | — | — | — |
| swizzle2_test2.test | RUN | 0.143748 | 0.294850 | 0.419256 | 0.001590 | 0.001588 | 0.001565 |
| swizzle2_test3.test | RUN | 0.143140 | 0.295627 | 0.415549 | 0.001562 | 0.001738 | 0.001333 |
| swizzle2_test4.test | RUN | 0.147133 | 0.295835 | 0.409708 | 0.001287 | 0.001786 | 0.001182 |
| swizzle2_test.test | RUN | 0.450566 | 0.832660 | 1.358650 | 0.001752 | 0.001876 | 0.001678 |
| swizzle3_test2.test | RUN | 0.161146 | 0.315610 | 0.457932 | 0.001402 | 0.001854 | 0.001722 |
| swizzle3_test3.test | RUN | 0.158613 | 0.326046 | 0.470787 | 0.001718 | 0.001853 | 0.001435 |
| swizzle3_test4.test | RUN | 0.162455 | 0.310568 | 0.459387 | 0.001520 | 0.001742 | 0.001426 |
| swizzle3_test.test | RUN | 0.450839 | 0.845845 | 1.357509 | 0.001613 | 0.001554 | 0.001754 |
| swizzle4_test2.test | RUN | 0.263617 | 0.445073 | 0.784597 | 0.001802 | 0.001816 | 0.001772 |
| swizzle4_test3.test | RUN | 0.268913 | 0.485952 | 0.817104 | 0.001603 | 0.001676 | 0.001540 |
| swizzle4_test4.test | RUN | 0.274797 | 0.454286 | 0.782484 | 0.001418 | 0.001546 | 0.001822 |
| swizzle4_test.test | RUN | 0.460001 | 0.923033 | 1.339875 | 0.001618 | 0.001816 | 0.001788 |
| deduce_scalar_test.test | COMPILE | 0.023492 | 0.045435 | 0.072747 | — | — | — |
| deduce_scalar_mq_test.test | RUN | 0.172931 | 0.348882 | 0.451171 | 0.001360 | 0.001526 | 0.001572 |
| deduce_scalar_mv_test.test | RUN | 0.213590 | 0.414259 | 0.551011 | 0.001729 | 0.001653 | 0.001582 |
| deduce_scalar_fail.test | COMPILE_FAIL | 0.020273 | 0.029077 | 0.057323 | — | — | — |
| deduce_mat_test.test | COMPILE | 0.022666 | 0.043776 | 0.062713 | — | — | — |
| deduce_quat_test.test | COMPILE | 0.021651 | 0.036999 | 0.063503 | — | — | — |
| deduce_vec_test.test | COMPILE | 0.020977 | 0.037357 | 0.065531 | — | — | — |
| interop_test.test | RUN | 0.188268 | 0.507247 | 0.849378 | 0.000598 | 0.000813 | 0.001215 |
| transform_test.test | RUN | 0.406519 | 0.680921 | 1.186126 | 0.001325 | 0.001057 | 0.001248 |
| projection_test.test | RUN | 0.280671 | 0.583910 | 0.624013 | 0.001298 | 0.001326 | 0.001397 |
| scalar_traits_test.test | COMPILE | 0.022646 | 0.041785 | 0.065823 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.839×† | 0.810×, 0.867× | 1.083× |
| clang | 0.803×† | 0.730×, 0.875× | 0.941× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| access_m_test.test | 10 | 0.001051s | 0.001009s | 0.001062s | byte equality observed |
| access_q_test.test | 10 | 0.001033s | 0.001032s | 0.001036s | byte equality observed |
| access_v_test.test | 10 | 0.001030s | 0.001026s | 0.001047s | byte equality observed |
| assign_test.test | 10 | 0.001051s | 0.001028s | 0.001034s | byte equality observed |
| cmp_mm_test.test | 10 | 0.001028s | 0.001027s | 0.001036s | byte equality observed |
| cmp_qq_test.test | 10 | 0.001035s | 0.001004s | 0.001016s | byte equality observed |
| cmp_vv_test.test | 10 | 0.001018s | 0.001029s | 0.001013s | byte equality observed |
| col_mat_test.test | 10 | 0.001058s | 0.001024s | 0.001033s | byte equality observed |
| col_test.test | 10 | 0.001020s | 0.001018s | 0.001032s | byte equality observed |
| conjugate_test.test | 10 | 0.001012s | 0.001021s | 0.001024s | byte equality observed |
| convert_to_test.test | 10 | 0.001014s | 0.001009s | 0.001025s | byte equality observed |
| cross_test.test | 10 | 0.001016s | 0.001013s | 0.001054s | byte equality observed |
| deduce_scalar_mq_test.test | 10 | 0.001048s | 0.001032s | 0.001046s | byte equality observed |
| deduce_scalar_mv_test.test | 10 | 0.001018s | 0.001007s | 0.001048s | byte equality observed |
| del_col_test.test | 10 | 0.001028s | 0.001008s | 0.001102s | byte equality observed |
| del_row_col_test.test | 10 | 0.001048s | 0.001013s | 0.001045s | byte equality observed |
| del_row_test.test | 10 | 0.001031s | 0.001001s | 0.001032s | byte equality observed |
| determinant_test.test | 10 | 0.001060s | 0.001040s | 0.001067s | byte equality observed |
| diag_mat_test.test | 10 | 0.001038s | 0.001007s | 0.001035s | byte equality observed |
| diag_test.test | 10 | 0.001013s | 0.001004s | 0.001043s | byte equality observed |
| div_eq_ms_test.test | 10 | 0.001021s | 0.001033s | 0.001066s | byte equality observed |
| div_eq_qs_test.test | 10 | 0.001033s | 0.001018s | 0.001058s | byte equality observed |
| div_eq_vs_test.test | 10 | 0.001020s | 0.001023s | 0.001082s | byte equality observed |
| div_ms_test.test | 10 | 0.001033s | 0.001012s | 0.001075s | byte equality observed |
| div_qs_test.test | 10 | 0.001052s | 0.001039s | 0.001039s | byte equality observed |
| div_vs_test.test | 10 | 0.001018s | 0.001020s | 0.001092s | byte equality observed |
| dot_qq_test.test | 10 | 0.001046s | 0.001023s | 0.001057s | byte equality observed |
| dot_vv_test.test | 10 | 0.001013s | 0.001018s | 0.001027s | byte equality observed |
| eq_mm_test.test | 10 | 0.001043s | 0.001039s | 0.001048s | byte equality observed |
| eq_qq_test.test | 10 | 0.001059s | 0.001056s | 0.001052s | byte equality observed |
| eq_vv_test.test | 10 | 0.001059s | 0.001001s | 0.001023s | byte equality observed |
| identity_mat_test.test | 10 | 0.001035s | 0.001018s | 0.001071s | byte equality observed |
| identity_quat_test.test | 10 | 0.001017s | 0.001029s | 0.001053s | byte equality observed |
| interop_test.test | 10 | 0.000489s | 0.000482s | 0.001045s | byte equality observed |
| inverse_m_test.test | 10 | 0.001118s | 0.001064s | 0.001078s | byte equality observed |
| inverse_q_test.test | 10 | 0.001024s | 0.001044s | 0.001021s | byte equality observed |
| mag_q_test.test | 10 | 0.001047s | 0.001034s | 0.001022s | byte equality observed |
| mag_sqr_q_test.test | 10 | 0.001075s | 0.001008s | 0.001072s | byte equality observed |
| mag_sqr_v_test.test | 10 | 0.001037s | 0.001030s | 0.001035s | byte equality observed |
| mag_v_test.test | 10 | 0.001049s | 0.001045s | 0.001043s | byte equality observed |
| mat_index_test.test | 10 | 0.001041s | 0.001053s | 0.001051s | byte equality observed |
| mat_traits_array_test.test | 10 | 0.001019s | 0.001056s | 0.001061s | byte equality observed |
| mat_traits_std_array_test.test | 10 | 0.001012s | 0.001014s | 0.001013s | byte equality observed |
| math_test.test | 10 | 0.001576s | 0.001489s | 0.001399s | byte equality observed |
| minus_eq_mm_test.test | 10 | 0.001033s | 0.001025s | 0.001023s | byte equality observed |
| minus_eq_qq_test.test | 10 | 0.001055s | 0.001000s | 0.001043s | byte equality observed |
| minus_eq_vv_test.test | 10 | 0.001038s | 0.001025s | 0.001034s | byte equality observed |
| minus_m_test.test | 10 | 0.001051s | 0.001031s | 0.001063s | byte equality observed |
| minus_mm_test.test | 10 | 0.001047s | 0.001024s | 0.001048s | byte equality observed |
| minus_q_test.test | 10 | 0.001054s | 0.001028s | 0.001045s | byte equality observed |
| minus_qq_test.test | 10 | 0.001052s | 0.001038s | 0.001064s | byte equality observed |
| minus_v_test.test | 10 | 0.001045s | 0.001057s | 0.001054s | byte equality observed |
| minus_vv_test.test | 10 | 0.001055s | 0.001039s | 0.001021s | byte equality observed |
| mul_eq_mm_test.test | 10 | 0.001045s | 0.001029s | 0.001028s | byte equality observed |
| mul_eq_ms_test.test | 10 | 0.001032s | 0.001026s | 0.001030s | byte equality observed |
| mul_eq_qq_test.test | 10 | 0.001044s | 0.001044s | 0.001067s | byte equality observed |
| mul_eq_qs_test.test | 10 | 0.001050s | 0.001063s | 0.001044s | byte equality observed |
| mul_eq_vs_test.test | 10 | 0.001058s | 0.001029s | 0.001022s | byte equality observed |
| mul_mm_test.test | 10 | 0.001055s | 0.001016s | 0.001038s | byte equality observed |
| mul_ms_test.test | 10 | 0.001081s | 0.001035s | 0.001042s | byte equality observed |
| mul_mv_test.test | 10 | 0.001102s | 0.001017s | 0.001038s | byte equality observed |
| mul_qq_test.test | 10 | 0.001034s | 0.001052s | 0.001064s | byte equality observed |
| mul_qs_test.test | 10 | 0.001085s | 0.001029s | 0.001050s | byte equality observed |
| mul_qv_test.test | 10 | 0.001052s | 0.001041s | 0.001076s | byte equality observed |
| mul_sm_test.test | 10 | 0.001109s | 0.001011s | 0.001030s | byte equality observed |
| mul_sv_test.test | 10 | 0.001107s | 0.000991s | 0.001026s | byte equality observed |
| mul_vm_test.test | 10 | 0.001110s | 0.001036s | 0.001017s | byte equality observed |
| mul_vs_test.test | 10 | 0.001107s | 0.001015s | 0.001012s | byte equality observed |
| neg_col_test.test | 10 | 0.001095s | 0.001009s | 0.001027s | byte equality observed |
| neg_row_test.test | 10 | 0.001123s | 0.001021s | 0.001047s | byte equality observed |
| normalize_q_test.test | 10 | 0.001128s | 0.001014s | 0.001054s | byte equality observed |
| normalize_v_test.test | 10 | 0.001121s | 0.001043s | 0.001071s | byte equality observed |
| plus_eq_mm_test.test | 10 | 0.001122s | 0.001018s | 0.001039s | byte equality observed |
| plus_eq_qq_test.test | 10 | 0.001047s | 0.001025s | 0.001053s | byte equality observed |
| plus_eq_vv_test.test | 10 | 0.001052s | 0.001014s | 0.001062s | byte equality observed |
| plus_mm_test.test | 10 | 0.001089s | 0.001021s | 0.001045s | byte equality observed |
| plus_qq_test.test | 10 | 0.001177s | 0.001023s | 0.001042s | byte equality observed |
| plus_vv_test.test | 10 | 0.001159s | 0.001018s | 0.001053s | byte equality observed |
| projection_test.test | 10 | 0.001133s | 0.001045s | 0.001092s | byte equality observed |
| quat_traits_array_test.test | 10 | 0.001094s | 0.001022s | 0.001047s | byte equality observed |
| quat_traits_std_array_test.test | 10 | 0.001107s | 0.001010s | 0.001026s | byte equality observed |
| rot_mat_test.test | 1 | 0.054773s | 0.025772s | 0.026272s | byte equality observed |
| rot_quat_test.test | 10 | 0.001075s | 0.000986s | 0.000973s | byte equality observed |
| rotx_mat_test.test | 10 | 0.001104s | 0.001018s | 0.001026s | byte equality observed |
| rotx_quat_test.test | 10 | 0.001039s | 0.001041s | 0.001053s | byte equality observed |
| roty_mat_test.test | 10 | 0.001147s | 0.001042s | 0.001052s | byte equality observed |
| roty_quat_test.test | 10 | 0.001124s | 0.000999s | 0.001042s | byte equality observed |
| rotz_mat_test.test | 10 | 0.001116s | 0.001033s | 0.001084s | byte equality observed |
| rotz_quat_test.test | 10 | 0.001139s | 0.001004s | 0.001058s | byte equality observed |
| row_mat_test.test | 10 | 0.001095s | 0.001005s | 0.001015s | byte equality observed |
| row_test.test | 10 | 0.001099s | 0.001020s | 0.001019s | byte equality observed |
| scalar_cast_m_test.test | 10 | 0.001119s | 0.001034s | 0.001027s | byte equality observed |
| scalar_cast_q_test.test | 10 | 0.001037s | 0.001029s | 0.001046s | byte equality observed |
| scalar_cast_v_test.test | 10 | 0.001072s | 0.001000s | 0.001021s | byte equality observed |
| slerp_test.test | 2 | 0.016067s | 0.010963s | 0.011874s | byte equality observed |
| swap_cols_test.test | 10 | 0.001087s | 0.000978s | 0.001009s | byte equality observed |
| swap_rows_test.test | 10 | 0.001111s | 0.001004s | 0.001060s | byte equality observed |
| swizzle2_test.test | 10 | 0.001102s | 0.001007s | 0.001051s | byte equality observed |
| swizzle2_test2.test | 10 | 0.001115s | 0.001042s | 0.001050s | byte equality observed |
| swizzle2_test3.test | 10 | 0.001130s | 0.001022s | 0.001018s | byte equality observed |
| swizzle2_test4.test | 10 | 0.001127s | 0.001013s | 0.001052s | byte equality observed |
| swizzle3_test.test | 10 | 0.001096s | 0.000996s | 0.001034s | byte equality observed |
| swizzle3_test2.test | 10 | 0.001100s | 0.001023s | 0.001004s | byte equality observed |
| swizzle3_test3.test | 10 | 0.001092s | 0.001025s | 0.001066s | byte equality observed |
| swizzle3_test4.test | 10 | 0.001118s | 0.001015s | 0.001050s | byte equality observed |
| swizzle4_test.test | 10 | 0.001120s | 0.001015s | 0.001044s | byte equality observed |
| swizzle4_test2.test | 10 | 0.001115s | 0.001060s | 0.001056s | byte equality observed |
| swizzle4_test3.test | 10 | 0.001029s | 0.001039s | 0.001044s | byte equality observed |
| swizzle4_test4.test | 10 | 0.001077s | 0.001064s | 0.001088s | byte equality observed |
| to_string_test.test | 10 | 0.001160s | 0.001056s | 0.001081s | byte equality observed |
| transform_test.test | 10 | 0.001139s | 0.001056s | 0.001055s | byte equality observed |
| translation_mat_test.test | 10 | 0.001125s | 0.001036s | 0.001012s | byte equality observed |
| translation_test.test | 10 | 0.001100s | 0.001029s | 0.001035s | byte equality observed |
| transpose_test.test | 10 | 0.001117s | 0.001032s | 0.001054s | byte equality observed |
| vec_index_test.test | 10 | 0.001136s | 0.001025s | 0.001055s | byte equality observed |
| vec_register_test.test | 10 | 0.001094s | 0.001030s | 0.001051s | byte equality observed |
| vec_traits_array_test.test | 10 | 0.001115s | 0.001070s | 0.001059s | byte equality observed |
| vec_traits_gnuc_test.test | 10 | 0.001084s | 0.001018s | 0.001066s | byte equality observed |
| vec_traits_std_array_test.test | 10 | 0.001144s | 0.001013s | 0.001021s | byte equality observed |
| zero_mat_test.test | 10 | 0.001114s | 0.000996s | 0.001055s | byte equality observed |
| zero_quat_test.test | 10 | 0.001094s | 0.001008s | 0.001081s | byte equality observed |
| zero_vec_test.test | 10 | 0.001114s | 0.001001s | 0.001053s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
