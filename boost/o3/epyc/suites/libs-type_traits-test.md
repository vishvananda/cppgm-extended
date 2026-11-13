# libs/type_traits/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| type_with_alignment_test.test | RUN | 0.155281 | 0.442977 | 0.753598 | 0.001837 | 0.002024 | 0.001920 |
| type_traits_test.test | RUN | 0.218132 | 0.463060 | 0.778529 | 0.000637 | 0.000826 | 0.002132 |
| type_identity_test.test | RUN | 0.438045 | 1.728289 | 4.583285 | 0.001913 | 0.001888 | 0.002055 |
| tricky_rvalue_test.test | RUN | 0.143177 | 0.354035 | 0.607514 | 0.001835 | 0.001884 | 0.001619 |
| tricky_partial_spec_test.test | RUN | 0.176479 | 0.516356 | 0.939053 | 0.001742 | 0.001772 | 0.002012 |
| tricky_is_enum_test.test | RUN | 0.137960 | 0.338980 | 0.554189 | 0.001778 | 0.001950 | 0.001733 |
| tricky_incomplete_type_test.test | RUN | 0.232623 | 0.539210 | 0.978720 | 0.001874 | 0.001858 | 0.002012 |
| tricky_function_type_test.test | RUN | 0.185007 | 0.564514 | 1.032933 | 0.001452 | 0.001702 | 0.001766 |
| tricky_add_pointer_test.test | RUN | 0.291422 | 1.026142 | 2.521398 | 0.001763 | 0.002478 | 0.001912 |
| tricky_abstract_type_test.test | RUN | 0.145619 | 0.356376 | 0.579753 | 0.001742 | 0.001916 | 0.001766 |
| remove_volatile_test.test | RUN | 0.390900 | 1.405680 | 3.870526 | 0.001902 | 0.001884 | 0.002002 |
| remove_reference_test.test | RUN | 0.416664 | 1.546559 | 4.022026 | 0.001996 | 0.001935 | 0.002112 |
| remove_pointer_test.test | RUN | 0.429688 | 1.538407 | 4.348184 | 0.002058 | 0.001620 | 0.001812 |
| remove_extent_test.test | RUN | 0.408279 | 1.553749 | 4.162163 | 0.001924 | 0.001962 | 0.001813 |
| remove_cv_test.test | RUN | 0.407331 | 1.554163 | 4.090046 | 0.001928 | 0.001993 | 0.002004 |
| remove_cv_ref_test.test | RUN | 0.854751 | 3.639525 | 10.194111 | 0.001618 | 0.001770 | 0.002074 |
| remove_const_test.test | RUN | 0.387710 | 1.403697 | 3.924642 | 0.001926 | 0.001934 | 0.002084 |
| remove_bounds_test.test | RUN | 0.421073 | 1.566296 | 4.109168 | 0.001720 | 0.001944 | 0.002057 |
| remove_all_extents_test.test | RUN | 0.416260 | 1.562752 | 4.054356 | 0.001786 | 0.002095 | 0.001828 |
| rank_test.test | RUN | 0.145260 | 0.358902 | 0.528543 | 0.001776 | 0.001881 | 0.001889 |
| promote_mpl_test.test | RUN | 0.127188 | 0.317159 | 0.377865 | 0.000707 | 0.000838 | 0.002022 |
| promote_enum_test.test | RUN | 0.028876 | 0.080967 | 0.089136 | 0.000711 | 0.000815 | 0.001606 |
| promote_enum_msvc_bug_test.test | RUN | 0.024458 | 0.066782 | 0.064831 | 0.000722 | 0.000824 | 0.001708 |
| promote_basic_test.test | RUN | 0.031352 | 0.094960 | 0.093371 | 0.000791 | 0.000778 | 0.001948 |
| negation_test.test | RUN | 0.139128 | 0.334692 | 0.492157 | 0.001790 | 0.002018 | 0.001588 |
| mpl_interop_test3.test | RUN | 0.115566 | 0.299905 | 0.356174 | 0.000786 | 0.000657 | 0.001795 |
| mpl_interop_test2.test | RUN | 0.039546 | 0.082479 | 0.087664 | 0.000711 | 0.000738 | 0.001592 |
| mpl_interop_test1.test | RUN | 0.021812 | 0.063120 | 0.059483 | 0.000657 | 0.000696 | 0.001764 |
| make_void_test.test | RUN | 0.148702 | 0.358400 | 0.604311 | 0.001861 | 0.002063 | 0.002023 |
| make_unsigned_test.test | RUN | 0.172636 | 0.479791 | 0.828262 | 0.001813 | 0.001898 | 0.001760 |
| make_signed_test.test | RUN | 0.171083 | 0.480132 | 0.860636 | 0.001654 | 0.001886 | 0.001591 |
| is_volatile_test.test | RUN | 0.150569 | 0.407617 | 0.687659 | 0.001716 | 0.001778 | 0.001914 |
| is_void_test.test | RUN | 0.152214 | 0.369066 | 0.655661 | 0.001912 | 0.002094 | 0.001880 |
| is_virtual_base_of_test.test | RUN | 0.173333 | 0.495625 | 0.895408 | 0.001660 | 0.002310 | 0.002031 |
| is_unsigned_test.test | RUN | 0.151164 | 0.401682 | 0.643985 | 0.001661 | 0.001743 | 0.001854 |
| is_unscoped_enum_test.test | RUN | 0.145136 | 0.343825 | 0.550547 | 0.001902 | 0.001856 | 0.001986 |
| is_union_test.test | RUN | 0.159779 | 0.443804 | 0.800946 | 0.001848 | 0.002068 | 0.001677 |
| is_unbounded_array_test.test | RUN | 0.142103 | 0.374188 | 0.634338 | 0.001722 | 0.002104 | 0.002007 |
| is_trivially_copyable_test.test | RUN | 0.242230 | 0.810538 | 1.606000 | 0.001623 | 0.001781 | 0.002135 |
| is_swappable_test.test | RUN | 0.197791 | 0.596903 | 1.233517 | 0.001690 | 0.002060 | 0.001767 |
| is_stateless_test.test | RUN | 0.225346 | 0.703117 | 1.399796 | 0.001848 | 0.001900 | 0.002083 |
| is_signed_test.test | RUN | 0.155775 | 0.377936 | 0.668858 | 0.001512 | 0.001886 | 0.002070 |
| is_scoped_enum_test.test | RUN | 0.145554 | 0.347675 | 0.537433 | 0.001695 | 0.002020 | 0.002043 |
| is_scalar_test.test | RUN | 0.192156 | 0.638345 | 1.223589 | 0.001924 | 0.002188 | 0.001968 |
| is_same_test.test | RUN | 0.144577 | 0.366354 | 0.652762 | 0.001826 | 0.001708 | 0.001954 |
| is_rvalue_reference_test.test | RUN | 0.152251 | 0.422513 | 0.734144 | 0.001448 | 0.001738 | 0.001747 |
| is_reference_test.test | RUN | 0.156142 | 0.417686 | 0.735963 | 0.001724 | 0.002054 | 0.002060 |
| is_polymorphic_test.test | RUN | 0.158291 | 0.431309 | 0.787788 | 0.001756 | 0.002188 | 0.001935 |
| is_pointer_test.test | RUN | 0.158438 | 0.436457 | 0.753775 | 0.001890 | 0.001718 | 0.001696 |
| is_pod_test.test | RUN | 0.207613 | 0.708753 | 1.465068 | 0.001760 | 0.002001 | 0.002045 |
| is_object_test.test | RUN | 0.143127 | 0.354666 | 0.592624 | 0.001696 | 0.001880 | 0.001838 |
| is_nothrow_swappable_test.test | RUN | 0.210554 | 0.651394 | 1.335601 | 0.001932 | 0.001964 | 0.002080 |
| is_nothrow_move_constructible_test.test | RUN | 0.205567 | 0.683757 | 1.413196 | 0.001764 | 0.001806 | 0.001832 |
| is_nothrow_move_assignable_test.test | RUN | 0.205136 | 0.685563 | 1.363318 | 0.001748 | 0.001746 | 0.001706 |
| is_noncopyable_test.test | RUN | 0.145565 | 0.378592 | 0.621205 | 0.001596 | 0.001863 | 0.002338 |
| is_member_pointer_test.test | RUN | 0.145105 | 0.373527 | 0.611075 | 0.001710 | 0.001781 | 0.001985 |
| is_member_obj_test.test | RUN | 0.147924 | 0.375780 | 0.605413 | 0.001944 | 0.001747 | 0.002025 |
| is_member_func_test.test | RUN | 0.152615 | 0.397115 | 0.707531 | 0.001680 | 0.002026 | 0.001842 |
| is_lvalue_reference_test.test | RUN | 0.152402 | 0.433683 | 0.748476 | 0.001665 | 0.001901 | 0.001965 |
| is_list_constructible_test.test | RUN | 0.179131 | 0.498830 | 0.830104 | 0.001956 | 0.001830 | 0.002018 |
| is_integral_test.test | RUN | 0.176840 | 0.597659 | 1.178164 | 0.001537 | 0.001819 | 0.001626 |
| is_fundamental_test.test | RUN | 0.178846 | 0.606701 | 1.180590 | 0.001522 | 0.002039 | 0.001698 |
| is_function_test.test | RUN | 0.151320 | 0.399765 | 0.673127 | 0.001775 | 0.001756 | 0.002056 |
| is_floating_point_test.test | RUN | 0.154873 | 0.417665 | 0.794061 | 0.001986 | 0.001952 | 0.002072 |
| is_float_test.test | RUN | 0.157788 | 0.427562 | 0.754054 | 0.001619 | 0.001902 | 0.002042 |
| is_final_test.test | RUN | 0.152812 | 0.435698 | 0.782246 | 0.001729 | 0.001764 | 0.001916 |
| is_enum_test.test | RUN | 0.141808 | 0.353903 | 0.610114 | 0.001666 | 0.001851 | 0.002000 |
| is_empty_test.test | RUN | 0.156733 | 0.396330 | 0.626833 | 0.001631 | 0.002056 | 0.002054 |
| is_detected_test.test | RUN | 0.134862 | 0.336324 | 0.553403 | 0.001964 | 0.002401 | 0.001684 |
| is_detected_exact_test.test | RUN | 0.135136 | 0.341563 | 0.546532 | 0.001644 | 0.001850 | 0.002293 |
| is_destructible_test.test | RUN | 0.212037 | 0.671533 | 1.364290 | 0.001756 | 0.002092 | 0.001851 |
| is_default_constr_test.test | RUN | 0.208132 | 0.690299 | 1.375664 | 0.002070 | 0.001770 | 0.001870 |
| is_copy_constructible_test.test | RUN | 0.221805 | 0.720284 | 1.518388 | 0.001996 | 0.001776 | 0.001607 |
| is_copy_assignable_test.test | RUN | 0.224185 | 0.725596 | 1.494537 | 0.001810 | 0.001745 | 0.001969 |
| is_convertible_test.test | RUN | 0.214910 | 0.639933 | 1.200516 | 0.001882 | 0.002086 | 0.001595 |
| is_constructible_test.test | RUN | 0.160027 | 0.470302 | 0.866381 | 0.001615 | 0.001714 | 0.001746 |
| is_const_test.test | RUN | 0.148432 | 0.410866 | 0.701138 | 0.001452 | 0.001576 | 0.001807 |
| is_compound_test.test | RUN | 0.138918 | 0.340309 | 0.566028 | 0.001532 | 0.001870 | 0.002084 |
| is_complex_test.test | RUN | 0.192060 | 0.453708 | 0.779173 | 0.001800 | 0.002089 | 0.001703 |
| is_complete_test.test | RUN | 0.167825 | 0.473941 | 0.846060 | 0.001762 | 0.001621 | 0.001702 |
| is_class_test.test | RUN | 0.156531 | 0.413147 | 0.747287 | 0.001850 | 0.001688 | 0.001925 |
| is_bounded_array_test.test | RUN | 0.145962 | 0.376852 | 0.660213 | 0.001735 | 0.001671 | 0.002062 |
| is_base_of_test.test | RUN | 0.160063 | 0.409543 | 0.710435 | 0.001882 | 0.002078 | 0.001796 |
| is_base_and_derived_test.test | RUN | 0.157746 | 0.399528 | 0.713337 | 0.001608 | 0.002085 | 0.001708 |
| is_assignable_test.test | RUN | 0.188615 | 0.572183 | 1.090727 | 0.001736 | 0.002033 | 0.001800 |
| is_array_test.test | RUN | 0.154662 | 0.399578 | 0.683855 | 0.001752 | 0.001888 | 0.002002 |
| is_arithmetic_test.test | RUN | 0.189430 | 0.610850 | 1.193430 | 0.001925 | 0.002056 | 0.002102 |
| is_abstract_test.test | RUN | 0.355561 | 1.643630 | 3.865896 | 0.001810 | 0.002180 | 0.001846 |
| integral_constant_test.test | RUN | 0.022187 | 0.065022 | 0.063382 | 0.000791 | 0.000806 | 0.001628 |
| has_virtual_destructor_test.test | RUN | 0.166735 | 0.465686 | 0.853890 | 0.001842 | 0.002134 | 0.002001 |
| has_unary_plus_test.test | RUN | 0.328157 | 1.860051 | 2.939885 | 0.001879 | 0.002076 | 0.002106 |
| has_unary_minus_test.test | RUN | 0.326322 | 1.800895 | 2.877036 | 0.001558 | 0.001988 | 0.002068 |
| has_trivial_move_constructor_test.test | RUN | 0.217148 | 0.707100 | 1.418122 | 0.001902 | 0.001736 | 0.001882 |
| has_trivial_move_assign_test.test | RUN | 0.213701 | 0.613743 | 1.383639 | 0.001844 | 0.001668 | 0.002035 |
| has_trivial_destructor_test.test | RUN | 0.217746 | 0.663427 | 1.396218 | 0.001946 | 0.001876 | 0.002040 |
| has_trivial_copy_test.test | RUN | 0.216038 | 0.744129 | 1.485390 | 0.001808 | 0.001934 | 0.002185 |
| has_trivial_constr_test.test | RUN | 0.217201 | 0.721418 | 1.499033 | 0.001916 | 0.001940 | 0.002187 |
| has_trivial_assign_test.test | RUN | 0.219238 | 0.746385 | 1.474272 | 0.001942 | 0.002076 | 0.002210 |
| has_right_shift_test.test | RUN | 0.328765 | 1.352484 | 3.160007 | 0.001811 | 0.002042 | 0.002011 |
| has_right_shift_assign_test.test | RUN | 0.320646 | 1.288095 | 2.965478 | 0.001776 | 0.002343 | 0.001871 |
| has_prefix_classes3_test.test | RUN | 0.233162 | 0.760695 | 1.588455 | 0.001940 | 0.002000 | 0.001776 |
| has_prefix_classes2_test.test | RUN | 0.238095 | 0.731775 | 1.513496 | 0.001806 | 0.002099 | 0.001830 |
| has_prefix_classes1_test.test | RUN | 0.232403 | 0.765295 | 1.543993 | 0.001818 | 0.001945 | 0.002018 |
| has_prefix_classes0_test.test | RUN | 0.231501 | 0.747766 | 1.510449 | 0.001762 | 0.002106 | 0.002160 |
| has_pre_increment_test.test | RUN | 0.316128 | 1.821068 | 2.944815 | 0.001787 | 0.002025 | 0.001733 |
| has_pre_decrement_test.test | RUN | 0.318807 | 1.785695 | 2.924092 | 0.001730 | 0.002022 | 0.001550 |
| has_postfix_classes3_test.test | RUN | 0.222356 | 0.775769 | 1.566005 | 0.001650 | 0.001750 | 0.001787 |
| has_postfix_classes2_test.test | RUN | 0.222022 | 0.747420 | 1.531839 | 0.001584 | 0.001832 | 0.001860 |
| has_postfix_classes1_test.test | RUN | 0.232539 | 0.761989 | 1.516379 | 0.001927 | 0.002116 | 0.002000 |
| has_postfix_classes0_test.test | RUN | 0.237760 | 0.756229 | 1.528475 | 0.001724 | 0.002080 | 0.002026 |
| has_post_increment_test.test | RUN | 0.317448 | 1.847814 | 2.930883 | 0.001898 | 0.001844 | 0.001610 |
| has_post_decrement_test.test | RUN | 0.323347 | 1.851151 | 2.950707 | 0.001609 | 0.001920 | 0.001789 |
| has_plus_test.test | RUN | 0.324365 | 1.298593 | 2.956302 | 0.001837 | 0.001662 | 0.001874 |
| has_plus_assign_test.test | RUN | 0.330410 | 1.266767 | 2.995673 | 0.001682 | 0.001860 | 0.001643 |
| has_operator_new_test.test | RUN | 0.212483 | 0.680565 | 1.406566 | 0.001868 | 0.001724 | 0.002014 |
| has_nothrow_destructor_test.test | RUN | 0.218393 | 0.725946 | 1.492153 | 0.001708 | 0.002289 | 0.001682 |
| has_nothrow_copy_test.test | RUN | 0.216866 | 0.699263 | 1.433176 | 0.001877 | 0.001896 | 0.002006 |
| has_nothrow_constr_test.test | RUN | 0.209343 | 0.685388 | 1.389160 | 0.001743 | 0.001972 | 0.002118 |
| has_nothrow_assign_test.test | RUN | 0.206041 | 0.673029 | 1.384528 | 0.001954 | 0.002166 | 0.001848 |
| has_not_equal_to_test.test | RUN | 0.325079 | 1.756811 | 3.044951 | 0.001800 | 0.002104 | 0.002131 |
| has_negate_test.test | RUN | 0.309846 | 1.214500 | 2.868323 | 0.002070 | 0.001987 | 0.001837 |
| has_multiplies_test.test | RUN | 0.320806 | 1.338950 | 2.998428 | 0.001763 | 0.001965 | 0.002054 |
| has_multiplies_assign_test.test | RUN | 0.319752 | 1.314007 | 2.953517 | 0.002018 | 0.002020 | 0.001915 |
| has_modulus_test.test | RUN | 0.324286 | 1.319050 | 2.959659 | 0.001870 | 0.002103 | 0.002074 |
| has_modulus_assign_test.test | RUN | 0.316525 | 1.310125 | 2.995937 | 0.001907 | 0.001798 | 0.001989 |
| has_minus_test.test | RUN | 0.332576 | 1.282839 | 2.976736 | 0.001362 | 0.002068 | 0.002098 |
| has_minus_assign_test.test | RUN | 0.337776 | 1.334002 | 2.988905 | 0.001777 | 0.001985 | 0.002022 |
| has_logical_or_test.test | RUN | 0.323174 | 1.319870 | 2.967436 | 0.001807 | 0.002192 | 0.001928 |
| has_logical_not_test.test | RUN | 0.309302 | 1.259239 | 2.910489 | 0.001699 | 0.001731 | 0.001940 |
| has_logical_and_test.test | RUN | 0.319619 | 1.279030 | 2.976412 | 0.001683 | 0.002136 | 0.001930 |
| has_less_test.test | RUN | 0.328981 | 1.808233 | 2.976690 | 0.001682 | 0.002011 | 0.001986 |
| has_less_equal_test.test | RUN | 0.325433 | 1.733303 | 3.024520 | 0.001926 | 0.001952 | 0.002084 |
| has_left_shift_test.test | RUN | 0.353538 | 1.601501 | 3.642418 | 0.001779 | 0.002134 | 0.001926 |
| has_left_shift_assign_test.test | RUN | 0.317814 | 1.246434 | 2.933551 | 0.001555 | 0.001976 | 0.001978 |
| has_greater_test.test | RUN | 0.322400 | 1.773584 | 2.986041 | 0.001893 | 0.001846 | 0.002078 |
| has_greater_equal_test.test | RUN | 0.327515 | 1.770971 | 3.002897 | 0.001738 | 0.001998 | 0.002003 |
| has_equal_to_test.test | RUN | 0.312340 | 1.744380 | 3.020520 | 0.001935 | 0.002012 | 0.001738 |
| has_divides_test.test | RUN | 0.315207 | 1.251675 | 2.923110 | 0.001543 | 0.001741 | 0.001550 |
| has_divides_assign_test.test | RUN | 0.324478 | 1.265658 | 2.919535 | 0.002022 | 0.002104 | 0.001870 |
| has_dereference_test.test | RUN | 0.319239 | 1.253934 | 2.878556 | 0.001886 | 0.001972 | 0.002048 |
| has_complement_test.test | RUN | 0.315958 | 1.806358 | 2.981867 | 0.001686 | 0.002092 | 0.001548 |
| has_bit_xor_test.test | RUN | 0.326552 | 1.301541 | 2.962078 | 0.001801 | 0.002128 | 0.001912 |
| has_bit_xor_assign_test.test | RUN | 0.323280 | 1.299182 | 2.921897 | 0.001945 | 0.002038 | 0.001880 |
| has_bit_or_test.test | RUN | 0.323979 | 1.244411 | 2.967050 | 0.001896 | 0.001948 | 0.002010 |
| has_bit_or_assign_test.test | RUN | 0.316518 | 1.274455 | 2.988935 | 0.001978 | 0.001947 | 0.001541 |
| has_bit_and_test.test | RUN | 0.328291 | 1.253814 | 2.954913 | 0.001457 | 0.002038 | 0.002083 |
| has_bit_and_assign_test.test | RUN | 0.329881 | 1.288870 | 2.976385 | 0.001907 | 0.002001 | 0.001926 |
| has_binary_classes9_test.test | RUN | 0.355766 | 1.464588 | 3.259049 | 0.002024 | 0.001995 | 0.002058 |
| has_binary_classes8_test.test | RUN | 0.349226 | 1.484752 | 3.295859 | 0.001848 | 0.002064 | 0.001620 |
| has_binary_classes7_test.test | RUN | 0.349073 | 1.511599 | 3.219589 | 0.001815 | 0.002100 | 0.001612 |
| has_binary_classes6_test.test | RUN | 0.351466 | 1.439512 | 3.173680 | 0.001972 | 0.002010 | 0.001956 |
| has_binary_classes5_test.test | RUN | 0.355234 | 1.443263 | 3.247170 | 0.001884 | 0.002074 | 0.001933 |
| has_binary_classes4_test.test | RUN | 0.352557 | 1.518375 | 3.275398 | 0.001971 | 0.001951 | 0.001724 |
| has_binary_classes3_test.test | RUN | 0.350533 | 1.521138 | 3.273979 | 0.001815 | 0.002043 | 0.001674 |
| has_binary_classes2_test.test | RUN | 0.356289 | 1.518996 | 3.251107 | 0.002030 | 0.001953 | 0.001941 |
| has_binary_classes1_test.test | RUN | 0.351406 | 1.503074 | 3.233150 | 0.001711 | 0.001902 | 0.002090 |
| has_binary_classes0_test.test | RUN | 0.350410 | 1.486541 | 3.177780 | 0.001880 | 0.002006 | 0.002044 |
| function_traits_test.test | RUN | 0.156021 | 0.385934 | 0.725750 | 0.001692 | 0.001877 | 0.002036 |
| floating_point_promotion_test.test | RUN | 0.149695 | 0.383294 | 0.669026 | 0.001830 | 0.001932 | 0.001894 |
| extent_test.test | RUN | 0.153762 | 0.397452 | 0.684251 | 0.001865 | 0.001730 | 0.001702 |
| enable_if_test.test | RUN | 0.143610 | 0.319291 | 0.531089 | 0.002012 | 0.001798 | 0.001733 |
| disjunction_test.test | RUN | 0.145557 | 0.339915 | 0.566403 | 0.002035 | 0.002034 | 0.001910 |
| detected_test.test | RUN | 0.137424 | 0.325367 | 0.526484 | 0.001967 | 0.001883 | 0.001724 |
| detected_or_test.test | RUN | 0.147338 | 0.351340 | 0.565141 | 0.001913 | 0.002270 | 0.002054 |
| decay_test.test | RUN | 0.395255 | 1.460347 | 3.382659 | 0.001843 | 0.002334 | 0.001904 |
| cxx14_aliases_test.test | RUN | 0.231874 | 0.513595 | 0.884193 | 0.001822 | 0.001996 | 0.001706 |
| copy_reference_test.test | RUN | 0.143903 | 0.351166 | 0.589196 | 0.001761 | 0.001632 | 0.001814 |
| copy_cv_test.test | RUN | 0.147538 | 0.368071 | 0.645741 | 0.001787 | 0.001890 | 0.002002 |
| copy_cv_ref_test.test | RUN | 0.212194 | 0.647500 | 1.436253 | 0.001864 | 0.001934 | 0.002038 |
| conjunction_test.test | RUN | 0.143768 | 0.346946 | 0.587770 | 0.001505 | 0.001662 | 0.002028 |
| conditional_test.test | RUN | 0.140183 | 0.355064 | 0.566407 | 0.001658 | 0.001689 | 0.002072 |
| common_type_test.test | RUN | 0.198128 | 0.539915 | 1.106463 | 0.001694 | 0.001693 | 0.002060 |
| common_type_sfinae_test.test | RUN | 0.139387 | 0.326757 | 0.555334 | 0.001439 | 0.001878 | 0.001929 |
| common_type_sfinae2_test.test | RUN | 0.141864 | 0.340094 | 0.550095 | 0.001607 | 0.001794 | 0.001762 |
| common_type_6_test.test | RUN | 0.149198 | 0.364499 | 0.563865 | 0.001657 | 0.002012 | 0.001896 |
| common_type_5_test.test | RUN | 0.154468 | 0.345648 | 0.587548 | 0.001772 | 0.001750 | 0.001916 |
| common_type_4_test.test | RUN | 0.159425 | 0.387604 | 0.709133 | 0.001910 | 0.001934 | 0.001986 |
| common_type_3_test.test | RUN | 0.138166 | 0.324441 | 0.465232 | 0.001776 | 0.001654 | 0.002082 |
| common_type_2_test.test | RUN | 0.192214 | 0.551137 | 1.106359 | 0.001872 | 0.001914 | 0.001718 |
| alignment_of_test.test | RUN | 0.150832 | 0.415898 | 0.700083 | 0.001922 | 0.001768 | 0.001866 |
| alignment_of_a2_test.test | RUN | 0.155494 | 0.416245 | 0.728523 | 0.001892 | 0.002027 | 0.001974 |
| aligned_storage_test.test | RUN | 0.162217 | 0.404158 | 0.627860 | 0.002052 | 0.001833 | 0.002124 |
| aligned_storage_empy_test.test | RUN | 0.180254 | 0.458736 | 0.858454 | 0.001918 | 0.002122 | 0.001904 |
| aligned_storage_a2_test.test | RUN | 0.158637 | 0.392281 | 0.630591 | 0.001941 | 0.002106 | 0.002309 |
| add_volatile_test.test | RUN | 0.350914 | 1.265352 | 3.313822 | 0.001788 | 0.001813 | 0.002103 |
| add_rvalue_reference_test.test | RUN | 0.395673 | 1.434100 | 3.788886 | 0.001739 | 0.001879 | 0.001966 |
| add_reference_test.test | RUN | 0.384321 | 1.416671 | 3.694745 | 0.001842 | 0.002074 | 0.002023 |
| add_pointer_test.test | RUN | 0.246529 | 0.773037 | 1.927073 | 0.001776 | 0.001808 | 0.001950 |
| add_lvalue_reference_test.test | RUN | 0.375824 | 1.442339 | 3.811529 | 0.001819 | 0.001982 | 0.001939 |
| add_cv_test.test | RUN | 0.367525 | 1.291272 | 3.485558 | 0.001812 | 0.001783 | 0.001792 |
| add_const_test.test | RUN | 0.355533 | 1.283502 | 3.497007 | 0.001599 | 0.001637 | 0.001865 |
| is_virtual_base_of_fail.test | COMPILE_FAIL | 0.138024 | 0.323057 | 0.525476 | — | — | — |
| is_stateless_fail.test | COMPILE_FAIL | 0.138601 | 0.331829 | 0.547845 | — | — | — |
| is_polymorphic_fail.test | COMPILE_FAIL | 0.133449 | 0.308426 | 0.521403 | — | — | — |
| is_nothrow_move_constructible_fail.test | COMPILE_FAIL | 0.130773 | 0.322129 | 0.484110 | — | — | — |
| is_nothrow_move_assignable_fail.test | COMPILE_FAIL | 0.130776 | 0.305334 | 0.500177 | — | — | — |
| is_list_constructible_fail.test | COMPILE_FAIL | 0.134333 | 0.300863 | 0.473474 | — | — | — |
| is_empty_fail.test | COMPILE_FAIL | 0.138979 | 0.310378 | 0.506021 | — | — | — |
| is_destructible_fail.test | COMPILE_FAIL | 0.129482 | 0.320864 | 0.510043 | — | — | — |
| is_default_constructible_fail.test | COMPILE_FAIL | 0.129591 | 0.292311 | 0.508198 | — | — | — |
| is_copy_constructible_fail.test | COMPILE_FAIL | 0.131990 | 0.299710 | 0.494364 | — | — | — |
| is_copy_assignable_fail.test | COMPILE_FAIL | 0.130134 | 0.317786 | 0.513991 | — | — | — |
| is_convertible_fail.test | COMPILE_FAIL | 0.135098 | 0.321784 | 0.500314 | — | — | — |
| is_convertible2_fail.test | COMPILE_FAIL | 0.134280 | 0.311286 | 0.505065 | — | — | — |
| is_constructible_fail.test | COMPILE_FAIL | 0.135223 | 0.310322 | 0.517338 | — | — | — |
| is_base_of_fail.test | COMPILE_FAIL | 0.134702 | 0.310160 | 0.492030 | — | — | — |
| is_assignable_fail.test | COMPILE_FAIL | 0.132170 | 0.321445 | 0.512466 | — | — | — |
| has_nothrow_destruct_fail.test | COMPILE_FAIL | 0.131928 | 0.313101 | 0.496222 | — | — | — |
| has_nothrow_copy_fail.test | COMPILE_FAIL | 0.135783 | 0.312088 | 0.518826 | — | — | — |
| has_nothrow_cons_fail.test | COMPILE_FAIL | 0.133818 | 0.317465 | 0.503483 | — | — | — |
| has_nothrow_assign_fail.test | COMPILE_FAIL | 0.135428 | 0.309631 | 0.503096 | — | — | — |
| common_type_fail.test | COMPILE_FAIL | 0.134244 | 0.314358 | 0.519502 | — | — | — |
| common_type2_fail.test | COMPILE_FAIL | 0.136384 | 0.324474 | 0.498911 | — | — | — |
| has_nothrow_assign_test_no_intrinsics.test | RUN | 0.218328 | 0.712121 | 1.469494 | 0.001840 | 0.001830 | 0.002014 |
| has_nothrow_constr_test_no_intrinsics.test | RUN | 0.208374 | 0.685859 | 1.417229 | 0.001728 | 0.001743 | 0.001702 |
| has_nothrow_copy_test_no_intrinsics.test | RUN | 0.214782 | 0.704237 | 1.404767 | 0.001349 | 0.001874 | 0.001754 |
| is_nothrow_move_assignable_test_no_intrinsics.test | RUN | 0.214794 | 0.690568 | 1.421337 | 0.001234 | 0.002048 | 0.001350 |
| is_nothrow_move_constructible_test_no_intrinsics.test | RUN | 0.122242 | 0.407173 | 0.855439 | 0.001424 | 0.001338 | 0.001359 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.996× | 0.996×, 0.996× | 1.004× |
| clang | 1.023× | 1.028×, 1.019× | 0.998× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824563).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| add_const_test.test | 10 | 0.001513s | 0.001470s | 0.001468s | byte equality observed |
| add_cv_test.test | 10 | 0.001458s | 0.001434s | 0.001446s | byte equality observed |
| add_lvalue_reference_test.test | 10 | 0.001488s | 0.001454s | 0.001461s | byte equality observed |
| add_pointer_test.test | 10 | 0.001454s | 0.001462s | 0.001462s | byte equality observed |
| add_reference_test.test | 9 | 0.001464s | 0.001473s | 0.001470s | byte equality observed |
| add_rvalue_reference_test.test | 10 | 0.001440s | 0.001433s | 0.001451s | byte equality observed |
| add_volatile_test.test | 10 | 0.001464s | 0.001440s | 0.001444s | byte equality observed |
| aligned_storage_a2_test.test | 10 | 0.001577s | 0.001633s | 0.001589s | byte equality observed |
| aligned_storage_empy_test.test | 10 | 0.001448s | 0.001451s | 0.001509s | byte equality observed |
| aligned_storage_test.test | 10 | 0.001593s | 0.001590s | 0.001553s | byte equality observed |
| alignment_of_a2_test.test | 10 | 0.001463s | 0.001460s | 0.001434s | byte equality observed |
| alignment_of_test.test | 10 | 0.001485s | 0.001416s | 0.001475s | byte equality observed |
| common_type_2_test.test | 10 | 0.001446s | 0.001424s | 0.001447s | byte equality observed |
| common_type_3_test.test | 10 | 0.001455s | 0.001439s | 0.001434s | byte equality observed |
| common_type_4_test.test | 10 | 0.001437s | 0.001435s | 0.001478s | byte equality observed |
| common_type_5_test.test | 8 | 0.001437s | 0.001437s | 0.001476s | byte equality observed |
| common_type_6_test.test | 10 | 0.001461s | 0.001472s | 0.001438s | byte equality observed |
| common_type_sfinae2_test.test | 10 | 0.001460s | 0.001416s | 0.001462s | byte equality observed |
| common_type_sfinae_test.test | 10 | 0.001453s | 0.001461s | 0.001428s | byte equality observed |
| common_type_test.test | 10 | 0.001465s | 0.001443s | 0.001447s | byte equality observed |
| conditional_test.test | 10 | 0.001459s | 0.001438s | 0.001460s | byte equality observed |
| conjunction_test.test | 10 | 0.001426s | 0.001450s | 0.001434s | byte equality observed |
| copy_cv_ref_test.test | 10 | 0.001490s | 0.001430s | 0.001423s | byte equality observed |
| copy_cv_test.test | 10 | 0.001451s | 0.001437s | 0.001456s | byte equality observed |
| copy_reference_test.test | 10 | 0.001461s | 0.001455s | 0.001441s | byte equality observed |
| cxx14_aliases_test.test | 10 | 0.001484s | 0.001424s | 0.001485s | byte equality observed |
| decay_test.test | 10 | 0.001474s | 0.001498s | 0.001470s | byte equality observed |
| detected_or_test.test | 10 | 0.001439s | 0.001459s | 0.001487s | byte equality observed |
| detected_test.test | 10 | 0.001465s | 0.001442s | 0.001438s | byte equality observed |
| disjunction_test.test | 10 | 0.001491s | 0.001457s | 0.001440s | byte equality observed |
| enable_if_test.test | 9 | 0.001442s | 0.001450s | 0.001460s | byte equality observed |
| extent_test.test | 10 | 0.001447s | 0.001428s | 0.001474s | byte equality observed |
| floating_point_promotion_test.test | 10 | 0.001470s | 0.001476s | 0.001421s | byte equality observed |
| function_traits_test.test | 10 | 0.001435s | 0.001450s | 0.001461s | byte equality observed |
| has_binary_classes0_test.test | 10 | 0.001441s | 0.001426s | 0.001451s | byte equality observed |
| has_binary_classes1_test.test | 10 | 0.001466s | 0.001403s | 0.001425s | byte equality observed |
| has_binary_classes2_test.test | 10 | 0.001453s | 0.001445s | 0.001441s | byte equality observed |
| has_binary_classes3_test.test | 10 | 0.001448s | 0.001445s | 0.001471s | byte equality observed |
| has_binary_classes4_test.test | 10 | 0.001438s | 0.001449s | 0.001429s | byte equality observed |
| has_binary_classes5_test.test | 10 | 0.001447s | 0.001436s | 0.001476s | byte equality observed |
| has_binary_classes6_test.test | 10 | 0.001458s | 0.001442s | 0.001452s | byte equality observed |
| has_binary_classes7_test.test | 10 | 0.001442s | 0.001444s | 0.001478s | byte equality observed |
| has_binary_classes8_test.test | 10 | 0.001444s | 0.001430s | 0.001485s | byte equality observed |
| has_binary_classes9_test.test | 10 | 0.001461s | 0.001487s | 0.001453s | byte equality observed |
| has_bit_and_assign_test.test | 10 | 0.001448s | 0.001454s | 0.001447s | byte equality observed |
| has_bit_and_test.test | 10 | 0.001448s | 0.001435s | 0.001470s | byte equality observed |
| has_bit_or_assign_test.test | 10 | 0.001424s | 0.001432s | 0.001427s | byte equality observed |
| has_bit_or_test.test | 10 | 0.001471s | 0.001463s | 0.001415s | byte equality observed |
| has_bit_xor_assign_test.test | 10 | 0.001448s | 0.001448s | 0.001446s | byte equality observed |
| has_bit_xor_test.test | 10 | 0.001454s | 0.001439s | 0.001460s | byte equality observed |
| has_complement_test.test | 10 | 0.001445s | 0.001469s | 0.001416s | byte equality observed |
| has_dereference_test.test | 10 | 0.001449s | 0.001451s | 0.001458s | byte equality observed |
| has_divides_assign_test.test | 10 | 0.001455s | 0.001451s | 0.001456s | byte equality observed |
| has_divides_test.test | 10 | 0.001431s | 0.001454s | 0.001433s | byte equality observed |
| has_equal_to_test.test | 10 | 0.001452s | 0.001469s | 0.001469s | byte equality observed |
| has_greater_equal_test.test | 10 | 0.001451s | 0.001452s | 0.001458s | byte equality observed |
| has_greater_test.test | 10 | 0.001445s | 0.001451s | 0.001452s | byte equality observed |
| has_left_shift_assign_test.test | 10 | 0.001472s | 0.001490s | 0.001466s | byte equality observed |
| has_left_shift_test.test | 10 | 0.001445s | 0.001458s | 0.001436s | byte equality observed |
| has_less_equal_test.test | 10 | 0.001478s | 0.001458s | 0.001454s | byte equality observed |
| has_less_test.test | 10 | 0.001451s | 0.001471s | 0.001447s | byte equality observed |
| has_logical_and_test.test | 10 | 0.001420s | 0.001474s | 0.001446s | byte equality observed |
| has_logical_not_test.test | 10 | 0.001486s | 0.001439s | 0.001439s | byte equality observed |
| has_logical_or_test.test | 10 | 0.001458s | 0.001500s | 0.001524s | byte equality observed |
| has_minus_assign_test.test | 10 | 0.001475s | 0.001452s | 0.001472s | byte equality observed |
| has_minus_test.test | 10 | 0.001456s | 0.001462s | 0.001466s | byte equality observed |
| has_modulus_assign_test.test | 10 | 0.001423s | 0.001484s | 0.001453s | byte equality observed |
| has_modulus_test.test | 10 | 0.001450s | 0.001461s | 0.001456s | byte equality observed |
| has_multiplies_assign_test.test | 10 | 0.001460s | 0.001453s | 0.001457s | byte equality observed |
| has_multiplies_test.test | 10 | 0.001472s | 0.001441s | 0.001444s | byte equality observed |
| has_negate_test.test | 10 | 0.001453s | 0.001448s | 0.001448s | byte equality observed |
| has_not_equal_to_test.test | 10 | 0.001462s | 0.001453s | 0.001424s | byte equality observed |
| has_nothrow_assign_test.test | 10 | 0.001444s | 0.001494s | 0.001447s | byte equality observed |
| has_nothrow_assign_test_no_intrinsics.test | 10 | 0.001452s | 0.001494s | 0.001448s | byte equality observed |
| has_nothrow_constr_test.test | 10 | 0.001450s | 0.001461s | 0.001429s | byte equality observed |
| has_nothrow_constr_test_no_intrinsics.test | 10 | 0.001449s | 0.001505s | 0.001483s | byte equality observed |
| has_nothrow_copy_test.test | 10 | 0.001460s | 0.001479s | 0.001488s | byte equality observed |
| has_nothrow_copy_test_no_intrinsics.test | 10 | 0.001468s | 0.001421s | 0.001442s | byte equality observed |
| has_nothrow_destructor_test.test | 10 | 0.001465s | 0.001446s | 0.001453s | byte equality observed |
| has_operator_new_test.test | 10 | 0.001467s | 0.001473s | 0.001463s | byte equality observed |
| has_plus_assign_test.test | 10 | 0.001434s | 0.001425s | 0.001457s | byte equality observed |
| has_plus_test.test | 10 | 0.001444s | 0.001455s | 0.001455s | byte equality observed |
| has_post_decrement_test.test | 10 | 0.001466s | 0.001430s | 0.001463s | byte equality observed |
| has_post_increment_test.test | 10 | 0.001455s | 0.001466s | 0.001466s | byte equality observed |
| has_postfix_classes0_test.test | 10 | 0.001460s | 0.001450s | 0.001470s | byte equality observed |
| has_postfix_classes1_test.test | 10 | 0.001461s | 0.001484s | 0.001483s | byte equality observed |
| has_postfix_classes2_test.test | 10 | 0.001452s | 0.001473s | 0.001477s | byte equality observed |
| has_postfix_classes3_test.test | 10 | 0.001462s | 0.001464s | 0.001448s | byte equality observed |
| has_pre_decrement_test.test | 10 | 0.001450s | 0.001424s | 0.001441s | byte equality observed |
| has_pre_increment_test.test | 10 | 0.001456s | 0.001466s | 0.001426s | byte equality observed |
| has_prefix_classes0_test.test | 10 | 0.001452s | 0.001459s | 0.001452s | byte equality observed |
| has_prefix_classes1_test.test | 10 | 0.001491s | 0.001461s | 0.001457s | byte equality observed |
| has_prefix_classes2_test.test | 10 | 0.001469s | 0.001441s | 0.001449s | byte equality observed |
| has_prefix_classes3_test.test | 10 | 0.001445s | 0.001435s | 0.001427s | byte equality observed |
| has_right_shift_assign_test.test | 10 | 0.001462s | 0.001451s | 0.001445s | byte equality observed |
| has_right_shift_test.test | 10 | 0.001485s | 0.001436s | 0.001487s | byte equality observed |
| has_trivial_assign_test.test | 10 | 0.001558s | 0.001525s | 0.001537s | byte equality observed |
| has_trivial_constr_test.test | 10 | 0.001512s | 0.001511s | 0.001580s | native test oracle; output varies |
| has_trivial_copy_test.test | 10 | 0.001544s | 0.001545s | 0.001576s | native test oracle; output varies |
| has_trivial_destructor_test.test | 10 | 0.001452s | 0.001468s | 0.001442s | byte equality observed |
| has_trivial_move_assign_test.test | 8 | 0.001456s | 0.001442s | 0.001462s | byte equality observed |
| has_trivial_move_constructor_test.test | 9 | 0.001455s | 0.001497s | 0.001477s | byte equality observed |
| has_unary_minus_test.test | 10 | 0.001489s | 0.001458s | 0.001433s | byte equality observed |
| has_unary_plus_test.test | 10 | 0.001478s | 0.001446s | 0.001422s | byte equality observed |
| has_virtual_destructor_test.test | 10 | 0.001504s | 0.001455s | 0.001437s | byte equality observed |
| integral_constant_test.test | 9 | 0.000646s | 0.000643s | 0.001424s | byte equality observed |
| is_abstract_test.test | 10 | 0.001488s | 0.001452s | 0.001465s | byte equality observed |
| is_arithmetic_test.test | 10 | 0.001466s | 0.001487s | 0.001452s | byte equality observed |
| is_array_test.test | 10 | 0.001466s | 0.001451s | 0.001459s | byte equality observed |
| is_assignable_test.test | 7 | 0.001449s | 0.001464s | 0.001460s | byte equality observed |
| is_base_and_derived_test.test | 9 | 0.001450s | 0.001445s | 0.001460s | byte equality observed |
| is_base_of_test.test | 10 | 0.001479s | 0.001476s | 0.001392s | byte equality observed |
| is_bounded_array_test.test | 10 | 0.001430s | 0.001450s | 0.001444s | byte equality observed |
| is_class_test.test | 10 | 0.001456s | 0.001430s | 0.001455s | byte equality observed |
| is_complete_test.test | 10 | 0.001477s | 0.001486s | 0.001448s | byte equality observed |
| is_complex_test.test | 10 | 0.001461s | 0.001429s | 0.001439s | byte equality observed |
| is_compound_test.test | 9 | 0.001459s | 0.001473s | 0.001479s | byte equality observed |
| is_const_test.test | 10 | 0.001477s | 0.001423s | 0.001454s | byte equality observed |
| is_constructible_test.test | 10 | 0.001432s | 0.001424s | 0.001462s | byte equality observed |
| is_convertible_test.test | 10 | 0.001450s | 0.001453s | 0.001472s | byte equality observed |
| is_copy_assignable_test.test | 10 | 0.001453s | 0.001423s | 0.001466s | byte equality observed |
| is_copy_constructible_test.test | 10 | 0.001470s | 0.001476s | 0.001419s | byte equality observed |
| is_default_constr_test.test | 10 | 0.001448s | 0.001443s | 0.001440s | byte equality observed |
| is_destructible_test.test | 9 | 0.001471s | 0.001478s | 0.001489s | byte equality observed |
| is_detected_exact_test.test | 10 | 0.001472s | 0.001440s | 0.001444s | byte equality observed |
| is_detected_test.test | 10 | 0.001454s | 0.001451s | 0.001443s | byte equality observed |
| is_empty_test.test | 10 | 0.001427s | 0.001459s | 0.001473s | byte equality observed |
| is_enum_test.test | 10 | 0.001454s | 0.001433s | 0.001467s | byte equality observed |
| is_final_test.test | 10 | 0.001443s | 0.001424s | 0.001418s | byte equality observed |
| is_float_test.test | 10 | 0.001459s | 0.001475s | 0.001444s | byte equality observed |
| is_floating_point_test.test | 10 | 0.001434s | 0.001454s | 0.001414s | byte equality observed |
| is_function_test.test | 10 | 0.001462s | 0.001415s | 0.001440s | byte equality observed |
| is_fundamental_test.test | 10 | 0.001446s | 0.001478s | 0.001447s | byte equality observed |
| is_integral_test.test | 10 | 0.001459s | 0.001466s | 0.001439s | byte equality observed |
| is_list_constructible_test.test | 10 | 0.001459s | 0.001448s | 0.001461s | byte equality observed |
| is_lvalue_reference_test.test | 10 | 0.001445s | 0.001457s | 0.001450s | byte equality observed |
| is_member_func_test.test | 10 | 0.001435s | 0.001473s | 0.001457s | byte equality observed |
| is_member_obj_test.test | 10 | 0.001439s | 0.001488s | 0.001455s | byte equality observed |
| is_member_pointer_test.test | 10 | 0.001473s | 0.001438s | 0.001415s | byte equality observed |
| is_noncopyable_test.test | 9 | 0.001481s | 0.001427s | 0.001445s | byte equality observed |
| is_nothrow_move_assignable_test.test | 9 | 0.001472s | 0.001450s | 0.001446s | byte equality observed |
| is_nothrow_move_assignable_test_no_intrinsics.test | 10 | 0.001463s | 0.001424s | 0.001442s | byte equality observed |
| is_nothrow_move_constructible_test.test | 10 | 0.001474s | 0.001423s | 0.001410s | byte equality observed |
| is_nothrow_move_constructible_test_no_intrinsics.test | 10 | 0.001449s | 0.001470s | 0.001434s | byte equality observed |
| is_nothrow_swappable_test.test | 9 | 0.001467s | 0.001458s | 0.001437s | byte equality observed |
| is_object_test.test | 10 | 0.001460s | 0.001425s | 0.001421s | byte equality observed |
| is_pod_test.test | 10 | 0.001439s | 0.001471s | 0.001474s | byte equality observed |
| is_pointer_test.test | 10 | 0.001483s | 0.001468s | 0.001440s | byte equality observed |
| is_polymorphic_test.test | 10 | 0.001467s | 0.001440s | 0.001464s | byte equality observed |
| is_reference_test.test | 10 | 0.001462s | 0.001441s | 0.001474s | byte equality observed |
| is_rvalue_reference_test.test | 10 | 0.001442s | 0.001437s | 0.001447s | byte equality observed |
| is_same_test.test | 10 | 0.001471s | 0.001450s | 0.001453s | byte equality observed |
| is_scalar_test.test | 10 | 0.001444s | 0.001467s | 0.001453s | byte equality observed |
| is_scoped_enum_test.test | 10 | 0.001455s | 0.001452s | 0.001439s | byte equality observed |
| is_signed_test.test | 10 | 0.001449s | 0.001447s | 0.001430s | byte equality observed |
| is_stateless_test.test | 10 | 0.001461s | 0.001470s | 0.001484s | byte equality observed |
| is_swappable_test.test | 10 | 0.001449s | 0.001456s | 0.001454s | byte equality observed |
| is_trivially_copyable_test.test | 9 | 0.001504s | 0.001487s | 0.001457s | byte equality observed |
| is_unbounded_array_test.test | 10 | 0.001461s | 0.001465s | 0.001459s | byte equality observed |
| is_union_test.test | 10 | 0.001447s | 0.001444s | 0.001453s | byte equality observed |
| is_unscoped_enum_test.test | 10 | 0.001465s | 0.001445s | 0.001446s | byte equality observed |
| is_unsigned_test.test | 10 | 0.001446s | 0.001442s | 0.001470s | byte equality observed |
| is_virtual_base_of_test.test | 10 | 0.001431s | 0.001417s | 0.001455s | byte equality observed |
| is_void_test.test | 10 | 0.001441s | 0.001475s | 0.001443s | byte equality observed |
| is_volatile_test.test | 10 | 0.001472s | 0.001430s | 0.001419s | byte equality observed |
| make_signed_test.test | 10 | 0.001447s | 0.001462s | 0.001427s | byte equality observed |
| make_unsigned_test.test | 10 | 0.001450s | 0.001496s | 0.001485s | byte equality observed |
| make_void_test.test | 10 | 0.001464s | 0.001511s | 0.001471s | byte equality observed |
| mpl_interop_test1.test | 10 | 0.000639s | 0.000650s | 0.001435s | byte equality observed |
| mpl_interop_test2.test | 10 | 0.000651s | 0.000668s | 0.001470s | byte equality observed |
| mpl_interop_test3.test | 10 | 0.000654s | 0.000671s | 0.001449s | byte equality observed |
| negation_test.test | 10 | 0.001535s | 0.001523s | 0.001433s | byte equality observed |
| promote_basic_test.test | 10 | 0.000656s | 0.000665s | 0.001426s | byte equality observed |
| promote_enum_msvc_bug_test.test | 10 | 0.000643s | 0.000658s | 0.001460s | byte equality observed |
| promote_enum_test.test | 10 | 0.000646s | 0.000671s | 0.001439s | byte equality observed |
| promote_mpl_test.test | 10 | 0.000654s | 0.000656s | 0.001481s | byte equality observed |
| rank_test.test | 10 | 0.001470s | 0.001468s | 0.001464s | byte equality observed |
| remove_all_extents_test.test | 10 | 0.001487s | 0.001492s | 0.001485s | byte equality observed |
| remove_bounds_test.test | 10 | 0.001458s | 0.001464s | 0.001479s | byte equality observed |
| remove_const_test.test | 10 | 0.001461s | 0.001462s | 0.001478s | byte equality observed |
| remove_cv_ref_test.test | 10 | 0.001437s | 0.001439s | 0.001458s | byte equality observed |
| remove_cv_test.test | 10 | 0.001453s | 0.001471s | 0.001485s | byte equality observed |
| remove_extent_test.test | 10 | 0.001458s | 0.001471s | 0.001450s | byte equality observed |
| remove_pointer_test.test | 10 | 0.001471s | 0.001461s | 0.001479s | byte equality observed |
| remove_reference_test.test | 10 | 0.001464s | 0.001428s | 0.001455s | byte equality observed |
| remove_volatile_test.test | 10 | 0.001453s | 0.001455s | 0.001455s | byte equality observed |
| tricky_abstract_type_test.test | 10 | 0.001452s | 0.001410s | 0.001455s | byte equality observed |
| tricky_add_pointer_test.test | 10 | 0.001472s | 0.001407s | 0.001451s | byte equality observed |
| tricky_function_type_test.test | 10 | 0.001464s | 0.001451s | 0.001431s | byte equality observed |
| tricky_incomplete_type_test.test | 10 | 0.001470s | 0.001402s | 0.001466s | byte equality observed |
| tricky_is_enum_test.test | 10 | 0.001467s | 0.001419s | 0.001479s | byte equality observed |
| tricky_partial_spec_test.test | 10 | 0.001468s | 0.001417s | 0.001500s | byte equality observed |
| tricky_rvalue_test.test | 10 | 0.001477s | 0.001474s | 0.001458s | byte equality observed |
| type_identity_test.test | 10 | 0.001504s | 0.001470s | 0.001439s | byte equality observed |
| type_traits_test.test | 9 | 0.000651s | 0.000647s | 0.001459s | byte equality observed |
| type_with_alignment_test.test | 10 | 0.001626s | 0.001610s | 0.001599s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824563).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
