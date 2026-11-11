# libs/function/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| function_test.test | RUN | 0.221141 | 1.087662 | 0.916861 | 0.001421 | 0.001324 | 0.001291 |
| function_test_no_rtti.test | RUN | 0.218524 | 1.092134 | 0.914123 | 0.001198 | 0.001559 | 0.001217 |
| function_n_test.test | RUN | 0.203099 | 0.890498 | 0.736704 | 0.001440 | 0.001539 | 0.001469 |
| allocator_test.test | RUN | 0.164538 | 0.510459 | 0.498172 | 0.001661 | 0.001592 | 0.001508 |
| stateless_test.test | RUN | 0.128574 | 0.278296 | 0.322624 | 0.001629 | 0.001460 | 0.001426 |
| lambda_test.test | RUN | 0.280216 | 0.672381 | 0.686105 | 0.001590 | 0.001472 | 0.001377 |
| function_test_fail1.test | COMPILE_FAIL | 0.098048 | 0.180722 | 0.213339 | — | — | — |
| function_test_fail2.test | COMPILE_FAIL | 0.098509 | 0.185654 | 0.215566 | — | — | — |
| function_30.test | COMPILE | 0.102245 | 0.195137 | 0.229984 | — | — | — |
| function_30_repeat.test | COMPILE | 0.101755 | 0.197374 | 0.231589 | — | — | — |
| function_arith_cxx98.test | RUN | 0.130578 | 0.341604 | 0.347294 | 0.001536 | 0.001402 | 0.001506 |
| function_arith_portable.test | RUN | 0.131561 | 0.308769 | 0.323292 | 0.001359 | 0.001375 | 0.001643 |
| sum_avg_cxx98.test | RUN | 0.129218 | 0.282126 | 0.309563 | 0.001158 | 0.001513 | 0.001523 |
| sum_avg_portable.test | RUN | 0.127447 | 0.248586 | 0.300964 | 0.001501 | 0.001572 | 0.001512 |
| mem_fun_cxx98.test | RUN | 0.143464 | 0.386706 | 0.396394 | 0.001610 | 0.002904 | 0.001416 |
| mem_fun_portable.test | RUN | 0.146424 | 0.354057 | 0.374702 | 0.001251 | 0.001432 | 0.001350 |
| std_bind_cxx98.test | RUN | 0.136200 | 0.331553 | 0.347459 | 0.001645 | 0.001443 | 0.001417 |
| std_bind_portable.test | RUN | 0.139841 | 0.352111 | 0.316100 | 0.001646 | 0.001600 | 0.001464 |
| function_ref_cxx98.test | RUN | 0.131510 | 0.271431 | 0.302984 | 0.001526 | 0.001196 | 0.001322 |
| function_ref_portable.test | RUN | 0.127371 | 0.245857 | 0.300501 | 0.001507 | 0.001574 | 0.001369 |
| contains_test.test | RUN | 0.151185 | 0.510930 | 0.494078 | 0.001358 | 0.001598 | 0.001300 |
| contains2_test.test | RUN | 0.144432 | 0.368600 | 0.382884 | 0.001450 | 0.001520 | 0.001437 |
| nothrow_swap.test | RUN | 0.137802 | 0.350321 | 0.358621 | 0.001375 | 0.001610 | 0.001590 |
| rvalues_test.test | RUN | 0.155597 | 0.430774 | 0.427366 | 0.001424 | 0.001472 | 0.001371 |
| function_typeof_test.test | COMPILE | 0.103402 | 0.194631 | 0.216960 | — | — | — |
| result_arg_types_test.test | RUN | 0.143090 | 0.292303 | 0.377210 | 0.001263 | 0.001445 | 0.001357 |
| result_arg_n_types_test.test | RUN | 0.153598 | 0.319610 | 0.442069 | 0.001229 | 0.001548 | 0.001479 |
| test_bad_function_call_shared.test | RUN | 0.127622 | 0.268524 | 0.312892 | 0.001443 | 0.001458 | 0.001692 |
| test_bad_function_call_static.test | RUN | 0.129020 | 0.267094 | 0.327777 | 0.001493 | 0.001544 | 0.001481 |
| mixed_cxxstd_shared.test | RUN | 0.140818 | 0.328139 | 0.350150 | 0.001512 | 0.001485 | 0.001495 |
| mixed_cxxstd_static.test | RUN | 0.139236 | 0.321133 | 0.361499 | 0.001510 | 0.001487 | 0.001519 |
| mixed_cxxstd_shared_0x.test | RUN | 0.138384 | 0.321945 | 0.346200 | 0.001638 | 0.001782 | 0.001630 |
| mixed_cxxstd_static_0x.test | RUN | 0.143729 | 0.327258 | 0.362395 | 0.001610 | 0.001434 | 0.001475 |
| mixed_cxxstd_shared_14.test | RUN | 0.141371 | 0.334321 | 0.350016 | 0.001660 | 0.001572 | 0.001453 |
| mixed_cxxstd_static_14.test | RUN | 0.139360 | 0.322720 | 0.351453 | 0.001435 | 0.001560 | 0.001472 |
| return_function_shared.test | RUN | 0.136181 | 0.317479 | 0.344181 | 0.001456 | 0.001659 | 0.001584 |
| return_function_static.test | RUN | 0.136105 | 0.321428 | 0.362609 | 0.001352 | 0.001432 | 0.001499 |
| return_function_shared_0x.test | RUN | 0.138346 | 0.332560 | 0.353145 | 0.001268 | 0.001722 | 0.001628 |
| return_function_static_0x.test | RUN | 0.135701 | 0.322478 | 0.332947 | 0.001438 | 0.001624 | 0.001518 |
| return_function_shared_14.test | RUN | 0.139442 | 0.331674 | 0.343317 | 0.001569 | 0.001541 | 0.001451 |
| return_function_static_14.test | RUN | 0.137661 | 0.329835 | 0.355453 | 0.001395 | 0.001588 | 0.001382 |
| quick.test | RUN | 0.141520 | 0.336053 | 0.328372 | 0.001326 | 0.001460 | 0.001438 |
| issue_42.test | COMPILE | 0.111351 | 0.209943 | 0.226964 | — | — | — |
| fn_eq_bind_test.test | RUN | 0.171594 | 0.440746 | 0.515439 | 0.001486 | 0.001525 | 0.001107 |
| contains_test_no_rtti.test | RUN | 0.153660 | 0.503637 | 0.499703 | 0.001345 | 0.001467 | 0.001131 |
| contains2_test_no_rtti.test | RUN | 0.141992 | 0.371118 | 0.386039 | 0.001334 | 0.001353 | 0.001195 |
| contains3_test.test | RUN | 0.134909 | 0.303671 | 0.344342 | 0.001428 | 0.001471 | 0.001279 |
| contains3_test_no_rtti.test | RUN | 0.096985 | 0.235400 | 0.219731 | 0.001151 | 0.001107 | 0.001070 |
| issue_53.test | COMPILE | 0.083070 | 0.161730 | 0.164483 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.019× | 1.027×, 1.011× | 0.961× |
| clang | 1.017×† | 1.048×, 0.987× | 0.988× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| allocator_test.test | 10 | 0.000925s | 0.000933s | 0.001005s | byte equality observed |
| contains2_test.test | 10 | 0.000949s | 0.000945s | 0.001000s | byte equality observed |
| contains2_test_no_rtti.test | 10 | 0.000955s | 0.000969s | 0.001029s | byte equality observed |
| contains3_test.test | 10 | 0.000936s | 0.000974s | 0.001006s | byte equality observed |
| contains3_test_no_rtti.test | 10 | 0.000998s | 0.000963s | 0.001028s | byte equality observed |
| contains_test.test | 10 | 0.000978s | 0.000945s | 0.001004s | byte equality observed |
| contains_test_no_rtti.test | 10 | 0.000980s | 0.000949s | 0.000997s | byte equality observed |
| fn_eq_bind_test.test | 10 | 0.000956s | 0.000954s | 0.000994s | byte equality observed |
| function_arith_cxx98.test | 10 | 0.000995s | 0.000962s | 0.001042s | byte equality observed |
| function_arith_portable.test | 10 | 0.000980s | 0.000995s | 0.001016s | byte equality observed |
| function_n_test.test | 10 | 0.000961s | 0.000973s | 0.000990s | byte equality observed |
| function_ref_cxx98.test | 10 | 0.000926s | 0.000941s | 0.000997s | byte equality observed |
| function_ref_portable.test | 10 | 0.000920s | 0.000942s | 0.000981s | byte equality observed |
| function_test.test | 10 | 0.001027s | 0.001009s | 0.001057s | byte equality observed |
| function_test_no_rtti.test | 10 | 0.001008s | 0.000992s | 0.001059s | byte equality observed |
| lambda_test.test | 10 | 0.000967s | 0.000949s | 0.001030s | byte equality observed |
| mem_fun_cxx98.test | 10 | 0.000943s | 0.000958s | 0.000989s | byte equality observed |
| mem_fun_portable.test | 10 | 0.000943s | 0.000925s | 0.001012s | byte equality observed |
| mixed_cxxstd_shared.test | 10 | 0.001035s | 0.001007s | 0.001097s | byte equality observed |
| mixed_cxxstd_shared_0x.test | 10 | 0.001001s | 0.001036s | 0.001082s | byte equality observed |
| mixed_cxxstd_shared_14.test | 10 | 0.001027s | 0.001011s | 0.001093s | byte equality observed |
| mixed_cxxstd_static.test | 10 | 0.000961s | 0.000973s | 0.001018s | byte equality observed |
| mixed_cxxstd_static_0x.test | 10 | 0.000958s | 0.000949s | 0.001010s | byte equality observed |
| mixed_cxxstd_static_14.test | 10 | 0.000968s | 0.000949s | 0.001038s | byte equality observed |
| nothrow_swap.test | 10 | 0.000944s | 0.000961s | 0.000981s | byte equality observed |
| quick.test | 10 | 0.000946s | 0.000941s | 0.000983s | byte equality observed |
| result_arg_n_types_test.test | 10 | 0.000955s | 0.000950s | 0.001000s | byte equality observed |
| result_arg_types_test.test | 10 | 0.000928s | 0.000929s | 0.000988s | byte equality observed |
| return_function_shared.test | 10 | 0.001027s | 0.001040s | 0.001096s | byte equality observed |
| return_function_shared_0x.test | 10 | 0.001006s | 0.001011s | 0.001079s | byte equality observed |
| return_function_shared_14.test | 10 | 0.001016s | 0.001015s | 0.001087s | byte equality observed |
| return_function_static.test | 10 | 0.000938s | 0.000951s | 0.001018s | byte equality observed |
| return_function_static_0x.test | 10 | 0.000954s | 0.000939s | 0.001011s | byte equality observed |
| return_function_static_14.test | 9 | 0.000944s | 0.000938s | 0.001005s | byte equality observed |
| rvalues_test.test | 10 | 0.000950s | 0.000966s | 0.000994s | byte equality observed |
| stateless_test.test | 10 | 0.000954s | 0.000934s | 0.000988s | byte equality observed |
| std_bind_cxx98.test | 10 | 0.000925s | 0.000929s | 0.000976s | byte equality observed |
| std_bind_portable.test | 10 | 0.000933s | 0.000943s | 0.001015s | byte equality observed |
| sum_avg_cxx98.test | 10 | 0.000918s | 0.000930s | 0.000976s | byte equality observed |
| sum_avg_portable.test | 10 | 0.000933s | 0.000925s | 0.000990s | byte equality observed |
| test_bad_function_call_shared.test | 10 | 0.001045s | 0.001072s | 0.001126s | byte equality observed |
| test_bad_function_call_static.test | 10 | 0.000992s | 0.000981s | 0.001035s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
