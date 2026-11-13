# libs/type_erasure/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_binding.test | RUN | 0.703973 | 2.849928 | 2.765246 | 0.004042 | 0.003741 | 0.003748 |
| test_increment.test | RUN | 0.654592 | 2.103760 | 2.423262 | 0.003965 | 0.003711 | 0.003896 |
| test_add.test | RUN | 0.851205 | 3.302229 | 3.584458 | 0.004143 | 0.004146 | 0.004126 |
| test_add_assign.test | RUN | 0.926961 | 3.514116 | 3.683923 | 0.004223 | 0.004189 | 0.004555 |
| test_callable.test | RUN | 1.440190 | 6.087524 | 7.765275 | 0.007848 | 0.007655 | 0.007599 |
| test_reference.test | RUN | 0.697728 | 2.485532 | 2.589453 | 0.004218 | 0.004254 | 0.003580 |
| test_construct.test | RUN | 3.128867 | 28.330623 | 22.208856 | 0.009172 | 0.008809 | 0.009973 |
| test_relaxed.test | RUN | 0.664336 | 1.740481 | 2.301881 | 0.003507 | 0.003594 | 0.003828 |
| test_assign.test | RUN | 7.032477 | 86.066427 | 70.549584 | 0.014082 | 0.008093 | 0.008573 |
| test_construct_ref.test | RUN | 0.838047 | 4.476069 | 3.571663 | 0.005314 | 0.004767 | 0.005786 |
| test_construct_cref.test | RUN | 0.905093 | 4.022121 | 4.530311 | 0.006368 | 0.005985 | 0.006340 |
| test_any_cast.test | RUN | 0.815840 | 4.107679 | 3.451070 | 0.005297 | 0.005005 | 0.005575 |
| test_binding_of.test | RUN | 0.616483 | 1.628958 | 2.089220 | 0.003798 | 0.003505 | 0.003685 |
| test_typeid_of.test | RUN | 0.691715 | 2.309509 | 2.398658 | 0.004176 | 0.004202 | 0.004274 |
| test_nested.test | RUN | 0.750926 | 2.287370 | 2.742507 | 0.003798 | 0.003662 | 0.004165 |
| test_less.test | RUN | 0.942692 | 4.537957 | 4.468554 | 0.005435 | 0.005118 | 0.005087 |
| test_equal.test | RUN | 0.921541 | 4.270914 | 3.667454 | 0.005191 | 0.004417 | 0.004884 |
| test_negate.test | RUN | 0.639663 | 1.730818 | 2.256688 | 0.003673 | 0.004044 | 0.003131 |
| test_dereference.test | RUN | 0.697179 | 2.050071 | 2.528953 | 0.004155 | 0.003690 | 0.003767 |
| test_subscript.test | RUN | 0.730983 | 2.415377 | 2.385715 | 0.004169 | 0.003798 | 0.003774 |
| test_forward_iterator.test | RUN | 0.847554 | 2.694451 | 3.303314 | 0.004080 | 0.003562 | 0.003346 |
| test_tuple.test | RUN | 0.740967 | 2.705523 | 2.913351 | 0.004409 | 0.003630 | 0.003774 |
| test_stream.test | RUN | 0.990249 | 4.313000 | 4.300439 | 0.005471 | 0.005951 | 0.006512 |
| test_deduced.test | RUN | 0.615120 | 1.621394 | 2.267081 | 0.003274 | 0.003401 | 0.003480 |
| test_same_type.test | RUN | 0.711186 | 2.224496 | 2.409711 | 0.003613 | 0.003714 | 0.003810 |
| test_member.test | RUN | 0.943405 | 4.317312 | 4.335764 | 0.006671 | 0.006223 | 0.006804 |
| test_null.test | RUN | 0.638872 | 2.197205 | 2.393869 | 0.004005 | 0.004028 | 0.004223 |
| test_free.test | RUN | 0.804943 | 3.793173 | 2.817322 | 0.004859 | 0.004288 | 0.004898 |
| test_is_empty.test | RUN | 0.631624 | 1.869194 | 2.135708 | 0.003876 | 0.003335 | 0.004143 |
| test_dynamic_any_cast.test | RUN | 1.226040 | 5.344409 | 5.482330 | 0.005920 | 0.007173 | 0.005218 |
| test_limits.test | RUN | 1.135472 | 3.505482 | 4.341928 | 0.004020 | 0.004633 | 0.004202 |
| test_param.test | COMPILE | 0.448670 | 1.079085 | 1.474415 | — | — | — |
| test_is_subconcept.test | COMPILE | 0.441017 | 0.834130 | 1.327264 | — | — | — |
| test_sfinae.test | COMPILE | 0.412691 | 0.872820 | 1.292840 | — | — | — |
| test_interface_order.test | COMPILE | 0.409702 | 0.842381 | 1.305655 | — | — | — |
| test_is_placeholder.test | COMPILE | 0.052362 | 0.077115 | 0.085568 | — | — | — |
| fail_default_construct.test | COMPILE_FAIL | 0.387635 | 0.713480 | 1.198827 | — | — | — |
| fail_construct_mismatch.test | COMPILE_FAIL | 0.394531 | 0.818386 | 1.054214 | — | — | — |
| fail_construct_mismatch_ref.test | COMPILE_FAIL | 0.393551 | 0.730873 | 1.203034 | — | — | — |
| fail_construct_mismatch_cref.test | COMPILE_FAIL | 0.398484 | 0.783235 | 1.189530 | — | — | — |
| fail_construct_mismatch_rref.test | COMPILE_FAIL | 0.384214 | 0.801864 | 1.110988 | — | — | — |
| fail_binding_convert_no_mapping.test | COMPILE_FAIL | 0.375219 | 0.728623 | 1.171388 | — | — | — |
| fail_increment_discard_const.test | COMPILE_FAIL | 0.393552 | 0.802978 | 1.083060 | — | — | — |
| fail_not_incrementable.test | COMPILE_FAIL | 0.416094 | 0.817560 | 1.284910 | — | — | — |
| fail_ref_assign.test | COMPILE_FAIL | 0.390727 | 0.790190 | 1.146647 | — | — | — |
| fail_cref_assign.test | COMPILE_FAIL | 0.389437 | 0.790951 | 1.248709 | — | — | — |
| fail_rref_assign.test | COMPILE_FAIL | 0.392546 | 0.784755 | 1.086872 | — | — | — |
| fail_ref_discard_const.test | COMPILE_FAIL | 0.398362 | 0.839928 | 1.209427 | — | — | — |
| fail_ref_discard_const_convert.test | COMPILE_FAIL | 0.390703 | 0.813477 | 1.238311 | — | — | — |
| fail_ref_discard_const_convert_ref.test | COMPILE_FAIL | 0.391976 | 0.849192 | 1.136464 | — | — | — |
| fail_ref_discard_const_convert_cref.test | COMPILE_FAIL | 0.394305 | 0.860622 | 1.274826 | — | — | — |
| fail_ref_discard_const_init.test | COMPILE_FAIL | 0.390645 | 0.781765 | 1.181792 | — | — | — |
| fail_any_cast_discard_const1.test | COMPILE_FAIL | 0.394615 | 0.797908 | 1.073625 | — | — | — |
| fail_any_cast_discard_const2.test | COMPILE_FAIL | 0.391691 | 0.857367 | 1.072941 | — | — | — |
| fail_any_cast_discard_const3.test | COMPILE_FAIL | 0.396563 | 0.799644 | 1.263158 | — | — | — |
| fail_any_cast_discard_const4.test | COMPILE_FAIL | 0.405710 | 0.798159 | 1.241270 | — | — | — |
| fail_any_cast_discard_const5.test | COMPILE_FAIL | 0.399550 | 0.818849 | 1.086490 | — | — | — |
| fail_any_cast_discard_const6.test | COMPILE_FAIL | 0.392442 | 0.845602 | 1.263732 | — | — | — |
| fail_any_cast_pointer_to_ref.test | COMPILE_FAIL | 0.370870 | 0.780110 | 1.262574 | — | — | — |
| fail_any_cast_pointer_to_val.test | COMPILE_FAIL | 0.328486 | 0.756556 | 0.895742 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.955× | 0.947×, 0.962× | 0.999× |
| clang | 0.952× | 0.954×, 0.949× | 0.998× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824563).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| test_add.test | 8 | 0.002917s | 0.002830s | 0.002740s | byte equality observed |
| test_add_assign.test | 8 | 0.002985s | 0.002914s | 0.002863s | byte equality observed |
| test_any_cast.test | 7 | 0.003425s | 0.003306s | 0.003314s | byte equality observed |
| test_assign.test | 3 | 0.010501s | 0.008759s | 0.009264s | byte equality observed |
| test_binding.test | 9 | 0.002752s | 0.002690s | 0.002603s | byte equality observed |
| test_binding_of.test | 9 | 0.002586s | 0.002465s | 0.002419s | byte equality observed |
| test_callable.test | 6 | 0.004377s | 0.004295s | 0.004222s | byte equality observed |
| test_construct.test | 5 | 0.005671s | 0.005726s | 0.005407s | byte equality observed |
| test_construct_cref.test | 6 | 0.003955s | 0.003821s | 0.003845s | byte equality observed |
| test_construct_ref.test | 7 | 0.003492s | 0.003428s | 0.003404s | byte equality observed |
| test_deduced.test | 7 | 0.002552s | 0.002502s | 0.002455s | byte equality observed |
| test_dereference.test | 8 | 0.002778s | 0.002696s | 0.002660s | byte equality observed |
| test_dynamic_any_cast.test | 6 | 0.003873s | 0.003862s | 0.003717s | byte equality observed |
| test_equal.test | 7 | 0.003234s | 0.003233s | 0.003164s | byte equality observed |
| test_forward_iterator.test | 8 | 0.002802s | 0.002652s | 0.002560s | byte equality observed |
| test_free.test | 7 | 0.003364s | 0.003172s | 0.003107s | byte equality observed |
| test_increment.test | 7 | 0.002687s | 0.002563s | 0.002559s | byte equality observed |
| test_is_empty.test | 9 | 0.002685s | 0.002582s | 0.002560s | byte equality observed |
| test_less.test | 7 | 0.003555s | 0.003461s | 0.003389s | byte equality observed |
| test_limits.test | 8 | 0.002952s | 0.002826s | 0.002783s | byte equality observed |
| test_member.test | 6 | 0.004042s | 0.003988s | 0.003997s | byte equality observed |
| test_negate.test | 9 | 0.002608s | 0.002485s | 0.002390s | byte equality observed |
| test_nested.test | 9 | 0.002655s | 0.002575s | 0.002553s | byte equality observed |
| test_null.test | 8 | 0.002734s | 0.002637s | 0.002630s | byte equality observed |
| test_reference.test | 9 | 0.002814s | 0.002777s | 0.002767s | byte equality observed |
| test_relaxed.test | 8 | 0.002671s | 0.002534s | 0.002496s | byte equality observed |
| test_same_type.test | 8 | 0.002767s | 0.002686s | 0.002712s | byte equality observed |
| test_stream.test | 7 | 0.003794s | 0.003682s | 0.003676s | byte equality observed |
| test_subscript.test | 8 | 0.002825s | 0.002761s | 0.002743s | byte equality observed |
| test_tuple.test | 8 | 0.002891s | 0.002822s | 0.002784s | byte equality observed |
| test_typeid_of.test | 7 | 0.002911s | 0.002766s | 0.002762s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824563).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
