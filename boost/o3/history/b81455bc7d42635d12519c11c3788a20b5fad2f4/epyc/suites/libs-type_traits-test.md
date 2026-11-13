# libs/type_traits/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| type_with_alignment_test.test | RUN | 0.081217 | 0.229504 | 0.311780 | 0.001352 | 0.001351 | 0.001217 |
| type_traits_test.test | RUN | 0.116101 | 0.236459 | 0.296563 | 0.000467 | 0.000536 | 0.001315 |
| type_identity_test.test | RUN | 0.209697 | 0.771074 | 1.613797 | 0.001509 | 0.001561 | 0.001309 |
| tricky_rvalue_test.test | RUN | 0.075674 | 0.183762 | 0.240844 | 0.001334 | 0.001414 | 0.001293 |
| tricky_partial_spec_test.test | RUN | 0.092369 | 0.257528 | 0.378704 | 0.001289 | 0.001342 | 0.001276 |
| tricky_is_enum_test.test | RUN | 0.074797 | 0.175617 | 0.224128 | 0.001362 | 0.001388 | 0.001305 |
| tricky_incomplete_type_test.test | RUN | 0.125618 | 0.267220 | 0.368663 | 0.001160 | 0.001322 | 0.001273 |
| tricky_function_type_test.test | RUN | 0.100508 | 0.270579 | 0.423182 | 0.001436 | 0.001506 | 0.001087 |
| tricky_add_pointer_test.test | RUN | 0.142825 | 0.460764 | 0.919180 | 0.001418 | 0.001548 | 0.001364 |
| tricky_abstract_type_test.test | RUN | 0.079640 | 0.181007 | 0.229143 | 0.001454 | 0.001161 | 0.001312 |
| remove_volatile_test.test | RUN | 0.191659 | 0.682258 | 1.381338 | 0.001424 | 0.001489 | 0.001411 |
| remove_reference_test.test | RUN | 0.199427 | 0.716304 | 1.473073 | 0.001382 | 0.001410 | 0.001333 |
| remove_pointer_test.test | RUN | 0.205437 | 0.739812 | 1.552784 | 0.001425 | 0.001329 | 0.001346 |
| remove_extent_test.test | RUN | 0.196930 | 0.717175 | 1.478682 | 0.001246 | 0.001562 | 0.001346 |
| remove_cv_test.test | RUN | 0.194139 | 0.723459 | 1.474026 | 0.001426 | 0.001523 | 0.001384 |
| remove_cv_ref_test.test | RUN | 0.406252 | 1.718893 | 3.679174 | 0.001436 | 0.001325 | 0.001361 |
| remove_const_test.test | RUN | 0.185848 | 0.668529 | 1.400571 | 0.001407 | 0.001436 | 0.001333 |
| remove_bounds_test.test | RUN | 0.197539 | 0.713009 | 1.476395 | 0.001369 | 0.001470 | 0.001181 |
| remove_all_extents_test.test | RUN | 0.195113 | 0.715467 | 1.520672 | 0.001341 | 0.001455 | 0.001210 |
| rank_test.test | RUN | 0.078714 | 0.178286 | 0.224574 | 0.001318 | 0.001470 | 0.001325 |
| promote_mpl_test.test | RUN | 0.063634 | 0.161351 | 0.166999 | 0.000497 | 0.000578 | 0.001101 |
| promote_enum_test.test | RUN | 0.015798 | 0.043543 | 0.046801 | 0.000492 | 0.000538 | 0.001279 |
| promote_enum_msvc_bug_test.test | RUN | 0.015255 | 0.038606 | 0.039718 | 0.000519 | 0.000546 | 0.001284 |
| promote_basic_test.test | RUN | 0.018529 | 0.050375 | 0.044321 | 0.000492 | 0.000540 | 0.001094 |
| negation_test.test | RUN | 0.074171 | 0.172674 | 0.208313 | 0.001210 | 0.001348 | 0.001293 |
| mpl_interop_test3.test | RUN | 0.061133 | 0.158972 | 0.163424 | 0.000536 | 0.000509 | 0.001202 |
| mpl_interop_test2.test | RUN | 0.023450 | 0.047311 | 0.049246 | 0.000511 | 0.000565 | 0.001316 |
| mpl_interop_test1.test | RUN | 0.013395 | 0.035297 | 0.038789 | 0.000528 | 0.000567 | 0.001271 |
| make_void_test.test | RUN | 0.078274 | 0.181530 | 0.237011 | 0.001492 | 0.001406 | 0.001347 |
| make_unsigned_test.test | RUN | 0.091094 | 0.237024 | 0.342206 | 0.001237 | 0.001395 | 0.001236 |
| make_signed_test.test | RUN | 0.092033 | 0.238608 | 0.315538 | 0.001206 | 0.001364 | 0.001313 |
| is_volatile_test.test | RUN | 0.083685 | 0.205874 | 0.285867 | 0.001248 | 0.001564 | 0.001324 |
| is_void_test.test | RUN | 0.078945 | 0.196668 | 0.260562 | 0.001555 | 0.001552 | 0.001482 |
| is_virtual_base_of_test.test | RUN | 0.092671 | 0.244297 | 0.348225 | 0.001267 | 0.001228 | 0.001203 |
| is_unsigned_test.test | RUN | 0.080959 | 0.203324 | 0.250782 | 0.001183 | 0.001301 | 0.001238 |
| is_unscoped_enum_test.test | RUN | 0.080650 | 0.179227 | 0.224745 | 0.001165 | 0.001344 | 0.001390 |
| is_union_test.test | RUN | 0.084674 | 0.222572 | 0.313218 | 0.001523 | 0.001447 | 0.001390 |
| is_unbounded_array_test.test | RUN | 0.079632 | 0.191154 | 0.260694 | 0.001260 | 0.001294 | 0.001170 |
| is_trivially_copyable_test.test | RUN | 0.123158 | 0.372081 | 0.625625 | 0.001356 | 0.001504 | 0.001127 |
| is_swappable_test.test | RUN | 0.104639 | 0.283844 | 0.469523 | 0.001411 | 0.001429 | 0.001598 |
| is_stateless_test.test | RUN | 0.116598 | 0.335115 | 0.528891 | 0.001455 | 0.001297 | 0.001385 |
| is_signed_test.test | RUN | 0.080484 | 0.202693 | 0.259884 | 0.001281 | 0.001280 | 0.001169 |
| is_scoped_enum_test.test | RUN | 0.080185 | 0.179300 | 0.221278 | 0.001308 | 0.001433 | 0.001370 |
| is_scalar_test.test | RUN | 0.102371 | 0.307120 | 0.483315 | 0.001376 | 0.001431 | 0.001347 |
| is_same_test.test | RUN | 0.077747 | 0.191952 | 0.257672 | 0.001493 | 0.001351 | 0.001384 |
| is_rvalue_reference_test.test | RUN | 0.080320 | 0.211061 | 0.305512 | 0.001425 | 0.001370 | 0.001282 |
| is_reference_test.test | RUN | 0.085997 | 0.208488 | 0.295607 | 0.001344 | 0.001341 | 0.001439 |
| is_polymorphic_test.test | RUN | 0.086326 | 0.221811 | 0.314303 | 0.001241 | 0.001492 | 0.001385 |
| is_pointer_test.test | RUN | 0.084190 | 0.216415 | 0.296938 | 0.001487 | 0.001461 | 0.001324 |
| is_pod_test.test | RUN | 0.107277 | 0.341077 | 0.546429 | 0.001374 | 0.001523 | 0.001438 |
| is_object_test.test | RUN | 0.079816 | 0.186564 | 0.251065 | 0.001344 | 0.001447 | 0.001392 |
| is_nothrow_swappable_test.test | RUN | 0.109751 | 0.322231 | 0.505193 | 0.001471 | 0.001399 | 0.001377 |
| is_nothrow_move_constructible_test.test | RUN | 0.108644 | 0.333980 | 0.507842 | 0.001301 | 0.001422 | 0.001331 |
| is_nothrow_move_assignable_test.test | RUN | 0.105309 | 0.331112 | 0.542204 | 0.001140 | 0.001292 | 0.001114 |
| is_noncopyable_test.test | RUN | 0.079951 | 0.188587 | 0.254768 | 0.001248 | 0.001299 | 0.001173 |
| is_member_pointer_test.test | RUN | 0.081803 | 0.190191 | 0.242308 | 0.001392 | 0.001387 | 0.001204 |
| is_member_obj_test.test | RUN | 0.075355 | 0.191835 | 0.263189 | 0.001514 | 0.001444 | 0.001090 |
| is_member_func_test.test | RUN | 0.080488 | 0.203072 | 0.272413 | 0.001250 | 0.001147 | 0.001257 |
| is_lvalue_reference_test.test | RUN | 0.080485 | 0.216032 | 0.310410 | 0.001249 | 0.001393 | 0.001410 |
| is_list_constructible_test.test | RUN | 0.097598 | 0.242509 | 0.336351 | 0.001412 | 0.001562 | 0.001390 |
| is_integral_test.test | RUN | 0.096986 | 0.287738 | 0.418050 | 0.001406 | 0.001239 | 0.001253 |
| is_fundamental_test.test | RUN | 0.096090 | 0.299365 | 0.464584 | 0.001226 | 0.001343 | 0.001391 |
| is_function_test.test | RUN | 0.079478 | 0.198999 | 0.281938 | 0.001309 | 0.001555 | 0.001205 |
| is_floating_point_test.test | RUN | 0.083692 | 0.210515 | 0.282271 | 0.001508 | 0.001545 | 0.001220 |
| is_float_test.test | RUN | 0.084301 | 0.211641 | 0.307149 | 0.001337 | 0.001167 | 0.001190 |
| is_final_test.test | RUN | 0.082112 | 0.214393 | 0.309496 | 0.001217 | 0.001439 | 0.001157 |
| is_enum_test.test | RUN | 0.078533 | 0.181639 | 0.241421 | 0.001217 | 0.001354 | 0.001321 |
| is_empty_test.test | RUN | 0.083821 | 0.201383 | 0.281455 | 0.001518 | 0.001347 | 0.001293 |
| is_detected_test.test | RUN | 0.075966 | 0.174164 | 0.224623 | 0.001432 | 0.001373 | 0.001227 |
| is_detected_exact_test.test | RUN | 0.075343 | 0.174891 | 0.221097 | 0.001271 | 0.001293 | 0.001139 |
| is_destructible_test.test | RUN | 0.106938 | 0.325125 | 0.531762 | 0.001331 | 0.001544 | 0.001396 |
| is_default_constr_test.test | RUN | 0.110446 | 0.331519 | 0.538701 | 0.001296 | 0.001427 | 0.001280 |
| is_copy_constructible_test.test | RUN | 0.113627 | 0.358554 | 0.578940 | 0.001365 | 0.001347 | 0.001192 |
| is_copy_assignable_test.test | RUN | 0.110912 | 0.347340 | 0.578743 | 0.001269 | 0.001321 | 0.001214 |
| is_convertible_test.test | RUN | 0.107599 | 0.304476 | 0.480922 | 0.001299 | 0.001464 | 0.001333 |
| is_constructible_test.test | RUN | 0.089349 | 0.232077 | 0.352528 | 0.001346 | 0.001294 | 0.001292 |
| is_const_test.test | RUN | 0.078911 | 0.199435 | 0.283550 | 0.001172 | 0.001321 | 0.001374 |
| is_compound_test.test | RUN | 0.075745 | 0.176811 | 0.234570 | 0.001344 | 0.001243 | 0.001171 |
| is_complex_test.test | RUN | 0.103632 | 0.236389 | 0.321061 | 0.001210 | 0.001543 | 0.001279 |
| is_complete_test.test | RUN | 0.089232 | 0.231822 | 0.348413 | 0.001400 | 0.001110 | 0.001352 |
| is_class_test.test | RUN | 0.082228 | 0.210284 | 0.300786 | 0.001204 | 0.001408 | 0.000981 |
| is_bounded_array_test.test | RUN | 0.077825 | 0.191143 | 0.261617 | 0.001183 | 0.001235 | 0.001264 |
| is_base_of_test.test | RUN | 0.085644 | 0.209220 | 0.300748 | 0.001218 | 0.001504 | 0.001293 |
| is_base_and_derived_test.test | RUN | 0.083017 | 0.202789 | 0.283781 | 0.001407 | 0.001189 | 0.001332 |
| is_assignable_test.test | RUN | 0.097191 | 0.287194 | 0.435384 | 0.001251 | 0.001498 | 0.001336 |
| is_array_test.test | RUN | 0.082111 | 0.185054 | 0.283547 | 0.001234 | 0.001364 | 0.001443 |
| is_arithmetic_test.test | RUN | 0.098440 | 0.289967 | 0.424886 | 0.001400 | 0.001525 | 0.001391 |
| is_abstract_test.test | RUN | 0.170692 | 0.808500 | 1.482322 | 0.001380 | 0.001534 | 0.001164 |
| integral_constant_test.test | RUN | 0.013365 | 0.034343 | 0.039390 | 0.000502 | 0.000625 | 0.001391 |
| has_virtual_destructor_test.test | RUN | 0.087342 | 0.227548 | 0.320075 | 0.001258 | 0.001512 | 0.001333 |
| has_unary_plus_test.test | RUN | 0.161520 | 0.837907 | 1.147022 | 0.001395 | 0.001593 | 0.001366 |
| has_unary_minus_test.test | RUN | 0.158143 | 0.867175 | 1.121246 | 0.001398 | 0.001512 | 0.001389 |
| has_trivial_move_constructor_test.test | RUN | 0.111546 | 0.348210 | 0.548210 | 0.001360 | 0.001295 | 0.001425 |
| has_trivial_move_assign_test.test | RUN | 0.108236 | 0.328070 | 0.559906 | 0.001030 | 0.001545 | 0.001416 |
| has_trivial_destructor_test.test | RUN | 0.107373 | 0.344115 | 0.537521 | 0.001282 | 0.001466 | 0.001413 |
| has_trivial_copy_test.test | RUN | 0.112925 | 0.353889 | 0.578994 | 0.001654 | 0.001798 | 0.001458 |
| has_trivial_constr_test.test | RUN | 0.115473 | 0.363636 | 0.566648 | 0.001225 | 0.001667 | 0.001492 |
| has_trivial_assign_test.test | RUN | 0.108500 | 0.358331 | 0.585492 | 0.001390 | 0.001589 | 0.001422 |
| has_right_shift_test.test | RUN | 0.165561 | 0.663720 | 1.219139 | 0.001520 | 0.001565 | 0.001240 |
| has_right_shift_assign_test.test | RUN | 0.159110 | 0.642831 | 1.116983 | 0.001302 | 0.001524 | 0.001203 |
| has_prefix_classes3_test.test | RUN | 0.119659 | 0.363930 | 0.597608 | 0.001055 | 0.001517 | 0.001328 |
| has_prefix_classes2_test.test | RUN | 0.118005 | 0.365303 | 0.582823 | 0.001307 | 0.001343 | 0.001236 |
| has_prefix_classes1_test.test | RUN | 0.118445 | 0.370159 | 0.589139 | 0.001514 | 0.001556 | 0.001233 |
| has_prefix_classes0_test.test | RUN | 0.116798 | 0.367419 | 0.598896 | 0.001319 | 0.001140 | 0.001343 |
| has_pre_increment_test.test | RUN | 0.155412 | 0.867344 | 1.110418 | 0.001067 | 0.001580 | 0.001428 |
| has_pre_decrement_test.test | RUN | 0.156511 | 0.874317 | 1.112419 | 0.001333 | 0.001313 | 0.001298 |
| has_postfix_classes3_test.test | RUN | 0.119641 | 0.363459 | 0.598644 | 0.001455 | 0.001602 | 0.001309 |
| has_postfix_classes2_test.test | RUN | 0.117909 | 0.357945 | 0.570819 | 0.001308 | 0.001267 | 0.001458 |
| has_postfix_classes1_test.test | RUN | 0.121504 | 0.368893 | 0.606494 | 0.001290 | 0.001567 | 0.001425 |
| has_postfix_classes0_test.test | RUN | 0.119477 | 0.373140 | 0.609109 | 0.001063 | 0.001432 | 0.001372 |
| has_post_increment_test.test | RUN | 0.157503 | 0.871868 | 1.122314 | 0.001414 | 0.001475 | 0.001366 |
| has_post_decrement_test.test | RUN | 0.155080 | 0.891463 | 1.186291 | 0.001284 | 0.001472 | 0.001353 |
| has_plus_test.test | RUN | 0.154879 | 0.624119 | 1.109525 | 0.001317 | 0.001513 | 0.001277 |
| has_plus_assign_test.test | RUN | 0.159471 | 0.630639 | 1.116350 | 0.001175 | 0.001478 | 0.001411 |
| has_operator_new_test.test | RUN | 0.109043 | 0.322717 | 0.522902 | 0.001502 | 0.001478 | 0.001365 |
| has_nothrow_destructor_test.test | RUN | 0.112742 | 0.365739 | 0.554376 | 0.001369 | 0.001398 | 0.001364 |
| has_nothrow_copy_test.test | RUN | 0.108593 | 0.340331 | 0.567771 | 0.001204 | 0.001485 | 0.001328 |
| has_nothrow_constr_test.test | RUN | 0.106271 | 0.334809 | 0.542866 | 0.001166 | 0.001386 | 0.001326 |
| has_nothrow_assign_test.test | RUN | 0.107452 | 0.331957 | 0.529246 | 0.001379 | 0.001470 | 0.001291 |
| has_not_equal_to_test.test | RUN | 0.159894 | 0.869260 | 1.163947 | 0.001462 | 0.001599 | 0.001359 |
| has_negate_test.test | RUN | 0.153949 | 0.610341 | 1.050307 | 0.001308 | 0.001515 | 0.001508 |
| has_multiplies_test.test | RUN | 0.156126 | 0.619058 | 1.155551 | 0.001133 | 0.001488 | 0.001263 |
| has_multiplies_assign_test.test | RUN | 0.161441 | 0.619492 | 1.162896 | 0.001410 | 0.001517 | 0.001356 |
| has_modulus_test.test | RUN | 0.156950 | 0.632322 | 1.128297 | 0.001469 | 0.001581 | 0.001372 |
| has_modulus_assign_test.test | RUN | 0.158639 | 0.630960 | 1.102806 | 0.001336 | 0.001363 | 0.001479 |
| has_minus_test.test | RUN | 0.161294 | 0.646535 | 1.159948 | 0.001045 | 0.001564 | 0.001337 |
| has_minus_assign_test.test | RUN | 0.166374 | 0.631581 | 1.186671 | 0.001445 | 0.001545 | 0.001356 |
| has_logical_or_test.test | RUN | 0.159458 | 0.643725 | 1.126524 | 0.001489 | 0.001548 | 0.001321 |
| has_logical_not_test.test | RUN | 0.153636 | 0.628209 | 1.087400 | 0.001279 | 0.001415 | 0.001417 |
| has_logical_and_test.test | RUN | 0.161486 | 0.638540 | 1.176230 | 0.001176 | 0.001563 | 0.001355 |
| has_less_test.test | RUN | 0.162464 | 0.833926 | 1.141819 | 0.001367 | 0.001619 | 0.001152 |
| has_less_equal_test.test | RUN | 0.159992 | 0.844715 | 1.145709 | 0.001482 | 0.001265 | 0.001325 |
| has_left_shift_test.test | RUN | 0.172329 | 0.805852 | 1.369481 | 0.001120 | 0.001319 | 0.001340 |
| has_left_shift_assign_test.test | RUN | 0.159859 | 0.630866 | 1.148780 | 0.001329 | 0.001313 | 0.001149 |
| has_greater_test.test | RUN | 0.157143 | 0.854814 | 1.153706 | 0.001386 | 0.001572 | 0.001263 |
| has_greater_equal_test.test | RUN | 0.159734 | 0.852058 | 1.118771 | 0.001475 | 0.001379 | 0.001355 |
| has_equal_to_test.test | RUN | 0.158567 | 0.847698 | 1.177569 | 0.001344 | 0.001354 | 0.001356 |
| has_divides_test.test | RUN | 0.156971 | 0.646088 | 1.092468 | 0.001032 | 0.001349 | 0.001428 |
| has_divides_assign_test.test | RUN | 0.161121 | 0.611031 | 1.142367 | 0.001357 | 0.001531 | 0.001326 |
| has_dereference_test.test | RUN | 0.155388 | 0.590227 | 1.063610 | 0.001151 | 0.001655 | 0.001359 |
| has_complement_test.test | RUN | 0.156711 | 0.865835 | 1.138042 | 0.001152 | 0.001418 | 0.001249 |
| has_bit_xor_test.test | RUN | 0.154258 | 0.630264 | 1.141661 | 0.001337 | 0.001474 | 0.001272 |
| has_bit_xor_assign_test.test | RUN | 0.160814 | 0.624947 | 1.098210 | 0.001328 | 0.001468 | 0.001284 |
| has_bit_or_test.test | RUN | 0.158308 | 0.647243 | 1.103514 | 0.001462 | 0.001532 | 0.001337 |
| has_bit_or_assign_test.test | RUN | 0.158131 | 0.625192 | 1.149381 | 0.001286 | 0.001595 | 0.001450 |
| has_bit_and_test.test | RUN | 0.156513 | 0.627411 | 1.144006 | 0.001173 | 0.001174 | 0.001226 |
| has_bit_and_assign_test.test | RUN | 0.159294 | 0.618453 | 1.098136 | 0.001303 | 0.001434 | 0.001264 |
| has_binary_classes9_test.test | RUN | 0.172487 | 0.737353 | 1.236070 | 0.001437 | 0.001615 | 0.001354 |
| has_binary_classes8_test.test | RUN | 0.173254 | 0.753729 | 1.252574 | 0.001337 | 0.001510 | 0.001194 |
| has_binary_classes7_test.test | RUN | 0.173547 | 0.734840 | 1.231223 | 0.001168 | 0.001469 | 0.001206 |
| has_binary_classes6_test.test | RUN | 0.174498 | 0.731324 | 1.274416 | 0.001487 | 0.001480 | 0.001151 |
| has_binary_classes5_test.test | RUN | 0.170995 | 0.722600 | 1.252980 | 0.001462 | 0.001439 | 0.001338 |
| has_binary_classes4_test.test | RUN | 0.176903 | 0.769346 | 1.271228 | 0.001284 | 0.001563 | 0.000908 |
| has_binary_classes3_test.test | RUN | 0.170788 | 0.804062 | 1.212163 | 0.001362 | 0.001600 | 0.001414 |
| has_binary_classes2_test.test | RUN | 0.174518 | 0.768220 | 1.262493 | 0.001376 | 0.001393 | 0.001393 |
| has_binary_classes1_test.test | RUN | 0.173352 | 0.728014 | 1.259428 | 0.001289 | 0.001322 | 0.001409 |
| has_binary_classes0_test.test | RUN | 0.175101 | 0.747001 | 1.205145 | 0.001270 | 0.001396 | 0.001237 |
| function_traits_test.test | RUN | 0.084360 | 0.196254 | 0.279688 | 0.001276 | 0.001419 | 0.001223 |
| floating_point_promotion_test.test | RUN | 0.078966 | 0.200932 | 0.259929 | 0.001420 | 0.001384 | 0.001152 |
| extent_test.test | RUN | 0.081525 | 0.202923 | 0.278155 | 0.001246 | 0.001354 | 0.001071 |
| enable_if_test.test | RUN | 0.077437 | 0.175716 | 0.226010 | 0.001367 | 0.001279 | 0.001197 |
| disjunction_test.test | RUN | 0.078693 | 0.177768 | 0.226874 | 0.001248 | 0.001258 | 0.001173 |
| detected_test.test | RUN | 0.076475 | 0.171324 | 0.210041 | 0.001498 | 0.001512 | 0.001258 |
| detected_or_test.test | RUN | 0.077546 | 0.183419 | 0.216462 | 0.001364 | 0.001365 | 0.001228 |
| decay_test.test | RUN | 0.191456 | 0.656164 | 1.293780 | 0.001244 | 0.001517 | 0.001379 |
| cxx14_aliases_test.test | RUN | 0.125046 | 0.266529 | 0.349940 | 0.001160 | 0.001533 | 0.001195 |
| copy_reference_test.test | RUN | 0.079103 | 0.175774 | 0.231359 | 0.001381 | 0.001470 | 0.001476 |
| copy_cv_test.test | RUN | 0.080595 | 0.189255 | 0.254789 | 0.001239 | 0.001504 | 0.001327 |
| copy_cv_ref_test.test | RUN | 0.108723 | 0.298915 | 0.527221 | 0.001323 | 0.001433 | 0.001367 |
| conjunction_test.test | RUN | 0.077615 | 0.180669 | 0.233312 | 0.001379 | 0.001543 | 0.001409 |
| conditional_test.test | RUN | 0.077791 | 0.181698 | 0.230600 | 0.001390 | 0.001365 | 0.001312 |
| common_type_test.test | RUN | 0.098864 | 0.260610 | 0.435330 | 0.001322 | 0.001434 | 0.001422 |
| common_type_sfinae_test.test | RUN | 0.076685 | 0.169477 | 0.210608 | 0.001382 | 0.001265 | 0.001477 |
| common_type_sfinae2_test.test | RUN | 0.078941 | 0.182723 | 0.221625 | 0.001334 | 0.001432 | 0.001315 |
| common_type_6_test.test | RUN | 0.081147 | 0.181107 | 0.234580 | 0.001335 | 0.001324 | 0.001371 |
| common_type_5_test.test | RUN | 0.079314 | 0.181583 | 0.245049 | 0.001358 | 0.001414 | 0.001364 |
| common_type_4_test.test | RUN | 0.085298 | 0.194814 | 0.277019 | 0.001292 | 0.001350 | 0.001260 |
| common_type_3_test.test | RUN | 0.075566 | 0.171219 | 0.202082 | 0.001327 | 0.001410 | 0.001223 |
| common_type_2_test.test | RUN | 0.097296 | 0.260630 | 0.430333 | 0.001422 | 0.001500 | 0.001136 |
| alignment_of_test.test | RUN | 0.081220 | 0.211027 | 0.287295 | 0.001280 | 0.001449 | 0.001381 |
| alignment_of_a2_test.test | RUN | 0.082290 | 0.213299 | 0.285566 | 0.001450 | 0.001342 | 0.001172 |
| aligned_storage_test.test | RUN | 0.083865 | 0.214304 | 0.260189 | 0.001329 | 0.001595 | 0.001267 |
| aligned_storage_empy_test.test | RUN | 0.096747 | 0.236571 | 0.333843 | 0.001355 | 0.001448 | 0.001290 |
| aligned_storage_a2_test.test | RUN | 0.085320 | 0.206302 | 0.266483 | 0.001305 | 0.001693 | 0.001645 |
| add_volatile_test.test | RUN | 0.169700 | 0.580464 | 1.202802 | 0.001482 | 0.001556 | 0.001170 |
| add_rvalue_reference_test.test | RUN | 0.193345 | 0.666845 | 1.368219 | 0.001319 | 0.001308 | 0.001411 |
| add_reference_test.test | RUN | 0.186255 | 0.643962 | 1.365858 | 0.001357 | 0.001582 | 0.001348 |
| add_pointer_test.test | RUN | 0.123138 | 0.360526 | 0.692644 | 0.001302 | 0.001401 | 0.001442 |
| add_lvalue_reference_test.test | RUN | 0.187978 | 0.656219 | 1.346876 | 0.001367 | 0.001349 | 0.001201 |
| add_cv_test.test | RUN | 0.178372 | 0.603548 | 1.273822 | 0.001341 | 0.001455 | 0.001322 |
| add_const_test.test | RUN | 0.177867 | 0.611111 | 1.241796 | 0.001241 | 0.001316 | 0.001426 |
| is_virtual_base_of_fail.test | COMPILE_FAIL | 0.074834 | 0.164131 | 0.209031 | — | — | — |
| is_stateless_fail.test | COMPILE_FAIL | 0.076170 | 0.168209 | 0.216590 | — | — | — |
| is_polymorphic_fail.test | COMPILE_FAIL | 0.071246 | 0.163354 | 0.204917 | — | — | — |
| is_nothrow_move_constructible_fail.test | COMPILE_FAIL | 0.071503 | 0.161775 | 0.206569 | — | — | — |
| is_nothrow_move_assignable_fail.test | COMPILE_FAIL | 0.071627 | 0.164726 | 0.196242 | — | — | — |
| is_list_constructible_fail.test | COMPILE_FAIL | 0.071982 | 0.158667 | 0.199719 | — | — | — |
| is_empty_fail.test | COMPILE_FAIL | 0.073326 | 0.164348 | 0.199759 | — | — | — |
| is_destructible_fail.test | COMPILE_FAIL | 0.071440 | 0.160473 | 0.208241 | — | — | — |
| is_default_constructible_fail.test | COMPILE_FAIL | 0.072235 | 0.163623 | 0.203900 | — | — | — |
| is_copy_constructible_fail.test | COMPILE_FAIL | 0.072017 | 0.159785 | 0.200269 | — | — | — |
| is_copy_assignable_fail.test | COMPILE_FAIL | 0.071959 | 0.161208 | 0.204928 | — | — | — |
| is_convertible_fail.test | COMPILE_FAIL | 0.072347 | 0.154607 | 0.205162 | — | — | — |
| is_convertible2_fail.test | COMPILE_FAIL | 0.072130 | 0.160350 | 0.201294 | — | — | — |
| is_constructible_fail.test | COMPILE_FAIL | 0.072605 | 0.162637 | 0.205095 | — | — | — |
| is_base_of_fail.test | COMPILE_FAIL | 0.071886 | 0.163611 | 0.207788 | — | — | — |
| is_assignable_fail.test | COMPILE_FAIL | 0.071214 | 0.162185 | 0.205498 | — | — | — |
| has_nothrow_destruct_fail.test | COMPILE_FAIL | 0.071526 | 0.159797 | 0.197834 | — | — | — |
| has_nothrow_copy_fail.test | COMPILE_FAIL | 0.072558 | 0.162566 | 0.208627 | — | — | — |
| has_nothrow_cons_fail.test | COMPILE_FAIL | 0.069853 | 0.161187 | 0.206875 | — | — | — |
| has_nothrow_assign_fail.test | COMPILE_FAIL | 0.069909 | 0.132947 | 0.201204 | — | — | — |
| common_type_fail.test | COMPILE_FAIL | 0.073125 | 0.148956 | 0.203426 | — | — | — |
| common_type2_fail.test | COMPILE_FAIL | 0.072940 | 0.159789 | 0.204318 | — | — | — |
| has_nothrow_assign_test_no_intrinsics.test | RUN | 0.110231 | 0.333396 | 0.566834 | 0.001317 | 0.001397 | 0.001257 |
| has_nothrow_constr_test_no_intrinsics.test | RUN | 0.104292 | 0.332682 | 0.543209 | 0.001445 | 0.001553 | 0.001064 |
| has_nothrow_copy_test_no_intrinsics.test | RUN | 0.106874 | 0.337935 | 0.540530 | 0.001246 | 0.001428 | 0.001210 |
| is_nothrow_move_assignable_test_no_intrinsics.test | RUN | 0.112714 | 0.317701 | 0.548996 | 0.001064 | 0.001107 | 0.001042 |
| is_nothrow_move_constructible_test_no_intrinsics.test | RUN | 0.071584 | 0.220142 | 0.306357 | 0.001044 | 0.001061 | 0.000963 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.992× | 1.001×, 0.983× | 1.001× |
| clang | 1.027× | 1.026×, 1.028× | 1.001× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409407).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| add_const_test.test | 10 | 0.000839s | 0.000824s | 0.000822s | byte equality observed |
| add_cv_test.test | 10 | 0.000845s | 0.000818s | 0.000826s | byte equality observed |
| add_lvalue_reference_test.test | 10 | 0.000837s | 0.000837s | 0.000833s | byte equality observed |
| add_pointer_test.test | 10 | 0.000827s | 0.000830s | 0.000832s | byte equality observed |
| add_reference_test.test | 10 | 0.000834s | 0.000835s | 0.000848s | byte equality observed |
| add_rvalue_reference_test.test | 10 | 0.000830s | 0.000822s | 0.000828s | byte equality observed |
| add_volatile_test.test | 10 | 0.000836s | 0.000839s | 0.000822s | byte equality observed |
| aligned_storage_a2_test.test | 10 | 0.000912s | 0.000896s | 0.000932s | byte equality observed |
| aligned_storage_empy_test.test | 10 | 0.000827s | 0.000843s | 0.000819s | byte equality observed |
| aligned_storage_test.test | 10 | 0.000914s | 0.000918s | 0.000909s | byte equality observed |
| alignment_of_a2_test.test | 10 | 0.000823s | 0.000835s | 0.000813s | byte equality observed |
| alignment_of_test.test | 10 | 0.000839s | 0.000805s | 0.000840s | byte equality observed |
| common_type_2_test.test | 10 | 0.000852s | 0.000819s | 0.000816s | byte equality observed |
| common_type_3_test.test | 10 | 0.000839s | 0.000817s | 0.000842s | byte equality observed |
| common_type_4_test.test | 10 | 0.000836s | 0.000830s | 0.000838s | byte equality observed |
| common_type_5_test.test | 10 | 0.000831s | 0.000816s | 0.000815s | byte equality observed |
| common_type_6_test.test | 10 | 0.000835s | 0.000843s | 0.000823s | byte equality observed |
| common_type_sfinae2_test.test | 10 | 0.000829s | 0.000838s | 0.000833s | byte equality observed |
| common_type_sfinae_test.test | 10 | 0.000831s | 0.000829s | 0.000833s | byte equality observed |
| common_type_test.test | 10 | 0.000827s | 0.000824s | 0.000834s | byte equality observed |
| conditional_test.test | 10 | 0.000831s | 0.000837s | 0.000841s | byte equality observed |
| conjunction_test.test | 10 | 0.000834s | 0.000824s | 0.000842s | byte equality observed |
| copy_cv_ref_test.test | 10 | 0.000832s | 0.000827s | 0.000833s | byte equality observed |
| copy_cv_test.test | 10 | 0.000813s | 0.000818s | 0.000830s | byte equality observed |
| copy_reference_test.test | 10 | 0.000835s | 0.000845s | 0.000836s | byte equality observed |
| cxx14_aliases_test.test | 10 | 0.000842s | 0.000816s | 0.000832s | byte equality observed |
| decay_test.test | 10 | 0.000848s | 0.000821s | 0.000829s | byte equality observed |
| detected_or_test.test | 10 | 0.000826s | 0.000831s | 0.000860s | byte equality observed |
| detected_test.test | 10 | 0.000840s | 0.000815s | 0.000820s | byte equality observed |
| disjunction_test.test | 10 | 0.000847s | 0.000816s | 0.000831s | byte equality observed |
| enable_if_test.test | 10 | 0.000849s | 0.000841s | 0.000833s | byte equality observed |
| extent_test.test | 10 | 0.000828s | 0.000827s | 0.000839s | byte equality observed |
| floating_point_promotion_test.test | 10 | 0.000842s | 0.000828s | 0.000828s | byte equality observed |
| function_traits_test.test | 10 | 0.000837s | 0.000828s | 0.000837s | byte equality observed |
| has_binary_classes0_test.test | 10 | 0.000815s | 0.000817s | 0.000830s | byte equality observed |
| has_binary_classes1_test.test | 10 | 0.000845s | 0.000832s | 0.000828s | byte equality observed |
| has_binary_classes2_test.test | 10 | 0.000836s | 0.000831s | 0.000848s | byte equality observed |
| has_binary_classes3_test.test | 10 | 0.000838s | 0.000831s | 0.000830s | byte equality observed |
| has_binary_classes4_test.test | 10 | 0.000828s | 0.000821s | 0.000832s | byte equality observed |
| has_binary_classes5_test.test | 10 | 0.000841s | 0.000830s | 0.000845s | byte equality observed |
| has_binary_classes6_test.test | 10 | 0.000836s | 0.000822s | 0.000824s | byte equality observed |
| has_binary_classes7_test.test | 10 | 0.000828s | 0.000809s | 0.000837s | byte equality observed |
| has_binary_classes8_test.test | 10 | 0.000845s | 0.000813s | 0.000827s | byte equality observed |
| has_binary_classes9_test.test | 10 | 0.000834s | 0.000828s | 0.000829s | byte equality observed |
| has_bit_and_assign_test.test | 10 | 0.000826s | 0.000823s | 0.000831s | byte equality observed |
| has_bit_and_test.test | 10 | 0.000840s | 0.000840s | 0.000838s | byte equality observed |
| has_bit_or_assign_test.test | 10 | 0.000836s | 0.000813s | 0.000820s | byte equality observed |
| has_bit_or_test.test | 10 | 0.000827s | 0.000839s | 0.000814s | byte equality observed |
| has_bit_xor_assign_test.test | 10 | 0.000828s | 0.000853s | 0.000843s | byte equality observed |
| has_bit_xor_test.test | 10 | 0.000829s | 0.000831s | 0.000835s | byte equality observed |
| has_complement_test.test | 10 | 0.000822s | 0.000838s | 0.000846s | byte equality observed |
| has_dereference_test.test | 10 | 0.000829s | 0.000850s | 0.000839s | byte equality observed |
| has_divides_assign_test.test | 10 | 0.000821s | 0.000839s | 0.000839s | byte equality observed |
| has_divides_test.test | 10 | 0.000828s | 0.000839s | 0.000830s | byte equality observed |
| has_equal_to_test.test | 10 | 0.000833s | 0.000820s | 0.000843s | byte equality observed |
| has_greater_equal_test.test | 10 | 0.000819s | 0.000823s | 0.000834s | byte equality observed |
| has_greater_test.test | 10 | 0.000843s | 0.000821s | 0.000846s | byte equality observed |
| has_left_shift_assign_test.test | 10 | 0.000831s | 0.000824s | 0.000835s | byte equality observed |
| has_left_shift_test.test | 10 | 0.000838s | 0.000827s | 0.000841s | byte equality observed |
| has_less_equal_test.test | 10 | 0.000828s | 0.000822s | 0.000837s | byte equality observed |
| has_less_test.test | 10 | 0.000822s | 0.000835s | 0.000850s | byte equality observed |
| has_logical_and_test.test | 10 | 0.000833s | 0.000826s | 0.000812s | byte equality observed |
| has_logical_not_test.test | 10 | 0.000838s | 0.000815s | 0.000834s | byte equality observed |
| has_logical_or_test.test | 10 | 0.000842s | 0.000849s | 0.000834s | byte equality observed |
| has_minus_assign_test.test | 10 | 0.000833s | 0.000815s | 0.000845s | byte equality observed |
| has_minus_test.test | 10 | 0.000828s | 0.000832s | 0.000822s | byte equality observed |
| has_modulus_assign_test.test | 10 | 0.000825s | 0.000835s | 0.000810s | byte equality observed |
| has_modulus_test.test | 10 | 0.000830s | 0.000832s | 0.000837s | byte equality observed |
| has_multiplies_assign_test.test | 10 | 0.000839s | 0.000823s | 0.000835s | byte equality observed |
| has_multiplies_test.test | 10 | 0.000836s | 0.000811s | 0.000814s | byte equality observed |
| has_negate_test.test | 10 | 0.000832s | 0.000841s | 0.000845s | byte equality observed |
| has_not_equal_to_test.test | 10 | 0.000839s | 0.000828s | 0.000831s | byte equality observed |
| has_nothrow_assign_test.test | 10 | 0.000833s | 0.000825s | 0.000826s | byte equality observed |
| has_nothrow_assign_test_no_intrinsics.test | 10 | 0.000840s | 0.000819s | 0.000828s | byte equality observed |
| has_nothrow_constr_test.test | 10 | 0.000835s | 0.000834s | 0.000838s | byte equality observed |
| has_nothrow_constr_test_no_intrinsics.test | 10 | 0.000837s | 0.000836s | 0.000816s | byte equality observed |
| has_nothrow_copy_test.test | 10 | 0.000824s | 0.000816s | 0.000828s | byte equality observed |
| has_nothrow_copy_test_no_intrinsics.test | 10 | 0.000839s | 0.000829s | 0.000841s | byte equality observed |
| has_nothrow_destructor_test.test | 10 | 0.000824s | 0.000836s | 0.000845s | byte equality observed |
| has_operator_new_test.test | 10 | 0.000832s | 0.000834s | 0.000822s | byte equality observed |
| has_plus_assign_test.test | 10 | 0.000845s | 0.000824s | 0.000811s | byte equality observed |
| has_plus_test.test | 10 | 0.000839s | 0.000817s | 0.000847s | byte equality observed |
| has_post_decrement_test.test | 10 | 0.000827s | 0.000837s | 0.000822s | byte equality observed |
| has_post_increment_test.test | 10 | 0.000839s | 0.000822s | 0.000839s | byte equality observed |
| has_postfix_classes0_test.test | 10 | 0.000838s | 0.000845s | 0.000827s | byte equality observed |
| has_postfix_classes1_test.test | 10 | 0.000816s | 0.000838s | 0.000826s | byte equality observed |
| has_postfix_classes2_test.test | 10 | 0.000845s | 0.000830s | 0.000853s | byte equality observed |
| has_postfix_classes3_test.test | 10 | 0.000831s | 0.000839s | 0.000843s | byte equality observed |
| has_pre_decrement_test.test | 10 | 0.000849s | 0.000829s | 0.000822s | byte equality observed |
| has_pre_increment_test.test | 10 | 0.000824s | 0.000834s | 0.000841s | byte equality observed |
| has_prefix_classes0_test.test | 10 | 0.000835s | 0.000837s | 0.000837s | byte equality observed |
| has_prefix_classes1_test.test | 10 | 0.000834s | 0.000823s | 0.000827s | byte equality observed |
| has_prefix_classes2_test.test | 10 | 0.000836s | 0.000836s | 0.000823s | byte equality observed |
| has_prefix_classes3_test.test | 10 | 0.000846s | 0.000828s | 0.000829s | byte equality observed |
| has_right_shift_assign_test.test | 10 | 0.000828s | 0.000812s | 0.000837s | byte equality observed |
| has_right_shift_test.test | 10 | 0.000834s | 0.000837s | 0.000842s | byte equality observed |
| has_trivial_assign_test.test | 10 | 0.000881s | 0.000873s | 0.000870s | byte equality observed |
| has_trivial_constr_test.test | 10 | 0.000874s | 0.000885s | 0.000870s | native test oracle; output varies |
| has_trivial_copy_test.test | 10 | 0.000876s | 0.000880s | 0.000901s | native test oracle; output varies |
| has_trivial_destructor_test.test | 10 | 0.000842s | 0.000825s | 0.000836s | byte equality observed |
| has_trivial_move_assign_test.test | 10 | 0.000833s | 0.000830s | 0.000818s | byte equality observed |
| has_trivial_move_constructor_test.test | 10 | 0.000836s | 0.000829s | 0.000832s | byte equality observed |
| has_unary_minus_test.test | 10 | 0.000837s | 0.000828s | 0.000844s | byte equality observed |
| has_unary_plus_test.test | 10 | 0.000829s | 0.000822s | 0.000836s | byte equality observed |
| has_virtual_destructor_test.test | 10 | 0.000827s | 0.000840s | 0.000849s | byte equality observed |
| integral_constant_test.test | 10 | 0.000387s | 0.000392s | 0.000829s | byte equality observed |
| is_abstract_test.test | 10 | 0.000842s | 0.000820s | 0.000839s | byte equality observed |
| is_arithmetic_test.test | 10 | 0.000822s | 0.000820s | 0.000822s | byte equality observed |
| is_array_test.test | 10 | 0.000836s | 0.000822s | 0.000845s | byte equality observed |
| is_assignable_test.test | 10 | 0.000830s | 0.000830s | 0.000826s | byte equality observed |
| is_base_and_derived_test.test | 10 | 0.000829s | 0.000834s | 0.000827s | byte equality observed |
| is_base_of_test.test | 10 | 0.000837s | 0.000834s | 0.000825s | byte equality observed |
| is_bounded_array_test.test | 10 | 0.000834s | 0.000830s | 0.000825s | byte equality observed |
| is_class_test.test | 10 | 0.000835s | 0.000831s | 0.000833s | byte equality observed |
| is_complete_test.test | 10 | 0.000824s | 0.000827s | 0.000844s | byte equality observed |
| is_complex_test.test | 10 | 0.000824s | 0.000819s | 0.000855s | byte equality observed |
| is_compound_test.test | 10 | 0.000824s | 0.000836s | 0.000826s | byte equality observed |
| is_const_test.test | 10 | 0.000837s | 0.000835s | 0.000846s | byte equality observed |
| is_constructible_test.test | 10 | 0.000842s | 0.000828s | 0.000847s | byte equality observed |
| is_convertible_test.test | 10 | 0.000830s | 0.000834s | 0.000832s | byte equality observed |
| is_copy_assignable_test.test | 10 | 0.000848s | 0.000842s | 0.000840s | byte equality observed |
| is_copy_constructible_test.test | 10 | 0.000834s | 0.000828s | 0.000832s | byte equality observed |
| is_default_constr_test.test | 10 | 0.000841s | 0.000829s | 0.000829s | byte equality observed |
| is_destructible_test.test | 10 | 0.000835s | 0.000831s | 0.000836s | byte equality observed |
| is_detected_exact_test.test | 10 | 0.000831s | 0.000822s | 0.000837s | byte equality observed |
| is_detected_test.test | 10 | 0.000841s | 0.000826s | 0.000839s | byte equality observed |
| is_empty_test.test | 10 | 0.000825s | 0.000822s | 0.000845s | byte equality observed |
| is_enum_test.test | 10 | 0.000848s | 0.000833s | 0.000839s | byte equality observed |
| is_final_test.test | 10 | 0.000836s | 0.000829s | 0.000834s | byte equality observed |
| is_float_test.test | 10 | 0.000840s | 0.000841s | 0.000852s | byte equality observed |
| is_floating_point_test.test | 10 | 0.000855s | 0.000820s | 0.000841s | byte equality observed |
| is_function_test.test | 10 | 0.000812s | 0.000845s | 0.000842s | byte equality observed |
| is_fundamental_test.test | 10 | 0.000832s | 0.000828s | 0.000834s | byte equality observed |
| is_integral_test.test | 10 | 0.000832s | 0.000834s | 0.000823s | byte equality observed |
| is_list_constructible_test.test | 10 | 0.000844s | 0.000832s | 0.000827s | byte equality observed |
| is_lvalue_reference_test.test | 10 | 0.000816s | 0.000847s | 0.000837s | byte equality observed |
| is_member_func_test.test | 10 | 0.000826s | 0.000831s | 0.000825s | byte equality observed |
| is_member_obj_test.test | 10 | 0.000825s | 0.000827s | 0.000818s | byte equality observed |
| is_member_pointer_test.test | 10 | 0.000844s | 0.000834s | 0.000847s | byte equality observed |
| is_noncopyable_test.test | 10 | 0.000824s | 0.000827s | 0.000827s | byte equality observed |
| is_nothrow_move_assignable_test.test | 10 | 0.000824s | 0.000814s | 0.000837s | byte equality observed |
| is_nothrow_move_assignable_test_no_intrinsics.test | 10 | 0.000842s | 0.000829s | 0.000828s | byte equality observed |
| is_nothrow_move_constructible_test.test | 10 | 0.000824s | 0.000835s | 0.000830s | byte equality observed |
| is_nothrow_move_constructible_test_no_intrinsics.test | 10 | 0.000848s | 0.000832s | 0.000829s | byte equality observed |
| is_nothrow_swappable_test.test | 10 | 0.000827s | 0.000848s | 0.000846s | byte equality observed |
| is_object_test.test | 10 | 0.000832s | 0.000835s | 0.000845s | byte equality observed |
| is_pod_test.test | 10 | 0.000849s | 0.000840s | 0.000825s | byte equality observed |
| is_pointer_test.test | 10 | 0.000829s | 0.000823s | 0.000839s | byte equality observed |
| is_polymorphic_test.test | 10 | 0.000841s | 0.000845s | 0.000835s | byte equality observed |
| is_reference_test.test | 10 | 0.000835s | 0.000836s | 0.000841s | byte equality observed |
| is_rvalue_reference_test.test | 10 | 0.000833s | 0.000850s | 0.000840s | byte equality observed |
| is_same_test.test | 10 | 0.000823s | 0.000830s | 0.000838s | byte equality observed |
| is_scalar_test.test | 10 | 0.000835s | 0.000838s | 0.000824s | byte equality observed |
| is_scoped_enum_test.test | 10 | 0.000826s | 0.000834s | 0.000835s | byte equality observed |
| is_signed_test.test | 10 | 0.000841s | 0.000840s | 0.000842s | byte equality observed |
| is_stateless_test.test | 10 | 0.000834s | 0.000852s | 0.000826s | byte equality observed |
| is_swappable_test.test | 10 | 0.000832s | 0.000833s | 0.000843s | byte equality observed |
| is_trivially_copyable_test.test | 10 | 0.000849s | 0.000848s | 0.000863s | byte equality observed |
| is_unbounded_array_test.test | 10 | 0.000844s | 0.000838s | 0.000836s | byte equality observed |
| is_union_test.test | 10 | 0.000832s | 0.000850s | 0.000840s | byte equality observed |
| is_unscoped_enum_test.test | 10 | 0.000831s | 0.000835s | 0.000822s | byte equality observed |
| is_unsigned_test.test | 10 | 0.000828s | 0.000830s | 0.000823s | byte equality observed |
| is_virtual_base_of_test.test | 10 | 0.000832s | 0.000849s | 0.000843s | byte equality observed |
| is_void_test.test | 10 | 0.000825s | 0.000827s | 0.000837s | byte equality observed |
| is_volatile_test.test | 10 | 0.000838s | 0.000844s | 0.000830s | byte equality observed |
| make_signed_test.test | 10 | 0.000829s | 0.000851s | 0.000829s | byte equality observed |
| make_unsigned_test.test | 10 | 0.000842s | 0.000813s | 0.000828s | byte equality observed |
| make_void_test.test | 10 | 0.000832s | 0.000835s | 0.000841s | byte equality observed |
| mpl_interop_test1.test | 10 | 0.000392s | 0.000392s | 0.000822s | byte equality observed |
| mpl_interop_test2.test | 10 | 0.000390s | 0.000400s | 0.000836s | byte equality observed |
| mpl_interop_test3.test | 10 | 0.000393s | 0.000394s | 0.000836s | byte equality observed |
| negation_test.test | 10 | 0.000857s | 0.000859s | 0.000827s | byte equality observed |
| promote_basic_test.test | 10 | 0.000394s | 0.000389s | 0.000831s | byte equality observed |
| promote_enum_msvc_bug_test.test | 10 | 0.000394s | 0.000400s | 0.000836s | byte equality observed |
| promote_enum_test.test | 10 | 0.000389s | 0.000399s | 0.000824s | byte equality observed |
| promote_mpl_test.test | 10 | 0.000385s | 0.000393s | 0.000831s | byte equality observed |
| rank_test.test | 10 | 0.000834s | 0.000857s | 0.000814s | byte equality observed |
| remove_all_extents_test.test | 10 | 0.000845s | 0.000844s | 0.000826s | byte equality observed |
| remove_bounds_test.test | 10 | 0.000832s | 0.000868s | 0.000830s | byte equality observed |
| remove_const_test.test | 10 | 0.000837s | 0.000837s | 0.000834s | byte equality observed |
| remove_cv_ref_test.test | 10 | 0.000828s | 0.000827s | 0.000821s | byte equality observed |
| remove_cv_test.test | 10 | 0.000842s | 0.000843s | 0.000838s | byte equality observed |
| remove_extent_test.test | 10 | 0.000853s | 0.000839s | 0.000846s | byte equality observed |
| remove_pointer_test.test | 10 | 0.000837s | 0.000825s | 0.000825s | byte equality observed |
| remove_reference_test.test | 10 | 0.000836s | 0.000849s | 0.000823s | byte equality observed |
| remove_volatile_test.test | 10 | 0.000835s | 0.000843s | 0.000813s | byte equality observed |
| tricky_abstract_type_test.test | 10 | 0.000844s | 0.000842s | 0.000854s | byte equality observed |
| tricky_add_pointer_test.test | 10 | 0.000836s | 0.000846s | 0.000840s | byte equality observed |
| tricky_function_type_test.test | 10 | 0.000847s | 0.000840s | 0.000840s | byte equality observed |
| tricky_incomplete_type_test.test | 10 | 0.000824s | 0.000851s | 0.000834s | byte equality observed |
| tricky_is_enum_test.test | 10 | 0.000830s | 0.000840s | 0.000830s | byte equality observed |
| tricky_partial_spec_test.test | 10 | 0.000842s | 0.000818s | 0.000842s | byte equality observed |
| tricky_rvalue_test.test | 10 | 0.000839s | 0.000836s | 0.000834s | byte equality observed |
| type_identity_test.test | 10 | 0.000856s | 0.000842s | 0.000850s | byte equality observed |
| type_traits_test.test | 10 | 0.000392s | 0.000389s | 0.000829s | byte equality observed |
| type_with_alignment_test.test | 10 | 0.000932s | 0.000933s | 0.000939s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409407).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
