# libs/type_traits/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| type_with_alignment_test.test | RUN | 0.094906 | 0.217187 | 0.331990 | 0.001405 | 0.001507 | 0.001438 |
| type_traits_test.test | RUN | 0.129843 | 0.241302 | 0.320668 | 0.000533 | 0.000512 | 0.001386 |
| type_identity_test.test | RUN | 0.217643 | 0.767452 | 1.691626 | 0.001451 | 0.001488 | 0.001461 |
| tricky_rvalue_test.test | RUN | 0.087160 | 0.176309 | 0.258199 | 0.001346 | 0.001473 | 0.001465 |
| tricky_partial_spec_test.test | RUN | 0.106469 | 0.255834 | 0.403572 | 0.001325 | 0.001333 | 0.001433 |
| tricky_is_enum_test.test | RUN | 0.084662 | 0.175931 | 0.238875 | 0.001377 | 0.001354 | 0.001210 |
| tricky_incomplete_type_test.test | RUN | 0.139855 | 0.271330 | 0.391029 | 0.001371 | 0.001340 | 0.001452 |
| tricky_function_type_test.test | RUN | 0.113763 | 0.275754 | 0.446724 | 0.001542 | 0.001592 | 0.001256 |
| tricky_add_pointer_test.test | RUN | 0.152296 | 0.457576 | 1.003991 | 0.001391 | 0.001551 | 0.001538 |
| tricky_abstract_type_test.test | RUN | 0.090074 | 0.175545 | 0.249940 | 0.001554 | 0.001185 | 0.001504 |
| remove_volatile_test.test | RUN | 0.200235 | 0.676765 | 1.475889 | 0.001386 | 0.001589 | 0.001485 |
| remove_reference_test.test | RUN | 0.206029 | 0.715484 | 1.608930 | 0.001437 | 0.001372 | 0.001496 |
| remove_pointer_test.test | RUN | 0.216395 | 0.740446 | 1.667159 | 0.001492 | 0.001337 | 0.001513 |
| remove_extent_test.test | RUN | 0.208381 | 0.721542 | 1.528540 | 0.001324 | 0.001556 | 0.001490 |
| remove_cv_test.test | RUN | 0.206725 | 0.726049 | 1.545426 | 0.001479 | 0.001566 | 0.001473 |
| remove_cv_ref_test.test | RUN | 0.418695 | 1.712311 | 3.825527 | 0.001315 | 0.001348 | 0.001456 |
| remove_const_test.test | RUN | 0.201174 | 0.662210 | 1.487993 | 0.001475 | 0.001539 | 0.001474 |
| remove_bounds_test.test | RUN | 0.208124 | 0.724190 | 1.547395 | 0.001192 | 0.001446 | 0.001471 |
| remove_all_extents_test.test | RUN | 0.208767 | 0.707195 | 1.560834 | 0.001320 | 0.001361 | 0.001415 |
| rank_test.test | RUN | 0.089236 | 0.184570 | 0.273099 | 0.001337 | 0.001506 | 0.001497 |
| promote_mpl_test.test | RUN | 0.078179 | 0.167496 | 0.189625 | 0.000462 | 0.000600 | 0.001353 |
| promote_enum_test.test | RUN | 0.017897 | 0.040523 | 0.049206 | 0.000561 | 0.000578 | 0.001444 |
| promote_enum_msvc_bug_test.test | RUN | 0.017218 | 0.039569 | 0.044710 | 0.000514 | 0.000611 | 0.002662 |
| promote_basic_test.test | RUN | 0.020430 | 0.048995 | 0.057203 | 0.000494 | 0.000590 | 0.001558 |
| negation_test.test | RUN | 0.084490 | 0.172256 | 0.242242 | 0.001309 | 0.001412 | 0.001313 |
| mpl_interop_test3.test | RUN | 0.074023 | 0.160424 | 0.170761 | 0.000548 | 0.000521 | 0.001292 |
| mpl_interop_test2.test | RUN | 0.031257 | 0.047356 | 0.054582 | 0.000549 | 0.000550 | 0.001608 |
| mpl_interop_test1.test | RUN | 0.015445 | 0.034926 | 0.039419 | 0.000584 | 0.000699 | 0.001375 |
| make_void_test.test | RUN | 0.089364 | 0.179949 | 0.248411 | 0.001388 | 0.001467 | 0.001429 |
| make_unsigned_test.test | RUN | 0.102745 | 0.234461 | 0.347484 | 0.001263 | 0.001368 | 0.001589 |
| make_signed_test.test | RUN | 0.101193 | 0.237091 | 0.356489 | 0.001213 | 0.001366 | 0.001368 |
| is_volatile_test.test | RUN | 0.095305 | 0.199552 | 0.288513 | 0.001213 | 0.001492 | 0.001502 |
| is_void_test.test | RUN | 0.090206 | 0.192438 | 0.268456 | 0.001392 | 0.001562 | 0.001589 |
| is_virtual_base_of_test.test | RUN | 0.102047 | 0.238301 | 0.358369 | 0.001226 | 0.001211 | 0.001281 |
| is_unsigned_test.test | RUN | 0.091244 | 0.195320 | 0.286663 | 0.001314 | 0.001485 | 0.001462 |
| is_unscoped_enum_test.test | RUN | 0.091469 | 0.173892 | 0.247383 | 0.001387 | 0.001211 | 0.001558 |
| is_union_test.test | RUN | 0.096730 | 0.220164 | 0.322354 | 0.001290 | 0.001554 | 0.001497 |
| is_unbounded_array_test.test | RUN | 0.090442 | 0.189245 | 0.280321 | 0.001386 | 0.001504 | 0.001519 |
| is_trivially_copyable_test.test | RUN | 0.139587 | 0.375205 | 0.672116 | 0.001331 | 0.001600 | 0.001247 |
| is_swappable_test.test | RUN | 0.118617 | 0.291095 | 0.511513 | 0.001315 | 0.001442 | 0.001672 |
| is_stateless_test.test | RUN | 0.133998 | 0.330948 | 0.575149 | 0.001336 | 0.001466 | 0.001494 |
| is_signed_test.test | RUN | 0.092996 | 0.195517 | 0.280948 | 0.001397 | 0.001230 | 0.001366 |
| is_scoped_enum_test.test | RUN | 0.091888 | 0.182327 | 0.251063 | 0.001308 | 0.001491 | 0.001515 |
| is_scalar_test.test | RUN | 0.120238 | 0.310214 | 0.488283 | 0.001285 | 0.001505 | 0.001247 |
| is_same_test.test | RUN | 0.089187 | 0.187706 | 0.278062 | 0.001484 | 0.001452 | 0.001414 |
| is_rvalue_reference_test.test | RUN | 0.093370 | 0.209413 | 0.310923 | 0.001372 | 0.001353 | 0.001475 |
| is_reference_test.test | RUN | 0.094212 | 0.216219 | 0.303391 | 0.001247 | 0.001495 | 0.001468 |
| is_polymorphic_test.test | RUN | 0.098215 | 0.223848 | 0.334122 | 0.001232 | 0.001526 | 0.001533 |
| is_pointer_test.test | RUN | 0.095385 | 0.217419 | 0.323463 | 0.001584 | 0.001392 | 0.001534 |
| is_pod_test.test | RUN | 0.123611 | 0.342297 | 0.582515 | 0.001383 | 0.001423 | 0.001539 |
| is_object_test.test | RUN | 0.089190 | 0.188548 | 0.236977 | 0.001354 | 0.001316 | 0.001480 |
| is_nothrow_swappable_test.test | RUN | 0.130342 | 0.318872 | 0.543021 | 0.001453 | 0.001439 | 0.001471 |
| is_nothrow_move_constructible_test.test | RUN | 0.122594 | 0.331061 | 0.558796 | 0.001427 | 0.001189 | 0.001260 |
| is_nothrow_move_assignable_test.test | RUN | 0.123713 | 0.337884 | 0.580994 | 0.001366 | 0.001572 | 0.001466 |
| is_noncopyable_test.test | RUN | 0.090758 | 0.186397 | 0.254881 | 0.001316 | 0.001511 | 0.001293 |
| is_member_pointer_test.test | RUN | 0.091766 | 0.194745 | 0.273244 | 0.001436 | 0.001335 | 0.001455 |
| is_member_obj_test.test | RUN | 0.090212 | 0.194862 | 0.257443 | 0.001550 | 0.001445 | 0.001466 |
| is_member_func_test.test | RUN | 0.093281 | 0.197756 | 0.294625 | 0.001306 | 0.001244 | 0.001540 |
| is_lvalue_reference_test.test | RUN | 0.095320 | 0.213803 | 0.314455 | 0.001344 | 0.001580 | 0.001510 |
| is_list_constructible_test.test | RUN | 0.110491 | 0.247664 | 0.330196 | 0.001522 | 0.001590 | 0.001439 |
| is_integral_test.test | RUN | 0.110294 | 0.289065 | 0.496493 | 0.001495 | 0.001377 | 0.001539 |
| is_fundamental_test.test | RUN | 0.113085 | 0.302140 | 0.495658 | 0.001337 | 0.001338 | 0.001283 |
| is_function_test.test | RUN | 0.091289 | 0.199923 | 0.275910 | 0.001135 | 0.001399 | 0.001416 |
| is_floating_point_test.test | RUN | 0.097654 | 0.212301 | 0.312663 | 0.001479 | 0.001587 | 0.001547 |
| is_float_test.test | RUN | 0.093978 | 0.214865 | 0.304810 | 0.001499 | 0.001337 | 0.001268 |
| is_final_test.test | RUN | 0.096360 | 0.216003 | 0.338657 | 0.001273 | 0.001269 | 0.001132 |
| is_enum_test.test | RUN | 0.087944 | 0.181194 | 0.264176 | 0.001395 | 0.001196 | 0.001297 |
| is_empty_test.test | RUN | 0.093264 | 0.198577 | 0.297601 | 0.001439 | 0.001400 | 0.001470 |
| is_detected_test.test | RUN | 0.086142 | 0.174760 | 0.244322 | 0.001311 | 0.001448 | 0.001458 |
| is_detected_exact_test.test | RUN | 0.085563 | 0.173741 | 0.240913 | 0.001281 | 0.001354 | 0.001382 |
| is_destructible_test.test | RUN | 0.121350 | 0.321908 | 0.531219 | 0.001202 | 0.001572 | 0.001480 |
| is_default_constr_test.test | RUN | 0.126362 | 0.337736 | 0.583844 | 0.001471 | 0.001421 | 0.001477 |
| is_copy_constructible_test.test | RUN | 0.129500 | 0.353894 | 0.593180 | 0.001559 | 0.001368 | 0.001454 |
| is_copy_assignable_test.test | RUN | 0.125906 | 0.349765 | 0.590971 | 0.001445 | 0.001419 | 0.001392 |
| is_convertible_test.test | RUN | 0.122675 | 0.313861 | 0.488017 | 0.001306 | 0.001525 | 0.001537 |
| is_constructible_test.test | RUN | 0.105577 | 0.230593 | 0.354058 | 0.001353 | 0.001402 | 0.001381 |
| is_const_test.test | RUN | 0.089439 | 0.202472 | 0.299155 | 0.001453 | 0.001293 | 0.001392 |
| is_compound_test.test | RUN | 0.085390 | 0.176685 | 0.231688 | 0.001298 | 0.001453 | 0.001625 |
| is_complex_test.test | RUN | 0.119185 | 0.234169 | 0.330292 | 0.001346 | 0.001595 | 0.001460 |
| is_complete_test.test | RUN | 0.102972 | 0.231220 | 0.369260 | 0.001518 | 0.001582 | 0.001665 |
| is_class_test.test | RUN | 0.094316 | 0.210905 | 0.330297 | 0.001178 | 0.001208 | 0.001417 |
| is_bounded_array_test.test | RUN | 0.091424 | 0.191819 | 0.267754 | 0.001492 | 0.001428 | 0.001266 |
| is_base_of_test.test | RUN | 0.094588 | 0.211681 | 0.318562 | 0.001255 | 0.001642 | 0.001472 |
| is_base_and_derived_test.test | RUN | 0.092646 | 0.205442 | 0.307510 | 0.001511 | 0.001528 | 0.001534 |
| is_assignable_test.test | RUN | 0.113095 | 0.285374 | 0.455969 | 0.001333 | 0.001434 | 0.001399 |
| is_array_test.test | RUN | 0.092834 | 0.201457 | 0.300230 | 0.001366 | 0.001435 | 0.001344 |
| is_arithmetic_test.test | RUN | 0.113802 | 0.292180 | 0.505434 | 0.002602 | 0.001480 | 0.001504 |
| is_abstract_test.test | RUN | 0.238728 | 0.811493 | 1.540015 | 0.001324 | 0.001633 | 0.001410 |
| integral_constant_test.test | RUN | 0.025348 | 0.036408 | 0.037737 | 0.000590 | 0.000579 | 0.001472 |
| has_virtual_destructor_test.test | RUN | 0.115879 | 0.225140 | 0.361533 | 0.001408 | 0.001613 | 0.001471 |
| has_unary_plus_test.test | RUN | 0.174721 | 0.847633 | 1.237456 | 0.001438 | 0.001482 | 0.001616 |
| has_unary_minus_test.test | RUN | 0.171795 | 0.863102 | 1.206277 | 0.001489 | 0.001388 | 0.001498 |
| has_trivial_move_constructor_test.test | RUN | 0.124829 | 0.326068 | 0.585008 | 0.001331 | 0.001368 | 0.001446 |
| has_trivial_move_assign_test.test | RUN | 0.125555 | 0.327578 | 0.593661 | 0.001340 | 0.001566 | 0.001489 |
| has_trivial_destructor_test.test | RUN | 0.125654 | 0.344554 | 0.581785 | 0.001319 | 0.001497 | 0.001609 |
| has_trivial_copy_test.test | RUN | 0.130282 | 0.354213 | 0.620464 | 0.001329 | 0.001585 | 0.001650 |
| has_trivial_constr_test.test | RUN | 0.129308 | 0.360694 | 0.613825 | 0.001370 | 0.001424 | 0.001566 |
| has_trivial_assign_test.test | RUN | 0.128656 | 0.360136 | 0.600293 | 0.001268 | 0.001630 | 0.001572 |
| has_right_shift_test.test | RUN | 0.183674 | 0.671303 | 1.319197 | 0.001540 | 0.001452 | 0.001349 |
| has_right_shift_assign_test.test | RUN | 0.175478 | 0.625719 | 1.206408 | 0.001306 | 0.001340 | 0.001511 |
| has_prefix_classes3_test.test | RUN | 0.132648 | 0.371682 | 0.628393 | 0.001304 | 0.001376 | 0.001570 |
| has_prefix_classes2_test.test | RUN | 0.130398 | 0.375113 | 0.599548 | 0.001265 | 0.001548 | 0.001552 |
| has_prefix_classes1_test.test | RUN | 0.132161 | 0.370621 | 0.630195 | 0.001399 | 0.001538 | 0.001493 |
| has_prefix_classes0_test.test | RUN | 0.131008 | 0.370257 | 0.641351 | 0.001342 | 0.001421 | 0.001518 |
| has_pre_increment_test.test | RUN | 0.172376 | 0.871924 | 1.180787 | 0.001326 | 0.001343 | 0.001301 |
| has_pre_decrement_test.test | RUN | 0.171311 | 0.864969 | 1.187163 | 0.001335 | 0.001418 | 0.001371 |
| has_postfix_classes3_test.test | RUN | 0.133265 | 0.366166 | 0.628443 | 0.001468 | 0.001389 | 0.001353 |
| has_postfix_classes2_test.test | RUN | 0.131468 | 0.361629 | 0.592246 | 0.001383 | 0.001138 | 0.001558 |
| has_postfix_classes1_test.test | RUN | 0.133212 | 0.369243 | 0.642997 | 0.001132 | 0.001439 | 0.001443 |
| has_postfix_classes0_test.test | RUN | 0.135974 | 0.370354 | 0.632411 | 0.001290 | 0.001382 | 0.001350 |
| has_post_increment_test.test | RUN | 0.173208 | 0.890401 | 1.193493 | 0.001432 | 0.001437 | 0.001221 |
| has_post_decrement_test.test | RUN | 0.171963 | 0.887922 | 1.200311 | 0.001342 | 0.001438 | 0.001513 |
| has_plus_test.test | RUN | 0.175109 | 0.628644 | 1.177222 | 0.001198 | 0.001474 | 0.001307 |
| has_plus_assign_test.test | RUN | 0.179988 | 0.633196 | 1.192291 | 0.001346 | 0.001229 | 0.001386 |
| has_operator_new_test.test | RUN | 0.125406 | 0.325488 | 0.568413 | 0.001417 | 0.001429 | 0.001185 |
| has_nothrow_destructor_test.test | RUN | 0.129338 | 0.351844 | 0.588149 | 0.001466 | 0.001340 | 0.001531 |
| has_nothrow_copy_test.test | RUN | 0.129798 | 0.339444 | 0.592211 | 0.001330 | 0.001520 | 0.001059 |
| has_nothrow_constr_test.test | RUN | 0.128306 | 0.332486 | 0.576986 | 0.001688 | 0.001192 | 0.001511 |
| has_nothrow_assign_test.test | RUN | 0.129775 | 0.335643 | 0.571198 | 0.001441 | 0.001498 | 0.001165 |
| has_not_equal_to_test.test | RUN | 0.184462 | 0.860339 | 1.213531 | 0.001404 | 0.001630 | 0.001447 |
| has_negate_test.test | RUN | 0.170159 | 0.599689 | 1.161958 | 0.001354 | 0.001457 | 0.001391 |
| has_multiplies_test.test | RUN | 0.175541 | 0.615964 | 1.218780 | 0.001352 | 0.001208 | 0.001403 |
| has_multiplies_assign_test.test | RUN | 0.175673 | 0.623297 | 1.160450 | 0.001442 | 0.001527 | 0.001460 |
| has_modulus_test.test | RUN | 0.174011 | 0.649675 | 1.192142 | 0.001393 | 0.001470 | 0.001508 |
| has_modulus_assign_test.test | RUN | 0.174412 | 0.625504 | 1.154190 | 0.001397 | 0.001448 | 0.001607 |
| has_minus_test.test | RUN | 0.178979 | 0.648790 | 1.211016 | 0.001308 | 0.001623 | 0.001382 |
| has_minus_assign_test.test | RUN | 0.176567 | 0.632638 | 1.223237 | 0.001443 | 0.001577 | 0.001257 |
| has_logical_or_test.test | RUN | 0.175647 | 0.644567 | 1.184791 | 0.001398 | 0.001526 | 0.001487 |
| has_logical_not_test.test | RUN | 0.168260 | 0.623599 | 1.195879 | 0.001366 | 0.001479 | 0.001423 |
| has_logical_and_test.test | RUN | 0.176158 | 0.637040 | 1.241531 | 0.001318 | 0.001648 | 0.001433 |
| has_less_test.test | RUN | 0.176416 | 0.864539 | 1.167721 | 0.001398 | 0.001623 | 0.001163 |
| has_less_equal_test.test | RUN | 0.175201 | 0.862206 | 1.192280 | 0.001355 | 0.001530 | 0.001442 |
| has_left_shift_test.test | RUN | 0.183121 | 0.776414 | 1.534118 | 0.001383 | 0.001217 | 0.001469 |
| has_left_shift_assign_test.test | RUN | 0.176378 | 0.620034 | 1.208811 | 0.001405 | 0.001415 | 0.001361 |
| has_greater_test.test | RUN | 0.176026 | 0.863741 | 1.165449 | 0.001425 | 0.001507 | 0.001628 |
| has_greater_equal_test.test | RUN | 0.176111 | 0.852722 | 1.191629 | 0.002754 | 0.001288 | 0.001489 |
| has_equal_to_test.test | RUN | 0.173683 | 0.840572 | 1.161853 | 0.001464 | 0.001502 | 0.001495 |
| has_divides_test.test | RUN | 0.173727 | 0.615142 | 1.226364 | 0.001344 | 0.001488 | 0.001463 |
| has_divides_assign_test.test | RUN | 0.176043 | 0.614142 | 1.189671 | 0.001352 | 0.001533 | 0.001218 |
| has_dereference_test.test | RUN | 0.170318 | 0.626245 | 1.116679 | 0.001466 | 0.001562 | 0.001471 |
| has_complement_test.test | RUN | 0.171355 | 0.853642 | 1.154143 | 0.001374 | 0.001589 | 0.001532 |
| has_bit_xor_test.test | RUN | 0.174742 | 0.626465 | 1.199808 | 0.001385 | 0.001550 | 0.001538 |
| has_bit_xor_assign_test.test | RUN | 0.174423 | 0.625420 | 1.191691 | 0.001423 | 0.001576 | 0.001461 |
| has_bit_or_test.test | RUN | 0.173470 | 0.622686 | 1.120372 | 0.001452 | 0.001620 | 0.001488 |
| has_bit_or_assign_test.test | RUN | 0.173294 | 0.620166 | 1.204213 | 0.001363 | 0.001621 | 0.001386 |
| has_bit_and_test.test | RUN | 0.171357 | 0.615390 | 1.150358 | 0.001344 | 0.001650 | 0.001434 |
| has_bit_and_assign_test.test | RUN | 0.176788 | 0.620136 | 1.164955 | 0.001394 | 0.001589 | 0.001524 |
| has_binary_classes9_test.test | RUN | 0.186616 | 0.743389 | 1.277717 | 0.001261 | 0.001532 | 0.001503 |
| has_binary_classes8_test.test | RUN | 0.186001 | 0.749980 | 1.351801 | 0.001412 | 0.001628 | 0.001426 |
| has_binary_classes7_test.test | RUN | 0.186587 | 0.737022 | 1.384318 | 0.001311 | 0.001639 | 0.001322 |
| has_binary_classes6_test.test | RUN | 0.185873 | 0.715449 | 1.292945 | 0.001411 | 0.001622 | 0.001331 |
| has_binary_classes5_test.test | RUN | 0.186875 | 0.727136 | 1.360594 | 0.001288 | 0.001636 | 0.001495 |
| has_binary_classes4_test.test | RUN | 0.189492 | 0.747639 | 1.296488 | 0.001365 | 0.001642 | 0.001535 |
| has_binary_classes3_test.test | RUN | 0.189950 | 0.743785 | 1.375792 | 0.001301 | 0.001650 | 0.001554 |
| has_binary_classes2_test.test | RUN | 0.188151 | 0.746006 | 1.348431 | 0.001438 | 0.001624 | 0.001284 |
| has_binary_classes1_test.test | RUN | 0.189054 | 0.745129 | 1.305774 | 0.001426 | 0.001716 | 0.001492 |
| has_binary_classes0_test.test | RUN | 0.186589 | 0.745196 | 1.316780 | 0.001322 | 0.001367 | 0.001525 |
| function_traits_test.test | RUN | 0.094614 | 0.202488 | 0.297632 | 0.001411 | 0.001603 | 0.001210 |
| floating_point_promotion_test.test | RUN | 0.090652 | 0.197438 | 0.285480 | 0.001556 | 0.001326 | 0.001310 |
| extent_test.test | RUN | 0.091515 | 0.203034 | 0.295258 | 0.001367 | 0.001368 | 0.001307 |
| enable_if_test.test | RUN | 0.085206 | 0.169052 | 0.230678 | 0.001270 | 0.001548 | 0.001363 |
| disjunction_test.test | RUN | 0.087680 | 0.179024 | 0.253398 | 0.001186 | 0.001439 | 0.001484 |
| detected_test.test | RUN | 0.086497 | 0.171360 | 0.234445 | 0.001275 | 0.001413 | 0.001390 |
| detected_or_test.test | RUN | 0.086626 | 0.179767 | 0.248424 | 0.001276 | 0.001471 | 0.001368 |
| decay_test.test | RUN | 0.202188 | 0.654450 | 1.389759 | 0.001352 | 0.001494 | 0.001362 |
| cxx14_aliases_test.test | RUN | 0.141187 | 0.258560 | 0.371649 | 0.001524 | 0.001404 | 0.001228 |
| copy_reference_test.test | RUN | 0.088318 | 0.177746 | 0.247215 | 0.001532 | 0.001305 | 0.001614 |
| copy_cv_test.test | RUN | 0.089968 | 0.185114 | 0.270735 | 0.001349 | 0.001409 | 0.001403 |
| copy_cv_ref_test.test | RUN | 0.120786 | 0.306120 | 0.568731 | 0.001227 | 0.001411 | 0.001451 |
| conjunction_test.test | RUN | 0.087049 | 0.181009 | 0.246666 | 0.001449 | 0.001408 | 0.001190 |
| conditional_test.test | RUN | 0.087482 | 0.180810 | 0.238988 | 0.001435 | 0.001428 | 0.001210 |
| common_type_test.test | RUN | 0.112794 | 0.264179 | 0.457510 | 0.001368 | 0.001283 | 0.001372 |
| common_type_sfinae_test.test | RUN | 0.084436 | 0.167477 | 0.218867 | 0.001189 | 0.001499 | 0.001485 |
| common_type_sfinae2_test.test | RUN | 0.086134 | 0.184340 | 0.243583 | 0.001366 | 0.001473 | 0.001760 |
| common_type_6_test.test | RUN | 0.090640 | 0.182992 | 0.252979 | 0.001380 | 0.001345 | 0.001360 |
| common_type_5_test.test | RUN | 0.092309 | 0.173268 | 0.257880 | 0.001123 | 0.001466 | 0.001552 |
| common_type_4_test.test | RUN | 0.091693 | 0.195475 | 0.330910 | 0.001239 | 0.001533 | 0.001501 |
| common_type_3_test.test | RUN | 0.084445 | 0.172737 | 0.235651 | 0.001693 | 0.001386 | 0.001293 |
| common_type_2_test.test | RUN | 0.110994 | 0.257906 | 0.443234 | 0.001535 | 0.001508 | 0.001277 |
| alignment_of_test.test | RUN | 0.093608 | 0.211199 | 0.296191 | 0.001360 | 0.001524 | 0.001374 |
| alignment_of_a2_test.test | RUN | 0.093358 | 0.212167 | 0.310158 | 0.001440 | 0.001511 | 0.001441 |
| aligned_storage_test.test | RUN | 0.096098 | 0.216243 | 0.285186 | 0.001318 | 0.001708 | 0.001703 |
| aligned_storage_empy_test.test | RUN | 0.109547 | 0.235666 | 0.347548 | 0.001574 | 0.001584 | 0.001373 |
| aligned_storage_a2_test.test | RUN | 0.096951 | 0.208936 | 0.275094 | 0.001404 | 0.001743 | 0.001648 |
| add_volatile_test.test | RUN | 0.182876 | 0.586706 | 1.273496 | 0.001283 | 0.001346 | 0.001465 |
| add_rvalue_reference_test.test | RUN | 0.199125 | 0.669350 | 1.447753 | 0.001263 | 0.001330 | 0.001559 |
| add_reference_test.test | RUN | 0.198783 | 0.656248 | 1.402829 | 0.001300 | 0.001608 | 0.001524 |
| add_pointer_test.test | RUN | 0.135156 | 0.368438 | 0.732577 | 0.001414 | 0.001552 | 0.001491 |
| add_lvalue_reference_test.test | RUN | 0.197023 | 0.642332 | 1.437445 | 0.001415 | 0.001308 | 0.001529 |
| add_cv_test.test | RUN | 0.186813 | 0.617362 | 1.323371 | 0.001205 | 0.001504 | 0.001306 |
| add_const_test.test | RUN | 0.189552 | 0.614673 | 1.323328 | 0.001389 | 0.001316 | 0.001501 |
| is_virtual_base_of_fail.test | COMPILE_FAIL | 0.083760 | 0.164365 | 0.219596 | — | — | — |
| is_stateless_fail.test | COMPILE_FAIL | 0.086556 | 0.166456 | 0.228353 | — | — | — |
| is_polymorphic_fail.test | COMPILE_FAIL | 0.081776 | 0.160642 | 0.217792 | — | — | — |
| is_nothrow_move_constructible_fail.test | COMPILE_FAIL | 0.083009 | 0.160783 | 0.210067 | — | — | — |
| is_nothrow_move_assignable_fail.test | COMPILE_FAIL | 0.082526 | 0.160742 | 0.223048 | — | — | — |
| is_list_constructible_fail.test | COMPILE_FAIL | 0.078512 | 0.160028 | 0.220860 | — | — | — |
| is_empty_fail.test | COMPILE_FAIL | 0.083310 | 0.165040 | 0.225577 | — | — | — |
| is_destructible_fail.test | COMPILE_FAIL | 0.081275 | 0.163006 | 0.209501 | — | — | — |
| is_default_constructible_fail.test | COMPILE_FAIL | 0.082219 | 0.160318 | 0.219136 | — | — | — |
| is_copy_constructible_fail.test | COMPILE_FAIL | 0.081852 | 0.162529 | 0.215304 | — | — | — |
| is_copy_assignable_fail.test | COMPILE_FAIL | 0.081938 | 0.157606 | 0.207231 | — | — | — |
| is_convertible_fail.test | COMPILE_FAIL | 0.081789 | 0.165127 | 0.213584 | — | — | — |
| is_convertible2_fail.test | COMPILE_FAIL | 0.083145 | 0.162015 | 0.217077 | — | — | — |
| is_constructible_fail.test | COMPILE_FAIL | 0.082239 | 0.163895 | 0.213912 | — | — | — |
| is_base_of_fail.test | COMPILE_FAIL | 0.082281 | 0.163278 | 0.211477 | — | — | — |
| is_assignable_fail.test | COMPILE_FAIL | 0.081457 | 0.163044 | 0.220915 | — | — | — |
| has_nothrow_destruct_fail.test | COMPILE_FAIL | 0.082350 | 0.161543 | 0.220185 | — | — | — |
| has_nothrow_copy_fail.test | COMPILE_FAIL | 0.082178 | 0.163697 | 0.216605 | — | — | — |
| has_nothrow_cons_fail.test | COMPILE_FAIL | 0.082296 | 0.162713 | 0.215335 | — | — | — |
| has_nothrow_assign_fail.test | COMPILE_FAIL | 0.082071 | 0.165131 | 0.218741 | — | — | — |
| common_type_fail.test | COMPILE_FAIL | 0.083099 | 0.161090 | 0.219039 | — | — | — |
| common_type2_fail.test | COMPILE_FAIL | 0.082260 | 0.163955 | 0.213305 | — | — | — |
| has_nothrow_assign_test_no_intrinsics.test | RUN | 0.124409 | 0.339860 | 0.576267 | 0.001436 | 0.001402 | 0.001482 |
| has_nothrow_constr_test_no_intrinsics.test | RUN | 0.121511 | 0.335567 | 0.584262 | 0.001259 | 0.001409 | 0.001322 |
| has_nothrow_copy_test_no_intrinsics.test | RUN | 0.127739 | 0.344681 | 0.550767 | 0.001396 | 0.001234 | 0.001537 |
| is_nothrow_move_assignable_test_no_intrinsics.test | RUN | 0.124902 | 0.340754 | 0.558966 | 0.001096 | 0.001186 | 0.001124 |
| is_nothrow_move_constructible_test_no_intrinsics.test | RUN | 0.090670 | 0.239980 | 0.373328 | 0.001119 | 0.001117 | 0.001051 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.989× | 0.986×, 0.992× | 0.998× |
| clang | 1.038× | 1.046×, 1.030× | 0.983× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| add_const_test.test | 10 | 0.000986s | 0.000923s | 0.000971s | byte equality observed |
| add_cv_test.test | 10 | 0.000962s | 0.000937s | 0.000998s | byte equality observed |
| add_lvalue_reference_test.test | 10 | 0.000996s | 0.000961s | 0.000984s | byte equality observed |
| add_pointer_test.test | 10 | 0.000965s | 0.000947s | 0.000978s | byte equality observed |
| add_reference_test.test | 10 | 0.000972s | 0.000925s | 0.000949s | byte equality observed |
| add_rvalue_reference_test.test | 10 | 0.000960s | 0.000941s | 0.000975s | byte equality observed |
| add_volatile_test.test | 10 | 0.000961s | 0.000944s | 0.000974s | byte equality observed |
| aligned_storage_a2_test.test | 10 | 0.001068s | 0.001049s | 0.001056s | byte equality observed |
| aligned_storage_empy_test.test | 10 | 0.000969s | 0.000928s | 0.000970s | byte equality observed |
| aligned_storage_test.test | 10 | 0.001058s | 0.001046s | 0.001063s | byte equality observed |
| alignment_of_a2_test.test | 10 | 0.000974s | 0.000927s | 0.000970s | byte equality observed |
| alignment_of_test.test | 10 | 0.000976s | 0.000957s | 0.000990s | byte equality observed |
| common_type_2_test.test | 10 | 0.000972s | 0.000936s | 0.000981s | byte equality observed |
| common_type_3_test.test | 10 | 0.000964s | 0.000955s | 0.000959s | byte equality observed |
| common_type_4_test.test | 10 | 0.000953s | 0.000940s | 0.000968s | byte equality observed |
| common_type_5_test.test | 10 | 0.000973s | 0.000923s | 0.000976s | byte equality observed |
| common_type_6_test.test | 10 | 0.000952s | 0.000949s | 0.000975s | byte equality observed |
| common_type_sfinae2_test.test | 10 | 0.000956s | 0.000934s | 0.000954s | byte equality observed |
| common_type_sfinae_test.test | 10 | 0.000971s | 0.000934s | 0.000975s | byte equality observed |
| common_type_test.test | 10 | 0.000968s | 0.000948s | 0.000995s | byte equality observed |
| conditional_test.test | 10 | 0.000977s | 0.000926s | 0.000973s | byte equality observed |
| conjunction_test.test | 10 | 0.000963s | 0.000930s | 0.001003s | byte equality observed |
| copy_cv_ref_test.test | 10 | 0.000962s | 0.000916s | 0.000970s | byte equality observed |
| copy_cv_test.test | 10 | 0.000950s | 0.000934s | 0.000988s | byte equality observed |
| copy_reference_test.test | 10 | 0.000964s | 0.000963s | 0.001034s | byte equality observed |
| cxx14_aliases_test.test | 10 | 0.000952s | 0.000929s | 0.000997s | byte equality observed |
| decay_test.test | 10 | 0.000971s | 0.000947s | 0.000994s | byte equality observed |
| detected_or_test.test | 10 | 0.000957s | 0.000945s | 0.000993s | byte equality observed |
| detected_test.test | 10 | 0.000950s | 0.000935s | 0.000986s | byte equality observed |
| disjunction_test.test | 10 | 0.000943s | 0.000942s | 0.001016s | byte equality observed |
| enable_if_test.test | 10 | 0.000968s | 0.000948s | 0.000981s | byte equality observed |
| extent_test.test | 10 | 0.000942s | 0.000938s | 0.000970s | byte equality observed |
| floating_point_promotion_test.test | 10 | 0.000969s | 0.000971s | 0.001013s | byte equality observed |
| function_traits_test.test | 10 | 0.000933s | 0.000929s | 0.001011s | byte equality observed |
| has_binary_classes0_test.test | 10 | 0.000967s | 0.000935s | 0.001006s | byte equality observed |
| has_binary_classes1_test.test | 10 | 0.000931s | 0.000948s | 0.001004s | byte equality observed |
| has_binary_classes2_test.test | 10 | 0.000951s | 0.000944s | 0.000976s | byte equality observed |
| has_binary_classes3_test.test | 10 | 0.000942s | 0.000927s | 0.000998s | byte equality observed |
| has_binary_classes4_test.test | 10 | 0.000975s | 0.000948s | 0.000987s | byte equality observed |
| has_binary_classes5_test.test | 10 | 0.000948s | 0.000950s | 0.000998s | byte equality observed |
| has_binary_classes6_test.test | 10 | 0.000955s | 0.000933s | 0.000971s | byte equality observed |
| has_binary_classes7_test.test | 10 | 0.000947s | 0.000941s | 0.000982s | byte equality observed |
| has_binary_classes8_test.test | 10 | 0.000944s | 0.000924s | 0.000982s | byte equality observed |
| has_binary_classes9_test.test | 10 | 0.000957s | 0.000924s | 0.000975s | byte equality observed |
| has_bit_and_assign_test.test | 10 | 0.000961s | 0.000923s | 0.000990s | byte equality observed |
| has_bit_and_test.test | 10 | 0.000966s | 0.000927s | 0.000985s | byte equality observed |
| has_bit_or_assign_test.test | 10 | 0.000948s | 0.000930s | 0.001009s | byte equality observed |
| has_bit_or_test.test | 10 | 0.000965s | 0.000936s | 0.000978s | byte equality observed |
| has_bit_xor_assign_test.test | 10 | 0.000939s | 0.000931s | 0.000993s | byte equality observed |
| has_bit_xor_test.test | 10 | 0.000950s | 0.000939s | 0.001008s | byte equality observed |
| has_complement_test.test | 10 | 0.000945s | 0.000918s | 0.000968s | byte equality observed |
| has_dereference_test.test | 10 | 0.000940s | 0.000952s | 0.000979s | byte equality observed |
| has_divides_assign_test.test | 10 | 0.000979s | 0.000930s | 0.001042s | byte equality observed |
| has_divides_test.test | 10 | 0.000944s | 0.000938s | 0.000994s | byte equality observed |
| has_equal_to_test.test | 10 | 0.000953s | 0.000924s | 0.000981s | byte equality observed |
| has_greater_equal_test.test | 10 | 0.000951s | 0.000931s | 0.000985s | byte equality observed |
| has_greater_test.test | 10 | 0.000978s | 0.000949s | 0.000974s | byte equality observed |
| has_left_shift_assign_test.test | 10 | 0.000953s | 0.000915s | 0.000991s | byte equality observed |
| has_left_shift_test.test | 10 | 0.000940s | 0.000930s | 0.000997s | byte equality observed |
| has_less_equal_test.test | 10 | 0.000960s | 0.000933s | 0.000982s | byte equality observed |
| has_less_test.test | 10 | 0.000936s | 0.000942s | 0.000986s | byte equality observed |
| has_logical_and_test.test | 10 | 0.000959s | 0.000938s | 0.001022s | byte equality observed |
| has_logical_not_test.test | 10 | 0.000953s | 0.000937s | 0.000968s | byte equality observed |
| has_logical_or_test.test | 10 | 0.000959s | 0.000922s | 0.001000s | byte equality observed |
| has_minus_assign_test.test | 10 | 0.000957s | 0.000926s | 0.000984s | byte equality observed |
| has_minus_test.test | 10 | 0.000942s | 0.000912s | 0.001008s | byte equality observed |
| has_modulus_assign_test.test | 10 | 0.000946s | 0.000950s | 0.000999s | byte equality observed |
| has_modulus_test.test | 10 | 0.000961s | 0.000930s | 0.000986s | byte equality observed |
| has_multiplies_assign_test.test | 10 | 0.000980s | 0.000927s | 0.001009s | byte equality observed |
| has_multiplies_test.test | 10 | 0.000940s | 0.000923s | 0.001006s | byte equality observed |
| has_negate_test.test | 10 | 0.000972s | 0.000948s | 0.000985s | byte equality observed |
| has_not_equal_to_test.test | 10 | 0.000935s | 0.000927s | 0.001002s | byte equality observed |
| has_nothrow_assign_test.test | 10 | 0.000957s | 0.000929s | 0.000993s | byte equality observed |
| has_nothrow_assign_test_no_intrinsics.test | 10 | 0.000952s | 0.000948s | 0.000996s | byte equality observed |
| has_nothrow_constr_test.test | 10 | 0.000986s | 0.000926s | 0.000995s | byte equality observed |
| has_nothrow_constr_test_no_intrinsics.test | 10 | 0.000969s | 0.000927s | 0.000980s | byte equality observed |
| has_nothrow_copy_test.test | 10 | 0.000959s | 0.000950s | 0.001008s | byte equality observed |
| has_nothrow_copy_test_no_intrinsics.test | 10 | 0.000952s | 0.000916s | 0.000982s | byte equality observed |
| has_nothrow_destructor_test.test | 10 | 0.000976s | 0.000939s | 0.001006s | byte equality observed |
| has_operator_new_test.test | 10 | 0.000987s | 0.000935s | 0.000982s | byte equality observed |
| has_plus_assign_test.test | 10 | 0.000981s | 0.000936s | 0.001009s | byte equality observed |
| has_plus_test.test | 10 | 0.000971s | 0.000914s | 0.000981s | byte equality observed |
| has_post_decrement_test.test | 10 | 0.000985s | 0.000929s | 0.001009s | byte equality observed |
| has_post_increment_test.test | 10 | 0.000958s | 0.000925s | 0.000994s | byte equality observed |
| has_postfix_classes0_test.test | 10 | 0.000955s | 0.000952s | 0.000988s | byte equality observed |
| has_postfix_classes1_test.test | 10 | 0.000958s | 0.000914s | 0.001039s | byte equality observed |
| has_postfix_classes2_test.test | 10 | 0.000963s | 0.000930s | 0.000999s | byte equality observed |
| has_postfix_classes3_test.test | 10 | 0.000960s | 0.000926s | 0.000973s | byte equality observed |
| has_pre_decrement_test.test | 10 | 0.000953s | 0.000940s | 0.000992s | byte equality observed |
| has_pre_increment_test.test | 10 | 0.000962s | 0.000944s | 0.000995s | byte equality observed |
| has_prefix_classes0_test.test | 10 | 0.000952s | 0.000916s | 0.001010s | byte equality observed |
| has_prefix_classes1_test.test | 10 | 0.000964s | 0.000931s | 0.000983s | byte equality observed |
| has_prefix_classes2_test.test | 10 | 0.000951s | 0.000941s | 0.000993s | byte equality observed |
| has_prefix_classes3_test.test | 10 | 0.000966s | 0.000925s | 0.001004s | byte equality observed |
| has_right_shift_assign_test.test | 10 | 0.000975s | 0.000941s | 0.001000s | byte equality observed |
| has_right_shift_test.test | 10 | 0.000933s | 0.000934s | 0.000993s | byte equality observed |
| has_trivial_assign_test.test | 10 | 0.001007s | 0.000992s | 0.001053s | byte equality observed |
| has_trivial_constr_test.test | 10 | 0.001016s | 0.000943s | 0.001057s | native test oracle; output varies |
| has_trivial_copy_test.test | 10 | 0.001013s | 0.000985s | 0.001094s | native test oracle; output varies |
| has_trivial_destructor_test.test | 10 | 0.000964s | 0.000923s | 0.000994s | byte equality observed |
| has_trivial_move_assign_test.test | 10 | 0.000970s | 0.000915s | 0.001003s | byte equality observed |
| has_trivial_move_constructor_test.test | 10 | 0.000980s | 0.000926s | 0.001000s | byte equality observed |
| has_unary_minus_test.test | 10 | 0.000969s | 0.000938s | 0.001007s | byte equality observed |
| has_unary_plus_test.test | 10 | 0.000966s | 0.000933s | 0.000987s | byte equality observed |
| has_virtual_destructor_test.test | 10 | 0.000967s | 0.000939s | 0.001001s | byte equality observed |
| integral_constant_test.test | 10 | 0.000450s | 0.000448s | 0.001034s | byte equality observed |
| is_abstract_test.test | 10 | 0.000956s | 0.000954s | 0.001033s | byte equality observed |
| is_arithmetic_test.test | 10 | 0.000950s | 0.000954s | 0.001060s | byte equality observed |
| is_array_test.test | 10 | 0.000966s | 0.000938s | 0.001069s | byte equality observed |
| is_assignable_test.test | 10 | 0.000963s | 0.000933s | 0.001010s | byte equality observed |
| is_base_and_derived_test.test | 10 | 0.000937s | 0.000933s | 0.000973s | byte equality observed |
| is_base_of_test.test | 10 | 0.000969s | 0.000938s | 0.000964s | byte equality observed |
| is_bounded_array_test.test | 10 | 0.000962s | 0.000937s | 0.001057s | byte equality observed |
| is_class_test.test | 10 | 0.000971s | 0.000918s | 0.000981s | byte equality observed |
| is_complete_test.test | 10 | 0.000969s | 0.000929s | 0.000983s | byte equality observed |
| is_complex_test.test | 10 | 0.000982s | 0.000929s | 0.000981s | byte equality observed |
| is_compound_test.test | 10 | 0.000983s | 0.000935s | 0.000994s | byte equality observed |
| is_const_test.test | 10 | 0.000975s | 0.000922s | 0.000996s | byte equality observed |
| is_constructible_test.test | 10 | 0.000974s | 0.000920s | 0.000997s | byte equality observed |
| is_convertible_test.test | 10 | 0.000985s | 0.000914s | 0.000988s | byte equality observed |
| is_copy_assignable_test.test | 10 | 0.000962s | 0.000954s | 0.001000s | byte equality observed |
| is_copy_constructible_test.test | 10 | 0.000982s | 0.000932s | 0.000986s | byte equality observed |
| is_default_constr_test.test | 10 | 0.000988s | 0.000923s | 0.000985s | byte equality observed |
| is_destructible_test.test | 10 | 0.000970s | 0.000935s | 0.001005s | byte equality observed |
| is_detected_exact_test.test | 10 | 0.000947s | 0.000927s | 0.000976s | byte equality observed |
| is_detected_test.test | 10 | 0.000986s | 0.000929s | 0.000989s | byte equality observed |
| is_empty_test.test | 10 | 0.000952s | 0.000957s | 0.001013s | byte equality observed |
| is_enum_test.test | 10 | 0.000968s | 0.000940s | 0.000987s | byte equality observed |
| is_final_test.test | 10 | 0.000971s | 0.000951s | 0.001032s | byte equality observed |
| is_float_test.test | 10 | 0.000973s | 0.000941s | 0.001007s | byte equality observed |
| is_floating_point_test.test | 10 | 0.000953s | 0.000926s | 0.000982s | byte equality observed |
| is_function_test.test | 10 | 0.000983s | 0.000918s | 0.000994s | byte equality observed |
| is_fundamental_test.test | 10 | 0.000975s | 0.000935s | 0.000991s | byte equality observed |
| is_integral_test.test | 10 | 0.000947s | 0.000940s | 0.000989s | byte equality observed |
| is_list_constructible_test.test | 10 | 0.000980s | 0.000937s | 0.001003s | byte equality observed |
| is_lvalue_reference_test.test | 10 | 0.000984s | 0.000947s | 0.000989s | byte equality observed |
| is_member_func_test.test | 10 | 0.000962s | 0.000928s | 0.000989s | byte equality observed |
| is_member_obj_test.test | 10 | 0.000954s | 0.000944s | 0.001009s | byte equality observed |
| is_member_pointer_test.test | 10 | 0.000966s | 0.000935s | 0.000968s | byte equality observed |
| is_noncopyable_test.test | 10 | 0.000977s | 0.000932s | 0.000977s | byte equality observed |
| is_nothrow_move_assignable_test.test | 10 | 0.000965s | 0.000927s | 0.001027s | byte equality observed |
| is_nothrow_move_assignable_test_no_intrinsics.test | 10 | 0.000953s | 0.000926s | 0.001009s | byte equality observed |
| is_nothrow_move_constructible_test.test | 10 | 0.000985s | 0.000940s | 0.001004s | byte equality observed |
| is_nothrow_move_constructible_test_no_intrinsics.test | 10 | 0.000970s | 0.000945s | 0.001001s | byte equality observed |
| is_nothrow_swappable_test.test | 10 | 0.000963s | 0.000950s | 0.000998s | byte equality observed |
| is_object_test.test | 10 | 0.000944s | 0.000935s | 0.000998s | byte equality observed |
| is_pod_test.test | 10 | 0.000969s | 0.000928s | 0.001018s | byte equality observed |
| is_pointer_test.test | 10 | 0.000965s | 0.000924s | 0.001001s | byte equality observed |
| is_polymorphic_test.test | 10 | 0.000972s | 0.000951s | 0.001013s | byte equality observed |
| is_reference_test.test | 10 | 0.000974s | 0.000929s | 0.000983s | byte equality observed |
| is_rvalue_reference_test.test | 10 | 0.000964s | 0.000928s | 0.000996s | byte equality observed |
| is_same_test.test | 10 | 0.000939s | 0.000925s | 0.001004s | byte equality observed |
| is_scalar_test.test | 10 | 0.000949s | 0.000914s | 0.001031s | byte equality observed |
| is_scoped_enum_test.test | 10 | 0.000969s | 0.000939s | 0.000969s | byte equality observed |
| is_signed_test.test | 10 | 0.000949s | 0.000919s | 0.001024s | byte equality observed |
| is_stateless_test.test | 10 | 0.000956s | 0.000938s | 0.001050s | byte equality observed |
| is_swappable_test.test | 10 | 0.000957s | 0.000963s | 0.000994s | byte equality observed |
| is_trivially_copyable_test.test | 10 | 0.000994s | 0.000969s | 0.001071s | byte equality observed |
| is_unbounded_array_test.test | 10 | 0.000968s | 0.000924s | 0.001008s | byte equality observed |
| is_union_test.test | 10 | 0.000973s | 0.000935s | 0.001008s | byte equality observed |
| is_unscoped_enum_test.test | 10 | 0.000958s | 0.000927s | 0.001032s | byte equality observed |
| is_unsigned_test.test | 10 | 0.000971s | 0.000953s | 0.001078s | byte equality observed |
| is_virtual_base_of_test.test | 10 | 0.000956s | 0.000935s | 0.001087s | byte equality observed |
| is_void_test.test | 10 | 0.000959s | 0.000930s | 0.001098s | byte equality observed |
| is_volatile_test.test | 10 | 0.000969s | 0.000927s | 0.001098s | byte equality observed |
| make_signed_test.test | 10 | 0.000975s | 0.000920s | 0.001102s | byte equality observed |
| make_unsigned_test.test | 10 | 0.000963s | 0.000922s | 0.001126s | byte equality observed |
| make_void_test.test | 10 | 0.000955s | 0.000943s | 0.001108s | byte equality observed |
| mpl_interop_test1.test | 10 | 0.000463s | 0.000444s | 0.001132s | byte equality observed |
| mpl_interop_test2.test | 10 | 0.000465s | 0.000445s | 0.001106s | byte equality observed |
| mpl_interop_test3.test | 10 | 0.000443s | 0.000438s | 0.001125s | byte equality observed |
| negation_test.test | 10 | 0.000976s | 0.000950s | 0.001129s | byte equality observed |
| promote_basic_test.test | 10 | 0.000451s | 0.000443s | 0.001110s | byte equality observed |
| promote_enum_msvc_bug_test.test | 10 | 0.000455s | 0.000440s | 0.001099s | byte equality observed |
| promote_enum_test.test | 10 | 0.000466s | 0.000444s | 0.001108s | byte equality observed |
| promote_mpl_test.test | 10 | 0.000447s | 0.000443s | 0.001112s | byte equality observed |
| rank_test.test | 10 | 0.000972s | 0.000936s | 0.001115s | byte equality observed |
| remove_all_extents_test.test | 10 | 0.000986s | 0.000917s | 0.001102s | byte equality observed |
| remove_bounds_test.test | 10 | 0.000950s | 0.000927s | 0.001104s | byte equality observed |
| remove_const_test.test | 10 | 0.000960s | 0.000911s | 0.001119s | byte equality observed |
| remove_cv_ref_test.test | 10 | 0.000942s | 0.000920s | 0.001139s | byte equality observed |
| remove_cv_test.test | 10 | 0.000939s | 0.000931s | 0.001107s | byte equality observed |
| remove_extent_test.test | 10 | 0.000944s | 0.000921s | 0.001114s | byte equality observed |
| remove_pointer_test.test | 10 | 0.000941s | 0.000935s | 0.001096s | byte equality observed |
| remove_reference_test.test | 10 | 0.000957s | 0.000932s | 0.001110s | byte equality observed |
| remove_volatile_test.test | 10 | 0.000957s | 0.000940s | 0.001101s | byte equality observed |
| tricky_abstract_type_test.test | 10 | 0.000949s | 0.000918s | 0.001123s | byte equality observed |
| tricky_add_pointer_test.test | 10 | 0.000959s | 0.000923s | 0.001107s | byte equality observed |
| tricky_function_type_test.test | 10 | 0.000963s | 0.000926s | 0.001123s | byte equality observed |
| tricky_incomplete_type_test.test | 10 | 0.000958s | 0.000929s | 0.001092s | byte equality observed |
| tricky_is_enum_test.test | 10 | 0.000991s | 0.000938s | 0.001092s | byte equality observed |
| tricky_partial_spec_test.test | 10 | 0.000960s | 0.000934s | 0.001091s | byte equality observed |
| tricky_rvalue_test.test | 10 | 0.000992s | 0.000942s | 0.001131s | byte equality observed |
| type_identity_test.test | 10 | 0.000995s | 0.000953s | 0.001088s | byte equality observed |
| type_traits_test.test | 10 | 0.000462s | 0.000451s | 0.001077s | byte equality observed |
| type_with_alignment_test.test | 10 | 0.001099s | 0.001072s | 0.001221s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
