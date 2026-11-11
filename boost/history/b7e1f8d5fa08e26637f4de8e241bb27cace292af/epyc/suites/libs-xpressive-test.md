# libs/xpressive/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| regress.test | RUN | 1.539577 | 9.128594 | 7.244866 | 0.048510 | 0.054796 | 0.055563 |
| regress_u.test | RUN | 1.631235 | 9.279098 | 7.521116 | 0.049520 | 0.055779 | 0.056641 |
| c_traits.test | RUN | 1.545471 | 8.716114 | 7.022021 | 0.036957 | 0.043264 | 0.043387 |
| c_traits_u.test | RUN | 1.627945 | 9.733916 | 7.358061 | 0.044934 | 0.051990 | 0.055269 |
| test1.test | RUN | 1.036866 | 3.188828 | 3.396675 | 0.003778 | 0.004483 | 0.004442 |
| test2.test | RUN | 1.196020 | 4.879983 | 4.559274 | 0.003609 | 0.004867 | 0.004676 |
| test3.test | RUN | 1.194099 | 4.946843 | 4.642099 | 0.003892 | 0.004610 | 0.004090 |
| test4.test | RUN | 1.077217 | 4.054670 | 3.831456 | 0.004128 | 0.004610 | 0.004405 |
| test5.test | RUN | 1.106880 | 4.233000 | 4.159583 | 0.003949 | 0.004722 | 0.004374 |
| test6.test | RUN | 1.077953 | 3.746317 | 3.796989 | 0.003699 | 0.004519 | 0.004316 |
| test7.test | RUN | 1.062688 | 3.675171 | 3.656340 | 0.003940 | 0.004645 | 0.003887 |
| test8.test | RUN | 1.044056 | 3.612765 | 3.679382 | 0.003545 | 0.004471 | 0.004030 |
| test9.test | RUN | 1.124561 | 4.087524 | 4.002334 | 0.004164 | 0.004332 | 0.004303 |
| test10.test | RUN | 1.096053 | 3.928176 | 3.917273 | 0.003676 | 0.004414 | 0.004154 |
| test11.test | RUN | 1.063593 | 3.778177 | 3.719676 | 0.003763 | 0.004626 | 0.003714 |
| test1u.test | RUN | 1.026424 | 3.234034 | 3.340936 | 0.003653 | 0.004725 | 0.004275 |
| test2u.test | RUN | 1.246005 | 5.090648 | 4.772701 | 0.004212 | 0.004822 | 0.004713 |
| test3u.test | RUN | 1.249299 | 5.450586 | 4.911732 | 0.004081 | 0.004257 | 0.004480 |
| test4u.test | RUN | 1.086240 | 4.020378 | 3.888610 | 0.004211 | 0.004461 | 0.004024 |
| test5u.test | RUN | 1.131277 | 4.368589 | 4.224661 | 0.003994 | 0.004651 | 0.004463 |
| test6u.test | RUN | 1.123158 | 3.900129 | 3.812294 | 0.004031 | 0.004526 | 0.004428 |
| test7u.test | RUN | 1.075245 | 3.857597 | 3.871811 | 0.003900 | 0.004335 | 0.004522 |
| test8u.test | RUN | 1.029554 | 3.625954 | 3.651995 | 0.003929 | 0.004348 | 0.004282 |
| test9u.test | RUN | 1.118752 | 4.098274 | 4.049062 | 0.004104 | 0.004632 | 0.004406 |
| test10u.test | RUN | 1.106421 | 3.988018 | 3.939385 | 0.003983 | 0.004278 | 0.004600 |
| test11u.test | RUN | 1.095517 | 3.978154 | 3.892190 | 0.004270 | 0.004403 | 0.004264 |
| misc1.test | RUN | 1.448085 | 7.823840 | 6.464858 | 0.005283 | 0.006008 | 0.005921 |
| misc2.test | RUN | 1.581643 | 7.827752 | 6.555918 | 0.004726 | 0.004962 | 0.004973 |
| test_format.test | RUN | 1.401403 | 7.660560 | 5.984159 | 0.004102 | 0.004763 | 0.004535 |
| test_cycles.test | RUN | 0.990012 | 3.046047 | 3.243942 | 0.004144 | 0.004471 | 0.004404 |
| test_non_char.test | RUN | 1.299412 | 6.390728 | 5.235299 | 0.004233 | 0.004639 | 0.004366 |
| test_static.test | RUN | 0.946494 | 2.767775 | 2.852099 | 0.003901 | 0.004282 | 0.004164 |
| test_actions.test | RUN | 1.867967 | 9.736712 | 8.536849 | 0.005899 | 0.006250 | 0.006116 |
| test_assert.test | RUN | 1.236209 | 4.246785 | 4.351315 | 0.004421 | 0.004408 | 0.004504 |
| test_assert_with_placeholder.test | RUN | 1.215002 | 3.719611 | 3.887039 | 0.003912 | 0.003986 | 0.003691 |
| test_symbols.test | RUN | 2.030639 | 11.041647 | 9.132371 | 0.005523 | 0.005713 | 0.005539 |
| test_dynamic.test | RUN | 1.173232 | 4.837910 | 4.261431 | 0.003929 | 0.004201 | 0.003896 |
| test_dynamic_grammar.test | RUN | 1.158871 | 5.039869 | 4.358645 | 0.004381 | 0.004890 | 0.004597 |
| test_skip.test | RUN | 1.548887 | 7.013485 | 6.329773 | 0.003800 | 0.004107 | 0.004053 |
| multiple_defs.test | LINK | 2.233584 | 9.267090 | 8.231094 | — | — | — |
| test_basic_regex.test | COMPILE | 0.769229 | 1.376770 | 1.733695 | — | — | — |
| test_match_results.test | COMPILE | 0.501057 | 0.903889 | 1.241914 | — | — | — |
| test_regex_algorithms.test | COMPILE | 0.503940 | 0.907430 | 1.261874 | — | — | — |
| test_regex_compiler.test | COMPILE | 0.767940 | 1.437357 | 1.924978 | — | — | — |
| test_regex_constants.test | COMPILE | 0.047446 | 0.051212 | 0.068403 | — | — | — |
| test_regex_error.test | COMPILE | 0.267294 | 0.459924 | 0.695885 | — | — | — |
| test_regex_iterator.test | COMPILE | 0.510021 | 0.898136 | 1.277694 | — | — | — |
| test_regex_primitives.test | COMPILE | 0.756881 | 1.435839 | 1.689753 | — | — | — |
| test_regex_token_iterator.test | COMPILE | 0.492527 | 0.912339 | 1.243284 | — | — | — |
| test_regex_traits.test | COMPILE | 0.296315 | 0.455823 | 0.665979 | — | — | — |
| test_sub_match.test | COMPILE | 0.206058 | 0.284727 | 0.454651 | — | — | — |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497688) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497688) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497688).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
