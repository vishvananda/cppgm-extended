# libs/type_traits/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| type_with_alignment_test.test | RUN | 0.154362 | 0.536032 | 0.618102 | 0.001980 | 0.001866 | 0.002388 |
| type_traits_test.test | RUN | 0.200313 | 0.429337 | 0.639190 | 0.000656 | 0.000695 | 0.001808 |
| type_identity_test.test | RUN | 0.412323 | 1.697770 | 3.621393 | 0.001672 | 0.001584 | 0.001498 |
| tricky_rvalue_test.test | RUN | 0.137109 | 0.335378 | 0.477754 | 0.001732 | 0.001684 | 0.001465 |
| tricky_partial_spec_test.test | RUN | 0.178589 | 0.491756 | 0.911075 | 0.001725 | 0.001634 | 0.001672 |
| tricky_is_enum_test.test | RUN | 0.132579 | 0.326348 | 0.470697 | 0.002080 | 0.002040 | 0.001490 |
| tricky_incomplete_type_test.test | RUN | 0.210729 | 0.477366 | 0.824412 | 0.001652 | 0.001986 | 0.002443 |
| tricky_function_type_test.test | RUN | 0.183395 | 0.531851 | 0.853297 | 0.001866 | 0.001525 | 0.002226 |
| tricky_add_pointer_test.test | RUN | 0.251962 | 0.948932 | 2.002938 | 0.001898 | 0.001936 | 0.001902 |
| tricky_abstract_type_test.test | RUN | 0.135068 | 0.323135 | 0.459453 | 0.001676 | 0.001714 | 0.001857 |
| remove_volatile_test.test | RUN | 0.333291 | 1.358031 | 3.152094 | 0.001588 | 0.001890 | 0.001894 |
| remove_reference_test.test | RUN | 0.369613 | 1.430049 | 3.376454 | 0.001682 | 0.002207 | 0.001826 |
| remove_pointer_test.test | RUN | 0.360489 | 1.504420 | 3.488431 | 0.001694 | 0.001650 | 0.001808 |
| remove_extent_test.test | RUN | 0.365592 | 1.647467 | 3.197250 | 0.001530 | 0.001683 | 0.001376 |
| remove_cv_test.test | RUN | 0.338541 | 1.437441 | 3.192303 | 0.001900 | 0.001906 | 0.001875 |
| remove_cv_ref_test.test | RUN | 0.726184 | 3.735925 | 8.168335 | 0.001440 | 0.001798 | 0.002046 |
| remove_const_test.test | RUN | 0.339258 | 1.359884 | 3.171205 | 0.001988 | 0.001739 | 0.002196 |
| remove_bounds_test.test | RUN | 0.355430 | 1.500543 | 3.426391 | 0.002045 | 0.001984 | 0.001767 |
| remove_all_extents_test.test | RUN | 0.352069 | 1.495046 | 3.142172 | 0.001794 | 0.001680 | 0.001720 |
| rank_test.test | RUN | 0.140024 | 0.328635 | 0.495828 | 0.001760 | 0.001908 | 0.001944 |
| promote_mpl_test.test | RUN | 0.133364 | 0.279167 | 0.315398 | 0.000603 | 0.000737 | 0.001773 |
| promote_enum_test.test | RUN | 0.029159 | 0.070193 | 0.085597 | 0.000567 | 0.000673 | 0.002184 |
| promote_enum_msvc_bug_test.test | RUN | 0.029037 | 0.058588 | 0.067686 | 0.000611 | 0.000616 | 0.001892 |
| promote_basic_test.test | RUN | 0.029886 | 0.080635 | 0.091385 | 0.000725 | 0.000636 | 0.001597 |
| negation_test.test | RUN | 0.139342 | 0.317185 | 0.458988 | 0.001801 | 0.001826 | 0.001847 |
| mpl_interop_test3.test | RUN | 0.120172 | 0.267794 | 0.428443 | 0.000733 | 0.000876 | 0.002153 |
| mpl_interop_test2.test | RUN | 0.047150 | 0.074617 | 0.086363 | 0.000702 | 0.000664 | 0.002686 |
| mpl_interop_test1.test | RUN | 0.021017 | 0.061929 | 0.061915 | 0.000691 | 0.000677 | 0.001773 |
| make_void_test.test | RUN | 0.141036 | 0.349557 | 0.528254 | 0.001386 | 0.002272 | 0.002270 |
| make_unsigned_test.test | RUN | 0.189952 | 0.454017 | 0.742491 | 0.001656 | 0.001676 | 0.001945 |
| make_signed_test.test | RUN | 0.177074 | 0.477075 | 0.723223 | 0.001694 | 0.001774 | 0.001680 |
| is_volatile_test.test | RUN | 0.149020 | 0.379697 | 0.584399 | 0.001607 | 0.002020 | 0.001766 |
| is_void_test.test | RUN | 0.148529 | 0.368519 | 0.526504 | 0.001676 | 0.001955 | 0.001772 |
| is_virtual_base_of_test.test | RUN | 0.179295 | 0.492076 | 0.733275 | 0.001742 | 0.001528 | 0.001884 |
| is_unsigned_test.test | RUN | 0.165315 | 0.370937 | 0.562904 | 0.001778 | 0.001682 | 0.002018 |
| is_unscoped_enum_test.test | RUN | 0.149330 | 0.323191 | 0.478647 | 0.001751 | 0.002428 | 0.001854 |
| is_union_test.test | RUN | 0.155556 | 0.423775 | 0.693203 | 0.001634 | 0.001722 | 0.001562 |
| is_unbounded_array_test.test | RUN | 0.149451 | 0.352576 | 0.523572 | 0.001635 | 0.001491 | 0.002092 |
| is_trivially_copyable_test.test | RUN | 0.247198 | 0.799160 | 1.381504 | 0.002134 | 0.001684 | 0.002827 |
| is_swappable_test.test | RUN | 0.198553 | 0.565456 | 0.969102 | 0.001663 | 0.001679 | 0.001543 |
| is_stateless_test.test | RUN | 0.235313 | 0.697722 | 1.174175 | 0.001627 | 0.002004 | 0.001687 |
| is_signed_test.test | RUN | 0.178411 | 0.383683 | 0.548777 | 0.002052 | 0.002272 | 0.001516 |
| is_scoped_enum_test.test | RUN | 0.145174 | 0.329162 | 0.479074 | 0.001711 | 0.001797 | 0.001773 |
| is_scalar_test.test | RUN | 0.202408 | 0.599488 | 1.101985 | 0.001696 | 0.001865 | 0.001629 |
| is_same_test.test | RUN | 0.149087 | 0.354802 | 0.537717 | 0.001719 | 0.001988 | 0.002836 |
| is_rvalue_reference_test.test | RUN | 0.174458 | 0.409350 | 0.622807 | 0.001833 | 0.001594 | 0.001577 |
| is_reference_test.test | RUN | 0.153098 | 0.402181 | 0.603350 | 0.001706 | 0.001609 | 0.001680 |
| is_polymorphic_test.test | RUN | 0.159930 | 0.434600 | 0.659900 | 0.001628 | 0.001380 | 0.001884 |
| is_pointer_test.test | RUN | 0.172267 | 0.413125 | 0.652587 | 0.001570 | 0.001583 | 0.001774 |
| is_pod_test.test | RUN | 0.212629 | 0.690584 | 1.173765 | 0.002084 | 0.001718 | 0.001355 |
| is_object_test.test | RUN | 0.143827 | 0.340547 | 0.488603 | 0.001775 | 0.001549 | 0.002100 |
| is_nothrow_swappable_test.test | RUN | 0.222933 | 0.638072 | 1.129428 | 0.001940 | 0.001828 | 0.001710 |
| is_nothrow_move_constructible_test.test | RUN | 0.214576 | 0.650839 | 1.130712 | 0.001831 | 0.001984 | 0.001388 |
| is_nothrow_move_assignable_test.test | RUN | 0.202885 | 0.676229 | 1.139154 | 0.001684 | 0.001905 | 0.001684 |
| is_noncopyable_test.test | RUN | 0.145925 | 0.357049 | 0.520471 | 0.001635 | 0.001686 | 0.002031 |
| is_member_pointer_test.test | RUN | 0.142187 | 0.369862 | 0.532430 | 0.002567 | 0.002071 | 0.001950 |
| is_member_obj_test.test | RUN | 0.140592 | 0.353752 | 0.537872 | 0.001486 | 0.001685 | 0.001662 |
| is_member_func_test.test | RUN | 0.149587 | 0.387175 | 0.592193 | 0.001948 | 0.001800 | 0.001942 |
| is_lvalue_reference_test.test | RUN | 0.149884 | 0.399731 | 0.618179 | 0.001988 | 0.001631 | 0.001934 |
| is_list_constructible_test.test | RUN | 0.178700 | 0.477286 | 0.772618 | 0.001415 | 0.002032 | 0.001784 |
| is_integral_test.test | RUN | 0.181645 | 0.578533 | 0.999722 | 0.001668 | 0.002042 | 0.001593 |
| is_fundamental_test.test | RUN | 0.184247 | 0.595425 | 1.029525 | 0.001465 | 0.001782 | 0.001608 |
| is_function_test.test | RUN | 0.156681 | 0.371987 | 0.541952 | 0.001399 | 0.001958 | 0.001483 |
| is_floating_point_test.test | RUN | 0.182241 | 0.430162 | 0.645515 | 0.001445 | 0.001750 | 0.002341 |
| is_float_test.test | RUN | 0.182044 | 0.415862 | 0.645790 | 0.001483 | 0.001656 | 0.002204 |
| is_final_test.test | RUN | 0.179031 | 0.407817 | 0.643164 | 0.001860 | 0.001680 | 0.001782 |
| is_enum_test.test | RUN | 0.142186 | 0.349202 | 0.510996 | 0.001381 | 0.001862 | 0.001331 |
| is_empty_test.test | RUN | 0.149744 | 0.418502 | 0.575901 | 0.001370 | 0.001521 | 0.001772 |
| is_detected_test.test | RUN | 0.136689 | 0.328805 | 0.467556 | 0.001478 | 0.001657 | 0.001615 |
| is_detected_exact_test.test | RUN | 0.144064 | 0.330454 | 0.463570 | 0.001361 | 0.001594 | 0.001447 |
| is_destructible_test.test | RUN | 0.198168 | 0.625404 | 1.126175 | 0.001672 | 0.001912 | 0.001456 |
| is_default_constr_test.test | RUN | 0.201912 | 0.650183 | 1.195118 | 0.001398 | 0.001982 | 0.001912 |
| is_copy_constructible_test.test | RUN | 0.229867 | 0.699919 | 1.269980 | 0.001941 | 0.001771 | 0.001464 |
| is_copy_assignable_test.test | RUN | 0.210512 | 0.706584 | 1.204078 | 0.001642 | 0.002253 | 0.001640 |
| is_convertible_test.test | RUN | 0.199316 | 0.609035 | 0.980320 | 0.001667 | 0.001966 | 0.001443 |
| is_constructible_test.test | RUN | 0.160127 | 0.461002 | 0.695550 | 0.001770 | 0.001734 | 0.001866 |
| is_const_test.test | RUN | 0.150847 | 0.381037 | 0.566571 | 0.001653 | 0.001748 | 0.002834 |
| is_compound_test.test | RUN | 0.137817 | 0.327792 | 0.487103 | 0.001699 | 0.002012 | 0.002196 |
| is_complex_test.test | RUN | 0.188885 | 0.450726 | 0.664786 | 0.001728 | 0.001768 | 0.001972 |
| is_complete_test.test | RUN | 0.158173 | 0.449808 | 0.736894 | 0.001628 | 0.003578 | 0.001663 |
| is_class_test.test | RUN | 0.148078 | 0.402655 | 0.619118 | 0.001658 | 0.001907 | 0.001660 |
| is_bounded_array_test.test | RUN | 0.151491 | 0.361234 | 0.534088 | 0.001426 | 0.001782 | 0.001894 |
| is_base_of_test.test | RUN | 0.151346 | 0.397192 | 0.608779 | 0.001995 | 0.001675 | 0.001553 |
| is_base_and_derived_test.test | RUN | 0.148980 | 0.408644 | 0.605566 | 0.001634 | 0.001586 | 0.001587 |
| is_assignable_test.test | RUN | 0.195441 | 0.560629 | 0.965950 | 0.001686 | 0.003268 | 0.001846 |
| is_array_test.test | RUN | 0.148534 | 0.390122 | 0.571481 | 0.001309 | 0.001836 | 0.001900 |
| is_arithmetic_test.test | RUN | 0.179283 | 0.567686 | 0.950647 | 0.001988 | 0.001596 | 0.003156 |
| is_abstract_test.test | RUN | 0.340921 | 1.524843 | 3.399911 | 0.001660 | 0.001810 | 0.002220 |
| integral_constant_test.test | RUN | 0.021040 | 0.062482 | 0.084474 | 0.001197 | 0.000604 | 0.001628 |
| has_virtual_destructor_test.test | RUN | 0.155764 | 0.432283 | 0.704920 | 0.001630 | 0.001837 | 0.001814 |
| has_unary_plus_test.test | RUN | 0.288356 | 1.777853 | 2.550744 | 0.001987 | 0.002186 | 0.001798 |
| has_unary_minus_test.test | RUN | 0.298948 | 1.790495 | 2.482740 | 0.001639 | 0.001640 | 0.002017 |
| has_trivial_move_constructor_test.test | RUN | 0.201247 | 0.639261 | 1.156708 | 0.001931 | 0.001652 | 0.002052 |
| has_trivial_move_assign_test.test | RUN | 0.209381 | 0.637276 | 1.183981 | 0.001732 | 0.002317 | 0.001568 |
| has_trivial_destructor_test.test | RUN | 0.201352 | 0.648958 | 1.177801 | 0.001774 | 0.002206 | 0.001400 |
| has_trivial_copy_test.test | RUN | 0.203941 | 0.703899 | 1.195969 | 0.001742 | 0.001838 | 0.001692 |
| has_trivial_constr_test.test | RUN | 0.214092 | 0.700416 | 1.237888 | 0.002052 | 0.002040 | 0.001810 |
| has_trivial_assign_test.test | RUN | 0.202032 | 0.700977 | 1.185829 | 0.001548 | 0.001924 | 0.001914 |
| has_right_shift_test.test | RUN | 0.305890 | 1.286443 | 2.628221 | 0.001584 | 0.001658 | 0.001439 |
| has_right_shift_assign_test.test | RUN | 0.288958 | 1.229667 | 2.403519 | 0.001734 | 0.002093 | 0.001996 |
| has_prefix_classes3_test.test | RUN | 0.208979 | 0.701248 | 1.318144 | 0.001506 | 0.002116 | 0.001687 |
| has_prefix_classes2_test.test | RUN | 0.250648 | 0.780085 | 1.253026 | 0.001736 | 0.002322 | 0.001646 |
| has_prefix_classes1_test.test | RUN | 0.219554 | 0.742901 | 1.245979 | 0.001588 | 0.002355 | 0.001960 |
| has_prefix_classes0_test.test | RUN | 0.210772 | 0.851438 | 1.207285 | 0.001844 | 0.002020 | 0.001870 |
| has_pre_increment_test.test | RUN | 0.288859 | 1.832377 | 2.402863 | 0.001682 | 0.002207 | 0.001994 |
| has_pre_decrement_test.test | RUN | 0.288489 | 1.831840 | 2.512981 | 0.001546 | 0.001984 | 0.001832 |
| has_postfix_classes3_test.test | RUN | 0.211771 | 0.741450 | 1.228154 | 0.001674 | 0.002050 | 0.001404 |
| has_postfix_classes2_test.test | RUN | 0.208769 | 0.753770 | 1.231944 | 0.001589 | 0.001946 | 0.002026 |
| has_postfix_classes1_test.test | RUN | 0.215520 | 0.753187 | 1.257190 | 0.001585 | 0.001534 | 0.001276 |
| has_postfix_classes0_test.test | RUN | 0.214775 | 0.760559 | 1.223199 | 0.001782 | 0.001617 | 0.001832 |
| has_post_increment_test.test | RUN | 0.279277 | 1.828121 | 2.488189 | 0.001326 | 0.002396 | 0.001758 |
| has_post_decrement_test.test | RUN | 0.284131 | 2.123178 | 2.561910 | 0.001800 | 0.002004 | 0.002110 |
| has_plus_test.test | RUN | 0.294769 | 1.341721 | 2.544143 | 0.001465 | 0.002016 | 0.001784 |
| has_plus_assign_test.test | RUN | 0.302109 | 1.345133 | 2.449423 | 0.001435 | 0.001994 | 0.002380 |
| has_operator_new_test.test | RUN | 0.206449 | 0.638906 | 1.148206 | 0.001944 | 0.001882 | 0.002974 |
| has_nothrow_destructor_test.test | RUN | 0.209720 | 0.705707 | 1.271661 | 0.001455 | 0.001714 | 0.001770 |
| has_nothrow_copy_test.test | RUN | 0.219374 | 0.667445 | 1.216792 | 0.001642 | 0.001861 | 0.001650 |
| has_nothrow_constr_test.test | RUN | 0.199750 | 0.673545 | 1.194031 | 0.001694 | 0.002008 | 0.001546 |
| has_nothrow_assign_test.test | RUN | 0.208946 | 0.660980 | 1.163635 | 0.001655 | 0.002304 | 0.002190 |
| has_not_equal_to_test.test | RUN | 0.296494 | 1.741018 | 2.439392 | 0.002185 | 0.002306 | 0.001589 |
| has_negate_test.test | RUN | 0.272197 | 1.200166 | 2.327739 | 0.002029 | 0.001992 | 0.001799 |
| has_multiplies_test.test | RUN | 0.287657 | 1.273709 | 2.829701 | 0.002005 | 0.001384 | 0.001555 |
| has_multiplies_assign_test.test | RUN | 0.311651 | 1.258861 | 2.433426 | 0.001622 | 0.002116 | 0.001625 |
| has_modulus_test.test | RUN | 0.291728 | 1.309653 | 2.485839 | 0.001673 | 0.001918 | 0.001988 |
| has_modulus_assign_test.test | RUN | 0.285876 | 1.236079 | 2.400131 | 0.001583 | 0.002286 | 0.001912 |
| has_minus_test.test | RUN | 0.294283 | 1.383039 | 2.523698 | 0.001666 | 0.001982 | 0.002544 |
| has_minus_assign_test.test | RUN | 0.297314 | 1.303383 | 2.540602 | 0.001718 | 0.001842 | 0.001869 |
| has_logical_or_test.test | RUN | 0.286622 | 1.227119 | 2.458827 | 0.001698 | 0.002090 | 0.001936 |
| has_logical_not_test.test | RUN | 0.293132 | 1.200270 | 2.529374 | 0.001545 | 0.001709 | 0.002048 |
| has_logical_and_test.test | RUN | 0.288155 | 1.270171 | 2.496211 | 0.001450 | 0.002182 | 0.001397 |
| has_less_test.test | RUN | 0.281254 | 1.757703 | 2.504047 | 0.001664 | 0.001720 | 0.002141 |
| has_less_equal_test.test | RUN | 0.295295 | 1.849707 | 2.711299 | 0.001794 | 0.002053 | 0.002107 |
| has_left_shift_test.test | RUN | 0.311819 | 1.560939 | 2.963678 | 0.001420 | 0.002240 | 0.001996 |
| has_left_shift_assign_test.test | RUN | 0.288857 | 1.245711 | 2.514507 | 0.001610 | 0.002324 | 0.002100 |
| has_greater_test.test | RUN | 0.300728 | 1.757434 | 2.445905 | 0.001622 | 0.002146 | 0.003333 |
| has_greater_equal_test.test | RUN | 0.295445 | 1.748172 | 2.562058 | 0.001646 | 0.001810 | 0.001760 |
| has_equal_to_test.test | RUN | 0.298100 | 1.805647 | 2.518997 | 0.002056 | 0.002048 | 0.002140 |
| has_divides_test.test | RUN | 0.296244 | 1.243747 | 2.415170 | 0.002036 | 0.002159 | 0.002208 |
| has_divides_assign_test.test | RUN | 0.286215 | 1.241915 | 2.445886 | 0.001727 | 0.001738 | 0.001764 |
| has_dereference_test.test | RUN | 0.285245 | 1.258410 | 2.345376 | 0.001777 | 0.001428 | 0.002226 |
| has_complement_test.test | RUN | 0.285503 | 1.813840 | 2.391983 | 0.001900 | 0.002362 | 0.002895 |
| has_bit_xor_test.test | RUN | 0.284140 | 1.243049 | 2.554408 | 0.001660 | 0.002132 | 0.001558 |
| has_bit_xor_assign_test.test | RUN | 0.282344 | 1.308142 | 2.521945 | 0.001993 | 0.002034 | 0.002233 |
| has_bit_or_test.test | RUN | 0.281004 | 1.302077 | 2.480760 | 0.001626 | 0.001713 | 0.001470 |
| has_bit_or_assign_test.test | RUN | 0.306059 | 1.259739 | 2.738308 | 0.001317 | 0.001399 | 0.001951 |
| has_bit_and_test.test | RUN | 0.294938 | 1.277487 | 2.482699 | 0.001375 | 0.002248 | 0.001892 |
| has_bit_and_assign_test.test | RUN | 0.292479 | 1.261035 | 2.454851 | 0.001497 | 0.002032 | 0.001672 |
| has_binary_classes9_test.test | RUN | 0.306777 | 1.430869 | 2.799692 | 0.001469 | 0.001658 | 0.001840 |
| has_binary_classes8_test.test | RUN | 0.314888 | 1.508314 | 2.676816 | 0.001574 | 0.002116 | 0.001892 |
| has_binary_classes7_test.test | RUN | 0.307161 | 1.514575 | 2.893893 | 0.001552 | 0.002022 | 0.001725 |
| has_binary_classes6_test.test | RUN | 0.308717 | 1.407432 | 2.672997 | 0.001505 | 0.001456 | 0.001531 |
| has_binary_classes5_test.test | RUN | 0.309572 | 1.514606 | 2.823293 | 0.001882 | 0.002002 | 0.001579 |
| has_binary_classes4_test.test | RUN | 0.317136 | 1.509825 | 2.711480 | 0.001684 | 0.001950 | 0.001592 |
| has_binary_classes3_test.test | RUN | 0.327278 | 1.546470 | 2.689522 | 0.001622 | 0.001669 | 0.001835 |
| has_binary_classes2_test.test | RUN | 0.319432 | 1.458080 | 2.858987 | 0.001626 | 0.001587 | 0.002896 |
| has_binary_classes1_test.test | RUN | 0.310445 | 1.443203 | 2.676481 | 0.001820 | 0.001655 | 0.001924 |
| has_binary_classes0_test.test | RUN | 0.312666 | 1.472070 | 2.780630 | 0.001572 | 0.001473 | 0.001745 |
| function_traits_test.test | RUN | 0.143608 | 0.369521 | 0.567477 | 0.002032 | 0.001948 | 0.002126 |
| floating_point_promotion_test.test | RUN | 0.142560 | 0.367168 | 0.535426 | 0.001658 | 0.001800 | 0.001880 |
| extent_test.test | RUN | 0.145916 | 0.373562 | 0.572258 | 0.001633 | 0.001903 | 0.001496 |
| enable_if_test.test | RUN | 0.130476 | 0.329285 | 0.435866 | 0.001678 | 0.001934 | 0.001925 |
| disjunction_test.test | RUN | 0.139309 | 0.343068 | 0.489938 | 0.001902 | 0.001898 | 0.001516 |
| detected_test.test | RUN | 0.132454 | 0.324453 | 0.444194 | 0.001531 | 0.001986 | 0.002788 |
| detected_or_test.test | RUN | 0.140641 | 0.343777 | 0.473353 | 0.001688 | 0.001735 | 0.002006 |
| decay_test.test | RUN | 0.341444 | 1.419374 | 2.950800 | 0.001864 | 0.001937 | 0.002102 |
| cxx14_aliases_test.test | RUN | 0.226183 | 0.477799 | 0.748451 | 0.001650 | 0.001846 | 0.001627 |
| copy_reference_test.test | RUN | 0.136861 | 0.328550 | 0.486489 | 0.001589 | 0.001801 | 0.002203 |
| copy_cv_test.test | RUN | 0.140341 | 0.356414 | 0.575829 | 0.001934 | 0.001631 | 0.001862 |
| copy_cv_ref_test.test | RUN | 0.190277 | 0.582788 | 1.272063 | 0.001703 | 0.001709 | 0.001490 |
| conjunction_test.test | RUN | 0.138196 | 0.331733 | 0.483135 | 0.001650 | 0.001666 | 0.002371 |
| conditional_test.test | RUN | 0.138964 | 0.320104 | 0.474847 | 0.001782 | 0.001723 | 0.001478 |
| common_type_test.test | RUN | 0.181579 | 0.527192 | 0.929074 | 0.001718 | 0.001866 | 0.001976 |
| common_type_sfinae_test.test | RUN | 0.136286 | 0.323229 | 0.438899 | 0.001937 | 0.001876 | 0.001852 |
| common_type_sfinae2_test.test | RUN | 0.136603 | 0.334474 | 0.475955 | 0.001517 | 0.001716 | 0.001922 |
| common_type_6_test.test | RUN | 0.138314 | 0.340560 | 0.527763 | 0.001826 | 0.002079 | 0.001638 |
| common_type_5_test.test | RUN | 0.137613 | 0.340411 | 0.503225 | 0.001413 | 0.001864 | 0.001586 |
| common_type_4_test.test | RUN | 0.149776 | 0.373505 | 0.602325 | 0.001508 | 0.001745 | 0.002122 |
| common_type_3_test.test | RUN | 0.138278 | 0.310420 | 0.449263 | 0.001635 | 0.001607 | 0.001587 |
| common_type_2_test.test | RUN | 0.178930 | 0.508163 | 0.972346 | 0.001422 | 0.001972 | 0.002024 |
| alignment_of_test.test | RUN | 0.144678 | 0.400548 | 0.602569 | 0.001410 | 0.001879 | 0.001746 |
| alignment_of_a2_test.test | RUN | 0.149572 | 0.400064 | 0.582037 | 0.001562 | 0.002172 | 0.001904 |
| aligned_storage_test.test | RUN | 0.161271 | 0.408204 | 0.574120 | 0.002024 | 0.001844 | 0.001568 |
| aligned_storage_empy_test.test | RUN | 0.183428 | 0.471406 | 0.752712 | 0.002092 | 0.001381 | 0.002764 |
| aligned_storage_a2_test.test | RUN | 0.180820 | 0.391711 | 0.523858 | 0.001815 | 0.001968 | 0.001896 |
| add_volatile_test.test | RUN | 0.315362 | 1.230308 | 2.696129 | 0.001684 | 0.002562 | 0.002064 |
| add_rvalue_reference_test.test | RUN | 0.336032 | 1.464364 | 3.053618 | 0.001488 | 0.001888 | 0.002567 |
| add_reference_test.test | RUN | 0.328350 | 1.423664 | 2.995909 | 0.001421 | 0.001898 | 0.001828 |
| add_pointer_test.test | RUN | 0.213317 | 0.760515 | 1.774375 | 0.001719 | 0.001776 | 0.001866 |
| add_lvalue_reference_test.test | RUN | 0.331164 | 1.425074 | 3.079872 | 0.001791 | 0.002194 | 0.002156 |
| add_cv_test.test | RUN | 0.308329 | 1.394702 | 2.652698 | 0.001432 | 0.002326 | 0.001702 |
| add_const_test.test | RUN | 0.308395 | 1.440031 | 2.924845 | 0.001944 | 0.002329 | 0.001735 |
| is_virtual_base_of_fail.test | COMPILE_FAIL | 0.131470 | 0.303110 | 0.443213 | — | — | — |
| is_stateless_fail.test | COMPILE_FAIL | 0.130027 | 0.324715 | 0.448253 | — | — | — |
| is_polymorphic_fail.test | COMPILE_FAIL | 0.124795 | 0.336176 | 0.412811 | — | — | — |
| is_nothrow_move_constructible_fail.test | COMPILE_FAIL | 0.129152 | 0.293555 | 0.519095 | — | — | — |
| is_nothrow_move_assignable_fail.test | COMPILE_FAIL | 0.127136 | 0.428592 | 0.426440 | — | — | — |
| is_list_constructible_fail.test | COMPILE_FAIL | 0.126676 | 0.302614 | 0.412675 | — | — | — |
| is_empty_fail.test | COMPILE_FAIL | 0.133175 | 0.302591 | 0.417326 | — | — | — |
| is_destructible_fail.test | COMPILE_FAIL | 0.140137 | 0.316059 | 0.425991 | — | — | — |
| is_default_constructible_fail.test | COMPILE_FAIL | 0.128304 | 0.308278 | 0.428068 | — | — | — |
| is_copy_constructible_fail.test | COMPILE_FAIL | 0.128960 | 0.301386 | 0.425421 | — | — | — |
| is_copy_assignable_fail.test | COMPILE_FAIL | 0.125967 | 0.299404 | 0.409373 | — | — | — |
| is_convertible_fail.test | COMPILE_FAIL | 0.134137 | 0.322880 | 0.399142 | — | — | — |
| is_convertible2_fail.test | COMPILE_FAIL | 0.137213 | 0.300025 | 0.422988 | — | — | — |
| is_constructible_fail.test | COMPILE_FAIL | 0.132058 | 0.303429 | 0.421889 | — | — | — |
| is_base_of_fail.test | COMPILE_FAIL | 0.133536 | 0.286251 | 0.408532 | — | — | — |
| is_assignable_fail.test | COMPILE_FAIL | 0.132623 | 0.404248 | 0.421446 | — | — | — |
| has_nothrow_destruct_fail.test | COMPILE_FAIL | 0.129722 | 0.365302 | 0.479057 | — | — | — |
| has_nothrow_copy_fail.test | COMPILE_FAIL | 0.124701 | 0.438555 | 0.421709 | — | — | — |
| has_nothrow_cons_fail.test | COMPILE_FAIL | 0.123605 | 0.461784 | 0.476300 | — | — | — |
| has_nothrow_assign_fail.test | COMPILE_FAIL | 0.126089 | 0.400702 | 0.411919 | — | — | — |
| common_type_fail.test | COMPILE_FAIL | 0.130804 | 0.414099 | 0.424141 | — | — | — |
| common_type2_fail.test | COMPILE_FAIL | 0.131153 | 0.457716 | 0.468480 | — | — | — |
| has_nothrow_assign_test_no_intrinsics.test | RUN | 0.204296 | 0.797220 | 1.191125 | 0.001982 | 0.001615 | 0.002223 |
| has_nothrow_constr_test_no_intrinsics.test | RUN | 0.193092 | 0.693413 | 1.108624 | 0.001658 | 0.002222 | 0.001468 |
| has_nothrow_copy_test_no_intrinsics.test | RUN | 0.197502 | 0.742929 | 1.130889 | 0.002014 | 0.001745 | 0.001884 |
| is_nothrow_move_assignable_test_no_intrinsics.test | RUN | 0.205054 | 0.660358 | 1.158121 | 0.001720 | 0.001733 | 0.002920 |
| is_nothrow_move_constructible_test_no_intrinsics.test | RUN | 0.199777 | 0.645154 | 1.099862 | 0.001851 | 0.001593 | 0.002242 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.976×† | 0.948×, 1.004× | 0.942× |
| clang | 0.990×† | 0.959×, 1.021× | 1.007× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1353 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| add_const_test.test | 7 | 0.001552s | 0.001533s | 0.001557s | byte equality observed |
| add_cv_test.test | 8 | 0.001591s | 0.001597s | 0.001588s | byte equality observed |
| add_lvalue_reference_test.test | 6 | 0.001578s | 0.001580s | 0.001569s | byte equality observed |
| add_pointer_test.test | 6 | 0.001542s | 0.001586s | 0.001494s | byte equality observed |
| add_reference_test.test | 7 | 0.001656s | 0.001593s | 0.001531s | byte equality observed |
| add_rvalue_reference_test.test | 8 | 0.001817s | 0.001569s | 0.001643s | byte equality observed |
| add_volatile_test.test | 7 | 0.001565s | 0.001661s | 0.001608s | byte equality observed |
| aligned_storage_a2_test.test | 6 | 0.001767s | 0.001743s | 0.001708s | byte equality observed |
| aligned_storage_empy_test.test | 6 | 0.001551s | 0.001562s | 0.001636s | byte equality observed |
| aligned_storage_test.test | 7 | 0.001819s | 0.001784s | 0.001732s | byte equality observed |
| alignment_of_a2_test.test | 7 | 0.001589s | 0.001540s | 0.001564s | byte equality observed |
| alignment_of_test.test | 9 | 0.001538s | 0.001637s | 0.001638s | byte equality observed |
| common_type_2_test.test | 8 | 0.001609s | 0.001577s | 0.001627s | byte equality observed |
| common_type_3_test.test | 8 | 0.001584s | 0.001663s | 0.001582s | byte equality observed |
| common_type_4_test.test | 8 | 0.001600s | 0.001598s | 0.001577s | byte equality observed |
| common_type_5_test.test | 8 | 0.001581s | 0.001682s | 0.001599s | byte equality observed |
| common_type_6_test.test | 7 | 0.001644s | 0.001613s | 0.001607s | byte equality observed |
| common_type_sfinae2_test.test | 8 | 0.001525s | 0.001611s | 0.001575s | byte equality observed |
| common_type_sfinae_test.test | 8 | 0.001500s | 0.001676s | 0.001555s | byte equality observed |
| common_type_test.test | 9 | 0.001521s | 0.001590s | 0.001564s | byte equality observed |
| conditional_test.test | 9 | 0.001507s | 0.001709s | 0.001533s | byte equality observed |
| conjunction_test.test | 8 | 0.001547s | 0.001636s | 0.001547s | byte equality observed |
| copy_cv_ref_test.test | 8 | 0.001532s | 0.001644s | 0.001548s | byte equality observed |
| copy_cv_test.test | 9 | 0.001597s | 0.001543s | 0.001556s | byte equality observed |
| copy_reference_test.test | 7 | 0.001572s | 0.001625s | 0.001554s | byte equality observed |
| cxx14_aliases_test.test | 8 | 0.001559s | 0.001588s | 0.001554s | byte equality observed |
| decay_test.test | 7 | 0.001557s | 0.001623s | 0.001532s | byte equality observed |
| detected_or_test.test | 8 | 0.001534s | 0.001578s | 0.001589s | byte equality observed |
| detected_test.test | 8 | 0.001555s | 0.001688s | 0.001555s | byte equality observed |
| disjunction_test.test | 10 | 0.001574s | 0.001555s | 0.001535s | byte equality observed |
| enable_if_test.test | 7 | 0.001564s | 0.001584s | 0.001624s | byte equality observed |
| extent_test.test | 8 | 0.001594s | 0.001588s | 0.001649s | byte equality observed |
| floating_point_promotion_test.test | 8 | 0.001682s | 0.001650s | 0.001607s | byte equality observed |
| function_traits_test.test | 7 | 0.001587s | 0.001541s | 0.001531s | byte equality observed |
| has_binary_classes0_test.test | 8 | 0.001549s | 0.001640s | 0.001523s | byte equality observed |
| has_binary_classes1_test.test | 7 | 0.001537s | 0.001588s | 0.001597s | byte equality observed |
| has_binary_classes2_test.test | 6 | 0.001574s | 0.001702s | 0.001628s | byte equality observed |
| has_binary_classes3_test.test | 7 | 0.001573s | 0.001561s | 0.001632s | byte equality observed |
| has_binary_classes4_test.test | 7 | 0.001611s | 0.001578s | 0.001641s | byte equality observed |
| has_binary_classes5_test.test | 8 | 0.001635s | 0.001594s | 0.001626s | byte equality observed |
| has_binary_classes6_test.test | 7 | 0.001603s | 0.001572s | 0.001600s | byte equality observed |
| has_binary_classes7_test.test | 6 | 0.001602s | 0.001595s | 0.001610s | byte equality observed |
| has_binary_classes8_test.test | 8 | 0.001573s | 0.001560s | 0.001646s | byte equality observed |
| has_binary_classes9_test.test | 7 | 0.001597s | 0.001590s | 0.001558s | byte equality observed |
| has_bit_and_assign_test.test | 8 | 0.001543s | 0.001638s | 0.001672s | byte equality observed |
| has_bit_and_test.test | 6 | 0.001534s | 0.001572s | 0.001643s | byte equality observed |
| has_bit_or_assign_test.test | 7 | 0.001552s | 0.001586s | 0.001649s | byte equality observed |
| has_bit_or_test.test | 7 | 0.001621s | 0.001578s | 0.001553s | byte equality observed |
| has_bit_xor_assign_test.test | 8 | 0.001635s | 0.001572s | 0.001659s | byte equality observed |
| has_bit_xor_test.test | 8 | 0.001625s | 0.001562s | 0.001529s | byte equality observed |
| has_complement_test.test | 7 | 0.001595s | 0.001590s | 0.001608s | byte equality observed |
| has_dereference_test.test | 8 | 0.001622s | 0.001558s | 0.001576s | byte equality observed |
| has_divides_assign_test.test | 7 | 0.001632s | 0.001629s | 0.001628s | byte equality observed |
| has_divides_test.test | 7 | 0.001597s | 0.001570s | 0.001680s | byte equality observed |
| has_equal_to_test.test | 7 | 0.001582s | 0.001714s | 0.001584s | byte equality observed |
| has_greater_equal_test.test | 7 | 0.001607s | 0.001560s | 0.001591s | byte equality observed |
| has_greater_test.test | 8 | 0.001612s | 0.001593s | 0.001569s | byte equality observed |
| has_left_shift_assign_test.test | 8 | 0.001588s | 0.001597s | 0.001541s | byte equality observed |
| has_left_shift_test.test | 7 | 0.001631s | 0.001649s | 0.001557s | byte equality observed |
| has_less_equal_test.test | 7 | 0.001550s | 0.001569s | 0.001556s | byte equality observed |
| has_less_test.test | 7 | 0.001601s | 0.001580s | 0.001605s | byte equality observed |
| has_logical_and_test.test | 7 | 0.001564s | 0.001638s | 0.001631s | byte equality observed |
| has_logical_not_test.test | 8 | 0.001563s | 0.001568s | 0.001589s | byte equality observed |
| has_logical_or_test.test | 8 | 0.001585s | 0.001576s | 0.001540s | byte equality observed |
| has_minus_assign_test.test | 6 | 0.001593s | 0.001608s | 0.001621s | byte equality observed |
| has_minus_test.test | 6 | 0.001556s | 0.001647s | 0.001618s | byte equality observed |
| has_modulus_assign_test.test | 7 | 0.001593s | 0.001546s | 0.001577s | byte equality observed |
| has_modulus_test.test | 7 | 0.001643s | 0.001589s | 0.001525s | byte equality observed |
| has_multiplies_assign_test.test | 8 | 0.001613s | 0.001556s | 0.001659s | byte equality observed |
| has_multiplies_test.test | 7 | 0.001630s | 0.001606s | 0.001525s | byte equality observed |
| has_negate_test.test | 8 | 0.001550s | 0.001608s | 0.001604s | byte equality observed |
| has_not_equal_to_test.test | 8 | 0.001594s | 0.001607s | 0.001561s | byte equality observed |
| has_nothrow_assign_test.test | 8 | 0.001627s | 0.001655s | 0.001623s | byte equality observed |
| has_nothrow_assign_test_no_intrinsics.test | 7 | 0.001567s | 0.001599s | 0.001560s | byte equality observed |
| has_nothrow_constr_test.test | 7 | 0.001619s | 0.001572s | 0.001558s | byte equality observed |
| has_nothrow_constr_test_no_intrinsics.test | 7 | 0.001604s | 0.001625s | 0.001532s | byte equality observed |
| has_nothrow_copy_test.test | 7 | 0.001559s | 0.001568s | 0.001641s | byte equality observed |
| has_nothrow_copy_test_no_intrinsics.test | 8 | 0.001614s | 0.001568s | 0.001664s | byte equality observed |
| has_nothrow_destructor_test.test | 8 | 0.001630s | 0.001600s | 0.001580s | byte equality observed |
| has_operator_new_test.test | 8 | 0.001544s | 0.001549s | 0.001556s | byte equality observed |
| has_plus_assign_test.test | 7 | 0.001573s | 0.001588s | 0.001580s | byte equality observed |
| has_plus_test.test | 7 | 0.001528s | 0.001771s | 0.001536s | byte equality observed |
| has_post_decrement_test.test | 7 | 0.001568s | 0.001535s | 0.001662s | byte equality observed |
| has_post_increment_test.test | 8 | 0.001599s | 0.001575s | 0.001518s | byte equality observed |
| has_postfix_classes0_test.test | 7 | 0.001603s | 0.001583s | 0.001552s | byte equality observed |
| has_postfix_classes1_test.test | 8 | 0.001606s | 0.001551s | 0.001675s | byte equality observed |
| has_postfix_classes2_test.test | 9 | 0.001604s | 0.001555s | 0.001689s | byte equality observed |
| has_postfix_classes3_test.test | 8 | 0.001599s | 0.001586s | 0.001527s | byte equality observed |
| has_pre_decrement_test.test | 6 | 0.001672s | 0.001678s | 0.001509s | byte equality observed |
| has_pre_increment_test.test | 7 | 0.001639s | 0.001588s | 0.001646s | byte equality observed |
| has_prefix_classes0_test.test | 7 | 0.001631s | 0.001687s | 0.001647s | byte equality observed |
| has_prefix_classes1_test.test | 7 | 0.001740s | 0.001569s | 0.001548s | byte equality observed |
| has_prefix_classes2_test.test | 6 | 0.001598s | 0.001603s | 0.001574s | byte equality observed |
| has_prefix_classes3_test.test | 8 | 0.001651s | 0.001621s | 0.001526s | byte equality observed |
| has_right_shift_assign_test.test | 9 | 0.001607s | 0.001631s | 0.001567s | byte equality observed |
| has_right_shift_test.test | 7 | 0.001628s | 0.001595s | 0.001589s | byte equality observed |
| has_trivial_assign_test.test | 7 | 0.001675s | 0.001723s | 0.001729s | byte equality observed |
| has_trivial_constr_test.test | 7 | 0.001701s | 0.001667s | 0.001763s | native test oracle; output varies |
| has_trivial_copy_test.test | 6 | 0.001792s | 0.001774s | 0.001673s | native test oracle; output varies |
| has_trivial_destructor_test.test | 8 | 0.001777s | 0.001593s | 0.001633s | byte equality observed |
| has_trivial_move_assign_test.test | 8 | 0.001669s | 0.001573s | 0.001532s | byte equality observed |
| has_trivial_move_constructor_test.test | 7 | 0.001682s | 0.001557s | 0.001664s | byte equality observed |
| has_unary_minus_test.test | 8 | 0.001580s | 0.001604s | 0.001550s | byte equality observed |
| has_unary_plus_test.test | 7 | 0.001652s | 0.001612s | 0.001578s | byte equality observed |
| has_virtual_destructor_test.test | 8 | 0.001628s | 0.001558s | 0.001550s | byte equality observed |
| integral_constant_test.test | 8 | 0.000812s | 0.000861s | 0.001599s | byte equality observed |
| is_abstract_test.test | 8 | 0.001613s | 0.001681s | 0.001724s | byte equality observed |
| is_arithmetic_test.test | 8 | 0.001758s | 0.001564s | 0.001615s | byte equality observed |
| is_array_test.test | 7 | 0.001610s | 0.001653s | 0.001604s | byte equality observed |
| is_assignable_test.test | 8 | 0.001686s | 0.001554s | 0.001544s | byte equality observed |
| is_base_and_derived_test.test | 6 | 0.001633s | 0.001535s | 0.001555s | byte equality observed |
| is_base_of_test.test | 7 | 0.001621s | 0.001549s | 0.001606s | byte equality observed |
| is_bounded_array_test.test | 7 | 0.001621s | 0.001582s | 0.001524s | byte equality observed |
| is_class_test.test | 7 | 0.001573s | 0.001546s | 0.001640s | byte equality observed |
| is_complete_test.test | 7 | 0.001635s | 0.001571s | 0.001599s | byte equality observed |
| is_complex_test.test | 8 | 0.001608s | 0.001612s | 0.001537s | byte equality observed |
| is_compound_test.test | 6 | 0.001617s | 0.001578s | 0.001598s | byte equality observed |
| is_const_test.test | 8 | 0.001664s | 0.001581s | 0.001526s | byte equality observed |
| is_constructible_test.test | 7 | 0.001606s | 0.001635s | 0.001511s | byte equality observed |
| is_convertible_test.test | 10 | 0.001620s | 0.001739s | 0.001574s | byte equality observed |
| is_copy_assignable_test.test | 8 | 0.001547s | 0.001703s | 0.001565s | byte equality observed |
| is_copy_constructible_test.test | 8 | 0.001679s | 0.001627s | 0.001666s | byte equality observed |
| is_default_constr_test.test | 7 | 0.001613s | 0.001611s | 0.001518s | byte equality observed |
| is_destructible_test.test | 8 | 0.001625s | 0.001597s | 0.001769s | byte equality observed |
| is_detected_exact_test.test | 7 | 0.001604s | 0.001607s | 0.001498s | byte equality observed |
| is_detected_test.test | 9 | 0.001592s | 0.001626s | 0.001537s | byte equality observed |
| is_empty_test.test | 7 | 0.001636s | 0.001625s | 0.001557s | byte equality observed |
| is_enum_test.test | 7 | 0.001563s | 0.001579s | 0.001528s | byte equality observed |
| is_final_test.test | 8 | 0.001552s | 0.001588s | 0.001618s | byte equality observed |
| is_float_test.test | 7 | 0.001612s | 0.001718s | 0.001604s | byte equality observed |
| is_floating_point_test.test | 8 | 0.001581s | 0.001628s | 0.001506s | byte equality observed |
| is_function_test.test | 7 | 0.001570s | 0.001605s | 0.001513s | byte equality observed |
| is_fundamental_test.test | 6 | 0.001553s | 0.001528s | 0.001595s | byte equality observed |
| is_integral_test.test | 7 | 0.001728s | 0.001655s | 0.001511s | byte equality observed |
| is_list_constructible_test.test | 8 | 0.001585s | 0.001605s | 0.001472s | byte equality observed |
| is_lvalue_reference_test.test | 9 | 0.001601s | 0.001612s | 0.001507s | byte equality observed |
| is_member_func_test.test | 8 | 0.001591s | 0.001657s | 0.001528s | byte equality observed |
| is_member_obj_test.test | 7 | 0.001560s | 0.001613s | 0.001659s | byte equality observed |
| is_member_pointer_test.test | 8 | 0.001578s | 0.001604s | 0.001577s | byte equality observed |
| is_noncopyable_test.test | 8 | 0.001608s | 0.001897s | 0.001587s | byte equality observed |
| is_nothrow_move_assignable_test.test | 8 | 0.001566s | 0.001649s | 0.001596s | byte equality observed |
| is_nothrow_move_assignable_test_no_intrinsics.test | 8 | 0.001620s | 0.001593s | 0.001566s | byte equality observed |
| is_nothrow_move_constructible_test.test | 7 | 0.001578s | 0.001643s | 0.001520s | byte equality observed |
| is_nothrow_move_constructible_test_no_intrinsics.test | 7 | 0.001570s | 0.001562s | 0.001524s | byte equality observed |
| is_nothrow_swappable_test.test | 9 | 0.001561s | 0.001584s | 0.001645s | byte equality observed |
| is_object_test.test | 9 | 0.001605s | 0.001573s | 0.001557s | byte equality observed |
| is_pod_test.test | 7 | 0.001533s | 0.001598s | 0.001548s | byte equality observed |
| is_pointer_test.test | 7 | 0.001572s | 0.001655s | 0.001567s | byte equality observed |
| is_polymorphic_test.test | 8 | 0.001634s | 0.001618s | 0.001589s | byte equality observed |
| is_reference_test.test | 8 | 0.001645s | 0.001636s | 0.001523s | byte equality observed |
| is_rvalue_reference_test.test | 8 | 0.001588s | 0.001802s | 0.001525s | byte equality observed |
| is_same_test.test | 8 | 0.001604s | 0.001583s | 0.001487s | byte equality observed |
| is_scalar_test.test | 8 | 0.001550s | 0.001586s | 0.001612s | byte equality observed |
| is_scoped_enum_test.test | 8 | 0.001574s | 0.001592s | 0.001601s | byte equality observed |
| is_signed_test.test | 8 | 0.001575s | 0.001589s | 0.001594s | byte equality observed |
| is_stateless_test.test | 7 | 0.001742s | 0.001673s | 0.001572s | byte equality observed |
| is_swappable_test.test | 9 | 0.001757s | 0.001593s | 0.001544s | byte equality observed |
| is_trivially_copyable_test.test | 8 | 0.001637s | 0.001624s | 0.001616s | byte equality observed |
| is_unbounded_array_test.test | 7 | 0.001555s | 0.001625s | 0.001523s | byte equality observed |
| is_union_test.test | 8 | 0.001637s | 0.001570s | 0.001525s | byte equality observed |
| is_unscoped_enum_test.test | 7 | 0.001597s | 0.001644s | 0.001590s | byte equality observed |
| is_unsigned_test.test | 8 | 0.001586s | 0.001621s | 0.001602s | byte equality observed |
| is_virtual_base_of_test.test | 8 | 0.001583s | 0.001528s | 0.001561s | byte equality observed |
| is_void_test.test | 8 | 0.001638s | 0.001815s | 0.001579s | byte equality observed |
| is_volatile_test.test | 8 | 0.001569s | 0.001580s | 0.001606s | byte equality observed |
| make_signed_test.test | 8 | 0.001592s | 0.001680s | 0.001620s | byte equality observed |
| make_unsigned_test.test | 7 | 0.001545s | 0.001624s | 0.001615s | byte equality observed |
| make_void_test.test | 8 | 0.001586s | 0.001661s | 0.001588s | byte equality observed |
| mpl_interop_test1.test | 6 | 0.000799s | 0.000798s | 0.001488s | byte equality observed |
| mpl_interop_test2.test | 7 | 0.000816s | 0.000791s | 0.001604s | byte equality observed |
| mpl_interop_test3.test | 8 | 0.000801s | 0.000828s | 0.001596s | byte equality observed |
| negation_test.test | 7 | 0.001620s | 0.001592s | 0.001589s | byte equality observed |
| promote_basic_test.test | 8 | 0.000865s | 0.000803s | 0.001488s | byte equality observed |
| promote_enum_msvc_bug_test.test | 6 | 0.000822s | 0.000825s | 0.001583s | byte equality observed |
| promote_enum_test.test | 8 | 0.000780s | 0.000829s | 0.001506s | byte equality observed |
| promote_mpl_test.test | 8 | 0.000814s | 0.000858s | 0.001576s | byte equality observed |
| rank_test.test | 6 | 0.001633s | 0.001553s | 0.001612s | byte equality observed |
| remove_all_extents_test.test | 9 | 0.001583s | 0.001789s | 0.001611s | byte equality observed |
| remove_bounds_test.test | 9 | 0.001555s | 0.001667s | 0.001583s | byte equality observed |
| remove_const_test.test | 5 | 0.001707s | 0.001605s | 0.001579s | byte equality observed |
| remove_cv_ref_test.test | 6 | 0.001571s | 0.001726s | 0.001566s | byte equality observed |
| remove_cv_test.test | 7 | 0.001527s | 0.001590s | 0.001568s | byte equality observed |
| remove_extent_test.test | 7 | 0.001601s | 0.001608s | 0.001507s | byte equality observed |
| remove_pointer_test.test | 8 | 0.001543s | 0.001755s | 0.001555s | byte equality observed |
| remove_reference_test.test | 7 | 0.001532s | 0.001583s | 0.001616s | byte equality observed |
| remove_volatile_test.test | 6 | 0.001648s | 0.001635s | 0.001591s | byte equality observed |
| tricky_abstract_type_test.test | 7 | 0.001541s | 0.001569s | 0.001537s | byte equality observed |
| tricky_add_pointer_test.test | 7 | 0.001542s | 0.001638s | 0.001545s | byte equality observed |
| tricky_function_type_test.test | 7 | 0.001552s | 0.001571s | 0.001536s | byte equality observed |
| tricky_incomplete_type_test.test | 6 | 0.001600s | 0.001578s | 0.001578s | byte equality observed |
| tricky_is_enum_test.test | 7 | 0.001595s | 0.001587s | 0.001545s | byte equality observed |
| tricky_partial_spec_test.test | 7 | 0.001635s | 0.001561s | 0.001522s | byte equality observed |
| tricky_rvalue_test.test | 8 | 0.001708s | 0.001599s | 0.001518s | byte equality observed |
| type_identity_test.test | 8 | 0.001631s | 0.001753s | 0.001609s | byte equality observed |
| type_traits_test.test | 7 | 0.000800s | 0.000884s | 0.001640s | byte equality observed |
| type_with_alignment_test.test | 6 | 0.001888s | 0.001767s | 0.001806s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
