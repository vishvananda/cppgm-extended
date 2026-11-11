# libs/type_erasure/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_binding.test | RUN | 0.581367 | 2.020012 | 1.858302 | 0.003566 | 0.003205 | 0.003292 |
| test_increment.test | RUN | 0.555881 | 1.528112 | 1.589644 | 0.003384 | 0.003248 | 0.003223 |
| test_add.test | RUN | 0.697835 | 2.414648 | 2.366192 | 0.003537 | 0.003628 | 0.003663 |
| test_add_assign.test | RUN | 0.759376 | 2.548280 | 2.488844 | 0.003427 | 0.003783 | 0.003519 |
| test_callable.test | RUN | 1.147303 | 4.464117 | 5.266541 | 0.005853 | 0.005894 | 0.005864 |
| test_reference.test | RUN | 0.582366 | 1.790689 | 1.714545 | 0.003318 | 0.003693 | 0.003182 |
| test_construct.test | RUN | 2.303555 | 22.019436 | 15.611424 | 0.007486 | 0.007897 | 0.006925 |
| test_relaxed.test | RUN | 0.538887 | 1.312268 | 1.508228 | 0.003188 | 0.003307 | 0.003019 |
| test_assign.test | RUN | 6.105429 | 68.633416 | 52.292028 | 0.009878 | 0.007014 | 0.007280 |
| test_construct_ref.test | RUN | 0.705212 | 3.070724 | 2.371667 | 0.004597 | 0.004439 | 0.004552 |
| test_construct_cref.test | RUN | 0.773670 | 2.954163 | 3.002379 | 0.004958 | 0.005397 | 0.005291 |
| test_any_cast.test | RUN | 0.665357 | 3.005854 | 2.295754 | 0.004384 | 0.004251 | 0.004331 |
| test_binding_of.test | RUN | 0.522960 | 1.220903 | 1.379216 | 0.003158 | 0.002967 | 0.003034 |
| test_typeid_of.test | RUN | 0.578403 | 1.573947 | 1.592308 | 0.003472 | 0.003331 | 0.003305 |
| test_nested.test | RUN | 0.622112 | 1.675555 | 1.857180 | 0.003225 | 0.003336 | 0.003081 |
| test_less.test | RUN | 0.768416 | 3.253204 | 2.888458 | 0.004748 | 0.004628 | 0.004653 |
| test_equal.test | RUN | 0.749586 | 3.107417 | 2.551581 | 0.004240 | 0.003910 | 0.004157 |
| test_negate.test | RUN | 0.535986 | 1.277890 | 1.426004 | 0.002832 | 0.003064 | 0.002955 |
| test_dereference.test | RUN | 0.593138 | 1.533709 | 1.690630 | 0.003474 | 0.003349 | 0.003079 |
| test_subscript.test | RUN | 0.615130 | 1.754055 | 1.816882 | 0.003647 | 0.003413 | 0.003583 |
| test_forward_iterator.test | RUN | 0.736650 | 2.128266 | 1.859138 | 0.003587 | 0.003412 | 0.003375 |
| test_tuple.test | RUN | 0.645047 | 2.140706 | 1.951275 | 0.003563 | 0.003640 | 0.003490 |
| test_stream.test | RUN | 0.806966 | 3.556810 | 2.978609 | 0.004910 | 0.004507 | 0.004783 |
| test_deduced.test | RUN | 0.543724 | 1.320489 | 1.402058 | 0.003132 | 0.003093 | 0.003042 |
| test_same_type.test | RUN | 0.600171 | 1.690314 | 1.619470 | 0.003487 | 0.003303 | 0.003588 |
| test_member.test | RUN | 0.756869 | 3.332655 | 2.647993 | 0.005260 | 0.004724 | 0.005421 |
| test_null.test | RUN | 0.542734 | 1.632946 | 1.577925 | 0.003449 | 0.003374 | 0.003337 |
| test_free.test | RUN | 0.663752 | 2.652915 | 2.197968 | 0.004107 | 0.004037 | 0.004320 |
| test_is_empty.test | RUN | 0.518518 | 1.343084 | 1.410436 | 0.003165 | 0.003147 | 0.003076 |
| test_dynamic_any_cast.test | RUN | 0.978816 | 3.946031 | 4.076507 | 0.004579 | 0.004907 | 0.004906 |
| test_limits.test | RUN | 0.905284 | 2.772556 | 2.903954 | 0.003587 | 0.003483 | 0.003472 |
| test_param.test | COMPILE | 0.372996 | 0.727213 | 0.884563 | — | — | — |
| test_is_subconcept.test | COMPILE | 0.357536 | 0.574655 | 0.768314 | — | — | — |
| test_sfinae.test | COMPILE | 0.353148 | 0.613128 | 0.765592 | — | — | — |
| test_interface_order.test | COMPILE | 0.352454 | 0.584159 | 0.835314 | — | — | — |
| test_is_placeholder.test | COMPILE | 0.050036 | 0.060637 | 0.065557 | — | — | — |
| fail_default_construct.test | COMPILE_FAIL | 0.337549 | 0.557933 | 0.789719 | — | — | — |
| fail_construct_mismatch.test | COMPILE_FAIL | 0.337951 | 0.597654 | 0.684318 | — | — | — |
| fail_construct_mismatch_ref.test | COMPILE_FAIL | 0.339113 | 0.536300 | 0.792256 | — | — | — |
| fail_construct_mismatch_cref.test | COMPILE_FAIL | 0.341117 | 0.566899 | 0.802370 | — | — | — |
| fail_construct_mismatch_rref.test | COMPILE_FAIL | 0.335802 | 0.590494 | 0.688917 | — | — | — |
| fail_binding_convert_no_mapping.test | COMPILE_FAIL | 0.332103 | 0.508055 | 0.760347 | — | — | — |
| fail_increment_discard_const.test | COMPILE_FAIL | 0.343889 | 0.575873 | 0.770675 | — | — | — |
| fail_not_incrementable.test | COMPILE_FAIL | 0.353724 | 0.610619 | 0.772843 | — | — | — |
| fail_ref_assign.test | COMPILE_FAIL | 0.338751 | 0.533371 | 0.769211 | — | — | — |
| fail_cref_assign.test | COMPILE_FAIL | 0.340444 | 0.574411 | 0.786896 | — | — | — |
| fail_rref_assign.test | COMPILE_FAIL | 0.343995 | 0.579899 | 0.676139 | — | — | — |
| fail_ref_discard_const.test | COMPILE_FAIL | 0.344178 | 0.561129 | 0.792966 | — | — | — |
| fail_ref_discard_const_convert.test | COMPILE_FAIL | 0.342209 | 0.603562 | 0.758437 | — | — | — |
| fail_ref_discard_const_convert_ref.test | COMPILE_FAIL | 0.342861 | 0.618980 | 0.842926 | — | — | — |
| fail_ref_discard_const_convert_cref.test | COMPILE_FAIL | 0.347531 | 0.600376 | 0.829995 | — | — | — |
| fail_ref_discard_const_init.test | COMPILE_FAIL | 0.346893 | 0.578265 | 0.726727 | — | — | — |
| fail_any_cast_discard_const1.test | COMPILE_FAIL | 0.353195 | 0.604509 | 0.820308 | — | — | — |
| fail_any_cast_discard_const2.test | COMPILE_FAIL | 0.340649 | 0.568207 | 0.812097 | — | — | — |
| fail_any_cast_discard_const3.test | COMPILE_FAIL | 0.351063 | 0.602499 | 0.719215 | — | — | — |
| fail_any_cast_discard_const4.test | COMPILE_FAIL | 0.348413 | 0.612763 | 0.825668 | — | — | — |
| fail_any_cast_discard_const5.test | COMPILE_FAIL | 0.342913 | 0.556446 | 0.708644 | — | — | — |
| fail_any_cast_discard_const6.test | COMPILE_FAIL | 0.341292 | 0.603910 | 0.817939 | — | — | — |
| fail_any_cast_pointer_to_ref.test | COMPILE_FAIL | 0.310641 | 0.533636 | 0.678952 | — | — | — |
| fail_any_cast_pointer_to_val.test | COMPILE_FAIL | 0.312560 | 0.551008 | 0.593625 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.932×† | 0.896×, 0.967× | 1.098× |
| clang | 0.901×† | 0.826×, 0.976× | 0.929× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| test_add.test | 9 | 0.002158s | 0.001923s | 0.001955s | byte equality observed |
| test_add_assign.test | 10 | 0.002183s | 0.001976s | 0.001923s | byte equality observed |
| test_any_cast.test | 8 | 0.002435s | 0.002153s | 0.002220s | byte equality observed |
| test_assign.test | 4 | 0.006593s | 0.005123s | 0.005536s | byte equality observed |
| test_binding.test | 10 | 0.001935s | 0.001793s | 0.001840s | byte equality observed |
| test_binding_of.test | 10 | 0.001876s | 0.001692s | 0.001711s | byte equality observed |
| test_callable.test | 7 | 0.003005s | 0.002893s | 0.002908s | byte equality observed |
| test_construct.test | 5 | 0.004017s | 0.003627s | 0.003620s | byte equality observed |
| test_construct_cref.test | 7 | 0.002854s | 0.002618s | 0.002586s | byte equality observed |
| test_construct_ref.test | 8 | 0.002602s | 0.002276s | 0.002302s | byte equality observed |
| test_deduced.test | 10 | 0.001871s | 0.001681s | 0.001700s | byte equality observed |
| test_dereference.test | 10 | 0.002001s | 0.001791s | 0.001833s | byte equality observed |
| test_dynamic_any_cast.test | 7 | 0.002837s | 0.002573s | 0.002604s | byte equality observed |
| test_equal.test | 8 | 0.002357s | 0.002123s | 0.002189s | byte equality observed |
| test_forward_iterator.test | 10 | 0.001988s | 0.001799s | 0.001782s | byte equality observed |
| test_free.test | 9 | 0.002385s | 0.002116s | 0.002185s | byte equality observed |
| test_increment.test | 10 | 0.001961s | 0.001721s | 0.001743s | byte equality observed |
| test_is_empty.test | 10 | 0.001908s | 0.001765s | 0.001774s | byte equality observed |
| test_less.test | 8 | 0.002471s | 0.002286s | 0.002356s | byte equality observed |
| test_limits.test | 9 | 0.002132s | 0.001901s | 0.001911s | byte equality observed |
| test_member.test | 7 | 0.002929s | 0.002628s | 0.002667s | byte equality observed |
| test_negate.test | 9 | 0.001887s | 0.001664s | 0.001758s | byte equality observed |
| test_nested.test | 10 | 0.002004s | 0.001752s | 0.001814s | byte equality observed |
| test_null.test | 7 | 0.001991s | 0.001778s | 0.001942s | byte equality observed |
| test_reference.test | 9 | 0.002085s | 0.001871s | 0.001946s | byte equality observed |
| test_relaxed.test | 10 | 0.001954s | 0.001726s | 0.001885s | byte equality observed |
| test_same_type.test | 10 | 0.001985s | 0.001820s | 0.001957s | byte equality observed |
| test_stream.test | 8 | 0.002762s | 0.002418s | 0.002641s | byte equality observed |
| test_subscript.test | 10 | 0.002020s | 0.001858s | 0.002064s | byte equality observed |
| test_tuple.test | 9 | 0.002086s | 0.001852s | 0.002021s | byte equality observed |
| test_typeid_of.test | 9 | 0.002109s | 0.001871s | 0.002073s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
