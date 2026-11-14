# libs/type_erasure/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_binding.test | RUN | 0.512046 | 2.052233 | 1.924987 | 0.003514 | 0.003598 | 0.003419 |
| test_increment.test | RUN | 0.500936 | 1.536910 | 1.664220 | 0.002841 | 0.003301 | 0.003224 |
| test_add.test | RUN | 0.637416 | 2.452513 | 2.469136 | 0.003537 | 0.003508 | 0.003422 |
| test_add_assign.test | RUN | 0.685234 | 2.616553 | 2.580002 | 0.003215 | 0.003540 | 0.003924 |
| test_callable.test | RUN | 1.058307 | 4.579461 | 5.680238 | 0.005755 | 0.005955 | 0.006226 |
| test_reference.test | RUN | 0.527067 | 1.857449 | 1.744328 | 0.003516 | 0.003616 | 0.003520 |
| test_construct.test | RUN | 2.372225 | 22.786903 | 16.545012 | 0.007448 | 0.007320 | 0.006478 |
| test_relaxed.test | RUN | 0.488611 | 1.316450 | 1.575020 | 0.003245 | 0.003082 | 0.003165 |
| test_assign.test | RUN | 5.364059 | 73.695328 | 63.633281 | 0.010658 | 0.006986 | 0.007455 |
| test_construct_ref.test | RUN | 0.624983 | 3.261196 | 2.558330 | 0.004584 | 0.004185 | 0.004499 |
| test_construct_cref.test | RUN | 0.748459 | 2.922383 | 3.257570 | 0.005166 | 0.005461 | 0.005159 |
| test_any_cast.test | RUN | 0.586490 | 3.058232 | 2.414501 | 0.004438 | 0.004348 | 0.004688 |
| test_binding_of.test | RUN | 0.474376 | 1.141624 | 1.473111 | 0.002854 | 0.003155 | 0.003086 |
| test_typeid_of.test | RUN | 0.531327 | 1.751814 | 1.699371 | 0.003530 | 0.003687 | 0.003699 |
| test_nested.test | RUN | 0.571355 | 1.709083 | 1.939928 | 0.003399 | 0.003264 | 0.003377 |
| test_less.test | RUN | 0.725131 | 3.290572 | 3.244092 | 0.004597 | 0.004189 | 0.004503 |
| test_equal.test | RUN | 0.670049 | 3.191886 | 2.690612 | 0.004038 | 0.004160 | 0.004419 |
| test_negate.test | RUN | 0.508858 | 1.268812 | 1.529662 | 0.002993 | 0.003057 | 0.003293 |
| test_dereference.test | RUN | 0.549269 | 1.556728 | 1.839782 | 0.003404 | 0.003413 | 0.003399 |
| test_subscript.test | RUN | 0.554632 | 1.797955 | 1.962732 | 0.003261 | 0.003484 | 0.003460 |
| test_forward_iterator.test | RUN | 0.656200 | 2.124595 | 2.336506 | 0.003385 | 0.003391 | 0.003586 |
| test_tuple.test | RUN | 0.572227 | 2.183570 | 1.736645 | 0.003669 | 0.003397 | 0.003382 |
| test_stream.test | RUN | 0.724215 | 3.357435 | 3.218639 | 0.004459 | 0.004576 | 0.004902 |
| test_deduced.test | RUN | 0.494884 | 1.154475 | 1.333785 | 0.002828 | 0.002912 | 0.002922 |
| test_same_type.test | RUN | 0.532091 | 1.568994 | 1.838519 | 0.003335 | 0.003310 | 0.003344 |
| test_member.test | RUN | 0.688061 | 3.266771 | 2.660811 | 0.005049 | 0.005385 | 0.004797 |
| test_null.test | RUN | 0.481807 | 1.539995 | 1.607795 | 0.003348 | 0.003356 | 0.003469 |
| test_free.test | RUN | 0.595196 | 2.751268 | 2.345053 | 0.004003 | 0.003990 | 0.003814 |
| test_is_empty.test | RUN | 0.484505 | 1.350395 | 1.544441 | 0.003306 | 0.003222 | 0.003229 |
| test_dynamic_any_cast.test | RUN | 0.878092 | 3.916550 | 4.344757 | 0.004698 | 0.004958 | 0.004339 |
| test_limits.test | RUN | 0.841072 | 2.846236 | 2.659402 | 0.003107 | 0.003345 | 0.003250 |
| test_param.test | COMPILE | 0.315189 | 0.749643 | 1.011897 | — | — | — |
| test_is_subconcept.test | COMPILE | 0.320690 | 0.625516 | 0.912200 | — | — | — |
| test_sfinae.test | COMPILE | 0.313582 | 0.663783 | 0.802937 | — | — | — |
| test_interface_order.test | COMPILE | 0.309734 | 0.597928 | 0.821567 | — | — | — |
| test_is_placeholder.test | COMPILE | 0.039185 | 0.058942 | 0.070986 | — | — | — |
| fail_default_construct.test | COMPILE_FAIL | 0.296929 | 0.593809 | 0.811740 | — | — | — |
| fail_construct_mismatch.test | COMPILE_FAIL | 0.300364 | 0.601260 | 0.689521 | — | — | — |
| fail_construct_mismatch_ref.test | COMPILE_FAIL | 0.294960 | 0.568997 | 0.812923 | — | — | — |
| fail_construct_mismatch_cref.test | COMPILE_FAIL | 0.292712 | 0.584519 | 0.754506 | — | — | — |
| fail_construct_mismatch_rref.test | COMPILE_FAIL | 0.295283 | 0.584521 | 0.804893 | — | — | — |
| fail_binding_convert_no_mapping.test | COMPILE_FAIL | 0.285559 | 0.563378 | 0.775006 | — | — | — |
| fail_increment_discard_const.test | COMPILE_FAIL | 0.298242 | 0.576580 | 0.674484 | — | — | — |
| fail_not_incrementable.test | COMPILE_FAIL | 0.309530 | 0.605822 | 0.854433 | — | — | — |
| fail_ref_assign.test | COMPILE_FAIL | 0.299704 | 0.574033 | 0.788383 | — | — | — |
| fail_cref_assign.test | COMPILE_FAIL | 0.294984 | 0.586588 | 0.676034 | — | — | — |
| fail_rref_assign.test | COMPILE_FAIL | 0.296442 | 0.532704 | 0.798521 | — | — | — |
| fail_ref_discard_const.test | COMPILE_FAIL | 0.293551 | 0.593278 | 0.817018 | — | — | — |
| fail_ref_discard_const_convert.test | COMPILE_FAIL | 0.299258 | 0.632226 | 0.698719 | — | — | — |
| fail_ref_discard_const_convert_ref.test | COMPILE_FAIL | 0.306538 | 0.554914 | 0.851521 | — | — | — |
| fail_ref_discard_const_convert_cref.test | COMPILE_FAIL | 0.306731 | 0.634645 | 0.851805 | — | — | — |
| fail_ref_discard_const_init.test | COMPILE_FAIL | 0.300154 | 0.541039 | 0.657465 | — | — | — |
| fail_any_cast_discard_const1.test | COMPILE_FAIL | 0.298992 | 0.618373 | 0.703522 | — | — | — |
| fail_any_cast_discard_const2.test | COMPILE_FAIL | 0.298646 | 0.611727 | 0.844553 | — | — | — |
| fail_any_cast_discard_const3.test | COMPILE_FAIL | 0.300293 | 0.569040 | 0.842354 | — | — | — |
| fail_any_cast_discard_const4.test | COMPILE_FAIL | 0.302301 | 0.586511 | 0.831919 | — | — | — |
| fail_any_cast_discard_const5.test | COMPILE_FAIL | 0.299715 | 0.650359 | 0.841053 | — | — | — |
| fail_any_cast_discard_const6.test | COMPILE_FAIL | 0.303832 | 0.616813 | 0.768079 | — | — | — |
| fail_any_cast_pointer_to_ref.test | COMPILE_FAIL | 0.271889 | 0.610977 | 0.707487 | — | — | — |
| fail_any_cast_pointer_to_val.test | COMPILE_FAIL | 0.290812 | 0.441001 | 0.526778 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.911×† | 0.886×, 0.935× | 1.075× |
| clang | 0.917×† | 0.872×, 0.962× | 0.921× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267198).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| test_add.test | 9 | 0.001994s | 0.001888s | 0.001919s | byte equality observed |
| test_add_assign.test | 7 | 0.001994s | 0.001897s | 0.001925s | byte equality observed |
| test_any_cast.test | 6 | 0.002228s | 0.002161s | 0.002148s | byte equality observed |
| test_assign.test | 4 | 0.006039s | 0.005148s | 0.005405s | byte equality observed |
| test_binding.test | 10 | 0.001837s | 0.001843s | 0.001754s | byte equality observed |
| test_binding_of.test | 10 | 0.001767s | 0.001664s | 0.001657s | byte equality observed |
| test_callable.test | 7 | 0.002966s | 0.002838s | 0.002806s | byte equality observed |
| test_construct.test | 5 | 0.003766s | 0.003646s | 0.003583s | byte equality observed |
| test_construct_cref.test | 6 | 0.002629s | 0.002595s | 0.002506s | byte equality observed |
| test_construct_ref.test | 8 | 0.002374s | 0.002271s | 0.002282s | byte equality observed |
| test_deduced.test | 10 | 0.001751s | 0.001709s | 0.001642s | byte equality observed |
| test_dereference.test | 10 | 0.001818s | 0.001765s | 0.001737s | byte equality observed |
| test_dynamic_any_cast.test | 7 | 0.002627s | 0.002584s | 0.002576s | byte equality observed |
| test_equal.test | 8 | 0.002216s | 0.002100s | 0.002096s | byte equality observed |
| test_forward_iterator.test | 10 | 0.001873s | 0.001790s | 0.001768s | byte equality observed |
| test_free.test | 9 | 0.002188s | 0.002138s | 0.002146s | byte equality observed |
| test_increment.test | 9 | 0.001803s | 0.001709s | 0.001675s | byte equality observed |
| test_is_empty.test | 10 | 0.001834s | 0.001721s | 0.001704s | byte equality observed |
| test_less.test | 7 | 0.002425s | 0.002333s | 0.002288s | byte equality observed |
| test_limits.test | 9 | 0.001984s | 0.001849s | 0.001828s | byte equality observed |
| test_member.test | 7 | 0.002746s | 0.002659s | 0.002613s | byte equality observed |
| test_negate.test | 9 | 0.001765s | 0.001656s | 0.001655s | byte equality observed |
| test_nested.test | 10 | 0.001813s | 0.001719s | 0.001751s | byte equality observed |
| test_null.test | 9 | 0.001831s | 0.001804s | 0.001774s | byte equality observed |
| test_reference.test | 10 | 0.001912s | 0.001829s | 0.001850s | byte equality observed |
| test_relaxed.test | 10 | 0.001836s | 0.001728s | 0.001671s | byte equality observed |
| test_same_type.test | 9 | 0.001882s | 0.001786s | 0.001782s | byte equality observed |
| test_stream.test | 6 | 0.002549s | 0.002419s | 0.002419s | byte equality observed |
| test_subscript.test | 10 | 0.001937s | 0.001825s | 0.001846s | byte equality observed |
| test_tuple.test | 10 | 0.001985s | 0.001830s | 0.001821s | byte equality observed |
| test_typeid_of.test | 8 | 0.001935s | 0.001894s | 0.001845s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267198).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
