# libs/type_traits/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| type_with_alignment_test.test | RUN | 0.115389 | 0.328756 | 0.511548 | 0.001293 | 0.001666 | 0.001619 |
| type_traits_test.test | RUN | 0.159977 | 0.340213 | 0.499329 | 0.000570 | 0.000613 | 0.001648 |
| type_identity_test.test | RUN | 0.303665 | 1.177368 | 2.938000 | 0.001659 | 0.001949 | 0.001795 |
| tricky_rvalue_test.test | RUN | 0.108220 | 0.263960 | 0.399240 | 0.001622 | 0.001790 | 0.001716 |
| tricky_partial_spec_test.test | RUN | 0.132275 | 0.377411 | 0.625147 | 0.001553 | 0.001794 | 0.001536 |
| tricky_is_enum_test.test | RUN | 0.100746 | 0.252959 | 0.363694 | 0.001581 | 0.001561 | 0.001696 |
| tricky_incomplete_type_test.test | RUN | 0.174236 | 0.386134 | 0.629444 | 0.001491 | 0.001776 | 0.001793 |
| tricky_function_type_test.test | RUN | 0.141697 | 0.422975 | 0.713231 | 0.001693 | 0.001778 | 0.001529 |
| tricky_add_pointer_test.test | RUN | 0.207624 | 0.716385 | 1.637558 | 0.001692 | 0.001869 | 0.001775 |
| tricky_abstract_type_test.test | RUN | 0.113284 | 0.264340 | 0.386348 | 0.001803 | 0.001441 | 0.001725 |
| remove_volatile_test.test | RUN | 0.278005 | 1.035309 | 2.602942 | 0.001786 | 0.001816 | 0.001805 |
| remove_reference_test.test | RUN | 0.295795 | 1.063939 | 2.771915 | 0.001650 | 0.001868 | 0.001830 |
| remove_pointer_test.test | RUN | 0.294066 | 1.117028 | 2.872838 | 0.001802 | 0.001776 | 0.001868 |
| remove_extent_test.test | RUN | 0.294091 | 1.038229 | 2.727071 | 0.001674 | 0.001876 | 0.001786 |
| remove_cv_test.test | RUN | 0.293366 | 1.048574 | 2.764961 | 0.001711 | 0.001756 | 0.001790 |
| remove_cv_ref_test.test | RUN | 0.597603 | 2.496692 | 6.617206 | 0.001642 | 0.001536 | 0.001710 |
| remove_const_test.test | RUN | 0.279977 | 0.990892 | 2.574867 | 0.001882 | 0.001852 | 0.001812 |
| remove_bounds_test.test | RUN | 0.292180 | 1.082827 | 2.780098 | 0.001426 | 0.001834 | 0.001785 |
| remove_all_extents_test.test | RUN | 0.293164 | 1.058869 | 2.579717 | 0.001582 | 0.001746 | 0.001551 |
| rank_test.test | RUN | 0.111250 | 0.257834 | 0.409228 | 0.001528 | 0.001624 | 0.001759 |
| promote_mpl_test.test | RUN | 0.096212 | 0.233636 | 0.282685 | 0.000591 | 0.000699 | 0.001823 |
| promote_enum_test.test | RUN | 0.022308 | 0.060493 | 0.073115 | 0.000589 | 0.000681 | 0.001767 |
| promote_enum_msvc_bug_test.test | RUN | 0.020003 | 0.049262 | 0.060515 | 0.000591 | 0.000622 | 0.001846 |
| promote_basic_test.test | RUN | 0.024895 | 0.067321 | 0.069945 | 0.000662 | 0.000636 | 0.001775 |
| negation_test.test | RUN | 0.105997 | 0.240022 | 0.362455 | 0.001495 | 0.001550 | 0.001732 |
| mpl_interop_test3.test | RUN | 0.088695 | 0.214324 | 0.262527 | 0.000611 | 0.000604 | 0.001628 |
| mpl_interop_test2.test | RUN | 0.031834 | 0.067658 | 0.074638 | 0.000575 | 0.000598 | 0.001871 |
| mpl_interop_test1.test | RUN | 0.018082 | 0.046225 | 0.048585 | 0.000708 | 0.000765 | 0.001407 |
| make_void_test.test | RUN | 0.105661 | 0.259719 | 0.396271 | 0.001608 | 0.001658 | 0.001742 |
| make_unsigned_test.test | RUN | 0.129973 | 0.341260 | 0.536408 | 0.001536 | 0.001598 | 0.001635 |
| make_signed_test.test | RUN | 0.131261 | 0.345056 | 0.596500 | 0.001486 | 0.001504 | 0.001491 |
| is_volatile_test.test | RUN | 0.116878 | 0.285224 | 0.451851 | 0.001262 | 0.001767 | 0.002103 |
| is_void_test.test | RUN | 0.109510 | 0.272842 | 0.430907 | 0.001542 | 0.001832 | 0.001780 |
| is_virtual_base_of_test.test | RUN | 0.128744 | 0.353407 | 0.618436 | 0.001593 | 0.001341 | 0.001476 |
| is_unsigned_test.test | RUN | 0.113025 | 0.292659 | 0.465321 | 0.001443 | 0.001802 | 0.001593 |
| is_unscoped_enum_test.test | RUN | 0.110234 | 0.253424 | 0.335413 | 0.001582 | 0.001598 | 0.001558 |
| is_union_test.test | RUN | 0.121508 | 0.314756 | 0.519768 | 0.001490 | 0.001719 | 0.001728 |
| is_unbounded_array_test.test | RUN | 0.111142 | 0.275233 | 0.419305 | 0.001535 | 0.001750 | 0.001692 |
| is_trivially_copyable_test.test | RUN | 0.178900 | 0.573997 | 1.067795 | 0.001703 | 0.001935 | 0.001456 |
| is_swappable_test.test | RUN | 0.150957 | 0.427885 | 0.787261 | 0.001429 | 0.001751 | 0.001762 |
| is_stateless_test.test | RUN | 0.165535 | 0.497630 | 0.893648 | 0.001574 | 0.001580 | 0.001394 |
| is_signed_test.test | RUN | 0.116957 | 0.275622 | 0.444616 | 0.001667 | 0.001826 | 0.001508 |
| is_scoped_enum_test.test | RUN | 0.114680 | 0.255468 | 0.359580 | 0.001562 | 0.001677 | 0.001416 |
| is_scalar_test.test | RUN | 0.146493 | 0.461816 | 0.803362 | 0.001614 | 0.001744 | 0.001880 |
| is_same_test.test | RUN | 0.109137 | 0.279933 | 0.420717 | 0.001517 | 0.001640 | 0.001764 |
| is_rvalue_reference_test.test | RUN | 0.119303 | 0.308663 | 0.497475 | 0.001458 | 0.001632 | 0.001493 |
| is_reference_test.test | RUN | 0.119610 | 0.320875 | 0.503260 | 0.001564 | 0.001883 | 0.001120 |
| is_polymorphic_test.test | RUN | 0.118478 | 0.311379 | 0.491881 | 0.001575 | 0.001689 | 0.001809 |
| is_pointer_test.test | RUN | 0.116410 | 0.314243 | 0.524490 | 0.001494 | 0.001668 | 0.001403 |
| is_pod_test.test | RUN | 0.155585 | 0.507921 | 0.938062 | 0.001358 | 0.001715 | 0.001781 |
| is_object_test.test | RUN | 0.110561 | 0.255454 | 0.348362 | 0.001351 | 0.001700 | 0.001718 |
| is_nothrow_swappable_test.test | RUN | 0.156047 | 0.477045 | 0.883888 | 0.001744 | 0.001578 | 0.001798 |
| is_nothrow_move_constructible_test.test | RUN | 0.152329 | 0.493119 | 0.942822 | 0.001786 | 0.001530 | 0.001555 |
| is_nothrow_move_assignable_test.test | RUN | 0.152520 | 0.510263 | 0.887466 | 0.001540 | 0.001715 | 0.001524 |
| is_noncopyable_test.test | RUN | 0.110325 | 0.284987 | 0.372977 | 0.001466 | 0.001580 | 0.001788 |
| is_member_pointer_test.test | RUN | 0.108950 | 0.278079 | 0.391817 | 0.001493 | 0.001717 | 0.001817 |
| is_member_obj_test.test | RUN | 0.111977 | 0.279675 | 0.426046 | 0.001586 | 0.001556 | 0.001459 |
| is_member_func_test.test | RUN | 0.114770 | 0.299226 | 0.453638 | 0.001658 | 0.001646 | 0.001549 |
| is_lvalue_reference_test.test | RUN | 0.115207 | 0.316544 | 0.484420 | 0.001564 | 0.001703 | 0.001740 |
| is_list_constructible_test.test | RUN | 0.135168 | 0.364603 | 0.551835 | 0.001549 | 0.001506 | 0.001715 |
| is_integral_test.test | RUN | 0.127873 | 0.424847 | 0.753863 | 0.001716 | 0.001747 | 0.001793 |
| is_fundamental_test.test | RUN | 0.141437 | 0.450966 | 0.799722 | 0.001478 | 0.001596 | 0.001660 |
| is_function_test.test | RUN | 0.113388 | 0.297299 | 0.457549 | 0.001495 | 0.001822 | 0.001438 |
| is_floating_point_test.test | RUN | 0.114944 | 0.312026 | 0.510651 | 0.001308 | 0.001709 | 0.001403 |
| is_float_test.test | RUN | 0.112104 | 0.310327 | 0.503729 | 0.001624 | 0.001743 | 0.001533 |
| is_final_test.test | RUN | 0.115376 | 0.319247 | 0.501589 | 0.001531 | 0.001403 | 0.001677 |
| is_enum_test.test | RUN | 0.106813 | 0.271648 | 0.394721 | 0.001496 | 0.001727 | 0.001602 |
| is_empty_test.test | RUN | 0.117949 | 0.287085 | 0.454690 | 0.001420 | 0.001385 | 0.001821 |
| is_detected_test.test | RUN | 0.108183 | 0.244541 | 0.327782 | 0.001515 | 0.001780 | 0.001828 |
| is_detected_exact_test.test | RUN | 0.109276 | 0.246654 | 0.374400 | 0.001484 | 0.001597 | 0.001799 |
| is_destructible_test.test | RUN | 0.150909 | 0.474793 | 0.876816 | 0.001526 | 0.001922 | 0.001632 |
| is_default_constr_test.test | RUN | 0.153890 | 0.503576 | 0.912279 | 0.001864 | 0.001536 | 0.001838 |
| is_copy_constructible_test.test | RUN | 0.160468 | 0.532402 | 0.958917 | 0.001723 | 0.001980 | 0.001772 |
| is_copy_assignable_test.test | RUN | 0.158795 | 0.508628 | 0.956599 | 0.001444 | 0.001511 | 0.001844 |
| is_convertible_test.test | RUN | 0.151669 | 0.455232 | 0.792922 | 0.001576 | 0.001832 | 0.001589 |
| is_constructible_test.test | RUN | 0.125484 | 0.338574 | 0.510051 | 0.001824 | 0.001839 | 0.001668 |
| is_const_test.test | RUN | 0.118092 | 0.278480 | 0.433898 | 0.001492 | 0.001833 | 0.001736 |
| is_compound_test.test | RUN | 0.109492 | 0.258396 | 0.340671 | 0.001674 | 0.001606 | 0.001823 |
| is_complex_test.test | RUN | 0.150287 | 0.333533 | 0.499963 | 0.001580 | 0.001818 | 0.001810 |
| is_complete_test.test | RUN | 0.133418 | 0.348525 | 0.566531 | 0.001582 | 0.001493 | 0.001423 |
| is_class_test.test | RUN | 0.117013 | 0.294339 | 0.475866 | 0.001573 | 0.001632 | 0.001486 |
| is_bounded_array_test.test | RUN | 0.111036 | 0.280383 | 0.422563 | 0.001224 | 0.001479 | 0.001673 |
| is_base_of_test.test | RUN | 0.119688 | 0.298409 | 0.449991 | 0.001635 | 0.001876 | 0.001828 |
| is_base_and_derived_test.test | RUN | 0.117091 | 0.284419 | 0.477066 | 0.001556 | 0.001606 | 0.001643 |
| is_assignable_test.test | RUN | 0.138051 | 0.426876 | 0.762851 | 0.001544 | 0.001756 | 0.001808 |
| is_array_test.test | RUN | 0.115532 | 0.294199 | 0.431812 | 0.001604 | 0.001624 | 0.001671 |
| is_arithmetic_test.test | RUN | 0.142691 | 0.445030 | 0.759317 | 0.001642 | 0.001940 | 0.001741 |
| is_abstract_test.test | RUN | 0.253533 | 1.191813 | 2.549931 | 0.001522 | 0.001617 | 0.001893 |
| integral_constant_test.test | RUN | 0.016871 | 0.046708 | 0.056261 | 0.000785 | 0.000764 | 0.001589 |
| has_virtual_destructor_test.test | RUN | 0.123873 | 0.330521 | 0.549440 | 0.001768 | 0.001890 | 0.001800 |
| has_unary_plus_test.test | RUN | 0.235552 | 1.320900 | 1.963956 | 0.001583 | 0.001548 | 0.001812 |
| has_unary_minus_test.test | RUN | 0.235291 | 1.246349 | 1.922369 | 0.001414 | 0.001746 | 0.001790 |
| has_trivial_move_constructor_test.test | RUN | 0.160296 | 0.512058 | 0.972176 | 0.001684 | 0.001483 | 0.001854 |
| has_trivial_move_assign_test.test | RUN | 0.158968 | 0.512290 | 0.878415 | 0.001626 | 0.001736 | 0.001826 |
| has_trivial_destructor_test.test | RUN | 0.163294 | 0.513333 | 0.941878 | 0.001658 | 0.001526 | 0.001772 |
| has_trivial_copy_test.test | RUN | 0.164204 | 0.541307 | 0.969049 | 0.001371 | 0.001656 | 0.001590 |
| has_trivial_constr_test.test | RUN | 0.162881 | 0.537688 | 0.936150 | 0.001680 | 0.001819 | 0.001885 |
| has_trivial_assign_test.test | RUN | 0.160432 | 0.542314 | 1.008850 | 0.001717 | 0.001952 | 0.001864 |
| has_right_shift_test.test | RUN | 0.247193 | 0.979175 | 2.120825 | 0.001682 | 0.001591 | 0.001501 |
| has_right_shift_assign_test.test | RUN | 0.231058 | 0.939085 | 1.969455 | 0.001698 | 0.001619 | 0.001790 |
| has_prefix_classes3_test.test | RUN | 0.173722 | 0.518718 | 1.055025 | 0.001629 | 0.001628 | 0.001750 |
| has_prefix_classes2_test.test | RUN | 0.178527 | 0.553380 | 1.037289 | 0.001556 | 0.001646 | 0.001668 |
| has_prefix_classes1_test.test | RUN | 0.170972 | 0.553368 | 0.968320 | 0.001742 | 0.001877 | 0.001700 |
| has_prefix_classes0_test.test | RUN | 0.170468 | 0.543144 | 1.014368 | 0.001586 | 0.001886 | 0.001778 |
| has_pre_increment_test.test | RUN | 0.232833 | 1.287159 | 1.918682 | 0.001703 | 0.001811 | 0.001728 |
| has_pre_decrement_test.test | RUN | 0.233493 | 1.308069 | 1.962651 | 0.001528 | 0.001694 | 0.001432 |
| has_postfix_classes3_test.test | RUN | 0.174238 | 0.556244 | 1.035361 | 0.001716 | 0.001793 | 0.001807 |
| has_postfix_classes2_test.test | RUN | 0.172663 | 0.536483 | 0.988315 | 0.001513 | 0.001734 | 0.001726 |
| has_postfix_classes1_test.test | RUN | 0.177121 | 0.547994 | 1.010132 | 0.001743 | 0.001747 | 0.001744 |
| has_postfix_classes0_test.test | RUN | 0.174135 | 0.547242 | 1.000410 | 0.001568 | 0.001672 | 0.001833 |
| has_post_increment_test.test | RUN | 0.224483 | 1.318366 | 2.014407 | 0.001816 | 0.001623 | 0.001727 |
| has_post_decrement_test.test | RUN | 0.223846 | 1.269243 | 1.998756 | 0.001600 | 0.001568 | 0.001776 |
| has_plus_test.test | RUN | 0.229703 | 0.923492 | 1.979380 | 0.001744 | 0.001511 | 0.001710 |
| has_plus_assign_test.test | RUN | 0.240383 | 0.901811 | 1.998216 | 0.001577 | 0.001541 | 0.001816 |
| has_operator_new_test.test | RUN | 0.155985 | 0.484096 | 0.904806 | 0.001721 | 0.001760 | 0.001589 |
| has_nothrow_destructor_test.test | RUN | 0.160682 | 0.530203 | 0.988668 | 0.001498 | 0.001566 | 0.001866 |
| has_nothrow_copy_test.test | RUN | 0.157990 | 0.496041 | 0.915331 | 0.001803 | 0.001587 | 0.001867 |
| has_nothrow_constr_test.test | RUN | 0.155299 | 0.491995 | 0.915044 | 0.001635 | 0.001688 | 0.001865 |
| has_nothrow_assign_test.test | RUN | 0.153232 | 0.501700 | 0.895471 | 0.001676 | 0.001880 | 0.001614 |
| has_not_equal_to_test.test | RUN | 0.229959 | 1.220940 | 2.013133 | 0.001596 | 0.001831 | 0.001804 |
| has_negate_test.test | RUN | 0.223073 | 0.915029 | 1.924876 | 0.001699 | 0.001797 | 0.001814 |
| has_multiplies_test.test | RUN | 0.233566 | 0.937010 | 1.948296 | 0.001662 | 0.001860 | 0.001713 |
| has_multiplies_assign_test.test | RUN | 0.232297 | 0.909550 | 1.942949 | 0.001680 | 0.001936 | 0.001389 |
| has_modulus_test.test | RUN | 0.228962 | 0.961357 | 1.957051 | 0.001520 | 0.001760 | 0.001855 |
| has_modulus_assign_test.test | RUN | 0.230898 | 0.940887 | 1.957780 | 0.001828 | 0.001922 | 0.001790 |
| has_minus_test.test | RUN | 0.238940 | 0.947416 | 1.963216 | 0.001637 | 0.001790 | 0.001678 |
| has_minus_assign_test.test | RUN | 0.235911 | 0.921664 | 1.977547 | 0.001687 | 0.001966 | 0.001555 |
| has_logical_or_test.test | RUN | 0.229538 | 0.993229 | 1.952118 | 0.001655 | 0.001957 | 0.001888 |
| has_logical_not_test.test | RUN | 0.220182 | 0.948530 | 1.901253 | 0.001834 | 0.001792 | 0.001806 |
| has_logical_and_test.test | RUN | 0.228808 | 0.919513 | 1.948856 | 0.001649 | 0.001747 | 0.001639 |
| has_less_test.test | RUN | 0.225125 | 1.238030 | 1.960883 | 0.001714 | 0.001746 | 0.001598 |
| has_less_equal_test.test | RUN | 0.231748 | 1.190314 | 1.975661 | 0.001726 | 0.001470 | 0.001785 |
| has_left_shift_test.test | RUN | 0.248845 | 1.151274 | 2.437204 | 0.001797 | 0.001518 | 0.001351 |
| has_left_shift_assign_test.test | RUN | 0.226134 | 0.933849 | 1.987932 | 0.001631 | 0.001593 | 0.001577 |
| has_greater_test.test | RUN | 0.230991 | 1.250831 | 1.962859 | 0.001722 | 0.001785 | 0.001618 |
| has_greater_equal_test.test | RUN | 0.235769 | 1.218039 | 1.973498 | 0.001699 | 0.001748 | 0.001812 |
| has_equal_to_test.test | RUN | 0.230655 | 1.238573 | 2.039489 | 0.001690 | 0.001861 | 0.001685 |
| has_divides_test.test | RUN | 0.231605 | 0.895563 | 2.004932 | 0.001484 | 0.001670 | 0.001524 |
| has_divides_assign_test.test | RUN | 0.230653 | 0.974344 | 1.949643 | 0.001621 | 0.001694 | 0.001667 |
| has_dereference_test.test | RUN | 0.228835 | 0.948445 | 1.891597 | 0.001415 | 0.001782 | 0.001870 |
| has_complement_test.test | RUN | 0.225027 | 1.294956 | 2.003304 | 0.001758 | 0.001780 | 0.001614 |
| has_bit_xor_test.test | RUN | 0.227901 | 0.918558 | 1.959763 | 0.001492 | 0.001976 | 0.001542 |
| has_bit_xor_assign_test.test | RUN | 0.233544 | 0.883750 | 2.003298 | 0.001632 | 0.001800 | 0.001472 |
| has_bit_or_test.test | RUN | 0.229685 | 0.958296 | 1.951540 | 0.001673 | 0.001715 | 0.001740 |
| has_bit_or_assign_test.test | RUN | 0.226264 | 0.898786 | 1.991871 | 0.001775 | 0.001773 | 0.001625 |
| has_bit_and_test.test | RUN | 0.228387 | 0.916793 | 1.948730 | 0.001574 | 0.001878 | 0.001600 |
| has_bit_and_assign_test.test | RUN | 0.228895 | 0.927246 | 1.994582 | 0.001615 | 0.001830 | 0.001524 |
| has_binary_classes9_test.test | RUN | 0.247044 | 1.081492 | 2.157598 | 0.001656 | 0.001701 | 0.001842 |
| has_binary_classes8_test.test | RUN | 0.245727 | 1.113533 | 2.206263 | 0.001730 | 0.001798 | 0.001582 |
| has_binary_classes7_test.test | RUN | 0.252433 | 1.071355 | 2.165319 | 0.001586 | 0.001752 | 0.001676 |
| has_binary_classes6_test.test | RUN | 0.248291 | 1.002178 | 2.211283 | 0.001614 | 0.001730 | 0.001453 |
| has_binary_classes5_test.test | RUN | 0.247398 | 1.045144 | 2.159551 | 0.001608 | 0.001912 | 0.001776 |
| has_binary_classes4_test.test | RUN | 0.249551 | 1.081113 | 2.242621 | 0.001691 | 0.001692 | 0.001444 |
| has_binary_classes3_test.test | RUN | 0.252808 | 1.040609 | 2.167548 | 0.001574 | 0.001742 | 0.001668 |
| has_binary_classes2_test.test | RUN | 0.256933 | 1.084026 | 2.237981 | 0.001718 | 0.001777 | 0.001427 |
| has_binary_classes1_test.test | RUN | 0.250905 | 1.076975 | 2.140091 | 0.001708 | 0.001875 | 0.001690 |
| has_binary_classes0_test.test | RUN | 0.251235 | 1.115425 | 2.156372 | 0.001469 | 0.001715 | 0.001744 |
| function_traits_test.test | RUN | 0.111608 | 0.296603 | 0.487350 | 0.001576 | 0.001691 | 0.001544 |
| floating_point_promotion_test.test | RUN | 0.111944 | 0.284726 | 0.450242 | 0.001722 | 0.001532 | 0.001837 |
| extent_test.test | RUN | 0.110260 | 0.278615 | 0.476100 | 0.001696 | 0.001882 | 0.001842 |
| enable_if_test.test | RUN | 0.105136 | 0.231560 | 0.314075 | 0.001541 | 0.001490 | 0.001532 |
| disjunction_test.test | RUN | 0.106679 | 0.246864 | 0.379877 | 0.001756 | 0.001482 | 0.001541 |
| detected_test.test | RUN | 0.104623 | 0.238256 | 0.355893 | 0.001634 | 0.001719 | 0.001430 |
| detected_or_test.test | RUN | 0.104912 | 0.256421 | 0.391123 | 0.001566 | 0.001896 | 0.001664 |
| decay_test.test | RUN | 0.281550 | 0.985239 | 2.368971 | 0.001537 | 0.001312 | 0.001574 |
| cxx14_aliases_test.test | RUN | 0.184128 | 0.375593 | 0.624415 | 0.001208 | 0.001839 | 0.001909 |
| copy_reference_test.test | RUN | 0.111826 | 0.259270 | 0.335522 | 0.001648 | 0.001642 | 0.001674 |
| copy_cv_test.test | RUN | 0.111668 | 0.259973 | 0.391522 | 0.001428 | 0.001907 | 0.001723 |
| copy_cv_ref_test.test | RUN | 0.152356 | 0.456928 | 0.931493 | 0.001420 | 0.001512 | 0.001815 |
| conjunction_test.test | RUN | 0.105494 | 0.254751 | 0.392388 | 0.001482 | 0.001649 | 0.001781 |
| conditional_test.test | RUN | 0.102657 | 0.245908 | 0.378976 | 0.001445 | 0.001570 | 0.001854 |
| common_type_test.test | RUN | 0.140299 | 0.390690 | 0.721117 | 0.001486 | 0.001579 | 0.001410 |
| common_type_sfinae_test.test | RUN | 0.104416 | 0.249561 | 0.332430 | 0.001530 | 0.001596 | 0.001699 |
| common_type_sfinae2_test.test | RUN | 0.109238 | 0.256537 | 0.391279 | 0.001542 | 0.001514 | 0.001464 |
| common_type_6_test.test | RUN | 0.110066 | 0.265540 | 0.397760 | 0.001520 | 0.001573 | 0.001721 |
| common_type_5_test.test | RUN | 0.111440 | 0.260747 | 0.384952 | 0.001418 | 0.001683 | 0.001762 |
| common_type_4_test.test | RUN | 0.115471 | 0.279411 | 0.467104 | 0.001548 | 0.003936 | 0.001666 |
| common_type_3_test.test | RUN | 0.107021 | 0.236968 | 0.356273 | 0.001534 | 0.001533 | 0.001575 |
| common_type_2_test.test | RUN | 0.145102 | 0.386097 | 0.731965 | 0.001481 | 0.001820 | 0.001532 |
| alignment_of_test.test | RUN | 0.114795 | 0.371571 | 0.484641 | 0.001530 | 0.001960 | 0.001638 |
| alignment_of_a2_test.test | RUN | 0.113834 | 0.346908 | 0.465427 | 0.001516 | 0.001579 | 0.001428 |
| aligned_storage_test.test | RUN | 0.121243 | 0.327272 | 0.430147 | 0.001412 | 0.001606 | 0.001653 |
| aligned_storage_empy_test.test | RUN | 0.135431 | 0.353533 | 0.575830 | 0.001534 | 0.001889 | 0.001507 |
| aligned_storage_a2_test.test | RUN | 0.117865 | 0.299361 | 0.423679 | 0.001641 | 0.002099 | 0.001745 |
| add_volatile_test.test | RUN | 0.250584 | 0.883290 | 2.243029 | 0.001597 | 0.001582 | 0.001765 |
| add_rvalue_reference_test.test | RUN | 0.280917 | 0.992832 | 2.491611 | 0.001426 | 0.001616 | 0.001746 |
| add_reference_test.test | RUN | 0.270369 | 0.966588 | 2.502804 | 0.001592 | 0.001856 | 0.001757 |
| add_pointer_test.test | RUN | 0.179214 | 0.548981 | 1.276354 | 0.001482 | 0.001630 | 0.001816 |
| add_lvalue_reference_test.test | RUN | 0.270586 | 0.966203 | 2.473909 | 0.001510 | 0.001523 | 0.001654 |
| add_cv_test.test | RUN | 0.252320 | 0.867788 | 2.362138 | 0.001334 | 0.001414 | 0.001701 |
| add_const_test.test | RUN | 0.252002 | 0.872078 | 2.338048 | 0.001387 | 0.001642 | 0.001561 |
| is_virtual_base_of_fail.test | COMPILE_FAIL | 0.103787 | 0.237299 | 0.344702 | — | — | — |
| is_stateless_fail.test | COMPILE_FAIL | 0.102679 | 0.241986 | 0.359625 | — | — | — |
| is_polymorphic_fail.test | COMPILE_FAIL | 0.099952 | 0.226138 | 0.339086 | — | — | — |
| is_nothrow_move_constructible_fail.test | COMPILE_FAIL | 0.100852 | 0.229573 | 0.294032 | — | — | — |
| is_nothrow_move_assignable_fail.test | COMPILE_FAIL | 0.099700 | 0.215319 | 0.323398 | — | — | — |
| is_list_constructible_fail.test | COMPILE_FAIL | 0.099926 | 0.214240 | 0.318431 | — | — | — |
| is_empty_fail.test | COMPILE_FAIL | 0.101566 | 0.237865 | 0.292185 | — | — | — |
| is_destructible_fail.test | COMPILE_FAIL | 0.099870 | 0.226816 | 0.324009 | — | — | — |
| is_default_constructible_fail.test | COMPILE_FAIL | 0.099023 | 0.225199 | 0.319946 | — | — | — |
| is_copy_constructible_fail.test | COMPILE_FAIL | 0.100066 | 0.227978 | 0.326377 | — | — | — |
| is_copy_assignable_fail.test | COMPILE_FAIL | 0.098510 | 0.236223 | 0.325161 | — | — | — |
| is_convertible_fail.test | COMPILE_FAIL | 0.101584 | 0.232763 | 0.320565 | — | — | — |
| is_convertible2_fail.test | COMPILE_FAIL | 0.098544 | 0.227533 | 0.320468 | — | — | — |
| is_constructible_fail.test | COMPILE_FAIL | 0.098884 | 0.228246 | 0.331615 | — | — | — |
| is_base_of_fail.test | COMPILE_FAIL | 0.099431 | 0.243525 | 0.328489 | — | — | — |
| is_assignable_fail.test | COMPILE_FAIL | 0.097604 | 0.224278 | 0.322173 | — | — | — |
| has_nothrow_destruct_fail.test | COMPILE_FAIL | 0.096452 | 0.229707 | 0.325558 | — | — | — |
| has_nothrow_copy_fail.test | COMPILE_FAIL | 0.098554 | 0.232223 | 0.338434 | — | — | — |
| has_nothrow_cons_fail.test | COMPILE_FAIL | 0.103277 | 0.232482 | 0.330823 | — | — | — |
| has_nothrow_assign_fail.test | COMPILE_FAIL | 0.097537 | 0.229557 | 0.326152 | — | — | — |
| common_type_fail.test | COMPILE_FAIL | 0.097482 | 0.227055 | 0.320642 | — | — | — |
| common_type2_fail.test | COMPILE_FAIL | 0.102209 | 0.228390 | 0.326896 | — | — | — |
| has_nothrow_assign_test_no_intrinsics.test | RUN | 0.156966 | 0.518362 | 0.952115 | 0.001328 | 0.001699 | 0.001725 |
| has_nothrow_constr_test_no_intrinsics.test | RUN | 0.146966 | 0.494321 | 0.911824 | 0.001556 | 0.001461 | 0.001628 |
| has_nothrow_copy_test_no_intrinsics.test | RUN | 0.152172 | 0.489449 | 0.944779 | 0.001350 | 0.001188 | 0.001820 |
| is_nothrow_move_assignable_test_no_intrinsics.test | RUN | 0.150013 | 0.483938 | 0.928651 | 0.001096 | 0.001558 | 0.001342 |
| is_nothrow_move_constructible_test_no_intrinsics.test | RUN | 0.096605 | 0.347177 | 0.564808 | 0.001288 | 0.001348 | 0.001279 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.016×† | 1.016×, 1.017× | 0.931× |
| clang | 1.025× | 1.024×, 1.025× | 1.004× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737952).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| add_const_test.test | 10 | 0.001136s | 0.001155s | 0.001146s | byte equality observed |
| add_cv_test.test | 10 | 0.001131s | 0.001151s | 0.001153s | byte equality observed |
| add_lvalue_reference_test.test | 10 | 0.001156s | 0.001191s | 0.001148s | byte equality observed |
| add_pointer_test.test | 10 | 0.001145s | 0.001160s | 0.001165s | byte equality observed |
| add_reference_test.test | 10 | 0.001170s | 0.001167s | 0.001170s | byte equality observed |
| add_rvalue_reference_test.test | 10 | 0.001162s | 0.001181s | 0.001173s | byte equality observed |
| add_volatile_test.test | 10 | 0.001135s | 0.001151s | 0.001137s | byte equality observed |
| aligned_storage_a2_test.test | 10 | 0.001261s | 0.001244s | 0.001273s | byte equality observed |
| aligned_storage_empy_test.test | 10 | 0.001135s | 0.001148s | 0.001143s | byte equality observed |
| aligned_storage_test.test | 10 | 0.001273s | 0.001264s | 0.001298s | byte equality observed |
| alignment_of_a2_test.test | 10 | 0.001130s | 0.001144s | 0.001149s | byte equality observed |
| alignment_of_test.test | 10 | 0.001131s | 0.001169s | 0.001138s | byte equality observed |
| common_type_2_test.test | 10 | 0.001125s | 0.001164s | 0.001142s | byte equality observed |
| common_type_3_test.test | 10 | 0.001126s | 0.001159s | 0.001145s | byte equality observed |
| common_type_4_test.test | 10 | 0.001108s | 0.001157s | 0.001160s | byte equality observed |
| common_type_5_test.test | 9 | 0.001078s | 0.001158s | 0.001159s | byte equality observed |
| common_type_6_test.test | 10 | 0.001093s | 0.001172s | 0.001154s | byte equality observed |
| common_type_sfinae2_test.test | 10 | 0.001129s | 0.001158s | 0.001145s | byte equality observed |
| common_type_sfinae_test.test | 10 | 0.001097s | 0.001157s | 0.001171s | byte equality observed |
| common_type_test.test | 10 | 0.001099s | 0.001145s | 0.001126s | byte equality observed |
| conditional_test.test | 10 | 0.001105s | 0.001154s | 0.001151s | byte equality observed |
| conjunction_test.test | 10 | 0.001105s | 0.001168s | 0.001167s | byte equality observed |
| copy_cv_ref_test.test | 10 | 0.001094s | 0.001162s | 0.001177s | byte equality observed |
| copy_cv_test.test | 10 | 0.001103s | 0.001146s | 0.001146s | byte equality observed |
| copy_reference_test.test | 10 | 0.001126s | 0.001165s | 0.001137s | byte equality observed |
| cxx14_aliases_test.test | 10 | 0.001133s | 0.001183s | 0.001157s | byte equality observed |
| decay_test.test | 10 | 0.001153s | 0.001185s | 0.001154s | byte equality observed |
| detected_or_test.test | 10 | 0.001135s | 0.001148s | 0.001153s | byte equality observed |
| detected_test.test | 10 | 0.001129s | 0.001193s | 0.001165s | byte equality observed |
| disjunction_test.test | 10 | 0.001142s | 0.001160s | 0.001135s | byte equality observed |
| enable_if_test.test | 10 | 0.001139s | 0.001154s | 0.001165s | byte equality observed |
| extent_test.test | 10 | 0.001148s | 0.001148s | 0.001158s | byte equality observed |
| floating_point_promotion_test.test | 10 | 0.001149s | 0.001153s | 0.001147s | byte equality observed |
| function_traits_test.test | 10 | 0.001146s | 0.001156s | 0.001160s | byte equality observed |
| has_binary_classes0_test.test | 10 | 0.001157s | 0.001167s | 0.001141s | byte equality observed |
| has_binary_classes1_test.test | 10 | 0.001146s | 0.001142s | 0.001130s | byte equality observed |
| has_binary_classes2_test.test | 10 | 0.001150s | 0.001165s | 0.001150s | byte equality observed |
| has_binary_classes3_test.test | 10 | 0.001143s | 0.001151s | 0.001142s | byte equality observed |
| has_binary_classes4_test.test | 10 | 0.001144s | 0.001138s | 0.001159s | byte equality observed |
| has_binary_classes5_test.test | 10 | 0.001170s | 0.001150s | 0.001145s | byte equality observed |
| has_binary_classes6_test.test | 10 | 0.001153s | 0.001161s | 0.001184s | byte equality observed |
| has_binary_classes7_test.test | 10 | 0.001142s | 0.001146s | 0.001151s | byte equality observed |
| has_binary_classes8_test.test | 10 | 0.001133s | 0.001166s | 0.001169s | byte equality observed |
| has_binary_classes9_test.test | 10 | 0.001142s | 0.001129s | 0.001151s | byte equality observed |
| has_bit_and_assign_test.test | 10 | 0.001133s | 0.001140s | 0.001162s | byte equality observed |
| has_bit_and_test.test | 10 | 0.001150s | 0.001145s | 0.001137s | byte equality observed |
| has_bit_or_assign_test.test | 10 | 0.001133s | 0.001156s | 0.001161s | byte equality observed |
| has_bit_or_test.test | 10 | 0.001155s | 0.001160s | 0.001123s | byte equality observed |
| has_bit_xor_assign_test.test | 10 | 0.001146s | 0.001159s | 0.001157s | byte equality observed |
| has_bit_xor_test.test | 10 | 0.001150s | 0.001155s | 0.001139s | byte equality observed |
| has_complement_test.test | 10 | 0.001137s | 0.001161s | 0.001158s | byte equality observed |
| has_dereference_test.test | 10 | 0.001114s | 0.001149s | 0.001141s | byte equality observed |
| has_divides_assign_test.test | 10 | 0.001147s | 0.001141s | 0.001139s | byte equality observed |
| has_divides_test.test | 10 | 0.001142s | 0.001184s | 0.001137s | byte equality observed |
| has_equal_to_test.test | 10 | 0.001144s | 0.001192s | 0.001150s | byte equality observed |
| has_greater_equal_test.test | 10 | 0.001140s | 0.001158s | 0.001151s | byte equality observed |
| has_greater_test.test | 10 | 0.001146s | 0.001190s | 0.001155s | byte equality observed |
| has_left_shift_assign_test.test | 10 | 0.001119s | 0.001149s | 0.001136s | byte equality observed |
| has_left_shift_test.test | 10 | 0.001151s | 0.001175s | 0.001149s | byte equality observed |
| has_less_equal_test.test | 10 | 0.001130s | 0.001174s | 0.001152s | byte equality observed |
| has_less_test.test | 10 | 0.001142s | 0.001170s | 0.001151s | byte equality observed |
| has_logical_and_test.test | 10 | 0.001131s | 0.001165s | 0.001133s | byte equality observed |
| has_logical_not_test.test | 10 | 0.001148s | 0.001138s | 0.001133s | byte equality observed |
| has_logical_or_test.test | 10 | 0.001149s | 0.001127s | 0.001154s | byte equality observed |
| has_minus_assign_test.test | 10 | 0.001126s | 0.001158s | 0.001154s | byte equality observed |
| has_minus_test.test | 10 | 0.001124s | 0.001155s | 0.001155s | byte equality observed |
| has_modulus_assign_test.test | 10 | 0.001152s | 0.001163s | 0.001154s | byte equality observed |
| has_modulus_test.test | 10 | 0.001163s | 0.001141s | 0.001144s | byte equality observed |
| has_multiplies_assign_test.test | 10 | 0.001141s | 0.001152s | 0.001137s | byte equality observed |
| has_multiplies_test.test | 10 | 0.001131s | 0.001179s | 0.001138s | byte equality observed |
| has_negate_test.test | 10 | 0.001147s | 0.001175s | 0.001139s | byte equality observed |
| has_not_equal_to_test.test | 10 | 0.001135s | 0.001141s | 0.001141s | byte equality observed |
| has_nothrow_assign_test.test | 10 | 0.001131s | 0.001163s | 0.001147s | byte equality observed |
| has_nothrow_assign_test_no_intrinsics.test | 10 | 0.001137s | 0.001131s | 0.001134s | byte equality observed |
| has_nothrow_constr_test.test | 10 | 0.001141s | 0.001149s | 0.001141s | byte equality observed |
| has_nothrow_constr_test_no_intrinsics.test | 10 | 0.001148s | 0.001136s | 0.001130s | byte equality observed |
| has_nothrow_copy_test.test | 10 | 0.001154s | 0.001153s | 0.001143s | byte equality observed |
| has_nothrow_copy_test_no_intrinsics.test | 10 | 0.001157s | 0.001121s | 0.001138s | byte equality observed |
| has_nothrow_destructor_test.test | 10 | 0.001144s | 0.001117s | 0.001161s | byte equality observed |
| has_operator_new_test.test | 10 | 0.001144s | 0.001157s | 0.001152s | byte equality observed |
| has_plus_assign_test.test | 10 | 0.001157s | 0.001154s | 0.001142s | byte equality observed |
| has_plus_test.test | 10 | 0.001141s | 0.001162s | 0.001144s | byte equality observed |
| has_post_decrement_test.test | 10 | 0.001154s | 0.001150s | 0.001122s | byte equality observed |
| has_post_increment_test.test | 10 | 0.001138s | 0.001157s | 0.001155s | byte equality observed |
| has_postfix_classes0_test.test | 10 | 0.001147s | 0.001149s | 0.001162s | byte equality observed |
| has_postfix_classes1_test.test | 10 | 0.001151s | 0.001161s | 0.001155s | byte equality observed |
| has_postfix_classes2_test.test | 10 | 0.001140s | 0.001163s | 0.001152s | byte equality observed |
| has_postfix_classes3_test.test | 10 | 0.001131s | 0.001153s | 0.001151s | byte equality observed |
| has_pre_decrement_test.test | 10 | 0.001142s | 0.001153s | 0.001165s | byte equality observed |
| has_pre_increment_test.test | 10 | 0.001146s | 0.001156s | 0.001119s | byte equality observed |
| has_prefix_classes0_test.test | 10 | 0.001157s | 0.001157s | 0.001156s | byte equality observed |
| has_prefix_classes1_test.test | 10 | 0.001148s | 0.001143s | 0.001134s | byte equality observed |
| has_prefix_classes2_test.test | 10 | 0.001166s | 0.001151s | 0.001144s | byte equality observed |
| has_prefix_classes3_test.test | 10 | 0.001151s | 0.001148s | 0.001140s | byte equality observed |
| has_right_shift_assign_test.test | 10 | 0.001154s | 0.001128s | 0.001141s | byte equality observed |
| has_right_shift_test.test | 10 | 0.001136s | 0.001123s | 0.001120s | byte equality observed |
| has_trivial_assign_test.test | 10 | 0.001217s | 0.001187s | 0.001210s | byte equality observed |
| has_trivial_constr_test.test | 10 | 0.001187s | 0.001217s | 0.001197s | native test oracle; output varies |
| has_trivial_copy_test.test | 10 | 0.001206s | 0.001219s | 0.001249s | native test oracle; output varies |
| has_trivial_destructor_test.test | 10 | 0.001119s | 0.001064s | 0.001079s | byte equality observed |
| has_trivial_move_assign_test.test | 10 | 0.001136s | 0.001059s | 0.001076s | byte equality observed |
| has_trivial_move_constructor_test.test | 10 | 0.001145s | 0.001093s | 0.001098s | byte equality observed |
| has_unary_minus_test.test | 10 | 0.001133s | 0.001123s | 0.001150s | byte equality observed |
| has_unary_plus_test.test | 10 | 0.001152s | 0.001186s | 0.001104s | byte equality observed |
| has_virtual_destructor_test.test | 10 | 0.001144s | 0.001151s | 0.001101s | byte equality observed |
| integral_constant_test.test | 10 | 0.000516s | 0.000515s | 0.001132s | byte equality observed |
| is_abstract_test.test | 10 | 0.001139s | 0.001161s | 0.001152s | byte equality observed |
| is_arithmetic_test.test | 10 | 0.001160s | 0.001137s | 0.001133s | byte equality observed |
| is_array_test.test | 10 | 0.001150s | 0.001135s | 0.001116s | byte equality observed |
| is_assignable_test.test | 10 | 0.001145s | 0.001140s | 0.001143s | byte equality observed |
| is_base_and_derived_test.test | 10 | 0.001153s | 0.001148s | 0.001149s | byte equality observed |
| is_base_of_test.test | 10 | 0.001145s | 0.001148s | 0.001125s | byte equality observed |
| is_bounded_array_test.test | 10 | 0.001150s | 0.001145s | 0.001109s | byte equality observed |
| is_class_test.test | 10 | 0.001139s | 0.001145s | 0.001130s | byte equality observed |
| is_complete_test.test | 10 | 0.001142s | 0.001136s | 0.001143s | byte equality observed |
| is_complex_test.test | 10 | 0.001127s | 0.001156s | 0.001139s | byte equality observed |
| is_compound_test.test | 10 | 0.001147s | 0.001153s | 0.001115s | byte equality observed |
| is_const_test.test | 7 | 0.001145s | 0.001170s | 0.001166s | byte equality observed |
| is_constructible_test.test | 10 | 0.001135s | 0.001143s | 0.001116s | byte equality observed |
| is_convertible_test.test | 10 | 0.001149s | 0.001149s | 0.001141s | byte equality observed |
| is_copy_assignable_test.test | 10 | 0.001142s | 0.001139s | 0.001148s | byte equality observed |
| is_copy_constructible_test.test | 10 | 0.001131s | 0.001114s | 0.001169s | byte equality observed |
| is_default_constr_test.test | 10 | 0.001152s | 0.001148s | 0.001139s | byte equality observed |
| is_destructible_test.test | 10 | 0.001151s | 0.001137s | 0.001171s | byte equality observed |
| is_detected_exact_test.test | 10 | 0.001152s | 0.001159s | 0.001159s | byte equality observed |
| is_detected_test.test | 10 | 0.001139s | 0.001150s | 0.001159s | byte equality observed |
| is_empty_test.test | 10 | 0.001148s | 0.001136s | 0.001122s | byte equality observed |
| is_enum_test.test | 10 | 0.001128s | 0.001138s | 0.001160s | byte equality observed |
| is_final_test.test | 10 | 0.001152s | 0.001133s | 0.001126s | byte equality observed |
| is_float_test.test | 10 | 0.001155s | 0.001144s | 0.001152s | byte equality observed |
| is_floating_point_test.test | 10 | 0.001133s | 0.001131s | 0.001141s | byte equality observed |
| is_function_test.test | 10 | 0.001144s | 0.001147s | 0.001153s | byte equality observed |
| is_fundamental_test.test | 10 | 0.001174s | 0.001129s | 0.001135s | byte equality observed |
| is_integral_test.test | 10 | 0.001158s | 0.001158s | 0.001141s | byte equality observed |
| is_list_constructible_test.test | 10 | 0.001152s | 0.001149s | 0.001154s | byte equality observed |
| is_lvalue_reference_test.test | 10 | 0.001140s | 0.001153s | 0.001169s | byte equality observed |
| is_member_func_test.test | 10 | 0.001164s | 0.001144s | 0.001147s | byte equality observed |
| is_member_obj_test.test | 10 | 0.001142s | 0.001133s | 0.001157s | byte equality observed |
| is_member_pointer_test.test | 10 | 0.001145s | 0.001155s | 0.001152s | byte equality observed |
| is_noncopyable_test.test | 10 | 0.001143s | 0.001151s | 0.001158s | byte equality observed |
| is_nothrow_move_assignable_test.test | 10 | 0.001158s | 0.001164s | 0.001160s | byte equality observed |
| is_nothrow_move_assignable_test_no_intrinsics.test | 10 | 0.001151s | 0.001154s | 0.001147s | byte equality observed |
| is_nothrow_move_constructible_test.test | 10 | 0.001137s | 0.001151s | 0.001167s | byte equality observed |
| is_nothrow_move_constructible_test_no_intrinsics.test | 10 | 0.001150s | 0.001176s | 0.001169s | byte equality observed |
| is_nothrow_swappable_test.test | 10 | 0.001154s | 0.001128s | 0.001179s | byte equality observed |
| is_object_test.test | 10 | 0.001154s | 0.001149s | 0.001168s | byte equality observed |
| is_pod_test.test | 10 | 0.001147s | 0.001162s | 0.001160s | byte equality observed |
| is_pointer_test.test | 10 | 0.001168s | 0.001144s | 0.001163s | byte equality observed |
| is_polymorphic_test.test | 10 | 0.001160s | 0.001161s | 0.001143s | byte equality observed |
| is_reference_test.test | 10 | 0.001141s | 0.001160s | 0.001155s | byte equality observed |
| is_rvalue_reference_test.test | 10 | 0.001132s | 0.001154s | 0.001161s | byte equality observed |
| is_same_test.test | 10 | 0.001139s | 0.001158s | 0.001152s | byte equality observed |
| is_scalar_test.test | 10 | 0.001147s | 0.001149s | 0.001135s | byte equality observed |
| is_scoped_enum_test.test | 10 | 0.001145s | 0.001143s | 0.001161s | byte equality observed |
| is_signed_test.test | 10 | 0.001141s | 0.001141s | 0.001161s | byte equality observed |
| is_stateless_test.test | 10 | 0.001158s | 0.001126s | 0.001162s | byte equality observed |
| is_swappable_test.test | 10 | 0.001154s | 0.001132s | 0.001177s | byte equality observed |
| is_trivially_copyable_test.test | 10 | 0.001191s | 0.001185s | 0.001216s | byte equality observed |
| is_unbounded_array_test.test | 10 | 0.001145s | 0.001119s | 0.001172s | byte equality observed |
| is_union_test.test | 10 | 0.001156s | 0.001142s | 0.001168s | byte equality observed |
| is_unscoped_enum_test.test | 10 | 0.001159s | 0.001107s | 0.001140s | byte equality observed |
| is_unsigned_test.test | 10 | 0.001158s | 0.001131s | 0.001159s | byte equality observed |
| is_virtual_base_of_test.test | 10 | 0.001162s | 0.001138s | 0.001171s | byte equality observed |
| is_void_test.test | 10 | 0.001169s | 0.001115s | 0.001157s | byte equality observed |
| is_volatile_test.test | 10 | 0.001146s | 0.001140s | 0.001175s | byte equality observed |
| make_signed_test.test | 10 | 0.001141s | 0.001130s | 0.001158s | byte equality observed |
| make_unsigned_test.test | 10 | 0.001172s | 0.001155s | 0.001161s | byte equality observed |
| make_void_test.test | 10 | 0.001174s | 0.001173s | 0.001146s | byte equality observed |
| mpl_interop_test1.test | 10 | 0.000517s | 0.000511s | 0.001154s | byte equality observed |
| mpl_interop_test2.test | 10 | 0.000517s | 0.000518s | 0.001143s | byte equality observed |
| mpl_interop_test3.test | 10 | 0.000523s | 0.000518s | 0.001174s | byte equality observed |
| negation_test.test | 10 | 0.001212s | 0.001207s | 0.001141s | byte equality observed |
| promote_basic_test.test | 10 | 0.000525s | 0.000518s | 0.001152s | byte equality observed |
| promote_enum_msvc_bug_test.test | 10 | 0.000513s | 0.000522s | 0.001168s | byte equality observed |
| promote_enum_test.test | 10 | 0.000517s | 0.000527s | 0.001153s | byte equality observed |
| promote_mpl_test.test | 10 | 0.000522s | 0.000513s | 0.001150s | byte equality observed |
| rank_test.test | 10 | 0.001169s | 0.001166s | 0.001149s | byte equality observed |
| remove_all_extents_test.test | 10 | 0.001179s | 0.001149s | 0.001168s | byte equality observed |
| remove_bounds_test.test | 10 | 0.001163s | 0.001173s | 0.001140s | byte equality observed |
| remove_const_test.test | 10 | 0.001156s | 0.001149s | 0.001159s | byte equality observed |
| remove_cv_ref_test.test | 10 | 0.001161s | 0.001156s | 0.001134s | byte equality observed |
| remove_cv_test.test | 10 | 0.001156s | 0.001151s | 0.001163s | byte equality observed |
| remove_extent_test.test | 10 | 0.001159s | 0.001160s | 0.001150s | byte equality observed |
| remove_pointer_test.test | 10 | 0.001160s | 0.001143s | 0.001162s | byte equality observed |
| remove_reference_test.test | 10 | 0.001172s | 0.001163s | 0.001164s | byte equality observed |
| remove_volatile_test.test | 10 | 0.001166s | 0.001155s | 0.001149s | byte equality observed |
| tricky_abstract_type_test.test | 10 | 0.001157s | 0.001134s | 0.001152s | byte equality observed |
| tricky_add_pointer_test.test | 10 | 0.001149s | 0.001143s | 0.001147s | byte equality observed |
| tricky_function_type_test.test | 10 | 0.001156s | 0.001168s | 0.001157s | byte equality observed |
| tricky_incomplete_type_test.test | 10 | 0.001156s | 0.001143s | 0.001136s | byte equality observed |
| tricky_is_enum_test.test | 10 | 0.001156s | 0.001138s | 0.001141s | byte equality observed |
| tricky_partial_spec_test.test | 10 | 0.001159s | 0.001148s | 0.001162s | byte equality observed |
| tricky_rvalue_test.test | 10 | 0.001147s | 0.001153s | 0.001153s | byte equality observed |
| type_identity_test.test | 10 | 0.001182s | 0.001145s | 0.001156s | byte equality observed |
| type_traits_test.test | 10 | 0.000520s | 0.000512s | 0.001128s | byte equality observed |
| type_with_alignment_test.test | 10 | 0.001297s | 0.001294s | 0.001302s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737952).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
