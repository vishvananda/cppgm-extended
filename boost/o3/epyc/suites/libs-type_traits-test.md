# libs/type_traits/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| type_with_alignment_test.test | RUN | 0.092347 | 0.239101 | 0.329782 | 0.001676 | 0.001467 | 0.001678 |
| type_traits_test.test | RUN | 0.128230 | 0.261861 | 0.318959 | 0.000526 | 0.000564 | 0.001381 |
| type_identity_test.test | RUN | 0.224968 | 0.845074 | 1.681356 | 0.001729 | 0.001634 | 0.001680 |
| tricky_rvalue_test.test | RUN | 0.085637 | 0.191524 | 0.258842 | 0.001502 | 0.001550 | 0.001538 |
| tricky_partial_spec_test.test | RUN | 0.104068 | 0.282547 | 0.413936 | 0.001496 | 0.001510 | 0.001451 |
| tricky_is_enum_test.test | RUN | 0.084692 | 0.192286 | 0.246175 | 0.001414 | 0.001485 | 0.001407 |
| tricky_incomplete_type_test.test | RUN | 0.140684 | 0.296412 | 0.382965 | 0.001401 | 0.001576 | 0.001298 |
| tricky_function_type_test.test | RUN | 0.112621 | 0.296095 | 0.471276 | 0.001503 | 0.001535 | 0.001431 |
| tricky_add_pointer_test.test | RUN | 0.159699 | 0.502049 | 0.993104 | 0.001397 | 0.001862 | 0.001689 |
| tricky_abstract_type_test.test | RUN | 0.090510 | 0.197611 | 0.257248 | 0.001718 | 0.001455 | 0.001632 |
| remove_volatile_test.test | RUN | 0.212157 | 0.746097 | 1.546829 | 0.001542 | 0.001725 | 0.001644 |
| remove_reference_test.test | RUN | 0.220127 | 0.785030 | 1.649060 | 0.001696 | 0.001763 | 0.001605 |
| remove_pointer_test.test | RUN | 0.231260 | 0.822109 | 1.675835 | 0.001732 | 0.001583 | 0.001611 |
| remove_extent_test.test | RUN | 0.219408 | 0.776949 | 1.675914 | 0.002602 | 0.001716 | 0.001660 |
| remove_cv_test.test | RUN | 0.229221 | 0.777915 | 1.602844 | 0.001827 | 0.001670 | 0.001789 |
| remove_cv_ref_test.test | RUN | 0.477718 | 1.867389 | 3.883199 | 0.001160 | 0.001366 | 0.001495 |
| remove_const_test.test | RUN | 0.212621 | 0.737023 | 1.522826 | 0.001758 | 0.001701 | 0.001686 |
| remove_bounds_test.test | RUN | 0.226018 | 0.783964 | 1.628124 | 0.001387 | 0.001713 | 0.001475 |
| remove_all_extents_test.test | RUN | 0.216265 | 0.769424 | 1.538362 | 0.001638 | 0.001433 | 0.001375 |
| rank_test.test | RUN | 0.088378 | 0.202744 | 0.269580 | 0.001476 | 0.001704 | 0.001712 |
| promote_mpl_test.test | RUN | 0.072596 | 0.181541 | 0.199158 | 0.000595 | 0.000736 | 0.001538 |
| promote_enum_test.test | RUN | 0.017567 | 0.044593 | 0.052983 | 0.000558 | 0.000532 | 0.001638 |
| promote_enum_msvc_bug_test.test | RUN | 0.017545 | 0.041182 | 0.048043 | 0.000609 | 0.000709 | 0.001548 |
| promote_basic_test.test | RUN | 0.020981 | 0.052832 | 0.054966 | 0.000553 | 0.000532 | 0.001499 |
| negation_test.test | RUN | 0.084883 | 0.184415 | 0.243111 | 0.001377 | 0.001468 | 0.001517 |
| mpl_interop_test3.test | RUN | 0.066671 | 0.170827 | 0.184915 | 0.000664 | 0.000547 | 0.001332 |
| mpl_interop_test2.test | RUN | 0.025920 | 0.052463 | 0.059553 | 0.000553 | 0.000512 | 0.001650 |
| mpl_interop_test1.test | RUN | 0.014790 | 0.038952 | 0.041209 | 0.000526 | 0.000690 | 0.001643 |
| make_void_test.test | RUN | 0.123461 | 0.197696 | 0.257180 | 0.001536 | 0.001719 | 0.001661 |
| make_unsigned_test.test | RUN | 0.135419 | 0.254827 | 0.346155 | 0.001710 | 0.001456 | 0.001677 |
| make_signed_test.test | RUN | 0.124063 | 0.257412 | 0.351105 | 0.001428 | 0.001720 | 0.001672 |
| is_volatile_test.test | RUN | 0.096913 | 0.218566 | 0.306637 | 0.001334 | 0.001531 | 0.001846 |
| is_void_test.test | RUN | 0.094310 | 0.208498 | 0.272482 | 0.001493 | 0.001684 | 0.001350 |
| is_virtual_base_of_test.test | RUN | 0.105937 | 0.264751 | 0.393790 | 0.001466 | 0.001651 | 0.001369 |
| is_unsigned_test.test | RUN | 0.093663 | 0.219633 | 0.298076 | 0.001395 | 0.001526 | 0.001630 |
| is_unscoped_enum_test.test | RUN | 0.090562 | 0.195219 | 0.238872 | 0.001469 | 0.001310 | 0.001758 |
| is_union_test.test | RUN | 0.097965 | 0.250830 | 0.341961 | 0.001490 | 0.001690 | 0.001672 |
| is_unbounded_array_test.test | RUN | 0.089376 | 0.212449 | 0.296253 | 0.001374 | 0.001696 | 0.001682 |
| is_trivially_copyable_test.test | RUN | 0.139319 | 0.428373 | 0.681826 | 0.001561 | 0.001501 | 0.001372 |
| is_swappable_test.test | RUN | 0.111750 | 0.327107 | 0.524257 | 0.001475 | 0.001575 | 0.001435 |
| is_stateless_test.test | RUN | 0.130948 | 0.371916 | 0.591883 | 0.001451 | 0.001492 | 0.001678 |
| is_signed_test.test | RUN | 0.093462 | 0.222442 | 0.285482 | 0.001511 | 0.001474 | 0.001402 |
| is_scoped_enum_test.test | RUN | 0.089384 | 0.199674 | 0.252900 | 0.001438 | 0.001624 | 0.001480 |
| is_scalar_test.test | RUN | 0.116554 | 0.337845 | 0.554801 | 0.001402 | 0.001518 | 0.001578 |
| is_same_test.test | RUN | 0.091617 | 0.209359 | 0.268404 | 0.001547 | 0.001526 | 0.001727 |
| is_rvalue_reference_test.test | RUN | 0.098415 | 0.230953 | 0.363878 | 0.001494 | 0.001426 | 0.001309 |
| is_reference_test.test | RUN | 0.095840 | 0.229304 | 0.372933 | 0.001558 | 0.001661 | 0.001588 |
| is_polymorphic_test.test | RUN | 0.099291 | 0.241016 | 0.345262 | 0.001208 | 0.001671 | 0.001461 |
| is_pointer_test.test | RUN | 0.096590 | 0.229983 | 0.354576 | 0.001778 | 0.001490 | 0.001663 |
| is_pod_test.test | RUN | 0.124670 | 0.371929 | 0.589966 | 0.001494 | 0.001604 | 0.001663 |
| is_object_test.test | RUN | 0.088903 | 0.200584 | 0.264802 | 0.001432 | 0.001650 | 0.001750 |
| is_nothrow_swappable_test.test | RUN | 0.131229 | 0.345477 | 0.582106 | 0.001705 | 0.001614 | 0.001684 |
| is_nothrow_move_constructible_test.test | RUN | 0.125116 | 0.361675 | 0.588299 | 0.001687 | 0.001422 | 0.001329 |
| is_nothrow_move_assignable_test.test | RUN | 0.123399 | 0.360317 | 0.570251 | 0.001384 | 0.001625 | 0.001724 |
| is_noncopyable_test.test | RUN | 0.088212 | 0.203153 | 0.288242 | 0.001460 | 0.001450 | 0.001680 |
| is_member_pointer_test.test | RUN | 0.086617 | 0.205522 | 0.289676 | 0.001404 | 0.001742 | 0.001553 |
| is_member_obj_test.test | RUN | 0.088282 | 0.211643 | 0.271241 | 0.001427 | 0.001464 | 0.001428 |
| is_member_func_test.test | RUN | 0.088940 | 0.226716 | 0.317536 | 0.001324 | 0.001581 | 0.001595 |
| is_lvalue_reference_test.test | RUN | 0.092829 | 0.236064 | 0.323967 | 0.001436 | 0.001632 | 0.001548 |
| is_list_constructible_test.test | RUN | 0.109388 | 0.270587 | 0.361716 | 0.002012 | 0.001573 | 0.001598 |
| is_integral_test.test | RUN | 0.113165 | 0.326491 | 0.461735 | 0.001580 | 0.001656 | 0.001407 |
| is_fundamental_test.test | RUN | 0.114946 | 0.331710 | 0.493093 | 0.001587 | 0.001448 | 0.001258 |
| is_function_test.test | RUN | 0.091706 | 0.218921 | 0.299265 | 0.001498 | 0.001568 | 0.001446 |
| is_floating_point_test.test | RUN | 0.095782 | 0.239715 | 0.321071 | 0.001446 | 0.001566 | 0.001684 |
| is_float_test.test | RUN | 0.092947 | 0.241020 | 0.318866 | 0.001535 | 0.001627 | 0.001428 |
| is_final_test.test | RUN | 0.093155 | 0.240936 | 0.341156 | 0.001543 | 0.001394 | 0.001504 |
| is_enum_test.test | RUN | 0.088439 | 0.202758 | 0.274980 | 0.001516 | 0.001522 | 0.001339 |
| is_empty_test.test | RUN | 0.097887 | 0.219779 | 0.298462 | 0.001654 | 0.001603 | 0.001597 |
| is_detected_test.test | RUN | 0.085241 | 0.189895 | 0.250503 | 0.001516 | 0.001708 | 0.001656 |
| is_detected_exact_test.test | RUN | 0.085027 | 0.191714 | 0.227494 | 0.001435 | 0.001529 | 0.001572 |
| is_destructible_test.test | RUN | 0.120593 | 0.362490 | 0.591004 | 0.001364 | 0.001770 | 0.001653 |
| is_default_constr_test.test | RUN | 0.124389 | 0.375848 | 0.582362 | 0.001663 | 0.001530 | 0.001689 |
| is_copy_constructible_test.test | RUN | 0.125747 | 0.386844 | 0.634975 | 0.001452 | 0.001770 | 0.001950 |
| is_copy_assignable_test.test | RUN | 0.126185 | 0.379679 | 0.626479 | 0.001297 | 0.001426 | 0.001360 |
| is_convertible_test.test | RUN | 0.120788 | 0.343267 | 0.521009 | 0.001387 | 0.001815 | 0.001485 |
| is_constructible_test.test | RUN | 0.102084 | 0.256405 | 0.376020 | 0.001189 | 0.001638 | 0.001265 |
| is_const_test.test | RUN | 0.086881 | 0.223902 | 0.312346 | 0.001394 | 0.001295 | 0.001482 |
| is_compound_test.test | RUN | 0.084557 | 0.194553 | 0.263728 | 0.001396 | 0.001510 | 0.001306 |
| is_complex_test.test | RUN | 0.115706 | 0.255280 | 0.339412 | 0.001408 | 0.001708 | 0.001528 |
| is_complete_test.test | RUN | 0.103050 | 0.260036 | 0.370368 | 0.001458 | 0.001430 | 0.002219 |
| is_class_test.test | RUN | 0.093317 | 0.234377 | 0.326210 | 0.001391 | 0.001442 | 0.001415 |
| is_bounded_array_test.test | RUN | 0.088820 | 0.205152 | 0.280228 | 0.001492 | 0.001544 | 0.003006 |
| is_base_of_test.test | RUN | 0.094046 | 0.230970 | 0.380762 | 0.001347 | 0.001798 | 0.001492 |
| is_base_and_derived_test.test | RUN | 0.092285 | 0.228038 | 0.317157 | 0.001459 | 0.001431 | 0.001702 |
| is_assignable_test.test | RUN | 0.109283 | 0.316422 | 0.486657 | 0.001489 | 0.001593 | 0.001701 |
| is_array_test.test | RUN | 0.088909 | 0.215067 | 0.329390 | 0.001528 | 0.001482 | 0.001498 |
| is_arithmetic_test.test | RUN | 0.110153 | 0.317421 | 0.496280 | 0.001459 | 0.001745 | 0.001656 |
| is_abstract_test.test | RUN | 0.192028 | 0.879618 | 1.617014 | 0.001333 | 0.001708 | 0.001598 |
| integral_constant_test.test | RUN | 0.014359 | 0.036403 | 0.045515 | 0.000532 | 0.000642 | 0.001629 |
| has_virtual_destructor_test.test | RUN | 0.098186 | 0.246435 | 0.382437 | 0.001696 | 0.001754 | 0.001706 |
| has_unary_plus_test.test | RUN | 0.180676 | 0.914241 | 1.276072 | 0.001721 | 0.001675 | 0.001746 |
| has_unary_minus_test.test | RUN | 0.176433 | 0.927809 | 1.257594 | 0.001577 | 0.001620 | 0.001693 |
| has_trivial_move_constructor_test.test | RUN | 0.122816 | 0.386508 | 0.613157 | 0.001406 | 0.001464 | 0.001773 |
| has_trivial_move_assign_test.test | RUN | 0.123195 | 0.365264 | 0.630822 | 0.001819 | 0.001553 | 0.001653 |
| has_trivial_destructor_test.test | RUN | 0.131135 | 0.368880 | 0.604171 | 0.001764 | 0.001555 | 0.001683 |
| has_trivial_copy_test.test | RUN | 0.134071 | 0.399799 | 0.634148 | 0.001584 | 0.001548 | 0.001575 |
| has_trivial_constr_test.test | RUN | 0.131566 | 0.397157 | 0.648950 | 0.001448 | 0.001710 | 0.001790 |
| has_trivial_assign_test.test | RUN | 0.132901 | 0.398605 | 0.689645 | 0.001856 | 0.001776 | 0.001983 |
| has_right_shift_test.test | RUN | 0.192272 | 0.712022 | 1.437680 | 0.002177 | 0.001704 | 0.001486 |
| has_right_shift_assign_test.test | RUN | 0.181254 | 0.695230 | 1.258249 | 0.001443 | 0.001392 | 0.001695 |
| has_prefix_classes3_test.test | RUN | 0.137020 | 0.405191 | 0.737458 | 0.001417 | 0.001405 | 0.001594 |
| has_prefix_classes2_test.test | RUN | 0.136947 | 0.389992 | 0.688991 | 0.002766 | 0.001605 | 0.001452 |
| has_prefix_classes1_test.test | RUN | 0.152881 | 0.402900 | 0.708915 | 0.001592 | 0.001596 | 0.001634 |
| has_prefix_classes0_test.test | RUN | 0.150151 | 0.398532 | 0.635870 | 0.001477 | 0.001385 | 0.001664 |
| has_pre_increment_test.test | RUN | 0.187101 | 0.985289 | 1.235760 | 0.001730 | 0.001702 | 0.001560 |
| has_pre_decrement_test.test | RUN | 0.186612 | 0.984966 | 1.243673 | 0.001509 | 0.001483 | 0.001308 |
| has_postfix_classes3_test.test | RUN | 0.137290 | 0.401054 | 0.639653 | 0.001662 | 0.001624 | 0.001711 |
| has_postfix_classes2_test.test | RUN | 0.138806 | 0.394939 | 0.634635 | 0.001115 | 0.001434 | 0.001674 |
| has_postfix_classes1_test.test | RUN | 0.144332 | 0.405824 | 0.662421 | 0.001732 | 0.001736 | 0.001618 |
| has_postfix_classes0_test.test | RUN | 0.137359 | 0.405768 | 0.650845 | 0.001705 | 0.001594 | 0.001310 |
| has_post_increment_test.test | RUN | 0.179095 | 1.003924 | 1.322467 | 0.001423 | 0.001703 | 0.001565 |
| has_post_decrement_test.test | RUN | 0.180674 | 1.009565 | 1.319737 | 0.001697 | 0.001384 | 0.001680 |
| has_plus_test.test | RUN | 0.184753 | 0.716076 | 1.339580 | 0.001440 | 0.001627 | 0.001364 |
| has_plus_assign_test.test | RUN | 0.187359 | 0.726640 | 1.336517 | 0.001688 | 0.001545 | 0.001502 |
| has_operator_new_test.test | RUN | 0.121452 | 0.361429 | 0.594243 | 0.001700 | 0.001506 | 0.001586 |
| has_nothrow_destructor_test.test | RUN | 0.124605 | 0.404524 | 0.636185 | 0.001310 | 0.001455 | 0.001652 |
| has_nothrow_copy_test.test | RUN | 0.122775 | 0.378082 | 0.618138 | 0.001566 | 0.001582 | 0.001366 |
| has_nothrow_constr_test.test | RUN | 0.123068 | 0.362400 | 0.600622 | 0.001365 | 0.001413 | 0.001590 |
| has_nothrow_assign_test.test | RUN | 0.119662 | 0.401621 | 0.591877 | 0.001672 | 0.001650 | 0.001320 |
| has_not_equal_to_test.test | RUN | 0.175940 | 0.951524 | 1.295000 | 0.001415 | 0.001652 | 0.001750 |
| has_negate_test.test | RUN | 0.171859 | 0.710501 | 1.202628 | 0.001707 | 0.001359 | 0.001709 |
| has_multiplies_test.test | RUN | 0.175851 | 0.737725 | 1.245356 | 0.001660 | 0.001371 | 0.001568 |
| has_multiplies_assign_test.test | RUN | 0.177127 | 0.678660 | 1.264465 | 0.001498 | 0.001776 | 0.001631 |
| has_modulus_test.test | RUN | 0.171173 | 0.724204 | 1.264630 | 0.001418 | 0.001594 | 0.001729 |
| has_modulus_assign_test.test | RUN | 0.173475 | 0.723625 | 1.235593 | 0.001584 | 0.001395 | 0.001453 |
| has_minus_test.test | RUN | 0.178179 | 0.757431 | 1.276514 | 0.001646 | 0.001715 | 0.001327 |
| has_minus_assign_test.test | RUN | 0.186138 | 0.692214 | 1.298229 | 0.001455 | 0.001679 | 0.001590 |
| has_logical_or_test.test | RUN | 0.182560 | 0.715084 | 1.229422 | 0.001390 | 0.001609 | 0.001752 |
| has_logical_not_test.test | RUN | 0.174077 | 0.680540 | 1.164962 | 0.001535 | 0.001495 | 0.001722 |
| has_logical_and_test.test | RUN | 0.175454 | 0.693848 | 1.284539 | 0.001552 | 0.001688 | 0.001480 |
| has_less_test.test | RUN | 0.166170 | 0.952815 | 1.273917 | 0.001439 | 0.001728 | 0.001540 |
| has_less_equal_test.test | RUN | 0.168130 | 0.911490 | 1.205338 | 0.001332 | 0.001478 | 0.001658 |
| has_left_shift_test.test | RUN | 0.186472 | 0.873598 | 1.524963 | 0.001606 | 0.001451 | 0.001395 |
| has_left_shift_assign_test.test | RUN | 0.172568 | 0.683641 | 1.217242 | 0.001428 | 0.001469 | 0.001328 |
| has_greater_test.test | RUN | 0.176757 | 0.961042 | 1.220765 | 0.001522 | 0.001759 | 0.001431 |
| has_greater_equal_test.test | RUN | 0.176402 | 0.920265 | 1.200032 | 0.001383 | 0.001569 | 0.001664 |
| has_equal_to_test.test | RUN | 0.173159 | 0.915474 | 1.253699 | 0.001562 | 0.001418 | 0.001346 |
| has_divides_test.test | RUN | 0.170160 | 0.716659 | 1.207955 | 0.001396 | 0.001525 | 0.001299 |
| has_divides_assign_test.test | RUN | 0.172293 | 0.692958 | 1.222703 | 0.001394 | 0.001733 | 0.001286 |
| has_dereference_test.test | RUN | 0.167837 | 0.690573 | 1.150510 | 0.001314 | 0.001638 | 0.001630 |
| has_complement_test.test | RUN | 0.164607 | 0.927061 | 1.182624 | 0.001470 | 0.001736 | 0.001637 |
| has_bit_xor_test.test | RUN | 0.162134 | 0.687684 | 1.237878 | 0.001428 | 0.001668 | 0.001350 |
| has_bit_xor_assign_test.test | RUN | 0.169165 | 0.692540 | 1.251707 | 0.001470 | 0.001622 | 0.001234 |
| has_bit_or_test.test | RUN | 0.170041 | 0.670504 | 1.211815 | 0.001374 | 0.001739 | 0.001629 |
| has_bit_or_assign_test.test | RUN | 0.173960 | 0.715731 | 1.215783 | 0.001303 | 0.001564 | 0.001495 |
| has_bit_and_test.test | RUN | 0.171846 | 0.693095 | 1.278601 | 0.001377 | 0.001602 | 0.001380 |
| has_bit_and_assign_test.test | RUN | 0.170405 | 0.690749 | 1.264877 | 0.001594 | 0.001703 | 0.001279 |
| has_binary_classes9_test.test | RUN | 0.182632 | 0.788508 | 1.367216 | 0.001425 | 0.001736 | 0.001643 |
| has_binary_classes8_test.test | RUN | 0.182439 | 0.825549 | 1.356618 | 0.001525 | 0.001771 | 0.001724 |
| has_binary_classes7_test.test | RUN | 0.183831 | 0.795310 | 1.430007 | 0.001463 | 0.001646 | 0.001475 |
| has_binary_classes6_test.test | RUN | 0.187623 | 0.833211 | 1.434879 | 0.001604 | 0.001730 | 0.001257 |
| has_binary_classes5_test.test | RUN | 0.190129 | 0.809745 | 1.416764 | 0.001467 | 0.001726 | 0.001680 |
| has_binary_classes4_test.test | RUN | 0.188004 | 0.814904 | 1.378025 | 0.001485 | 0.001481 | 0.001657 |
| has_binary_classes3_test.test | RUN | 0.187931 | 0.803687 | 1.395602 | 0.001395 | 0.001702 | 0.001615 |
| has_binary_classes2_test.test | RUN | 0.185352 | 0.835895 | 1.389562 | 0.001623 | 0.003610 | 0.001448 |
| has_binary_classes1_test.test | RUN | 0.184789 | 0.922369 | 1.358800 | 0.001367 | 0.001813 | 0.001744 |
| has_binary_classes0_test.test | RUN | 0.187107 | 0.939139 | 1.375277 | 0.001383 | 0.001595 | 0.001394 |
| function_traits_test.test | RUN | 0.089371 | 0.297618 | 0.317697 | 0.001316 | 0.001792 | 0.001343 |
| floating_point_promotion_test.test | RUN | 0.086822 | 0.224567 | 0.295289 | 0.001560 | 0.001803 | 0.001492 |
| extent_test.test | RUN | 0.086410 | 0.225681 | 0.326448 | 0.001426 | 0.001644 | 0.001321 |
| enable_if_test.test | RUN | 0.080193 | 0.192943 | 0.226547 | 0.001375 | 0.001495 | 0.001564 |
| disjunction_test.test | RUN | 0.085596 | 0.199479 | 0.240748 | 0.001454 | 0.001525 | 0.001655 |
| detected_test.test | RUN | 0.086725 | 0.186473 | 0.238617 | 0.001678 | 0.001599 | 0.001536 |
| detected_or_test.test | RUN | 0.088370 | 0.200180 | 0.252262 | 0.001523 | 0.001670 | 0.001524 |
| decay_test.test | RUN | 0.216192 | 0.719078 | 1.424653 | 0.001619 | 0.001493 | 0.001507 |
| cxx14_aliases_test.test | RUN | 0.140194 | 0.282537 | 0.405697 | 0.001241 | 0.001682 | 0.001400 |
| copy_reference_test.test | RUN | 0.089673 | 0.192006 | 0.258219 | 0.001615 | 0.001450 | 0.001558 |
| copy_cv_test.test | RUN | 0.090238 | 0.197202 | 0.270295 | 0.001462 | 0.001732 | 0.001616 |
| copy_cv_ref_test.test | RUN | 0.121903 | 0.324727 | 0.595036 | 0.001433 | 0.001592 | 0.001618 |
| conjunction_test.test | RUN | 0.086591 | 0.199798 | 0.255426 | 0.001546 | 0.001607 | 0.001572 |
| conditional_test.test | RUN | 0.089680 | 0.194417 | 0.252130 | 0.001485 | 0.001432 | 0.001439 |
| common_type_test.test | RUN | 0.116465 | 0.288759 | 0.513103 | 0.001817 | 0.001184 | 0.001414 |
| common_type_sfinae_test.test | RUN | 0.086195 | 0.185388 | 0.284513 | 0.001388 | 0.001346 | 0.001760 |
| common_type_sfinae2_test.test | RUN | 0.089682 | 0.197990 | 0.263233 | 0.001414 | 0.001402 | 0.001371 |
| common_type_6_test.test | RUN | 0.091871 | 0.198289 | 0.267261 | 0.001738 | 0.001337 | 0.001857 |
| common_type_5_test.test | RUN | 0.089860 | 0.197815 | 0.258276 | 0.001406 | 0.001600 | 0.001656 |
| common_type_4_test.test | RUN | 0.096666 | 0.213413 | 0.296359 | 0.001706 | 0.001412 | 0.001424 |
| common_type_3_test.test | RUN | 0.087249 | 0.185498 | 0.234912 | 0.001530 | 0.001358 | 0.001568 |
| common_type_2_test.test | RUN | 0.115478 | 0.285211 | 0.456992 | 0.001746 | 0.001709 | 0.001476 |
| alignment_of_test.test | RUN | 0.094980 | 0.235982 | 0.311485 | 0.001420 | 0.002106 | 0.001473 |
| alignment_of_a2_test.test | RUN | 0.096386 | 0.232446 | 0.306311 | 0.001705 | 0.001438 | 0.001206 |
| aligned_storage_test.test | RUN | 0.099142 | 0.229046 | 0.285110 | 0.001614 | 0.001450 | 0.001686 |
| aligned_storage_empy_test.test | RUN | 0.109070 | 0.264139 | 0.395238 | 0.001571 | 0.001724 | 0.001526 |
| aligned_storage_a2_test.test | RUN | 0.096121 | 0.226880 | 0.296286 | 0.001541 | 0.001903 | 0.001649 |
| add_volatile_test.test | RUN | 0.188693 | 0.644857 | 1.308427 | 0.001653 | 0.001670 | 0.001484 |
| add_rvalue_reference_test.test | RUN | 0.210741 | 0.738145 | 1.482472 | 0.001613 | 0.001396 | 0.001738 |
| add_reference_test.test | RUN | 0.203204 | 0.696808 | 1.454662 | 0.001481 | 0.001676 | 0.001711 |
| add_pointer_test.test | RUN | 0.132637 | 0.396841 | 0.786137 | 0.001444 | 0.001406 | 0.001792 |
| add_lvalue_reference_test.test | RUN | 0.202258 | 0.700346 | 1.489984 | 0.001680 | 0.001482 | 0.001523 |
| add_cv_test.test | RUN | 0.194687 | 0.673210 | 1.417857 | 0.001453 | 0.001567 | 0.001639 |
| add_const_test.test | RUN | 0.196812 | 0.669654 | 1.394918 | 0.001360 | 0.001408 | 0.001546 |
| is_virtual_base_of_fail.test | COMPILE_FAIL | 0.081291 | 0.178282 | 0.242502 | — | — | — |
| is_stateless_fail.test | COMPILE_FAIL | 0.087244 | 0.181825 | 0.256031 | — | — | — |
| is_polymorphic_fail.test | COMPILE_FAIL | 0.083580 | 0.180324 | 0.244280 | — | — | — |
| is_nothrow_move_constructible_fail.test | COMPILE_FAIL | 0.082587 | 0.179018 | 0.230060 | — | — | — |
| is_nothrow_move_assignable_fail.test | COMPILE_FAIL | 0.082682 | 0.175151 | 0.236782 | — | — | — |
| is_list_constructible_fail.test | COMPILE_FAIL | 0.081000 | 0.175148 | 0.228859 | — | — | — |
| is_empty_fail.test | COMPILE_FAIL | 0.081074 | 0.176830 | 0.237056 | — | — | — |
| is_destructible_fail.test | COMPILE_FAIL | 0.077059 | 0.178351 | 0.230322 | — | — | — |
| is_default_constructible_fail.test | COMPILE_FAIL | 0.076842 | 0.176149 | 0.233822 | — | — | — |
| is_copy_constructible_fail.test | COMPILE_FAIL | 0.076865 | 0.179100 | 0.229256 | — | — | — |
| is_copy_assignable_fail.test | COMPILE_FAIL | 0.080631 | 0.176864 | 0.226924 | — | — | — |
| is_convertible_fail.test | COMPILE_FAIL | 0.081786 | 0.178994 | 0.227062 | — | — | — |
| is_convertible2_fail.test | COMPILE_FAIL | 0.080596 | 0.181688 | 0.227393 | — | — | — |
| is_constructible_fail.test | COMPILE_FAIL | 0.079655 | 0.168517 | 0.236495 | — | — | — |
| is_base_of_fail.test | COMPILE_FAIL | 0.079229 | 0.169465 | 0.224800 | — | — | — |
| is_assignable_fail.test | COMPILE_FAIL | 0.078476 | 0.176023 | 0.224217 | — | — | — |
| has_nothrow_destruct_fail.test | COMPILE_FAIL | 0.078001 | 0.180309 | 0.228499 | — | — | — |
| has_nothrow_copy_fail.test | COMPILE_FAIL | 0.079826 | 0.179212 | 0.233288 | — | — | — |
| has_nothrow_cons_fail.test | COMPILE_FAIL | 0.079222 | 0.181477 | 0.233784 | — | — | — |
| has_nothrow_assign_fail.test | COMPILE_FAIL | 0.078305 | 0.176062 | 0.228515 | — | — | — |
| common_type_fail.test | COMPILE_FAIL | 0.077885 | 0.179479 | 0.233651 | — | — | — |
| common_type2_fail.test | COMPILE_FAIL | 0.079307 | 0.173517 | 0.231378 | — | — | — |
| has_nothrow_assign_test_no_intrinsics.test | RUN | 0.117392 | 0.360127 | 0.617596 | 0.001591 | 0.001749 | 0.001465 |
| has_nothrow_constr_test_no_intrinsics.test | RUN | 0.113951 | 0.374869 | 0.631799 | 0.001395 | 0.001440 | 0.001398 |
| has_nothrow_copy_test_no_intrinsics.test | RUN | 0.117048 | 0.373738 | 0.608033 | 0.001389 | 0.001197 | 0.001196 |
| is_nothrow_move_assignable_test_no_intrinsics.test | RUN | 0.122598 | 0.363159 | 0.592465 | 0.001194 | 0.001088 | 0.001162 |
| is_nothrow_move_constructible_test_no_intrinsics.test | RUN | 0.078096 | 0.286730 | 0.363113 | 0.001348 | 0.001116 | 0.001211 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.023× | 1.023×, 1.024× | 1.028× |
| clang | 1.021× | 1.016×, 1.026× | 0.993× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051632).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| add_const_test.test | 10 | 0.000861s | 0.000897s | 0.000879s | byte equality observed |
| add_cv_test.test | 10 | 0.000857s | 0.000887s | 0.000886s | byte equality observed |
| add_lvalue_reference_test.test | 10 | 0.000892s | 0.000897s | 0.000890s | byte equality observed |
| add_pointer_test.test | 10 | 0.000875s | 0.000888s | 0.000878s | byte equality observed |
| add_reference_test.test | 10 | 0.000898s | 0.000877s | 0.000878s | byte equality observed |
| add_rvalue_reference_test.test | 10 | 0.000868s | 0.000904s | 0.000891s | byte equality observed |
| add_volatile_test.test | 10 | 0.000878s | 0.000922s | 0.000885s | byte equality observed |
| aligned_storage_a2_test.test | 10 | 0.000947s | 0.000989s | 0.000964s | byte equality observed |
| aligned_storage_empy_test.test | 10 | 0.000886s | 0.000875s | 0.000894s | byte equality observed |
| aligned_storage_test.test | 10 | 0.000967s | 0.000998s | 0.000974s | byte equality observed |
| alignment_of_a2_test.test | 10 | 0.000864s | 0.000898s | 0.000872s | byte equality observed |
| alignment_of_test.test | 10 | 0.000889s | 0.000904s | 0.000892s | byte equality observed |
| common_type_2_test.test | 10 | 0.000870s | 0.000885s | 0.000875s | byte equality observed |
| common_type_3_test.test | 10 | 0.000878s | 0.000908s | 0.000869s | byte equality observed |
| common_type_4_test.test | 10 | 0.000886s | 0.000888s | 0.000874s | byte equality observed |
| common_type_5_test.test | 10 | 0.000865s | 0.000880s | 0.000884s | byte equality observed |
| common_type_6_test.test | 10 | 0.000873s | 0.000899s | 0.000865s | byte equality observed |
| common_type_sfinae2_test.test | 10 | 0.000902s | 0.000875s | 0.000876s | byte equality observed |
| common_type_sfinae_test.test | 10 | 0.000879s | 0.000889s | 0.000881s | byte equality observed |
| common_type_test.test | 10 | 0.000882s | 0.000891s | 0.000893s | byte equality observed |
| conditional_test.test | 10 | 0.000882s | 0.000875s | 0.000878s | byte equality observed |
| conjunction_test.test | 10 | 0.000862s | 0.000885s | 0.000894s | byte equality observed |
| copy_cv_ref_test.test | 10 | 0.000886s | 0.000863s | 0.000902s | byte equality observed |
| copy_cv_test.test | 10 | 0.000883s | 0.000872s | 0.000906s | byte equality observed |
| copy_reference_test.test | 10 | 0.000890s | 0.000908s | 0.000883s | byte equality observed |
| cxx14_aliases_test.test | 10 | 0.000889s | 0.000889s | 0.000881s | byte equality observed |
| decay_test.test | 10 | 0.000887s | 0.000873s | 0.000911s | byte equality observed |
| detected_or_test.test | 10 | 0.000871s | 0.000867s | 0.000872s | byte equality observed |
| detected_test.test | 10 | 0.000874s | 0.000867s | 0.000917s | byte equality observed |
| disjunction_test.test | 10 | 0.000862s | 0.000891s | 0.000889s | byte equality observed |
| enable_if_test.test | 10 | 0.000895s | 0.000889s | 0.000885s | byte equality observed |
| extent_test.test | 10 | 0.000874s | 0.000876s | 0.000888s | byte equality observed |
| floating_point_promotion_test.test | 10 | 0.000882s | 0.000904s | 0.000895s | byte equality observed |
| function_traits_test.test | 10 | 0.000858s | 0.000871s | 0.000902s | byte equality observed |
| has_binary_classes0_test.test | 10 | 0.000877s | 0.000892s | 0.000902s | byte equality observed |
| has_binary_classes1_test.test | 10 | 0.000864s | 0.000880s | 0.000890s | byte equality observed |
| has_binary_classes2_test.test | 10 | 0.000876s | 0.000893s | 0.000894s | byte equality observed |
| has_binary_classes3_test.test | 10 | 0.000889s | 0.000875s | 0.000895s | byte equality observed |
| has_binary_classes4_test.test | 10 | 0.000879s | 0.000886s | 0.000885s | byte equality observed |
| has_binary_classes5_test.test | 10 | 0.000882s | 0.000875s | 0.000877s | byte equality observed |
| has_binary_classes6_test.test | 10 | 0.000894s | 0.000883s | 0.000897s | byte equality observed |
| has_binary_classes7_test.test | 10 | 0.000887s | 0.000887s | 0.000881s | byte equality observed |
| has_binary_classes8_test.test | 10 | 0.000894s | 0.000923s | 0.000895s | byte equality observed |
| has_binary_classes9_test.test | 10 | 0.000879s | 0.000930s | 0.000881s | byte equality observed |
| has_bit_and_assign_test.test | 10 | 0.000890s | 0.000892s | 0.000893s | byte equality observed |
| has_bit_and_test.test | 10 | 0.000881s | 0.000902s | 0.000878s | byte equality observed |
| has_bit_or_assign_test.test | 10 | 0.000885s | 0.000931s | 0.000926s | byte equality observed |
| has_bit_or_test.test | 10 | 0.000884s | 0.000940s | 0.000867s | byte equality observed |
| has_bit_xor_assign_test.test | 10 | 0.000889s | 0.000918s | 0.000903s | byte equality observed |
| has_bit_xor_test.test | 10 | 0.000890s | 0.000922s | 0.000890s | byte equality observed |
| has_complement_test.test | 10 | 0.000883s | 0.000943s | 0.000884s | byte equality observed |
| has_dereference_test.test | 10 | 0.000885s | 0.000919s | 0.000881s | byte equality observed |
| has_divides_assign_test.test | 10 | 0.000892s | 0.000947s | 0.000891s | byte equality observed |
| has_divides_test.test | 10 | 0.000892s | 0.000917s | 0.000894s | byte equality observed |
| has_equal_to_test.test | 10 | 0.000907s | 0.000918s | 0.000891s | byte equality observed |
| has_greater_equal_test.test | 10 | 0.000891s | 0.000928s | 0.000905s | byte equality observed |
| has_greater_test.test | 10 | 0.000880s | 0.000922s | 0.000888s | byte equality observed |
| has_left_shift_assign_test.test | 10 | 0.000897s | 0.000883s | 0.000888s | byte equality observed |
| has_left_shift_test.test | 10 | 0.000896s | 0.000905s | 0.000871s | byte equality observed |
| has_less_equal_test.test | 10 | 0.000905s | 0.000886s | 0.000890s | byte equality observed |
| has_less_test.test | 10 | 0.000881s | 0.000893s | 0.000876s | byte equality observed |
| has_logical_and_test.test | 10 | 0.000887s | 0.000898s | 0.000910s | byte equality observed |
| has_logical_not_test.test | 10 | 0.000898s | 0.000887s | 0.000862s | byte equality observed |
| has_logical_or_test.test | 10 | 0.000895s | 0.000870s | 0.000891s | byte equality observed |
| has_minus_assign_test.test | 10 | 0.000886s | 0.000919s | 0.000897s | byte equality observed |
| has_minus_test.test | 10 | 0.000900s | 0.000861s | 0.000911s | byte equality observed |
| has_modulus_assign_test.test | 10 | 0.000896s | 0.000890s | 0.000888s | byte equality observed |
| has_modulus_test.test | 10 | 0.000885s | 0.000897s | 0.000883s | byte equality observed |
| has_multiplies_assign_test.test | 10 | 0.000908s | 0.000870s | 0.000890s | byte equality observed |
| has_multiplies_test.test | 10 | 0.000895s | 0.000867s | 0.000899s | byte equality observed |
| has_negate_test.test | 10 | 0.000883s | 0.000892s | 0.000916s | byte equality observed |
| has_not_equal_to_test.test | 10 | 0.000903s | 0.000889s | 0.000900s | byte equality observed |
| has_nothrow_assign_test.test | 10 | 0.000894s | 0.000866s | 0.000878s | byte equality observed |
| has_nothrow_assign_test_no_intrinsics.test | 10 | 0.000888s | 0.000882s | 0.000900s | byte equality observed |
| has_nothrow_constr_test.test | 10 | 0.000878s | 0.000882s | 0.000893s | byte equality observed |
| has_nothrow_constr_test_no_intrinsics.test | 10 | 0.000884s | 0.000886s | 0.000865s | byte equality observed |
| has_nothrow_copy_test.test | 10 | 0.000889s | 0.000874s | 0.000913s | byte equality observed |
| has_nothrow_copy_test_no_intrinsics.test | 10 | 0.000880s | 0.000876s | 0.000913s | byte equality observed |
| has_nothrow_destructor_test.test | 10 | 0.000902s | 0.000873s | 0.000890s | byte equality observed |
| has_operator_new_test.test | 10 | 0.000902s | 0.000901s | 0.000897s | byte equality observed |
| has_plus_assign_test.test | 10 | 0.000882s | 0.000878s | 0.000914s | byte equality observed |
| has_plus_test.test | 10 | 0.000891s | 0.000897s | 0.000903s | byte equality observed |
| has_post_decrement_test.test | 10 | 0.000891s | 0.000887s | 0.000883s | byte equality observed |
| has_post_increment_test.test | 10 | 0.000899s | 0.000881s | 0.000883s | byte equality observed |
| has_postfix_classes0_test.test | 10 | 0.000882s | 0.000896s | 0.000901s | byte equality observed |
| has_postfix_classes1_test.test | 10 | 0.000881s | 0.000876s | 0.000904s | byte equality observed |
| has_postfix_classes2_test.test | 10 | 0.000885s | 0.000888s | 0.000890s | byte equality observed |
| has_postfix_classes3_test.test | 10 | 0.000894s | 0.000875s | 0.000916s | byte equality observed |
| has_pre_decrement_test.test | 10 | 0.000890s | 0.000873s | 0.000895s | byte equality observed |
| has_pre_increment_test.test | 10 | 0.000881s | 0.000874s | 0.000892s | byte equality observed |
| has_prefix_classes0_test.test | 10 | 0.000894s | 0.000886s | 0.000888s | byte equality observed |
| has_prefix_classes1_test.test | 10 | 0.000908s | 0.000866s | 0.000887s | byte equality observed |
| has_prefix_classes2_test.test | 10 | 0.000891s | 0.000890s | 0.000890s | byte equality observed |
| has_prefix_classes3_test.test | 10 | 0.000879s | 0.000877s | 0.000912s | byte equality observed |
| has_right_shift_assign_test.test | 10 | 0.000884s | 0.000883s | 0.000896s | byte equality observed |
| has_right_shift_test.test | 10 | 0.000883s | 0.000883s | 0.000893s | byte equality observed |
| has_trivial_assign_test.test | 10 | 0.000937s | 0.000932s | 0.000951s | byte equality observed |
| has_trivial_constr_test.test | 10 | 0.000911s | 0.000915s | 0.000958s | native test oracle; output varies |
| has_trivial_copy_test.test | 10 | 0.000933s | 0.000935s | 0.000963s | native test oracle; output varies |
| has_trivial_destructor_test.test | 10 | 0.000891s | 0.000898s | 0.000876s | byte equality observed |
| has_trivial_move_assign_test.test | 10 | 0.000856s | 0.000887s | 0.000897s | byte equality observed |
| has_trivial_move_constructor_test.test | 10 | 0.000898s | 0.000910s | 0.000900s | byte equality observed |
| has_unary_minus_test.test | 10 | 0.000876s | 0.000910s | 0.000885s | byte equality observed |
| has_unary_plus_test.test | 10 | 0.000878s | 0.000906s | 0.000877s | byte equality observed |
| has_virtual_destructor_test.test | 10 | 0.000890s | 0.000924s | 0.000875s | byte equality observed |
| integral_constant_test.test | 10 | 0.000417s | 0.000424s | 0.000895s | byte equality observed |
| is_abstract_test.test | 10 | 0.000882s | 0.000897s | 0.000894s | byte equality observed |
| is_arithmetic_test.test | 10 | 0.000872s | 0.000904s | 0.000877s | byte equality observed |
| is_array_test.test | 10 | 0.000887s | 0.000895s | 0.000913s | byte equality observed |
| is_assignable_test.test | 10 | 0.000892s | 0.000901s | 0.000890s | byte equality observed |
| is_base_and_derived_test.test | 10 | 0.000908s | 0.000896s | 0.000911s | byte equality observed |
| is_base_of_test.test | 10 | 0.000895s | 0.000916s | 0.000914s | byte equality observed |
| is_bounded_array_test.test | 10 | 0.000899s | 0.000881s | 0.000890s | byte equality observed |
| is_class_test.test | 10 | 0.000890s | 0.000901s | 0.000906s | byte equality observed |
| is_complete_test.test | 10 | 0.000870s | 0.000886s | 0.000888s | byte equality observed |
| is_complex_test.test | 10 | 0.000916s | 0.000921s | 0.000893s | byte equality observed |
| is_compound_test.test | 10 | 0.000883s | 0.000888s | 0.000878s | byte equality observed |
| is_const_test.test | 10 | 0.000897s | 0.000900s | 0.000894s | byte equality observed |
| is_constructible_test.test | 10 | 0.000880s | 0.000886s | 0.000887s | byte equality observed |
| is_convertible_test.test | 10 | 0.000897s | 0.000875s | 0.000896s | byte equality observed |
| is_copy_assignable_test.test | 10 | 0.000893s | 0.000887s | 0.000900s | byte equality observed |
| is_copy_constructible_test.test | 10 | 0.000884s | 0.000893s | 0.000905s | byte equality observed |
| is_default_constr_test.test | 10 | 0.000904s | 0.000887s | 0.000885s | byte equality observed |
| is_destructible_test.test | 10 | 0.000896s | 0.000896s | 0.000893s | byte equality observed |
| is_detected_exact_test.test | 10 | 0.000888s | 0.000876s | 0.000887s | byte equality observed |
| is_detected_test.test | 10 | 0.000879s | 0.000884s | 0.000922s | byte equality observed |
| is_empty_test.test | 10 | 0.000891s | 0.000898s | 0.000923s | byte equality observed |
| is_enum_test.test | 10 | 0.000894s | 0.000898s | 0.000908s | byte equality observed |
| is_final_test.test | 10 | 0.000883s | 0.000899s | 0.000916s | byte equality observed |
| is_float_test.test | 10 | 0.000893s | 0.000876s | 0.000898s | byte equality observed |
| is_floating_point_test.test | 10 | 0.000897s | 0.000903s | 0.000882s | byte equality observed |
| is_function_test.test | 10 | 0.000887s | 0.000920s | 0.000874s | byte equality observed |
| is_fundamental_test.test | 10 | 0.000903s | 0.000899s | 0.000897s | byte equality observed |
| is_integral_test.test | 10 | 0.000897s | 0.000884s | 0.000892s | byte equality observed |
| is_list_constructible_test.test | 10 | 0.000888s | 0.000891s | 0.000869s | byte equality observed |
| is_lvalue_reference_test.test | 10 | 0.000899s | 0.000912s | 0.000900s | byte equality observed |
| is_member_func_test.test | 10 | 0.000886s | 0.000899s | 0.000902s | byte equality observed |
| is_member_obj_test.test | 10 | 0.000898s | 0.000914s | 0.000889s | byte equality observed |
| is_member_pointer_test.test | 10 | 0.000887s | 0.000912s | 0.000906s | byte equality observed |
| is_noncopyable_test.test | 10 | 0.000900s | 0.000899s | 0.000877s | byte equality observed |
| is_nothrow_move_assignable_test.test | 10 | 0.000890s | 0.000900s | 0.000911s | byte equality observed |
| is_nothrow_move_assignable_test_no_intrinsics.test | 10 | 0.000882s | 0.000906s | 0.000893s | byte equality observed |
| is_nothrow_move_constructible_test.test | 10 | 0.000891s | 0.000903s | 0.000871s | byte equality observed |
| is_nothrow_move_constructible_test_no_intrinsics.test | 10 | 0.000881s | 0.000902s | 0.000918s | byte equality observed |
| is_nothrow_swappable_test.test | 10 | 0.000900s | 0.000918s | 0.000881s | byte equality observed |
| is_object_test.test | 10 | 0.000893s | 0.000915s | 0.000868s | byte equality observed |
| is_pod_test.test | 10 | 0.000888s | 0.000900s | 0.000909s | byte equality observed |
| is_pointer_test.test | 10 | 0.000896s | 0.000928s | 0.000866s | byte equality observed |
| is_polymorphic_test.test | 10 | 0.000883s | 0.000910s | 0.000895s | byte equality observed |
| is_reference_test.test | 10 | 0.000915s | 0.000915s | 0.000897s | byte equality observed |
| is_rvalue_reference_test.test | 10 | 0.000875s | 0.000919s | 0.000899s | byte equality observed |
| is_same_test.test | 10 | 0.000896s | 0.000889s | 0.000888s | byte equality observed |
| is_scalar_test.test | 10 | 0.000883s | 0.000912s | 0.000882s | byte equality observed |
| is_scoped_enum_test.test | 10 | 0.000875s | 0.000905s | 0.000870s | byte equality observed |
| is_signed_test.test | 10 | 0.000885s | 0.000918s | 0.000889s | byte equality observed |
| is_stateless_test.test | 10 | 0.000889s | 0.000890s | 0.000864s | byte equality observed |
| is_swappable_test.test | 10 | 0.000875s | 0.000900s | 0.000902s | byte equality observed |
| is_trivially_copyable_test.test | 10 | 0.000920s | 0.000915s | 0.000914s | byte equality observed |
| is_unbounded_array_test.test | 10 | 0.000887s | 0.000877s | 0.000874s | byte equality observed |
| is_union_test.test | 10 | 0.000891s | 0.000915s | 0.000865s | byte equality observed |
| is_unscoped_enum_test.test | 10 | 0.000890s | 0.000872s | 0.000875s | byte equality observed |
| is_unsigned_test.test | 10 | 0.000890s | 0.000896s | 0.000913s | byte equality observed |
| is_virtual_base_of_test.test | 10 | 0.000888s | 0.000892s | 0.000892s | byte equality observed |
| is_void_test.test | 10 | 0.000886s | 0.000900s | 0.000891s | byte equality observed |
| is_volatile_test.test | 10 | 0.000878s | 0.000875s | 0.000884s | byte equality observed |
| make_signed_test.test | 10 | 0.000874s | 0.000902s | 0.000888s | byte equality observed |
| make_unsigned_test.test | 10 | 0.000889s | 0.000890s | 0.000874s | byte equality observed |
| make_void_test.test | 10 | 0.000908s | 0.000903s | 0.000905s | byte equality observed |
| mpl_interop_test1.test | 10 | 0.000414s | 0.000415s | 0.000912s | byte equality observed |
| mpl_interop_test2.test | 10 | 0.000421s | 0.000428s | 0.000898s | byte equality observed |
| mpl_interop_test3.test | 10 | 0.000416s | 0.000435s | 0.000907s | byte equality observed |
| negation_test.test | 10 | 0.000900s | 0.000956s | 0.000902s | byte equality observed |
| promote_basic_test.test | 10 | 0.000410s | 0.000427s | 0.000919s | byte equality observed |
| promote_enum_msvc_bug_test.test | 10 | 0.000420s | 0.000416s | 0.000909s | byte equality observed |
| promote_enum_test.test | 10 | 0.000419s | 0.000432s | 0.000933s | byte equality observed |
| promote_mpl_test.test | 10 | 0.000408s | 0.000419s | 0.000901s | byte equality observed |
| rank_test.test | 10 | 0.000908s | 0.000925s | 0.000900s | byte equality observed |
| remove_all_extents_test.test | 10 | 0.000890s | 0.000926s | 0.000889s | byte equality observed |
| remove_bounds_test.test | 10 | 0.000887s | 0.000878s | 0.000893s | byte equality observed |
| remove_const_test.test | 10 | 0.000882s | 0.000892s | 0.000887s | byte equality observed |
| remove_cv_ref_test.test | 10 | 0.000884s | 0.000887s | 0.000906s | byte equality observed |
| remove_cv_test.test | 10 | 0.000882s | 0.000892s | 0.000904s | byte equality observed |
| remove_extent_test.test | 10 | 0.000893s | 0.000891s | 0.000895s | byte equality observed |
| remove_pointer_test.test | 10 | 0.000890s | 0.000910s | 0.000900s | byte equality observed |
| remove_reference_test.test | 10 | 0.000867s | 0.000892s | 0.000924s | byte equality observed |
| remove_volatile_test.test | 10 | 0.000904s | 0.000891s | 0.000874s | byte equality observed |
| tricky_abstract_type_test.test | 10 | 0.000885s | 0.000878s | 0.000892s | byte equality observed |
| tricky_add_pointer_test.test | 10 | 0.000897s | 0.000921s | 0.000873s | byte equality observed |
| tricky_function_type_test.test | 10 | 0.000873s | 0.000906s | 0.000892s | byte equality observed |
| tricky_incomplete_type_test.test | 10 | 0.000901s | 0.000906s | 0.000888s | byte equality observed |
| tricky_is_enum_test.test | 10 | 0.000874s | 0.000882s | 0.000924s | byte equality observed |
| tricky_partial_spec_test.test | 10 | 0.000882s | 0.000872s | 0.000877s | byte equality observed |
| tricky_rvalue_test.test | 10 | 0.000905s | 0.000916s | 0.000899s | byte equality observed |
| type_identity_test.test | 10 | 0.000912s | 0.000918s | 0.000898s | byte equality observed |
| type_traits_test.test | 10 | 0.000427s | 0.000420s | 0.000890s | byte equality observed |
| type_with_alignment_test.test | 10 | 0.001006s | 0.001038s | 0.000983s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051632).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
