# libs/type_erasure/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_binding.test | RUN | 0.539163 | 2.005323 | 1.880206 | 0.003377 | 0.003241 | 0.003303 |
| test_increment.test | RUN | 0.494982 | 1.495418 | 1.612283 | 0.003177 | 0.003209 | 0.003236 |
| test_add.test | RUN | 0.631587 | 2.405998 | 2.378788 | 0.003199 | 0.003748 | 0.003765 |
| test_add_assign.test | RUN | 0.688920 | 2.598566 | 2.470734 | 0.003537 | 0.003558 | 0.003567 |
| test_callable.test | RUN | 1.085781 | 4.479478 | 5.361124 | 0.005608 | 0.006028 | 0.005884 |
| test_reference.test | RUN | 0.518898 | 1.804853 | 1.685306 | 0.002926 | 0.003538 | 0.003629 |
| test_construct.test | RUN | 2.309731 | 22.591680 | 15.428931 | 0.007417 | 0.007541 | 0.007606 |
| test_relaxed.test | RUN | 0.495054 | 1.285075 | 1.507820 | 0.003259 | 0.003237 | 0.003224 |
| test_assign.test | RUN | 5.220882 | 69.224141 | 52.958212 | 0.010348 | 0.006883 | 0.007207 |
| test_construct_ref.test | RUN | 0.611524 | 3.094559 | 2.503909 | 0.004492 | 0.004428 | 0.004655 |
| test_construct_cref.test | RUN | 0.683950 | 2.967112 | 3.064644 | 0.005092 | 0.005279 | 0.004877 |
| test_any_cast.test | RUN | 0.611335 | 2.996559 | 2.319707 | 0.004287 | 0.004131 | 0.004362 |
| test_binding_of.test | RUN | 0.468736 | 1.219452 | 1.451027 | 0.003128 | 0.003030 | 0.003132 |
| test_typeid_of.test | RUN | 0.499473 | 1.592096 | 1.610056 | 0.003632 | 0.003128 | 0.003595 |
| test_nested.test | RUN | 0.558320 | 1.704345 | 1.782215 | 0.003268 | 0.003376 | 0.003222 |
| test_less.test | RUN | 0.719939 | 3.265862 | 3.029118 | 0.004570 | 0.004756 | 0.004690 |
| test_equal.test | RUN | 0.677110 | 3.017284 | 2.441377 | 0.003844 | 0.003633 | 0.003963 |
| test_negate.test | RUN | 0.496924 | 1.222859 | 1.548250 | 0.003062 | 0.003319 | 0.003198 |
| test_dereference.test | RUN | 0.522884 | 1.507259 | 1.807339 | 0.003473 | 0.003130 | 0.003401 |
| test_subscript.test | RUN | 0.558681 | 1.763267 | 1.860689 | 0.003552 | 0.003430 | 0.003484 |
| test_forward_iterator.test | RUN | 0.661596 | 1.996184 | 1.974289 | 0.003426 | 0.003258 | 0.003123 |
| test_tuple.test | RUN | 0.575818 | 2.001997 | 1.939304 | 0.003530 | 0.003281 | 0.003432 |
| test_stream.test | RUN | 0.738582 | 3.359347 | 3.040586 | 0.004839 | 0.004846 | 0.004672 |
| test_deduced.test | RUN | 0.489635 | 1.180961 | 1.344336 | 0.002851 | 0.002856 | 0.002710 |
| test_same_type.test | RUN | 0.535884 | 1.639433 | 1.796752 | 0.003300 | 0.003075 | 0.003519 |
| test_member.test | RUN | 0.690425 | 3.220628 | 2.789328 | 0.005263 | 0.005252 | 0.005096 |
| test_null.test | RUN | 0.485961 | 1.541155 | 1.378427 | 0.003293 | 0.003221 | 0.003188 |
| test_free.test | RUN | 0.599970 | 2.710632 | 2.214851 | 0.004273 | 0.003998 | 0.003923 |
| test_is_empty.test | RUN | 0.481580 | 1.316220 | 1.389480 | 0.003303 | 0.003149 | 0.002873 |
| test_dynamic_any_cast.test | RUN | 0.930201 | 3.981442 | 4.211047 | 0.004635 | 0.005018 | 0.005273 |
| test_limits.test | RUN | 0.840748 | 2.787889 | 2.938216 | 0.003321 | 0.003661 | 0.003010 |
| test_param.test | COMPILE | 0.321848 | 0.734132 | 0.858281 | — | — | — |
| test_is_subconcept.test | COMPILE | 0.310984 | 0.561952 | 0.740739 | — | — | — |
| test_sfinae.test | COMPILE | 0.314621 | 0.577392 | 0.871083 | — | — | — |
| test_interface_order.test | COMPILE | 0.312319 | 0.593860 | 0.807812 | — | — | — |
| test_is_placeholder.test | COMPILE | 0.036833 | 0.061192 | 0.062293 | — | — | — |
| fail_default_construct.test | COMPILE_FAIL | 0.298356 | 0.576661 | 0.695493 | — | — | — |
| fail_construct_mismatch.test | COMPILE_FAIL | 0.308781 | 0.575270 | 0.686561 | — | — | — |
| fail_construct_mismatch_ref.test | COMPILE_FAIL | 0.293318 | 0.583264 | 0.785277 | — | — | — |
| fail_construct_mismatch_cref.test | COMPILE_FAIL | 0.292106 | 0.589094 | 0.786243 | — | — | — |
| fail_construct_mismatch_rref.test | COMPILE_FAIL | 0.296844 | 0.572647 | 0.673735 | — | — | — |
| fail_binding_convert_no_mapping.test | COMPILE_FAIL | 0.286186 | 0.574021 | 0.762893 | — | — | — |
| fail_increment_discard_const.test | COMPILE_FAIL | 0.294925 | 0.598319 | 0.791487 | — | — | — |
| fail_not_incrementable.test | COMPILE_FAIL | 0.317179 | 0.586332 | 0.721885 | — | — | — |
| fail_ref_assign.test | COMPILE_FAIL | 0.300220 | 0.501952 | 0.765241 | — | — | — |
| fail_cref_assign.test | COMPILE_FAIL | 0.292228 | 0.572686 | 0.788231 | — | — | — |
| fail_rref_assign.test | COMPILE_FAIL | 0.290384 | 0.589363 | 0.691506 | — | — | — |
| fail_ref_discard_const.test | COMPILE_FAIL | 0.304153 | 0.546372 | 0.793488 | — | — | — |
| fail_ref_discard_const_convert.test | COMPILE_FAIL | 0.298125 | 0.618777 | 0.830153 | — | — | — |
| fail_ref_discard_const_convert_ref.test | COMPILE_FAIL | 0.295635 | 0.628430 | 0.717855 | — | — | — |
| fail_ref_discard_const_convert_cref.test | COMPILE_FAIL | 0.294052 | 0.571684 | 0.825842 | — | — | — |
| fail_ref_discard_const_init.test | COMPILE_FAIL | 0.298789 | 0.584992 | 0.780619 | — | — | — |
| fail_any_cast_discard_const1.test | COMPILE_FAIL | 0.295218 | 0.606614 | 0.718700 | — | — | — |
| fail_any_cast_discard_const2.test | COMPILE_FAIL | 0.293558 | 0.590193 | 0.820378 | — | — | — |
| fail_any_cast_discard_const3.test | COMPILE_FAIL | 0.298121 | 0.595966 | 0.794682 | — | — | — |
| fail_any_cast_discard_const4.test | COMPILE_FAIL | 0.302130 | 0.609095 | 0.823413 | — | — | — |
| fail_any_cast_discard_const5.test | COMPILE_FAIL | 0.297699 | 0.569833 | 0.831388 | — | — | — |
| fail_any_cast_discard_const6.test | COMPILE_FAIL | 0.295114 | 0.598273 | 0.695684 | — | — | — |
| fail_any_cast_pointer_to_ref.test | COMPILE_FAIL | 0.290529 | 0.571612 | 0.795806 | — | — | — |
| fail_any_cast_pointer_to_val.test | COMPILE_FAIL | 0.273985 | 0.505317 | 0.506349 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.927×† | 0.883×, 0.972× | 0.940× |
| clang | 1.053×† | 1.015×, 1.091× | 1.055× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224918).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| test_add.test | 8 | 0.001976s | 0.001900s | 0.001931s | byte equality observed |
| test_add_assign.test | 10 | 0.001897s | 0.001903s | 0.001907s | byte equality observed |
| test_any_cast.test | 8 | 0.002236s | 0.002115s | 0.002112s | byte equality observed |
| test_assign.test | 4 | 0.006020s | 0.005069s | 0.005468s | byte equality observed |
| test_binding.test | 10 | 0.001807s | 0.001744s | 0.001759s | byte equality observed |
| test_binding_of.test | 10 | 0.001687s | 0.001627s | 0.001664s | byte equality observed |
| test_callable.test | 7 | 0.002860s | 0.002827s | 0.002856s | byte equality observed |
| test_construct.test | 5 | 0.003726s | 0.003587s | 0.003826s | byte equality observed |
| test_construct_cref.test | 7 | 0.002551s | 0.002534s | 0.002717s | byte equality observed |
| test_construct_ref.test | 8 | 0.002333s | 0.002228s | 0.002409s | byte equality observed |
| test_deduced.test | 10 | 0.001714s | 0.001633s | 0.001871s | byte equality observed |
| test_dereference.test | 10 | 0.001786s | 0.001768s | 0.001992s | byte equality observed |
| test_dynamic_any_cast.test | 7 | 0.002647s | 0.002481s | 0.002951s | byte equality observed |
| test_equal.test | 8 | 0.002151s | 0.002134s | 0.002451s | byte equality observed |
| test_forward_iterator.test | 10 | 0.001849s | 0.001771s | 0.002050s | byte equality observed |
| test_free.test | 8 | 0.002125s | 0.002131s | 0.002429s | byte equality observed |
| test_increment.test | 9 | 0.001767s | 0.001712s | 0.001982s | byte equality observed |
| test_is_empty.test | 10 | 0.001771s | 0.001677s | 0.001981s | byte equality observed |
| test_less.test | 8 | 0.002326s | 0.002269s | 0.002656s | byte equality observed |
| test_limits.test | 9 | 0.001890s | 0.001877s | 0.002174s | byte equality observed |
| test_member.test | 7 | 0.002664s | 0.002664s | 0.003042s | byte equality observed |
| test_negate.test | 10 | 0.001680s | 0.001681s | 0.001878s | byte equality observed |
| test_nested.test | 10 | 0.001783s | 0.001739s | 0.001967s | byte equality observed |
| test_null.test | 10 | 0.001797s | 0.001770s | 0.002086s | byte equality observed |
| test_reference.test | 10 | 0.001914s | 0.001819s | 0.002059s | byte equality observed |
| test_relaxed.test | 10 | 0.001747s | 0.001692s | 0.001937s | byte equality observed |
| test_same_type.test | 10 | 0.001877s | 0.001778s | 0.002055s | byte equality observed |
| test_stream.test | 8 | 0.002509s | 0.002400s | 0.002785s | byte equality observed |
| test_subscript.test | 9 | 0.001901s | 0.001793s | 0.002117s | byte equality observed |
| test_tuple.test | 10 | 0.001850s | 0.001814s | 0.002115s | byte equality observed |
| test_typeid_of.test | 8 | 0.001907s | 0.001828s | 0.002154s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224918).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
