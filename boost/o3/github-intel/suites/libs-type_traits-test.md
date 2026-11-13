# libs/type_traits/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| type_with_alignment_test.test | RUN | 0.114403 | 0.319541 | 0.451240 | 0.001356 | 0.001278 | 0.001321 |
| type_traits_test.test | RUN | 0.155875 | 0.327435 | 0.432739 | 0.000509 | 0.000531 | 0.001226 |
| type_identity_test.test | RUN | 0.295922 | 1.113896 | 2.331854 | 0.000998 | 0.001298 | 0.001286 |
| tricky_rvalue_test.test | RUN | 0.106325 | 0.255341 | 0.340027 | 0.001310 | 0.001397 | 0.001172 |
| tricky_partial_spec_test.test | RUN | 0.125547 | 0.360127 | 0.583514 | 0.001231 | 0.001123 | 0.001235 |
| tricky_is_enum_test.test | RUN | 0.101474 | 0.243487 | 0.316822 | 0.001379 | 0.001388 | 0.001350 |
| tricky_incomplete_type_test.test | RUN | 0.163676 | 0.374628 | 0.530238 | 0.001226 | 0.001254 | 0.001214 |
| tricky_function_type_test.test | RUN | 0.128962 | 0.382667 | 0.605739 | 0.001280 | 0.001111 | 0.001244 |
| tricky_add_pointer_test.test | RUN | 0.199997 | 0.668217 | 1.359118 | 0.001197 | 0.001296 | 0.001303 |
| tricky_abstract_type_test.test | RUN | 0.105116 | 0.245828 | 0.323970 | 0.001231 | 0.001105 | 0.001254 |
| remove_volatile_test.test | RUN | 0.261948 | 0.983882 | 2.134589 | 0.001365 | 0.001424 | 0.001223 |
| remove_reference_test.test | RUN | 0.273069 | 1.031184 | 2.199241 | 0.001291 | 0.001375 | 0.001178 |
| remove_pointer_test.test | RUN | 0.286641 | 1.066603 | 2.321484 | 0.001342 | 0.001194 | 0.001229 |
| remove_extent_test.test | RUN | 0.275754 | 1.051517 | 2.242192 | 0.001303 | 0.001307 | 0.001246 |
| remove_cv_test.test | RUN | 0.265613 | 1.033558 | 2.213682 | 0.001318 | 0.001322 | 0.001270 |
| remove_cv_ref_test.test | RUN | 0.577071 | 2.488204 | 5.424453 | 0.001224 | 0.001202 | 0.001135 |
| remove_const_test.test | RUN | 0.264921 | 0.962694 | 2.093794 | 0.001325 | 0.001260 | 0.001238 |
| remove_bounds_test.test | RUN | 0.279207 | 1.036948 | 2.214280 | 0.001306 | 0.001268 | 0.001195 |
| remove_all_extents_test.test | RUN | 0.274941 | 1.024803 | 2.242287 | 0.001250 | 0.001211 | 0.001030 |
| rank_test.test | RUN | 0.105891 | 0.253699 | 0.336204 | 0.001397 | 0.001343 | 0.001207 |
| promote_mpl_test.test | RUN | 0.091179 | 0.239708 | 0.244360 | 0.000483 | 0.000523 | 0.001118 |
| promote_enum_test.test | RUN | 0.019429 | 0.054953 | 0.057027 | 0.000468 | 0.000580 | 0.001155 |
| promote_enum_msvc_bug_test.test | RUN | 0.017566 | 0.047760 | 0.047076 | 0.000586 | 0.000499 | 0.001107 |
| promote_basic_test.test | RUN | 0.022210 | 0.062943 | 0.056984 | 0.000451 | 0.000480 | 0.001130 |
| negation_test.test | RUN | 0.098560 | 0.230655 | 0.292771 | 0.001420 | 0.001214 | 0.001243 |
| mpl_interop_test3.test | RUN | 0.084350 | 0.227120 | 0.228500 | 0.000537 | 0.000489 | 0.001160 |
| mpl_interop_test2.test | RUN | 0.027957 | 0.060842 | 0.058445 | 0.000552 | 0.000486 | 0.001170 |
| mpl_interop_test1.test | RUN | 0.015840 | 0.041952 | 0.040748 | 0.000487 | 0.000507 | 0.001091 |
| make_void_test.test | RUN | 0.107357 | 0.249648 | 0.352763 | 0.001321 | 0.001313 | 0.001257 |
| make_unsigned_test.test | RUN | 0.122923 | 0.343987 | 0.480708 | 0.001287 | 0.001540 | 0.001230 |
| make_signed_test.test | RUN | 0.122046 | 0.346415 | 0.479023 | 0.001172 | 0.001265 | 0.001182 |
| is_volatile_test.test | RUN | 0.106219 | 0.311300 | 0.388176 | 0.001180 | 0.001205 | 0.001313 |
| is_void_test.test | RUN | 0.107048 | 0.263750 | 0.357348 | 0.001448 | 0.001282 | 0.001265 |
| is_virtual_base_of_test.test | RUN | 0.122768 | 0.336067 | 0.500558 | 0.001319 | 0.001181 | 0.001159 |
| is_unsigned_test.test | RUN | 0.109377 | 0.266906 | 0.372356 | 0.001290 | 0.001295 | 0.001198 |
| is_unscoped_enum_test.test | RUN | 0.103441 | 0.249436 | 0.308932 | 0.001174 | 0.001218 | 0.001244 |
| is_union_test.test | RUN | 0.112953 | 0.301793 | 0.460744 | 0.001293 | 0.001323 | 0.001182 |
| is_unbounded_array_test.test | RUN | 0.104730 | 0.266329 | 0.362221 | 0.001366 | 0.001298 | 0.001063 |
| is_trivially_copyable_test.test | RUN | 0.167960 | 0.539022 | 0.913923 | 0.001364 | 0.001316 | 0.001324 |
| is_swappable_test.test | RUN | 0.137325 | 0.409078 | 0.684816 | 0.001161 | 0.001290 | 0.001314 |
| is_stateless_test.test | RUN | 0.154592 | 0.470880 | 0.761634 | 0.001295 | 0.001148 | 0.001336 |
| is_signed_test.test | RUN | 0.103061 | 0.271463 | 0.380961 | 0.001339 | 0.001258 | 0.001199 |
| is_scoped_enum_test.test | RUN | 0.104472 | 0.240332 | 0.322328 | 0.001335 | 0.001268 | 0.001269 |
| is_scalar_test.test | RUN | 0.138153 | 0.424672 | 0.686434 | 0.001187 | 0.001292 | 0.001205 |
| is_same_test.test | RUN | 0.104710 | 0.259292 | 0.352555 | 0.001287 | 0.001416 | 0.001136 |
| is_rvalue_reference_test.test | RUN | 0.107330 | 0.289961 | 0.405971 | 0.001162 | 0.001199 | 0.001252 |
| is_reference_test.test | RUN | 0.109336 | 0.288616 | 0.421333 | 0.001358 | 0.001303 | 0.001129 |
| is_polymorphic_test.test | RUN | 0.114242 | 0.308193 | 0.449977 | 0.001169 | 0.001330 | 0.001089 |
| is_pointer_test.test | RUN | 0.111314 | 0.298803 | 0.445987 | 0.001405 | 0.001274 | 0.001214 |
| is_pod_test.test | RUN | 0.142287 | 0.477241 | 0.772064 | 0.001206 | 0.001276 | 0.001177 |
| is_object_test.test | RUN | 0.102701 | 0.254602 | 0.328194 | 0.001169 | 0.001355 | 0.001207 |
| is_nothrow_swappable_test.test | RUN | 0.148955 | 0.457810 | 0.744781 | 0.001258 | 0.001311 | 0.001172 |
| is_nothrow_move_constructible_test.test | RUN | 0.143104 | 0.467865 | 0.746149 | 0.001451 | 0.001269 | 0.001192 |
| is_nothrow_move_assignable_test.test | RUN | 0.142963 | 0.473416 | 0.763590 | 0.001188 | 0.001245 | 0.001150 |
| is_noncopyable_test.test | RUN | 0.102371 | 0.256521 | 0.351405 | 0.001225 | 0.001126 | 0.001107 |
| is_member_pointer_test.test | RUN | 0.106186 | 0.259696 | 0.337273 | 0.001211 | 0.001348 | 0.001138 |
| is_member_obj_test.test | RUN | 0.105674 | 0.265267 | 0.358050 | 0.001259 | 0.001538 | 0.001224 |
| is_member_func_test.test | RUN | 0.106170 | 0.279003 | 0.378545 | 0.001143 | 0.001222 | 0.001202 |
| is_lvalue_reference_test.test | RUN | 0.109038 | 0.298987 | 0.405402 | 0.001174 | 0.001260 | 0.001231 |
| is_list_constructible_test.test | RUN | 0.128862 | 0.353217 | 0.462328 | 0.001239 | 0.001452 | 0.001173 |
| is_integral_test.test | RUN | 0.130998 | 0.414829 | 0.642408 | 0.001287 | 0.001342 | 0.001154 |
| is_fundamental_test.test | RUN | 0.131683 | 0.436711 | 0.666652 | 0.001280 | 0.001205 | 0.001213 |
| is_function_test.test | RUN | 0.106845 | 0.297629 | 0.383851 | 0.001316 | 0.001284 | 0.001176 |
| is_floating_point_test.test | RUN | 0.111405 | 0.308482 | 0.422584 | 0.001210 | 0.001334 | 0.001202 |
| is_float_test.test | RUN | 0.110208 | 0.301612 | 0.435021 | 0.001119 | 0.001226 | 0.001254 |
| is_final_test.test | RUN | 0.110144 | 0.299783 | 0.443197 | 0.001294 | 0.001192 | 0.001166 |
| is_enum_test.test | RUN | 0.101808 | 0.257076 | 0.338517 | 0.001325 | 0.001343 | 0.001235 |
| is_empty_test.test | RUN | 0.111113 | 0.272704 | 0.387851 | 0.001176 | 0.001230 | 0.001116 |
| is_detected_test.test | RUN | 0.098604 | 0.234964 | 0.309796 | 0.001198 | 0.001455 | 0.001345 |
| is_detected_exact_test.test | RUN | 0.098798 | 0.241718 | 0.309904 | 0.001174 | 0.001154 | 0.001135 |
| is_destructible_test.test | RUN | 0.142211 | 0.461752 | 0.750756 | 0.001296 | 0.001398 | 0.001243 |
| is_default_constr_test.test | RUN | 0.147686 | 0.476158 | 0.762052 | 0.001194 | 0.001336 | 0.001248 |
| is_copy_constructible_test.test | RUN | 0.150698 | 0.499313 | 0.806138 | 0.001391 | 0.001004 | 0.001226 |
| is_copy_assignable_test.test | RUN | 0.151110 | 0.495723 | 0.805019 | 0.001290 | 0.001225 | 0.001265 |
| is_convertible_test.test | RUN | 0.141637 | 0.446744 | 0.656175 | 0.001431 | 0.001291 | 0.001139 |
| is_constructible_test.test | RUN | 0.117689 | 0.319204 | 0.456090 | 0.001200 | 0.001369 | 0.001183 |
| is_const_test.test | RUN | 0.107260 | 0.276530 | 0.380483 | 0.001344 | 0.001284 | 0.001250 |
| is_compound_test.test | RUN | 0.099888 | 0.246319 | 0.304146 | 0.001301 | 0.001213 | 0.001237 |
| is_complex_test.test | RUN | 0.134764 | 0.320392 | 0.433900 | 0.001176 | 0.001348 | 0.001303 |
| is_complete_test.test | RUN | 0.117949 | 0.329363 | 0.494602 | 0.001231 | 0.001221 | 0.001248 |
| is_class_test.test | RUN | 0.105972 | 0.292938 | 0.416398 | 0.001260 | 0.001252 | 0.001082 |
| is_bounded_array_test.test | RUN | 0.103887 | 0.267694 | 0.345534 | 0.001207 | 0.001326 | 0.001220 |
| is_base_of_test.test | RUN | 0.111992 | 0.285048 | 0.397096 | 0.001181 | 0.001288 | 0.001220 |
| is_base_and_derived_test.test | RUN | 0.109629 | 0.278373 | 0.399443 | 0.001189 | 0.001250 | 0.001243 |
| is_assignable_test.test | RUN | 0.130972 | 0.403981 | 0.604380 | 0.001236 | 0.001208 | 0.001289 |
| is_array_test.test | RUN | 0.107387 | 0.279583 | 0.386570 | 0.001347 | 0.001179 | 0.001167 |
| is_arithmetic_test.test | RUN | 0.130707 | 0.404243 | 0.664669 | 0.001270 | 0.001300 | 0.001286 |
| is_abstract_test.test | RUN | 0.234771 | 1.067967 | 2.335744 | 0.001313 | 0.001401 | 0.001254 |
| integral_constant_test.test | RUN | 0.014970 | 0.043019 | 0.041788 | 0.000492 | 0.000536 | 0.001151 |
| has_virtual_destructor_test.test | RUN | 0.115178 | 0.325432 | 0.479326 | 0.001368 | 0.001302 | 0.001335 |
| has_unary_plus_test.test | RUN | 0.221863 | 1.152391 | 1.710689 | 0.001310 | 0.001364 | 0.001339 |
| has_unary_minus_test.test | RUN | 0.216300 | 1.127312 | 1.679570 | 0.001246 | 0.001303 | 0.001044 |
| has_trivial_move_constructor_test.test | RUN | 0.148196 | 0.483719 | 0.811839 | 0.001366 | 0.001000 | 0.001332 |
| has_trivial_move_assign_test.test | RUN | 0.146570 | 0.481041 | 0.821358 | 0.001229 | 0.001324 | 0.001217 |
| has_trivial_destructor_test.test | RUN | 0.145300 | 0.492174 | 0.817301 | 0.001276 | 0.001252 | 0.001190 |
| has_trivial_copy_test.test | RUN | 0.151680 | 0.522682 | 0.871679 | 0.001325 | 0.001479 | 0.001331 |
| has_trivial_constr_test.test | RUN | 0.152886 | 0.549663 | 0.867662 | 0.001264 | 0.001358 | 0.001206 |
| has_trivial_assign_test.test | RUN | 0.149374 | 0.541112 | 0.829637 | 0.001235 | 0.001378 | 0.001246 |
| has_right_shift_test.test | RUN | 0.230082 | 0.947493 | 1.862783 | 0.001250 | 0.001394 | 0.001187 |
| has_right_shift_assign_test.test | RUN | 0.215955 | 0.876303 | 1.731281 | 0.001243 | 0.001221 | 0.001210 |
| has_prefix_classes3_test.test | RUN | 0.160191 | 0.528220 | 0.877120 | 0.001309 | 0.001480 | 0.001535 |
| has_prefix_classes2_test.test | RUN | 0.162593 | 0.507120 | 0.876923 | 0.001165 | 0.001413 | 0.001192 |
| has_prefix_classes1_test.test | RUN | 0.161161 | 0.530047 | 0.835251 | 0.001164 | 0.001238 | 0.001303 |
| has_prefix_classes0_test.test | RUN | 0.160463 | 0.532885 | 0.860535 | 0.001379 | 0.001221 | 0.001232 |
| has_pre_increment_test.test | RUN | 0.218271 | 1.140372 | 1.678479 | 0.001244 | 0.001198 | 0.001283 |
| has_pre_decrement_test.test | RUN | 0.214996 | 1.157562 | 1.686857 | 0.001210 | 0.001160 | 0.001203 |
| has_postfix_classes3_test.test | RUN | 0.161632 | 0.532049 | 0.884694 | 0.001243 | 0.001380 | 0.001326 |
| has_postfix_classes2_test.test | RUN | 0.161532 | 0.520946 | 0.816947 | 0.001246 | 0.001275 | 0.001216 |
| has_postfix_classes1_test.test | RUN | 0.161145 | 0.533197 | 0.862715 | 0.001379 | 0.001350 | 0.001305 |
| has_postfix_classes0_test.test | RUN | 0.160737 | 0.524984 | 0.852084 | 0.001263 | 0.001240 | 0.001268 |
| has_post_increment_test.test | RUN | 0.213049 | 1.092621 | 1.727351 | 0.001438 | 0.001685 | 0.001491 |
| has_post_decrement_test.test | RUN | 0.214194 | 1.104689 | 1.732547 | 0.001276 | 0.001881 | 0.001313 |
| has_plus_test.test | RUN | 0.221956 | 0.896631 | 1.751530 | 0.001270 | 0.001365 | 0.001330 |
| has_plus_assign_test.test | RUN | 0.214351 | 0.906506 | 1.735612 | 0.001250 | 0.001244 | 0.001183 |
| has_operator_new_test.test | RUN | 0.148324 | 0.469528 | 0.791485 | 0.001277 | 0.001310 | 0.001263 |
| has_nothrow_destructor_test.test | RUN | 0.152881 | 0.513450 | 0.845602 | 0.001256 | 0.001346 | 0.001255 |
| has_nothrow_copy_test.test | RUN | 0.146003 | 0.494289 | 0.792954 | 0.001233 | 0.001304 | 0.001231 |
| has_nothrow_constr_test.test | RUN | 0.143481 | 0.479985 | 0.796587 | 0.001190 | 0.001364 | 0.001251 |
| has_nothrow_assign_test.test | RUN | 0.144017 | 0.474405 | 0.783387 | 0.001333 | 0.001352 | 0.001224 |
| has_not_equal_to_test.test | RUN | 0.220365 | 1.100805 | 1.726897 | 0.001384 | 0.001344 | 0.001224 |
| has_negate_test.test | RUN | 0.210109 | 0.850281 | 1.677786 | 0.001259 | 0.001278 | 0.001317 |
| has_multiplies_test.test | RUN | 0.217445 | 0.880508 | 1.701997 | 0.001237 | 0.001253 | 0.001141 |
| has_multiplies_assign_test.test | RUN | 0.219431 | 0.862500 | 1.669012 | 0.001263 | 0.001359 | 0.001230 |
| has_modulus_test.test | RUN | 0.219204 | 0.879765 | 1.728605 | 0.001430 | 0.001285 | 0.001293 |
| has_modulus_assign_test.test | RUN | 0.217229 | 0.870585 | 1.666175 | 0.001326 | 0.001304 | 0.001310 |
| has_minus_test.test | RUN | 0.227210 | 0.890209 | 1.761275 | 0.000948 | 0.001262 | 0.001249 |
| has_minus_assign_test.test | RUN | 0.224888 | 0.883070 | 1.768488 | 0.001311 | 0.001562 | 0.001120 |
| has_logical_or_test.test | RUN | 0.219014 | 0.922112 | 1.730916 | 0.001363 | 0.001343 | 0.001288 |
| has_logical_not_test.test | RUN | 0.210409 | 0.893471 | 1.636958 | 0.001281 | 0.001316 | 0.001247 |
| has_logical_and_test.test | RUN | 0.218728 | 0.921712 | 1.766966 | 0.001233 | 0.001281 | 0.001220 |
| has_less_test.test | RUN | 0.221182 | 1.107929 | 1.769196 | 0.001300 | 0.001313 | 0.001075 |
| has_less_equal_test.test | RUN | 0.220260 | 1.094394 | 1.721617 | 0.001299 | 0.001356 | 0.001278 |
| has_left_shift_test.test | RUN | 0.238677 | 1.135952 | 2.120868 | 0.001264 | 0.001236 | 0.001347 |
| has_left_shift_assign_test.test | RUN | 0.213473 | 0.887505 | 1.750484 | 0.001143 | 0.001314 | 0.001256 |
| has_greater_test.test | RUN | 0.215889 | 1.090909 | 1.753333 | 0.001375 | 0.001353 | 0.001140 |
| has_greater_equal_test.test | RUN | 0.221244 | 1.089317 | 1.662931 | 0.001508 | 0.001275 | 0.001244 |
| has_equal_to_test.test | RUN | 0.218877 | 1.105813 | 1.677500 | 0.001132 | 0.001180 | 0.001315 |
| has_divides_test.test | RUN | 0.214084 | 0.879732 | 1.754971 | 0.001302 | 0.001396 | 0.001281 |
| has_divides_assign_test.test | RUN | 0.218602 | 0.855592 | 1.751610 | 0.001298 | 0.001347 | 0.001350 |
| has_dereference_test.test | RUN | 0.213081 | 0.861145 | 1.599841 | 0.001290 | 0.001322 | 0.001242 |
| has_complement_test.test | RUN | 0.215038 | 1.119968 | 1.649526 | 0.001396 | 0.001387 | 0.001367 |
| has_bit_xor_test.test | RUN | 0.218891 | 0.886390 | 1.746136 | 0.001085 | 0.001426 | 0.001221 |
| has_bit_xor_assign_test.test | RUN | 0.221267 | 0.878913 | 1.745108 | 0.001290 | 0.001236 | 0.001237 |
| has_bit_or_test.test | RUN | 0.218571 | 0.874595 | 1.649658 | 0.001275 | 0.001301 | 0.001258 |
| has_bit_or_assign_test.test | RUN | 0.216343 | 0.856088 | 1.707617 | 0.001300 | 0.001365 | 0.001258 |
| has_bit_and_test.test | RUN | 0.216922 | 0.863758 | 1.745103 | 0.001101 | 0.001424 | 0.001169 |
| has_bit_and_assign_test.test | RUN | 0.218795 | 0.876005 | 1.691148 | 0.001348 | 0.001406 | 0.001330 |
| has_binary_classes9_test.test | RUN | 0.239598 | 1.032216 | 1.874627 | 0.001456 | 0.001222 | 0.001329 |
| has_binary_classes8_test.test | RUN | 0.235099 | 1.057658 | 1.947118 | 0.001244 | 0.001336 | 0.001300 |
| has_binary_classes7_test.test | RUN | 0.233806 | 1.046166 | 1.965853 | 0.001142 | 0.001319 | 0.001163 |
| has_binary_classes6_test.test | RUN | 0.232325 | 0.998935 | 1.839907 | 0.001280 | 0.001286 | 0.001228 |
| has_binary_classes5_test.test | RUN | 0.240927 | 1.024082 | 1.924950 | 0.001483 | 0.001360 | 0.001307 |
| has_binary_classes4_test.test | RUN | 0.233682 | 1.038321 | 1.926756 | 0.001363 | 0.001398 | 0.001204 |
| has_binary_classes3_test.test | RUN | 0.235404 | 1.044260 | 1.859279 | 0.001163 | 0.001318 | 0.001303 |
| has_binary_classes2_test.test | RUN | 0.239038 | 1.036064 | 1.950169 | 0.001394 | 0.001352 | 0.001275 |
| has_binary_classes1_test.test | RUN | 0.236965 | 1.032563 | 1.888723 | 0.001297 | 0.001222 | 0.001312 |
| has_binary_classes0_test.test | RUN | 0.237528 | 1.044806 | 1.924044 | 0.001354 | 0.001365 | 0.001184 |
| function_traits_test.test | RUN | 0.108394 | 0.283911 | 0.406360 | 0.001282 | 0.001361 | 0.001230 |
| floating_point_promotion_test.test | RUN | 0.106205 | 0.278069 | 0.366987 | 0.001261 | 0.001303 | 0.001216 |
| extent_test.test | RUN | 0.108486 | 0.272687 | 0.400064 | 0.001381 | 0.001387 | 0.001179 |
| enable_if_test.test | RUN | 0.103121 | 0.234943 | 0.281760 | 0.001308 | 0.001187 | 0.001176 |
| disjunction_test.test | RUN | 0.105057 | 0.247233 | 0.342985 | 0.001394 | 0.001310 | 0.001182 |
| detected_test.test | RUN | 0.101961 | 0.233200 | 0.292766 | 0.001248 | 0.001338 | 0.001299 |
| detected_or_test.test | RUN | 0.099475 | 0.252185 | 0.323321 | 0.001257 | 0.001210 | 0.001143 |
| decay_test.test | RUN | 0.265774 | 0.954376 | 1.884055 | 0.001244 | 0.001340 | 0.001309 |
| cxx14_aliases_test.test | RUN | 0.162312 | 0.363291 | 0.486115 | 0.001420 | 0.001323 | 0.001116 |
| copy_reference_test.test | RUN | 0.102700 | 0.245515 | 0.318996 | 0.001271 | 0.001289 | 0.001228 |
| copy_cv_test.test | RUN | 0.106762 | 0.265277 | 0.343321 | 0.001260 | 0.001272 | 0.001066 |
| copy_cv_ref_test.test | RUN | 0.144722 | 0.428102 | 0.781549 | 0.001130 | 0.001321 | 0.001222 |
| conjunction_test.test | RUN | 0.101895 | 0.245484 | 0.318966 | 0.001371 | 0.001366 | 0.001241 |
| conditional_test.test | RUN | 0.103278 | 0.244209 | 0.305747 | 0.001216 | 0.001254 | 0.001226 |
| common_type_test.test | RUN | 0.138948 | 0.376898 | 0.615423 | 0.001334 | 0.001357 | 0.001227 |
| common_type_sfinae_test.test | RUN | 0.102610 | 0.231614 | 0.304447 | 0.001241 | 0.001352 | 0.001295 |
| common_type_sfinae2_test.test | RUN | 0.103335 | 0.251059 | 0.309518 | 0.001179 | 0.001218 | 0.001138 |
| common_type_6_test.test | RUN | 0.106993 | 0.247322 | 0.330198 | 0.001183 | 0.001244 | 0.001210 |
| common_type_5_test.test | RUN | 0.107306 | 0.252912 | 0.326071 | 0.001210 | 0.001396 | 0.001287 |
| common_type_4_test.test | RUN | 0.110429 | 0.274279 | 0.393541 | 0.001359 | 0.001140 | 0.001202 |
| common_type_3_test.test | RUN | 0.100423 | 0.229530 | 0.295847 | 0.001362 | 0.001320 | 0.001155 |
| common_type_2_test.test | RUN | 0.136360 | 0.373486 | 0.613068 | 0.001336 | 0.001278 | 0.001370 |
| alignment_of_test.test | RUN | 0.111360 | 0.297042 | 0.411544 | 0.001335 | 0.001366 | 0.001191 |
| alignment_of_a2_test.test | RUN | 0.109452 | 0.299610 | 0.406534 | 0.001291 | 0.001187 | 0.001189 |
| aligned_storage_test.test | RUN | 0.114864 | 0.294222 | 0.376434 | 0.001322 | 0.001374 | 0.001433 |
| aligned_storage_empy_test.test | RUN | 0.128273 | 0.322183 | 0.502330 | 0.001254 | 0.001310 | 0.001191 |
| aligned_storage_a2_test.test | RUN | 0.113915 | 0.287067 | 0.372332 | 0.001329 | 0.001527 | 0.001382 |
| add_volatile_test.test | RUN | 0.239331 | 0.855384 | 1.783099 | 0.001457 | 0.001325 | 0.001308 |
| add_rvalue_reference_test.test | RUN | 0.272161 | 0.968645 | 2.027043 | 0.001332 | 0.001009 | 0.001276 |
| add_reference_test.test | RUN | 0.261204 | 0.941134 | 2.001712 | 0.001263 | 0.001352 | 0.001327 |
| add_pointer_test.test | RUN | 0.165223 | 0.543754 | 1.044420 | 0.001212 | 0.001246 | 0.001283 |
| add_lvalue_reference_test.test | RUN | 0.261224 | 0.947667 | 1.969289 | 0.001278 | 0.001327 | 0.001186 |
| add_cv_test.test | RUN | 0.245058 | 0.889796 | 1.885418 | 0.001217 | 0.001309 | 0.001332 |
| add_const_test.test | RUN | 0.248199 | 0.881702 | 1.879660 | 0.001283 | 0.001320 | 0.001213 |
| is_virtual_base_of_fail.test | COMPILE_FAIL | 0.095484 | 0.229713 | 0.296606 | — | — | — |
| is_stateless_fail.test | COMPILE_FAIL | 0.101077 | 0.229588 | 0.304570 | — | — | — |
| is_polymorphic_fail.test | COMPILE_FAIL | 0.095159 | 0.219806 | 0.290025 | — | — | — |
| is_nothrow_move_constructible_fail.test | COMPILE_FAIL | 0.096570 | 0.223313 | 0.277983 | — | — | — |
| is_nothrow_move_assignable_fail.test | COMPILE_FAIL | 0.095574 | 0.224884 | 0.274727 | — | — | — |
| is_list_constructible_fail.test | COMPILE_FAIL | 0.095300 | 0.222450 | 0.278684 | — | — | — |
| is_empty_fail.test | COMPILE_FAIL | 0.097729 | 0.223656 | 0.283616 | — | — | — |
| is_destructible_fail.test | COMPILE_FAIL | 0.095919 | 0.223306 | 0.282227 | — | — | — |
| is_default_constructible_fail.test | COMPILE_FAIL | 0.095006 | 0.223530 | 0.285933 | — | — | — |
| is_copy_constructible_fail.test | COMPILE_FAIL | 0.095507 | 0.223730 | 0.285722 | — | — | — |
| is_copy_assignable_fail.test | COMPILE_FAIL | 0.097788 | 0.221665 | 0.282770 | — | — | — |
| is_convertible_fail.test | COMPILE_FAIL | 0.096181 | 0.227003 | 0.281421 | — | — | — |
| is_convertible2_fail.test | COMPILE_FAIL | 0.095648 | 0.218897 | 0.282207 | — | — | — |
| is_constructible_fail.test | COMPILE_FAIL | 0.096655 | 0.226655 | 0.284927 | — | — | — |
| is_base_of_fail.test | COMPILE_FAIL | 0.095954 | 0.225439 | 0.280859 | — | — | — |
| is_assignable_fail.test | COMPILE_FAIL | 0.095221 | 0.219071 | 0.281723 | — | — | — |
| has_nothrow_destruct_fail.test | COMPILE_FAIL | 0.097057 | 0.220980 | 0.283572 | — | — | — |
| has_nothrow_copy_fail.test | COMPILE_FAIL | 0.096651 | 0.227913 | 0.288749 | — | — | — |
| has_nothrow_cons_fail.test | COMPILE_FAIL | 0.095537 | 0.222465 | 0.284128 | — | — | — |
| has_nothrow_assign_fail.test | COMPILE_FAIL | 0.096061 | 0.220644 | 0.283272 | — | — | — |
| common_type_fail.test | COMPILE_FAIL | 0.096176 | 0.225131 | 0.284972 | — | — | — |
| common_type2_fail.test | COMPILE_FAIL | 0.095331 | 0.218402 | 0.282724 | — | — | — |
| has_nothrow_assign_test_no_intrinsics.test | RUN | 0.153576 | 0.479800 | 0.809926 | 0.001358 | 0.001423 | 0.001320 |
| has_nothrow_constr_test_no_intrinsics.test | RUN | 0.148759 | 0.478911 | 0.800357 | 0.001371 | 0.001283 | 0.001259 |
| has_nothrow_copy_test_no_intrinsics.test | RUN | 0.152672 | 0.482162 | 0.810433 | 0.001320 | 0.001278 | 0.000924 |
| is_nothrow_move_assignable_test_no_intrinsics.test | RUN | 0.152227 | 0.481391 | 0.769482 | 0.000950 | 0.001353 | 0.000863 |
| is_nothrow_move_constructible_test_no_intrinsics.test | RUN | 0.091262 | 0.327123 | 0.458423 | 0.001007 | 0.000927 | 0.000912 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.972× | 0.987×, 0.957× | 0.997× |
| clang | 1.047× | 1.047×, 1.048× | 0.994× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345325).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| add_const_test.test | 10 | 0.000846s | 0.000844s | 0.000840s | byte equality observed |
| add_cv_test.test | 10 | 0.000886s | 0.000855s | 0.000867s | byte equality observed |
| add_lvalue_reference_test.test | 10 | 0.000879s | 0.000893s | 0.000866s | byte equality observed |
| add_pointer_test.test | 10 | 0.000873s | 0.000863s | 0.000899s | byte equality observed |
| add_reference_test.test | 10 | 0.000869s | 0.000903s | 0.000877s | byte equality observed |
| add_rvalue_reference_test.test | 10 | 0.000879s | 0.000852s | 0.000861s | byte equality observed |
| add_volatile_test.test | 10 | 0.000880s | 0.000881s | 0.000879s | byte equality observed |
| aligned_storage_a2_test.test | 10 | 0.000940s | 0.000938s | 0.000924s | byte equality observed |
| aligned_storage_empy_test.test | 10 | 0.000868s | 0.000860s | 0.000875s | byte equality observed |
| aligned_storage_test.test | 10 | 0.000941s | 0.000934s | 0.000925s | byte equality observed |
| alignment_of_a2_test.test | 10 | 0.000877s | 0.000861s | 0.000873s | byte equality observed |
| alignment_of_test.test | 10 | 0.000868s | 0.000873s | 0.000872s | byte equality observed |
| common_type_2_test.test | 10 | 0.000868s | 0.000878s | 0.000883s | byte equality observed |
| common_type_3_test.test | 10 | 0.000892s | 0.000856s | 0.000859s | byte equality observed |
| common_type_4_test.test | 10 | 0.000854s | 0.000879s | 0.000859s | byte equality observed |
| common_type_5_test.test | 10 | 0.000876s | 0.000865s | 0.000890s | byte equality observed |
| common_type_6_test.test | 10 | 0.000874s | 0.000848s | 0.000869s | byte equality observed |
| common_type_sfinae2_test.test | 10 | 0.000876s | 0.000873s | 0.000852s | byte equality observed |
| common_type_sfinae_test.test | 10 | 0.000858s | 0.000845s | 0.000874s | byte equality observed |
| common_type_test.test | 10 | 0.000861s | 0.000849s | 0.000864s | byte equality observed |
| conditional_test.test | 10 | 0.000860s | 0.000847s | 0.000891s | byte equality observed |
| conjunction_test.test | 10 | 0.000864s | 0.000848s | 0.000854s | byte equality observed |
| copy_cv_ref_test.test | 10 | 0.000878s | 0.000865s | 0.000883s | byte equality observed |
| copy_cv_test.test | 10 | 0.000866s | 0.000845s | 0.000879s | byte equality observed |
| copy_reference_test.test | 10 | 0.000852s | 0.000840s | 0.000878s | byte equality observed |
| cxx14_aliases_test.test | 10 | 0.000862s | 0.000895s | 0.000845s | byte equality observed |
| decay_test.test | 10 | 0.000855s | 0.000839s | 0.000860s | byte equality observed |
| detected_or_test.test | 10 | 0.000864s | 0.000847s | 0.000858s | byte equality observed |
| detected_test.test | 10 | 0.000864s | 0.000876s | 0.000867s | byte equality observed |
| disjunction_test.test | 10 | 0.000857s | 0.000843s | 0.000860s | byte equality observed |
| enable_if_test.test | 10 | 0.000871s | 0.000831s | 0.000840s | byte equality observed |
| extent_test.test | 10 | 0.000861s | 0.000849s | 0.000878s | byte equality observed |
| floating_point_promotion_test.test | 10 | 0.000846s | 0.000861s | 0.000861s | byte equality observed |
| function_traits_test.test | 10 | 0.000859s | 0.000836s | 0.000856s | byte equality observed |
| has_binary_classes0_test.test | 10 | 0.000856s | 0.000874s | 0.000876s | byte equality observed |
| has_binary_classes1_test.test | 10 | 0.000851s | 0.000851s | 0.000850s | byte equality observed |
| has_binary_classes2_test.test | 10 | 0.000871s | 0.000882s | 0.000832s | byte equality observed |
| has_binary_classes3_test.test | 10 | 0.000853s | 0.000870s | 0.000844s | byte equality observed |
| has_binary_classes4_test.test | 10 | 0.000846s | 0.000847s | 0.000855s | byte equality observed |
| has_binary_classes5_test.test | 10 | 0.000864s | 0.000850s | 0.000854s | byte equality observed |
| has_binary_classes6_test.test | 10 | 0.000841s | 0.000851s | 0.000846s | byte equality observed |
| has_binary_classes7_test.test | 10 | 0.000872s | 0.000849s | 0.000839s | byte equality observed |
| has_binary_classes8_test.test | 10 | 0.000857s | 0.000846s | 0.000868s | byte equality observed |
| has_binary_classes9_test.test | 10 | 0.000876s | 0.000861s | 0.000857s | byte equality observed |
| has_bit_and_assign_test.test | 10 | 0.000863s | 0.000840s | 0.000832s | byte equality observed |
| has_bit_and_test.test | 10 | 0.000867s | 0.000847s | 0.000867s | byte equality observed |
| has_bit_or_assign_test.test | 10 | 0.000853s | 0.000856s | 0.000864s | byte equality observed |
| has_bit_or_test.test | 10 | 0.000843s | 0.000848s | 0.000864s | byte equality observed |
| has_bit_xor_assign_test.test | 10 | 0.000879s | 0.000873s | 0.000851s | byte equality observed |
| has_bit_xor_test.test | 10 | 0.000856s | 0.000836s | 0.000900s | byte equality observed |
| has_complement_test.test | 10 | 0.000858s | 0.000848s | 0.000850s | byte equality observed |
| has_dereference_test.test | 10 | 0.000852s | 0.000830s | 0.000840s | byte equality observed |
| has_divides_assign_test.test | 10 | 0.000841s | 0.000847s | 0.000870s | byte equality observed |
| has_divides_test.test | 10 | 0.000873s | 0.000861s | 0.000856s | byte equality observed |
| has_equal_to_test.test | 10 | 0.000836s | 0.000862s | 0.000871s | byte equality observed |
| has_greater_equal_test.test | 10 | 0.000855s | 0.000862s | 0.000892s | byte equality observed |
| has_greater_test.test | 10 | 0.000875s | 0.000833s | 0.000880s | byte equality observed |
| has_left_shift_assign_test.test | 10 | 0.000844s | 0.000860s | 0.000840s | byte equality observed |
| has_left_shift_test.test | 10 | 0.000846s | 0.000836s | 0.000839s | byte equality observed |
| has_less_equal_test.test | 10 | 0.000845s | 0.000850s | 0.000887s | byte equality observed |
| has_less_test.test | 10 | 0.000862s | 0.000857s | 0.000869s | byte equality observed |
| has_logical_and_test.test | 10 | 0.000853s | 0.000831s | 0.000847s | byte equality observed |
| has_logical_not_test.test | 10 | 0.000823s | 0.000848s | 0.000857s | byte equality observed |
| has_logical_or_test.test | 10 | 0.000859s | 0.000879s | 0.000852s | byte equality observed |
| has_minus_assign_test.test | 10 | 0.000849s | 0.000853s | 0.000839s | byte equality observed |
| has_minus_test.test | 10 | 0.000856s | 0.000871s | 0.000843s | byte equality observed |
| has_modulus_assign_test.test | 10 | 0.000856s | 0.000828s | 0.000867s | byte equality observed |
| has_modulus_test.test | 10 | 0.000871s | 0.000835s | 0.000847s | byte equality observed |
| has_multiplies_assign_test.test | 10 | 0.000832s | 0.000877s | 0.000843s | byte equality observed |
| has_multiplies_test.test | 10 | 0.000857s | 0.000846s | 0.000840s | byte equality observed |
| has_negate_test.test | 10 | 0.000850s | 0.000842s | 0.000854s | byte equality observed |
| has_not_equal_to_test.test | 10 | 0.000871s | 0.000835s | 0.000856s | byte equality observed |
| has_nothrow_assign_test.test | 10 | 0.000856s | 0.000876s | 0.000832s | byte equality observed |
| has_nothrow_assign_test_no_intrinsics.test | 10 | 0.000846s | 0.000854s | 0.000851s | byte equality observed |
| has_nothrow_constr_test.test | 10 | 0.000873s | 0.000828s | 0.000879s | byte equality observed |
| has_nothrow_constr_test_no_intrinsics.test | 10 | 0.000845s | 0.000867s | 0.000842s | byte equality observed |
| has_nothrow_copy_test.test | 10 | 0.000864s | 0.000873s | 0.000874s | byte equality observed |
| has_nothrow_copy_test_no_intrinsics.test | 10 | 0.000850s | 0.000878s | 0.000876s | byte equality observed |
| has_nothrow_destructor_test.test | 10 | 0.000836s | 0.000864s | 0.000860s | byte equality observed |
| has_operator_new_test.test | 10 | 0.000838s | 0.000844s | 0.000844s | byte equality observed |
| has_plus_assign_test.test | 10 | 0.000869s | 0.000870s | 0.000869s | byte equality observed |
| has_plus_test.test | 10 | 0.000858s | 0.000828s | 0.000865s | byte equality observed |
| has_post_decrement_test.test | 10 | 0.000850s | 0.000851s | 0.000846s | byte equality observed |
| has_post_increment_test.test | 10 | 0.000883s | 0.000852s | 0.000867s | byte equality observed |
| has_postfix_classes0_test.test | 10 | 0.000862s | 0.000858s | 0.000843s | byte equality observed |
| has_postfix_classes1_test.test | 10 | 0.000870s | 0.000851s | 0.000843s | byte equality observed |
| has_postfix_classes2_test.test | 10 | 0.000846s | 0.000854s | 0.000864s | byte equality observed |
| has_postfix_classes3_test.test | 10 | 0.000851s | 0.000837s | 0.000853s | byte equality observed |
| has_pre_decrement_test.test | 10 | 0.000845s | 0.000851s | 0.000864s | byte equality observed |
| has_pre_increment_test.test | 10 | 0.000837s | 0.000847s | 0.000874s | byte equality observed |
| has_prefix_classes0_test.test | 10 | 0.000857s | 0.000855s | 0.000877s | byte equality observed |
| has_prefix_classes1_test.test | 10 | 0.000895s | 0.000843s | 0.000839s | byte equality observed |
| has_prefix_classes2_test.test | 10 | 0.000846s | 0.000846s | 0.000887s | byte equality observed |
| has_prefix_classes3_test.test | 10 | 0.000850s | 0.000847s | 0.000876s | byte equality observed |
| has_right_shift_assign_test.test | 10 | 0.000871s | 0.000848s | 0.000878s | byte equality observed |
| has_right_shift_test.test | 10 | 0.000844s | 0.000835s | 0.000853s | byte equality observed |
| has_trivial_assign_test.test | 10 | 0.000887s | 0.000884s | 0.000906s | byte equality observed |
| has_trivial_constr_test.test | 10 | 0.000925s | 0.000877s | 0.000903s | native test oracle; output varies |
| has_trivial_copy_test.test | 10 | 0.000899s | 0.000912s | 0.000901s | native test oracle; output varies |
| has_trivial_destructor_test.test | 10 | 0.000876s | 0.000870s | 0.000861s | byte equality observed |
| has_trivial_move_assign_test.test | 10 | 0.000868s | 0.000850s | 0.000873s | byte equality observed |
| has_trivial_move_constructor_test.test | 10 | 0.000885s | 0.000844s | 0.000861s | byte equality observed |
| has_unary_minus_test.test | 10 | 0.000869s | 0.000856s | 0.000869s | byte equality observed |
| has_unary_plus_test.test | 10 | 0.000845s | 0.000840s | 0.000860s | byte equality observed |
| has_virtual_destructor_test.test | 10 | 0.000897s | 0.000852s | 0.000836s | byte equality observed |
| integral_constant_test.test | 10 | 0.000392s | 0.000394s | 0.000891s | byte equality observed |
| is_abstract_test.test | 10 | 0.000872s | 0.000880s | 0.000859s | byte equality observed |
| is_arithmetic_test.test | 10 | 0.000847s | 0.000847s | 0.000872s | byte equality observed |
| is_array_test.test | 10 | 0.000852s | 0.000839s | 0.000874s | byte equality observed |
| is_assignable_test.test | 10 | 0.000879s | 0.000852s | 0.000839s | byte equality observed |
| is_base_and_derived_test.test | 10 | 0.000853s | 0.000864s | 0.000838s | byte equality observed |
| is_base_of_test.test | 10 | 0.000874s | 0.000875s | 0.000843s | byte equality observed |
| is_bounded_array_test.test | 10 | 0.000844s | 0.000855s | 0.000899s | byte equality observed |
| is_class_test.test | 10 | 0.000883s | 0.000873s | 0.000829s | byte equality observed |
| is_complete_test.test | 10 | 0.000851s | 0.000843s | 0.000871s | byte equality observed |
| is_complex_test.test | 10 | 0.000876s | 0.000869s | 0.000859s | byte equality observed |
| is_compound_test.test | 10 | 0.000877s | 0.000848s | 0.000856s | byte equality observed |
| is_const_test.test | 10 | 0.000868s | 0.000833s | 0.000846s | byte equality observed |
| is_constructible_test.test | 10 | 0.000857s | 0.000835s | 0.000831s | byte equality observed |
| is_convertible_test.test | 10 | 0.000875s | 0.000866s | 0.000887s | byte equality observed |
| is_copy_assignable_test.test | 5 | 0.000832s | 0.000828s | 0.000884s | byte equality observed |
| is_copy_constructible_test.test | 10 | 0.000868s | 0.000846s | 0.000878s | byte equality observed |
| is_default_constr_test.test | 10 | 0.000854s | 0.000863s | 0.000873s | byte equality observed |
| is_destructible_test.test | 10 | 0.000859s | 0.000844s | 0.000895s | byte equality observed |
| is_detected_exact_test.test | 10 | 0.000859s | 0.000906s | 0.000861s | byte equality observed |
| is_detected_test.test | 10 | 0.000860s | 0.000831s | 0.000884s | byte equality observed |
| is_empty_test.test | 10 | 0.000848s | 0.000826s | 0.000855s | byte equality observed |
| is_enum_test.test | 10 | 0.000886s | 0.000857s | 0.000864s | byte equality observed |
| is_final_test.test | 10 | 0.000885s | 0.000852s | 0.000845s | byte equality observed |
| is_float_test.test | 10 | 0.000867s | 0.000835s | 0.000842s | byte equality observed |
| is_floating_point_test.test | 10 | 0.000858s | 0.000831s | 0.000862s | byte equality observed |
| is_function_test.test | 10 | 0.000859s | 0.000824s | 0.000841s | byte equality observed |
| is_fundamental_test.test | 10 | 0.000884s | 0.000835s | 0.000892s | byte equality observed |
| is_integral_test.test | 10 | 0.000863s | 0.000852s | 0.000843s | byte equality observed |
| is_list_constructible_test.test | 10 | 0.000853s | 0.000852s | 0.000862s | byte equality observed |
| is_lvalue_reference_test.test | 10 | 0.000848s | 0.000871s | 0.000862s | byte equality observed |
| is_member_func_test.test | 10 | 0.000863s | 0.000862s | 0.000851s | byte equality observed |
| is_member_obj_test.test | 10 | 0.000860s | 0.000839s | 0.000836s | byte equality observed |
| is_member_pointer_test.test | 10 | 0.000856s | 0.000838s | 0.000840s | byte equality observed |
| is_noncopyable_test.test | 10 | 0.000850s | 0.000883s | 0.000856s | byte equality observed |
| is_nothrow_move_assignable_test.test | 10 | 0.000850s | 0.000822s | 0.000843s | byte equality observed |
| is_nothrow_move_assignable_test_no_intrinsics.test | 10 | 0.000888s | 0.000868s | 0.000835s | byte equality observed |
| is_nothrow_move_constructible_test.test | 10 | 0.000833s | 0.000838s | 0.000860s | byte equality observed |
| is_nothrow_move_constructible_test_no_intrinsics.test | 10 | 0.000849s | 0.000846s | 0.000839s | byte equality observed |
| is_nothrow_swappable_test.test | 10 | 0.000859s | 0.000845s | 0.000890s | byte equality observed |
| is_object_test.test | 10 | 0.000869s | 0.000855s | 0.000886s | byte equality observed |
| is_pod_test.test | 10 | 0.000844s | 0.000848s | 0.000847s | byte equality observed |
| is_pointer_test.test | 10 | 0.000854s | 0.000838s | 0.000882s | byte equality observed |
| is_polymorphic_test.test | 10 | 0.000884s | 0.000870s | 0.000865s | byte equality observed |
| is_reference_test.test | 10 | 0.000863s | 0.000866s | 0.000892s | byte equality observed |
| is_rvalue_reference_test.test | 10 | 0.000865s | 0.000856s | 0.000865s | byte equality observed |
| is_same_test.test | 10 | 0.000873s | 0.000863s | 0.000865s | byte equality observed |
| is_scalar_test.test | 10 | 0.000865s | 0.000855s | 0.000857s | byte equality observed |
| is_scoped_enum_test.test | 10 | 0.000865s | 0.000884s | 0.000932s | byte equality observed |
| is_signed_test.test | 10 | 0.000859s | 0.000841s | 0.000894s | byte equality observed |
| is_stateless_test.test | 10 | 0.000861s | 0.000844s | 0.000931s | byte equality observed |
| is_swappable_test.test | 10 | 0.000856s | 0.000854s | 0.000883s | byte equality observed |
| is_trivially_copyable_test.test | 10 | 0.000870s | 0.000881s | 0.000956s | byte equality observed |
| is_unbounded_array_test.test | 10 | 0.000876s | 0.000845s | 0.000916s | byte equality observed |
| is_union_test.test | 10 | 0.000846s | 0.000840s | 0.000959s | byte equality observed |
| is_unscoped_enum_test.test | 10 | 0.000848s | 0.000845s | 0.000949s | byte equality observed |
| is_unsigned_test.test | 10 | 0.000879s | 0.000824s | 0.000932s | byte equality observed |
| is_virtual_base_of_test.test | 10 | 0.000852s | 0.000844s | 0.000926s | byte equality observed |
| is_void_test.test | 10 | 0.000854s | 0.000862s | 0.000998s | byte equality observed |
| is_volatile_test.test | 10 | 0.000865s | 0.000847s | 0.000964s | byte equality observed |
| make_signed_test.test | 10 | 0.000851s | 0.000861s | 0.000978s | byte equality observed |
| make_unsigned_test.test | 10 | 0.000870s | 0.000819s | 0.000984s | byte equality observed |
| make_void_test.test | 10 | 0.000869s | 0.000875s | 0.000949s | byte equality observed |
| mpl_interop_test1.test | 10 | 0.000393s | 0.000385s | 0.000937s | byte equality observed |
| mpl_interop_test2.test | 10 | 0.000394s | 0.000385s | 0.000938s | byte equality observed |
| mpl_interop_test3.test | 10 | 0.000390s | 0.000386s | 0.000930s | byte equality observed |
| negation_test.test | 10 | 0.000891s | 0.000875s | 0.000947s | byte equality observed |
| promote_basic_test.test | 10 | 0.000380s | 0.000375s | 0.000937s | byte equality observed |
| promote_enum_msvc_bug_test.test | 10 | 0.000386s | 0.000398s | 0.000910s | byte equality observed |
| promote_enum_test.test | 10 | 0.000391s | 0.000394s | 0.000927s | byte equality observed |
| promote_mpl_test.test | 10 | 0.000388s | 0.000379s | 0.000923s | byte equality observed |
| rank_test.test | 10 | 0.000870s | 0.000853s | 0.000928s | byte equality observed |
| remove_all_extents_test.test | 10 | 0.000857s | 0.000843s | 0.000942s | byte equality observed |
| remove_bounds_test.test | 10 | 0.000869s | 0.000861s | 0.000947s | byte equality observed |
| remove_const_test.test | 10 | 0.000858s | 0.000858s | 0.000949s | byte equality observed |
| remove_cv_ref_test.test | 10 | 0.000897s | 0.000853s | 0.000912s | byte equality observed |
| remove_cv_test.test | 10 | 0.000878s | 0.000840s | 0.000927s | byte equality observed |
| remove_extent_test.test | 10 | 0.000888s | 0.000833s | 0.000929s | byte equality observed |
| remove_pointer_test.test | 10 | 0.000861s | 0.000869s | 0.000942s | byte equality observed |
| remove_reference_test.test | 10 | 0.000869s | 0.000838s | 0.000944s | byte equality observed |
| remove_volatile_test.test | 10 | 0.000878s | 0.000864s | 0.000949s | byte equality observed |
| tricky_abstract_type_test.test | 10 | 0.000867s | 0.000857s | 0.000949s | byte equality observed |
| tricky_add_pointer_test.test | 10 | 0.000871s | 0.000829s | 0.000933s | byte equality observed |
| tricky_function_type_test.test | 10 | 0.000856s | 0.000838s | 0.000930s | byte equality observed |
| tricky_incomplete_type_test.test | 10 | 0.000853s | 0.000894s | 0.000923s | byte equality observed |
| tricky_is_enum_test.test | 10 | 0.000875s | 0.000844s | 0.000920s | byte equality observed |
| tricky_partial_spec_test.test | 10 | 0.000847s | 0.000835s | 0.000933s | byte equality observed |
| tricky_rvalue_test.test | 10 | 0.000913s | 0.000845s | 0.000921s | byte equality observed |
| type_identity_test.test | 10 | 0.000901s | 0.000842s | 0.000922s | byte equality observed |
| type_traits_test.test | 10 | 0.000395s | 0.000382s | 0.000951s | byte equality observed |
| type_with_alignment_test.test | 10 | 0.000938s | 0.000944s | 0.001027s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345325).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
