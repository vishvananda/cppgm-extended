# libs/type_traits/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| type_with_alignment_test.test | RUN | 0.091450 | 0.271257 | 0.380217 | 0.001191 | 0.001319 | 0.001310 |
| type_traits_test.test | RUN | 0.126645 | 0.276368 | 0.383614 | 0.000506 | 0.000498 | 0.001127 |
| type_identity_test.test | RUN | 0.236830 | 0.937533 | 1.958139 | 0.001155 | 0.001324 | 0.001371 |
| tricky_rvalue_test.test | RUN | 0.083283 | 0.220277 | 0.300583 | 0.001086 | 0.001292 | 0.001236 |
| tricky_partial_spec_test.test | RUN | 0.103964 | 0.306854 | 0.473896 | 0.001032 | 0.001176 | 0.001179 |
| tricky_is_enum_test.test | RUN | 0.079713 | 0.201006 | 0.273876 | 0.001125 | 0.001164 | 0.001178 |
| tricky_incomplete_type_test.test | RUN | 0.130030 | 0.315735 | 0.446085 | 0.001088 | 0.001205 | 0.001113 |
| tricky_function_type_test.test | RUN | 0.104870 | 0.327824 | 0.529758 | 0.001041 | 0.001196 | 0.001249 |
| tricky_add_pointer_test.test | RUN | 0.157615 | 0.555714 | 1.127523 | 0.001025 | 0.001218 | 0.001268 |
| tricky_abstract_type_test.test | RUN | 0.081004 | 0.208518 | 0.280699 | 0.001006 | 0.001153 | 0.001127 |
| remove_volatile_test.test | RUN | 0.207658 | 0.804724 | 1.801493 | 0.001109 | 0.001250 | 0.001405 |
| remove_reference_test.test | RUN | 0.212062 | 0.846931 | 1.858742 | 0.001033 | 0.001273 | 0.001542 |
| remove_pointer_test.test | RUN | 0.221722 | 0.890360 | 1.947897 | 0.001054 | 0.001256 | 0.001269 |
| remove_extent_test.test | RUN | 0.212939 | 0.860690 | 1.908505 | 0.001048 | 0.001173 | 0.001412 |
| remove_cv_test.test | RUN | 0.217187 | 0.857559 | 1.871229 | 0.001056 | 0.001306 | 0.001398 |
| remove_cv_ref_test.test | RUN | 0.463032 | 2.076941 | 4.487954 | 0.001060 | 0.001161 | 0.001358 |
| remove_const_test.test | RUN | 0.209975 | 0.805172 | 1.796311 | 0.001214 | 0.001265 | 0.001438 |
| remove_bounds_test.test | RUN | 0.223819 | 0.861774 | 1.890069 | 0.001092 | 0.001253 | 0.001174 |
| remove_all_extents_test.test | RUN | 0.224088 | 0.856003 | 1.789284 | 0.001021 | 0.001283 | 0.001230 |
| rank_test.test | RUN | 0.081227 | 0.218897 | 0.305434 | 0.000999 | 0.001149 | 0.001183 |
| promote_mpl_test.test | RUN | 0.074246 | 0.193006 | 0.219284 | 0.000440 | 0.000518 | 0.001255 |
| promote_enum_test.test | RUN | 0.014996 | 0.044771 | 0.054891 | 0.000515 | 0.000493 | 0.001218 |
| promote_enum_msvc_bug_test.test | RUN | 0.013391 | 0.036999 | 0.046223 | 0.000480 | 0.000483 | 0.001309 |
| promote_basic_test.test | RUN | 0.016362 | 0.050810 | 0.055655 | 0.000484 | 0.000443 | 0.001414 |
| negation_test.test | RUN | 0.080967 | 0.204430 | 0.281539 | 0.001131 | 0.001178 | 0.001264 |
| mpl_interop_test3.test | RUN | 0.070310 | 0.181893 | 0.196065 | 0.000403 | 0.000455 | 0.001152 |
| mpl_interop_test2.test | RUN | 0.022824 | 0.049833 | 0.056654 | 0.000390 | 0.000475 | 0.001588 |
| mpl_interop_test1.test | RUN | 0.012725 | 0.037253 | 0.042082 | 0.000434 | 0.000436 | 0.001251 |
| make_void_test.test | RUN | 0.080272 | 0.208936 | 0.307619 | 0.001122 | 0.000857 | 0.001150 |
| make_unsigned_test.test | RUN | 0.095843 | 0.283149 | 0.423540 | 0.001003 | 0.001078 | 0.001289 |
| make_signed_test.test | RUN | 0.095973 | 0.282622 | 0.414987 | 0.000985 | 0.001232 | 0.001212 |
| is_volatile_test.test | RUN | 0.085541 | 0.240241 | 0.342287 | 0.001053 | 0.001139 | 0.001332 |
| is_void_test.test | RUN | 0.083557 | 0.223784 | 0.324825 | 0.001088 | 0.001137 | 0.001319 |
| is_virtual_base_of_test.test | RUN | 0.098117 | 0.284169 | 0.429975 | 0.000971 | 0.001046 | 0.001346 |
| is_unsigned_test.test | RUN | 0.087116 | 0.228631 | 0.336646 | 0.000974 | 0.001242 | 0.001569 |
| is_unscoped_enum_test.test | RUN | 0.087199 | 0.208538 | 0.277775 | 0.001014 | 0.001165 | 0.001322 |
| is_union_test.test | RUN | 0.088190 | 0.261769 | 0.365248 | 0.001091 | 0.001260 | 0.001491 |
| is_unbounded_array_test.test | RUN | 0.080367 | 0.222592 | 0.321095 | 0.001086 | 0.001152 | 0.001424 |
| is_trivially_copyable_test.test | RUN | 0.130727 | 0.463748 | 0.790700 | 0.001075 | 0.001238 | 0.001457 |
| is_swappable_test.test | RUN | 0.109001 | 0.338914 | 0.587220 | 0.001065 | 0.001295 | 0.001374 |
| is_stateless_test.test | RUN | 0.122662 | 0.402910 | 0.672649 | 0.001100 | 0.001151 | 0.001233 |
| is_signed_test.test | RUN | 0.086029 | 0.230225 | 0.340202 | 0.000994 | 0.001242 | 0.001297 |
| is_scoped_enum_test.test | RUN | 0.079028 | 0.209145 | 0.280621 | 0.001016 | 0.001176 | 0.001495 |
| is_scalar_test.test | RUN | 0.104252 | 0.365221 | 0.600554 | 0.000988 | 0.001086 | 0.001348 |
| is_same_test.test | RUN | 0.078921 | 0.221151 | 0.317148 | 0.001066 | 0.000948 | 0.001482 |
| is_rvalue_reference_test.test | RUN | 0.085677 | 0.249281 | 0.368492 | 0.001027 | 0.001311 | 0.001404 |
| is_reference_test.test | RUN | 0.085642 | 0.243725 | 0.369732 | 0.001013 | 0.001101 | 0.001464 |
| is_polymorphic_test.test | RUN | 0.088074 | 0.251619 | 0.393660 | 0.001043 | 0.001305 | 0.001349 |
| is_pointer_test.test | RUN | 0.088511 | 0.247636 | 0.377883 | 0.001043 | 0.001275 | 0.001219 |
| is_pod_test.test | RUN | 0.116780 | 0.396203 | 0.699901 | 0.001071 | 0.001176 | 0.001508 |
| is_object_test.test | RUN | 0.083922 | 0.214302 | 0.306257 | 0.000958 | 0.001138 | 0.001366 |
| is_nothrow_swappable_test.test | RUN | 0.118148 | 0.388085 | 0.647215 | 0.001028 | 0.001165 | 0.001481 |
| is_nothrow_move_constructible_test.test | RUN | 0.111604 | 0.393296 | 0.688978 | 0.001050 | 0.001142 | 0.001358 |
| is_nothrow_move_assignable_test.test | RUN | 0.110552 | 0.403479 | 0.684491 | 0.001027 | 0.000975 | 0.001343 |
| is_noncopyable_test.test | RUN | 0.081141 | 0.220022 | 0.303214 | 0.001010 | 0.001038 | 0.001240 |
| is_member_pointer_test.test | RUN | 0.081626 | 0.217998 | 0.317379 | 0.001021 | 0.001120 | 0.001442 |
| is_member_obj_test.test | RUN | 0.076846 | 0.223921 | 0.331857 | 0.001157 | 0.001086 | 0.001187 |
| is_member_func_test.test | RUN | 0.081160 | 0.237518 | 0.353726 | 0.001031 | 0.001068 | 0.001078 |
| is_lvalue_reference_test.test | RUN | 0.081020 | 0.249499 | 0.379431 | 0.001019 | 0.001156 | 0.001412 |
| is_list_constructible_test.test | RUN | 0.095466 | 0.291061 | 0.421706 | 0.000884 | 0.001149 | 0.001407 |
| is_integral_test.test | RUN | 0.100086 | 0.339511 | 0.575357 | 0.001282 | 0.001130 | 0.001512 |
| is_fundamental_test.test | RUN | 0.102183 | 0.355860 | 0.592223 | 0.000983 | 0.001080 | 0.001328 |
| is_function_test.test | RUN | 0.083421 | 0.236475 | 0.349413 | 0.001172 | 0.001126 | 0.001410 |
| is_floating_point_test.test | RUN | 0.090432 | 0.252791 | 0.380809 | 0.001044 | 0.001176 | 0.001515 |
| is_float_test.test | RUN | 0.085632 | 0.249232 | 0.384909 | 0.001068 | 0.001180 | 0.001502 |
| is_final_test.test | RUN | 0.087427 | 0.258780 | 0.395148 | 0.001024 | 0.001071 | 0.001276 |
| is_enum_test.test | RUN | 0.081857 | 0.214833 | 0.303941 | 0.000928 | 0.001089 | 0.001346 |
| is_empty_test.test | RUN | 0.084425 | 0.229118 | 0.350692 | 0.000963 | 0.001088 | 0.001499 |
| is_detected_test.test | RUN | 0.076655 | 0.200776 | 0.284497 | 0.001100 | 0.001132 | 0.001621 |
| is_detected_exact_test.test | RUN | 0.076465 | 0.198749 | 0.282892 | 0.000949 | 0.001089 | 0.001521 |
| is_destructible_test.test | RUN | 0.114268 | 0.393683 | 0.683973 | 0.000976 | 0.001200 | 0.001217 |
| is_default_constr_test.test | RUN | 0.109751 | 0.405827 | 0.695456 | 0.000991 | 0.001200 | 0.001528 |
| is_copy_constructible_test.test | RUN | 0.115686 | 0.419738 | 0.752261 | 0.000809 | 0.001308 | 0.001619 |
| is_copy_assignable_test.test | RUN | 0.117153 | 0.413293 | 0.733835 | 0.001133 | 0.001099 | 0.001310 |
| is_convertible_test.test | RUN | 0.112863 | 0.376506 | 0.612340 | 0.001010 | 0.001160 | 0.001346 |
| is_constructible_test.test | RUN | 0.090822 | 0.266687 | 0.444700 | 0.001059 | 0.001121 | 0.001703 |
| is_const_test.test | RUN | 0.083829 | 0.234926 | 0.353221 | 0.001167 | 0.001260 | 0.001306 |
| is_compound_test.test | RUN | 0.077310 | 0.206278 | 0.294083 | 0.001229 | 0.001111 | 0.001338 |
| is_complex_test.test | RUN | 0.108175 | 0.270127 | 0.393935 | 0.000953 | 0.001144 | 0.001475 |
| is_complete_test.test | RUN | 0.092851 | 0.277900 | 0.443513 | 0.001028 | 0.001036 | 0.001396 |
| is_class_test.test | RUN | 0.090120 | 0.248319 | 0.369162 | 0.001111 | 0.001089 | 0.001268 |
| is_bounded_array_test.test | RUN | 0.081486 | 0.221307 | 0.316433 | 0.001041 | 0.001291 | 0.001464 |
| is_base_of_test.test | RUN | 0.087004 | 0.242052 | 0.370796 | 0.001060 | 0.001243 | 0.001314 |
| is_base_and_derived_test.test | RUN | 0.087443 | 0.239799 | 0.363154 | 0.000992 | 0.001363 | 0.001374 |
| is_assignable_test.test | RUN | 0.102999 | 0.334911 | 0.551109 | 0.001029 | 0.001106 | 0.001393 |
| is_array_test.test | RUN | 0.079076 | 0.234411 | 0.352780 | 0.001049 | 0.001144 | 0.001354 |
| is_arithmetic_test.test | RUN | 0.109355 | 0.344577 | 0.555829 | 0.000990 | 0.001161 | 0.001419 |
| is_abstract_test.test | RUN | 0.190620 | 0.899838 | 2.060639 | 0.000978 | 0.001215 | 0.001302 |
| integral_constant_test.test | RUN | 0.011733 | 0.036259 | 0.043780 | 0.000456 | 0.000516 | 0.001540 |
| has_virtual_destructor_test.test | RUN | 0.085712 | 0.268609 | 0.434215 | 0.001037 | 0.001252 | 0.001731 |
| has_unary_plus_test.test | RUN | 0.177013 | 0.954358 | 1.504528 | 0.001264 | 0.001288 | 0.001580 |
| has_unary_minus_test.test | RUN | 0.170922 | 0.951974 | 1.481496 | 0.001210 | 0.001256 | 0.001303 |
| has_trivial_move_constructor_test.test | RUN | 0.116309 | 0.401565 | 0.717596 | 0.001245 | 0.001097 | 0.001607 |
| has_trivial_move_assign_test.test | RUN | 0.118286 | 0.408491 | 0.684867 | 0.001065 | 0.001333 | 0.001383 |
| has_trivial_destructor_test.test | RUN | 0.118317 | 0.398990 | 0.693098 | 0.001080 | 0.001088 | 0.001408 |
| has_trivial_copy_test.test | RUN | 0.118480 | 0.439601 | 0.708170 | 0.001046 | 0.001275 | 0.001269 |
| has_trivial_constr_test.test | RUN | 0.122802 | 0.437002 | 0.708646 | 0.000986 | 0.001296 | 0.001270 |
| has_trivial_assign_test.test | RUN | 0.119038 | 0.426237 | 0.699211 | 0.001023 | 0.001205 | 0.001477 |
| has_right_shift_test.test | RUN | 0.178451 | 0.761398 | 1.547734 | 0.000990 | 0.001250 | 0.001312 |
| has_right_shift_assign_test.test | RUN | 0.171470 | 0.742900 | 1.452353 | 0.000989 | 0.001275 | 0.001200 |
| has_prefix_classes3_test.test | RUN | 0.125206 | 0.443180 | 0.741585 | 0.001145 | 0.001375 | 0.001204 |
| has_prefix_classes2_test.test | RUN | 0.126810 | 0.428071 | 0.743648 | 0.000854 | 0.001229 | 0.001346 |
| has_prefix_classes1_test.test | RUN | 0.123870 | 0.440308 | 0.711793 | 0.001218 | 0.001294 | 0.001425 |
| has_prefix_classes0_test.test | RUN | 0.124012 | 0.446770 | 0.715790 | 0.001188 | 0.001225 | 0.001317 |
| has_pre_increment_test.test | RUN | 0.171176 | 0.944772 | 1.483761 | 0.001079 | 0.001173 | 0.001250 |
| has_pre_decrement_test.test | RUN | 0.170551 | 0.952233 | 1.473511 | 0.000752 | 0.001144 | 0.001094 |
| has_postfix_classes3_test.test | RUN | 0.129452 | 0.446412 | 0.722061 | 0.001244 | 0.001043 | 0.001188 |
| has_postfix_classes2_test.test | RUN | 0.128439 | 0.437451 | 0.717824 | 0.001119 | 0.001134 | 0.001134 |
| has_postfix_classes1_test.test | RUN | 0.131339 | 0.446499 | 0.724433 | 0.000973 | 0.001286 | 0.001224 |
| has_postfix_classes0_test.test | RUN | 0.129868 | 0.440885 | 0.743773 | 0.001003 | 0.001157 | 0.001285 |
| has_post_increment_test.test | RUN | 0.175382 | 0.949928 | 1.457846 | 0.001158 | 0.001111 | 0.001156 |
| has_post_decrement_test.test | RUN | 0.171237 | 0.924913 | 1.452187 | 0.000944 | 0.001215 | 0.001103 |
| has_plus_test.test | RUN | 0.178222 | 0.731995 | 1.428392 | 0.001030 | 0.001267 | 0.001205 |
| has_plus_assign_test.test | RUN | 0.183267 | 0.737912 | 1.429276 | 0.000824 | 0.001130 | 0.001061 |
| has_operator_new_test.test | RUN | 0.114097 | 0.388705 | 0.637357 | 0.001174 | 0.001256 | 0.001109 |
| has_nothrow_destructor_test.test | RUN | 0.117158 | 0.417215 | 0.690816 | 0.001100 | 0.001137 | 0.001030 |
| has_nothrow_copy_test.test | RUN | 0.115424 | 0.396067 | 0.664102 | 0.001017 | 0.001358 | 0.001184 |
| has_nothrow_constr_test.test | RUN | 0.113784 | 0.396334 | 0.676566 | 0.001032 | 0.001236 | 0.001228 |
| has_nothrow_assign_test.test | RUN | 0.117243 | 0.404880 | 0.636708 | 0.001180 | 0.001182 | 0.001064 |
| has_not_equal_to_test.test | RUN | 0.176757 | 0.926685 | 1.465659 | 0.001094 | 0.001161 | 0.001178 |
| has_negate_test.test | RUN | 0.172061 | 0.697083 | 1.387106 | 0.001027 | 0.001089 | 0.001106 |
| has_multiplies_test.test | RUN | 0.177699 | 0.735678 | 1.431544 | 0.001014 | 0.001239 | 0.001156 |
| has_multiplies_assign_test.test | RUN | 0.173346 | 0.719775 | 1.430470 | 0.001072 | 0.001169 | 0.001088 |
| has_modulus_test.test | RUN | 0.173837 | 0.733532 | 1.440746 | 0.001153 | 0.001222 | 0.001177 |
| has_modulus_assign_test.test | RUN | 0.174047 | 0.742425 | 1.446631 | 0.001117 | 0.001328 | 0.001139 |
| has_minus_test.test | RUN | 0.182546 | 0.737534 | 1.445840 | 0.001029 | 0.001246 | 0.001264 |
| has_minus_assign_test.test | RUN | 0.178273 | 0.739906 | 1.452675 | 0.001089 | 0.001185 | 0.001191 |
| has_logical_or_test.test | RUN | 0.170239 | 0.739434 | 1.447555 | 0.001029 | 0.001342 | 0.001266 |
| has_logical_not_test.test | RUN | 0.166341 | 0.724448 | 1.403942 | 0.001017 | 0.001125 | 0.001161 |
| has_logical_and_test.test | RUN | 0.174449 | 0.741710 | 1.432179 | 0.000989 | 0.001082 | 0.001149 |
| has_less_test.test | RUN | 0.176723 | 0.936176 | 1.438496 | 0.001184 | 0.001304 | 0.001222 |
| has_less_equal_test.test | RUN | 0.177082 | 0.923096 | 1.445598 | 0.001078 | 0.001083 | 0.001425 |
| has_left_shift_test.test | RUN | 0.194090 | 0.929288 | 1.798883 | 0.001075 | 0.001044 | 0.001194 |
| has_left_shift_assign_test.test | RUN | 0.178784 | 0.730030 | 1.449941 | 0.001093 | 0.001146 | 0.001053 |
| has_greater_test.test | RUN | 0.170839 | 0.917003 | 1.443560 | 0.001103 | 0.001307 | 0.001149 |
| has_greater_equal_test.test | RUN | 0.173312 | 0.961454 | 1.446541 | 0.001068 | 0.001126 | 0.001270 |
| has_equal_to_test.test | RUN | 0.173966 | 0.959679 | 1.480371 | 0.001087 | 0.001115 | 0.001240 |
| has_divides_test.test | RUN | 0.171656 | 0.728885 | 1.420076 | 0.001065 | 0.001252 | 0.001204 |
| has_divides_assign_test.test | RUN | 0.177367 | 0.729817 | 1.460472 | 0.001056 | 0.001178 | 0.001042 |
| has_dereference_test.test | RUN | 0.167796 | 0.718332 | 1.400347 | 0.001089 | 0.001165 | 0.001121 |
| has_complement_test.test | RUN | 0.173471 | 0.936838 | 1.417096 | 0.001027 | 0.001174 | 0.001137 |
| has_bit_xor_test.test | RUN | 0.173399 | 0.731630 | 1.478930 | 0.001052 | 0.001266 | 0.001281 |
| has_bit_xor_assign_test.test | RUN | 0.170749 | 0.728190 | 1.479418 | 0.001082 | 0.001188 | 0.001125 |
| has_bit_or_test.test | RUN | 0.177161 | 0.725459 | 1.432678 | 0.001014 | 0.001264 | 0.001380 |
| has_bit_or_assign_test.test | RUN | 0.174112 | 0.735883 | 1.427324 | 0.001083 | 0.001196 | 0.001261 |
| has_bit_and_test.test | RUN | 0.175828 | 0.720668 | 1.471681 | 0.000961 | 0.001251 | 0.001185 |
| has_bit_and_assign_test.test | RUN | 0.171200 | 0.729992 | 1.464336 | 0.001138 | 0.001182 | 0.001114 |
| has_binary_classes9_test.test | RUN | 0.191835 | 0.875854 | 1.732249 | 0.001129 | 0.001326 | 0.001198 |
| has_binary_classes8_test.test | RUN | 0.189713 | 0.895171 | 1.698767 | 0.001072 | 0.001352 | 0.001327 |
| has_binary_classes7_test.test | RUN | 0.187990 | 0.865949 | 1.695168 | 0.001016 | 0.001237 | 0.001247 |
| has_binary_classes6_test.test | RUN | 0.187104 | 0.849582 | 1.691827 | 0.001112 | 0.001311 | 0.001191 |
| has_binary_classes5_test.test | RUN | 0.185450 | 0.845877 | 1.646851 | 0.001053 | 0.001305 | 0.001305 |
| has_binary_classes4_test.test | RUN | 0.189421 | 0.876522 | 1.661574 | 0.001079 | 0.001144 | 0.001254 |
| has_binary_classes3_test.test | RUN | 0.198072 | 0.877334 | 1.627150 | 0.001124 | 0.001274 | 0.001132 |
| has_binary_classes2_test.test | RUN | 0.191739 | 0.893967 | 1.645125 | 0.001076 | 0.001225 | 0.001087 |
| has_binary_classes1_test.test | RUN | 0.187445 | 0.863296 | 1.648855 | 0.001126 | 0.001193 | 0.001156 |
| has_binary_classes0_test.test | RUN | 0.189774 | 0.902883 | 1.629853 | 0.001063 | 0.001159 | 0.001114 |
| function_traits_test.test | RUN | 0.088090 | 0.229287 | 0.341731 | 0.001075 | 0.001309 | 0.001245 |
| floating_point_promotion_test.test | RUN | 0.083794 | 0.230543 | 0.321923 | 0.001154 | 0.001230 | 0.001222 |
| extent_test.test | RUN | 0.084597 | 0.237644 | 0.330857 | 0.001180 | 0.001216 | 0.001222 |
| enable_if_test.test | RUN | 0.078555 | 0.199553 | 0.261503 | 0.001134 | 0.001090 | 0.001259 |
| disjunction_test.test | RUN | 0.083496 | 0.209228 | 0.284923 | 0.001044 | 0.001144 | 0.001168 |
| detected_test.test | RUN | 0.082059 | 0.194926 | 0.264273 | 0.001107 | 0.001103 | 0.001245 |
| detected_or_test.test | RUN | 0.082436 | 0.201529 | 0.276579 | 0.001003 | 0.001140 | 0.001065 |
| decay_test.test | RUN | 0.216209 | 0.787619 | 1.580059 | 0.001083 | 0.001141 | 0.001041 |
| cxx14_aliases_test.test | RUN | 0.132473 | 0.297095 | 0.428085 | 0.001124 | 0.001159 | 0.001177 |
| copy_reference_test.test | RUN | 0.081224 | 0.207158 | 0.280084 | 0.001000 | 0.001223 | 0.001113 |
| copy_cv_test.test | RUN | 0.087990 | 0.215965 | 0.301448 | 0.001124 | 0.001176 | 0.001164 |
| copy_cv_ref_test.test | RUN | 0.124041 | 0.365934 | 0.669947 | 0.001188 | 0.001210 | 0.001113 |
| conjunction_test.test | RUN | 0.085814 | 0.204310 | 0.282789 | 0.001091 | 0.001097 | 0.001142 |
| conditional_test.test | RUN | 0.083306 | 0.206170 | 0.270231 | 0.000974 | 0.001243 | 0.001115 |
| common_type_test.test | RUN | 0.107446 | 0.311907 | 0.515664 | 0.001034 | 0.001144 | 0.001138 |
| common_type_sfinae_test.test | RUN | 0.077431 | 0.195889 | 0.258442 | 0.001134 | 0.001346 | 0.000925 |
| common_type_sfinae2_test.test | RUN | 0.081508 | 0.200624 | 0.281033 | 0.000911 | 0.001107 | 0.001066 |
| common_type_6_test.test | RUN | 0.083942 | 0.213188 | 0.293378 | 0.000953 | 0.001226 | 0.001184 |
| common_type_5_test.test | RUN | 0.080225 | 0.209413 | 0.286591 | 0.000943 | 0.001174 | 0.001274 |
| common_type_4_test.test | RUN | 0.088456 | 0.230569 | 0.337512 | 0.001077 | 0.001108 | 0.001147 |
| common_type_3_test.test | RUN | 0.079531 | 0.193817 | 0.259546 | 0.001031 | 0.001257 | 0.001018 |
| common_type_2_test.test | RUN | 0.105666 | 0.307550 | 0.511712 | 0.000954 | 0.001104 | 0.001196 |
| alignment_of_test.test | RUN | 0.088068 | 0.252467 | 0.347493 | 0.001052 | 0.001121 | 0.001035 |
| alignment_of_a2_test.test | RUN | 0.085022 | 0.255486 | 0.343466 | 0.000959 | 0.001163 | 0.001077 |
| aligned_storage_test.test | RUN | 0.091820 | 0.245266 | 0.321511 | 0.001145 | 0.001304 | 0.001113 |
| aligned_storage_empy_test.test | RUN | 0.103404 | 0.276420 | 0.423895 | 0.001056 | 0.001170 | 0.001240 |
| aligned_storage_a2_test.test | RUN | 0.089148 | 0.246455 | 0.319529 | 0.001129 | 0.001339 | 0.001386 |
| add_volatile_test.test | RUN | 0.192391 | 0.693815 | 1.496192 | 0.001087 | 0.001340 | 0.001286 |
| add_rvalue_reference_test.test | RUN | 0.216297 | 0.783026 | 1.675268 | 0.000815 | 0.001238 | 0.001197 |
| add_reference_test.test | RUN | 0.211168 | 0.777480 | 1.674020 | 0.001074 | 0.001147 | 0.001327 |
| add_pointer_test.test | RUN | 0.131690 | 0.446724 | 0.898241 | 0.001138 | 0.001217 | 0.001205 |
| add_lvalue_reference_test.test | RUN | 0.203054 | 0.777884 | 1.759040 | 0.001103 | 0.001241 | 0.001223 |
| add_cv_test.test | RUN | 0.194900 | 0.725051 | 1.550381 | 0.001013 | 0.001189 | 0.001301 |
| add_const_test.test | RUN | 0.192147 | 0.732692 | 1.654100 | 0.001023 | 0.001117 | 0.001331 |
| is_virtual_base_of_fail.test | COMPILE_FAIL | 0.073039 | 0.188745 | 0.256471 | — | — | — |
| is_stateless_fail.test | COMPILE_FAIL | 0.077329 | 0.193062 | 0.266327 | — | — | — |
| is_polymorphic_fail.test | COMPILE_FAIL | 0.070477 | 0.184064 | 0.257955 | — | — | — |
| is_nothrow_move_constructible_fail.test | COMPILE_FAIL | 0.074639 | 0.186456 | 0.245541 | — | — | — |
| is_nothrow_move_assignable_fail.test | COMPILE_FAIL | 0.076041 | 0.184629 | 0.250441 | — | — | — |
| is_list_constructible_fail.test | COMPILE_FAIL | 0.074347 | 0.188099 | 0.250797 | — | — | — |
| is_empty_fail.test | COMPILE_FAIL | 0.075556 | 0.188734 | 0.252434 | — | — | — |
| is_destructible_fail.test | COMPILE_FAIL | 0.075219 | 0.187434 | 0.250969 | — | — | — |
| is_default_constructible_fail.test | COMPILE_FAIL | 0.074757 | 0.185978 | 0.254595 | — | — | — |
| is_copy_constructible_fail.test | COMPILE_FAIL | 0.074161 | 0.187625 | 0.255201 | — | — | — |
| is_copy_assignable_fail.test | COMPILE_FAIL | 0.078996 | 0.184392 | 0.250310 | — | — | — |
| is_convertible_fail.test | COMPILE_FAIL | 0.078212 | 0.186888 | 0.253456 | — | — | — |
| is_convertible2_fail.test | COMPILE_FAIL | 0.077427 | 0.191980 | 0.255705 | — | — | — |
| is_constructible_fail.test | COMPILE_FAIL | 0.077025 | 0.185531 | 0.256925 | — | — | — |
| is_base_of_fail.test | COMPILE_FAIL | 0.074841 | 0.186431 | 0.248127 | — | — | — |
| is_assignable_fail.test | COMPILE_FAIL | 0.073998 | 0.180555 | 0.253535 | — | — | — |
| has_nothrow_destruct_fail.test | COMPILE_FAIL | 0.076416 | 0.182327 | 0.254633 | — | — | — |
| has_nothrow_copy_fail.test | COMPILE_FAIL | 0.074629 | 0.188585 | 0.258317 | — | — | — |
| has_nothrow_cons_fail.test | COMPILE_FAIL | 0.078037 | 0.188885 | 0.250795 | — | — | — |
| has_nothrow_assign_fail.test | COMPILE_FAIL | 0.074715 | 0.182856 | 0.251042 | — | — | — |
| common_type_fail.test | COMPILE_FAIL | 0.076863 | 0.186357 | 0.253165 | — | — | — |
| common_type2_fail.test | COMPILE_FAIL | 0.075577 | 0.189577 | 0.250866 | — | — | — |
| has_nothrow_assign_test_no_intrinsics.test | RUN | 0.117838 | 0.405127 | 0.674191 | 0.001109 | 0.001157 | 0.001266 |
| has_nothrow_constr_test_no_intrinsics.test | RUN | 0.112371 | 0.401352 | 0.683709 | 0.001114 | 0.001264 | 0.001306 |
| has_nothrow_copy_test_no_intrinsics.test | RUN | 0.117442 | 0.412670 | 0.688093 | 0.000801 | 0.001326 | 0.001373 |
| is_nothrow_move_assignable_test_no_intrinsics.test | RUN | 0.120388 | 0.405276 | 0.645293 | 0.000747 | 0.000801 | 0.000986 |
| is_nothrow_move_constructible_test_no_intrinsics.test | RUN | 0.075303 | 0.298988 | 0.434620 | 0.000781 | 0.000818 | 0.001016 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.001× | 1.000×, 1.001× | 0.996× |
| clang | 1.029× | 1.031×, 1.027× | 0.996× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814672).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| add_const_test.test | 10 | 0.000787s | 0.000771s | 0.000781s | byte equality observed |
| add_cv_test.test | 10 | 0.000786s | 0.000792s | 0.000797s | byte equality observed |
| add_lvalue_reference_test.test | 10 | 0.000800s | 0.000817s | 0.000800s | byte equality observed |
| add_pointer_test.test | 10 | 0.000786s | 0.000812s | 0.000793s | byte equality observed |
| add_reference_test.test | 10 | 0.000786s | 0.000799s | 0.000784s | byte equality observed |
| add_rvalue_reference_test.test | 10 | 0.000794s | 0.000803s | 0.000755s | byte equality observed |
| add_volatile_test.test | 10 | 0.000804s | 0.000804s | 0.000778s | byte equality observed |
| aligned_storage_a2_test.test | 10 | 0.000825s | 0.000819s | 0.000816s | byte equality observed |
| aligned_storage_empy_test.test | 10 | 0.000795s | 0.000772s | 0.000797s | byte equality observed |
| aligned_storage_test.test | 10 | 0.000838s | 0.000822s | 0.000816s | byte equality observed |
| alignment_of_a2_test.test | 10 | 0.000787s | 0.000771s | 0.000794s | byte equality observed |
| alignment_of_test.test | 10 | 0.000770s | 0.000764s | 0.000789s | byte equality observed |
| common_type_2_test.test | 10 | 0.000791s | 0.000800s | 0.000803s | byte equality observed |
| common_type_3_test.test | 10 | 0.000816s | 0.000822s | 0.000774s | byte equality observed |
| common_type_4_test.test | 10 | 0.000785s | 0.000793s | 0.000752s | byte equality observed |
| common_type_5_test.test | 10 | 0.000785s | 0.000795s | 0.000785s | byte equality observed |
| common_type_6_test.test | 10 | 0.000765s | 0.000804s | 0.000789s | byte equality observed |
| common_type_sfinae2_test.test | 10 | 0.000783s | 0.000772s | 0.000804s | byte equality observed |
| common_type_sfinae_test.test | 10 | 0.000770s | 0.000774s | 0.000798s | byte equality observed |
| common_type_test.test | 10 | 0.000800s | 0.000762s | 0.000796s | byte equality observed |
| conditional_test.test | 10 | 0.000793s | 0.000775s | 0.000775s | byte equality observed |
| conjunction_test.test | 10 | 0.000787s | 0.000786s | 0.000778s | byte equality observed |
| copy_cv_ref_test.test | 10 | 0.000780s | 0.000795s | 0.000807s | byte equality observed |
| copy_cv_test.test | 10 | 0.000789s | 0.000772s | 0.000814s | byte equality observed |
| copy_reference_test.test | 10 | 0.000787s | 0.000781s | 0.000778s | byte equality observed |
| cxx14_aliases_test.test | 10 | 0.000797s | 0.000776s | 0.000787s | byte equality observed |
| decay_test.test | 10 | 0.000794s | 0.000814s | 0.000807s | byte equality observed |
| detected_or_test.test | 10 | 0.000794s | 0.000801s | 0.000806s | byte equality observed |
| detected_test.test | 10 | 0.000789s | 0.000771s | 0.000794s | byte equality observed |
| disjunction_test.test | 10 | 0.000777s | 0.000759s | 0.000805s | byte equality observed |
| enable_if_test.test | 10 | 0.000787s | 0.000777s | 0.000763s | byte equality observed |
| extent_test.test | 10 | 0.000785s | 0.000799s | 0.000783s | byte equality observed |
| floating_point_promotion_test.test | 10 | 0.000794s | 0.000779s | 0.000791s | byte equality observed |
| function_traits_test.test | 10 | 0.000768s | 0.000758s | 0.000765s | byte equality observed |
| has_binary_classes0_test.test | 10 | 0.000783s | 0.000786s | 0.000791s | byte equality observed |
| has_binary_classes1_test.test | 10 | 0.000776s | 0.000790s | 0.000786s | byte equality observed |
| has_binary_classes2_test.test | 10 | 0.000793s | 0.000795s | 0.000788s | byte equality observed |
| has_binary_classes3_test.test | 10 | 0.000786s | 0.000815s | 0.000796s | byte equality observed |
| has_binary_classes4_test.test | 10 | 0.000765s | 0.000832s | 0.000781s | byte equality observed |
| has_binary_classes5_test.test | 10 | 0.000776s | 0.000811s | 0.000792s | byte equality observed |
| has_binary_classes6_test.test | 10 | 0.000786s | 0.000818s | 0.000804s | byte equality observed |
| has_binary_classes7_test.test | 10 | 0.000788s | 0.000795s | 0.000760s | byte equality observed |
| has_binary_classes8_test.test | 10 | 0.000782s | 0.000829s | 0.000785s | byte equality observed |
| has_binary_classes9_test.test | 10 | 0.000790s | 0.000781s | 0.000803s | byte equality observed |
| has_bit_and_assign_test.test | 10 | 0.000793s | 0.000778s | 0.000800s | byte equality observed |
| has_bit_and_test.test | 10 | 0.000777s | 0.000790s | 0.000799s | byte equality observed |
| has_bit_or_assign_test.test | 10 | 0.000785s | 0.000798s | 0.000786s | byte equality observed |
| has_bit_or_test.test | 10 | 0.000767s | 0.000795s | 0.000788s | byte equality observed |
| has_bit_xor_assign_test.test | 10 | 0.000784s | 0.000807s | 0.000766s | byte equality observed |
| has_bit_xor_test.test | 10 | 0.000783s | 0.000795s | 0.000784s | byte equality observed |
| has_complement_test.test | 10 | 0.000792s | 0.000788s | 0.000771s | byte equality observed |
| has_dereference_test.test | 10 | 0.000751s | 0.000784s | 0.000750s | byte equality observed |
| has_divides_assign_test.test | 10 | 0.000771s | 0.000786s | 0.000807s | byte equality observed |
| has_divides_test.test | 10 | 0.000783s | 0.000764s | 0.000782s | byte equality observed |
| has_equal_to_test.test | 10 | 0.000793s | 0.000763s | 0.000789s | byte equality observed |
| has_greater_equal_test.test | 10 | 0.000781s | 0.000786s | 0.000805s | byte equality observed |
| has_greater_test.test | 10 | 0.000790s | 0.000791s | 0.000758s | byte equality observed |
| has_left_shift_assign_test.test | 10 | 0.000789s | 0.000769s | 0.000773s | byte equality observed |
| has_left_shift_test.test | 10 | 0.000794s | 0.000775s | 0.000765s | byte equality observed |
| has_less_equal_test.test | 10 | 0.000801s | 0.000782s | 0.000790s | byte equality observed |
| has_less_test.test | 10 | 0.000790s | 0.000785s | 0.000784s | byte equality observed |
| has_logical_and_test.test | 10 | 0.000777s | 0.000763s | 0.000802s | byte equality observed |
| has_logical_not_test.test | 10 | 0.000770s | 0.000802s | 0.000802s | byte equality observed |
| has_logical_or_test.test | 10 | 0.000773s | 0.000787s | 0.000803s | byte equality observed |
| has_minus_assign_test.test | 10 | 0.000775s | 0.000776s | 0.000810s | byte equality observed |
| has_minus_test.test | 10 | 0.000786s | 0.000783s | 0.000802s | byte equality observed |
| has_modulus_assign_test.test | 10 | 0.000792s | 0.000768s | 0.000811s | byte equality observed |
| has_modulus_test.test | 10 | 0.000800s | 0.000770s | 0.000808s | byte equality observed |
| has_multiplies_assign_test.test | 10 | 0.000781s | 0.000770s | 0.000818s | byte equality observed |
| has_multiplies_test.test | 10 | 0.000772s | 0.000767s | 0.000808s | byte equality observed |
| has_negate_test.test | 10 | 0.000802s | 0.000786s | 0.000797s | byte equality observed |
| has_not_equal_to_test.test | 10 | 0.000776s | 0.000777s | 0.000797s | byte equality observed |
| has_nothrow_assign_test.test | 10 | 0.000772s | 0.000775s | 0.000787s | byte equality observed |
| has_nothrow_assign_test_no_intrinsics.test | 10 | 0.000792s | 0.000781s | 0.000782s | byte equality observed |
| has_nothrow_constr_test.test | 10 | 0.000762s | 0.000798s | 0.000797s | byte equality observed |
| has_nothrow_constr_test_no_intrinsics.test | 10 | 0.000773s | 0.000776s | 0.000802s | byte equality observed |
| has_nothrow_copy_test.test | 10 | 0.000774s | 0.000794s | 0.000771s | byte equality observed |
| has_nothrow_copy_test_no_intrinsics.test | 10 | 0.000787s | 0.000827s | 0.000815s | byte equality observed |
| has_nothrow_destructor_test.test | 10 | 0.000800s | 0.000813s | 0.000797s | byte equality observed |
| has_operator_new_test.test | 10 | 0.000789s | 0.000774s | 0.000765s | byte equality observed |
| has_plus_assign_test.test | 10 | 0.000789s | 0.000787s | 0.000789s | byte equality observed |
| has_plus_test.test | 10 | 0.000797s | 0.000784s | 0.000753s | byte equality observed |
| has_post_decrement_test.test | 10 | 0.000765s | 0.000771s | 0.000803s | byte equality observed |
| has_post_increment_test.test | 10 | 0.000777s | 0.000788s | 0.000800s | byte equality observed |
| has_postfix_classes0_test.test | 10 | 0.000776s | 0.000801s | 0.000770s | byte equality observed |
| has_postfix_classes1_test.test | 10 | 0.000783s | 0.000808s | 0.000769s | byte equality observed |
| has_postfix_classes2_test.test | 10 | 0.000780s | 0.000821s | 0.000782s | byte equality observed |
| has_postfix_classes3_test.test | 10 | 0.000771s | 0.000796s | 0.000803s | byte equality observed |
| has_pre_decrement_test.test | 10 | 0.000790s | 0.000789s | 0.000775s | byte equality observed |
| has_pre_increment_test.test | 10 | 0.000785s | 0.000773s | 0.000773s | byte equality observed |
| has_prefix_classes0_test.test | 10 | 0.000799s | 0.000785s | 0.000811s | byte equality observed |
| has_prefix_classes1_test.test | 10 | 0.000796s | 0.000776s | 0.000809s | byte equality observed |
| has_prefix_classes2_test.test | 10 | 0.000793s | 0.000787s | 0.000821s | byte equality observed |
| has_prefix_classes3_test.test | 10 | 0.000786s | 0.000797s | 0.000791s | byte equality observed |
| has_right_shift_assign_test.test | 10 | 0.000786s | 0.000755s | 0.000773s | byte equality observed |
| has_right_shift_test.test | 10 | 0.000795s | 0.000766s | 0.000796s | byte equality observed |
| has_trivial_assign_test.test | 10 | 0.000814s | 0.000796s | 0.000802s | byte equality observed |
| has_trivial_constr_test.test | 10 | 0.000822s | 0.000791s | 0.000837s | native test oracle; output varies |
| has_trivial_copy_test.test | 10 | 0.000824s | 0.000827s | 0.000829s | native test oracle; output varies |
| has_trivial_destructor_test.test | 10 | 0.000776s | 0.000769s | 0.000790s | byte equality observed |
| has_trivial_move_assign_test.test | 10 | 0.000787s | 0.000789s | 0.000768s | byte equality observed |
| has_trivial_move_constructor_test.test | 10 | 0.000783s | 0.000773s | 0.000772s | byte equality observed |
| has_unary_minus_test.test | 10 | 0.000785s | 0.000758s | 0.000779s | byte equality observed |
| has_unary_plus_test.test | 10 | 0.000780s | 0.000756s | 0.000784s | byte equality observed |
| has_virtual_destructor_test.test | 10 | 0.000763s | 0.000758s | 0.000759s | byte equality observed |
| integral_constant_test.test | 10 | 0.000363s | 0.000364s | 0.000785s | byte equality observed |
| is_abstract_test.test | 10 | 0.000764s | 0.000793s | 0.000778s | byte equality observed |
| is_arithmetic_test.test | 10 | 0.000767s | 0.000808s | 0.000776s | byte equality observed |
| is_array_test.test | 10 | 0.000795s | 0.000807s | 0.000776s | byte equality observed |
| is_assignable_test.test | 10 | 0.000800s | 0.000803s | 0.000777s | byte equality observed |
| is_base_and_derived_test.test | 10 | 0.000773s | 0.000807s | 0.000775s | byte equality observed |
| is_base_of_test.test | 10 | 0.000797s | 0.000768s | 0.000747s | byte equality observed |
| is_bounded_array_test.test | 10 | 0.000789s | 0.000781s | 0.000776s | byte equality observed |
| is_class_test.test | 10 | 0.000788s | 0.000788s | 0.000782s | byte equality observed |
| is_complete_test.test | 10 | 0.000785s | 0.000767s | 0.000787s | byte equality observed |
| is_complex_test.test | 10 | 0.000802s | 0.000776s | 0.000785s | byte equality observed |
| is_compound_test.test | 10 | 0.000771s | 0.000770s | 0.000801s | byte equality observed |
| is_const_test.test | 10 | 0.000781s | 0.000781s | 0.000771s | byte equality observed |
| is_constructible_test.test | 10 | 0.000773s | 0.000792s | 0.000777s | byte equality observed |
| is_convertible_test.test | 10 | 0.000772s | 0.000801s | 0.000785s | byte equality observed |
| is_copy_assignable_test.test | 10 | 0.000783s | 0.000804s | 0.000805s | byte equality observed |
| is_copy_constructible_test.test | 10 | 0.000805s | 0.000788s | 0.000800s | byte equality observed |
| is_default_constr_test.test | 10 | 0.000800s | 0.000748s | 0.000783s | byte equality observed |
| is_destructible_test.test | 10 | 0.000815s | 0.000782s | 0.000764s | byte equality observed |
| is_detected_exact_test.test | 10 | 0.000807s | 0.000784s | 0.000775s | byte equality observed |
| is_detected_test.test | 10 | 0.000807s | 0.000782s | 0.000798s | byte equality observed |
| is_empty_test.test | 10 | 0.000794s | 0.000792s | 0.000810s | byte equality observed |
| is_enum_test.test | 10 | 0.000771s | 0.000759s | 0.000780s | byte equality observed |
| is_final_test.test | 10 | 0.000795s | 0.000781s | 0.000797s | byte equality observed |
| is_float_test.test | 10 | 0.000790s | 0.000787s | 0.000814s | byte equality observed |
| is_floating_point_test.test | 10 | 0.000779s | 0.000774s | 0.000800s | byte equality observed |
| is_function_test.test | 10 | 0.000776s | 0.000795s | 0.000782s | byte equality observed |
| is_fundamental_test.test | 10 | 0.000773s | 0.000769s | 0.000773s | byte equality observed |
| is_integral_test.test | 10 | 0.000770s | 0.000784s | 0.000780s | byte equality observed |
| is_list_constructible_test.test | 10 | 0.000771s | 0.000805s | 0.000786s | byte equality observed |
| is_lvalue_reference_test.test | 10 | 0.000782s | 0.000816s | 0.000795s | byte equality observed |
| is_member_func_test.test | 10 | 0.000799s | 0.000793s | 0.000774s | byte equality observed |
| is_member_obj_test.test | 10 | 0.000773s | 0.000775s | 0.000780s | byte equality observed |
| is_member_pointer_test.test | 10 | 0.000789s | 0.000811s | 0.000799s | byte equality observed |
| is_noncopyable_test.test | 10 | 0.000790s | 0.000771s | 0.000788s | byte equality observed |
| is_nothrow_move_assignable_test.test | 10 | 0.000762s | 0.000777s | 0.000781s | byte equality observed |
| is_nothrow_move_assignable_test_no_intrinsics.test | 10 | 0.000782s | 0.000786s | 0.000797s | byte equality observed |
| is_nothrow_move_constructible_test.test | 10 | 0.000773s | 0.000780s | 0.000792s | byte equality observed |
| is_nothrow_move_constructible_test_no_intrinsics.test | 10 | 0.000773s | 0.000786s | 0.000799s | byte equality observed |
| is_nothrow_swappable_test.test | 10 | 0.000775s | 0.000785s | 0.000809s | byte equality observed |
| is_object_test.test | 10 | 0.000771s | 0.000772s | 0.000799s | byte equality observed |
| is_pod_test.test | 10 | 0.000795s | 0.000774s | 0.000767s | byte equality observed |
| is_pointer_test.test | 10 | 0.000797s | 0.000788s | 0.000795s | byte equality observed |
| is_polymorphic_test.test | 10 | 0.000794s | 0.000804s | 0.000820s | byte equality observed |
| is_reference_test.test | 10 | 0.000783s | 0.000805s | 0.000769s | byte equality observed |
| is_rvalue_reference_test.test | 10 | 0.000783s | 0.000814s | 0.000793s | byte equality observed |
| is_same_test.test | 10 | 0.000765s | 0.000773s | 0.000760s | byte equality observed |
| is_scalar_test.test | 10 | 0.000773s | 0.000781s | 0.000776s | byte equality observed |
| is_scoped_enum_test.test | 10 | 0.000769s | 0.000799s | 0.000797s | byte equality observed |
| is_signed_test.test | 10 | 0.000762s | 0.000797s | 0.000794s | byte equality observed |
| is_stateless_test.test | 10 | 0.000767s | 0.000788s | 0.000781s | byte equality observed |
| is_swappable_test.test | 10 | 0.000788s | 0.000809s | 0.000804s | byte equality observed |
| is_trivially_copyable_test.test | 10 | 0.000804s | 0.000791s | 0.000814s | byte equality observed |
| is_unbounded_array_test.test | 10 | 0.000782s | 0.000775s | 0.000773s | byte equality observed |
| is_union_test.test | 10 | 0.000789s | 0.000791s | 0.000761s | byte equality observed |
| is_unscoped_enum_test.test | 10 | 0.000773s | 0.000762s | 0.000773s | byte equality observed |
| is_unsigned_test.test | 10 | 0.000787s | 0.000797s | 0.000769s | byte equality observed |
| is_virtual_base_of_test.test | 10 | 0.000778s | 0.000773s | 0.000773s | byte equality observed |
| is_void_test.test | 10 | 0.000762s | 0.000780s | 0.000790s | byte equality observed |
| is_volatile_test.test | 10 | 0.000779s | 0.000780s | 0.000770s | byte equality observed |
| make_signed_test.test | 10 | 0.000787s | 0.000769s | 0.000788s | byte equality observed |
| make_unsigned_test.test | 10 | 0.000778s | 0.000767s | 0.000808s | byte equality observed |
| make_void_test.test | 10 | 0.000762s | 0.000778s | 0.000779s | byte equality observed |
| mpl_interop_test1.test | 10 | 0.000369s | 0.000365s | 0.000790s | byte equality observed |
| mpl_interop_test2.test | 10 | 0.000363s | 0.000358s | 0.000774s | byte equality observed |
| mpl_interop_test3.test | 10 | 0.000367s | 0.000360s | 0.000760s | byte equality observed |
| negation_test.test | 10 | 0.000772s | 0.000769s | 0.000770s | byte equality observed |
| promote_basic_test.test | 10 | 0.000364s | 0.000360s | 0.000768s | byte equality observed |
| promote_enum_msvc_bug_test.test | 10 | 0.000353s | 0.000364s | 0.000794s | byte equality observed |
| promote_enum_test.test | 10 | 0.000356s | 0.000366s | 0.000797s | byte equality observed |
| promote_mpl_test.test | 10 | 0.000361s | 0.000363s | 0.000774s | byte equality observed |
| rank_test.test | 10 | 0.000763s | 0.000783s | 0.000780s | byte equality observed |
| remove_all_extents_test.test | 10 | 0.000803s | 0.000800s | 0.000782s | byte equality observed |
| remove_bounds_test.test | 10 | 0.000813s | 0.000787s | 0.000804s | byte equality observed |
| remove_const_test.test | 10 | 0.000788s | 0.000789s | 0.000803s | byte equality observed |
| remove_cv_ref_test.test | 10 | 0.000785s | 0.000748s | 0.000802s | byte equality observed |
| remove_cv_test.test | 10 | 0.000764s | 0.000755s | 0.000790s | byte equality observed |
| remove_extent_test.test | 10 | 0.000785s | 0.000790s | 0.000789s | byte equality observed |
| remove_pointer_test.test | 10 | 0.000775s | 0.000746s | 0.000788s | byte equality observed |
| remove_reference_test.test | 10 | 0.000804s | 0.000761s | 0.000781s | byte equality observed |
| remove_volatile_test.test | 10 | 0.000783s | 0.000766s | 0.000800s | byte equality observed |
| tricky_abstract_type_test.test | 10 | 0.000774s | 0.000766s | 0.000779s | byte equality observed |
| tricky_add_pointer_test.test | 10 | 0.000764s | 0.000783s | 0.000764s | byte equality observed |
| tricky_function_type_test.test | 10 | 0.000781s | 0.000791s | 0.000773s | byte equality observed |
| tricky_incomplete_type_test.test | 10 | 0.000794s | 0.000796s | 0.000788s | byte equality observed |
| tricky_is_enum_test.test | 10 | 0.000784s | 0.000803s | 0.000816s | byte equality observed |
| tricky_partial_spec_test.test | 10 | 0.000798s | 0.000813s | 0.000777s | byte equality observed |
| tricky_rvalue_test.test | 10 | 0.000799s | 0.000791s | 0.000793s | byte equality observed |
| type_identity_test.test | 10 | 0.000783s | 0.000796s | 0.000798s | byte equality observed |
| type_traits_test.test | 10 | 0.000359s | 0.000365s | 0.000762s | byte equality observed |
| type_with_alignment_test.test | 10 | 0.000836s | 0.000834s | 0.000835s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814672).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
