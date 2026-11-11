# libs/type_erasure/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_binding.test | RUN | 0.571197 | 1.363315 | 1.656838 | 0.004458 | 0.004714 | 0.004888 |
| test_increment.test | RUN | 0.568840 | 1.266832 | 1.561990 | 0.004193 | 0.004184 | 0.004460 |
| test_add.test | RUN | 0.686832 | 1.957682 | 2.143615 | 0.004302 | 0.004766 | 0.004941 |
| test_add_assign.test | RUN | 0.703981 | 2.113384 | 2.193390 | 0.004304 | 0.004867 | 0.004950 |
| test_callable.test | RUN | 1.062307 | 4.574034 | 4.494756 | 0.006653 | 0.008144 | 0.007970 |
| test_reference.test | RUN | 0.560965 | 1.306234 | 1.527759 | 0.004094 | 0.004894 | 0.004778 |
| test_construct.test | RUN | 1.515473 | 9.396682 | 8.224083 | 0.010428 | 0.010798 | 0.010992 |
| test_relaxed.test | RUN | 0.548966 | 1.195570 | 1.494501 | 0.004246 | 0.004409 | 0.004413 |
| test_assign.test | RUN | 2.745415 | 29.060801 | 21.653686 | 0.017384 | 0.012369 | 0.017834 |
| test_construct_ref.test | RUN | 0.661599 | 1.655191 | 2.002426 | 0.005204 | 0.006035 | 0.006402 |
| test_construct_cref.test | RUN | 0.682264 | 1.965629 | 2.336751 | 0.006701 | 0.006511 | 0.006810 |
| test_any_cast.test | RUN | 0.582098 | 1.581727 | 1.846858 | 0.005542 | 0.005865 | 0.005393 |
| test_binding_of.test | RUN | 0.535344 | 1.117946 | 1.468430 | 0.003820 | 0.004452 | 0.004286 |
| test_typeid_of.test | RUN | 0.600401 | 1.243733 | 1.511190 | 0.004815 | 0.004814 | 0.004647 |
| test_nested.test | RUN | 0.601359 | 1.358564 | 1.709454 | 0.004078 | 0.004369 | 0.004253 |
| test_less.test | RUN | 0.702641 | 2.226752 | 2.432901 | 0.005648 | 0.006158 | 0.006188 |
| test_equal.test | RUN | 0.683012 | 2.050808 | 2.226119 | 0.005057 | 0.005651 | 0.005487 |
| test_negate.test | RUN | 0.559251 | 1.163033 | 1.461613 | 0.003664 | 0.004026 | 0.004004 |
| test_dereference.test | RUN | 0.592517 | 1.352283 | 1.683421 | 0.004354 | 0.004634 | 0.004299 |
| test_subscript.test | RUN | 0.614007 | 1.390038 | 1.742601 | 0.004208 | 0.004678 | 0.004776 |
| test_forward_iterator.test | RUN | 0.696779 | 1.997550 | 2.207346 | 0.004305 | 0.004391 | 0.004619 |
| test_tuple.test | RUN | 0.616579 | 1.492030 | 1.742904 | 0.004363 | 0.004661 | 0.004572 |
| test_stream.test | RUN | 0.728712 | 2.385976 | 2.396726 | 0.005821 | 0.006301 | 0.006439 |
| test_deduced.test | RUN | 0.550363 | 1.161188 | 1.410925 | 0.003668 | 0.004187 | 0.004081 |
| test_same_type.test | RUN | 0.602333 | 1.308612 | 1.578309 | 0.004277 | 0.004556 | 0.004664 |
| test_member.test | RUN | 0.701689 | 2.172076 | 2.479290 | 0.006274 | 0.006213 | 0.006891 |
| test_null.test | RUN | 0.545671 | 1.220528 | 1.490099 | 0.004149 | 0.004318 | 0.004574 |
| test_free.test | RUN | 0.654346 | 1.641505 | 1.878157 | 0.004907 | 0.005143 | 0.005928 |
| test_is_empty.test | RUN | 0.553200 | 1.109594 | 1.382984 | 0.004072 | 0.004503 | 0.004558 |
| test_dynamic_any_cast.test | RUN | 0.845512 | 3.127076 | 3.228846 | 0.006012 | 0.006887 | 0.006173 |
| test_limits.test | RUN | 0.879365 | 2.925992 | 2.863776 | 0.004451 | 0.005105 | 0.005052 |
| test_param.test | COMPILE | 0.372130 | 0.731081 | 0.995225 | — | — | — |
| test_is_subconcept.test | COMPILE | 0.367362 | 0.640764 | 0.767207 | — | — | — |
| test_sfinae.test | COMPILE | 0.377250 | 0.629626 | 0.850071 | — | — | — |
| test_interface_order.test | COMPILE | 0.366023 | 0.600445 | 0.748754 | — | — | — |
| test_is_placeholder.test | COMPILE | 0.052334 | 0.054222 | 0.071472 | — | — | — |
| fail_default_construct.test | COMPILE_FAIL | 0.360017 | 0.573331 | 0.803631 | — | — | — |
| fail_construct_mismatch.test | COMPILE_FAIL | 0.363555 | 0.622944 | 0.812970 | — | — | — |
| fail_construct_mismatch_ref.test | COMPILE_FAIL | 0.350690 | 0.581762 | 0.804117 | — | — | — |
| fail_construct_mismatch_cref.test | COMPILE_FAIL | 0.352506 | 0.569714 | 0.800614 | — | — | — |
| fail_construct_mismatch_rref.test | COMPILE_FAIL | 0.351026 | 0.599187 | 0.787415 | — | — | — |
| fail_binding_convert_no_mapping.test | COMPILE_FAIL | 0.345779 | 0.565794 | 0.771320 | — | — | — |
| fail_increment_discard_const.test | COMPILE_FAIL | 0.351058 | 0.572675 | 0.806530 | — | — | — |
| fail_not_incrementable.test | COMPILE_FAIL | 0.366993 | 0.528122 | 0.844993 | — | — | — |
| fail_ref_assign.test | COMPILE_FAIL | 0.352717 | 0.570960 | 0.778885 | — | — | — |
| fail_cref_assign.test | COMPILE_FAIL | 0.356943 | 0.560367 | 0.802554 | — | — | — |
| fail_rref_assign.test | COMPILE_FAIL | 0.345670 | 0.584837 | 0.796540 | — | — | — |
| fail_ref_discard_const.test | COMPILE_FAIL | 0.341344 | 0.580903 | 0.808389 | — | — | — |
| fail_ref_discard_const_convert.test | COMPILE_FAIL | 0.357835 | 0.605381 | 0.829163 | — | — | — |
| fail_ref_discard_const_convert_ref.test | COMPILE_FAIL | 0.359596 | 0.640466 | 0.842377 | — | — | — |
| fail_ref_discard_const_convert_cref.test | COMPILE_FAIL | 0.360905 | 0.613096 | 0.843047 | — | — | — |
| fail_ref_discard_const_init.test | COMPILE_FAIL | 0.355049 | 0.555138 | 0.793702 | — | — | — |
| fail_any_cast_discard_const1.test | COMPILE_FAIL | 0.357838 | 0.619212 | 0.851446 | — | — | — |
| fail_any_cast_discard_const2.test | COMPILE_FAIL | 0.364421 | 0.585983 | 0.859945 | — | — | — |
| fail_any_cast_discard_const3.test | COMPILE_FAIL | 0.363117 | 0.622153 | 0.859606 | — | — | — |
| fail_any_cast_discard_const4.test | COMPILE_FAIL | 0.363885 | 0.654435 | 0.855164 | — | — | — |
| fail_any_cast_discard_const5.test | COMPILE_FAIL | 0.362298 | 0.589093 | 0.835838 | — | — | — |
| fail_any_cast_discard_const6.test | COMPILE_FAIL | 0.360297 | 0.598466 | 0.785664 | — | — | — |
| fail_any_cast_pointer_to_ref.test | COMPILE_FAIL | 0.321776 | 0.575958 | 0.775602 | — | — | — |
| fail_any_cast_pointer_to_val.test | COMPILE_FAIL | 0.291558 | 0.475450 | 0.768041 | — | — | — |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42570601152) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42570601152) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42570601152).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
