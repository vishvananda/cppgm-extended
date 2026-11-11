# libs/type_erasure/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_binding.test | RUN | 0.546629 | 1.800675 | 1.452285 | 0.003502 | 0.003365 | 0.003124 |
| test_increment.test | RUN | 0.516082 | 1.331537 | 1.237488 | 0.003219 | 0.003180 | 0.002930 |
| test_add.test | RUN | 0.642345 | 2.153149 | 1.896247 | 0.003225 | 0.003358 | 0.003567 |
| test_add_assign.test | RUN | 0.697851 | 2.292055 | 2.006015 | 0.003354 | 0.003335 | 0.003570 |
| test_callable.test | RUN | 1.082589 | 4.143490 | 4.616392 | 0.005555 | 0.005779 | 0.005492 |
| test_reference.test | RUN | 0.526089 | 1.645547 | 1.402600 | 0.003422 | 0.003618 | 0.003606 |
| test_construct.test | RUN | 1.991553 | 24.438289 | 14.853100 | 0.009683 | 0.006387 | 0.007738 |
| test_relaxed.test | RUN | 0.481680 | 1.150085 | 1.211417 | 0.003594 | 0.003790 | 0.003694 |
| test_assign.test | RUN | 4.973696 | 82.298773 | 54.143339 | 0.010792 | 0.006622 | 0.007526 |
| test_construct_ref.test | RUN | 0.606373 | 2.767692 | 2.053921 | 0.004355 | 0.004458 | 0.004345 |
| test_construct_cref.test | RUN | 0.698344 | 2.622058 | 2.617706 | 0.004886 | 0.004905 | 0.004999 |
| test_any_cast.test | RUN | 0.597414 | 2.680241 | 1.983797 | 0.004300 | 0.004191 | 0.004233 |
| test_binding_of.test | RUN | 0.525828 | 1.088931 | 1.146072 | 0.003370 | 0.003138 | 0.003204 |
| test_typeid_of.test | RUN | 0.563321 | 1.511011 | 1.371436 | 0.003196 | 0.003243 | 0.003829 |
| test_nested.test | RUN | 0.608874 | 1.590489 | 1.578073 | 0.003350 | 0.003308 | 0.003335 |
| test_less.test | RUN | 0.765196 | 2.886873 | 2.618854 | 0.004526 | 0.004195 | 0.004844 |
| test_equal.test | RUN | 0.662638 | 2.742569 | 2.186164 | 0.004058 | 0.004356 | 0.003857 |
| test_negate.test | RUN | 0.503815 | 1.063397 | 1.159953 | 0.003229 | 0.003420 | 0.003175 |
| test_dereference.test | RUN | 0.553232 | 1.423997 | 1.303103 | 0.003605 | 0.002994 | 0.003402 |
| test_subscript.test | RUN | 0.565668 | 1.551747 | 1.591463 | 0.003561 | 0.003422 | 0.003822 |
| test_forward_iterator.test | RUN | 0.659971 | 1.810325 | 1.714156 | 0.003302 | 0.003235 | 0.003193 |
| test_tuple.test | RUN | 0.572004 | 1.730568 | 1.636771 | 0.003431 | 0.003425 | 0.003682 |
| test_stream.test | RUN | 0.738074 | 2.982097 | 2.692412 | 0.004158 | 0.004241 | 0.004555 |
| test_deduced.test | RUN | 0.492554 | 1.095989 | 1.174984 | 0.003118 | 0.003231 | 0.003129 |
| test_same_type.test | RUN | 0.538026 | 1.428032 | 1.534420 | 0.003502 | 0.003325 | 0.003462 |
| test_member.test | RUN | 0.738928 | 2.898637 | 2.313134 | 0.004934 | 0.005394 | 0.004830 |
| test_null.test | RUN | 0.542584 | 1.334211 | 1.289385 | 0.003709 | 0.003133 | 0.003427 |
| test_free.test | RUN | 0.648417 | 2.380326 | 1.884001 | 0.004077 | 0.003439 | 0.004347 |
| test_is_empty.test | RUN | 0.511771 | 1.203224 | 1.077483 | 0.003358 | 0.002971 | 0.003241 |
| test_dynamic_any_cast.test | RUN | 0.892396 | 3.709362 | 3.248075 | 0.004322 | 0.004819 | 0.005183 |
| test_limits.test | RUN | 0.798982 | 2.516805 | 2.382513 | 0.003514 | 0.003413 | 0.003622 |
| test_param.test | COMPILE | 0.336434 | 0.632800 | 0.755275 | — | — | — |
| test_is_subconcept.test | COMPILE | 0.333637 | 0.540900 | 0.664445 | — | — | — |
| test_sfinae.test | COMPILE | 0.339734 | 0.550317 | 0.687574 | — | — | — |
| test_interface_order.test | COMPILE | 0.321386 | 0.504483 | 0.694241 | — | — | — |
| test_is_placeholder.test | COMPILE | 0.042395 | 0.053364 | 0.061971 | — | — | — |
| fail_default_construct.test | COMPILE_FAIL | 0.308735 | 0.500026 | 0.644171 | — | — | — |
| fail_construct_mismatch.test | COMPILE_FAIL | 0.305019 | 0.539155 | 0.610166 | — | — | — |
| fail_construct_mismatch_ref.test | COMPILE_FAIL | 0.301875 | 0.488303 | 0.648250 | — | — | — |
| fail_construct_mismatch_cref.test | COMPILE_FAIL | 0.308623 | 0.475756 | 0.565014 | — | — | — |
| fail_construct_mismatch_rref.test | COMPILE_FAIL | 0.309794 | 0.482573 | 0.633964 | — | — | — |
| fail_binding_convert_no_mapping.test | COMPILE_FAIL | 0.298781 | 0.492027 | 0.626440 | — | — | — |
| fail_increment_discard_const.test | COMPILE_FAIL | 0.307265 | 0.479814 | 0.638408 | — | — | — |
| fail_not_incrementable.test | COMPILE_FAIL | 0.330638 | 0.509513 | 0.617042 | — | — | — |
| fail_ref_assign.test | COMPILE_FAIL | 0.304260 | 0.470072 | 0.528718 | — | — | — |
| fail_cref_assign.test | COMPILE_FAIL | 0.303696 | 0.498239 | 0.601547 | — | — | — |
| fail_rref_assign.test | COMPILE_FAIL | 0.302275 | 0.479871 | 0.592603 | — | — | — |
| fail_ref_discard_const.test | COMPILE_FAIL | 0.306590 | 0.515119 | 0.538526 | — | — | — |
| fail_ref_discard_const_convert.test | COMPILE_FAIL | 0.307927 | 0.509428 | 0.600556 | — | — | — |
| fail_ref_discard_const_convert_ref.test | COMPILE_FAIL | 0.306315 | 0.570254 | 0.602317 | — | — | — |
| fail_ref_discard_const_convert_cref.test | COMPILE_FAIL | 0.305258 | 0.534747 | 0.583052 | — | — | — |
| fail_ref_discard_const_init.test | COMPILE_FAIL | 0.309068 | 0.536013 | 0.579107 | — | — | — |
| fail_any_cast_discard_const1.test | COMPILE_FAIL | 0.309494 | 0.500981 | 0.613252 | — | — | — |
| fail_any_cast_discard_const2.test | COMPILE_FAIL | 0.307033 | 0.504949 | 0.559347 | — | — | — |
| fail_any_cast_discard_const3.test | COMPILE_FAIL | 0.311259 | 0.484133 | 0.632528 | — | — | — |
| fail_any_cast_discard_const4.test | COMPILE_FAIL | 0.319896 | 0.500795 | 0.630792 | — | — | — |
| fail_any_cast_discard_const5.test | COMPILE_FAIL | 0.318828 | 0.509404 | 0.551885 | — | — | — |
| fail_any_cast_discard_const6.test | COMPILE_FAIL | 0.318931 | 0.488943 | 0.664034 | — | — | — |
| fail_any_cast_pointer_to_ref.test | COMPILE_FAIL | 0.311950 | 0.375308 | 0.652582 | — | — | — |
| fail_any_cast_pointer_to_val.test | COMPILE_FAIL | 0.263498 | 0.444926 | 0.539840 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.912×† | 0.882×, 0.943× | 1.029× |
| clang | 0.823×† | 0.766×, 0.881× | 0.984× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| test_add.test | 9 | 0.001916s | 0.001757s | 0.001788s | byte equality observed |
| test_add_assign.test | 10 | 0.001968s | 0.001765s | 0.001796s | byte equality observed |
| test_any_cast.test | 9 | 0.002230s | 0.001951s | 0.002028s | byte equality observed |
| test_assign.test | 4 | 0.006199s | 0.004526s | 0.005025s | byte equality observed |
| test_binding.test | 10 | 0.001847s | 0.001631s | 0.001751s | byte equality observed |
| test_binding_of.test | 10 | 0.001725s | 0.001537s | 0.001550s | byte equality observed |
| test_callable.test | 6 | 0.002759s | 0.002571s | 0.002583s | byte equality observed |
| test_construct.test | 5 | 0.003630s | 0.003298s | 0.003427s | byte equality observed |
| test_construct_cref.test | 8 | 0.002489s | 0.002336s | 0.002427s | byte equality observed |
| test_construct_ref.test | 9 | 0.002224s | 0.002135s | 0.002170s | byte equality observed |
| test_deduced.test | 10 | 0.001663s | 0.001595s | 0.001581s | byte equality observed |
| test_dereference.test | 10 | 0.001747s | 0.001684s | 0.001731s | byte equality observed |
| test_dynamic_any_cast.test | 8 | 0.002464s | 0.002377s | 0.002403s | byte equality observed |
| test_equal.test | 9 | 0.002142s | 0.001980s | 0.002049s | byte equality observed |
| test_forward_iterator.test | 10 | 0.001908s | 0.001683s | 0.001744s | byte equality observed |
| test_free.test | 9 | 0.002105s | 0.001975s | 0.002036s | byte equality observed |
| test_increment.test | 10 | 0.001771s | 0.001614s | 0.001640s | byte equality observed |
| test_is_empty.test | 10 | 0.001790s | 0.001614s | 0.001692s | byte equality observed |
| test_less.test | 9 | 0.002311s | 0.002142s | 0.002216s | byte equality observed |
| test_limits.test | 10 | 0.001890s | 0.001758s | 0.001767s | byte equality observed |
| test_member.test | 7 | 0.002551s | 0.002395s | 0.002463s | byte equality observed |
| test_negate.test | 10 | 0.001641s | 0.001580s | 0.001721s | byte equality observed |
| test_nested.test | 10 | 0.001789s | 0.001630s | 0.001745s | byte equality observed |
| test_null.test | 10 | 0.001885s | 0.001667s | 0.001644s | byte equality observed |
| test_reference.test | 10 | 0.001857s | 0.001756s | 0.001696s | byte equality observed |
| test_relaxed.test | 10 | 0.001741s | 0.001604s | 0.001608s | byte equality observed |
| test_same_type.test | 10 | 0.001924s | 0.001690s | 0.001678s | byte equality observed |
| test_stream.test | 8 | 0.002376s | 0.002241s | 0.002304s | byte equality observed |
| test_subscript.test | 9 | 0.001823s | 0.001698s | 0.001771s | byte equality observed |
| test_tuple.test | 8 | 0.001806s | 0.001768s | 0.001753s | byte equality observed |
| test_typeid_of.test | 9 | 0.001847s | 0.001752s | 0.001815s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
