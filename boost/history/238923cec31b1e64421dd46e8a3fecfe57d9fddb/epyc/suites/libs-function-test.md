# libs/function/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| function_test.test | RUN | 0.193875 | 0.693575 | 0.618456 | 0.001587 | 0.001788 | 0.001632 |
| function_test_no_rtti.test | RUN | 0.196482 | 0.691496 | 0.616337 | 0.001572 | 0.001823 | 0.001567 |
| function_n_test.test | RUN | 0.185793 | 0.603353 | 0.546689 | 0.001634 | 0.001557 | 0.001760 |
| allocator_test.test | RUN | 0.174750 | 0.485043 | 0.491770 | 0.001622 | 0.001686 | 0.001688 |
| stateless_test.test | RUN | 0.155334 | 0.283798 | 0.340333 | 0.001914 | 0.001702 | 0.001726 |
| lambda_test.test | RUN | 0.323151 | 0.621650 | 0.665178 | 0.001878 | 0.001765 | 0.001531 |
| function_test_fail1.test | COMPILE_FAIL | 0.120323 | 0.194881 | 0.230734 | — | — | — |
| function_test_fail2.test | COMPILE_FAIL | 0.122394 | 0.192610 | 0.236467 | — | — | — |
| function_30.test | COMPILE | 0.126400 | 0.215743 | 0.245173 | — | — | — |
| function_30_repeat.test | COMPILE | 0.128208 | 0.209261 | 0.249532 | — | — | — |
| function_arith_cxx98.test | RUN | 0.158617 | 0.302361 | 0.325567 | 0.001773 | 0.001884 | 0.001657 |
| function_arith_portable.test | RUN | 0.157319 | 0.305605 | 0.346535 | 0.001738 | 0.001607 | 0.001504 |
| sum_avg_cxx98.test | RUN | 0.150048 | 0.277111 | 0.326085 | 0.001488 | 0.001578 | 0.001453 |
| sum_avg_portable.test | RUN | 0.155651 | 0.261009 | 0.307412 | 0.001808 | 0.001700 | 0.001465 |
| mem_fun_cxx98.test | RUN | 0.164642 | 0.354873 | 0.350168 | 0.001840 | 0.001924 | 0.001547 |
| mem_fun_portable.test | RUN | 0.163457 | 0.341409 | 0.371740 | 0.001448 | 0.001598 | 0.001415 |
| std_bind_cxx98.test | RUN | 0.158089 | 0.311599 | 0.342180 | 0.001434 | 0.001460 | 0.001502 |
| std_bind_portable.test | RUN | 0.159638 | 0.308788 | 0.333631 | 0.001791 | 0.001688 | 0.001624 |
| function_ref_cxx98.test | RUN | 0.149242 | 0.277230 | 0.323499 | 0.001532 | 0.001878 | 0.001511 |
| function_ref_portable.test | RUN | 0.150965 | 0.270732 | 0.306490 | 0.001304 | 0.001792 | 0.001540 |
| contains_test.test | RUN | 0.160019 | 0.387907 | 0.392840 | 0.001548 | 0.001736 | 0.001369 |
| contains2_test.test | RUN | 0.161842 | 0.344842 | 0.349450 | 0.001960 | 0.001657 | 0.001552 |
| nothrow_swap.test | RUN | 0.156788 | 0.310563 | 0.353327 | 0.001910 | 0.001832 | 0.001568 |
| rvalues_test.test | RUN | 0.173864 | 0.399819 | 0.395250 | 0.001706 | 0.001425 | 0.001589 |
| function_typeof_test.test | COMPILE | 0.121505 | 0.204301 | 0.238413 | — | — | — |
| result_arg_types_test.test | RUN | 0.174516 | 0.346188 | 0.369362 | 0.001603 | 0.001898 | 0.001420 |
| result_arg_n_types_test.test | RUN | 0.179384 | 0.410977 | 0.422841 | 0.001502 | 0.001747 | 0.001374 |
| test_bad_function_call_shared.test | RUN | 0.152890 | 0.264296 | 0.320928 | 0.001765 | 0.002034 | 0.001748 |
| test_bad_function_call_static.test | RUN | 0.150451 | 0.301543 | 0.309515 | 0.001808 | 0.001510 | 0.001224 |
| mixed_cxxstd_shared.test | RUN | 0.158444 | 0.332785 | 0.348016 | 0.001672 | 0.001700 | 0.001727 |
| mixed_cxxstd_static.test | RUN | 0.153025 | 0.306934 | 0.326994 | 0.001299 | 0.001596 | 0.001578 |
| mixed_cxxstd_shared_0x.test | RUN | 0.154690 | 0.310651 | 0.343933 | 0.001768 | 0.002059 | 0.001759 |
| mixed_cxxstd_static_0x.test | RUN | 0.153491 | 0.306635 | 0.326788 | 0.001843 | 0.001725 | 0.001499 |
| mixed_cxxstd_shared_14.test | RUN | 0.155837 | 0.315226 | 0.331729 | 0.001641 | 0.001763 | 0.001786 |
| mixed_cxxstd_static_14.test | RUN | 0.158268 | 0.306831 | 0.346622 | 0.001736 | 0.001808 | 0.001551 |
| return_function_shared.test | RUN | 0.147447 | 0.296771 | 0.349122 | 0.001798 | 0.001629 | 0.001933 |
| return_function_static.test | RUN | 0.156708 | 0.295570 | 0.333835 | 0.001516 | 0.001715 | 0.001144 |
| return_function_shared_0x.test | RUN | 0.151867 | 0.306465 | 0.335442 | 0.001910 | 0.001792 | 0.001826 |
| return_function_static_0x.test | RUN | 0.147863 | 0.292235 | 0.326062 | 0.001796 | 0.001886 | 0.001628 |
| return_function_shared_14.test | RUN | 0.148214 | 0.303104 | 0.344896 | 0.001599 | 0.002056 | 0.001487 |
| return_function_static_14.test | RUN | 0.153046 | 0.294280 | 0.348639 | 0.001766 | 0.001886 | 0.001564 |
| quick.test | RUN | 0.158087 | 0.314791 | 0.331764 | 0.001826 | 0.001510 | 0.001530 |
| issue_42.test | COMPILE | 0.125997 | 0.232289 | 0.247243 | — | — | — |
| fn_eq_bind_test.test | RUN | 0.192749 | 0.511103 | 0.478997 | 0.001474 | 0.001798 | 0.001727 |
| contains_test_no_rtti.test | RUN | 0.160849 | 0.400729 | 0.409812 | 0.001576 | 0.001620 | 0.001600 |
| contains2_test_no_rtti.test | RUN | 0.157413 | 0.344948 | 0.370421 | 0.001847 | 0.001480 | 0.001467 |
| contains3_test.test | RUN | 0.155201 | 0.290278 | 0.344023 | 0.001474 | 0.001429 | 0.001381 |
| contains3_test_no_rtti.test | RUN | 0.126273 | 0.212029 | 0.246869 | 0.001439 | 0.001283 | 0.001140 |
| issue_53.test | COMPILE | 0.117479 | 0.185860 | 0.182896 | — | — | — |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42562778965) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42562778965) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42562778965).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
