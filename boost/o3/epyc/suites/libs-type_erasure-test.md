# libs/type_erasure/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_binding.test | RUN | 0.508359 | 2.036124 | 1.905161 | 0.003368 | 0.003295 | 0.003495 |
| test_increment.test | RUN | 0.508099 | 1.528659 | 1.654346 | 0.003424 | 0.003396 | 0.003400 |
| test_add.test | RUN | 0.652270 | 2.457048 | 2.461598 | 0.003634 | 0.003675 | 0.003791 |
| test_add_assign.test | RUN | 0.717751 | 2.636290 | 2.546426 | 0.003664 | 0.003518 | 0.003661 |
| test_callable.test | RUN | 1.098609 | 4.570637 | 5.494386 | 0.005721 | 0.006314 | 0.005958 |
| test_reference.test | RUN | 0.529282 | 1.831504 | 1.725493 | 0.003501 | 0.003613 | 0.003560 |
| test_construct.test | RUN | 2.458673 | 22.703143 | 16.004397 | 0.007465 | 0.007278 | 0.006355 |
| test_relaxed.test | RUN | 0.493781 | 1.326394 | 1.561989 | 0.003282 | 0.003233 | 0.003386 |
| test_assign.test | RUN | 5.394303 | 74.428462 | 58.149580 | 0.011189 | 0.007204 | 0.007340 |
| test_construct_ref.test | RUN | 0.630305 | 3.093475 | 2.552684 | 0.004716 | 0.004589 | 0.004706 |
| test_construct_cref.test | RUN | 0.678628 | 3.014329 | 3.070875 | 0.005108 | 0.005495 | 0.005337 |
| test_any_cast.test | RUN | 0.588393 | 3.096845 | 2.489947 | 0.004119 | 0.004460 | 0.004021 |
| test_binding_of.test | RUN | 0.475958 | 1.219011 | 1.494524 | 0.003097 | 0.003097 | 0.003092 |
| test_typeid_of.test | RUN | 0.508365 | 1.732936 | 1.650903 | 0.003450 | 0.003653 | 0.003490 |
| test_nested.test | RUN | 0.570732 | 1.696293 | 1.879597 | 0.003382 | 0.003464 | 0.003206 |
| test_less.test | RUN | 0.727739 | 3.344756 | 3.047976 | 0.004340 | 0.004575 | 0.004052 |
| test_equal.test | RUN | 0.678601 | 3.152464 | 2.582413 | 0.004053 | 0.004323 | 0.004312 |
| test_negate.test | RUN | 0.503678 | 1.308081 | 1.466748 | 0.002946 | 0.003388 | 0.003156 |
| test_dereference.test | RUN | 0.552561 | 1.718507 | 1.752371 | 0.003274 | 0.003366 | 0.003117 |
| test_subscript.test | RUN | 0.587959 | 1.959191 | 1.906367 | 0.003516 | 0.003728 | 0.003490 |
| test_forward_iterator.test | RUN | 0.664209 | 2.199909 | 2.327589 | 0.003392 | 0.003629 | 0.003174 |
| test_tuple.test | RUN | 0.589254 | 2.139510 | 1.716633 | 0.003586 | 0.003417 | 0.003276 |
| test_stream.test | RUN | 0.771115 | 3.287968 | 3.142581 | 0.004916 | 0.004493 | 0.004915 |
| test_deduced.test | RUN | 0.506623 | 1.183025 | 1.509311 | 0.003381 | 0.002993 | 0.002771 |
| test_same_type.test | RUN | 0.559285 | 1.634239 | 1.640377 | 0.003321 | 0.003197 | 0.003009 |
| test_member.test | RUN | 0.736805 | 3.342290 | 2.572039 | 0.005336 | 0.005421 | 0.004888 |
| test_null.test | RUN | 0.516702 | 1.666064 | 1.582501 | 0.003450 | 0.003330 | 0.003478 |
| test_free.test | RUN | 0.644167 | 2.745735 | 2.305422 | 0.004366 | 0.003880 | 0.004397 |
| test_is_empty.test | RUN | 0.497046 | 1.383014 | 1.495014 | 0.003376 | 0.003271 | 0.003343 |
| test_dynamic_any_cast.test | RUN | 0.933638 | 4.028847 | 4.107628 | 0.005094 | 0.004513 | 0.005460 |
| test_limits.test | RUN | 0.862921 | 2.840173 | 2.775516 | 0.003121 | 0.003225 | 0.003674 |
| test_param.test | COMPILE | 0.332029 | 0.748064 | 0.996685 | — | — | — |
| test_is_subconcept.test | COMPILE | 0.327373 | 0.570387 | 0.902210 | — | — | — |
| test_sfinae.test | COMPILE | 0.337312 | 0.584444 | 0.793702 | — | — | — |
| test_interface_order.test | COMPILE | 0.325052 | 0.578664 | 0.850780 | — | — | — |
| test_is_placeholder.test | COMPILE | 0.039950 | 0.059402 | 0.066844 | — | — | — |
| fail_default_construct.test | COMPILE_FAIL | 0.314688 | 0.581724 | 0.774026 | — | — | — |
| fail_construct_mismatch.test | COMPILE_FAIL | 0.303353 | 0.590637 | 0.794166 | — | — | — |
| fail_construct_mismatch_ref.test | COMPILE_FAIL | 0.309360 | 0.583754 | 0.763025 | — | — | — |
| fail_construct_mismatch_cref.test | COMPILE_FAIL | 0.304689 | 0.597667 | 0.690319 | — | — | — |
| fail_construct_mismatch_rref.test | COMPILE_FAIL | 0.305090 | 0.574442 | 0.797983 | — | — | — |
| fail_binding_convert_no_mapping.test | COMPILE_FAIL | 0.297201 | 0.551561 | 0.778267 | — | — | — |
| fail_increment_discard_const.test | COMPILE_FAIL | 0.307598 | 0.584830 | 0.674311 | — | — | — |
| fail_not_incrementable.test | COMPILE_FAIL | 0.318763 | 0.620587 | 0.841427 | — | — | — |
| fail_ref_assign.test | COMPILE_FAIL | 0.299250 | 0.527961 | 0.782575 | — | — | — |
| fail_cref_assign.test | COMPILE_FAIL | 0.306557 | 0.581903 | 0.675533 | — | — | — |
| fail_rref_assign.test | COMPILE_FAIL | 0.307264 | 0.581201 | 0.799122 | — | — | — |
| fail_ref_discard_const.test | COMPILE_FAIL | 0.304906 | 0.543319 | 0.817471 | — | — | — |
| fail_ref_discard_const_convert.test | COMPILE_FAIL | 0.306287 | 0.630376 | 0.695996 | — | — | — |
| fail_ref_discard_const_convert_ref.test | COMPILE_FAIL | 0.306916 | 0.628810 | 0.833299 | — | — | — |
| fail_ref_discard_const_convert_cref.test | COMPILE_FAIL | 0.309261 | 0.575993 | 0.826358 | — | — | — |
| fail_ref_discard_const_init.test | COMPILE_FAIL | 0.306829 | 0.592026 | 0.662768 | — | — | — |
| fail_any_cast_discard_const1.test | COMPILE_FAIL | 0.337828 | 0.592095 | 0.836349 | — | — | — |
| fail_any_cast_discard_const2.test | COMPILE_FAIL | 0.337474 | 0.573173 | 0.734017 | — | — | — |
| fail_any_cast_discard_const3.test | COMPILE_FAIL | 0.375947 | 0.599920 | 0.834797 | — | — | — |
| fail_any_cast_discard_const4.test | COMPILE_FAIL | 0.383290 | 0.604256 | 0.703418 | — | — | — |
| fail_any_cast_discard_const5.test | COMPILE_FAIL | 0.334613 | 0.586981 | 0.842939 | — | — | — |
| fail_any_cast_discard_const6.test | COMPILE_FAIL | 0.325123 | 0.601441 | 0.840510 | — | — | — |
| fail_any_cast_pointer_to_ref.test | COMPILE_FAIL | 0.306605 | 0.599709 | 0.736642 | — | — | — |
| fail_any_cast_pointer_to_val.test | COMPILE_FAIL | 0.286765 | 0.529680 | 0.503665 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.966×† | 0.906×, 1.026× | 0.909× |
| clang | 0.950×† | 0.990×, 0.909× | 1.023× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737952).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| test_add.test | 9 | 0.002261s | 0.002279s | 0.002244s | byte equality observed |
| test_add_assign.test | 9 | 0.002326s | 0.002247s | 0.002227s | byte equality observed |
| test_any_cast.test | 8 | 0.002699s | 0.002599s | 0.002667s | byte equality observed |
| test_assign.test | 4 | 0.007606s | 0.005925s | 0.006747s | byte equality observed |
| test_binding.test | 10 | 0.002198s | 0.002155s | 0.002129s | byte equality observed |
| test_binding_of.test | 10 | 0.002045s | 0.001944s | 0.001956s | byte equality observed |
| test_callable.test | 7 | 0.003478s | 0.003331s | 0.003280s | byte equality observed |
| test_construct.test | 5 | 0.004533s | 0.004379s | 0.004393s | byte equality observed |
| test_construct_cref.test | 6 | 0.003128s | 0.003148s | 0.003032s | byte equality observed |
| test_construct_ref.test | 7 | 0.002819s | 0.002743s | 0.002738s | byte equality observed |
| test_deduced.test | 8 | 0.002059s | 0.001988s | 0.001978s | byte equality observed |
| test_dereference.test | 7 | 0.002177s | 0.002111s | 0.002143s | byte equality observed |
| test_dynamic_any_cast.test | 6 | 0.003042s | 0.003117s | 0.003061s | byte equality observed |
| test_equal.test | 8 | 0.002591s | 0.002510s | 0.002536s | byte equality observed |
| test_forward_iterator.test | 8 | 0.002167s | 0.002129s | 0.002103s | byte equality observed |
| test_free.test | 8 | 0.002548s | 0.002530s | 0.002516s | byte equality observed |
| test_increment.test | 10 | 0.002077s | 0.002007s | 0.002020s | byte equality observed |
| test_is_empty.test | 10 | 0.002114s | 0.001961s | 0.002007s | byte equality observed |
| test_less.test | 8 | 0.002771s | 0.002677s | 0.002750s | byte equality observed |
| test_limits.test | 9 | 0.002289s | 0.002181s | 0.002157s | byte equality observed |
| test_member.test | 7 | 0.003178s | 0.003118s | 0.003048s | byte equality observed |
| test_negate.test | 10 | 0.002069s | 0.001964s | 0.001943s | byte equality observed |
| test_nested.test | 10 | 0.002133s | 0.002039s | 0.002067s | byte equality observed |
| test_null.test | 7 | 0.002215s | 0.002128s | 0.002144s | byte equality observed |
| test_reference.test | 10 | 0.002240s | 0.002149s | 0.002151s | byte equality observed |
| test_relaxed.test | 10 | 0.002109s | 0.002007s | 0.001987s | byte equality observed |
| test_same_type.test | 10 | 0.002152s | 0.002093s | 0.002133s | byte equality observed |
| test_stream.test | 8 | 0.002953s | 0.002839s | 0.002895s | byte equality observed |
| test_subscript.test | 10 | 0.002256s | 0.002163s | 0.002158s | byte equality observed |
| test_tuple.test | 10 | 0.002233s | 0.002140s | 0.002168s | byte equality observed |
| test_typeid_of.test | 10 | 0.002270s | 0.002284s | 0.002172s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737952).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
