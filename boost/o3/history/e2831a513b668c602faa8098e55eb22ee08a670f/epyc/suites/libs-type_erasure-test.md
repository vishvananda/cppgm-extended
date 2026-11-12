# libs/type_erasure/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_binding.test | RUN | 0.753910 | 2.814322 | 2.893458 | 0.004254 | 0.004458 | 0.004326 |
| test_increment.test | RUN | 0.732089 | 2.090116 | 2.548930 | 0.004640 | 0.004520 | 0.004541 |
| test_add.test | RUN | 0.887659 | 3.328161 | 3.683896 | 0.004489 | 0.004279 | 0.004116 |
| test_add_assign.test | RUN | 0.909188 | 3.572073 | 3.844962 | 0.004218 | 0.004705 | 0.004637 |
| test_callable.test | RUN | 1.512589 | 6.067759 | 8.092504 | 0.007245 | 0.008013 | 0.008920 |
| test_reference.test | RUN | 0.694701 | 2.507832 | 2.550592 | 0.004333 | 0.004454 | 0.004555 |
| test_construct.test | RUN | 3.155917 | 28.595973 | 22.809754 | 0.009000 | 0.008967 | 0.008132 |
| test_relaxed.test | RUN | 0.709500 | 1.820438 | 2.355762 | 0.004022 | 0.003810 | 0.004061 |
| test_assign.test | RUN | 7.316679 | 85.812599 | 77.051731 | 0.014026 | 0.008065 | 0.008978 |
| test_construct_ref.test | RUN | 0.856716 | 4.369859 | 3.850044 | 0.005947 | 0.004847 | 0.006297 |
| test_construct_cref.test | RUN | 0.933508 | 4.052061 | 4.569910 | 0.006671 | 0.007357 | 0.006837 |
| test_any_cast.test | RUN | 0.791065 | 4.113201 | 3.747163 | 0.004726 | 0.004638 | 0.005649 |
| test_binding_of.test | RUN | 0.675831 | 1.711286 | 2.191425 | 0.003866 | 0.004159 | 0.003801 |
| test_typeid_of.test | RUN | 0.697981 | 2.252827 | 2.476062 | 0.004604 | 0.004390 | 0.005030 |
| test_nested.test | RUN | 0.795559 | 2.228154 | 2.777791 | 0.003970 | 0.004046 | 0.004091 |
| test_less.test | RUN | 1.013675 | 4.484746 | 4.493459 | 0.005793 | 0.005610 | 0.005863 |
| test_equal.test | RUN | 0.936392 | 4.174299 | 3.879709 | 0.005060 | 0.006466 | 0.005700 |
| test_negate.test | RUN | 0.649924 | 1.688233 | 2.296597 | 0.004013 | 0.004746 | 0.003257 |
| test_dereference.test | RUN | 0.746401 | 2.327981 | 2.761470 | 0.004014 | 0.004181 | 0.004760 |
| test_subscript.test | RUN | 0.757287 | 2.622592 | 2.876458 | 0.004288 | 0.004290 | 0.004412 |
| test_forward_iterator.test | RUN | 0.892385 | 2.792177 | 3.005712 | 0.003916 | 0.003809 | 0.003998 |
| test_tuple.test | RUN | 0.795292 | 2.724962 | 3.003006 | 0.004429 | 0.004602 | 0.004599 |
| test_stream.test | RUN | 0.977940 | 4.636879 | 4.531365 | 0.005997 | 0.006282 | 0.005600 |
| test_deduced.test | RUN | 0.655455 | 1.829034 | 2.067119 | 0.003972 | 0.003331 | 0.003380 |
| test_same_type.test | RUN | 0.759486 | 2.234570 | 2.765735 | 0.003890 | 0.004289 | 0.003802 |
| test_member.test | RUN | 0.996826 | 4.572736 | 3.854983 | 0.006378 | 0.005639 | 0.006547 |
| test_null.test | RUN | 0.679185 | 2.118376 | 2.410023 | 0.004120 | 0.003890 | 0.004240 |
| test_free.test | RUN | 0.844876 | 3.785802 | 3.384076 | 0.005117 | 0.004803 | 0.005358 |
| test_is_empty.test | RUN | 0.682222 | 1.774881 | 2.179620 | 0.003969 | 0.003678 | 0.004352 |
| test_dynamic_any_cast.test | RUN | 1.258199 | 5.262973 | 5.613559 | 0.005515 | 0.007494 | 0.006014 |
| test_limits.test | RUN | 1.129352 | 3.708840 | 4.452682 | 0.004306 | 0.004567 | 0.004552 |
| test_param.test | COMPILE | 0.444038 | 1.101223 | 1.506013 | — | — | — |
| test_is_subconcept.test | COMPILE | 0.435899 | 0.828557 | 1.323726 | — | — | — |
| test_sfinae.test | COMPILE | 0.443129 | 0.825757 | 1.311305 | — | — | — |
| test_interface_order.test | COMPILE | 0.433975 | 0.804874 | 1.341095 | — | — | — |
| test_is_placeholder.test | COMPILE | 0.051699 | 0.081840 | 0.086215 | — | — | — |
| fail_default_construct.test | COMPILE_FAIL | 0.421312 | 0.822774 | 1.055820 | — | — | — |
| fail_construct_mismatch.test | COMPILE_FAIL | 0.419637 | 0.704819 | 1.251498 | — | — | — |
| fail_construct_mismatch_ref.test | COMPILE_FAIL | 0.418694 | 0.752751 | 1.238260 | — | — | — |
| fail_construct_mismatch_cref.test | COMPILE_FAIL | 0.412636 | 0.790373 | 1.059131 | — | — | — |
| fail_construct_mismatch_rref.test | COMPILE_FAIL | 0.414000 | 0.814456 | 1.224685 | — | — | — |
| fail_binding_convert_no_mapping.test | COMPILE_FAIL | 0.408004 | 0.752232 | 1.102067 | — | — | — |
| fail_increment_discard_const.test | COMPILE_FAIL | 0.421107 | 0.831423 | 1.260607 | — | — | — |
| fail_not_incrementable.test | COMPILE_FAIL | 0.443791 | 0.840868 | 1.205257 | — | — | — |
| fail_ref_assign.test | COMPILE_FAIL | 0.415399 | 0.777219 | 1.198842 | — | — | — |
| fail_cref_assign.test | COMPILE_FAIL | 0.408266 | 0.824383 | 1.238798 | — | — | — |
| fail_rref_assign.test | COMPILE_FAIL | 0.417324 | 0.798051 | 1.105089 | — | — | — |
| fail_ref_discard_const.test | COMPILE_FAIL | 0.424438 | 0.787437 | 1.226458 | — | — | — |
| fail_ref_discard_const_convert.test | COMPILE_FAIL | 0.405855 | 0.873535 | 1.303389 | — | — | — |
| fail_ref_discard_const_convert_ref.test | COMPILE_FAIL | 0.415812 | 0.864087 | 1.172531 | — | — | — |
| fail_ref_discard_const_convert_cref.test | COMPILE_FAIL | 0.428694 | 0.795999 | 1.277727 | — | — | — |
| fail_ref_discard_const_init.test | COMPILE_FAIL | 0.421557 | 0.840355 | 1.233689 | — | — | — |
| fail_any_cast_discard_const1.test | COMPILE_FAIL | 0.413768 | 0.840301 | 1.126835 | — | — | — |
| fail_any_cast_discard_const2.test | COMPILE_FAIL | 0.418046 | 0.820688 | 1.306028 | — | — | — |
| fail_any_cast_discard_const3.test | COMPILE_FAIL | 0.420410 | 0.806463 | 1.312448 | — | — | — |
| fail_any_cast_discard_const4.test | COMPILE_FAIL | 0.428874 | 0.840141 | 1.111804 | — | — | — |
| fail_any_cast_discard_const5.test | COMPILE_FAIL | 0.455677 | 0.830825 | 1.306350 | — | — | — |
| fail_any_cast_discard_const6.test | COMPILE_FAIL | 0.428586 | 0.866673 | 1.336148 | — | — | — |
| fail_any_cast_pointer_to_ref.test | COMPILE_FAIL | 0.365635 | 0.659444 | 1.081594 | — | — | — |
| fail_any_cast_pointer_to_val.test | COMPILE_FAIL | 0.332080 | 0.774540 | 0.840330 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.960× | 0.957×, 0.963× | 0.986× |
| clang | 0.963× | 0.959×, 0.967× | 1.017× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898266).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| test_add.test | 7 | 0.003010s | 0.002976s | 0.002883s | byte equality observed |
| test_add_assign.test | 8 | 0.002919s | 0.002932s | 0.002809s | byte equality observed |
| test_any_cast.test | 6 | 0.003460s | 0.003440s | 0.003344s | byte equality observed |
| test_assign.test | 3 | 0.010494s | 0.008529s | 0.009634s | byte equality observed |
| test_binding.test | 8 | 0.002807s | 0.002818s | 0.002653s | byte equality observed |
| test_binding_of.test | 8 | 0.002590s | 0.002572s | 0.002514s | byte equality observed |
| test_callable.test | 6 | 0.004490s | 0.004364s | 0.004272s | byte equality observed |
| test_construct.test | 4 | 0.005839s | 0.005777s | 0.005573s | byte equality observed |
| test_construct_cref.test | 6 | 0.004075s | 0.004065s | 0.003853s | byte equality observed |
| test_construct_ref.test | 6 | 0.003647s | 0.003537s | 0.003404s | byte equality observed |
| test_deduced.test | 7 | 0.002602s | 0.002639s | 0.002572s | byte equality observed |
| test_dereference.test | 7 | 0.002788s | 0.002788s | 0.002671s | byte equality observed |
| test_dynamic_any_cast.test | 6 | 0.004018s | 0.003968s | 0.003903s | byte equality observed |
| test_equal.test | 7 | 0.003368s | 0.003276s | 0.003215s | byte equality observed |
| test_forward_iterator.test | 7 | 0.002780s | 0.002821s | 0.002727s | byte equality observed |
| test_free.test | 7 | 0.003372s | 0.003315s | 0.003222s | byte equality observed |
| test_increment.test | 8 | 0.002742s | 0.002728s | 0.002695s | byte equality observed |
| test_is_empty.test | 8 | 0.002756s | 0.002664s | 0.002655s | byte equality observed |
| test_less.test | 6 | 0.003676s | 0.003607s | 0.003477s | byte equality observed |
| test_limits.test | 8 | 0.002956s | 0.002813s | 0.002799s | byte equality observed |
| test_member.test | 6 | 0.004061s | 0.004113s | 0.003976s | byte equality observed |
| test_negate.test | 8 | 0.002631s | 0.002567s | 0.002542s | byte equality observed |
| test_nested.test | 8 | 0.002698s | 0.002631s | 0.002669s | byte equality observed |
| test_null.test | 7 | 0.002853s | 0.002760s | 0.002699s | byte equality observed |
| test_reference.test | 8 | 0.002882s | 0.002867s | 0.002825s | byte equality observed |
| test_relaxed.test | 8 | 0.002724s | 0.002628s | 0.002632s | byte equality observed |
| test_same_type.test | 8 | 0.002825s | 0.002713s | 0.002712s | byte equality observed |
| test_stream.test | 6 | 0.003875s | 0.003778s | 0.003788s | byte equality observed |
| test_subscript.test | 7 | 0.002962s | 0.002883s | 0.002874s | byte equality observed |
| test_tuple.test | 7 | 0.002912s | 0.002905s | 0.002840s | byte equality observed |
| test_typeid_of.test | 7 | 0.002961s | 0.002887s | 0.002774s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898266).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
