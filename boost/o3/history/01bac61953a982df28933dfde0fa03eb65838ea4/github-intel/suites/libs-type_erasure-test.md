# libs/type_erasure/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_binding.test | RUN | 0.406862 | 1.782718 | 1.514804 | 0.002361 | 0.002212 | 0.002405 |
| test_increment.test | RUN | 0.408554 | 1.354232 | 1.301333 | 0.002099 | 0.002196 | 0.002091 |
| test_add.test | RUN | 0.501701 | 2.192339 | 1.916685 | 0.002329 | 0.002449 | 0.002250 |
| test_add_assign.test | RUN | 0.516709 | 2.359998 | 1.993666 | 0.002188 | 0.002357 | 0.002150 |
| test_callable.test | RUN | 0.877808 | 4.057280 | 4.467869 | 0.003254 | 0.003165 | 0.003132 |
| test_reference.test | RUN | 0.390965 | 1.617206 | 1.346335 | 0.001915 | 0.002192 | 0.002383 |
| test_construct.test | RUN | 2.054688 | 18.941314 | 13.626016 | 0.003879 | 0.003731 | 0.004221 |
| test_relaxed.test | RUN | 0.389802 | 1.151513 | 1.204337 | 0.001920 | 0.002207 | 0.002057 |
| test_assign.test | RUN | 4.861633 | 62.175554 | 51.846319 | 0.006305 | 0.003863 | 0.004409 |
| test_construct_ref.test | RUN | 0.509720 | 2.851254 | 1.960648 | 0.002440 | 0.002556 | 0.002784 |
| test_construct_cref.test | RUN | 0.556585 | 2.667102 | 2.552584 | 0.002898 | 0.003055 | 0.002814 |
| test_any_cast.test | RUN | 0.471258 | 2.804761 | 1.924540 | 0.002480 | 0.002633 | 0.002606 |
| test_binding_of.test | RUN | 0.385095 | 1.067797 | 1.119786 | 0.002070 | 0.002095 | 0.002191 |
| test_typeid_of.test | RUN | 0.390728 | 1.554371 | 1.299784 | 0.002276 | 0.002210 | 0.002451 |
| test_nested.test | RUN | 0.434727 | 1.498896 | 1.490578 | 0.002086 | 0.002223 | 0.002302 |
| test_less.test | RUN | 0.581282 | 2.941850 | 2.486076 | 0.002489 | 0.002731 | 0.002578 |
| test_equal.test | RUN | 0.530057 | 2.835701 | 2.069332 | 0.002407 | 0.002677 | 0.002214 |
| test_negate.test | RUN | 0.400157 | 1.131114 | 1.201474 | 0.002045 | 0.002155 | 0.002211 |
| test_dereference.test | RUN | 0.405821 | 1.482543 | 1.437627 | 0.002219 | 0.002151 | 0.002338 |
| test_subscript.test | RUN | 0.446548 | 1.692405 | 1.526811 | 0.002118 | 0.002305 | 0.002309 |
| test_forward_iterator.test | RUN | 0.536206 | 1.889905 | 1.749626 | 0.002328 | 0.002310 | 0.002291 |
| test_tuple.test | RUN | 0.450282 | 1.876277 | 1.556107 | 0.002012 | 0.002113 | 0.002169 |
| test_stream.test | RUN | 0.581390 | 3.185052 | 2.482818 | 0.002611 | 0.002890 | 0.002684 |
| test_deduced.test | RUN | 0.384864 | 1.106770 | 1.153135 | 0.001877 | 0.001951 | 0.002272 |
| test_same_type.test | RUN | 0.462990 | 1.512335 | 1.477910 | 0.002183 | 0.002110 | 0.002237 |
| test_member.test | RUN | 0.554238 | 2.990949 | 2.323988 | 0.002865 | 0.002740 | 0.003165 |
| test_null.test | RUN | 0.387948 | 1.434328 | 1.246881 | 0.001874 | 0.002291 | 0.002234 |
| test_free.test | RUN | 0.487961 | 2.489609 | 1.816311 | 0.002383 | 0.002481 | 0.002587 |
| test_is_empty.test | RUN | 0.369656 | 1.221311 | 1.200431 | 0.002100 | 0.001946 | 0.002581 |
| test_dynamic_any_cast.test | RUN | 0.710221 | 3.606748 | 3.446421 | 0.002649 | 0.003020 | 0.002956 |
| test_limits.test | RUN | 0.638895 | 2.504144 | 2.432461 | 0.001972 | 0.002522 | 0.002532 |
| test_param.test | COMPILE | 0.252971 | 0.679769 | 0.754656 | — | — | — |
| test_is_subconcept.test | COMPILE | 0.242517 | 0.523825 | 0.657545 | — | — | — |
| test_sfinae.test | COMPILE | 0.241590 | 0.524583 | 0.674619 | — | — | — |
| test_interface_order.test | COMPILE | 0.235584 | 0.519208 | 0.642392 | — | — | — |
| test_is_placeholder.test | COMPILE | 0.027195 | 0.048027 | 0.055156 | — | — | — |
| fail_default_construct.test | COMPILE_FAIL | 0.231349 | 0.499960 | 0.607807 | — | — | — |
| fail_construct_mismatch.test | COMPILE_FAIL | 0.230408 | 0.509521 | 0.627280 | — | — | — |
| fail_construct_mismatch_ref.test | COMPILE_FAIL | 0.229546 | 0.492230 | 0.617970 | — | — | — |
| fail_construct_mismatch_cref.test | COMPILE_FAIL | 0.227973 | 0.490647 | 0.623945 | — | — | — |
| fail_construct_mismatch_rref.test | COMPILE_FAIL | 0.231865 | 0.498337 | 0.577555 | — | — | — |
| fail_binding_convert_no_mapping.test | COMPILE_FAIL | 0.223862 | 0.480087 | 0.595428 | — | — | — |
| fail_increment_discard_const.test | COMPILE_FAIL | 0.228447 | 0.481711 | 0.617387 | — | — | — |
| fail_not_incrementable.test | COMPILE_FAIL | 0.240027 | 0.513231 | 0.605062 | — | — | — |
| fail_ref_assign.test | COMPILE_FAIL | 0.234519 | 0.478521 | 0.604539 | — | — | — |
| fail_cref_assign.test | COMPILE_FAIL | 0.226345 | 0.490167 | 0.618038 | — | — | — |
| fail_rref_assign.test | COMPILE_FAIL | 0.229751 | 0.491167 | 0.571566 | — | — | — |
| fail_ref_discard_const.test | COMPILE_FAIL | 0.226279 | 0.493455 | 0.626658 | — | — | — |
| fail_ref_discard_const_convert.test | COMPILE_FAIL | 0.234379 | 0.525886 | 0.639722 | — | — | — |
| fail_ref_discard_const_convert_ref.test | COMPILE_FAIL | 0.229764 | 0.516836 | 0.603500 | — | — | — |
| fail_ref_discard_const_convert_cref.test | COMPILE_FAIL | 0.233128 | 0.515577 | 0.647544 | — | — | — |
| fail_ref_discard_const_init.test | COMPILE_FAIL | 0.225666 | 0.507120 | 0.608731 | — | — | — |
| fail_any_cast_discard_const1.test | COMPILE_FAIL | 0.236812 | 0.509657 | 0.602173 | — | — | — |
| fail_any_cast_discard_const2.test | COMPILE_FAIL | 0.233633 | 0.498886 | 0.649137 | — | — | — |
| fail_any_cast_discard_const3.test | COMPILE_FAIL | 0.236973 | 0.512183 | 0.651777 | — | — | — |
| fail_any_cast_discard_const4.test | COMPILE_FAIL | 0.230546 | 0.505075 | 0.642704 | — | — | — |
| fail_any_cast_discard_const5.test | COMPILE_FAIL | 0.235833 | 0.497939 | 0.646509 | — | — | — |
| fail_any_cast_discard_const6.test | COMPILE_FAIL | 0.229393 | 0.514941 | 0.646534 | — | — | — |
| fail_any_cast_pointer_to_ref.test | COMPILE_FAIL | 0.205922 | 0.507173 | 0.610184 | — | — | — |
| fail_any_cast_pointer_to_val.test | COMPILE_FAIL | 0.176853 | 0.378528 | 0.506818 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.962× | 0.958×, 0.966× | 1.000× |
| clang | 0.959× | 0.959×, 0.959× | 1.005× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814672).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| test_add.test | 10 | 0.001384s | 0.001362s | 0.001348s | byte equality observed |
| test_add_assign.test | 10 | 0.001394s | 0.001347s | 0.001334s | byte equality observed |
| test_any_cast.test | 10 | 0.001482s | 0.001458s | 0.001467s | byte equality observed |
| test_assign.test | 5 | 0.003849s | 0.003217s | 0.003658s | byte equality observed |
| test_binding.test | 10 | 0.001298s | 0.001272s | 0.001262s | byte equality observed |
| test_binding_of.test | 10 | 0.001282s | 0.001233s | 0.001223s | byte equality observed |
| test_callable.test | 10 | 0.001739s | 0.001678s | 0.001665s | byte equality observed |
| test_construct.test | 9 | 0.002257s | 0.002183s | 0.002102s | byte equality observed |
| test_construct_cref.test | 10 | 0.001623s | 0.001593s | 0.001573s | byte equality observed |
| test_construct_ref.test | 10 | 0.001516s | 0.001464s | 0.001433s | byte equality observed |
| test_deduced.test | 10 | 0.001291s | 0.001244s | 0.001228s | byte equality observed |
| test_dereference.test | 10 | 0.001323s | 0.001295s | 0.001266s | byte equality observed |
| test_dynamic_any_cast.test | 10 | 0.001626s | 0.001636s | 0.001560s | byte equality observed |
| test_equal.test | 10 | 0.001452s | 0.001441s | 0.001407s | byte equality observed |
| test_forward_iterator.test | 10 | 0.001336s | 0.001273s | 0.001262s | byte equality observed |
| test_free.test | 10 | 0.001422s | 0.001408s | 0.001383s | byte equality observed |
| test_increment.test | 10 | 0.001355s | 0.001275s | 0.001229s | byte equality observed |
| test_is_empty.test | 10 | 0.001298s | 0.001256s | 0.001272s | byte equality observed |
| test_less.test | 10 | 0.001516s | 0.001479s | 0.001470s | byte equality observed |
| test_limits.test | 10 | 0.001375s | 0.001333s | 0.001345s | byte equality observed |
| test_member.test | 10 | 0.001644s | 0.001607s | 0.001569s | byte equality observed |
| test_negate.test | 10 | 0.001263s | 0.001260s | 0.001184s | byte equality observed |
| test_nested.test | 10 | 0.001282s | 0.001237s | 0.001252s | byte equality observed |
| test_null.test | 10 | 0.001302s | 0.001303s | 0.001232s | byte equality observed |
| test_reference.test | 10 | 0.001322s | 0.001302s | 0.001266s | byte equality observed |
| test_relaxed.test | 10 | 0.001294s | 0.001259s | 0.001248s | byte equality observed |
| test_same_type.test | 10 | 0.001309s | 0.001313s | 0.001270s | byte equality observed |
| test_stream.test | 10 | 0.001575s | 0.001548s | 0.001501s | byte equality observed |
| test_subscript.test | 10 | 0.001353s | 0.001325s | 0.001291s | byte equality observed |
| test_tuple.test | 10 | 0.001345s | 0.001285s | 0.001260s | byte equality observed |
| test_typeid_of.test | 10 | 0.001333s | 0.001326s | 0.001270s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814672).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
