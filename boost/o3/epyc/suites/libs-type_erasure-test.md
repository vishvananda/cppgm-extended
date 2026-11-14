# libs/type_erasure/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_binding.test | RUN | 0.684937 | 2.838842 | 2.914174 | 0.004177 | 0.004365 | 0.004052 |
| test_increment.test | RUN | 0.712702 | 2.116043 | 2.562025 | 0.003725 | 0.004037 | 0.004050 |
| test_add.test | RUN | 0.845444 | 3.320525 | 3.753944 | 0.004352 | 0.004088 | 0.004542 |
| test_add_assign.test | RUN | 0.867748 | 3.545219 | 3.906510 | 0.004218 | 0.004270 | 0.005003 |
| test_callable.test | RUN | 1.455549 | 6.104691 | 8.333956 | 0.007238 | 0.006522 | 0.008022 |
| test_reference.test | RUN | 0.668514 | 2.558109 | 2.724362 | 0.004079 | 0.004132 | 0.004364 |
| test_construct.test | RUN | 3.155473 | 29.851143 | 23.346156 | 0.009128 | 0.008438 | 0.010393 |
| test_relaxed.test | RUN | 0.682940 | 1.797772 | 2.394273 | 0.003706 | 0.004018 | 0.004159 |
| test_assign.test | RUN | 7.015127 | 98.803872 | 75.047835 | 0.014339 | 0.008267 | 0.008538 |
| test_construct_ref.test | RUN | 0.803816 | 4.325809 | 3.802630 | 0.005671 | 0.005453 | 0.005998 |
| test_construct_cref.test | RUN | 0.876168 | 4.147764 | 4.829901 | 0.005877 | 0.006093 | 0.006841 |
| test_any_cast.test | RUN | 0.797943 | 4.304635 | 3.541450 | 0.005296 | 0.004972 | 0.004699 |
| test_binding_of.test | RUN | 0.607834 | 1.731065 | 2.282876 | 0.003624 | 0.003776 | 0.004073 |
| test_typeid_of.test | RUN | 0.680643 | 2.382003 | 2.527358 | 0.003752 | 0.004250 | 0.004650 |
| test_nested.test | RUN | 0.727976 | 2.359767 | 2.919474 | 0.003648 | 0.004137 | 0.004058 |
| test_less.test | RUN | 0.922851 | 4.517586 | 4.767532 | 0.005569 | 0.005624 | 0.006099 |
| test_equal.test | RUN | 0.868553 | 4.214381 | 3.857785 | 0.005205 | 0.004650 | 0.005378 |
| test_negate.test | RUN | 0.626305 | 1.685873 | 2.217884 | 0.003610 | 0.003577 | 0.003702 |
| test_dereference.test | RUN | 0.711566 | 2.087189 | 2.536433 | 0.004087 | 0.004178 | 0.003899 |
| test_subscript.test | RUN | 0.725556 | 2.391206 | 2.751393 | 0.004009 | 0.003854 | 0.004646 |
| test_forward_iterator.test | RUN | 0.882107 | 2.861932 | 2.937147 | 0.004029 | 0.003858 | 0.004296 |
| test_tuple.test | RUN | 0.770353 | 2.764481 | 2.965841 | 0.004182 | 0.004308 | 0.004496 |
| test_stream.test | RUN | 0.963889 | 4.710812 | 4.605754 | 0.005624 | 0.005642 | 0.006465 |
| test_deduced.test | RUN | 0.654394 | 1.640127 | 2.077658 | 0.003711 | 0.004034 | 0.003646 |
| test_same_type.test | RUN | 0.721546 | 2.401003 | 2.797649 | 0.003962 | 0.003868 | 0.004130 |
| test_member.test | RUN | 0.927222 | 4.555763 | 3.918467 | 0.006566 | 0.006556 | 0.005837 |
| test_null.test | RUN | 0.630846 | 2.288633 | 2.308457 | 0.003629 | 0.004219 | 0.004095 |
| test_free.test | RUN | 0.799503 | 3.875450 | 3.418774 | 0.004747 | 0.004898 | 0.005582 |
| test_is_empty.test | RUN | 0.631832 | 1.875330 | 2.286470 | 0.003803 | 0.003704 | 0.004295 |
| test_dynamic_any_cast.test | RUN | 1.178707 | 5.262046 | 5.582436 | 0.006037 | 0.005850 | 0.006778 |
| test_limits.test | RUN | 1.108897 | 3.923030 | 4.473819 | 0.004326 | 0.004075 | 0.004507 |
| test_param.test | COMPILE | 0.423705 | 1.150427 | 1.526741 | — | — | — |
| test_is_subconcept.test | COMPILE | 0.420286 | 0.872692 | 1.370486 | — | — | — |
| test_sfinae.test | COMPILE | 0.421905 | 0.892345 | 1.297477 | — | — | — |
| test_interface_order.test | COMPILE | 0.422390 | 0.850837 | 1.332497 | — | — | — |
| test_is_placeholder.test | COMPILE | 0.051484 | 0.089155 | 0.092573 | — | — | — |
| fail_default_construct.test | COMPILE_FAIL | 0.396632 | 0.835815 | 1.064896 | — | — | — |
| fail_construct_mismatch.test | COMPILE_FAIL | 0.410389 | 0.869640 | 1.256465 | — | — | — |
| fail_construct_mismatch_ref.test | COMPILE_FAIL | 0.393801 | 0.804349 | 1.223946 | — | — | — |
| fail_construct_mismatch_cref.test | COMPILE_FAIL | 0.398489 | 0.827391 | 1.050836 | — | — | — |
| fail_construct_mismatch_rref.test | COMPILE_FAIL | 0.392626 | 0.853457 | 1.259144 | — | — | — |
| fail_binding_convert_no_mapping.test | COMPILE_FAIL | 0.383720 | 0.771816 | 1.218509 | — | — | — |
| fail_increment_discard_const.test | COMPILE_FAIL | 0.395201 | 0.842509 | 1.144319 | — | — | — |
| fail_not_incrementable.test | COMPILE_FAIL | 0.416543 | 0.773623 | 1.293943 | — | — | — |
| fail_ref_assign.test | COMPILE_FAIL | 0.385168 | 0.820114 | 1.234358 | — | — | — |
| fail_cref_assign.test | COMPILE_FAIL | 0.382455 | 0.844207 | 1.069967 | — | — | — |
| fail_rref_assign.test | COMPILE_FAIL | 0.393650 | 0.752176 | 1.233536 | — | — | — |
| fail_ref_discard_const.test | COMPILE_FAIL | 0.391968 | 0.844215 | 1.263264 | — | — | — |
| fail_ref_discard_const_convert.test | COMPILE_FAIL | 0.396555 | 0.896577 | 1.159687 | — | — | — |
| fail_ref_discard_const_convert_ref.test | COMPILE_FAIL | 0.389616 | 0.837702 | 1.295825 | — | — | — |
| fail_ref_discard_const_convert_cref.test | COMPILE_FAIL | 0.399540 | 0.903408 | 1.302681 | — | — | — |
| fail_ref_discard_const_init.test | COMPILE_FAIL | 0.394636 | 0.780764 | 1.099707 | — | — | — |
| fail_any_cast_discard_const1.test | COMPILE_FAIL | 0.392957 | 0.848774 | 1.281821 | — | — | — |
| fail_any_cast_discard_const2.test | COMPILE_FAIL | 0.390530 | 0.857298 | 1.103055 | — | — | — |
| fail_any_cast_discard_const3.test | COMPILE_FAIL | 0.399965 | 0.832369 | 1.305603 | — | — | — |
| fail_any_cast_discard_const4.test | COMPILE_FAIL | 0.404783 | 0.867075 | 1.281298 | — | — | — |
| fail_any_cast_discard_const5.test | COMPILE_FAIL | 0.395844 | 0.873521 | 1.103802 | — | — | — |
| fail_any_cast_discard_const6.test | COMPILE_FAIL | 0.393606 | 0.809894 | 1.265919 | — | — | — |
| fail_any_cast_pointer_to_ref.test | COMPILE_FAIL | 0.367964 | 0.870978 | 0.918464 | — | — | — |
| fail_any_cast_pointer_to_val.test | COMPILE_FAIL | 0.329952 | 0.730082 | 1.043335 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.966× | 0.953×, 0.980× | 1.014× |
| clang | 0.954× | 0.964×, 0.944× | 0.998× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274616).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| test_add.test | 7 | 0.002946s | 0.002863s | 0.002838s | byte equality observed |
| test_add_assign.test | 8 | 0.002930s | 0.002777s | 0.002845s | byte equality observed |
| test_any_cast.test | 7 | 0.003459s | 0.003238s | 0.003325s | byte equality observed |
| test_assign.test | 3 | 0.010204s | 0.008705s | 0.009326s | byte equality observed |
| test_binding.test | 7 | 0.002759s | 0.002652s | 0.002573s | byte equality observed |
| test_binding_of.test | 8 | 0.002608s | 0.002516s | 0.002495s | byte equality observed |
| test_callable.test | 6 | 0.004402s | 0.004315s | 0.004153s | byte equality observed |
| test_construct.test | 5 | 0.005798s | 0.005617s | 0.005475s | byte equality observed |
| test_construct_cref.test | 6 | 0.003994s | 0.003959s | 0.003754s | byte equality observed |
| test_construct_ref.test | 7 | 0.003598s | 0.003441s | 0.003383s | byte equality observed |
| test_deduced.test | 8 | 0.002539s | 0.002491s | 0.002487s | byte equality observed |
| test_dereference.test | 8 | 0.002823s | 0.002663s | 0.002676s | byte equality observed |
| test_dynamic_any_cast.test | 6 | 0.003967s | 0.003933s | 0.003864s | byte equality observed |
| test_equal.test | 7 | 0.003275s | 0.003193s | 0.003201s | byte equality observed |
| test_forward_iterator.test | 8 | 0.002792s | 0.002748s | 0.002687s | byte equality observed |
| test_free.test | 7 | 0.003312s | 0.003144s | 0.003213s | byte equality observed |
| test_increment.test | 8 | 0.002722s | 0.002645s | 0.002570s | byte equality observed |
| test_is_empty.test | 8 | 0.002703s | 0.002553s | 0.002492s | byte equality observed |
| test_less.test | 7 | 0.003588s | 0.003396s | 0.003535s | byte equality observed |
| test_limits.test | 8 | 0.002875s | 0.002854s | 0.002776s | byte equality observed |
| test_member.test | 6 | 0.004127s | 0.004072s | 0.003979s | byte equality observed |
| test_negate.test | 9 | 0.002607s | 0.002525s | 0.002507s | byte equality observed |
| test_nested.test | 8 | 0.002661s | 0.002619s | 0.002560s | byte equality observed |
| test_null.test | 8 | 0.002836s | 0.002697s | 0.002703s | byte equality observed |
| test_reference.test | 8 | 0.002885s | 0.002753s | 0.002787s | byte equality observed |
| test_relaxed.test | 9 | 0.002605s | 0.002550s | 0.002602s | byte equality observed |
| test_same_type.test | 7 | 0.002791s | 0.002683s | 0.002712s | byte equality observed |
| test_stream.test | 6 | 0.003789s | 0.003721s | 0.003562s | byte equality observed |
| test_subscript.test | 7 | 0.002893s | 0.002800s | 0.002721s | byte equality observed |
| test_tuple.test | 8 | 0.002855s | 0.002785s | 0.002817s | byte equality observed |
| test_typeid_of.test | 7 | 0.002878s | 0.002793s | 0.002763s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274616).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
