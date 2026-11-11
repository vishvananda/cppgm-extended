# libs/type_erasure/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_binding.test | RUN | 0.389382 | 1.002253 | 1.036264 | 0.003928 | 0.003859 | 0.003650 |
| test_increment.test | RUN | 0.396559 | 0.936839 | 0.983083 | 0.003499 | 0.003670 | 0.003292 |
| test_add.test | RUN | 0.475966 | 1.523405 | 1.384528 | 0.003700 | 0.004231 | 0.003831 |
| test_add_assign.test | RUN | 0.499572 | 1.585219 | 1.359534 | 0.003619 | 0.003840 | 0.003643 |
| test_callable.test | RUN | 0.758855 | 3.574527 | 2.898486 | 0.005763 | 0.006056 | 0.005908 |
| test_reference.test | RUN | 0.416136 | 0.972120 | 0.994030 | 0.003396 | 0.003892 | 0.003920 |
| test_construct.test | RUN | 1.061935 | 7.154737 | 5.417580 | 0.008062 | 0.008193 | 0.008749 |
| test_relaxed.test | RUN | 0.402588 | 0.865418 | 0.953959 | 0.003381 | 0.003792 | 0.003424 |
| test_assign.test | RUN | 1.962439 | 22.621832 | 13.897286 | 0.014072 | 0.009593 | 0.014644 |
| test_construct_ref.test | RUN | 0.451027 | 1.244420 | 1.293275 | 0.004578 | 0.004759 | 0.004773 |
| test_construct_cref.test | RUN | 0.487716 | 1.399595 | 1.387214 | 0.005112 | 0.005572 | 0.005319 |
| test_any_cast.test | RUN | 0.422579 | 1.177607 | 1.123714 | 0.004212 | 0.004739 | 0.004851 |
| test_binding_of.test | RUN | 0.388281 | 0.808148 | 0.873173 | 0.003153 | 0.003542 | 0.003442 |
| test_typeid_of.test | RUN | 0.415097 | 0.904535 | 0.954731 | 0.003707 | 0.003602 | 0.003813 |
| test_nested.test | RUN | 0.440475 | 1.069415 | 1.134601 | 0.003677 | 0.003590 | 0.003477 |
| test_less.test | RUN | 0.506326 | 1.663991 | 1.568965 | 0.004477 | 0.004755 | 0.004908 |
| test_equal.test | RUN | 0.499677 | 1.542343 | 1.388815 | 0.003835 | 0.004148 | 0.003828 |
| test_negate.test | RUN | 0.373453 | 0.879201 | 0.934196 | 0.003235 | 0.003512 | 0.002972 |
| test_dereference.test | RUN | 0.417363 | 1.039847 | 1.080517 | 0.003663 | 0.003827 | 0.003467 |
| test_subscript.test | RUN | 0.442372 | 1.037542 | 1.123306 | 0.003316 | 0.003873 | 0.003665 |
| test_forward_iterator.test | RUN | 0.502961 | 1.508606 | 1.419151 | 0.003471 | 0.003808 | 0.003719 |
| test_tuple.test | RUN | 0.433391 | 1.084692 | 1.105687 | 0.003631 | 0.003802 | 0.003425 |
| test_stream.test | RUN | 0.519794 | 1.816045 | 1.605544 | 0.004524 | 0.005127 | 0.005098 |
| test_deduced.test | RUN | 0.395966 | 0.854159 | 0.920669 | 0.003061 | 0.003387 | 0.003103 |
| test_same_type.test | RUN | 0.427976 | 0.981537 | 1.059643 | 0.003122 | 0.004075 | 0.003596 |
| test_member.test | RUN | 0.490824 | 1.742181 | 1.591619 | 0.005252 | 0.005583 | 0.005439 |
| test_null.test | RUN | 0.390979 | 0.895907 | 0.953558 | 0.003372 | 0.003358 | 0.003494 |
| test_free.test | RUN | 0.459976 | 1.176788 | 1.282191 | 0.003843 | 0.004394 | 0.004381 |
| test_is_empty.test | RUN | 0.387981 | 0.814731 | 0.904959 | 0.003253 | 0.003716 | 0.003553 |
| test_dynamic_any_cast.test | RUN | 0.577397 | 2.368147 | 2.136615 | 0.004818 | 0.005470 | 0.005218 |
| test_limits.test | RUN | 0.606705 | 2.214730 | 1.777036 | 0.003707 | 0.004086 | 0.003795 |
| test_param.test | COMPILE | 0.262177 | 0.538458 | 0.564634 | — | — | — |
| test_is_subconcept.test | COMPILE | 0.267292 | 0.473467 | 0.480662 | — | — | — |
| test_sfinae.test | COMPILE | 0.268121 | 0.461872 | 0.514510 | — | — | — |
| test_interface_order.test | COMPILE | 0.257919 | 0.431473 | 0.502976 | — | — | — |
| test_is_placeholder.test | COMPILE | 0.036309 | 0.040742 | 0.051451 | — | — | — |
| fail_default_construct.test | COMPILE_FAIL | 0.246197 | 0.407885 | 0.502318 | — | — | — |
| fail_construct_mismatch.test | COMPILE_FAIL | 0.254513 | 0.443320 | 0.506011 | — | — | — |
| fail_construct_mismatch_ref.test | COMPILE_FAIL | 0.252216 | 0.416192 | 0.491916 | — | — | — |
| fail_construct_mismatch_cref.test | COMPILE_FAIL | 0.247571 | 0.408740 | 0.496625 | — | — | — |
| fail_construct_mismatch_rref.test | COMPILE_FAIL | 0.245337 | 0.440881 | 0.499040 | — | — | — |
| fail_binding_convert_no_mapping.test | COMPILE_FAIL | 0.238777 | 0.410551 | 0.478364 | — | — | — |
| fail_increment_discard_const.test | COMPILE_FAIL | 0.248866 | 0.411463 | 0.491382 | — | — | — |
| fail_not_incrementable.test | COMPILE_FAIL | 0.256922 | 0.421367 | 0.525216 | — | — | — |
| fail_ref_assign.test | COMPILE_FAIL | 0.245136 | 0.410870 | 0.482403 | — | — | — |
| fail_cref_assign.test | COMPILE_FAIL | 0.245867 | 0.475634 | 0.494060 | — | — | — |
| fail_rref_assign.test | COMPILE_FAIL | 0.248776 | 0.610595 | 0.490771 | — | — | — |
| fail_ref_discard_const.test | COMPILE_FAIL | 0.247654 | 0.537387 | 0.502225 | — | — | — |
| fail_ref_discard_const_convert.test | COMPILE_FAIL | 0.247419 | 0.537423 | 0.514309 | — | — | — |
| fail_ref_discard_const_convert_ref.test | COMPILE_FAIL | 0.251532 | 0.453716 | 0.512432 | — | — | — |
| fail_ref_discard_const_convert_cref.test | COMPILE_FAIL | 0.250990 | 0.471312 | 0.512389 | — | — | — |
| fail_ref_discard_const_init.test | COMPILE_FAIL | 0.246120 | 0.413941 | 0.489150 | — | — | — |
| fail_any_cast_discard_const1.test | COMPILE_FAIL | 0.254320 | 0.434012 | 0.514840 | — | — | — |
| fail_any_cast_discard_const2.test | COMPILE_FAIL | 0.251668 | 0.451110 | 0.511902 | — | — | — |
| fail_any_cast_discard_const3.test | COMPILE_FAIL | 0.254806 | 0.419953 | 0.513653 | — | — | — |
| fail_any_cast_discard_const4.test | COMPILE_FAIL | 0.251425 | 0.400531 | 0.519743 | — | — | — |
| fail_any_cast_discard_const5.test | COMPILE_FAIL | 0.249536 | 0.449977 | 0.517057 | — | — | — |
| fail_any_cast_discard_const6.test | COMPILE_FAIL | 0.247514 | 0.366911 | 0.508279 | — | — | — |
| fail_any_cast_pointer_to_ref.test | COMPILE_FAIL | 0.232127 | 0.360846 | 0.416880 | — | — | — |
| fail_any_cast_pointer_to_val.test | COMPILE_FAIL | 0.178401 | 0.415671 | 0.371823 | — | — | — |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872153) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872153) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872153).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
