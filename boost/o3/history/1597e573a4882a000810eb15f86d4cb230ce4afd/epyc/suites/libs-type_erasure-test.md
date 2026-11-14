# libs/type_erasure/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_binding.test | RUN | 0.376597 | 1.478317 | 1.234030 | 0.003061 | 0.002864 | 0.002744 |
| test_increment.test | RUN | 0.370900 | 1.113873 | 1.070460 | 0.002882 | 0.002643 | 0.002731 |
| test_add.test | RUN | 0.508327 | 1.777672 | 1.610085 | 0.003143 | 0.003125 | 0.002979 |
| test_add_assign.test | RUN | 0.488919 | 1.910951 | 1.650489 | 0.003227 | 0.002974 | 0.002992 |
| test_callable.test | RUN | 0.790662 | 3.370629 | 3.745199 | 0.005030 | 0.004953 | 0.004853 |
| test_reference.test | RUN | 0.393292 | 1.349240 | 1.145804 | 0.002787 | 0.003026 | 0.003074 |
| test_construct.test | RUN | 1.761141 | 17.898778 | 10.819805 | 0.006630 | 0.005647 | 0.006161 |
| test_relaxed.test | RUN | 0.354292 | 0.968791 | 1.007289 | 0.002808 | 0.002759 | 0.002715 |
| test_assign.test | RUN | 4.141927 | 51.954730 | 37.149206 | 0.008086 | 0.005999 | 0.006167 |
| test_construct_ref.test | RUN | 0.478913 | 2.270745 | 1.660740 | 0.003827 | 0.003748 | 0.003421 |
| test_construct_cref.test | RUN | 0.553356 | 2.191659 | 2.106864 | 0.004492 | 0.004370 | 0.004326 |
| test_any_cast.test | RUN | 0.474997 | 2.239086 | 1.559905 | 0.003997 | 0.003610 | 0.003498 |
| test_binding_of.test | RUN | 0.381487 | 0.885986 | 0.942818 | 0.002736 | 0.002652 | 0.002565 |
| test_typeid_of.test | RUN | 0.421812 | 1.182566 | 1.100362 | 0.003095 | 0.003073 | 0.002936 |
| test_nested.test | RUN | 0.474193 | 1.251801 | 1.217115 | 0.002906 | 0.002802 | 0.002669 |
| test_less.test | RUN | 0.548478 | 2.466309 | 2.020099 | 0.004090 | 0.003922 | 0.003582 |
| test_equal.test | RUN | 0.547883 | 2.283074 | 1.712014 | 0.003471 | 0.003866 | 0.003554 |
| test_negate.test | RUN | 0.398407 | 0.960917 | 1.014470 | 0.002794 | 0.002928 | 0.002612 |
| test_dereference.test | RUN | 0.439987 | 1.254964 | 1.191890 | 0.002857 | 0.003025 | 0.002881 |
| test_subscript.test | RUN | 0.467674 | 1.413338 | 1.254502 | 0.003102 | 0.003144 | 0.002866 |
| test_forward_iterator.test | RUN | 0.533911 | 1.584689 | 1.505997 | 0.003050 | 0.002964 | 0.002910 |
| test_tuple.test | RUN | 0.457945 | 1.585312 | 1.190350 | 0.002784 | 0.003133 | 0.002920 |
| test_stream.test | RUN | 0.579831 | 2.608560 | 2.060769 | 0.003876 | 0.003994 | 0.003965 |
| test_deduced.test | RUN | 0.386124 | 0.959721 | 0.994029 | 0.002809 | 0.002686 | 0.002434 |
| test_same_type.test | RUN | 0.402789 | 1.243427 | 1.128975 | 0.003035 | 0.002797 | 0.002979 |
| test_member.test | RUN | 0.516029 | 2.432915 | 1.963760 | 0.004292 | 0.004418 | 0.004304 |
| test_null.test | RUN | 0.364651 | 1.119037 | 0.937179 | 0.002852 | 0.002951 | 0.002683 |
| test_free.test | RUN | 0.449380 | 2.014819 | 1.484441 | 0.003293 | 0.003665 | 0.003332 |
| test_is_empty.test | RUN | 0.349531 | 1.013178 | 0.904030 | 0.002878 | 0.002904 | 0.002647 |
| test_dynamic_any_cast.test | RUN | 0.649644 | 3.001833 | 2.833956 | 0.004113 | 0.004048 | 0.004386 |
| test_limits.test | RUN | 0.607180 | 2.105336 | 1.945843 | 0.002717 | 0.002930 | 0.003105 |
| test_param.test | COMPILE | 0.237368 | 0.553292 | 0.576322 | — | — | — |
| test_is_subconcept.test | COMPILE | 0.226252 | 0.425747 | 0.496828 | — | — | — |
| test_sfinae.test | COMPILE | 0.235437 | 0.441298 | 0.532306 | — | — | — |
| test_interface_order.test | COMPILE | 0.236761 | 0.441755 | 0.487304 | — | — | — |
| test_is_placeholder.test | COMPILE | 0.029572 | 0.046071 | 0.050638 | — | — | — |
| fail_default_construct.test | COMPILE_FAIL | 0.231306 | 0.414726 | 0.522407 | — | — | — |
| fail_construct_mismatch.test | COMPILE_FAIL | 0.231976 | 0.433632 | 0.469267 | — | — | — |
| fail_construct_mismatch_ref.test | COMPILE_FAIL | 0.227063 | 0.430533 | 0.500597 | — | — | — |
| fail_construct_mismatch_cref.test | COMPILE_FAIL | 0.224906 | 0.426357 | 0.507906 | — | — | — |
| fail_construct_mismatch_rref.test | COMPILE_FAIL | 0.215294 | 0.432912 | 0.455547 | — | — | — |
| fail_binding_convert_no_mapping.test | COMPILE_FAIL | 0.210418 | 0.425183 | 0.492462 | — | — | — |
| fail_increment_discard_const.test | COMPILE_FAIL | 0.219219 | 0.431783 | 0.505523 | — | — | — |
| fail_not_incrementable.test | COMPILE_FAIL | 0.237228 | 0.410686 | 0.504060 | — | — | — |
| fail_ref_assign.test | COMPILE_FAIL | 0.216858 | 0.417756 | 0.480762 | — | — | — |
| fail_cref_assign.test | COMPILE_FAIL | 0.224577 | 0.421353 | 0.505815 | — | — | — |
| fail_rref_assign.test | COMPILE_FAIL | 0.221804 | 0.428981 | 0.459867 | — | — | — |
| fail_ref_discard_const.test | COMPILE_FAIL | 0.232431 | 0.415275 | 0.509682 | — | — | — |
| fail_ref_discard_const_convert.test | COMPILE_FAIL | 0.220500 | 0.508149 | 0.535490 | — | — | — |
| fail_ref_discard_const_convert_ref.test | COMPILE_FAIL | 0.226378 | 0.508592 | 0.580041 | — | — | — |
| fail_ref_discard_const_convert_cref.test | COMPILE_FAIL | 0.222138 | 0.487940 | 0.561736 | — | — | — |
| fail_ref_discard_const_init.test | COMPILE_FAIL | 0.230474 | 0.425322 | 0.461264 | — | — | — |
| fail_any_cast_discard_const1.test | COMPILE_FAIL | 0.220402 | 0.429469 | 0.529715 | — | — | — |
| fail_any_cast_discard_const2.test | COMPILE_FAIL | 0.233250 | 0.441800 | 0.521147 | — | — | — |
| fail_any_cast_discard_const3.test | COMPILE_FAIL | 0.233000 | 0.439631 | 0.524057 | — | — | — |
| fail_any_cast_discard_const4.test | COMPILE_FAIL | 0.242544 | 0.438980 | 0.527761 | — | — | — |
| fail_any_cast_discard_const5.test | COMPILE_FAIL | 0.229587 | 0.446445 | 0.477833 | — | — | — |
| fail_any_cast_discard_const6.test | COMPILE_FAIL | 0.227959 | 0.430748 | 0.524571 | — | — | — |
| fail_any_cast_pointer_to_ref.test | COMPILE_FAIL | 0.188330 | 0.435877 | 0.463829 | — | — | — |
| fail_any_cast_pointer_to_val.test | COMPILE_FAIL | 0.200706 | 0.416048 | 0.378573 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.974× | 0.975×, 0.972× | 0.983× |
| clang | 0.985× | 0.971×, 1.000× | 0.996× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026855).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| test_add.test | 10 | 0.001663s | 0.001611s | 0.001632s | byte equality observed |
| test_add_assign.test | 10 | 0.001639s | 0.001598s | 0.001588s | byte equality observed |
| test_any_cast.test | 10 | 0.001863s | 0.001799s | 0.001889s | byte equality observed |
| test_assign.test | 5 | 0.004538s | 0.004122s | 0.004476s | byte equality observed |
| test_binding.test | 10 | 0.001542s | 0.001494s | 0.001525s | byte equality observed |
| test_binding_of.test | 10 | 0.001444s | 0.001437s | 0.001404s | byte equality observed |
| test_callable.test | 8 | 0.002398s | 0.002339s | 0.002323s | byte equality observed |
| test_construct.test | 6 | 0.003122s | 0.003060s | 0.003055s | byte equality observed |
| test_construct_cref.test | 9 | 0.002185s | 0.002152s | 0.002090s | byte equality observed |
| test_construct_ref.test | 10 | 0.001959s | 0.001902s | 0.001885s | byte equality observed |
| test_deduced.test | 10 | 0.001481s | 0.001395s | 0.001414s | byte equality observed |
| test_dereference.test | 10 | 0.001520s | 0.001531s | 0.001476s | byte equality observed |
| test_dynamic_any_cast.test | 9 | 0.002174s | 0.002137s | 0.002136s | byte equality observed |
| test_equal.test | 10 | 0.001811s | 0.001805s | 0.001776s | byte equality observed |
| test_forward_iterator.test | 10 | 0.001546s | 0.001528s | 0.001488s | byte equality observed |
| test_free.test | 10 | 0.001831s | 0.001789s | 0.001761s | byte equality observed |
| test_increment.test | 10 | 0.001497s | 0.001482s | 0.001477s | byte equality observed |
| test_is_empty.test | 10 | 0.001509s | 0.001450s | 0.001427s | byte equality observed |
| test_less.test | 10 | 0.001962s | 0.001960s | 0.001912s | byte equality observed |
| test_limits.test | 10 | 0.001639s | 0.001582s | 0.001584s | byte equality observed |
| test_member.test | 8 | 0.002254s | 0.002188s | 0.002203s | byte equality observed |
| test_negate.test | 10 | 0.001466s | 0.001413s | 0.001400s | byte equality observed |
| test_nested.test | 10 | 0.001541s | 0.001438s | 0.001448s | byte equality observed |
| test_null.test | 10 | 0.001549s | 0.001491s | 0.001541s | byte equality observed |
| test_reference.test | 10 | 0.001619s | 0.001579s | 0.001546s | byte equality observed |
| test_relaxed.test | 10 | 0.001477s | 0.001452s | 0.001452s | byte equality observed |
| test_same_type.test | 10 | 0.001526s | 0.001510s | 0.001517s | byte equality observed |
| test_stream.test | 9 | 0.002082s | 0.002020s | 0.002090s | byte equality observed |
| test_subscript.test | 10 | 0.001596s | 0.001542s | 0.001566s | byte equality observed |
| test_tuple.test | 10 | 0.001581s | 0.001564s | 0.001588s | byte equality observed |
| test_typeid_of.test | 10 | 0.001609s | 0.001566s | 0.001562s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026855).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
