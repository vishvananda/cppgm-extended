# libs/type_traits/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| type_with_alignment_test.test | RUN | 0.180156 | 0.420282 | 0.793513 | 0.002008 | 0.002180 | 0.002140 |
| type_traits_test.test | RUN | 0.257820 | 0.469473 | 0.809263 | 0.000719 | 0.000600 | 0.001865 |
| type_identity_test.test | RUN | 0.464439 | 1.767209 | 4.542917 | 0.002090 | 0.001681 | 0.002050 |
| tricky_rvalue_test.test | RUN | 0.167166 | 0.334637 | 0.603017 | 0.001670 | 0.002091 | 0.001923 |
| tricky_partial_spec_test.test | RUN | 0.207185 | 0.528734 | 1.014764 | 0.001980 | 0.001831 | 0.001976 |
| tricky_is_enum_test.test | RUN | 0.160039 | 0.351134 | 0.559943 | 0.001922 | 0.001963 | 0.002089 |
| tricky_incomplete_type_test.test | RUN | 0.270182 | 0.543495 | 0.942179 | 0.002383 | 0.002060 | 0.001938 |
| tricky_function_type_test.test | RUN | 0.225022 | 0.574212 | 1.091475 | 0.002256 | 0.001728 | 0.002152 |
| tricky_add_pointer_test.test | RUN | 0.317701 | 1.023442 | 2.531435 | 0.002032 | 0.002111 | 0.001967 |
| tricky_abstract_type_test.test | RUN | 0.171021 | 0.359501 | 0.586251 | 0.002092 | 0.001928 | 0.002074 |
| remove_volatile_test.test | RUN | 0.429008 | 1.434145 | 3.925941 | 0.002191 | 0.002188 | 0.002162 |
| remove_reference_test.test | RUN | 0.440354 | 1.503518 | 4.146559 | 0.002066 | 0.001844 | 0.002202 |
| remove_pointer_test.test | RUN | 0.453108 | 1.635286 | 4.428879 | 0.002394 | 0.002180 | 0.002030 |
| remove_extent_test.test | RUN | 0.439879 | 1.569026 | 4.173419 | 0.002050 | 0.002173 | 0.001945 |
| remove_cv_test.test | RUN | 0.442358 | 1.527417 | 4.160251 | 0.002324 | 0.002133 | 0.001992 |
| remove_cv_ref_test.test | RUN | 0.909749 | 3.660253 | 10.191220 | 0.002082 | 0.001876 | 0.002098 |
| remove_const_test.test | RUN | 0.413196 | 1.466837 | 3.869649 | 0.001999 | 0.002136 | 0.001967 |
| remove_bounds_test.test | RUN | 0.434142 | 1.568041 | 4.161657 | 0.002315 | 0.002034 | 0.002137 |
| remove_all_extents_test.test | RUN | 0.450557 | 1.522154 | 4.028129 | 0.001861 | 0.001978 | 0.001771 |
| rank_test.test | RUN | 0.169028 | 0.353966 | 0.616742 | 0.001791 | 0.001834 | 0.002062 |
| promote_mpl_test.test | RUN | 0.156543 | 0.317073 | 0.417842 | 0.000742 | 0.000800 | 0.002002 |
| promote_enum_test.test | RUN | 0.032142 | 0.075154 | 0.102092 | 0.000750 | 0.001008 | 0.002034 |
| promote_enum_msvc_bug_test.test | RUN | 0.030567 | 0.067322 | 0.082447 | 0.000813 | 0.000831 | 0.001947 |
| promote_basic_test.test | RUN | 0.038752 | 0.091703 | 0.107895 | 0.000762 | 0.000909 | 0.002045 |
| negation_test.test | RUN | 0.168797 | 0.322738 | 0.560529 | 0.001583 | 0.001841 | 0.001755 |
| mpl_interop_test3.test | RUN | 0.152025 | 0.296508 | 0.384434 | 0.000712 | 0.000817 | 0.002018 |
| mpl_interop_test2.test | RUN | 0.058265 | 0.083869 | 0.090110 | 0.000720 | 0.000751 | 0.002049 |
| mpl_interop_test1.test | RUN | 0.026410 | 0.061792 | 0.073388 | 0.000688 | 0.000882 | 0.001867 |
| make_void_test.test | RUN | 0.168864 | 0.358194 | 0.610407 | 0.002130 | 0.002162 | 0.002116 |
| make_unsigned_test.test | RUN | 0.195876 | 0.489306 | 0.829621 | 0.001702 | 0.002102 | 0.002099 |
| make_signed_test.test | RUN | 0.193422 | 0.490217 | 0.889956 | 0.001822 | 0.001890 | 0.002022 |
| is_volatile_test.test | RUN | 0.177964 | 0.412807 | 0.681309 | 0.001798 | 0.001916 | 0.002112 |
| is_void_test.test | RUN | 0.172594 | 0.378825 | 0.628929 | 0.001880 | 0.002141 | 0.002028 |
| is_virtual_base_of_test.test | RUN | 0.197714 | 0.491769 | 0.914089 | 0.001781 | 0.001965 | 0.002182 |
| is_unsigned_test.test | RUN | 0.182318 | 0.388451 | 0.650561 | 0.001803 | 0.001778 | 0.002160 |
| is_unscoped_enum_test.test | RUN | 0.164332 | 0.362521 | 0.541196 | 0.001916 | 0.002006 | 0.001950 |
| is_union_test.test | RUN | 0.189020 | 0.446716 | 0.825303 | 0.001884 | 0.002136 | 0.002136 |
| is_unbounded_array_test.test | RUN | 0.169939 | 0.378115 | 0.640639 | 0.002204 | 0.002220 | 0.002029 |
| is_trivially_copyable_test.test | RUN | 0.281070 | 0.798792 | 1.725532 | 0.002048 | 0.001898 | 0.002111 |
| is_swappable_test.test | RUN | 0.231674 | 0.598671 | 1.178047 | 0.001950 | 0.002054 | 0.001970 |
| is_stateless_test.test | RUN | 0.265122 | 0.687929 | 1.431378 | 0.001736 | 0.001969 | 0.001921 |
| is_signed_test.test | RUN | 0.181911 | 0.378854 | 0.660173 | 0.001906 | 0.002080 | 0.002106 |
| is_scoped_enum_test.test | RUN | 0.175693 | 0.346051 | 0.511110 | 0.001846 | 0.001931 | 0.001850 |
| is_scalar_test.test | RUN | 0.235572 | 0.656339 | 1.259427 | 0.002151 | 0.002062 | 0.002018 |
| is_same_test.test | RUN | 0.174736 | 0.385664 | 0.605215 | 0.001936 | 0.001779 | 0.002038 |
| is_rvalue_reference_test.test | RUN | 0.183953 | 0.417973 | 0.756190 | 0.001888 | 0.001808 | 0.001890 |
| is_reference_test.test | RUN | 0.183287 | 0.438430 | 0.738008 | 0.001981 | 0.002158 | 0.001778 |
| is_polymorphic_test.test | RUN | 0.193302 | 0.450898 | 0.793851 | 0.002152 | 0.002194 | 0.002174 |
| is_pointer_test.test | RUN | 0.188758 | 0.435690 | 0.793968 | 0.001918 | 0.002054 | 0.001946 |
| is_pod_test.test | RUN | 0.245263 | 0.718762 | 1.473984 | 0.001803 | 0.002108 | 0.002024 |
| is_object_test.test | RUN | 0.176074 | 0.363281 | 0.616030 | 0.001846 | 0.001952 | 0.001925 |
| is_nothrow_swappable_test.test | RUN | 0.249459 | 0.657474 | 1.343317 | 0.002066 | 0.001894 | 0.002043 |
| is_nothrow_move_constructible_test.test | RUN | 0.245886 | 0.686969 | 1.399156 | 0.002041 | 0.001842 | 0.001818 |
| is_nothrow_move_assignable_test.test | RUN | 0.243470 | 0.696153 | 1.425730 | 0.001790 | 0.002048 | 0.001814 |
| is_noncopyable_test.test | RUN | 0.173136 | 0.378785 | 0.576982 | 0.002008 | 0.001795 | 0.001850 |
| is_member_pointer_test.test | RUN | 0.168566 | 0.374451 | 0.615079 | 0.002086 | 0.001833 | 0.002158 |
| is_member_obj_test.test | RUN | 0.174874 | 0.376223 | 0.645713 | 0.001948 | 0.001871 | 0.002025 |
| is_member_func_test.test | RUN | 0.176274 | 0.412380 | 0.692370 | 0.002018 | 0.001812 | 0.001794 |
| is_lvalue_reference_test.test | RUN | 0.178880 | 0.434601 | 0.757846 | 0.001598 | 0.002072 | 0.002136 |
| is_list_constructible_test.test | RUN | 0.208379 | 0.491637 | 0.821673 | 0.002071 | 0.002054 | 0.002153 |
| is_integral_test.test | RUN | 0.222126 | 0.603006 | 1.221266 | 0.001874 | 0.001703 | 0.001641 |
| is_fundamental_test.test | RUN | 0.225568 | 0.623892 | 1.247845 | 0.001762 | 0.001880 | 0.001719 |
| is_function_test.test | RUN | 0.181794 | 0.393252 | 0.649200 | 0.001550 | 0.002078 | 0.002178 |
| is_floating_point_test.test | RUN | 0.185459 | 0.430439 | 0.759829 | 0.002056 | 0.001964 | 0.002090 |
| is_float_test.test | RUN | 0.182109 | 0.433477 | 0.766378 | 0.001853 | 0.001944 | 0.002120 |
| is_final_test.test | RUN | 0.184443 | 0.442884 | 0.783538 | 0.001998 | 0.001868 | 0.001953 |
| is_enum_test.test | RUN | 0.170885 | 0.367888 | 0.621641 | 0.001920 | 0.001670 | 0.002169 |
| is_empty_test.test | RUN | 0.183848 | 0.405325 | 0.643540 | 0.001900 | 0.001884 | 0.002163 |
| is_detected_test.test | RUN | 0.162121 | 0.356485 | 0.493266 | 0.001999 | 0.002041 | 0.001933 |
| is_detected_exact_test.test | RUN | 0.161730 | 0.331289 | 0.550069 | 0.001884 | 0.001817 | 0.002220 |
| is_destructible_test.test | RUN | 0.243086 | 0.677371 | 1.409163 | 0.001913 | 0.002105 | 0.001888 |
| is_default_constr_test.test | RUN | 0.251874 | 0.710910 | 1.407038 | 0.002306 | 0.001890 | 0.002054 |
| is_copy_constructible_test.test | RUN | 0.257343 | 0.731988 | 1.493325 | 0.001915 | 0.001828 | 0.002068 |
| is_copy_assignable_test.test | RUN | 0.259507 | 0.733310 | 1.428862 | 0.001996 | 0.001824 | 0.002069 |
| is_convertible_test.test | RUN | 0.245062 | 0.654010 | 1.230149 | 0.002200 | 0.002108 | 0.001765 |
| is_constructible_test.test | RUN | 0.196392 | 0.474106 | 0.880790 | 0.001920 | 0.001857 | 0.001673 |
| is_const_test.test | RUN | 0.175357 | 0.414039 | 0.689056 | 0.002000 | 0.001860 | 0.001589 |
| is_compound_test.test | RUN | 0.163769 | 0.346319 | 0.553326 | 0.001928 | 0.001856 | 0.002281 |
| is_complex_test.test | RUN | 0.221292 | 0.465123 | 0.814861 | 0.001968 | 0.002094 | 0.001874 |
| is_complete_test.test | RUN | 0.196805 | 0.487256 | 0.885000 | 0.001899 | 0.001811 | 0.001642 |
| is_class_test.test | RUN | 0.181660 | 0.420837 | 0.763318 | 0.001925 | 0.001700 | 0.001878 |
| is_bounded_array_test.test | RUN | 0.170682 | 0.377531 | 0.669478 | 0.001533 | 0.001840 | 0.001624 |
| is_base_of_test.test | RUN | 0.183320 | 0.413221 | 0.749460 | 0.002060 | 0.002114 | 0.001579 |
| is_base_and_derived_test.test | RUN | 0.180492 | 0.406253 | 0.727311 | 0.001750 | 0.001698 | 0.001676 |
| is_assignable_test.test | RUN | 0.222059 | 0.577496 | 1.104119 | 0.001787 | 0.001867 | 0.001872 |
| is_array_test.test | RUN | 0.176505 | 0.397243 | 0.726008 | 0.001999 | 0.001893 | 0.002108 |
| is_arithmetic_test.test | RUN | 0.221133 | 0.600998 | 1.211717 | 0.002119 | 0.002148 | 0.002052 |
| is_abstract_test.test | RUN | 0.422620 | 1.643515 | 3.799538 | 0.002127 | 0.001940 | 0.002152 |
| integral_constant_test.test | RUN | 0.025105 | 0.065826 | 0.066022 | 0.000745 | 0.000920 | 0.001724 |
| has_virtual_destructor_test.test | RUN | 0.192509 | 0.476031 | 0.873026 | 0.002193 | 0.002138 | 0.002000 |
| has_unary_plus_test.test | RUN | 0.363512 | 1.865902 | 3.010126 | 0.001967 | 0.002300 | 0.002092 |
| has_unary_minus_test.test | RUN | 0.349173 | 1.843334 | 3.012407 | 0.001823 | 0.001886 | 0.001858 |
| has_trivial_move_constructor_test.test | RUN | 0.258634 | 0.687713 | 1.460968 | 0.001740 | 0.001839 | 0.002104 |
| has_trivial_move_assign_test.test | RUN | 0.248466 | 0.618463 | 1.249604 | 0.001977 | 0.001827 | 0.002081 |
| has_trivial_destructor_test.test | RUN | 0.253218 | 0.669680 | 1.492987 | 0.001820 | 0.001982 | 0.001848 |
| has_trivial_copy_test.test | RUN | 0.253688 | 0.754810 | 1.498236 | 0.001978 | 0.002324 | 0.002287 |
| has_trivial_constr_test.test | RUN | 0.258714 | 0.740044 | 1.535520 | 0.001982 | 0.001948 | 0.001810 |
| has_trivial_assign_test.test | RUN | 0.259285 | 0.731228 | 1.501049 | 0.002062 | 0.001860 | 0.001840 |
| has_right_shift_test.test | RUN | 0.374958 | 1.408553 | 3.210966 | 0.001931 | 0.002043 | 0.001545 |
| has_right_shift_assign_test.test | RUN | 0.356369 | 1.289352 | 2.999102 | 0.002360 | 0.002088 | 0.002151 |
| has_prefix_classes3_test.test | RUN | 0.270510 | 0.756079 | 1.551198 | 0.001853 | 0.001992 | 0.001958 |
| has_prefix_classes2_test.test | RUN | 0.271716 | 0.750929 | 1.510765 | 0.001845 | 0.002027 | 0.001887 |
| has_prefix_classes1_test.test | RUN | 0.259575 | 0.786563 | 1.567893 | 0.001794 | 0.002118 | 0.002155 |
| has_prefix_classes0_test.test | RUN | 0.265454 | 0.796011 | 1.558370 | 0.001876 | 0.002212 | 0.001908 |
| has_pre_increment_test.test | RUN | 0.356799 | 1.867754 | 2.991326 | 0.002030 | 0.001784 | 0.001881 |
| has_pre_decrement_test.test | RUN | 0.354736 | 1.911024 | 2.998844 | 0.001869 | 0.002052 | 0.001952 |
| has_postfix_classes3_test.test | RUN | 0.267770 | 0.740349 | 1.567692 | 0.001926 | 0.002087 | 0.001830 |
| has_postfix_classes2_test.test | RUN | 0.272197 | 0.761662 | 1.539518 | 0.001871 | 0.001876 | 0.001809 |
| has_postfix_classes1_test.test | RUN | 0.269842 | 0.715562 | 1.532670 | 0.002200 | 0.002286 | 0.002138 |
| has_postfix_classes0_test.test | RUN | 0.271525 | 0.770855 | 1.550838 | 0.001852 | 0.002340 | 0.001894 |
| has_post_increment_test.test | RUN | 0.353716 | 1.901229 | 2.981096 | 0.002080 | 0.002128 | 0.002034 |
| has_post_decrement_test.test | RUN | 0.362054 | 1.882339 | 3.051885 | 0.001836 | 0.002163 | 0.002069 |
| has_plus_test.test | RUN | 0.363019 | 1.298009 | 2.968211 | 0.002104 | 0.002018 | 0.001661 |
| has_plus_assign_test.test | RUN | 0.372110 | 1.266456 | 2.935696 | 0.002070 | 0.002052 | 0.001742 |
| has_operator_new_test.test | RUN | 0.255086 | 0.682319 | 1.378195 | 0.002022 | 0.002096 | 0.001922 |
| has_nothrow_destructor_test.test | RUN | 0.262801 | 0.765320 | 1.518817 | 0.001912 | 0.002086 | 0.002096 |
| has_nothrow_copy_test.test | RUN | 0.252615 | 0.707395 | 1.448072 | 0.001962 | 0.001922 | 0.001721 |
| has_nothrow_constr_test.test | RUN | 0.251017 | 0.703638 | 1.426420 | 0.002068 | 0.001740 | 0.002015 |
| has_nothrow_assign_test.test | RUN | 0.245015 | 0.711079 | 1.406729 | 0.001888 | 0.002130 | 0.001862 |
| has_not_equal_to_test.test | RUN | 0.366637 | 1.875339 | 3.053475 | 0.002080 | 0.001868 | 0.002285 |
| has_negate_test.test | RUN | 0.342389 | 1.247363 | 2.842942 | 0.001795 | 0.001863 | 0.001692 |
| has_multiplies_test.test | RUN | 0.367376 | 1.266108 | 2.942906 | 0.002240 | 0.001721 | 0.001722 |
| has_multiplies_assign_test.test | RUN | 0.371723 | 1.255962 | 3.067582 | 0.001913 | 0.002096 | 0.001946 |
| has_modulus_test.test | RUN | 0.349962 | 1.269851 | 2.972841 | 0.001839 | 0.001890 | 0.002037 |
| has_modulus_assign_test.test | RUN | 0.351750 | 1.262101 | 3.012669 | 0.001829 | 0.001774 | 0.001860 |
| has_minus_test.test | RUN | 0.372074 | 1.331011 | 3.037942 | 0.002156 | 0.001978 | 0.001824 |
| has_minus_assign_test.test | RUN | 0.374014 | 1.295651 | 3.061966 | 0.001582 | 0.001971 | 0.001938 |
| has_logical_or_test.test | RUN | 0.353645 | 1.273238 | 2.954328 | 0.001791 | 0.002020 | 0.002192 |
| has_logical_not_test.test | RUN | 0.340390 | 1.234650 | 2.898361 | 0.001685 | 0.001912 | 0.001858 |
| has_logical_and_test.test | RUN | 0.356403 | 1.311108 | 3.011912 | 0.002122 | 0.002159 | 0.002004 |
| has_less_test.test | RUN | 0.360427 | 1.877308 | 3.030542 | 0.001856 | 0.002119 | 0.002332 |
| has_less_equal_test.test | RUN | 0.354779 | 1.766321 | 2.955140 | 0.001736 | 0.002066 | 0.002108 |
| has_left_shift_test.test | RUN | 0.386203 | 1.678454 | 3.615424 | 0.001880 | 0.002050 | 0.001860 |
| has_left_shift_assign_test.test | RUN | 0.354185 | 1.252548 | 3.034897 | 0.002147 | 0.001780 | 0.001945 |
| has_greater_test.test | RUN | 0.367052 | 1.867160 | 3.042790 | 0.002084 | 0.001734 | 0.002033 |
| has_greater_equal_test.test | RUN | 0.364605 | 1.809704 | 3.018381 | 0.001984 | 0.002229 | 0.001956 |
| has_equal_to_test.test | RUN | 0.359018 | 1.804084 | 2.995307 | 0.001522 | 0.002058 | 0.001965 |
| has_divides_test.test | RUN | 0.356835 | 1.274686 | 3.007485 | 0.002067 | 0.001747 | 0.001857 |
| has_divides_assign_test.test | RUN | 0.362477 | 1.297733 | 3.000810 | 0.001974 | 0.002078 | 0.001444 |
| has_dereference_test.test | RUN | 0.346326 | 1.279682 | 2.904970 | 0.001980 | 0.001964 | 0.002032 |
| has_complement_test.test | RUN | 0.353245 | 1.849950 | 2.970658 | 0.001732 | 0.002176 | 0.002128 |
| has_bit_xor_test.test | RUN | 0.362096 | 1.337693 | 2.966824 | 0.002113 | 0.002238 | 0.002062 |
| has_bit_xor_assign_test.test | RUN | 0.361383 | 1.351519 | 3.012911 | 0.002042 | 0.002022 | 0.001880 |
| has_bit_or_test.test | RUN | 0.354297 | 1.236558 | 2.947689 | 0.001934 | 0.001898 | 0.002104 |
| has_bit_or_assign_test.test | RUN | 0.348244 | 1.321748 | 2.972477 | 0.001814 | 0.002106 | 0.001853 |
| has_bit_and_test.test | RUN | 0.355189 | 1.318800 | 2.939520 | 0.002170 | 0.002204 | 0.001813 |
| has_bit_and_assign_test.test | RUN | 0.360473 | 1.286290 | 2.997946 | 0.001967 | 0.002244 | 0.001898 |
| has_binary_classes9_test.test | RUN | 0.390088 | 1.565553 | 3.315989 | 0.002078 | 0.002344 | 0.002068 |
| has_binary_classes8_test.test | RUN | 0.386760 | 1.534493 | 3.267956 | 0.001552 | 0.002130 | 0.001794 |
| has_binary_classes7_test.test | RUN | 0.391516 | 1.495134 | 3.302761 | 0.002216 | 0.002120 | 0.001659 |
| has_binary_classes6_test.test | RUN | 0.386089 | 1.472098 | 3.309367 | 0.002011 | 0.002414 | 0.001986 |
| has_binary_classes5_test.test | RUN | 0.385833 | 1.585864 | 3.303377 | 0.002184 | 0.002326 | 0.002206 |
| has_binary_classes4_test.test | RUN | 0.387168 | 1.598541 | 3.209168 | 0.001790 | 0.002271 | 0.002022 |
| has_binary_classes3_test.test | RUN | 0.393630 | 1.545510 | 3.319379 | 0.002068 | 0.002107 | 0.001570 |
| has_binary_classes2_test.test | RUN | 0.395807 | 1.605944 | 3.309331 | 0.002100 | 0.002167 | 0.001961 |
| has_binary_classes1_test.test | RUN | 0.389446 | 1.529746 | 3.299100 | 0.001826 | 0.002105 | 0.001668 |
| has_binary_classes0_test.test | RUN | 0.390137 | 1.561514 | 3.251085 | 0.001564 | 0.001874 | 0.001968 |
| function_traits_test.test | RUN | 0.184132 | 0.388982 | 0.723318 | 0.002046 | 0.002260 | 0.001806 |
| floating_point_promotion_test.test | RUN | 0.174951 | 0.381718 | 0.672355 | 0.002008 | 0.002169 | 0.001942 |
| extent_test.test | RUN | 0.175169 | 0.403011 | 0.706980 | 0.002143 | 0.001844 | 0.001786 |
| enable_if_test.test | RUN | 0.162236 | 0.332842 | 0.558295 | 0.001795 | 0.001951 | 0.001920 |
| disjunction_test.test | RUN | 0.164733 | 0.345021 | 0.586460 | 0.001872 | 0.002240 | 0.002028 |
| detected_test.test | RUN | 0.165269 | 0.333210 | 0.541968 | 0.002290 | 0.002042 | 0.001623 |
| detected_or_test.test | RUN | 0.162733 | 0.351941 | 0.588500 | 0.001898 | 0.001794 | 0.001840 |
| decay_test.test | RUN | 0.418513 | 1.496048 | 3.556542 | 0.002005 | 0.001813 | 0.002072 |
| cxx14_aliases_test.test | RUN | 0.271728 | 0.517214 | 0.924210 | 0.002325 | 0.001882 | 0.002292 |
| copy_reference_test.test | RUN | 0.164284 | 0.354140 | 0.582006 | 0.002208 | 0.001780 | 0.001975 |
| copy_cv_test.test | RUN | 0.172262 | 0.377593 | 0.664067 | 0.002166 | 0.002045 | 0.002037 |
| copy_cv_ref_test.test | RUN | 0.236750 | 0.642101 | 1.401230 | 0.001870 | 0.001917 | 0.002041 |
| conjunction_test.test | RUN | 0.167990 | 0.356431 | 0.603889 | 0.001869 | 0.002176 | 0.002029 |
| conditional_test.test | RUN | 0.163600 | 0.347710 | 0.584550 | 0.001632 | 0.001730 | 0.002130 |
| common_type_test.test | RUN | 0.221536 | 0.550432 | 1.100930 | 0.001856 | 0.001855 | 0.002369 |
| common_type_sfinae_test.test | RUN | 0.164495 | 0.332033 | 0.543981 | 0.002046 | 0.001934 | 0.002021 |
| common_type_sfinae2_test.test | RUN | 0.171018 | 0.344458 | 0.592342 | 0.001920 | 0.001954 | 0.002027 |
| common_type_6_test.test | RUN | 0.176425 | 0.357369 | 0.544392 | 0.001734 | 0.001994 | 0.002111 |
| common_type_5_test.test | RUN | 0.175123 | 0.351089 | 0.622258 | 0.002072 | 0.002184 | 0.002048 |
| common_type_4_test.test | RUN | 0.176859 | 0.398571 | 0.741855 | 0.001791 | 0.001995 | 0.001880 |
| common_type_3_test.test | RUN | 0.162972 | 0.326139 | 0.502600 | 0.001877 | 0.002021 | 0.001842 |
| common_type_2_test.test | RUN | 0.218055 | 0.560788 | 1.084611 | 0.002114 | 0.002224 | 0.001742 |
| alignment_of_test.test | RUN | 0.179410 | 0.416318 | 0.710516 | 0.001676 | 0.001794 | 0.001671 |
| alignment_of_a2_test.test | RUN | 0.180548 | 0.428455 | 0.712626 | 0.002088 | 0.002124 | 0.002030 |
| aligned_storage_test.test | RUN | 0.186061 | 0.425588 | 0.643611 | 0.001997 | 0.002192 | 0.002107 |
| aligned_storage_empy_test.test | RUN | 0.212983 | 0.468766 | 0.892237 | 0.002036 | 0.002027 | 0.002062 |
| aligned_storage_a2_test.test | RUN | 0.187889 | 0.403109 | 0.640342 | 0.002059 | 0.002069 | 0.002478 |
| add_volatile_test.test | RUN | 0.376493 | 1.278904 | 3.350789 | 0.002330 | 0.002124 | 0.002040 |
| add_rvalue_reference_test.test | RUN | 0.425338 | 1.418121 | 3.752360 | 0.001808 | 0.001974 | 0.002036 |
| add_reference_test.test | RUN | 0.420228 | 1.433290 | 3.784624 | 0.001984 | 0.002244 | 0.002086 |
| add_pointer_test.test | RUN | 0.273822 | 0.783269 | 1.947101 | 0.001947 | 0.001860 | 0.002054 |
| add_lvalue_reference_test.test | RUN | 0.419552 | 1.355333 | 3.718445 | 0.001928 | 0.001938 | 0.001784 |
| add_cv_test.test | RUN | 0.390640 | 1.283873 | 3.517350 | 0.001854 | 0.001921 | 0.001994 |
| add_const_test.test | RUN | 0.403223 | 1.390832 | 3.532601 | 0.001925 | 0.001953 | 0.002038 |
| is_virtual_base_of_fail.test | COMPILE_FAIL | 0.166022 | 0.331160 | 0.536686 | — | — | — |
| is_stateless_fail.test | COMPILE_FAIL | 0.169047 | 0.336598 | 0.558648 | — | — | — |
| is_polymorphic_fail.test | COMPILE_FAIL | 0.154988 | 0.310599 | 0.517913 | — | — | — |
| is_nothrow_move_constructible_fail.test | COMPILE_FAIL | 0.159145 | 0.320632 | 0.480969 | — | — | — |
| is_nothrow_move_assignable_fail.test | COMPILE_FAIL | 0.162267 | 0.309614 | 0.499082 | — | — | — |
| is_list_constructible_fail.test | COMPILE_FAIL | 0.159903 | 0.320974 | 0.482438 | — | — | — |
| is_empty_fail.test | COMPILE_FAIL | 0.159714 | 0.315078 | 0.513094 | — | — | — |
| is_destructible_fail.test | COMPILE_FAIL | 0.156680 | 0.317809 | 0.502857 | — | — | — |
| is_default_constructible_fail.test | COMPILE_FAIL | 0.161275 | 0.323948 | 0.500499 | — | — | — |
| is_copy_constructible_fail.test | COMPILE_FAIL | 0.160918 | 0.318610 | 0.517560 | — | — | — |
| is_copy_assignable_fail.test | COMPILE_FAIL | 0.155888 | 0.312419 | 0.500096 | — | — | — |
| is_convertible_fail.test | COMPILE_FAIL | 0.163132 | 0.321587 | 0.507283 | — | — | — |
| is_convertible2_fail.test | COMPILE_FAIL | 0.166227 | 0.325403 | 0.509970 | — | — | — |
| is_constructible_fail.test | COMPILE_FAIL | 0.159456 | 0.320990 | 0.515997 | — | — | — |
| is_base_of_fail.test | COMPILE_FAIL | 0.156952 | 0.311147 | 0.512536 | — | — | — |
| is_assignable_fail.test | COMPILE_FAIL | 0.157652 | 0.318721 | 0.499044 | — | — | — |
| has_nothrow_destruct_fail.test | COMPILE_FAIL | 0.161829 | 0.319896 | 0.515540 | — | — | — |
| has_nothrow_copy_fail.test | COMPILE_FAIL | 0.160510 | 0.320135 | 0.513431 | — | — | — |
| has_nothrow_cons_fail.test | COMPILE_FAIL | 0.159286 | 0.316035 | 0.501227 | — | — | — |
| has_nothrow_assign_fail.test | COMPILE_FAIL | 0.159174 | 0.320539 | 0.502683 | — | — | — |
| common_type_fail.test | COMPILE_FAIL | 0.165728 | 0.320163 | 0.498016 | — | — | — |
| common_type2_fail.test | COMPILE_FAIL | 0.158649 | 0.312127 | 0.512630 | — | — | — |
| has_nothrow_assign_test_no_intrinsics.test | RUN | 0.252409 | 0.714483 | 1.442184 | 0.002154 | 0.001805 | 0.002076 |
| has_nothrow_constr_test_no_intrinsics.test | RUN | 0.250103 | 0.723118 | 1.425476 | 0.001693 | 0.001896 | 0.001584 |
| has_nothrow_copy_test_no_intrinsics.test | RUN | 0.257337 | 0.712204 | 1.401653 | 0.001645 | 0.002079 | 0.001780 |
| is_nothrow_move_assignable_test_no_intrinsics.test | RUN | 0.246859 | 0.705844 | 1.413640 | 0.001768 | 0.001458 | 0.001495 |
| is_nothrow_move_constructible_test_no_intrinsics.test | RUN | 0.152834 | 0.450625 | 0.905388 | 0.001609 | 0.001442 | 0.001347 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.993× | 0.987×, 0.998× | 0.996× |
| clang | 1.024× | 1.024×, 1.023× | 0.996× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| add_const_test.test | 10 | 0.001496s | 0.001516s | 0.001492s | byte equality observed |
| add_cv_test.test | 10 | 0.001499s | 0.001510s | 0.001512s | byte equality observed |
| add_lvalue_reference_test.test | 10 | 0.001496s | 0.001508s | 0.001509s | byte equality observed |
| add_pointer_test.test | 10 | 0.001539s | 0.001493s | 0.001522s | byte equality observed |
| add_reference_test.test | 10 | 0.001517s | 0.001498s | 0.001530s | byte equality observed |
| add_rvalue_reference_test.test | 10 | 0.001521s | 0.001495s | 0.001489s | byte equality observed |
| add_volatile_test.test | 10 | 0.001538s | 0.001490s | 0.001529s | byte equality observed |
| aligned_storage_a2_test.test | 10 | 0.001664s | 0.001680s | 0.001604s | byte equality observed |
| aligned_storage_empy_test.test | 10 | 0.001495s | 0.001499s | 0.001518s | byte equality observed |
| aligned_storage_test.test | 10 | 0.001653s | 0.001655s | 0.001681s | byte equality observed |
| alignment_of_a2_test.test | 10 | 0.001525s | 0.001491s | 0.001528s | byte equality observed |
| alignment_of_test.test | 10 | 0.001529s | 0.001524s | 0.001486s | byte equality observed |
| common_type_2_test.test | 10 | 0.001526s | 0.001523s | 0.001517s | byte equality observed |
| common_type_3_test.test | 10 | 0.001488s | 0.001506s | 0.001526s | byte equality observed |
| common_type_4_test.test | 10 | 0.001500s | 0.001499s | 0.001509s | byte equality observed |
| common_type_5_test.test | 10 | 0.001520s | 0.001523s | 0.001501s | byte equality observed |
| common_type_6_test.test | 10 | 0.001495s | 0.001510s | 0.001499s | byte equality observed |
| common_type_sfinae2_test.test | 10 | 0.001503s | 0.001479s | 0.001526s | byte equality observed |
| common_type_sfinae_test.test | 8 | 0.001520s | 0.001520s | 0.001533s | byte equality observed |
| common_type_test.test | 10 | 0.001494s | 0.001520s | 0.001492s | byte equality observed |
| conditional_test.test | 10 | 0.001513s | 0.001482s | 0.001505s | byte equality observed |
| conjunction_test.test | 10 | 0.001518s | 0.001491s | 0.001497s | byte equality observed |
| copy_cv_ref_test.test | 8 | 0.001502s | 0.001501s | 0.001507s | byte equality observed |
| copy_cv_test.test | 8 | 0.001511s | 0.001513s | 0.001506s | byte equality observed |
| copy_reference_test.test | 10 | 0.001505s | 0.001482s | 0.001482s | byte equality observed |
| cxx14_aliases_test.test | 10 | 0.001514s | 0.001491s | 0.001516s | byte equality observed |
| decay_test.test | 10 | 0.001528s | 0.001516s | 0.001489s | byte equality observed |
| detected_or_test.test | 10 | 0.001510s | 0.001467s | 0.001504s | byte equality observed |
| detected_test.test | 10 | 0.001511s | 0.001492s | 0.001504s | byte equality observed |
| disjunction_test.test | 10 | 0.001504s | 0.001513s | 0.001521s | byte equality observed |
| enable_if_test.test | 10 | 0.001512s | 0.001496s | 0.001530s | byte equality observed |
| extent_test.test | 10 | 0.001489s | 0.001488s | 0.001514s | byte equality observed |
| floating_point_promotion_test.test | 10 | 0.001503s | 0.001519s | 0.001551s | byte equality observed |
| function_traits_test.test | 10 | 0.001512s | 0.001495s | 0.001570s | byte equality observed |
| has_binary_classes0_test.test | 10 | 0.001499s | 0.001495s | 0.001485s | byte equality observed |
| has_binary_classes1_test.test | 10 | 0.001520s | 0.001506s | 0.001539s | byte equality observed |
| has_binary_classes2_test.test | 10 | 0.001497s | 0.001527s | 0.001527s | byte equality observed |
| has_binary_classes3_test.test | 10 | 0.001527s | 0.001512s | 0.001547s | byte equality observed |
| has_binary_classes4_test.test | 10 | 0.001509s | 0.001508s | 0.001532s | byte equality observed |
| has_binary_classes5_test.test | 10 | 0.001490s | 0.001474s | 0.001531s | byte equality observed |
| has_binary_classes6_test.test | 10 | 0.001508s | 0.001473s | 0.001524s | byte equality observed |
| has_binary_classes7_test.test | 10 | 0.001493s | 0.001497s | 0.001520s | byte equality observed |
| has_binary_classes8_test.test | 10 | 0.001497s | 0.001514s | 0.001485s | byte equality observed |
| has_binary_classes9_test.test | 10 | 0.001507s | 0.001512s | 0.001519s | byte equality observed |
| has_bit_and_assign_test.test | 10 | 0.001524s | 0.001513s | 0.001534s | byte equality observed |
| has_bit_and_test.test | 10 | 0.001507s | 0.001487s | 0.001500s | byte equality observed |
| has_bit_or_assign_test.test | 10 | 0.001530s | 0.001505s | 0.001504s | byte equality observed |
| has_bit_or_test.test | 10 | 0.001523s | 0.001540s | 0.001513s | byte equality observed |
| has_bit_xor_assign_test.test | 9 | 0.001504s | 0.001503s | 0.001531s | byte equality observed |
| has_bit_xor_test.test | 10 | 0.001500s | 0.001489s | 0.001541s | byte equality observed |
| has_complement_test.test | 10 | 0.001516s | 0.001505s | 0.001523s | byte equality observed |
| has_dereference_test.test | 10 | 0.001503s | 0.001498s | 0.001531s | byte equality observed |
| has_divides_assign_test.test | 10 | 0.001511s | 0.001514s | 0.001501s | byte equality observed |
| has_divides_test.test | 10 | 0.001523s | 0.001483s | 0.001482s | byte equality observed |
| has_equal_to_test.test | 10 | 0.001516s | 0.001502s | 0.001513s | byte equality observed |
| has_greater_equal_test.test | 10 | 0.001514s | 0.001496s | 0.001498s | byte equality observed |
| has_greater_test.test | 10 | 0.001513s | 0.001507s | 0.001512s | byte equality observed |
| has_left_shift_assign_test.test | 10 | 0.001509s | 0.001501s | 0.001510s | byte equality observed |
| has_left_shift_test.test | 10 | 0.001501s | 0.001451s | 0.001498s | byte equality observed |
| has_less_equal_test.test | 10 | 0.001504s | 0.001532s | 0.001472s | byte equality observed |
| has_less_test.test | 10 | 0.001511s | 0.001499s | 0.001501s | byte equality observed |
| has_logical_and_test.test | 10 | 0.001534s | 0.001499s | 0.001519s | byte equality observed |
| has_logical_not_test.test | 10 | 0.001488s | 0.001538s | 0.001500s | byte equality observed |
| has_logical_or_test.test | 10 | 0.001515s | 0.001503s | 0.001541s | byte equality observed |
| has_minus_assign_test.test | 10 | 0.001503s | 0.001512s | 0.001514s | byte equality observed |
| has_minus_test.test | 10 | 0.001506s | 0.001511s | 0.001512s | byte equality observed |
| has_modulus_assign_test.test | 8 | 0.001540s | 0.001491s | 0.001509s | byte equality observed |
| has_modulus_test.test | 10 | 0.001513s | 0.001523s | 0.001490s | byte equality observed |
| has_multiplies_assign_test.test | 10 | 0.001545s | 0.001532s | 0.001489s | byte equality observed |
| has_multiplies_test.test | 10 | 0.001499s | 0.001494s | 0.001494s | byte equality observed |
| has_negate_test.test | 10 | 0.001519s | 0.001485s | 0.001472s | byte equality observed |
| has_not_equal_to_test.test | 10 | 0.001517s | 0.001515s | 0.001507s | byte equality observed |
| has_nothrow_assign_test.test | 10 | 0.001506s | 0.001477s | 0.001495s | byte equality observed |
| has_nothrow_assign_test_no_intrinsics.test | 10 | 0.001518s | 0.001496s | 0.001514s | byte equality observed |
| has_nothrow_constr_test.test | 10 | 0.001491s | 0.001485s | 0.001461s | byte equality observed |
| has_nothrow_constr_test_no_intrinsics.test | 8 | 0.001508s | 0.001530s | 0.001513s | byte equality observed |
| has_nothrow_copy_test.test | 10 | 0.001503s | 0.001499s | 0.001530s | byte equality observed |
| has_nothrow_copy_test_no_intrinsics.test | 10 | 0.001508s | 0.001468s | 0.001491s | byte equality observed |
| has_nothrow_destructor_test.test | 10 | 0.001508s | 0.001464s | 0.001490s | byte equality observed |
| has_operator_new_test.test | 10 | 0.001487s | 0.001489s | 0.001498s | byte equality observed |
| has_plus_assign_test.test | 10 | 0.001511s | 0.001509s | 0.001502s | byte equality observed |
| has_plus_test.test | 10 | 0.001494s | 0.001505s | 0.001484s | byte equality observed |
| has_post_decrement_test.test | 10 | 0.001489s | 0.001478s | 0.001490s | byte equality observed |
| has_post_increment_test.test | 10 | 0.001512s | 0.001505s | 0.001502s | byte equality observed |
| has_postfix_classes0_test.test | 10 | 0.001503s | 0.001487s | 0.001514s | byte equality observed |
| has_postfix_classes1_test.test | 10 | 0.001492s | 0.001490s | 0.001476s | byte equality observed |
| has_postfix_classes2_test.test | 10 | 0.001469s | 0.001476s | 0.001479s | byte equality observed |
| has_postfix_classes3_test.test | 10 | 0.001506s | 0.001508s | 0.001509s | byte equality observed |
| has_pre_decrement_test.test | 10 | 0.001514s | 0.001427s | 0.001516s | byte equality observed |
| has_pre_increment_test.test | 10 | 0.001512s | 0.001495s | 0.001465s | byte equality observed |
| has_prefix_classes0_test.test | 10 | 0.001497s | 0.001515s | 0.001507s | byte equality observed |
| has_prefix_classes1_test.test | 10 | 0.001527s | 0.001526s | 0.001498s | byte equality observed |
| has_prefix_classes2_test.test | 10 | 0.001506s | 0.001530s | 0.001472s | byte equality observed |
| has_prefix_classes3_test.test | 10 | 0.001504s | 0.001493s | 0.001471s | byte equality observed |
| has_right_shift_assign_test.test | 10 | 0.001509s | 0.001475s | 0.001518s | byte equality observed |
| has_right_shift_test.test | 10 | 0.001514s | 0.001506s | 0.001491s | byte equality observed |
| has_trivial_assign_test.test | 10 | 0.001581s | 0.001628s | 0.001584s | byte equality observed |
| has_trivial_constr_test.test | 10 | 0.001586s | 0.001558s | 0.001584s | native test oracle; output varies |
| has_trivial_copy_test.test | 10 | 0.001603s | 0.001561s | 0.001620s | native test oracle; output varies |
| has_trivial_destructor_test.test | 10 | 0.001514s | 0.001503s | 0.001446s | byte equality observed |
| has_trivial_move_assign_test.test | 10 | 0.001476s | 0.001507s | 0.001500s | byte equality observed |
| has_trivial_move_constructor_test.test | 10 | 0.001503s | 0.001497s | 0.001464s | byte equality observed |
| has_unary_minus_test.test | 10 | 0.001513s | 0.001505s | 0.001490s | byte equality observed |
| has_unary_plus_test.test | 8 | 0.001500s | 0.001524s | 0.001487s | byte equality observed |
| has_virtual_destructor_test.test | 10 | 0.001533s | 0.001516s | 0.001496s | byte equality observed |
| integral_constant_test.test | 10 | 0.000686s | 0.000666s | 0.001527s | byte equality observed |
| is_abstract_test.test | 10 | 0.001527s | 0.001483s | 0.001488s | byte equality observed |
| is_arithmetic_test.test | 10 | 0.001524s | 0.001527s | 0.001532s | byte equality observed |
| is_array_test.test | 10 | 0.001508s | 0.001493s | 0.001502s | byte equality observed |
| is_assignable_test.test | 10 | 0.001502s | 0.001498s | 0.001498s | byte equality observed |
| is_base_and_derived_test.test | 10 | 0.001501s | 0.001487s | 0.001502s | byte equality observed |
| is_base_of_test.test | 10 | 0.001492s | 0.001484s | 0.001451s | byte equality observed |
| is_bounded_array_test.test | 10 | 0.001516s | 0.001460s | 0.001509s | byte equality observed |
| is_class_test.test | 10 | 0.001492s | 0.001482s | 0.001501s | byte equality observed |
| is_complete_test.test | 10 | 0.001485s | 0.001474s | 0.001496s | byte equality observed |
| is_complex_test.test | 10 | 0.001503s | 0.001507s | 0.001484s | byte equality observed |
| is_compound_test.test | 10 | 0.001524s | 0.001510s | 0.001493s | byte equality observed |
| is_const_test.test | 10 | 0.001524s | 0.001480s | 0.001496s | byte equality observed |
| is_constructible_test.test | 10 | 0.001514s | 0.001467s | 0.001462s | byte equality observed |
| is_convertible_test.test | 10 | 0.001520s | 0.001520s | 0.001502s | byte equality observed |
| is_copy_assignable_test.test | 10 | 0.001501s | 0.001549s | 0.001527s | byte equality observed |
| is_copy_constructible_test.test | 10 | 0.001494s | 0.001514s | 0.001495s | byte equality observed |
| is_default_constr_test.test | 10 | 0.001504s | 0.001459s | 0.001485s | byte equality observed |
| is_destructible_test.test | 10 | 0.001513s | 0.001515s | 0.001465s | byte equality observed |
| is_detected_exact_test.test | 10 | 0.001509s | 0.001501s | 0.001478s | byte equality observed |
| is_detected_test.test | 10 | 0.001513s | 0.001521s | 0.001543s | byte equality observed |
| is_empty_test.test | 10 | 0.001499s | 0.001495s | 0.001494s | byte equality observed |
| is_enum_test.test | 10 | 0.001523s | 0.001538s | 0.001516s | byte equality observed |
| is_final_test.test | 10 | 0.001513s | 0.001501s | 0.001494s | byte equality observed |
| is_float_test.test | 10 | 0.001525s | 0.001507s | 0.001483s | byte equality observed |
| is_floating_point_test.test | 10 | 0.001518s | 0.001483s | 0.001507s | byte equality observed |
| is_function_test.test | 10 | 0.001532s | 0.001526s | 0.001527s | byte equality observed |
| is_fundamental_test.test | 10 | 0.001536s | 0.001503s | 0.001519s | byte equality observed |
| is_integral_test.test | 10 | 0.001514s | 0.001483s | 0.001478s | byte equality observed |
| is_list_constructible_test.test | 10 | 0.001515s | 0.001497s | 0.001509s | byte equality observed |
| is_lvalue_reference_test.test | 10 | 0.001500s | 0.001528s | 0.001464s | byte equality observed |
| is_member_func_test.test | 10 | 0.001467s | 0.001501s | 0.001497s | byte equality observed |
| is_member_obj_test.test | 10 | 0.001518s | 0.001477s | 0.001507s | byte equality observed |
| is_member_pointer_test.test | 10 | 0.001497s | 0.001482s | 0.001513s | byte equality observed |
| is_noncopyable_test.test | 10 | 0.001483s | 0.001508s | 0.001510s | byte equality observed |
| is_nothrow_move_assignable_test.test | 10 | 0.001504s | 0.001507s | 0.001501s | byte equality observed |
| is_nothrow_move_assignable_test_no_intrinsics.test | 10 | 0.001509s | 0.001499s | 0.001490s | byte equality observed |
| is_nothrow_move_constructible_test.test | 10 | 0.001520s | 0.001486s | 0.001505s | byte equality observed |
| is_nothrow_move_constructible_test_no_intrinsics.test | 10 | 0.001507s | 0.001503s | 0.001539s | byte equality observed |
| is_nothrow_swappable_test.test | 10 | 0.001516s | 0.001495s | 0.001524s | byte equality observed |
| is_object_test.test | 10 | 0.001507s | 0.001493s | 0.001523s | byte equality observed |
| is_pod_test.test | 10 | 0.001534s | 0.001496s | 0.001510s | byte equality observed |
| is_pointer_test.test | 10 | 0.001522s | 0.001523s | 0.001529s | byte equality observed |
| is_polymorphic_test.test | 10 | 0.001524s | 0.001495s | 0.001500s | byte equality observed |
| is_reference_test.test | 10 | 0.001517s | 0.001506s | 0.001502s | byte equality observed |
| is_rvalue_reference_test.test | 10 | 0.001466s | 0.001493s | 0.001504s | byte equality observed |
| is_same_test.test | 10 | 0.001495s | 0.001472s | 0.001460s | byte equality observed |
| is_scalar_test.test | 10 | 0.001479s | 0.001467s | 0.001505s | byte equality observed |
| is_scoped_enum_test.test | 10 | 0.001514s | 0.001496s | 0.001493s | byte equality observed |
| is_signed_test.test | 10 | 0.001493s | 0.001523s | 0.001513s | byte equality observed |
| is_stateless_test.test | 10 | 0.001504s | 0.001505s | 0.001503s | byte equality observed |
| is_swappable_test.test | 10 | 0.001515s | 0.001486s | 0.001504s | byte equality observed |
| is_trivially_copyable_test.test | 10 | 0.001529s | 0.001542s | 0.001565s | byte equality observed |
| is_unbounded_array_test.test | 7 | 0.001526s | 0.001517s | 0.001503s | byte equality observed |
| is_union_test.test | 10 | 0.001533s | 0.001485s | 0.001493s | byte equality observed |
| is_unscoped_enum_test.test | 10 | 0.001500s | 0.001507s | 0.001492s | byte equality observed |
| is_unsigned_test.test | 10 | 0.001500s | 0.001500s | 0.001508s | byte equality observed |
| is_virtual_base_of_test.test | 10 | 0.001502s | 0.001494s | 0.001472s | byte equality observed |
| is_void_test.test | 10 | 0.001497s | 0.001495s | 0.001500s | byte equality observed |
| is_volatile_test.test | 8 | 0.001512s | 0.001532s | 0.001482s | byte equality observed |
| make_signed_test.test | 10 | 0.001532s | 0.001504s | 0.001470s | byte equality observed |
| make_unsigned_test.test | 10 | 0.001501s | 0.001547s | 0.001500s | byte equality observed |
| make_void_test.test | 10 | 0.001526s | 0.001522s | 0.001494s | byte equality observed |
| mpl_interop_test1.test | 10 | 0.000681s | 0.000677s | 0.001518s | byte equality observed |
| mpl_interop_test2.test | 10 | 0.000692s | 0.000683s | 0.001519s | byte equality observed |
| mpl_interop_test3.test | 10 | 0.000696s | 0.000695s | 0.001512s | byte equality observed |
| negation_test.test | 10 | 0.001557s | 0.001572s | 0.001480s | byte equality observed |
| promote_basic_test.test | 10 | 0.000682s | 0.000693s | 0.001556s | byte equality observed |
| promote_enum_msvc_bug_test.test | 10 | 0.000694s | 0.000697s | 0.001489s | byte equality observed |
| promote_enum_test.test | 10 | 0.000690s | 0.000698s | 0.001484s | byte equality observed |
| promote_mpl_test.test | 10 | 0.000675s | 0.000693s | 0.001494s | byte equality observed |
| rank_test.test | 10 | 0.001554s | 0.001500s | 0.001510s | byte equality observed |
| remove_all_extents_test.test | 10 | 0.001537s | 0.001493s | 0.001460s | byte equality observed |
| remove_bounds_test.test | 10 | 0.001506s | 0.001497s | 0.001493s | byte equality observed |
| remove_const_test.test | 10 | 0.001524s | 0.001482s | 0.001504s | byte equality observed |
| remove_cv_ref_test.test | 10 | 0.001519s | 0.001506s | 0.001510s | byte equality observed |
| remove_cv_test.test | 10 | 0.001523s | 0.001493s | 0.001493s | byte equality observed |
| remove_extent_test.test | 10 | 0.001499s | 0.001483s | 0.001481s | byte equality observed |
| remove_pointer_test.test | 10 | 0.001514s | 0.001512s | 0.001528s | byte equality observed |
| remove_reference_test.test | 10 | 0.001527s | 0.001497s | 0.001510s | byte equality observed |
| remove_volatile_test.test | 10 | 0.001489s | 0.001481s | 0.001496s | byte equality observed |
| tricky_abstract_type_test.test | 10 | 0.001504s | 0.001487s | 0.001525s | byte equality observed |
| tricky_add_pointer_test.test | 10 | 0.001502s | 0.001499s | 0.001508s | byte equality observed |
| tricky_function_type_test.test | 10 | 0.001492s | 0.001517s | 0.001462s | byte equality observed |
| tricky_incomplete_type_test.test | 10 | 0.001492s | 0.001502s | 0.001514s | byte equality observed |
| tricky_is_enum_test.test | 10 | 0.001516s | 0.001534s | 0.001548s | byte equality observed |
| tricky_partial_spec_test.test | 10 | 0.001522s | 0.001508s | 0.001517s | byte equality observed |
| tricky_rvalue_test.test | 10 | 0.001510s | 0.001498s | 0.001520s | byte equality observed |
| type_identity_test.test | 10 | 0.001522s | 0.001531s | 0.001491s | byte equality observed |
| type_traits_test.test | 10 | 0.000664s | 0.000681s | 0.001492s | byte equality observed |
| type_with_alignment_test.test | 10 | 0.001683s | 0.001660s | 0.001666s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
