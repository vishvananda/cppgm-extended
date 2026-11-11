# libs/type_traits/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| type_with_alignment_test.test | RUN | 0.130722 | 0.317512 | 0.505237 | 0.001545 | 0.001727 | 0.001850 |
| type_traits_test.test | RUN | 0.177450 | 0.330932 | 0.505798 | 0.000586 | 0.000566 | 0.001331 |
| type_identity_test.test | RUN | 0.312528 | 1.150055 | 2.939117 | 0.001713 | 0.001726 | 0.001676 |
| tricky_rvalue_test.test | RUN | 0.119942 | 0.255380 | 0.395627 | 0.001517 | 0.001698 | 0.001586 |
| tricky_partial_spec_test.test | RUN | 0.146458 | 0.370430 | 0.667873 | 0.001552 | 0.001575 | 0.001457 |
| tricky_is_enum_test.test | RUN | 0.115034 | 0.241940 | 0.358392 | 0.001636 | 0.001660 | 0.001715 |
| tricky_incomplete_type_test.test | RUN | 0.190019 | 0.380726 | 0.582948 | 0.001411 | 0.001742 | 0.001733 |
| tricky_function_type_test.test | RUN | 0.154957 | 0.378793 | 0.696538 | 0.001658 | 0.001942 | 0.001572 |
| tricky_add_pointer_test.test | RUN | 0.217304 | 0.696660 | 1.658019 | 0.001416 | 0.001788 | 0.001686 |
| tricky_abstract_type_test.test | RUN | 0.122707 | 0.255803 | 0.373032 | 0.001596 | 0.001662 | 0.001510 |
| remove_volatile_test.test | RUN | 0.291280 | 1.004148 | 2.599955 | 0.001674 | 0.001685 | 0.001750 |
| remove_reference_test.test | RUN | 0.305752 | 1.009973 | 2.743616 | 0.001582 | 0.001700 | 0.001692 |
| remove_pointer_test.test | RUN | 0.307273 | 1.072768 | 2.860892 | 0.001631 | 0.001736 | 0.001697 |
| remove_extent_test.test | RUN | 0.302009 | 1.089008 | 2.743670 | 0.001466 | 0.001913 | 0.001713 |
| remove_cv_test.test | RUN | 0.296195 | 1.071018 | 2.684405 | 0.001707 | 0.001796 | 0.001710 |
| remove_cv_ref_test.test | RUN | 0.603799 | 2.511447 | 6.672644 | 0.001565 | 0.001630 | 0.001596 |
| remove_const_test.test | RUN | 0.291887 | 1.018646 | 2.652013 | 0.001552 | 0.001774 | 0.001494 |
| remove_bounds_test.test | RUN | 0.297043 | 1.046563 | 2.702586 | 0.001553 | 0.001785 | 0.001686 |
| remove_all_extents_test.test | RUN | 0.303719 | 1.028917 | 2.662547 | 0.001599 | 0.001562 | 0.001628 |
| rank_test.test | RUN | 0.121043 | 0.246892 | 0.372553 | 0.001446 | 0.001479 | 0.001674 |
| promote_mpl_test.test | RUN | 0.111132 | 0.227608 | 0.283909 | 0.000610 | 0.000604 | 0.001751 |
| promote_enum_test.test | RUN | 0.023948 | 0.055780 | 0.070934 | 0.000610 | 0.000776 | 0.001604 |
| promote_enum_msvc_bug_test.test | RUN | 0.022099 | 0.049467 | 0.055875 | 0.000610 | 0.000660 | 0.001742 |
| promote_basic_test.test | RUN | 0.027497 | 0.064294 | 0.074096 | 0.000731 | 0.000692 | 0.001794 |
| negation_test.test | RUN | 0.115061 | 0.230986 | 0.362309 | 0.001522 | 0.001248 | 0.001620 |
| mpl_interop_test3.test | RUN | 0.104787 | 0.223593 | 0.243185 | 0.000702 | 0.000735 | 0.001579 |
| mpl_interop_test2.test | RUN | 0.043127 | 0.061780 | 0.063929 | 0.000597 | 0.000609 | 0.001647 |
| mpl_interop_test1.test | RUN | 0.019111 | 0.045587 | 0.049686 | 0.000619 | 0.000785 | 0.001765 |
| make_void_test.test | RUN | 0.120459 | 0.250119 | 0.389350 | 0.001707 | 0.001914 | 0.001786 |
| make_unsigned_test.test | RUN | 0.139836 | 0.328377 | 0.544948 | 0.001507 | 0.001638 | 0.001730 |
| make_signed_test.test | RUN | 0.142867 | 0.324571 | 0.577088 | 0.001487 | 0.001412 | 0.001518 |
| is_volatile_test.test | RUN | 0.127649 | 0.290614 | 0.434088 | 0.001560 | 0.001722 | 0.001596 |
| is_void_test.test | RUN | 0.121303 | 0.268477 | 0.384063 | 0.001661 | 0.001684 | 0.001689 |
| is_virtual_base_of_test.test | RUN | 0.140517 | 0.338441 | 0.580867 | 0.001511 | 0.001630 | 0.001609 |
| is_unsigned_test.test | RUN | 0.124897 | 0.273511 | 0.446455 | 0.001473 | 0.001762 | 0.001569 |
| is_unscoped_enum_test.test | RUN | 0.122143 | 0.252165 | 0.336504 | 0.001436 | 0.001522 | 0.001408 |
| is_union_test.test | RUN | 0.131646 | 0.319232 | 0.537386 | 0.001626 | 0.001748 | 0.001784 |
| is_unbounded_array_test.test | RUN | 0.121004 | 0.264086 | 0.426782 | 0.001544 | 0.001593 | 0.001708 |
| is_trivially_copyable_test.test | RUN | 0.192155 | 0.552982 | 1.030488 | 0.001319 | 0.001540 | 0.001824 |
| is_swappable_test.test | RUN | 0.161401 | 0.417312 | 0.774100 | 0.001572 | 0.001687 | 0.001502 |
| is_stateless_test.test | RUN | 0.185625 | 0.475015 | 0.905398 | 0.001522 | 0.001704 | 0.001380 |
| is_signed_test.test | RUN | 0.127070 | 0.268341 | 0.445081 | 0.001600 | 0.001721 | 0.001706 |
| is_scoped_enum_test.test | RUN | 0.119630 | 0.252015 | 0.344170 | 0.001559 | 0.001526 | 0.001751 |
| is_scalar_test.test | RUN | 0.159052 | 0.447143 | 0.813935 | 0.001270 | 0.001512 | 0.001593 |
| is_same_test.test | RUN | 0.119390 | 0.274846 | 0.373839 | 0.001601 | 0.001624 | 0.001764 |
| is_rvalue_reference_test.test | RUN | 0.127745 | 0.298989 | 0.478035 | 0.001531 | 0.001767 | 0.001442 |
| is_reference_test.test | RUN | 0.127381 | 0.305354 | 0.459495 | 0.001666 | 0.001652 | 0.001609 |
| is_polymorphic_test.test | RUN | 0.132556 | 0.309881 | 0.509190 | 0.001328 | 0.001782 | 0.001751 |
| is_pointer_test.test | RUN | 0.128350 | 0.303583 | 0.513967 | 0.001836 | 0.001637 | 0.001748 |
| is_pod_test.test | RUN | 0.173972 | 0.491030 | 0.922836 | 0.001465 | 0.001780 | 0.001716 |
| is_object_test.test | RUN | 0.120315 | 0.253766 | 0.342888 | 0.001650 | 0.001782 | 0.001676 |
| is_nothrow_swappable_test.test | RUN | 0.175587 | 0.460821 | 0.869027 | 0.001448 | 0.001690 | 0.001559 |
| is_nothrow_move_constructible_test.test | RUN | 0.169245 | 0.458965 | 0.843168 | 0.001597 | 0.001616 | 0.001611 |
| is_nothrow_move_assignable_test.test | RUN | 0.172921 | 0.477924 | 0.928599 | 0.001531 | 0.001680 | 0.001455 |
| is_noncopyable_test.test | RUN | 0.120093 | 0.254555 | 0.415614 | 0.001579 | 0.001856 | 0.001802 |
| is_member_pointer_test.test | RUN | 0.123012 | 0.258553 | 0.376965 | 0.001488 | 0.001704 | 0.001734 |
| is_member_obj_test.test | RUN | 0.122286 | 0.268095 | 0.417739 | 0.001514 | 0.001512 | 0.001666 |
| is_member_func_test.test | RUN | 0.126376 | 0.293979 | 0.429595 | 0.001478 | 0.001814 | 0.001624 |
| is_lvalue_reference_test.test | RUN | 0.127280 | 0.300200 | 0.467541 | 0.001508 | 0.001658 | 0.001610 |
| is_list_constructible_test.test | RUN | 0.150216 | 0.338192 | 0.556768 | 0.001564 | 0.001794 | 0.001732 |
| is_integral_test.test | RUN | 0.152780 | 0.409885 | 0.749860 | 0.001457 | 0.001594 | 0.001554 |
| is_fundamental_test.test | RUN | 0.155909 | 0.427791 | 0.755241 | 0.001278 | 0.001796 | 0.001762 |
| is_function_test.test | RUN | 0.125657 | 0.269714 | 0.451301 | 0.001572 | 0.001684 | 0.001669 |
| is_floating_point_test.test | RUN | 0.132662 | 0.297876 | 0.484891 | 0.001628 | 0.001847 | 0.001744 |
| is_float_test.test | RUN | 0.129862 | 0.301705 | 0.487715 | 0.001557 | 0.001592 | 0.001390 |
| is_final_test.test | RUN | 0.126614 | 0.305297 | 0.493439 | 0.001373 | 0.001736 | 0.001483 |
| is_enum_test.test | RUN | 0.119045 | 0.255542 | 0.378457 | 0.001592 | 0.001780 | 0.001632 |
| is_empty_test.test | RUN | 0.130271 | 0.282370 | 0.433037 | 0.001343 | 0.001825 | 0.001758 |
| is_detected_test.test | RUN | 0.117502 | 0.237751 | 0.358344 | 0.001645 | 0.001426 | 0.001447 |
| is_detected_exact_test.test | RUN | 0.115332 | 0.241016 | 0.350197 | 0.001570 | 0.001788 | 0.001469 |
| is_destructible_test.test | RUN | 0.166166 | 0.465488 | 0.897845 | 0.001653 | 0.001818 | 0.001660 |
| is_default_constr_test.test | RUN | 0.171257 | 0.497669 | 0.904210 | 0.001718 | 0.001845 | 0.001707 |
| is_copy_constructible_test.test | RUN | 0.179602 | 0.501818 | 0.990171 | 0.001773 | 0.001715 | 0.001521 |
| is_copy_assignable_test.test | RUN | 0.176820 | 0.506278 | 0.980169 | 0.001526 | 0.001825 | 0.001389 |
| is_convertible_test.test | RUN | 0.169552 | 0.441078 | 0.796044 | 0.001529 | 0.001574 | 0.001417 |
| is_constructible_test.test | RUN | 0.145132 | 0.324189 | 0.575073 | 0.001387 | 0.001656 | 0.001692 |
| is_const_test.test | RUN | 0.126374 | 0.278077 | 0.458642 | 0.001322 | 0.001404 | 0.001612 |
| is_compound_test.test | RUN | 0.116421 | 0.247643 | 0.379514 | 0.001530 | 0.001693 | 0.001282 |
| is_complex_test.test | RUN | 0.157628 | 0.325423 | 0.521522 | 0.001418 | 0.001770 | 0.001447 |
| is_complete_test.test | RUN | 0.139531 | 0.337976 | 0.584471 | 0.001533 | 0.001638 | 0.001540 |
| is_class_test.test | RUN | 0.129775 | 0.293910 | 0.481832 | 0.001477 | 0.001550 | 0.001575 |
| is_bounded_array_test.test | RUN | 0.121000 | 0.267771 | 0.411978 | 0.001527 | 0.001495 | 0.001574 |
| is_base_of_test.test | RUN | 0.128213 | 0.290414 | 0.479607 | 0.001352 | 0.001770 | 0.001524 |
| is_base_and_derived_test.test | RUN | 0.128922 | 0.289046 | 0.459181 | 0.001522 | 0.001730 | 0.001560 |
| is_assignable_test.test | RUN | 0.155118 | 0.407394 | 0.718140 | 0.001411 | 0.001802 | 0.001590 |
| is_array_test.test | RUN | 0.125394 | 0.282139 | 0.467637 | 0.001678 | 0.001795 | 0.001600 |
| is_arithmetic_test.test | RUN | 0.155254 | 0.414697 | 0.781865 | 0.001410 | 0.001615 | 0.001570 |
| is_abstract_test.test | RUN | 0.282963 | 1.147834 | 2.560754 | 0.001317 | 0.001711 | 0.001810 |
| integral_constant_test.test | RUN | 0.019676 | 0.048082 | 0.045958 | 0.000672 | 0.000746 | 0.001650 |
| has_virtual_destructor_test.test | RUN | 0.135053 | 0.320301 | 0.548405 | 0.001636 | 0.001706 | 0.001734 |
| has_unary_plus_test.test | RUN | 0.246614 | 1.276168 | 1.923602 | 0.001693 | 0.001785 | 0.001722 |
| has_unary_minus_test.test | RUN | 0.243761 | 1.290648 | 1.885211 | 0.001534 | 0.001486 | 0.001860 |
| has_trivial_move_constructor_test.test | RUN | 0.175812 | 0.493734 | 0.957978 | 0.001429 | 0.001551 | 0.001652 |
| has_trivial_move_assign_test.test | RUN | 0.169736 | 0.454906 | 0.918911 | 0.001697 | 0.001418 | 0.001822 |
| has_trivial_destructor_test.test | RUN | 0.172828 | 0.477798 | 0.924170 | 0.001704 | 0.001671 | 0.001710 |
| has_trivial_copy_test.test | RUN | 0.177598 | 0.510694 | 0.977426 | 0.001744 | 0.001818 | 0.001921 |
| has_trivial_constr_test.test | RUN | 0.180302 | 0.529367 | 0.974848 | 0.001637 | 0.001365 | 0.001405 |
| has_trivial_assign_test.test | RUN | 0.174698 | 0.503986 | 0.959745 | 0.001670 | 0.001804 | 0.001732 |
| has_right_shift_test.test | RUN | 0.256525 | 0.970703 | 2.145935 | 0.001644 | 0.001746 | 0.001579 |
| has_right_shift_assign_test.test | RUN | 0.243334 | 0.894106 | 1.925768 | 0.001506 | 0.001678 | 0.001556 |
| has_prefix_classes3_test.test | RUN | 0.183514 | 0.517848 | 0.981579 | 0.001590 | 0.001487 | 0.001665 |
| has_prefix_classes2_test.test | RUN | 0.187206 | 0.517044 | 0.972860 | 0.001417 | 0.001634 | 0.001690 |
| has_prefix_classes1_test.test | RUN | 0.186212 | 0.537099 | 0.948103 | 0.001633 | 0.001638 | 0.001786 |
| has_prefix_classes0_test.test | RUN | 0.184329 | 0.528268 | 0.995405 | 0.001529 | 0.001898 | 0.001737 |
| has_pre_increment_test.test | RUN | 0.243513 | 1.277058 | 1.923272 | 0.001445 | 0.001397 | 0.001516 |
| has_pre_decrement_test.test | RUN | 0.245227 | 1.243212 | 1.935016 | 0.001546 | 0.001621 | 0.001453 |
| has_postfix_classes3_test.test | RUN | 0.183929 | 0.545312 | 1.051590 | 0.001531 | 0.001795 | 0.001698 |
| has_postfix_classes2_test.test | RUN | 0.182180 | 0.529896 | 0.997640 | 0.001523 | 0.001553 | 0.001438 |
| has_postfix_classes1_test.test | RUN | 0.185236 | 0.545325 | 0.991478 | 0.001493 | 0.001619 | 0.001825 |
| has_postfix_classes0_test.test | RUN | 0.186482 | 0.531949 | 0.998172 | 0.001630 | 0.001660 | 0.001639 |
| has_post_increment_test.test | RUN | 0.243019 | 1.272306 | 1.977845 | 0.001778 | 0.001738 | 0.001637 |
| has_post_decrement_test.test | RUN | 0.243218 | 1.258443 | 2.016937 | 0.001356 | 0.001604 | 0.001714 |
| has_plus_test.test | RUN | 0.248476 | 0.929775 | 1.896433 | 0.001452 | 0.001650 | 0.001572 |
| has_plus_assign_test.test | RUN | 0.254144 | 0.908293 | 1.962085 | 0.001677 | 0.001673 | 0.001461 |
| has_operator_new_test.test | RUN | 0.173433 | 0.486375 | 0.831346 | 0.001716 | 0.001374 | 0.001549 |
| has_nothrow_destructor_test.test | RUN | 0.181684 | 0.485244 | 0.972372 | 0.001494 | 0.001641 | 0.001682 |
| has_nothrow_copy_test.test | RUN | 0.176161 | 0.479451 | 0.929440 | 0.001746 | 0.001568 | 0.001720 |
| has_nothrow_constr_test.test | RUN | 0.171328 | 0.467799 | 0.901242 | 0.001703 | 0.001826 | 0.001678 |
| has_nothrow_assign_test.test | RUN | 0.170545 | 0.469450 | 0.920207 | 0.001695 | 0.001824 | 0.001714 |
| has_not_equal_to_test.test | RUN | 0.249091 | 1.198564 | 2.008356 | 0.001498 | 0.001600 | 0.001771 |
| has_negate_test.test | RUN | 0.238404 | 0.871166 | 1.885417 | 0.001554 | 0.001784 | 0.001823 |
| has_multiplies_test.test | RUN | 0.245267 | 0.902748 | 2.010407 | 0.001388 | 0.001651 | 0.001622 |
| has_multiplies_assign_test.test | RUN | 0.249359 | 0.877592 | 1.912034 | 0.001563 | 0.001780 | 0.001382 |
| has_modulus_test.test | RUN | 0.246933 | 0.914538 | 1.961769 | 0.001424 | 0.001780 | 0.001796 |
| has_modulus_assign_test.test | RUN | 0.248818 | 0.903942 | 1.935110 | 0.001559 | 0.001828 | 0.001694 |
| has_minus_test.test | RUN | 0.259641 | 0.900365 | 2.012003 | 0.001557 | 0.001542 | 0.001554 |
| has_minus_assign_test.test | RUN | 0.256483 | 0.883310 | 2.021862 | 0.001584 | 0.001788 | 0.001348 |
| has_logical_or_test.test | RUN | 0.252967 | 0.916449 | 1.949379 | 0.001674 | 0.001582 | 0.001674 |
| has_logical_not_test.test | RUN | 0.238205 | 0.867577 | 1.876480 | 0.001525 | 0.001566 | 0.001682 |
| has_logical_and_test.test | RUN | 0.241553 | 0.930214 | 2.003551 | 0.001657 | 0.001805 | 0.001558 |
| has_less_test.test | RUN | 0.246289 | 1.233924 | 1.998606 | 0.001560 | 0.001776 | 0.001363 |
| has_less_equal_test.test | RUN | 0.245741 | 1.207476 | 1.950415 | 0.001613 | 0.001737 | 0.001657 |
| has_left_shift_test.test | RUN | 0.267632 | 1.133260 | 2.447949 | 0.001577 | 0.001743 | 0.001377 |
| has_left_shift_assign_test.test | RUN | 0.246186 | 0.896783 | 1.985234 | 0.001391 | 0.001618 | 0.001427 |
| has_greater_test.test | RUN | 0.247120 | 1.220706 | 1.998010 | 0.001658 | 0.001711 | 0.001512 |
| has_greater_equal_test.test | RUN | 0.249715 | 1.194866 | 1.931610 | 0.001559 | 0.001592 | 0.001708 |
| has_equal_to_test.test | RUN | 0.252652 | 1.194328 | 2.007312 | 0.001639 | 0.001736 | 0.001465 |
| has_divides_test.test | RUN | 0.243870 | 0.904177 | 1.983320 | 0.001631 | 0.001636 | 0.001353 |
| has_divides_assign_test.test | RUN | 0.249677 | 0.886829 | 1.930198 | 0.001545 | 0.001666 | 0.001477 |
| has_dereference_test.test | RUN | 0.241654 | 0.886339 | 1.873371 | 0.001619 | 0.001878 | 0.001677 |
| has_complement_test.test | RUN | 0.242084 | 1.210577 | 1.902570 | 0.001668 | 0.001862 | 0.001626 |
| has_bit_xor_test.test | RUN | 0.250271 | 0.870609 | 1.977680 | 0.001628 | 0.001674 | 0.001532 |
| has_bit_xor_assign_test.test | RUN | 0.245686 | 0.918991 | 1.991741 | 0.001668 | 0.001729 | 0.001269 |
| has_bit_or_test.test | RUN | 0.242053 | 0.919027 | 1.933713 | 0.001620 | 0.001766 | 0.001692 |
| has_bit_or_assign_test.test | RUN | 0.244036 | 0.881197 | 1.935454 | 0.001588 | 0.001754 | 0.001707 |
| has_bit_and_test.test | RUN | 0.246886 | 0.872666 | 1.993880 | 0.001656 | 0.001782 | 0.001471 |
| has_bit_and_assign_test.test | RUN | 0.243795 | 0.897845 | 1.992553 | 0.001705 | 0.001817 | 0.001308 |
| has_binary_classes9_test.test | RUN | 0.264539 | 1.046920 | 2.140080 | 0.001538 | 0.001778 | 0.001811 |
| has_binary_classes8_test.test | RUN | 0.265474 | 1.055156 | 2.125308 | 0.001692 | 0.001802 | 0.001601 |
| has_binary_classes7_test.test | RUN | 0.272904 | 1.086031 | 2.180007 | 0.001775 | 0.001762 | 0.001619 |
| has_binary_classes6_test.test | RUN | 0.261658 | 1.001844 | 2.187440 | 0.001748 | 0.001868 | 0.001268 |
| has_binary_classes5_test.test | RUN | 0.263568 | 1.037640 | 2.127929 | 0.001730 | 0.001838 | 0.001620 |
| has_binary_classes4_test.test | RUN | 0.267314 | 1.044592 | 2.137324 | 0.001620 | 0.001921 | 0.001709 |
| has_binary_classes3_test.test | RUN | 0.261321 | 1.080529 | 2.179852 | 0.001735 | 0.001822 | 0.001260 |
| has_binary_classes2_test.test | RUN | 0.271251 | 1.083657 | 2.170461 | 0.001696 | 0.001658 | 0.001581 |
| has_binary_classes1_test.test | RUN | 0.263468 | 1.052862 | 2.135297 | 0.001612 | 0.001637 | 0.001649 |
| has_binary_classes0_test.test | RUN | 0.268024 | 1.074631 | 2.081692 | 0.001337 | 0.001708 | 0.001708 |
| function_traits_test.test | RUN | 0.124184 | 0.283427 | 0.461392 | 0.001532 | 0.001758 | 0.001386 |
| floating_point_promotion_test.test | RUN | 0.122198 | 0.262689 | 0.430042 | 0.001679 | 0.001552 | 0.001428 |
| extent_test.test | RUN | 0.126616 | 0.270772 | 0.452028 | 0.001672 | 0.001548 | 0.001663 |
| enable_if_test.test | RUN | 0.117216 | 0.239743 | 0.356779 | 0.001455 | 0.001459 | 0.001745 |
| disjunction_test.test | RUN | 0.120870 | 0.248677 | 0.361346 | 0.001686 | 0.001582 | 0.001425 |
| detected_test.test | RUN | 0.112378 | 0.237409 | 0.335393 | 0.001672 | 0.001645 | 0.001444 |
| detected_or_test.test | RUN | 0.115939 | 0.257352 | 0.381230 | 0.001750 | 0.001633 | 0.001536 |
| decay_test.test | RUN | 0.282264 | 0.983607 | 2.284843 | 0.001543 | 0.001414 | 0.001628 |
| cxx14_aliases_test.test | RUN | 0.197503 | 0.372957 | 0.604665 | 0.001542 | 0.001688 | 0.001594 |
| copy_reference_test.test | RUN | 0.117583 | 0.241008 | 0.350416 | 0.001448 | 0.001648 | 0.001599 |
| copy_cv_test.test | RUN | 0.122443 | 0.259433 | 0.400798 | 0.001401 | 0.001374 | 0.001680 |
| copy_cv_ref_test.test | RUN | 0.164735 | 0.449058 | 0.928059 | 0.001502 | 0.001694 | 0.001755 |
| conjunction_test.test | RUN | 0.117330 | 0.246654 | 0.386746 | 0.001587 | 0.001322 | 0.001458 |
| conditional_test.test | RUN | 0.119098 | 0.240935 | 0.375097 | 0.001492 | 0.001356 | 0.001430 |
| common_type_test.test | RUN | 0.156165 | 0.376095 | 0.752750 | 0.001560 | 0.001668 | 0.001465 |
| common_type_sfinae_test.test | RUN | 0.117639 | 0.228910 | 0.316993 | 0.001535 | 0.001574 | 0.001753 |
| common_type_sfinae2_test.test | RUN | 0.120708 | 0.250913 | 0.334770 | 0.001554 | 0.001570 | 0.001459 |
| common_type_6_test.test | RUN | 0.121697 | 0.248847 | 0.369185 | 0.001496 | 0.001299 | 0.001757 |
| common_type_5_test.test | RUN | 0.121907 | 0.247992 | 0.401271 | 0.001570 | 0.001785 | 0.001499 |
| common_type_4_test.test | RUN | 0.129688 | 0.276053 | 0.464935 | 0.001379 | 0.001615 | 0.001536 |
| common_type_3_test.test | RUN | 0.116690 | 0.238161 | 0.301155 | 0.001514 | 0.001615 | 0.001411 |
| common_type_2_test.test | RUN | 0.152573 | 0.375805 | 0.717008 | 0.001394 | 0.001796 | 0.001538 |
| alignment_of_test.test | RUN | 0.126140 | 0.292317 | 0.467638 | 0.001513 | 0.001814 | 0.001189 |
| alignment_of_a2_test.test | RUN | 0.125476 | 0.300264 | 0.457140 | 0.001720 | 0.001658 | 0.001543 |
| aligned_storage_test.test | RUN | 0.135199 | 0.298831 | 0.421994 | 0.001630 | 0.001736 | 0.001493 |
| aligned_storage_empy_test.test | RUN | 0.147817 | 0.329918 | 0.562599 | 0.001638 | 0.001808 | 0.001715 |
| aligned_storage_a2_test.test | RUN | 0.132014 | 0.283642 | 0.422762 | 0.001872 | 0.001903 | 0.001941 |
| add_volatile_test.test | RUN | 0.255329 | 0.848469 | 2.183524 | 0.001746 | 0.001676 | 0.001787 |
| add_rvalue_reference_test.test | RUN | 0.290303 | 0.956738 | 2.450947 | 0.001641 | 0.001824 | 0.001737 |
| add_reference_test.test | RUN | 0.283029 | 0.949432 | 2.518968 | 0.001363 | 0.001800 | 0.001826 |
| add_pointer_test.test | RUN | 0.184615 | 0.544716 | 1.296064 | 0.001568 | 0.001328 | 0.001481 |
| add_lvalue_reference_test.test | RUN | 0.283682 | 0.945102 | 2.500806 | 0.001576 | 0.001720 | 0.001596 |
| add_cv_test.test | RUN | 0.271966 | 0.875143 | 2.329838 | 0.001545 | 0.001612 | 0.001861 |
| add_const_test.test | RUN | 0.264287 | 0.926463 | 2.317920 | 0.001343 | 0.001518 | 0.001809 |
| is_virtual_base_of_fail.test | COMPILE_FAIL | 0.115972 | 0.235936 | 0.347398 | — | — | — |
| is_stateless_fail.test | COMPILE_FAIL | 0.118422 | 0.238867 | 0.360661 | — | — | — |
| is_polymorphic_fail.test | COMPILE_FAIL | 0.110377 | 0.222067 | 0.343816 | — | — | — |
| is_nothrow_move_constructible_fail.test | COMPILE_FAIL | 0.110732 | 0.224805 | 0.306291 | — | — | — |
| is_nothrow_move_assignable_fail.test | COMPILE_FAIL | 0.111036 | 0.230968 | 0.298170 | — | — | — |
| is_list_constructible_fail.test | COMPILE_FAIL | 0.114835 | 0.222662 | 0.327350 | — | — | — |
| is_empty_fail.test | COMPILE_FAIL | 0.114932 | 0.226769 | 0.337145 | — | — | — |
| is_destructible_fail.test | COMPILE_FAIL | 0.113183 | 0.216693 | 0.328651 | — | — | — |
| is_default_constructible_fail.test | COMPILE_FAIL | 0.111920 | 0.225586 | 0.332138 | — | — | — |
| is_copy_constructible_fail.test | COMPILE_FAIL | 0.113983 | 0.224364 | 0.330505 | — | — | — |
| is_copy_assignable_fail.test | COMPILE_FAIL | 0.111249 | 0.225890 | 0.332517 | — | — | — |
| is_convertible_fail.test | COMPILE_FAIL | 0.113357 | 0.223423 | 0.327336 | — | — | — |
| is_convertible2_fail.test | COMPILE_FAIL | 0.113847 | 0.228380 | 0.330466 | — | — | — |
| is_constructible_fail.test | COMPILE_FAIL | 0.115685 | 0.228391 | 0.332620 | — | — | — |
| is_base_of_fail.test | COMPILE_FAIL | 0.110918 | 0.226183 | 0.333758 | — | — | — |
| is_assignable_fail.test | COMPILE_FAIL | 0.109737 | 0.219084 | 0.328683 | — | — | — |
| has_nothrow_destruct_fail.test | COMPILE_FAIL | 0.113066 | 0.224934 | 0.329781 | — | — | — |
| has_nothrow_copy_fail.test | COMPILE_FAIL | 0.114014 | 0.223458 | 0.336001 | — | — | — |
| has_nothrow_cons_fail.test | COMPILE_FAIL | 0.112490 | 0.230031 | 0.334929 | — | — | — |
| has_nothrow_assign_fail.test | COMPILE_FAIL | 0.118431 | 0.224890 | 0.327616 | — | — | — |
| common_type_fail.test | COMPILE_FAIL | 0.112877 | 0.224335 | 0.331524 | — | — | — |
| common_type2_fail.test | COMPILE_FAIL | 0.113777 | 0.229100 | 0.328222 | — | — | — |
| has_nothrow_assign_test_no_intrinsics.test | RUN | 0.174620 | 0.496568 | 0.960027 | 0.001777 | 0.001666 | 0.001580 |
| has_nothrow_constr_test_no_intrinsics.test | RUN | 0.170660 | 0.486296 | 0.937288 | 0.001544 | 0.001665 | 0.001602 |
| has_nothrow_copy_test_no_intrinsics.test | RUN | 0.172396 | 0.490343 | 0.944480 | 0.001186 | 0.001640 | 0.001622 |
| is_nothrow_move_assignable_test_no_intrinsics.test | RUN | 0.177037 | 0.487491 | 0.896456 | 0.001311 | 0.001194 | 0.001278 |
| is_nothrow_move_constructible_test_no_intrinsics.test | RUN | 0.120388 | 0.352847 | 0.544989 | 0.001276 | 0.001312 | 0.001260 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.951×† | 0.915×, 0.986× | 1.052× |
| clang | 0.996× | 0.997×, 0.995× | 0.963× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| add_const_test.test | 10 | 0.001071s | 0.000982s | 0.001090s | byte equality observed |
| add_cv_test.test | 10 | 0.001091s | 0.000981s | 0.001079s | byte equality observed |
| add_lvalue_reference_test.test | 10 | 0.001110s | 0.000976s | 0.001095s | byte equality observed |
| add_pointer_test.test | 10 | 0.001095s | 0.001025s | 0.001054s | byte equality observed |
| add_reference_test.test | 10 | 0.001093s | 0.000982s | 0.001122s | byte equality observed |
| add_rvalue_reference_test.test | 10 | 0.001095s | 0.001018s | 0.001083s | byte equality observed |
| add_volatile_test.test | 10 | 0.001105s | 0.000964s | 0.001102s | byte equality observed |
| aligned_storage_a2_test.test | 10 | 0.001192s | 0.001110s | 0.001201s | byte equality observed |
| aligned_storage_empy_test.test | 10 | 0.001100s | 0.001009s | 0.001067s | byte equality observed |
| aligned_storage_test.test | 10 | 0.001201s | 0.001088s | 0.001195s | byte equality observed |
| alignment_of_a2_test.test | 10 | 0.001063s | 0.001027s | 0.001071s | byte equality observed |
| alignment_of_test.test | 10 | 0.001105s | 0.000985s | 0.001095s | byte equality observed |
| common_type_2_test.test | 10 | 0.001094s | 0.001002s | 0.001054s | byte equality observed |
| common_type_3_test.test | 10 | 0.001082s | 0.001003s | 0.001082s | byte equality observed |
| common_type_4_test.test | 10 | 0.001089s | 0.000986s | 0.001070s | byte equality observed |
| common_type_5_test.test | 10 | 0.001088s | 0.001025s | 0.001100s | byte equality observed |
| common_type_6_test.test | 10 | 0.001099s | 0.001026s | 0.001109s | byte equality observed |
| common_type_sfinae2_test.test | 10 | 0.001104s | 0.000971s | 0.001081s | byte equality observed |
| common_type_sfinae_test.test | 10 | 0.001096s | 0.001019s | 0.001117s | byte equality observed |
| common_type_test.test | 10 | 0.001106s | 0.001003s | 0.001104s | byte equality observed |
| conditional_test.test | 10 | 0.001058s | 0.001012s | 0.001099s | byte equality observed |
| conjunction_test.test | 10 | 0.001107s | 0.000996s | 0.001099s | byte equality observed |
| copy_cv_ref_test.test | 10 | 0.001078s | 0.000990s | 0.001087s | byte equality observed |
| copy_cv_test.test | 10 | 0.001095s | 0.001018s | 0.001104s | byte equality observed |
| copy_reference_test.test | 10 | 0.001089s | 0.000982s | 0.001075s | byte equality observed |
| cxx14_aliases_test.test | 10 | 0.001120s | 0.000986s | 0.001099s | byte equality observed |
| decay_test.test | 10 | 0.001117s | 0.001004s | 0.001105s | byte equality observed |
| detected_or_test.test | 10 | 0.001142s | 0.001005s | 0.001075s | byte equality observed |
| detected_test.test | 10 | 0.001158s | 0.001000s | 0.001097s | byte equality observed |
| disjunction_test.test | 10 | 0.001162s | 0.001013s | 0.001079s | byte equality observed |
| enable_if_test.test | 10 | 0.001170s | 0.001016s | 0.001105s | byte equality observed |
| extent_test.test | 10 | 0.001142s | 0.001002s | 0.001061s | byte equality observed |
| floating_point_promotion_test.test | 10 | 0.001164s | 0.000995s | 0.001105s | byte equality observed |
| function_traits_test.test | 10 | 0.001148s | 0.001002s | 0.001090s | byte equality observed |
| has_binary_classes0_test.test | 10 | 0.001143s | 0.000986s | 0.001109s | byte equality observed |
| has_binary_classes1_test.test | 10 | 0.001155s | 0.001001s | 0.001110s | byte equality observed |
| has_binary_classes2_test.test | 10 | 0.001146s | 0.001001s | 0.001097s | byte equality observed |
| has_binary_classes3_test.test | 10 | 0.001177s | 0.001004s | 0.001125s | byte equality observed |
| has_binary_classes4_test.test | 10 | 0.001141s | 0.000998s | 0.001080s | byte equality observed |
| has_binary_classes5_test.test | 10 | 0.001143s | 0.000992s | 0.001101s | byte equality observed |
| has_binary_classes6_test.test | 10 | 0.001165s | 0.001002s | 0.001111s | byte equality observed |
| has_binary_classes7_test.test | 10 | 0.001165s | 0.000978s | 0.001083s | byte equality observed |
| has_binary_classes8_test.test | 10 | 0.001156s | 0.001035s | 0.001100s | byte equality observed |
| has_binary_classes9_test.test | 10 | 0.001168s | 0.000998s | 0.001089s | byte equality observed |
| has_bit_and_assign_test.test | 10 | 0.001172s | 0.001005s | 0.001119s | byte equality observed |
| has_bit_and_test.test | 10 | 0.001149s | 0.001008s | 0.001078s | byte equality observed |
| has_bit_or_assign_test.test | 10 | 0.001176s | 0.000989s | 0.001104s | byte equality observed |
| has_bit_or_test.test | 10 | 0.001156s | 0.000998s | 0.001110s | byte equality observed |
| has_bit_xor_assign_test.test | 10 | 0.001171s | 0.001040s | 0.001090s | byte equality observed |
| has_bit_xor_test.test | 10 | 0.001174s | 0.000991s | 0.001132s | byte equality observed |
| has_complement_test.test | 10 | 0.001159s | 0.001007s | 0.001085s | byte equality observed |
| has_dereference_test.test | 10 | 0.001167s | 0.001016s | 0.001099s | byte equality observed |
| has_divides_assign_test.test | 10 | 0.001168s | 0.000991s | 0.001135s | byte equality observed |
| has_divides_test.test | 10 | 0.001168s | 0.001010s | 0.001111s | byte equality observed |
| has_equal_to_test.test | 10 | 0.001177s | 0.000975s | 0.001089s | byte equality observed |
| has_greater_equal_test.test | 10 | 0.001161s | 0.001001s | 0.001120s | byte equality observed |
| has_greater_test.test | 10 | 0.001162s | 0.001008s | 0.001073s | byte equality observed |
| has_left_shift_assign_test.test | 10 | 0.001168s | 0.001011s | 0.001108s | byte equality observed |
| has_left_shift_test.test | 10 | 0.001163s | 0.000992s | 0.001094s | byte equality observed |
| has_less_equal_test.test | 10 | 0.001155s | 0.001003s | 0.001091s | byte equality observed |
| has_less_test.test | 10 | 0.001169s | 0.001021s | 0.001122s | byte equality observed |
| has_logical_and_test.test | 10 | 0.001176s | 0.001013s | 0.001106s | byte equality observed |
| has_logical_not_test.test | 10 | 0.001159s | 0.001016s | 0.001107s | byte equality observed |
| has_logical_or_test.test | 10 | 0.001176s | 0.001016s | 0.001093s | byte equality observed |
| has_minus_assign_test.test | 10 | 0.001173s | 0.000983s | 0.001086s | byte equality observed |
| has_minus_test.test | 10 | 0.001180s | 0.001018s | 0.001079s | byte equality observed |
| has_modulus_assign_test.test | 10 | 0.001168s | 0.001002s | 0.001126s | byte equality observed |
| has_modulus_test.test | 10 | 0.001172s | 0.000997s | 0.001089s | byte equality observed |
| has_multiplies_assign_test.test | 10 | 0.001181s | 0.000990s | 0.001096s | byte equality observed |
| has_multiplies_test.test | 10 | 0.001161s | 0.000998s | 0.001100s | byte equality observed |
| has_negate_test.test | 10 | 0.001140s | 0.000996s | 0.001107s | byte equality observed |
| has_not_equal_to_test.test | 10 | 0.001140s | 0.001008s | 0.001092s | byte equality observed |
| has_nothrow_assign_test.test | 10 | 0.001160s | 0.000993s | 0.001106s | byte equality observed |
| has_nothrow_assign_test_no_intrinsics.test | 10 | 0.001190s | 0.001007s | 0.001075s | byte equality observed |
| has_nothrow_constr_test.test | 10 | 0.001171s | 0.000989s | 0.001070s | byte equality observed |
| has_nothrow_constr_test_no_intrinsics.test | 10 | 0.001174s | 0.000998s | 0.001101s | byte equality observed |
| has_nothrow_copy_test.test | 10 | 0.001171s | 0.001017s | 0.001089s | byte equality observed |
| has_nothrow_copy_test_no_intrinsics.test | 10 | 0.001164s | 0.000994s | 0.001143s | byte equality observed |
| has_nothrow_destructor_test.test | 10 | 0.001177s | 0.000995s | 0.001081s | byte equality observed |
| has_operator_new_test.test | 10 | 0.001180s | 0.000994s | 0.001110s | byte equality observed |
| has_plus_assign_test.test | 10 | 0.001173s | 0.000985s | 0.001084s | byte equality observed |
| has_plus_test.test | 10 | 0.001155s | 0.001030s | 0.001090s | byte equality observed |
| has_post_decrement_test.test | 10 | 0.001162s | 0.000981s | 0.001093s | byte equality observed |
| has_post_increment_test.test | 10 | 0.001172s | 0.000982s | 0.001107s | byte equality observed |
| has_postfix_classes0_test.test | 10 | 0.001171s | 0.001039s | 0.001088s | byte equality observed |
| has_postfix_classes1_test.test | 10 | 0.001153s | 0.001021s | 0.001109s | byte equality observed |
| has_postfix_classes2_test.test | 10 | 0.001152s | 0.000991s | 0.001132s | byte equality observed |
| has_postfix_classes3_test.test | 10 | 0.001172s | 0.000997s | 0.001201s | byte equality observed |
| has_pre_decrement_test.test | 10 | 0.001156s | 0.001017s | 0.001141s | byte equality observed |
| has_pre_increment_test.test | 10 | 0.001164s | 0.001028s | 0.001130s | byte equality observed |
| has_prefix_classes0_test.test | 10 | 0.001143s | 0.001001s | 0.001158s | byte equality observed |
| has_prefix_classes1_test.test | 10 | 0.001157s | 0.000995s | 0.001183s | byte equality observed |
| has_prefix_classes2_test.test | 10 | 0.001189s | 0.000996s | 0.001173s | byte equality observed |
| has_prefix_classes3_test.test | 10 | 0.001165s | 0.001002s | 0.001177s | byte equality observed |
| has_right_shift_assign_test.test | 10 | 0.001170s | 0.000995s | 0.001165s | byte equality observed |
| has_right_shift_test.test | 10 | 0.001164s | 0.001013s | 0.001157s | byte equality observed |
| has_trivial_assign_test.test | 10 | 0.001215s | 0.001052s | 0.001225s | byte equality observed |
| has_trivial_constr_test.test | 10 | 0.001212s | 0.001079s | 0.001231s | native test oracle; output varies |
| has_trivial_copy_test.test | 10 | 0.001228s | 0.001097s | 0.001241s | native test oracle; output varies |
| has_trivial_destructor_test.test | 10 | 0.001141s | 0.001007s | 0.001158s | byte equality observed |
| has_trivial_move_assign_test.test | 10 | 0.001143s | 0.001012s | 0.001148s | byte equality observed |
| has_trivial_move_constructor_test.test | 10 | 0.001161s | 0.001011s | 0.001171s | byte equality observed |
| has_unary_minus_test.test | 10 | 0.001168s | 0.001022s | 0.001163s | byte equality observed |
| has_unary_plus_test.test | 10 | 0.001150s | 0.001025s | 0.001162s | byte equality observed |
| has_virtual_destructor_test.test | 10 | 0.001191s | 0.001014s | 0.001138s | byte equality observed |
| integral_constant_test.test | 10 | 0.000526s | 0.000482s | 0.001175s | byte equality observed |
| is_abstract_test.test | 10 | 0.001189s | 0.001003s | 0.001172s | byte equality observed |
| is_arithmetic_test.test | 10 | 0.001184s | 0.001045s | 0.001140s | byte equality observed |
| is_array_test.test | 10 | 0.001167s | 0.001029s | 0.001179s | byte equality observed |
| is_assignable_test.test | 10 | 0.001190s | 0.000993s | 0.001157s | byte equality observed |
| is_base_and_derived_test.test | 10 | 0.001159s | 0.001009s | 0.001160s | byte equality observed |
| is_base_of_test.test | 10 | 0.001161s | 0.001024s | 0.001170s | byte equality observed |
| is_bounded_array_test.test | 10 | 0.001166s | 0.001059s | 0.001181s | byte equality observed |
| is_class_test.test | 10 | 0.001187s | 0.000983s | 0.001163s | byte equality observed |
| is_complete_test.test | 10 | 0.001184s | 0.001027s | 0.001187s | byte equality observed |
| is_complex_test.test | 10 | 0.001165s | 0.001000s | 0.001149s | byte equality observed |
| is_compound_test.test | 10 | 0.001185s | 0.001021s | 0.001174s | byte equality observed |
| is_const_test.test | 10 | 0.001161s | 0.000993s | 0.001161s | byte equality observed |
| is_constructible_test.test | 10 | 0.001181s | 0.001002s | 0.001182s | byte equality observed |
| is_convertible_test.test | 10 | 0.001180s | 0.001001s | 0.001180s | byte equality observed |
| is_copy_assignable_test.test | 10 | 0.001173s | 0.001011s | 0.001177s | byte equality observed |
| is_copy_constructible_test.test | 10 | 0.001173s | 0.001032s | 0.001176s | byte equality observed |
| is_default_constr_test.test | 10 | 0.001167s | 0.001058s | 0.001158s | byte equality observed |
| is_destructible_test.test | 10 | 0.001164s | 0.001012s | 0.001160s | byte equality observed |
| is_detected_exact_test.test | 10 | 0.001169s | 0.001068s | 0.001178s | byte equality observed |
| is_detected_test.test | 10 | 0.001169s | 0.001079s | 0.001184s | byte equality observed |
| is_empty_test.test | 10 | 0.001159s | 0.001063s | 0.001178s | byte equality observed |
| is_enum_test.test | 10 | 0.001168s | 0.001070s | 0.001177s | byte equality observed |
| is_final_test.test | 10 | 0.001162s | 0.001144s | 0.001194s | byte equality observed |
| is_float_test.test | 10 | 0.001167s | 0.001115s | 0.001197s | byte equality observed |
| is_floating_point_test.test | 10 | 0.001179s | 0.001136s | 0.001171s | byte equality observed |
| is_function_test.test | 10 | 0.001165s | 0.001152s | 0.001181s | byte equality observed |
| is_fundamental_test.test | 10 | 0.001171s | 0.001126s | 0.001169s | byte equality observed |
| is_integral_test.test | 10 | 0.001185s | 0.001156s | 0.001185s | byte equality observed |
| is_list_constructible_test.test | 10 | 0.001177s | 0.001160s | 0.001166s | byte equality observed |
| is_lvalue_reference_test.test | 10 | 0.001175s | 0.001167s | 0.001157s | byte equality observed |
| is_member_func_test.test | 10 | 0.001173s | 0.001166s | 0.001181s | byte equality observed |
| is_member_obj_test.test | 10 | 0.001176s | 0.001143s | 0.001185s | byte equality observed |
| is_member_pointer_test.test | 10 | 0.001159s | 0.001173s | 0.001212s | byte equality observed |
| is_noncopyable_test.test | 10 | 0.001190s | 0.001147s | 0.001161s | byte equality observed |
| is_nothrow_move_assignable_test.test | 10 | 0.001174s | 0.001146s | 0.001172s | byte equality observed |
| is_nothrow_move_assignable_test_no_intrinsics.test | 10 | 0.001160s | 0.001169s | 0.001151s | byte equality observed |
| is_nothrow_move_constructible_test.test | 10 | 0.001168s | 0.001157s | 0.001189s | byte equality observed |
| is_nothrow_move_constructible_test_no_intrinsics.test | 10 | 0.001172s | 0.001136s | 0.001189s | byte equality observed |
| is_nothrow_swappable_test.test | 10 | 0.001161s | 0.001151s | 0.001174s | byte equality observed |
| is_object_test.test | 10 | 0.001170s | 0.001146s | 0.001188s | byte equality observed |
| is_pod_test.test | 10 | 0.001172s | 0.001159s | 0.001158s | byte equality observed |
| is_pointer_test.test | 10 | 0.001180s | 0.001146s | 0.001166s | byte equality observed |
| is_polymorphic_test.test | 10 | 0.001193s | 0.001164s | 0.001191s | byte equality observed |
| is_reference_test.test | 10 | 0.001169s | 0.001162s | 0.001166s | byte equality observed |
| is_rvalue_reference_test.test | 10 | 0.001163s | 0.001186s | 0.001167s | byte equality observed |
| is_same_test.test | 10 | 0.001164s | 0.001143s | 0.001184s | byte equality observed |
| is_scalar_test.test | 10 | 0.001166s | 0.001176s | 0.001161s | byte equality observed |
| is_scoped_enum_test.test | 10 | 0.001167s | 0.001172s | 0.001199s | byte equality observed |
| is_signed_test.test | 10 | 0.001172s | 0.001154s | 0.001158s | byte equality observed |
| is_stateless_test.test | 10 | 0.001166s | 0.001164s | 0.001165s | byte equality observed |
| is_swappable_test.test | 10 | 0.001169s | 0.001157s | 0.001149s | byte equality observed |
| is_trivially_copyable_test.test | 10 | 0.001197s | 0.001197s | 0.001203s | byte equality observed |
| is_unbounded_array_test.test | 10 | 0.001176s | 0.001158s | 0.001166s | byte equality observed |
| is_union_test.test | 10 | 0.001165s | 0.001149s | 0.001166s | byte equality observed |
| is_unscoped_enum_test.test | 10 | 0.001169s | 0.001145s | 0.001146s | byte equality observed |
| is_unsigned_test.test | 10 | 0.001163s | 0.001161s | 0.001145s | byte equality observed |
| is_virtual_base_of_test.test | 10 | 0.001160s | 0.001136s | 0.001162s | byte equality observed |
| is_void_test.test | 10 | 0.001172s | 0.001160s | 0.001175s | byte equality observed |
| is_volatile_test.test | 10 | 0.001170s | 0.001167s | 0.001176s | byte equality observed |
| make_signed_test.test | 10 | 0.001180s | 0.001159s | 0.001190s | byte equality observed |
| make_unsigned_test.test | 10 | 0.001171s | 0.001154s | 0.001176s | byte equality observed |
| make_void_test.test | 10 | 0.001182s | 0.001169s | 0.001179s | byte equality observed |
| mpl_interop_test1.test | 10 | 0.000531s | 0.000523s | 0.001185s | byte equality observed |
| mpl_interop_test2.test | 10 | 0.000538s | 0.000517s | 0.001167s | byte equality observed |
| mpl_interop_test3.test | 10 | 0.000533s | 0.000532s | 0.001182s | byte equality observed |
| negation_test.test | 10 | 0.001208s | 0.001202s | 0.001169s | byte equality observed |
| promote_basic_test.test | 10 | 0.000535s | 0.000520s | 0.001178s | byte equality observed |
| promote_enum_msvc_bug_test.test | 10 | 0.000536s | 0.000533s | 0.001149s | byte equality observed |
| promote_enum_test.test | 10 | 0.000536s | 0.000531s | 0.001192s | byte equality observed |
| promote_mpl_test.test | 10 | 0.000533s | 0.000528s | 0.001180s | byte equality observed |
| rank_test.test | 10 | 0.001174s | 0.001166s | 0.001162s | byte equality observed |
| remove_all_extents_test.test | 10 | 0.001166s | 0.001164s | 0.001188s | byte equality observed |
| remove_bounds_test.test | 10 | 0.001187s | 0.001165s | 0.001188s | byte equality observed |
| remove_const_test.test | 10 | 0.001176s | 0.001157s | 0.001171s | byte equality observed |
| remove_cv_ref_test.test | 10 | 0.001157s | 0.001144s | 0.001180s | byte equality observed |
| remove_cv_test.test | 10 | 0.001176s | 0.001174s | 0.001186s | byte equality observed |
| remove_extent_test.test | 10 | 0.001175s | 0.001161s | 0.001168s | byte equality observed |
| remove_pointer_test.test | 10 | 0.001180s | 0.001188s | 0.001180s | byte equality observed |
| remove_reference_test.test | 10 | 0.001175s | 0.001152s | 0.001192s | byte equality observed |
| remove_volatile_test.test | 10 | 0.001160s | 0.001169s | 0.001178s | byte equality observed |
| tricky_abstract_type_test.test | 10 | 0.001165s | 0.001147s | 0.001175s | byte equality observed |
| tricky_add_pointer_test.test | 10 | 0.001170s | 0.001161s | 0.001193s | byte equality observed |
| tricky_function_type_test.test | 10 | 0.001172s | 0.001143s | 0.001188s | byte equality observed |
| tricky_incomplete_type_test.test | 10 | 0.001175s | 0.001174s | 0.001171s | byte equality observed |
| tricky_is_enum_test.test | 10 | 0.001166s | 0.001158s | 0.001165s | byte equality observed |
| tricky_partial_spec_test.test | 10 | 0.001152s | 0.001168s | 0.001182s | byte equality observed |
| tricky_rvalue_test.test | 10 | 0.001179s | 0.001148s | 0.001179s | byte equality observed |
| type_identity_test.test | 10 | 0.001186s | 0.001183s | 0.001203s | byte equality observed |
| type_traits_test.test | 10 | 0.000529s | 0.000524s | 0.001160s | byte equality observed |
| type_with_alignment_test.test | 10 | 0.001320s | 0.001310s | 0.001317s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
