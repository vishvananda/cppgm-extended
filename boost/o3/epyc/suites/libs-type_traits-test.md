# libs/type_traits/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| type_with_alignment_test.test | RUN | 0.113075 | 0.320288 | 0.508622 | 0.001733 | 0.001618 | 0.001711 |
| type_traits_test.test | RUN | 0.164155 | 0.331984 | 0.493409 | 0.000593 | 0.000529 | 0.001470 |
| type_identity_test.test | RUN | 0.308285 | 1.143394 | 2.925162 | 0.001607 | 0.001778 | 0.001624 |
| tricky_rvalue_test.test | RUN | 0.106385 | 0.257524 | 0.390879 | 0.001395 | 0.001694 | 0.001463 |
| tricky_partial_spec_test.test | RUN | 0.126135 | 0.362689 | 0.644716 | 0.001546 | 0.001734 | 0.001622 |
| tricky_is_enum_test.test | RUN | 0.103247 | 0.242507 | 0.360145 | 0.001504 | 0.001650 | 0.001710 |
| tricky_incomplete_type_test.test | RUN | 0.170153 | 0.382333 | 0.602079 | 0.001419 | 0.001586 | 0.001560 |
| tricky_function_type_test.test | RUN | 0.136918 | 0.403017 | 0.679140 | 0.001608 | 0.001693 | 0.001334 |
| tricky_add_pointer_test.test | RUN | 0.202025 | 0.695898 | 1.655137 | 0.001764 | 0.001781 | 0.001748 |
| tricky_abstract_type_test.test | RUN | 0.109095 | 0.242325 | 0.376359 | 0.001742 | 0.001678 | 0.001445 |
| remove_volatile_test.test | RUN | 0.267286 | 0.981313 | 2.623714 | 0.001653 | 0.001814 | 0.001720 |
| remove_reference_test.test | RUN | 0.281880 | 1.054656 | 2.710250 | 0.001670 | 0.001810 | 0.001674 |
| remove_pointer_test.test | RUN | 0.296537 | 1.079891 | 2.863337 | 0.001701 | 0.001670 | 0.001649 |
| remove_extent_test.test | RUN | 0.284054 | 1.061715 | 2.751442 | 0.001553 | 0.001694 | 0.001597 |
| remove_cv_test.test | RUN | 0.281082 | 1.046624 | 2.710030 | 0.001708 | 0.001759 | 0.001666 |
| remove_cv_ref_test.test | RUN | 0.592744 | 2.505226 | 6.623978 | 0.001517 | 0.001589 | 0.001326 |
| remove_const_test.test | RUN | 0.268586 | 0.974573 | 2.614103 | 0.001760 | 0.001864 | 0.001795 |
| remove_bounds_test.test | RUN | 0.288796 | 1.075479 | 2.714506 | 0.001665 | 0.001851 | 0.001687 |
| remove_all_extents_test.test | RUN | 0.286590 | 1.049497 | 2.750893 | 0.001588 | 0.001583 | 0.001241 |
| rank_test.test | RUN | 0.106072 | 0.264045 | 0.355638 | 0.001680 | 0.001717 | 0.001611 |
| promote_mpl_test.test | RUN | 0.094257 | 0.233182 | 0.262880 | 0.000576 | 0.000609 | 0.001627 |
| promote_enum_test.test | RUN | 0.021164 | 0.060655 | 0.063279 | 0.000579 | 0.000695 | 0.001714 |
| promote_enum_msvc_bug_test.test | RUN | 0.019020 | 0.049005 | 0.050221 | 0.000634 | 0.000604 | 0.001545 |
| promote_basic_test.test | RUN | 0.025088 | 0.070222 | 0.066126 | 0.000674 | 0.000667 | 0.001546 |
| negation_test.test | RUN | 0.102590 | 0.231352 | 0.356831 | 0.001584 | 0.001477 | 0.001616 |
| mpl_interop_test3.test | RUN | 0.088631 | 0.226690 | 0.260763 | 0.000614 | 0.000615 | 0.001393 |
| mpl_interop_test2.test | RUN | 0.032401 | 0.060635 | 0.073594 | 0.000545 | 0.000600 | 0.001524 |
| mpl_interop_test1.test | RUN | 0.017119 | 0.045213 | 0.049132 | 0.000678 | 0.000782 | 0.001760 |
| make_void_test.test | RUN | 0.108899 | 0.248567 | 0.409965 | 0.001739 | 0.001762 | 0.001506 |
| make_unsigned_test.test | RUN | 0.127645 | 0.337863 | 0.547028 | 0.001497 | 0.001632 | 0.001710 |
| make_signed_test.test | RUN | 0.126648 | 0.339192 | 0.581226 | 0.001394 | 0.001542 | 0.001591 |
| is_volatile_test.test | RUN | 0.114030 | 0.282431 | 0.425611 | 0.001447 | 0.001713 | 0.001526 |
| is_void_test.test | RUN | 0.110582 | 0.265956 | 0.376692 | 0.001546 | 0.001694 | 0.001302 |
| is_virtual_base_of_test.test | RUN | 0.124427 | 0.338155 | 0.590971 | 0.001389 | 0.001579 | 0.001441 |
| is_unsigned_test.test | RUN | 0.110460 | 0.278244 | 0.447686 | 0.001577 | 0.001516 | 0.001568 |
| is_unscoped_enum_test.test | RUN | 0.107049 | 0.244564 | 0.358978 | 0.001394 | 0.001515 | 0.001527 |
| is_union_test.test | RUN | 0.117570 | 0.318250 | 0.517618 | 0.001573 | 0.001617 | 0.001682 |
| is_unbounded_array_test.test | RUN | 0.108889 | 0.269949 | 0.391910 | 0.001612 | 0.001558 | 0.001710 |
| is_trivially_copyable_test.test | RUN | 0.176034 | 0.548813 | 1.025222 | 0.001321 | 0.001622 | 0.001825 |
| is_swappable_test.test | RUN | 0.145073 | 0.423222 | 0.799478 | 0.001688 | 0.001570 | 0.001608 |
| is_stateless_test.test | RUN | 0.160223 | 0.481822 | 0.888010 | 0.001269 | 0.001608 | 0.001710 |
| is_signed_test.test | RUN | 0.112373 | 0.288542 | 0.449978 | 0.001497 | 0.001680 | 0.001535 |
| is_scoped_enum_test.test | RUN | 0.106287 | 0.248763 | 0.372563 | 0.001678 | 0.001702 | 0.001614 |
| is_scalar_test.test | RUN | 0.139762 | 0.433018 | 0.767256 | 0.001504 | 0.001659 | 0.001264 |
| is_same_test.test | RUN | 0.108091 | 0.265291 | 0.387030 | 0.001425 | 0.001625 | 0.001646 |
| is_rvalue_reference_test.test | RUN | 0.113426 | 0.294511 | 0.479127 | 0.001611 | 0.001597 | 0.001333 |
| is_reference_test.test | RUN | 0.116069 | 0.296003 | 0.506367 | 0.001596 | 0.001708 | 0.001555 |
| is_polymorphic_test.test | RUN | 0.116031 | 0.306425 | 0.506011 | 0.001406 | 0.001645 | 0.001608 |
| is_pointer_test.test | RUN | 0.114073 | 0.301537 | 0.523491 | 0.001714 | 0.001612 | 0.001842 |
| is_pod_test.test | RUN | 0.149370 | 0.487716 | 0.920842 | 0.001482 | 0.001689 | 0.001675 |
| is_object_test.test | RUN | 0.106251 | 0.253703 | 0.375669 | 0.001565 | 0.001630 | 0.002002 |
| is_nothrow_swappable_test.test | RUN | 0.151918 | 0.458486 | 0.829298 | 0.001547 | 0.001782 | 0.001612 |
| is_nothrow_move_constructible_test.test | RUN | 0.146855 | 0.475355 | 0.927571 | 0.001585 | 0.001636 | 0.001661 |
| is_nothrow_move_assignable_test.test | RUN | 0.147379 | 0.486420 | 0.922105 | 0.001563 | 0.001768 | 0.001658 |
| is_noncopyable_test.test | RUN | 0.107104 | 0.271936 | 0.369882 | 0.001440 | 0.001697 | 0.001654 |
| is_member_pointer_test.test | RUN | 0.110921 | 0.258744 | 0.361196 | 0.001620 | 0.001828 | 0.001433 |
| is_member_obj_test.test | RUN | 0.108192 | 0.262619 | 0.383581 | 0.001709 | 0.001571 | 0.001658 |
| is_member_func_test.test | RUN | 0.108912 | 0.279466 | 0.436788 | 0.001352 | 0.001631 | 0.001745 |
| is_lvalue_reference_test.test | RUN | 0.112874 | 0.300062 | 0.468538 | 0.001524 | 0.001635 | 0.001633 |
| is_list_constructible_test.test | RUN | 0.131314 | 0.348991 | 0.519591 | 0.001648 | 0.001702 | 0.001422 |
| is_integral_test.test | RUN | 0.134662 | 0.425291 | 0.774388 | 0.001676 | 0.001596 | 0.001692 |
| is_fundamental_test.test | RUN | 0.136132 | 0.432978 | 0.752891 | 0.001546 | 0.001772 | 0.001552 |
| is_function_test.test | RUN | 0.110165 | 0.276037 | 0.438447 | 0.001407 | 0.001774 | 0.001493 |
| is_floating_point_test.test | RUN | 0.117022 | 0.293425 | 0.492411 | 0.001487 | 0.001639 | 0.001309 |
| is_float_test.test | RUN | 0.113406 | 0.300927 | 0.500935 | 0.001582 | 0.001626 | 0.001625 |
| is_final_test.test | RUN | 0.114809 | 0.312502 | 0.506728 | 0.001382 | 0.001677 | 0.001406 |
| is_enum_test.test | RUN | 0.103735 | 0.247885 | 0.392441 | 0.001337 | 0.001278 | 0.001468 |
| is_empty_test.test | RUN | 0.115901 | 0.284365 | 0.447105 | 0.001316 | 0.001634 | 0.001266 |
| is_detected_test.test | RUN | 0.103749 | 0.243151 | 0.357550 | 0.001643 | 0.001832 | 0.001698 |
| is_detected_exact_test.test | RUN | 0.103938 | 0.244641 | 0.358518 | 0.001496 | 0.001431 | 0.001406 |
| is_destructible_test.test | RUN | 0.146656 | 0.471481 | 0.879933 | 0.001467 | 0.001800 | 0.001660 |
| is_default_constr_test.test | RUN | 0.151692 | 0.488787 | 0.895893 | 0.001476 | 0.001689 | 0.001450 |
| is_copy_constructible_test.test | RUN | 0.158039 | 0.508043 | 0.957592 | 0.001612 | 0.001786 | 0.001249 |
| is_copy_assignable_test.test | RUN | 0.156433 | 0.501235 | 0.957424 | 0.001313 | 0.001518 | 0.001411 |
| is_convertible_test.test | RUN | 0.149199 | 0.453268 | 0.775220 | 0.001546 | 0.001806 | 0.001735 |
| is_constructible_test.test | RUN | 0.123848 | 0.326506 | 0.526978 | 0.001506 | 0.001586 | 0.001704 |
| is_const_test.test | RUN | 0.110814 | 0.284391 | 0.453874 | 0.001400 | 0.001396 | 0.001662 |
| is_compound_test.test | RUN | 0.103452 | 0.241136 | 0.341563 | 0.001635 | 0.001735 | 0.001424 |
| is_complex_test.test | RUN | 0.139183 | 0.328229 | 0.533404 | 0.001634 | 0.001778 | 0.001781 |
| is_complete_test.test | RUN | 0.121871 | 0.342476 | 0.544406 | 0.001492 | 0.001864 | 0.001619 |
| is_class_test.test | RUN | 0.110717 | 0.290784 | 0.487173 | 0.001344 | 0.001411 | 0.001582 |
| is_bounded_array_test.test | RUN | 0.104624 | 0.272909 | 0.418532 | 0.001222 | 0.001624 | 0.001500 |
| is_base_of_test.test | RUN | 0.116443 | 0.293156 | 0.446111 | 0.001292 | 0.001823 | 0.001708 |
| is_base_and_derived_test.test | RUN | 0.115750 | 0.285272 | 0.438618 | 0.001576 | 0.001706 | 0.001700 |
| is_assignable_test.test | RUN | 0.136188 | 0.405525 | 0.713334 | 0.001370 | 0.001748 | 0.001438 |
| is_array_test.test | RUN | 0.110128 | 0.284672 | 0.468251 | 0.001484 | 0.001618 | 0.001629 |
| is_arithmetic_test.test | RUN | 0.140701 | 0.407240 | 0.764913 | 0.001561 | 0.001794 | 0.001668 |
| is_abstract_test.test | RUN | 0.246592 | 1.136138 | 2.560548 | 0.001408 | 0.001711 | 0.001396 |
| integral_constant_test.test | RUN | 0.017224 | 0.046351 | 0.046592 | 0.000700 | 0.000667 | 0.001446 |
| has_virtual_destructor_test.test | RUN | 0.119381 | 0.317933 | 0.557011 | 0.001456 | 0.001948 | 0.001684 |
| has_unary_plus_test.test | RUN | 0.227194 | 1.220737 | 1.939830 | 0.001605 | 0.001973 | 0.001667 |
| has_unary_minus_test.test | RUN | 0.224211 | 1.242190 | 1.926618 | 0.001371 | 0.001668 | 0.001470 |
| has_trivial_move_constructor_test.test | RUN | 0.154716 | 0.493844 | 0.943607 | 0.001668 | 0.001687 | 0.001715 |
| has_trivial_move_assign_test.test | RUN | 0.153961 | 0.462183 | 0.925959 | 0.001325 | 0.001624 | 0.001594 |
| has_trivial_destructor_test.test | RUN | 0.154807 | 0.484670 | 0.927513 | 0.001752 | 0.001792 | 0.001673 |
| has_trivial_copy_test.test | RUN | 0.158455 | 0.515316 | 0.985156 | 0.001401 | 0.001901 | 0.001711 |
| has_trivial_constr_test.test | RUN | 0.157364 | 0.536805 | 0.988749 | 0.001649 | 0.001637 | 0.001512 |
| has_trivial_assign_test.test | RUN | 0.158023 | 0.520382 | 0.922822 | 0.001487 | 0.001719 | 0.001727 |
| has_right_shift_test.test | RUN | 0.235935 | 0.971841 | 2.030796 | 0.001425 | 0.001824 | 0.001513 |
| has_right_shift_assign_test.test | RUN | 0.228793 | 0.909168 | 1.935816 | 0.001592 | 0.001775 | 0.001745 |
| has_prefix_classes3_test.test | RUN | 0.170092 | 0.548084 | 1.020896 | 0.001656 | 0.001676 | 0.001644 |
| has_prefix_classes2_test.test | RUN | 0.169998 | 0.534971 | 0.996126 | 0.001433 | 0.001746 | 0.001654 |
| has_prefix_classes1_test.test | RUN | 0.161423 | 0.545172 | 0.987482 | 0.001375 | 0.001790 | 0.001679 |
| has_prefix_classes0_test.test | RUN | 0.162342 | 0.537765 | 0.943146 | 0.001571 | 0.001707 | 0.001718 |
| has_pre_increment_test.test | RUN | 0.225016 | 1.248768 | 1.915224 | 0.001672 | 0.001694 | 0.001733 |
| has_pre_decrement_test.test | RUN | 0.222698 | 1.262414 | 1.967076 | 0.001376 | 0.001810 | 0.001280 |
| has_postfix_classes3_test.test | RUN | 0.169710 | 0.515052 | 1.039757 | 0.001474 | 0.001487 | 0.001582 |
| has_postfix_classes2_test.test | RUN | 0.165401 | 0.517761 | 1.017106 | 0.001269 | 0.001558 | 0.001492 |
| has_postfix_classes1_test.test | RUN | 0.168825 | 0.545463 | 0.999750 | 0.001375 | 0.001871 | 0.001731 |
| has_postfix_classes0_test.test | RUN | 0.162787 | 0.543972 | 0.989377 | 0.001726 | 0.001798 | 0.001629 |
| has_post_increment_test.test | RUN | 0.219758 | 1.253728 | 1.977680 | 0.001449 | 0.001758 | 0.001616 |
| has_post_decrement_test.test | RUN | 0.220409 | 1.280187 | 1.979056 | 0.001449 | 0.001700 | 0.001491 |
| has_plus_test.test | RUN | 0.226077 | 0.899674 | 1.961961 | 0.001653 | 0.001620 | 0.001579 |
| has_plus_assign_test.test | RUN | 0.232926 | 0.887354 | 1.967668 | 0.001391 | 0.001795 | 0.001744 |
| has_operator_new_test.test | RUN | 0.148726 | 0.468950 | 0.898380 | 0.001557 | 0.001790 | 0.001326 |
| has_nothrow_destructor_test.test | RUN | 0.157501 | 0.524996 | 0.988657 | 0.001502 | 0.001750 | 0.001697 |
| has_nothrow_copy_test.test | RUN | 0.152235 | 0.499650 | 0.927464 | 0.001361 | 0.001721 | 0.001764 |
| has_nothrow_constr_test.test | RUN | 0.147675 | 0.476021 | 0.890042 | 0.001301 | 0.001499 | 0.001636 |
| has_nothrow_assign_test.test | RUN | 0.151988 | 0.476237 | 0.917282 | 0.001567 | 0.001719 | 0.001640 |
| has_not_equal_to_test.test | RUN | 0.226620 | 1.207802 | 1.996230 | 0.001380 | 0.001622 | 0.001683 |
| has_negate_test.test | RUN | 0.217521 | 0.881198 | 1.889947 | 0.001579 | 0.001707 | 0.001560 |
| has_multiplies_test.test | RUN | 0.223094 | 0.919904 | 1.874093 | 0.001472 | 0.001700 | 0.001737 |
| has_multiplies_assign_test.test | RUN | 0.205414 | 0.898287 | 2.014520 | 0.001539 | 0.001847 | 0.001642 |
| has_modulus_test.test | RUN | 0.206111 | 0.913327 | 1.924733 | 0.001439 | 0.001665 | 0.001657 |
| has_modulus_assign_test.test | RUN | 0.225466 | 0.926501 | 1.973503 | 0.001613 | 0.001587 | 0.001646 |
| has_minus_test.test | RUN | 0.235716 | 0.905958 | 1.968880 | 0.001675 | 0.001701 | 0.001636 |
| has_minus_assign_test.test | RUN | 0.236178 | 0.906252 | 1.961681 | 0.001587 | 0.001723 | 0.001636 |
| has_logical_or_test.test | RUN | 0.231017 | 0.914134 | 2.003275 | 0.001602 | 0.001697 | 0.001716 |
| has_logical_not_test.test | RUN | 0.225492 | 0.890279 | 1.827706 | 0.001624 | 0.001522 | 0.001661 |
| has_logical_and_test.test | RUN | 0.222916 | 0.904118 | 1.949578 | 0.001627 | 0.001832 | 0.001662 |
| has_less_test.test | RUN | 0.226843 | 1.208613 | 2.036531 | 0.001623 | 0.001780 | 0.001678 |
| has_less_equal_test.test | RUN | 0.228097 | 1.217204 | 1.901187 | 0.001477 | 0.001508 | 0.001632 |
| has_left_shift_test.test | RUN | 0.248919 | 1.135937 | 2.410441 | 0.001512 | 0.001710 | 0.001606 |
| has_left_shift_assign_test.test | RUN | 0.227212 | 0.889687 | 1.972724 | 0.001730 | 0.001564 | 0.001562 |
| has_greater_test.test | RUN | 0.228302 | 1.230870 | 1.960905 | 0.001668 | 0.001751 | 0.001767 |
| has_greater_equal_test.test | RUN | 0.227761 | 1.187148 | 1.950231 | 0.001440 | 0.001671 | 0.001654 |
| has_equal_to_test.test | RUN | 0.230954 | 1.196221 | 2.016386 | 0.001354 | 0.001741 | 0.001580 |
| has_divides_test.test | RUN | 0.226303 | 0.904849 | 1.932703 | 0.001695 | 0.001726 | 0.001357 |
| has_divides_assign_test.test | RUN | 0.227965 | 0.876989 | 1.944616 | 0.001706 | 0.001777 | 0.001665 |
| has_dereference_test.test | RUN | 0.218181 | 0.881135 | 1.850420 | 0.001702 | 0.001756 | 0.001654 |
| has_complement_test.test | RUN | 0.226106 | 1.267827 | 1.952465 | 0.001414 | 0.001628 | 0.001435 |
| has_bit_xor_test.test | RUN | 0.222869 | 0.872116 | 1.915518 | 0.001469 | 0.001819 | 0.001562 |
| has_bit_xor_assign_test.test | RUN | 0.231819 | 0.873466 | 1.936483 | 0.001675 | 0.001865 | 0.001711 |
| has_bit_or_test.test | RUN | 0.227615 | 0.902983 | 1.906693 | 0.001703 | 0.001816 | 0.001635 |
| has_bit_or_assign_test.test | RUN | 0.224717 | 0.887411 | 1.929719 | 0.001634 | 0.001835 | 0.001766 |
| has_bit_and_test.test | RUN | 0.223966 | 0.882260 | 1.956543 | 0.001602 | 0.001747 | 0.001573 |
| has_bit_and_assign_test.test | RUN | 0.226774 | 0.953545 | 1.950700 | 0.001618 | 0.001804 | 0.001687 |
| has_binary_classes9_test.test | RUN | 0.248217 | 1.120716 | 2.139035 | 0.001600 | 0.001660 | 0.001728 |
| has_binary_classes8_test.test | RUN | 0.246023 | 1.136567 | 2.151764 | 0.001774 | 0.001813 | 0.001381 |
| has_binary_classes7_test.test | RUN | 0.244315 | 1.041887 | 2.110794 | 0.001413 | 0.001838 | 0.001647 |
| has_binary_classes6_test.test | RUN | 0.240219 | 1.013427 | 2.157097 | 0.001694 | 0.001734 | 0.001666 |
| has_binary_classes5_test.test | RUN | 0.247969 | 1.032184 | 2.126165 | 0.001721 | 0.001708 | 0.001651 |
| has_binary_classes4_test.test | RUN | 0.249387 | 1.078108 | 2.201114 | 0.001712 | 0.001758 | 0.001626 |
| has_binary_classes3_test.test | RUN | 0.247745 | 1.067332 | 2.161349 | 0.001489 | 0.001590 | 0.001718 |
| has_binary_classes2_test.test | RUN | 0.249427 | 1.061785 | 2.054782 | 0.001612 | 0.001806 | 0.001666 |
| has_binary_classes1_test.test | RUN | 0.252279 | 1.047639 | 2.066787 | 0.001723 | 0.001547 | 0.001544 |
| has_binary_classes0_test.test | RUN | 0.252437 | 1.097342 | 2.120613 | 0.001304 | 0.001667 | 0.001706 |
| function_traits_test.test | RUN | 0.111901 | 0.273057 | 0.470636 | 0.001703 | 0.001641 | 0.001570 |
| floating_point_promotion_test.test | RUN | 0.107558 | 0.270066 | 0.437944 | 0.001499 | 0.001640 | 0.001553 |
| extent_test.test | RUN | 0.113344 | 0.283999 | 0.468878 | 0.001696 | 0.001526 | 0.001530 |
| enable_if_test.test | RUN | 0.104993 | 0.242094 | 0.306379 | 0.001550 | 0.001514 | 0.001592 |
| disjunction_test.test | RUN | 0.106223 | 0.248604 | 0.364472 | 0.001277 | 0.001526 | 0.001471 |
| detected_test.test | RUN | 0.099869 | 0.229235 | 0.337267 | 0.001457 | 0.001725 | 0.001387 |
| detected_or_test.test | RUN | 0.104768 | 0.245720 | 0.384772 | 0.001603 | 0.001648 | 0.001770 |
| decay_test.test | RUN | 0.276764 | 0.961547 | 2.272456 | 0.001632 | 0.001641 | 0.001323 |
| cxx14_aliases_test.test | RUN | 0.172446 | 0.368493 | 0.587088 | 0.001501 | 0.001842 | 0.001396 |
| copy_reference_test.test | RUN | 0.103968 | 0.249644 | 0.381363 | 0.001634 | 0.001735 | 0.001508 |
| copy_cv_test.test | RUN | 0.110745 | 0.259019 | 0.430219 | 0.001334 | 0.002060 | 0.001722 |
| copy_cv_ref_test.test | RUN | 0.152923 | 0.444260 | 0.907972 | 0.001505 | 0.001612 | 0.001684 |
| conjunction_test.test | RUN | 0.104217 | 0.258591 | 0.384978 | 0.001335 | 0.001327 | 0.001636 |
| conditional_test.test | RUN | 0.102665 | 0.242483 | 0.375286 | 0.001294 | 0.001608 | 0.001669 |
| common_type_test.test | RUN | 0.143368 | 0.382789 | 0.726551 | 0.001713 | 0.001561 | 0.001275 |
| common_type_sfinae_test.test | RUN | 0.103229 | 0.232480 | 0.357833 | 0.001507 | 0.001498 | 0.001408 |
| common_type_sfinae2_test.test | RUN | 0.104827 | 0.255824 | 0.367369 | 0.001575 | 0.001502 | 0.001510 |
| common_type_6_test.test | RUN | 0.107468 | 0.257635 | 0.389381 | 0.001388 | 0.001594 | 0.001692 |
| common_type_5_test.test | RUN | 0.112204 | 0.249230 | 0.379498 | 0.001678 | 0.001431 | 0.001492 |
| common_type_4_test.test | RUN | 0.111461 | 0.271297 | 0.455386 | 0.001456 | 0.001498 | 0.001562 |
| common_type_3_test.test | RUN | 0.101512 | 0.238685 | 0.315456 | 0.001767 | 0.001439 | 0.001618 |
| common_type_2_test.test | RUN | 0.139486 | 0.377230 | 0.717308 | 0.001489 | 0.001790 | 0.001402 |
| alignment_of_test.test | RUN | 0.113953 | 0.301772 | 0.463120 | 0.001550 | 0.001718 | 0.001583 |
| alignment_of_a2_test.test | RUN | 0.113093 | 0.296834 | 0.465151 | 0.001589 | 0.001457 | 0.001446 |
| aligned_storage_test.test | RUN | 0.115730 | 0.288653 | 0.428941 | 0.001620 | 0.001672 | 0.001564 |
| aligned_storage_empy_test.test | RUN | 0.134305 | 0.334591 | 0.566046 | 0.001518 | 0.001822 | 0.001595 |
| aligned_storage_a2_test.test | RUN | 0.117517 | 0.284664 | 0.406162 | 0.001478 | 0.001926 | 0.001668 |
| add_volatile_test.test | RUN | 0.245696 | 0.864969 | 2.163800 | 0.001678 | 0.001479 | 0.001689 |
| add_rvalue_reference_test.test | RUN | 0.273733 | 0.989602 | 2.504201 | 0.001534 | 0.001476 | 0.001615 |
| add_reference_test.test | RUN | 0.266912 | 0.952470 | 2.433449 | 0.001311 | 0.001806 | 0.001642 |
| add_pointer_test.test | RUN | 0.176079 | 0.536742 | 1.275168 | 0.001225 | 0.001662 | 0.001500 |
| add_lvalue_reference_test.test | RUN | 0.264897 | 0.938565 | 2.476424 | 0.001582 | 0.001623 | 0.001494 |
| add_cv_test.test | RUN | 0.254474 | 0.875276 | 2.340188 | 0.001487 | 0.001544 | 0.001366 |
| add_const_test.test | RUN | 0.253952 | 0.924741 | 2.277773 | 0.001531 | 0.001487 | 0.001605 |
| is_virtual_base_of_fail.test | COMPILE_FAIL | 0.102052 | 0.234398 | 0.336188 | — | — | — |
| is_stateless_fail.test | COMPILE_FAIL | 0.102929 | 0.240315 | 0.352972 | — | — | — |
| is_polymorphic_fail.test | COMPILE_FAIL | 0.098350 | 0.217167 | 0.326562 | — | — | — |
| is_nothrow_move_constructible_fail.test | COMPILE_FAIL | 0.099127 | 0.222904 | 0.345881 | — | — | — |
| is_nothrow_move_assignable_fail.test | COMPILE_FAIL | 0.099676 | 0.227297 | 0.334343 | — | — | — |
| is_list_constructible_fail.test | COMPILE_FAIL | 0.099495 | 0.224985 | 0.328627 | — | — | — |
| is_empty_fail.test | COMPILE_FAIL | 0.101131 | 0.225611 | 0.302047 | — | — | — |
| is_destructible_fail.test | COMPILE_FAIL | 0.101446 | 0.224732 | 0.331644 | — | — | — |
| is_default_constructible_fail.test | COMPILE_FAIL | 0.100245 | 0.227912 | 0.332103 | — | — | — |
| is_copy_constructible_fail.test | COMPILE_FAIL | 0.100047 | 0.223030 | 0.331151 | — | — | — |
| is_copy_assignable_fail.test | COMPILE_FAIL | 0.099507 | 0.221720 | 0.332931 | — | — | — |
| is_convertible_fail.test | COMPILE_FAIL | 0.098355 | 0.230149 | 0.327673 | — | — | — |
| is_convertible2_fail.test | COMPILE_FAIL | 0.099831 | 0.231084 | 0.328121 | — | — | — |
| is_constructible_fail.test | COMPILE_FAIL | 0.099483 | 0.225559 | 0.330552 | — | — | — |
| is_base_of_fail.test | COMPILE_FAIL | 0.098913 | 0.224384 | 0.325454 | — | — | — |
| is_assignable_fail.test | COMPILE_FAIL | 0.098838 | 0.221560 | 0.333340 | — | — | — |
| has_nothrow_destruct_fail.test | COMPILE_FAIL | 0.099540 | 0.227203 | 0.328114 | — | — | — |
| has_nothrow_copy_fail.test | COMPILE_FAIL | 0.099966 | 0.225549 | 0.336506 | — | — | — |
| has_nothrow_cons_fail.test | COMPILE_FAIL | 0.098990 | 0.224103 | 0.329717 | — | — | — |
| has_nothrow_assign_fail.test | COMPILE_FAIL | 0.099635 | 0.221879 | 0.327836 | — | — | — |
| common_type_fail.test | COMPILE_FAIL | 0.100216 | 0.224682 | 0.325356 | — | — | — |
| common_type2_fail.test | COMPILE_FAIL | 0.099242 | 0.226462 | 0.332851 | — | — | — |
| has_nothrow_assign_test_no_intrinsics.test | RUN | 0.155448 | 0.491734 | 0.941733 | 0.001676 | 0.001716 | 0.001677 |
| has_nothrow_constr_test_no_intrinsics.test | RUN | 0.149395 | 0.483937 | 0.926595 | 0.001468 | 0.001858 | 0.001628 |
| has_nothrow_copy_test_no_intrinsics.test | RUN | 0.155392 | 0.484948 | 0.896713 | 0.001340 | 0.001657 | 0.001485 |
| is_nothrow_move_assignable_test_no_intrinsics.test | RUN | 0.156031 | 0.480415 | 0.935357 | 0.001227 | 0.001188 | 0.001247 |
| is_nothrow_move_constructible_test_no_intrinsics.test | RUN | 0.098362 | 0.340709 | 0.598944 | 0.001251 | 0.001248 | 0.001174 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.013×† | 0.988×, 1.038× | 0.953× |
| clang | 1.050×† | 1.068×, 1.032× | 1.057× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224918).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| add_const_test.test | 10 | 0.001003s | 0.000993s | 0.001123s | byte equality observed |
| add_cv_test.test | 10 | 0.000995s | 0.000981s | 0.001123s | byte equality observed |
| add_lvalue_reference_test.test | 10 | 0.000988s | 0.000984s | 0.001127s | byte equality observed |
| add_pointer_test.test | 10 | 0.001015s | 0.000962s | 0.001129s | byte equality observed |
| add_reference_test.test | 10 | 0.001003s | 0.001028s | 0.001131s | byte equality observed |
| add_rvalue_reference_test.test | 10 | 0.001016s | 0.000997s | 0.001115s | byte equality observed |
| add_volatile_test.test | 10 | 0.000983s | 0.001001s | 0.001110s | byte equality observed |
| aligned_storage_a2_test.test | 10 | 0.001080s | 0.001074s | 0.001238s | byte equality observed |
| aligned_storage_empy_test.test | 10 | 0.001007s | 0.000977s | 0.001135s | byte equality observed |
| aligned_storage_test.test | 10 | 0.001107s | 0.001076s | 0.001245s | byte equality observed |
| alignment_of_a2_test.test | 10 | 0.000989s | 0.001006s | 0.001128s | byte equality observed |
| alignment_of_test.test | 10 | 0.000992s | 0.000979s | 0.001136s | byte equality observed |
| common_type_2_test.test | 10 | 0.000977s | 0.000974s | 0.001120s | byte equality observed |
| common_type_3_test.test | 10 | 0.001007s | 0.001045s | 0.001116s | byte equality observed |
| common_type_4_test.test | 10 | 0.000976s | 0.001027s | 0.001108s | byte equality observed |
| common_type_5_test.test | 10 | 0.001001s | 0.000976s | 0.001127s | byte equality observed |
| common_type_6_test.test | 10 | 0.000969s | 0.001031s | 0.001104s | byte equality observed |
| common_type_sfinae2_test.test | 10 | 0.000986s | 0.000979s | 0.001077s | byte equality observed |
| common_type_sfinae_test.test | 10 | 0.000992s | 0.000957s | 0.001122s | byte equality observed |
| common_type_test.test | 10 | 0.000994s | 0.000998s | 0.001134s | byte equality observed |
| conditional_test.test | 10 | 0.001024s | 0.000969s | 0.001120s | byte equality observed |
| conjunction_test.test | 10 | 0.001010s | 0.000987s | 0.001120s | byte equality observed |
| copy_cv_ref_test.test | 10 | 0.000998s | 0.000966s | 0.001086s | byte equality observed |
| copy_cv_test.test | 10 | 0.001015s | 0.001006s | 0.001124s | byte equality observed |
| copy_reference_test.test | 10 | 0.001005s | 0.000957s | 0.001118s | byte equality observed |
| cxx14_aliases_test.test | 10 | 0.001000s | 0.000978s | 0.001128s | byte equality observed |
| decay_test.test | 10 | 0.001003s | 0.001062s | 0.001129s | byte equality observed |
| detected_or_test.test | 10 | 0.000988s | 0.001065s | 0.001101s | byte equality observed |
| detected_test.test | 10 | 0.001009s | 0.001049s | 0.001125s | byte equality observed |
| disjunction_test.test | 10 | 0.000977s | 0.001057s | 0.001096s | byte equality observed |
| enable_if_test.test | 10 | 0.001038s | 0.001042s | 0.001154s | byte equality observed |
| extent_test.test | 10 | 0.001022s | 0.001071s | 0.001135s | byte equality observed |
| floating_point_promotion_test.test | 10 | 0.000997s | 0.001049s | 0.001128s | byte equality observed |
| function_traits_test.test | 10 | 0.000999s | 0.001033s | 0.001115s | byte equality observed |
| has_binary_classes0_test.test | 10 | 0.001012s | 0.001082s | 0.001123s | byte equality observed |
| has_binary_classes1_test.test | 10 | 0.001013s | 0.001068s | 0.001153s | byte equality observed |
| has_binary_classes2_test.test | 10 | 0.000997s | 0.001072s | 0.001122s | byte equality observed |
| has_binary_classes3_test.test | 10 | 0.001019s | 0.001072s | 0.001133s | byte equality observed |
| has_binary_classes4_test.test | 10 | 0.001022s | 0.001039s | 0.001125s | byte equality observed |
| has_binary_classes5_test.test | 10 | 0.000989s | 0.001103s | 0.001145s | byte equality observed |
| has_binary_classes6_test.test | 10 | 0.001006s | 0.001048s | 0.001107s | byte equality observed |
| has_binary_classes7_test.test | 10 | 0.000980s | 0.001071s | 0.001109s | byte equality observed |
| has_binary_classes8_test.test | 10 | 0.001015s | 0.001045s | 0.001132s | byte equality observed |
| has_binary_classes9_test.test | 10 | 0.001024s | 0.001060s | 0.001119s | byte equality observed |
| has_bit_and_assign_test.test | 10 | 0.001012s | 0.001070s | 0.001140s | byte equality observed |
| has_bit_and_test.test | 10 | 0.001032s | 0.001063s | 0.001118s | byte equality observed |
| has_bit_or_assign_test.test | 10 | 0.001025s | 0.001078s | 0.001099s | byte equality observed |
| has_bit_or_test.test | 10 | 0.000992s | 0.001071s | 0.001140s | byte equality observed |
| has_bit_xor_assign_test.test | 10 | 0.001008s | 0.001048s | 0.001113s | byte equality observed |
| has_bit_xor_test.test | 10 | 0.001017s | 0.001070s | 0.001126s | byte equality observed |
| has_complement_test.test | 10 | 0.001022s | 0.001051s | 0.001124s | byte equality observed |
| has_dereference_test.test | 10 | 0.001017s | 0.001073s | 0.001107s | byte equality observed |
| has_divides_assign_test.test | 10 | 0.001015s | 0.001068s | 0.001124s | byte equality observed |
| has_divides_test.test | 10 | 0.001024s | 0.001037s | 0.001136s | byte equality observed |
| has_equal_to_test.test | 10 | 0.000997s | 0.001064s | 0.001133s | byte equality observed |
| has_greater_equal_test.test | 10 | 0.000990s | 0.001056s | 0.001135s | byte equality observed |
| has_greater_test.test | 10 | 0.001014s | 0.001079s | 0.001138s | byte equality observed |
| has_left_shift_assign_test.test | 10 | 0.000990s | 0.001065s | 0.001118s | byte equality observed |
| has_left_shift_test.test | 10 | 0.001016s | 0.001060s | 0.001125s | byte equality observed |
| has_less_equal_test.test | 10 | 0.000984s | 0.001033s | 0.001128s | byte equality observed |
| has_less_test.test | 10 | 0.000998s | 0.001060s | 0.001148s | byte equality observed |
| has_logical_and_test.test | 10 | 0.001007s | 0.001023s | 0.001117s | byte equality observed |
| has_logical_not_test.test | 10 | 0.000994s | 0.001092s | 0.001133s | byte equality observed |
| has_logical_or_test.test | 10 | 0.001007s | 0.001102s | 0.001144s | byte equality observed |
| has_minus_assign_test.test | 10 | 0.001015s | 0.001093s | 0.001115s | byte equality observed |
| has_minus_test.test | 10 | 0.001027s | 0.001032s | 0.001122s | byte equality observed |
| has_modulus_assign_test.test | 10 | 0.001028s | 0.001080s | 0.001093s | byte equality observed |
| has_modulus_test.test | 10 | 0.000994s | 0.001046s | 0.001120s | byte equality observed |
| has_multiplies_assign_test.test | 10 | 0.001019s | 0.001041s | 0.001122s | byte equality observed |
| has_multiplies_test.test | 10 | 0.001005s | 0.001038s | 0.001120s | byte equality observed |
| has_negate_test.test | 10 | 0.001031s | 0.001071s | 0.001129s | byte equality observed |
| has_not_equal_to_test.test | 10 | 0.001004s | 0.001070s | 0.001116s | byte equality observed |
| has_nothrow_assign_test.test | 10 | 0.000969s | 0.001050s | 0.001119s | byte equality observed |
| has_nothrow_assign_test_no_intrinsics.test | 10 | 0.001041s | 0.001049s | 0.001100s | byte equality observed |
| has_nothrow_constr_test.test | 10 | 0.001011s | 0.001051s | 0.001111s | byte equality observed |
| has_nothrow_constr_test_no_intrinsics.test | 10 | 0.000981s | 0.001070s | 0.001106s | byte equality observed |
| has_nothrow_copy_test.test | 10 | 0.001015s | 0.001065s | 0.001125s | byte equality observed |
| has_nothrow_copy_test_no_intrinsics.test | 10 | 0.001013s | 0.001123s | 0.001106s | byte equality observed |
| has_nothrow_destructor_test.test | 10 | 0.001031s | 0.001098s | 0.001118s | byte equality observed |
| has_operator_new_test.test | 10 | 0.001032s | 0.001135s | 0.001127s | byte equality observed |
| has_plus_assign_test.test | 10 | 0.001062s | 0.001124s | 0.001127s | byte equality observed |
| has_plus_test.test | 10 | 0.001081s | 0.001118s | 0.001114s | byte equality observed |
| has_post_decrement_test.test | 10 | 0.001068s | 0.001118s | 0.001122s | byte equality observed |
| has_post_increment_test.test | 10 | 0.001021s | 0.001118s | 0.001133s | byte equality observed |
| has_postfix_classes0_test.test | 10 | 0.001058s | 0.001118s | 0.001099s | byte equality observed |
| has_postfix_classes1_test.test | 10 | 0.001077s | 0.001143s | 0.001131s | byte equality observed |
| has_postfix_classes2_test.test | 10 | 0.001068s | 0.001114s | 0.001099s | byte equality observed |
| has_postfix_classes3_test.test | 10 | 0.001068s | 0.001097s | 0.001109s | byte equality observed |
| has_pre_decrement_test.test | 10 | 0.001056s | 0.001143s | 0.001117s | byte equality observed |
| has_pre_increment_test.test | 10 | 0.001033s | 0.001127s | 0.001135s | byte equality observed |
| has_prefix_classes0_test.test | 10 | 0.001069s | 0.001143s | 0.001111s | byte equality observed |
| has_prefix_classes1_test.test | 10 | 0.001050s | 0.001130s | 0.001123s | byte equality observed |
| has_prefix_classes2_test.test | 10 | 0.001070s | 0.001156s | 0.001115s | byte equality observed |
| has_prefix_classes3_test.test | 10 | 0.001084s | 0.001145s | 0.001118s | byte equality observed |
| has_right_shift_assign_test.test | 10 | 0.001054s | 0.001108s | 0.001125s | byte equality observed |
| has_right_shift_test.test | 10 | 0.001104s | 0.001107s | 0.001099s | byte equality observed |
| has_trivial_assign_test.test | 10 | 0.001144s | 0.001178s | 0.001175s | byte equality observed |
| has_trivial_constr_test.test | 10 | 0.001120s | 0.001184s | 0.001163s | native test oracle; output varies |
| has_trivial_copy_test.test | 10 | 0.001156s | 0.001183s | 0.001211s | native test oracle; output varies |
| has_trivial_destructor_test.test | 10 | 0.001062s | 0.001112s | 0.001065s | byte equality observed |
| has_trivial_move_assign_test.test | 10 | 0.001087s | 0.001035s | 0.001061s | byte equality observed |
| has_trivial_move_constructor_test.test | 10 | 0.001104s | 0.001044s | 0.001041s | byte equality observed |
| has_unary_minus_test.test | 10 | 0.001123s | 0.001089s | 0.001064s | byte equality observed |
| has_unary_plus_test.test | 10 | 0.001136s | 0.001080s | 0.001046s | byte equality observed |
| has_virtual_destructor_test.test | 10 | 0.001118s | 0.001062s | 0.001072s | byte equality observed |
| integral_constant_test.test | 10 | 0.000497s | 0.000471s | 0.001059s | byte equality observed |
| is_abstract_test.test | 10 | 0.001092s | 0.001037s | 0.001078s | byte equality observed |
| is_arithmetic_test.test | 10 | 0.001101s | 0.001037s | 0.001123s | byte equality observed |
| is_array_test.test | 10 | 0.001119s | 0.001057s | 0.001107s | byte equality observed |
| is_assignable_test.test | 10 | 0.001104s | 0.001082s | 0.001107s | byte equality observed |
| is_base_and_derived_test.test | 10 | 0.001129s | 0.001050s | 0.001133s | byte equality observed |
| is_base_of_test.test | 10 | 0.001111s | 0.001074s | 0.001124s | byte equality observed |
| is_bounded_array_test.test | 10 | 0.001108s | 0.001061s | 0.001117s | byte equality observed |
| is_class_test.test | 10 | 0.001109s | 0.001067s | 0.001118s | byte equality observed |
| is_complete_test.test | 10 | 0.001117s | 0.001096s | 0.001107s | byte equality observed |
| is_complex_test.test | 10 | 0.001111s | 0.001042s | 0.001117s | byte equality observed |
| is_compound_test.test | 10 | 0.001103s | 0.001075s | 0.001104s | byte equality observed |
| is_const_test.test | 10 | 0.001111s | 0.001035s | 0.001138s | byte equality observed |
| is_constructible_test.test | 10 | 0.001135s | 0.001082s | 0.001143s | byte equality observed |
| is_convertible_test.test | 10 | 0.001107s | 0.001055s | 0.001114s | byte equality observed |
| is_copy_assignable_test.test | 10 | 0.001113s | 0.001121s | 0.001128s | byte equality observed |
| is_copy_constructible_test.test | 10 | 0.001106s | 0.001137s | 0.001132s | byte equality observed |
| is_default_constr_test.test | 10 | 0.001127s | 0.001123s | 0.001118s | byte equality observed |
| is_destructible_test.test | 10 | 0.001131s | 0.001095s | 0.001142s | byte equality observed |
| is_detected_exact_test.test | 10 | 0.001137s | 0.001137s | 0.001134s | byte equality observed |
| is_detected_test.test | 10 | 0.001109s | 0.001116s | 0.001140s | byte equality observed |
| is_empty_test.test | 10 | 0.001116s | 0.001119s | 0.001132s | byte equality observed |
| is_enum_test.test | 10 | 0.001137s | 0.001114s | 0.001130s | byte equality observed |
| is_final_test.test | 10 | 0.001120s | 0.001108s | 0.001124s | byte equality observed |
| is_float_test.test | 10 | 0.001119s | 0.001129s | 0.001126s | byte equality observed |
| is_floating_point_test.test | 10 | 0.001127s | 0.001127s | 0.001151s | byte equality observed |
| is_function_test.test | 10 | 0.001132s | 0.001105s | 0.001133s | byte equality observed |
| is_fundamental_test.test | 10 | 0.001111s | 0.001126s | 0.001130s | byte equality observed |
| is_integral_test.test | 10 | 0.001125s | 0.001114s | 0.001143s | byte equality observed |
| is_list_constructible_test.test | 10 | 0.001137s | 0.001105s | 0.001120s | byte equality observed |
| is_lvalue_reference_test.test | 10 | 0.001125s | 0.001134s | 0.001153s | byte equality observed |
| is_member_func_test.test | 10 | 0.001128s | 0.001121s | 0.001122s | byte equality observed |
| is_member_obj_test.test | 10 | 0.001128s | 0.001135s | 0.001143s | byte equality observed |
| is_member_pointer_test.test | 10 | 0.001128s | 0.001134s | 0.001148s | byte equality observed |
| is_noncopyable_test.test | 10 | 0.001123s | 0.001128s | 0.001139s | byte equality observed |
| is_nothrow_move_assignable_test.test | 10 | 0.001127s | 0.001119s | 0.001124s | byte equality observed |
| is_nothrow_move_assignable_test_no_intrinsics.test | 10 | 0.001134s | 0.001128s | 0.001131s | byte equality observed |
| is_nothrow_move_constructible_test.test | 10 | 0.001128s | 0.001137s | 0.001151s | byte equality observed |
| is_nothrow_move_constructible_test_no_intrinsics.test | 10 | 0.001135s | 0.001132s | 0.001128s | byte equality observed |
| is_nothrow_swappable_test.test | 10 | 0.001119s | 0.001138s | 0.001119s | byte equality observed |
| is_object_test.test | 10 | 0.001117s | 0.001112s | 0.001137s | byte equality observed |
| is_pod_test.test | 10 | 0.001139s | 0.001132s | 0.001137s | byte equality observed |
| is_pointer_test.test | 10 | 0.001126s | 0.001113s | 0.001140s | byte equality observed |
| is_polymorphic_test.test | 10 | 0.001123s | 0.001122s | 0.001148s | byte equality observed |
| is_reference_test.test | 10 | 0.001121s | 0.001111s | 0.001138s | byte equality observed |
| is_rvalue_reference_test.test | 10 | 0.001121s | 0.001110s | 0.001124s | byte equality observed |
| is_same_test.test | 10 | 0.001135s | 0.001115s | 0.001135s | byte equality observed |
| is_scalar_test.test | 10 | 0.001143s | 0.001114s | 0.001113s | byte equality observed |
| is_scoped_enum_test.test | 10 | 0.001124s | 0.001122s | 0.001126s | byte equality observed |
| is_signed_test.test | 10 | 0.001123s | 0.001125s | 0.001128s | byte equality observed |
| is_stateless_test.test | 10 | 0.001133s | 0.001138s | 0.001138s | byte equality observed |
| is_swappable_test.test | 10 | 0.001131s | 0.001137s | 0.001125s | byte equality observed |
| is_trivially_copyable_test.test | 10 | 0.001163s | 0.001171s | 0.001158s | byte equality observed |
| is_unbounded_array_test.test | 10 | 0.001128s | 0.001123s | 0.001130s | byte equality observed |
| is_union_test.test | 10 | 0.001135s | 0.001123s | 0.001125s | byte equality observed |
| is_unscoped_enum_test.test | 10 | 0.001122s | 0.001118s | 0.001122s | byte equality observed |
| is_unsigned_test.test | 10 | 0.001120s | 0.001140s | 0.001142s | byte equality observed |
| is_virtual_base_of_test.test | 10 | 0.001131s | 0.001124s | 0.001109s | byte equality observed |
| is_void_test.test | 10 | 0.001134s | 0.001148s | 0.001121s | byte equality observed |
| is_volatile_test.test | 10 | 0.001121s | 0.001114s | 0.001139s | byte equality observed |
| make_signed_test.test | 10 | 0.001137s | 0.001119s | 0.001123s | byte equality observed |
| make_unsigned_test.test | 10 | 0.001132s | 0.001135s | 0.001138s | byte equality observed |
| make_void_test.test | 10 | 0.001140s | 0.001142s | 0.001133s | byte equality observed |
| mpl_interop_test1.test | 10 | 0.000505s | 0.000511s | 0.001120s | byte equality observed |
| mpl_interop_test2.test | 10 | 0.000513s | 0.000514s | 0.001121s | byte equality observed |
| mpl_interop_test3.test | 10 | 0.000504s | 0.000507s | 0.001135s | byte equality observed |
| negation_test.test | 10 | 0.001173s | 0.001165s | 0.001128s | byte equality observed |
| promote_basic_test.test | 10 | 0.000513s | 0.000510s | 0.001139s | byte equality observed |
| promote_enum_msvc_bug_test.test | 10 | 0.000507s | 0.000502s | 0.001160s | byte equality observed |
| promote_enum_test.test | 10 | 0.000510s | 0.000512s | 0.001146s | byte equality observed |
| promote_mpl_test.test | 10 | 0.000510s | 0.000503s | 0.001141s | byte equality observed |
| rank_test.test | 10 | 0.001157s | 0.001152s | 0.001137s | byte equality observed |
| remove_all_extents_test.test | 10 | 0.001137s | 0.001114s | 0.001143s | byte equality observed |
| remove_bounds_test.test | 10 | 0.001139s | 0.001139s | 0.001118s | byte equality observed |
| remove_const_test.test | 10 | 0.001121s | 0.001146s | 0.001146s | byte equality observed |
| remove_cv_ref_test.test | 10 | 0.001122s | 0.001145s | 0.001151s | byte equality observed |
| remove_cv_test.test | 10 | 0.001128s | 0.001116s | 0.001132s | byte equality observed |
| remove_extent_test.test | 10 | 0.001151s | 0.001123s | 0.001135s | byte equality observed |
| remove_pointer_test.test | 10 | 0.001143s | 0.001126s | 0.001145s | byte equality observed |
| remove_reference_test.test | 10 | 0.001138s | 0.001134s | 0.001141s | byte equality observed |
| remove_volatile_test.test | 10 | 0.001132s | 0.001146s | 0.001139s | byte equality observed |
| tricky_abstract_type_test.test | 10 | 0.001122s | 0.001143s | 0.001134s | byte equality observed |
| tricky_add_pointer_test.test | 10 | 0.001123s | 0.001123s | 0.001123s | byte equality observed |
| tricky_function_type_test.test | 10 | 0.001135s | 0.001117s | 0.001145s | byte equality observed |
| tricky_incomplete_type_test.test | 10 | 0.001133s | 0.001137s | 0.001138s | byte equality observed |
| tricky_is_enum_test.test | 10 | 0.001149s | 0.001136s | 0.001124s | byte equality observed |
| tricky_partial_spec_test.test | 10 | 0.001138s | 0.001127s | 0.001137s | byte equality observed |
| tricky_rvalue_test.test | 10 | 0.001132s | 0.001142s | 0.001151s | byte equality observed |
| type_identity_test.test | 10 | 0.001139s | 0.001148s | 0.001136s | byte equality observed |
| type_traits_test.test | 10 | 0.000498s | 0.000502s | 0.001131s | byte equality observed |
| type_with_alignment_test.test | 10 | 0.001278s | 0.001267s | 0.001265s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224918).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
