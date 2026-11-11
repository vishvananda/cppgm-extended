# libs/type_traits/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| type_with_alignment_test.test | RUN | 0.171126 | 0.422871 | 0.594257 | 0.001600 | 0.001654 | 0.001510 |
| type_traits_test.test | RUN | 0.227644 | 0.427570 | 0.590134 | 0.000631 | 0.000783 | 0.001356 |
| type_identity_test.test | RUN | 0.403543 | 1.539390 | 3.425801 | 0.001657 | 0.001721 | 0.001613 |
| tricky_rvalue_test.test | RUN | 0.156823 | 0.329724 | 0.453489 | 0.001692 | 0.001610 | 0.001642 |
| tricky_partial_spec_test.test | RUN | 0.185194 | 0.477244 | 0.750227 | 0.001562 | 0.001564 | 0.001536 |
| tricky_is_enum_test.test | RUN | 0.145250 | 0.312901 | 0.409524 | 0.001634 | 0.001692 | 0.001622 |
| tricky_incomplete_type_test.test | RUN | 0.235082 | 0.488865 | 0.713063 | 0.001654 | 0.001656 | 0.001639 |
| tricky_function_type_test.test | RUN | 0.185733 | 0.511395 | 0.822545 | 0.001680 | 0.001490 | 0.001498 |
| tricky_add_pointer_test.test | RUN | 0.268395 | 0.916061 | 1.902022 | 0.001766 | 0.001663 | 0.001608 |
| tricky_abstract_type_test.test | RUN | 0.151237 | 0.306496 | 0.428708 | 0.001680 | 0.001534 | 0.001580 |
| remove_volatile_test.test | RUN | 0.354910 | 1.324716 | 2.902244 | 0.001614 | 0.001691 | 0.001645 |
| remove_reference_test.test | RUN | 0.375597 | 1.403630 | 3.084141 | 0.001489 | 0.001661 | 0.001666 |
| remove_pointer_test.test | RUN | 0.385545 | 1.461304 | 3.234193 | 0.001724 | 0.001684 | 0.001559 |
| remove_extent_test.test | RUN | 0.375859 | 1.403273 | 3.072757 | 0.001710 | 0.001700 | 0.001437 |
| remove_cv_test.test | RUN | 0.372100 | 1.397384 | 3.068770 | 0.001654 | 0.001642 | 0.001654 |
| remove_cv_ref_test.test | RUN | 0.780568 | 3.360912 | 7.702425 | 0.001606 | 0.001496 | 0.001600 |
| remove_const_test.test | RUN | 0.361791 | 1.319827 | 2.934744 | 0.001649 | 0.001684 | 0.001697 |
| remove_bounds_test.test | RUN | 0.387210 | 1.408088 | 3.085805 | 0.001489 | 0.001685 | 0.001678 |
| remove_all_extents_test.test | RUN | 0.377584 | 1.378400 | 3.103075 | 0.001508 | 0.001473 | 0.001546 |
| rank_test.test | RUN | 0.145550 | 0.330082 | 0.460760 | 0.001596 | 0.001507 | 0.001651 |
| promote_mpl_test.test | RUN | 0.133114 | 0.298589 | 0.329916 | 0.000629 | 0.000735 | 0.001624 |
| promote_enum_test.test | RUN | 0.027907 | 0.070107 | 0.076859 | 0.000589 | 0.000739 | 0.001668 |
| promote_enum_msvc_bug_test.test | RUN | 0.024525 | 0.058122 | 0.061793 | 0.000636 | 0.000723 | 0.001448 |
| promote_basic_test.test | RUN | 0.030676 | 0.083097 | 0.080808 | 0.000662 | 0.000654 | 0.001666 |
| negation_test.test | RUN | 0.142787 | 0.299118 | 0.408491 | 0.001430 | 0.001121 | 0.001464 |
| mpl_interop_test3.test | RUN | 0.127449 | 0.277844 | 0.298803 | 0.000599 | 0.000628 | 0.001382 |
| mpl_interop_test2.test | RUN | 0.050490 | 0.079387 | 0.078561 | 0.000757 | 0.000612 | 0.001629 |
| mpl_interop_test1.test | RUN | 0.021792 | 0.053489 | 0.053250 | 0.000586 | 0.000725 | 0.001608 |
| make_void_test.test | RUN | 0.151223 | 0.325012 | 0.463233 | 0.001696 | 0.001688 | 0.001650 |
| make_unsigned_test.test | RUN | 0.176731 | 0.442616 | 0.655470 | 0.001631 | 0.001598 | 0.001659 |
| make_signed_test.test | RUN | 0.175984 | 0.441990 | 0.661658 | 0.001438 | 0.001564 | 0.001571 |
| is_volatile_test.test | RUN | 0.160424 | 0.366239 | 0.542591 | 0.001457 | 0.001579 | 0.001518 |
| is_void_test.test | RUN | 0.149915 | 0.343064 | 0.481397 | 0.001584 | 0.001711 | 0.001628 |
| is_virtual_base_of_test.test | RUN | 0.178303 | 0.446127 | 0.697363 | 0.001670 | 0.001432 | 0.001599 |
| is_unsigned_test.test | RUN | 0.157422 | 0.355395 | 0.505455 | 0.001423 | 0.001574 | 0.001589 |
| is_unscoped_enum_test.test | RUN | 0.147617 | 0.325293 | 0.432010 | 0.001552 | 0.001504 | 0.001403 |
| is_union_test.test | RUN | 0.164715 | 0.408000 | 0.633775 | 0.001647 | 0.001673 | 0.001670 |
| is_unbounded_array_test.test | RUN | 0.155721 | 0.343584 | 0.489095 | 0.001592 | 0.001622 | 0.001587 |
| is_trivially_copyable_test.test | RUN | 0.248101 | 0.716361 | 1.290080 | 0.001557 | 0.001666 | 0.001630 |
| is_swappable_test.test | RUN | 0.206464 | 0.540858 | 0.934257 | 0.001552 | 0.001531 | 0.001639 |
| is_stateless_test.test | RUN | 0.223931 | 0.630474 | 1.096412 | 0.001718 | 0.001465 | 0.001471 |
| is_signed_test.test | RUN | 0.156117 | 0.363072 | 0.518103 | 0.001580 | 0.001718 | 0.001622 |
| is_scoped_enum_test.test | RUN | 0.148978 | 0.319725 | 0.429870 | 0.001520 | 0.001614 | 0.001454 |
| is_scalar_test.test | RUN | 0.201902 | 0.587536 | 0.969447 | 0.001684 | 0.001411 | 0.001494 |
| is_same_test.test | RUN | 0.149663 | 0.347767 | 0.482012 | 0.001664 | 0.001622 | 0.001614 |
| is_rvalue_reference_test.test | RUN | 0.161030 | 0.384178 | 0.566918 | 0.001503 | 0.001475 | 0.001479 |
| is_reference_test.test | RUN | 0.161398 | 0.390768 | 0.576016 | 0.001599 | 0.001718 | 0.001697 |
| is_polymorphic_test.test | RUN | 0.161510 | 0.405928 | 0.614713 | 0.001617 | 0.001451 | 0.001601 |
| is_pointer_test.test | RUN | 0.161341 | 0.394880 | 0.595826 | 0.001746 | 0.001517 | 0.001383 |
| is_pod_test.test | RUN | 0.216911 | 0.640898 | 1.124514 | 0.001466 | 0.001647 | 0.001642 |
| is_object_test.test | RUN | 0.150372 | 0.329152 | 0.451646 | 0.001251 | 0.001601 | 0.001680 |
| is_nothrow_swappable_test.test | RUN | 0.213789 | 0.598223 | 1.030143 | 0.001674 | 0.001255 | 0.001619 |
| is_nothrow_move_constructible_test.test | RUN | 0.209226 | 0.629129 | 1.072391 | 0.001654 | 0.001353 | 0.001382 |
| is_nothrow_move_assignable_test.test | RUN | 0.214060 | 0.623251 | 1.074488 | 0.001582 | 0.001546 | 0.001444 |
| is_noncopyable_test.test | RUN | 0.150067 | 0.333313 | 0.453561 | 0.001470 | 0.001358 | 0.001565 |
| is_member_pointer_test.test | RUN | 0.148009 | 0.342810 | 0.477765 | 0.001608 | 0.001658 | 0.001625 |
| is_member_obj_test.test | RUN | 0.152428 | 0.343849 | 0.488856 | 0.001718 | 0.001394 | 0.001580 |
| is_member_func_test.test | RUN | 0.156541 | 0.366928 | 0.526060 | 0.001611 | 0.001500 | 0.001626 |
| is_lvalue_reference_test.test | RUN | 0.159202 | 0.389113 | 0.575130 | 0.001478 | 0.001600 | 0.001629 |
| is_list_constructible_test.test | RUN | 0.185466 | 0.454139 | 0.640111 | 0.001607 | 0.001564 | 0.001577 |
| is_integral_test.test | RUN | 0.190190 | 0.546366 | 0.900591 | 0.001718 | 0.001482 | 0.001618 |
| is_fundamental_test.test | RUN | 0.197573 | 0.556606 | 0.918062 | 0.001534 | 0.001322 | 0.001525 |
| is_function_test.test | RUN | 0.152981 | 0.365131 | 0.505211 | 0.001484 | 0.001481 | 0.001586 |
| is_floating_point_test.test | RUN | 0.160932 | 0.391893 | 0.576402 | 0.001438 | 0.001531 | 0.001634 |
| is_float_test.test | RUN | 0.162929 | 0.389446 | 0.577975 | 0.001824 | 0.001557 | 0.001569 |
| is_final_test.test | RUN | 0.165903 | 0.396635 | 0.598609 | 0.001422 | 0.001417 | 0.001405 |
| is_enum_test.test | RUN | 0.149257 | 0.330273 | 0.454285 | 0.001514 | 0.001573 | 0.001608 |
| is_empty_test.test | RUN | 0.158857 | 0.363539 | 0.523538 | 0.001616 | 0.001655 | 0.001544 |
| is_detected_test.test | RUN | 0.146177 | 0.307904 | 0.413707 | 0.001565 | 0.001429 | 0.001678 |
| is_detected_exact_test.test | RUN | 0.145002 | 0.308935 | 0.415109 | 0.001547 | 0.001552 | 0.001632 |
| is_destructible_test.test | RUN | 0.209176 | 0.613654 | 1.049035 | 0.001796 | 0.001662 | 0.001372 |
| is_default_constr_test.test | RUN | 0.210362 | 0.635342 | 1.064929 | 0.001689 | 0.001662 | 0.001604 |
| is_copy_constructible_test.test | RUN | 0.222683 | 0.663400 | 1.153008 | 0.001546 | 0.001598 | 0.001604 |
| is_copy_assignable_test.test | RUN | 0.223602 | 0.645584 | 1.135276 | 0.001588 | 0.001539 | 0.001642 |
| is_convertible_test.test | RUN | 0.212661 | 0.587787 | 0.930066 | 0.001545 | 0.001704 | 0.001509 |
| is_constructible_test.test | RUN | 0.172243 | 0.423501 | 0.652015 | 0.001477 | 0.001562 | 0.001382 |
| is_const_test.test | RUN | 0.154226 | 0.365034 | 0.529494 | 0.001674 | 0.001397 | 0.001491 |
| is_compound_test.test | RUN | 0.147132 | 0.315075 | 0.429558 | 0.001413 | 0.001614 | 0.001514 |
| is_complex_test.test | RUN | 0.199058 | 0.424042 | 0.598575 | 0.001610 | 0.001746 | 0.001633 |
| is_complete_test.test | RUN | 0.172570 | 0.448512 | 0.674655 | 0.001660 | 0.001446 | 0.001454 |
| is_class_test.test | RUN | 0.163663 | 0.393678 | 0.570997 | 0.001720 | 0.001423 | 0.001314 |
| is_bounded_array_test.test | RUN | 0.151943 | 0.356979 | 0.482894 | 0.001629 | 0.001577 | 0.001413 |
| is_base_of_test.test | RUN | 0.159818 | 0.390412 | 0.557274 | 0.001307 | 0.001675 | 0.001628 |
| is_base_and_derived_test.test | RUN | 0.159955 | 0.382664 | 0.543375 | 0.001623 | 0.001595 | 0.001606 |
| is_assignable_test.test | RUN | 0.197358 | 0.547126 | 0.861312 | 0.001434 | 0.001570 | 0.001389 |
| is_array_test.test | RUN | 0.159512 | 0.370881 | 0.522435 | 0.001698 | 0.001450 | 0.001636 |
| is_arithmetic_test.test | RUN | 0.191640 | 0.555964 | 0.904984 | 0.001685 | 0.001727 | 0.001681 |
| is_abstract_test.test | RUN | 0.366725 | 1.490937 | 3.104191 | 0.001764 | 0.001678 | 0.001603 |
| integral_constant_test.test | RUN | 0.022317 | 0.058003 | 0.055321 | 0.000790 | 0.000727 | 0.001632 |
| has_virtual_destructor_test.test | RUN | 0.166272 | 0.439104 | 0.652733 | 0.001632 | 0.001732 | 0.001610 |
| has_unary_plus_test.test | RUN | 0.305951 | 1.690431 | 2.347463 | 0.001685 | 0.001546 | 0.001627 |
| has_unary_minus_test.test | RUN | 0.297975 | 1.626088 | 2.326503 | 0.001652 | 0.001623 | 0.001606 |
| has_trivial_move_constructor_test.test | RUN | 0.221667 | 0.634237 | 1.087104 | 0.001452 | 0.001406 | 0.001542 |
| has_trivial_move_assign_test.test | RUN | 0.216159 | 0.642650 | 1.056436 | 0.001662 | 0.001738 | 0.001579 |
| has_trivial_destructor_test.test | RUN | 0.214691 | 0.649870 | 1.100733 | 0.001558 | 0.001379 | 0.001604 |
| has_trivial_copy_test.test | RUN | 0.224351 | 0.689316 | 1.150899 | 0.001804 | 0.001487 | 0.001696 |
| has_trivial_constr_test.test | RUN | 0.226158 | 0.684441 | 1.166176 | 0.001553 | 0.001700 | 0.001696 |
| has_trivial_assign_test.test | RUN | 0.216125 | 0.676461 | 1.145278 | 0.001708 | 0.001784 | 0.001689 |
| has_right_shift_test.test | RUN | 0.320374 | 1.239953 | 2.481312 | 0.001638 | 0.001636 | 0.001692 |
| has_right_shift_assign_test.test | RUN | 0.309620 | 1.186439 | 2.295934 | 0.001485 | 0.001721 | 0.001596 |
| has_prefix_classes3_test.test | RUN | 0.232744 | 0.701897 | 1.182965 | 0.001433 | 0.001692 | 0.001596 |
| has_prefix_classes2_test.test | RUN | 0.235532 | 0.674348 | 1.169842 | 0.001446 | 0.001586 | 0.001575 |
| has_prefix_classes1_test.test | RUN | 0.233255 | 0.692105 | 1.179745 | 0.001612 | 0.001678 | 0.001493 |
| has_prefix_classes0_test.test | RUN | 0.230954 | 0.697214 | 1.172307 | 0.001416 | 0.001688 | 0.001615 |
| has_pre_increment_test.test | RUN | 0.310577 | 1.590331 | 2.315343 | 0.001510 | 0.001494 | 0.001518 |
| has_pre_decrement_test.test | RUN | 0.307512 | 1.563975 | 2.296644 | 0.001549 | 0.001593 | 0.001564 |
| has_postfix_classes3_test.test | RUN | 0.229344 | 0.697767 | 1.182893 | 0.001667 | 0.001578 | 0.001608 |
| has_postfix_classes2_test.test | RUN | 0.230604 | 0.683497 | 1.174663 | 0.001488 | 0.001462 | 0.001513 |
| has_postfix_classes1_test.test | RUN | 0.231138 | 0.702715 | 1.184836 | 0.001616 | 0.001681 | 0.001625 |
| has_postfix_classes0_test.test | RUN | 0.232397 | 0.701111 | 1.195832 | 0.001439 | 0.001644 | 0.001395 |
| has_post_increment_test.test | RUN | 0.312416 | 1.607841 | 2.364021 | 0.001609 | 0.001492 | 0.001679 |
| has_post_decrement_test.test | RUN | 0.310714 | 1.618389 | 2.392936 | 0.001426 | 0.001520 | 0.001610 |
| has_plus_test.test | RUN | 0.310912 | 1.177212 | 2.364635 | 0.001569 | 0.001593 | 0.001645 |
| has_plus_assign_test.test | RUN | 0.318358 | 1.169572 | 2.402209 | 0.001560 | 0.001627 | 0.001634 |
| has_operator_new_test.test | RUN | 0.215039 | 0.618272 | 1.042969 | 0.001454 | 0.001589 | 0.001588 |
| has_nothrow_destructor_test.test | RUN | 0.226670 | 0.681039 | 1.169037 | 0.001546 | 0.001627 | 0.001602 |
| has_nothrow_copy_test.test | RUN | 0.214466 | 0.641439 | 1.102163 | 0.001611 | 0.001683 | 0.001458 |
| has_nothrow_constr_test.test | RUN | 0.212809 | 0.633234 | 1.081192 | 0.001517 | 0.001580 | 0.001517 |
| has_nothrow_assign_test.test | RUN | 0.212890 | 0.622902 | 1.065585 | 0.001584 | 0.001712 | 0.001556 |
| has_not_equal_to_test.test | RUN | 0.310139 | 1.580855 | 2.369857 | 0.001408 | 0.001691 | 0.001629 |
| has_negate_test.test | RUN | 0.303060 | 1.138988 | 2.235770 | 0.001697 | 0.001449 | 0.001647 |
| has_multiplies_test.test | RUN | 0.312382 | 1.183055 | 2.353743 | 0.001434 | 0.001622 | 0.001429 |
| has_multiplies_assign_test.test | RUN | 0.309037 | 1.154434 | 2.357546 | 0.001588 | 0.001654 | 0.001624 |
| has_modulus_test.test | RUN | 0.310324 | 1.193545 | 2.323163 | 0.001648 | 0.001682 | 0.001609 |
| has_modulus_assign_test.test | RUN | 0.308368 | 1.177287 | 2.324479 | 0.001633 | 0.001778 | 0.001553 |
| has_minus_test.test | RUN | 0.314234 | 1.194440 | 2.371402 | 0.001485 | 0.001692 | 0.001582 |
| has_minus_assign_test.test | RUN | 0.317728 | 1.188474 | 2.359065 | 0.001632 | 0.001658 | 0.001657 |
| has_logical_or_test.test | RUN | 0.307776 | 1.183975 | 2.360633 | 0.001543 | 0.001690 | 0.001648 |
| has_logical_not_test.test | RUN | 0.299246 | 1.154886 | 2.262306 | 0.001293 | 0.001699 | 0.001626 |
| has_logical_and_test.test | RUN | 0.314180 | 1.182629 | 2.364848 | 0.001435 | 0.001694 | 0.001561 |
| has_less_test.test | RUN | 0.314376 | 1.543113 | 2.344859 | 0.001670 | 0.001700 | 0.001608 |
| has_less_equal_test.test | RUN | 0.322248 | 1.542407 | 2.394039 | 0.001615 | 0.001541 | 0.001641 |
| has_left_shift_test.test | RUN | 0.343210 | 1.520497 | 2.939628 | 0.001548 | 0.001585 | 0.001468 |
| has_left_shift_assign_test.test | RUN | 0.313976 | 1.176776 | 2.336653 | 0.001508 | 0.001680 | 0.001577 |
| has_greater_test.test | RUN | 0.310911 | 1.547424 | 2.365371 | 0.001506 | 0.001674 | 0.001631 |
| has_greater_equal_test.test | RUN | 0.312645 | 1.548848 | 2.382261 | 0.001559 | 0.001659 | 0.001630 |
| has_equal_to_test.test | RUN | 0.311727 | 1.559880 | 2.406110 | 0.001534 | 0.001604 | 0.001421 |
| has_divides_test.test | RUN | 0.309788 | 1.158047 | 2.363587 | 0.001458 | 0.001616 | 0.001620 |
| has_divides_assign_test.test | RUN | 0.313661 | 1.164246 | 2.382430 | 0.001572 | 0.001693 | 0.001394 |
| has_dereference_test.test | RUN | 0.301711 | 1.168187 | 2.290732 | 0.001546 | 0.001656 | 0.001620 |
| has_complement_test.test | RUN | 0.301665 | 1.594063 | 2.326262 | 0.001428 | 0.001600 | 0.001437 |
| has_bit_xor_test.test | RUN | 0.306328 | 1.164472 | 2.324207 | 0.001459 | 0.001721 | 0.001716 |
| has_bit_xor_assign_test.test | RUN | 0.307602 | 1.155581 | 2.371515 | 0.001491 | 0.001668 | 0.001667 |
| has_bit_or_test.test | RUN | 0.311176 | 1.177305 | 2.360286 | 0.001617 | 0.001634 | 0.001647 |
| has_bit_or_assign_test.test | RUN | 0.309874 | 1.170003 | 2.339558 | 0.001503 | 0.001675 | 0.001642 |
| has_bit_and_test.test | RUN | 0.309390 | 1.191940 | 2.329807 | 0.001531 | 0.001659 | 0.001540 |
| has_bit_and_assign_test.test | RUN | 0.309625 | 1.168095 | 2.366118 | 0.001661 | 0.001587 | 0.001588 |
| has_binary_classes9_test.test | RUN | 0.331000 | 1.397424 | 2.625395 | 0.001436 | 0.001692 | 0.001639 |
| has_binary_classes8_test.test | RUN | 0.335709 | 1.389258 | 2.611455 | 0.001660 | 0.001640 | 0.001543 |
| has_binary_classes7_test.test | RUN | 0.333555 | 1.387693 | 2.619841 | 0.001548 | 0.001628 | 0.001494 |
| has_binary_classes6_test.test | RUN | 0.324481 | 1.326723 | 2.657118 | 0.001641 | 0.001638 | 0.001654 |
| has_binary_classes5_test.test | RUN | 0.328970 | 1.366998 | 2.663764 | 0.001628 | 0.001675 | 0.001648 |
| has_binary_classes4_test.test | RUN | 0.332439 | 1.378026 | 2.644812 | 0.001622 | 0.001671 | 0.001719 |
| has_binary_classes3_test.test | RUN | 0.336936 | 1.393252 | 2.637840 | 0.001437 | 0.001564 | 0.001561 |
| has_binary_classes2_test.test | RUN | 0.332594 | 1.395007 | 2.627785 | 0.001582 | 0.001586 | 0.001548 |
| has_binary_classes1_test.test | RUN | 0.329156 | 1.397997 | 2.639179 | 0.001436 | 0.001664 | 0.001612 |
| has_binary_classes0_test.test | RUN | 0.332759 | 1.393588 | 2.633784 | 0.001692 | 0.001429 | 0.001616 |
| function_traits_test.test | RUN | 0.155989 | 0.361129 | 0.539984 | 0.001685 | 0.001610 | 0.001639 |
| floating_point_promotion_test.test | RUN | 0.151202 | 0.353024 | 0.501641 | 0.001694 | 0.001600 | 0.001570 |
| extent_test.test | RUN | 0.155949 | 0.363870 | 0.526592 | 0.001518 | 0.001664 | 0.001437 |
| enable_if_test.test | RUN | 0.143006 | 0.307818 | 0.406602 | 0.001439 | 0.001543 | 0.001548 |
| disjunction_test.test | RUN | 0.146640 | 0.314366 | 0.438134 | 0.001711 | 0.001702 | 0.001403 |
| detected_test.test | RUN | 0.144073 | 0.301224 | 0.394928 | 0.001527 | 0.001718 | 0.001621 |
| detected_or_test.test | RUN | 0.149035 | 0.322618 | 0.438002 | 0.001534 | 0.001589 | 0.001594 |
| decay_test.test | RUN | 0.360979 | 1.293289 | 2.725479 | 0.001448 | 0.001644 | 0.001676 |
| cxx14_aliases_test.test | RUN | 0.231511 | 0.476424 | 0.669233 | 0.001520 | 0.001691 | 0.001508 |
| copy_reference_test.test | RUN | 0.145071 | 0.311970 | 0.437669 | 0.001598 | 0.001685 | 0.001489 |
| copy_cv_test.test | RUN | 0.149989 | 0.334883 | 0.471779 | 0.001504 | 0.001471 | 0.001642 |
| copy_cv_ref_test.test | RUN | 0.204542 | 0.577087 | 1.085120 | 0.001468 | 0.001400 | 0.001646 |
| conjunction_test.test | RUN | 0.137380 | 0.315558 | 0.442874 | 0.001543 | 0.001490 | 0.001633 |
| conditional_test.test | RUN | 0.143604 | 0.314909 | 0.432454 | 0.001737 | 0.001582 | 0.001636 |
| common_type_test.test | RUN | 0.188831 | 0.486224 | 0.883618 | 0.001634 | 0.001406 | 0.001476 |
| common_type_sfinae_test.test | RUN | 0.145605 | 0.306208 | 0.393773 | 0.001566 | 0.001423 | 0.001620 |
| common_type_sfinae2_test.test | RUN | 0.150332 | 0.316414 | 0.425154 | 0.001440 | 0.001564 | 0.001459 |
| common_type_6_test.test | RUN | 0.150845 | 0.324987 | 0.453601 | 0.001723 | 0.001429 | 0.001595 |
| common_type_5_test.test | RUN | 0.150336 | 0.320452 | 0.457953 | 0.001666 | 0.001613 | 0.001601 |
| common_type_4_test.test | RUN | 0.160251 | 0.356200 | 0.535902 | 0.001709 | 0.001488 | 0.001612 |
| common_type_3_test.test | RUN | 0.142408 | 0.298682 | 0.383543 | 0.001608 | 0.001626 | 0.001472 |
| common_type_2_test.test | RUN | 0.191834 | 0.485150 | 0.843324 | 0.001554 | 0.001694 | 0.001495 |
| alignment_of_test.test | RUN | 0.155794 | 0.391435 | 0.540757 | 0.001511 | 0.001816 | 0.001559 |
| alignment_of_a2_test.test | RUN | 0.155077 | 0.385218 | 0.543050 | 0.001615 | 0.001502 | 0.001293 |
| aligned_storage_test.test | RUN | 0.163199 | 0.374805 | 0.498677 | 0.001667 | 0.001653 | 0.001650 |
| aligned_storage_empy_test.test | RUN | 0.181097 | 0.434146 | 0.669514 | 0.001698 | 0.001651 | 0.001496 |
| aligned_storage_a2_test.test | RUN | 0.161108 | 0.367394 | 0.488364 | 0.001664 | 0.001772 | 0.001628 |
| add_volatile_test.test | RUN | 0.329181 | 1.154315 | 2.445564 | 0.001634 | 0.001643 | 0.001617 |
| add_rvalue_reference_test.test | RUN | 0.369325 | 1.297916 | 2.841609 | 0.001576 | 0.001550 | 0.001640 |
| add_reference_test.test | RUN | 0.356012 | 1.270942 | 2.800591 | 0.001682 | 0.001661 | 0.001587 |
| add_pointer_test.test | RUN | 0.242241 | 0.717103 | 1.425655 | 0.001577 | 0.001714 | 0.001624 |
| add_lvalue_reference_test.test | RUN | 0.364462 | 1.296298 | 2.795681 | 0.001236 | 0.001531 | 0.001572 |
| add_cv_test.test | RUN | 0.343952 | 1.187450 | 2.617109 | 0.001357 | 0.001762 | 0.001464 |
| add_const_test.test | RUN | 0.343974 | 1.187703 | 2.700436 | 0.001587 | 0.001381 | 0.001526 |
| is_virtual_base_of_fail.test | COMPILE_FAIL | 0.141206 | 0.294550 | 0.387504 | — | — | — |
| is_stateless_fail.test | COMPILE_FAIL | 0.144724 | 0.300269 | 0.401742 | — | — | — |
| is_polymorphic_fail.test | COMPILE_FAIL | 0.140363 | 0.286056 | 0.382396 | — | — | — |
| is_nothrow_move_constructible_fail.test | COMPILE_FAIL | 0.142560 | 0.281771 | 0.378539 | — | — | — |
| is_nothrow_move_assignable_fail.test | COMPILE_FAIL | 0.141063 | 0.278153 | 0.380524 | — | — | — |
| is_list_constructible_fail.test | COMPILE_FAIL | 0.138579 | 0.274861 | 0.381709 | — | — | — |
| is_empty_fail.test | COMPILE_FAIL | 0.143990 | 0.291815 | 0.388172 | — | — | — |
| is_destructible_fail.test | COMPILE_FAIL | 0.140708 | 0.286634 | 0.385943 | — | — | — |
| is_default_constructible_fail.test | COMPILE_FAIL | 0.140464 | 0.287381 | 0.388104 | — | — | — |
| is_copy_constructible_fail.test | COMPILE_FAIL | 0.144296 | 0.288516 | 0.391586 | — | — | — |
| is_copy_assignable_fail.test | COMPILE_FAIL | 0.139386 | 0.290914 | 0.386055 | — | — | — |
| is_convertible_fail.test | COMPILE_FAIL | 0.143128 | 0.287657 | 0.385715 | — | — | — |
| is_convertible2_fail.test | COMPILE_FAIL | 0.141296 | 0.287991 | 0.384009 | — | — | — |
| is_constructible_fail.test | COMPILE_FAIL | 0.138633 | 0.283995 | 0.388402 | — | — | — |
| is_base_of_fail.test | COMPILE_FAIL | 0.139360 | 0.284323 | 0.381190 | — | — | — |
| is_assignable_fail.test | COMPILE_FAIL | 0.139730 | 0.273116 | 0.385129 | — | — | — |
| has_nothrow_destruct_fail.test | COMPILE_FAIL | 0.137730 | 0.275680 | 0.385257 | — | — | — |
| has_nothrow_copy_fail.test | COMPILE_FAIL | 0.140429 | 0.286617 | 0.389819 | — | — | — |
| has_nothrow_cons_fail.test | COMPILE_FAIL | 0.140429 | 0.290205 | 0.382378 | — | — | — |
| has_nothrow_assign_fail.test | COMPILE_FAIL | 0.140224 | 0.285236 | 0.389087 | — | — | — |
| common_type_fail.test | COMPILE_FAIL | 0.140037 | 0.288603 | 0.390212 | — | — | — |
| common_type2_fail.test | COMPILE_FAIL | 0.141262 | 0.287144 | 0.392978 | — | — | — |
| has_nothrow_assign_test_no_intrinsics.test | RUN | 0.224086 | 0.635113 | 1.111705 | 0.001374 | 0.001658 | 0.001678 |
| has_nothrow_constr_test_no_intrinsics.test | RUN | 0.215489 | 0.629466 | 1.088579 | 0.001263 | 0.001443 | 0.001447 |
| has_nothrow_copy_test_no_intrinsics.test | RUN | 0.217675 | 0.634558 | 1.099509 | 0.001519 | 0.001646 | 0.001085 |
| is_nothrow_move_assignable_test_no_intrinsics.test | RUN | 0.216943 | 0.623114 | 1.084712 | 0.001251 | 0.001526 | 0.001149 |
| is_nothrow_move_constructible_test_no_intrinsics.test | RUN | 0.126019 | 0.441104 | 0.727917 | 0.001202 | 0.001084 | 0.001095 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.973×† | 0.998×, 0.948× | 1.007× |
| clang | 1.042× | 1.049×, 1.035× | 0.998× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| add_const_test.test | 10 | 0.000957s | 0.000954s | 0.000987s | byte equality observed |
| add_cv_test.test | 10 | 0.000971s | 0.000953s | 0.000981s | byte equality observed |
| add_lvalue_reference_test.test | 10 | 0.000981s | 0.000946s | 0.001019s | byte equality observed |
| add_pointer_test.test | 10 | 0.000974s | 0.000997s | 0.000960s | byte equality observed |
| add_reference_test.test | 10 | 0.000975s | 0.000945s | 0.000983s | byte equality observed |
| add_rvalue_reference_test.test | 10 | 0.000970s | 0.000950s | 0.000951s | byte equality observed |
| add_volatile_test.test | 10 | 0.000991s | 0.000951s | 0.000967s | byte equality observed |
| aligned_storage_a2_test.test | 10 | 0.001007s | 0.001020s | 0.001046s | byte equality observed |
| aligned_storage_empy_test.test | 10 | 0.001009s | 0.000997s | 0.000974s | byte equality observed |
| aligned_storage_test.test | 10 | 0.001045s | 0.001027s | 0.001046s | byte equality observed |
| alignment_of_a2_test.test | 10 | 0.000992s | 0.000977s | 0.000967s | byte equality observed |
| alignment_of_test.test | 10 | 0.000961s | 0.000947s | 0.000963s | byte equality observed |
| common_type_2_test.test | 10 | 0.000970s | 0.000956s | 0.000982s | byte equality observed |
| common_type_3_test.test | 10 | 0.000945s | 0.000971s | 0.000976s | byte equality observed |
| common_type_4_test.test | 10 | 0.000983s | 0.000969s | 0.000989s | byte equality observed |
| common_type_5_test.test | 10 | 0.000956s | 0.001003s | 0.000969s | byte equality observed |
| common_type_6_test.test | 10 | 0.000966s | 0.000948s | 0.000976s | byte equality observed |
| common_type_sfinae2_test.test | 10 | 0.000970s | 0.000955s | 0.000946s | byte equality observed |
| common_type_sfinae_test.test | 10 | 0.000952s | 0.001009s | 0.000994s | byte equality observed |
| common_type_test.test | 10 | 0.000970s | 0.000974s | 0.000961s | byte equality observed |
| conditional_test.test | 10 | 0.000975s | 0.000952s | 0.000970s | byte equality observed |
| conjunction_test.test | 10 | 0.000929s | 0.000954s | 0.000978s | byte equality observed |
| copy_cv_ref_test.test | 10 | 0.000969s | 0.000950s | 0.000963s | byte equality observed |
| copy_cv_test.test | 10 | 0.000984s | 0.000966s | 0.000973s | byte equality observed |
| copy_reference_test.test | 10 | 0.000956s | 0.000971s | 0.000948s | byte equality observed |
| cxx14_aliases_test.test | 10 | 0.000969s | 0.001000s | 0.000979s | byte equality observed |
| decay_test.test | 10 | 0.000976s | 0.000969s | 0.000958s | byte equality observed |
| detected_or_test.test | 10 | 0.000970s | 0.000974s | 0.000970s | byte equality observed |
| detected_test.test | 10 | 0.000976s | 0.000950s | 0.000937s | byte equality observed |
| disjunction_test.test | 10 | 0.000963s | 0.000966s | 0.000971s | byte equality observed |
| enable_if_test.test | 10 | 0.000970s | 0.000988s | 0.000973s | byte equality observed |
| extent_test.test | 10 | 0.000971s | 0.000991s | 0.000979s | byte equality observed |
| floating_point_promotion_test.test | 10 | 0.000958s | 0.000965s | 0.000930s | byte equality observed |
| function_traits_test.test | 10 | 0.000990s | 0.000952s | 0.000976s | byte equality observed |
| has_binary_classes0_test.test | 10 | 0.000961s | 0.000994s | 0.000940s | byte equality observed |
| has_binary_classes1_test.test | 10 | 0.000950s | 0.001006s | 0.000956s | byte equality observed |
| has_binary_classes2_test.test | 10 | 0.000987s | 0.000936s | 0.000996s | byte equality observed |
| has_binary_classes3_test.test | 10 | 0.000972s | 0.000994s | 0.000948s | byte equality observed |
| has_binary_classes4_test.test | 10 | 0.000941s | 0.000952s | 0.000976s | byte equality observed |
| has_binary_classes5_test.test | 10 | 0.000963s | 0.000976s | 0.000982s | byte equality observed |
| has_binary_classes6_test.test | 10 | 0.000989s | 0.000963s | 0.000972s | byte equality observed |
| has_binary_classes7_test.test | 10 | 0.000949s | 0.000957s | 0.000960s | byte equality observed |
| has_binary_classes8_test.test | 10 | 0.000953s | 0.000944s | 0.000953s | byte equality observed |
| has_binary_classes9_test.test | 9 | 0.000972s | 0.000954s | 0.000965s | byte equality observed |
| has_bit_and_assign_test.test | 10 | 0.000959s | 0.000971s | 0.000963s | byte equality observed |
| has_bit_and_test.test | 10 | 0.001006s | 0.000975s | 0.000963s | byte equality observed |
| has_bit_or_assign_test.test | 10 | 0.000968s | 0.000997s | 0.000975s | byte equality observed |
| has_bit_or_test.test | 10 | 0.000968s | 0.000944s | 0.000962s | byte equality observed |
| has_bit_xor_assign_test.test | 10 | 0.000955s | 0.000959s | 0.000964s | byte equality observed |
| has_bit_xor_test.test | 10 | 0.000982s | 0.000962s | 0.001000s | byte equality observed |
| has_complement_test.test | 10 | 0.000957s | 0.000941s | 0.000970s | byte equality observed |
| has_dereference_test.test | 10 | 0.000967s | 0.000973s | 0.000968s | byte equality observed |
| has_divides_assign_test.test | 10 | 0.000973s | 0.000978s | 0.001006s | byte equality observed |
| has_divides_test.test | 10 | 0.000957s | 0.001000s | 0.000959s | byte equality observed |
| has_equal_to_test.test | 10 | 0.000979s | 0.000977s | 0.001008s | byte equality observed |
| has_greater_equal_test.test | 10 | 0.000966s | 0.000951s | 0.000952s | byte equality observed |
| has_greater_test.test | 10 | 0.000956s | 0.000961s | 0.000980s | byte equality observed |
| has_left_shift_assign_test.test | 10 | 0.000938s | 0.000978s | 0.000967s | byte equality observed |
| has_left_shift_test.test | 10 | 0.000966s | 0.000998s | 0.000981s | byte equality observed |
| has_less_equal_test.test | 10 | 0.000993s | 0.000943s | 0.000973s | byte equality observed |
| has_less_test.test | 10 | 0.000965s | 0.000961s | 0.000945s | byte equality observed |
| has_logical_and_test.test | 10 | 0.000948s | 0.000997s | 0.001002s | byte equality observed |
| has_logical_not_test.test | 10 | 0.000982s | 0.000975s | 0.000980s | byte equality observed |
| has_logical_or_test.test | 10 | 0.000987s | 0.000945s | 0.000995s | byte equality observed |
| has_minus_assign_test.test | 10 | 0.000963s | 0.000971s | 0.000962s | byte equality observed |
| has_minus_test.test | 10 | 0.000954s | 0.000948s | 0.000999s | byte equality observed |
| has_modulus_assign_test.test | 10 | 0.000966s | 0.000974s | 0.000940s | byte equality observed |
| has_modulus_test.test | 10 | 0.000976s | 0.000979s | 0.000994s | byte equality observed |
| has_multiplies_assign_test.test | 10 | 0.000958s | 0.000936s | 0.000978s | byte equality observed |
| has_multiplies_test.test | 10 | 0.000956s | 0.000985s | 0.000952s | byte equality observed |
| has_negate_test.test | 10 | 0.000986s | 0.000945s | 0.000972s | byte equality observed |
| has_not_equal_to_test.test | 10 | 0.000932s | 0.000939s | 0.000938s | byte equality observed |
| has_nothrow_assign_test.test | 10 | 0.000981s | 0.000995s | 0.000986s | byte equality observed |
| has_nothrow_assign_test_no_intrinsics.test | 10 | 0.000961s | 0.000957s | 0.000974s | byte equality observed |
| has_nothrow_constr_test.test | 10 | 0.000956s | 0.000995s | 0.000953s | byte equality observed |
| has_nothrow_constr_test_no_intrinsics.test | 10 | 0.000989s | 0.000971s | 0.000968s | byte equality observed |
| has_nothrow_copy_test.test | 10 | 0.000951s | 0.000960s | 0.000975s | byte equality observed |
| has_nothrow_copy_test_no_intrinsics.test | 10 | 0.000967s | 0.000948s | 0.000962s | byte equality observed |
| has_nothrow_destructor_test.test | 10 | 0.000943s | 0.000946s | 0.000949s | byte equality observed |
| has_operator_new_test.test | 10 | 0.000972s | 0.000976s | 0.000997s | byte equality observed |
| has_plus_assign_test.test | 10 | 0.000949s | 0.000977s | 0.000938s | byte equality observed |
| has_plus_test.test | 10 | 0.000975s | 0.000985s | 0.000977s | byte equality observed |
| has_post_decrement_test.test | 10 | 0.000979s | 0.000937s | 0.000976s | byte equality observed |
| has_post_increment_test.test | 10 | 0.000948s | 0.000959s | 0.001000s | byte equality observed |
| has_postfix_classes0_test.test | 10 | 0.000966s | 0.000954s | 0.000957s | byte equality observed |
| has_postfix_classes1_test.test | 10 | 0.000942s | 0.000979s | 0.000955s | byte equality observed |
| has_postfix_classes2_test.test | 10 | 0.000965s | 0.000983s | 0.000973s | byte equality observed |
| has_postfix_classes3_test.test | 10 | 0.000966s | 0.000971s | 0.000970s | byte equality observed |
| has_pre_decrement_test.test | 10 | 0.000950s | 0.000954s | 0.000964s | byte equality observed |
| has_pre_increment_test.test | 10 | 0.000979s | 0.000947s | 0.000989s | byte equality observed |
| has_prefix_classes0_test.test | 10 | 0.000980s | 0.000994s | 0.000944s | byte equality observed |
| has_prefix_classes1_test.test | 10 | 0.000951s | 0.000950s | 0.000936s | byte equality observed |
| has_prefix_classes2_test.test | 10 | 0.000986s | 0.000949s | 0.000957s | byte equality observed |
| has_prefix_classes3_test.test | 10 | 0.000962s | 0.001007s | 0.000947s | byte equality observed |
| has_right_shift_assign_test.test | 10 | 0.000973s | 0.000953s | 0.000971s | byte equality observed |
| has_right_shift_test.test | 10 | 0.000975s | 0.000958s | 0.001019s | byte equality observed |
| has_trivial_assign_test.test | 10 | 0.000990s | 0.000993s | 0.000986s | byte equality observed |
| has_trivial_constr_test.test | 10 | 0.000994s | 0.001048s | 0.001025s | native test oracle; output varies |
| has_trivial_copy_test.test | 10 | 0.001014s | 0.001065s | 0.001034s | native test oracle; output varies |
| has_trivial_destructor_test.test | 10 | 0.000982s | 0.000966s | 0.001043s | byte equality observed |
| has_trivial_move_assign_test.test | 10 | 0.000960s | 0.000949s | 0.000940s | byte equality observed |
| has_trivial_move_constructor_test.test | 10 | 0.000969s | 0.000945s | 0.000968s | byte equality observed |
| has_unary_minus_test.test | 10 | 0.000957s | 0.000967s | 0.000955s | byte equality observed |
| has_unary_plus_test.test | 10 | 0.000973s | 0.000957s | 0.000931s | byte equality observed |
| has_virtual_destructor_test.test | 10 | 0.000982s | 0.000989s | 0.001013s | byte equality observed |
| integral_constant_test.test | 10 | 0.000441s | 0.000452s | 0.000963s | byte equality observed |
| is_abstract_test.test | 10 | 0.000961s | 0.000951s | 0.000950s | byte equality observed |
| is_arithmetic_test.test | 10 | 0.000999s | 0.000983s | 0.001019s | byte equality observed |
| is_array_test.test | 10 | 0.000954s | 0.000972s | 0.000943s | byte equality observed |
| is_assignable_test.test | 10 | 0.000974s | 0.001000s | 0.000974s | byte equality observed |
| is_base_and_derived_test.test | 10 | 0.000955s | 0.000980s | 0.000954s | byte equality observed |
| is_base_of_test.test | 10 | 0.000974s | 0.000937s | 0.000990s | byte equality observed |
| is_bounded_array_test.test | 10 | 0.000948s | 0.000994s | 0.000987s | byte equality observed |
| is_class_test.test | 10 | 0.000943s | 0.000976s | 0.000987s | byte equality observed |
| is_complete_test.test | 10 | 0.000952s | 0.000949s | 0.000975s | byte equality observed |
| is_complex_test.test | 10 | 0.000990s | 0.000984s | 0.001015s | byte equality observed |
| is_compound_test.test | 9 | 0.001013s | 0.000943s | 0.000942s | byte equality observed |
| is_const_test.test | 10 | 0.000938s | 0.000968s | 0.000964s | byte equality observed |
| is_constructible_test.test | 10 | 0.000956s | 0.000940s | 0.001013s | byte equality observed |
| is_convertible_test.test | 10 | 0.000994s | 0.000942s | 0.001020s | byte equality observed |
| is_copy_assignable_test.test | 10 | 0.000987s | 0.001002s | 0.001025s | byte equality observed |
| is_copy_constructible_test.test | 10 | 0.000973s | 0.000956s | 0.001045s | byte equality observed |
| is_default_constr_test.test | 10 | 0.001020s | 0.000954s | 0.001056s | byte equality observed |
| is_destructible_test.test | 10 | 0.001000s | 0.000972s | 0.001085s | byte equality observed |
| is_detected_exact_test.test | 10 | 0.000966s | 0.000940s | 0.001061s | byte equality observed |
| is_detected_test.test | 10 | 0.000990s | 0.000987s | 0.001062s | byte equality observed |
| is_empty_test.test | 10 | 0.000988s | 0.000983s | 0.001060s | byte equality observed |
| is_enum_test.test | 10 | 0.000995s | 0.000937s | 0.001066s | byte equality observed |
| is_final_test.test | 10 | 0.001001s | 0.000973s | 0.001081s | byte equality observed |
| is_float_test.test | 10 | 0.001008s | 0.000980s | 0.001112s | byte equality observed |
| is_floating_point_test.test | 10 | 0.000988s | 0.000950s | 0.001147s | byte equality observed |
| is_function_test.test | 10 | 0.001017s | 0.001002s | 0.001140s | byte equality observed |
| is_fundamental_test.test | 10 | 0.001069s | 0.000971s | 0.001158s | byte equality observed |
| is_integral_test.test | 10 | 0.001041s | 0.000987s | 0.001175s | byte equality observed |
| is_list_constructible_test.test | 10 | 0.001041s | 0.000941s | 0.001171s | byte equality observed |
| is_lvalue_reference_test.test | 10 | 0.001062s | 0.000993s | 0.001143s | byte equality observed |
| is_member_func_test.test | 10 | 0.001092s | 0.000940s | 0.001145s | byte equality observed |
| is_member_obj_test.test | 10 | 0.001081s | 0.000987s | 0.001159s | byte equality observed |
| is_member_pointer_test.test | 10 | 0.001066s | 0.000965s | 0.001148s | byte equality observed |
| is_noncopyable_test.test | 10 | 0.001068s | 0.000950s | 0.001147s | byte equality observed |
| is_nothrow_move_assignable_test.test | 10 | 0.001087s | 0.000968s | 0.001183s | byte equality observed |
| is_nothrow_move_assignable_test_no_intrinsics.test | 10 | 0.001154s | 0.000945s | 0.001169s | byte equality observed |
| is_nothrow_move_constructible_test.test | 10 | 0.001115s | 0.000934s | 0.001155s | byte equality observed |
| is_nothrow_move_constructible_test_no_intrinsics.test | 10 | 0.001155s | 0.001043s | 0.001160s | byte equality observed |
| is_nothrow_swappable_test.test | 10 | 0.001120s | 0.000948s | 0.001155s | byte equality observed |
| is_object_test.test | 10 | 0.001129s | 0.000964s | 0.001146s | byte equality observed |
| is_pod_test.test | 10 | 0.001156s | 0.000965s | 0.001152s | byte equality observed |
| is_pointer_test.test | 10 | 0.001164s | 0.000954s | 0.001165s | byte equality observed |
| is_polymorphic_test.test | 10 | 0.001152s | 0.000965s | 0.001175s | byte equality observed |
| is_reference_test.test | 10 | 0.001149s | 0.000972s | 0.001142s | byte equality observed |
| is_rvalue_reference_test.test | 10 | 0.001162s | 0.000960s | 0.001153s | byte equality observed |
| is_same_test.test | 10 | 0.001172s | 0.000956s | 0.001147s | byte equality observed |
| is_scalar_test.test | 10 | 0.001179s | 0.000932s | 0.001163s | byte equality observed |
| is_scoped_enum_test.test | 10 | 0.001172s | 0.000982s | 0.001161s | byte equality observed |
| is_signed_test.test | 10 | 0.001176s | 0.000965s | 0.001142s | byte equality observed |
| is_stateless_test.test | 10 | 0.001174s | 0.000944s | 0.001151s | byte equality observed |
| is_swappable_test.test | 10 | 0.001167s | 0.001051s | 0.001172s | byte equality observed |
| is_trivially_copyable_test.test | 10 | 0.001198s | 0.000947s | 0.001229s | byte equality observed |
| is_unbounded_array_test.test | 10 | 0.001149s | 0.001078s | 0.001185s | byte equality observed |
| is_union_test.test | 10 | 0.001169s | 0.001018s | 0.001173s | byte equality observed |
| is_unscoped_enum_test.test | 10 | 0.001156s | 0.001046s | 0.001182s | byte equality observed |
| is_unsigned_test.test | 10 | 0.001165s | 0.001045s | 0.001167s | byte equality observed |
| is_virtual_base_of_test.test | 10 | 0.001149s | 0.001044s | 0.001162s | byte equality observed |
| is_void_test.test | 10 | 0.001155s | 0.001098s | 0.001179s | byte equality observed |
| is_volatile_test.test | 10 | 0.001150s | 0.001077s | 0.001166s | byte equality observed |
| make_signed_test.test | 10 | 0.001159s | 0.001100s | 0.001165s | byte equality observed |
| make_unsigned_test.test | 10 | 0.001169s | 0.001124s | 0.001148s | byte equality observed |
| make_void_test.test | 10 | 0.001166s | 0.001109s | 0.001157s | byte equality observed |
| mpl_interop_test1.test | 10 | 0.000516s | 0.000505s | 0.001154s | byte equality observed |
| mpl_interop_test2.test | 10 | 0.000503s | 0.000497s | 0.001162s | byte equality observed |
| mpl_interop_test3.test | 10 | 0.000502s | 0.000508s | 0.001161s | byte equality observed |
| negation_test.test | 10 | 0.001166s | 0.001189s | 0.001149s | byte equality observed |
| promote_basic_test.test | 10 | 0.000505s | 0.000505s | 0.001156s | byte equality observed |
| promote_enum_msvc_bug_test.test | 10 | 0.000504s | 0.000493s | 0.001158s | byte equality observed |
| promote_enum_test.test | 10 | 0.000504s | 0.000504s | 0.001147s | byte equality observed |
| promote_mpl_test.test | 10 | 0.000501s | 0.000513s | 0.001184s | byte equality observed |
| rank_test.test | 10 | 0.001169s | 0.001180s | 0.001148s | byte equality observed |
| remove_all_extents_test.test | 10 | 0.001185s | 0.001185s | 0.001158s | byte equality observed |
| remove_bounds_test.test | 10 | 0.001174s | 0.001151s | 0.001143s | byte equality observed |
| remove_const_test.test | 10 | 0.001165s | 0.001165s | 0.001145s | byte equality observed |
| remove_cv_ref_test.test | 10 | 0.001172s | 0.001156s | 0.001153s | byte equality observed |
| remove_cv_test.test | 10 | 0.001162s | 0.001175s | 0.001188s | byte equality observed |
| remove_extent_test.test | 10 | 0.001167s | 0.001154s | 0.001169s | byte equality observed |
| remove_pointer_test.test | 10 | 0.001168s | 0.001147s | 0.001179s | byte equality observed |
| remove_reference_test.test | 10 | 0.001155s | 0.001153s | 0.001164s | byte equality observed |
| remove_volatile_test.test | 10 | 0.001183s | 0.001156s | 0.001160s | byte equality observed |
| tricky_abstract_type_test.test | 10 | 0.001164s | 0.001165s | 0.001159s | byte equality observed |
| tricky_add_pointer_test.test | 10 | 0.001145s | 0.001139s | 0.001176s | byte equality observed |
| tricky_function_type_test.test | 10 | 0.001150s | 0.001165s | 0.001162s | byte equality observed |
| tricky_incomplete_type_test.test | 10 | 0.001171s | 0.001174s | 0.001178s | byte equality observed |
| tricky_is_enum_test.test | 10 | 0.001171s | 0.001167s | 0.001169s | byte equality observed |
| tricky_partial_spec_test.test | 10 | 0.001176s | 0.001168s | 0.001179s | byte equality observed |
| tricky_rvalue_test.test | 10 | 0.001166s | 0.001160s | 0.001140s | byte equality observed |
| type_identity_test.test | 10 | 0.001183s | 0.001171s | 0.001187s | byte equality observed |
| type_traits_test.test | 10 | 0.000511s | 0.000503s | 0.001168s | byte equality observed |
| type_with_alignment_test.test | 10 | 0.001286s | 0.001240s | 0.001279s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
