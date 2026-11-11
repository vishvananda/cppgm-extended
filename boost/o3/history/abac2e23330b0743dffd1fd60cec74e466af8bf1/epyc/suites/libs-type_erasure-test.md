# libs/type_erasure/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_binding.test | RUN | 0.817362 | 2.949317 | 2.916857 | 0.004432 | 0.004023 | 0.004098 |
| test_increment.test | RUN | 0.778180 | 2.193948 | 2.599998 | 0.003989 | 0.004002 | 0.004006 |
| test_add.test | RUN | 1.048398 | 3.456769 | 3.722420 | 0.004627 | 0.004598 | 0.004913 |
| test_add_assign.test | RUN | 0.982800 | 3.712635 | 3.802381 | 0.004794 | 0.004829 | 0.004608 |
| test_callable.test | RUN | 1.618304 | 6.374309 | 8.077673 | 0.007064 | 0.007931 | 0.007674 |
| test_reference.test | RUN | 0.816476 | 2.599420 | 2.691510 | 0.004038 | 0.004534 | 0.003800 |
| test_construct.test | RUN | 3.242438 | 31.342529 | 23.663301 | 0.010406 | 0.009467 | 0.008111 |
| test_relaxed.test | RUN | 0.751848 | 1.882001 | 2.379970 | 0.003860 | 0.003847 | 0.003944 |
| test_assign.test | RUN | 8.766080 | 101.658503 | 79.440100 | 0.013298 | 0.008685 | 0.008881 |
| test_construct_ref.test | RUN | 0.948091 | 4.441515 | 3.687920 | 0.005694 | 0.005724 | 0.005880 |
| test_construct_cref.test | RUN | 1.078215 | 4.177832 | 4.679072 | 0.006928 | 0.006244 | 0.007054 |
| test_any_cast.test | RUN | 0.914982 | 4.416660 | 3.591028 | 0.005363 | 0.005335 | 0.005638 |
| test_binding_of.test | RUN | 0.749320 | 1.790151 | 2.352097 | 0.003821 | 0.003894 | 0.003868 |
| test_typeid_of.test | RUN | 0.794572 | 2.393131 | 2.579195 | 0.004251 | 0.004660 | 0.004467 |
| test_nested.test | RUN | 0.849830 | 2.390882 | 2.733647 | 0.003981 | 0.004316 | 0.004064 |
| test_less.test | RUN | 1.070780 | 4.699879 | 4.703491 | 0.005828 | 0.005368 | 0.005303 |
| test_equal.test | RUN | 1.005000 | 4.351773 | 3.821422 | 0.004617 | 0.004764 | 0.005001 |
| test_negate.test | RUN | 0.764423 | 1.714987 | 2.314828 | 0.003827 | 0.003562 | 0.003200 |
| test_dereference.test | RUN | 0.823597 | 2.128192 | 2.599555 | 0.004139 | 0.004248 | 0.003815 |
| test_subscript.test | RUN | 0.853785 | 2.446037 | 2.592886 | 0.004278 | 0.004096 | 0.004639 |
| test_forward_iterator.test | RUN | 0.997782 | 2.708785 | 3.346049 | 0.004209 | 0.003840 | 0.004112 |
| test_tuple.test | RUN | 0.852836 | 2.818058 | 2.638397 | 0.004459 | 0.003810 | 0.004451 |
| test_stream.test | RUN | 1.110410 | 4.543352 | 4.571727 | 0.005879 | 0.005890 | 0.005946 |
| test_deduced.test | RUN | 0.757442 | 1.611236 | 2.284747 | 0.003690 | 0.003399 | 0.003704 |
| test_same_type.test | RUN | 0.813350 | 2.168383 | 2.485647 | 0.003975 | 0.003639 | 0.003541 |
| test_member.test | RUN | 1.067241 | 4.405310 | 4.392130 | 0.006285 | 0.006538 | 0.006987 |
| test_null.test | RUN | 0.779964 | 2.224267 | 2.465893 | 0.003916 | 0.004346 | 0.004255 |
| test_free.test | RUN | 0.926466 | 3.908333 | 2.887571 | 0.005447 | 0.005476 | 0.004545 |
| test_is_empty.test | RUN | 0.727299 | 1.971557 | 2.226542 | 0.003872 | 0.003964 | 0.004099 |
| test_dynamic_any_cast.test | RUN | 1.316252 | 5.159562 | 5.660689 | 0.005902 | 0.006403 | 0.005219 |
| test_limits.test | RUN | 1.248353 | 3.937395 | 4.480380 | 0.003805 | 0.004263 | 0.003970 |
| test_param.test | COMPILE | 0.525642 | 1.166001 | 1.526351 | — | — | — |
| test_is_subconcept.test | COMPILE | 0.500360 | 0.882914 | 1.361558 | — | — | — |
| test_sfinae.test | COMPILE | 0.508055 | 0.890792 | 1.294271 | — | — | — |
| test_interface_order.test | COMPILE | 0.488430 | 0.843231 | 1.294257 | — | — | — |
| test_is_placeholder.test | COMPILE | 0.068253 | 0.085725 | 0.091436 | — | — | — |
| fail_default_construct.test | COMPILE_FAIL | 0.470978 | 0.827732 | 1.049477 | — | — | — |
| fail_construct_mismatch.test | COMPILE_FAIL | 0.492316 | 0.849756 | 1.237353 | — | — | — |
| fail_construct_mismatch_ref.test | COMPILE_FAIL | 0.479303 | 0.807040 | 1.222883 | — | — | — |
| fail_construct_mismatch_cref.test | COMPILE_FAIL | 0.471599 | 0.831051 | 1.073076 | — | — | — |
| fail_construct_mismatch_rref.test | COMPILE_FAIL | 0.487960 | 0.845640 | 1.248859 | — | — | — |
| fail_binding_convert_no_mapping.test | COMPILE_FAIL | 0.465685 | 0.764517 | 1.042750 | — | — | — |
| fail_increment_discard_const.test | COMPILE_FAIL | 0.503288 | 0.837686 | 1.251688 | — | — | — |
| fail_not_incrementable.test | COMPILE_FAIL | 0.495062 | 0.886515 | 1.316687 | — | — | — |
| fail_ref_assign.test | COMPILE_FAIL | 0.492145 | 0.758496 | 1.071226 | — | — | — |
| fail_cref_assign.test | COMPILE_FAIL | 0.483696 | 0.824437 | 1.239921 | — | — | — |
| fail_rref_assign.test | COMPILE_FAIL | 0.498727 | 0.759139 | 1.228844 | — | — | — |
| fail_ref_discard_const.test | COMPILE_FAIL | 0.471398 | 0.852274 | 1.114365 | — | — | — |
| fail_ref_discard_const_convert.test | COMPILE_FAIL | 0.491852 | 0.892670 | 1.261359 | — | — | — |
| fail_ref_discard_const_convert_ref.test | COMPILE_FAIL | 0.485940 | 0.769777 | 1.124834 | — | — | — |
| fail_ref_discard_const_convert_cref.test | COMPILE_FAIL | 0.506210 | 0.902255 | 1.290446 | — | — | — |
| fail_ref_discard_const_init.test | COMPILE_FAIL | 0.478546 | 0.834297 | 1.222582 | — | — | — |
| fail_any_cast_discard_const1.test | COMPILE_FAIL | 0.493593 | 0.769844 | 1.290225 | — | — | — |
| fail_any_cast_discard_const2.test | COMPILE_FAIL | 0.490691 | 0.871319 | 1.133679 | — | — | — |
| fail_any_cast_discard_const3.test | COMPILE_FAIL | 0.492886 | 0.851526 | 1.267164 | — | — | — |
| fail_any_cast_discard_const4.test | COMPILE_FAIL | 0.477577 | 0.771895 | 1.265963 | — | — | — |
| fail_any_cast_discard_const5.test | COMPILE_FAIL | 0.483327 | 0.860531 | 1.232277 | — | — | — |
| fail_any_cast_discard_const6.test | COMPILE_FAIL | 0.489484 | 0.827084 | 1.293456 | — | — | — |
| fail_any_cast_pointer_to_ref.test | COMPILE_FAIL | 0.427955 | 0.855720 | 1.037354 | — | — | — |
| fail_any_cast_pointer_to_val.test | COMPILE_FAIL | 0.355805 | 0.717351 | 1.086551 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.953× | 0.947×, 0.959× | 0.996× |
| clang | 0.950× | 0.951×, 0.949× | 1.003× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| test_add.test | 7 | 0.003097s | 0.002925s | 0.002919s | byte equality observed |
| test_add_assign.test | 7 | 0.003093s | 0.002922s | 0.002913s | byte equality observed |
| test_any_cast.test | 7 | 0.003586s | 0.003423s | 0.003413s | byte equality observed |
| test_assign.test | 3 | 0.010262s | 0.008464s | 0.009768s | byte equality observed |
| test_binding.test | 9 | 0.002893s | 0.002826s | 0.002751s | byte equality observed |
| test_binding_of.test | 8 | 0.002681s | 0.002570s | 0.002496s | byte equality observed |
| test_callable.test | 5 | 0.004649s | 0.004420s | 0.004364s | byte equality observed |
| test_construct.test | 4 | 0.006015s | 0.005939s | 0.005646s | byte equality observed |
| test_construct_cref.test | 6 | 0.004160s | 0.003922s | 0.003883s | byte equality observed |
| test_construct_ref.test | 7 | 0.003704s | 0.003586s | 0.003528s | byte equality observed |
| test_deduced.test | 8 | 0.002738s | 0.002616s | 0.002656s | byte equality observed |
| test_dereference.test | 8 | 0.002897s | 0.002796s | 0.002752s | byte equality observed |
| test_dynamic_any_cast.test | 6 | 0.004151s | 0.004051s | 0.003805s | byte equality observed |
| test_equal.test | 7 | 0.003508s | 0.003415s | 0.003378s | byte equality observed |
| test_forward_iterator.test | 8 | 0.002909s | 0.002804s | 0.002729s | byte equality observed |
| test_free.test | 7 | 0.003482s | 0.003386s | 0.003279s | byte equality observed |
| test_increment.test | 9 | 0.002872s | 0.002732s | 0.002675s | byte equality observed |
| test_is_empty.test | 8 | 0.002836s | 0.002695s | 0.002659s | byte equality observed |
| test_less.test | 7 | 0.003735s | 0.003660s | 0.003504s | byte equality observed |
| test_limits.test | 8 | 0.003066s | 0.002921s | 0.002933s | byte equality observed |
| test_member.test | 6 | 0.004291s | 0.004139s | 0.004056s | byte equality observed |
| test_negate.test | 8 | 0.002753s | 0.002631s | 0.002610s | byte equality observed |
| test_nested.test | 8 | 0.002783s | 0.002736s | 0.002667s | byte equality observed |
| test_null.test | 8 | 0.002929s | 0.002839s | 0.002725s | byte equality observed |
| test_reference.test | 6 | 0.003001s | 0.002919s | 0.002908s | byte equality observed |
| test_relaxed.test | 7 | 0.002790s | 0.002646s | 0.002663s | byte equality observed |
| test_same_type.test | 8 | 0.002885s | 0.002817s | 0.002696s | byte equality observed |
| test_stream.test | 6 | 0.003977s | 0.003818s | 0.003753s | byte equality observed |
| test_subscript.test | 7 | 0.002994s | 0.002850s | 0.002822s | byte equality observed |
| test_tuple.test | 6 | 0.002961s | 0.002908s | 0.002890s | byte equality observed |
| test_typeid_of.test | 7 | 0.002953s | 0.002908s | 0.002856s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
