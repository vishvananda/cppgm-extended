# libs/function/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| function_test.test | RUN | 0.371121 | 1.961321 | 1.654243 | 0.001498 | 0.001535 | 0.001682 |
| function_test_no_rtti.test | RUN | 0.370519 | 1.936340 | 1.653853 | 0.001574 | 0.001656 | 0.001801 |
| function_n_test.test | RUN | 0.348438 | 1.591523 | 1.398023 | 0.001623 | 0.001727 | 0.001716 |
| allocator_test.test | RUN | 0.274468 | 0.907314 | 0.924589 | 0.001731 | 0.001724 | 0.001631 |
| stateless_test.test | RUN | 0.218061 | 0.494236 | 0.612816 | 0.001644 | 0.001560 | 0.001694 |
| lambda_test.test | RUN | 0.457000 | 1.199439 | 1.327593 | 0.001463 | 0.001568 | 0.001740 |
| function_test_fail1.test | COMPILE_FAIL | 0.169792 | 0.338332 | 0.412981 | — | — | — |
| function_test_fail2.test | COMPILE_FAIL | 0.171191 | 0.335451 | 0.424369 | — | — | — |
| function_30.test | COMPILE | 0.168780 | 0.345097 | 0.429100 | — | — | — |
| function_30_repeat.test | COMPILE | 0.170348 | 0.352391 | 0.442030 | — | — | — |
| function_arith_cxx98.test | RUN | 0.216947 | 0.584470 | 0.655127 | 0.001543 | 0.001621 | 0.001580 |
| function_arith_portable.test | RUN | 0.219988 | 0.550850 | 0.624201 | 0.001622 | 0.001636 | 0.001592 |
| sum_avg_cxx98.test | RUN | 0.210443 | 0.478274 | 0.586119 | 0.001742 | 0.001366 | 0.001585 |
| sum_avg_portable.test | RUN | 0.202946 | 0.442672 | 0.565624 | 0.001610 | 0.001732 | 0.001622 |
| mem_fun_cxx98.test | RUN | 0.234929 | 0.684312 | 0.732633 | 0.001669 | 0.001630 | 0.001776 |
| mem_fun_portable.test | RUN | 0.232755 | 0.624942 | 0.700304 | 0.001108 | 0.001572 | 0.001557 |
| std_bind_cxx98.test | RUN | 0.220429 | 0.568834 | 0.638559 | 0.001635 | 0.001710 | 0.001620 |
| std_bind_portable.test | RUN | 0.223165 | 0.529313 | 0.608475 | 0.001591 | 0.001715 | 0.001578 |
| function_ref_cxx98.test | RUN | 0.209763 | 0.481041 | 0.588324 | 0.001575 | 0.001463 | 0.001358 |
| function_ref_portable.test | RUN | 0.210428 | 0.442698 | 0.568324 | 0.001669 | 0.001538 | 0.001385 |
| contains_test.test | RUN | 0.257220 | 0.912122 | 0.899015 | 0.001591 | 0.001765 | 0.001587 |
| contains2_test.test | RUN | 0.233369 | 0.647234 | 0.714143 | 0.001660 | 0.001699 | 0.001512 |
| nothrow_swap.test | RUN | 0.226955 | 0.617974 | 0.685597 | 0.001491 | 0.001732 | 0.001665 |
| rvalues_test.test | RUN | 0.261648 | 0.754532 | 0.793960 | 0.001507 | 0.001729 | 0.001631 |
| function_typeof_test.test | COMPILE | 0.172614 | 0.343149 | 0.425377 | — | — | — |
| result_arg_types_test.test | RUN | 0.237258 | 0.525249 | 0.715956 | 0.001544 | 0.001760 | 0.001665 |
| result_arg_n_types_test.test | RUN | 0.252980 | 0.581105 | 0.833971 | 0.001595 | 0.001741 | 0.001600 |
| test_bad_function_call_shared.test | RUN | 0.209575 | 0.477662 | 0.599995 | 0.001826 | 0.001802 | 0.001831 |
| test_bad_function_call_static.test | RUN | 0.211294 | 0.481676 | 0.607077 | 0.001482 | 0.001503 | 0.001719 |
| mixed_cxxstd_shared.test | RUN | 0.224826 | 0.567332 | 0.657114 | 0.001825 | 0.001556 | 0.001801 |
| mixed_cxxstd_static.test | RUN | 0.227954 | 0.570614 | 0.683966 | 0.001668 | 0.001755 | 0.001468 |
| mixed_cxxstd_shared_0x.test | RUN | 0.228701 | 0.571667 | 0.677268 | 0.001631 | 0.001870 | 0.001775 |
| mixed_cxxstd_static_0x.test | RUN | 0.223275 | 0.575723 | 0.670399 | 0.001548 | 0.001574 | 0.001652 |
| mixed_cxxstd_shared_14.test | RUN | 0.226681 | 0.580311 | 0.658469 | 0.001799 | 0.001823 | 0.001677 |
| mixed_cxxstd_static_14.test | RUN | 0.224703 | 0.581210 | 0.671456 | 0.001466 | 0.001732 | 0.001617 |
| return_function_shared.test | RUN | 0.223240 | 0.559788 | 0.638088 | 0.001744 | 0.001891 | 0.001764 |
| return_function_static.test | RUN | 0.224064 | 0.583940 | 0.660018 | 0.001128 | 0.001686 | 0.001716 |
| return_function_shared_0x.test | RUN | 0.224663 | 0.584017 | 0.657542 | 0.001690 | 0.001845 | 0.001785 |
| return_function_static_0x.test | RUN | 0.220905 | 0.576215 | 0.636630 | 0.001944 | 0.001716 | 0.001714 |
| return_function_shared_14.test | RUN | 0.215509 | 0.579873 | 0.658300 | 0.001607 | 0.001803 | 0.001791 |
| return_function_static_14.test | RUN | 0.221224 | 0.573717 | 0.639947 | 0.001500 | 0.001739 | 0.001531 |
| quick.test | RUN | 0.225959 | 0.577717 | 0.644798 | 0.001667 | 0.001435 | 0.001623 |
| issue_42.test | COMPILE | 0.177331 | 0.370109 | 0.449651 | — | — | — |
| fn_eq_bind_test.test | RUN | 0.288564 | 0.796842 | 0.943264 | 0.001584 | 0.001766 | 0.001627 |
| contains_test_no_rtti.test | RUN | 0.258211 | 0.902047 | 0.909292 | 0.001521 | 0.001746 | 0.001654 |
| contains2_test_no_rtti.test | RUN | 0.232800 | 0.660321 | 0.712531 | 0.001435 | 0.001544 | 0.001480 |
| contains3_test.test | RUN | 0.221592 | 0.517664 | 0.635899 | 0.001487 | 0.001564 | 0.001622 |
| contains3_test_no_rtti.test | RUN | 0.140101 | 0.416279 | 0.437738 | 0.001209 | 0.001203 | 0.001173 |
| issue_53.test | COMPILE | 0.117882 | 0.286218 | 0.308863 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.988× | 0.991×, 0.985× | 1.011× |
| clang | 0.994× | 0.985×, 1.002× | 1.013× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| allocator_test.test | 10 | 0.000951s | 0.000964s | 0.000950s | byte equality observed |
| contains2_test.test | 10 | 0.000943s | 0.000962s | 0.000921s | byte equality observed |
| contains2_test_no_rtti.test | 10 | 0.000936s | 0.001017s | 0.000919s | byte equality observed |
| contains3_test.test | 10 | 0.000932s | 0.000998s | 0.000929s | byte equality observed |
| contains3_test_no_rtti.test | 10 | 0.000938s | 0.001027s | 0.000925s | byte equality observed |
| contains_test.test | 10 | 0.000991s | 0.000988s | 0.000931s | byte equality observed |
| contains_test_no_rtti.test | 10 | 0.000999s | 0.001011s | 0.000942s | byte equality observed |
| fn_eq_bind_test.test | 10 | 0.001006s | 0.001009s | 0.000979s | byte equality observed |
| function_arith_cxx98.test | 10 | 0.001049s | 0.001024s | 0.001027s | byte equality observed |
| function_arith_portable.test | 10 | 0.001066s | 0.001078s | 0.001014s | byte equality observed |
| function_n_test.test | 10 | 0.001016s | 0.000997s | 0.001013s | byte equality observed |
| function_ref_cxx98.test | 10 | 0.001029s | 0.001011s | 0.000996s | byte equality observed |
| function_ref_portable.test | 10 | 0.001005s | 0.000949s | 0.000995s | byte equality observed |
| function_test.test | 10 | 0.001043s | 0.001052s | 0.001034s | byte equality observed |
| function_test_no_rtti.test | 10 | 0.001104s | 0.001062s | 0.001104s | byte equality observed |
| lambda_test.test | 10 | 0.001011s | 0.001030s | 0.000999s | byte equality observed |
| mem_fun_cxx98.test | 10 | 0.001022s | 0.001033s | 0.001010s | byte equality observed |
| mem_fun_portable.test | 10 | 0.001014s | 0.000997s | 0.001013s | byte equality observed |
| mixed_cxxstd_shared.test | 10 | 0.001058s | 0.001070s | 0.001086s | byte equality observed |
| mixed_cxxstd_shared_0x.test | 10 | 0.001082s | 0.001066s | 0.001051s | byte equality observed |
| mixed_cxxstd_shared_14.test | 10 | 0.001064s | 0.001055s | 0.001098s | byte equality observed |
| mixed_cxxstd_static.test | 10 | 0.001021s | 0.001063s | 0.000971s | byte equality observed |
| mixed_cxxstd_static_0x.test | 9 | 0.001007s | 0.000978s | 0.000989s | byte equality observed |
| mixed_cxxstd_static_14.test | 10 | 0.001003s | 0.001007s | 0.001006s | byte equality observed |
| nothrow_swap.test | 10 | 0.001002s | 0.000976s | 0.000984s | byte equality observed |
| quick.test | 10 | 0.001019s | 0.000978s | 0.000986s | byte equality observed |
| result_arg_n_types_test.test | 10 | 0.001000s | 0.000970s | 0.000986s | byte equality observed |
| result_arg_types_test.test | 10 | 0.001016s | 0.001002s | 0.000989s | byte equality observed |
| return_function_shared.test | 10 | 0.001093s | 0.001055s | 0.001071s | byte equality observed |
| return_function_shared_0x.test | 10 | 0.001046s | 0.001064s | 0.001098s | byte equality observed |
| return_function_shared_14.test | 10 | 0.001059s | 0.001053s | 0.001058s | byte equality observed |
| return_function_static.test | 10 | 0.001025s | 0.000970s | 0.001045s | byte equality observed |
| return_function_static_0x.test | 10 | 0.000998s | 0.001000s | 0.000980s | byte equality observed |
| return_function_static_14.test | 10 | 0.001007s | 0.001005s | 0.000983s | byte equality observed |
| rvalues_test.test | 10 | 0.001036s | 0.000987s | 0.001010s | byte equality observed |
| stateless_test.test | 10 | 0.000993s | 0.000976s | 0.000969s | byte equality observed |
| std_bind_cxx98.test | 10 | 0.000990s | 0.001006s | 0.001030s | byte equality observed |
| std_bind_portable.test | 10 | 0.001001s | 0.000984s | 0.000946s | byte equality observed |
| sum_avg_cxx98.test | 10 | 0.000997s | 0.001007s | 0.000959s | byte equality observed |
| sum_avg_portable.test | 10 | 0.000966s | 0.000946s | 0.000983s | byte equality observed |
| test_bad_function_call_shared.test | 10 | 0.001085s | 0.001088s | 0.001091s | byte equality observed |
| test_bad_function_call_static.test | 10 | 0.001026s | 0.001028s | 0.001002s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
