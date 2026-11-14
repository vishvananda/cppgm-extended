# libs/type_traits/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| type_with_alignment_test.test | RUN | 0.086505 | 0.229260 | 0.306535 | 0.001370 | 0.001448 | 0.001146 |
| type_traits_test.test | RUN | 0.117481 | 0.241256 | 0.303240 | 0.000470 | 0.000498 | 0.001134 |
| type_identity_test.test | RUN | 0.211256 | 0.783528 | 1.609226 | 0.001223 | 0.001569 | 0.001372 |
| tricky_rvalue_test.test | RUN | 0.078329 | 0.184326 | 0.242203 | 0.001408 | 0.001437 | 0.001754 |
| tricky_partial_spec_test.test | RUN | 0.093702 | 0.261050 | 0.397697 | 0.001395 | 0.001366 | 0.001315 |
| tricky_is_enum_test.test | RUN | 0.076042 | 0.175836 | 0.219309 | 0.001362 | 0.001458 | 0.001439 |
| tricky_incomplete_type_test.test | RUN | 0.126380 | 0.272187 | 0.339877 | 0.001289 | 0.001356 | 0.001346 |
| tricky_function_type_test.test | RUN | 0.102136 | 0.271776 | 0.423372 | 0.001482 | 0.001179 | 0.001304 |
| tricky_add_pointer_test.test | RUN | 0.144094 | 0.477536 | 0.922088 | 0.001407 | 0.001508 | 0.001348 |
| tricky_abstract_type_test.test | RUN | 0.080262 | 0.182352 | 0.235841 | 0.001428 | 0.001456 | 0.001501 |
| remove_volatile_test.test | RUN | 0.191674 | 0.692100 | 1.416224 | 0.001519 | 0.001621 | 0.001240 |
| remove_reference_test.test | RUN | 0.200071 | 0.721360 | 1.463708 | 0.001459 | 0.001412 | 0.001388 |
| remove_pointer_test.test | RUN | 0.207514 | 0.745214 | 1.574947 | 0.001499 | 0.001308 | 0.001358 |
| remove_extent_test.test | RUN | 0.200235 | 0.734633 | 1.472335 | 0.001384 | 0.001547 | 0.001360 |
| remove_cv_test.test | RUN | 0.201241 | 0.722629 | 1.492512 | 0.001489 | 0.001595 | 0.001389 |
| remove_cv_ref_test.test | RUN | 0.411558 | 1.730990 | 3.736552 | 0.001516 | 0.001416 | 0.001305 |
| remove_const_test.test | RUN | 0.192492 | 0.661070 | 1.418780 | 0.001434 | 0.001529 | 0.001387 |
| remove_bounds_test.test | RUN | 0.200960 | 0.725822 | 1.465486 | 0.001486 | 0.001415 | 0.001297 |
| remove_all_extents_test.test | RUN | 0.203451 | 0.713948 | 1.511936 | 0.001366 | 0.001402 | 0.001147 |
| rank_test.test | RUN | 0.080770 | 0.177263 | 0.227924 | 0.001342 | 0.001390 | 0.001351 |
| promote_mpl_test.test | RUN | 0.066530 | 0.167400 | 0.168607 | 0.000527 | 0.000537 | 0.001314 |
| promote_enum_test.test | RUN | 0.016276 | 0.043336 | 0.047170 | 0.000510 | 0.000636 | 0.001176 |
| promote_enum_msvc_bug_test.test | RUN | 0.015113 | 0.038406 | 0.039449 | 0.000545 | 0.000512 | 0.001387 |
| promote_basic_test.test | RUN | 0.019105 | 0.049988 | 0.046472 | 0.000520 | 0.000702 | 0.001120 |
| negation_test.test | RUN | 0.073417 | 0.174467 | 0.209465 | 0.001338 | 0.001256 | 0.001153 |
| mpl_interop_test3.test | RUN | 0.062524 | 0.156846 | 0.165948 | 0.000548 | 0.000518 | 0.001421 |
| mpl_interop_test2.test | RUN | 0.023976 | 0.045424 | 0.050636 | 0.000515 | 0.000515 | 0.001323 |
| mpl_interop_test1.test | RUN | 0.014357 | 0.036060 | 0.035258 | 0.000604 | 0.000664 | 0.001265 |
| make_void_test.test | RUN | 0.079805 | 0.184055 | 0.236692 | 0.001533 | 0.001370 | 0.001427 |
| make_unsigned_test.test | RUN | 0.094004 | 0.237015 | 0.329028 | 0.001382 | 0.001338 | 0.001299 |
| make_signed_test.test | RUN | 0.094019 | 0.233639 | 0.328521 | 0.001217 | 0.001313 | 0.001170 |
| is_volatile_test.test | RUN | 0.086272 | 0.205976 | 0.280956 | 0.001210 | 0.001548 | 0.001415 |
| is_void_test.test | RUN | 0.080575 | 0.194264 | 0.267781 | 0.001531 | 0.001360 | 0.001253 |
| is_virtual_base_of_test.test | RUN | 0.093567 | 0.242131 | 0.359605 | 0.001329 | 0.001274 | 0.001386 |
| is_unsigned_test.test | RUN | 0.083773 | 0.196771 | 0.259430 | 0.001334 | 0.001348 | 0.001408 |
| is_unscoped_enum_test.test | RUN | 0.083595 | 0.183714 | 0.234259 | 0.001198 | 0.001382 | 0.001187 |
| is_union_test.test | RUN | 0.086940 | 0.224309 | 0.332171 | 0.001409 | 0.001595 | 0.001220 |
| is_unbounded_array_test.test | RUN | 0.082198 | 0.189725 | 0.265046 | 0.001462 | 0.001404 | 0.001175 |
| is_trivially_copyable_test.test | RUN | 0.127879 | 0.382571 | 0.637887 | 0.001549 | 0.001323 | 0.001164 |
| is_swappable_test.test | RUN | 0.104438 | 0.295872 | 0.489544 | 0.001360 | 0.001505 | 0.001308 |
| is_stateless_test.test | RUN | 0.117241 | 0.335372 | 0.527569 | 0.001232 | 0.001450 | 0.001282 |
| is_signed_test.test | RUN | 0.082865 | 0.196819 | 0.276556 | 0.001483 | 0.001362 | 0.001399 |
| is_scoped_enum_test.test | RUN | 0.080825 | 0.184174 | 0.232994 | 0.001311 | 0.001568 | 0.001468 |
| is_scalar_test.test | RUN | 0.105779 | 0.305881 | 0.467657 | 0.001297 | 0.001596 | 0.001519 |
| is_same_test.test | RUN | 0.077273 | 0.193840 | 0.250290 | 0.001174 | 0.001357 | 0.001381 |
| is_rvalue_reference_test.test | RUN | 0.080669 | 0.212880 | 0.282342 | 0.001471 | 0.001294 | 0.001217 |
| is_reference_test.test | RUN | 0.085406 | 0.213575 | 0.286510 | 0.001334 | 0.001457 | 0.001322 |
| is_polymorphic_test.test | RUN | 0.088982 | 0.224494 | 0.302439 | 0.001474 | 0.001537 | 0.001420 |
| is_pointer_test.test | RUN | 0.083585 | 0.220758 | 0.321697 | 0.001502 | 0.001362 | 0.001488 |
| is_pod_test.test | RUN | 0.108779 | 0.344456 | 0.545581 | 0.001210 | 0.001404 | 0.001380 |
| is_object_test.test | RUN | 0.079991 | 0.183549 | 0.233186 | 0.001229 | 0.001592 | 0.001345 |
| is_nothrow_swappable_test.test | RUN | 0.112666 | 0.322844 | 0.526380 | 0.001530 | 0.001518 | 0.001420 |
| is_nothrow_move_constructible_test.test | RUN | 0.109371 | 0.326630 | 0.519127 | 0.001386 | 0.001326 | 0.001496 |
| is_nothrow_move_assignable_test.test | RUN | 0.109698 | 0.335762 | 0.525386 | 0.001271 | 0.001498 | 0.001266 |
| is_noncopyable_test.test | RUN | 0.080045 | 0.186461 | 0.245271 | 0.001297 | 0.001304 | 0.001242 |
| is_member_pointer_test.test | RUN | 0.082375 | 0.188170 | 0.251746 | 0.001654 | 0.001557 | 0.001390 |
| is_member_obj_test.test | RUN | 0.085507 | 0.192800 | 0.242930 | 0.001472 | 0.001270 | 0.001391 |
| is_member_func_test.test | RUN | 0.086242 | 0.205676 | 0.281877 | 0.001262 | 0.001373 | 0.001232 |
| is_lvalue_reference_test.test | RUN | 0.086453 | 0.210009 | 0.309236 | 0.001263 | 0.001462 | 0.001312 |
| is_list_constructible_test.test | RUN | 0.098386 | 0.250586 | 0.319388 | 0.001474 | 0.001437 | 0.001367 |
| is_integral_test.test | RUN | 0.098680 | 0.296179 | 0.456020 | 0.001415 | 0.001329 | 0.001390 |
| is_fundamental_test.test | RUN | 0.099841 | 0.296329 | 0.472683 | 0.001041 | 0.001440 | 0.001335 |
| is_function_test.test | RUN | 0.081041 | 0.202298 | 0.279957 | 0.001280 | 0.001194 | 0.001295 |
| is_floating_point_test.test | RUN | 0.085989 | 0.217986 | 0.300460 | 0.001516 | 0.001342 | 0.001316 |
| is_float_test.test | RUN | 0.084560 | 0.218747 | 0.299256 | 0.001405 | 0.001290 | 0.001365 |
| is_final_test.test | RUN | 0.085005 | 0.216211 | 0.319800 | 0.001043 | 0.001288 | 0.001222 |
| is_enum_test.test | RUN | 0.078025 | 0.185275 | 0.250664 | 0.001397 | 0.001414 | 0.001202 |
| is_empty_test.test | RUN | 0.084686 | 0.203065 | 0.270176 | 0.001581 | 0.001587 | 0.001413 |
| is_detected_test.test | RUN | 0.076927 | 0.173164 | 0.226597 | 0.001403 | 0.001258 | 0.001242 |
| is_detected_exact_test.test | RUN | 0.076756 | 0.176185 | 0.220898 | 0.001168 | 0.001484 | 0.001201 |
| is_destructible_test.test | RUN | 0.106071 | 0.327128 | 0.541164 | 0.001280 | 0.001512 | 0.001237 |
| is_default_constr_test.test | RUN | 0.109457 | 0.330363 | 0.537569 | 0.001504 | 0.001529 | 0.001445 |
| is_copy_constructible_test.test | RUN | 0.110665 | 0.364197 | 0.580194 | 0.001419 | 0.001493 | 0.001428 |
| is_copy_assignable_test.test | RUN | 0.111206 | 0.351291 | 0.599356 | 0.001090 | 0.001557 | 0.001346 |
| is_convertible_test.test | RUN | 0.106366 | 0.303398 | 0.465910 | 0.001261 | 0.001576 | 0.001322 |
| is_constructible_test.test | RUN | 0.092769 | 0.233648 | 0.312720 | 0.001506 | 0.001439 | 0.001371 |
| is_const_test.test | RUN | 0.081619 | 0.197880 | 0.273034 | 0.001492 | 0.001390 | 0.001466 |
| is_compound_test.test | RUN | 0.079674 | 0.179045 | 0.218545 | 0.001202 | 0.001406 | 0.001281 |
| is_complex_test.test | RUN | 0.104100 | 0.232885 | 0.301101 | 0.001362 | 0.001555 | 0.001426 |
| is_complete_test.test | RUN | 0.093310 | 0.240733 | 0.350781 | 0.001315 | 0.001447 | 0.001270 |
| is_class_test.test | RUN | 0.082129 | 0.213683 | 0.292026 | 0.001336 | 0.001405 | 0.001281 |
| is_bounded_array_test.test | RUN | 0.078965 | 0.190922 | 0.242438 | 0.001478 | 0.001302 | 0.001322 |
| is_base_of_test.test | RUN | 0.086241 | 0.213498 | 0.289309 | 0.001382 | 0.001580 | 0.001301 |
| is_base_and_derived_test.test | RUN | 0.086013 | 0.206810 | 0.287912 | 0.001457 | 0.001403 | 0.001071 |
| is_assignable_test.test | RUN | 0.099264 | 0.289070 | 0.436998 | 0.001327 | 0.001383 | 0.001346 |
| is_array_test.test | RUN | 0.081970 | 0.200702 | 0.269197 | 0.001401 | 0.001308 | 0.001406 |
| is_arithmetic_test.test | RUN | 0.102000 | 0.291187 | 0.454170 | 0.001410 | 0.001492 | 0.001374 |
| is_abstract_test.test | RUN | 0.180333 | 0.818518 | 1.476375 | 0.001558 | 0.001550 | 0.001270 |
| integral_constant_test.test | RUN | 0.014358 | 0.036751 | 0.036709 | 0.000670 | 0.000608 | 0.001289 |
| has_virtual_destructor_test.test | RUN | 0.093288 | 0.231460 | 0.318097 | 0.001308 | 0.001610 | 0.001408 |
| has_unary_plus_test.test | RUN | 0.168696 | 0.861469 | 1.164396 | 0.001398 | 0.001455 | 0.001291 |
| has_unary_minus_test.test | RUN | 0.163788 | 0.859487 | 1.123864 | 0.001460 | 0.001534 | 0.001406 |
| has_trivial_move_constructor_test.test | RUN | 0.113065 | 0.355256 | 0.557667 | 0.001332 | 0.001395 | 0.001402 |
| has_trivial_move_assign_test.test | RUN | 0.109743 | 0.334300 | 0.520387 | 0.001239 | 0.001398 | 0.001384 |
| has_trivial_destructor_test.test | RUN | 0.113443 | 0.347550 | 0.566668 | 0.001442 | 0.001486 | 0.001314 |
| has_trivial_copy_test.test | RUN | 0.115254 | 0.361630 | 0.589719 | 0.001587 | 0.001541 | 0.001307 |
| has_trivial_constr_test.test | RUN | 0.116267 | 0.373058 | 0.565890 | 0.001444 | 0.001496 | 0.001498 |
| has_trivial_assign_test.test | RUN | 0.112603 | 0.362932 | 0.566178 | 0.001309 | 0.001508 | 0.001401 |
| has_right_shift_test.test | RUN | 0.171252 | 0.674999 | 1.251945 | 0.001438 | 0.001433 | 0.001414 |
| has_right_shift_assign_test.test | RUN | 0.163133 | 0.630109 | 1.164110 | 0.001211 | 0.001404 | 0.001422 |
| has_prefix_classes3_test.test | RUN | 0.122516 | 0.373125 | 0.584403 | 0.001351 | 0.001292 | 0.001368 |
| has_prefix_classes2_test.test | RUN | 0.121632 | 0.374507 | 0.566308 | 0.001151 | 0.001518 | 0.001329 |
| has_prefix_classes1_test.test | RUN | 0.121964 | 0.370918 | 0.559952 | 0.001400 | 0.001404 | 0.001387 |
| has_prefix_classes0_test.test | RUN | 0.120871 | 0.371062 | 0.601118 | 0.001522 | 0.001388 | 0.001375 |
| has_pre_increment_test.test | RUN | 0.159812 | 0.873463 | 1.212015 | 0.001346 | 0.001414 | 0.001492 |
| has_pre_decrement_test.test | RUN | 0.157595 | 0.872129 | 1.176998 | 0.001154 | 0.001323 | 0.001179 |
| has_postfix_classes3_test.test | RUN | 0.126015 | 0.360718 | 0.669143 | 0.001521 | 0.001468 | 0.001340 |
| has_postfix_classes2_test.test | RUN | 0.125579 | 0.360018 | 0.604680 | 0.001514 | 0.001336 | 0.001387 |
| has_postfix_classes1_test.test | RUN | 0.122757 | 0.368333 | 0.579281 | 0.001337 | 0.001390 | 0.001441 |
| has_postfix_classes0_test.test | RUN | 0.121430 | 0.368625 | 0.612587 | 0.001354 | 0.001262 | 0.001424 |
| has_post_increment_test.test | RUN | 0.163686 | 0.916322 | 1.134550 | 0.001503 | 0.001426 | 0.001339 |
| has_post_decrement_test.test | RUN | 0.162552 | 0.913952 | 1.110688 | 0.001245 | 0.001284 | 0.001381 |
| has_plus_test.test | RUN | 0.163066 | 0.645451 | 1.124738 | 0.001302 | 0.001401 | 0.001459 |
| has_plus_assign_test.test | RUN | 0.167683 | 0.643631 | 1.186472 | 0.001406 | 0.001433 | 0.001503 |
| has_operator_new_test.test | RUN | 0.113549 | 0.325948 | 0.537589 | 0.001329 | 0.001430 | 0.001318 |
| has_nothrow_destructor_test.test | RUN | 0.117823 | 0.362204 | 0.561569 | 0.001357 | 0.001248 | 0.001337 |
| has_nothrow_copy_test.test | RUN | 0.113158 | 0.349512 | 0.559969 | 0.001328 | 0.001034 | 0.001410 |
| has_nothrow_constr_test.test | RUN | 0.111484 | 0.337010 | 0.567654 | 0.001379 | 0.001470 | 0.001505 |
| has_nothrow_assign_test.test | RUN | 0.110992 | 0.338182 | 0.560215 | 0.001553 | 0.001475 | 0.001395 |
| has_not_equal_to_test.test | RUN | 0.166798 | 0.880401 | 1.169277 | 0.001491 | 0.001579 | 0.001421 |
| has_negate_test.test | RUN | 0.158653 | 0.610837 | 1.055958 | 0.001269 | 0.001321 | 0.001194 |
| has_multiplies_test.test | RUN | 0.161958 | 0.626587 | 1.154680 | 0.001283 | 0.001298 | 0.001287 |
| has_multiplies_assign_test.test | RUN | 0.167861 | 0.627926 | 1.148254 | 0.001585 | 0.001477 | 0.001367 |
| has_modulus_test.test | RUN | 0.166329 | 0.634463 | 1.124138 | 0.001240 | 0.001521 | 0.001400 |
| has_modulus_assign_test.test | RUN | 0.160671 | 0.629085 | 1.104604 | 0.001461 | 0.001151 | 0.001410 |
| has_minus_test.test | RUN | 0.166840 | 0.659651 | 1.139023 | 0.001232 | 0.001578 | 0.001417 |
| has_minus_assign_test.test | RUN | 0.172458 | 0.628538 | 1.207640 | 0.001509 | 0.001547 | 0.001378 |
| has_logical_or_test.test | RUN | 0.163392 | 0.623875 | 1.157257 | 0.001438 | 0.001269 | 0.001388 |
| has_logical_not_test.test | RUN | 0.157838 | 0.611603 | 1.085406 | 0.001150 | 0.001419 | 0.001438 |
| has_logical_and_test.test | RUN | 0.163926 | 0.653771 | 1.130457 | 0.001394 | 0.001577 | 0.001385 |
| has_less_test.test | RUN | 0.167789 | 0.846210 | 1.117256 | 0.001606 | 0.001584 | 0.001389 |
| has_less_equal_test.test | RUN | 0.163310 | 0.838293 | 1.109485 | 0.001463 | 0.001275 | 0.001339 |
| has_left_shift_test.test | RUN | 0.176986 | 0.801168 | 1.455216 | 0.001389 | 0.001466 | 0.001300 |
| has_left_shift_assign_test.test | RUN | 0.166163 | 0.633797 | 1.135375 | 0.001321 | 0.001267 | 0.001351 |
| has_greater_test.test | RUN | 0.166665 | 0.859240 | 1.123450 | 0.001529 | 0.001484 | 0.001389 |
| has_greater_equal_test.test | RUN | 0.164412 | 0.831317 | 1.133250 | 0.001485 | 0.001289 | 0.001340 |
| has_equal_to_test.test | RUN | 0.165691 | 0.840891 | 1.154772 | 0.001349 | 0.001292 | 0.001334 |
| has_divides_test.test | RUN | 0.163940 | 0.646308 | 1.146525 | 0.001271 | 0.001573 | 0.001139 |
| has_divides_assign_test.test | RUN | 0.167978 | 0.630298 | 1.121223 | 0.001521 | 0.001481 | 0.001182 |
| has_dereference_test.test | RUN | 0.160233 | 0.602912 | 1.112346 | 0.001411 | 0.001569 | 0.001366 |
| has_complement_test.test | RUN | 0.160633 | 0.865035 | 1.099871 | 0.001292 | 0.001579 | 0.001372 |
| has_bit_xor_test.test | RUN | 0.162045 | 0.649247 | 1.169179 | 0.001426 | 0.001430 | 0.001370 |
| has_bit_xor_assign_test.test | RUN | 0.164153 | 0.639011 | 1.137857 | 0.001522 | 0.001516 | 0.001260 |
| has_bit_or_test.test | RUN | 0.161872 | 0.628150 | 1.103159 | 0.001292 | 0.001547 | 0.001378 |
| has_bit_or_assign_test.test | RUN | 0.161450 | 0.631323 | 1.153090 | 0.001140 | 0.001536 | 0.001418 |
| has_bit_and_test.test | RUN | 0.161597 | 0.652019 | 1.142743 | 0.001254 | 0.001569 | 0.001219 |
| has_bit_and_assign_test.test | RUN | 0.163322 | 0.632291 | 1.106605 | 0.001728 | 0.001575 | 0.001334 |
| has_binary_classes9_test.test | RUN | 0.178799 | 0.720871 | 1.243858 | 0.001416 | 0.001545 | 0.001370 |
| has_binary_classes8_test.test | RUN | 0.179787 | 0.774849 | 1.274758 | 0.001616 | 0.001786 | 0.001346 |
| has_binary_classes7_test.test | RUN | 0.176029 | 0.753324 | 1.236623 | 0.001189 | 0.001584 | 0.001098 |
| has_binary_classes6_test.test | RUN | 0.181285 | 0.706083 | 1.292642 | 0.001442 | 0.001570 | 0.001247 |
| has_binary_classes5_test.test | RUN | 0.175156 | 0.733160 | 1.291987 | 0.001435 | 0.001574 | 0.001345 |
| has_binary_classes4_test.test | RUN | 0.174593 | 0.745650 | 1.224023 | 0.001206 | 0.001530 | 0.001468 |
| has_binary_classes3_test.test | RUN | 0.176849 | 0.754835 | 1.289577 | 0.001297 | 0.001510 | 0.001320 |
| has_binary_classes2_test.test | RUN | 0.183200 | 0.741498 | 1.298592 | 0.001550 | 0.001571 | 0.001430 |
| has_binary_classes1_test.test | RUN | 0.176615 | 0.730998 | 1.250840 | 0.001387 | 0.001458 | 0.001387 |
| has_binary_classes0_test.test | RUN | 0.177242 | 0.746233 | 1.221120 | 0.001297 | 0.001393 | 0.001335 |
| function_traits_test.test | RUN | 0.084381 | 0.197248 | 0.279827 | 0.001288 | 0.001474 | 0.001262 |
| floating_point_promotion_test.test | RUN | 0.082170 | 0.198144 | 0.259593 | 0.001498 | 0.001411 | 0.001280 |
| extent_test.test | RUN | 0.084148 | 0.203776 | 0.290112 | 0.001353 | 0.001327 | 0.001380 |
| enable_if_test.test | RUN | 0.079145 | 0.178424 | 0.226394 | 0.001343 | 0.001242 | 0.001072 |
| disjunction_test.test | RUN | 0.081236 | 0.179854 | 0.225808 | 0.001268 | 0.001354 | 0.001400 |
| detected_test.test | RUN | 0.081319 | 0.171064 | 0.206135 | 0.001590 | 0.001515 | 0.001337 |
| detected_or_test.test | RUN | 0.082960 | 0.178843 | 0.225266 | 0.001409 | 0.001437 | 0.001195 |
| decay_test.test | RUN | 0.198787 | 0.664580 | 1.299741 | 0.001299 | 0.001626 | 0.001261 |
| cxx14_aliases_test.test | RUN | 0.127151 | 0.262061 | 0.355174 | 0.001310 | 0.001540 | 0.001308 |
| copy_reference_test.test | RUN | 0.080317 | 0.179521 | 0.239199 | 0.001289 | 0.001349 | 0.001329 |
| copy_cv_test.test | RUN | 0.081965 | 0.180449 | 0.254492 | 0.001177 | 0.001403 | 0.001198 |
| copy_cv_ref_test.test | RUN | 0.111815 | 0.304080 | 0.530419 | 0.001530 | 0.001458 | 0.001426 |
| conjunction_test.test | RUN | 0.081043 | 0.179914 | 0.236438 | 0.001382 | 0.001204 | 0.001361 |
| conditional_test.test | RUN | 0.080064 | 0.178221 | 0.230559 | 0.001325 | 0.001260 | 0.001271 |
| common_type_test.test | RUN | 0.105534 | 0.265156 | 0.440808 | 0.001438 | 0.001284 | 0.001227 |
| common_type_sfinae_test.test | RUN | 0.078833 | 0.171152 | 0.225246 | 0.001564 | 0.001350 | 0.001335 |
| common_type_sfinae2_test.test | RUN | 0.081109 | 0.180976 | 0.233575 | 0.001430 | 0.001292 | 0.001242 |
| common_type_6_test.test | RUN | 0.084214 | 0.182056 | 0.246984 | 0.001299 | 0.001302 | 0.001413 |
| common_type_5_test.test | RUN | 0.086785 | 0.179674 | 0.248585 | 0.001257 | 0.001400 | 0.001197 |
| common_type_4_test.test | RUN | 0.088871 | 0.197279 | 0.287264 | 0.001326 | 0.001268 | 0.001347 |
| common_type_3_test.test | RUN | 0.078544 | 0.170085 | 0.220802 | 0.001323 | 0.001390 | 0.001087 |
| common_type_2_test.test | RUN | 0.107612 | 0.263937 | 0.432535 | 0.001551 | 0.001517 | 0.001412 |
| alignment_of_test.test | RUN | 0.087236 | 0.216273 | 0.297670 | 0.001317 | 0.001372 | 0.001246 |
| alignment_of_a2_test.test | RUN | 0.087332 | 0.213881 | 0.291779 | 0.001542 | 0.001526 | 0.001218 |
| aligned_storage_test.test | RUN | 0.089928 | 0.213616 | 0.269033 | 0.001443 | 0.001218 | 0.001366 |
| aligned_storage_empy_test.test | RUN | 0.101828 | 0.239312 | 0.351801 | 0.001624 | 0.001588 | 0.001386 |
| aligned_storage_a2_test.test | RUN | 0.092951 | 0.217619 | 0.274919 | 0.001444 | 0.001678 | 0.001390 |
| add_volatile_test.test | RUN | 0.176764 | 0.637262 | 1.205890 | 0.001332 | 0.001397 | 0.001410 |
| add_rvalue_reference_test.test | RUN | 0.197778 | 0.719344 | 1.383098 | 0.001498 | 0.001122 | 0.001410 |
| add_reference_test.test | RUN | 0.190544 | 0.670083 | 1.328640 | 0.001340 | 0.001516 | 0.001410 |
| add_pointer_test.test | RUN | 0.127679 | 0.367043 | 0.703296 | 0.001288 | 0.001454 | 0.001341 |
| add_lvalue_reference_test.test | RUN | 0.187816 | 0.650521 | 1.342184 | 0.001348 | 0.001619 | 0.001360 |
| add_cv_test.test | RUN | 0.182466 | 0.620979 | 1.262266 | 0.001352 | 0.001270 | 0.001259 |
| add_const_test.test | RUN | 0.181653 | 0.615884 | 1.280640 | 0.001337 | 0.001408 | 0.001408 |
| is_virtual_base_of_fail.test | COMPILE_FAIL | 0.074319 | 0.166670 | 0.212889 | — | — | — |
| is_stateless_fail.test | COMPILE_FAIL | 0.080171 | 0.168877 | 0.217605 | — | — | — |
| is_polymorphic_fail.test | COMPILE_FAIL | 0.074994 | 0.166457 | 0.209678 | — | — | — |
| is_nothrow_move_constructible_fail.test | COMPILE_FAIL | 0.074863 | 0.160887 | 0.205119 | — | — | — |
| is_nothrow_move_assignable_fail.test | COMPILE_FAIL | 0.076041 | 0.162639 | 0.194603 | — | — | — |
| is_list_constructible_fail.test | COMPILE_FAIL | 0.073971 | 0.164071 | 0.201625 | — | — | — |
| is_empty_fail.test | COMPILE_FAIL | 0.076051 | 0.165229 | 0.208066 | — | — | — |
| is_destructible_fail.test | COMPILE_FAIL | 0.074688 | 0.161009 | 0.209906 | — | — | — |
| is_default_constructible_fail.test | COMPILE_FAIL | 0.075214 | 0.161368 | 0.211445 | — | — | — |
| is_copy_constructible_fail.test | COMPILE_FAIL | 0.075174 | 0.166453 | 0.208163 | — | — | — |
| is_copy_assignable_fail.test | COMPILE_FAIL | 0.074751 | 0.164098 | 0.208204 | — | — | — |
| is_convertible_fail.test | COMPILE_FAIL | 0.075817 | 0.160316 | 0.208672 | — | — | — |
| is_convertible2_fail.test | COMPILE_FAIL | 0.074413 | 0.161661 | 0.205494 | — | — | — |
| is_constructible_fail.test | COMPILE_FAIL | 0.074956 | 0.158803 | 0.209657 | — | — | — |
| is_base_of_fail.test | COMPILE_FAIL | 0.073722 | 0.164001 | 0.205979 | — | — | — |
| is_assignable_fail.test | COMPILE_FAIL | 0.074706 | 0.160985 | 0.205038 | — | — | — |
| has_nothrow_destruct_fail.test | COMPILE_FAIL | 0.074097 | 0.162562 | 0.203254 | — | — | — |
| has_nothrow_copy_fail.test | COMPILE_FAIL | 0.074697 | 0.162495 | 0.209980 | — | — | — |
| has_nothrow_cons_fail.test | COMPILE_FAIL | 0.074077 | 0.166200 | 0.208194 | — | — | — |
| has_nothrow_assign_fail.test | COMPILE_FAIL | 0.075437 | 0.162882 | 0.207250 | — | — | — |
| common_type_fail.test | COMPILE_FAIL | 0.075341 | 0.159416 | 0.201342 | — | — | — |
| common_type2_fail.test | COMPILE_FAIL | 0.074933 | 0.164671 | 0.209449 | — | — | — |
| has_nothrow_assign_test_no_intrinsics.test | RUN | 0.114857 | 0.335513 | 0.553832 | 0.001498 | 0.001542 | 0.001264 |
| has_nothrow_constr_test_no_intrinsics.test | RUN | 0.111184 | 0.341155 | 0.550796 | 0.001348 | 0.001533 | 0.000966 |
| has_nothrow_copy_test_no_intrinsics.test | RUN | 0.111163 | 0.342416 | 0.551954 | 0.001137 | 0.001051 | 0.000976 |
| is_nothrow_move_assignable_test_no_intrinsics.test | RUN | 0.113362 | 0.331712 | 0.535309 | 0.001185 | 0.001429 | 0.001067 |
| is_nothrow_move_constructible_test_no_intrinsics.test | RUN | 0.081140 | 0.256155 | 0.310392 | 0.001125 | 0.001088 | 0.000999 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.001× | 0.991×, 1.011× | 0.989× |
| clang | 1.059×† | 1.025×, 1.094× | 1.000× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026855).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| add_const_test.test | 10 | 0.000840s | 0.000843s | 0.000849s | byte equality observed |
| add_cv_test.test | 10 | 0.000841s | 0.000827s | 0.000842s | byte equality observed |
| add_lvalue_reference_test.test | 10 | 0.000855s | 0.000849s | 0.000853s | byte equality observed |
| add_pointer_test.test | 10 | 0.000846s | 0.000846s | 0.000846s | byte equality observed |
| add_reference_test.test | 10 | 0.000831s | 0.000848s | 0.000899s | byte equality observed |
| add_rvalue_reference_test.test | 10 | 0.000862s | 0.000844s | 0.000859s | byte equality observed |
| add_volatile_test.test | 10 | 0.000844s | 0.000837s | 0.000843s | byte equality observed |
| aligned_storage_a2_test.test | 10 | 0.000935s | 0.000924s | 0.000928s | byte equality observed |
| aligned_storage_empy_test.test | 10 | 0.000860s | 0.000840s | 0.000837s | byte equality observed |
| aligned_storage_test.test | 10 | 0.000924s | 0.000936s | 0.000970s | byte equality observed |
| alignment_of_a2_test.test | 10 | 0.000843s | 0.000843s | 0.000853s | byte equality observed |
| alignment_of_test.test | 10 | 0.000847s | 0.000845s | 0.000854s | byte equality observed |
| common_type_2_test.test | 10 | 0.000840s | 0.000834s | 0.000846s | byte equality observed |
| common_type_3_test.test | 10 | 0.000842s | 0.000850s | 0.000834s | byte equality observed |
| common_type_4_test.test | 10 | 0.000848s | 0.000843s | 0.000833s | byte equality observed |
| common_type_5_test.test | 10 | 0.000851s | 0.000832s | 0.000845s | byte equality observed |
| common_type_6_test.test | 10 | 0.000863s | 0.000850s | 0.000848s | byte equality observed |
| common_type_sfinae2_test.test | 10 | 0.000832s | 0.000850s | 0.000834s | byte equality observed |
| common_type_sfinae_test.test | 10 | 0.000845s | 0.000827s | 0.000853s | byte equality observed |
| common_type_test.test | 10 | 0.000841s | 0.000827s | 0.000838s | byte equality observed |
| conditional_test.test | 10 | 0.000847s | 0.000847s | 0.000842s | byte equality observed |
| conjunction_test.test | 10 | 0.000839s | 0.000841s | 0.000841s | byte equality observed |
| copy_cv_ref_test.test | 10 | 0.000838s | 0.000838s | 0.000836s | byte equality observed |
| copy_cv_test.test | 10 | 0.000838s | 0.000831s | 0.000835s | byte equality observed |
| copy_reference_test.test | 10 | 0.000837s | 0.000823s | 0.000842s | byte equality observed |
| cxx14_aliases_test.test | 10 | 0.000838s | 0.000859s | 0.000847s | byte equality observed |
| decay_test.test | 10 | 0.000847s | 0.000838s | 0.000855s | byte equality observed |
| detected_or_test.test | 10 | 0.000842s | 0.000855s | 0.000843s | byte equality observed |
| detected_test.test | 10 | 0.000840s | 0.000833s | 0.000855s | byte equality observed |
| disjunction_test.test | 10 | 0.000850s | 0.000856s | 0.000853s | byte equality observed |
| enable_if_test.test | 10 | 0.000843s | 0.000829s | 0.000856s | byte equality observed |
| extent_test.test | 10 | 0.000853s | 0.000852s | 0.000842s | byte equality observed |
| floating_point_promotion_test.test | 10 | 0.000834s | 0.000826s | 0.000853s | byte equality observed |
| function_traits_test.test | 10 | 0.000851s | 0.000841s | 0.000839s | byte equality observed |
| has_binary_classes0_test.test | 10 | 0.000834s | 0.000839s | 0.000847s | byte equality observed |
| has_binary_classes1_test.test | 10 | 0.000855s | 0.000831s | 0.000861s | byte equality observed |
| has_binary_classes2_test.test | 10 | 0.000849s | 0.000824s | 0.000865s | byte equality observed |
| has_binary_classes3_test.test | 10 | 0.000849s | 0.000847s | 0.000837s | byte equality observed |
| has_binary_classes4_test.test | 10 | 0.000843s | 0.000832s | 0.000865s | byte equality observed |
| has_binary_classes5_test.test | 10 | 0.000847s | 0.000837s | 0.000840s | byte equality observed |
| has_binary_classes6_test.test | 10 | 0.000833s | 0.000846s | 0.000860s | byte equality observed |
| has_binary_classes7_test.test | 10 | 0.000841s | 0.000848s | 0.000870s | byte equality observed |
| has_binary_classes8_test.test | 10 | 0.000831s | 0.000832s | 0.000846s | byte equality observed |
| has_binary_classes9_test.test | 10 | 0.000835s | 0.000834s | 0.000874s | byte equality observed |
| has_bit_and_assign_test.test | 10 | 0.000852s | 0.000858s | 0.000858s | byte equality observed |
| has_bit_and_test.test | 10 | 0.000840s | 0.000835s | 0.000861s | byte equality observed |
| has_bit_or_assign_test.test | 10 | 0.000854s | 0.000830s | 0.000845s | byte equality observed |
| has_bit_or_test.test | 10 | 0.000844s | 0.000834s | 0.000876s | byte equality observed |
| has_bit_xor_assign_test.test | 10 | 0.000852s | 0.000839s | 0.000865s | byte equality observed |
| has_bit_xor_test.test | 10 | 0.000857s | 0.000833s | 0.000830s | byte equality observed |
| has_complement_test.test | 10 | 0.000839s | 0.000832s | 0.000871s | byte equality observed |
| has_dereference_test.test | 10 | 0.000826s | 0.000858s | 0.000847s | byte equality observed |
| has_divides_assign_test.test | 10 | 0.000837s | 0.000838s | 0.000844s | byte equality observed |
| has_divides_test.test | 10 | 0.000841s | 0.000830s | 0.000848s | byte equality observed |
| has_equal_to_test.test | 10 | 0.000848s | 0.000833s | 0.000859s | byte equality observed |
| has_greater_equal_test.test | 10 | 0.000846s | 0.000836s | 0.000856s | byte equality observed |
| has_greater_test.test | 10 | 0.000835s | 0.000842s | 0.000844s | byte equality observed |
| has_left_shift_assign_test.test | 10 | 0.000851s | 0.000844s | 0.000841s | byte equality observed |
| has_left_shift_test.test | 10 | 0.000846s | 0.000858s | 0.000864s | byte equality observed |
| has_less_equal_test.test | 10 | 0.000844s | 0.000825s | 0.000834s | byte equality observed |
| has_less_test.test | 10 | 0.000856s | 0.000843s | 0.000847s | byte equality observed |
| has_logical_and_test.test | 10 | 0.000841s | 0.000832s | 0.000844s | byte equality observed |
| has_logical_not_test.test | 10 | 0.000852s | 0.000848s | 0.000834s | byte equality observed |
| has_logical_or_test.test | 10 | 0.000842s | 0.000829s | 0.000849s | byte equality observed |
| has_minus_assign_test.test | 10 | 0.000838s | 0.000839s | 0.000840s | byte equality observed |
| has_minus_test.test | 10 | 0.000852s | 0.000856s | 0.000835s | byte equality observed |
| has_modulus_assign_test.test | 10 | 0.000845s | 0.000828s | 0.000847s | byte equality observed |
| has_modulus_test.test | 10 | 0.000850s | 0.000847s | 0.000855s | byte equality observed |
| has_multiplies_assign_test.test | 10 | 0.000853s | 0.000844s | 0.000829s | byte equality observed |
| has_multiplies_test.test | 10 | 0.000858s | 0.000849s | 0.000850s | byte equality observed |
| has_negate_test.test | 10 | 0.000842s | 0.000844s | 0.000835s | byte equality observed |
| has_not_equal_to_test.test | 10 | 0.000859s | 0.000850s | 0.000848s | byte equality observed |
| has_nothrow_assign_test.test | 10 | 0.000841s | 0.000838s | 0.000836s | byte equality observed |
| has_nothrow_assign_test_no_intrinsics.test | 10 | 0.000872s | 0.000837s | 0.000863s | byte equality observed |
| has_nothrow_constr_test.test | 10 | 0.000844s | 0.000827s | 0.000847s | byte equality observed |
| has_nothrow_constr_test_no_intrinsics.test | 10 | 0.000845s | 0.000849s | 0.000849s | byte equality observed |
| has_nothrow_copy_test.test | 10 | 0.000834s | 0.000863s | 0.000834s | byte equality observed |
| has_nothrow_copy_test_no_intrinsics.test | 10 | 0.000855s | 0.000854s | 0.000845s | byte equality observed |
| has_nothrow_destructor_test.test | 10 | 0.000849s | 0.000832s | 0.000821s | byte equality observed |
| has_operator_new_test.test | 10 | 0.000843s | 0.000840s | 0.000829s | byte equality observed |
| has_plus_assign_test.test | 10 | 0.000857s | 0.000836s | 0.000846s | byte equality observed |
| has_plus_test.test | 10 | 0.000847s | 0.000849s | 0.000825s | byte equality observed |
| has_post_decrement_test.test | 10 | 0.000845s | 0.000830s | 0.000838s | byte equality observed |
| has_post_increment_test.test | 10 | 0.000841s | 0.000845s | 0.000840s | byte equality observed |
| has_postfix_classes0_test.test | 10 | 0.000843s | 0.000829s | 0.000844s | byte equality observed |
| has_postfix_classes1_test.test | 10 | 0.000836s | 0.000844s | 0.000846s | byte equality observed |
| has_postfix_classes2_test.test | 10 | 0.000841s | 0.000850s | 0.000844s | byte equality observed |
| has_postfix_classes3_test.test | 10 | 0.000843s | 0.000851s | 0.000845s | byte equality observed |
| has_pre_decrement_test.test | 10 | 0.000840s | 0.000831s | 0.000844s | byte equality observed |
| has_pre_increment_test.test | 10 | 0.000828s | 0.000837s | 0.000845s | byte equality observed |
| has_prefix_classes0_test.test | 10 | 0.000842s | 0.000829s | 0.000827s | byte equality observed |
| has_prefix_classes1_test.test | 10 | 0.000839s | 0.000822s | 0.000831s | byte equality observed |
| has_prefix_classes2_test.test | 10 | 0.000841s | 0.000840s | 0.000822s | byte equality observed |
| has_prefix_classes3_test.test | 10 | 0.000842s | 0.000877s | 0.000835s | byte equality observed |
| has_right_shift_assign_test.test | 10 | 0.000851s | 0.000850s | 0.000860s | byte equality observed |
| has_right_shift_test.test | 10 | 0.000829s | 0.000839s | 0.000827s | byte equality observed |
| has_trivial_assign_test.test | 10 | 0.000904s | 0.000901s | 0.000884s | byte equality observed |
| has_trivial_constr_test.test | 10 | 0.000872s | 0.000869s | 0.000882s | native test oracle; output varies |
| has_trivial_copy_test.test | 10 | 0.000897s | 0.000902s | 0.000906s | native test oracle; output varies |
| has_trivial_destructor_test.test | 10 | 0.000832s | 0.000824s | 0.000844s | byte equality observed |
| has_trivial_move_assign_test.test | 10 | 0.000844s | 0.000837s | 0.000825s | byte equality observed |
| has_trivial_move_constructor_test.test | 10 | 0.000830s | 0.000836s | 0.000847s | byte equality observed |
| has_unary_minus_test.test | 10 | 0.000834s | 0.000813s | 0.000834s | byte equality observed |
| has_unary_plus_test.test | 10 | 0.000841s | 0.000853s | 0.000833s | byte equality observed |
| has_virtual_destructor_test.test | 10 | 0.000845s | 0.000833s | 0.000850s | byte equality observed |
| integral_constant_test.test | 10 | 0.000393s | 0.000386s | 0.000836s | byte equality observed |
| is_abstract_test.test | 10 | 0.000843s | 0.000834s | 0.000848s | byte equality observed |
| is_arithmetic_test.test | 10 | 0.000830s | 0.000838s | 0.000865s | byte equality observed |
| is_array_test.test | 10 | 0.000840s | 0.000839s | 0.000842s | byte equality observed |
| is_assignable_test.test | 10 | 0.000841s | 0.000843s | 0.000860s | byte equality observed |
| is_base_and_derived_test.test | 10 | 0.000853s | 0.000853s | 0.000831s | byte equality observed |
| is_base_of_test.test | 10 | 0.000844s | 0.000834s | 0.000849s | byte equality observed |
| is_bounded_array_test.test | 10 | 0.000852s | 0.000850s | 0.000846s | byte equality observed |
| is_class_test.test | 10 | 0.000833s | 0.000831s | 0.000866s | byte equality observed |
| is_complete_test.test | 10 | 0.000839s | 0.000862s | 0.000844s | byte equality observed |
| is_complex_test.test | 10 | 0.000852s | 0.000829s | 0.000826s | byte equality observed |
| is_compound_test.test | 10 | 0.000837s | 0.000833s | 0.000844s | byte equality observed |
| is_const_test.test | 10 | 0.000839s | 0.000841s | 0.000845s | byte equality observed |
| is_constructible_test.test | 10 | 0.000843s | 0.000839s | 0.000847s | byte equality observed |
| is_convertible_test.test | 10 | 0.000854s | 0.000854s | 0.000838s | byte equality observed |
| is_copy_assignable_test.test | 10 | 0.000840s | 0.000852s | 0.000838s | byte equality observed |
| is_copy_constructible_test.test | 10 | 0.000843s | 0.000847s | 0.000868s | byte equality observed |
| is_default_constr_test.test | 10 | 0.000834s | 0.000867s | 0.000855s | byte equality observed |
| is_destructible_test.test | 10 | 0.000843s | 0.000867s | 0.000849s | byte equality observed |
| is_detected_exact_test.test | 10 | 0.000839s | 0.000875s | 0.000840s | byte equality observed |
| is_detected_test.test | 10 | 0.000840s | 0.000847s | 0.000836s | byte equality observed |
| is_empty_test.test | 10 | 0.000838s | 0.000872s | 0.000857s | byte equality observed |
| is_enum_test.test | 10 | 0.000840s | 0.000865s | 0.000854s | byte equality observed |
| is_final_test.test | 10 | 0.000846s | 0.000869s | 0.000844s | byte equality observed |
| is_float_test.test | 10 | 0.000853s | 0.000875s | 0.000841s | byte equality observed |
| is_floating_point_test.test | 10 | 0.000839s | 0.000872s | 0.000861s | byte equality observed |
| is_function_test.test | 10 | 0.000852s | 0.000867s | 0.000829s | byte equality observed |
| is_fundamental_test.test | 10 | 0.000839s | 0.000867s | 0.000834s | byte equality observed |
| is_integral_test.test | 10 | 0.000837s | 0.000847s | 0.000847s | byte equality observed |
| is_list_constructible_test.test | 10 | 0.000835s | 0.000868s | 0.000828s | byte equality observed |
| is_lvalue_reference_test.test | 10 | 0.000850s | 0.000868s | 0.000841s | byte equality observed |
| is_member_func_test.test | 10 | 0.000847s | 0.000855s | 0.000845s | byte equality observed |
| is_member_obj_test.test | 10 | 0.000842s | 0.000858s | 0.000858s | byte equality observed |
| is_member_pointer_test.test | 10 | 0.000849s | 0.000877s | 0.000828s | byte equality observed |
| is_noncopyable_test.test | 10 | 0.000832s | 0.000869s | 0.000860s | byte equality observed |
| is_nothrow_move_assignable_test.test | 10 | 0.000843s | 0.000849s | 0.000849s | byte equality observed |
| is_nothrow_move_assignable_test_no_intrinsics.test | 10 | 0.000828s | 0.000839s | 0.000843s | byte equality observed |
| is_nothrow_move_constructible_test.test | 10 | 0.000849s | 0.000835s | 0.000835s | byte equality observed |
| is_nothrow_move_constructible_test_no_intrinsics.test | 10 | 0.000842s | 0.000829s | 0.000835s | byte equality observed |
| is_nothrow_swappable_test.test | 10 | 0.000863s | 0.000828s | 0.000828s | byte equality observed |
| is_object_test.test | 10 | 0.000842s | 0.000859s | 0.000857s | byte equality observed |
| is_pod_test.test | 10 | 0.000835s | 0.000840s | 0.000841s | byte equality observed |
| is_pointer_test.test | 10 | 0.000834s | 0.000864s | 0.000824s | byte equality observed |
| is_polymorphic_test.test | 10 | 0.000838s | 0.000844s | 0.000837s | byte equality observed |
| is_reference_test.test | 10 | 0.000834s | 0.000843s | 0.000843s | byte equality observed |
| is_rvalue_reference_test.test | 10 | 0.000843s | 0.000832s | 0.000858s | byte equality observed |
| is_same_test.test | 10 | 0.000832s | 0.000845s | 0.000827s | byte equality observed |
| is_scalar_test.test | 10 | 0.000841s | 0.000846s | 0.000840s | byte equality observed |
| is_scoped_enum_test.test | 10 | 0.000839s | 0.000849s | 0.000835s | byte equality observed |
| is_signed_test.test | 10 | 0.000828s | 0.000847s | 0.000833s | byte equality observed |
| is_stateless_test.test | 10 | 0.000841s | 0.000836s | 0.000832s | byte equality observed |
| is_swappable_test.test | 10 | 0.000836s | 0.000848s | 0.000826s | byte equality observed |
| is_trivially_copyable_test.test | 10 | 0.000873s | 0.000876s | 0.000875s | byte equality observed |
| is_unbounded_array_test.test | 10 | 0.000838s | 0.000830s | 0.000832s | byte equality observed |
| is_union_test.test | 10 | 0.000829s | 0.000845s | 0.000836s | byte equality observed |
| is_unscoped_enum_test.test | 10 | 0.000825s | 0.000832s | 0.000846s | byte equality observed |
| is_unsigned_test.test | 10 | 0.000846s | 0.000836s | 0.000843s | byte equality observed |
| is_virtual_base_of_test.test | 10 | 0.000833s | 0.000835s | 0.000839s | byte equality observed |
| is_void_test.test | 10 | 0.000849s | 0.000825s | 0.000837s | byte equality observed |
| is_volatile_test.test | 10 | 0.000831s | 0.000854s | 0.000838s | byte equality observed |
| make_signed_test.test | 10 | 0.000836s | 0.000841s | 0.000851s | byte equality observed |
| make_unsigned_test.test | 10 | 0.000844s | 0.000854s | 0.000832s | byte equality observed |
| make_void_test.test | 10 | 0.000841s | 0.000852s | 0.000865s | byte equality observed |
| mpl_interop_test1.test | 10 | 0.000400s | 0.000390s | 0.000852s | byte equality observed |
| mpl_interop_test2.test | 10 | 0.000404s | 0.000393s | 0.000839s | byte equality observed |
| mpl_interop_test3.test | 10 | 0.000400s | 0.000399s | 0.000855s | byte equality observed |
| negation_test.test | 10 | 0.000863s | 0.000863s | 0.000866s | byte equality observed |
| promote_basic_test.test | 10 | 0.000395s | 0.000391s | 0.000836s | byte equality observed |
| promote_enum_msvc_bug_test.test | 10 | 0.000402s | 0.000394s | 0.000844s | byte equality observed |
| promote_enum_test.test | 10 | 0.000397s | 0.000408s | 0.000847s | byte equality observed |
| promote_mpl_test.test | 10 | 0.000397s | 0.000398s | 0.000842s | byte equality observed |
| rank_test.test | 10 | 0.000852s | 0.000844s | 0.000859s | byte equality observed |
| remove_all_extents_test.test | 10 | 0.000863s | 0.000857s | 0.000839s | byte equality observed |
| remove_bounds_test.test | 10 | 0.000852s | 0.000853s | 0.000866s | byte equality observed |
| remove_const_test.test | 10 | 0.000832s | 0.000851s | 0.000839s | byte equality observed |
| remove_cv_ref_test.test | 10 | 0.000852s | 0.000831s | 0.000833s | byte equality observed |
| remove_cv_test.test | 10 | 0.000843s | 0.000842s | 0.000867s | byte equality observed |
| remove_extent_test.test | 10 | 0.000851s | 0.000842s | 0.000822s | byte equality observed |
| remove_pointer_test.test | 10 | 0.000841s | 0.000830s | 0.000841s | byte equality observed |
| remove_reference_test.test | 10 | 0.000830s | 0.000840s | 0.000841s | byte equality observed |
| remove_volatile_test.test | 10 | 0.000836s | 0.000851s | 0.000854s | byte equality observed |
| tricky_abstract_type_test.test | 10 | 0.000850s | 0.000835s | 0.000829s | byte equality observed |
| tricky_add_pointer_test.test | 10 | 0.000844s | 0.000850s | 0.000852s | byte equality observed |
| tricky_function_type_test.test | 10 | 0.000838s | 0.000851s | 0.000845s | byte equality observed |
| tricky_incomplete_type_test.test | 10 | 0.000840s | 0.000851s | 0.000842s | byte equality observed |
| tricky_is_enum_test.test | 10 | 0.000840s | 0.000855s | 0.000829s | byte equality observed |
| tricky_partial_spec_test.test | 10 | 0.000837s | 0.000839s | 0.000853s | byte equality observed |
| tricky_rvalue_test.test | 10 | 0.000841s | 0.000846s | 0.000867s | byte equality observed |
| type_identity_test.test | 10 | 0.000845s | 0.000853s | 0.000854s | byte equality observed |
| type_traits_test.test | 10 | 0.000390s | 0.000391s | 0.000867s | byte equality observed |
| type_with_alignment_test.test | 10 | 0.000934s | 0.000971s | 0.000930s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026855).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
