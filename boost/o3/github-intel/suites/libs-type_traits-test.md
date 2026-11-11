# libs/type_traits/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| type_with_alignment_test.test | RUN | 0.152829 | 0.395779 | 0.544261 | 0.001595 | 0.001661 | 0.001594 |
| type_traits_test.test | RUN | 0.213181 | 0.405385 | 0.520185 | 0.000640 | 0.000694 | 0.001202 |
| type_identity_test.test | RUN | 0.364822 | 1.372716 | 2.863220 | 0.001733 | 0.001795 | 0.001731 |
| tricky_rvalue_test.test | RUN | 0.139347 | 0.317355 | 0.412978 | 0.001750 | 0.001782 | 0.001558 |
| tricky_partial_spec_test.test | RUN | 0.172280 | 0.447331 | 0.686166 | 0.001660 | 0.001833 | 0.001407 |
| tricky_is_enum_test.test | RUN | 0.132902 | 0.299465 | 0.374587 | 0.001661 | 0.001648 | 0.001757 |
| tricky_incomplete_type_test.test | RUN | 0.221792 | 0.461831 | 0.609449 | 0.001660 | 0.001797 | 0.001537 |
| tricky_function_type_test.test | RUN | 0.182644 | 0.494531 | 0.746843 | 0.001576 | 0.001381 | 0.001709 |
| tricky_add_pointer_test.test | RUN | 0.255705 | 0.827412 | 1.645588 | 0.001654 | 0.001807 | 0.001686 |
| tricky_abstract_type_test.test | RUN | 0.142900 | 0.316986 | 0.407141 | 0.001957 | 0.001603 | 0.001703 |
| remove_volatile_test.test | RUN | 0.334590 | 1.210763 | 2.570680 | 0.001510 | 0.001829 | 0.001724 |
| remove_reference_test.test | RUN | 0.339477 | 1.260797 | 2.672264 | 0.001697 | 0.001934 | 0.001729 |
| remove_pointer_test.test | RUN | 0.359246 | 1.338289 | 2.860997 | 0.001721 | 0.001713 | 0.001638 |
| remove_extent_test.test | RUN | 0.341999 | 1.272879 | 2.747489 | 0.001538 | 0.001768 | 0.001743 |
| remove_cv_test.test | RUN | 0.340587 | 1.268768 | 2.696877 | 0.001730 | 0.001807 | 0.001699 |
| remove_cv_ref_test.test | RUN | 0.704900 | 3.059939 | 6.499724 | 0.001852 | 0.001626 | 0.001556 |
| remove_const_test.test | RUN | 0.332444 | 1.203738 | 2.575945 | 0.002128 | 0.001778 | 0.001694 |
| remove_bounds_test.test | RUN | 0.347331 | 1.285164 | 2.714736 | 0.001627 | 0.001821 | 0.001644 |
| remove_all_extents_test.test | RUN | 0.351175 | 1.274637 | 2.606080 | 0.001785 | 0.001638 | 0.001601 |
| rank_test.test | RUN | 0.138184 | 0.323910 | 0.425938 | 0.001543 | 0.001801 | 0.001725 |
| promote_mpl_test.test | RUN | 0.128266 | 0.286175 | 0.310060 | 0.000630 | 0.000611 | 0.001442 |
| promote_enum_test.test | RUN | 0.026350 | 0.068655 | 0.072661 | 0.000647 | 0.000759 | 0.001708 |
| promote_enum_msvc_bug_test.test | RUN | 0.024274 | 0.057249 | 0.059788 | 0.000731 | 0.000688 | 0.001767 |
| promote_basic_test.test | RUN | 0.030342 | 0.080500 | 0.075104 | 0.000618 | 0.000766 | 0.001687 |
| negation_test.test | RUN | 0.133214 | 0.290019 | 0.383722 | 0.001502 | 0.001610 | 0.001523 |
| mpl_interop_test3.test | RUN | 0.120585 | 0.262600 | 0.296711 | 0.000576 | 0.000786 | 0.001718 |
| mpl_interop_test2.test | RUN | 0.044057 | 0.072808 | 0.071424 | 0.000645 | 0.000548 | 0.001692 |
| mpl_interop_test1.test | RUN | 0.022135 | 0.054308 | 0.051172 | 0.000794 | 0.000789 | 0.001604 |
| make_void_test.test | RUN | 0.142854 | 0.308526 | 0.409164 | 0.001641 | 0.001906 | 0.001676 |
| make_unsigned_test.test | RUN | 0.165323 | 0.418650 | 0.574442 | 0.001570 | 0.001734 | 0.001766 |
| make_signed_test.test | RUN | 0.162792 | 0.415921 | 0.570495 | 0.001492 | 0.001768 | 0.001482 |
| is_volatile_test.test | RUN | 0.151089 | 0.352187 | 0.476489 | 0.001726 | 0.001485 | 0.001692 |
| is_void_test.test | RUN | 0.144443 | 0.322871 | 0.433805 | 0.001648 | 0.001780 | 0.001681 |
| is_virtual_base_of_test.test | RUN | 0.167344 | 0.409955 | 0.621433 | 0.001578 | 0.001544 | 0.001613 |
| is_unsigned_test.test | RUN | 0.149060 | 0.336288 | 0.472489 | 0.001667 | 0.001676 | 0.001628 |
| is_unscoped_enum_test.test | RUN | 0.140339 | 0.302714 | 0.362495 | 0.001725 | 0.001878 | 0.001551 |
| is_union_test.test | RUN | 0.155242 | 0.373599 | 0.551986 | 0.001618 | 0.001796 | 0.001698 |
| is_unbounded_array_test.test | RUN | 0.140530 | 0.323520 | 0.434290 | 0.001680 | 0.001762 | 0.001686 |
| is_trivially_copyable_test.test | RUN | 0.226536 | 0.672347 | 1.119674 | 0.001716 | 0.001801 | 0.001591 |
| is_swappable_test.test | RUN | 0.190339 | 0.505952 | 0.804180 | 0.001611 | 0.001745 | 0.001676 |
| is_stateless_test.test | RUN | 0.211671 | 0.588698 | 0.942319 | 0.001489 | 0.001649 | 0.001758 |
| is_signed_test.test | RUN | 0.146303 | 0.332429 | 0.442218 | 0.001446 | 0.001795 | 0.001692 |
| is_scoped_enum_test.test | RUN | 0.140683 | 0.309515 | 0.384764 | 0.001681 | 0.001650 | 0.001512 |
| is_scalar_test.test | RUN | 0.188120 | 0.519669 | 0.800972 | 0.001772 | 0.001773 | 0.001668 |
| is_same_test.test | RUN | 0.141841 | 0.320176 | 0.419836 | 0.001568 | 0.001667 | 0.001765 |
| is_rvalue_reference_test.test | RUN | 0.150491 | 0.360656 | 0.516826 | 0.001501 | 0.001553 | 0.001695 |
| is_reference_test.test | RUN | 0.148383 | 0.357122 | 0.518759 | 0.001521 | 0.001752 | 0.001540 |
| is_polymorphic_test.test | RUN | 0.155484 | 0.377484 | 0.520069 | 0.001688 | 0.001652 | 0.001712 |
| is_pointer_test.test | RUN | 0.150847 | 0.374638 | 0.527193 | 0.001752 | 0.001515 | 0.001790 |
| is_pod_test.test | RUN | 0.199285 | 0.610042 | 0.968671 | 0.001470 | 0.001854 | 0.001693 |
| is_object_test.test | RUN | 0.140202 | 0.316152 | 0.406307 | 0.001659 | 0.001648 | 0.001720 |
| is_nothrow_swappable_test.test | RUN | 0.204111 | 0.567524 | 0.899686 | 0.001688 | 0.001801 | 0.001700 |
| is_nothrow_move_constructible_test.test | RUN | 0.197001 | 0.582647 | 0.952817 | 0.001540 | 0.001522 | 0.001612 |
| is_nothrow_move_assignable_test.test | RUN | 0.198819 | 0.592561 | 0.931944 | 0.001620 | 0.001538 | 0.001433 |
| is_noncopyable_test.test | RUN | 0.141105 | 0.318199 | 0.405271 | 0.001530 | 0.001834 | 0.001526 |
| is_member_pointer_test.test | RUN | 0.143883 | 0.324618 | 0.418270 | 0.001705 | 0.001897 | 0.001669 |
| is_member_obj_test.test | RUN | 0.144110 | 0.333345 | 0.443329 | 0.001754 | 0.001738 | 0.001701 |
| is_member_func_test.test | RUN | 0.146152 | 0.347936 | 0.478026 | 0.001518 | 0.001557 | 0.001558 |
| is_lvalue_reference_test.test | RUN | 0.148622 | 0.366198 | 0.505348 | 0.001568 | 0.001726 | 0.001440 |
| is_list_constructible_test.test | RUN | 0.177343 | 0.428959 | 0.555373 | 0.001716 | 0.001479 | 0.001758 |
| is_integral_test.test | RUN | 0.179679 | 0.515385 | 0.807571 | 0.001499 | 0.001828 | 0.001646 |
| is_fundamental_test.test | RUN | 0.182675 | 0.525129 | 0.822631 | 0.001618 | 0.001729 | 0.001703 |
| is_function_test.test | RUN | 0.148603 | 0.342078 | 0.440300 | 0.001534 | 0.001675 | 0.001702 |
| is_floating_point_test.test | RUN | 0.152923 | 0.364185 | 0.506385 | 0.001828 | 0.001713 | 0.001585 |
| is_float_test.test | RUN | 0.152902 | 0.368351 | 0.539870 | 0.001750 | 0.001716 | 0.001685 |
| is_final_test.test | RUN | 0.151822 | 0.376677 | 0.534572 | 0.001749 | 0.001716 | 0.001556 |
| is_enum_test.test | RUN | 0.140221 | 0.308905 | 0.418550 | 0.001488 | 0.001599 | 0.001482 |
| is_empty_test.test | RUN | 0.150606 | 0.340431 | 0.475144 | 0.001699 | 0.001634 | 0.001706 |
| is_detected_test.test | RUN | 0.138755 | 0.294838 | 0.359666 | 0.001567 | 0.001719 | 0.001914 |
| is_detected_exact_test.test | RUN | 0.136223 | 0.288386 | 0.384620 | 0.001544 | 0.001519 | 0.001708 |
| is_destructible_test.test | RUN | 0.200537 | 0.560671 | 0.913664 | 0.001658 | 0.001771 | 0.001622 |
| is_default_constr_test.test | RUN | 0.204876 | 0.596372 | 0.952559 | 0.001753 | 0.001706 | 0.001720 |
| is_copy_constructible_test.test | RUN | 0.209904 | 0.618743 | 1.002118 | 0.001648 | 0.001735 | 0.001690 |
| is_copy_assignable_test.test | RUN | 0.212943 | 0.611985 | 0.981612 | 0.001826 | 0.001563 | 0.001720 |
| is_convertible_test.test | RUN | 0.200127 | 0.537873 | 0.785244 | 0.001730 | 0.001802 | 0.001624 |
| is_constructible_test.test | RUN | 0.159805 | 0.399578 | 0.557405 | 0.001716 | 0.001534 | 0.001703 |
| is_const_test.test | RUN | 0.147971 | 0.340192 | 0.463948 | 0.001739 | 0.001583 | 0.001622 |
| is_compound_test.test | RUN | 0.138668 | 0.303735 | 0.372616 | 0.001692 | 0.001470 | 0.001624 |
| is_complex_test.test | RUN | 0.188964 | 0.392826 | 0.529094 | 0.001441 | 0.001834 | 0.001545 |
| is_complete_test.test | RUN | 0.166451 | 0.407203 | 0.597593 | 0.001553 | 0.001402 | 0.001623 |
| is_class_test.test | RUN | 0.150460 | 0.359916 | 0.494973 | 0.001426 | 0.001487 | 0.001807 |
| is_bounded_array_test.test | RUN | 0.140788 | 0.314897 | 0.431706 | 0.001558 | 0.001608 | 0.001437 |
| is_base_of_test.test | RUN | 0.149872 | 0.351081 | 0.501505 | 0.001607 | 0.001863 | 0.001658 |
| is_base_and_derived_test.test | RUN | 0.151354 | 0.352413 | 0.483003 | 0.001574 | 0.001522 | 0.001590 |
| is_assignable_test.test | RUN | 0.184791 | 0.495115 | 0.751945 | 0.001682 | 0.001819 | 0.001589 |
| is_array_test.test | RUN | 0.146833 | 0.342740 | 0.488878 | 0.001593 | 0.001476 | 0.001695 |
| is_arithmetic_test.test | RUN | 0.180649 | 0.497741 | 0.801340 | 0.001555 | 0.001860 | 0.001645 |
| is_abstract_test.test | RUN | 0.330672 | 1.347064 | 2.744009 | 0.001518 | 0.001808 | 0.001682 |
| integral_constant_test.test | RUN | 0.020545 | 0.051550 | 0.052342 | 0.000796 | 0.000749 | 0.001668 |
| has_virtual_destructor_test.test | RUN | 0.157834 | 0.392287 | 0.577994 | 0.001682 | 0.001758 | 0.001767 |
| has_unary_plus_test.test | RUN | 0.284109 | 1.369989 | 2.086001 | 0.001678 | 0.001764 | 0.001748 |
| has_unary_minus_test.test | RUN | 0.282468 | 1.367944 | 2.022898 | 0.001509 | 0.001749 | 0.001713 |
| has_trivial_move_constructor_test.test | RUN | 0.207375 | 0.600270 | 1.023266 | 0.001570 | 0.001766 | 0.001787 |
| has_trivial_move_assign_test.test | RUN | 0.202548 | 0.575769 | 0.954696 | 0.001658 | 0.001830 | 0.001767 |
| has_trivial_destructor_test.test | RUN | 0.200978 | 0.604137 | 0.986744 | 0.001698 | 0.001599 | 0.001602 |
| has_trivial_copy_test.test | RUN | 0.210887 | 0.629513 | 0.996634 | 0.001706 | 0.001897 | 0.001840 |
| has_trivial_constr_test.test | RUN | 0.209952 | 0.631540 | 1.016402 | 0.001785 | 0.001912 | 0.001815 |
| has_trivial_assign_test.test | RUN | 0.206308 | 0.627054 | 1.015934 | 0.001649 | 0.001883 | 0.001862 |
| has_right_shift_test.test | RUN | 0.296700 | 1.162776 | 2.176586 | 0.001606 | 0.001468 | 0.001658 |
| has_right_shift_assign_test.test | RUN | 0.290549 | 1.086540 | 2.057737 | 0.001547 | 0.001821 | 0.001699 |
| has_prefix_classes3_test.test | RUN | 0.214753 | 0.650536 | 1.075761 | 0.001932 | 0.001572 | 0.001682 |
| has_prefix_classes2_test.test | RUN | 0.214235 | 0.636727 | 1.038481 | 0.001531 | 0.001836 | 0.001689 |
| has_prefix_classes1_test.test | RUN | 0.212860 | 0.652664 | 0.970556 | 0.001650 | 0.001816 | 0.001704 |
| has_prefix_classes0_test.test | RUN | 0.214058 | 0.643429 | 1.046142 | 0.001641 | 0.001626 | 0.001675 |
| has_pre_increment_test.test | RUN | 0.285616 | 1.372576 | 2.015233 | 0.001626 | 0.001473 | 0.001690 |
| has_pre_decrement_test.test | RUN | 0.284811 | 1.392118 | 1.998657 | 0.001598 | 0.001613 | 0.001710 |
| has_postfix_classes3_test.test | RUN | 0.211758 | 0.631549 | 1.078851 | 0.001738 | 0.001647 | 0.001724 |
| has_postfix_classes2_test.test | RUN | 0.212764 | 0.639220 | 0.987345 | 0.001537 | 0.001626 | 0.001652 |
| has_postfix_classes1_test.test | RUN | 0.216251 | 0.654356 | 1.045258 | 0.001600 | 0.001683 | 0.001700 |
| has_postfix_classes0_test.test | RUN | 0.212665 | 0.658200 | 1.066385 | 0.001594 | 0.001586 | 0.001687 |
| has_post_increment_test.test | RUN | 0.280409 | 1.362834 | 2.051577 | 0.001698 | 0.001633 | 0.001851 |
| has_post_decrement_test.test | RUN | 0.284893 | 1.381318 | 2.054277 | 0.001529 | 0.001948 | 0.001506 |
| has_plus_test.test | RUN | 0.285903 | 1.070194 | 2.005318 | 0.001689 | 0.001876 | 0.001652 |
| has_plus_assign_test.test | RUN | 0.294344 | 1.086235 | 2.027419 | 0.001586 | 0.001460 | 0.001664 |
| has_operator_new_test.test | RUN | 0.201138 | 0.556308 | 0.870222 | 0.001612 | 0.001655 | 0.001659 |
| has_nothrow_destructor_test.test | RUN | 0.209070 | 0.621403 | 0.991210 | 0.001662 | 0.001822 | 0.001734 |
| has_nothrow_copy_test.test | RUN | 0.204793 | 0.603298 | 0.966450 | 0.001646 | 0.001644 | 0.001720 |
| has_nothrow_constr_test.test | RUN | 0.196220 | 0.585658 | 0.964099 | 0.001748 | 0.001539 | 0.001546 |
| has_nothrow_assign_test.test | RUN | 0.198532 | 0.590346 | 0.927594 | 0.001796 | 0.001753 | 0.001519 |
| has_not_equal_to_test.test | RUN | 0.288109 | 1.354909 | 2.064070 | 0.001825 | 0.001648 | 0.001763 |
| has_negate_test.test | RUN | 0.273259 | 1.063772 | 2.002664 | 0.001536 | 0.001713 | 0.001752 |
| has_multiplies_test.test | RUN | 0.285995 | 1.091868 | 1.997790 | 0.001717 | 0.001445 | 0.001720 |
| has_multiplies_assign_test.test | RUN | 0.285536 | 1.063919 | 2.055270 | 0.001880 | 0.001793 | 0.001680 |
| has_modulus_test.test | RUN | 0.280752 | 1.090092 | 1.997411 | 0.001630 | 0.001627 | 0.001718 |
| has_modulus_assign_test.test | RUN | 0.284682 | 1.097769 | 2.125330 | 0.001640 | 0.001720 | 0.001774 |
| has_minus_test.test | RUN | 0.288492 | 1.100350 | 2.045530 | 0.001830 | 0.001507 | 0.001653 |
| has_minus_assign_test.test | RUN | 0.284795 | 1.097706 | 2.126142 | 0.001738 | 0.001663 | 0.001738 |
| has_logical_or_test.test | RUN | 0.288224 | 1.100169 | 1.980836 | 0.001738 | 0.001638 | 0.001765 |
| has_logical_not_test.test | RUN | 0.277398 | 1.056597 | 2.027477 | 0.001557 | 0.001677 | 0.001780 |
| has_logical_and_test.test | RUN | 0.292489 | 1.105200 | 2.063759 | 0.001495 | 0.001644 | 0.001664 |
| has_less_test.test | RUN | 0.284532 | 1.334117 | 2.052852 | 0.001794 | 0.001757 | 0.001710 |
| has_less_equal_test.test | RUN | 0.287671 | 1.364051 | 2.027595 | 0.001656 | 0.001600 | 0.001735 |
| has_left_shift_test.test | RUN | 0.313231 | 1.386383 | 2.612740 | 0.001740 | 0.001701 | 0.001735 |
| has_left_shift_assign_test.test | RUN | 0.285926 | 1.100470 | 2.095105 | 0.001561 | 0.001714 | 0.001724 |
| has_greater_test.test | RUN | 0.283533 | 1.335969 | 2.037068 | 0.001780 | 0.001807 | 0.001519 |
| has_greater_equal_test.test | RUN | 0.288400 | 1.366507 | 2.036231 | 0.001578 | 0.001477 | 0.001693 |
| has_equal_to_test.test | RUN | 0.290359 | 1.347811 | 2.037116 | 0.001714 | 0.001551 | 0.001671 |
| has_divides_test.test | RUN | 0.285061 | 1.103542 | 2.094742 | 0.001669 | 0.001932 | 0.001706 |
| has_divides_assign_test.test | RUN | 0.286328 | 1.057882 | 2.078464 | 0.001760 | 0.001750 | 0.001678 |
| has_dereference_test.test | RUN | 0.277045 | 1.058021 | 1.943120 | 0.001503 | 0.001606 | 0.001659 |
| has_complement_test.test | RUN | 0.280975 | 1.344987 | 2.006553 | 0.001730 | 0.001827 | 0.001765 |
| has_bit_xor_test.test | RUN | 0.289996 | 1.100016 | 2.103626 | 0.001541 | 0.001813 | 0.001681 |
| has_bit_xor_assign_test.test | RUN | 0.282744 | 1.074785 | 2.081365 | 0.001805 | 0.001781 | 0.001626 |
| has_bit_or_test.test | RUN | 0.284833 | 1.084616 | 1.970956 | 0.001638 | 0.001646 | 0.001678 |
| has_bit_or_assign_test.test | RUN | 0.280290 | 1.106755 | 2.090648 | 0.001642 | 0.001642 | 0.001703 |
| has_bit_and_test.test | RUN | 0.287072 | 1.085207 | 2.054039 | 0.001590 | 0.001746 | 0.001626 |
| has_bit_and_assign_test.test | RUN | 0.282242 | 1.087382 | 2.069551 | 0.001728 | 0.001704 | 0.001744 |
| has_binary_classes9_test.test | RUN | 0.302744 | 1.287892 | 2.289975 | 0.001646 | 0.001726 | 0.001704 |
| has_binary_classes8_test.test | RUN | 0.311770 | 1.308815 | 2.320614 | 0.001690 | 0.001724 | 0.001770 |
| has_binary_classes7_test.test | RUN | 0.313701 | 1.252069 | 2.289491 | 0.001575 | 0.001795 | 0.001562 |
| has_binary_classes6_test.test | RUN | 0.303041 | 1.256898 | 2.325288 | 0.001770 | 0.001780 | 0.001676 |
| has_binary_classes5_test.test | RUN | 0.301025 | 1.298988 | 2.276033 | 0.001713 | 0.001788 | 0.001709 |
| has_binary_classes4_test.test | RUN | 0.308659 | 1.305622 | 2.341947 | 0.001692 | 0.001626 | 0.001498 |
| has_binary_classes3_test.test | RUN | 0.308787 | 1.295501 | 2.337639 | 0.001622 | 0.001584 | 0.001697 |
| has_binary_classes2_test.test | RUN | 0.305676 | 1.307446 | 2.291155 | 0.001884 | 0.001828 | 0.001629 |
| has_binary_classes1_test.test | RUN | 0.302976 | 1.273750 | 2.244403 | 0.001586 | 0.001570 | 0.001460 |
| has_binary_classes0_test.test | RUN | 0.307304 | 1.317015 | 2.256488 | 0.001688 | 0.001825 | 0.001742 |
| function_traits_test.test | RUN | 0.147568 | 0.345798 | 0.495052 | 0.001504 | 0.001750 | 0.001684 |
| floating_point_promotion_test.test | RUN | 0.143324 | 0.342348 | 0.415045 | 0.001515 | 0.001833 | 0.001618 |
| extent_test.test | RUN | 0.142016 | 0.341785 | 0.475331 | 0.001561 | 0.001921 | 0.001550 |
| enable_if_test.test | RUN | 0.136245 | 0.287566 | 0.372042 | 0.001687 | 0.001599 | 0.001737 |
| disjunction_test.test | RUN | 0.136974 | 0.298741 | 0.396672 | 0.001686 | 0.001608 | 0.001621 |
| detected_test.test | RUN | 0.136752 | 0.283999 | 0.356932 | 0.001552 | 0.001625 | 0.001756 |
| detected_or_test.test | RUN | 0.135981 | 0.306383 | 0.403721 | 0.001603 | 0.001543 | 0.001633 |
| decay_test.test | RUN | 0.331626 | 1.166400 | 2.221308 | 0.001674 | 0.001872 | 0.001731 |
| cxx14_aliases_test.test | RUN | 0.222092 | 0.431435 | 0.599280 | 0.001484 | 0.001484 | 0.001549 |
| copy_reference_test.test | RUN | 0.139551 | 0.301868 | 0.401765 | 0.001591 | 0.001736 | 0.001515 |
| copy_cv_test.test | RUN | 0.141150 | 0.315606 | 0.426320 | 0.001714 | 0.001566 | 0.001658 |
| copy_cv_ref_test.test | RUN | 0.189059 | 0.527287 | 0.928125 | 0.001649 | 0.001767 | 0.001684 |
| conjunction_test.test | RUN | 0.135357 | 0.303682 | 0.405951 | 0.001582 | 0.001631 | 0.001782 |
| conditional_test.test | RUN | 0.135657 | 0.299176 | 0.390807 | 0.001477 | 0.001596 | 0.001755 |
| common_type_test.test | RUN | 0.180902 | 0.459878 | 0.759149 | 0.001700 | 0.001635 | 0.001653 |
| common_type_sfinae_test.test | RUN | 0.135601 | 0.286699 | 0.348171 | 0.001602 | 0.001576 | 0.001713 |
| common_type_sfinae2_test.test | RUN | 0.140834 | 0.314156 | 0.401939 | 0.001483 | 0.001490 | 0.001699 |
| common_type_6_test.test | RUN | 0.145267 | 0.307415 | 0.400316 | 0.001551 | 0.001614 | 0.001692 |
| common_type_5_test.test | RUN | 0.143545 | 0.310886 | 0.390475 | 0.001684 | 0.001874 | 0.001732 |
| common_type_4_test.test | RUN | 0.149340 | 0.336711 | 0.466387 | 0.001896 | 0.001525 | 0.001561 |
| common_type_3_test.test | RUN | 0.134794 | 0.291234 | 0.355321 | 0.001677 | 0.001737 | 0.001486 |
| common_type_2_test.test | RUN | 0.179881 | 0.451993 | 0.744866 | 0.001589 | 0.001169 | 0.001402 |
| alignment_of_test.test | RUN | 0.147292 | 0.306501 | 0.497802 | 0.001728 | 0.001688 | 0.001639 |
| alignment_of_a2_test.test | RUN | 0.148127 | 0.359201 | 0.484991 | 0.001662 | 0.001655 | 0.001866 |
| aligned_storage_test.test | RUN | 0.157051 | 0.355536 | 0.449806 | 0.001594 | 0.001798 | 0.001808 |
| aligned_storage_empy_test.test | RUN | 0.175180 | 0.396934 | 0.593232 | 0.001736 | 0.001767 | 0.001508 |
| aligned_storage_a2_test.test | RUN | 0.154898 | 0.352058 | 0.455025 | 0.001862 | 0.001944 | 0.001705 |
| add_volatile_test.test | RUN | 0.305476 | 1.047167 | 2.194137 | 0.001823 | 0.001462 | 0.001698 |
| add_rvalue_reference_test.test | RUN | 0.334515 | 1.187083 | 2.470415 | 0.001562 | 0.001570 | 0.001693 |
| add_reference_test.test | RUN | 0.328401 | 1.147619 | 2.459243 | 0.001724 | 0.001710 | 0.001742 |
| add_pointer_test.test | RUN | 0.217382 | 0.655205 | 1.300081 | 0.001532 | 0.001734 | 0.001760 |
| add_lvalue_reference_test.test | RUN | 0.325402 | 1.162426 | 2.399462 | 0.001554 | 0.001765 | 0.001681 |
| add_cv_test.test | RUN | 0.311332 | 1.084026 | 2.355156 | 0.001456 | 0.001615 | 0.001526 |
| add_const_test.test | RUN | 0.312764 | 1.082174 | 2.339760 | 0.001675 | 0.001498 | 0.001698 |
| is_virtual_base_of_fail.test | COMPILE_FAIL | 0.129816 | 0.282258 | 0.363278 | — | — | — |
| is_stateless_fail.test | COMPILE_FAIL | 0.137818 | 0.284623 | 0.371530 | — | — | — |
| is_polymorphic_fail.test | COMPILE_FAIL | 0.128006 | 0.266788 | 0.355046 | — | — | — |
| is_nothrow_move_constructible_fail.test | COMPILE_FAIL | 0.129508 | 0.270365 | 0.339647 | — | — | — |
| is_nothrow_move_assignable_fail.test | COMPILE_FAIL | 0.132187 | 0.274894 | 0.350294 | — | — | — |
| is_list_constructible_fail.test | COMPILE_FAIL | 0.129956 | 0.277460 | 0.340726 | — | — | — |
| is_empty_fail.test | COMPILE_FAIL | 0.134157 | 0.272750 | 0.346673 | — | — | — |
| is_destructible_fail.test | COMPILE_FAIL | 0.129612 | 0.266614 | 0.338636 | — | — | — |
| is_default_constructible_fail.test | COMPILE_FAIL | 0.129764 | 0.273498 | 0.343768 | — | — | — |
| is_copy_constructible_fail.test | COMPILE_FAIL | 0.129392 | 0.276249 | 0.349349 | — | — | — |
| is_copy_assignable_fail.test | COMPILE_FAIL | 0.129413 | 0.273782 | 0.346553 | — | — | — |
| is_convertible_fail.test | COMPILE_FAIL | 0.130115 | 0.277589 | 0.329682 | — | — | — |
| is_convertible2_fail.test | COMPILE_FAIL | 0.133239 | 0.275989 | 0.343001 | — | — | — |
| is_constructible_fail.test | COMPILE_FAIL | 0.129561 | 0.274331 | 0.346652 | — | — | — |
| is_base_of_fail.test | COMPILE_FAIL | 0.127679 | 0.275391 | 0.335368 | — | — | — |
| is_assignable_fail.test | COMPILE_FAIL | 0.129623 | 0.273112 | 0.344453 | — | — | — |
| has_nothrow_destruct_fail.test | COMPILE_FAIL | 0.131276 | 0.272559 | 0.335468 | — | — | — |
| has_nothrow_copy_fail.test | COMPILE_FAIL | 0.130144 | 0.272828 | 0.353483 | — | — | — |
| has_nothrow_cons_fail.test | COMPILE_FAIL | 0.129087 | 0.278550 | 0.349657 | — | — | — |
| has_nothrow_assign_fail.test | COMPILE_FAIL | 0.131285 | 0.274103 | 0.346329 | — | — | — |
| common_type_fail.test | COMPILE_FAIL | 0.132257 | 0.271070 | 0.350757 | — | — | — |
| common_type2_fail.test | COMPILE_FAIL | 0.129331 | 0.270689 | 0.349426 | — | — | — |
| has_nothrow_assign_test_no_intrinsics.test | RUN | 0.201742 | 0.584384 | 0.977700 | 0.001569 | 0.001890 | 0.001568 |
| has_nothrow_constr_test_no_intrinsics.test | RUN | 0.197880 | 0.583719 | 0.990889 | 0.001791 | 0.002582 | 0.001649 |
| has_nothrow_copy_test_no_intrinsics.test | RUN | 0.203976 | 0.582751 | 0.990624 | 0.001782 | 0.002506 | 0.001236 |
| is_nothrow_move_assignable_test_no_intrinsics.test | RUN | 0.203011 | 0.585502 | 0.927423 | 0.001308 | 0.002040 | 0.001172 |
| is_nothrow_move_constructible_test_no_intrinsics.test | RUN | 0.129332 | 0.533697 | 0.650905 | 0.001296 | 0.001265 | 0.001256 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.996× | 0.995×, 0.997× | 1.015× |
| clang | 1.025× | 1.030×, 1.020× | 0.997× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| add_const_test.test | 10 | 0.001217s | 0.001154s | 0.001217s | byte equality observed |
| add_cv_test.test | 10 | 0.001211s | 0.001183s | 0.001155s | byte equality observed |
| add_lvalue_reference_test.test | 10 | 0.001203s | 0.001191s | 0.001202s | byte equality observed |
| add_pointer_test.test | 10 | 0.001171s | 0.001147s | 0.001190s | byte equality observed |
| add_reference_test.test | 10 | 0.001186s | 0.001171s | 0.001157s | byte equality observed |
| add_rvalue_reference_test.test | 10 | 0.001221s | 0.001189s | 0.001175s | byte equality observed |
| add_volatile_test.test | 10 | 0.001196s | 0.001185s | 0.001196s | byte equality observed |
| aligned_storage_a2_test.test | 10 | 0.001265s | 0.001277s | 0.001284s | byte equality observed |
| aligned_storage_empy_test.test | 10 | 0.001197s | 0.001171s | 0.001182s | byte equality observed |
| aligned_storage_test.test | 10 | 0.001270s | 0.001268s | 0.001270s | byte equality observed |
| alignment_of_a2_test.test | 10 | 0.001169s | 0.001176s | 0.001197s | byte equality observed |
| alignment_of_test.test | 10 | 0.001183s | 0.001191s | 0.001186s | byte equality observed |
| common_type_2_test.test | 10 | 0.001191s | 0.001173s | 0.001168s | byte equality observed |
| common_type_3_test.test | 10 | 0.001157s | 0.001194s | 0.001202s | byte equality observed |
| common_type_4_test.test | 10 | 0.001180s | 0.001192s | 0.001189s | byte equality observed |
| common_type_5_test.test | 10 | 0.001172s | 0.001177s | 0.001179s | byte equality observed |
| common_type_6_test.test | 10 | 0.001169s | 0.001183s | 0.001177s | byte equality observed |
| common_type_sfinae2_test.test | 10 | 0.001198s | 0.001154s | 0.001171s | byte equality observed |
| common_type_sfinae_test.test | 10 | 0.001181s | 0.001170s | 0.001176s | byte equality observed |
| common_type_test.test | 10 | 0.001169s | 0.001167s | 0.001173s | byte equality observed |
| conditional_test.test | 10 | 0.001171s | 0.001171s | 0.001162s | byte equality observed |
| conjunction_test.test | 10 | 0.001157s | 0.001180s | 0.001175s | byte equality observed |
| copy_cv_ref_test.test | 10 | 0.001180s | 0.001187s | 0.001193s | byte equality observed |
| copy_cv_test.test | 10 | 0.001174s | 0.001176s | 0.001163s | byte equality observed |
| copy_reference_test.test | 10 | 0.001181s | 0.001183s | 0.001194s | byte equality observed |
| cxx14_aliases_test.test | 10 | 0.001185s | 0.001174s | 0.001174s | byte equality observed |
| decay_test.test | 10 | 0.001171s | 0.001189s | 0.001183s | byte equality observed |
| detected_or_test.test | 10 | 0.001171s | 0.001175s | 0.001170s | byte equality observed |
| detected_test.test | 10 | 0.001173s | 0.001177s | 0.001173s | byte equality observed |
| disjunction_test.test | 10 | 0.001194s | 0.001175s | 0.001173s | byte equality observed |
| enable_if_test.test | 10 | 0.001199s | 0.001168s | 0.001178s | byte equality observed |
| extent_test.test | 10 | 0.001181s | 0.001180s | 0.001175s | byte equality observed |
| floating_point_promotion_test.test | 10 | 0.001173s | 0.001187s | 0.001180s | byte equality observed |
| function_traits_test.test | 10 | 0.001171s | 0.001194s | 0.001173s | byte equality observed |
| has_binary_classes0_test.test | 10 | 0.001172s | 0.001188s | 0.001173s | byte equality observed |
| has_binary_classes1_test.test | 10 | 0.001182s | 0.001184s | 0.001186s | byte equality observed |
| has_binary_classes2_test.test | 10 | 0.001195s | 0.001191s | 0.001167s | byte equality observed |
| has_binary_classes3_test.test | 10 | 0.001187s | 0.001161s | 0.001184s | byte equality observed |
| has_binary_classes4_test.test | 10 | 0.001173s | 0.001173s | 0.001173s | byte equality observed |
| has_binary_classes5_test.test | 10 | 0.001180s | 0.001183s | 0.001179s | byte equality observed |
| has_binary_classes6_test.test | 10 | 0.001167s | 0.001192s | 0.001146s | byte equality observed |
| has_binary_classes7_test.test | 10 | 0.001177s | 0.001188s | 0.001172s | byte equality observed |
| has_binary_classes8_test.test | 10 | 0.001179s | 0.001165s | 0.001167s | byte equality observed |
| has_binary_classes9_test.test | 8 | 0.001164s | 0.001186s | 0.001176s | byte equality observed |
| has_bit_and_assign_test.test | 10 | 0.001166s | 0.001177s | 0.001163s | byte equality observed |
| has_bit_and_test.test | 9 | 0.001165s | 0.001179s | 0.001165s | byte equality observed |
| has_bit_or_assign_test.test | 9 | 0.001195s | 0.001179s | 0.001170s | byte equality observed |
| has_bit_or_test.test | 10 | 0.001179s | 0.001192s | 0.001175s | byte equality observed |
| has_bit_xor_assign_test.test | 10 | 0.001188s | 0.001161s | 0.001166s | byte equality observed |
| has_bit_xor_test.test | 10 | 0.001182s | 0.001178s | 0.001153s | byte equality observed |
| has_complement_test.test | 10 | 0.001192s | 0.001179s | 0.001169s | byte equality observed |
| has_dereference_test.test | 10 | 0.001183s | 0.001177s | 0.001181s | byte equality observed |
| has_divides_assign_test.test | 10 | 0.001189s | 0.001210s | 0.001171s | byte equality observed |
| has_divides_test.test | 10 | 0.001188s | 0.001164s | 0.001167s | byte equality observed |
| has_equal_to_test.test | 10 | 0.001195s | 0.001211s | 0.001174s | byte equality observed |
| has_greater_equal_test.test | 10 | 0.001172s | 0.001169s | 0.001192s | byte equality observed |
| has_greater_test.test | 10 | 0.001163s | 0.001169s | 0.001173s | byte equality observed |
| has_left_shift_assign_test.test | 10 | 0.001195s | 0.001186s | 0.001166s | byte equality observed |
| has_left_shift_test.test | 10 | 0.001183s | 0.001174s | 0.001172s | byte equality observed |
| has_less_equal_test.test | 10 | 0.001193s | 0.001204s | 0.001184s | byte equality observed |
| has_less_test.test | 10 | 0.001194s | 0.001197s | 0.001192s | byte equality observed |
| has_logical_and_test.test | 10 | 0.001183s | 0.001182s | 0.001171s | byte equality observed |
| has_logical_not_test.test | 10 | 0.001177s | 0.001177s | 0.001187s | byte equality observed |
| has_logical_or_test.test | 10 | 0.001189s | 0.001188s | 0.001165s | byte equality observed |
| has_minus_assign_test.test | 10 | 0.001192s | 0.001182s | 0.001181s | byte equality observed |
| has_minus_test.test | 10 | 0.001185s | 0.001182s | 0.001150s | byte equality observed |
| has_modulus_assign_test.test | 10 | 0.001178s | 0.001183s | 0.001177s | byte equality observed |
| has_modulus_test.test | 10 | 0.001197s | 0.001181s | 0.001159s | byte equality observed |
| has_multiplies_assign_test.test | 10 | 0.001186s | 0.001176s | 0.001173s | byte equality observed |
| has_multiplies_test.test | 10 | 0.001195s | 0.001181s | 0.001187s | byte equality observed |
| has_negate_test.test | 10 | 0.001198s | 0.001163s | 0.001184s | byte equality observed |
| has_not_equal_to_test.test | 10 | 0.001176s | 0.001176s | 0.001167s | byte equality observed |
| has_nothrow_assign_test.test | 10 | 0.001199s | 0.001185s | 0.001182s | byte equality observed |
| has_nothrow_assign_test_no_intrinsics.test | 10 | 0.001174s | 0.001186s | 0.001173s | byte equality observed |
| has_nothrow_constr_test.test | 10 | 0.001187s | 0.001195s | 0.001182s | byte equality observed |
| has_nothrow_constr_test_no_intrinsics.test | 10 | 0.001198s | 0.001186s | 0.001163s | byte equality observed |
| has_nothrow_copy_test.test | 10 | 0.001191s | 0.001184s | 0.001168s | byte equality observed |
| has_nothrow_copy_test_no_intrinsics.test | 10 | 0.001184s | 0.001204s | 0.001182s | byte equality observed |
| has_nothrow_destructor_test.test | 10 | 0.001189s | 0.001174s | 0.001169s | byte equality observed |
| has_operator_new_test.test | 10 | 0.001175s | 0.001178s | 0.001185s | byte equality observed |
| has_plus_assign_test.test | 10 | 0.001189s | 0.001198s | 0.001169s | byte equality observed |
| has_plus_test.test | 10 | 0.001198s | 0.001181s | 0.001162s | byte equality observed |
| has_post_decrement_test.test | 10 | 0.001186s | 0.001214s | 0.001177s | byte equality observed |
| has_post_increment_test.test | 10 | 0.001193s | 0.001186s | 0.001168s | byte equality observed |
| has_postfix_classes0_test.test | 10 | 0.001179s | 0.001173s | 0.001188s | byte equality observed |
| has_postfix_classes1_test.test | 10 | 0.001149s | 0.001188s | 0.001167s | byte equality observed |
| has_postfix_classes2_test.test | 10 | 0.001176s | 0.001178s | 0.001165s | byte equality observed |
| has_postfix_classes3_test.test | 10 | 0.001180s | 0.001192s | 0.001186s | byte equality observed |
| has_pre_decrement_test.test | 10 | 0.001172s | 0.001210s | 0.001183s | byte equality observed |
| has_pre_increment_test.test | 10 | 0.001183s | 0.001205s | 0.001179s | byte equality observed |
| has_prefix_classes0_test.test | 10 | 0.001178s | 0.001209s | 0.001155s | byte equality observed |
| has_prefix_classes1_test.test | 10 | 0.001175s | 0.001173s | 0.001159s | byte equality observed |
| has_prefix_classes2_test.test | 10 | 0.001179s | 0.001182s | 0.001160s | byte equality observed |
| has_prefix_classes3_test.test | 10 | 0.001198s | 0.001196s | 0.001169s | byte equality observed |
| has_right_shift_assign_test.test | 10 | 0.001187s | 0.001216s | 0.001157s | byte equality observed |
| has_right_shift_test.test | 10 | 0.001174s | 0.001238s | 0.001185s | byte equality observed |
| has_trivial_assign_test.test | 10 | 0.001239s | 0.001240s | 0.001195s | byte equality observed |
| has_trivial_constr_test.test | 10 | 0.001236s | 0.001234s | 0.001236s | native test oracle; output varies |
| has_trivial_copy_test.test | 10 | 0.001237s | 0.001263s | 0.001252s | native test oracle; output varies |
| has_trivial_destructor_test.test | 10 | 0.001198s | 0.001197s | 0.001206s | byte equality observed |
| has_trivial_move_assign_test.test | 10 | 0.001187s | 0.001188s | 0.001196s | byte equality observed |
| has_trivial_move_constructor_test.test | 10 | 0.001215s | 0.001188s | 0.001175s | byte equality observed |
| has_unary_minus_test.test | 10 | 0.001189s | 0.001197s | 0.001161s | byte equality observed |
| has_unary_plus_test.test | 10 | 0.001187s | 0.001194s | 0.001178s | byte equality observed |
| has_virtual_destructor_test.test | 10 | 0.001184s | 0.001185s | 0.001189s | byte equality observed |
| integral_constant_test.test | 10 | 0.000526s | 0.000529s | 0.001188s | byte equality observed |
| is_abstract_test.test | 10 | 0.001193s | 0.001181s | 0.001178s | byte equality observed |
| is_arithmetic_test.test | 10 | 0.001177s | 0.001182s | 0.001161s | byte equality observed |
| is_array_test.test | 10 | 0.001198s | 0.001177s | 0.001201s | byte equality observed |
| is_assignable_test.test | 10 | 0.001184s | 0.001193s | 0.001197s | byte equality observed |
| is_base_and_derived_test.test | 10 | 0.001190s | 0.001188s | 0.001181s | byte equality observed |
| is_base_of_test.test | 10 | 0.001170s | 0.001167s | 0.001183s | byte equality observed |
| is_bounded_array_test.test | 10 | 0.001165s | 0.001183s | 0.001172s | byte equality observed |
| is_class_test.test | 10 | 0.001185s | 0.001180s | 0.001169s | byte equality observed |
| is_complete_test.test | 10 | 0.001165s | 0.001147s | 0.001181s | byte equality observed |
| is_complex_test.test | 10 | 0.001190s | 0.001174s | 0.001174s | byte equality observed |
| is_compound_test.test | 10 | 0.001193s | 0.001181s | 0.001180s | byte equality observed |
| is_const_test.test | 10 | 0.001178s | 0.001176s | 0.001188s | byte equality observed |
| is_constructible_test.test | 10 | 0.001172s | 0.001188s | 0.001175s | byte equality observed |
| is_convertible_test.test | 10 | 0.001185s | 0.001190s | 0.001187s | byte equality observed |
| is_copy_assignable_test.test | 10 | 0.001185s | 0.001153s | 0.001192s | byte equality observed |
| is_copy_constructible_test.test | 10 | 0.001186s | 0.001176s | 0.001165s | byte equality observed |
| is_default_constr_test.test | 10 | 0.001198s | 0.001189s | 0.001169s | byte equality observed |
| is_destructible_test.test | 10 | 0.001214s | 0.001182s | 0.001168s | byte equality observed |
| is_detected_exact_test.test | 10 | 0.001172s | 0.001206s | 0.001205s | byte equality observed |
| is_detected_test.test | 10 | 0.001154s | 0.001192s | 0.001164s | byte equality observed |
| is_empty_test.test | 10 | 0.001179s | 0.001172s | 0.001194s | byte equality observed |
| is_enum_test.test | 10 | 0.001177s | 0.001191s | 0.001182s | byte equality observed |
| is_final_test.test | 10 | 0.001189s | 0.001187s | 0.001170s | byte equality observed |
| is_float_test.test | 10 | 0.001174s | 0.001207s | 0.001173s | byte equality observed |
| is_floating_point_test.test | 10 | 0.001169s | 0.001153s | 0.001180s | byte equality observed |
| is_function_test.test | 10 | 0.001181s | 0.001177s | 0.001176s | byte equality observed |
| is_fundamental_test.test | 10 | 0.001176s | 0.001160s | 0.001186s | byte equality observed |
| is_integral_test.test | 10 | 0.001181s | 0.001185s | 0.001183s | byte equality observed |
| is_list_constructible_test.test | 10 | 0.001169s | 0.001154s | 0.001171s | byte equality observed |
| is_lvalue_reference_test.test | 10 | 0.001190s | 0.001171s | 0.001183s | byte equality observed |
| is_member_func_test.test | 10 | 0.001177s | 0.001177s | 0.001180s | byte equality observed |
| is_member_obj_test.test | 10 | 0.001194s | 0.001206s | 0.001157s | byte equality observed |
| is_member_pointer_test.test | 10 | 0.001183s | 0.001178s | 0.001175s | byte equality observed |
| is_noncopyable_test.test | 10 | 0.001177s | 0.001177s | 0.001192s | byte equality observed |
| is_nothrow_move_assignable_test.test | 10 | 0.001178s | 0.001178s | 0.001172s | byte equality observed |
| is_nothrow_move_assignable_test_no_intrinsics.test | 10 | 0.001182s | 0.001180s | 0.001174s | byte equality observed |
| is_nothrow_move_constructible_test.test | 10 | 0.001190s | 0.001174s | 0.001196s | byte equality observed |
| is_nothrow_move_constructible_test_no_intrinsics.test | 10 | 0.001182s | 0.001153s | 0.001194s | byte equality observed |
| is_nothrow_swappable_test.test | 10 | 0.001184s | 0.001195s | 0.001161s | byte equality observed |
| is_object_test.test | 10 | 0.001186s | 0.001182s | 0.001185s | byte equality observed |
| is_pod_test.test | 10 | 0.001182s | 0.001181s | 0.001191s | byte equality observed |
| is_pointer_test.test | 10 | 0.001184s | 0.001178s | 0.001171s | byte equality observed |
| is_polymorphic_test.test | 10 | 0.001166s | 0.001176s | 0.001183s | byte equality observed |
| is_reference_test.test | 9 | 0.001185s | 0.001172s | 0.001187s | byte equality observed |
| is_rvalue_reference_test.test | 10 | 0.001198s | 0.001179s | 0.001181s | byte equality observed |
| is_same_test.test | 10 | 0.001157s | 0.001190s | 0.001187s | byte equality observed |
| is_scalar_test.test | 10 | 0.001181s | 0.001188s | 0.001189s | byte equality observed |
| is_scoped_enum_test.test | 10 | 0.001171s | 0.001179s | 0.001159s | byte equality observed |
| is_signed_test.test | 10 | 0.001181s | 0.001175s | 0.001155s | byte equality observed |
| is_stateless_test.test | 10 | 0.001189s | 0.001186s | 0.001176s | byte equality observed |
| is_swappable_test.test | 10 | 0.001193s | 0.001205s | 0.001202s | byte equality observed |
| is_trivially_copyable_test.test | 10 | 0.001208s | 0.001208s | 0.001219s | byte equality observed |
| is_unbounded_array_test.test | 10 | 0.001178s | 0.001184s | 0.001197s | byte equality observed |
| is_union_test.test | 10 | 0.001183s | 0.001171s | 0.001171s | byte equality observed |
| is_unscoped_enum_test.test | 10 | 0.001187s | 0.001176s | 0.001176s | byte equality observed |
| is_unsigned_test.test | 10 | 0.001196s | 0.001183s | 0.001190s | byte equality observed |
| is_virtual_base_of_test.test | 10 | 0.001192s | 0.001187s | 0.001196s | byte equality observed |
| is_void_test.test | 10 | 0.001180s | 0.001187s | 0.001178s | byte equality observed |
| is_volatile_test.test | 10 | 0.001182s | 0.001194s | 0.001157s | byte equality observed |
| make_signed_test.test | 10 | 0.001169s | 0.001176s | 0.001196s | byte equality observed |
| make_unsigned_test.test | 10 | 0.001179s | 0.001179s | 0.001196s | byte equality observed |
| make_void_test.test | 10 | 0.001184s | 0.001183s | 0.001186s | byte equality observed |
| mpl_interop_test1.test | 10 | 0.000532s | 0.000525s | 0.001162s | byte equality observed |
| mpl_interop_test2.test | 10 | 0.000521s | 0.000520s | 0.001177s | byte equality observed |
| mpl_interop_test3.test | 10 | 0.000517s | 0.000521s | 0.001187s | byte equality observed |
| negation_test.test | 10 | 0.001190s | 0.001210s | 0.001152s | byte equality observed |
| promote_basic_test.test | 10 | 0.000518s | 0.000522s | 0.001181s | byte equality observed |
| promote_enum_msvc_bug_test.test | 10 | 0.000520s | 0.000522s | 0.001177s | byte equality observed |
| promote_enum_test.test | 10 | 0.000522s | 0.000504s | 0.001166s | byte equality observed |
| promote_mpl_test.test | 10 | 0.000528s | 0.000534s | 0.001164s | byte equality observed |
| rank_test.test | 10 | 0.001190s | 0.001188s | 0.001183s | byte equality observed |
| remove_all_extents_test.test | 10 | 0.001181s | 0.001195s | 0.001154s | byte equality observed |
| remove_bounds_test.test | 10 | 0.001200s | 0.001172s | 0.001169s | byte equality observed |
| remove_const_test.test | 10 | 0.001165s | 0.001166s | 0.001169s | byte equality observed |
| remove_cv_ref_test.test | 10 | 0.001185s | 0.001185s | 0.001180s | byte equality observed |
| remove_cv_test.test | 10 | 0.001188s | 0.001167s | 0.001196s | byte equality observed |
| remove_extent_test.test | 10 | 0.001166s | 0.001190s | 0.001178s | byte equality observed |
| remove_pointer_test.test | 10 | 0.001168s | 0.001200s | 0.001164s | byte equality observed |
| remove_reference_test.test | 10 | 0.001174s | 0.001175s | 0.001173s | byte equality observed |
| remove_volatile_test.test | 10 | 0.001156s | 0.001198s | 0.001181s | byte equality observed |
| tricky_abstract_type_test.test | 10 | 0.001183s | 0.001175s | 0.001185s | byte equality observed |
| tricky_add_pointer_test.test | 10 | 0.001179s | 0.001177s | 0.001180s | byte equality observed |
| tricky_function_type_test.test | 10 | 0.001193s | 0.001182s | 0.001188s | byte equality observed |
| tricky_incomplete_type_test.test | 10 | 0.001172s | 0.001158s | 0.001142s | byte equality observed |
| tricky_is_enum_test.test | 10 | 0.001180s | 0.001170s | 0.001183s | byte equality observed |
| tricky_partial_spec_test.test | 10 | 0.001182s | 0.001186s | 0.001177s | byte equality observed |
| tricky_rvalue_test.test | 10 | 0.001176s | 0.001188s | 0.001166s | byte equality observed |
| type_identity_test.test | 10 | 0.001195s | 0.001170s | 0.001175s | byte equality observed |
| type_traits_test.test | 10 | 0.000532s | 0.000524s | 0.001195s | byte equality observed |
| type_with_alignment_test.test | 10 | 0.001283s | 0.001277s | 0.001287s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
