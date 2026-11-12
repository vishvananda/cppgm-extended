# libs/type_traits/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| type_with_alignment_test.test | RUN | 0.159576 | 0.448641 | 0.801008 | 0.001903 | 0.002209 | 0.002202 |
| type_traits_test.test | RUN | 0.230487 | 0.471593 | 0.812987 | 0.000742 | 0.000799 | 0.001796 |
| type_identity_test.test | RUN | 0.452465 | 1.770779 | 4.578721 | 0.002226 | 0.001814 | 0.002116 |
| tricky_rvalue_test.test | RUN | 0.150128 | 0.357950 | 0.635001 | 0.001822 | 0.001816 | 0.002112 |
| tricky_partial_spec_test.test | RUN | 0.184292 | 0.533374 | 1.036569 | 0.001827 | 0.001917 | 0.001991 |
| tricky_is_enum_test.test | RUN | 0.153635 | 0.349423 | 0.562543 | 0.001788 | 0.002118 | 0.002176 |
| tricky_incomplete_type_test.test | RUN | 0.249083 | 0.550438 | 0.917407 | 0.001793 | 0.002365 | 0.001894 |
| tricky_function_type_test.test | RUN | 0.198672 | 0.581381 | 1.050003 | 0.002283 | 0.001784 | 0.001951 |
| tricky_add_pointer_test.test | RUN | 0.293257 | 1.054462 | 2.546608 | 0.001923 | 0.002186 | 0.002232 |
| tricky_abstract_type_test.test | RUN | 0.150472 | 0.335466 | 0.574482 | 0.002094 | 0.002073 | 0.001892 |
| remove_volatile_test.test | RUN | 0.403063 | 1.444850 | 3.964803 | 0.002010 | 0.001873 | 0.002531 |
| remove_reference_test.test | RUN | 0.423189 | 1.549424 | 4.188205 | 0.002148 | 0.001907 | 0.002126 |
| remove_pointer_test.test | RUN | 0.454693 | 1.572154 | 4.421151 | 0.002177 | 0.001868 | 0.001852 |
| remove_extent_test.test | RUN | 0.420981 | 1.563632 | 4.204790 | 0.001959 | 0.002117 | 0.001748 |
| remove_cv_test.test | RUN | 0.421260 | 1.498149 | 4.165367 | 0.002119 | 0.002046 | 0.002076 |
| remove_cv_ref_test.test | RUN | 0.880320 | 3.650186 | 10.451356 | 0.001760 | 0.002252 | 0.001767 |
| remove_const_test.test | RUN | 0.402656 | 1.429666 | 3.974245 | 0.002225 | 0.002439 | 0.002032 |
| remove_bounds_test.test | RUN | 0.424127 | 1.517482 | 4.094434 | 0.001942 | 0.002233 | 0.002018 |
| remove_all_extents_test.test | RUN | 0.424240 | 1.601780 | 4.251725 | 0.001890 | 0.001970 | 0.002246 |
| rank_test.test | RUN | 0.150676 | 0.366105 | 0.547789 | 0.001631 | 0.001721 | 0.002540 |
| promote_mpl_test.test | RUN | 0.128280 | 0.319282 | 0.400742 | 0.000776 | 0.000745 | 0.002079 |
| promote_enum_test.test | RUN | 0.031309 | 0.080300 | 0.090647 | 0.000780 | 0.000866 | 0.002056 |
| promote_enum_msvc_bug_test.test | RUN | 0.028926 | 0.068867 | 0.072004 | 0.000599 | 0.000814 | 0.001870 |
| promote_basic_test.test | RUN | 0.031906 | 0.096130 | 0.098352 | 0.000868 | 0.000939 | 0.002128 |
| negation_test.test | RUN | 0.146672 | 0.328970 | 0.496468 | 0.001852 | 0.002150 | 0.002030 |
| mpl_interop_test3.test | RUN | 0.133779 | 0.286432 | 0.352252 | 0.000771 | 0.000889 | 0.002376 |
| mpl_interop_test2.test | RUN | 0.046448 | 0.084418 | 0.092802 | 0.000787 | 0.000735 | 0.001895 |
| mpl_interop_test1.test | RUN | 0.023537 | 0.059880 | 0.062157 | 0.000810 | 0.000744 | 0.001698 |
| make_void_test.test | RUN | 0.154536 | 0.366504 | 0.628389 | 0.002046 | 0.002078 | 0.002088 |
| make_unsigned_test.test | RUN | 0.182248 | 0.482023 | 0.861804 | 0.001684 | 0.001864 | 0.002144 |
| make_signed_test.test | RUN | 0.181423 | 0.483057 | 0.897540 | 0.001515 | 0.001632 | 0.001928 |
| is_volatile_test.test | RUN | 0.162112 | 0.415020 | 0.694865 | 0.001828 | 0.001832 | 0.001832 |
| is_void_test.test | RUN | 0.156399 | 0.378167 | 0.632949 | 0.002020 | 0.002923 | 0.002188 |
| is_virtual_base_of_test.test | RUN | 0.181581 | 0.497261 | 0.918187 | 0.001800 | 0.001969 | 0.002142 |
| is_unsigned_test.test | RUN | 0.162063 | 0.398515 | 0.656030 | 0.001543 | 0.001757 | 0.002158 |
| is_unscoped_enum_test.test | RUN | 0.157741 | 0.355878 | 0.571921 | 0.002316 | 0.001760 | 0.001980 |
| is_union_test.test | RUN | 0.168597 | 0.446761 | 0.793233 | 0.001904 | 0.002175 | 0.002162 |
| is_unbounded_array_test.test | RUN | 0.164490 | 0.379832 | 0.611617 | 0.001952 | 0.002115 | 0.002254 |
| is_trivially_copyable_test.test | RUN | 0.259368 | 0.800398 | 1.636731 | 0.001897 | 0.002256 | 0.002177 |
| is_swappable_test.test | RUN | 0.211615 | 0.594128 | 1.189976 | 0.001986 | 0.002242 | 0.002150 |
| is_stateless_test.test | RUN | 0.235275 | 0.683413 | 1.419364 | 0.001844 | 0.001894 | 0.002104 |
| is_signed_test.test | RUN | 0.159220 | 0.385041 | 0.714791 | 0.001868 | 0.001949 | 0.002700 |
| is_scoped_enum_test.test | RUN | 0.154006 | 0.356099 | 0.511775 | 0.001792 | 0.002174 | 0.002108 |
| is_scalar_test.test | RUN | 0.206158 | 0.647307 | 1.252900 | 0.002264 | 0.001665 | 0.001970 |
| is_same_test.test | RUN | 0.158022 | 0.389032 | 0.658300 | 0.002140 | 0.001984 | 0.002115 |
| is_rvalue_reference_test.test | RUN | 0.165576 | 0.439306 | 0.727971 | 0.001789 | 0.001741 | 0.002168 |
| is_reference_test.test | RUN | 0.168002 | 0.441704 | 0.746801 | 0.001812 | 0.001951 | 0.002230 |
| is_polymorphic_test.test | RUN | 0.171516 | 0.447605 | 0.814349 | 0.001800 | 0.002008 | 0.002229 |
| is_pointer_test.test | RUN | 0.167379 | 0.433798 | 0.765740 | 0.001996 | 0.001336 | 0.002030 |
| is_pod_test.test | RUN | 0.224752 | 0.723551 | 1.473256 | 0.001902 | 0.002134 | 0.002200 |
| is_object_test.test | RUN | 0.158065 | 0.379016 | 0.618929 | 0.001822 | 0.002012 | 0.001865 |
| is_nothrow_swappable_test.test | RUN | 0.221366 | 0.634683 | 1.341599 | 0.001901 | 0.001792 | 0.002074 |
| is_nothrow_move_constructible_test.test | RUN | 0.217228 | 0.666373 | 1.425998 | 0.002664 | 0.001742 | 0.001977 |
| is_nothrow_move_assignable_test.test | RUN | 0.218649 | 0.694801 | 1.434910 | 0.002044 | 0.001946 | 0.002010 |
| is_noncopyable_test.test | RUN | 0.153425 | 0.384716 | 0.586754 | 0.001902 | 0.001873 | 0.002010 |
| is_member_pointer_test.test | RUN | 0.157507 | 0.377753 | 0.622844 | 0.001935 | 0.001882 | 0.002070 |
| is_member_obj_test.test | RUN | 0.156692 | 0.384391 | 0.650464 | 0.002133 | 0.001764 | 0.002466 |
| is_member_func_test.test | RUN | 0.161240 | 0.409224 | 0.672349 | 0.001962 | 0.001902 | 0.002032 |
| is_lvalue_reference_test.test | RUN | 0.163161 | 0.436632 | 0.752137 | 0.001728 | 0.001946 | 0.002572 |
| is_list_constructible_test.test | RUN | 0.186544 | 0.494835 | 0.842263 | 0.002013 | 0.001928 | 0.002063 |
| is_integral_test.test | RUN | 0.199373 | 0.592273 | 1.196940 | 0.002426 | 0.001940 | 0.002047 |
| is_fundamental_test.test | RUN | 0.202692 | 0.616608 | 1.189410 | 0.002184 | 0.001899 | 0.001927 |
| is_function_test.test | RUN | 0.164710 | 0.410176 | 0.697753 | 0.002156 | 0.001842 | 0.001898 |
| is_floating_point_test.test | RUN | 0.178201 | 0.436129 | 0.764061 | 0.001921 | 0.001950 | 0.002174 |
| is_float_test.test | RUN | 0.163530 | 0.436115 | 0.779556 | 0.001840 | 0.002076 | 0.001839 |
| is_final_test.test | RUN | 0.159780 | 0.446715 | 0.784108 | 0.001916 | 0.002008 | 0.002140 |
| is_enum_test.test | RUN | 0.150537 | 0.374400 | 0.616160 | 0.001700 | 0.002024 | 0.001718 |
| is_empty_test.test | RUN | 0.163473 | 0.393659 | 0.689118 | 0.002012 | 0.002181 | 0.002138 |
| is_detected_test.test | RUN | 0.142805 | 0.338909 | 0.569891 | 0.002144 | 0.002098 | 0.001772 |
| is_detected_exact_test.test | RUN | 0.146347 | 0.333539 | 0.569125 | 0.001802 | 0.002077 | 0.001702 |
| is_destructible_test.test | RUN | 0.215053 | 0.681736 | 1.379991 | 0.001926 | 0.002145 | 0.002402 |
| is_default_constr_test.test | RUN | 0.212186 | 0.704637 | 1.403175 | 0.002116 | 0.002262 | 0.002242 |
| is_copy_constructible_test.test | RUN | 0.228414 | 0.737470 | 1.545420 | 0.001993 | 0.002235 | 0.002120 |
| is_copy_assignable_test.test | RUN | 0.219839 | 0.721618 | 1.528630 | 0.001674 | 0.002090 | 0.002180 |
| is_convertible_test.test | RUN | 0.223108 | 0.649829 | 1.237948 | 0.001776 | 0.001832 | 0.001720 |
| is_constructible_test.test | RUN | 0.176165 | 0.449601 | 0.891891 | 0.001814 | 0.002185 | 0.001910 |
| is_const_test.test | RUN | 0.156907 | 0.402946 | 0.717445 | 0.001969 | 0.001782 | 0.001902 |
| is_compound_test.test | RUN | 0.148163 | 0.343247 | 0.580715 | 0.001869 | 0.002113 | 0.001827 |
| is_complex_test.test | RUN | 0.214990 | 0.469169 | 0.817044 | 0.002148 | 0.001960 | 0.002104 |
| is_complete_test.test | RUN | 0.183522 | 0.486541 | 0.918499 | 0.002027 | 0.002195 | 0.001946 |
| is_class_test.test | RUN | 0.163003 | 0.440452 | 0.748678 | 0.001622 | 0.001869 | 0.001806 |
| is_bounded_array_test.test | RUN | 0.148681 | 0.381976 | 0.665679 | 0.001936 | 0.002044 | 0.001494 |
| is_base_of_test.test | RUN | 0.168132 | 0.415108 | 0.756131 | 0.002043 | 0.002094 | 0.001763 |
| is_base_and_derived_test.test | RUN | 0.164874 | 0.415460 | 0.739235 | 0.001936 | 0.001900 | 0.001708 |
| is_assignable_test.test | RUN | 0.197188 | 0.587011 | 1.135200 | 0.001867 | 0.001966 | 0.002112 |
| is_array_test.test | RUN | 0.153523 | 0.408241 | 0.728401 | 0.002164 | 0.002162 | 0.002130 |
| is_arithmetic_test.test | RUN | 0.196459 | 0.614120 | 1.200655 | 0.001879 | 0.002038 | 0.002088 |
| is_abstract_test.test | RUN | 0.365531 | 1.607885 | 3.876155 | 0.002145 | 0.001947 | 0.001854 |
| integral_constant_test.test | RUN | 0.025350 | 0.065966 | 0.065523 | 0.000896 | 0.000901 | 0.002047 |
| has_virtual_destructor_test.test | RUN | 0.168717 | 0.469282 | 0.843904 | 0.001904 | 0.002117 | 0.001945 |
| has_unary_plus_test.test | RUN | 0.345308 | 1.865402 | 3.001968 | 0.002218 | 0.002010 | 0.002650 |
| has_unary_minus_test.test | RUN | 0.335003 | 1.816426 | 2.841493 | 0.001880 | 0.002037 | 0.002097 |
| has_trivial_move_constructor_test.test | RUN | 0.225965 | 0.676789 | 1.476174 | 0.001904 | 0.001636 | 0.002160 |
| has_trivial_move_assign_test.test | RUN | 0.232779 | 0.637323 | 1.424837 | 0.001925 | 0.001775 | 0.002078 |
| has_trivial_destructor_test.test | RUN | 0.225502 | 0.641287 | 1.386575 | 0.002476 | 0.001786 | 0.002064 |
| has_trivial_copy_test.test | RUN | 0.236307 | 0.744752 | 1.522866 | 0.002868 | 0.001892 | 0.002292 |
| has_trivial_constr_test.test | RUN | 0.235908 | 0.704385 | 1.523542 | 0.002090 | 0.001966 | 0.002257 |
| has_trivial_assign_test.test | RUN | 0.239079 | 0.700032 | 1.455617 | 0.001948 | 0.001999 | 0.002287 |
| has_right_shift_test.test | RUN | 0.351524 | 1.380546 | 3.203316 | 0.002048 | 0.002206 | 0.003090 |
| has_right_shift_assign_test.test | RUN | 0.335837 | 1.283334 | 3.020542 | 0.001954 | 0.002372 | 0.002136 |
| has_prefix_classes3_test.test | RUN | 0.242520 | 0.771782 | 1.578605 | 0.001782 | 0.001931 | 0.002244 |
| has_prefix_classes2_test.test | RUN | 0.242070 | 0.741758 | 1.573687 | 0.001851 | 0.001957 | 0.002574 |
| has_prefix_classes1_test.test | RUN | 0.242513 | 0.784582 | 1.501848 | 0.002052 | 0.002090 | 0.002190 |
| has_prefix_classes0_test.test | RUN | 0.239674 | 0.764590 | 1.559629 | 0.001871 | 0.002023 | 0.002220 |
| has_pre_increment_test.test | RUN | 0.324867 | 1.851011 | 3.084253 | 0.002217 | 0.001909 | 0.002132 |
| has_pre_decrement_test.test | RUN | 0.320770 | 1.882478 | 2.902229 | 0.001880 | 0.002061 | 0.002234 |
| has_postfix_classes3_test.test | RUN | 0.247668 | 0.783689 | 1.529170 | 0.002009 | 0.001902 | 0.002062 |
| has_postfix_classes2_test.test | RUN | 0.261087 | 0.699829 | 1.546502 | 0.001927 | 0.001859 | 0.001846 |
| has_postfix_classes1_test.test | RUN | 0.251572 | 0.702022 | 1.506365 | 0.001608 | 0.002232 | 0.002353 |
| has_postfix_classes0_test.test | RUN | 0.254284 | 0.771945 | 1.549284 | 0.001620 | 0.002198 | 0.002238 |
| has_post_increment_test.test | RUN | 0.326936 | 1.852234 | 3.037712 | 0.001818 | 0.001915 | 0.002144 |
| has_post_decrement_test.test | RUN | 0.325586 | 1.886217 | 3.023806 | 0.001841 | 0.002134 | 0.002172 |
| has_plus_test.test | RUN | 0.327186 | 1.305840 | 3.083365 | 0.001708 | 0.001777 | 0.002237 |
| has_plus_assign_test.test | RUN | 0.338194 | 1.232366 | 3.086862 | 0.002192 | 0.001801 | 0.001920 |
| has_operator_new_test.test | RUN | 0.223894 | 0.659202 | 1.365711 | 0.002293 | 0.002082 | 0.001854 |
| has_nothrow_destructor_test.test | RUN | 0.234053 | 0.764593 | 1.524141 | 0.001800 | 0.001947 | 0.002458 |
| has_nothrow_copy_test.test | RUN | 0.228607 | 0.702077 | 1.492581 | 0.001908 | 0.002092 | 0.002624 |
| has_nothrow_constr_test.test | RUN | 0.214989 | 0.697730 | 1.404980 | 0.001848 | 0.002174 | 0.002259 |
| has_nothrow_assign_test.test | RUN | 0.219642 | 0.678819 | 1.426297 | 0.002138 | 0.002102 | 0.002259 |
| has_not_equal_to_test.test | RUN | 0.340208 | 1.757314 | 3.073695 | 0.001948 | 0.002220 | 0.003054 |
| has_negate_test.test | RUN | 0.320816 | 1.228053 | 2.933247 | 0.001957 | 0.001944 | 0.001918 |
| has_multiplies_test.test | RUN | 0.334985 | 1.334170 | 3.017837 | 0.001881 | 0.001898 | 0.002488 |
| has_multiplies_assign_test.test | RUN | 0.333644 | 1.307925 | 3.016279 | 0.001860 | 0.002108 | 0.002169 |
| has_modulus_test.test | RUN | 0.330534 | 1.324307 | 3.022108 | 0.001871 | 0.002064 | 0.002179 |
| has_modulus_assign_test.test | RUN | 0.328655 | 1.309292 | 3.047672 | 0.001899 | 0.002563 | 0.002059 |
| has_minus_test.test | RUN | 0.347946 | 1.329140 | 3.088460 | 0.001724 | 0.002134 | 0.002287 |
| has_minus_assign_test.test | RUN | 0.364745 | 1.349211 | 3.011951 | 0.002338 | 0.002116 | 0.002172 |
| has_logical_or_test.test | RUN | 0.349465 | 1.321608 | 3.000275 | 0.001949 | 0.001807 | 0.002215 |
| has_logical_not_test.test | RUN | 0.322691 | 1.294285 | 2.964075 | 0.001818 | 0.002233 | 0.002042 |
| has_logical_and_test.test | RUN | 0.345003 | 1.313462 | 3.037407 | 0.002236 | 0.002178 | 0.002515 |
| has_less_test.test | RUN | 0.344093 | 1.843394 | 3.051210 | 0.002008 | 0.002018 | 0.001996 |
| has_less_equal_test.test | RUN | 0.332825 | 1.759188 | 2.991463 | 0.001838 | 0.001861 | 0.002186 |
| has_left_shift_test.test | RUN | 0.377000 | 1.644494 | 3.635497 | 0.001965 | 0.001652 | 0.002095 |
| has_left_shift_assign_test.test | RUN | 0.342070 | 1.288213 | 3.034070 | 0.002140 | 0.001831 | 0.002004 |
| has_greater_test.test | RUN | 0.339405 | 1.768930 | 3.091684 | 0.002083 | 0.002068 | 0.001950 |
| has_greater_equal_test.test | RUN | 0.342561 | 1.748918 | 3.066039 | 0.001903 | 0.001976 | 0.002179 |
| has_equal_to_test.test | RUN | 0.341803 | 1.749963 | 3.019758 | 0.002087 | 0.001852 | 0.001992 |
| has_divides_test.test | RUN | 0.321274 | 1.263322 | 3.025564 | 0.001993 | 0.001738 | 0.001729 |
| has_divides_assign_test.test | RUN | 0.336664 | 1.244963 | 3.022662 | 0.002062 | 0.002144 | 0.002394 |
| has_dereference_test.test | RUN | 0.334019 | 1.247608 | 2.912675 | 0.001874 | 0.001899 | 0.002214 |
| has_complement_test.test | RUN | 0.327537 | 1.873901 | 3.023506 | 0.002226 | 0.002038 | 0.001981 |
| has_bit_xor_test.test | RUN | 0.323636 | 1.312838 | 3.028183 | 0.002010 | 0.002063 | 0.001824 |
| has_bit_xor_assign_test.test | RUN | 0.326641 | 1.293473 | 3.023386 | 0.002168 | 0.001758 | 0.001706 |
| has_bit_or_test.test | RUN | 0.334040 | 1.261629 | 3.003023 | 0.001908 | 0.002222 | 0.002256 |
| has_bit_or_assign_test.test | RUN | 0.334196 | 1.279685 | 2.970918 | 0.001880 | 0.002066 | 0.002095 |
| has_bit_and_test.test | RUN | 0.322052 | 1.258480 | 3.059488 | 0.001878 | 0.001996 | 0.001749 |
| has_bit_and_assign_test.test | RUN | 0.327475 | 1.257150 | 2.997955 | 0.002471 | 0.001857 | 0.001838 |
| has_binary_classes9_test.test | RUN | 0.372538 | 1.524124 | 3.343749 | 0.002292 | 0.002682 | 0.002202 |
| has_binary_classes8_test.test | RUN | 0.368552 | 1.563320 | 3.289780 | 0.002076 | 0.002089 | 0.001654 |
| has_binary_classes7_test.test | RUN | 0.374628 | 1.484895 | 3.380928 | 0.002658 | 0.001962 | 0.002006 |
| has_binary_classes6_test.test | RUN | 0.372912 | 1.501906 | 3.307859 | 0.001931 | 0.001879 | 0.002138 |
| has_binary_classes5_test.test | RUN | 0.371799 | 1.484623 | 3.314819 | 0.002032 | 0.002058 | 0.002370 |
| has_binary_classes4_test.test | RUN | 0.362582 | 1.489470 | 3.372668 | 0.002556 | 0.002127 | 0.001980 |
| has_binary_classes3_test.test | RUN | 0.361422 | 1.496179 | 3.262770 | 0.001954 | 0.001968 | 0.001864 |
| has_binary_classes2_test.test | RUN | 0.378542 | 1.600149 | 3.334736 | 0.002086 | 0.002166 | 0.001724 |
| has_binary_classes1_test.test | RUN | 0.364444 | 1.503237 | 3.229903 | 0.001558 | 0.001900 | 0.001808 |
| has_binary_classes0_test.test | RUN | 0.376114 | 1.544853 | 3.320349 | 0.001848 | 0.001846 | 0.002204 |
| function_traits_test.test | RUN | 0.175625 | 0.403605 | 0.723101 | 0.002076 | 0.002033 | 0.001917 |
| floating_point_promotion_test.test | RUN | 0.159423 | 0.373071 | 0.677423 | 0.002140 | 0.001802 | 0.002090 |
| extent_test.test | RUN | 0.157681 | 0.401536 | 0.634273 | 0.001854 | 0.001788 | 0.001869 |
| enable_if_test.test | RUN | 0.156644 | 0.328262 | 0.545015 | 0.001688 | 0.001569 | 0.002085 |
| disjunction_test.test | RUN | 0.141215 | 0.367234 | 0.574586 | 0.001883 | 0.001836 | 0.001934 |
| detected_test.test | RUN | 0.149476 | 0.314069 | 0.552785 | 0.002028 | 0.001966 | 0.001902 |
| detected_or_test.test | RUN | 0.150775 | 0.353415 | 0.622939 | 0.001907 | 0.001980 | 0.001854 |
| decay_test.test | RUN | 0.409520 | 1.484845 | 3.541909 | 0.001797 | 0.002126 | 0.002226 |
| cxx14_aliases_test.test | RUN | 0.254627 | 0.518883 | 0.868108 | 0.001931 | 0.002134 | 0.002152 |
| copy_reference_test.test | RUN | 0.154650 | 0.336883 | 0.594147 | 0.002070 | 0.002114 | 0.002106 |
| copy_cv_test.test | RUN | 0.159832 | 0.374369 | 0.627954 | 0.001858 | 0.001974 | 0.002314 |
| copy_cv_ref_test.test | RUN | 0.230290 | 0.640230 | 1.439092 | 0.001742 | 0.001932 | 0.002281 |
| conjunction_test.test | RUN | 0.149533 | 0.347811 | 0.608904 | 0.002140 | 0.001769 | 0.002257 |
| conditional_test.test | RUN | 0.156093 | 0.344283 | 0.583495 | 0.001553 | 0.001830 | 0.002439 |
| common_type_test.test | RUN | 0.204382 | 0.554563 | 1.134883 | 0.002006 | 0.001514 | 0.001806 |
| common_type_sfinae_test.test | RUN | 0.148986 | 0.325634 | 0.564764 | 0.001966 | 0.001826 | 0.001908 |
| common_type_sfinae2_test.test | RUN | 0.149904 | 0.343668 | 0.582821 | 0.001998 | 0.001896 | 0.002392 |
| common_type_6_test.test | RUN | 0.156619 | 0.370803 | 0.604144 | 0.001765 | 0.001890 | 0.002030 |
| common_type_5_test.test | RUN | 0.155814 | 0.356019 | 0.590764 | 0.002122 | 0.002192 | 0.001763 |
| common_type_4_test.test | RUN | 0.168917 | 0.392042 | 0.713549 | 0.002497 | 0.001982 | 0.002236 |
| common_type_3_test.test | RUN | 0.142295 | 0.327145 | 0.518530 | 0.002163 | 0.001931 | 0.002354 |
| common_type_2_test.test | RUN | 0.203607 | 0.541236 | 1.100562 | 0.001963 | 0.002247 | 0.002152 |
| alignment_of_test.test | RUN | 0.174342 | 0.429958 | 0.707120 | 0.002078 | 0.001904 | 0.002208 |
| alignment_of_a2_test.test | RUN | 0.164224 | 0.433132 | 0.755411 | 0.002065 | 0.002151 | 0.001592 |
| aligned_storage_test.test | RUN | 0.160790 | 0.405480 | 0.638394 | 0.001982 | 0.002006 | 0.002188 |
| aligned_storage_empy_test.test | RUN | 0.190933 | 0.478748 | 0.858818 | 0.002120 | 0.002120 | 0.001876 |
| aligned_storage_a2_test.test | RUN | 0.164671 | 0.401032 | 0.643853 | 0.002101 | 0.002251 | 0.002128 |
| add_volatile_test.test | RUN | 0.366142 | 1.333697 | 3.314558 | 0.001872 | 0.001876 | 0.002090 |
| add_rvalue_reference_test.test | RUN | 0.418822 | 1.451470 | 3.812272 | 0.001856 | 0.001892 | 0.002004 |
| add_reference_test.test | RUN | 0.396438 | 1.418200 | 3.803213 | 0.002038 | 0.002122 | 0.002289 |
| add_pointer_test.test | RUN | 0.259753 | 0.790556 | 1.949586 | 0.001983 | 0.001734 | 0.002256 |
| add_lvalue_reference_test.test | RUN | 0.396132 | 1.358425 | 3.741844 | 0.001888 | 0.001948 | 0.002039 |
| add_cv_test.test | RUN | 0.380050 | 1.293366 | 3.556044 | 0.001788 | 0.001787 | 0.002150 |
| add_const_test.test | RUN | 0.374665 | 1.394354 | 3.622738 | 0.001944 | 0.001452 | 0.002010 |
| is_virtual_base_of_fail.test | COMPILE_FAIL | 0.145756 | 0.326902 | 0.532155 | — | — | — |
| is_stateless_fail.test | COMPILE_FAIL | 0.141429 | 0.334293 | 0.545348 | — | — | — |
| is_polymorphic_fail.test | COMPILE_FAIL | 0.139963 | 0.312630 | 0.527464 | — | — | — |
| is_nothrow_move_constructible_fail.test | COMPILE_FAIL | 0.138774 | 0.309432 | 0.539462 | — | — | — |
| is_nothrow_move_assignable_fail.test | COMPILE_FAIL | 0.148949 | 0.308585 | 0.491995 | — | — | — |
| is_list_constructible_fail.test | COMPILE_FAIL | 0.146589 | 0.314815 | 0.507305 | — | — | — |
| is_empty_fail.test | COMPILE_FAIL | 0.143159 | 0.324166 | 0.502240 | — | — | — |
| is_destructible_fail.test | COMPILE_FAIL | 0.140251 | 0.307750 | 0.499888 | — | — | — |
| is_default_constructible_fail.test | COMPILE_FAIL | 0.139380 | 0.310214 | 0.518804 | — | — | — |
| is_copy_constructible_fail.test | COMPILE_FAIL | 0.147909 | 0.328374 | 0.509221 | — | — | — |
| is_copy_assignable_fail.test | COMPILE_FAIL | 0.139450 | 0.313764 | 0.516366 | — | — | — |
| is_convertible_fail.test | COMPILE_FAIL | 0.141947 | 0.308182 | 0.497988 | — | — | — |
| is_convertible2_fail.test | COMPILE_FAIL | 0.141184 | 0.307375 | 0.518398 | — | — | — |
| is_constructible_fail.test | COMPILE_FAIL | 0.142897 | 0.314175 | 0.501475 | — | — | — |
| is_base_of_fail.test | COMPILE_FAIL | 0.141347 | 0.319656 | 0.504970 | — | — | — |
| is_assignable_fail.test | COMPILE_FAIL | 0.141111 | 0.314237 | 0.512015 | — | — | — |
| has_nothrow_destruct_fail.test | COMPILE_FAIL | 0.146431 | 0.311769 | 0.510168 | — | — | — |
| has_nothrow_copy_fail.test | COMPILE_FAIL | 0.141464 | 0.322061 | 0.525974 | — | — | — |
| has_nothrow_cons_fail.test | COMPILE_FAIL | 0.139428 | 0.320461 | 0.536339 | — | — | — |
| has_nothrow_assign_fail.test | COMPILE_FAIL | 0.138493 | 0.315689 | 0.527107 | — | — | — |
| common_type_fail.test | COMPILE_FAIL | 0.139552 | 0.308343 | 0.524866 | — | — | — |
| common_type2_fail.test | COMPILE_FAIL | 0.142938 | 0.325967 | 0.518937 | — | — | — |
| has_nothrow_assign_test_no_intrinsics.test | RUN | 0.225415 | 0.700901 | 1.479400 | 0.002007 | 0.001995 | 0.001854 |
| has_nothrow_constr_test_no_intrinsics.test | RUN | 0.219552 | 0.652116 | 1.423380 | 0.001716 | 0.002079 | 0.002144 |
| has_nothrow_copy_test_no_intrinsics.test | RUN | 0.221123 | 0.657875 | 1.464918 | 0.001755 | 0.001687 | 0.001890 |
| is_nothrow_move_assignable_test_no_intrinsics.test | RUN | 0.228385 | 0.700385 | 1.437439 | 0.001282 | 0.001469 | 0.001402 |
| is_nothrow_move_constructible_test_no_intrinsics.test | RUN | 0.125112 | 0.436788 | 0.890437 | 0.001602 | 0.001548 | 0.001684 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.998× | 0.993×, 1.003× | 0.994× |
| clang | 1.029× | 1.025×, 1.032× | 1.001× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898266).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| add_const_test.test | 10 | 0.001546s | 0.001506s | 0.001518s | byte equality observed |
| add_cv_test.test | 10 | 0.001509s | 0.001499s | 0.001510s | byte equality observed |
| add_lvalue_reference_test.test | 8 | 0.001519s | 0.001551s | 0.001492s | byte equality observed |
| add_pointer_test.test | 10 | 0.001480s | 0.001508s | 0.001479s | byte equality observed |
| add_reference_test.test | 8 | 0.001521s | 0.001452s | 0.001495s | byte equality observed |
| add_rvalue_reference_test.test | 10 | 0.001529s | 0.001528s | 0.001506s | byte equality observed |
| add_volatile_test.test | 10 | 0.001492s | 0.001503s | 0.001540s | byte equality observed |
| aligned_storage_a2_test.test | 10 | 0.001654s | 0.001646s | 0.001663s | byte equality observed |
| aligned_storage_empy_test.test | 10 | 0.001504s | 0.001502s | 0.001475s | byte equality observed |
| aligned_storage_test.test | 10 | 0.001648s | 0.001637s | 0.001618s | byte equality observed |
| alignment_of_a2_test.test | 10 | 0.001510s | 0.001536s | 0.001502s | byte equality observed |
| alignment_of_test.test | 10 | 0.001496s | 0.001506s | 0.001508s | byte equality observed |
| common_type_2_test.test | 10 | 0.001495s | 0.001502s | 0.001504s | byte equality observed |
| common_type_3_test.test | 10 | 0.001513s | 0.001512s | 0.001493s | byte equality observed |
| common_type_4_test.test | 10 | 0.001513s | 0.001509s | 0.001512s | byte equality observed |
| common_type_5_test.test | 10 | 0.001494s | 0.001487s | 0.001508s | byte equality observed |
| common_type_6_test.test | 10 | 0.001499s | 0.001457s | 0.001502s | byte equality observed |
| common_type_sfinae2_test.test | 10 | 0.001510s | 0.001513s | 0.001473s | byte equality observed |
| common_type_sfinae_test.test | 10 | 0.001499s | 0.001511s | 0.001504s | byte equality observed |
| common_type_test.test | 10 | 0.001514s | 0.001516s | 0.001515s | byte equality observed |
| conditional_test.test | 10 | 0.001492s | 0.001530s | 0.001492s | byte equality observed |
| conjunction_test.test | 10 | 0.001512s | 0.001460s | 0.001501s | byte equality observed |
| copy_cv_ref_test.test | 10 | 0.001512s | 0.001517s | 0.001504s | byte equality observed |
| copy_cv_test.test | 10 | 0.001505s | 0.001522s | 0.001464s | byte equality observed |
| copy_reference_test.test | 10 | 0.001511s | 0.001475s | 0.001524s | byte equality observed |
| cxx14_aliases_test.test | 10 | 0.001494s | 0.001530s | 0.001474s | byte equality observed |
| decay_test.test | 10 | 0.001513s | 0.001516s | 0.001542s | byte equality observed |
| detected_or_test.test | 10 | 0.001486s | 0.001518s | 0.001513s | byte equality observed |
| detected_test.test | 10 | 0.001510s | 0.001494s | 0.001492s | byte equality observed |
| disjunction_test.test | 10 | 0.001518s | 0.001508s | 0.001494s | byte equality observed |
| enable_if_test.test | 10 | 0.001500s | 0.001518s | 0.001477s | byte equality observed |
| extent_test.test | 10 | 0.001485s | 0.001516s | 0.001512s | byte equality observed |
| floating_point_promotion_test.test | 10 | 0.001501s | 0.001553s | 0.001488s | byte equality observed |
| function_traits_test.test | 8 | 0.001526s | 0.001515s | 0.001517s | byte equality observed |
| has_binary_classes0_test.test | 10 | 0.001487s | 0.001548s | 0.001518s | byte equality observed |
| has_binary_classes1_test.test | 10 | 0.001534s | 0.001504s | 0.001502s | byte equality observed |
| has_binary_classes2_test.test | 10 | 0.001511s | 0.001510s | 0.001471s | byte equality observed |
| has_binary_classes3_test.test | 10 | 0.001498s | 0.001507s | 0.001490s | byte equality observed |
| has_binary_classes4_test.test | 10 | 0.001512s | 0.001504s | 0.001537s | byte equality observed |
| has_binary_classes5_test.test | 10 | 0.001495s | 0.001512s | 0.001488s | byte equality observed |
| has_binary_classes6_test.test | 10 | 0.001499s | 0.001497s | 0.001506s | byte equality observed |
| has_binary_classes7_test.test | 10 | 0.001505s | 0.001492s | 0.001485s | byte equality observed |
| has_binary_classes8_test.test | 9 | 0.001500s | 0.001535s | 0.001473s | byte equality observed |
| has_binary_classes9_test.test | 10 | 0.001528s | 0.001479s | 0.001483s | byte equality observed |
| has_bit_and_assign_test.test | 10 | 0.001496s | 0.001525s | 0.001486s | byte equality observed |
| has_bit_and_test.test | 10 | 0.001496s | 0.001520s | 0.001520s | byte equality observed |
| has_bit_or_assign_test.test | 10 | 0.001505s | 0.001565s | 0.001475s | byte equality observed |
| has_bit_or_test.test | 10 | 0.001515s | 0.001507s | 0.001539s | byte equality observed |
| has_bit_xor_assign_test.test | 10 | 0.001512s | 0.001502s | 0.001496s | byte equality observed |
| has_bit_xor_test.test | 10 | 0.001539s | 0.001506s | 0.001497s | byte equality observed |
| has_complement_test.test | 10 | 0.001503s | 0.001503s | 0.001500s | byte equality observed |
| has_dereference_test.test | 10 | 0.001511s | 0.001475s | 0.001512s | byte equality observed |
| has_divides_assign_test.test | 9 | 0.001498s | 0.001497s | 0.001453s | byte equality observed |
| has_divides_test.test | 7 | 0.001497s | 0.001484s | 0.001502s | byte equality observed |
| has_equal_to_test.test | 9 | 0.001499s | 0.001513s | 0.001485s | byte equality observed |
| has_greater_equal_test.test | 10 | 0.001492s | 0.001506s | 0.001484s | byte equality observed |
| has_greater_test.test | 10 | 0.001512s | 0.001500s | 0.001479s | byte equality observed |
| has_left_shift_assign_test.test | 10 | 0.001484s | 0.001508s | 0.001491s | byte equality observed |
| has_left_shift_test.test | 10 | 0.001498s | 0.001518s | 0.001510s | byte equality observed |
| has_less_equal_test.test | 8 | 0.001527s | 0.001511s | 0.001515s | byte equality observed |
| has_less_test.test | 10 | 0.001500s | 0.001510s | 0.001487s | byte equality observed |
| has_logical_and_test.test | 10 | 0.001515s | 0.001516s | 0.001477s | byte equality observed |
| has_logical_not_test.test | 8 | 0.001524s | 0.001515s | 0.001525s | byte equality observed |
| has_logical_or_test.test | 10 | 0.001493s | 0.001491s | 0.001477s | byte equality observed |
| has_minus_assign_test.test | 10 | 0.001494s | 0.001523s | 0.001477s | byte equality observed |
| has_minus_test.test | 10 | 0.001525s | 0.001484s | 0.001491s | byte equality observed |
| has_modulus_assign_test.test | 10 | 0.001499s | 0.001522s | 0.001509s | byte equality observed |
| has_modulus_test.test | 10 | 0.001492s | 0.001468s | 0.001530s | byte equality observed |
| has_multiplies_assign_test.test | 10 | 0.001488s | 0.001527s | 0.001515s | byte equality observed |
| has_multiplies_test.test | 10 | 0.001516s | 0.001509s | 0.001547s | byte equality observed |
| has_negate_test.test | 10 | 0.001503s | 0.001517s | 0.001498s | byte equality observed |
| has_not_equal_to_test.test | 10 | 0.001497s | 0.001496s | 0.001522s | byte equality observed |
| has_nothrow_assign_test.test | 10 | 0.001497s | 0.001511s | 0.001492s | byte equality observed |
| has_nothrow_assign_test_no_intrinsics.test | 10 | 0.001513s | 0.001512s | 0.001493s | byte equality observed |
| has_nothrow_constr_test.test | 10 | 0.001514s | 0.001505s | 0.001492s | byte equality observed |
| has_nothrow_constr_test_no_intrinsics.test | 10 | 0.001503s | 0.001523s | 0.001507s | byte equality observed |
| has_nothrow_copy_test.test | 10 | 0.001468s | 0.001492s | 0.001501s | byte equality observed |
| has_nothrow_copy_test_no_intrinsics.test | 8 | 0.001495s | 0.001492s | 0.001505s | byte equality observed |
| has_nothrow_destructor_test.test | 10 | 0.001509s | 0.001496s | 0.001483s | byte equality observed |
| has_operator_new_test.test | 10 | 0.001499s | 0.001514s | 0.001485s | byte equality observed |
| has_plus_assign_test.test | 10 | 0.001503s | 0.001497s | 0.001522s | byte equality observed |
| has_plus_test.test | 9 | 0.001498s | 0.001482s | 0.001520s | byte equality observed |
| has_post_decrement_test.test | 10 | 0.001531s | 0.001521s | 0.001485s | byte equality observed |
| has_post_increment_test.test | 10 | 0.001506s | 0.001500s | 0.001524s | byte equality observed |
| has_postfix_classes0_test.test | 10 | 0.001513s | 0.001500s | 0.001520s | byte equality observed |
| has_postfix_classes1_test.test | 10 | 0.001500s | 0.001483s | 0.001517s | byte equality observed |
| has_postfix_classes2_test.test | 9 | 0.001491s | 0.001521s | 0.001504s | byte equality observed |
| has_postfix_classes3_test.test | 10 | 0.001497s | 0.001533s | 0.001494s | byte equality observed |
| has_pre_decrement_test.test | 10 | 0.001482s | 0.001486s | 0.001504s | byte equality observed |
| has_pre_increment_test.test | 10 | 0.001519s | 0.001493s | 0.001471s | byte equality observed |
| has_prefix_classes0_test.test | 10 | 0.001491s | 0.001489s | 0.001471s | byte equality observed |
| has_prefix_classes1_test.test | 10 | 0.001489s | 0.001455s | 0.001492s | byte equality observed |
| has_prefix_classes2_test.test | 10 | 0.001514s | 0.001494s | 0.001496s | byte equality observed |
| has_prefix_classes3_test.test | 10 | 0.001501s | 0.001532s | 0.001494s | byte equality observed |
| has_right_shift_assign_test.test | 10 | 0.001508s | 0.001492s | 0.001491s | byte equality observed |
| has_right_shift_test.test | 10 | 0.001511s | 0.001521s | 0.001496s | byte equality observed |
| has_trivial_assign_test.test | 10 | 0.001587s | 0.001583s | 0.001576s | byte equality observed |
| has_trivial_constr_test.test | 10 | 0.001546s | 0.001590s | 0.001575s | native test oracle; output varies |
| has_trivial_copy_test.test | 10 | 0.001577s | 0.001588s | 0.001616s | native test oracle; output varies |
| has_trivial_destructor_test.test | 10 | 0.001496s | 0.001506s | 0.001514s | byte equality observed |
| has_trivial_move_assign_test.test | 10 | 0.001523s | 0.001516s | 0.001508s | byte equality observed |
| has_trivial_move_constructor_test.test | 10 | 0.001514s | 0.001510s | 0.001494s | byte equality observed |
| has_unary_minus_test.test | 10 | 0.001485s | 0.001507s | 0.001469s | byte equality observed |
| has_unary_plus_test.test | 10 | 0.001508s | 0.001504s | 0.001499s | byte equality observed |
| has_virtual_destructor_test.test | 10 | 0.001510s | 0.001513s | 0.001504s | byte equality observed |
| integral_constant_test.test | 10 | 0.000675s | 0.000698s | 0.001522s | byte equality observed |
| is_abstract_test.test | 9 | 0.001521s | 0.001519s | 0.001504s | byte equality observed |
| is_arithmetic_test.test | 10 | 0.001498s | 0.001531s | 0.001509s | byte equality observed |
| is_array_test.test | 10 | 0.001515s | 0.001507s | 0.001509s | byte equality observed |
| is_assignable_test.test | 10 | 0.001508s | 0.001511s | 0.001489s | byte equality observed |
| is_base_and_derived_test.test | 10 | 0.001500s | 0.001536s | 0.001499s | byte equality observed |
| is_base_of_test.test | 10 | 0.001509s | 0.001541s | 0.001473s | byte equality observed |
| is_bounded_array_test.test | 10 | 0.001494s | 0.001464s | 0.001464s | byte equality observed |
| is_class_test.test | 10 | 0.001527s | 0.001503s | 0.001489s | byte equality observed |
| is_complete_test.test | 10 | 0.001499s | 0.001532s | 0.001477s | byte equality observed |
| is_complex_test.test | 10 | 0.001506s | 0.001498s | 0.001494s | byte equality observed |
| is_compound_test.test | 10 | 0.001478s | 0.001507s | 0.001500s | byte equality observed |
| is_const_test.test | 10 | 0.001496s | 0.001508s | 0.001491s | byte equality observed |
| is_constructible_test.test | 10 | 0.001489s | 0.001479s | 0.001495s | byte equality observed |
| is_convertible_test.test | 10 | 0.001486s | 0.001524s | 0.001496s | byte equality observed |
| is_copy_assignable_test.test | 10 | 0.001494s | 0.001491s | 0.001492s | byte equality observed |
| is_copy_constructible_test.test | 10 | 0.001472s | 0.001522s | 0.001462s | byte equality observed |
| is_default_constr_test.test | 10 | 0.001504s | 0.001486s | 0.001501s | byte equality observed |
| is_destructible_test.test | 10 | 0.001504s | 0.001508s | 0.001486s | byte equality observed |
| is_detected_exact_test.test | 10 | 0.001509s | 0.001505s | 0.001461s | byte equality observed |
| is_detected_test.test | 10 | 0.001505s | 0.001511s | 0.001484s | byte equality observed |
| is_empty_test.test | 10 | 0.001499s | 0.001498s | 0.001506s | byte equality observed |
| is_enum_test.test | 10 | 0.001463s | 0.001467s | 0.001504s | byte equality observed |
| is_final_test.test | 10 | 0.001485s | 0.001506s | 0.001455s | byte equality observed |
| is_float_test.test | 10 | 0.001499s | 0.001501s | 0.001484s | byte equality observed |
| is_floating_point_test.test | 10 | 0.001509s | 0.001512s | 0.001485s | byte equality observed |
| is_function_test.test | 10 | 0.001511s | 0.001533s | 0.001476s | byte equality observed |
| is_fundamental_test.test | 10 | 0.001496s | 0.001544s | 0.001486s | byte equality observed |
| is_integral_test.test | 10 | 0.001502s | 0.001488s | 0.001506s | byte equality observed |
| is_list_constructible_test.test | 10 | 0.001502s | 0.001522s | 0.001492s | byte equality observed |
| is_lvalue_reference_test.test | 10 | 0.001514s | 0.001510s | 0.001498s | byte equality observed |
| is_member_func_test.test | 10 | 0.001499s | 0.001540s | 0.001499s | byte equality observed |
| is_member_obj_test.test | 10 | 0.001481s | 0.001500s | 0.001478s | byte equality observed |
| is_member_pointer_test.test | 10 | 0.001510s | 0.001542s | 0.001482s | byte equality observed |
| is_noncopyable_test.test | 10 | 0.001503s | 0.001485s | 0.001490s | byte equality observed |
| is_nothrow_move_assignable_test.test | 10 | 0.001485s | 0.001499s | 0.001471s | byte equality observed |
| is_nothrow_move_assignable_test_no_intrinsics.test | 10 | 0.001508s | 0.001520s | 0.001499s | byte equality observed |
| is_nothrow_move_constructible_test.test | 10 | 0.001524s | 0.001513s | 0.001494s | byte equality observed |
| is_nothrow_move_constructible_test_no_intrinsics.test | 10 | 0.001519s | 0.001526s | 0.001482s | byte equality observed |
| is_nothrow_swappable_test.test | 10 | 0.001491s | 0.001481s | 0.001470s | byte equality observed |
| is_object_test.test | 10 | 0.001497s | 0.001495s | 0.001469s | byte equality observed |
| is_pod_test.test | 10 | 0.001490s | 0.001519s | 0.001512s | byte equality observed |
| is_pointer_test.test | 10 | 0.001470s | 0.001494s | 0.001465s | byte equality observed |
| is_polymorphic_test.test | 10 | 0.001512s | 0.001505s | 0.001486s | byte equality observed |
| is_reference_test.test | 10 | 0.001494s | 0.001546s | 0.001518s | byte equality observed |
| is_rvalue_reference_test.test | 10 | 0.001497s | 0.001512s | 0.001461s | byte equality observed |
| is_same_test.test | 8 | 0.001467s | 0.001471s | 0.001491s | byte equality observed |
| is_scalar_test.test | 10 | 0.001496s | 0.001498s | 0.001499s | byte equality observed |
| is_scoped_enum_test.test | 10 | 0.001467s | 0.001493s | 0.001475s | byte equality observed |
| is_signed_test.test | 10 | 0.001514s | 0.001498s | 0.001514s | byte equality observed |
| is_stateless_test.test | 10 | 0.001485s | 0.001509s | 0.001514s | byte equality observed |
| is_swappable_test.test | 10 | 0.001514s | 0.001543s | 0.001505s | byte equality observed |
| is_trivially_copyable_test.test | 9 | 0.001552s | 0.001533s | 0.001499s | byte equality observed |
| is_unbounded_array_test.test | 10 | 0.001531s | 0.001523s | 0.001505s | byte equality observed |
| is_union_test.test | 10 | 0.001489s | 0.001497s | 0.001457s | byte equality observed |
| is_unscoped_enum_test.test | 10 | 0.001492s | 0.001520s | 0.001495s | byte equality observed |
| is_unsigned_test.test | 10 | 0.001509s | 0.001464s | 0.001474s | byte equality observed |
| is_virtual_base_of_test.test | 10 | 0.001483s | 0.001486s | 0.001479s | byte equality observed |
| is_void_test.test | 9 | 0.001513s | 0.001514s | 0.001493s | byte equality observed |
| is_volatile_test.test | 8 | 0.001499s | 0.001491s | 0.001493s | byte equality observed |
| make_signed_test.test | 9 | 0.001465s | 0.001482s | 0.001478s | byte equality observed |
| make_unsigned_test.test | 10 | 0.001509s | 0.001537s | 0.001482s | byte equality observed |
| make_void_test.test | 10 | 0.001509s | 0.001534s | 0.001490s | byte equality observed |
| mpl_interop_test1.test | 10 | 0.000673s | 0.000678s | 0.001497s | byte equality observed |
| mpl_interop_test2.test | 10 | 0.000683s | 0.000685s | 0.001517s | byte equality observed |
| mpl_interop_test3.test | 10 | 0.000688s | 0.000690s | 0.001508s | byte equality observed |
| negation_test.test | 8 | 0.001527s | 0.001561s | 0.001476s | byte equality observed |
| promote_basic_test.test | 10 | 0.000677s | 0.000685s | 0.001494s | byte equality observed |
| promote_enum_msvc_bug_test.test | 10 | 0.000681s | 0.000704s | 0.001514s | byte equality observed |
| promote_enum_test.test | 10 | 0.000682s | 0.000684s | 0.001525s | byte equality observed |
| promote_mpl_test.test | 10 | 0.000682s | 0.000683s | 0.001488s | byte equality observed |
| rank_test.test | 10 | 0.001501s | 0.001542s | 0.001500s | byte equality observed |
| remove_all_extents_test.test | 10 | 0.001519s | 0.001556s | 0.001504s | byte equality observed |
| remove_bounds_test.test | 10 | 0.001500s | 0.001503s | 0.001487s | byte equality observed |
| remove_const_test.test | 10 | 0.001488s | 0.001526s | 0.001513s | byte equality observed |
| remove_cv_ref_test.test | 9 | 0.001495s | 0.001504s | 0.001495s | byte equality observed |
| remove_cv_test.test | 10 | 0.001501s | 0.001512s | 0.001505s | byte equality observed |
| remove_extent_test.test | 10 | 0.001520s | 0.001562s | 0.001490s | byte equality observed |
| remove_pointer_test.test | 10 | 0.001514s | 0.001507s | 0.001500s | byte equality observed |
| remove_reference_test.test | 10 | 0.001483s | 0.001479s | 0.001498s | byte equality observed |
| remove_volatile_test.test | 10 | 0.001505s | 0.001534s | 0.001512s | byte equality observed |
| tricky_abstract_type_test.test | 10 | 0.001517s | 0.001498s | 0.001486s | byte equality observed |
| tricky_add_pointer_test.test | 10 | 0.001508s | 0.001511s | 0.001517s | byte equality observed |
| tricky_function_type_test.test | 10 | 0.001518s | 0.001567s | 0.001460s | byte equality observed |
| tricky_incomplete_type_test.test | 10 | 0.001483s | 0.001516s | 0.001500s | byte equality observed |
| tricky_is_enum_test.test | 10 | 0.001501s | 0.001485s | 0.001505s | byte equality observed |
| tricky_partial_spec_test.test | 10 | 0.001504s | 0.001516s | 0.001508s | byte equality observed |
| tricky_rvalue_test.test | 10 | 0.001471s | 0.001528s | 0.001478s | byte equality observed |
| type_identity_test.test | 10 | 0.001509s | 0.001516s | 0.001484s | byte equality observed |
| type_traits_test.test | 10 | 0.000664s | 0.000678s | 0.001519s | byte equality observed |
| type_with_alignment_test.test | 10 | 0.001676s | 0.001690s | 0.001676s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898266).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
