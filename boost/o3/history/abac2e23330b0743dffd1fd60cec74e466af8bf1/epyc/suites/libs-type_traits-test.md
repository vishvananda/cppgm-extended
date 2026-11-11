# libs/type_traits/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| type_with_alignment_test.test | RUN | 0.184680 | 0.458528 | 0.798454 | 0.002326 | 0.002088 | 0.002404 |
| type_traits_test.test | RUN | 0.257958 | 0.488653 | 0.799318 | 0.000762 | 0.000941 | 0.001842 |
| type_identity_test.test | RUN | 0.466377 | 1.779976 | 4.612123 | 0.002086 | 0.002034 | 0.002248 |
| tricky_rvalue_test.test | RUN | 0.173002 | 0.371271 | 0.610921 | 0.001693 | 0.001869 | 0.002017 |
| tricky_partial_spec_test.test | RUN | 0.208119 | 0.552347 | 1.019413 | 0.001914 | 0.002250 | 0.002059 |
| tricky_is_enum_test.test | RUN | 0.161241 | 0.341405 | 0.575071 | 0.001984 | 0.001840 | 0.002207 |
| tricky_incomplete_type_test.test | RUN | 0.275092 | 0.544851 | 0.972447 | 0.002111 | 0.002179 | 0.002063 |
| tricky_function_type_test.test | RUN | 0.223764 | 0.589155 | 1.121335 | 0.002118 | 0.001768 | 0.002219 |
| tricky_add_pointer_test.test | RUN | 0.313820 | 1.046945 | 2.564081 | 0.002107 | 0.002206 | 0.002206 |
| tricky_abstract_type_test.test | RUN | 0.176520 | 0.338143 | 0.591791 | 0.001920 | 0.001898 | 0.002116 |
| remove_volatile_test.test | RUN | 0.415783 | 1.476850 | 4.008943 | 0.002109 | 0.002248 | 0.002168 |
| remove_reference_test.test | RUN | 0.442914 | 1.538109 | 4.175947 | 0.002098 | 0.002081 | 0.002188 |
| remove_pointer_test.test | RUN | 0.460722 | 1.649853 | 4.412234 | 0.002018 | 0.002236 | 0.002226 |
| remove_extent_test.test | RUN | 0.438933 | 1.591325 | 4.286371 | 0.001836 | 0.002235 | 0.002194 |
| remove_cv_test.test | RUN | 0.441868 | 1.592052 | 4.291064 | 0.002034 | 0.002214 | 0.002242 |
| remove_cv_ref_test.test | RUN | 0.904919 | 3.820038 | 10.471599 | 0.002048 | 0.001784 | 0.001746 |
| remove_const_test.test | RUN | 0.420832 | 1.468745 | 4.050728 | 0.002196 | 0.002187 | 0.002184 |
| remove_bounds_test.test | RUN | 0.439839 | 1.599368 | 4.241325 | 0.002612 | 0.001818 | 0.002188 |
| remove_all_extents_test.test | RUN | 0.443772 | 1.604280 | 4.231341 | 0.001858 | 0.001830 | 0.001669 |
| rank_test.test | RUN | 0.169409 | 0.373742 | 0.554690 | 0.001925 | 0.002246 | 0.002071 |
| promote_mpl_test.test | RUN | 0.155194 | 0.320347 | 0.391727 | 0.000798 | 0.000958 | 0.002164 |
| promote_enum_test.test | RUN | 0.033064 | 0.082381 | 0.092490 | 0.000798 | 0.000922 | 0.001824 |
| promote_enum_msvc_bug_test.test | RUN | 0.028587 | 0.071742 | 0.067198 | 0.000779 | 0.000967 | 0.001932 |
| promote_basic_test.test | RUN | 0.038838 | 0.088394 | 0.097443 | 0.000867 | 0.000701 | 0.002222 |
| negation_test.test | RUN | 0.165499 | 0.327091 | 0.505164 | 0.001880 | 0.001807 | 0.002102 |
| mpl_interop_test3.test | RUN | 0.150604 | 0.292681 | 0.362805 | 0.000760 | 0.000756 | 0.001959 |
| mpl_interop_test2.test | RUN | 0.058333 | 0.084117 | 0.092055 | 0.000798 | 0.000807 | 0.001942 |
| mpl_interop_test1.test | RUN | 0.026525 | 0.064815 | 0.067895 | 0.000760 | 0.000937 | 0.001766 |
| make_void_test.test | RUN | 0.172692 | 0.369353 | 0.626625 | 0.002116 | 0.002014 | 0.002195 |
| make_unsigned_test.test | RUN | 0.200958 | 0.499122 | 0.868301 | 0.001927 | 0.002162 | 0.002274 |
| make_signed_test.test | RUN | 0.205926 | 0.490340 | 0.886496 | 0.001794 | 0.001838 | 0.001883 |
| is_volatile_test.test | RUN | 0.180270 | 0.419656 | 0.703423 | 0.001969 | 0.001991 | 0.001898 |
| is_void_test.test | RUN | 0.175178 | 0.384974 | 0.679276 | 0.002012 | 0.002176 | 0.002179 |
| is_virtual_base_of_test.test | RUN | 0.204052 | 0.501097 | 0.915200 | 0.001866 | 0.001821 | 0.002082 |
| is_unsigned_test.test | RUN | 0.181270 | 0.392789 | 0.682942 | 0.002036 | 0.002039 | 0.002128 |
| is_unscoped_enum_test.test | RUN | 0.174691 | 0.369460 | 0.593859 | 0.001756 | 0.001926 | 0.002016 |
| is_union_test.test | RUN | 0.190273 | 0.457634 | 0.767289 | 0.001878 | 0.002064 | 0.002174 |
| is_unbounded_array_test.test | RUN | 0.171324 | 0.381474 | 0.613877 | 0.001855 | 0.002208 | 0.002174 |
| is_trivially_copyable_test.test | RUN | 0.288706 | 0.823149 | 1.642887 | 0.002017 | 0.002043 | 0.002236 |
| is_swappable_test.test | RUN | 0.227665 | 0.631218 | 1.219401 | 0.001769 | 0.002147 | 0.001723 |
| is_stateless_test.test | RUN | 0.271205 | 0.723551 | 1.471834 | 0.001672 | 0.001944 | 0.002174 |
| is_signed_test.test | RUN | 0.182491 | 0.422684 | 0.713607 | 0.002013 | 0.001870 | 0.002178 |
| is_scoped_enum_test.test | RUN | 0.182094 | 0.362573 | 0.521747 | 0.001883 | 0.002287 | 0.002206 |
| is_scalar_test.test | RUN | 0.229644 | 0.660349 | 1.298298 | 0.002027 | 0.001854 | 0.001936 |
| is_same_test.test | RUN | 0.174494 | 0.384619 | 0.606541 | 0.002057 | 0.002119 | 0.001930 |
| is_rvalue_reference_test.test | RUN | 0.185659 | 0.437901 | 0.764740 | 0.002004 | 0.001950 | 0.001734 |
| is_reference_test.test | RUN | 0.186553 | 0.420797 | 0.731638 | 0.001869 | 0.002028 | 0.002216 |
| is_polymorphic_test.test | RUN | 0.191820 | 0.463597 | 0.820654 | 0.002062 | 0.002150 | 0.002163 |
| is_pointer_test.test | RUN | 0.183018 | 0.448699 | 0.819067 | 0.002153 | 0.001874 | 0.002128 |
| is_pod_test.test | RUN | 0.245113 | 0.743610 | 1.501717 | 0.001976 | 0.002248 | 0.002168 |
| is_object_test.test | RUN | 0.164176 | 0.388159 | 0.569152 | 0.001832 | 0.002035 | 0.002011 |
| is_nothrow_swappable_test.test | RUN | 0.252876 | 0.683971 | 1.367974 | 0.001954 | 0.001514 | 0.002149 |
| is_nothrow_move_constructible_test.test | RUN | 0.247216 | 0.715525 | 1.423287 | 0.001974 | 0.001740 | 0.001758 |
| is_nothrow_move_assignable_test.test | RUN | 0.250589 | 0.731533 | 1.415355 | 0.001928 | 0.002230 | 0.001964 |
| is_noncopyable_test.test | RUN | 0.175741 | 0.379290 | 0.649466 | 0.001802 | 0.001986 | 0.001997 |
| is_member_pointer_test.test | RUN | 0.177754 | 0.392214 | 0.618703 | 0.001762 | 0.001996 | 0.002164 |
| is_member_obj_test.test | RUN | 0.177086 | 0.387079 | 0.671799 | 0.001934 | 0.002000 | 0.001906 |
| is_member_func_test.test | RUN | 0.182871 | 0.429819 | 0.691929 | 0.002064 | 0.001873 | 0.001772 |
| is_lvalue_reference_test.test | RUN | 0.183945 | 0.440359 | 0.765978 | 0.001613 | 0.002118 | 0.002102 |
| is_list_constructible_test.test | RUN | 0.213862 | 0.502689 | 0.843364 | 0.002038 | 0.001879 | 0.002186 |
| is_integral_test.test | RUN | 0.225078 | 0.617152 | 1.195561 | 0.001840 | 0.001942 | 0.002248 |
| is_fundamental_test.test | RUN | 0.228048 | 0.638687 | 1.224203 | 0.001988 | 0.001930 | 0.001931 |
| is_function_test.test | RUN | 0.183103 | 0.401802 | 0.711992 | 0.001753 | 0.001846 | 0.002191 |
| is_floating_point_test.test | RUN | 0.181900 | 0.445933 | 0.733913 | 0.002078 | 0.001882 | 0.002166 |
| is_float_test.test | RUN | 0.187074 | 0.439646 | 0.710299 | 0.002001 | 0.002108 | 0.002244 |
| is_final_test.test | RUN | 0.188863 | 0.456633 | 0.819452 | 0.001756 | 0.001992 | 0.002109 |
| is_enum_test.test | RUN | 0.174737 | 0.384574 | 0.588947 | 0.001830 | 0.002023 | 0.002174 |
| is_empty_test.test | RUN | 0.184170 | 0.407218 | 0.709093 | 0.001768 | 0.002291 | 0.001936 |
| is_detected_test.test | RUN | 0.165648 | 0.348406 | 0.489001 | 0.001906 | 0.001872 | 0.001765 |
| is_detected_exact_test.test | RUN | 0.168081 | 0.353105 | 0.556863 | 0.001826 | 0.001846 | 0.002250 |
| is_destructible_test.test | RUN | 0.245530 | 0.713704 | 1.424132 | 0.002024 | 0.002282 | 0.002190 |
| is_default_constr_test.test | RUN | 0.248986 | 0.720447 | 1.447337 | 0.002120 | 0.002011 | 0.002101 |
| is_copy_constructible_test.test | RUN | 0.260173 | 0.747091 | 1.488757 | 0.001908 | 0.001826 | 0.002133 |
| is_copy_assignable_test.test | RUN | 0.255954 | 0.730344 | 1.537578 | 0.001824 | 0.002012 | 0.002249 |
| is_convertible_test.test | RUN | 0.246177 | 0.654996 | 1.250226 | 0.002078 | 0.002228 | 0.001962 |
| is_constructible_test.test | RUN | 0.205057 | 0.482188 | 0.872000 | 0.001934 | 0.002096 | 0.001876 |
| is_const_test.test | RUN | 0.180801 | 0.404593 | 0.678442 | 0.001911 | 0.002141 | 0.002181 |
| is_compound_test.test | RUN | 0.169075 | 0.372379 | 0.559417 | 0.001856 | 0.001828 | 0.002134 |
| is_complex_test.test | RUN | 0.221968 | 0.473474 | 0.831909 | 0.002066 | 0.002308 | 0.001909 |
| is_complete_test.test | RUN | 0.197565 | 0.484412 | 0.939222 | 0.001900 | 0.001836 | 0.001852 |
| is_class_test.test | RUN | 0.185350 | 0.429271 | 0.762573 | 0.001720 | 0.001866 | 0.001994 |
| is_bounded_array_test.test | RUN | 0.175523 | 0.380146 | 0.672547 | 0.002055 | 0.002245 | 0.002021 |
| is_base_of_test.test | RUN | 0.183106 | 0.422727 | 0.760578 | 0.002063 | 0.002248 | 0.001902 |
| is_base_and_derived_test.test | RUN | 0.180311 | 0.416105 | 0.726390 | 0.001936 | 0.001925 | 0.002104 |
| is_assignable_test.test | RUN | 0.230488 | 0.600638 | 1.131243 | 0.001989 | 0.001807 | 0.001989 |
| is_array_test.test | RUN | 0.182020 | 0.404754 | 0.716844 | 0.002018 | 0.001998 | 0.001932 |
| is_arithmetic_test.test | RUN | 0.232601 | 0.635134 | 1.226302 | 0.001944 | 0.002224 | 0.002250 |
| is_abstract_test.test | RUN | 0.417934 | 1.744585 | 3.976202 | 0.001909 | 0.001878 | 0.002245 |
| integral_constant_test.test | RUN | 0.025331 | 0.059742 | 0.064306 | 0.000700 | 0.000923 | 0.001700 |
| has_virtual_destructor_test.test | RUN | 0.192254 | 0.474704 | 0.858400 | 0.002044 | 0.002218 | 0.002200 |
| has_unary_plus_test.test | RUN | 0.365992 | 1.917832 | 3.001401 | 0.001966 | 0.002269 | 0.002270 |
| has_unary_minus_test.test | RUN | 0.354872 | 1.889467 | 3.073360 | 0.001780 | 0.001752 | 0.002042 |
| has_trivial_move_constructor_test.test | RUN | 0.255818 | 0.739559 | 1.491015 | 0.001685 | 0.002030 | 0.002032 |
| has_trivial_move_assign_test.test | RUN | 0.256734 | 0.676271 | 1.327633 | 0.002004 | 0.001968 | 0.002084 |
| has_trivial_destructor_test.test | RUN | 0.252838 | 0.675910 | 1.443311 | 0.002162 | 0.001883 | 0.002142 |
| has_trivial_copy_test.test | RUN | 0.261482 | 0.778022 | 1.536611 | 0.001704 | 0.002308 | 0.002376 |
| has_trivial_constr_test.test | RUN | 0.261368 | 0.768976 | 1.519428 | 0.001866 | 0.002160 | 0.002292 |
| has_trivial_assign_test.test | RUN | 0.256301 | 0.764803 | 1.515323 | 0.001953 | 0.002354 | 0.002215 |
| has_right_shift_test.test | RUN | 0.375111 | 1.445833 | 3.200549 | 0.001858 | 0.002076 | 0.002233 |
| has_right_shift_assign_test.test | RUN | 0.363912 | 1.353668 | 3.005109 | 0.001640 | 0.002214 | 0.002234 |
| has_prefix_classes3_test.test | RUN | 0.271999 | 0.786394 | 1.591257 | 0.001951 | 0.002031 | 0.002080 |
| has_prefix_classes2_test.test | RUN | 0.278686 | 0.760835 | 1.592301 | 0.001727 | 0.002138 | 0.002086 |
| has_prefix_classes1_test.test | RUN | 0.262564 | 0.805961 | 1.577592 | 0.001864 | 0.002264 | 0.002160 |
| has_prefix_classes0_test.test | RUN | 0.269140 | 0.803756 | 1.502388 | 0.001872 | 0.002165 | 0.002024 |
| has_pre_increment_test.test | RUN | 0.351882 | 1.941450 | 3.007593 | 0.001900 | 0.001990 | 0.001884 |
| has_pre_decrement_test.test | RUN | 0.359386 | 1.940497 | 3.011187 | 0.001807 | 0.002264 | 0.002186 |
| has_postfix_classes3_test.test | RUN | 0.262158 | 0.781190 | 1.565818 | 0.002166 | 0.002031 | 0.001969 |
| has_postfix_classes2_test.test | RUN | 0.267062 | 0.777538 | 1.581724 | 0.001807 | 0.002088 | 0.001970 |
| has_postfix_classes1_test.test | RUN | 0.268164 | 0.741303 | 1.602388 | 0.001842 | 0.002240 | 0.001907 |
| has_postfix_classes0_test.test | RUN | 0.267130 | 0.818163 | 1.590528 | 0.001870 | 0.002395 | 0.001881 |
| has_post_increment_test.test | RUN | 0.358312 | 1.947656 | 2.992937 | 0.001905 | 0.001921 | 0.001924 |
| has_post_decrement_test.test | RUN | 0.353883 | 1.958088 | 3.022408 | 0.001835 | 0.002047 | 0.002204 |
| has_plus_test.test | RUN | 0.361163 | 1.359321 | 3.099724 | 0.001941 | 0.001815 | 0.002156 |
| has_plus_assign_test.test | RUN | 0.369675 | 1.291882 | 3.107665 | 0.001790 | 0.002226 | 0.002127 |
| has_operator_new_test.test | RUN | 0.252052 | 0.699333 | 1.364990 | 0.002025 | 0.001940 | 0.002147 |
| has_nothrow_destructor_test.test | RUN | 0.263427 | 0.766452 | 1.523736 | 0.002076 | 0.002157 | 0.002137 |
| has_nothrow_copy_test.test | RUN | 0.253171 | 0.742341 | 1.477181 | 0.002062 | 0.002062 | 0.002059 |
| has_nothrow_constr_test.test | RUN | 0.245598 | 0.720374 | 1.434907 | 0.001884 | 0.001804 | 0.001950 |
| has_nothrow_assign_test.test | RUN | 0.245176 | 0.721086 | 1.426905 | 0.002098 | 0.002066 | 0.002182 |
| has_not_equal_to_test.test | RUN | 0.371977 | 1.913855 | 3.094404 | 0.002104 | 0.002309 | 0.002230 |
| has_negate_test.test | RUN | 0.339014 | 1.290396 | 2.919835 | 0.001876 | 0.001902 | 0.001973 |
| has_multiplies_test.test | RUN | 0.365492 | 1.314470 | 3.077737 | 0.001790 | 0.001827 | 0.002239 |
| has_multiplies_assign_test.test | RUN | 0.366894 | 1.318634 | 2.977161 | 0.001994 | 0.002172 | 0.002148 |
| has_modulus_test.test | RUN | 0.355515 | 1.352795 | 3.028859 | 0.001956 | 0.001955 | 0.002143 |
| has_modulus_assign_test.test | RUN | 0.360738 | 1.400875 | 3.058018 | 0.002110 | 0.001989 | 0.001986 |
| has_minus_test.test | RUN | 0.370064 | 1.396828 | 3.070340 | 0.002004 | 0.001885 | 0.001998 |
| has_minus_assign_test.test | RUN | 0.369279 | 1.308126 | 3.075963 | 0.002225 | 0.002264 | 0.002026 |
| has_logical_or_test.test | RUN | 0.361279 | 1.401182 | 3.043514 | 0.001896 | 0.001964 | 0.002171 |
| has_logical_not_test.test | RUN | 0.351813 | 1.375134 | 2.992212 | 0.002083 | 0.002126 | 0.002156 |
| has_logical_and_test.test | RUN | 0.358825 | 1.359419 | 3.045336 | 0.001964 | 0.001964 | 0.002158 |
| has_less_test.test | RUN | 0.359308 | 1.860043 | 3.019297 | 0.002130 | 0.002157 | 0.002159 |
| has_less_equal_test.test | RUN | 0.365895 | 1.874818 | 3.074684 | 0.001890 | 0.001784 | 0.002218 |
| has_left_shift_test.test | RUN | 0.388822 | 1.746352 | 3.720575 | 0.002012 | 0.001916 | 0.002150 |
| has_left_shift_assign_test.test | RUN | 0.350552 | 1.277351 | 3.055389 | 0.002002 | 0.001824 | 0.001912 |
| has_greater_test.test | RUN | 0.358763 | 1.880533 | 2.998968 | 0.002263 | 0.002187 | 0.002011 |
| has_greater_equal_test.test | RUN | 0.363093 | 1.788389 | 3.033156 | 0.002100 | 0.002136 | 0.002175 |
| has_equal_to_test.test | RUN | 0.362538 | 1.838557 | 3.075638 | 0.001912 | 0.002076 | 0.001884 |
| has_divides_test.test | RUN | 0.359355 | 1.321282 | 3.064523 | 0.001771 | 0.002040 | 0.001901 |
| has_divides_assign_test.test | RUN | 0.368302 | 1.297158 | 2.945278 | 0.002136 | 0.002181 | 0.001732 |
| has_dereference_test.test | RUN | 0.357360 | 1.333839 | 2.923306 | 0.002000 | 0.002216 | 0.002169 |
| has_complement_test.test | RUN | 0.350437 | 1.910517 | 3.008796 | 0.001908 | 0.001918 | 0.002168 |
| has_bit_xor_test.test | RUN | 0.368726 | 1.342356 | 3.042390 | 0.001971 | 0.002244 | 0.001862 |
| has_bit_xor_assign_test.test | RUN | 0.361184 | 1.291662 | 3.047721 | 0.002178 | 0.002041 | 0.001751 |
| has_bit_or_test.test | RUN | 0.361408 | 1.373857 | 3.023710 | 0.001758 | 0.002295 | 0.001529 |
| has_bit_or_assign_test.test | RUN | 0.364680 | 1.361488 | 3.032159 | 0.001930 | 0.002178 | 0.001770 |
| has_bit_and_test.test | RUN | 0.355091 | 1.344378 | 2.971426 | 0.001912 | 0.002300 | 0.002216 |
| has_bit_and_assign_test.test | RUN | 0.356744 | 1.427354 | 3.033675 | 0.001985 | 0.002266 | 0.002199 |
| has_binary_classes9_test.test | RUN | 0.390767 | 1.612935 | 3.301996 | 0.001868 | 0.002289 | 0.002171 |
| has_binary_classes8_test.test | RUN | 0.399779 | 1.572189 | 3.319345 | 0.002074 | 0.002271 | 0.001956 |
| has_binary_classes7_test.test | RUN | 0.383341 | 1.616334 | 3.356713 | 0.001923 | 0.002238 | 0.001708 |
| has_binary_classes6_test.test | RUN | 0.387394 | 1.543399 | 3.261857 | 0.002185 | 0.002154 | 0.001744 |
| has_binary_classes5_test.test | RUN | 0.394647 | 1.531232 | 3.333383 | 0.001855 | 0.002146 | 0.002230 |
| has_binary_classes4_test.test | RUN | 0.387852 | 1.522203 | 3.321336 | 0.001858 | 0.002040 | 0.001560 |
| has_binary_classes3_test.test | RUN | 0.395592 | 1.556491 | 3.355622 | 0.001627 | 0.002186 | 0.002124 |
| has_binary_classes2_test.test | RUN | 0.402190 | 1.647714 | 3.359892 | 0.002128 | 0.002146 | 0.001984 |
| has_binary_classes1_test.test | RUN | 0.385265 | 1.555886 | 3.292128 | 0.001742 | 0.001976 | 0.001866 |
| has_binary_classes0_test.test | RUN | 0.389942 | 1.612318 | 3.246468 | 0.001870 | 0.002059 | 0.002222 |
| function_traits_test.test | RUN | 0.178673 | 0.401966 | 0.722618 | 0.001842 | 0.001948 | 0.001945 |
| floating_point_promotion_test.test | RUN | 0.175623 | 0.399753 | 0.687066 | 0.001834 | 0.001910 | 0.002122 |
| extent_test.test | RUN | 0.179219 | 0.411993 | 0.657511 | 0.001947 | 0.002075 | 0.001747 |
| enable_if_test.test | RUN | 0.164441 | 0.350701 | 0.568741 | 0.001935 | 0.001593 | 0.002016 |
| disjunction_test.test | RUN | 0.167075 | 0.353038 | 0.582834 | 0.001996 | 0.002151 | 0.001742 |
| detected_test.test | RUN | 0.163840 | 0.335323 | 0.550284 | 0.002235 | 0.002037 | 0.001982 |
| detected_or_test.test | RUN | 0.171167 | 0.354890 | 0.592429 | 0.002130 | 0.002054 | 0.001738 |
| decay_test.test | RUN | 0.423333 | 1.491153 | 3.616823 | 0.001761 | 0.001948 | 0.002182 |
| cxx14_aliases_test.test | RUN | 0.274304 | 0.528431 | 0.968786 | 0.001984 | 0.002000 | 0.002142 |
| copy_reference_test.test | RUN | 0.174284 | 0.363526 | 0.500903 | 0.001948 | 0.002248 | 0.002118 |
| copy_cv_test.test | RUN | 0.168062 | 0.370280 | 0.650875 | 0.001838 | 0.002022 | 0.002140 |
| copy_cv_ref_test.test | RUN | 0.245720 | 0.670871 | 1.464388 | 0.002084 | 0.001842 | 0.002004 |
| conjunction_test.test | RUN | 0.170528 | 0.374843 | 0.608403 | 0.001766 | 0.001891 | 0.002253 |
| conditional_test.test | RUN | 0.167889 | 0.352139 | 0.593683 | 0.001963 | 0.001940 | 0.002274 |
| common_type_test.test | RUN | 0.218461 | 0.560649 | 1.127105 | 0.002132 | 0.001862 | 0.002092 |
| common_type_sfinae_test.test | RUN | 0.163733 | 0.346627 | 0.565098 | 0.001957 | 0.002084 | 0.002225 |
| common_type_sfinae2_test.test | RUN | 0.174420 | 0.356147 | 0.608034 | 0.001790 | 0.001846 | 0.002154 |
| common_type_6_test.test | RUN | 0.177869 | 0.374408 | 0.552543 | 0.001870 | 0.002060 | 0.002149 |
| common_type_5_test.test | RUN | 0.173375 | 0.364702 | 0.612382 | 0.002118 | 0.001787 | 0.001841 |
| common_type_4_test.test | RUN | 0.186569 | 0.403647 | 0.715391 | 0.001924 | 0.002058 | 0.001762 |
| common_type_3_test.test | RUN | 0.170580 | 0.347538 | 0.498753 | 0.001824 | 0.002032 | 0.001802 |
| common_type_2_test.test | RUN | 0.224087 | 0.559901 | 1.144117 | 0.002040 | 0.002230 | 0.001718 |
| alignment_of_test.test | RUN | 0.188785 | 0.420579 | 0.751437 | 0.001884 | 0.001919 | 0.002076 |
| alignment_of_a2_test.test | RUN | 0.182081 | 0.433350 | 0.726138 | 0.001938 | 0.001608 | 0.001818 |
| aligned_storage_test.test | RUN | 0.193879 | 0.422848 | 0.663436 | 0.002088 | 0.002296 | 0.002316 |
| aligned_storage_empy_test.test | RUN | 0.215586 | 0.488178 | 0.872845 | 0.002392 | 0.002270 | 0.001820 |
| aligned_storage_a2_test.test | RUN | 0.193252 | 0.404276 | 0.672696 | 0.002004 | 0.002314 | 0.001896 |
| add_volatile_test.test | RUN | 0.378000 | 1.247997 | 3.334140 | 0.001925 | 0.002078 | 0.002210 |
| add_rvalue_reference_test.test | RUN | 0.430073 | 1.404961 | 3.795563 | 0.002113 | 0.002202 | 0.002033 |
| add_reference_test.test | RUN | 0.415613 | 1.443025 | 3.848642 | 0.002099 | 0.002124 | 0.002244 |
| add_pointer_test.test | RUN | 0.269516 | 0.790889 | 1.992966 | 0.001899 | 0.001822 | 0.002240 |
| add_lvalue_reference_test.test | RUN | 0.412980 | 1.484628 | 3.903270 | 0.001856 | 0.001589 | 0.002052 |
| add_cv_test.test | RUN | 0.401416 | 1.311452 | 3.568012 | 0.001846 | 0.002126 | 0.001912 |
| add_const_test.test | RUN | 0.401438 | 1.315251 | 3.608138 | 0.001931 | 0.001840 | 0.002196 |
| is_virtual_base_of_fail.test | COMPILE_FAIL | 0.170899 | 0.332348 | 0.534789 | — | — | — |
| is_stateless_fail.test | COMPILE_FAIL | 0.162308 | 0.345415 | 0.552558 | — | — | — |
| is_polymorphic_fail.test | COMPILE_FAIL | 0.160406 | 0.310225 | 0.529366 | — | — | — |
| is_nothrow_move_constructible_fail.test | COMPILE_FAIL | 0.163220 | 0.323779 | 0.512275 | — | — | — |
| is_nothrow_move_assignable_fail.test | COMPILE_FAIL | 0.158170 | 0.322518 | 0.493001 | — | — | — |
| is_list_constructible_fail.test | COMPILE_FAIL | 0.156169 | 0.323155 | 0.504781 | — | — | — |
| is_empty_fail.test | COMPILE_FAIL | 0.163884 | 0.333186 | 0.522750 | — | — | — |
| is_destructible_fail.test | COMPILE_FAIL | 0.158084 | 0.317840 | 0.524674 | — | — | — |
| is_default_constructible_fail.test | COMPILE_FAIL | 0.156544 | 0.320735 | 0.527165 | — | — | — |
| is_copy_constructible_fail.test | COMPILE_FAIL | 0.161894 | 0.320426 | 0.508260 | — | — | — |
| is_copy_assignable_fail.test | COMPILE_FAIL | 0.161572 | 0.332033 | 0.514140 | — | — | — |
| is_convertible_fail.test | COMPILE_FAIL | 0.159816 | 0.326817 | 0.525103 | — | — | — |
| is_convertible2_fail.test | COMPILE_FAIL | 0.165113 | 0.326869 | 0.501815 | — | — | — |
| is_constructible_fail.test | COMPILE_FAIL | 0.156860 | 0.319797 | 0.530502 | — | — | — |
| is_base_of_fail.test | COMPILE_FAIL | 0.160849 | 0.332988 | 0.524221 | — | — | — |
| is_assignable_fail.test | COMPILE_FAIL | 0.162048 | 0.321149 | 0.512243 | — | — | — |
| has_nothrow_destruct_fail.test | COMPILE_FAIL | 0.151124 | 0.321354 | 0.520010 | — | — | — |
| has_nothrow_copy_fail.test | COMPILE_FAIL | 0.159856 | 0.321979 | 0.531736 | — | — | — |
| has_nothrow_cons_fail.test | COMPILE_FAIL | 0.160214 | 0.336613 | 0.499158 | — | — | — |
| has_nothrow_assign_fail.test | COMPILE_FAIL | 0.163554 | 0.323450 | 0.524621 | — | — | — |
| common_type_fail.test | COMPILE_FAIL | 0.159033 | 0.314083 | 0.516072 | — | — | — |
| common_type2_fail.test | COMPILE_FAIL | 0.158488 | 0.315669 | 0.523724 | — | — | — |
| has_nothrow_assign_test_no_intrinsics.test | RUN | 0.255868 | 0.742387 | 1.456522 | 0.001980 | 0.002168 | 0.002142 |
| has_nothrow_constr_test_no_intrinsics.test | RUN | 0.248293 | 0.720337 | 1.458931 | 0.001647 | 0.001590 | 0.002238 |
| has_nothrow_copy_test_no_intrinsics.test | RUN | 0.245572 | 0.708469 | 1.440720 | 0.001854 | 0.001600 | 0.002194 |
| is_nothrow_move_assignable_test_no_intrinsics.test | RUN | 0.247484 | 0.695343 | 1.457935 | 0.001824 | 0.002075 | 0.001567 |
| is_nothrow_move_constructible_test_no_intrinsics.test | RUN | 0.151104 | 0.488354 | 0.879320 | 0.001706 | 0.001700 | 0.001590 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.993× | 0.985×, 1.001× | 0.994× |
| clang | 1.018× | 1.015×, 1.021× | 0.994× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| add_const_test.test | 10 | 0.001536s | 0.001523s | 0.001532s | byte equality observed |
| add_cv_test.test | 10 | 0.001536s | 0.001521s | 0.001520s | byte equality observed |
| add_lvalue_reference_test.test | 10 | 0.001552s | 0.001535s | 0.001518s | byte equality observed |
| add_pointer_test.test | 10 | 0.001512s | 0.001526s | 0.001525s | byte equality observed |
| add_reference_test.test | 10 | 0.001526s | 0.001550s | 0.001532s | byte equality observed |
| add_rvalue_reference_test.test | 10 | 0.001559s | 0.001503s | 0.001524s | byte equality observed |
| add_volatile_test.test | 10 | 0.001521s | 0.001532s | 0.001519s | byte equality observed |
| aligned_storage_a2_test.test | 10 | 0.001663s | 0.001673s | 0.001649s | byte equality observed |
| aligned_storage_empy_test.test | 10 | 0.001525s | 0.001523s | 0.001545s | byte equality observed |
| aligned_storage_test.test | 10 | 0.001670s | 0.001669s | 0.001714s | byte equality observed |
| alignment_of_a2_test.test | 10 | 0.001540s | 0.001540s | 0.001513s | byte equality observed |
| alignment_of_test.test | 10 | 0.001519s | 0.001536s | 0.001544s | byte equality observed |
| common_type_2_test.test | 10 | 0.001540s | 0.001501s | 0.001516s | byte equality observed |
| common_type_3_test.test | 10 | 0.001489s | 0.001514s | 0.001543s | byte equality observed |
| common_type_4_test.test | 9 | 0.001530s | 0.001511s | 0.001544s | byte equality observed |
| common_type_5_test.test | 10 | 0.001531s | 0.001498s | 0.001525s | byte equality observed |
| common_type_6_test.test | 10 | 0.001530s | 0.001526s | 0.001512s | byte equality observed |
| common_type_sfinae2_test.test | 10 | 0.001524s | 0.001517s | 0.001509s | byte equality observed |
| common_type_sfinae_test.test | 10 | 0.001537s | 0.001504s | 0.001483s | byte equality observed |
| common_type_test.test | 9 | 0.001533s | 0.001540s | 0.001483s | byte equality observed |
| conditional_test.test | 8 | 0.001542s | 0.001484s | 0.001537s | byte equality observed |
| conjunction_test.test | 10 | 0.001517s | 0.001522s | 0.001518s | byte equality observed |
| copy_cv_ref_test.test | 9 | 0.001540s | 0.001512s | 0.001514s | byte equality observed |
| copy_cv_test.test | 10 | 0.001554s | 0.001519s | 0.001507s | byte equality observed |
| copy_reference_test.test | 10 | 0.001530s | 0.001531s | 0.001471s | byte equality observed |
| cxx14_aliases_test.test | 10 | 0.001535s | 0.001535s | 0.001532s | byte equality observed |
| decay_test.test | 10 | 0.001539s | 0.001547s | 0.001497s | byte equality observed |
| detected_or_test.test | 10 | 0.001525s | 0.001505s | 0.001501s | byte equality observed |
| detected_test.test | 10 | 0.001525s | 0.001497s | 0.001531s | byte equality observed |
| disjunction_test.test | 10 | 0.001508s | 0.001524s | 0.001543s | byte equality observed |
| enable_if_test.test | 10 | 0.001527s | 0.001529s | 0.001507s | byte equality observed |
| extent_test.test | 10 | 0.001523s | 0.001545s | 0.001521s | byte equality observed |
| floating_point_promotion_test.test | 10 | 0.001563s | 0.001524s | 0.001510s | byte equality observed |
| function_traits_test.test | 10 | 0.001539s | 0.001539s | 0.001527s | byte equality observed |
| has_binary_classes0_test.test | 10 | 0.001540s | 0.001528s | 0.001508s | byte equality observed |
| has_binary_classes1_test.test | 10 | 0.001510s | 0.001534s | 0.001514s | byte equality observed |
| has_binary_classes2_test.test | 10 | 0.001538s | 0.001524s | 0.001503s | byte equality observed |
| has_binary_classes3_test.test | 10 | 0.001517s | 0.001531s | 0.001486s | byte equality observed |
| has_binary_classes4_test.test | 10 | 0.001516s | 0.001543s | 0.001510s | byte equality observed |
| has_binary_classes5_test.test | 10 | 0.001520s | 0.001528s | 0.001545s | byte equality observed |
| has_binary_classes6_test.test | 10 | 0.001527s | 0.001500s | 0.001530s | byte equality observed |
| has_binary_classes7_test.test | 10 | 0.001505s | 0.001533s | 0.001513s | byte equality observed |
| has_binary_classes8_test.test | 10 | 0.001520s | 0.001523s | 0.001481s | byte equality observed |
| has_binary_classes9_test.test | 10 | 0.001533s | 0.001537s | 0.001533s | byte equality observed |
| has_bit_and_assign_test.test | 10 | 0.001525s | 0.001521s | 0.001520s | byte equality observed |
| has_bit_and_test.test | 10 | 0.001518s | 0.001523s | 0.001536s | byte equality observed |
| has_bit_or_assign_test.test | 10 | 0.001533s | 0.001478s | 0.001510s | byte equality observed |
| has_bit_or_test.test | 10 | 0.001523s | 0.001502s | 0.001517s | byte equality observed |
| has_bit_xor_assign_test.test | 10 | 0.001514s | 0.001522s | 0.001493s | byte equality observed |
| has_bit_xor_test.test | 10 | 0.001518s | 0.001508s | 0.001512s | byte equality observed |
| has_complement_test.test | 10 | 0.001510s | 0.001512s | 0.001503s | byte equality observed |
| has_dereference_test.test | 10 | 0.001504s | 0.001525s | 0.001531s | byte equality observed |
| has_divides_assign_test.test | 10 | 0.001525s | 0.001537s | 0.001516s | byte equality observed |
| has_divides_test.test | 10 | 0.001531s | 0.001545s | 0.001527s | byte equality observed |
| has_equal_to_test.test | 10 | 0.001527s | 0.001537s | 0.001504s | byte equality observed |
| has_greater_equal_test.test | 10 | 0.001529s | 0.001539s | 0.001510s | byte equality observed |
| has_greater_test.test | 10 | 0.001512s | 0.001517s | 0.001532s | byte equality observed |
| has_left_shift_assign_test.test | 10 | 0.001529s | 0.001505s | 0.001489s | byte equality observed |
| has_left_shift_test.test | 10 | 0.001529s | 0.001504s | 0.001510s | byte equality observed |
| has_less_equal_test.test | 9 | 0.001505s | 0.001533s | 0.001528s | byte equality observed |
| has_less_test.test | 10 | 0.001528s | 0.001527s | 0.001520s | byte equality observed |
| has_logical_and_test.test | 10 | 0.001527s | 0.001529s | 0.001542s | byte equality observed |
| has_logical_not_test.test | 10 | 0.001520s | 0.001551s | 0.001506s | byte equality observed |
| has_logical_or_test.test | 10 | 0.001535s | 0.001526s | 0.001486s | byte equality observed |
| has_minus_assign_test.test | 9 | 0.001534s | 0.001531s | 0.001508s | byte equality observed |
| has_minus_test.test | 10 | 0.001523s | 0.001525s | 0.001478s | byte equality observed |
| has_modulus_assign_test.test | 10 | 0.001526s | 0.001508s | 0.001484s | byte equality observed |
| has_modulus_test.test | 10 | 0.001528s | 0.001522s | 0.001510s | byte equality observed |
| has_multiplies_assign_test.test | 10 | 0.001516s | 0.001503s | 0.001507s | byte equality observed |
| has_multiplies_test.test | 10 | 0.001522s | 0.001509s | 0.001499s | byte equality observed |
| has_negate_test.test | 10 | 0.001561s | 0.001510s | 0.001531s | byte equality observed |
| has_not_equal_to_test.test | 10 | 0.001520s | 0.001516s | 0.001482s | byte equality observed |
| has_nothrow_assign_test.test | 9 | 0.001495s | 0.001487s | 0.001480s | byte equality observed |
| has_nothrow_assign_test_no_intrinsics.test | 10 | 0.001537s | 0.001558s | 0.001546s | byte equality observed |
| has_nothrow_constr_test.test | 10 | 0.001514s | 0.001540s | 0.001517s | byte equality observed |
| has_nothrow_constr_test_no_intrinsics.test | 10 | 0.001507s | 0.001529s | 0.001530s | byte equality observed |
| has_nothrow_copy_test.test | 10 | 0.001517s | 0.001490s | 0.001516s | byte equality observed |
| has_nothrow_copy_test_no_intrinsics.test | 10 | 0.001522s | 0.001523s | 0.001497s | byte equality observed |
| has_nothrow_destructor_test.test | 10 | 0.001508s | 0.001490s | 0.001494s | byte equality observed |
| has_operator_new_test.test | 10 | 0.001521s | 0.001534s | 0.001501s | byte equality observed |
| has_plus_assign_test.test | 10 | 0.001506s | 0.001495s | 0.001537s | byte equality observed |
| has_plus_test.test | 10 | 0.001517s | 0.001524s | 0.001541s | byte equality observed |
| has_post_decrement_test.test | 10 | 0.001530s | 0.001510s | 0.001537s | byte equality observed |
| has_post_increment_test.test | 10 | 0.001519s | 0.001521s | 0.001524s | byte equality observed |
| has_postfix_classes0_test.test | 10 | 0.001513s | 0.001491s | 0.001517s | byte equality observed |
| has_postfix_classes1_test.test | 10 | 0.001535s | 0.001543s | 0.001510s | byte equality observed |
| has_postfix_classes2_test.test | 10 | 0.001517s | 0.001500s | 0.001528s | byte equality observed |
| has_postfix_classes3_test.test | 10 | 0.001518s | 0.001544s | 0.001537s | byte equality observed |
| has_pre_decrement_test.test | 10 | 0.001539s | 0.001500s | 0.001535s | byte equality observed |
| has_pre_increment_test.test | 10 | 0.001553s | 0.001496s | 0.001525s | byte equality observed |
| has_prefix_classes0_test.test | 10 | 0.001520s | 0.001543s | 0.001492s | byte equality observed |
| has_prefix_classes1_test.test | 10 | 0.001536s | 0.001504s | 0.001546s | byte equality observed |
| has_prefix_classes2_test.test | 10 | 0.001526s | 0.001495s | 0.001523s | byte equality observed |
| has_prefix_classes3_test.test | 10 | 0.001495s | 0.001538s | 0.001543s | byte equality observed |
| has_right_shift_assign_test.test | 10 | 0.001533s | 0.001505s | 0.001534s | byte equality observed |
| has_right_shift_test.test | 10 | 0.001539s | 0.001524s | 0.001522s | byte equality observed |
| has_trivial_assign_test.test | 10 | 0.001602s | 0.001601s | 0.001591s | byte equality observed |
| has_trivial_constr_test.test | 10 | 0.001595s | 0.001574s | 0.001637s | native test oracle; output varies |
| has_trivial_copy_test.test | 9 | 0.001616s | 0.001604s | 0.001655s | native test oracle; output varies |
| has_trivial_destructor_test.test | 10 | 0.001548s | 0.001530s | 0.001517s | byte equality observed |
| has_trivial_move_assign_test.test | 10 | 0.001543s | 0.001528s | 0.001555s | byte equality observed |
| has_trivial_move_constructor_test.test | 10 | 0.001528s | 0.001523s | 0.001535s | byte equality observed |
| has_unary_minus_test.test | 10 | 0.001554s | 0.001516s | 0.001530s | byte equality observed |
| has_unary_plus_test.test | 10 | 0.001555s | 0.001523s | 0.001511s | byte equality observed |
| has_virtual_destructor_test.test | 10 | 0.001573s | 0.001556s | 0.001492s | byte equality observed |
| integral_constant_test.test | 10 | 0.000703s | 0.000692s | 0.001539s | byte equality observed |
| is_abstract_test.test | 10 | 0.001576s | 0.001548s | 0.001481s | byte equality observed |
| is_arithmetic_test.test | 10 | 0.001540s | 0.001502s | 0.001512s | byte equality observed |
| is_array_test.test | 10 | 0.001539s | 0.001546s | 0.001485s | byte equality observed |
| is_assignable_test.test | 10 | 0.001536s | 0.001540s | 0.001493s | byte equality observed |
| is_base_and_derived_test.test | 10 | 0.001553s | 0.001525s | 0.001509s | byte equality observed |
| is_base_of_test.test | 10 | 0.001536s | 0.001494s | 0.001536s | byte equality observed |
| is_bounded_array_test.test | 10 | 0.001541s | 0.001511s | 0.001511s | byte equality observed |
| is_class_test.test | 10 | 0.001505s | 0.001522s | 0.001511s | byte equality observed |
| is_complete_test.test | 10 | 0.001532s | 0.001525s | 0.001537s | byte equality observed |
| is_complex_test.test | 10 | 0.001509s | 0.001491s | 0.001535s | byte equality observed |
| is_compound_test.test | 10 | 0.001506s | 0.001525s | 0.001521s | byte equality observed |
| is_const_test.test | 10 | 0.001540s | 0.001535s | 0.001534s | byte equality observed |
| is_constructible_test.test | 10 | 0.001525s | 0.001544s | 0.001501s | byte equality observed |
| is_convertible_test.test | 10 | 0.001530s | 0.001510s | 0.001508s | byte equality observed |
| is_copy_assignable_test.test | 9 | 0.001553s | 0.001506s | 0.001521s | byte equality observed |
| is_copy_constructible_test.test | 10 | 0.001546s | 0.001512s | 0.001477s | byte equality observed |
| is_default_constr_test.test | 10 | 0.001507s | 0.001497s | 0.001514s | byte equality observed |
| is_destructible_test.test | 10 | 0.001530s | 0.001506s | 0.001528s | byte equality observed |
| is_detected_exact_test.test | 10 | 0.001522s | 0.001485s | 0.001501s | byte equality observed |
| is_detected_test.test | 10 | 0.001529s | 0.001496s | 0.001497s | byte equality observed |
| is_empty_test.test | 10 | 0.001546s | 0.001547s | 0.001531s | byte equality observed |
| is_enum_test.test | 10 | 0.001509s | 0.001501s | 0.001524s | byte equality observed |
| is_final_test.test | 10 | 0.001534s | 0.001502s | 0.001530s | byte equality observed |
| is_float_test.test | 10 | 0.001520s | 0.001514s | 0.001492s | byte equality observed |
| is_floating_point_test.test | 10 | 0.001521s | 0.001516s | 0.001512s | byte equality observed |
| is_function_test.test | 10 | 0.001533s | 0.001516s | 0.001516s | byte equality observed |
| is_fundamental_test.test | 10 | 0.001554s | 0.001481s | 0.001494s | byte equality observed |
| is_integral_test.test | 10 | 0.001544s | 0.001500s | 0.001506s | byte equality observed |
| is_list_constructible_test.test | 10 | 0.001493s | 0.001500s | 0.001531s | byte equality observed |
| is_lvalue_reference_test.test | 10 | 0.001520s | 0.001513s | 0.001488s | byte equality observed |
| is_member_func_test.test | 10 | 0.001508s | 0.001538s | 0.001492s | byte equality observed |
| is_member_obj_test.test | 10 | 0.001525s | 0.001535s | 0.001480s | byte equality observed |
| is_member_pointer_test.test | 10 | 0.001504s | 0.001523s | 0.001503s | byte equality observed |
| is_noncopyable_test.test | 10 | 0.001510s | 0.001518s | 0.001503s | byte equality observed |
| is_nothrow_move_assignable_test.test | 10 | 0.001537s | 0.001549s | 0.001489s | byte equality observed |
| is_nothrow_move_assignable_test_no_intrinsics.test | 10 | 0.001539s | 0.001513s | 0.001528s | byte equality observed |
| is_nothrow_move_constructible_test.test | 10 | 0.001545s | 0.001487s | 0.001517s | byte equality observed |
| is_nothrow_move_constructible_test_no_intrinsics.test | 10 | 0.001537s | 0.001529s | 0.001506s | byte equality observed |
| is_nothrow_swappable_test.test | 8 | 0.001506s | 0.001530s | 0.001498s | byte equality observed |
| is_object_test.test | 10 | 0.001507s | 0.001516s | 0.001520s | byte equality observed |
| is_pod_test.test | 10 | 0.001513s | 0.001501s | 0.001495s | byte equality observed |
| is_pointer_test.test | 10 | 0.001539s | 0.001541s | 0.001554s | byte equality observed |
| is_polymorphic_test.test | 10 | 0.001512s | 0.001514s | 0.001504s | byte equality observed |
| is_reference_test.test | 9 | 0.001528s | 0.001492s | 0.001483s | byte equality observed |
| is_rvalue_reference_test.test | 8 | 0.001555s | 0.001540s | 0.001530s | byte equality observed |
| is_same_test.test | 10 | 0.001531s | 0.001536s | 0.001507s | byte equality observed |
| is_scalar_test.test | 10 | 0.001532s | 0.001517s | 0.001496s | byte equality observed |
| is_scoped_enum_test.test | 10 | 0.001518s | 0.001492s | 0.001502s | byte equality observed |
| is_signed_test.test | 10 | 0.001491s | 0.001498s | 0.001509s | byte equality observed |
| is_stateless_test.test | 10 | 0.001513s | 0.001488s | 0.001533s | byte equality observed |
| is_swappable_test.test | 10 | 0.001563s | 0.001508s | 0.001507s | byte equality observed |
| is_trivially_copyable_test.test | 10 | 0.001582s | 0.001551s | 0.001526s | byte equality observed |
| is_unbounded_array_test.test | 8 | 0.001513s | 0.001496s | 0.001510s | byte equality observed |
| is_union_test.test | 10 | 0.001532s | 0.001527s | 0.001503s | byte equality observed |
| is_unscoped_enum_test.test | 10 | 0.001535s | 0.001490s | 0.001504s | byte equality observed |
| is_unsigned_test.test | 10 | 0.001545s | 0.001489s | 0.001514s | byte equality observed |
| is_virtual_base_of_test.test | 10 | 0.001529s | 0.001493s | 0.001546s | byte equality observed |
| is_void_test.test | 10 | 0.001527s | 0.001527s | 0.001532s | byte equality observed |
| is_volatile_test.test | 10 | 0.001505s | 0.001491s | 0.001505s | byte equality observed |
| make_signed_test.test | 9 | 0.001510s | 0.001535s | 0.001523s | byte equality observed |
| make_unsigned_test.test | 10 | 0.001559s | 0.001526s | 0.001532s | byte equality observed |
| make_void_test.test | 10 | 0.001527s | 0.001542s | 0.001517s | byte equality observed |
| mpl_interop_test1.test | 10 | 0.000694s | 0.000685s | 0.001509s | byte equality observed |
| mpl_interop_test2.test | 10 | 0.000694s | 0.000683s | 0.001498s | byte equality observed |
| mpl_interop_test3.test | 10 | 0.000700s | 0.000684s | 0.001528s | byte equality observed |
| negation_test.test | 10 | 0.001603s | 0.001612s | 0.001540s | byte equality observed |
| promote_basic_test.test | 10 | 0.000688s | 0.000687s | 0.001482s | byte equality observed |
| promote_enum_msvc_bug_test.test | 10 | 0.000692s | 0.000705s | 0.001513s | byte equality observed |
| promote_enum_test.test | 10 | 0.000695s | 0.000692s | 0.001525s | byte equality observed |
| promote_mpl_test.test | 10 | 0.000694s | 0.000690s | 0.001506s | byte equality observed |
| rank_test.test | 10 | 0.001557s | 0.001497s | 0.001491s | byte equality observed |
| remove_all_extents_test.test | 10 | 0.001551s | 0.001520s | 0.001507s | byte equality observed |
| remove_bounds_test.test | 10 | 0.001565s | 0.001498s | 0.001517s | byte equality observed |
| remove_const_test.test | 10 | 0.001538s | 0.001481s | 0.001512s | byte equality observed |
| remove_cv_ref_test.test | 10 | 0.001494s | 0.001522s | 0.001527s | byte equality observed |
| remove_cv_test.test | 8 | 0.001532s | 0.001511s | 0.001509s | byte equality observed |
| remove_extent_test.test | 10 | 0.001513s | 0.001509s | 0.001510s | byte equality observed |
| remove_pointer_test.test | 10 | 0.001543s | 0.001525s | 0.001529s | byte equality observed |
| remove_reference_test.test | 10 | 0.001533s | 0.001524s | 0.001492s | byte equality observed |
| remove_volatile_test.test | 10 | 0.001539s | 0.001510s | 0.001505s | byte equality observed |
| tricky_abstract_type_test.test | 10 | 0.001509s | 0.001509s | 0.001511s | byte equality observed |
| tricky_add_pointer_test.test | 10 | 0.001541s | 0.001517s | 0.001530s | byte equality observed |
| tricky_function_type_test.test | 10 | 0.001512s | 0.001486s | 0.001500s | byte equality observed |
| tricky_incomplete_type_test.test | 10 | 0.001513s | 0.001491s | 0.001529s | byte equality observed |
| tricky_is_enum_test.test | 10 | 0.001533s | 0.001507s | 0.001503s | byte equality observed |
| tricky_partial_spec_test.test | 10 | 0.001530s | 0.001527s | 0.001501s | byte equality observed |
| tricky_rvalue_test.test | 10 | 0.001516s | 0.001533s | 0.001485s | byte equality observed |
| type_identity_test.test | 10 | 0.001553s | 0.001523s | 0.001470s | byte equality observed |
| type_traits_test.test | 10 | 0.000681s | 0.000702s | 0.001534s | byte equality observed |
| type_with_alignment_test.test | 10 | 0.001672s | 0.001726s | 0.001699s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
