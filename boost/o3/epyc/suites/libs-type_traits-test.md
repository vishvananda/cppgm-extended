# libs/type_traits/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| type_with_alignment_test.test | RUN | 0.158158 | 0.463277 | 0.795852 | 0.001805 | 0.002044 | 0.001974 |
| type_traits_test.test | RUN | 0.224681 | 0.482560 | 0.798750 | 0.000714 | 0.000747 | 0.002158 |
| type_identity_test.test | RUN | 0.439848 | 1.790352 | 4.462156 | 0.001955 | 0.002041 | 0.001957 |
| tricky_rvalue_test.test | RUN | 0.145339 | 0.372061 | 0.616484 | 0.001708 | 0.001640 | 0.001699 |
| tricky_partial_spec_test.test | RUN | 0.178022 | 0.546799 | 0.994077 | 0.001719 | 0.002055 | 0.001467 |
| tricky_is_enum_test.test | RUN | 0.138618 | 0.357036 | 0.560173 | 0.001862 | 0.002030 | 0.001930 |
| tricky_incomplete_type_test.test | RUN | 0.232174 | 0.554316 | 0.954115 | 0.001618 | 0.002120 | 0.001870 |
| tricky_function_type_test.test | RUN | 0.191784 | 0.592731 | 1.094995 | 0.001950 | 0.001872 | 0.001847 |
| tricky_add_pointer_test.test | RUN | 0.294232 | 1.056337 | 2.476334 | 0.001715 | 0.002176 | 0.001948 |
| tricky_abstract_type_test.test | RUN | 0.150518 | 0.372635 | 0.571761 | 0.001945 | 0.002183 | 0.002106 |
| remove_volatile_test.test | RUN | 0.403488 | 1.476205 | 3.906683 | 0.001986 | 0.001911 | 0.002000 |
| remove_reference_test.test | RUN | 0.408355 | 1.544714 | 4.194352 | 0.001818 | 0.001962 | 0.001928 |
| remove_pointer_test.test | RUN | 0.427174 | 1.633424 | 4.417396 | 0.001926 | 0.002244 | 0.002127 |
| remove_extent_test.test | RUN | 0.416455 | 1.585018 | 4.067468 | 0.001898 | 0.002140 | 0.002032 |
| remove_cv_test.test | RUN | 0.409347 | 1.587502 | 4.181450 | 0.001880 | 0.002129 | 0.002189 |
| remove_cv_ref_test.test | RUN | 0.854459 | 3.791211 | 10.284926 | 0.001833 | 0.002070 | 0.001888 |
| remove_const_test.test | RUN | 0.394612 | 1.436563 | 3.965901 | 0.001937 | 0.002018 | 0.001648 |
| remove_bounds_test.test | RUN | 0.416292 | 1.544837 | 4.205906 | 0.002068 | 0.002172 | 0.002096 |
| remove_all_extents_test.test | RUN | 0.411650 | 1.632755 | 4.054935 | 0.001788 | 0.001867 | 0.001702 |
| rank_test.test | RUN | 0.143165 | 0.382837 | 0.610935 | 0.001724 | 0.001906 | 0.001999 |
| promote_mpl_test.test | RUN | 0.128375 | 0.331619 | 0.408640 | 0.000783 | 0.000905 | 0.001994 |
| promote_enum_test.test | RUN | 0.029257 | 0.081534 | 0.102586 | 0.000713 | 0.000917 | 0.001960 |
| promote_enum_msvc_bug_test.test | RUN | 0.025775 | 0.063518 | 0.083338 | 0.000691 | 0.000862 | 0.002066 |
| promote_basic_test.test | RUN | 0.032217 | 0.094222 | 0.111602 | 0.000715 | 0.000837 | 0.002081 |
| negation_test.test | RUN | 0.145087 | 0.343885 | 0.571818 | 0.001747 | 0.002008 | 0.001850 |
| mpl_interop_test3.test | RUN | 0.121732 | 0.289421 | 0.381472 | 0.000714 | 0.000745 | 0.001671 |
| mpl_interop_test2.test | RUN | 0.042618 | 0.084139 | 0.090834 | 0.000737 | 0.000763 | 0.001902 |
| mpl_interop_test1.test | RUN | 0.021472 | 0.063324 | 0.062658 | 0.000634 | 0.000725 | 0.002023 |
| make_void_test.test | RUN | 0.149408 | 0.371561 | 0.636006 | 0.001705 | 0.002286 | 0.001923 |
| make_unsigned_test.test | RUN | 0.167844 | 0.499240 | 0.875195 | 0.001836 | 0.001948 | 0.001872 |
| make_signed_test.test | RUN | 0.167857 | 0.501056 | 0.878688 | 0.001547 | 0.001829 | 0.001731 |
| is_volatile_test.test | RUN | 0.155108 | 0.428615 | 0.735336 | 0.001712 | 0.001808 | 0.001838 |
| is_void_test.test | RUN | 0.149270 | 0.393670 | 0.635230 | 0.002208 | 0.002091 | 0.002024 |
| is_virtual_base_of_test.test | RUN | 0.174581 | 0.511959 | 0.888435 | 0.001768 | 0.001971 | 0.002085 |
| is_unsigned_test.test | RUN | 0.151765 | 0.388094 | 0.677305 | 0.001828 | 0.001598 | 0.001986 |
| is_unscoped_enum_test.test | RUN | 0.145473 | 0.370201 | 0.538588 | 0.001613 | 0.001807 | 0.002032 |
| is_union_test.test | RUN | 0.162975 | 0.462648 | 0.804637 | 0.001658 | 0.002050 | 0.002167 |
| is_unbounded_array_test.test | RUN | 0.151270 | 0.391700 | 0.630603 | 0.001690 | 0.002378 | 0.002037 |
| is_trivially_copyable_test.test | RUN | 0.246451 | 0.830093 | 1.612070 | 0.001634 | 0.002286 | 0.002037 |
| is_swappable_test.test | RUN | 0.197583 | 0.619782 | 1.249684 | 0.001619 | 0.002212 | 0.001905 |
| is_stateless_test.test | RUN | 0.227000 | 0.723439 | 1.443802 | 0.001786 | 0.001860 | 0.002190 |
| is_signed_test.test | RUN | 0.153385 | 0.388612 | 0.700468 | 0.001890 | 0.002182 | 0.001662 |
| is_scoped_enum_test.test | RUN | 0.153097 | 0.365037 | 0.523005 | 0.001564 | 0.002020 | 0.002278 |
| is_scalar_test.test | RUN | 0.197606 | 0.671855 | 1.277400 | 0.001709 | 0.001700 | 0.001691 |
| is_same_test.test | RUN | 0.149031 | 0.401121 | 0.614670 | 0.001850 | 0.001962 | 0.002042 |
| is_rvalue_reference_test.test | RUN | 0.155775 | 0.437193 | 0.780004 | 0.001717 | 0.002166 | 0.001956 |
| is_reference_test.test | RUN | 0.157577 | 0.450477 | 0.750338 | 0.001703 | 0.002198 | 0.002247 |
| is_polymorphic_test.test | RUN | 0.159935 | 0.452632 | 0.847144 | 0.001769 | 0.002264 | 0.001878 |
| is_pointer_test.test | RUN | 0.160303 | 0.440350 | 0.776855 | 0.001782 | 0.002124 | 0.001936 |
| is_pod_test.test | RUN | 0.217124 | 0.748449 | 1.443229 | 0.001738 | 0.002214 | 0.001934 |
| is_object_test.test | RUN | 0.148924 | 0.371492 | 0.589049 | 0.001792 | 0.002160 | 0.001788 |
| is_nothrow_swappable_test.test | RUN | 0.215025 | 0.666641 | 1.342109 | 0.001768 | 0.002144 | 0.002365 |
| is_nothrow_move_constructible_test.test | RUN | 0.210739 | 0.742070 | 1.386860 | 0.001868 | 0.001744 | 0.001896 |
| is_nothrow_move_assignable_test.test | RUN | 0.208952 | 0.737730 | 1.388231 | 0.001736 | 0.002018 | 0.001600 |
| is_noncopyable_test.test | RUN | 0.148343 | 0.371506 | 0.634444 | 0.001509 | 0.002025 | 0.001793 |
| is_member_pointer_test.test | RUN | 0.150050 | 0.387241 | 0.623788 | 0.001658 | 0.002121 | 0.002010 |
| is_member_obj_test.test | RUN | 0.150611 | 0.390214 | 0.645897 | 0.001894 | 0.001962 | 0.001625 |
| is_member_func_test.test | RUN | 0.151854 | 0.413257 | 0.712939 | 0.001709 | 0.002242 | 0.001705 |
| is_lvalue_reference_test.test | RUN | 0.153822 | 0.443640 | 0.710552 | 0.001600 | 0.002297 | 0.001596 |
| is_list_constructible_test.test | RUN | 0.178568 | 0.520548 | 0.839256 | 0.001737 | 0.002049 | 0.001682 |
| is_integral_test.test | RUN | 0.187528 | 0.630962 | 1.184895 | 0.001708 | 0.001966 | 0.001948 |
| is_fundamental_test.test | RUN | 0.192881 | 0.643495 | 1.225313 | 0.001569 | 0.002099 | 0.002060 |
| is_function_test.test | RUN | 0.155116 | 0.415272 | 0.665254 | 0.001881 | 0.001396 | 0.002050 |
| is_floating_point_test.test | RUN | 0.159594 | 0.453615 | 0.766878 | 0.001609 | 0.002127 | 0.001944 |
| is_float_test.test | RUN | 0.155876 | 0.450212 | 0.737757 | 0.001922 | 0.001868 | 0.002150 |
| is_final_test.test | RUN | 0.155223 | 0.458550 | 0.822414 | 0.001555 | 0.001766 | 0.002121 |
| is_enum_test.test | RUN | 0.146606 | 0.386502 | 0.631632 | 0.001580 | 0.001939 | 0.001735 |
| is_empty_test.test | RUN | 0.156912 | 0.404316 | 0.659103 | 0.001652 | 0.002210 | 0.001759 |
| is_detected_test.test | RUN | 0.141980 | 0.351102 | 0.518647 | 0.001670 | 0.001677 | 0.001636 |
| is_detected_exact_test.test | RUN | 0.138480 | 0.361229 | 0.571371 | 0.001934 | 0.001811 | 0.001984 |
| is_destructible_test.test | RUN | 0.202285 | 0.724624 | 1.409520 | 0.001782 | 0.002220 | 0.002094 |
| is_default_constr_test.test | RUN | 0.212207 | 0.703294 | 1.395879 | 0.001725 | 0.002177 | 0.002199 |
| is_copy_constructible_test.test | RUN | 0.216737 | 0.758470 | 1.525893 | 0.001484 | 0.001745 | 0.001876 |
| is_copy_assignable_test.test | RUN | 0.214609 | 0.740918 | 1.511929 | 0.001670 | 0.001796 | 0.001925 |
| is_convertible_test.test | RUN | 0.208351 | 0.659608 | 1.226389 | 0.001956 | 0.002260 | 0.001822 |
| is_constructible_test.test | RUN | 0.174323 | 0.468137 | 0.868569 | 0.001840 | 0.001959 | 0.001710 |
| is_const_test.test | RUN | 0.148223 | 0.416793 | 0.649836 | 0.001700 | 0.002062 | 0.001917 |
| is_compound_test.test | RUN | 0.144122 | 0.364433 | 0.531689 | 0.001814 | 0.002017 | 0.001926 |
| is_complex_test.test | RUN | 0.192970 | 0.474760 | 0.801114 | 0.001798 | 0.002035 | 0.001656 |
| is_complete_test.test | RUN | 0.168040 | 0.493207 | 0.881057 | 0.001802 | 0.001974 | 0.001619 |
| is_class_test.test | RUN | 0.157989 | 0.439058 | 0.776575 | 0.001649 | 0.001580 | 0.001642 |
| is_bounded_array_test.test | RUN | 0.148338 | 0.385147 | 0.613010 | 0.001936 | 0.002002 | 0.001696 |
| is_base_of_test.test | RUN | 0.153438 | 0.426064 | 0.729946 | 0.001813 | 0.002230 | 0.001894 |
| is_base_and_derived_test.test | RUN | 0.157237 | 0.422686 | 0.729031 | 0.001780 | 0.001984 | 0.001806 |
| is_assignable_test.test | RUN | 0.193810 | 0.600534 | 1.129996 | 0.001711 | 0.002196 | 0.001827 |
| is_array_test.test | RUN | 0.152748 | 0.412269 | 0.674600 | 0.001791 | 0.001946 | 0.001905 |
| is_arithmetic_test.test | RUN | 0.188345 | 0.620464 | 1.200724 | 0.001750 | 0.002190 | 0.002129 |
| is_abstract_test.test | RUN | 0.342431 | 1.696612 | 3.804627 | 0.001854 | 0.001872 | 0.001942 |
| integral_constant_test.test | RUN | 0.023959 | 0.063491 | 0.058452 | 0.000794 | 0.000945 | 0.001581 |
| has_virtual_destructor_test.test | RUN | 0.164012 | 0.487520 | 0.867501 | 0.001968 | 0.002181 | 0.001825 |
| has_unary_plus_test.test | RUN | 0.326616 | 1.958675 | 3.039632 | 0.001742 | 0.001976 | 0.001777 |
| has_unary_minus_test.test | RUN | 0.323090 | 1.908849 | 2.933280 | 0.001587 | 0.002130 | 0.002070 |
| has_trivial_move_constructor_test.test | RUN | 0.220199 | 0.700307 | 1.448110 | 0.001544 | 0.001668 | 0.002004 |
| has_trivial_move_assign_test.test | RUN | 0.218428 | 0.659014 | 1.323736 | 0.001920 | 0.002108 | 0.002079 |
| has_trivial_destructor_test.test | RUN | 0.213490 | 0.685045 | 1.424085 | 0.001748 | 0.001714 | 0.001899 |
| has_trivial_copy_test.test | RUN | 0.220868 | 0.782291 | 1.504040 | 0.001819 | 0.002222 | 0.002106 |
| has_trivial_constr_test.test | RUN | 0.220824 | 0.773178 | 1.513341 | 0.001768 | 0.002072 | 0.002122 |
| has_trivial_assign_test.test | RUN | 0.225532 | 0.745828 | 1.489894 | 0.002070 | 0.002248 | 0.002185 |
| has_right_shift_test.test | RUN | 0.343268 | 1.441425 | 3.118373 | 0.001705 | 0.002268 | 0.002072 |
| has_right_shift_assign_test.test | RUN | 0.321610 | 1.367393 | 3.012617 | 0.001851 | 0.002200 | 0.001928 |
| has_prefix_classes3_test.test | RUN | 0.229558 | 0.795763 | 1.570118 | 0.001880 | 0.001984 | 0.001993 |
| has_prefix_classes2_test.test | RUN | 0.236575 | 0.748552 | 1.554128 | 0.001786 | 0.002200 | 0.001730 |
| has_prefix_classes1_test.test | RUN | 0.236857 | 0.800357 | 1.512524 | 0.001762 | 0.002016 | 0.001884 |
| has_prefix_classes0_test.test | RUN | 0.236671 | 0.782507 | 1.531978 | 0.001837 | 0.002261 | 0.001899 |
| has_pre_increment_test.test | RUN | 0.321640 | 1.885387 | 2.997919 | 0.001823 | 0.002084 | 0.002083 |
| has_pre_decrement_test.test | RUN | 0.322039 | 1.871948 | 2.933035 | 0.001696 | 0.002262 | 0.001988 |
| has_postfix_classes3_test.test | RUN | 0.236586 | 0.804364 | 1.591305 | 0.001852 | 0.002041 | 0.001948 |
| has_postfix_classes2_test.test | RUN | 0.232993 | 0.790645 | 1.534786 | 0.001526 | 0.002301 | 0.001758 |
| has_postfix_classes1_test.test | RUN | 0.232051 | 0.804567 | 1.563386 | 0.001892 | 0.002403 | 0.001982 |
| has_postfix_classes0_test.test | RUN | 0.234431 | 0.824038 | 1.539920 | 0.001772 | 0.002190 | 0.001940 |
| has_post_increment_test.test | RUN | 0.322643 | 1.969224 | 3.007954 | 0.001907 | 0.001926 | 0.002054 |
| has_post_decrement_test.test | RUN | 0.316561 | 1.952273 | 3.031042 | 0.001812 | 0.002116 | 0.001937 |
| has_plus_test.test | RUN | 0.325953 | 1.302830 | 2.948361 | 0.001729 | 0.001926 | 0.001951 |
| has_plus_assign_test.test | RUN | 0.337578 | 1.292999 | 2.966010 | 0.001778 | 0.002174 | 0.001766 |
| has_operator_new_test.test | RUN | 0.216812 | 0.730935 | 1.388259 | 0.001755 | 0.001740 | 0.001635 |
| has_nothrow_destructor_test.test | RUN | 0.223012 | 0.765429 | 1.484051 | 0.001697 | 0.001804 | 0.002026 |
| has_nothrow_copy_test.test | RUN | 0.216971 | 0.750046 | 1.416367 | 0.001866 | 0.002050 | 0.001826 |
| has_nothrow_constr_test.test | RUN | 0.208971 | 0.735942 | 1.423590 | 0.001900 | 0.002196 | 0.001790 |
| has_nothrow_assign_test.test | RUN | 0.210266 | 0.738761 | 1.414908 | 0.001865 | 0.002242 | 0.001742 |
| has_not_equal_to_test.test | RUN | 0.330346 | 1.892003 | 3.052046 | 0.001880 | 0.002208 | 0.001945 |
| has_negate_test.test | RUN | 0.316050 | 1.373798 | 2.913023 | 0.001800 | 0.002206 | 0.002012 |
| has_multiplies_test.test | RUN | 0.318238 | 1.344564 | 2.965551 | 0.001817 | 0.001998 | 0.001743 |
| has_multiplies_assign_test.test | RUN | 0.322266 | 1.355932 | 2.988860 | 0.001897 | 0.002125 | 0.001859 |
| has_modulus_test.test | RUN | 0.325918 | 1.371742 | 2.958880 | 0.001836 | 0.002123 | 0.001892 |
| has_modulus_assign_test.test | RUN | 0.325633 | 1.394910 | 2.999720 | 0.001878 | 0.001886 | 0.001857 |
| has_minus_test.test | RUN | 0.330351 | 1.362208 | 3.027252 | 0.001891 | 0.002124 | 0.001976 |
| has_minus_assign_test.test | RUN | 0.333008 | 1.365631 | 3.007063 | 0.001936 | 0.002200 | 0.001884 |
| has_logical_or_test.test | RUN | 0.331992 | 1.361105 | 3.012503 | 0.001898 | 0.002184 | 0.001947 |
| has_logical_not_test.test | RUN | 0.313664 | 1.282800 | 2.881069 | 0.002012 | 0.002013 | 0.001927 |
| has_logical_and_test.test | RUN | 0.318754 | 1.374041 | 2.961258 | 0.001940 | 0.002308 | 0.002028 |
| has_less_test.test | RUN | 0.329540 | 1.928677 | 2.970949 | 0.001747 | 0.002256 | 0.001950 |
| has_less_equal_test.test | RUN | 0.330320 | 1.809896 | 3.039802 | 0.001692 | 0.001990 | 0.001899 |
| has_left_shift_test.test | RUN | 0.363216 | 1.737630 | 3.619095 | 0.001933 | 0.001823 | 0.001980 |
| has_left_shift_assign_test.test | RUN | 0.313859 | 1.292720 | 2.923189 | 0.001770 | 0.001848 | 0.001796 |
| has_greater_test.test | RUN | 0.326972 | 1.904730 | 2.945605 | 0.001974 | 0.002108 | 0.001705 |
| has_greater_equal_test.test | RUN | 0.331575 | 1.879857 | 2.994059 | 0.001742 | 0.002102 | 0.001858 |
| has_equal_to_test.test | RUN | 0.330089 | 1.867741 | 2.977388 | 0.001908 | 0.002171 | 0.002050 |
| has_divides_test.test | RUN | 0.318262 | 1.334108 | 2.981353 | 0.001749 | 0.001920 | 0.001675 |
| has_divides_assign_test.test | RUN | 0.323625 | 1.314247 | 2.911174 | 0.001881 | 0.002108 | 0.002148 |
| has_dereference_test.test | RUN | 0.322051 | 1.311688 | 2.880996 | 0.001704 | 0.002178 | 0.001940 |
| has_complement_test.test | RUN | 0.308295 | 1.866960 | 2.937361 | 0.001845 | 0.002078 | 0.001897 |
| has_bit_xor_test.test | RUN | 0.325903 | 1.343655 | 2.940332 | 0.001711 | 0.002033 | 0.001698 |
| has_bit_xor_assign_test.test | RUN | 0.324123 | 1.291878 | 2.968563 | 0.001752 | 0.002237 | 0.002082 |
| has_bit_or_test.test | RUN | 0.328137 | 1.294876 | 2.975949 | 0.001648 | 0.002257 | 0.001913 |
| has_bit_or_assign_test.test | RUN | 0.319305 | 1.327744 | 2.991106 | 0.001926 | 0.002110 | 0.001799 |
| has_bit_and_test.test | RUN | 0.322053 | 1.338575 | 2.982983 | 0.001781 | 0.001999 | 0.001842 |
| has_bit_and_assign_test.test | RUN | 0.325702 | 1.307484 | 2.969046 | 0.001894 | 0.002200 | 0.002038 |
| has_binary_classes9_test.test | RUN | 0.361343 | 1.533775 | 3.233306 | 0.001812 | 0.002315 | 0.002070 |
| has_binary_classes8_test.test | RUN | 0.357739 | 1.502434 | 3.303513 | 0.001767 | 0.001923 | 0.001811 |
| has_binary_classes7_test.test | RUN | 0.348363 | 1.473946 | 3.222973 | 0.001858 | 0.002045 | 0.001913 |
| has_binary_classes6_test.test | RUN | 0.347125 | 1.456691 | 3.292151 | 0.002028 | 0.002144 | 0.002123 |
| has_binary_classes5_test.test | RUN | 0.356772 | 1.461302 | 3.318638 | 0.001735 | 0.002162 | 0.002045 |
| has_binary_classes4_test.test | RUN | 0.355327 | 1.534754 | 3.266142 | 0.001922 | 0.002154 | 0.001888 |
| has_binary_classes3_test.test | RUN | 0.356991 | 1.527247 | 3.321244 | 0.001763 | 0.001994 | 0.001758 |
| has_binary_classes2_test.test | RUN | 0.355483 | 1.529102 | 3.294039 | 0.001894 | 0.002096 | 0.002107 |
| has_binary_classes1_test.test | RUN | 0.358443 | 1.541050 | 3.256810 | 0.001506 | 0.002098 | 0.001994 |
| has_binary_classes0_test.test | RUN | 0.359893 | 1.549499 | 3.192151 | 0.001788 | 0.001878 | 0.001822 |
| function_traits_test.test | RUN | 0.152249 | 0.383436 | 0.721738 | 0.001660 | 0.002088 | 0.002090 |
| floating_point_promotion_test.test | RUN | 0.149589 | 0.380085 | 0.690568 | 0.001882 | 0.002106 | 0.002088 |
| extent_test.test | RUN | 0.156392 | 0.401010 | 0.729713 | 0.001696 | 0.001696 | 0.002065 |
| enable_if_test.test | RUN | 0.140698 | 0.327037 | 0.533243 | 0.001702 | 0.001868 | 0.001820 |
| disjunction_test.test | RUN | 0.144631 | 0.350628 | 0.581188 | 0.001768 | 0.002224 | 0.001952 |
| detected_test.test | RUN | 0.145710 | 0.330243 | 0.541952 | 0.001762 | 0.001879 | 0.001954 |
| detected_or_test.test | RUN | 0.144300 | 0.350759 | 0.538309 | 0.001702 | 0.002138 | 0.001575 |
| decay_test.test | RUN | 0.395945 | 1.496626 | 3.609279 | 0.001546 | 0.001819 | 0.001800 |
| cxx14_aliases_test.test | RUN | 0.239728 | 0.541266 | 0.976486 | 0.001576 | 0.002095 | 0.001663 |
| copy_reference_test.test | RUN | 0.146440 | 0.352299 | 0.563844 | 0.001714 | 0.001618 | 0.001772 |
| copy_cv_test.test | RUN | 0.151895 | 0.369213 | 0.638186 | 0.001729 | 0.002170 | 0.001674 |
| copy_cv_ref_test.test | RUN | 0.212769 | 0.645198 | 1.445256 | 0.001727 | 0.001934 | 0.002052 |
| conjunction_test.test | RUN | 0.145340 | 0.362673 | 0.586470 | 0.001951 | 0.001936 | 0.001696 |
| conditional_test.test | RUN | 0.145916 | 0.348673 | 0.564796 | 0.001718 | 0.001891 | 0.001675 |
| common_type_test.test | RUN | 0.195860 | 0.552018 | 1.112873 | 0.002043 | 0.001748 | 0.001767 |
| common_type_sfinae_test.test | RUN | 0.139034 | 0.326959 | 0.524962 | 0.001772 | 0.001943 | 0.001860 |
| common_type_sfinae2_test.test | RUN | 0.147034 | 0.358230 | 0.584439 | 0.001790 | 0.001864 | 0.001776 |
| common_type_6_test.test | RUN | 0.151805 | 0.367280 | 0.617183 | 0.001723 | 0.001796 | 0.001709 |
| common_type_5_test.test | RUN | 0.143468 | 0.360083 | 0.588517 | 0.001872 | 0.001841 | 0.002078 |
| common_type_4_test.test | RUN | 0.157117 | 0.401932 | 0.727035 | 0.001848 | 0.001908 | 0.002125 |
| common_type_3_test.test | RUN | 0.138323 | 0.338515 | 0.511261 | 0.001801 | 0.001814 | 0.001891 |
| common_type_2_test.test | RUN | 0.196372 | 0.552171 | 1.129889 | 0.002028 | 0.002100 | 0.001951 |
| alignment_of_test.test | RUN | 0.153010 | 0.423953 | 0.712113 | 0.001878 | 0.002032 | 0.001876 |
| alignment_of_a2_test.test | RUN | 0.160007 | 0.411707 | 0.696462 | 0.001821 | 0.001699 | 0.002076 |
| aligned_storage_test.test | RUN | 0.159512 | 0.417717 | 0.622349 | 0.001818 | 0.001906 | 0.002346 |
| aligned_storage_empy_test.test | RUN | 0.177834 | 0.466357 | 0.873707 | 0.001826 | 0.001932 | 0.002074 |
| aligned_storage_a2_test.test | RUN | 0.158569 | 0.400712 | 0.605662 | 0.002009 | 0.002301 | 0.002309 |
| add_volatile_test.test | RUN | 0.361984 | 1.221452 | 3.382240 | 0.001982 | 0.001697 | 0.002340 |
| add_rvalue_reference_test.test | RUN | 0.399768 | 1.383090 | 3.809857 | 0.001777 | 0.001880 | 0.002076 |
| add_reference_test.test | RUN | 0.389049 | 1.409218 | 3.793102 | 0.001676 | 0.002097 | 0.002062 |
| add_pointer_test.test | RUN | 0.252513 | 0.775993 | 1.943370 | 0.001822 | 0.002015 | 0.001922 |
| add_lvalue_reference_test.test | RUN | 0.383146 | 1.377470 | 3.831913 | 0.001888 | 0.002215 | 0.002184 |
| add_cv_test.test | RUN | 0.365389 | 1.306360 | 3.622815 | 0.001702 | 0.001734 | 0.002080 |
| add_const_test.test | RUN | 0.371494 | 1.391343 | 3.610378 | 0.001716 | 0.001769 | 0.001942 |
| is_virtual_base_of_fail.test | COMPILE_FAIL | 0.139227 | 0.327707 | 0.533707 | — | — | — |
| is_stateless_fail.test | COMPILE_FAIL | 0.140255 | 0.334696 | 0.550874 | — | — | — |
| is_polymorphic_fail.test | COMPILE_FAIL | 0.135588 | 0.312664 | 0.506089 | — | — | — |
| is_nothrow_move_constructible_fail.test | COMPILE_FAIL | 0.137934 | 0.314724 | 0.540603 | — | — | — |
| is_nothrow_move_assignable_fail.test | COMPILE_FAIL | 0.137277 | 0.313579 | 0.528146 | — | — | — |
| is_list_constructible_fail.test | COMPILE_FAIL | 0.138209 | 0.319870 | 0.454075 | — | — | — |
| is_empty_fail.test | COMPILE_FAIL | 0.138803 | 0.323407 | 0.482311 | — | — | — |
| is_destructible_fail.test | COMPILE_FAIL | 0.137192 | 0.315574 | 0.525632 | — | — | — |
| is_default_constructible_fail.test | COMPILE_FAIL | 0.133492 | 0.312998 | 0.510712 | — | — | — |
| is_copy_constructible_fail.test | COMPILE_FAIL | 0.138858 | 0.325051 | 0.528841 | — | — | — |
| is_copy_assignable_fail.test | COMPILE_FAIL | 0.136011 | 0.320975 | 0.492995 | — | — | — |
| is_convertible_fail.test | COMPILE_FAIL | 0.137469 | 0.315387 | 0.524189 | — | — | — |
| is_convertible2_fail.test | COMPILE_FAIL | 0.136651 | 0.311739 | 0.518939 | — | — | — |
| is_constructible_fail.test | COMPILE_FAIL | 0.138965 | 0.330106 | 0.517305 | — | — | — |
| is_base_of_fail.test | COMPILE_FAIL | 0.136104 | 0.325795 | 0.515012 | — | — | — |
| is_assignable_fail.test | COMPILE_FAIL | 0.137954 | 0.314995 | 0.517936 | — | — | — |
| has_nothrow_destruct_fail.test | COMPILE_FAIL | 0.133391 | 0.310219 | 0.515502 | — | — | — |
| has_nothrow_copy_fail.test | COMPILE_FAIL | 0.138686 | 0.326709 | 0.509112 | — | — | — |
| has_nothrow_cons_fail.test | COMPILE_FAIL | 0.136626 | 0.321668 | 0.529120 | — | — | — |
| has_nothrow_assign_fail.test | COMPILE_FAIL | 0.138712 | 0.314241 | 0.503574 | — | — | — |
| common_type_fail.test | COMPILE_FAIL | 0.137730 | 0.316009 | 0.509082 | — | — | — |
| common_type2_fail.test | COMPILE_FAIL | 0.138812 | 0.322355 | 0.507928 | — | — | — |
| has_nothrow_assign_test_no_intrinsics.test | RUN | 0.218917 | 0.720881 | 1.449119 | 0.001691 | 0.002080 | 0.002035 |
| has_nothrow_constr_test_no_intrinsics.test | RUN | 0.211782 | 0.707168 | 1.404506 | 0.001721 | 0.001794 | 0.001840 |
| has_nothrow_copy_test_no_intrinsics.test | RUN | 0.216932 | 0.723242 | 1.442133 | 0.001759 | 0.001612 | 0.001361 |
| is_nothrow_move_assignable_test_no_intrinsics.test | RUN | 0.223297 | 0.710087 | 1.374603 | 0.001194 | 0.001642 | 0.001468 |
| is_nothrow_move_constructible_test_no_intrinsics.test | RUN | 0.122839 | 0.494298 | 0.819032 | 0.001536 | 0.001733 | 0.001508 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.998× | 0.996×, 1.001× | 1.005× |
| clang | 1.027× | 1.033×, 1.020× | 0.996× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274616).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| add_const_test.test | 10 | 0.001456s | 0.001465s | 0.001454s | byte equality observed |
| add_cv_test.test | 10 | 0.001437s | 0.001474s | 0.001471s | byte equality observed |
| add_lvalue_reference_test.test | 10 | 0.001501s | 0.001486s | 0.001477s | byte equality observed |
| add_pointer_test.test | 10 | 0.001492s | 0.001462s | 0.001489s | byte equality observed |
| add_reference_test.test | 10 | 0.001457s | 0.001481s | 0.001495s | byte equality observed |
| add_rvalue_reference_test.test | 10 | 0.001477s | 0.001469s | 0.001462s | byte equality observed |
| add_volatile_test.test | 8 | 0.001509s | 0.001480s | 0.001472s | byte equality observed |
| aligned_storage_a2_test.test | 10 | 0.001615s | 0.001586s | 0.001597s | byte equality observed |
| aligned_storage_empy_test.test | 10 | 0.001455s | 0.001495s | 0.001485s | byte equality observed |
| aligned_storage_test.test | 9 | 0.001625s | 0.001616s | 0.001564s | byte equality observed |
| alignment_of_a2_test.test | 9 | 0.001464s | 0.001455s | 0.001456s | byte equality observed |
| alignment_of_test.test | 10 | 0.001479s | 0.001458s | 0.001445s | byte equality observed |
| common_type_2_test.test | 10 | 0.001460s | 0.001464s | 0.001467s | byte equality observed |
| common_type_3_test.test | 10 | 0.001461s | 0.001456s | 0.001449s | byte equality observed |
| common_type_4_test.test | 10 | 0.001472s | 0.001492s | 0.001449s | byte equality observed |
| common_type_5_test.test | 10 | 0.001471s | 0.001468s | 0.001449s | byte equality observed |
| common_type_6_test.test | 9 | 0.001474s | 0.001452s | 0.001460s | byte equality observed |
| common_type_sfinae2_test.test | 10 | 0.001469s | 0.001439s | 0.001470s | byte equality observed |
| common_type_sfinae_test.test | 10 | 0.001454s | 0.001471s | 0.001449s | byte equality observed |
| common_type_test.test | 10 | 0.001484s | 0.001473s | 0.001453s | byte equality observed |
| conditional_test.test | 10 | 0.001468s | 0.001449s | 0.001489s | byte equality observed |
| conjunction_test.test | 10 | 0.001454s | 0.001464s | 0.001474s | byte equality observed |
| copy_cv_ref_test.test | 10 | 0.001468s | 0.001456s | 0.001464s | byte equality observed |
| copy_cv_test.test | 10 | 0.001476s | 0.001461s | 0.001463s | byte equality observed |
| copy_reference_test.test | 10 | 0.001426s | 0.001406s | 0.001483s | byte equality observed |
| cxx14_aliases_test.test | 10 | 0.001465s | 0.001479s | 0.001442s | byte equality observed |
| decay_test.test | 10 | 0.001465s | 0.001453s | 0.001468s | byte equality observed |
| detected_or_test.test | 10 | 0.001468s | 0.001454s | 0.001466s | byte equality observed |
| detected_test.test | 10 | 0.001476s | 0.001459s | 0.001470s | byte equality observed |
| disjunction_test.test | 10 | 0.001471s | 0.001455s | 0.001471s | byte equality observed |
| enable_if_test.test | 10 | 0.001425s | 0.001485s | 0.001469s | byte equality observed |
| extent_test.test | 10 | 0.001452s | 0.001458s | 0.001454s | byte equality observed |
| floating_point_promotion_test.test | 10 | 0.001470s | 0.001486s | 0.001470s | byte equality observed |
| function_traits_test.test | 10 | 0.001452s | 0.001448s | 0.001495s | byte equality observed |
| has_binary_classes0_test.test | 10 | 0.001487s | 0.001471s | 0.001460s | byte equality observed |
| has_binary_classes1_test.test | 10 | 0.001461s | 0.001478s | 0.001430s | byte equality observed |
| has_binary_classes2_test.test | 10 | 0.001462s | 0.001432s | 0.001458s | byte equality observed |
| has_binary_classes3_test.test | 10 | 0.001463s | 0.001446s | 0.001477s | byte equality observed |
| has_binary_classes4_test.test | 10 | 0.001461s | 0.001474s | 0.001486s | byte equality observed |
| has_binary_classes5_test.test | 10 | 0.001455s | 0.001442s | 0.001470s | byte equality observed |
| has_binary_classes6_test.test | 10 | 0.001454s | 0.001466s | 0.001459s | byte equality observed |
| has_binary_classes7_test.test | 10 | 0.001461s | 0.001466s | 0.001460s | byte equality observed |
| has_binary_classes8_test.test | 10 | 0.001453s | 0.001480s | 0.001441s | byte equality observed |
| has_binary_classes9_test.test | 10 | 0.001479s | 0.001472s | 0.001467s | byte equality observed |
| has_bit_and_assign_test.test | 9 | 0.001469s | 0.001463s | 0.001489s | byte equality observed |
| has_bit_and_test.test | 10 | 0.001463s | 0.001454s | 0.001457s | byte equality observed |
| has_bit_or_assign_test.test | 10 | 0.001473s | 0.001476s | 0.001476s | byte equality observed |
| has_bit_or_test.test | 10 | 0.001472s | 0.001451s | 0.001480s | byte equality observed |
| has_bit_xor_assign_test.test | 8 | 0.001461s | 0.001465s | 0.001430s | byte equality observed |
| has_bit_xor_test.test | 10 | 0.001448s | 0.001471s | 0.001468s | byte equality observed |
| has_complement_test.test | 10 | 0.001478s | 0.001477s | 0.001469s | byte equality observed |
| has_dereference_test.test | 10 | 0.001474s | 0.001445s | 0.001457s | byte equality observed |
| has_divides_assign_test.test | 8 | 0.001442s | 0.001436s | 0.001468s | byte equality observed |
| has_divides_test.test | 10 | 0.001445s | 0.001461s | 0.001484s | byte equality observed |
| has_equal_to_test.test | 10 | 0.001451s | 0.001474s | 0.001445s | byte equality observed |
| has_greater_equal_test.test | 10 | 0.001467s | 0.001460s | 0.001482s | byte equality observed |
| has_greater_test.test | 10 | 0.001465s | 0.001435s | 0.001490s | byte equality observed |
| has_left_shift_assign_test.test | 10 | 0.001502s | 0.001465s | 0.001468s | byte equality observed |
| has_left_shift_test.test | 9 | 0.001467s | 0.001450s | 0.001439s | byte equality observed |
| has_less_equal_test.test | 10 | 0.001472s | 0.001469s | 0.001454s | byte equality observed |
| has_less_test.test | 10 | 0.001471s | 0.001460s | 0.001431s | byte equality observed |
| has_logical_and_test.test | 10 | 0.001467s | 0.001432s | 0.001435s | byte equality observed |
| has_logical_not_test.test | 10 | 0.001432s | 0.001473s | 0.001480s | byte equality observed |
| has_logical_or_test.test | 10 | 0.001481s | 0.001465s | 0.001464s | byte equality observed |
| has_minus_assign_test.test | 9 | 0.001475s | 0.001453s | 0.001461s | byte equality observed |
| has_minus_test.test | 10 | 0.001472s | 0.001450s | 0.001465s | byte equality observed |
| has_modulus_assign_test.test | 10 | 0.001455s | 0.001471s | 0.001501s | byte equality observed |
| has_modulus_test.test | 10 | 0.001467s | 0.001462s | 0.001438s | byte equality observed |
| has_multiplies_assign_test.test | 10 | 0.001478s | 0.001435s | 0.001465s | byte equality observed |
| has_multiplies_test.test | 10 | 0.001463s | 0.001467s | 0.001484s | byte equality observed |
| has_negate_test.test | 10 | 0.001447s | 0.001469s | 0.001478s | byte equality observed |
| has_not_equal_to_test.test | 10 | 0.001448s | 0.001437s | 0.001460s | byte equality observed |
| has_nothrow_assign_test.test | 9 | 0.001472s | 0.001428s | 0.001490s | byte equality observed |
| has_nothrow_assign_test_no_intrinsics.test | 10 | 0.001448s | 0.001483s | 0.001431s | byte equality observed |
| has_nothrow_constr_test.test | 10 | 0.001459s | 0.001455s | 0.001488s | byte equality observed |
| has_nothrow_constr_test_no_intrinsics.test | 10 | 0.001491s | 0.001471s | 0.001446s | byte equality observed |
| has_nothrow_copy_test.test | 10 | 0.001431s | 0.001500s | 0.001475s | byte equality observed |
| has_nothrow_copy_test_no_intrinsics.test | 10 | 0.001473s | 0.001431s | 0.001489s | byte equality observed |
| has_nothrow_destructor_test.test | 8 | 0.001437s | 0.001429s | 0.001454s | byte equality observed |
| has_operator_new_test.test | 10 | 0.001439s | 0.001472s | 0.001446s | byte equality observed |
| has_plus_assign_test.test | 10 | 0.001466s | 0.001457s | 0.001434s | byte equality observed |
| has_plus_test.test | 10 | 0.001479s | 0.001457s | 0.001444s | byte equality observed |
| has_post_decrement_test.test | 10 | 0.001462s | 0.001452s | 0.001448s | byte equality observed |
| has_post_increment_test.test | 10 | 0.001442s | 0.001481s | 0.001477s | byte equality observed |
| has_postfix_classes0_test.test | 10 | 0.001486s | 0.001464s | 0.001440s | byte equality observed |
| has_postfix_classes1_test.test | 9 | 0.001465s | 0.001476s | 0.001447s | byte equality observed |
| has_postfix_classes2_test.test | 10 | 0.001455s | 0.001483s | 0.001494s | byte equality observed |
| has_postfix_classes3_test.test | 10 | 0.001448s | 0.001467s | 0.001469s | byte equality observed |
| has_pre_decrement_test.test | 10 | 0.001475s | 0.001462s | 0.001453s | byte equality observed |
| has_pre_increment_test.test | 10 | 0.001442s | 0.001425s | 0.001498s | byte equality observed |
| has_prefix_classes0_test.test | 10 | 0.001451s | 0.001445s | 0.001472s | byte equality observed |
| has_prefix_classes1_test.test | 10 | 0.001445s | 0.001465s | 0.001453s | byte equality observed |
| has_prefix_classes2_test.test | 10 | 0.001452s | 0.001456s | 0.001467s | byte equality observed |
| has_prefix_classes3_test.test | 10 | 0.001462s | 0.001476s | 0.001464s | byte equality observed |
| has_right_shift_assign_test.test | 10 | 0.001450s | 0.001485s | 0.001475s | byte equality observed |
| has_right_shift_test.test | 10 | 0.001462s | 0.001469s | 0.001452s | byte equality observed |
| has_trivial_assign_test.test | 10 | 0.001531s | 0.001573s | 0.001526s | byte equality observed |
| has_trivial_constr_test.test | 10 | 0.001537s | 0.001518s | 0.001559s | native test oracle; output varies |
| has_trivial_copy_test.test | 10 | 0.001570s | 0.001552s | 0.001575s | native test oracle; output varies |
| has_trivial_destructor_test.test | 10 | 0.001486s | 0.001450s | 0.001458s | byte equality observed |
| has_trivial_move_assign_test.test | 10 | 0.001453s | 0.001446s | 0.001451s | byte equality observed |
| has_trivial_move_constructor_test.test | 10 | 0.001459s | 0.001487s | 0.001492s | byte equality observed |
| has_unary_minus_test.test | 10 | 0.001462s | 0.001478s | 0.001447s | byte equality observed |
| has_unary_plus_test.test | 10 | 0.001447s | 0.001492s | 0.001459s | byte equality observed |
| has_virtual_destructor_test.test | 10 | 0.001470s | 0.001475s | 0.001440s | byte equality observed |
| integral_constant_test.test | 10 | 0.000649s | 0.000651s | 0.001432s | byte equality observed |
| is_abstract_test.test | 10 | 0.001458s | 0.001486s | 0.001477s | byte equality observed |
| is_arithmetic_test.test | 10 | 0.001470s | 0.001473s | 0.001472s | byte equality observed |
| is_array_test.test | 10 | 0.001481s | 0.001436s | 0.001449s | byte equality observed |
| is_assignable_test.test | 10 | 0.001463s | 0.001469s | 0.001435s | byte equality observed |
| is_base_and_derived_test.test | 10 | 0.001472s | 0.001454s | 0.001463s | byte equality observed |
| is_base_of_test.test | 10 | 0.001458s | 0.001450s | 0.001458s | byte equality observed |
| is_bounded_array_test.test | 10 | 0.001451s | 0.001442s | 0.001457s | byte equality observed |
| is_class_test.test | 10 | 0.001461s | 0.001458s | 0.001454s | byte equality observed |
| is_complete_test.test | 10 | 0.001457s | 0.001465s | 0.001438s | byte equality observed |
| is_complex_test.test | 10 | 0.001444s | 0.001474s | 0.001463s | byte equality observed |
| is_compound_test.test | 10 | 0.001453s | 0.001481s | 0.001455s | byte equality observed |
| is_const_test.test | 10 | 0.001453s | 0.001465s | 0.001462s | byte equality observed |
| is_constructible_test.test | 10 | 0.001445s | 0.001459s | 0.001461s | byte equality observed |
| is_convertible_test.test | 10 | 0.001467s | 0.001495s | 0.001458s | byte equality observed |
| is_copy_assignable_test.test | 10 | 0.001468s | 0.001479s | 0.001472s | byte equality observed |
| is_copy_constructible_test.test | 10 | 0.001484s | 0.001477s | 0.001444s | byte equality observed |
| is_default_constr_test.test | 10 | 0.001448s | 0.001478s | 0.001480s | byte equality observed |
| is_destructible_test.test | 10 | 0.001455s | 0.001466s | 0.001444s | byte equality observed |
| is_detected_exact_test.test | 10 | 0.001450s | 0.001472s | 0.001454s | byte equality observed |
| is_detected_test.test | 10 | 0.001476s | 0.001479s | 0.001450s | byte equality observed |
| is_empty_test.test | 10 | 0.001481s | 0.001447s | 0.001472s | byte equality observed |
| is_enum_test.test | 10 | 0.001450s | 0.001453s | 0.001429s | byte equality observed |
| is_final_test.test | 10 | 0.001452s | 0.001433s | 0.001426s | byte equality observed |
| is_float_test.test | 10 | 0.001453s | 0.001437s | 0.001459s | byte equality observed |
| is_floating_point_test.test | 10 | 0.001465s | 0.001463s | 0.001446s | byte equality observed |
| is_function_test.test | 10 | 0.001442s | 0.001419s | 0.001457s | byte equality observed |
| is_fundamental_test.test | 10 | 0.001458s | 0.001434s | 0.001453s | byte equality observed |
| is_integral_test.test | 10 | 0.001469s | 0.001452s | 0.001454s | byte equality observed |
| is_list_constructible_test.test | 10 | 0.001454s | 0.001428s | 0.001491s | byte equality observed |
| is_lvalue_reference_test.test | 10 | 0.001491s | 0.001482s | 0.001481s | byte equality observed |
| is_member_func_test.test | 10 | 0.001462s | 0.001512s | 0.001423s | byte equality observed |
| is_member_obj_test.test | 10 | 0.001468s | 0.001481s | 0.001467s | byte equality observed |
| is_member_pointer_test.test | 10 | 0.001463s | 0.001461s | 0.001487s | byte equality observed |
| is_noncopyable_test.test | 10 | 0.001456s | 0.001455s | 0.001498s | byte equality observed |
| is_nothrow_move_assignable_test.test | 10 | 0.001444s | 0.001451s | 0.001461s | byte equality observed |
| is_nothrow_move_assignable_test_no_intrinsics.test | 10 | 0.001466s | 0.001453s | 0.001490s | byte equality observed |
| is_nothrow_move_constructible_test.test | 10 | 0.001458s | 0.001470s | 0.001489s | byte equality observed |
| is_nothrow_move_constructible_test_no_intrinsics.test | 10 | 0.001475s | 0.001448s | 0.001464s | byte equality observed |
| is_nothrow_swappable_test.test | 10 | 0.001480s | 0.001460s | 0.001460s | byte equality observed |
| is_object_test.test | 10 | 0.001456s | 0.001460s | 0.001475s | byte equality observed |
| is_pod_test.test | 10 | 0.001456s | 0.001467s | 0.001440s | byte equality observed |
| is_pointer_test.test | 10 | 0.001484s | 0.001465s | 0.001485s | byte equality observed |
| is_polymorphic_test.test | 10 | 0.001467s | 0.001466s | 0.001439s | byte equality observed |
| is_reference_test.test | 10 | 0.001445s | 0.001480s | 0.001471s | byte equality observed |
| is_rvalue_reference_test.test | 10 | 0.001484s | 0.001416s | 0.001448s | byte equality observed |
| is_same_test.test | 10 | 0.001460s | 0.001475s | 0.001446s | byte equality observed |
| is_scalar_test.test | 7 | 0.001484s | 0.001438s | 0.001458s | byte equality observed |
| is_scoped_enum_test.test | 9 | 0.001463s | 0.001471s | 0.001486s | byte equality observed |
| is_signed_test.test | 10 | 0.001466s | 0.001465s | 0.001463s | byte equality observed |
| is_stateless_test.test | 10 | 0.001454s | 0.001437s | 0.001468s | byte equality observed |
| is_swappable_test.test | 10 | 0.001463s | 0.001454s | 0.001472s | byte equality observed |
| is_trivially_copyable_test.test | 10 | 0.001507s | 0.001457s | 0.001500s | byte equality observed |
| is_unbounded_array_test.test | 10 | 0.001467s | 0.001475s | 0.001464s | byte equality observed |
| is_union_test.test | 10 | 0.001448s | 0.001487s | 0.001478s | byte equality observed |
| is_unscoped_enum_test.test | 9 | 0.001452s | 0.001469s | 0.001462s | byte equality observed |
| is_unsigned_test.test | 10 | 0.001446s | 0.001460s | 0.001445s | byte equality observed |
| is_virtual_base_of_test.test | 10 | 0.001467s | 0.001454s | 0.001459s | byte equality observed |
| is_void_test.test | 10 | 0.001475s | 0.001463s | 0.001468s | byte equality observed |
| is_volatile_test.test | 10 | 0.001443s | 0.001465s | 0.001449s | byte equality observed |
| make_signed_test.test | 10 | 0.001457s | 0.001482s | 0.001474s | byte equality observed |
| make_unsigned_test.test | 9 | 0.001466s | 0.001451s | 0.001492s | byte equality observed |
| make_void_test.test | 10 | 0.001494s | 0.001465s | 0.001473s | byte equality observed |
| mpl_interop_test1.test | 10 | 0.000641s | 0.000656s | 0.001475s | byte equality observed |
| mpl_interop_test2.test | 10 | 0.000646s | 0.000640s | 0.001476s | byte equality observed |
| mpl_interop_test3.test | 10 | 0.000660s | 0.000661s | 0.001458s | byte equality observed |
| negation_test.test | 10 | 0.001532s | 0.001510s | 0.001436s | byte equality observed |
| promote_basic_test.test | 10 | 0.000659s | 0.000661s | 0.001483s | byte equality observed |
| promote_enum_msvc_bug_test.test | 10 | 0.000660s | 0.000652s | 0.001496s | byte equality observed |
| promote_enum_test.test | 10 | 0.000655s | 0.000670s | 0.001452s | byte equality observed |
| promote_mpl_test.test | 10 | 0.000657s | 0.000652s | 0.001477s | byte equality observed |
| rank_test.test | 10 | 0.001503s | 0.001492s | 0.001468s | byte equality observed |
| remove_all_extents_test.test | 10 | 0.001499s | 0.001482s | 0.001464s | byte equality observed |
| remove_bounds_test.test | 10 | 0.001483s | 0.001444s | 0.001448s | byte equality observed |
| remove_const_test.test | 8 | 0.001435s | 0.001425s | 0.001466s | byte equality observed |
| remove_cv_ref_test.test | 10 | 0.001457s | 0.001449s | 0.001450s | byte equality observed |
| remove_cv_test.test | 10 | 0.001453s | 0.001454s | 0.001455s | byte equality observed |
| remove_extent_test.test | 10 | 0.001476s | 0.001422s | 0.001487s | byte equality observed |
| remove_pointer_test.test | 10 | 0.001456s | 0.001451s | 0.001468s | byte equality observed |
| remove_reference_test.test | 10 | 0.001466s | 0.001499s | 0.001487s | byte equality observed |
| remove_volatile_test.test | 10 | 0.001460s | 0.001426s | 0.001472s | byte equality observed |
| tricky_abstract_type_test.test | 9 | 0.001453s | 0.001488s | 0.001477s | byte equality observed |
| tricky_add_pointer_test.test | 10 | 0.001435s | 0.001448s | 0.001468s | byte equality observed |
| tricky_function_type_test.test | 10 | 0.001448s | 0.001480s | 0.001439s | byte equality observed |
| tricky_incomplete_type_test.test | 10 | 0.001462s | 0.001448s | 0.001460s | byte equality observed |
| tricky_is_enum_test.test | 10 | 0.001481s | 0.001450s | 0.001468s | byte equality observed |
| tricky_partial_spec_test.test | 10 | 0.001461s | 0.001435s | 0.001499s | byte equality observed |
| tricky_rvalue_test.test | 10 | 0.001481s | 0.001456s | 0.001464s | byte equality observed |
| type_identity_test.test | 10 | 0.001492s | 0.001486s | 0.001456s | byte equality observed |
| type_traits_test.test | 10 | 0.000640s | 0.000631s | 0.001467s | byte equality observed |
| type_with_alignment_test.test | 10 | 0.001635s | 0.001631s | 0.001634s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274616).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
