# libs/function/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| function_test.test | RUN | 0.163537 | 0.668865 | 0.549256 | 0.001272 | 0.001530 | 0.001336 |
| function_test_no_rtti.test | RUN | 0.161928 | 0.659928 | 0.558337 | 0.001469 | 0.001694 | 0.001414 |
| function_n_test.test | RUN | 0.158237 | 0.578024 | 0.512292 | 0.001086 | 0.001671 | 0.001510 |
| allocator_test.test | RUN | 0.143112 | 0.474686 | 0.459189 | 0.001443 | 0.001669 | 0.001506 |
| stateless_test.test | RUN | 0.127695 | 0.268532 | 0.318336 | 0.001593 | 0.001580 | 0.001472 |
| lambda_test.test | RUN | 0.265021 | 0.591534 | 0.610339 | 0.001554 | 0.001703 | 0.001420 |
| function_test_fail1.test | COMPILE_FAIL | 0.099562 | 0.185301 | 0.218666 | — | — | — |
| function_test_fail2.test | COMPILE_FAIL | 0.099761 | 0.185842 | 0.222002 | — | — | — |
| function_30.test | COMPILE | 0.104707 | 0.200365 | 0.233071 | — | — | — |
| function_30_repeat.test | COMPILE | 0.104672 | 0.208785 | 0.222931 | — | — | — |
| function_arith_cxx98.test | RUN | 0.128882 | 0.294723 | 0.310065 | 0.001597 | 0.001688 | 0.001468 |
| function_arith_portable.test | RUN | 0.128839 | 0.291480 | 0.308182 | 0.001277 | 0.001492 | 0.001285 |
| sum_avg_cxx98.test | RUN | 0.126194 | 0.261141 | 0.303480 | 0.001472 | 0.001509 | 0.001433 |
| sum_avg_portable.test | RUN | 0.126604 | 0.262850 | 0.284923 | 0.001517 | 0.001744 | 0.001440 |
| mem_fun_cxx98.test | RUN | 0.135233 | 0.345193 | 0.338426 | 0.001540 | 0.001817 | 0.001393 |
| mem_fun_portable.test | RUN | 0.137957 | 0.328041 | 0.350907 | 0.001257 | 0.001347 | 0.001347 |
| std_bind_cxx98.test | RUN | 0.132377 | 0.309883 | 0.322207 | 0.001500 | 0.001705 | 0.001505 |
| std_bind_portable.test | RUN | 0.139181 | 0.305466 | 0.303707 | 0.001629 | 0.001496 | 0.001414 |
| function_ref_cxx98.test | RUN | 0.131088 | 0.267311 | 0.302904 | 0.001521 | 0.001709 | 0.001427 |
| function_ref_portable.test | RUN | 0.133245 | 0.255091 | 0.308393 | 0.001432 | 0.001469 | 0.001530 |
| contains_test.test | RUN | 0.142407 | 0.366004 | 0.373347 | 0.001273 | 0.001508 | 0.001505 |
| contains2_test.test | RUN | 0.134890 | 0.329724 | 0.325997 | 0.001562 | 0.001555 | 0.001441 |
| nothrow_swap.test | RUN | 0.130221 | 0.312997 | 0.338872 | 0.001405 | 0.001794 | 0.001476 |
| rvalues_test.test | RUN | 0.146923 | 0.377186 | 0.387249 | 0.001388 | 0.001773 | 0.001448 |
| function_typeof_test.test | COMPILE | 0.107928 | 0.192701 | 0.228233 | — | — | — |
| result_arg_types_test.test | RUN | 0.139918 | 0.341532 | 0.362358 | 0.001377 | 0.001740 | 0.001335 |
| result_arg_n_types_test.test | RUN | 0.143858 | 0.392047 | 0.393657 | 0.001355 | 0.001633 | 0.001376 |
| test_bad_function_call_shared.test | RUN | 0.127990 | 0.254767 | 0.304904 | 0.001544 | 0.001962 | 0.001734 |
| test_bad_function_call_static.test | RUN | 0.129505 | 0.260755 | 0.316251 | 0.001416 | 0.001667 | 0.001552 |
| mixed_cxxstd_shared.test | RUN | 0.133219 | 0.298426 | 0.319612 | 0.001482 | 0.001589 | 0.001389 |
| mixed_cxxstd_static.test | RUN | 0.134538 | 0.296147 | 0.324127 | 0.001345 | 0.001648 | 0.001409 |
| mixed_cxxstd_shared_0x.test | RUN | 0.130310 | 0.296361 | 0.337955 | 0.001638 | 0.001799 | 0.001652 |
| mixed_cxxstd_static_0x.test | RUN | 0.131575 | 0.294590 | 0.322227 | 0.001323 | 0.001592 | 0.001354 |
| mixed_cxxstd_shared_14.test | RUN | 0.132002 | 0.298970 | 0.337931 | 0.001532 | 0.001564 | 0.001503 |
| mixed_cxxstd_static_14.test | RUN | 0.134911 | 0.300800 | 0.324914 | 0.001540 | 0.001772 | 0.001468 |
| return_function_shared.test | RUN | 0.130588 | 0.283887 | 0.315323 | 0.001328 | 0.001694 | 0.001506 |
| return_function_static.test | RUN | 0.130871 | 0.287713 | 0.318641 | 0.001454 | 0.001425 | 0.001221 |
| return_function_shared_0x.test | RUN | 0.129468 | 0.280037 | 0.332431 | 0.001589 | 0.001388 | 0.001592 |
| return_function_static_0x.test | RUN | 0.130658 | 0.283840 | 0.329241 | 0.001994 | 0.001698 | 0.001457 |
| return_function_shared_14.test | RUN | 0.131697 | 0.285538 | 0.310702 | 0.001338 | 0.001733 | 0.001714 |
| return_function_static_14.test | RUN | 0.139340 | 0.279817 | 0.306666 | 0.001582 | 0.001512 | 0.001419 |
| quick.test | RUN | 0.141178 | 0.287480 | 0.319880 | 0.001465 | 0.001620 | 0.001511 |
| issue_42.test | COMPILE | 0.113684 | 0.212456 | 0.233243 | — | — | — |
| fn_eq_bind_test.test | RUN | 0.163969 | 0.480606 | 0.469716 | 0.001349 | 0.001439 | 0.001430 |
| contains_test_no_rtti.test | RUN | 0.142579 | 0.371562 | 0.375741 | 0.001398 | 0.001605 | 0.001525 |
| contains2_test_no_rtti.test | RUN | 0.138848 | 0.328008 | 0.355011 | 0.001143 | 0.001537 | 0.001394 |
| contains3_test.test | RUN | 0.139014 | 0.276740 | 0.326950 | 0.001423 | 0.001453 | 0.001345 |
| contains3_test_no_rtti.test | RUN | 0.105670 | 0.194573 | 0.216970 | 0.001140 | 0.001186 | 0.001046 |
| issue_53.test | COMPILE | 0.087392 | 0.174025 | 0.174839 | — | — | — |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872153) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872153) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872153).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
