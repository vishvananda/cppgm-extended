# libs/type_erasure/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_binding.test | RUN | 0.590859 | 2.019553 | 1.852024 | 0.003466 | 0.003329 | 0.003279 |
| test_increment.test | RUN | 0.606635 | 1.514610 | 1.555805 | 0.003183 | 0.003386 | 0.003242 |
| test_add.test | RUN | 0.697216 | 2.464916 | 2.346071 | 0.003594 | 0.003319 | 0.003595 |
| test_add_assign.test | RUN | 0.710784 | 2.616237 | 2.512491 | 0.003688 | 0.003584 | 0.003597 |
| test_callable.test | RUN | 1.181602 | 4.546321 | 5.286142 | 0.005725 | 0.005844 | 0.006017 |
| test_reference.test | RUN | 0.580287 | 1.820935 | 1.688435 | 0.003177 | 0.003605 | 0.003487 |
| test_construct.test | RUN | 2.272129 | 23.459651 | 15.376562 | 0.007728 | 0.007134 | 0.007624 |
| test_relaxed.test | RUN | 0.536945 | 1.298424 | 1.497311 | 0.003228 | 0.003309 | 0.003163 |
| test_assign.test | RUN | 6.178716 | 75.789808 | 54.037253 | 0.011021 | 0.007088 | 0.007233 |
| test_construct_ref.test | RUN | 0.710674 | 3.250610 | 2.480706 | 0.004505 | 0.004182 | 0.004694 |
| test_construct_cref.test | RUN | 0.787118 | 2.899138 | 2.945396 | 0.005334 | 0.005252 | 0.005338 |
| test_any_cast.test | RUN | 0.657456 | 3.117587 | 2.425147 | 0.004164 | 0.003947 | 0.004146 |
| test_binding_of.test | RUN | 0.550251 | 1.146018 | 1.456243 | 0.003159 | 0.003339 | 0.003111 |
| test_typeid_of.test | RUN | 0.578656 | 1.653678 | 1.635035 | 0.003677 | 0.003654 | 0.003528 |
| test_nested.test | RUN | 0.623654 | 1.593978 | 1.849825 | 0.003361 | 0.003486 | 0.003110 |
| test_less.test | RUN | 0.802581 | 3.267242 | 3.037337 | 0.004785 | 0.004527 | 0.004691 |
| test_equal.test | RUN | 0.746567 | 3.118339 | 2.524383 | 0.004130 | 0.003550 | 0.004079 |
| test_negate.test | RUN | 0.561266 | 1.218179 | 1.433836 | 0.003148 | 0.003158 | 0.002971 |
| test_dereference.test | RUN | 0.609002 | 1.604058 | 1.697436 | 0.003557 | 0.003200 | 0.003384 |
| test_subscript.test | RUN | 0.619504 | 1.719432 | 1.641387 | 0.003621 | 0.003416 | 0.003604 |
| test_forward_iterator.test | RUN | 0.716204 | 1.968146 | 2.011435 | 0.003369 | 0.003328 | 0.003398 |
| test_tuple.test | RUN | 0.637001 | 2.051309 | 1.811211 | 0.003675 | 0.003495 | 0.003374 |
| test_stream.test | RUN | 0.803186 | 3.345739 | 3.014419 | 0.004611 | 0.004462 | 0.005029 |
| test_deduced.test | RUN | 0.548635 | 1.189982 | 1.485988 | 0.002977 | 0.003129 | 0.003169 |
| test_same_type.test | RUN | 0.625291 | 1.562337 | 1.604431 | 0.003346 | 0.002945 | 0.002901 |
| test_member.test | RUN | 0.773371 | 3.403172 | 2.603472 | 0.005240 | 0.005361 | 0.005380 |
| test_null.test | RUN | 0.556646 | 1.608008 | 1.590925 | 0.003284 | 0.003419 | 0.003381 |
| test_free.test | RUN | 0.659073 | 2.731490 | 2.242451 | 0.004384 | 0.004276 | 0.004203 |
| test_is_empty.test | RUN | 0.539868 | 1.365181 | 1.442134 | 0.003346 | 0.003402 | 0.003228 |
| test_dynamic_any_cast.test | RUN | 1.019637 | 4.040659 | 3.783585 | 0.005014 | 0.005257 | 0.004396 |
| test_limits.test | RUN | 0.940060 | 2.673141 | 2.923517 | 0.003199 | 0.003680 | 0.003544 |
| test_param.test | COMPILE | 0.374785 | 0.775379 | 0.954320 | — | — | — |
| test_is_subconcept.test | COMPILE | 0.391005 | 0.591015 | 0.848716 | — | — | — |
| test_sfinae.test | COMPILE | 0.363377 | 0.573722 | 0.833996 | — | — | — |
| test_interface_order.test | COMPILE | 0.357292 | 0.611858 | 0.850757 | — | — | — |
| test_is_placeholder.test | COMPILE | 0.050122 | 0.058750 | 0.073804 | — | — | — |
| fail_default_construct.test | COMPILE_FAIL | 0.339281 | 0.545941 | 0.675595 | — | — | — |
| fail_construct_mismatch.test | COMPILE_FAIL | 0.348193 | 0.590030 | 0.801440 | — | — | — |
| fail_construct_mismatch_ref.test | COMPILE_FAIL | 0.338000 | 0.579329 | 0.805576 | — | — | — |
| fail_construct_mismatch_cref.test | COMPILE_FAIL | 0.352077 | 0.590076 | 0.672063 | — | — | — |
| fail_construct_mismatch_rref.test | COMPILE_FAIL | 0.342070 | 0.595797 | 0.793005 | — | — | — |
| fail_binding_convert_no_mapping.test | COMPILE_FAIL | 0.333948 | 0.556740 | 0.775416 | — | — | — |
| fail_increment_discard_const.test | COMPILE_FAIL | 0.343886 | 0.586351 | 0.673885 | — | — | — |
| fail_not_incrementable.test | COMPILE_FAIL | 0.361536 | 0.645705 | 0.832471 | — | — | — |
| fail_ref_assign.test | COMPILE_FAIL | 0.339823 | 0.562260 | 0.776388 | — | — | — |
| fail_cref_assign.test | COMPILE_FAIL | 0.338896 | 0.591403 | 0.739188 | — | — | — |
| fail_rref_assign.test | COMPILE_FAIL | 0.339460 | 0.608994 | 0.788610 | — | — | — |
| fail_ref_discard_const.test | COMPILE_FAIL | 0.343641 | 0.560161 | 0.809814 | — | — | — |
| fail_ref_discard_const_convert.test | COMPILE_FAIL | 0.342627 | 0.625984 | 0.707008 | — | — | — |
| fail_ref_discard_const_convert_ref.test | COMPILE_FAIL | 0.345238 | 0.644532 | 0.828062 | — | — | — |
| fail_ref_discard_const_convert_cref.test | COMPILE_FAIL | 0.350053 | 0.542773 | 0.833686 | — | — | — |
| fail_ref_discard_const_init.test | COMPILE_FAIL | 0.346443 | 0.595492 | 0.703292 | — | — | — |
| fail_any_cast_discard_const1.test | COMPILE_FAIL | 0.351450 | 0.619403 | 0.822755 | — | — | — |
| fail_any_cast_discard_const2.test | COMPILE_FAIL | 0.349170 | 0.547731 | 0.814488 | — | — | — |
| fail_any_cast_discard_const3.test | COMPILE_FAIL | 0.352863 | 0.600963 | 0.773138 | — | — | — |
| fail_any_cast_discard_const4.test | COMPILE_FAIL | 0.354735 | 0.610138 | 0.826940 | — | — | — |
| fail_any_cast_discard_const5.test | COMPILE_FAIL | 0.340592 | 0.555710 | 0.698907 | — | — | — |
| fail_any_cast_discard_const6.test | COMPILE_FAIL | 0.342515 | 0.600266 | 0.832011 | — | — | — |
| fail_any_cast_pointer_to_ref.test | COMPILE_FAIL | 0.327031 | 0.560319 | 0.666392 | — | — | — |
| fail_any_cast_pointer_to_val.test | COMPILE_FAIL | 0.303542 | 0.555018 | 0.499158 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.019×† | 0.980×, 1.057× | 0.990× |
| clang | 1.002× | 1.001×, 1.003× | 0.994× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| test_add.test | 9 | 0.002045s | 0.002283s | 0.002330s | byte equality observed |
| test_add_assign.test | 9 | 0.002035s | 0.002352s | 0.002291s | byte equality observed |
| test_any_cast.test | 8 | 0.002347s | 0.002690s | 0.002663s | byte equality observed |
| test_assign.test | 4 | 0.006372s | 0.005814s | 0.006703s | byte equality observed |
| test_binding.test | 10 | 0.001984s | 0.002066s | 0.002145s | byte equality observed |
| test_binding_of.test | 10 | 0.001981s | 0.001826s | 0.002015s | byte equality observed |
| test_callable.test | 6 | 0.003294s | 0.003125s | 0.003468s | byte equality observed |
| test_construct.test | 5 | 0.004179s | 0.004185s | 0.004410s | byte equality observed |
| test_construct_cref.test | 7 | 0.002908s | 0.003091s | 0.003063s | byte equality observed |
| test_construct_ref.test | 8 | 0.002691s | 0.002608s | 0.002780s | byte equality observed |
| test_deduced.test | 10 | 0.002037s | 0.001995s | 0.001996s | byte equality observed |
| test_dereference.test | 10 | 0.002152s | 0.002107s | 0.002136s | byte equality observed |
| test_dynamic_any_cast.test | 7 | 0.003092s | 0.003044s | 0.003064s | byte equality observed |
| test_equal.test | 9 | 0.002577s | 0.002587s | 0.002492s | byte equality observed |
| test_forward_iterator.test | 10 | 0.002177s | 0.002072s | 0.002115s | byte equality observed |
| test_free.test | 9 | 0.002545s | 0.002347s | 0.002491s | byte equality observed |
| test_increment.test | 10 | 0.002101s | 0.002070s | 0.002054s | byte equality observed |
| test_is_empty.test | 10 | 0.002068s | 0.002061s | 0.002072s | byte equality observed |
| test_less.test | 8 | 0.002722s | 0.002762s | 0.002727s | byte equality observed |
| test_limits.test | 9 | 0.002185s | 0.002199s | 0.002269s | byte equality observed |
| test_member.test | 7 | 0.003176s | 0.002993s | 0.003083s | byte equality observed |
| test_negate.test | 10 | 0.002027s | 0.001995s | 0.001993s | byte equality observed |
| test_nested.test | 9 | 0.002070s | 0.002086s | 0.002060s | byte equality observed |
| test_null.test | 9 | 0.002166s | 0.002171s | 0.002166s | byte equality observed |
| test_reference.test | 9 | 0.002236s | 0.002215s | 0.002195s | byte equality observed |
| test_relaxed.test | 10 | 0.002073s | 0.002026s | 0.002055s | byte equality observed |
| test_same_type.test | 10 | 0.002168s | 0.002139s | 0.002121s | byte equality observed |
| test_stream.test | 8 | 0.002927s | 0.002930s | 0.002889s | byte equality observed |
| test_subscript.test | 9 | 0.002255s | 0.002196s | 0.002222s | byte equality observed |
| test_tuple.test | 9 | 0.002258s | 0.002204s | 0.002263s | byte equality observed |
| test_typeid_of.test | 9 | 0.002233s | 0.002238s | 0.002265s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
