# libs/type_traits/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| type_with_alignment_test.test | RUN | 0.130162 | 0.306679 | 0.507234 | 0.001688 | 0.001816 | 0.001630 |
| type_traits_test.test | RUN | 0.184288 | 0.334144 | 0.496884 | 0.000605 | 0.000703 | 0.001662 |
| type_identity_test.test | RUN | 0.318572 | 1.166384 | 2.942556 | 0.001736 | 0.001871 | 0.001771 |
| tricky_rvalue_test.test | RUN | 0.119826 | 0.249096 | 0.395481 | 0.001614 | 0.001646 | 0.001464 |
| tricky_partial_spec_test.test | RUN | 0.151817 | 0.371907 | 0.663949 | 0.001738 | 0.001932 | 0.001665 |
| tricky_is_enum_test.test | RUN | 0.118650 | 0.241703 | 0.363626 | 0.001413 | 0.001772 | 0.001848 |
| tricky_incomplete_type_test.test | RUN | 0.192998 | 0.372510 | 0.610214 | 0.001743 | 0.001698 | 0.001452 |
| tricky_function_type_test.test | RUN | 0.164287 | 0.402802 | 0.731551 | 0.001555 | 0.001634 | 0.001760 |
| tricky_add_pointer_test.test | RUN | 0.221788 | 0.696141 | 1.628652 | 0.001510 | 0.001893 | 0.001863 |
| tricky_abstract_type_test.test | RUN | 0.124885 | 0.243533 | 0.374728 | 0.001790 | 0.002003 | 0.001903 |
| remove_volatile_test.test | RUN | 0.292546 | 0.989755 | 2.666740 | 0.001791 | 0.001784 | 0.001734 |
| remove_reference_test.test | RUN | 0.304580 | 1.040250 | 2.730412 | 0.001654 | 0.001873 | 0.001862 |
| remove_pointer_test.test | RUN | 0.321897 | 1.101861 | 2.887582 | 0.001802 | 0.001424 | 0.001601 |
| remove_extent_test.test | RUN | 0.306366 | 1.061703 | 2.755202 | 0.001338 | 0.001937 | 0.001634 |
| remove_cv_test.test | RUN | 0.302279 | 1.018242 | 2.769275 | 0.001784 | 0.001846 | 0.001799 |
| remove_cv_ref_test.test | RUN | 0.614855 | 2.476642 | 6.809519 | 0.001756 | 0.001628 | 0.001624 |
| remove_const_test.test | RUN | 0.297967 | 0.983842 | 2.609365 | 0.001710 | 0.001882 | 0.001774 |
| remove_bounds_test.test | RUN | 0.303617 | 1.045250 | 2.752373 | 0.001747 | 0.001732 | 0.001821 |
| remove_all_extents_test.test | RUN | 0.304054 | 1.077954 | 2.747278 | 0.001690 | 0.001658 | 0.001746 |
| rank_test.test | RUN | 0.122624 | 0.263520 | 0.357794 | 0.001579 | 0.001768 | 0.001436 |
| promote_mpl_test.test | RUN | 0.113486 | 0.233709 | 0.259119 | 0.000599 | 0.000750 | 0.001729 |
| promote_enum_test.test | RUN | 0.024750 | 0.059615 | 0.063648 | 0.000596 | 0.000759 | 0.001433 |
| promote_enum_msvc_bug_test.test | RUN | 0.023432 | 0.047723 | 0.047652 | 0.000642 | 0.000727 | 0.001528 |
| promote_basic_test.test | RUN | 0.027344 | 0.063709 | 0.066004 | 0.000717 | 0.000648 | 0.001808 |
| negation_test.test | RUN | 0.114969 | 0.232658 | 0.318849 | 0.001504 | 0.001578 | 0.001581 |
| mpl_interop_test3.test | RUN | 0.106805 | 0.222533 | 0.239205 | 0.000616 | 0.000644 | 0.001612 |
| mpl_interop_test2.test | RUN | 0.043350 | 0.061104 | 0.065867 | 0.000743 | 0.000682 | 0.001379 |
| mpl_interop_test1.test | RUN | 0.019611 | 0.046671 | 0.047426 | 0.000734 | 0.000746 | 0.001522 |
| make_void_test.test | RUN | 0.121225 | 0.260315 | 0.403728 | 0.001700 | 0.001581 | 0.001822 |
| make_unsigned_test.test | RUN | 0.142468 | 0.336496 | 0.541599 | 0.001645 | 0.001694 | 0.001708 |
| make_signed_test.test | RUN | 0.143888 | 0.341651 | 0.564976 | 0.001602 | 0.001514 | 0.001655 |
| is_volatile_test.test | RUN | 0.129670 | 0.293465 | 0.437521 | 0.001546 | 0.001648 | 0.001658 |
| is_void_test.test | RUN | 0.123960 | 0.265945 | 0.405887 | 0.001975 | 0.001650 | 0.001768 |
| is_virtual_base_of_test.test | RUN | 0.143046 | 0.344171 | 0.573422 | 0.001645 | 0.001577 | 0.001789 |
| is_unsigned_test.test | RUN | 0.125872 | 0.282974 | 0.454136 | 0.001642 | 0.001340 | 0.001765 |
| is_unscoped_enum_test.test | RUN | 0.125706 | 0.248886 | 0.321230 | 0.001556 | 0.001623 | 0.001816 |
| is_union_test.test | RUN | 0.134972 | 0.318912 | 0.535740 | 0.001676 | 0.001585 | 0.001637 |
| is_unbounded_array_test.test | RUN | 0.122712 | 0.270800 | 0.384670 | 0.001640 | 0.001662 | 0.001600 |
| is_trivially_copyable_test.test | RUN | 0.197681 | 0.555270 | 1.064805 | 0.001646 | 0.001786 | 0.001812 |
| is_swappable_test.test | RUN | 0.164889 | 0.423320 | 0.790190 | 0.001260 | 0.001806 | 0.001405 |
| is_stateless_test.test | RUN | 0.190082 | 0.489586 | 0.910149 | 0.001565 | 0.001684 | 0.001792 |
| is_signed_test.test | RUN | 0.130491 | 0.290454 | 0.458785 | 0.001667 | 0.001274 | 0.001558 |
| is_scoped_enum_test.test | RUN | 0.122549 | 0.252851 | 0.360297 | 0.001673 | 0.001580 | 0.001773 |
| is_scalar_test.test | RUN | 0.163289 | 0.449164 | 0.772843 | 0.001257 | 0.001909 | 0.001650 |
| is_same_test.test | RUN | 0.123097 | 0.261966 | 0.409923 | 0.001648 | 0.001652 | 0.001792 |
| is_rvalue_reference_test.test | RUN | 0.129021 | 0.296244 | 0.463989 | 0.001620 | 0.001550 | 0.001686 |
| is_reference_test.test | RUN | 0.132244 | 0.296705 | 0.445882 | 0.001608 | 0.001531 | 0.001800 |
| is_polymorphic_test.test | RUN | 0.134713 | 0.322178 | 0.482063 | 0.001458 | 0.001804 | 0.001849 |
| is_pointer_test.test | RUN | 0.132991 | 0.313490 | 0.512751 | 0.001884 | 0.001555 | 0.001928 |
| is_pod_test.test | RUN | 0.176391 | 0.497699 | 0.914094 | 0.001403 | 0.001848 | 0.001762 |
| is_object_test.test | RUN | 0.123066 | 0.262814 | 0.356553 | 0.001662 | 0.001807 | 0.001775 |
| is_nothrow_swappable_test.test | RUN | 0.176763 | 0.457602 | 0.856340 | 0.001595 | 0.001708 | 0.001757 |
| is_nothrow_move_constructible_test.test | RUN | 0.172447 | 0.476271 | 0.879195 | 0.001840 | 0.001614 | 0.001544 |
| is_nothrow_move_assignable_test.test | RUN | 0.174019 | 0.490594 | 0.922047 | 0.001625 | 0.001493 | 0.001757 |
| is_noncopyable_test.test | RUN | 0.121589 | 0.266013 | 0.418394 | 0.001564 | 0.001706 | 0.001557 |
| is_member_pointer_test.test | RUN | 0.128833 | 0.263797 | 0.392304 | 0.001513 | 0.001854 | 0.001776 |
| is_member_obj_test.test | RUN | 0.127488 | 0.272573 | 0.413462 | 0.001820 | 0.001587 | 0.001414 |
| is_member_func_test.test | RUN | 0.129047 | 0.278856 | 0.426856 | 0.001460 | 0.001751 | 0.001486 |
| is_lvalue_reference_test.test | RUN | 0.132928 | 0.299366 | 0.496836 | 0.001798 | 0.001544 | 0.002027 |
| is_list_constructible_test.test | RUN | 0.155600 | 0.345743 | 0.537137 | 0.001756 | 0.001905 | 0.001842 |
| is_integral_test.test | RUN | 0.151694 | 0.425541 | 0.736040 | 0.001456 | 0.001407 | 0.001742 |
| is_fundamental_test.test | RUN | 0.158277 | 0.435839 | 0.802689 | 0.001412 | 0.001769 | 0.001710 |
| is_function_test.test | RUN | 0.126002 | 0.289225 | 0.453352 | 0.001581 | 0.001624 | 0.001711 |
| is_floating_point_test.test | RUN | 0.131823 | 0.304232 | 0.487369 | 0.001740 | 0.001832 | 0.001800 |
| is_float_test.test | RUN | 0.131546 | 0.308434 | 0.497999 | 0.001284 | 0.001395 | 0.001810 |
| is_final_test.test | RUN | 0.130605 | 0.306769 | 0.502809 | 0.001531 | 0.001643 | 0.001378 |
| is_enum_test.test | RUN | 0.123160 | 0.263294 | 0.391979 | 0.001555 | 0.001735 | 0.001480 |
| is_empty_test.test | RUN | 0.135013 | 0.282657 | 0.448515 | 0.001383 | 0.001616 | 0.001726 |
| is_detected_test.test | RUN | 0.118016 | 0.244220 | 0.361031 | 0.001513 | 0.001435 | 0.001752 |
| is_detected_exact_test.test | RUN | 0.118454 | 0.244217 | 0.351003 | 0.001724 | 0.001766 | 0.001635 |
| is_destructible_test.test | RUN | 0.171770 | 0.473291 | 0.880310 | 0.001718 | 0.001860 | 0.001763 |
| is_default_constr_test.test | RUN | 0.174598 | 0.504114 | 0.906282 | 0.001776 | 0.001676 | 0.001869 |
| is_copy_constructible_test.test | RUN | 0.181055 | 0.517973 | 0.968295 | 0.001800 | 0.001620 | 0.001778 |
| is_copy_assignable_test.test | RUN | 0.179793 | 0.500591 | 0.949109 | 0.001548 | 0.001817 | 0.001539 |
| is_convertible_test.test | RUN | 0.172982 | 0.440740 | 0.772561 | 0.001712 | 0.001881 | 0.001644 |
| is_constructible_test.test | RUN | 0.143637 | 0.336813 | 0.529629 | 0.001522 | 0.001896 | 0.001823 |
| is_const_test.test | RUN | 0.127395 | 0.277455 | 0.432878 | 0.001464 | 0.001529 | 0.001618 |
| is_compound_test.test | RUN | 0.118639 | 0.258743 | 0.341662 | 0.001588 | 0.001678 | 0.001638 |
| is_complex_test.test | RUN | 0.162238 | 0.332435 | 0.493450 | 0.001537 | 0.001848 | 0.001750 |
| is_complete_test.test | RUN | 0.142372 | 0.337002 | 0.578543 | 0.001596 | 0.001521 | 0.001371 |
| is_class_test.test | RUN | 0.130737 | 0.299095 | 0.488379 | 0.001602 | 0.001571 | 0.001747 |
| is_bounded_array_test.test | RUN | 0.124740 | 0.268702 | 0.390137 | 0.001672 | 0.001640 | 0.001346 |
| is_base_of_test.test | RUN | 0.131526 | 0.293657 | 0.465405 | 0.001432 | 0.001891 | 0.001792 |
| is_base_and_derived_test.test | RUN | 0.131729 | 0.293034 | 0.467330 | 0.001528 | 0.001692 | 0.001563 |
| is_assignable_test.test | RUN | 0.157620 | 0.404085 | 0.737497 | 0.001653 | 0.001859 | 0.001802 |
| is_array_test.test | RUN | 0.127213 | 0.287482 | 0.451748 | 0.001668 | 0.001632 | 0.001530 |
| is_arithmetic_test.test | RUN | 0.155062 | 0.420270 | 0.769058 | 0.001570 | 0.001906 | 0.001892 |
| is_abstract_test.test | RUN | 0.285999 | 1.180942 | 2.501035 | 0.001694 | 0.001839 | 0.001780 |
| integral_constant_test.test | RUN | 0.019934 | 0.044789 | 0.053589 | 0.000772 | 0.000786 | 0.001427 |
| has_virtual_destructor_test.test | RUN | 0.138631 | 0.340753 | 0.577092 | 0.001709 | 0.001842 | 0.001776 |
| has_unary_plus_test.test | RUN | 0.253483 | 1.207368 | 1.980420 | 0.001794 | 0.001914 | 0.001584 |
| has_unary_minus_test.test | RUN | 0.244496 | 1.248259 | 1.926943 | 0.001572 | 0.001812 | 0.001851 |
| has_trivial_move_constructor_test.test | RUN | 0.181790 | 0.514259 | 0.945663 | 0.001853 | 0.001557 | 0.001716 |
| has_trivial_move_assign_test.test | RUN | 0.178000 | 0.485205 | 0.909158 | 0.001842 | 0.001786 | 0.001808 |
| has_trivial_destructor_test.test | RUN | 0.179621 | 0.476535 | 0.909334 | 0.001766 | 0.001618 | 0.001792 |
| has_trivial_copy_test.test | RUN | 0.183086 | 0.521751 | 0.942804 | 0.001838 | 0.001934 | 0.001967 |
| has_trivial_constr_test.test | RUN | 0.182843 | 0.536467 | 1.008529 | 0.001467 | 0.001716 | 0.001937 |
| has_trivial_assign_test.test | RUN | 0.183865 | 0.526641 | 0.959959 | 0.001918 | 0.002005 | 0.001886 |
| has_right_shift_test.test | RUN | 0.264857 | 0.983562 | 2.083462 | 0.001966 | 0.001467 | 0.001423 |
| has_right_shift_assign_test.test | RUN | 0.254288 | 0.945256 | 1.981652 | 0.001740 | 0.001640 | 0.001666 |
| has_prefix_classes3_test.test | RUN | 0.186608 | 0.537934 | 1.042039 | 0.001644 | 0.001313 | 0.001732 |
| has_prefix_classes2_test.test | RUN | 0.196041 | 0.522574 | 1.011975 | 0.001355 | 0.001567 | 0.001781 |
| has_prefix_classes1_test.test | RUN | 0.186019 | 0.541139 | 0.966184 | 0.001674 | 0.001789 | 0.001712 |
| has_prefix_classes0_test.test | RUN | 0.183430 | 0.535868 | 1.037244 | 0.001253 | 0.001553 | 0.001779 |
| has_pre_increment_test.test | RUN | 0.246552 | 1.284802 | 1.962033 | 0.001564 | 0.001882 | 0.001635 |
| has_pre_decrement_test.test | RUN | 0.247301 | 1.264170 | 1.949327 | 0.001603 | 0.001716 | 0.001475 |
| has_postfix_classes3_test.test | RUN | 0.185130 | 0.534182 | 1.047720 | 0.001305 | 0.001750 | 0.001854 |
| has_postfix_classes2_test.test | RUN | 0.184775 | 0.523362 | 0.961429 | 0.001578 | 0.001560 | 0.001837 |
| has_postfix_classes1_test.test | RUN | 0.191070 | 0.535210 | 1.020483 | 0.001707 | 0.001905 | 0.001887 |
| has_postfix_classes0_test.test | RUN | 0.185983 | 0.549901 | 1.008471 | 0.001508 | 0.001706 | 0.001813 |
| has_post_increment_test.test | RUN | 0.249539 | 1.305870 | 2.007278 | 0.001493 | 0.001796 | 0.001836 |
| has_post_decrement_test.test | RUN | 0.246080 | 1.289965 | 1.989936 | 0.001646 | 0.001538 | 0.001818 |
| has_plus_test.test | RUN | 0.252882 | 0.930314 | 2.007405 | 0.001649 | 0.001708 | 0.001537 |
| has_plus_assign_test.test | RUN | 0.260534 | 0.922946 | 2.026646 | 0.001464 | 0.001685 | 0.001636 |
| has_operator_new_test.test | RUN | 0.167933 | 0.473747 | 0.855773 | 0.001753 | 0.001903 | 0.001898 |
| has_nothrow_destructor_test.test | RUN | 0.171389 | 0.536586 | 0.968667 | 0.001458 | 0.001768 | 0.001724 |
| has_nothrow_copy_test.test | RUN | 0.179016 | 0.508550 | 0.941523 | 0.001831 | 0.001706 | 0.001638 |
| has_nothrow_constr_test.test | RUN | 0.175318 | 0.486374 | 0.917981 | 0.001629 | 0.001520 | 0.001916 |
| has_nothrow_assign_test.test | RUN | 0.171801 | 0.494869 | 0.926120 | 0.001742 | 0.001829 | 0.001673 |
| has_not_equal_to_test.test | RUN | 0.253569 | 1.227489 | 2.008036 | 0.001531 | 0.001743 | 0.001765 |
| has_negate_test.test | RUN | 0.241354 | 0.879569 | 1.909720 | 0.001724 | 0.001915 | 0.001614 |
| has_multiplies_test.test | RUN | 0.245050 | 0.936604 | 1.920340 | 0.001619 | 0.001648 | 0.001154 |
| has_multiplies_assign_test.test | RUN | 0.249344 | 0.912313 | 1.994761 | 0.001771 | 0.001867 | 0.001706 |
| has_modulus_test.test | RUN | 0.247535 | 0.902201 | 1.947294 | 0.001389 | 0.001891 | 0.001780 |
| has_modulus_assign_test.test | RUN | 0.251328 | 0.933926 | 1.965244 | 0.001757 | 0.001684 | 0.001778 |
| has_minus_test.test | RUN | 0.258512 | 0.930658 | 1.965066 | 0.001546 | 0.001773 | 0.001475 |
| has_minus_assign_test.test | RUN | 0.257857 | 0.915705 | 2.014138 | 0.001850 | 0.001912 | 0.001776 |
| has_logical_or_test.test | RUN | 0.253255 | 0.921568 | 2.032488 | 0.001430 | 0.001753 | 0.001840 |
| has_logical_not_test.test | RUN | 0.245868 | 0.909351 | 1.926076 | 0.001895 | 0.001593 | 0.001866 |
| has_logical_and_test.test | RUN | 0.251574 | 0.923635 | 1.973680 | 0.001647 | 0.001860 | 0.001734 |
| has_less_test.test | RUN | 0.254494 | 1.230927 | 2.002012 | 0.001646 | 0.001886 | 0.001828 |
| has_less_equal_test.test | RUN | 0.258681 | 1.243916 | 1.970458 | 0.001319 | 0.001575 | 0.001822 |
| has_left_shift_test.test | RUN | 0.276634 | 1.145917 | 2.456614 | 0.001648 | 0.001560 | 0.001665 |
| has_left_shift_assign_test.test | RUN | 0.250389 | 0.935092 | 1.930999 | 0.001648 | 0.001717 | 0.001872 |
| has_greater_test.test | RUN | 0.255186 | 1.251035 | 1.986505 | 0.001906 | 0.001826 | 0.001688 |
| has_greater_equal_test.test | RUN | 0.265104 | 1.250000 | 1.975239 | 0.001686 | 0.001850 | 0.001848 |
| has_equal_to_test.test | RUN | 0.258896 | 1.241844 | 1.959798 | 0.001702 | 0.001755 | 0.001902 |
| has_divides_test.test | RUN | 0.253626 | 0.895342 | 1.998938 | 0.001652 | 0.001814 | 0.001882 |
| has_divides_assign_test.test | RUN | 0.251173 | 0.903774 | 2.000307 | 0.001840 | 0.001912 | 0.001737 |
| has_dereference_test.test | RUN | 0.243318 | 0.913392 | 1.857052 | 0.001778 | 0.005197 | 0.001802 |
| has_complement_test.test | RUN | 0.250016 | 1.241578 | 1.948125 | 0.001724 | 0.001928 | 0.001724 |
| has_bit_xor_test.test | RUN | 0.243073 | 0.907265 | 1.945762 | 0.001622 | 0.001939 | 0.001830 |
| has_bit_xor_assign_test.test | RUN | 0.251553 | 0.919284 | 1.954565 | 0.001500 | 0.001771 | 0.001756 |
| has_bit_or_test.test | RUN | 0.250332 | 0.895234 | 1.926764 | 0.001744 | 0.001882 | 0.001796 |
| has_bit_or_assign_test.test | RUN | 0.254021 | 0.904266 | 1.975319 | 0.001728 | 0.001812 | 0.001710 |
| has_bit_and_test.test | RUN | 0.248164 | 0.898760 | 1.950787 | 0.001508 | 0.001840 | 0.001830 |
| has_bit_and_assign_test.test | RUN | 0.250528 | 0.926923 | 1.946986 | 0.001737 | 0.001842 | 0.001709 |
| has_binary_classes9_test.test | RUN | 0.272311 | 1.064361 | 2.166355 | 0.001572 | 0.001845 | 0.001811 |
| has_binary_classes8_test.test | RUN | 0.270970 | 1.103838 | 2.182551 | 0.001354 | 0.002000 | 0.001662 |
| has_binary_classes7_test.test | RUN | 0.264186 | 1.092855 | 2.135960 | 0.001858 | 0.001714 | 0.001495 |
| has_binary_classes6_test.test | RUN | 0.267193 | 1.014585 | 2.155372 | 0.001604 | 0.001820 | 0.001525 |
| has_binary_classes5_test.test | RUN | 0.273921 | 1.041715 | 2.205234 | 0.001698 | 0.001876 | 0.001862 |
| has_binary_classes4_test.test | RUN | 0.270581 | 1.098431 | 2.115726 | 0.001650 | 0.001888 | 0.001825 |
| has_binary_classes3_test.test | RUN | 0.268720 | 1.069876 | 2.202495 | 0.001684 | 0.001670 | 0.001582 |
| has_binary_classes2_test.test | RUN | 0.272574 | 1.070809 | 2.187439 | 0.001690 | 0.001865 | 0.001552 |
| has_binary_classes1_test.test | RUN | 0.267903 | 1.100106 | 2.166115 | 0.001738 | 0.001723 | 0.001532 |
| has_binary_classes0_test.test | RUN | 0.268738 | 1.104858 | 2.116914 | 0.001714 | 0.001740 | 0.001617 |
| function_traits_test.test | RUN | 0.129799 | 0.270534 | 0.465153 | 0.001524 | 0.001924 | 0.001704 |
| floating_point_promotion_test.test | RUN | 0.126198 | 0.259998 | 0.437324 | 0.001668 | 0.001659 | 0.001444 |
| extent_test.test | RUN | 0.125703 | 0.281350 | 0.453890 | 0.001419 | 0.001760 | 0.001469 |
| enable_if_test.test | RUN | 0.117785 | 0.234127 | 0.357111 | 0.001537 | 0.001561 | 0.001570 |
| disjunction_test.test | RUN | 0.123143 | 0.243714 | 0.359509 | 0.001738 | 0.001650 | 0.001432 |
| detected_test.test | RUN | 0.121745 | 0.233481 | 0.352812 | 0.001682 | 0.001838 | 0.001503 |
| detected_or_test.test | RUN | 0.123266 | 0.245596 | 0.383506 | 0.001658 | 0.001738 | 0.001666 |
| decay_test.test | RUN | 0.289086 | 0.963886 | 2.307757 | 0.001670 | 0.001731 | 0.001398 |
| cxx14_aliases_test.test | RUN | 0.190079 | 0.368954 | 0.575234 | 0.001624 | 0.001774 | 0.001427 |
| copy_reference_test.test | RUN | 0.122942 | 0.256909 | 0.384050 | 0.001669 | 0.001874 | 0.001455 |
| copy_cv_test.test | RUN | 0.125753 | 0.243311 | 0.419580 | 0.001776 | 0.001708 | 0.001531 |
| copy_cv_ref_test.test | RUN | 0.170016 | 0.448372 | 0.922372 | 0.001660 | 0.001358 | 0.001567 |
| conjunction_test.test | RUN | 0.120255 | 0.258089 | 0.382875 | 0.001898 | 0.001342 | 0.001586 |
| conditional_test.test | RUN | 0.118970 | 0.236624 | 0.374334 | 0.001620 | 0.001602 | 0.001850 |
| common_type_test.test | RUN | 0.162009 | 0.387956 | 0.776953 | 0.001739 | 0.001549 | 0.001207 |
| common_type_sfinae_test.test | RUN | 0.118580 | 0.237749 | 0.323176 | 0.001842 | 0.001651 | 0.001638 |
| common_type_sfinae2_test.test | RUN | 0.120793 | 0.249738 | 0.362207 | 0.001504 | 0.001406 | 0.001592 |
| common_type_6_test.test | RUN | 0.125977 | 0.260995 | 0.380834 | 0.001680 | 0.001447 | 0.001782 |
| common_type_5_test.test | RUN | 0.127129 | 0.257492 | 0.379229 | 0.001734 | 0.001566 | 0.001761 |
| common_type_4_test.test | RUN | 0.127088 | 0.280313 | 0.455796 | 0.001312 | 0.001599 | 0.001592 |
| common_type_3_test.test | RUN | 0.117921 | 0.235723 | 0.353501 | 0.001640 | 0.001730 | 0.001531 |
| common_type_2_test.test | RUN | 0.158302 | 0.381440 | 0.705342 | 0.001580 | 0.001794 | 0.001849 |
| alignment_of_test.test | RUN | 0.129706 | 0.293977 | 0.461771 | 0.001804 | 0.001500 | 0.001631 |
| alignment_of_a2_test.test | RUN | 0.128711 | 0.290593 | 0.466400 | 0.001710 | 0.001466 | 0.001553 |
| aligned_storage_test.test | RUN | 0.137693 | 0.294371 | 0.401205 | 0.001515 | 0.001649 | 0.001738 |
| aligned_storage_empy_test.test | RUN | 0.153533 | 0.341480 | 0.554773 | 0.001800 | 0.001851 | 0.001677 |
| aligned_storage_a2_test.test | RUN | 0.130423 | 0.291164 | 0.409894 | 0.001772 | 0.001956 | 0.001794 |
| add_volatile_test.test | RUN | 0.263705 | 0.877992 | 2.233478 | 0.001559 | 0.001656 | 0.001772 |
| add_rvalue_reference_test.test | RUN | 0.296034 | 0.975190 | 2.530605 | 0.001804 | 0.001591 | 0.001778 |
| add_reference_test.test | RUN | 0.284310 | 0.949282 | 2.467002 | 0.001553 | 0.001565 | 0.001805 |
| add_pointer_test.test | RUN | 0.192046 | 0.547616 | 1.288920 | 0.001528 | 0.001684 | 0.001814 |
| add_lvalue_reference_test.test | RUN | 0.288209 | 0.959490 | 2.451459 | 0.001775 | 0.001623 | 0.001754 |
| add_cv_test.test | RUN | 0.271627 | 0.937892 | 2.327019 | 0.001568 | 0.001712 | 0.001720 |
| add_const_test.test | RUN | 0.280591 | 0.896948 | 2.329665 | 0.001427 | 0.001515 | 0.001587 |
| is_virtual_base_of_fail.test | COMPILE_FAIL | 0.117287 | 0.236482 | 0.344232 | — | — | — |
| is_stateless_fail.test | COMPILE_FAIL | 0.118432 | 0.237864 | 0.359672 | — | — | — |
| is_polymorphic_fail.test | COMPILE_FAIL | 0.110895 | 0.225485 | 0.339715 | — | — | — |
| is_nothrow_move_constructible_fail.test | COMPILE_FAIL | 0.111846 | 0.226436 | 0.295904 | — | — | — |
| is_nothrow_move_assignable_fail.test | COMPILE_FAIL | 0.113836 | 0.226800 | 0.311002 | — | — | — |
| is_list_constructible_fail.test | COMPILE_FAIL | 0.116465 | 0.229286 | 0.310393 | — | — | — |
| is_empty_fail.test | COMPILE_FAIL | 0.114466 | 0.230872 | 0.319431 | — | — | — |
| is_destructible_fail.test | COMPILE_FAIL | 0.113629 | 0.224466 | 0.323362 | — | — | — |
| is_default_constructible_fail.test | COMPILE_FAIL | 0.111449 | 0.221694 | 0.336983 | — | — | — |
| is_copy_constructible_fail.test | COMPILE_FAIL | 0.117074 | 0.232087 | 0.327172 | — | — | — |
| is_copy_assignable_fail.test | COMPILE_FAIL | 0.112756 | 0.225134 | 0.322439 | — | — | — |
| is_convertible_fail.test | COMPILE_FAIL | 0.115616 | 0.228090 | 0.331064 | — | — | — |
| is_convertible2_fail.test | COMPILE_FAIL | 0.112888 | 0.224128 | 0.323176 | — | — | — |
| is_constructible_fail.test | COMPILE_FAIL | 0.116322 | 0.229067 | 0.332526 | — | — | — |
| is_base_of_fail.test | COMPILE_FAIL | 0.110909 | 0.226098 | 0.331848 | — | — | — |
| is_assignable_fail.test | COMPILE_FAIL | 0.110047 | 0.232022 | 0.321350 | — | — | — |
| has_nothrow_destruct_fail.test | COMPILE_FAIL | 0.114199 | 0.225494 | 0.321923 | — | — | — |
| has_nothrow_copy_fail.test | COMPILE_FAIL | 0.117174 | 0.231933 | 0.329573 | — | — | — |
| has_nothrow_cons_fail.test | COMPILE_FAIL | 0.112771 | 0.235818 | 0.329021 | — | — | — |
| has_nothrow_assign_fail.test | COMPILE_FAIL | 0.114086 | 0.228143 | 0.322824 | — | — | — |
| common_type_fail.test | COMPILE_FAIL | 0.112003 | 0.228460 | 0.324464 | — | — | — |
| common_type2_fail.test | COMPILE_FAIL | 0.115969 | 0.222980 | 0.322140 | — | — | — |
| has_nothrow_assign_test_no_intrinsics.test | RUN | 0.180118 | 0.504923 | 0.962259 | 0.001360 | 0.001683 | 0.001644 |
| has_nothrow_constr_test_no_intrinsics.test | RUN | 0.168731 | 0.499679 | 0.939577 | 0.001553 | 0.001683 | 0.001712 |
| has_nothrow_copy_test_no_intrinsics.test | RUN | 0.171746 | 0.500141 | 0.907174 | 0.001631 | 0.001476 | 0.001713 |
| is_nothrow_move_assignable_test_no_intrinsics.test | RUN | 0.177895 | 0.500610 | 0.916410 | 0.001220 | 0.001356 | 0.001315 |
| is_nothrow_move_constructible_test_no_intrinsics.test | RUN | 0.114149 | 0.338041 | 0.558007 | 0.001296 | 0.001329 | 0.001312 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.987× | 0.977×, 0.998× | 0.969× |
| clang | 1.016×† | 1.047×, 0.984× | 1.026× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| add_const_test.test | 10 | 0.001052s | 0.000994s | 0.001103s | byte equality observed |
| add_cv_test.test | 10 | 0.001051s | 0.001073s | 0.001125s | byte equality observed |
| add_lvalue_reference_test.test | 10 | 0.001053s | 0.001004s | 0.001104s | byte equality observed |
| add_pointer_test.test | 10 | 0.001044s | 0.001060s | 0.001094s | byte equality observed |
| add_reference_test.test | 10 | 0.001060s | 0.001018s | 0.001110s | byte equality observed |
| add_rvalue_reference_test.test | 10 | 0.001058s | 0.001042s | 0.001076s | byte equality observed |
| add_volatile_test.test | 10 | 0.001037s | 0.001004s | 0.001138s | byte equality observed |
| aligned_storage_a2_test.test | 10 | 0.001130s | 0.001106s | 0.001321s | byte equality observed |
| aligned_storage_empy_test.test | 10 | 0.001029s | 0.001030s | 0.001094s | byte equality observed |
| aligned_storage_test.test | 10 | 0.001116s | 0.001122s | 0.001201s | byte equality observed |
| alignment_of_a2_test.test | 10 | 0.001010s | 0.001046s | 0.001117s | byte equality observed |
| alignment_of_test.test | 10 | 0.001052s | 0.001012s | 0.001098s | byte equality observed |
| common_type_2_test.test | 10 | 0.001048s | 0.001001s | 0.001112s | byte equality observed |
| common_type_3_test.test | 10 | 0.001034s | 0.001050s | 0.001112s | byte equality observed |
| common_type_4_test.test | 10 | 0.001080s | 0.000996s | 0.001113s | byte equality observed |
| common_type_5_test.test | 10 | 0.001046s | 0.001052s | 0.001095s | byte equality observed |
| common_type_6_test.test | 10 | 0.001051s | 0.000998s | 0.001103s | byte equality observed |
| common_type_sfinae2_test.test | 10 | 0.001041s | 0.001020s | 0.001135s | byte equality observed |
| common_type_sfinae_test.test | 10 | 0.001060s | 0.001061s | 0.001116s | byte equality observed |
| common_type_test.test | 10 | 0.001017s | 0.001077s | 0.001082s | byte equality observed |
| conditional_test.test | 10 | 0.001042s | 0.001010s | 0.001131s | byte equality observed |
| conjunction_test.test | 10 | 0.001071s | 0.001073s | 0.001101s | byte equality observed |
| copy_cv_ref_test.test | 10 | 0.001086s | 0.001029s | 0.001119s | byte equality observed |
| copy_cv_test.test | 10 | 0.001087s | 0.001049s | 0.001132s | byte equality observed |
| copy_reference_test.test | 10 | 0.001100s | 0.001076s | 0.001138s | byte equality observed |
| cxx14_aliases_test.test | 10 | 0.001098s | 0.001034s | 0.001100s | byte equality observed |
| decay_test.test | 10 | 0.001127s | 0.001097s | 0.001106s | byte equality observed |
| detected_or_test.test | 10 | 0.001136s | 0.001097s | 0.001122s | byte equality observed |
| detected_test.test | 10 | 0.001148s | 0.001090s | 0.001073s | byte equality observed |
| disjunction_test.test | 10 | 0.001143s | 0.001092s | 0.001093s | byte equality observed |
| enable_if_test.test | 10 | 0.001151s | 0.001085s | 0.001074s | byte equality observed |
| extent_test.test | 10 | 0.001156s | 0.001111s | 0.001097s | byte equality observed |
| floating_point_promotion_test.test | 10 | 0.001169s | 0.001081s | 0.001089s | byte equality observed |
| function_traits_test.test | 10 | 0.001178s | 0.001101s | 0.001112s | byte equality observed |
| has_binary_classes0_test.test | 10 | 0.001179s | 0.001125s | 0.001095s | byte equality observed |
| has_binary_classes1_test.test | 10 | 0.001165s | 0.001089s | 0.001094s | byte equality observed |
| has_binary_classes2_test.test | 10 | 0.001154s | 0.001105s | 0.001111s | byte equality observed |
| has_binary_classes3_test.test | 10 | 0.001182s | 0.001077s | 0.001123s | byte equality observed |
| has_binary_classes4_test.test | 10 | 0.001159s | 0.001095s | 0.001081s | byte equality observed |
| has_binary_classes5_test.test | 10 | 0.001162s | 0.001104s | 0.001113s | byte equality observed |
| has_binary_classes6_test.test | 10 | 0.001174s | 0.001101s | 0.001097s | byte equality observed |
| has_binary_classes7_test.test | 10 | 0.001167s | 0.001090s | 0.001111s | byte equality observed |
| has_binary_classes8_test.test | 10 | 0.001163s | 0.001095s | 0.001153s | byte equality observed |
| has_binary_classes9_test.test | 10 | 0.001177s | 0.001113s | 0.001074s | byte equality observed |
| has_bit_and_assign_test.test | 10 | 0.001171s | 0.001081s | 0.001110s | byte equality observed |
| has_bit_and_test.test | 10 | 0.001167s | 0.001105s | 0.001120s | byte equality observed |
| has_bit_or_assign_test.test | 10 | 0.001183s | 0.001087s | 0.001093s | byte equality observed |
| has_bit_or_test.test | 10 | 0.001200s | 0.001105s | 0.001098s | byte equality observed |
| has_bit_xor_assign_test.test | 10 | 0.001161s | 0.001091s | 0.001108s | byte equality observed |
| has_bit_xor_test.test | 10 | 0.001185s | 0.001122s | 0.001081s | byte equality observed |
| has_complement_test.test | 10 | 0.001154s | 0.001072s | 0.001102s | byte equality observed |
| has_dereference_test.test | 10 | 0.001172s | 0.001073s | 0.001118s | byte equality observed |
| has_divides_assign_test.test | 10 | 0.001166s | 0.001090s | 0.001089s | byte equality observed |
| has_divides_test.test | 10 | 0.001176s | 0.001121s | 0.001094s | byte equality observed |
| has_equal_to_test.test | 10 | 0.001187s | 0.001138s | 0.001101s | byte equality observed |
| has_greater_equal_test.test | 10 | 0.001168s | 0.001122s | 0.001093s | byte equality observed |
| has_greater_test.test | 10 | 0.001156s | 0.001154s | 0.001120s | byte equality observed |
| has_left_shift_assign_test.test | 10 | 0.001173s | 0.001152s | 0.001096s | byte equality observed |
| has_left_shift_test.test | 10 | 0.001136s | 0.001161s | 0.001109s | byte equality observed |
| has_less_equal_test.test | 10 | 0.001174s | 0.001118s | 0.001092s | byte equality observed |
| has_less_test.test | 10 | 0.001157s | 0.001150s | 0.001137s | byte equality observed |
| has_logical_and_test.test | 10 | 0.001178s | 0.001177s | 0.001107s | byte equality observed |
| has_logical_not_test.test | 10 | 0.001166s | 0.001205s | 0.001097s | byte equality observed |
| has_logical_or_test.test | 10 | 0.001170s | 0.001157s | 0.001100s | byte equality observed |
| has_minus_assign_test.test | 10 | 0.001182s | 0.001165s | 0.001086s | byte equality observed |
| has_minus_test.test | 10 | 0.001175s | 0.001179s | 0.001127s | byte equality observed |
| has_modulus_assign_test.test | 10 | 0.001186s | 0.001176s | 0.001075s | byte equality observed |
| has_modulus_test.test | 10 | 0.001165s | 0.001171s | 0.001101s | byte equality observed |
| has_multiplies_assign_test.test | 10 | 0.001184s | 0.001161s | 0.001079s | byte equality observed |
| has_multiplies_test.test | 10 | 0.001167s | 0.001165s | 0.001104s | byte equality observed |
| has_negate_test.test | 10 | 0.001166s | 0.001163s | 0.001112s | byte equality observed |
| has_not_equal_to_test.test | 10 | 0.001162s | 0.001160s | 0.001142s | byte equality observed |
| has_nothrow_assign_test.test | 10 | 0.001187s | 0.001173s | 0.001146s | byte equality observed |
| has_nothrow_assign_test_no_intrinsics.test | 10 | 0.001175s | 0.001151s | 0.001168s | byte equality observed |
| has_nothrow_constr_test.test | 10 | 0.001176s | 0.001163s | 0.001148s | byte equality observed |
| has_nothrow_constr_test_no_intrinsics.test | 10 | 0.001163s | 0.001144s | 0.001180s | byte equality observed |
| has_nothrow_copy_test.test | 10 | 0.001178s | 0.001167s | 0.001186s | byte equality observed |
| has_nothrow_copy_test_no_intrinsics.test | 10 | 0.001176s | 0.001172s | 0.001180s | byte equality observed |
| has_nothrow_destructor_test.test | 10 | 0.001170s | 0.001181s | 0.001187s | byte equality observed |
| has_operator_new_test.test | 10 | 0.001161s | 0.001186s | 0.001162s | byte equality observed |
| has_plus_assign_test.test | 10 | 0.001166s | 0.001177s | 0.001170s | byte equality observed |
| has_plus_test.test | 10 | 0.001182s | 0.001182s | 0.001178s | byte equality observed |
| has_post_decrement_test.test | 10 | 0.001192s | 0.001178s | 0.001172s | byte equality observed |
| has_post_increment_test.test | 10 | 0.001166s | 0.001160s | 0.001175s | byte equality observed |
| has_postfix_classes0_test.test | 10 | 0.001178s | 0.001144s | 0.001200s | byte equality observed |
| has_postfix_classes1_test.test | 10 | 0.001185s | 0.001163s | 0.001172s | byte equality observed |
| has_postfix_classes2_test.test | 10 | 0.001171s | 0.001156s | 0.001172s | byte equality observed |
| has_postfix_classes3_test.test | 10 | 0.001177s | 0.001183s | 0.001209s | byte equality observed |
| has_pre_decrement_test.test | 10 | 0.001187s | 0.001145s | 0.001181s | byte equality observed |
| has_pre_increment_test.test | 10 | 0.001171s | 0.001177s | 0.001166s | byte equality observed |
| has_prefix_classes0_test.test | 10 | 0.001184s | 0.001166s | 0.001174s | byte equality observed |
| has_prefix_classes1_test.test | 10 | 0.001173s | 0.001164s | 0.001187s | byte equality observed |
| has_prefix_classes2_test.test | 10 | 0.001160s | 0.001177s | 0.001170s | byte equality observed |
| has_prefix_classes3_test.test | 10 | 0.001188s | 0.001166s | 0.001159s | byte equality observed |
| has_right_shift_assign_test.test | 10 | 0.001177s | 0.001165s | 0.001189s | byte equality observed |
| has_right_shift_test.test | 10 | 0.001164s | 0.001164s | 0.001172s | byte equality observed |
| has_trivial_assign_test.test | 10 | 0.001234s | 0.001219s | 0.001230s | byte equality observed |
| has_trivial_constr_test.test | 10 | 0.001243s | 0.001228s | 0.001261s | native test oracle; output varies |
| has_trivial_copy_test.test | 10 | 0.001250s | 0.001261s | 0.001286s | native test oracle; output varies |
| has_trivial_destructor_test.test | 10 | 0.001140s | 0.001173s | 0.001173s | byte equality observed |
| has_trivial_move_assign_test.test | 10 | 0.001173s | 0.001202s | 0.001212s | byte equality observed |
| has_trivial_move_constructor_test.test | 10 | 0.001193s | 0.001167s | 0.001160s | byte equality observed |
| has_unary_minus_test.test | 10 | 0.001178s | 0.001138s | 0.001187s | byte equality observed |
| has_unary_plus_test.test | 10 | 0.001170s | 0.001161s | 0.001173s | byte equality observed |
| has_virtual_destructor_test.test | 10 | 0.001178s | 0.001187s | 0.001173s | byte equality observed |
| integral_constant_test.test | 10 | 0.000536s | 0.000536s | 0.001172s | byte equality observed |
| is_abstract_test.test | 10 | 0.001192s | 0.001206s | 0.001161s | byte equality observed |
| is_arithmetic_test.test | 10 | 0.001195s | 0.001158s | 0.001162s | byte equality observed |
| is_array_test.test | 10 | 0.001191s | 0.001173s | 0.001160s | byte equality observed |
| is_assignable_test.test | 10 | 0.001185s | 0.001183s | 0.001167s | byte equality observed |
| is_base_and_derived_test.test | 10 | 0.001176s | 0.001159s | 0.001169s | byte equality observed |
| is_base_of_test.test | 10 | 0.001188s | 0.001173s | 0.001146s | byte equality observed |
| is_bounded_array_test.test | 10 | 0.001182s | 0.001156s | 0.001183s | byte equality observed |
| is_class_test.test | 10 | 0.001182s | 0.001160s | 0.001181s | byte equality observed |
| is_complete_test.test | 10 | 0.001205s | 0.001176s | 0.001183s | byte equality observed |
| is_complex_test.test | 10 | 0.001172s | 0.001170s | 0.001180s | byte equality observed |
| is_compound_test.test | 10 | 0.001173s | 0.001181s | 0.001180s | byte equality observed |
| is_const_test.test | 10 | 0.001180s | 0.001161s | 0.001179s | byte equality observed |
| is_constructible_test.test | 10 | 0.001182s | 0.001186s | 0.001172s | byte equality observed |
| is_convertible_test.test | 10 | 0.001186s | 0.001174s | 0.001198s | byte equality observed |
| is_copy_assignable_test.test | 10 | 0.001189s | 0.001156s | 0.001183s | byte equality observed |
| is_copy_constructible_test.test | 10 | 0.001178s | 0.001185s | 0.001189s | byte equality observed |
| is_default_constr_test.test | 10 | 0.001185s | 0.001181s | 0.001183s | byte equality observed |
| is_destructible_test.test | 10 | 0.001187s | 0.001174s | 0.001177s | byte equality observed |
| is_detected_exact_test.test | 10 | 0.001181s | 0.001175s | 0.001182s | byte equality observed |
| is_detected_test.test | 10 | 0.001177s | 0.001172s | 0.001190s | byte equality observed |
| is_empty_test.test | 10 | 0.001162s | 0.001164s | 0.001174s | byte equality observed |
| is_enum_test.test | 10 | 0.001181s | 0.001183s | 0.001190s | byte equality observed |
| is_final_test.test | 10 | 0.001171s | 0.001185s | 0.001174s | byte equality observed |
| is_float_test.test | 10 | 0.001173s | 0.001189s | 0.001161s | byte equality observed |
| is_floating_point_test.test | 10 | 0.001185s | 0.001193s | 0.001194s | byte equality observed |
| is_function_test.test | 10 | 0.001170s | 0.001157s | 0.001175s | byte equality observed |
| is_fundamental_test.test | 10 | 0.001184s | 0.001169s | 0.001184s | byte equality observed |
| is_integral_test.test | 10 | 0.001172s | 0.001177s | 0.001153s | byte equality observed |
| is_list_constructible_test.test | 10 | 0.001169s | 0.001173s | 0.001169s | byte equality observed |
| is_lvalue_reference_test.test | 10 | 0.001191s | 0.001183s | 0.001178s | byte equality observed |
| is_member_func_test.test | 10 | 0.001167s | 0.001172s | 0.001176s | byte equality observed |
| is_member_obj_test.test | 10 | 0.001176s | 0.001182s | 0.001160s | byte equality observed |
| is_member_pointer_test.test | 10 | 0.001190s | 0.001168s | 0.001166s | byte equality observed |
| is_noncopyable_test.test | 10 | 0.001168s | 0.001164s | 0.001173s | byte equality observed |
| is_nothrow_move_assignable_test.test | 10 | 0.001178s | 0.001177s | 0.001169s | byte equality observed |
| is_nothrow_move_assignable_test_no_intrinsics.test | 10 | 0.001179s | 0.001171s | 0.001200s | byte equality observed |
| is_nothrow_move_constructible_test.test | 10 | 0.001182s | 0.001173s | 0.001178s | byte equality observed |
| is_nothrow_move_constructible_test_no_intrinsics.test | 10 | 0.001172s | 0.001185s | 0.001183s | byte equality observed |
| is_nothrow_swappable_test.test | 10 | 0.001180s | 0.001169s | 0.001163s | byte equality observed |
| is_object_test.test | 10 | 0.001169s | 0.001190s | 0.001175s | byte equality observed |
| is_pod_test.test | 10 | 0.001199s | 0.001174s | 0.001155s | byte equality observed |
| is_pointer_test.test | 10 | 0.001178s | 0.001166s | 0.001185s | byte equality observed |
| is_polymorphic_test.test | 10 | 0.001174s | 0.001177s | 0.001181s | byte equality observed |
| is_reference_test.test | 10 | 0.001181s | 0.001191s | 0.001175s | byte equality observed |
| is_rvalue_reference_test.test | 10 | 0.001181s | 0.001178s | 0.001183s | byte equality observed |
| is_same_test.test | 10 | 0.001168s | 0.001165s | 0.001183s | byte equality observed |
| is_scalar_test.test | 10 | 0.001173s | 0.001144s | 0.001149s | byte equality observed |
| is_scoped_enum_test.test | 10 | 0.001153s | 0.001173s | 0.001177s | byte equality observed |
| is_signed_test.test | 10 | 0.001168s | 0.001182s | 0.001161s | byte equality observed |
| is_stateless_test.test | 10 | 0.001180s | 0.001167s | 0.001170s | byte equality observed |
| is_swappable_test.test | 10 | 0.001175s | 0.001155s | 0.001168s | byte equality observed |
| is_trivially_copyable_test.test | 10 | 0.001206s | 0.001206s | 0.001216s | byte equality observed |
| is_unbounded_array_test.test | 10 | 0.001188s | 0.001175s | 0.001176s | byte equality observed |
| is_union_test.test | 10 | 0.001187s | 0.001176s | 0.001176s | byte equality observed |
| is_unscoped_enum_test.test | 10 | 0.001179s | 0.001183s | 0.001164s | byte equality observed |
| is_unsigned_test.test | 10 | 0.001204s | 0.001184s | 0.001172s | byte equality observed |
| is_virtual_base_of_test.test | 10 | 0.001167s | 0.001164s | 0.001207s | byte equality observed |
| is_void_test.test | 10 | 0.001179s | 0.001188s | 0.001164s | byte equality observed |
| is_volatile_test.test | 10 | 0.001173s | 0.001183s | 0.001177s | byte equality observed |
| make_signed_test.test | 10 | 0.001183s | 0.001173s | 0.001194s | byte equality observed |
| make_unsigned_test.test | 10 | 0.001186s | 0.001181s | 0.001147s | byte equality observed |
| make_void_test.test | 10 | 0.001188s | 0.001192s | 0.001184s | byte equality observed |
| mpl_interop_test1.test | 10 | 0.000531s | 0.000538s | 0.001183s | byte equality observed |
| mpl_interop_test2.test | 10 | 0.000537s | 0.000538s | 0.001204s | byte equality observed |
| mpl_interop_test3.test | 10 | 0.000538s | 0.000549s | 0.001166s | byte equality observed |
| negation_test.test | 10 | 0.001217s | 0.001230s | 0.001156s | byte equality observed |
| promote_basic_test.test | 10 | 0.000541s | 0.000538s | 0.001181s | byte equality observed |
| promote_enum_msvc_bug_test.test | 10 | 0.000533s | 0.000535s | 0.001184s | byte equality observed |
| promote_enum_test.test | 10 | 0.000537s | 0.000535s | 0.001208s | byte equality observed |
| promote_mpl_test.test | 10 | 0.000531s | 0.000540s | 0.001181s | byte equality observed |
| rank_test.test | 10 | 0.001179s | 0.001183s | 0.001172s | byte equality observed |
| remove_all_extents_test.test | 10 | 0.001199s | 0.001167s | 0.001168s | byte equality observed |
| remove_bounds_test.test | 10 | 0.001160s | 0.001166s | 0.001178s | byte equality observed |
| remove_const_test.test | 10 | 0.001185s | 0.001181s | 0.001170s | byte equality observed |
| remove_cv_ref_test.test | 10 | 0.001194s | 0.001180s | 0.001186s | byte equality observed |
| remove_cv_test.test | 10 | 0.001154s | 0.001183s | 0.001174s | byte equality observed |
| remove_extent_test.test | 10 | 0.001174s | 0.001169s | 0.001162s | byte equality observed |
| remove_pointer_test.test | 10 | 0.001182s | 0.001181s | 0.001164s | byte equality observed |
| remove_reference_test.test | 10 | 0.001172s | 0.001171s | 0.001183s | byte equality observed |
| remove_volatile_test.test | 10 | 0.001179s | 0.001188s | 0.001187s | byte equality observed |
| tricky_abstract_type_test.test | 10 | 0.001174s | 0.001187s | 0.001159s | byte equality observed |
| tricky_add_pointer_test.test | 10 | 0.001185s | 0.001154s | 0.001164s | byte equality observed |
| tricky_function_type_test.test | 10 | 0.001180s | 0.001178s | 0.001161s | byte equality observed |
| tricky_incomplete_type_test.test | 10 | 0.001189s | 0.001157s | 0.001165s | byte equality observed |
| tricky_is_enum_test.test | 10 | 0.001164s | 0.001159s | 0.001178s | byte equality observed |
| tricky_partial_spec_test.test | 10 | 0.001184s | 0.001172s | 0.001183s | byte equality observed |
| tricky_rvalue_test.test | 10 | 0.001179s | 0.001199s | 0.001186s | byte equality observed |
| type_identity_test.test | 10 | 0.001190s | 0.001207s | 0.001171s | byte equality observed |
| type_traits_test.test | 10 | 0.000537s | 0.000529s | 0.001174s | byte equality observed |
| type_with_alignment_test.test | 10 | 0.001316s | 0.001298s | 0.001313s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
