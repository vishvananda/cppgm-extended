# libs/function/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| function_test.test | RUN | 0.221226 | 0.835584 | 0.882295 | 0.001513 | 0.001745 | 0.001777 |
| function_test_no_rtti.test | RUN | 0.220403 | 0.837255 | 0.886600 | 0.001404 | 0.001704 | 0.001562 |
| function_n_test.test | RUN | 0.213421 | 0.741682 | 0.789032 | 0.001476 | 0.001765 | 0.001586 |
| allocator_test.test | RUN | 0.198856 | 0.597757 | 0.701397 | 0.001704 | 0.001777 | 0.001628 |
| stateless_test.test | RUN | 0.172622 | 0.336109 | 0.488535 | 0.001768 | 0.001896 | 0.001470 |
| lambda_test.test | RUN | 0.361119 | 0.748523 | 0.984342 | 0.001766 | 0.001906 | 0.001697 |
| function_test_fail1.test | COMPILE_FAIL | 0.133496 | 0.234632 | 0.362649 | — | — | — |
| function_test_fail2.test | COMPILE_FAIL | 0.135234 | 0.233482 | 0.353540 | — | — | — |
| function_30.test | COMPILE | 0.145620 | 0.258395 | 0.368343 | — | — | — |
| function_30_repeat.test | COMPILE | 0.142267 | 0.258119 | 0.367083 | — | — | — |
| function_arith_cxx98.test | RUN | 0.176138 | 0.370771 | 0.511662 | 0.001799 | 0.001766 | 0.001752 |
| function_arith_portable.test | RUN | 0.173104 | 0.359774 | 0.450047 | 0.001609 | 0.001694 | 0.001752 |
| sum_avg_cxx98.test | RUN | 0.166109 | 0.328092 | 0.471397 | 0.001381 | 0.001808 | 0.001464 |
| sum_avg_portable.test | RUN | 0.174906 | 0.321293 | 0.457404 | 0.001674 | 0.001729 | 0.001777 |
| mem_fun_cxx98.test | RUN | 0.188092 | 0.417371 | 0.539977 | 0.001686 | 0.001924 | 0.001748 |
| mem_fun_portable.test | RUN | 0.182942 | 0.399789 | 0.532314 | 0.001579 | 0.001819 | 0.001637 |
| std_bind_cxx98.test | RUN | 0.179563 | 0.370532 | 0.483248 | 0.001208 | 0.001478 | 0.001685 |
| std_bind_portable.test | RUN | 0.180428 | 0.368065 | 0.488669 | 0.001731 | 0.001796 | 0.001671 |
| function_ref_cxx98.test | RUN | 0.162203 | 0.327290 | 0.457413 | 0.001673 | 0.001863 | 0.001675 |
| function_ref_portable.test | RUN | 0.168988 | 0.321633 | 0.457615 | 0.001782 | 0.001732 | 0.001684 |
| contains_test.test | RUN | 0.183489 | 0.469199 | 0.587572 | 0.001364 | 0.001762 | 0.001730 |
| contains2_test.test | RUN | 0.183831 | 0.408074 | 0.519569 | 0.001801 | 0.001719 | 0.001504 |
| nothrow_swap.test | RUN | 0.176073 | 0.368755 | 0.495171 | 0.001774 | 0.001734 | 0.001762 |
| rvalues_test.test | RUN | 0.197658 | 0.476194 | 0.608067 | 0.001570 | 0.001890 | 0.001852 |
| function_typeof_test.test | COMPILE | 0.143455 | 0.248243 | 0.379858 | — | — | — |
| result_arg_types_test.test | RUN | 0.194132 | 0.425351 | 0.572751 | 0.001665 | 0.001758 | 0.001764 |
| result_arg_n_types_test.test | RUN | 0.189675 | 0.504140 | 0.626149 | 0.001597 | 0.001891 | 0.001772 |
| test_bad_function_call_shared.test | RUN | 0.175735 | 0.319446 | 0.482572 | 0.001710 | 0.002109 | 0.001574 |
| test_bad_function_call_static.test | RUN | 0.174674 | 0.328267 | 0.469260 | 0.001802 | 0.001690 | 0.001617 |
| mixed_cxxstd_shared.test | RUN | 0.179689 | 0.366642 | 0.536332 | 0.001731 | 0.001753 | 0.001716 |
| mixed_cxxstd_static.test | RUN | 0.182357 | 0.366372 | 0.504670 | 0.001559 | 0.001737 | 0.001552 |
| mixed_cxxstd_shared_0x.test | RUN | 0.182384 | 0.375314 | 0.536179 | 0.001936 | 0.001938 | 0.002055 |
| mixed_cxxstd_static_0x.test | RUN | 0.181306 | 0.368690 | 0.531518 | 0.001784 | 0.001616 | 0.001655 |
| mixed_cxxstd_shared_14.test | RUN | 0.182729 | 0.375841 | 0.498777 | 0.001450 | 0.001886 | 0.001898 |
| mixed_cxxstd_static_14.test | RUN | 0.180473 | 0.373259 | 0.533898 | 0.001725 | 0.001674 | 0.001834 |
| return_function_shared.test | RUN | 0.172978 | 0.355338 | 0.518663 | 0.001866 | 0.001932 | 0.001694 |
| return_function_static.test | RUN | 0.182272 | 0.363360 | 0.516399 | 0.001698 | 0.001594 | 0.001834 |
| return_function_shared_0x.test | RUN | 0.181234 | 0.359472 | 0.531362 | 0.001546 | 0.002026 | 0.001910 |
| return_function_static_0x.test | RUN | 0.175141 | 0.353433 | 0.522396 | 0.001704 | 0.001854 | 0.001388 |
| return_function_shared_14.test | RUN | 0.176961 | 0.362948 | 0.514619 | 0.001724 | 0.001858 | 0.001826 |
| return_function_static_14.test | RUN | 0.179138 | 0.361413 | 0.508524 | 0.001656 | 0.001781 | 0.001625 |
| quick.test | RUN | 0.184149 | 0.362207 | 0.517074 | 0.001479 | 0.001720 | 0.001824 |
| issue_42.test | COMPILE | 0.143159 | 0.273541 | 0.378538 | — | — | — |
| fn_eq_bind_test.test | RUN | 0.221355 | 0.616281 | 0.760123 | 0.001504 | 0.001610 | 0.001259 |
| contains_test_no_rtti.test | RUN | 0.189150 | 0.481611 | 0.601474 | 0.001646 | 0.001706 | 0.001303 |
| contains2_test_no_rtti.test | RUN | 0.182098 | 0.420966 | 0.557525 | 0.001594 | 0.001424 | 0.001813 |
| contains3_test.test | RUN | 0.179658 | 0.361314 | 0.510996 | 0.001427 | 0.001722 | 0.001327 |
| contains3_test_no_rtti.test | RUN | 0.126594 | 0.241231 | 0.306772 | 0.001285 | 0.001388 | 0.001243 |
| issue_53.test | COMPILE | 0.112051 | 0.222434 | 0.276318 | — | — | — |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497688) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497688) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497688).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
