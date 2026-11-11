# libs/type_erasure/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_binding.test | RUN | 0.501921 | 1.787037 | 1.436176 | 0.003406 | 0.003477 | 0.003167 |
| test_increment.test | RUN | 0.467735 | 1.334493 | 1.187958 | 0.003336 | 0.003223 | 0.003060 |
| test_add.test | RUN | 0.585049 | 2.143416 | 1.786697 | 0.003705 | 0.003619 | 0.003379 |
| test_add_assign.test | RUN | 0.629435 | 2.286381 | 1.879264 | 0.003832 | 0.003634 | 0.003463 |
| test_callable.test | RUN | 0.981606 | 4.143321 | 4.263797 | 0.005638 | 0.005786 | 0.005193 |
| test_reference.test | RUN | 0.481592 | 1.623105 | 1.271138 | 0.003385 | 0.003557 | 0.003249 |
| test_construct.test | RUN | 1.954029 | 22.146280 | 12.358682 | 0.007386 | 0.006471 | 0.006461 |
| test_relaxed.test | RUN | 0.488059 | 1.148259 | 1.102965 | 0.003336 | 0.003345 | 0.003051 |
| test_assign.test | RUN | 5.582915 | 67.170880 | 42.699700 | 0.009588 | 0.006360 | 0.006733 |
| test_construct_ref.test | RUN | 0.643524 | 2.721060 | 1.914968 | 0.004254 | 0.004635 | 0.004236 |
| test_construct_cref.test | RUN | 0.728476 | 2.619385 | 2.255883 | 0.005121 | 0.005110 | 0.004767 |
| test_any_cast.test | RUN | 0.583592 | 2.690146 | 1.804088 | 0.004098 | 0.004046 | 0.003841 |
| test_binding_of.test | RUN | 0.529777 | 1.113722 | 1.078201 | 0.003133 | 0.003207 | 0.002893 |
| test_typeid_of.test | RUN | 0.483632 | 1.558128 | 1.210405 | 0.003597 | 0.003538 | 0.003313 |
| test_nested.test | RUN | 0.520588 | 1.523512 | 1.383642 | 0.003463 | 0.003454 | 0.003132 |
| test_less.test | RUN | 0.690062 | 2.999817 | 2.286495 | 0.004334 | 0.004619 | 0.004236 |
| test_equal.test | RUN | 0.662941 | 2.815590 | 1.919705 | 0.003956 | 0.004095 | 0.003998 |
| test_negate.test | RUN | 0.475040 | 1.159854 | 1.121770 | 0.003164 | 0.003403 | 0.002974 |
| test_dereference.test | RUN | 0.528599 | 1.470047 | 1.321016 | 0.003302 | 0.003175 | 0.003182 |
| test_subscript.test | RUN | 0.549343 | 1.604310 | 1.393954 | 0.003456 | 0.003486 | 0.003130 |
| test_forward_iterator.test | RUN | 0.655114 | 1.790395 | 1.643840 | 0.003282 | 0.003246 | 0.003383 |
| test_tuple.test | RUN | 0.559234 | 1.769366 | 1.307671 | 0.004057 | 0.003553 | 0.003170 |
| test_stream.test | RUN | 0.709671 | 2.978472 | 2.343656 | 0.004354 | 0.004441 | 0.004100 |
| test_deduced.test | RUN | 0.482949 | 1.082638 | 1.134129 | 0.003145 | 0.003077 | 0.002482 |
| test_same_type.test | RUN | 0.551228 | 1.391285 | 1.275367 | 0.003479 | 0.003286 | 0.003108 |
| test_member.test | RUN | 0.708379 | 2.858771 | 1.995997 | 0.005043 | 0.005339 | 0.004208 |
| test_null.test | RUN | 0.480788 | 1.350435 | 1.126347 | 0.003737 | 0.003117 | 0.003262 |
| test_free.test | RUN | 0.625576 | 2.459676 | 1.681656 | 0.003864 | 0.004012 | 0.003481 |
| test_is_empty.test | RUN | 0.477491 | 1.213181 | 1.022816 | 0.003362 | 0.003142 | 0.003123 |
| test_dynamic_any_cast.test | RUN | 0.875735 | 3.587472 | 3.197211 | 0.004498 | 0.004817 | 0.004598 |
| test_limits.test | RUN | 0.817851 | 2.372535 | 2.075848 | 0.003629 | 0.003409 | 0.003446 |
| test_param.test | COMPILE | 0.361925 | 0.601018 | 0.683974 | — | — | — |
| test_is_subconcept.test | COMPILE | 0.346908 | 0.464633 | 0.627616 | — | — | — |
| test_sfinae.test | COMPILE | 0.316795 | 0.512287 | 0.602455 | — | — | — |
| test_interface_order.test | COMPILE | 0.311650 | 0.478009 | 0.601980 | — | — | — |
| test_is_placeholder.test | COMPILE | 0.041622 | 0.048428 | 0.055136 | — | — | — |
| fail_default_construct.test | COMPILE_FAIL | 0.293990 | 0.448272 | 0.539202 | — | — | — |
| fail_construct_mismatch.test | COMPILE_FAIL | 0.299180 | 0.471239 | 0.540370 | — | — | — |
| fail_construct_mismatch_ref.test | COMPILE_FAIL | 0.295399 | 0.473633 | 0.488573 | — | — | — |
| fail_construct_mismatch_cref.test | COMPILE_FAIL | 0.299248 | 0.449942 | 0.548910 | — | — | — |
| fail_construct_mismatch_rref.test | COMPILE_FAIL | 0.300515 | 0.464471 | 0.557563 | — | — | — |
| fail_binding_convert_no_mapping.test | COMPILE_FAIL | 0.289123 | 0.452292 | 0.479680 | — | — | — |
| fail_increment_discard_const.test | COMPILE_FAIL | 0.309780 | 0.472256 | 0.553869 | — | — | — |
| fail_not_incrementable.test | COMPILE_FAIL | 0.313451 | 0.484556 | 0.590172 | — | — | — |
| fail_ref_assign.test | COMPILE_FAIL | 0.294978 | 0.461631 | 0.493015 | — | — | — |
| fail_cref_assign.test | COMPILE_FAIL | 0.296191 | 0.484152 | 0.558096 | — | — | — |
| fail_rref_assign.test | COMPILE_FAIL | 0.292155 | 0.479967 | 0.483725 | — | — | — |
| fail_ref_discard_const.test | COMPILE_FAIL | 0.297266 | 0.499516 | 0.545637 | — | — | — |
| fail_ref_discard_const_convert.test | COMPILE_FAIL | 0.298464 | 0.491998 | 0.567880 | — | — | — |
| fail_ref_discard_const_convert_ref.test | COMPILE_FAIL | 0.301659 | 0.499913 | 0.508740 | — | — | — |
| fail_ref_discard_const_convert_cref.test | COMPILE_FAIL | 0.300148 | 0.506426 | 0.549233 | — | — | — |
| fail_ref_discard_const_init.test | COMPILE_FAIL | 0.299788 | 0.434285 | 0.509951 | — | — | — |
| fail_any_cast_discard_const1.test | COMPILE_FAIL | 0.301056 | 0.464149 | 0.540728 | — | — | — |
| fail_any_cast_discard_const2.test | COMPILE_FAIL | 0.302975 | 0.472261 | 0.538356 | — | — | — |
| fail_any_cast_discard_const3.test | COMPILE_FAIL | 0.301061 | 0.425340 | 0.558347 | — | — | — |
| fail_any_cast_discard_const4.test | COMPILE_FAIL | 0.304827 | 0.481590 | 0.580369 | — | — | — |
| fail_any_cast_discard_const5.test | COMPILE_FAIL | 0.299231 | 0.483420 | 0.516101 | — | — | — |
| fail_any_cast_discard_const6.test | COMPILE_FAIL | 0.298338 | 0.464217 | 0.579376 | — | — | — |
| fail_any_cast_pointer_to_ref.test | COMPILE_FAIL | 0.281294 | 0.487141 | 0.527975 | — | — | — |
| fail_any_cast_pointer_to_val.test | COMPILE_FAIL | 0.269296 | 0.471078 | 0.518681 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.988× | 0.980×, 0.997× | 0.985× |
| clang | 0.961× | 0.967×, 0.954× | 1.007× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| test_add.test | 10 | 0.001849s | 0.001836s | 0.001755s | byte equality observed |
| test_add_assign.test | 10 | 0.001811s | 0.001807s | 0.001735s | byte equality observed |
| test_any_cast.test | 9 | 0.002079s | 0.002073s | 0.002018s | byte equality observed |
| test_assign.test | 4 | 0.005098s | 0.004668s | 0.004855s | byte equality observed |
| test_binding.test | 10 | 0.001739s | 0.001700s | 0.001660s | byte equality observed |
| test_binding_of.test | 10 | 0.001634s | 0.001586s | 0.001583s | byte equality observed |
| test_callable.test | 7 | 0.002671s | 0.002693s | 0.002565s | byte equality observed |
| test_construct.test | 6 | 0.003497s | 0.003503s | 0.003327s | byte equality observed |
| test_construct_cref.test | 8 | 0.002428s | 0.002484s | 0.002391s | byte equality observed |
| test_construct_ref.test | 8 | 0.002151s | 0.002189s | 0.002133s | byte equality observed |
| test_deduced.test | 10 | 0.001626s | 0.001620s | 0.001556s | byte equality observed |
| test_dereference.test | 10 | 0.001715s | 0.001756s | 0.001678s | byte equality observed |
| test_dynamic_any_cast.test | 8 | 0.002373s | 0.002419s | 0.002332s | byte equality observed |
| test_equal.test | 9 | 0.002021s | 0.002034s | 0.001960s | byte equality observed |
| test_forward_iterator.test | 10 | 0.001730s | 0.001721s | 0.001692s | byte equality observed |
| test_free.test | 9 | 0.001994s | 0.002040s | 0.001938s | byte equality observed |
| test_increment.test | 10 | 0.001682s | 0.001639s | 0.001580s | byte equality observed |
| test_is_empty.test | 10 | 0.001639s | 0.001666s | 0.001610s | byte equality observed |
| test_less.test | 9 | 0.002177s | 0.002166s | 0.002129s | byte equality observed |
| test_limits.test | 10 | 0.001809s | 0.001801s | 0.001767s | byte equality observed |
| test_member.test | 8 | 0.002470s | 0.002433s | 0.002390s | byte equality observed |
| test_negate.test | 10 | 0.001611s | 0.001591s | 0.001553s | byte equality observed |
| test_nested.test | 10 | 0.001674s | 0.001642s | 0.001654s | byte equality observed |
| test_null.test | 10 | 0.001698s | 0.001681s | 0.001641s | byte equality observed |
| test_reference.test | 10 | 0.001755s | 0.001758s | 0.001694s | byte equality observed |
| test_relaxed.test | 10 | 0.001640s | 0.001624s | 0.001567s | byte equality observed |
| test_same_type.test | 10 | 0.001721s | 0.001670s | 0.001680s | byte equality observed |
| test_stream.test | 8 | 0.002263s | 0.002318s | 0.002250s | byte equality observed |
| test_subscript.test | 10 | 0.001813s | 0.001733s | 0.001743s | byte equality observed |
| test_tuple.test | 10 | 0.001767s | 0.001770s | 0.001732s | byte equality observed |
| test_typeid_of.test | 10 | 0.001769s | 0.001768s | 0.001776s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
