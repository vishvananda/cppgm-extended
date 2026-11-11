# libs/function/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| function_test.test | RUN | 0.228128 | 0.839954 | 0.919230 | 0.001762 | 0.001882 | 0.001802 |
| function_test_no_rtti.test | RUN | 0.229021 | 0.845325 | 0.926447 | 0.001668 | 0.001564 | 0.001654 |
| function_n_test.test | RUN | 0.213139 | 0.728920 | 0.818246 | 0.001694 | 0.001788 | 0.001760 |
| allocator_test.test | RUN | 0.203355 | 0.581590 | 0.734585 | 0.001710 | 0.001716 | 0.001762 |
| stateless_test.test | RUN | 0.185600 | 0.344524 | 0.526685 | 0.001832 | 0.001874 | 0.001846 |
| lambda_test.test | RUN | 0.372705 | 0.742941 | 1.019739 | 0.001782 | 0.001852 | 0.001729 |
| function_test_fail1.test | COMPILE_FAIL | 0.136355 | 0.237138 | 0.376138 | — | — | — |
| function_test_fail2.test | COMPILE_FAIL | 0.145765 | 0.237119 | 0.381683 | — | — | — |
| function_30.test | COMPILE | 0.147412 | 0.264908 | 0.379338 | — | — | — |
| function_30_repeat.test | COMPILE | 0.151366 | 0.257828 | 0.368091 | — | — | — |
| function_arith_cxx98.test | RUN | 0.187445 | 0.363965 | 0.519683 | 0.001810 | 0.001748 | 0.001818 |
| function_arith_portable.test | RUN | 0.185525 | 0.361558 | 0.469238 | 0.001892 | 0.001644 | 0.001799 |
| sum_avg_cxx98.test | RUN | 0.185031 | 0.335774 | 0.461511 | 0.001660 | 0.001660 | 0.001336 |
| sum_avg_portable.test | RUN | 0.189028 | 0.316047 | 0.470272 | 0.001762 | 0.001341 | 0.001728 |
| mem_fun_cxx98.test | RUN | 0.194960 | 0.430829 | 0.535239 | 0.001786 | 0.002018 | 0.001955 |
| mem_fun_portable.test | RUN | 0.190125 | 0.415003 | 0.579227 | 0.001706 | 0.001612 | 0.001578 |
| std_bind_cxx98.test | RUN | 0.191177 | 0.385845 | 0.529936 | 0.001553 | 0.001673 | 0.001700 |
| std_bind_portable.test | RUN | 0.190516 | 0.380214 | 0.511344 | 0.001826 | 0.001237 | 0.001784 |
| function_ref_cxx98.test | RUN | 0.176805 | 0.334641 | 0.486466 | 0.001810 | 0.001648 | 0.001894 |
| function_ref_portable.test | RUN | 0.179853 | 0.335101 | 0.508136 | 0.001866 | 0.001722 | 0.001754 |
| contains_test.test | RUN | 0.190834 | 0.464658 | 0.616284 | 0.001530 | 0.001624 | 0.001956 |
| contains2_test.test | RUN | 0.190708 | 0.408001 | 0.550915 | 0.001901 | 0.002004 | 0.001736 |
| nothrow_swap.test | RUN | 0.203450 | 0.385971 | 0.532398 | 0.001609 | 0.001734 | 0.001927 |
| rvalues_test.test | RUN | 0.229797 | 0.480784 | 0.629608 | 0.001828 | 0.001801 | 0.002050 |
| function_typeof_test.test | COMPILE | 0.158352 | 0.243440 | 0.376682 | — | — | — |
| result_arg_types_test.test | RUN | 0.201418 | 0.409214 | 0.548919 | 0.001769 | 0.001621 | 0.001845 |
| result_arg_n_types_test.test | RUN | 0.202812 | 0.500114 | 0.630824 | 0.001643 | 0.001749 | 0.001843 |
| test_bad_function_call_shared.test | RUN | 0.180111 | 0.326517 | 0.489885 | 0.001894 | 0.001890 | 0.002072 |
| test_bad_function_call_static.test | RUN | 0.181113 | 0.329659 | 0.475583 | 0.001611 | 0.001591 | 0.001660 |
| mixed_cxxstd_shared.test | RUN | 0.181968 | 0.366909 | 0.525477 | 0.001725 | 0.001487 | 0.001642 |
| mixed_cxxstd_static.test | RUN | 0.182686 | 0.371072 | 0.525660 | 0.001706 | 0.001465 | 0.001916 |
| mixed_cxxstd_shared_0x.test | RUN | 0.190718 | 0.345598 | 0.553001 | 0.002022 | 0.002106 | 0.002024 |
| mixed_cxxstd_static_0x.test | RUN | 0.182599 | 0.371528 | 0.509900 | 0.001872 | 0.001810 | 0.001615 |
| mixed_cxxstd_shared_14.test | RUN | 0.185424 | 0.372952 | 0.537634 | 0.001260 | 0.001812 | 0.001961 |
| mixed_cxxstd_static_14.test | RUN | 0.189743 | 0.366917 | 0.519680 | 0.001802 | 0.001857 | 0.001904 |
| return_function_shared.test | RUN | 0.180915 | 0.361279 | 0.521541 | 0.001917 | 0.001736 | 0.001954 |
| return_function_static.test | RUN | 0.183887 | 0.370013 | 0.528699 | 0.001450 | 0.001661 | 0.001740 |
| return_function_shared_0x.test | RUN | 0.183848 | 0.359931 | 0.478032 | 0.001668 | 0.001844 | 0.002008 |
| return_function_static_0x.test | RUN | 0.178413 | 0.359897 | 0.499660 | 0.001818 | 0.001792 | 0.001768 |
| return_function_shared_14.test | RUN | 0.182523 | 0.369042 | 0.495857 | 0.001975 | 0.001956 | 0.001927 |
| return_function_static_14.test | RUN | 0.184070 | 0.357461 | 0.517785 | 0.001471 | 0.001667 | 0.001556 |
| quick.test | RUN | 0.185902 | 0.373133 | 0.469511 | 0.001838 | 0.001636 | 0.001814 |
| issue_42.test | COMPILE | 0.148326 | 0.269939 | 0.385337 | — | — | — |
| fn_eq_bind_test.test | RUN | 0.222164 | 0.615121 | 0.756525 | 0.001347 | 0.001382 | 0.001421 |
| contains_test_no_rtti.test | RUN | 0.197769 | 0.478838 | 0.616826 | 0.001593 | 0.001627 | 0.001683 |
| contains2_test_no_rtti.test | RUN | 0.194341 | 0.423264 | 0.570298 | 0.001604 | 0.001436 | 0.001596 |
| contains3_test.test | RUN | 0.186452 | 0.356401 | 0.508557 | 0.001380 | 0.001502 | 0.001772 |
| contains3_test_no_rtti.test | RUN | 0.149936 | 0.279361 | 0.324664 | 0.001404 | 0.001334 | 0.001239 |
| issue_53.test | COMPILE | 0.133900 | 0.257125 | 0.263913 | — | — | — |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42570601152) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42570601152) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42570601152).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
