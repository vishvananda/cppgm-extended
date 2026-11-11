# libs/function/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| function_test.test | RUN | 0.169754 | 0.773040 | 0.638798 | 0.001258 | 0.001296 | 0.001600 |
| function_test_no_rtti.test | RUN | 0.172747 | 0.764751 | 0.635626 | 0.001376 | 0.001694 | 0.001545 |
| function_n_test.test | RUN | 0.165298 | 0.688104 | 0.595118 | 0.001488 | 0.001669 | 0.001462 |
| allocator_test.test | RUN | 0.154577 | 0.562471 | 0.517293 | 0.001470 | 0.001548 | 0.001924 |
| stateless_test.test | RUN | 0.131820 | 0.267257 | 0.366006 | 0.001681 | 0.001785 | 0.001844 |
| lambda_test.test | RUN | 0.281352 | 0.617464 | 0.691712 | 0.001685 | 0.001703 | 0.001762 |
| function_test_fail1.test | COMPILE_FAIL | 0.102774 | 0.186695 | 0.246412 | — | — | — |
| function_test_fail2.test | COMPILE_FAIL | 0.103791 | 0.190473 | 0.251688 | — | — | — |
| function_30.test | COMPILE | 0.113924 | 0.215633 | 0.256991 | — | — | — |
| function_30_repeat.test | COMPILE | 0.112542 | 0.204821 | 0.249825 | — | — | — |
| function_arith_cxx98.test | RUN | 0.136134 | 0.298351 | 0.369970 | 0.001796 | 0.001862 | 0.001780 |
| function_arith_portable.test | RUN | 0.137926 | 0.299426 | 0.348444 | 0.001332 | 0.001857 | 0.001728 |
| sum_avg_cxx98.test | RUN | 0.135699 | 0.268633 | 0.340250 | 0.001617 | 0.001452 | 0.001565 |
| sum_avg_portable.test | RUN | 0.139669 | 0.263907 | 0.335536 | 0.001625 | 0.001734 | 0.001663 |
| mem_fun_cxx98.test | RUN | 0.145428 | 0.345292 | 0.414828 | 0.001821 | 0.001829 | 0.001601 |
| mem_fun_portable.test | RUN | 0.147562 | 0.341536 | 0.394506 | 0.001498 | 0.001557 | 0.001701 |
| std_bind_cxx98.test | RUN | 0.140276 | 0.305431 | 0.349425 | 0.001713 | 0.001567 | 0.001778 |
| std_bind_portable.test | RUN | 0.150516 | 0.309659 | 0.356413 | 0.001697 | 0.001790 | 0.001747 |
| function_ref_cxx98.test | RUN | 0.139731 | 0.284174 | 0.352608 | 0.001688 | 0.001758 | 0.001706 |
| function_ref_portable.test | RUN | 0.138451 | 0.264717 | 0.323945 | 0.001564 | 0.001680 | 0.001696 |
| contains_test.test | RUN | 0.149777 | 0.378916 | 0.422750 | 0.001366 | 0.001755 | 0.001816 |
| contains2_test.test | RUN | 0.149430 | 0.334218 | 0.386132 | 0.001786 | 0.001537 | 0.001887 |
| nothrow_swap.test | RUN | 0.145288 | 0.304066 | 0.368381 | 0.001664 | 0.001826 | 0.001754 |
| rvalues_test.test | RUN | 0.151971 | 0.386700 | 0.437803 | 0.001513 | 0.001784 | 0.001918 |
| function_typeof_test.test | COMPILE | 0.111848 | 0.200796 | 0.255095 | — | — | — |
| result_arg_types_test.test | RUN | 0.148568 | 0.351464 | 0.429125 | 0.001517 | 0.001881 | 0.001780 |
| result_arg_n_types_test.test | RUN | 0.163869 | 0.404745 | 0.455580 | 0.001652 | 0.002242 | 0.001717 |
| test_bad_function_call_shared.test | RUN | 0.136427 | 0.303880 | 0.355495 | 0.001665 | 0.001894 | 0.001726 |
| test_bad_function_call_static.test | RUN | 0.137206 | 0.329972 | 0.335473 | 0.001413 | 0.001499 | 0.001846 |
| mixed_cxxstd_shared.test | RUN | 0.141046 | 0.291639 | 0.390162 | 0.001425 | 0.001581 | 0.001823 |
| mixed_cxxstd_static.test | RUN | 0.141651 | 0.297877 | 0.379332 | 0.001388 | 0.001782 | 0.001495 |
| mixed_cxxstd_shared_0x.test | RUN | 0.144302 | 0.300948 | 0.373402 | 0.001781 | 0.001965 | 0.002032 |
| mixed_cxxstd_static_0x.test | RUN | 0.142693 | 0.288240 | 0.369118 | 0.001709 | 0.001746 | 0.001703 |
| mixed_cxxstd_shared_14.test | RUN | 0.162325 | 0.297901 | 0.387142 | 0.001580 | 0.001636 | 0.001608 |
| mixed_cxxstd_static_14.test | RUN | 0.159904 | 0.288214 | 0.373686 | 0.001660 | 0.001719 | 0.001690 |
| return_function_shared.test | RUN | 0.137232 | 0.286047 | 0.358927 | 0.001587 | 0.001906 | 0.001843 |
| return_function_static.test | RUN | 0.134648 | 0.290567 | 0.364788 | 0.001485 | 0.001452 | 0.001394 |
| return_function_shared_0x.test | RUN | 0.143050 | 0.292107 | 0.365927 | 0.001480 | 0.001796 | 0.001728 |
| return_function_static_0x.test | RUN | 0.137492 | 0.291295 | 0.373951 | 0.001748 | 0.001444 | 0.001768 |
| return_function_shared_14.test | RUN | 0.125979 | 0.294689 | 0.370537 | 0.001654 | 0.001884 | 0.001762 |
| return_function_static_14.test | RUN | 0.134391 | 0.292939 | 0.356908 | 0.001698 | 0.001730 | 0.001742 |
| quick.test | RUN | 0.141184 | 0.299875 | 0.357981 | 0.001644 | 0.001896 | 0.001750 |
| issue_42.test | COMPILE | 0.115289 | 0.226102 | 0.265131 | — | — | — |
| fn_eq_bind_test.test | RUN | 0.172351 | 0.475484 | 0.548690 | 0.001605 | 0.001491 | 0.001475 |
| contains_test_no_rtti.test | RUN | 0.148544 | 0.383765 | 0.439511 | 0.001412 | 0.001712 | 0.001437 |
| contains2_test_no_rtti.test | RUN | 0.143418 | 0.387887 | 0.403524 | 0.001267 | 0.001645 | 0.001468 |
| contains3_test.test | RUN | 0.142525 | 0.321747 | 0.364991 | 0.001778 | 0.001854 | 0.001744 |
| contains3_test_no_rtti.test | RUN | 0.111640 | 0.222358 | 0.229008 | 0.001306 | 0.001246 | 0.001226 |
| issue_53.test | COMPILE | 0.101626 | 0.209584 | 0.196397 | — | — | — |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114108) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114108) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114108).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
