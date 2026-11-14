# libs/type_traits/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| type_with_alignment_test.test | RUN | 0.113996 | 0.324216 | 0.517364 | 0.001665 | 0.001820 | 0.001768 |
| type_traits_test.test | RUN | 0.158704 | 0.336437 | 0.503536 | 0.000620 | 0.000572 | 0.001534 |
| type_identity_test.test | RUN | 0.302148 | 1.170138 | 2.943963 | 0.001583 | 0.001830 | 0.001718 |
| tricky_rvalue_test.test | RUN | 0.105004 | 0.258229 | 0.404640 | 0.001526 | 0.001654 | 0.001614 |
| tricky_partial_spec_test.test | RUN | 0.128502 | 0.381057 | 0.624871 | 0.001565 | 0.001506 | 0.001601 |
| tricky_is_enum_test.test | RUN | 0.102277 | 0.252595 | 0.368913 | 0.001658 | 0.001574 | 0.001543 |
| tricky_incomplete_type_test.test | RUN | 0.171072 | 0.386167 | 0.634760 | 0.001617 | 0.001574 | 0.001794 |
| tricky_function_type_test.test | RUN | 0.138723 | 0.406030 | 0.711867 | 0.001688 | 0.001680 | 0.001604 |
| tricky_add_pointer_test.test | RUN | 0.201287 | 0.693271 | 1.682354 | 0.001516 | 0.001832 | 0.001716 |
| tricky_abstract_type_test.test | RUN | 0.109157 | 0.247477 | 0.388862 | 0.001612 | 0.001787 | 0.001722 |
| remove_volatile_test.test | RUN | 0.267980 | 0.999295 | 2.616888 | 0.001654 | 0.001903 | 0.001661 |
| remove_reference_test.test | RUN | 0.282979 | 1.056426 | 2.714301 | 0.001722 | 0.001624 | 0.001816 |
| remove_pointer_test.test | RUN | 0.290587 | 1.081790 | 2.912488 | 0.001674 | 0.001660 | 0.001747 |
| remove_extent_test.test | RUN | 0.289688 | 1.079302 | 2.806211 | 0.001684 | 0.001838 | 0.001915 |
| remove_cv_test.test | RUN | 0.283658 | 1.075075 | 2.782721 | 0.001736 | 0.001866 | 0.001867 |
| remove_cv_ref_test.test | RUN | 0.584560 | 2.526026 | 6.671069 | 0.001489 | 0.001550 | 0.001715 |
| remove_const_test.test | RUN | 0.276328 | 0.996882 | 2.632910 | 0.001507 | 0.001780 | 0.001803 |
| remove_bounds_test.test | RUN | 0.287424 | 1.081938 | 2.785017 | 0.001624 | 0.001828 | 0.001878 |
| remove_all_extents_test.test | RUN | 0.284481 | 1.058161 | 2.694541 | 0.001455 | 0.001664 | 0.001382 |
| rank_test.test | RUN | 0.105481 | 0.262475 | 0.414161 | 0.001608 | 0.001600 | 0.001802 |
| promote_mpl_test.test | RUN | 0.096983 | 0.231866 | 0.291741 | 0.000599 | 0.000657 | 0.001656 |
| promote_enum_test.test | RUN | 0.022096 | 0.057059 | 0.074899 | 0.000572 | 0.000816 | 0.001742 |
| promote_enum_msvc_bug_test.test | RUN | 0.020631 | 0.052525 | 0.063167 | 0.000555 | 0.000597 | 0.002028 |
| promote_basic_test.test | RUN | 0.024707 | 0.064539 | 0.080289 | 0.000659 | 0.000750 | 0.001890 |
| negation_test.test | RUN | 0.100923 | 0.235806 | 0.381567 | 0.001536 | 0.001511 | 0.001730 |
| mpl_interop_test3.test | RUN | 0.088603 | 0.225639 | 0.265141 | 0.000605 | 0.000571 | 0.001435 |
| mpl_interop_test2.test | RUN | 0.032418 | 0.062982 | 0.069845 | 0.000710 | 0.000527 | 0.001790 |
| mpl_interop_test1.test | RUN | 0.017083 | 0.046181 | 0.053103 | 0.000558 | 0.000797 | 0.001603 |
| make_void_test.test | RUN | 0.107418 | 0.259778 | 0.376103 | 0.001641 | 0.001882 | 0.001690 |
| make_unsigned_test.test | RUN | 0.127129 | 0.337230 | 0.590504 | 0.001500 | 0.001792 | 0.003394 |
| make_signed_test.test | RUN | 0.126371 | 0.339887 | 0.551924 | 0.001540 | 0.001491 | 0.003224 |
| is_volatile_test.test | RUN | 0.111303 | 0.291283 | 0.499133 | 0.001334 | 0.001738 | 0.001939 |
| is_void_test.test | RUN | 0.107832 | 0.264824 | 0.402348 | 0.001715 | 0.001571 | 0.001908 |
| is_virtual_base_of_test.test | RUN | 0.127888 | 0.348710 | 0.603334 | 0.001569 | 0.001437 | 0.001890 |
| is_unsigned_test.test | RUN | 0.112726 | 0.288956 | 0.463175 | 0.001586 | 0.001586 | 0.002079 |
| is_unscoped_enum_test.test | RUN | 0.104044 | 0.251258 | 0.358670 | 0.001311 | 0.001800 | 0.001766 |
| is_union_test.test | RUN | 0.117679 | 0.316538 | 0.528619 | 0.001392 | 0.001740 | 0.001432 |
| is_unbounded_array_test.test | RUN | 0.108705 | 0.273760 | 0.417368 | 0.001597 | 0.001670 | 0.001689 |
| is_trivially_copyable_test.test | RUN | 0.172313 | 0.566134 | 1.084657 | 0.001555 | 0.001712 | 0.001713 |
| is_swappable_test.test | RUN | 0.141660 | 0.424676 | 0.826618 | 0.001510 | 0.001683 | 0.001744 |
| is_stateless_test.test | RUN | 0.161591 | 0.490333 | 0.907783 | 0.001564 | 0.001678 | 0.001733 |
| is_signed_test.test | RUN | 0.111616 | 0.275409 | 0.463073 | 0.001452 | 0.001715 | 0.001940 |
| is_scoped_enum_test.test | RUN | 0.109649 | 0.252479 | 0.354055 | 0.001482 | 0.001718 | 0.001511 |
| is_scalar_test.test | RUN | 0.140897 | 0.463392 | 0.841293 | 0.001262 | 0.001650 | 0.001493 |
| is_same_test.test | RUN | 0.107884 | 0.273725 | 0.404288 | 0.001673 | 0.001502 | 0.001791 |
| is_rvalue_reference_test.test | RUN | 0.110560 | 0.303083 | 0.451945 | 0.001474 | 0.001699 | 0.001785 |
| is_reference_test.test | RUN | 0.112723 | 0.311556 | 0.507044 | 0.001503 | 0.001871 | 0.001502 |
| is_polymorphic_test.test | RUN | 0.122214 | 0.316961 | 0.515965 | 0.001377 | 0.001665 | 0.001884 |
| is_pointer_test.test | RUN | 0.114158 | 0.307635 | 0.488322 | 0.001716 | 0.001658 | 0.001627 |
| is_pod_test.test | RUN | 0.150234 | 0.514775 | 0.931050 | 0.001522 | 0.001846 | 0.001818 |
| is_object_test.test | RUN | 0.106280 | 0.255014 | 0.410028 | 0.001207 | 0.001696 | 0.001570 |
| is_nothrow_swappable_test.test | RUN | 0.156491 | 0.464331 | 0.881129 | 0.001743 | 0.001764 | 0.001787 |
| is_nothrow_move_constructible_test.test | RUN | 0.150740 | 0.481420 | 0.875695 | 0.001610 | 0.001461 | 0.001689 |
| is_nothrow_move_assignable_test.test | RUN | 0.150971 | 0.493953 | 0.915573 | 0.001518 | 0.001810 | 0.001485 |
| is_noncopyable_test.test | RUN | 0.107650 | 0.259277 | 0.424862 | 0.001354 | 0.001807 | 0.001632 |
| is_member_pointer_test.test | RUN | 0.112323 | 0.264323 | 0.383959 | 0.001633 | 0.001938 | 0.001954 |
| is_member_obj_test.test | RUN | 0.106781 | 0.267986 | 0.442851 | 0.001752 | 0.001631 | 0.001743 |
| is_member_func_test.test | RUN | 0.108192 | 0.289022 | 0.479926 | 0.001460 | 0.001793 | 0.001727 |
| is_lvalue_reference_test.test | RUN | 0.113865 | 0.294908 | 0.469926 | 0.001518 | 0.001834 | 0.001774 |
| is_list_constructible_test.test | RUN | 0.133134 | 0.343936 | 0.544832 | 0.001340 | 0.001836 | 0.001875 |
| is_integral_test.test | RUN | 0.133744 | 0.420573 | 0.749290 | 0.001544 | 0.001662 | 0.001639 |
| is_fundamental_test.test | RUN | 0.135116 | 0.433315 | 0.783375 | 0.001486 | 0.001654 | 0.001695 |
| is_function_test.test | RUN | 0.113551 | 0.273923 | 0.455736 | 0.001499 | 0.001722 | 0.001810 |
| is_floating_point_test.test | RUN | 0.118111 | 0.302734 | 0.503371 | 0.001372 | 0.001774 | 0.001450 |
| is_float_test.test | RUN | 0.112877 | 0.299796 | 0.484591 | 0.001516 | 0.001502 | 0.001828 |
| is_final_test.test | RUN | 0.114988 | 0.302608 | 0.498105 | 0.001472 | 0.001537 | 0.001856 |
| is_enum_test.test | RUN | 0.107738 | 0.258879 | 0.402784 | 0.001465 | 0.001780 | 0.001639 |
| is_empty_test.test | RUN | 0.116350 | 0.282939 | 0.460891 | 0.001426 | 0.001900 | 0.001732 |
| is_detected_test.test | RUN | 0.101826 | 0.243310 | 0.339848 | 0.001263 | 0.001682 | 0.001814 |
| is_detected_exact_test.test | RUN | 0.103199 | 0.245539 | 0.327113 | 0.001600 | 0.001717 | 0.001780 |
| is_destructible_test.test | RUN | 0.154631 | 0.474024 | 0.941202 | 0.001688 | 0.001868 | 0.001687 |
| is_default_constr_test.test | RUN | 0.151304 | 0.499989 | 0.963914 | 0.001727 | 0.001729 | 0.001774 |
| is_copy_constructible_test.test | RUN | 0.153508 | 0.515131 | 1.015851 | 0.001308 | 0.001371 | 0.001848 |
| is_copy_assignable_test.test | RUN | 0.154986 | 0.508230 | 0.997951 | 0.001681 | 0.001790 | 0.001858 |
| is_convertible_test.test | RUN | 0.147040 | 0.460305 | 0.806313 | 0.001772 | 0.001856 | 0.001493 |
| is_constructible_test.test | RUN | 0.125503 | 0.328612 | 0.586898 | 0.001568 | 0.001660 | 0.001630 |
| is_const_test.test | RUN | 0.110302 | 0.286210 | 0.433175 | 0.001593 | 0.001517 | 0.001456 |
| is_compound_test.test | RUN | 0.104682 | 0.253302 | 0.372117 | 0.001601 | 0.001650 | 0.001890 |
| is_complex_test.test | RUN | 0.140589 | 0.338638 | 0.520127 | 0.001659 | 0.001670 | 0.001611 |
| is_complete_test.test | RUN | 0.125682 | 0.340963 | 0.568475 | 0.001495 | 0.001709 | 0.001604 |
| is_class_test.test | RUN | 0.112157 | 0.305494 | 0.504667 | 0.001596 | 0.001526 | 0.001323 |
| is_bounded_array_test.test | RUN | 0.109725 | 0.280341 | 0.418234 | 0.001302 | 0.001706 | 0.001536 |
| is_base_of_test.test | RUN | 0.113586 | 0.290921 | 0.488326 | 0.001521 | 0.001700 | 0.001739 |
| is_base_and_derived_test.test | RUN | 0.110259 | 0.294295 | 0.483756 | 0.001506 | 0.001536 | 0.001423 |
| is_assignable_test.test | RUN | 0.137355 | 0.418218 | 0.733870 | 0.001441 | 0.001778 | 0.001706 |
| is_array_test.test | RUN | 0.109979 | 0.274203 | 0.480755 | 0.001500 | 0.001911 | 0.001988 |
| is_arithmetic_test.test | RUN | 0.136098 | 0.420772 | 0.809898 | 0.001427 | 0.001861 | 0.001821 |
| is_abstract_test.test | RUN | 0.246782 | 1.154767 | 2.579734 | 0.001674 | 0.001850 | 0.001816 |
| integral_constant_test.test | RUN | 0.016765 | 0.048543 | 0.048503 | 0.000745 | 0.000831 | 0.001838 |
| has_virtual_destructor_test.test | RUN | 0.117258 | 0.325934 | 0.562099 | 0.001784 | 0.001826 | 0.001824 |
| has_unary_plus_test.test | RUN | 0.232028 | 1.330635 | 1.973613 | 0.001668 | 0.001855 | 0.001732 |
| has_unary_minus_test.test | RUN | 0.228317 | 1.278574 | 1.950151 | 0.001679 | 0.001648 | 0.001838 |
| has_trivial_move_constructor_test.test | RUN | 0.160678 | 0.463738 | 0.979027 | 0.001386 | 0.001544 | 0.001742 |
| has_trivial_move_assign_test.test | RUN | 0.155187 | 0.480269 | 0.890490 | 0.001683 | 0.001486 | 0.001825 |
| has_trivial_destructor_test.test | RUN | 0.153766 | 0.497732 | 0.910275 | 0.001719 | 0.001494 | 0.001806 |
| has_trivial_copy_test.test | RUN | 0.158467 | 0.536460 | 1.006146 | 0.001735 | 0.001598 | 0.001982 |
| has_trivial_constr_test.test | RUN | 0.159694 | 0.531973 | 1.001016 | 0.001403 | 0.001621 | 0.001912 |
| has_trivial_assign_test.test | RUN | 0.154918 | 0.532056 | 0.952340 | 0.001698 | 0.001636 | 0.001790 |
| has_right_shift_test.test | RUN | 0.239384 | 0.960746 | 2.186072 | 0.001559 | 0.001838 | 0.001695 |
| has_right_shift_assign_test.test | RUN | 0.229912 | 0.893906 | 2.003967 | 0.001538 | 0.001799 | 0.001703 |
| has_prefix_classes3_test.test | RUN | 0.171771 | 0.529967 | 1.013437 | 0.001693 | 0.001722 | 0.001756 |
| has_prefix_classes2_test.test | RUN | 0.171103 | 0.531769 | 1.003776 | 0.001544 | 0.001635 | 0.001774 |
| has_prefix_classes1_test.test | RUN | 0.170411 | 0.510937 | 1.021914 | 0.001551 | 0.001839 | 0.001850 |
| has_prefix_classes0_test.test | RUN | 0.166114 | 0.554136 | 1.021793 | 0.001287 | 0.001584 | 0.001746 |
| has_pre_increment_test.test | RUN | 0.226272 | 1.252208 | 2.030448 | 0.001550 | 0.001520 | 0.001952 |
| has_pre_decrement_test.test | RUN | 0.226256 | 1.259289 | 2.013617 | 0.001583 | 0.001644 | 0.001945 |
| has_postfix_classes3_test.test | RUN | 0.173796 | 0.545460 | 1.033467 | 0.001447 | 0.001688 | 0.001754 |
| has_postfix_classes2_test.test | RUN | 0.169296 | 0.525556 | 1.017930 | 0.001596 | 0.001510 | 0.001815 |
| has_postfix_classes1_test.test | RUN | 0.166983 | 0.512836 | 1.013464 | 0.001452 | 0.001796 | 0.001938 |
| has_postfix_classes0_test.test | RUN | 0.166810 | 0.542022 | 1.029248 | 0.001520 | 0.001800 | 0.001808 |
| has_post_increment_test.test | RUN | 0.226087 | 1.256543 | 2.009370 | 0.001415 | 0.001643 | 0.001826 |
| has_post_decrement_test.test | RUN | 0.224339 | 1.239172 | 1.991181 | 0.001542 | 0.001763 | 0.001629 |
| has_plus_test.test | RUN | 0.230930 | 0.915072 | 1.967956 | 0.001524 | 0.001669 | 0.001816 |
| has_plus_assign_test.test | RUN | 0.240298 | 0.885911 | 2.026300 | 0.001531 | 0.001786 | 0.001730 |
| has_operator_new_test.test | RUN | 0.159374 | 0.459953 | 0.904171 | 0.001646 | 0.001826 | 0.001921 |
| has_nothrow_destructor_test.test | RUN | 0.160369 | 0.531378 | 0.947297 | 0.001414 | 0.001664 | 0.001940 |
| has_nothrow_copy_test.test | RUN | 0.154778 | 0.498294 | 0.961126 | 0.001526 | 0.001640 | 0.001801 |
| has_nothrow_constr_test.test | RUN | 0.153637 | 0.472798 | 0.939904 | 0.001531 | 0.001821 | 0.001864 |
| has_nothrow_assign_test.test | RUN | 0.159507 | 0.483516 | 0.936032 | 0.001630 | 0.001893 | 0.001638 |
| has_not_equal_to_test.test | RUN | 0.235987 | 1.221756 | 1.965934 | 0.001636 | 0.001715 | 0.001838 |
| has_negate_test.test | RUN | 0.222920 | 0.891470 | 1.927028 | 0.001478 | 0.001802 | 0.001812 |
| has_multiplies_test.test | RUN | 0.231128 | 0.910248 | 1.987795 | 0.001623 | 0.001525 | 0.001571 |
| has_multiplies_assign_test.test | RUN | 0.233452 | 0.903253 | 1.960378 | 0.001635 | 0.001837 | 0.001747 |
| has_modulus_test.test | RUN | 0.229110 | 0.922550 | 1.942284 | 0.001691 | 0.001662 | 0.001773 |
| has_modulus_assign_test.test | RUN | 0.230482 | 0.929203 | 1.984566 | 0.001531 | 0.001694 | 0.001713 |
| has_minus_test.test | RUN | 0.233382 | 0.921898 | 1.936374 | 0.001621 | 0.001863 | 0.001784 |
| has_minus_assign_test.test | RUN | 0.245035 | 0.902661 | 2.062020 | 0.001780 | 0.001790 | 0.001762 |
| has_logical_or_test.test | RUN | 0.231904 | 0.939028 | 1.950112 | 0.001680 | 0.001609 | 0.001764 |
| has_logical_not_test.test | RUN | 0.223074 | 0.929982 | 1.917345 | 0.001553 | 0.001523 | 0.001834 |
| has_logical_and_test.test | RUN | 0.228664 | 0.923166 | 2.047635 | 0.001677 | 0.001650 | 0.001573 |
| has_less_test.test | RUN | 0.224876 | 1.237507 | 1.947737 | 0.001462 | 0.001679 | 0.001710 |
| has_less_equal_test.test | RUN | 0.233975 | 1.209465 | 1.907179 | 0.001655 | 0.001616 | 0.001387 |
| has_left_shift_test.test | RUN | 0.247910 | 1.133068 | 2.471498 | 0.001572 | 0.001751 | 0.001412 |
| has_left_shift_assign_test.test | RUN | 0.230423 | 0.891390 | 2.005719 | 0.001494 | 0.001259 | 0.001569 |
| has_greater_test.test | RUN | 0.231166 | 1.163498 | 1.933147 | 0.001496 | 0.001686 | 0.001454 |
| has_greater_equal_test.test | RUN | 0.228379 | 1.225358 | 1.962521 | 0.001527 | 0.001677 | 0.001821 |
| has_equal_to_test.test | RUN | 0.230262 | 1.216082 | 2.017694 | 0.001466 | 0.001849 | 0.001530 |
| has_divides_test.test | RUN | 0.226619 | 0.893329 | 2.001785 | 0.001559 | 0.001586 | 0.001348 |
| has_divides_assign_test.test | RUN | 0.228908 | 0.896693 | 1.925399 | 0.001683 | 0.001798 | 0.001278 |
| has_dereference_test.test | RUN | 0.219917 | 0.904332 | 1.853005 | 0.001528 | 0.001777 | 0.001839 |
| has_complement_test.test | RUN | 0.218920 | 1.260060 | 1.986659 | 0.001496 | 0.001680 | 0.001493 |
| has_bit_xor_test.test | RUN | 0.223095 | 0.889861 | 1.940623 | 0.001538 | 0.001851 | 0.001680 |
| has_bit_xor_assign_test.test | RUN | 0.228143 | 0.884188 | 1.947250 | 0.001492 | 0.001612 | 0.001550 |
| has_bit_or_test.test | RUN | 0.225679 | 0.926079 | 1.960359 | 0.001308 | 0.001746 | 0.001647 |
| has_bit_or_assign_test.test | RUN | 0.228162 | 0.902370 | 1.984515 | 0.001457 | 0.001831 | 0.001511 |
| has_bit_and_test.test | RUN | 0.225520 | 0.921812 | 1.988473 | 0.001562 | 0.001582 | 0.001363 |
| has_bit_and_assign_test.test | RUN | 0.227908 | 0.901510 | 1.943061 | 0.001494 | 0.001626 | 0.001592 |
| has_binary_classes9_test.test | RUN | 0.249263 | 1.074867 | 2.154534 | 0.001582 | 0.001845 | 0.001798 |
| has_binary_classes8_test.test | RUN | 0.250335 | 1.057216 | 2.122664 | 0.001569 | 0.001865 | 0.001726 |
| has_binary_classes7_test.test | RUN | 0.246602 | 1.056341 | 2.192868 | 0.001454 | 0.001716 | 0.001529 |
| has_binary_classes6_test.test | RUN | 0.248406 | 1.056146 | 2.195135 | 0.001637 | 0.001616 | 0.001424 |
| has_binary_classes5_test.test | RUN | 0.250967 | 1.036536 | 2.139805 | 0.001649 | 0.001805 | 0.001612 |
| has_binary_classes4_test.test | RUN | 0.242816 | 1.082916 | 2.124530 | 0.001418 | 0.001726 | 0.001772 |
| has_binary_classes3_test.test | RUN | 0.246955 | 1.074790 | 2.173567 | 0.001470 | 0.001662 | 0.001646 |
| has_binary_classes2_test.test | RUN | 0.250245 | 1.075206 | 2.174113 | 0.001229 | 0.001754 | 0.001387 |
| has_binary_classes1_test.test | RUN | 0.248121 | 1.056800 | 2.139635 | 0.001480 | 0.001742 | 0.001739 |
| has_binary_classes0_test.test | RUN | 0.250001 | 1.101469 | 2.092636 | 0.001407 | 0.001540 | 0.001820 |
| function_traits_test.test | RUN | 0.114152 | 0.284517 | 0.458795 | 0.001606 | 0.001608 | 0.001508 |
| floating_point_promotion_test.test | RUN | 0.109583 | 0.289588 | 0.432889 | 0.001275 | 0.001715 | 0.001414 |
| extent_test.test | RUN | 0.111965 | 0.291328 | 0.452583 | 0.001626 | 0.001679 | 0.001540 |
| enable_if_test.test | RUN | 0.105409 | 0.251870 | 0.342776 | 0.001684 | 0.001568 | 0.001758 |
| disjunction_test.test | RUN | 0.105923 | 0.249379 | 0.370669 | 0.001524 | 0.001783 | 0.001730 |
| detected_test.test | RUN | 0.104533 | 0.237333 | 0.349129 | 0.001714 | 0.001717 | 0.001555 |
| detected_or_test.test | RUN | 0.104011 | 0.260012 | 0.377761 | 0.001394 | 0.001766 | 0.001618 |
| decay_test.test | RUN | 0.276328 | 0.985338 | 2.275079 | 0.001621 | 0.001650 | 0.001746 |
| cxx14_aliases_test.test | RUN | 0.169514 | 0.360922 | 0.568786 | 0.001709 | 0.001472 | 0.001549 |
| copy_reference_test.test | RUN | 0.102027 | 0.245963 | 0.388030 | 0.001586 | 0.001622 | 0.001811 |
| copy_cv_test.test | RUN | 0.108590 | 0.270563 | 0.406882 | 0.001406 | 0.001727 | 0.001811 |
| copy_cv_ref_test.test | RUN | 0.139156 | 0.459294 | 0.897170 | 0.001520 | 0.001542 | 0.001758 |
| conjunction_test.test | RUN | 0.105687 | 0.265747 | 0.383554 | 0.001682 | 0.001624 | 0.001728 |
| conditional_test.test | RUN | 0.104392 | 0.239745 | 0.374944 | 0.001377 | 0.001497 | 0.001727 |
| common_type_test.test | RUN | 0.139040 | 0.391064 | 0.728463 | 0.001452 | 0.001540 | 0.001630 |
| common_type_sfinae_test.test | RUN | 0.105792 | 0.236308 | 0.325021 | 0.001546 | 0.001514 | 0.001732 |
| common_type_sfinae2_test.test | RUN | 0.108016 | 0.255201 | 0.347925 | 0.001633 | 0.001412 | 0.001642 |
| common_type_6_test.test | RUN | 0.110589 | 0.264872 | 0.382696 | 0.001529 | 0.001564 | 0.001412 |
| common_type_5_test.test | RUN | 0.109737 | 0.257603 | 0.361950 | 0.001536 | 0.001592 | 0.001438 |
| common_type_4_test.test | RUN | 0.119113 | 0.281477 | 0.462174 | 0.001628 | 0.001469 | 0.001441 |
| common_type_3_test.test | RUN | 0.104374 | 0.243869 | 0.352309 | 0.001700 | 0.001592 | 0.001567 |
| common_type_2_test.test | RUN | 0.139386 | 0.380358 | 0.714008 | 0.001659 | 0.001770 | 0.001492 |
| alignment_of_test.test | RUN | 0.110001 | 0.312131 | 0.480808 | 0.001325 | 0.001700 | 0.001752 |
| alignment_of_a2_test.test | RUN | 0.113487 | 0.311141 | 0.449069 | 0.001572 | 0.001680 | 0.001701 |
| aligned_storage_test.test | RUN | 0.117412 | 0.294949 | 0.423110 | 0.001645 | 0.001907 | 0.001449 |
| aligned_storage_empy_test.test | RUN | 0.128447 | 0.366982 | 0.538590 | 0.001448 | 0.001786 | 0.001673 |
| aligned_storage_a2_test.test | RUN | 0.113190 | 0.313536 | 0.408050 | 0.001691 | 0.002050 | 0.001799 |
| add_volatile_test.test | RUN | 0.247378 | 0.892361 | 2.130333 | 0.001647 | 0.001622 | 0.001807 |
| add_rvalue_reference_test.test | RUN | 0.276467 | 0.980682 | 2.520885 | 0.001524 | 0.001304 | 0.001822 |
| add_reference_test.test | RUN | 0.264136 | 0.961041 | 2.454817 | 0.001503 | 0.001885 | 0.001744 |
| add_pointer_test.test | RUN | 0.171587 | 0.546528 | 1.258016 | 0.001210 | 0.001664 | 0.001785 |
| add_lvalue_reference_test.test | RUN | 0.271020 | 0.953076 | 2.485933 | 0.001644 | 0.001614 | 0.001745 |
| add_cv_test.test | RUN | 0.255507 | 0.888284 | 2.347662 | 0.001550 | 0.001551 | 0.001792 |
| add_const_test.test | RUN | 0.254733 | 0.943334 | 2.290846 | 0.001372 | 0.001503 | 0.001706 |
| is_virtual_base_of_fail.test | COMPILE_FAIL | 0.101946 | 0.239882 | 0.353088 | — | — | — |
| is_stateless_fail.test | COMPILE_FAIL | 0.101012 | 0.244382 | 0.363261 | — | — | — |
| is_polymorphic_fail.test | COMPILE_FAIL | 0.098553 | 0.225749 | 0.335550 | — | — | — |
| is_nothrow_move_constructible_fail.test | COMPILE_FAIL | 0.099114 | 0.227563 | 0.345359 | — | — | — |
| is_nothrow_move_assignable_fail.test | COMPILE_FAIL | 0.100219 | 0.231652 | 0.308994 | — | — | — |
| is_list_constructible_fail.test | COMPILE_FAIL | 0.100871 | 0.225350 | 0.318284 | — | — | — |
| is_empty_fail.test | COMPILE_FAIL | 0.101743 | 0.229504 | 0.329092 | — | — | — |
| is_destructible_fail.test | COMPILE_FAIL | 0.099492 | 0.229144 | 0.313226 | — | — | — |
| is_default_constructible_fail.test | COMPILE_FAIL | 0.100891 | 0.230888 | 0.320594 | — | — | — |
| is_copy_constructible_fail.test | COMPILE_FAIL | 0.103910 | 0.227810 | 0.324109 | — | — | — |
| is_copy_assignable_fail.test | COMPILE_FAIL | 0.100162 | 0.225578 | 0.322208 | — | — | — |
| is_convertible_fail.test | COMPILE_FAIL | 0.100412 | 0.227974 | 0.317724 | — | — | — |
| is_convertible2_fail.test | COMPILE_FAIL | 0.100263 | 0.234089 | 0.323142 | — | — | — |
| is_constructible_fail.test | COMPILE_FAIL | 0.099899 | 0.231672 | 0.324648 | — | — | — |
| is_base_of_fail.test | COMPILE_FAIL | 0.097905 | 0.234135 | 0.319621 | — | — | — |
| is_assignable_fail.test | COMPILE_FAIL | 0.098073 | 0.229310 | 0.321514 | — | — | — |
| has_nothrow_destruct_fail.test | COMPILE_FAIL | 0.099711 | 0.231515 | 0.318184 | — | — | — |
| has_nothrow_copy_fail.test | COMPILE_FAIL | 0.101456 | 0.230335 | 0.324627 | — | — | — |
| has_nothrow_cons_fail.test | COMPILE_FAIL | 0.098138 | 0.229247 | 0.320584 | — | — | — |
| has_nothrow_assign_fail.test | COMPILE_FAIL | 0.098923 | 0.228358 | 0.321726 | — | — | — |
| common_type_fail.test | COMPILE_FAIL | 0.099837 | 0.230427 | 0.328699 | — | — | — |
| common_type2_fail.test | COMPILE_FAIL | 0.101249 | 0.231407 | 0.305302 | — | — | — |
| has_nothrow_assign_test_no_intrinsics.test | RUN | 0.157118 | 0.501712 | 0.944762 | 0.001175 | 0.001747 | 0.001728 |
| has_nothrow_constr_test_no_intrinsics.test | RUN | 0.147582 | 0.486116 | 0.938214 | 0.001596 | 0.001711 | 0.001573 |
| has_nothrow_copy_test_no_intrinsics.test | RUN | 0.158510 | 0.486477 | 0.893626 | 0.001374 | 0.001258 | 0.001860 |
| is_nothrow_move_assignable_test_no_intrinsics.test | RUN | 0.151868 | 0.483740 | 0.898902 | 0.001267 | 0.001438 | 0.001370 |
| is_nothrow_move_constructible_test_no_intrinsics.test | RUN | 0.101507 | 0.370146 | 0.516083 | 0.001230 | 0.001279 | 0.001198 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.970×† | 0.930×, 1.009× | 1.085× |
| clang | 1.001× | 0.996×, 1.006× | 0.976× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267198).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| add_const_test.test | 10 | 0.001022s | 0.000968s | 0.000979s | byte equality observed |
| add_cv_test.test | 10 | 0.001054s | 0.000958s | 0.001019s | byte equality observed |
| add_lvalue_reference_test.test | 10 | 0.001045s | 0.000998s | 0.000983s | byte equality observed |
| add_pointer_test.test | 10 | 0.001030s | 0.000984s | 0.000966s | byte equality observed |
| add_reference_test.test | 10 | 0.001035s | 0.000983s | 0.000973s | byte equality observed |
| add_rvalue_reference_test.test | 10 | 0.001003s | 0.001015s | 0.000975s | byte equality observed |
| add_volatile_test.test | 10 | 0.001007s | 0.000971s | 0.000980s | byte equality observed |
| aligned_storage_a2_test.test | 10 | 0.001094s | 0.001088s | 0.001083s | byte equality observed |
| aligned_storage_empy_test.test | 10 | 0.001014s | 0.000995s | 0.000987s | byte equality observed |
| aligned_storage_test.test | 10 | 0.001130s | 0.001081s | 0.001066s | byte equality observed |
| alignment_of_a2_test.test | 10 | 0.001007s | 0.001001s | 0.001009s | byte equality observed |
| alignment_of_test.test | 10 | 0.001026s | 0.000984s | 0.000994s | byte equality observed |
| common_type_2_test.test | 10 | 0.001046s | 0.001013s | 0.000965s | byte equality observed |
| common_type_3_test.test | 9 | 0.001011s | 0.000982s | 0.000991s | byte equality observed |
| common_type_4_test.test | 8 | 0.001028s | 0.001026s | 0.000966s | byte equality observed |
| common_type_5_test.test | 8 | 0.001018s | 0.000982s | 0.000988s | byte equality observed |
| common_type_6_test.test | 9 | 0.001003s | 0.000967s | 0.001003s | byte equality observed |
| common_type_sfinae2_test.test | 10 | 0.001046s | 0.000976s | 0.001016s | byte equality observed |
| common_type_sfinae_test.test | 6 | 0.000996s | 0.000990s | 0.000998s | byte equality observed |
| common_type_test.test | 9 | 0.001004s | 0.000974s | 0.000966s | byte equality observed |
| conditional_test.test | 10 | 0.001011s | 0.000983s | 0.000983s | byte equality observed |
| conjunction_test.test | 10 | 0.001027s | 0.000975s | 0.001017s | byte equality observed |
| copy_cv_ref_test.test | 10 | 0.000993s | 0.000994s | 0.000963s | byte equality observed |
| copy_cv_test.test | 10 | 0.001065s | 0.000983s | 0.000981s | byte equality observed |
| copy_reference_test.test | 10 | 0.001061s | 0.001000s | 0.001012s | byte equality observed |
| cxx14_aliases_test.test | 10 | 0.001055s | 0.000987s | 0.000999s | byte equality observed |
| decay_test.test | 10 | 0.001079s | 0.000992s | 0.000968s | byte equality observed |
| detected_or_test.test | 10 | 0.001054s | 0.000988s | 0.000983s | byte equality observed |
| detected_test.test | 10 | 0.001097s | 0.000969s | 0.000997s | byte equality observed |
| disjunction_test.test | 10 | 0.001093s | 0.000979s | 0.000983s | byte equality observed |
| enable_if_test.test | 10 | 0.001100s | 0.001019s | 0.001004s | byte equality observed |
| extent_test.test | 10 | 0.001074s | 0.000960s | 0.000967s | byte equality observed |
| floating_point_promotion_test.test | 10 | 0.001075s | 0.001016s | 0.000967s | byte equality observed |
| function_traits_test.test | 10 | 0.001110s | 0.001014s | 0.000969s | byte equality observed |
| has_binary_classes0_test.test | 10 | 0.001093s | 0.000987s | 0.000985s | byte equality observed |
| has_binary_classes1_test.test | 10 | 0.001091s | 0.001005s | 0.000972s | byte equality observed |
| has_binary_classes2_test.test | 10 | 0.001075s | 0.000985s | 0.001035s | byte equality observed |
| has_binary_classes3_test.test | 10 | 0.001067s | 0.000980s | 0.000996s | byte equality observed |
| has_binary_classes4_test.test | 10 | 0.001101s | 0.000997s | 0.000983s | byte equality observed |
| has_binary_classes5_test.test | 10 | 0.001074s | 0.001003s | 0.001024s | byte equality observed |
| has_binary_classes6_test.test | 10 | 0.001066s | 0.000995s | 0.000993s | byte equality observed |
| has_binary_classes7_test.test | 10 | 0.001081s | 0.000987s | 0.000985s | byte equality observed |
| has_binary_classes8_test.test | 10 | 0.001084s | 0.001038s | 0.000998s | byte equality observed |
| has_binary_classes9_test.test | 10 | 0.001082s | 0.000977s | 0.001026s | byte equality observed |
| has_bit_and_assign_test.test | 10 | 0.001062s | 0.000987s | 0.000974s | byte equality observed |
| has_bit_and_test.test | 9 | 0.001074s | 0.000979s | 0.000977s | byte equality observed |
| has_bit_or_assign_test.test | 10 | 0.001068s | 0.000994s | 0.001012s | byte equality observed |
| has_bit_or_test.test | 10 | 0.001054s | 0.000981s | 0.000989s | byte equality observed |
| has_bit_xor_assign_test.test | 10 | 0.001084s | 0.000978s | 0.000993s | byte equality observed |
| has_bit_xor_test.test | 10 | 0.001076s | 0.001012s | 0.000976s | byte equality observed |
| has_complement_test.test | 10 | 0.001068s | 0.000990s | 0.001067s | byte equality observed |
| has_dereference_test.test | 9 | 0.001075s | 0.001064s | 0.000984s | byte equality observed |
| has_divides_assign_test.test | 10 | 0.001066s | 0.001036s | 0.000981s | byte equality observed |
| has_divides_test.test | 10 | 0.001093s | 0.001044s | 0.001036s | byte equality observed |
| has_equal_to_test.test | 10 | 0.001054s | 0.001059s | 0.000998s | byte equality observed |
| has_greater_equal_test.test | 10 | 0.001113s | 0.001032s | 0.000991s | byte equality observed |
| has_greater_test.test | 10 | 0.001103s | 0.001053s | 0.001031s | byte equality observed |
| has_left_shift_assign_test.test | 10 | 0.001123s | 0.001082s | 0.001022s | byte equality observed |
| has_left_shift_test.test | 10 | 0.001118s | 0.001087s | 0.001032s | byte equality observed |
| has_less_equal_test.test | 10 | 0.001106s | 0.001075s | 0.001062s | byte equality observed |
| has_less_test.test | 10 | 0.001132s | 0.001093s | 0.001059s | byte equality observed |
| has_logical_and_test.test | 10 | 0.001121s | 0.001064s | 0.001042s | byte equality observed |
| has_logical_not_test.test | 10 | 0.001152s | 0.001069s | 0.001076s | byte equality observed |
| has_logical_or_test.test | 10 | 0.001142s | 0.001086s | 0.001074s | byte equality observed |
| has_minus_assign_test.test | 10 | 0.001146s | 0.001072s | 0.001066s | byte equality observed |
| has_minus_test.test | 10 | 0.001130s | 0.001057s | 0.001056s | byte equality observed |
| has_modulus_assign_test.test | 10 | 0.001129s | 0.001057s | 0.001093s | byte equality observed |
| has_modulus_test.test | 10 | 0.001101s | 0.001082s | 0.001082s | byte equality observed |
| has_multiplies_assign_test.test | 10 | 0.001140s | 0.001060s | 0.001088s | byte equality observed |
| has_multiplies_test.test | 10 | 0.001146s | 0.001063s | 0.001091s | byte equality observed |
| has_negate_test.test | 10 | 0.001114s | 0.001054s | 0.001095s | byte equality observed |
| has_not_equal_to_test.test | 10 | 0.001153s | 0.001079s | 0.001062s | byte equality observed |
| has_nothrow_assign_test.test | 10 | 0.001138s | 0.001038s | 0.001074s | byte equality observed |
| has_nothrow_assign_test_no_intrinsics.test | 10 | 0.001133s | 0.001060s | 0.001075s | byte equality observed |
| has_nothrow_constr_test.test | 10 | 0.001141s | 0.001040s | 0.001101s | byte equality observed |
| has_nothrow_constr_test_no_intrinsics.test | 10 | 0.001141s | 0.001041s | 0.001057s | byte equality observed |
| has_nothrow_copy_test.test | 10 | 0.001121s | 0.001097s | 0.001072s | byte equality observed |
| has_nothrow_copy_test_no_intrinsics.test | 10 | 0.001132s | 0.001064s | 0.001094s | byte equality observed |
| has_nothrow_destructor_test.test | 10 | 0.001115s | 0.001043s | 0.001047s | byte equality observed |
| has_operator_new_test.test | 10 | 0.001121s | 0.001042s | 0.001085s | byte equality observed |
| has_plus_assign_test.test | 10 | 0.001153s | 0.001072s | 0.001125s | byte equality observed |
| has_plus_test.test | 10 | 0.001142s | 0.001089s | 0.001105s | byte equality observed |
| has_post_decrement_test.test | 10 | 0.001127s | 0.001084s | 0.001076s | byte equality observed |
| has_post_increment_test.test | 10 | 0.001136s | 0.001093s | 0.001052s | byte equality observed |
| has_postfix_classes0_test.test | 10 | 0.001103s | 0.001125s | 0.001075s | byte equality observed |
| has_postfix_classes1_test.test | 10 | 0.001123s | 0.001119s | 0.001062s | byte equality observed |
| has_postfix_classes2_test.test | 10 | 0.001136s | 0.001099s | 0.001075s | byte equality observed |
| has_postfix_classes3_test.test | 10 | 0.001130s | 0.001122s | 0.001070s | byte equality observed |
| has_pre_decrement_test.test | 10 | 0.001141s | 0.001144s | 0.001097s | byte equality observed |
| has_pre_increment_test.test | 10 | 0.001147s | 0.001127s | 0.001066s | byte equality observed |
| has_prefix_classes0_test.test | 10 | 0.001153s | 0.001147s | 0.001095s | byte equality observed |
| has_prefix_classes1_test.test | 10 | 0.001130s | 0.001123s | 0.001077s | byte equality observed |
| has_prefix_classes2_test.test | 10 | 0.001131s | 0.001142s | 0.001094s | byte equality observed |
| has_prefix_classes3_test.test | 10 | 0.001144s | 0.001155s | 0.001071s | byte equality observed |
| has_right_shift_assign_test.test | 10 | 0.001135s | 0.001105s | 0.001109s | byte equality observed |
| has_right_shift_test.test | 10 | 0.001118s | 0.001141s | 0.001035s | byte equality observed |
| has_trivial_assign_test.test | 10 | 0.001199s | 0.001230s | 0.001150s | byte equality observed |
| has_trivial_constr_test.test | 10 | 0.001178s | 0.001191s | 0.001112s | native test oracle; output varies |
| has_trivial_copy_test.test | 10 | 0.001205s | 0.001179s | 0.001193s | native test oracle; output varies |
| has_trivial_destructor_test.test | 10 | 0.001125s | 0.001147s | 0.001023s | byte equality observed |
| has_trivial_move_assign_test.test | 10 | 0.001127s | 0.001113s | 0.001084s | byte equality observed |
| has_trivial_move_constructor_test.test | 10 | 0.001117s | 0.001144s | 0.001089s | byte equality observed |
| has_unary_minus_test.test | 10 | 0.001124s | 0.001118s | 0.001138s | byte equality observed |
| has_unary_plus_test.test | 10 | 0.001118s | 0.001145s | 0.001150s | byte equality observed |
| has_virtual_destructor_test.test | 10 | 0.001127s | 0.001124s | 0.001142s | byte equality observed |
| integral_constant_test.test | 10 | 0.000507s | 0.000511s | 0.001146s | byte equality observed |
| is_abstract_test.test | 10 | 0.001129s | 0.001131s | 0.001131s | byte equality observed |
| is_arithmetic_test.test | 10 | 0.001151s | 0.001139s | 0.001147s | byte equality observed |
| is_array_test.test | 10 | 0.001139s | 0.001137s | 0.001137s | byte equality observed |
| is_assignable_test.test | 10 | 0.001161s | 0.001126s | 0.001105s | byte equality observed |
| is_base_and_derived_test.test | 10 | 0.001146s | 0.001144s | 0.001134s | byte equality observed |
| is_base_of_test.test | 10 | 0.001147s | 0.001136s | 0.001123s | byte equality observed |
| is_bounded_array_test.test | 10 | 0.001148s | 0.001136s | 0.001120s | byte equality observed |
| is_class_test.test | 10 | 0.001140s | 0.001116s | 0.001147s | byte equality observed |
| is_complete_test.test | 10 | 0.001131s | 0.001137s | 0.001131s | byte equality observed |
| is_complex_test.test | 10 | 0.001149s | 0.001118s | 0.001153s | byte equality observed |
| is_compound_test.test | 10 | 0.001151s | 0.001098s | 0.001148s | byte equality observed |
| is_const_test.test | 10 | 0.001140s | 0.001115s | 0.001147s | byte equality observed |
| is_constructible_test.test | 10 | 0.001153s | 0.001096s | 0.001171s | byte equality observed |
| is_convertible_test.test | 10 | 0.001171s | 0.001127s | 0.001149s | byte equality observed |
| is_copy_assignable_test.test | 10 | 0.001139s | 0.001130s | 0.001151s | byte equality observed |
| is_copy_constructible_test.test | 10 | 0.001148s | 0.001133s | 0.001167s | byte equality observed |
| is_default_constr_test.test | 10 | 0.001144s | 0.001141s | 0.001156s | byte equality observed |
| is_destructible_test.test | 10 | 0.001151s | 0.001145s | 0.001141s | byte equality observed |
| is_detected_exact_test.test | 10 | 0.001146s | 0.001118s | 0.001149s | byte equality observed |
| is_detected_test.test | 10 | 0.001155s | 0.001127s | 0.001155s | byte equality observed |
| is_empty_test.test | 10 | 0.001142s | 0.001139s | 0.001155s | byte equality observed |
| is_enum_test.test | 10 | 0.001135s | 0.001141s | 0.001140s | byte equality observed |
| is_final_test.test | 10 | 0.001155s | 0.001133s | 0.001133s | byte equality observed |
| is_float_test.test | 10 | 0.001154s | 0.001138s | 0.001155s | byte equality observed |
| is_floating_point_test.test | 10 | 0.001156s | 0.001141s | 0.001154s | byte equality observed |
| is_function_test.test | 10 | 0.001144s | 0.001146s | 0.001160s | byte equality observed |
| is_fundamental_test.test | 10 | 0.001152s | 0.001145s | 0.001138s | byte equality observed |
| is_integral_test.test | 10 | 0.001142s | 0.001156s | 0.001154s | byte equality observed |
| is_list_constructible_test.test | 10 | 0.001149s | 0.001150s | 0.001160s | byte equality observed |
| is_lvalue_reference_test.test | 10 | 0.001144s | 0.001138s | 0.001142s | byte equality observed |
| is_member_func_test.test | 10 | 0.001160s | 0.001154s | 0.001148s | byte equality observed |
| is_member_obj_test.test | 10 | 0.001157s | 0.001120s | 0.001149s | byte equality observed |
| is_member_pointer_test.test | 10 | 0.001150s | 0.001156s | 0.001170s | byte equality observed |
| is_noncopyable_test.test | 10 | 0.001156s | 0.001146s | 0.001136s | byte equality observed |
| is_nothrow_move_assignable_test.test | 10 | 0.001150s | 0.001141s | 0.001149s | byte equality observed |
| is_nothrow_move_assignable_test_no_intrinsics.test | 10 | 0.001144s | 0.001125s | 0.001168s | byte equality observed |
| is_nothrow_move_constructible_test.test | 10 | 0.001136s | 0.001134s | 0.001156s | byte equality observed |
| is_nothrow_move_constructible_test_no_intrinsics.test | 10 | 0.001159s | 0.001114s | 0.001138s | byte equality observed |
| is_nothrow_swappable_test.test | 10 | 0.001150s | 0.001133s | 0.001149s | byte equality observed |
| is_object_test.test | 10 | 0.001144s | 0.001164s | 0.001146s | byte equality observed |
| is_pod_test.test | 10 | 0.001144s | 0.001136s | 0.001145s | byte equality observed |
| is_pointer_test.test | 10 | 0.001134s | 0.001141s | 0.001149s | byte equality observed |
| is_polymorphic_test.test | 10 | 0.001149s | 0.001148s | 0.001154s | byte equality observed |
| is_reference_test.test | 10 | 0.001146s | 0.001111s | 0.001172s | byte equality observed |
| is_rvalue_reference_test.test | 10 | 0.001169s | 0.001142s | 0.001161s | byte equality observed |
| is_same_test.test | 10 | 0.001149s | 0.001141s | 0.001156s | byte equality observed |
| is_scalar_test.test | 10 | 0.001140s | 0.001144s | 0.001148s | byte equality observed |
| is_scoped_enum_test.test | 10 | 0.001156s | 0.001136s | 0.001153s | byte equality observed |
| is_signed_test.test | 10 | 0.001159s | 0.001151s | 0.001159s | byte equality observed |
| is_stateless_test.test | 10 | 0.001150s | 0.001148s | 0.001144s | byte equality observed |
| is_swappable_test.test | 10 | 0.001150s | 0.001148s | 0.001152s | byte equality observed |
| is_trivially_copyable_test.test | 10 | 0.001207s | 0.001164s | 0.001204s | byte equality observed |
| is_unbounded_array_test.test | 10 | 0.001150s | 0.001159s | 0.001167s | byte equality observed |
| is_union_test.test | 10 | 0.001143s | 0.001152s | 0.001140s | byte equality observed |
| is_unscoped_enum_test.test | 10 | 0.001145s | 0.001126s | 0.001149s | byte equality observed |
| is_unsigned_test.test | 10 | 0.001158s | 0.001124s | 0.001159s | byte equality observed |
| is_virtual_base_of_test.test | 10 | 0.001151s | 0.001158s | 0.001160s | byte equality observed |
| is_void_test.test | 10 | 0.001158s | 0.001137s | 0.001161s | byte equality observed |
| is_volatile_test.test | 10 | 0.001171s | 0.001153s | 0.001159s | byte equality observed |
| make_signed_test.test | 10 | 0.001161s | 0.001152s | 0.001154s | byte equality observed |
| make_unsigned_test.test | 10 | 0.001182s | 0.001174s | 0.001147s | byte equality observed |
| make_void_test.test | 10 | 0.001169s | 0.001187s | 0.001175s | byte equality observed |
| mpl_interop_test1.test | 10 | 0.000513s | 0.000519s | 0.001151s | byte equality observed |
| mpl_interop_test2.test | 10 | 0.000521s | 0.000513s | 0.001165s | byte equality observed |
| mpl_interop_test3.test | 10 | 0.000525s | 0.000523s | 0.001142s | byte equality observed |
| negation_test.test | 10 | 0.001210s | 0.001198s | 0.001147s | byte equality observed |
| promote_basic_test.test | 10 | 0.000525s | 0.000525s | 0.001140s | byte equality observed |
| promote_enum_msvc_bug_test.test | 10 | 0.000520s | 0.000510s | 0.001143s | byte equality observed |
| promote_enum_test.test | 10 | 0.000520s | 0.000523s | 0.001164s | byte equality observed |
| promote_mpl_test.test | 10 | 0.000519s | 0.000515s | 0.001129s | byte equality observed |
| rank_test.test | 10 | 0.001174s | 0.001155s | 0.001127s | byte equality observed |
| remove_all_extents_test.test | 10 | 0.001177s | 0.001169s | 0.001141s | byte equality observed |
| remove_bounds_test.test | 10 | 0.001173s | 0.001150s | 0.001149s | byte equality observed |
| remove_const_test.test | 10 | 0.001152s | 0.001147s | 0.001148s | byte equality observed |
| remove_cv_ref_test.test | 10 | 0.001153s | 0.001155s | 0.001174s | byte equality observed |
| remove_cv_test.test | 10 | 0.001158s | 0.001145s | 0.001151s | byte equality observed |
| remove_extent_test.test | 10 | 0.001175s | 0.001130s | 0.001160s | byte equality observed |
| remove_pointer_test.test | 10 | 0.001167s | 0.001126s | 0.001148s | byte equality observed |
| remove_reference_test.test | 10 | 0.001155s | 0.001156s | 0.001140s | byte equality observed |
| remove_volatile_test.test | 10 | 0.001137s | 0.001146s | 0.001138s | byte equality observed |
| tricky_abstract_type_test.test | 10 | 0.001153s | 0.001146s | 0.001159s | byte equality observed |
| tricky_add_pointer_test.test | 10 | 0.001139s | 0.001146s | 0.001157s | byte equality observed |
| tricky_function_type_test.test | 10 | 0.001153s | 0.001146s | 0.001154s | byte equality observed |
| tricky_incomplete_type_test.test | 10 | 0.001154s | 0.001166s | 0.001145s | byte equality observed |
| tricky_is_enum_test.test | 10 | 0.001148s | 0.001144s | 0.001165s | byte equality observed |
| tricky_partial_spec_test.test | 10 | 0.001159s | 0.001154s | 0.001146s | byte equality observed |
| tricky_rvalue_test.test | 10 | 0.001151s | 0.001178s | 0.001148s | byte equality observed |
| type_identity_test.test | 10 | 0.001178s | 0.001173s | 0.001157s | byte equality observed |
| type_traits_test.test | 10 | 0.000516s | 0.000515s | 0.001156s | byte equality observed |
| type_with_alignment_test.test | 10 | 0.001282s | 0.001301s | 0.001276s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267198).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
