# libs/type_traits/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| type_with_alignment_test.test | RUN | 0.112714 | 0.261156 | 0.350009 | 0.001694 | 0.001627 | 0.001660 |
| type_traits_test.test | RUN | 0.156482 | 0.273212 | 0.335429 | 0.000559 | 0.000594 | 0.001675 |
| type_identity_test.test | RUN | 0.250417 | 0.859325 | 1.805862 | 0.002031 | 0.001501 | 0.001707 |
| tricky_rvalue_test.test | RUN | 0.102689 | 0.206348 | 0.269518 | 0.001789 | 0.001644 | 0.001878 |
| tricky_partial_spec_test.test | RUN | 0.123027 | 0.293427 | 0.444258 | 0.001556 | 0.001285 | 0.001641 |
| tricky_is_enum_test.test | RUN | 0.101400 | 0.191020 | 0.260044 | 0.001916 | 0.001585 | 0.001740 |
| tricky_incomplete_type_test.test | RUN | 0.167770 | 0.303980 | 0.419456 | 0.001766 | 0.001492 | 0.001395 |
| tricky_function_type_test.test | RUN | 0.134997 | 0.314621 | 0.495782 | 0.001737 | 0.001599 | 0.001450 |
| tricky_add_pointer_test.test | RUN | 0.175793 | 0.517190 | 1.011243 | 0.001664 | 0.001845 | 0.001732 |
| tricky_abstract_type_test.test | RUN | 0.104997 | 0.198049 | 0.265912 | 0.001772 | 0.001549 | 0.001604 |
| remove_volatile_test.test | RUN | 0.232503 | 0.764780 | 1.540400 | 0.001839 | 0.001786 | 0.001782 |
| remove_reference_test.test | RUN | 0.236613 | 0.839544 | 1.625824 | 0.001839 | 0.001784 | 0.001828 |
| remove_pointer_test.test | RUN | 0.243959 | 0.859290 | 1.752114 | 0.001743 | 0.001542 | 0.001721 |
| remove_extent_test.test | RUN | 0.232547 | 0.813945 | 1.666280 | 0.001582 | 0.001841 | 0.001706 |
| remove_cv_test.test | RUN | 0.236693 | 0.798811 | 1.717809 | 0.001861 | 0.001841 | 0.001725 |
| remove_cv_ref_test.test | RUN | 0.465812 | 1.923433 | 3.973348 | 0.001497 | 0.001479 | 0.001771 |
| remove_const_test.test | RUN | 0.230943 | 0.767491 | 1.594896 | 0.001874 | 0.001862 | 0.001713 |
| remove_bounds_test.test | RUN | 0.237784 | 0.811161 | 1.662174 | 0.001578 | 0.001681 | 0.001523 |
| remove_all_extents_test.test | RUN | 0.237843 | 0.813085 | 1.550826 | 0.001714 | 0.001588 | 0.001872 |
| rank_test.test | RUN | 0.104950 | 0.204271 | 0.282251 | 0.001470 | 0.001730 | 0.001724 |
| promote_mpl_test.test | RUN | 0.085916 | 0.180214 | 0.202373 | 0.000596 | 0.000786 | 0.001549 |
| promote_enum_test.test | RUN | 0.020710 | 0.046477 | 0.053539 | 0.000719 | 0.000636 | 0.001693 |
| promote_enum_msvc_bug_test.test | RUN | 0.024672 | 0.041189 | 0.048343 | 0.000635 | 0.000611 | 0.001584 |
| promote_basic_test.test | RUN | 0.023817 | 0.053552 | 0.055942 | 0.000592 | 0.000512 | 0.001569 |
| negation_test.test | RUN | 0.102615 | 0.182100 | 0.237760 | 0.001480 | 0.001557 | 0.001423 |
| mpl_interop_test3.test | RUN | 0.083245 | 0.171772 | 0.186450 | 0.000626 | 0.000622 | 0.001653 |
| mpl_interop_test2.test | RUN | 0.036092 | 0.051078 | 0.054016 | 0.000616 | 0.000547 | 0.001682 |
| mpl_interop_test1.test | RUN | 0.017134 | 0.037521 | 0.042368 | 0.000593 | 0.000646 | 0.001818 |
| make_void_test.test | RUN | 0.104396 | 0.196993 | 0.287007 | 0.001884 | 0.001710 | 0.001456 |
| make_unsigned_test.test | RUN | 0.121356 | 0.259187 | 0.373989 | 0.001722 | 0.001602 | 0.001604 |
| make_signed_test.test | RUN | 0.122573 | 0.253884 | 0.363941 | 0.001604 | 0.001506 | 0.001748 |
| is_volatile_test.test | RUN | 0.111436 | 0.215118 | 0.316603 | 0.001422 | 0.001551 | 0.001443 |
| is_void_test.test | RUN | 0.106794 | 0.207132 | 0.291283 | 0.001906 | 0.001775 | 0.001114 |
| is_virtual_base_of_test.test | RUN | 0.121179 | 0.265981 | 0.412839 | 0.001596 | 0.001306 | 0.001744 |
| is_unsigned_test.test | RUN | 0.109192 | 0.214290 | 0.320157 | 0.001614 | 0.001447 | 0.001240 |
| is_unscoped_enum_test.test | RUN | 0.104660 | 0.197175 | 0.250832 | 0.001560 | 0.001442 | 0.001344 |
| is_union_test.test | RUN | 0.116350 | 0.254997 | 0.343404 | 0.001788 | 0.001808 | 0.001620 |
| is_unbounded_array_test.test | RUN | 0.108187 | 0.218505 | 0.280207 | 0.001568 | 0.001608 | 0.001588 |
| is_trivially_copyable_test.test | RUN | 0.165580 | 0.453809 | 0.681887 | 0.001598 | 0.001920 | 0.001487 |
| is_swappable_test.test | RUN | 0.138329 | 0.336478 | 0.522738 | 0.001516 | 0.001674 | 0.001633 |
| is_stateless_test.test | RUN | 0.155465 | 0.398091 | 0.591316 | 0.001789 | 0.001535 | 0.001443 |
| is_signed_test.test | RUN | 0.107295 | 0.239375 | 0.303892 | 0.001527 | 0.001558 | 0.001377 |
| is_scoped_enum_test.test | RUN | 0.105606 | 0.210320 | 0.251930 | 0.001490 | 0.001775 | 0.001548 |
| is_scalar_test.test | RUN | 0.136506 | 0.364805 | 0.540395 | 0.001499 | 0.001737 | 0.001606 |
| is_same_test.test | RUN | 0.105869 | 0.216464 | 0.287828 | 0.001585 | 0.001450 | 0.001593 |
| is_rvalue_reference_test.test | RUN | 0.112099 | 0.242729 | 0.305297 | 0.001560 | 0.001550 | 0.001292 |
| is_reference_test.test | RUN | 0.111651 | 0.232284 | 0.356036 | 0.001552 | 0.001587 | 0.001601 |
| is_polymorphic_test.test | RUN | 0.115427 | 0.242886 | 0.354415 | 0.001390 | 0.001498 | 0.001670 |
| is_pointer_test.test | RUN | 0.110779 | 0.236109 | 0.350254 | 0.001501 | 0.001325 | 0.001479 |
| is_pod_test.test | RUN | 0.140943 | 0.381820 | 0.590708 | 0.001454 | 0.001905 | 0.001629 |
| is_object_test.test | RUN | 0.102626 | 0.207360 | 0.249281 | 0.001523 | 0.001581 | 0.001630 |
| is_nothrow_swappable_test.test | RUN | 0.145022 | 0.388235 | 0.584407 | 0.001592 | 0.002088 | 0.001533 |
| is_nothrow_move_constructible_test.test | RUN | 0.141359 | 0.403833 | 0.577296 | 0.001624 | 0.001984 | 0.001261 |
| is_nothrow_move_assignable_test.test | RUN | 0.140790 | 0.448545 | 0.587624 | 0.001472 | 0.001822 | 0.001732 |
| is_noncopyable_test.test | RUN | 0.104015 | 0.242725 | 0.283304 | 0.001578 | 0.001569 | 0.001627 |
| is_member_pointer_test.test | RUN | 0.105296 | 0.239894 | 0.292554 | 0.001514 | 0.001487 | 0.001624 |
| is_member_obj_test.test | RUN | 0.105084 | 0.237922 | 0.294089 | 0.001816 | 0.001377 | 0.001437 |
| is_member_func_test.test | RUN | 0.107812 | 0.249819 | 0.309580 | 0.001617 | 0.001640 | 0.001610 |
| is_lvalue_reference_test.test | RUN | 0.112032 | 0.251050 | 0.334990 | 0.001449 | 0.001597 | 0.001696 |
| is_list_constructible_test.test | RUN | 0.131956 | 0.306562 | 0.354158 | 0.001508 | 0.001622 | 0.001638 |
| is_integral_test.test | RUN | 0.127628 | 0.376376 | 0.512438 | 0.001620 | 0.001846 | 0.001721 |
| is_fundamental_test.test | RUN | 0.128042 | 0.384174 | 0.549507 | 0.001574 | 0.001700 | 0.001473 |
| is_function_test.test | RUN | 0.108250 | 0.252641 | 0.313356 | 0.001515 | 0.001733 | 0.001790 |
| is_floating_point_test.test | RUN | 0.111555 | 0.271844 | 0.344369 | 0.001436 | 0.001595 | 0.001659 |
| is_float_test.test | RUN | 0.113296 | 0.268695 | 0.337264 | 0.001701 | 0.001904 | 0.001706 |
| is_final_test.test | RUN | 0.109311 | 0.267897 | 0.348213 | 0.001690 | 0.001513 | 0.001299 |
| is_enum_test.test | RUN | 0.101385 | 0.233366 | 0.251734 | 0.001486 | 0.001794 | 0.001344 |
| is_empty_test.test | RUN | 0.111861 | 0.252244 | 0.301751 | 0.001440 | 0.001840 | 0.001679 |
| is_detected_test.test | RUN | 0.097485 | 0.217909 | 0.234919 | 0.001730 | 0.001746 | 0.001563 |
| is_detected_exact_test.test | RUN | 0.097512 | 0.220065 | 0.246195 | 0.001561 | 0.001687 | 0.001464 |
| is_destructible_test.test | RUN | 0.137722 | 0.415865 | 0.551103 | 0.001656 | 0.001956 | 0.001518 |
| is_default_constr_test.test | RUN | 0.140352 | 0.462524 | 0.589846 | 0.001514 | 0.001736 | 0.001567 |
| is_copy_constructible_test.test | RUN | 0.147938 | 0.473928 | 0.609531 | 0.001752 | 0.001667 | 0.001764 |
| is_copy_assignable_test.test | RUN | 0.146116 | 0.466577 | 0.611873 | 0.001658 | 0.001574 | 0.001484 |
| is_convertible_test.test | RUN | 0.140528 | 0.408817 | 0.521732 | 0.001476 | 0.001898 | 0.001574 |
| is_constructible_test.test | RUN | 0.120951 | 0.301007 | 0.383937 | 0.001608 | 0.001838 | 0.001544 |
| is_const_test.test | RUN | 0.109869 | 0.246747 | 0.291944 | 0.001852 | 0.001543 | 0.001668 |
| is_compound_test.test | RUN | 0.105193 | 0.230742 | 0.260412 | 0.001769 | 0.001576 | 0.001402 |
| is_complex_test.test | RUN | 0.136223 | 0.295105 | 0.345928 | 0.001590 | 0.002024 | 0.001771 |
| is_complete_test.test | RUN | 0.121729 | 0.302271 | 0.405321 | 0.001512 | 0.001510 | 0.001402 |
| is_class_test.test | RUN | 0.109085 | 0.271101 | 0.349477 | 0.001672 | 0.001431 | 0.001363 |
| is_bounded_array_test.test | RUN | 0.103463 | 0.243900 | 0.284691 | 0.001824 | 0.001588 | 0.001644 |
| is_base_of_test.test | RUN | 0.112701 | 0.256842 | 0.322309 | 0.001590 | 0.001977 | 0.001752 |
| is_base_and_derived_test.test | RUN | 0.110344 | 0.254155 | 0.320637 | 0.001756 | 0.001985 | 0.001740 |
| is_assignable_test.test | RUN | 0.130712 | 0.373002 | 0.485039 | 0.001442 | 0.001704 | 0.001820 |
| is_array_test.test | RUN | 0.107393 | 0.244081 | 0.309361 | 0.001681 | 0.001546 | 0.001607 |
| is_arithmetic_test.test | RUN | 0.128508 | 0.369616 | 0.521195 | 0.001695 | 0.001565 | 0.001878 |
| is_abstract_test.test | RUN | 0.224330 | 1.044790 | 1.656705 | 0.001798 | 0.001765 | 0.001567 |
| integral_constant_test.test | RUN | 0.016904 | 0.041767 | 0.045496 | 0.000618 | 0.000835 | 0.001797 |
| has_virtual_destructor_test.test | RUN | 0.114123 | 0.303521 | 0.357183 | 0.001800 | 0.001772 | 0.001708 |
| has_unary_plus_test.test | RUN | 0.203268 | 1.137100 | 1.309514 | 0.001785 | 0.001954 | 0.001499 |
| has_unary_minus_test.test | RUN | 0.200360 | 1.128412 | 1.230941 | 0.001539 | 0.001978 | 0.001692 |
| has_trivial_move_constructor_test.test | RUN | 0.148380 | 0.414632 | 0.627150 | 0.001549 | 0.001682 | 0.001660 |
| has_trivial_move_assign_test.test | RUN | 0.150490 | 0.453182 | 0.601616 | 0.001862 | 0.001850 | 0.001698 |
| has_trivial_destructor_test.test | RUN | 0.149814 | 0.454628 | 0.617738 | 0.001926 | 0.001728 | 0.001805 |
| has_trivial_copy_test.test | RUN | 0.152367 | 0.485736 | 0.685553 | 0.001695 | 0.002634 | 0.001440 |
| has_trivial_constr_test.test | RUN | 0.156891 | 0.467272 | 0.672456 | 0.001630 | 0.001544 | 0.001784 |
| has_trivial_assign_test.test | RUN | 0.152622 | 0.487905 | 0.637811 | 0.001941 | 0.001867 | 0.001699 |
| has_right_shift_test.test | RUN | 0.211818 | 0.825949 | 1.302065 | 0.001652 | 0.001740 | 0.001176 |
| has_right_shift_assign_test.test | RUN | 0.205929 | 0.802046 | 1.229079 | 0.001572 | 0.001720 | 0.001405 |
| has_prefix_classes3_test.test | RUN | 0.158996 | 0.466012 | 0.660896 | 0.001456 | 0.001556 | 0.001685 |
| has_prefix_classes2_test.test | RUN | 0.155890 | 0.416334 | 0.650407 | 0.001799 | 0.002006 | 0.001786 |
| has_prefix_classes1_test.test | RUN | 0.153713 | 0.442540 | 0.625606 | 0.001784 | 0.001491 | 0.001628 |
| has_prefix_classes0_test.test | RUN | 0.154243 | 0.459300 | 0.653973 | 0.001539 | 0.001439 | 0.001575 |
| has_pre_increment_test.test | RUN | 0.195791 | 1.074164 | 1.261054 | 0.001479 | 0.001681 | 0.001335 |
| has_pre_decrement_test.test | RUN | 0.197147 | 1.070093 | 1.260631 | 0.001874 | 0.001492 | 0.001663 |
| has_postfix_classes3_test.test | RUN | 0.153603 | 0.449631 | 0.636410 | 0.001804 | 0.001713 | 0.001603 |
| has_postfix_classes2_test.test | RUN | 0.152477 | 0.440756 | 0.616959 | 0.001632 | 0.001446 | 0.001641 |
| has_postfix_classes1_test.test | RUN | 0.156159 | 0.458922 | 0.646471 | 0.001530 | 0.001752 | 0.001700 |
| has_postfix_classes0_test.test | RUN | 0.158497 | 0.453890 | 0.668022 | 0.001834 | 0.001552 | 0.001692 |
| has_post_increment_test.test | RUN | 0.196854 | 1.096847 | 1.292862 | 0.001810 | 0.001823 | 0.001676 |
| has_post_decrement_test.test | RUN | 0.198511 | 1.095873 | 1.280228 | 0.001572 | 0.001542 | 0.001695 |
| has_plus_test.test | RUN | 0.196019 | 0.781128 | 1.252928 | 0.001593 | 0.001585 | 0.001588 |
| has_plus_assign_test.test | RUN | 0.229976 | 0.783693 | 1.279213 | 0.001892 | 0.001514 | 0.001603 |
| has_operator_new_test.test | RUN | 0.152471 | 0.393419 | 0.549594 | 0.001884 | 0.001795 | 0.001571 |
| has_nothrow_destructor_test.test | RUN | 0.150425 | 0.441035 | 0.654418 | 0.001551 | 0.001764 | 0.001805 |
| has_nothrow_copy_test.test | RUN | 0.153261 | 0.408138 | 0.637696 | 0.001758 | 0.001267 | 0.001732 |
| has_nothrow_constr_test.test | RUN | 0.144341 | 0.404021 | 0.594500 | 0.001846 | 0.001562 | 0.001716 |
| has_nothrow_assign_test.test | RUN | 0.146067 | 0.403049 | 0.626392 | 0.001801 | 0.001763 | 0.001708 |
| has_not_equal_to_test.test | RUN | 0.201361 | 0.985091 | 1.277257 | 0.001778 | 0.001822 | 0.001779 |
| has_negate_test.test | RUN | 0.197558 | 0.732324 | 1.237158 | 0.001490 | 0.001806 | 0.001733 |
| has_multiplies_test.test | RUN | 0.207018 | 0.751630 | 1.286413 | 0.001864 | 0.001572 | 0.001567 |
| has_multiplies_assign_test.test | RUN | 0.209786 | 0.770838 | 1.216641 | 0.001810 | 0.001896 | 0.001576 |
| has_modulus_test.test | RUN | 0.196085 | 0.902955 | 1.281516 | 0.001722 | 0.001954 | 0.001750 |
| has_modulus_assign_test.test | RUN | 0.199704 | 0.908172 | 1.309119 | 0.001732 | 0.001471 | 0.001748 |
| has_minus_test.test | RUN | 0.211039 | 0.852991 | 1.303629 | 0.001914 | 0.001821 | 0.001538 |
| has_minus_assign_test.test | RUN | 0.206322 | 0.860582 | 1.279936 | 0.001658 | 0.001874 | 0.001548 |
| has_logical_or_test.test | RUN | 0.202197 | 0.786735 | 1.257372 | 0.001503 | 0.001798 | 0.001698 |
| has_logical_not_test.test | RUN | 0.200465 | 0.780510 | 1.232846 | 0.001843 | 0.001683 | 0.001744 |
| has_logical_and_test.test | RUN | 0.205798 | 0.789679 | 1.311487 | 0.001848 | 0.001750 | 0.001458 |
| has_less_test.test | RUN | 0.207461 | 1.032251 | 1.260740 | 0.001677 | 0.001808 | 0.001613 |
| has_less_equal_test.test | RUN | 0.209583 | 0.979504 | 1.278132 | 0.001555 | 0.001651 | 0.001740 |
| has_left_shift_test.test | RUN | 0.224981 | 0.965635 | 1.608984 | 0.001654 | 0.001582 | 0.001547 |
| has_left_shift_assign_test.test | RUN | 0.208099 | 0.765584 | 1.212582 | 0.001805 | 0.001787 | 0.001772 |
| has_greater_test.test | RUN | 0.205542 | 1.048214 | 1.274160 | 0.001829 | 0.001825 | 0.001592 |
| has_greater_equal_test.test | RUN | 0.204431 | 1.021840 | 1.220943 | 0.001432 | 0.001583 | 0.001670 |
| has_equal_to_test.test | RUN | 0.202589 | 0.995325 | 1.296371 | 0.001823 | 0.001647 | 0.001539 |
| has_divides_test.test | RUN | 0.207820 | 0.783628 | 1.241588 | 0.001841 | 0.001784 | 0.001437 |
| has_divides_assign_test.test | RUN | 0.211911 | 0.763030 | 1.273118 | 0.001441 | 0.001766 | 0.001548 |
| has_dereference_test.test | RUN | 0.204478 | 0.755187 | 1.176656 | 0.001551 | 0.001862 | 0.001698 |
| has_complement_test.test | RUN | 0.209310 | 0.989174 | 1.247648 | 0.001845 | 0.001853 | 0.001538 |
| has_bit_xor_test.test | RUN | 0.204149 | 0.759731 | 1.243817 | 0.001824 | 0.001754 | 0.001484 |
| has_bit_xor_assign_test.test | RUN | 0.196767 | 0.791150 | 1.245229 | 0.001813 | 0.001575 | 0.001480 |
| has_bit_or_test.test | RUN | 0.195767 | 0.788654 | 1.215642 | 0.001674 | 0.001684 | 0.001664 |
| has_bit_or_assign_test.test | RUN | 0.201785 | 0.725386 | 1.268827 | 0.001713 | 0.001746 | 0.001538 |
| has_bit_and_test.test | RUN | 0.202722 | 0.739611 | 1.253190 | 0.001902 | 0.001795 | 0.001316 |
| has_bit_and_assign_test.test | RUN | 0.205041 | 0.741521 | 1.260023 | 0.001607 | 0.001928 | 0.001638 |
| has_binary_classes9_test.test | RUN | 0.217568 | 0.904119 | 1.368844 | 0.001518 | 0.001603 | 0.001718 |
| has_binary_classes8_test.test | RUN | 0.223966 | 0.875578 | 1.356545 | 0.001842 | 0.001672 | 0.001575 |
| has_binary_classes7_test.test | RUN | 0.219438 | 0.867717 | 1.424791 | 0.001868 | 0.001821 | 0.001382 |
| has_binary_classes6_test.test | RUN | 0.213173 | 0.835135 | 1.426176 | 0.001606 | 0.001924 | 0.001656 |
| has_binary_classes5_test.test | RUN | 0.210193 | 0.868769 | 1.394330 | 0.001541 | 0.001718 | 0.001781 |
| has_binary_classes4_test.test | RUN | 0.214131 | 0.854452 | 1.369537 | 0.001697 | 0.001768 | 0.001758 |
| has_binary_classes3_test.test | RUN | 0.210683 | 0.875604 | 1.421737 | 0.001752 | 0.001757 | 0.001781 |
| has_binary_classes2_test.test | RUN | 0.212988 | 0.831858 | 1.439421 | 0.001762 | 0.001584 | 0.001708 |
| has_binary_classes1_test.test | RUN | 0.206928 | 0.860182 | 1.369187 | 0.001580 | 0.001517 | 0.001708 |
| has_binary_classes0_test.test | RUN | 0.213141 | 0.847085 | 1.353321 | 0.001538 | 0.001482 | 0.001682 |
| function_traits_test.test | RUN | 0.111279 | 0.214168 | 0.315242 | 0.001582 | 0.001303 | 0.001632 |
| floating_point_promotion_test.test | RUN | 0.105098 | 0.212322 | 0.301285 | 0.001545 | 0.001382 | 0.001738 |
| extent_test.test | RUN | 0.107566 | 0.218927 | 0.298831 | 0.001403 | 0.001172 | 0.001562 |
| enable_if_test.test | RUN | 0.100933 | 0.191600 | 0.248074 | 0.001396 | 0.001642 | 0.001577 |
| disjunction_test.test | RUN | 0.101416 | 0.194137 | 0.253311 | 0.001516 | 0.001490 | 0.001716 |
| detected_test.test | RUN | 0.104264 | 0.185891 | 0.245089 | 0.001647 | 0.001461 | 0.001688 |
| detected_or_test.test | RUN | 0.102139 | 0.193309 | 0.244973 | 0.001446 | 0.001331 | 0.001322 |
| decay_test.test | RUN | 0.227234 | 0.719655 | 1.455902 | 0.001686 | 0.001550 | 0.001484 |
| cxx14_aliases_test.test | RUN | 0.163276 | 0.284151 | 0.395011 | 0.001552 | 0.001779 | 0.001471 |
| copy_reference_test.test | RUN | 0.103322 | 0.195247 | 0.252632 | 0.001984 | 0.001661 | 0.001539 |
| copy_cv_test.test | RUN | 0.103988 | 0.199471 | 0.270286 | 0.001663 | 0.001440 | 0.001559 |
| copy_cv_ref_test.test | RUN | 0.139424 | 0.327452 | 0.591640 | 0.001507 | 0.001409 | 0.001574 |
| conjunction_test.test | RUN | 0.102066 | 0.191196 | 0.289274 | 0.001484 | 0.001566 | 0.001507 |
| conditional_test.test | RUN | 0.103256 | 0.200092 | 0.277404 | 0.001908 | 0.001454 | 0.001502 |
| common_type_test.test | RUN | 0.131188 | 0.313205 | 0.471931 | 0.001545 | 0.001350 | 0.001522 |
| common_type_sfinae_test.test | RUN | 0.102824 | 0.194418 | 0.226664 | 0.001541 | 0.001744 | 0.001649 |
| common_type_sfinae2_test.test | RUN | 0.108820 | 0.211486 | 0.242125 | 0.001523 | 0.001594 | 0.001433 |
| common_type_6_test.test | RUN | 0.109607 | 0.232210 | 0.261143 | 0.001689 | 0.001472 | 0.001521 |
| common_type_5_test.test | RUN | 0.105290 | 0.209592 | 0.252590 | 0.001507 | 0.001986 | 0.001640 |
| common_type_4_test.test | RUN | 0.109753 | 0.226931 | 0.301948 | 0.001456 | 0.001260 | 0.001631 |
| common_type_3_test.test | RUN | 0.098082 | 0.194871 | 0.230115 | 0.001443 | 0.001755 | 0.001708 |
| common_type_2_test.test | RUN | 0.129421 | 0.301246 | 0.449137 | 0.001362 | 0.001844 | 0.001305 |
| alignment_of_test.test | RUN | 0.108341 | 0.236646 | 0.302061 | 0.001390 | 0.001817 | 0.001564 |
| alignment_of_a2_test.test | RUN | 0.099904 | 0.230866 | 0.296688 | 0.001578 | 0.001574 | 0.001688 |
| aligned_storage_test.test | RUN | 0.105600 | 0.228079 | 0.273279 | 0.001548 | 0.001604 | 0.001644 |
| aligned_storage_empy_test.test | RUN | 0.117393 | 0.263944 | 0.364502 | 0.001588 | 0.001736 | 0.001409 |
| aligned_storage_a2_test.test | RUN | 0.100261 | 0.238169 | 0.272693 | 0.001554 | 0.001994 | 0.001644 |
| add_volatile_test.test | RUN | 0.191668 | 0.687617 | 1.234294 | 0.001762 | 0.001776 | 0.001629 |
| add_rvalue_reference_test.test | RUN | 0.215744 | 0.787578 | 1.431098 | 0.001677 | 0.001823 | 0.002724 |
| add_reference_test.test | RUN | 0.207440 | 0.814326 | 1.423455 | 0.001494 | 0.001865 | 0.001613 |
| add_pointer_test.test | RUN | 0.143837 | 0.454785 | 0.732814 | 0.001540 | 0.001622 | 0.001647 |
| add_lvalue_reference_test.test | RUN | 0.212894 | 0.748899 | 1.545103 | 0.001776 | 0.001728 | 0.001636 |
| add_cv_test.test | RUN | 0.202418 | 0.709835 | 1.456882 | 0.001493 | 0.001737 | 0.001339 |
| add_const_test.test | RUN | 0.204225 | 0.705398 | 1.327958 | 0.001338 | 0.001609 | 0.001624 |
| is_virtual_base_of_fail.test | COMPILE_FAIL | 0.092288 | 0.189803 | 0.225600 | — | — | — |
| is_stateless_fail.test | COMPILE_FAIL | 0.095959 | 0.193302 | 0.240874 | — | — | — |
| is_polymorphic_fail.test | COMPILE_FAIL | 0.091829 | 0.189787 | 0.220133 | — | — | — |
| is_nothrow_move_constructible_fail.test | COMPILE_FAIL | 0.091401 | 0.188183 | 0.229955 | — | — | — |
| is_nothrow_move_assignable_fail.test | COMPILE_FAIL | 0.091196 | 0.188491 | 0.225386 | — | — | — |
| is_list_constructible_fail.test | COMPILE_FAIL | 0.095049 | 0.185721 | 0.208647 | — | — | — |
| is_empty_fail.test | COMPILE_FAIL | 0.093559 | 0.188506 | 0.225104 | — | — | — |
| is_destructible_fail.test | COMPILE_FAIL | 0.092838 | 0.185836 | 0.221136 | — | — | — |
| is_default_constructible_fail.test | COMPILE_FAIL | 0.091917 | 0.189343 | 0.212180 | — | — | — |
| is_copy_constructible_fail.test | COMPILE_FAIL | 0.094997 | 0.184975 | 0.227043 | — | — | — |
| is_copy_assignable_fail.test | COMPILE_FAIL | 0.095381 | 0.184120 | 0.228997 | — | — | — |
| is_convertible_fail.test | COMPILE_FAIL | 0.092141 | 0.187517 | 0.220535 | — | — | — |
| is_convertible2_fail.test | COMPILE_FAIL | 0.092294 | 0.189914 | 0.215486 | — | — | — |
| is_constructible_fail.test | COMPILE_FAIL | 0.096267 | 0.185839 | 0.224147 | — | — | — |
| is_base_of_fail.test | COMPILE_FAIL | 0.095914 | 0.186676 | 0.219983 | — | — | — |
| is_assignable_fail.test | COMPILE_FAIL | 0.096137 | 0.181757 | 0.222888 | — | — | — |
| has_nothrow_destruct_fail.test | COMPILE_FAIL | 0.097105 | 0.184121 | 0.223216 | — | — | — |
| has_nothrow_copy_fail.test | COMPILE_FAIL | 0.098308 | 0.187569 | 0.233344 | — | — | — |
| has_nothrow_cons_fail.test | COMPILE_FAIL | 0.095334 | 0.184915 | 0.227590 | — | — | — |
| has_nothrow_assign_fail.test | COMPILE_FAIL | 0.094731 | 0.179902 | 0.224980 | — | — | — |
| common_type_fail.test | COMPILE_FAIL | 0.095628 | 0.180305 | 0.230315 | — | — | — |
| common_type2_fail.test | COMPILE_FAIL | 0.097177 | 0.183708 | 0.233344 | — | — | — |
| has_nothrow_assign_test_no_intrinsics.test | RUN | 0.145076 | 0.377563 | 0.613086 | 0.001536 | 0.001923 | 0.001496 |
| has_nothrow_constr_test_no_intrinsics.test | RUN | 0.136645 | 0.404186 | 0.607869 | 0.001320 | 0.001918 | 0.001317 |
| has_nothrow_copy_test_no_intrinsics.test | RUN | 0.138518 | 0.398192 | 0.592312 | 0.001433 | 0.001270 | 0.001678 |
| is_nothrow_move_assignable_test_no_intrinsics.test | RUN | 0.142641 | 0.389534 | 0.564437 | 0.001256 | 0.001321 | 0.001296 |
| is_nothrow_move_constructible_test_no_intrinsics.test | RUN | 0.093351 | 0.333129 | 0.353783 | 0.001317 | 0.001210 | 0.001061 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.017× | 1.021×, 1.012× | 0.986× |
| clang | 1.035× | 1.033×, 1.038× | 1.014× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| add_const_test.test | 10 | 0.000941s | 0.000922s | 0.000948s | byte equality observed |
| add_cv_test.test | 10 | 0.000935s | 0.000936s | 0.000964s | byte equality observed |
| add_lvalue_reference_test.test | 10 | 0.000942s | 0.000948s | 0.000974s | byte equality observed |
| add_pointer_test.test | 10 | 0.000935s | 0.000956s | 0.000960s | byte equality observed |
| add_reference_test.test | 10 | 0.000934s | 0.000937s | 0.000985s | byte equality observed |
| add_rvalue_reference_test.test | 10 | 0.000941s | 0.000959s | 0.000962s | byte equality observed |
| add_volatile_test.test | 10 | 0.000937s | 0.000921s | 0.000960s | byte equality observed |
| aligned_storage_a2_test.test | 10 | 0.001030s | 0.001035s | 0.001056s | byte equality observed |
| aligned_storage_empy_test.test | 10 | 0.000928s | 0.000952s | 0.000947s | byte equality observed |
| aligned_storage_test.test | 10 | 0.001013s | 0.001041s | 0.001045s | byte equality observed |
| alignment_of_a2_test.test | 10 | 0.000934s | 0.000947s | 0.000968s | byte equality observed |
| alignment_of_test.test | 10 | 0.000940s | 0.000969s | 0.000963s | byte equality observed |
| common_type_2_test.test | 10 | 0.000935s | 0.000946s | 0.000942s | byte equality observed |
| common_type_3_test.test | 10 | 0.000933s | 0.000941s | 0.000954s | byte equality observed |
| common_type_4_test.test | 10 | 0.000921s | 0.000970s | 0.000946s | byte equality observed |
| common_type_5_test.test | 10 | 0.000929s | 0.000938s | 0.000945s | byte equality observed |
| common_type_6_test.test | 10 | 0.000921s | 0.000947s | 0.000956s | byte equality observed |
| common_type_sfinae2_test.test | 10 | 0.000917s | 0.000952s | 0.000960s | byte equality observed |
| common_type_sfinae_test.test | 10 | 0.000948s | 0.000950s | 0.000938s | byte equality observed |
| common_type_test.test | 10 | 0.000931s | 0.000923s | 0.000958s | byte equality observed |
| conditional_test.test | 10 | 0.000948s | 0.000967s | 0.000952s | byte equality observed |
| conjunction_test.test | 10 | 0.000922s | 0.000937s | 0.000935s | byte equality observed |
| copy_cv_ref_test.test | 10 | 0.000944s | 0.000956s | 0.000945s | byte equality observed |
| copy_cv_test.test | 10 | 0.000933s | 0.000948s | 0.000940s | byte equality observed |
| copy_reference_test.test | 10 | 0.000955s | 0.000952s | 0.000948s | byte equality observed |
| cxx14_aliases_test.test | 10 | 0.000944s | 0.000949s | 0.000956s | byte equality observed |
| decay_test.test | 10 | 0.000946s | 0.000953s | 0.000958s | byte equality observed |
| detected_or_test.test | 10 | 0.000940s | 0.000955s | 0.000940s | byte equality observed |
| detected_test.test | 10 | 0.000918s | 0.000950s | 0.000962s | byte equality observed |
| disjunction_test.test | 10 | 0.000930s | 0.000924s | 0.000933s | byte equality observed |
| enable_if_test.test | 10 | 0.000949s | 0.000945s | 0.000950s | byte equality observed |
| extent_test.test | 10 | 0.000922s | 0.000934s | 0.000946s | byte equality observed |
| floating_point_promotion_test.test | 10 | 0.000945s | 0.000941s | 0.000964s | byte equality observed |
| function_traits_test.test | 10 | 0.000930s | 0.000943s | 0.000949s | byte equality observed |
| has_binary_classes0_test.test | 10 | 0.000938s | 0.000922s | 0.000955s | byte equality observed |
| has_binary_classes1_test.test | 10 | 0.000931s | 0.000943s | 0.000959s | byte equality observed |
| has_binary_classes2_test.test | 10 | 0.000939s | 0.000944s | 0.000937s | byte equality observed |
| has_binary_classes3_test.test | 10 | 0.000932s | 0.000947s | 0.000938s | byte equality observed |
| has_binary_classes4_test.test | 10 | 0.000949s | 0.000975s | 0.000949s | byte equality observed |
| has_binary_classes5_test.test | 10 | 0.000931s | 0.000925s | 0.000945s | byte equality observed |
| has_binary_classes6_test.test | 10 | 0.000947s | 0.000955s | 0.000931s | byte equality observed |
| has_binary_classes7_test.test | 10 | 0.000932s | 0.000937s | 0.000958s | byte equality observed |
| has_binary_classes8_test.test | 10 | 0.000932s | 0.000939s | 0.000931s | byte equality observed |
| has_binary_classes9_test.test | 10 | 0.000932s | 0.000939s | 0.000929s | byte equality observed |
| has_bit_and_assign_test.test | 10 | 0.000932s | 0.000967s | 0.000942s | byte equality observed |
| has_bit_and_test.test | 10 | 0.000950s | 0.000950s | 0.000925s | byte equality observed |
| has_bit_or_assign_test.test | 10 | 0.000934s | 0.000940s | 0.000953s | byte equality observed |
| has_bit_or_test.test | 10 | 0.000927s | 0.000965s | 0.000938s | byte equality observed |
| has_bit_xor_assign_test.test | 10 | 0.000936s | 0.000961s | 0.000940s | byte equality observed |
| has_bit_xor_test.test | 10 | 0.000941s | 0.000945s | 0.000942s | byte equality observed |
| has_complement_test.test | 10 | 0.000947s | 0.000957s | 0.000947s | byte equality observed |
| has_dereference_test.test | 10 | 0.000925s | 0.001050s | 0.000916s | byte equality observed |
| has_divides_assign_test.test | 10 | 0.000950s | 0.001099s | 0.000955s | byte equality observed |
| has_divides_test.test | 10 | 0.000939s | 0.000970s | 0.000945s | byte equality observed |
| has_equal_to_test.test | 10 | 0.000918s | 0.000931s | 0.000950s | byte equality observed |
| has_greater_equal_test.test | 10 | 0.000941s | 0.000980s | 0.000941s | byte equality observed |
| has_greater_test.test | 10 | 0.000937s | 0.000943s | 0.000931s | byte equality observed |
| has_left_shift_assign_test.test | 10 | 0.000939s | 0.000957s | 0.000945s | byte equality observed |
| has_left_shift_test.test | 10 | 0.000945s | 0.000949s | 0.000958s | byte equality observed |
| has_less_equal_test.test | 10 | 0.000935s | 0.000942s | 0.000950s | byte equality observed |
| has_less_test.test | 10 | 0.000964s | 0.000956s | 0.000928s | byte equality observed |
| has_logical_and_test.test | 10 | 0.000928s | 0.000948s | 0.000958s | byte equality observed |
| has_logical_not_test.test | 10 | 0.000953s | 0.000937s | 0.000915s | byte equality observed |
| has_logical_or_test.test | 10 | 0.000933s | 0.000934s | 0.000940s | byte equality observed |
| has_minus_assign_test.test | 10 | 0.000939s | 0.000949s | 0.000934s | byte equality observed |
| has_minus_test.test | 10 | 0.000942s | 0.000932s | 0.000957s | byte equality observed |
| has_modulus_assign_test.test | 10 | 0.000932s | 0.000938s | 0.000921s | byte equality observed |
| has_modulus_test.test | 10 | 0.000950s | 0.000954s | 0.000962s | byte equality observed |
| has_multiplies_assign_test.test | 10 | 0.000923s | 0.000953s | 0.000948s | byte equality observed |
| has_multiplies_test.test | 10 | 0.000933s | 0.000961s | 0.000940s | byte equality observed |
| has_negate_test.test | 10 | 0.000937s | 0.000943s | 0.000958s | byte equality observed |
| has_not_equal_to_test.test | 10 | 0.000928s | 0.000949s | 0.000959s | byte equality observed |
| has_nothrow_assign_test.test | 10 | 0.000934s | 0.000946s | 0.000930s | byte equality observed |
| has_nothrow_assign_test_no_intrinsics.test | 10 | 0.000932s | 0.000948s | 0.000947s | byte equality observed |
| has_nothrow_constr_test.test | 10 | 0.000944s | 0.000953s | 0.000944s | byte equality observed |
| has_nothrow_constr_test_no_intrinsics.test | 10 | 0.000945s | 0.000930s | 0.000944s | byte equality observed |
| has_nothrow_copy_test.test | 10 | 0.000946s | 0.000959s | 0.000947s | byte equality observed |
| has_nothrow_copy_test_no_intrinsics.test | 10 | 0.000929s | 0.000932s | 0.000928s | byte equality observed |
| has_nothrow_destructor_test.test | 10 | 0.000928s | 0.000959s | 0.000944s | byte equality observed |
| has_operator_new_test.test | 10 | 0.000931s | 0.000943s | 0.000941s | byte equality observed |
| has_plus_assign_test.test | 10 | 0.000924s | 0.000968s | 0.000953s | byte equality observed |
| has_plus_test.test | 10 | 0.000943s | 0.000939s | 0.000933s | byte equality observed |
| has_post_decrement_test.test | 10 | 0.000943s | 0.000952s | 0.000956s | byte equality observed |
| has_post_increment_test.test | 10 | 0.000940s | 0.000942s | 0.000930s | byte equality observed |
| has_postfix_classes0_test.test | 10 | 0.000939s | 0.000952s | 0.000931s | byte equality observed |
| has_postfix_classes1_test.test | 10 | 0.000933s | 0.000947s | 0.000933s | byte equality observed |
| has_postfix_classes2_test.test | 10 | 0.000938s | 0.000937s | 0.000930s | byte equality observed |
| has_postfix_classes3_test.test | 10 | 0.000922s | 0.000936s | 0.000930s | byte equality observed |
| has_pre_decrement_test.test | 10 | 0.000947s | 0.000943s | 0.000953s | byte equality observed |
| has_pre_increment_test.test | 10 | 0.000923s | 0.000943s | 0.000927s | byte equality observed |
| has_prefix_classes0_test.test | 10 | 0.000935s | 0.000949s | 0.000933s | byte equality observed |
| has_prefix_classes1_test.test | 10 | 0.000929s | 0.000956s | 0.000937s | byte equality observed |
| has_prefix_classes2_test.test | 10 | 0.000950s | 0.000953s | 0.000929s | byte equality observed |
| has_prefix_classes3_test.test | 10 | 0.000934s | 0.000932s | 0.000924s | byte equality observed |
| has_right_shift_assign_test.test | 10 | 0.000929s | 0.000959s | 0.000939s | byte equality observed |
| has_right_shift_test.test | 10 | 0.000947s | 0.000916s | 0.000949s | byte equality observed |
| has_trivial_assign_test.test | 10 | 0.000991s | 0.000997s | 0.000995s | byte equality observed |
| has_trivial_constr_test.test | 10 | 0.000988s | 0.000982s | 0.000978s | native test oracle; output varies |
| has_trivial_copy_test.test | 10 | 0.000978s | 0.001001s | 0.001014s | native test oracle; output varies |
| has_trivial_destructor_test.test | 10 | 0.000931s | 0.000947s | 0.000958s | byte equality observed |
| has_trivial_move_assign_test.test | 10 | 0.000934s | 0.000940s | 0.000929s | byte equality observed |
| has_trivial_move_constructor_test.test | 10 | 0.000929s | 0.000941s | 0.000954s | byte equality observed |
| has_unary_minus_test.test | 10 | 0.000933s | 0.000926s | 0.000934s | byte equality observed |
| has_unary_plus_test.test | 10 | 0.000935s | 0.000965s | 0.000932s | byte equality observed |
| has_virtual_destructor_test.test | 10 | 0.000955s | 0.000945s | 0.000949s | byte equality observed |
| integral_constant_test.test | 10 | 0.000442s | 0.000453s | 0.000953s | byte equality observed |
| is_abstract_test.test | 10 | 0.000964s | 0.000969s | 0.000950s | byte equality observed |
| is_arithmetic_test.test | 9 | 0.000942s | 0.000940s | 0.000938s | byte equality observed |
| is_array_test.test | 10 | 0.000937s | 0.000931s | 0.000942s | byte equality observed |
| is_assignable_test.test | 10 | 0.000972s | 0.000969s | 0.000935s | byte equality observed |
| is_base_and_derived_test.test | 10 | 0.000951s | 0.000930s | 0.000960s | byte equality observed |
| is_base_of_test.test | 10 | 0.000942s | 0.000966s | 0.000968s | byte equality observed |
| is_bounded_array_test.test | 10 | 0.000956s | 0.000955s | 0.000931s | byte equality observed |
| is_class_test.test | 10 | 0.000935s | 0.000946s | 0.000961s | byte equality observed |
| is_complete_test.test | 10 | 0.000961s | 0.000953s | 0.000945s | byte equality observed |
| is_complex_test.test | 10 | 0.000952s | 0.000925s | 0.000940s | byte equality observed |
| is_compound_test.test | 10 | 0.000937s | 0.000939s | 0.000953s | byte equality observed |
| is_const_test.test | 10 | 0.000962s | 0.000961s | 0.000938s | byte equality observed |
| is_constructible_test.test | 10 | 0.000949s | 0.000938s | 0.000951s | byte equality observed |
| is_convertible_test.test | 10 | 0.000951s | 0.000951s | 0.000951s | byte equality observed |
| is_copy_assignable_test.test | 10 | 0.000932s | 0.000938s | 0.000955s | byte equality observed |
| is_copy_constructible_test.test | 10 | 0.000974s | 0.000958s | 0.000926s | byte equality observed |
| is_default_constr_test.test | 10 | 0.000951s | 0.000930s | 0.000944s | byte equality observed |
| is_destructible_test.test | 10 | 0.000950s | 0.000952s | 0.000970s | byte equality observed |
| is_detected_exact_test.test | 10 | 0.000942s | 0.000956s | 0.000950s | byte equality observed |
| is_detected_test.test | 10 | 0.000948s | 0.000946s | 0.000955s | byte equality observed |
| is_empty_test.test | 10 | 0.000927s | 0.000956s | 0.000954s | byte equality observed |
| is_enum_test.test | 10 | 0.000934s | 0.000931s | 0.000940s | byte equality observed |
| is_final_test.test | 10 | 0.000933s | 0.000943s | 0.000936s | byte equality observed |
| is_float_test.test | 10 | 0.000929s | 0.000953s | 0.000923s | byte equality observed |
| is_floating_point_test.test | 10 | 0.000935s | 0.000969s | 0.000933s | byte equality observed |
| is_function_test.test | 10 | 0.000933s | 0.000959s | 0.000944s | byte equality observed |
| is_fundamental_test.test | 10 | 0.000939s | 0.000968s | 0.000926s | byte equality observed |
| is_integral_test.test | 10 | 0.000955s | 0.000956s | 0.000944s | byte equality observed |
| is_list_constructible_test.test | 10 | 0.000958s | 0.000934s | 0.000936s | byte equality observed |
| is_lvalue_reference_test.test | 10 | 0.000954s | 0.000945s | 0.000934s | byte equality observed |
| is_member_func_test.test | 10 | 0.000954s | 0.000942s | 0.000942s | byte equality observed |
| is_member_obj_test.test | 10 | 0.000967s | 0.000945s | 0.000930s | byte equality observed |
| is_member_pointer_test.test | 10 | 0.000937s | 0.000948s | 0.000949s | byte equality observed |
| is_noncopyable_test.test | 10 | 0.000958s | 0.000973s | 0.000939s | byte equality observed |
| is_nothrow_move_assignable_test.test | 10 | 0.000950s | 0.000941s | 0.000934s | byte equality observed |
| is_nothrow_move_assignable_test_no_intrinsics.test | 10 | 0.000937s | 0.000953s | 0.000912s | byte equality observed |
| is_nothrow_move_constructible_test.test | 10 | 0.000954s | 0.000944s | 0.000949s | byte equality observed |
| is_nothrow_move_constructible_test_no_intrinsics.test | 10 | 0.000940s | 0.000942s | 0.000929s | byte equality observed |
| is_nothrow_swappable_test.test | 10 | 0.000942s | 0.000939s | 0.000930s | byte equality observed |
| is_object_test.test | 10 | 0.000936s | 0.000955s | 0.000945s | byte equality observed |
| is_pod_test.test | 10 | 0.000948s | 0.000944s | 0.000943s | byte equality observed |
| is_pointer_test.test | 10 | 0.000969s | 0.000957s | 0.000959s | byte equality observed |
| is_polymorphic_test.test | 10 | 0.000948s | 0.000952s | 0.000966s | byte equality observed |
| is_reference_test.test | 10 | 0.000947s | 0.000949s | 0.000936s | byte equality observed |
| is_rvalue_reference_test.test | 10 | 0.000970s | 0.000958s | 0.000938s | byte equality observed |
| is_same_test.test | 10 | 0.000958s | 0.000952s | 0.000928s | byte equality observed |
| is_scalar_test.test | 10 | 0.000935s | 0.000947s | 0.000950s | byte equality observed |
| is_scoped_enum_test.test | 10 | 0.000959s | 0.000931s | 0.000943s | byte equality observed |
| is_signed_test.test | 10 | 0.000957s | 0.000945s | 0.000958s | byte equality observed |
| is_stateless_test.test | 10 | 0.000928s | 0.000956s | 0.000961s | byte equality observed |
| is_swappable_test.test | 10 | 0.000945s | 0.000953s | 0.000932s | byte equality observed |
| is_trivially_copyable_test.test | 10 | 0.000969s | 0.000977s | 0.000963s | byte equality observed |
| is_unbounded_array_test.test | 10 | 0.000938s | 0.000957s | 0.000942s | byte equality observed |
| is_union_test.test | 10 | 0.000935s | 0.000949s | 0.000955s | byte equality observed |
| is_unscoped_enum_test.test | 10 | 0.000935s | 0.000955s | 0.000941s | byte equality observed |
| is_unsigned_test.test | 10 | 0.000932s | 0.000946s | 0.000952s | byte equality observed |
| is_virtual_base_of_test.test | 10 | 0.000932s | 0.000956s | 0.000930s | byte equality observed |
| is_void_test.test | 10 | 0.000938s | 0.000965s | 0.000945s | byte equality observed |
| is_volatile_test.test | 10 | 0.000944s | 0.000945s | 0.000946s | byte equality observed |
| make_signed_test.test | 10 | 0.000929s | 0.000946s | 0.000934s | byte equality observed |
| make_unsigned_test.test | 10 | 0.000947s | 0.000962s | 0.000960s | byte equality observed |
| make_void_test.test | 10 | 0.000939s | 0.000945s | 0.000961s | byte equality observed |
| mpl_interop_test1.test | 10 | 0.000450s | 0.000467s | 0.000944s | byte equality observed |
| mpl_interop_test2.test | 10 | 0.000461s | 0.000460s | 0.000949s | byte equality observed |
| mpl_interop_test3.test | 10 | 0.000453s | 0.000452s | 0.000939s | byte equality observed |
| negation_test.test | 10 | 0.000981s | 0.001023s | 0.000951s | byte equality observed |
| promote_basic_test.test | 10 | 0.000445s | 0.000452s | 0.000943s | byte equality observed |
| promote_enum_msvc_bug_test.test | 10 | 0.000454s | 0.000456s | 0.000979s | byte equality observed |
| promote_enum_test.test | 10 | 0.000455s | 0.000459s | 0.000944s | byte equality observed |
| promote_mpl_test.test | 10 | 0.000440s | 0.000458s | 0.000972s | byte equality observed |
| rank_test.test | 10 | 0.000979s | 0.001033s | 0.000970s | byte equality observed |
| remove_all_extents_test.test | 10 | 0.000949s | 0.000950s | 0.000974s | byte equality observed |
| remove_bounds_test.test | 10 | 0.000944s | 0.000966s | 0.000947s | byte equality observed |
| remove_const_test.test | 10 | 0.000941s | 0.001004s | 0.000960s | byte equality observed |
| remove_cv_ref_test.test | 10 | 0.000938s | 0.000954s | 0.000934s | byte equality observed |
| remove_cv_test.test | 10 | 0.000936s | 0.000991s | 0.000954s | byte equality observed |
| remove_extent_test.test | 10 | 0.000962s | 0.000978s | 0.000955s | byte equality observed |
| remove_pointer_test.test | 10 | 0.000952s | 0.000954s | 0.001004s | byte equality observed |
| remove_reference_test.test | 10 | 0.000946s | 0.000968s | 0.000989s | byte equality observed |
| remove_volatile_test.test | 10 | 0.000944s | 0.000987s | 0.000975s | byte equality observed |
| tricky_abstract_type_test.test | 10 | 0.000945s | 0.000941s | 0.000981s | byte equality observed |
| tricky_add_pointer_test.test | 10 | 0.000950s | 0.000977s | 0.001024s | byte equality observed |
| tricky_function_type_test.test | 10 | 0.000939s | 0.000945s | 0.001006s | byte equality observed |
| tricky_incomplete_type_test.test | 10 | 0.000954s | 0.000947s | 0.001007s | byte equality observed |
| tricky_is_enum_test.test | 10 | 0.000949s | 0.000956s | 0.001007s | byte equality observed |
| tricky_partial_spec_test.test | 10 | 0.000944s | 0.000981s | 0.001004s | byte equality observed |
| tricky_rvalue_test.test | 10 | 0.000935s | 0.000979s | 0.000990s | byte equality observed |
| type_identity_test.test | 10 | 0.000963s | 0.000969s | 0.000954s | byte equality observed |
| type_traits_test.test | 10 | 0.000452s | 0.000447s | 0.000958s | byte equality observed |
| type_with_alignment_test.test | 10 | 0.001032s | 0.001070s | 0.001059s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
