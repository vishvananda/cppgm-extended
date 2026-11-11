# libs/type_erasure/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_binding.test | RUN | 0.687286 | 2.518988 | 2.075534 | 0.002829 | 0.003067 | 0.002980 |
| test_increment.test | RUN | 0.688109 | 1.900756 | 1.815488 | 0.003246 | 0.003109 | 0.003014 |
| test_add.test | RUN | 0.873471 | 3.047154 | 2.716365 | 0.003405 | 0.003354 | 0.003045 |
| test_add_assign.test | RUN | 0.864094 | 3.270227 | 2.845749 | 0.003125 | 0.003379 | 0.003294 |
| test_callable.test | RUN | 1.434210 | 5.602732 | 6.236747 | 0.004785 | 0.004791 | 0.004543 |
| test_reference.test | RUN | 0.674915 | 2.273556 | 1.901748 | 0.003357 | 0.003227 | 0.003079 |
| test_construct.test | RUN | 2.926822 | 26.973030 | 18.801844 | 0.006179 | 0.005925 | 0.005850 |
| test_relaxed.test | RUN | 0.635650 | 1.605312 | 1.687625 | 0.003100 | 0.002919 | 0.002879 |
| test_assign.test | RUN | 8.285197 | 90.235653 | 68.836964 | 0.009556 | 0.005737 | 0.006332 |
| test_construct_ref.test | RUN | 0.831091 | 4.061661 | 2.765957 | 0.003823 | 0.003699 | 0.003748 |
| test_construct_cref.test | RUN | 0.954670 | 3.709501 | 3.635468 | 0.004317 | 0.004177 | 0.004230 |
| test_any_cast.test | RUN | 0.766091 | 3.872472 | 2.695488 | 0.004027 | 0.003799 | 0.003609 |
| test_binding_of.test | RUN | 0.616388 | 1.440365 | 1.581017 | 0.003254 | 0.003163 | 0.002920 |
| test_typeid_of.test | RUN | 0.676370 | 2.159147 | 1.860217 | 0.003348 | 0.003193 | 0.003139 |
| test_nested.test | RUN | 0.708215 | 2.092326 | 2.092921 | 0.003197 | 0.003179 | 0.002952 |
| test_less.test | RUN | 0.952928 | 4.070797 | 3.437587 | 0.003938 | 0.003778 | 0.003869 |
| test_equal.test | RUN | 0.864407 | 3.934805 | 2.890470 | 0.003810 | 0.003626 | 0.003548 |
| test_negate.test | RUN | 0.623692 | 1.523754 | 1.625902 | 0.002991 | 0.002644 | 0.003048 |
| test_dereference.test | RUN | 0.708402 | 2.068619 | 1.805253 | 0.003083 | 0.003163 | 0.002838 |
| test_subscript.test | RUN | 0.722403 | 2.330266 | 2.059765 | 0.003361 | 0.003207 | 0.003144 |
| test_forward_iterator.test | RUN | 0.856218 | 2.543486 | 2.367807 | 0.003272 | 0.003219 | 0.002867 |
| test_tuple.test | RUN | 0.739194 | 2.604167 | 2.163220 | 0.003550 | 0.003213 | 0.003204 |
| test_stream.test | RUN | 0.971960 | 4.202182 | 3.472018 | 0.004281 | 0.003993 | 0.004131 |
| test_deduced.test | RUN | 0.641664 | 1.514650 | 1.549580 | 0.002983 | 0.003038 | 0.002848 |
| test_same_type.test | RUN | 0.705070 | 1.998511 | 2.010088 | 0.003241 | 0.002877 | 0.003082 |
| test_member.test | RUN | 0.900290 | 4.179578 | 3.022668 | 0.004085 | 0.004274 | 0.004110 |
| test_null.test | RUN | 0.639615 | 1.993778 | 1.735497 | 0.003185 | 0.003144 | 0.002886 |
| test_free.test | RUN | 0.783602 | 3.505592 | 2.493032 | 0.003689 | 0.003683 | 0.003602 |
| test_is_empty.test | RUN | 0.605017 | 1.663506 | 1.621095 | 0.003352 | 0.002878 | 0.003018 |
| test_dynamic_any_cast.test | RUN | 1.154496 | 4.926966 | 4.702815 | 0.004473 | 0.004164 | 0.004241 |
| test_limits.test | RUN | 1.025893 | 3.428528 | 3.191736 | 0.002735 | 0.003337 | 0.002975 |
| test_param.test | COMPILE | 0.440892 | 0.914390 | 1.049061 | — | — | — |
| test_is_subconcept.test | COMPILE | 0.415685 | 0.704346 | 0.918338 | — | — | — |
| test_sfinae.test | COMPILE | 0.415710 | 0.742941 | 0.951758 | — | — | — |
| test_interface_order.test | COMPILE | 0.400201 | 0.718300 | 0.888347 | — | — | — |
| test_is_placeholder.test | COMPILE | 0.052397 | 0.068964 | 0.072895 | — | — | — |
| fail_default_construct.test | COMPILE_FAIL | 0.398594 | 0.691727 | 0.841172 | — | — | — |
| fail_construct_mismatch.test | COMPILE_FAIL | 0.406942 | 0.727217 | 0.780540 | — | — | — |
| fail_construct_mismatch_ref.test | COMPILE_FAIL | 0.383578 | 0.687575 | 0.848934 | — | — | — |
| fail_construct_mismatch_cref.test | COMPILE_FAIL | 0.388921 | 0.700066 | 0.848549 | — | — | — |
| fail_construct_mismatch_rref.test | COMPILE_FAIL | 0.400496 | 0.714343 | 0.771203 | — | — | — |
| fail_binding_convert_no_mapping.test | COMPILE_FAIL | 0.380111 | 0.664414 | 0.822841 | — | — | — |
| fail_increment_discard_const.test | COMPILE_FAIL | 0.387634 | 0.708478 | 0.850271 | — | — | — |
| fail_not_incrementable.test | COMPILE_FAIL | 0.406281 | 0.744810 | 0.818257 | — | — | — |
| fail_ref_assign.test | COMPILE_FAIL | 0.386199 | 0.674120 | 0.829230 | — | — | — |
| fail_cref_assign.test | COMPILE_FAIL | 0.386068 | 0.706081 | 0.857926 | — | — | — |
| fail_rref_assign.test | COMPILE_FAIL | 0.387684 | 0.707586 | 0.767431 | — | — | — |
| fail_ref_discard_const.test | COMPILE_FAIL | 0.386267 | 0.703976 | 0.870630 | — | — | — |
| fail_ref_discard_const_convert.test | COMPILE_FAIL | 0.392404 | 0.748355 | 0.897511 | — | — | — |
| fail_ref_discard_const_convert_ref.test | COMPILE_FAIL | 0.396783 | 0.765383 | 0.826056 | — | — | — |
| fail_ref_discard_const_convert_cref.test | COMPILE_FAIL | 0.391585 | 0.755640 | 0.899430 | — | — | — |
| fail_ref_discard_const_init.test | COMPILE_FAIL | 0.384109 | 0.718080 | 0.832371 | — | — | — |
| fail_any_cast_discard_const1.test | COMPILE_FAIL | 0.391374 | 0.725802 | 0.858637 | — | — | — |
| fail_any_cast_discard_const2.test | COMPILE_FAIL | 0.396562 | 0.707839 | 0.885092 | — | — | — |
| fail_any_cast_discard_const3.test | COMPILE_FAIL | 0.392753 | 0.720155 | 0.892256 | — | — | — |
| fail_any_cast_discard_const4.test | COMPILE_FAIL | 0.392582 | 0.693056 | 0.808825 | — | — | — |
| fail_any_cast_discard_const5.test | COMPILE_FAIL | 0.395196 | 0.728246 | 0.885559 | — | — | — |
| fail_any_cast_discard_const6.test | COMPILE_FAIL | 0.332705 | 0.720791 | 0.890730 | — | — | — |
| fail_any_cast_pointer_to_ref.test | COMPILE_FAIL | 0.386531 | 0.697689 | 0.841126 | — | — | — |
| fail_any_cast_pointer_to_val.test | COMPILE_FAIL | 0.378156 | 0.631884 | 0.600907 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.924× | 0.932×, 0.917× | 0.958× |
| clang | 0.956× | 0.955×, 0.957× | 1.002× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| test_add.test | 9 | 0.002221s | 0.001890s | 0.002140s | byte equality observed |
| test_add_assign.test | 10 | 0.002233s | 0.001975s | 0.002104s | byte equality observed |
| test_any_cast.test | 8 | 0.002522s | 0.002159s | 0.002380s | byte equality observed |
| test_assign.test | 4 | 0.006876s | 0.005246s | 0.006460s | byte equality observed |
| test_binding.test | 10 | 0.002159s | 0.002090s | 0.002059s | byte equality observed |
| test_binding_of.test | 10 | 0.002056s | 0.001964s | 0.001928s | byte equality observed |
| test_callable.test | 8 | 0.002995s | 0.002850s | 0.002805s | byte equality observed |
| test_construct.test | 6 | 0.003859s | 0.003739s | 0.003613s | byte equality observed |
| test_construct_cref.test | 8 | 0.002779s | 0.002738s | 0.002644s | byte equality observed |
| test_construct_ref.test | 9 | 0.002511s | 0.002434s | 0.002436s | byte equality observed |
| test_deduced.test | 10 | 0.002062s | 0.001958s | 0.001900s | byte equality observed |
| test_dereference.test | 10 | 0.002159s | 0.002065s | 0.002050s | byte equality observed |
| test_dynamic_any_cast.test | 8 | 0.002726s | 0.002658s | 0.002657s | byte equality observed |
| test_equal.test | 9 | 0.002365s | 0.002343s | 0.002295s | byte equality observed |
| test_forward_iterator.test | 10 | 0.002116s | 0.002061s | 0.002057s | byte equality observed |
| test_free.test | 9 | 0.002378s | 0.002263s | 0.002303s | byte equality observed |
| test_increment.test | 10 | 0.002107s | 0.002041s | 0.001987s | byte equality observed |
| test_is_empty.test | 10 | 0.002076s | 0.001972s | 0.001948s | byte equality observed |
| test_less.test | 8 | 0.002559s | 0.002418s | 0.002388s | byte equality observed |
| test_limits.test | 10 | 0.002234s | 0.002158s | 0.002074s | byte equality observed |
| test_member.test | 8 | 0.002761s | 0.002671s | 0.002617s | byte equality observed |
| test_negate.test | 9 | 0.002045s | 0.001990s | 0.001973s | byte equality observed |
| test_nested.test | 10 | 0.002065s | 0.002060s | 0.001990s | byte equality observed |
| test_null.test | 9 | 0.002123s | 0.002058s | 0.002005s | byte equality observed |
| test_reference.test | 10 | 0.002205s | 0.002066s | 0.002044s | byte equality observed |
| test_relaxed.test | 10 | 0.002107s | 0.002038s | 0.001995s | byte equality observed |
| test_same_type.test | 9 | 0.002144s | 0.002080s | 0.001990s | byte equality observed |
| test_stream.test | 8 | 0.002656s | 0.002561s | 0.002537s | byte equality observed |
| test_subscript.test | 9 | 0.002189s | 0.002105s | 0.002076s | byte equality observed |
| test_tuple.test | 9 | 0.002233s | 0.002103s | 0.002083s | byte equality observed |
| test_typeid_of.test | 9 | 0.002189s | 0.002095s | 0.002081s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
