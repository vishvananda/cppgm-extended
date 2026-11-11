# libs/type_traits/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| type_with_alignment_test.test | RUN | 0.132689 | 0.325383 | 0.507099 | 0.001504 | 0.001819 | 0.001543 |
| type_traits_test.test | RUN | 0.183345 | 0.331953 | 0.504663 | 0.000616 | 0.000620 | 0.001531 |
| type_identity_test.test | RUN | 0.320044 | 1.165504 | 2.920950 | 0.001639 | 0.001906 | 0.001617 |
| tricky_rvalue_test.test | RUN | 0.122217 | 0.257056 | 0.393204 | 0.001676 | 0.001816 | 0.001726 |
| tricky_partial_spec_test.test | RUN | 0.148499 | 0.372908 | 0.666127 | 0.001600 | 0.001650 | 0.001750 |
| tricky_is_enum_test.test | RUN | 0.116223 | 0.248603 | 0.356164 | 0.001544 | 0.001755 | 0.002193 |
| tricky_incomplete_type_test.test | RUN | 0.199369 | 0.384213 | 0.575590 | 0.001513 | 0.001702 | 0.001340 |
| tricky_function_type_test.test | RUN | 0.155452 | 0.397747 | 0.694725 | 0.001566 | 0.001670 | 0.001664 |
| tricky_add_pointer_test.test | RUN | 0.217623 | 0.680647 | 1.649330 | 0.001614 | 0.001947 | 0.001690 |
| tricky_abstract_type_test.test | RUN | 0.126065 | 0.254106 | 0.370698 | 0.001699 | 0.001691 | 0.001580 |
| remove_volatile_test.test | RUN | 0.292438 | 0.997868 | 2.564484 | 0.001784 | 0.001819 | 0.001629 |
| remove_reference_test.test | RUN | 0.302537 | 1.068908 | 2.714534 | 0.001649 | 0.001925 | 0.001658 |
| remove_pointer_test.test | RUN | 0.312977 | 1.098168 | 2.864390 | 0.002760 | 0.001686 | 0.002184 |
| remove_extent_test.test | RUN | 0.303438 | 1.070240 | 2.719644 | 0.001667 | 0.001864 | 0.001834 |
| remove_cv_test.test | RUN | 0.316067 | 1.050957 | 2.706883 | 0.001711 | 0.001784 | 0.001694 |
| remove_cv_ref_test.test | RUN | 0.641292 | 2.523216 | 6.618469 | 0.001788 | 0.001494 | 0.001685 |
| remove_const_test.test | RUN | 0.289737 | 1.003592 | 2.600540 | 0.002974 | 0.001916 | 0.001727 |
| remove_bounds_test.test | RUN | 0.300153 | 1.060248 | 2.743175 | 0.002828 | 0.001854 | 0.001686 |
| remove_all_extents_test.test | RUN | 0.323669 | 1.039684 | 2.593106 | 0.001658 | 0.001822 | 0.001591 |
| rank_test.test | RUN | 0.122728 | 0.248869 | 0.401806 | 0.001537 | 0.001800 | 0.001639 |
| promote_mpl_test.test | RUN | 0.114630 | 0.229873 | 0.276143 | 0.000603 | 0.000659 | 0.001745 |
| promote_enum_test.test | RUN | 0.024327 | 0.057189 | 0.071568 | 0.000663 | 0.000763 | 0.001731 |
| promote_enum_msvc_bug_test.test | RUN | 0.023177 | 0.050386 | 0.057595 | 0.000610 | 0.000662 | 0.001694 |
| promote_basic_test.test | RUN | 0.027054 | 0.068589 | 0.073272 | 0.000677 | 0.000703 | 0.001691 |
| negation_test.test | RUN | 0.116215 | 0.228554 | 0.353057 | 0.001664 | 0.001571 | 0.001555 |
| mpl_interop_test3.test | RUN | 0.104931 | 0.217454 | 0.235295 | 0.000580 | 0.000712 | 0.001488 |
| mpl_interop_test2.test | RUN | 0.043290 | 0.063649 | 0.072225 | 0.000570 | 0.000709 | 0.001672 |
| mpl_interop_test1.test | RUN | 0.019166 | 0.045436 | 0.046431 | 0.000630 | 0.000783 | 0.001463 |
| make_void_test.test | RUN | 0.121555 | 0.261096 | 0.385382 | 0.001638 | 0.001638 | 0.001749 |
| make_unsigned_test.test | RUN | 0.140940 | 0.350364 | 0.540393 | 0.001526 | 0.001677 | 0.001729 |
| make_signed_test.test | RUN | 0.143656 | 0.352039 | 0.576675 | 0.001524 | 0.001578 | 0.001620 |
| is_volatile_test.test | RUN | 0.131559 | 0.296280 | 0.438435 | 0.001566 | 0.001531 | 0.001506 |
| is_void_test.test | RUN | 0.123696 | 0.269006 | 0.398584 | 0.001676 | 0.001864 | 0.001756 |
| is_virtual_base_of_test.test | RUN | 0.144521 | 0.352695 | 0.553589 | 0.001606 | 0.001657 | 0.001680 |
| is_unsigned_test.test | RUN | 0.128795 | 0.291095 | 0.413859 | 0.001405 | 0.001482 | 0.001497 |
| is_unscoped_enum_test.test | RUN | 0.122894 | 0.247409 | 0.365485 | 0.001479 | 0.001779 | 0.001476 |
| is_union_test.test | RUN | 0.131092 | 0.319660 | 0.518906 | 0.001654 | 0.001822 | 0.001452 |
| is_unbounded_array_test.test | RUN | 0.121809 | 0.269331 | 0.409569 | 0.001548 | 0.001800 | 0.001460 |
| is_trivially_copyable_test.test | RUN | 0.200844 | 0.570104 | 1.054494 | 0.001351 | 0.001985 | 0.001366 |
| is_swappable_test.test | RUN | 0.166480 | 0.433562 | 0.801368 | 0.001576 | 0.001780 | 0.001725 |
| is_stateless_test.test | RUN | 0.183814 | 0.495781 | 0.920924 | 0.001548 | 0.001599 | 0.001608 |
| is_signed_test.test | RUN | 0.126510 | 0.299097 | 0.448342 | 0.001750 | 0.001794 | 0.001588 |
| is_scoped_enum_test.test | RUN | 0.122676 | 0.251523 | 0.371325 | 0.001534 | 0.001573 | 0.001222 |
| is_scalar_test.test | RUN | 0.161803 | 0.451692 | 0.791484 | 0.001494 | 0.001702 | 0.001892 |
| is_same_test.test | RUN | 0.122550 | 0.273234 | 0.409877 | 0.001680 | 0.001718 | 0.001741 |
| is_rvalue_reference_test.test | RUN | 0.135062 | 0.301054 | 0.491363 | 0.001512 | 0.001597 | 0.001452 |
| is_reference_test.test | RUN | 0.132029 | 0.314727 | 0.490622 | 0.001666 | 0.001589 | 0.001309 |
| is_polymorphic_test.test | RUN | 0.139864 | 0.331027 | 0.490188 | 0.001559 | 0.002026 | 0.001666 |
| is_pointer_test.test | RUN | 0.133567 | 0.310619 | 0.482364 | 0.001826 | 0.001524 | 0.001525 |
| is_pod_test.test | RUN | 0.182084 | 0.508840 | 0.912145 | 0.001540 | 0.001873 | 0.001846 |
| is_object_test.test | RUN | 0.127582 | 0.265928 | 0.391917 | 0.001667 | 0.001540 | 0.001665 |
| is_nothrow_swappable_test.test | RUN | 0.182370 | 0.476294 | 0.861532 | 0.001556 | 0.001754 | 0.001670 |
| is_nothrow_move_constructible_test.test | RUN | 0.181318 | 0.495142 | 0.879329 | 0.001765 | 0.001684 | 0.001504 |
| is_nothrow_move_assignable_test.test | RUN | 0.174772 | 0.481257 | 0.888740 | 0.001561 | 0.001707 | 0.001243 |
| is_noncopyable_test.test | RUN | 0.130854 | 0.270963 | 0.401318 | 0.001532 | 0.001460 | 0.001423 |
| is_member_pointer_test.test | RUN | 0.130074 | 0.271279 | 0.378349 | 0.001464 | 0.001521 | 0.001666 |
| is_member_obj_test.test | RUN | 0.123966 | 0.274977 | 0.424763 | 0.001716 | 0.001657 | 0.001442 |
| is_member_func_test.test | RUN | 0.129686 | 0.291201 | 0.427739 | 0.001314 | 0.001649 | 0.001617 |
| is_lvalue_reference_test.test | RUN | 0.130575 | 0.305356 | 0.488482 | 0.001640 | 0.001548 | 0.001378 |
| is_list_constructible_test.test | RUN | 0.152850 | 0.360622 | 0.516931 | 0.001482 | 0.001672 | 0.001754 |
| is_integral_test.test | RUN | 0.154175 | 0.433282 | 0.736788 | 0.001836 | 0.001715 | 0.001564 |
| is_fundamental_test.test | RUN | 0.155557 | 0.431773 | 0.808008 | 0.001476 | 0.001554 | 0.001449 |
| is_function_test.test | RUN | 0.125361 | 0.276120 | 0.411036 | 0.001560 | 0.001490 | 0.001774 |
| is_floating_point_test.test | RUN | 0.128327 | 0.320642 | 0.514869 | 0.001487 | 0.001404 | 0.001340 |
| is_float_test.test | RUN | 0.134605 | 0.322085 | 0.495779 | 0.001584 | 0.001672 | 0.001708 |
| is_final_test.test | RUN | 0.132087 | 0.318990 | 0.524171 | 0.001520 | 0.001562 | 0.001489 |
| is_enum_test.test | RUN | 0.123879 | 0.271102 | 0.386495 | 0.001564 | 0.001512 | 0.001680 |
| is_empty_test.test | RUN | 0.135381 | 0.300012 | 0.429083 | 0.001452 | 0.001580 | 0.001399 |
| is_detected_test.test | RUN | 0.120881 | 0.248476 | 0.328578 | 0.001757 | 0.001792 | 0.001627 |
| is_detected_exact_test.test | RUN | 0.119133 | 0.253647 | 0.319357 | 0.001626 | 0.001769 | 0.001760 |
| is_destructible_test.test | RUN | 0.180084 | 0.484188 | 0.912645 | 0.001677 | 0.001992 | 0.001629 |
| is_default_constr_test.test | RUN | 0.199244 | 0.493776 | 0.928196 | 0.001442 | 0.001834 | 0.001407 |
| is_copy_constructible_test.test | RUN | 0.179792 | 0.527969 | 0.959194 | 0.001733 | 0.001540 | 0.001716 |
| is_copy_assignable_test.test | RUN | 0.175825 | 0.482153 | 0.975802 | 0.001558 | 0.001710 | 0.001740 |
| is_convertible_test.test | RUN | 0.168621 | 0.439610 | 0.793981 | 0.001597 | 0.001907 | 0.001444 |
| is_constructible_test.test | RUN | 0.144529 | 0.337768 | 0.530820 | 0.001449 | 0.001945 | 0.001707 |
| is_const_test.test | RUN | 0.126488 | 0.292878 | 0.399051 | 0.001640 | 0.001520 | 0.001714 |
| is_compound_test.test | RUN | 0.117687 | 0.247309 | 0.355153 | 0.001390 | 0.001599 | 0.001618 |
| is_complex_test.test | RUN | 0.157590 | 0.333444 | 0.472583 | 0.001599 | 0.001940 | 0.001716 |
| is_complete_test.test | RUN | 0.142013 | 0.336662 | 0.604408 | 0.001482 | 0.001814 | 0.001706 |
| is_class_test.test | RUN | 0.127178 | 0.310008 | 0.475627 | 0.001402 | 0.001647 | 0.001396 |
| is_bounded_array_test.test | RUN | 0.121927 | 0.279095 | 0.427971 | 0.001706 | 0.001613 | 0.001433 |
| is_base_of_test.test | RUN | 0.133759 | 0.296074 | 0.456321 | 0.001612 | 0.001870 | 0.001674 |
| is_base_and_derived_test.test | RUN | 0.134783 | 0.289016 | 0.469695 | 0.001500 | 0.001722 | 0.001728 |
| is_assignable_test.test | RUN | 0.156461 | 0.414681 | 0.720431 | 0.001577 | 0.001842 | 0.001614 |
| is_array_test.test | RUN | 0.125758 | 0.277008 | 0.430048 | 0.001644 | 0.001909 | 0.001799 |
| is_arithmetic_test.test | RUN | 0.154743 | 0.428687 | 0.697966 | 0.001586 | 0.001696 | 0.001823 |
| is_abstract_test.test | RUN | 0.285144 | 1.195733 | 2.536883 | 0.001388 | 0.001646 | 0.001620 |
| integral_constant_test.test | RUN | 0.019315 | 0.050105 | 0.053468 | 0.000697 | 0.000694 | 0.001670 |
| has_virtual_destructor_test.test | RUN | 0.133563 | 0.350583 | 0.565768 | 0.001333 | 0.001888 | 0.001784 |
| has_unary_plus_test.test | RUN | 0.248091 | 1.294484 | 1.945466 | 0.001688 | 0.001802 | 0.001785 |
| has_unary_minus_test.test | RUN | 0.249589 | 1.245649 | 1.907073 | 0.001441 | 0.001590 | 0.001626 |
| has_trivial_move_constructor_test.test | RUN | 0.175171 | 0.506843 | 0.955225 | 0.001425 | 0.001529 | 0.001799 |
| has_trivial_move_assign_test.test | RUN | 0.178899 | 0.507379 | 0.854399 | 0.001433 | 0.001988 | 0.001679 |
| has_trivial_destructor_test.test | RUN | 0.178276 | 0.511761 | 0.946021 | 0.001719 | 0.001797 | 0.001763 |
| has_trivial_copy_test.test | RUN | 0.181918 | 0.560926 | 0.958816 | 0.001456 | 0.001857 | 0.001785 |
| has_trivial_constr_test.test | RUN | 0.182717 | 0.557924 | 0.954274 | 0.001368 | 0.001766 | 0.001874 |
| has_trivial_assign_test.test | RUN | 0.182075 | 0.579702 | 0.977335 | 0.001472 | 0.002157 | 0.001850 |
| has_right_shift_test.test | RUN | 0.258562 | 1.003965 | 2.047911 | 0.001712 | 0.001760 | 0.001620 |
| has_right_shift_assign_test.test | RUN | 0.250769 | 0.920529 | 1.937313 | 0.001619 | 0.001934 | 0.001660 |
| has_prefix_classes3_test.test | RUN | 0.186993 | 0.544476 | 1.019555 | 0.001466 | 0.001625 | 0.001694 |
| has_prefix_classes2_test.test | RUN | 0.189707 | 0.514774 | 1.019139 | 0.001297 | 0.001901 | 0.001708 |
| has_prefix_classes1_test.test | RUN | 0.186229 | 0.564439 | 0.918308 | 0.001750 | 0.001622 | 0.001649 |
| has_prefix_classes0_test.test | RUN | 0.185161 | 0.527674 | 1.014039 | 0.001307 | 0.001924 | 0.001689 |
| has_pre_increment_test.test | RUN | 0.262620 | 1.292111 | 1.922647 | 0.001644 | 0.001906 | 0.001797 |
| has_pre_decrement_test.test | RUN | 0.257095 | 1.262747 | 1.949658 | 0.001472 | 0.001674 | 0.001657 |
| has_postfix_classes3_test.test | RUN | 0.186756 | 0.543212 | 1.020116 | 0.001658 | 0.001782 | 0.001633 |
| has_postfix_classes2_test.test | RUN | 0.184516 | 0.544331 | 0.949379 | 0.001713 | 0.001536 | 0.001407 |
| has_postfix_classes1_test.test | RUN | 0.189822 | 0.564228 | 0.984333 | 0.001351 | 0.001671 | 0.001753 |
| has_postfix_classes0_test.test | RUN | 0.184850 | 0.539310 | 1.010506 | 0.001408 | 0.001895 | 0.001681 |
| has_post_increment_test.test | RUN | 0.243599 | 1.313689 | 1.955661 | 0.001720 | 0.001608 | 0.001788 |
| has_post_decrement_test.test | RUN | 0.245270 | 1.291826 | 1.965633 | 0.001427 | 0.001513 | 0.001462 |
| has_plus_test.test | RUN | 0.247028 | 0.925395 | 1.943882 | 0.001681 | 0.001788 | 0.001760 |
| has_plus_assign_test.test | RUN | 0.261328 | 0.921029 | 1.863631 | 0.001418 | 0.001428 | 0.001701 |
| has_operator_new_test.test | RUN | 0.173250 | 0.478320 | 0.900200 | 0.001703 | 0.001760 | 0.001550 |
| has_nothrow_destructor_test.test | RUN | 0.177740 | 0.524488 | 0.992885 | 0.001351 | 0.001790 | 0.001841 |
| has_nothrow_copy_test.test | RUN | 0.178719 | 0.487224 | 0.921696 | 0.001662 | 0.001716 | 0.001882 |
| has_nothrow_constr_test.test | RUN | 0.173147 | 0.477969 | 0.894916 | 0.001558 | 0.001852 | 0.001644 |
| has_nothrow_assign_test.test | RUN | 0.172285 | 0.507528 | 0.920896 | 0.001717 | 0.001922 | 0.001731 |
| has_not_equal_to_test.test | RUN | 0.251634 | 1.292177 | 1.979131 | 0.001713 | 0.001891 | 0.001822 |
| has_negate_test.test | RUN | 0.234630 | 0.901314 | 1.895574 | 0.001641 | 0.001674 | 0.001660 |
| has_multiplies_test.test | RUN | 0.252329 | 0.917250 | 1.934084 | 0.001584 | 0.001596 | 0.001551 |
| has_multiplies_assign_test.test | RUN | 0.247209 | 0.897595 | 1.911466 | 0.001760 | 0.001826 | 0.001652 |
| has_modulus_test.test | RUN | 0.246339 | 0.915633 | 1.924893 | 0.001624 | 0.001734 | 0.001673 |
| has_modulus_assign_test.test | RUN | 0.245177 | 0.913646 | 1.944593 | 0.001651 | 0.001578 | 0.001656 |
| has_minus_test.test | RUN | 0.252497 | 0.978835 | 1.935990 | 0.001546 | 0.001730 | 0.001689 |
| has_minus_assign_test.test | RUN | 0.254676 | 0.936285 | 1.938136 | 0.001810 | 0.002041 | 0.001662 |
| has_logical_or_test.test | RUN | 0.244783 | 0.964379 | 1.944208 | 0.001661 | 0.001448 | 0.001718 |
| has_logical_not_test.test | RUN | 0.239838 | 0.935156 | 1.887211 | 0.001648 | 0.001854 | 0.001704 |
| has_logical_and_test.test | RUN | 0.246433 | 1.019055 | 1.934549 | 0.001658 | 0.001894 | 0.001678 |
| has_less_test.test | RUN | 0.249589 | 1.297247 | 1.939674 | 0.001913 | 0.001940 | 0.001704 |
| has_less_equal_test.test | RUN | 0.252096 | 1.264262 | 1.967676 | 0.001488 | 0.001660 | 0.001680 |
| has_left_shift_test.test | RUN | 0.268260 | 1.196054 | 2.413173 | 0.001783 | 0.001786 | 0.001628 |
| has_left_shift_assign_test.test | RUN | 0.250774 | 0.971807 | 1.932227 | 0.001532 | 0.001689 | 0.001399 |
| has_greater_test.test | RUN | 0.249423 | 1.264824 | 1.972665 | 0.001844 | 0.001814 | 0.001694 |
| has_greater_equal_test.test | RUN | 0.251275 | 1.257507 | 1.917593 | 0.001704 | 0.001905 | 0.001756 |
| has_equal_to_test.test | RUN | 0.243927 | 1.223641 | 2.005946 | 0.001617 | 0.001874 | 0.001437 |
| has_divides_test.test | RUN | 0.250428 | 0.914315 | 1.983010 | 0.001647 | 0.001738 | 0.001740 |
| has_divides_assign_test.test | RUN | 0.251053 | 0.855041 | 1.903411 | 0.001842 | 0.001800 | 0.001429 |
| has_dereference_test.test | RUN | 0.246298 | 0.908104 | 1.852148 | 0.001724 | 0.001833 | 0.001779 |
| has_complement_test.test | RUN | 0.245000 | 1.324956 | 1.994937 | 0.001684 | 0.001866 | 0.001468 |
| has_bit_xor_test.test | RUN | 0.250740 | 0.923519 | 1.964101 | 0.001605 | 0.001802 | 0.001756 |
| has_bit_xor_assign_test.test | RUN | 0.250927 | 0.919691 | 1.950076 | 0.001785 | 0.002006 | 0.001414 |
| has_bit_or_test.test | RUN | 0.250983 | 0.863295 | 1.927082 | 0.001582 | 0.001925 | 0.001882 |
| has_bit_or_assign_test.test | RUN | 0.251768 | 0.935617 | 1.903479 | 0.001698 | 0.001760 | 0.001791 |
| has_bit_and_test.test | RUN | 0.246840 | 0.926736 | 1.965639 | 0.001602 | 0.001876 | 0.001452 |
| has_bit_and_assign_test.test | RUN | 0.249822 | 0.910714 | 1.992264 | 0.001867 | 0.001936 | 0.001383 |
| has_binary_classes9_test.test | RUN | 0.269921 | 0.987617 | 2.150021 | 0.001832 | 0.001879 | 0.001680 |
| has_binary_classes8_test.test | RUN | 0.271646 | 1.098877 | 2.136420 | 0.001712 | 0.001835 | 0.001504 |
| has_binary_classes7_test.test | RUN | 0.272114 | 1.073129 | 2.159120 | 0.001671 | 0.001732 | 0.001651 |
| has_binary_classes6_test.test | RUN | 0.271685 | 1.049739 | 2.174211 | 0.001727 | 0.002166 | 0.001696 |
| has_binary_classes5_test.test | RUN | 0.262046 | 1.046209 | 2.137551 | 0.001666 | 0.001917 | 0.001684 |
| has_binary_classes4_test.test | RUN | 0.267765 | 1.123508 | 2.176984 | 0.001737 | 0.001708 | 0.001618 |
| has_binary_classes3_test.test | RUN | 0.274353 | 1.078651 | 2.109858 | 0.001578 | 0.001814 | 0.001373 |
| has_binary_classes2_test.test | RUN | 0.275820 | 1.125886 | 2.153608 | 0.001624 | 0.001734 | 0.001644 |
| has_binary_classes1_test.test | RUN | 0.266564 | 1.088397 | 2.144863 | 0.001614 | 0.001867 | 0.001363 |
| has_binary_classes0_test.test | RUN | 0.270766 | 1.138724 | 2.088756 | 0.001659 | 0.001727 | 0.001632 |
| function_traits_test.test | RUN | 0.132347 | 0.293530 | 0.454736 | 0.001337 | 0.001900 | 0.001475 |
| floating_point_promotion_test.test | RUN | 0.125248 | 0.270099 | 0.421484 | 0.001741 | 0.001912 | 0.001596 |
| extent_test.test | RUN | 0.126874 | 0.280938 | 0.443657 | 0.001633 | 0.001836 | 0.001327 |
| enable_if_test.test | RUN | 0.116850 | 0.234652 | 0.350834 | 0.001489 | 0.001636 | 0.001238 |
| disjunction_test.test | RUN | 0.120714 | 0.257188 | 0.365037 | 0.001745 | 0.001948 | 0.001524 |
| detected_test.test | RUN | 0.119866 | 0.244741 | 0.341460 | 0.001715 | 0.001979 | 0.001537 |
| detected_or_test.test | RUN | 0.123574 | 0.254778 | 0.385409 | 0.001497 | 0.001642 | 0.001539 |
| decay_test.test | RUN | 0.296930 | 1.010748 | 2.322664 | 0.001371 | 0.001853 | 0.001629 |
| cxx14_aliases_test.test | RUN | 0.199240 | 0.368367 | 0.604027 | 0.001616 | 0.001689 | 0.001746 |
| copy_reference_test.test | RUN | 0.121298 | 0.247580 | 0.345115 | 0.001451 | 0.001808 | 0.001661 |
| copy_cv_test.test | RUN | 0.123299 | 0.275178 | 0.396184 | 0.001676 | 0.001959 | 0.001758 |
| copy_cv_ref_test.test | RUN | 0.169325 | 0.462907 | 0.954071 | 0.001697 | 0.001522 | 0.001743 |
| conjunction_test.test | RUN | 0.117417 | 0.264855 | 0.353116 | 0.001635 | 0.001650 | 0.001494 |
| conditional_test.test | RUN | 0.122653 | 0.232975 | 0.344602 | 0.001457 | 0.001665 | 0.001728 |
| common_type_test.test | RUN | 0.161856 | 0.405852 | 0.737376 | 0.001606 | 0.001640 | 0.001597 |
| common_type_sfinae_test.test | RUN | 0.124537 | 0.258587 | 0.338364 | 0.001656 | 0.001784 | 0.001694 |
| common_type_sfinae2_test.test | RUN | 0.123900 | 0.273670 | 0.340479 | 0.001660 | 0.001692 | 0.001395 |
| common_type_6_test.test | RUN | 0.128305 | 0.275417 | 0.390014 | 0.001539 | 0.001655 | 0.001450 |
| common_type_5_test.test | RUN | 0.128227 | 0.260111 | 0.354126 | 0.001740 | 0.001883 | 0.001630 |
| common_type_4_test.test | RUN | 0.131117 | 0.293812 | 0.451126 | 0.001662 | 0.001554 | 0.001527 |
| common_type_3_test.test | RUN | 0.119280 | 0.239208 | 0.341019 | 0.001501 | 0.001558 | 0.001596 |
| common_type_2_test.test | RUN | 0.160592 | 0.393933 | 0.703823 | 0.001608 | 0.001763 | 0.001551 |
| alignment_of_test.test | RUN | 0.132466 | 0.311975 | 0.461165 | 0.001702 | 0.001919 | 0.001492 |
| alignment_of_a2_test.test | RUN | 0.129290 | 0.314245 | 0.468341 | 0.001785 | 0.001428 | 0.001562 |
| aligned_storage_test.test | RUN | 0.137337 | 0.295132 | 0.408414 | 0.001782 | 0.002012 | 0.001440 |
| aligned_storage_empy_test.test | RUN | 0.152215 | 0.350207 | 0.567028 | 0.001739 | 0.001802 | 0.001319 |
| aligned_storage_a2_test.test | RUN | 0.135742 | 0.296857 | 0.410977 | 0.001454 | 0.002062 | 0.001773 |
| add_volatile_test.test | RUN | 0.261675 | 0.881346 | 2.164282 | 0.001588 | 0.001659 | 0.001404 |
| add_rvalue_reference_test.test | RUN | 0.295533 | 0.982406 | 2.446716 | 0.001385 | 0.001577 | 0.001722 |
| add_reference_test.test | RUN | 0.286776 | 0.964514 | 2.475625 | 0.001431 | 0.001752 | 0.001698 |
| add_pointer_test.test | RUN | 0.189487 | 0.547416 | 1.265969 | 0.001514 | 0.001581 | 0.001635 |
| add_lvalue_reference_test.test | RUN | 0.287490 | 0.964091 | 2.510033 | 0.001779 | 0.001616 | 0.001637 |
| add_cv_test.test | RUN | 0.270061 | 0.947233 | 2.332958 | 0.001541 | 0.001574 | 0.001880 |
| add_const_test.test | RUN | 0.270133 | 0.877628 | 2.290834 | 0.001520 | 0.001664 | 0.001704 |
| is_virtual_base_of_fail.test | COMPILE_FAIL | 0.118600 | 0.241580 | 0.335076 | — | — | — |
| is_stateless_fail.test | COMPILE_FAIL | 0.123101 | 0.241851 | 0.350941 | — | — | — |
| is_polymorphic_fail.test | COMPILE_FAIL | 0.112809 | 0.211215 | 0.331339 | — | — | — |
| is_nothrow_move_constructible_fail.test | COMPILE_FAIL | 0.119167 | 0.231046 | 0.352682 | — | — | — |
| is_nothrow_move_assignable_fail.test | COMPILE_FAIL | 0.114182 | 0.231488 | 0.335841 | — | — | — |
| is_list_constructible_fail.test | COMPILE_FAIL | 0.113872 | 0.227999 | 0.331688 | — | — | — |
| is_empty_fail.test | COMPILE_FAIL | 0.117342 | 0.235815 | 0.301812 | — | — | — |
| is_destructible_fail.test | COMPILE_FAIL | 0.115156 | 0.229017 | 0.319148 | — | — | — |
| is_default_constructible_fail.test | COMPILE_FAIL | 0.113315 | 0.226770 | 0.330842 | — | — | — |
| is_copy_constructible_fail.test | COMPILE_FAIL | 0.110527 | 0.222616 | 0.333174 | — | — | — |
| is_copy_assignable_fail.test | COMPILE_FAIL | 0.114535 | 0.228281 | 0.314800 | — | — | — |
| is_convertible_fail.test | COMPILE_FAIL | 0.115523 | 0.232550 | 0.318753 | — | — | — |
| is_convertible2_fail.test | COMPILE_FAIL | 0.116493 | 0.231856 | 0.330722 | — | — | — |
| is_constructible_fail.test | COMPILE_FAIL | 0.116964 | 0.227359 | 0.331115 | — | — | — |
| is_base_of_fail.test | COMPILE_FAIL | 0.114672 | 0.231127 | 0.312397 | — | — | — |
| is_assignable_fail.test | COMPILE_FAIL | 0.114360 | 0.227939 | 0.316252 | — | — | — |
| has_nothrow_destruct_fail.test | COMPILE_FAIL | 0.113135 | 0.225788 | 0.332762 | — | — | — |
| has_nothrow_copy_fail.test | COMPILE_FAIL | 0.113374 | 0.238030 | 0.325126 | — | — | — |
| has_nothrow_cons_fail.test | COMPILE_FAIL | 0.113398 | 0.228893 | 0.328942 | — | — | — |
| has_nothrow_assign_fail.test | COMPILE_FAIL | 0.114959 | 0.230411 | 0.318484 | — | — | — |
| common_type_fail.test | COMPILE_FAIL | 0.115948 | 0.228465 | 0.332610 | — | — | — |
| common_type2_fail.test | COMPILE_FAIL | 0.116159 | 0.236376 | 0.317201 | — | — | — |
| has_nothrow_assign_test_no_intrinsics.test | RUN | 0.180954 | 0.511010 | 0.948506 | 0.001707 | 0.001727 | 0.001697 |
| has_nothrow_constr_test_no_intrinsics.test | RUN | 0.172787 | 0.498473 | 0.928830 | 0.001688 | 0.001742 | 0.001755 |
| has_nothrow_copy_test_no_intrinsics.test | RUN | 0.175137 | 0.495318 | 0.897219 | 0.001367 | 0.001456 | 0.001258 |
| is_nothrow_move_assignable_test_no_intrinsics.test | RUN | 0.176107 | 0.494093 | 0.900805 | 0.001066 | 0.001408 | 0.001046 |
| is_nothrow_move_constructible_test_no_intrinsics.test | RUN | 0.112119 | 0.331165 | 0.569089 | 0.001322 | 0.001316 | 0.001260 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.006× | 1.011×, 1.001× | 1.006× |
| clang | 1.025× | 1.020×, 1.031× | 1.004× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| add_const_test.test | 10 | 0.001149s | 0.001152s | 0.001173s | byte equality observed |
| add_cv_test.test | 10 | 0.001134s | 0.001141s | 0.001171s | byte equality observed |
| add_lvalue_reference_test.test | 10 | 0.001167s | 0.001175s | 0.001182s | byte equality observed |
| add_pointer_test.test | 10 | 0.001184s | 0.001148s | 0.001178s | byte equality observed |
| add_reference_test.test | 10 | 0.001145s | 0.001170s | 0.001176s | byte equality observed |
| add_rvalue_reference_test.test | 10 | 0.001169s | 0.001169s | 0.001188s | byte equality observed |
| add_volatile_test.test | 10 | 0.001168s | 0.001190s | 0.001169s | byte equality observed |
| aligned_storage_a2_test.test | 10 | 0.001276s | 0.001272s | 0.001342s | byte equality observed |
| aligned_storage_empy_test.test | 10 | 0.001149s | 0.001180s | 0.001155s | byte equality observed |
| aligned_storage_test.test | 10 | 0.001273s | 0.001279s | 0.001285s | byte equality observed |
| alignment_of_a2_test.test | 10 | 0.001111s | 0.001172s | 0.001192s | byte equality observed |
| alignment_of_test.test | 10 | 0.001105s | 0.001176s | 0.001173s | byte equality observed |
| common_type_2_test.test | 10 | 0.001126s | 0.001180s | 0.001172s | byte equality observed |
| common_type_3_test.test | 10 | 0.001162s | 0.001169s | 0.001194s | byte equality observed |
| common_type_4_test.test | 10 | 0.001135s | 0.001169s | 0.001148s | byte equality observed |
| common_type_5_test.test | 10 | 0.001164s | 0.001170s | 0.001195s | byte equality observed |
| common_type_6_test.test | 10 | 0.001175s | 0.001167s | 0.001167s | byte equality observed |
| common_type_sfinae2_test.test | 10 | 0.001159s | 0.001161s | 0.001178s | byte equality observed |
| common_type_sfinae_test.test | 10 | 0.001165s | 0.001165s | 0.001188s | byte equality observed |
| common_type_test.test | 10 | 0.001148s | 0.001163s | 0.001194s | byte equality observed |
| conditional_test.test | 10 | 0.001150s | 0.001173s | 0.001161s | byte equality observed |
| conjunction_test.test | 10 | 0.001168s | 0.001178s | 0.001185s | byte equality observed |
| copy_cv_ref_test.test | 10 | 0.001159s | 0.001173s | 0.001188s | byte equality observed |
| copy_cv_test.test | 10 | 0.001163s | 0.001169s | 0.001186s | byte equality observed |
| copy_reference_test.test | 10 | 0.001156s | 0.001158s | 0.001162s | byte equality observed |
| cxx14_aliases_test.test | 10 | 0.001159s | 0.001189s | 0.001184s | byte equality observed |
| decay_test.test | 10 | 0.001174s | 0.001193s | 0.001173s | byte equality observed |
| detected_or_test.test | 10 | 0.001152s | 0.001161s | 0.001191s | byte equality observed |
| detected_test.test | 10 | 0.001167s | 0.001171s | 0.001182s | byte equality observed |
| disjunction_test.test | 10 | 0.001164s | 0.001158s | 0.001199s | byte equality observed |
| enable_if_test.test | 10 | 0.001152s | 0.001152s | 0.001178s | byte equality observed |
| extent_test.test | 10 | 0.001139s | 0.001175s | 0.001176s | byte equality observed |
| floating_point_promotion_test.test | 10 | 0.001164s | 0.001197s | 0.001198s | byte equality observed |
| function_traits_test.test | 10 | 0.001187s | 0.001176s | 0.001208s | byte equality observed |
| has_binary_classes0_test.test | 10 | 0.001174s | 0.001165s | 0.001168s | byte equality observed |
| has_binary_classes1_test.test | 10 | 0.001157s | 0.001158s | 0.001177s | byte equality observed |
| has_binary_classes2_test.test | 10 | 0.001174s | 0.001174s | 0.001164s | byte equality observed |
| has_binary_classes3_test.test | 10 | 0.001167s | 0.001179s | 0.001178s | byte equality observed |
| has_binary_classes4_test.test | 10 | 0.001172s | 0.001163s | 0.001210s | byte equality observed |
| has_binary_classes5_test.test | 10 | 0.001175s | 0.001190s | 0.001183s | byte equality observed |
| has_binary_classes6_test.test | 10 | 0.001169s | 0.001134s | 0.001166s | byte equality observed |
| has_binary_classes7_test.test | 10 | 0.001162s | 0.001162s | 0.001189s | byte equality observed |
| has_binary_classes8_test.test | 10 | 0.001177s | 0.001170s | 0.001167s | byte equality observed |
| has_binary_classes9_test.test | 10 | 0.001163s | 0.001170s | 0.001189s | byte equality observed |
| has_bit_and_assign_test.test | 10 | 0.001147s | 0.001182s | 0.001162s | byte equality observed |
| has_bit_and_test.test | 10 | 0.001172s | 0.001171s | 0.001164s | byte equality observed |
| has_bit_or_assign_test.test | 10 | 0.001182s | 0.001190s | 0.001194s | byte equality observed |
| has_bit_or_test.test | 10 | 0.001171s | 0.001189s | 0.001192s | byte equality observed |
| has_bit_xor_assign_test.test | 10 | 0.001164s | 0.001197s | 0.001173s | byte equality observed |
| has_bit_xor_test.test | 10 | 0.001190s | 0.001170s | 0.001198s | byte equality observed |
| has_complement_test.test | 10 | 0.001185s | 0.001152s | 0.001169s | byte equality observed |
| has_dereference_test.test | 10 | 0.001187s | 0.001162s | 0.001157s | byte equality observed |
| has_divides_assign_test.test | 10 | 0.001149s | 0.001174s | 0.001180s | byte equality observed |
| has_divides_test.test | 10 | 0.001191s | 0.001178s | 0.001179s | byte equality observed |
| has_equal_to_test.test | 10 | 0.001177s | 0.001170s | 0.001188s | byte equality observed |
| has_greater_equal_test.test | 10 | 0.001165s | 0.001154s | 0.001195s | byte equality observed |
| has_greater_test.test | 10 | 0.001188s | 0.001151s | 0.001185s | byte equality observed |
| has_left_shift_assign_test.test | 10 | 0.001184s | 0.001174s | 0.001178s | byte equality observed |
| has_left_shift_test.test | 10 | 0.001190s | 0.001199s | 0.001187s | byte equality observed |
| has_less_equal_test.test | 10 | 0.001173s | 0.001194s | 0.001196s | byte equality observed |
| has_less_test.test | 10 | 0.001175s | 0.001170s | 0.001179s | byte equality observed |
| has_logical_and_test.test | 10 | 0.001193s | 0.001183s | 0.001195s | byte equality observed |
| has_logical_not_test.test | 10 | 0.001192s | 0.001200s | 0.001178s | byte equality observed |
| has_logical_or_test.test | 10 | 0.001174s | 0.001184s | 0.001185s | byte equality observed |
| has_minus_assign_test.test | 10 | 0.001193s | 0.001156s | 0.001173s | byte equality observed |
| has_minus_test.test | 10 | 0.001167s | 0.001174s | 0.001180s | byte equality observed |
| has_modulus_assign_test.test | 10 | 0.001165s | 0.001165s | 0.001167s | byte equality observed |
| has_modulus_test.test | 10 | 0.001185s | 0.001191s | 0.001176s | byte equality observed |
| has_multiplies_assign_test.test | 10 | 0.001194s | 0.001198s | 0.001195s | byte equality observed |
| has_multiplies_test.test | 10 | 0.001192s | 0.001169s | 0.001179s | byte equality observed |
| has_negate_test.test | 10 | 0.001190s | 0.001186s | 0.001189s | byte equality observed |
| has_not_equal_to_test.test | 10 | 0.001207s | 0.001184s | 0.001178s | byte equality observed |
| has_nothrow_assign_test.test | 10 | 0.001183s | 0.001172s | 0.001187s | byte equality observed |
| has_nothrow_assign_test_no_intrinsics.test | 10 | 0.001198s | 0.001174s | 0.001188s | byte equality observed |
| has_nothrow_constr_test.test | 10 | 0.001161s | 0.001169s | 0.001188s | byte equality observed |
| has_nothrow_constr_test_no_intrinsics.test | 10 | 0.001187s | 0.001187s | 0.001180s | byte equality observed |
| has_nothrow_copy_test.test | 10 | 0.001178s | 0.001200s | 0.001182s | byte equality observed |
| has_nothrow_copy_test_no_intrinsics.test | 10 | 0.001170s | 0.001179s | 0.001208s | byte equality observed |
| has_nothrow_destructor_test.test | 10 | 0.001180s | 0.001182s | 0.001184s | byte equality observed |
| has_operator_new_test.test | 10 | 0.001184s | 0.001187s | 0.001170s | byte equality observed |
| has_plus_assign_test.test | 10 | 0.001185s | 0.001173s | 0.001180s | byte equality observed |
| has_plus_test.test | 10 | 0.001176s | 0.001160s | 0.001212s | byte equality observed |
| has_post_decrement_test.test | 10 | 0.001203s | 0.001172s | 0.001189s | byte equality observed |
| has_post_increment_test.test | 10 | 0.001194s | 0.001167s | 0.001199s | byte equality observed |
| has_postfix_classes0_test.test | 10 | 0.001186s | 0.001162s | 0.001165s | byte equality observed |
| has_postfix_classes1_test.test | 10 | 0.001188s | 0.001191s | 0.001191s | byte equality observed |
| has_postfix_classes2_test.test | 10 | 0.001163s | 0.001167s | 0.001184s | byte equality observed |
| has_postfix_classes3_test.test | 10 | 0.001183s | 0.001182s | 0.001182s | byte equality observed |
| has_pre_decrement_test.test | 10 | 0.001182s | 0.001195s | 0.001190s | byte equality observed |
| has_pre_increment_test.test | 10 | 0.001178s | 0.001171s | 0.001196s | byte equality observed |
| has_prefix_classes0_test.test | 10 | 0.001180s | 0.001176s | 0.001198s | byte equality observed |
| has_prefix_classes1_test.test | 10 | 0.001190s | 0.001180s | 0.001167s | byte equality observed |
| has_prefix_classes2_test.test | 10 | 0.001191s | 0.001181s | 0.001196s | byte equality observed |
| has_prefix_classes3_test.test | 10 | 0.001174s | 0.001185s | 0.001184s | byte equality observed |
| has_right_shift_assign_test.test | 10 | 0.001174s | 0.001183s | 0.001162s | byte equality observed |
| has_right_shift_test.test | 10 | 0.001185s | 0.001149s | 0.001172s | byte equality observed |
| has_trivial_assign_test.test | 10 | 0.001240s | 0.001240s | 0.001246s | byte equality observed |
| has_trivial_constr_test.test | 10 | 0.001227s | 0.001220s | 0.001239s | native test oracle; output varies |
| has_trivial_copy_test.test | 10 | 0.001262s | 0.001234s | 0.001241s | native test oracle; output varies |
| has_trivial_destructor_test.test | 10 | 0.001177s | 0.001108s | 0.001108s | byte equality observed |
| has_trivial_move_assign_test.test | 10 | 0.001148s | 0.001111s | 0.001133s | byte equality observed |
| has_trivial_move_constructor_test.test | 10 | 0.001168s | 0.001087s | 0.001161s | byte equality observed |
| has_unary_minus_test.test | 10 | 0.001182s | 0.001125s | 0.001158s | byte equality observed |
| has_unary_plus_test.test | 10 | 0.001194s | 0.001084s | 0.001185s | byte equality observed |
| has_virtual_destructor_test.test | 10 | 0.001170s | 0.001129s | 0.001172s | byte equality observed |
| integral_constant_test.test | 10 | 0.000539s | 0.000519s | 0.001159s | byte equality observed |
| is_abstract_test.test | 10 | 0.001180s | 0.001165s | 0.001175s | byte equality observed |
| is_arithmetic_test.test | 10 | 0.001195s | 0.001193s | 0.001179s | byte equality observed |
| is_array_test.test | 10 | 0.001167s | 0.001193s | 0.001175s | byte equality observed |
| is_assignable_test.test | 10 | 0.001183s | 0.001187s | 0.001163s | byte equality observed |
| is_base_and_derived_test.test | 10 | 0.001175s | 0.001194s | 0.001189s | byte equality observed |
| is_base_of_test.test | 10 | 0.001181s | 0.001173s | 0.001148s | byte equality observed |
| is_bounded_array_test.test | 10 | 0.001175s | 0.001173s | 0.001174s | byte equality observed |
| is_class_test.test | 10 | 0.001179s | 0.001174s | 0.001158s | byte equality observed |
| is_complete_test.test | 10 | 0.001182s | 0.001173s | 0.001159s | byte equality observed |
| is_complex_test.test | 10 | 0.001172s | 0.001188s | 0.001156s | byte equality observed |
| is_compound_test.test | 10 | 0.001176s | 0.001177s | 0.001156s | byte equality observed |
| is_const_test.test | 10 | 0.001184s | 0.001196s | 0.001155s | byte equality observed |
| is_constructible_test.test | 10 | 0.001178s | 0.001157s | 0.001177s | byte equality observed |
| is_convertible_test.test | 10 | 0.001196s | 0.001182s | 0.001174s | byte equality observed |
| is_copy_assignable_test.test | 10 | 0.001182s | 0.001192s | 0.001184s | byte equality observed |
| is_copy_constructible_test.test | 10 | 0.001189s | 0.001188s | 0.001177s | byte equality observed |
| is_default_constr_test.test | 10 | 0.001188s | 0.001168s | 0.001165s | byte equality observed |
| is_destructible_test.test | 10 | 0.001171s | 0.001174s | 0.001180s | byte equality observed |
| is_detected_exact_test.test | 10 | 0.001161s | 0.001161s | 0.001162s | byte equality observed |
| is_detected_test.test | 10 | 0.001175s | 0.001182s | 0.001184s | byte equality observed |
| is_empty_test.test | 10 | 0.001160s | 0.001175s | 0.001166s | byte equality observed |
| is_enum_test.test | 10 | 0.001159s | 0.001173s | 0.001183s | byte equality observed |
| is_final_test.test | 10 | 0.001171s | 0.001159s | 0.001167s | byte equality observed |
| is_float_test.test | 10 | 0.001183s | 0.001153s | 0.001183s | byte equality observed |
| is_floating_point_test.test | 10 | 0.001176s | 0.001191s | 0.001155s | byte equality observed |
| is_function_test.test | 10 | 0.001171s | 0.001173s | 0.001184s | byte equality observed |
| is_fundamental_test.test | 10 | 0.001191s | 0.001174s | 0.001148s | byte equality observed |
| is_integral_test.test | 10 | 0.001173s | 0.001165s | 0.001166s | byte equality observed |
| is_list_constructible_test.test | 10 | 0.001178s | 0.001195s | 0.001166s | byte equality observed |
| is_lvalue_reference_test.test | 10 | 0.001177s | 0.001174s | 0.001173s | byte equality observed |
| is_member_func_test.test | 10 | 0.001193s | 0.001180s | 0.001158s | byte equality observed |
| is_member_obj_test.test | 10 | 0.001175s | 0.001199s | 0.001161s | byte equality observed |
| is_member_pointer_test.test | 10 | 0.001170s | 0.001163s | 0.001176s | byte equality observed |
| is_noncopyable_test.test | 10 | 0.001170s | 0.001166s | 0.001163s | byte equality observed |
| is_nothrow_move_assignable_test.test | 10 | 0.001184s | 0.001203s | 0.001155s | byte equality observed |
| is_nothrow_move_assignable_test_no_intrinsics.test | 10 | 0.001172s | 0.001161s | 0.001184s | byte equality observed |
| is_nothrow_move_constructible_test.test | 10 | 0.001159s | 0.001193s | 0.001176s | byte equality observed |
| is_nothrow_move_constructible_test_no_intrinsics.test | 10 | 0.001183s | 0.001188s | 0.001193s | byte equality observed |
| is_nothrow_swappable_test.test | 10 | 0.001183s | 0.001162s | 0.001161s | byte equality observed |
| is_object_test.test | 10 | 0.001161s | 0.001179s | 0.001170s | byte equality observed |
| is_pod_test.test | 10 | 0.001167s | 0.001202s | 0.001170s | byte equality observed |
| is_pointer_test.test | 10 | 0.001182s | 0.001185s | 0.001167s | byte equality observed |
| is_polymorphic_test.test | 10 | 0.001193s | 0.001170s | 0.001160s | byte equality observed |
| is_reference_test.test | 10 | 0.001180s | 0.001171s | 0.001182s | byte equality observed |
| is_rvalue_reference_test.test | 10 | 0.001177s | 0.001179s | 0.001163s | byte equality observed |
| is_same_test.test | 10 | 0.001179s | 0.001161s | 0.001154s | byte equality observed |
| is_scalar_test.test | 10 | 0.001166s | 0.001170s | 0.001177s | byte equality observed |
| is_scoped_enum_test.test | 10 | 0.001163s | 0.001190s | 0.001173s | byte equality observed |
| is_signed_test.test | 10 | 0.001164s | 0.001155s | 0.001182s | byte equality observed |
| is_stateless_test.test | 10 | 0.001174s | 0.001192s | 0.001172s | byte equality observed |
| is_swappable_test.test | 10 | 0.001174s | 0.001153s | 0.001172s | byte equality observed |
| is_trivially_copyable_test.test | 10 | 0.001224s | 0.001197s | 0.001201s | byte equality observed |
| is_unbounded_array_test.test | 10 | 0.001160s | 0.001174s | 0.001144s | byte equality observed |
| is_union_test.test | 10 | 0.001180s | 0.001185s | 0.001155s | byte equality observed |
| is_unscoped_enum_test.test | 10 | 0.001178s | 0.001165s | 0.001146s | byte equality observed |
| is_unsigned_test.test | 10 | 0.001181s | 0.001179s | 0.001179s | byte equality observed |
| is_virtual_base_of_test.test | 10 | 0.001168s | 0.001170s | 0.001173s | byte equality observed |
| is_void_test.test | 10 | 0.001166s | 0.001172s | 0.001171s | byte equality observed |
| is_volatile_test.test | 10 | 0.001177s | 0.001175s | 0.001158s | byte equality observed |
| make_signed_test.test | 10 | 0.001197s | 0.001181s | 0.001167s | byte equality observed |
| make_unsigned_test.test | 10 | 0.001200s | 0.001193s | 0.001160s | byte equality observed |
| make_void_test.test | 10 | 0.001196s | 0.001200s | 0.001173s | byte equality observed |
| mpl_interop_test1.test | 10 | 0.000543s | 0.000541s | 0.001149s | byte equality observed |
| mpl_interop_test2.test | 10 | 0.000536s | 0.000530s | 0.001177s | byte equality observed |
| mpl_interop_test3.test | 10 | 0.000533s | 0.000545s | 0.001159s | byte equality observed |
| negation_test.test | 10 | 0.001206s | 0.001232s | 0.001174s | byte equality observed |
| promote_basic_test.test | 10 | 0.000538s | 0.000537s | 0.001187s | byte equality observed |
| promote_enum_msvc_bug_test.test | 10 | 0.000533s | 0.000538s | 0.001189s | byte equality observed |
| promote_enum_test.test | 10 | 0.000546s | 0.000538s | 0.001188s | byte equality observed |
| promote_mpl_test.test | 10 | 0.000540s | 0.000541s | 0.001159s | byte equality observed |
| rank_test.test | 10 | 0.001193s | 0.001186s | 0.001144s | byte equality observed |
| remove_all_extents_test.test | 10 | 0.001195s | 0.001164s | 0.001164s | byte equality observed |
| remove_bounds_test.test | 10 | 0.001188s | 0.001173s | 0.001181s | byte equality observed |
| remove_const_test.test | 10 | 0.001187s | 0.001155s | 0.001168s | byte equality observed |
| remove_cv_ref_test.test | 10 | 0.001167s | 0.001164s | 0.001179s | byte equality observed |
| remove_cv_test.test | 10 | 0.001179s | 0.001176s | 0.001152s | byte equality observed |
| remove_extent_test.test | 10 | 0.001189s | 0.001181s | 0.001166s | byte equality observed |
| remove_pointer_test.test | 10 | 0.001179s | 0.001193s | 0.001175s | byte equality observed |
| remove_reference_test.test | 10 | 0.001183s | 0.001196s | 0.001171s | byte equality observed |
| remove_volatile_test.test | 10 | 0.001175s | 0.001185s | 0.001169s | byte equality observed |
| tricky_abstract_type_test.test | 10 | 0.001169s | 0.001167s | 0.001150s | byte equality observed |
| tricky_add_pointer_test.test | 10 | 0.001163s | 0.001183s | 0.001175s | byte equality observed |
| tricky_function_type_test.test | 10 | 0.001191s | 0.001179s | 0.001172s | byte equality observed |
| tricky_incomplete_type_test.test | 10 | 0.001170s | 0.001152s | 0.001172s | byte equality observed |
| tricky_is_enum_test.test | 10 | 0.001175s | 0.001169s | 0.001158s | byte equality observed |
| tricky_partial_spec_test.test | 10 | 0.001187s | 0.001185s | 0.001171s | byte equality observed |
| tricky_rvalue_test.test | 10 | 0.001186s | 0.001174s | 0.001167s | byte equality observed |
| type_identity_test.test | 10 | 0.001187s | 0.001176s | 0.001191s | byte equality observed |
| type_traits_test.test | 10 | 0.000531s | 0.000525s | 0.001178s | byte equality observed |
| type_with_alignment_test.test | 10 | 0.001318s | 0.001312s | 0.001306s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
