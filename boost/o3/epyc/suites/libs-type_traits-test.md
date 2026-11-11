# libs/type_traits/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| type_with_alignment_test.test | RUN | 0.116382 | 0.268510 | 0.377416 | 0.001650 | 0.001682 | 0.001832 |
| type_traits_test.test | RUN | 0.161854 | 0.293043 | 0.389223 | 0.000692 | 0.000587 | 0.001810 |
| type_identity_test.test | RUN | 0.262585 | 0.913388 | 1.931016 | 0.001729 | 0.001518 | 0.001700 |
| tricky_rvalue_test.test | RUN | 0.107287 | 0.217048 | 0.303457 | 0.001554 | 0.001682 | 0.002019 |
| tricky_partial_spec_test.test | RUN | 0.129973 | 0.319673 | 0.492597 | 0.001361 | 0.001757 | 0.001902 |
| tricky_is_enum_test.test | RUN | 0.105474 | 0.217947 | 0.294899 | 0.001748 | 0.001822 | 0.002083 |
| tricky_incomplete_type_test.test | RUN | 0.174846 | 0.326060 | 0.464389 | 0.001746 | 0.001620 | 0.001618 |
| tricky_function_type_test.test | RUN | 0.138069 | 0.361606 | 0.536968 | 0.002493 | 0.001536 | 0.001801 |
| tricky_add_pointer_test.test | RUN | 0.181821 | 0.568939 | 1.114385 | 0.001642 | 0.001816 | 0.001932 |
| tricky_abstract_type_test.test | RUN | 0.112376 | 0.219043 | 0.295854 | 0.002010 | 0.002101 | 0.001838 |
| remove_volatile_test.test | RUN | 0.237609 | 0.794838 | 1.669371 | 0.001739 | 0.001881 | 0.001900 |
| remove_reference_test.test | RUN | 0.245366 | 0.851100 | 1.778326 | 0.001773 | 0.001906 | 0.001616 |
| remove_pointer_test.test | RUN | 0.247441 | 0.879618 | 1.856154 | 0.001763 | 0.001579 | 0.001894 |
| remove_extent_test.test | RUN | 0.243809 | 0.865367 | 1.773818 | 0.001640 | 0.001839 | 0.001807 |
| remove_cv_test.test | RUN | 0.241998 | 0.885309 | 1.792000 | 0.001858 | 0.001853 | 0.002049 |
| remove_cv_ref_test.test | RUN | 0.473893 | 2.034465 | 4.268386 | 0.001748 | 0.001557 | 0.001677 |
| remove_const_test.test | RUN | 0.231439 | 0.798583 | 1.737829 | 0.001769 | 0.001872 | 0.001954 |
| remove_bounds_test.test | RUN | 0.238967 | 0.873367 | 1.832691 | 0.001425 | 0.002000 | 0.001649 |
| remove_all_extents_test.test | RUN | 0.239554 | 0.830858 | 1.625254 | 0.001771 | 0.001522 | 0.001787 |
| rank_test.test | RUN | 0.112512 | 0.222418 | 0.309749 | 0.001481 | 0.001889 | 0.001908 |
| promote_mpl_test.test | RUN | 0.091353 | 0.189386 | 0.223460 | 0.000599 | 0.000544 | 0.001871 |
| promote_enum_test.test | RUN | 0.022672 | 0.049576 | 0.059207 | 0.000724 | 0.000571 | 0.001964 |
| promote_enum_msvc_bug_test.test | RUN | 0.021049 | 0.044423 | 0.052566 | 0.000632 | 0.000766 | 0.001916 |
| promote_basic_test.test | RUN | 0.025082 | 0.057091 | 0.066432 | 0.000780 | 0.000561 | 0.002228 |
| negation_test.test | RUN | 0.106235 | 0.194990 | 0.278521 | 0.001487 | 0.001780 | 0.001768 |
| mpl_interop_test3.test | RUN | 0.088016 | 0.230016 | 0.183478 | 0.000601 | 0.001227 | 0.001646 |
| mpl_interop_test2.test | RUN | 0.037494 | 0.061695 | 0.058844 | 0.000732 | 0.001417 | 0.001740 |
| mpl_interop_test1.test | RUN | 0.017487 | 0.039466 | 0.047601 | 0.000618 | 0.000778 | 0.001798 |
| make_void_test.test | RUN | 0.106896 | 0.274746 | 0.293909 | 0.001711 | 0.001690 | 0.001956 |
| make_unsigned_test.test | RUN | 0.127478 | 0.321826 | 0.454551 | 0.001604 | 0.001763 | 0.002380 |
| make_signed_test.test | RUN | 0.124080 | 0.303703 | 0.429626 | 0.001600 | 0.001786 | 0.001996 |
| is_volatile_test.test | RUN | 0.118953 | 0.265033 | 0.362950 | 0.001427 | 0.001761 | 0.001750 |
| is_void_test.test | RUN | 0.109903 | 0.236806 | 0.337385 | 0.001868 | 0.001901 | 0.001778 |
| is_virtual_base_of_test.test | RUN | 0.123477 | 0.301206 | 0.447982 | 0.001786 | 0.001598 | 0.001514 |
| is_unsigned_test.test | RUN | 0.109527 | 0.245901 | 0.334388 | 0.001978 | 0.001509 | 0.001765 |
| is_unscoped_enum_test.test | RUN | 0.107556 | 0.212007 | 0.288718 | 0.001437 | 0.001468 | 0.001692 |
| is_union_test.test | RUN | 0.120889 | 0.262160 | 0.388371 | 0.001784 | 0.002015 | 0.001706 |
| is_unbounded_array_test.test | RUN | 0.107462 | 0.227105 | 0.326830 | 0.001548 | 0.001755 | 0.001976 |
| is_trivially_copyable_test.test | RUN | 0.168914 | 0.456247 | 0.766294 | 0.001935 | 0.001862 | 0.001790 |
| is_swappable_test.test | RUN | 0.142049 | 0.345518 | 0.596805 | 0.001543 | 0.001888 | 0.001830 |
| is_stateless_test.test | RUN | 0.161755 | 0.388674 | 0.685858 | 0.001708 | 0.001689 | 0.001476 |
| is_signed_test.test | RUN | 0.114255 | 0.227356 | 0.355104 | 0.001875 | 0.001763 | 0.001870 |
| is_scoped_enum_test.test | RUN | 0.109077 | 0.213118 | 0.297246 | 0.001682 | 0.001730 | 0.001908 |
| is_scalar_test.test | RUN | 0.139908 | 0.380017 | 0.565274 | 0.001362 | 0.001964 | 0.001593 |
| is_same_test.test | RUN | 0.105868 | 0.228914 | 0.315000 | 0.001779 | 0.001947 | 0.001873 |
| is_rvalue_reference_test.test | RUN | 0.111920 | 0.256033 | 0.359552 | 0.001636 | 0.001979 | 0.001694 |
| is_reference_test.test | RUN | 0.112284 | 0.270713 | 0.361508 | 0.001489 | 0.001868 | 0.001820 |
| is_polymorphic_test.test | RUN | 0.117576 | 0.272605 | 0.394768 | 0.001452 | 0.001908 | 0.001870 |
| is_pointer_test.test | RUN | 0.113110 | 0.271829 | 0.353406 | 0.001573 | 0.001780 | 0.001756 |
| is_pod_test.test | RUN | 0.145547 | 0.449587 | 0.672147 | 0.001617 | 0.001725 | 0.001940 |
| is_object_test.test | RUN | 0.105491 | 0.220384 | 0.314442 | 0.001589 | 0.001856 | 0.002070 |
| is_nothrow_swappable_test.test | RUN | 0.152379 | 0.392545 | 0.631054 | 0.001700 | 0.001868 | 0.001475 |
| is_nothrow_move_constructible_test.test | RUN | 0.147795 | 0.410852 | 0.651095 | 0.001522 | 0.001672 | 0.001622 |
| is_nothrow_move_assignable_test.test | RUN | 0.148689 | 0.415921 | 0.632131 | 0.001563 | 0.001968 | 0.001945 |
| is_noncopyable_test.test | RUN | 0.108950 | 0.233305 | 0.310041 | 0.001710 | 0.001462 | 0.001524 |
| is_member_pointer_test.test | RUN | 0.113271 | 0.227232 | 0.312615 | 0.001666 | 0.001603 | 0.001780 |
| is_member_obj_test.test | RUN | 0.106265 | 0.229994 | 0.316718 | 0.001860 | 0.001646 | 0.001424 |
| is_member_func_test.test | RUN | 0.108183 | 0.241154 | 0.311108 | 0.001648 | 0.001537 | 0.001605 |
| is_lvalue_reference_test.test | RUN | 0.112918 | 0.250624 | 0.357784 | 0.001786 | 0.001968 | 0.001842 |
| is_list_constructible_test.test | RUN | 0.134111 | 0.303911 | 0.418037 | 0.001481 | 0.001743 | 0.001820 |
| is_integral_test.test | RUN | 0.135358 | 0.356357 | 0.518423 | 0.001896 | 0.001719 | 0.001820 |
| is_fundamental_test.test | RUN | 0.135572 | 0.372601 | 0.566482 | 0.001757 | 0.001450 | 0.001571 |
| is_function_test.test | RUN | 0.114573 | 0.230400 | 0.317752 | 0.001527 | 0.001487 | 0.001990 |
| is_floating_point_test.test | RUN | 0.118964 | 0.294745 | 0.349468 | 0.001630 | 0.001680 | 0.001807 |
| is_float_test.test | RUN | 0.120065 | 0.294360 | 0.380684 | 0.001813 | 0.002017 | 0.001786 |
| is_final_test.test | RUN | 0.123514 | 0.282522 | 0.381954 | 0.001619 | 0.001451 | 0.001754 |
| is_enum_test.test | RUN | 0.114410 | 0.241641 | 0.286672 | 0.001395 | 0.001359 | 0.001786 |
| is_empty_test.test | RUN | 0.123978 | 0.267944 | 0.343860 | 0.001588 | 0.001996 | 0.001485 |
| is_detected_test.test | RUN | 0.107969 | 0.223412 | 0.270641 | 0.001963 | 0.001381 | 0.001910 |
| is_detected_exact_test.test | RUN | 0.106218 | 0.223923 | 0.263851 | 0.001696 | 0.001723 | 0.001645 |
| is_destructible_test.test | RUN | 0.147710 | 0.413356 | 0.612457 | 0.001634 | 0.001812 | 0.001764 |
| is_default_constr_test.test | RUN | 0.146259 | 0.430328 | 0.648989 | 0.001477 | 0.001766 | 0.001804 |
| is_copy_constructible_test.test | RUN | 0.157351 | 0.460563 | 0.661414 | 0.001852 | 0.001508 | 0.001798 |
| is_copy_assignable_test.test | RUN | 0.153451 | 0.445305 | 0.683575 | 0.001502 | 0.001567 | 0.001883 |
| is_convertible_test.test | RUN | 0.147198 | 0.406008 | 0.538992 | 0.001664 | 0.001881 | 0.001904 |
| is_constructible_test.test | RUN | 0.124582 | 0.274327 | 0.408748 | 0.001512 | 0.001894 | 0.001615 |
| is_const_test.test | RUN | 0.113276 | 0.245351 | 0.346255 | 0.001794 | 0.001698 | 0.001498 |
| is_compound_test.test | RUN | 0.104342 | 0.217550 | 0.263557 | 0.002068 | 0.001817 | 0.001678 |
| is_complex_test.test | RUN | 0.138814 | 0.276421 | 0.376390 | 0.001361 | 0.001874 | 0.001942 |
| is_complete_test.test | RUN | 0.123801 | 0.289555 | 0.402608 | 0.001932 | 0.001469 | 0.001472 |
| is_class_test.test | RUN | 0.118004 | 0.251779 | 0.364904 | 0.001586 | 0.001394 | 0.001492 |
| is_bounded_array_test.test | RUN | 0.110032 | 0.232990 | 0.317821 | 0.001630 | 0.001746 | 0.001497 |
| is_base_of_test.test | RUN | 0.118812 | 0.251373 | 0.336338 | 0.001498 | 0.001815 | 0.001888 |
| is_base_and_derived_test.test | RUN | 0.116991 | 0.258261 | 0.349780 | 0.001476 | 0.001615 | 0.001766 |
| is_assignable_test.test | RUN | 0.133336 | 0.361594 | 0.508147 | 0.001517 | 0.001776 | 0.001816 |
| is_array_test.test | RUN | 0.108183 | 0.247614 | 0.322478 | 0.001787 | 0.001460 | 0.001626 |
| is_arithmetic_test.test | RUN | 0.133433 | 0.361817 | 0.495143 | 0.001876 | 0.001902 | 0.001904 |
| is_abstract_test.test | RUN | 0.237517 | 1.028634 | 1.778024 | 0.001762 | 0.001497 | 0.001851 |
| integral_constant_test.test | RUN | 0.017719 | 0.038923 | 0.048596 | 0.000777 | 0.000790 | 0.001941 |
| has_virtual_destructor_test.test | RUN | 0.119697 | 0.282395 | 0.412047 | 0.001929 | 0.001860 | 0.001819 |
| has_unary_plus_test.test | RUN | 0.212645 | 1.089533 | 1.399094 | 0.001805 | 0.001947 | 0.001918 |
| has_unary_minus_test.test | RUN | 0.206805 | 1.071379 | 1.383120 | 0.001899 | 0.001580 | 0.001984 |
| has_trivial_move_constructor_test.test | RUN | 0.152205 | 0.404439 | 0.689262 | 0.001849 | 0.001534 | 0.001874 |
| has_trivial_move_assign_test.test | RUN | 0.155391 | 0.405111 | 0.670684 | 0.001860 | 0.001923 | 0.001994 |
| has_trivial_destructor_test.test | RUN | 0.158195 | 0.414152 | 0.703850 | 0.001672 | 0.001684 | 0.002058 |
| has_trivial_copy_test.test | RUN | 0.157295 | 0.434593 | 0.721230 | 0.002072 | 0.001930 | 0.002082 |
| has_trivial_constr_test.test | RUN | 0.162018 | 0.452855 | 0.709931 | 0.002266 | 0.001810 | 0.002136 |
| has_trivial_assign_test.test | RUN | 0.160493 | 0.439515 | 0.711055 | 0.001770 | 0.001926 | 0.001990 |
| has_right_shift_test.test | RUN | 0.218495 | 0.861891 | 1.482140 | 0.001868 | 0.001684 | 0.001525 |
| has_right_shift_assign_test.test | RUN | 0.217320 | 0.758844 | 1.407007 | 0.001513 | 0.001599 | 0.001480 |
| has_prefix_classes3_test.test | RUN | 0.168527 | 0.455532 | 0.687002 | 0.001525 | 0.001502 | 0.001948 |
| has_prefix_classes2_test.test | RUN | 0.163181 | 0.433437 | 0.695346 | 0.001786 | 0.001819 | 0.001650 |
| has_prefix_classes1_test.test | RUN | 0.164283 | 0.445419 | 0.704292 | 0.001782 | 0.001636 | 0.001940 |
| has_prefix_classes0_test.test | RUN | 0.157854 | 0.447672 | 0.729324 | 0.001710 | 0.001668 | 0.001786 |
| has_pre_increment_test.test | RUN | 0.208094 | 1.042237 | 1.355029 | 0.001858 | 0.001737 | 0.001535 |
| has_pre_decrement_test.test | RUN | 0.203298 | 1.040401 | 1.358518 | 0.001916 | 0.001665 | 0.001449 |
| has_postfix_classes3_test.test | RUN | 0.159374 | 0.455209 | 0.768443 | 0.001991 | 0.001620 | 0.001942 |
| has_postfix_classes2_test.test | RUN | 0.161315 | 0.451876 | 0.768055 | 0.001778 | 0.001534 | 0.001928 |
| has_postfix_classes1_test.test | RUN | 0.165779 | 0.455333 | 0.709759 | 0.001658 | 0.001867 | 0.001910 |
| has_postfix_classes0_test.test | RUN | 0.163246 | 0.484343 | 0.775158 | 0.001845 | 0.001449 | 0.001936 |
| has_post_increment_test.test | RUN | 0.203763 | 1.087093 | 1.414789 | 0.001861 | 0.001680 | 0.001781 |
| has_post_decrement_test.test | RUN | 0.206333 | 1.084069 | 1.375038 | 0.001764 | 0.001458 | 0.001829 |
| has_plus_test.test | RUN | 0.209945 | 0.795766 | 1.320262 | 0.001769 | 0.001957 | 0.001780 |
| has_plus_assign_test.test | RUN | 0.217371 | 0.782040 | 1.356383 | 0.001712 | 0.001837 | 0.001751 |
| has_operator_new_test.test | RUN | 0.151901 | 0.418909 | 0.605354 | 0.001812 | 0.001879 | 0.001726 |
| has_nothrow_destructor_test.test | RUN | 0.161922 | 0.464977 | 0.721229 | 0.002094 | 0.001561 | 0.001708 |
| has_nothrow_copy_test.test | RUN | 0.150425 | 0.416988 | 0.644368 | 0.001492 | 0.001464 | 0.001886 |
| has_nothrow_constr_test.test | RUN | 0.148476 | 0.408237 | 0.655852 | 0.001556 | 0.001881 | 0.001878 |
| has_nothrow_assign_test.test | RUN | 0.154208 | 0.410457 | 0.619597 | 0.001874 | 0.001898 | 0.001700 |
| has_not_equal_to_test.test | RUN | 0.208112 | 1.053300 | 1.362281 | 0.001768 | 0.001512 | 0.002035 |
| has_negate_test.test | RUN | 0.210181 | 0.784449 | 1.319245 | 0.001761 | 0.001798 | 0.001792 |
| has_multiplies_test.test | RUN | 0.214958 | 0.832172 | 1.297470 | 0.001525 | 0.001538 | 0.001876 |
| has_multiplies_assign_test.test | RUN | 0.209658 | 0.766325 | 1.331782 | 0.001933 | 0.001847 | 0.001843 |
| has_modulus_test.test | RUN | 0.209589 | 0.785684 | 1.330870 | 0.001792 | 0.001599 | 0.001792 |
| has_modulus_assign_test.test | RUN | 0.213989 | 0.765421 | 1.351158 | 0.002016 | 0.001542 | 0.001811 |
| has_minus_test.test | RUN | 0.223372 | 0.801197 | 1.343800 | 0.001748 | 0.001450 | 0.001952 |
| has_minus_assign_test.test | RUN | 0.224906 | 0.758613 | 1.372350 | 0.001886 | 0.001824 | 0.001956 |
| has_logical_or_test.test | RUN | 0.216260 | 0.821300 | 1.291976 | 0.001930 | 0.001476 | 0.002006 |
| has_logical_not_test.test | RUN | 0.207034 | 0.787772 | 1.328253 | 0.002161 | 0.001865 | 0.001726 |
| has_logical_and_test.test | RUN | 0.213812 | 0.790462 | 1.345794 | 0.001674 | 0.001775 | 0.001660 |
| has_less_test.test | RUN | 0.219648 | 1.044946 | 1.341455 | 0.001767 | 0.001808 | 0.001697 |
| has_less_equal_test.test | RUN | 0.212830 | 1.021479 | 1.340063 | 0.002036 | 0.002003 | 0.001769 |
| has_left_shift_test.test | RUN | 0.226221 | 1.004602 | 1.688352 | 0.001826 | 0.001530 | 0.001368 |
| has_left_shift_assign_test.test | RUN | 0.208323 | 0.809507 | 1.351049 | 0.001718 | 0.001652 | 0.001619 |
| has_greater_test.test | RUN | 0.211606 | 1.013466 | 1.279055 | 0.001897 | 0.001730 | 0.001153 |
| has_greater_equal_test.test | RUN | 0.205978 | 1.007812 | 1.321738 | 0.001760 | 0.001885 | 0.001900 |
| has_equal_to_test.test | RUN | 0.206130 | 1.027967 | 1.381565 | 0.001773 | 0.002046 | 0.001400 |
| has_divides_test.test | RUN | 0.206586 | 0.789056 | 1.340600 | 0.001545 | 0.001628 | 0.001507 |
| has_divides_assign_test.test | RUN | 0.212675 | 0.807492 | 1.326812 | 0.001707 | 0.001811 | 0.001290 |
| has_dereference_test.test | RUN | 0.210350 | 0.831835 | 1.227007 | 0.001930 | 0.001763 | 0.001808 |
| has_complement_test.test | RUN | 0.238578 | 1.032231 | 1.318910 | 0.001932 | 0.001748 | 0.001644 |
| has_bit_xor_test.test | RUN | 0.236815 | 0.794375 | 1.252401 | 0.001461 | 0.001865 | 0.001668 |
| has_bit_xor_assign_test.test | RUN | 0.237789 | 0.772801 | 1.318808 | 0.001756 | 0.001926 | 0.001402 |
| has_bit_or_test.test | RUN | 0.209481 | 0.848380 | 1.285105 | 0.001352 | 0.001762 | 0.001910 |
| has_bit_or_assign_test.test | RUN | 0.206893 | 0.845825 | 1.336504 | 0.001870 | 0.001787 | 0.001470 |
| has_bit_and_test.test | RUN | 0.207546 | 0.800333 | 1.266126 | 0.001655 | 0.001334 | 0.001747 |
| has_bit_and_assign_test.test | RUN | 0.211387 | 0.771674 | 1.319057 | 0.002124 | 0.001897 | 0.001384 |
| has_binary_classes9_test.test | RUN | 0.220261 | 0.968818 | 1.440194 | 0.001729 | 0.001862 | 0.001852 |
| has_binary_classes8_test.test | RUN | 0.224971 | 0.965332 | 1.457318 | 0.001837 | 0.001537 | 0.001709 |
| has_binary_classes7_test.test | RUN | 0.221199 | 0.914259 | 1.487723 | 0.001752 | 0.001742 | 0.001448 |
| has_binary_classes6_test.test | RUN | 0.224136 | 0.909955 | 1.503197 | 0.003409 | 0.001814 | 0.001439 |
| has_binary_classes5_test.test | RUN | 0.257719 | 0.925821 | 1.422530 | 0.001736 | 0.001729 | 0.001780 |
| has_binary_classes4_test.test | RUN | 0.258426 | 0.940767 | 1.499313 | 0.001843 | 0.001716 | 0.001449 |
| has_binary_classes3_test.test | RUN | 0.249050 | 0.903478 | 1.442194 | 0.001684 | 0.001469 | 0.001507 |
| has_binary_classes2_test.test | RUN | 0.224855 | 0.907335 | 1.472408 | 0.001590 | 0.001976 | 0.001739 |
| has_binary_classes1_test.test | RUN | 0.227917 | 1.009937 | 1.421401 | 0.001619 | 0.001523 | 0.001706 |
| has_binary_classes0_test.test | RUN | 0.232849 | 1.013392 | 1.420328 | 0.001348 | 0.001846 | 0.001364 |
| function_traits_test.test | RUN | 0.119688 | 0.240702 | 0.332942 | 0.001709 | 0.001882 | 0.001592 |
| floating_point_promotion_test.test | RUN | 0.119790 | 0.234653 | 0.315383 | 0.001674 | 0.003945 | 0.001750 |
| extent_test.test | RUN | 0.113225 | 0.266118 | 0.326522 | 0.001746 | 0.001967 | 0.001633 |
| enable_if_test.test | RUN | 0.104767 | 0.214061 | 0.240054 | 0.001414 | 0.001778 | 0.001540 |
| disjunction_test.test | RUN | 0.109729 | 0.213555 | 0.277424 | 0.001770 | 0.002014 | 0.001718 |
| detected_test.test | RUN | 0.108320 | 0.201726 | 0.236371 | 0.001486 | 0.001996 | 0.001204 |
| detected_or_test.test | RUN | 0.108816 | 0.264636 | 0.260409 | 0.001715 | 0.001761 | 0.001389 |
| decay_test.test | RUN | 0.244684 | 0.813130 | 1.453190 | 0.001735 | 0.001878 | 0.001735 |
| cxx14_aliases_test.test | RUN | 0.169462 | 0.309512 | 0.417361 | 0.001738 | 0.001626 | 0.001619 |
| copy_reference_test.test | RUN | 0.111453 | 0.202106 | 0.271852 | 0.001934 | 0.001860 | 0.001728 |
| copy_cv_test.test | RUN | 0.123316 | 0.217018 | 0.290597 | 0.001822 | 0.001764 | 0.001723 |
| copy_cv_ref_test.test | RUN | 0.154606 | 0.355677 | 0.609386 | 0.001807 | 0.001437 | 0.001693 |
| conjunction_test.test | RUN | 0.110018 | 0.216122 | 0.287855 | 0.001746 | 0.001577 | 0.001768 |
| conditional_test.test | RUN | 0.113972 | 0.203315 | 0.277603 | 0.001565 | 0.001846 | 0.001723 |
| common_type_test.test | RUN | 0.134521 | 0.308253 | 0.524534 | 0.001798 | 0.001562 | 0.001690 |
| common_type_sfinae_test.test | RUN | 0.115301 | 0.202190 | 0.237275 | 0.001747 | 0.001609 | 0.001748 |
| common_type_sfinae2_test.test | RUN | 0.113311 | 0.213685 | 0.288020 | 0.001417 | 0.001471 | 0.001411 |
| common_type_6_test.test | RUN | 0.114446 | 0.209879 | 0.274119 | 0.001351 | 0.001892 | 0.001613 |
| common_type_5_test.test | RUN | 0.119099 | 0.219838 | 0.274129 | 0.001638 | 0.002104 | 0.001870 |
| common_type_4_test.test | RUN | 0.116099 | 0.241297 | 0.326859 | 0.001610 | 0.001609 | 0.001684 |
| common_type_3_test.test | RUN | 0.107966 | 0.209820 | 0.259034 | 0.001882 | 0.001886 | 0.001838 |
| common_type_2_test.test | RUN | 0.137964 | 0.333603 | 0.495547 | 0.001922 | 0.001919 | 0.001790 |
| alignment_of_test.test | RUN | 0.113984 | 0.262461 | 0.325298 | 0.001783 | 0.001732 | 0.001672 |
| alignment_of_a2_test.test | RUN | 0.113153 | 0.251436 | 0.324906 | 0.001933 | 0.001584 | 0.001727 |
| aligned_storage_test.test | RUN | 0.115937 | 0.244557 | 0.306622 | 0.001926 | 0.001760 | 0.002006 |
| aligned_storage_empy_test.test | RUN | 0.133900 | 0.289855 | 0.400723 | 0.001712 | 0.001809 | 0.001757 |
| aligned_storage_a2_test.test | RUN | 0.121897 | 0.250787 | 0.300224 | 0.001820 | 0.002156 | 0.001983 |
| add_volatile_test.test | RUN | 0.213288 | 0.684525 | 1.361945 | 0.001534 | 0.001676 | 0.001916 |
| add_rvalue_reference_test.test | RUN | 0.236817 | 0.778515 | 1.593298 | 0.001708 | 0.001802 | 0.001782 |
| add_reference_test.test | RUN | 0.230049 | 0.775196 | 1.553941 | 0.001477 | 0.001786 | 0.001770 |
| add_pointer_test.test | RUN | 0.162391 | 0.456658 | 0.813406 | 0.001602 | 0.001662 | 0.001548 |
| add_lvalue_reference_test.test | RUN | 0.233823 | 0.772495 | 1.631569 | 0.001417 | 0.001562 | 0.001795 |
| add_cv_test.test | RUN | 0.224210 | 0.732866 | 1.545331 | 0.001629 | 0.001499 | 0.001668 |
| add_const_test.test | RUN | 0.222692 | 0.717157 | 1.493907 | 0.001418 | 0.001959 | 0.001753 |
| is_virtual_base_of_fail.test | COMPILE_FAIL | 0.105381 | 0.202743 | 0.253259 | — | — | — |
| is_stateless_fail.test | COMPILE_FAIL | 0.105192 | 0.211896 | 0.279401 | — | — | — |
| is_polymorphic_fail.test | COMPILE_FAIL | 0.100866 | 0.195991 | 0.313703 | — | — | — |
| is_nothrow_move_constructible_fail.test | COMPILE_FAIL | 0.100902 | 0.197750 | 0.241419 | — | — | — |
| is_nothrow_move_assignable_fail.test | COMPILE_FAIL | 0.106903 | 0.193609 | 0.242322 | — | — | — |
| is_list_constructible_fail.test | COMPILE_FAIL | 0.107559 | 0.198635 | 0.231835 | — | — | — |
| is_empty_fail.test | COMPILE_FAIL | 0.104291 | 0.202783 | 0.228627 | — | — | — |
| is_destructible_fail.test | COMPILE_FAIL | 0.111300 | 0.192347 | 0.247568 | — | — | — |
| is_default_constructible_fail.test | COMPILE_FAIL | 0.113956 | 0.196679 | 0.245105 | — | — | — |
| is_copy_constructible_fail.test | COMPILE_FAIL | 0.115188 | 0.196441 | 0.235132 | — | — | — |
| is_copy_assignable_fail.test | COMPILE_FAIL | 0.114252 | 0.201544 | 0.237073 | — | — | — |
| is_convertible_fail.test | COMPILE_FAIL | 0.109190 | 0.194884 | 0.237981 | — | — | — |
| is_convertible2_fail.test | COMPILE_FAIL | 0.108580 | 0.198155 | 0.235255 | — | — | — |
| is_constructible_fail.test | COMPILE_FAIL | 0.108568 | 0.191330 | 0.242155 | — | — | — |
| is_base_of_fail.test | COMPILE_FAIL | 0.102146 | 0.196774 | 0.238255 | — | — | — |
| is_assignable_fail.test | COMPILE_FAIL | 0.104696 | 0.193768 | 0.245582 | — | — | — |
| has_nothrow_destruct_fail.test | COMPILE_FAIL | 0.103707 | 0.197393 | 0.243209 | — | — | — |
| has_nothrow_copy_fail.test | COMPILE_FAIL | 0.106727 | 0.188684 | 0.254112 | — | — | — |
| has_nothrow_cons_fail.test | COMPILE_FAIL | 0.102814 | 0.191866 | 0.245070 | — | — | — |
| has_nothrow_assign_fail.test | COMPILE_FAIL | 0.102746 | 0.190500 | 0.241729 | — | — | — |
| common_type_fail.test | COMPILE_FAIL | 0.103375 | 0.186687 | 0.250588 | — | — | — |
| common_type2_fail.test | COMPILE_FAIL | 0.102512 | 0.192373 | 0.245571 | — | — | — |
| has_nothrow_assign_test_no_intrinsics.test | RUN | 0.150717 | 0.415826 | 0.664043 | 0.001792 | 0.001783 | 0.001646 |
| has_nothrow_constr_test_no_intrinsics.test | RUN | 0.145029 | 0.403671 | 0.705936 | 0.001792 | 0.001566 | 0.001643 |
| has_nothrow_copy_test_no_intrinsics.test | RUN | 0.151475 | 0.469915 | 0.706573 | 0.001922 | 0.001464 | 0.001454 |
| is_nothrow_move_assignable_test_no_intrinsics.test | RUN | 0.153524 | 0.449069 | 0.671971 | 0.001519 | 0.001426 | 0.001296 |
| is_nothrow_move_constructible_test_no_intrinsics.test | RUN | 0.107798 | 0.289062 | 0.385895 | 0.001406 | 0.001363 | 0.001324 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.975×† | 0.930×, 1.019× | 0.990× |
| clang | 1.020× | 1.015×, 1.024× | 0.994× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| add_const_test.test | 10 | 0.000970s | 0.000908s | 0.001041s | byte equality observed |
| add_cv_test.test | 10 | 0.000974s | 0.000928s | 0.000965s | byte equality observed |
| add_lvalue_reference_test.test | 10 | 0.000938s | 0.000939s | 0.000982s | byte equality observed |
| add_pointer_test.test | 10 | 0.000954s | 0.000943s | 0.000945s | byte equality observed |
| add_reference_test.test | 10 | 0.000950s | 0.000934s | 0.000956s | byte equality observed |
| add_rvalue_reference_test.test | 10 | 0.000951s | 0.000943s | 0.000950s | byte equality observed |
| add_volatile_test.test | 10 | 0.000968s | 0.000946s | 0.000940s | byte equality observed |
| aligned_storage_a2_test.test | 10 | 0.001035s | 0.001036s | 0.001045s | byte equality observed |
| aligned_storage_empy_test.test | 10 | 0.000965s | 0.000921s | 0.000921s | byte equality observed |
| aligned_storage_test.test | 10 | 0.001039s | 0.001041s | 0.001030s | byte equality observed |
| alignment_of_a2_test.test | 10 | 0.000947s | 0.000939s | 0.000938s | byte equality observed |
| alignment_of_test.test | 10 | 0.000946s | 0.000944s | 0.000937s | byte equality observed |
| common_type_2_test.test | 10 | 0.000941s | 0.000925s | 0.000949s | byte equality observed |
| common_type_3_test.test | 10 | 0.000956s | 0.000925s | 0.000951s | byte equality observed |
| common_type_4_test.test | 10 | 0.000932s | 0.000932s | 0.000973s | byte equality observed |
| common_type_5_test.test | 10 | 0.000952s | 0.000933s | 0.000942s | byte equality observed |
| common_type_6_test.test | 10 | 0.000945s | 0.000946s | 0.000947s | byte equality observed |
| common_type_sfinae2_test.test | 10 | 0.000937s | 0.000935s | 0.000940s | byte equality observed |
| common_type_sfinae_test.test | 10 | 0.000944s | 0.000930s | 0.000942s | byte equality observed |
| common_type_test.test | 10 | 0.000956s | 0.000941s | 0.000946s | byte equality observed |
| conditional_test.test | 10 | 0.000953s | 0.000936s | 0.000934s | byte equality observed |
| conjunction_test.test | 10 | 0.000950s | 0.000948s | 0.001022s | byte equality observed |
| copy_cv_ref_test.test | 10 | 0.000959s | 0.000948s | 0.001029s | byte equality observed |
| copy_cv_test.test | 10 | 0.000936s | 0.000921s | 0.000963s | byte equality observed |
| copy_reference_test.test | 10 | 0.000940s | 0.000940s | 0.000939s | byte equality observed |
| cxx14_aliases_test.test | 10 | 0.000957s | 0.000933s | 0.000931s | byte equality observed |
| decay_test.test | 10 | 0.000973s | 0.000936s | 0.000947s | byte equality observed |
| detected_or_test.test | 10 | 0.000959s | 0.000937s | 0.000935s | byte equality observed |
| detected_test.test | 10 | 0.000948s | 0.000939s | 0.000964s | byte equality observed |
| disjunction_test.test | 10 | 0.000953s | 0.000940s | 0.000943s | byte equality observed |
| enable_if_test.test | 10 | 0.000944s | 0.000919s | 0.000956s | byte equality observed |
| extent_test.test | 10 | 0.000946s | 0.000941s | 0.000944s | byte equality observed |
| floating_point_promotion_test.test | 10 | 0.000963s | 0.000919s | 0.000949s | byte equality observed |
| function_traits_test.test | 10 | 0.000943s | 0.000920s | 0.000936s | byte equality observed |
| has_binary_classes0_test.test | 10 | 0.000950s | 0.000935s | 0.000946s | byte equality observed |
| has_binary_classes1_test.test | 10 | 0.000944s | 0.000930s | 0.000970s | byte equality observed |
| has_binary_classes2_test.test | 10 | 0.000939s | 0.000956s | 0.000938s | byte equality observed |
| has_binary_classes3_test.test | 10 | 0.000949s | 0.000929s | 0.000935s | byte equality observed |
| has_binary_classes4_test.test | 10 | 0.000933s | 0.000940s | 0.000988s | byte equality observed |
| has_binary_classes5_test.test | 10 | 0.000936s | 0.000936s | 0.000925s | byte equality observed |
| has_binary_classes6_test.test | 10 | 0.000943s | 0.000919s | 0.000938s | byte equality observed |
| has_binary_classes7_test.test | 10 | 0.000933s | 0.000940s | 0.000947s | byte equality observed |
| has_binary_classes8_test.test | 10 | 0.000966s | 0.000921s | 0.000939s | byte equality observed |
| has_binary_classes9_test.test | 10 | 0.000940s | 0.000913s | 0.000949s | byte equality observed |
| has_bit_and_assign_test.test | 10 | 0.000969s | 0.000914s | 0.000938s | byte equality observed |
| has_bit_and_test.test | 10 | 0.000931s | 0.000915s | 0.000935s | byte equality observed |
| has_bit_or_assign_test.test | 10 | 0.000957s | 0.000914s | 0.000950s | byte equality observed |
| has_bit_or_test.test | 10 | 0.000971s | 0.000931s | 0.000961s | byte equality observed |
| has_bit_xor_assign_test.test | 10 | 0.000952s | 0.000894s | 0.000967s | byte equality observed |
| has_bit_xor_test.test | 10 | 0.000941s | 0.000926s | 0.000955s | byte equality observed |
| has_complement_test.test | 10 | 0.000957s | 0.000941s | 0.000958s | byte equality observed |
| has_dereference_test.test | 10 | 0.000944s | 0.000928s | 0.000944s | byte equality observed |
| has_divides_assign_test.test | 10 | 0.000949s | 0.000928s | 0.000958s | byte equality observed |
| has_divides_test.test | 10 | 0.000929s | 0.000932s | 0.000949s | byte equality observed |
| has_equal_to_test.test | 10 | 0.000948s | 0.000910s | 0.000943s | byte equality observed |
| has_greater_equal_test.test | 10 | 0.000939s | 0.000905s | 0.000958s | byte equality observed |
| has_greater_test.test | 10 | 0.000936s | 0.000918s | 0.000970s | byte equality observed |
| has_left_shift_assign_test.test | 10 | 0.000948s | 0.000918s | 0.000953s | byte equality observed |
| has_left_shift_test.test | 10 | 0.000932s | 0.000918s | 0.000962s | byte equality observed |
| has_less_equal_test.test | 10 | 0.000937s | 0.000915s | 0.000947s | byte equality observed |
| has_less_test.test | 10 | 0.000937s | 0.000931s | 0.000948s | byte equality observed |
| has_logical_and_test.test | 10 | 0.000943s | 0.000960s | 0.000951s | byte equality observed |
| has_logical_not_test.test | 10 | 0.000939s | 0.000931s | 0.000952s | byte equality observed |
| has_logical_or_test.test | 10 | 0.000921s | 0.000929s | 0.000938s | byte equality observed |
| has_minus_assign_test.test | 10 | 0.000931s | 0.000934s | 0.000938s | byte equality observed |
| has_minus_test.test | 10 | 0.000924s | 0.000939s | 0.000937s | byte equality observed |
| has_modulus_assign_test.test | 10 | 0.000927s | 0.000961s | 0.000935s | byte equality observed |
| has_modulus_test.test | 10 | 0.000940s | 0.000940s | 0.000958s | byte equality observed |
| has_multiplies_assign_test.test | 10 | 0.000920s | 0.000929s | 0.000938s | byte equality observed |
| has_multiplies_test.test | 10 | 0.000945s | 0.000928s | 0.000937s | byte equality observed |
| has_negate_test.test | 10 | 0.000946s | 0.000933s | 0.000960s | byte equality observed |
| has_not_equal_to_test.test | 10 | 0.000945s | 0.000931s | 0.000948s | byte equality observed |
| has_nothrow_assign_test.test | 10 | 0.000954s | 0.000949s | 0.000918s | byte equality observed |
| has_nothrow_assign_test_no_intrinsics.test | 10 | 0.000935s | 0.000938s | 0.000940s | byte equality observed |
| has_nothrow_constr_test.test | 10 | 0.000961s | 0.000921s | 0.000944s | byte equality observed |
| has_nothrow_constr_test_no_intrinsics.test | 10 | 0.000944s | 0.000934s | 0.000936s | byte equality observed |
| has_nothrow_copy_test.test | 10 | 0.000958s | 0.000921s | 0.000958s | byte equality observed |
| has_nothrow_copy_test_no_intrinsics.test | 10 | 0.000958s | 0.000924s | 0.000934s | byte equality observed |
| has_nothrow_destructor_test.test | 10 | 0.000972s | 0.000917s | 0.000955s | byte equality observed |
| has_operator_new_test.test | 10 | 0.000960s | 0.000934s | 0.000923s | byte equality observed |
| has_plus_assign_test.test | 10 | 0.000955s | 0.000934s | 0.000932s | byte equality observed |
| has_plus_test.test | 10 | 0.000940s | 0.000910s | 0.000935s | byte equality observed |
| has_post_decrement_test.test | 10 | 0.000939s | 0.000952s | 0.000940s | byte equality observed |
| has_post_increment_test.test | 10 | 0.000965s | 0.000919s | 0.000940s | byte equality observed |
| has_postfix_classes0_test.test | 10 | 0.000953s | 0.000936s | 0.000966s | byte equality observed |
| has_postfix_classes1_test.test | 10 | 0.000935s | 0.000932s | 0.000928s | byte equality observed |
| has_postfix_classes2_test.test | 10 | 0.000962s | 0.000942s | 0.000935s | byte equality observed |
| has_postfix_classes3_test.test | 10 | 0.000936s | 0.000918s | 0.000946s | byte equality observed |
| has_pre_decrement_test.test | 10 | 0.000938s | 0.000953s | 0.000935s | byte equality observed |
| has_pre_increment_test.test | 10 | 0.000935s | 0.000927s | 0.000958s | byte equality observed |
| has_prefix_classes0_test.test | 10 | 0.000948s | 0.000921s | 0.000949s | byte equality observed |
| has_prefix_classes1_test.test | 10 | 0.000922s | 0.000923s | 0.000938s | byte equality observed |
| has_prefix_classes2_test.test | 10 | 0.000934s | 0.000925s | 0.000931s | byte equality observed |
| has_prefix_classes3_test.test | 10 | 0.000946s | 0.000916s | 0.000927s | byte equality observed |
| has_right_shift_assign_test.test | 10 | 0.000927s | 0.000955s | 0.000952s | byte equality observed |
| has_right_shift_test.test | 10 | 0.000966s | 0.000948s | 0.000949s | byte equality observed |
| has_trivial_assign_test.test | 10 | 0.000991s | 0.000987s | 0.001001s | byte equality observed |
| has_trivial_constr_test.test | 10 | 0.000983s | 0.000984s | 0.000998s | native test oracle; output varies |
| has_trivial_copy_test.test | 10 | 0.000994s | 0.000984s | 0.001049s | native test oracle; output varies |
| has_trivial_destructor_test.test | 10 | 0.000930s | 0.000913s | 0.000944s | byte equality observed |
| has_trivial_move_assign_test.test | 10 | 0.000949s | 0.000918s | 0.000945s | byte equality observed |
| has_trivial_move_constructor_test.test | 10 | 0.000941s | 0.000906s | 0.000933s | byte equality observed |
| has_unary_minus_test.test | 10 | 0.000932s | 0.000930s | 0.000958s | byte equality observed |
| has_unary_plus_test.test | 10 | 0.000950s | 0.000914s | 0.000946s | byte equality observed |
| has_virtual_destructor_test.test | 10 | 0.000941s | 0.000915s | 0.000946s | byte equality observed |
| integral_constant_test.test | 10 | 0.000452s | 0.000440s | 0.000927s | byte equality observed |
| is_abstract_test.test | 10 | 0.000951s | 0.000943s | 0.000943s | byte equality observed |
| is_arithmetic_test.test | 10 | 0.000953s | 0.000933s | 0.000932s | byte equality observed |
| is_array_test.test | 10 | 0.000952s | 0.000945s | 0.000949s | byte equality observed |
| is_assignable_test.test | 10 | 0.000944s | 0.000927s | 0.000950s | byte equality observed |
| is_base_and_derived_test.test | 10 | 0.000951s | 0.000922s | 0.000949s | byte equality observed |
| is_base_of_test.test | 10 | 0.000945s | 0.000937s | 0.000950s | byte equality observed |
| is_bounded_array_test.test | 10 | 0.000949s | 0.000927s | 0.000966s | byte equality observed |
| is_class_test.test | 10 | 0.000947s | 0.000971s | 0.000943s | byte equality observed |
| is_complete_test.test | 10 | 0.000963s | 0.000950s | 0.000962s | byte equality observed |
| is_complex_test.test | 10 | 0.000956s | 0.000922s | 0.000927s | byte equality observed |
| is_compound_test.test | 10 | 0.000960s | 0.000959s | 0.000943s | byte equality observed |
| is_const_test.test | 10 | 0.000949s | 0.000933s | 0.000936s | byte equality observed |
| is_constructible_test.test | 10 | 0.000931s | 0.000947s | 0.000943s | byte equality observed |
| is_convertible_test.test | 10 | 0.000961s | 0.000940s | 0.000955s | byte equality observed |
| is_copy_assignable_test.test | 10 | 0.000949s | 0.000935s | 0.000934s | byte equality observed |
| is_copy_constructible_test.test | 10 | 0.000960s | 0.000935s | 0.000951s | byte equality observed |
| is_default_constr_test.test | 10 | 0.000947s | 0.000927s | 0.000957s | byte equality observed |
| is_destructible_test.test | 10 | 0.000946s | 0.000938s | 0.000964s | byte equality observed |
| is_detected_exact_test.test | 10 | 0.000947s | 0.000940s | 0.000968s | byte equality observed |
| is_detected_test.test | 10 | 0.000973s | 0.000924s | 0.000964s | byte equality observed |
| is_empty_test.test | 10 | 0.000953s | 0.000943s | 0.000964s | byte equality observed |
| is_enum_test.test | 10 | 0.000939s | 0.000933s | 0.000936s | byte equality observed |
| is_final_test.test | 10 | 0.000939s | 0.000927s | 0.000948s | byte equality observed |
| is_float_test.test | 10 | 0.000954s | 0.000928s | 0.000961s | byte equality observed |
| is_floating_point_test.test | 10 | 0.000940s | 0.000925s | 0.000962s | byte equality observed |
| is_function_test.test | 10 | 0.000957s | 0.000941s | 0.000958s | byte equality observed |
| is_fundamental_test.test | 10 | 0.000967s | 0.000930s | 0.000972s | byte equality observed |
| is_integral_test.test | 10 | 0.000927s | 0.000930s | 0.000926s | byte equality observed |
| is_list_constructible_test.test | 10 | 0.000951s | 0.000924s | 0.000945s | byte equality observed |
| is_lvalue_reference_test.test | 10 | 0.000930s | 0.000929s | 0.000959s | byte equality observed |
| is_member_func_test.test | 10 | 0.000937s | 0.000942s | 0.000924s | byte equality observed |
| is_member_obj_test.test | 10 | 0.000929s | 0.000958s | 0.000945s | byte equality observed |
| is_member_pointer_test.test | 10 | 0.000940s | 0.000933s | 0.000935s | byte equality observed |
| is_noncopyable_test.test | 10 | 0.000935s | 0.000932s | 0.000936s | byte equality observed |
| is_nothrow_move_assignable_test.test | 10 | 0.000942s | 0.000922s | 0.000946s | byte equality observed |
| is_nothrow_move_assignable_test_no_intrinsics.test | 10 | 0.000960s | 0.000923s | 0.000946s | byte equality observed |
| is_nothrow_move_constructible_test.test | 10 | 0.000937s | 0.000924s | 0.000966s | byte equality observed |
| is_nothrow_move_constructible_test_no_intrinsics.test | 10 | 0.000935s | 0.000912s | 0.000948s | byte equality observed |
| is_nothrow_swappable_test.test | 10 | 0.000943s | 0.000929s | 0.000940s | byte equality observed |
| is_object_test.test | 10 | 0.000943s | 0.000919s | 0.000960s | byte equality observed |
| is_pod_test.test | 10 | 0.000934s | 0.000920s | 0.000937s | byte equality observed |
| is_pointer_test.test | 10 | 0.000937s | 0.000915s | 0.000925s | byte equality observed |
| is_polymorphic_test.test | 10 | 0.000943s | 0.000923s | 0.000937s | byte equality observed |
| is_reference_test.test | 10 | 0.000938s | 0.000935s | 0.000947s | byte equality observed |
| is_rvalue_reference_test.test | 10 | 0.000920s | 0.000919s | 0.000940s | byte equality observed |
| is_same_test.test | 10 | 0.000952s | 0.000942s | 0.000953s | byte equality observed |
| is_scalar_test.test | 10 | 0.000931s | 0.000936s | 0.000927s | byte equality observed |
| is_scoped_enum_test.test | 10 | 0.000935s | 0.000955s | 0.000934s | byte equality observed |
| is_signed_test.test | 10 | 0.000913s | 0.000926s | 0.000943s | byte equality observed |
| is_stateless_test.test | 10 | 0.000955s | 0.000946s | 0.000962s | byte equality observed |
| is_swappable_test.test | 10 | 0.000937s | 0.000920s | 0.000930s | byte equality observed |
| is_trivially_copyable_test.test | 10 | 0.000982s | 0.000950s | 0.000960s | byte equality observed |
| is_unbounded_array_test.test | 10 | 0.000948s | 0.000934s | 0.000957s | byte equality observed |
| is_union_test.test | 10 | 0.000934s | 0.000917s | 0.000947s | byte equality observed |
| is_unscoped_enum_test.test | 10 | 0.000943s | 0.000946s | 0.000950s | byte equality observed |
| is_unsigned_test.test | 10 | 0.000952s | 0.000940s | 0.000931s | byte equality observed |
| is_virtual_base_of_test.test | 10 | 0.000939s | 0.000906s | 0.000935s | byte equality observed |
| is_void_test.test | 10 | 0.000956s | 0.000936s | 0.000932s | byte equality observed |
| is_volatile_test.test | 10 | 0.000925s | 0.000926s | 0.000944s | byte equality observed |
| make_signed_test.test | 10 | 0.000945s | 0.000932s | 0.000952s | byte equality observed |
| make_unsigned_test.test | 10 | 0.000964s | 0.000926s | 0.000970s | byte equality observed |
| make_void_test.test | 10 | 0.000941s | 0.000941s | 0.000940s | byte equality observed |
| mpl_interop_test1.test | 10 | 0.000463s | 0.000450s | 0.000950s | byte equality observed |
| mpl_interop_test2.test | 10 | 0.000462s | 0.000464s | 0.000947s | byte equality observed |
| mpl_interop_test3.test | 10 | 0.000449s | 0.000452s | 0.000942s | byte equality observed |
| negation_test.test | 10 | 0.001042s | 0.000995s | 0.000938s | byte equality observed |
| promote_basic_test.test | 10 | 0.000449s | 0.000443s | 0.000948s | byte equality observed |
| promote_enum_msvc_bug_test.test | 10 | 0.000452s | 0.000462s | 0.000955s | byte equality observed |
| promote_enum_test.test | 10 | 0.000457s | 0.000463s | 0.000945s | byte equality observed |
| promote_mpl_test.test | 10 | 0.000448s | 0.000454s | 0.000947s | byte equality observed |
| rank_test.test | 10 | 0.000977s | 0.001002s | 0.000949s | byte equality observed |
| remove_all_extents_test.test | 10 | 0.000959s | 0.000940s | 0.000929s | byte equality observed |
| remove_bounds_test.test | 10 | 0.000961s | 0.000923s | 0.000955s | byte equality observed |
| remove_const_test.test | 10 | 0.000957s | 0.000959s | 0.000979s | byte equality observed |
| remove_cv_ref_test.test | 10 | 0.000954s | 0.000913s | 0.000963s | byte equality observed |
| remove_cv_test.test | 10 | 0.000951s | 0.000924s | 0.000951s | byte equality observed |
| remove_extent_test.test | 10 | 0.000944s | 0.000928s | 0.000950s | byte equality observed |
| remove_pointer_test.test | 10 | 0.000938s | 0.000950s | 0.000950s | byte equality observed |
| remove_reference_test.test | 10 | 0.000936s | 0.000924s | 0.000969s | byte equality observed |
| remove_volatile_test.test | 10 | 0.000925s | 0.000929s | 0.001006s | byte equality observed |
| tricky_abstract_type_test.test | 10 | 0.000952s | 0.000936s | 0.000976s | byte equality observed |
| tricky_add_pointer_test.test | 10 | 0.000935s | 0.000909s | 0.000981s | byte equality observed |
| tricky_function_type_test.test | 10 | 0.000955s | 0.000941s | 0.001018s | byte equality observed |
| tricky_incomplete_type_test.test | 10 | 0.000930s | 0.000943s | 0.001012s | byte equality observed |
| tricky_is_enum_test.test | 10 | 0.000944s | 0.000941s | 0.000996s | byte equality observed |
| tricky_partial_spec_test.test | 10 | 0.000958s | 0.000926s | 0.000999s | byte equality observed |
| tricky_rvalue_test.test | 10 | 0.000949s | 0.000930s | 0.001019s | byte equality observed |
| type_identity_test.test | 10 | 0.000971s | 0.000931s | 0.000991s | byte equality observed |
| type_traits_test.test | 10 | 0.000447s | 0.000435s | 0.001006s | byte equality observed |
| type_with_alignment_test.test | 10 | 0.001052s | 0.001031s | 0.001119s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
