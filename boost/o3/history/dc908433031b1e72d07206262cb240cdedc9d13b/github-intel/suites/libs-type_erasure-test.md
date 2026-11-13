# libs/type_erasure/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_binding.test | RUN | 0.532699 | 2.078412 | 1.684396 | 0.002686 | 0.002406 | 0.002251 |
| test_increment.test | RUN | 0.489453 | 1.575768 | 1.468357 | 0.002562 | 0.002376 | 0.002084 |
| test_add.test | RUN | 0.626114 | 2.552230 | 2.190739 | 0.002698 | 0.002683 | 0.002733 |
| test_add_assign.test | RUN | 0.669170 | 2.751707 | 2.271721 | 0.002804 | 0.002578 | 0.002347 |
| test_callable.test | RUN | 1.103143 | 4.650158 | 4.961513 | 0.003923 | 0.003834 | 0.003687 |
| test_reference.test | RUN | 0.515470 | 1.925176 | 1.555013 | 0.002119 | 0.002642 | 0.002391 |
| test_construct.test | RUN | 2.518775 | 21.181249 | 14.953801 | 0.005253 | 0.004761 | 0.004265 |
| test_relaxed.test | RUN | 0.475483 | 1.329013 | 1.370882 | 0.002481 | 0.002351 | 0.002264 |
| test_assign.test | RUN | 5.927813 | 66.756618 | 50.509264 | 0.008059 | 0.004493 | 0.005069 |
| test_construct_ref.test | RUN | 0.617130 | 3.404300 | 2.280317 | 0.003253 | 0.002997 | 0.002955 |
| test_construct_cref.test | RUN | 0.693001 | 3.072131 | 2.925832 | 0.003458 | 0.003416 | 0.003315 |
| test_any_cast.test | RUN | 0.590972 | 3.228521 | 2.223763 | 0.003132 | 0.002838 | 0.002884 |
| test_binding_of.test | RUN | 0.463566 | 1.196433 | 1.293433 | 0.002218 | 0.002371 | 0.002202 |
| test_typeid_of.test | RUN | 0.506667 | 1.794175 | 1.497856 | 0.002680 | 0.002476 | 0.002393 |
| test_nested.test | RUN | 0.538146 | 1.702352 | 1.692248 | 0.002846 | 0.002432 | 0.002556 |
| test_less.test | RUN | 0.714218 | 3.393712 | 2.816886 | 0.003416 | 0.003078 | 0.003017 |
| test_equal.test | RUN | 0.676909 | 3.276087 | 2.319381 | 0.003372 | 0.002928 | 0.002666 |
| test_negate.test | RUN | 0.466528 | 1.285239 | 1.359902 | 0.002455 | 0.002367 | 0.002178 |
| test_dereference.test | RUN | 0.535832 | 1.668514 | 1.601171 | 0.002704 | 0.002490 | 0.002276 |
| test_subscript.test | RUN | 0.550420 | 1.900550 | 1.702812 | 0.002746 | 0.002497 | 0.002329 |
| test_forward_iterator.test | RUN | 0.638159 | 2.183066 | 2.066879 | 0.002650 | 0.002496 | 0.002210 |
| test_tuple.test | RUN | 0.549034 | 2.195896 | 1.782945 | 0.002684 | 0.002522 | 0.002398 |
| test_stream.test | RUN | 0.771610 | 3.507976 | 2.639392 | 0.003510 | 0.003333 | 0.003168 |
| test_deduced.test | RUN | 0.473714 | 1.258634 | 1.344093 | 0.002555 | 0.002389 | 0.002217 |
| test_same_type.test | RUN | 0.534618 | 1.716585 | 1.586405 | 0.002761 | 0.002440 | 0.002292 |
| test_member.test | RUN | 0.702842 | 3.449209 | 2.542106 | 0.003541 | 0.003516 | 0.003260 |
| test_null.test | RUN | 0.486052 | 1.667408 | 1.411433 | 0.002728 | 0.002438 | 0.002325 |
| test_free.test | RUN | 0.614730 | 2.826849 | 1.996373 | 0.003257 | 0.002819 | 0.002652 |
| test_is_empty.test | RUN | 0.465023 | 1.386921 | 1.306536 | 0.002565 | 0.002417 | 0.002121 |
| test_dynamic_any_cast.test | RUN | 0.912680 | 4.091643 | 3.922932 | 0.003458 | 0.003150 | 0.003233 |
| test_limits.test | RUN | 0.818046 | 2.815063 | 2.694570 | 0.002595 | 0.002586 | 0.002589 |
| test_param.test | COMPILE | 0.315961 | 0.798299 | 0.838526 | — | — | — |
| test_is_subconcept.test | COMPILE | 0.299553 | 0.617511 | 0.710821 | — | — | — |
| test_sfinae.test | COMPILE | 0.301980 | 0.622655 | 0.746133 | — | — | — |
| test_interface_order.test | COMPILE | 0.296481 | 0.609166 | 0.694113 | — | — | — |
| test_is_placeholder.test | COMPILE | 0.033324 | 0.054798 | 0.062530 | — | — | — |
| fail_default_construct.test | COMPILE_FAIL | 0.285520 | 0.601944 | 0.695080 | — | — | — |
| fail_construct_mismatch.test | COMPILE_FAIL | 0.287045 | 0.606980 | 0.653295 | — | — | — |
| fail_construct_mismatch_ref.test | COMPILE_FAIL | 0.281700 | 0.580304 | 0.709538 | — | — | — |
| fail_construct_mismatch_cref.test | COMPILE_FAIL | 0.286011 | 0.577165 | 0.701251 | — | — | — |
| fail_construct_mismatch_rref.test | COMPILE_FAIL | 0.282315 | 0.589057 | 0.689241 | — | — | — |
| fail_binding_convert_no_mapping.test | COMPILE_FAIL | 0.276417 | 0.572135 | 0.680622 | — | — | — |
| fail_increment_discard_const.test | COMPILE_FAIL | 0.286125 | 0.580349 | 0.706709 | — | — | — |
| fail_not_incrementable.test | COMPILE_FAIL | 0.294735 | 0.610886 | 0.704218 | — | — | — |
| fail_ref_assign.test | COMPILE_FAIL | 0.286577 | 0.564870 | 0.687359 | — | — | — |
| fail_cref_assign.test | COMPILE_FAIL | 0.283415 | 0.585974 | 0.694153 | — | — | — |
| fail_rref_assign.test | COMPILE_FAIL | 0.285616 | 0.584849 | 0.669970 | — | — | — |
| fail_ref_discard_const.test | COMPILE_FAIL | 0.283471 | 0.587064 | 0.724111 | — | — | — |
| fail_ref_discard_const_convert.test | COMPILE_FAIL | 0.286572 | 0.611298 | 0.738313 | — | — | — |
| fail_ref_discard_const_convert_ref.test | COMPILE_FAIL | 0.287730 | 0.627524 | 0.693372 | — | — | — |
| fail_ref_discard_const_convert_cref.test | COMPILE_FAIL | 0.288461 | 0.628626 | 0.726488 | — | — | — |
| fail_ref_discard_const_init.test | COMPILE_FAIL | 0.282211 | 0.599120 | 0.678435 | — | — | — |
| fail_any_cast_discard_const1.test | COMPILE_FAIL | 0.288693 | 0.614552 | 0.718755 | — | — | — |
| fail_any_cast_discard_const2.test | COMPILE_FAIL | 0.286570 | 0.592959 | 0.746941 | — | — | — |
| fail_any_cast_discard_const3.test | COMPILE_FAIL | 0.289336 | 0.590917 | 0.738352 | — | — | — |
| fail_any_cast_discard_const4.test | COMPILE_FAIL | 0.289905 | 0.605927 | 0.678768 | — | — | — |
| fail_any_cast_discard_const5.test | COMPILE_FAIL | 0.289369 | 0.606337 | 0.729231 | — | — | — |
| fail_any_cast_discard_const6.test | COMPILE_FAIL | 0.288953 | 0.591565 | 0.728820 | — | — | — |
| fail_any_cast_pointer_to_ref.test | COMPILE_FAIL | 0.269843 | 0.517929 | 0.680289 | — | — | — |
| fail_any_cast_pointer_to_val.test | COMPILE_FAIL | 0.213278 | 0.432261 | 0.433044 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.956× | 0.958×, 0.954× | 0.999× |
| clang | 0.960× | 0.960×, 0.960× | 1.014× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345325).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| test_add.test | 10 | 0.001587s | 0.001490s | 0.001520s | byte equality observed |
| test_add_assign.test | 10 | 0.001540s | 0.001481s | 0.001504s | byte equality observed |
| test_any_cast.test | 10 | 0.001695s | 0.001650s | 0.001605s | byte equality observed |
| test_assign.test | 4 | 0.004668s | 0.003805s | 0.004373s | byte equality observed |
| test_binding.test | 10 | 0.001539s | 0.001418s | 0.001407s | byte equality observed |
| test_binding_of.test | 10 | 0.001443s | 0.001359s | 0.001385s | byte equality observed |
| test_callable.test | 9 | 0.002016s | 0.001908s | 0.001923s | byte equality observed |
| test_construct.test | 7 | 0.002565s | 0.002512s | 0.002396s | byte equality observed |
| test_construct_cref.test | 10 | 0.001848s | 0.001805s | 0.001789s | byte equality observed |
| test_construct_ref.test | 10 | 0.001703s | 0.001675s | 0.001676s | byte equality observed |
| test_deduced.test | 10 | 0.001410s | 0.001379s | 0.001361s | byte equality observed |
| test_dereference.test | 10 | 0.001493s | 0.001457s | 0.001406s | byte equality observed |
| test_dynamic_any_cast.test | 10 | 0.001842s | 0.001830s | 0.001828s | byte equality observed |
| test_equal.test | 9 | 0.001646s | 0.001630s | 0.001561s | byte equality observed |
| test_forward_iterator.test | 10 | 0.001492s | 0.001420s | 0.001415s | byte equality observed |
| test_free.test | 10 | 0.001644s | 0.001621s | 0.001586s | byte equality observed |
| test_increment.test | 10 | 0.001456s | 0.001385s | 0.001421s | byte equality observed |
| test_is_empty.test | 10 | 0.001465s | 0.001405s | 0.001371s | byte equality observed |
| test_less.test | 10 | 0.001771s | 0.001719s | 0.001670s | byte equality observed |
| test_limits.test | 10 | 0.001515s | 0.001511s | 0.001489s | byte equality observed |
| test_member.test | 10 | 0.001906s | 0.001874s | 0.001824s | byte equality observed |
| test_negate.test | 10 | 0.001406s | 0.001392s | 0.001360s | byte equality observed |
| test_nested.test | 10 | 0.001458s | 0.001402s | 0.001357s | byte equality observed |
| test_null.test | 10 | 0.001466s | 0.001447s | 0.001410s | byte equality observed |
| test_reference.test | 10 | 0.001512s | 0.001475s | 0.001488s | byte equality observed |
| test_relaxed.test | 10 | 0.001442s | 0.001399s | 0.001379s | byte equality observed |
| test_same_type.test | 10 | 0.001466s | 0.001432s | 0.001445s | byte equality observed |
| test_stream.test | 10 | 0.001813s | 0.001772s | 0.001755s | byte equality observed |
| test_subscript.test | 10 | 0.001513s | 0.001448s | 0.001424s | byte equality observed |
| test_tuple.test | 10 | 0.001508s | 0.001447s | 0.001481s | byte equality observed |
| test_typeid_of.test | 9 | 0.001522s | 0.001541s | 0.001469s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345325).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
