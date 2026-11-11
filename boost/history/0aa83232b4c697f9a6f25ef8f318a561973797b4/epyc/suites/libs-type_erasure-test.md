# libs/type_erasure/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_binding.test | RUN | 0.441907 | 1.092726 | 1.162340 | 0.004181 | 0.004446 | 0.004117 |
| test_increment.test | RUN | 0.459257 | 1.029258 | 1.048039 | 0.003790 | 0.003831 | 0.004069 |
| test_add.test | RUN | 0.548745 | 1.685487 | 1.507515 | 0.004223 | 0.004410 | 0.004218 |
| test_add_assign.test | RUN | 0.567796 | 1.771425 | 1.583078 | 0.003996 | 0.004452 | 0.004287 |
| test_callable.test | RUN | 0.871927 | 4.038152 | 3.372621 | 0.006434 | 0.006345 | 0.006512 |
| test_reference.test | RUN | 0.470874 | 1.141736 | 1.148540 | 0.003744 | 0.004282 | 0.004390 |
| test_construct.test | RUN | 1.210791 | 8.529065 | 6.406347 | 0.008696 | 0.009942 | 0.009666 |
| test_relaxed.test | RUN | 0.459454 | 0.992471 | 1.080991 | 0.003770 | 0.004022 | 0.003888 |
| test_assign.test | RUN | 2.236429 | 27.822816 | 16.403612 | 0.015191 | 0.010419 | 0.015156 |
| test_construct_ref.test | RUN | 0.519312 | 1.406736 | 1.378880 | 0.006477 | 0.005253 | 0.005167 |
| test_construct_cref.test | RUN | 0.537006 | 1.713390 | 1.577867 | 0.005846 | 0.006384 | 0.006084 |
| test_any_cast.test | RUN | 0.483062 | 1.432342 | 1.305911 | 0.004995 | 0.005262 | 0.005484 |
| test_binding_of.test | RUN | 0.450774 | 0.998494 | 1.035887 | 0.003264 | 0.003948 | 0.003893 |
| test_typeid_of.test | RUN | 0.486076 | 1.110495 | 1.136319 | 0.003999 | 0.004171 | 0.004102 |
| test_nested.test | RUN | 0.504758 | 1.305308 | 1.382894 | 0.003830 | 0.003827 | 0.003837 |
| test_less.test | RUN | 0.594879 | 1.991495 | 1.804817 | 0.005123 | 0.005530 | 0.005057 |
| test_equal.test | RUN | 0.562178 | 1.944568 | 1.667934 | 0.004367 | 0.004922 | 0.004330 |
| test_negate.test | RUN | 0.436253 | 1.071188 | 1.038105 | 0.003758 | 0.003416 | 0.003926 |
| test_dereference.test | RUN | 0.478759 | 1.214373 | 1.228758 | 0.004032 | 0.004156 | 0.003774 |
| test_subscript.test | RUN | 0.493778 | 1.291414 | 1.292042 | 0.004054 | 0.004291 | 0.004328 |
| test_forward_iterator.test | RUN | 0.560261 | 1.808410 | 1.642249 | 0.003920 | 0.004398 | 0.004206 |
| test_tuple.test | RUN | 0.503358 | 1.235623 | 1.280290 | 0.003903 | 0.004239 | 0.004206 |
| test_stream.test | RUN | 0.613865 | 2.134492 | 1.859359 | 0.005402 | 0.005712 | 0.005565 |
| test_deduced.test | RUN | 0.456853 | 0.970144 | 0.988365 | 0.003645 | 0.003895 | 0.003977 |
| test_same_type.test | RUN | 0.494122 | 1.172790 | 1.196811 | 0.003641 | 0.004066 | 0.005389 |
| test_member.test | RUN | 0.578613 | 1.953698 | 1.790205 | 0.005650 | 0.006020 | 0.006214 |
| test_null.test | RUN | 0.449640 | 1.028581 | 1.058323 | 0.003745 | 0.004324 | 0.004090 |
| test_free.test | RUN | 0.535730 | 1.454945 | 1.431199 | 0.004449 | 0.004895 | 0.004828 |
| test_is_empty.test | RUN | 0.437392 | 0.975391 | 1.032649 | 0.003714 | 0.003895 | 0.004017 |
| test_dynamic_any_cast.test | RUN | 0.671725 | 2.789659 | 2.430949 | 0.005453 | 0.005243 | 0.005616 |
| test_limits.test | RUN | 0.692425 | 2.586308 | 2.022167 | 0.004175 | 0.004683 | 0.004163 |
| test_param.test | COMPILE | 0.292494 | 0.599866 | 0.688927 | — | — | — |
| test_is_subconcept.test | COMPILE | 0.293659 | 0.503246 | 0.547007 | — | — | — |
| test_sfinae.test | COMPILE | 0.301999 | 0.500742 | 0.547895 | — | — | — |
| test_interface_order.test | COMPILE | 0.298288 | 0.492624 | 0.606610 | — | — | — |
| test_is_placeholder.test | COMPILE | 0.041252 | 0.045891 | 0.059321 | — | — | — |
| fail_default_construct.test | COMPILE_FAIL | 0.286606 | 0.508321 | 0.561493 | — | — | — |
| fail_construct_mismatch.test | COMPILE_FAIL | 0.280563 | 0.471846 | 0.569221 | — | — | — |
| fail_construct_mismatch_ref.test | COMPILE_FAIL | 0.285974 | 0.481138 | 0.555037 | — | — | — |
| fail_construct_mismatch_cref.test | COMPILE_FAIL | 0.280553 | 0.475880 | 0.556924 | — | — | — |
| fail_construct_mismatch_rref.test | COMPILE_FAIL | 0.284232 | 0.463472 | 0.565544 | — | — | — |
| fail_binding_convert_no_mapping.test | COMPILE_FAIL | 0.267998 | 0.469140 | 0.545739 | — | — | — |
| fail_increment_discard_const.test | COMPILE_FAIL | 0.291034 | 0.481480 | 0.562890 | — | — | — |
| fail_not_incrementable.test | COMPILE_FAIL | 0.289132 | 0.511100 | 0.595977 | — | — | — |
| fail_ref_assign.test | COMPILE_FAIL | 0.276374 | 0.479085 | 0.566356 | — | — | — |
| fail_cref_assign.test | COMPILE_FAIL | 0.282258 | 0.464963 | 0.580012 | — | — | — |
| fail_rref_assign.test | COMPILE_FAIL | 0.286056 | 0.489431 | 0.572600 | — | — | — |
| fail_ref_discard_const.test | COMPILE_FAIL | 0.272848 | 0.467822 | 0.582605 | — | — | — |
| fail_ref_discard_const_convert.test | COMPILE_FAIL | 0.278795 | 0.536616 | 0.598828 | — | — | — |
| fail_ref_discard_const_convert_ref.test | COMPILE_FAIL | 0.285710 | 0.584385 | 0.599395 | — | — | — |
| fail_ref_discard_const_convert_cref.test | COMPILE_FAIL | 0.288795 | 0.544663 | 0.591842 | — | — | — |
| fail_ref_discard_const_init.test | COMPILE_FAIL | 0.274559 | 0.464468 | 0.555854 | — | — | — |
| fail_any_cast_discard_const1.test | COMPILE_FAIL | 0.287463 | 0.479584 | 0.593869 | — | — | — |
| fail_any_cast_discard_const2.test | COMPILE_FAIL | 0.281593 | 0.477076 | 0.590049 | — | — | — |
| fail_any_cast_discard_const3.test | COMPILE_FAIL | 0.280627 | 0.475439 | 0.598366 | — | — | — |
| fail_any_cast_discard_const4.test | COMPILE_FAIL | 0.281143 | 0.485527 | 0.594799 | — | — | — |
| fail_any_cast_discard_const5.test | COMPILE_FAIL | 0.278945 | 0.445785 | 0.590381 | — | — | — |
| fail_any_cast_discard_const6.test | COMPILE_FAIL | 0.281723 | 0.498823 | 0.585415 | — | — | — |
| fail_any_cast_pointer_to_ref.test | COMPILE_FAIL | 0.242151 | 0.481305 | 0.553303 | — | — | — |
| fail_any_cast_pointer_to_val.test | COMPILE_FAIL | 0.220782 | 0.475550 | 0.480916 | — | — | — |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114108) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114108) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114108).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
