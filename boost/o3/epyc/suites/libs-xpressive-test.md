# libs/xpressive/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| regress.test | RUN | 3.890981 | 23.669835 | 19.834315 | 0.014757 | 0.020004 | 0.013713 |
| regress_u.test | RUN | 4.031225 | 23.948983 | 19.610855 | 0.014971 | 0.019482 | 0.014383 |
| c_traits.test | RUN | 3.889031 | 22.937105 | 19.012365 | 0.011814 | 0.016204 | 0.010330 |
| c_traits_u.test | RUN | 4.074200 | 23.709697 | 19.848479 | 0.012807 | 0.017731 | 0.012257 |
| test1.test | RUN | 1.418048 | 6.067608 | 6.266907 | 0.003580 | 0.003752 | 0.003738 |
| test2.test | RUN | 1.893292 | 8.612590 | 8.944267 | 0.003876 | 0.003906 | 0.004444 |
| test3.test | RUN | 1.898263 | 8.543362 | 8.988928 | 0.003538 | 0.003603 | 0.003527 |
| test4.test | RUN | 1.622947 | 7.050563 | 7.483514 | 0.003348 | 0.003761 | 0.003739 |
| test5.test | RUN | 1.727342 | 7.684946 | 8.009575 | 0.003549 | 0.003745 | 0.003713 |
| test6.test | RUN | 1.592260 | 6.439970 | 6.919458 | 0.003553 | 0.003667 | 0.003863 |
| test7.test | RUN | 1.536378 | 6.621115 | 6.937031 | 0.003834 | 0.003686 | 0.003756 |
| test8.test | RUN | 1.517224 | 6.406634 | 6.654183 | 0.003663 | 0.003507 | 0.003687 |
| test9.test | RUN | 1.638945 | 6.981724 | 7.669299 | 0.003619 | 0.003819 | 0.003954 |
| test10.test | RUN | 1.623124 | 6.561601 | 7.087554 | 0.003436 | 0.003766 | 0.003498 |
| test11.test | RUN | 1.559643 | 6.363839 | 6.944950 | 0.002717 | 0.003715 | 0.003735 |
| test1u.test | RUN | 1.459677 | 6.013892 | 6.138577 | 0.003820 | 0.003480 | 0.003821 |
| test2u.test | RUN | 1.915619 | 8.931011 | 9.192562 | 0.004127 | 0.003901 | 0.003942 |
| test3u.test | RUN | 1.900931 | 9.056603 | 9.383330 | 0.003758 | 0.003665 | 0.003660 |
| test4u.test | RUN | 1.660012 | 6.894366 | 7.447910 | 0.003810 | 0.003994 | 0.003657 |
| test5u.test | RUN | 1.745829 | 7.839440 | 8.304793 | 0.003797 | 0.004041 | 0.003886 |
| test6u.test | RUN | 1.636979 | 6.582406 | 7.137999 | 0.003647 | 0.003675 | 0.003972 |
| test7u.test | RUN | 1.599779 | 6.798824 | 7.173571 | 0.003524 | 0.003978 | 0.003793 |
| test8u.test | RUN | 1.532422 | 6.425510 | 6.724320 | 0.003420 | 0.003658 | 0.003837 |
| test9u.test | RUN | 1.646024 | 6.790758 | 7.586482 | 0.003602 | 0.003946 | 0.003688 |
| test10u.test | RUN | 1.641207 | 6.520298 | 7.027643 | 0.003538 | 0.003300 | 0.003672 |
| test11u.test | RUN | 1.619103 | 6.507684 | 7.230600 | 0.003847 | 0.003821 | 0.004085 |
| misc1.test | RUN | 2.863607 | 16.146500 | 14.870534 | 0.004485 | 0.004770 | 0.004918 |
| misc2.test | RUN | 2.933583 | 15.507396 | 14.595254 | 0.004242 | 0.004218 | 0.004146 |
| test_format.test | RUN | 3.116693 | 18.771521 | 16.432542 | 0.003834 | 0.003931 | 0.004036 |
| test_cycles.test | RUN | 1.371629 | 5.930009 | 5.776302 | 0.003611 | 0.003798 | 0.003390 |
| test_non_char.test | RUN | 2.545282 | 12.952170 | 12.353446 | 0.003649 | 0.003990 | 0.004016 |
| test_static.test | RUN | 1.287768 | 5.405637 | 5.198460 | 0.003701 | 0.003512 | 0.003650 |
| test_actions.test | RUN | 2.967639 | 15.820814 | 15.079669 | 0.004767 | 0.005030 | 0.005276 |
| test_assert.test | RUN | 1.769656 | 6.952533 | 7.272015 | 0.003965 | 0.004040 | 0.004063 |
| test_assert_with_placeholder.test | RUN | 1.661436 | 6.422603 | 6.822948 | 0.004129 | 0.003908 | 0.003874 |
| test_symbols.test | RUN | 3.206047 | 15.281636 | 17.302708 | 0.004937 | 0.005123 | 0.005355 |
| test_dynamic.test | RUN | 2.086822 | 10.511117 | 9.776414 | 0.003942 | 0.003455 | 0.003701 |
| test_dynamic_grammar.test | RUN | 2.199904 | 11.199010 | 10.507595 | 0.003824 | 0.004557 | 0.003970 |
| test_skip.test | RUN | 2.470779 | 11.203532 | 12.019183 | 0.003697 | 0.003718 | 0.003741 |
| multiple_defs.test | LINK | 4.094137 | 21.043052 | 19.936909 | — | — | — |
| test_basic_regex.test | COMPILE | 0.877087 | 1.796602 | 2.979025 | — | — | — |
| test_match_results.test | COMPILE | 0.583478 | 1.261178 | 2.000380 | — | — | — |
| test_regex_algorithms.test | COMPILE | 0.583004 | 1.270169 | 1.953647 | — | — | — |
| test_regex_compiler.test | COMPILE | 0.887169 | 1.964315 | 2.891260 | — | — | — |
| test_regex_constants.test | COMPILE | 0.048010 | 0.085765 | 0.092289 | — | — | — |
| test_regex_error.test | COMPILE | 0.308826 | 0.659352 | 1.063978 | — | — | — |
| test_regex_iterator.test | COMPILE | 0.576068 | 1.233146 | 1.964770 | — | — | — |
| test_regex_primitives.test | COMPILE | 0.907825 | 1.940055 | 2.631856 | — | — | — |
| test_regex_token_iterator.test | COMPILE | 0.530829 | 1.250003 | 1.662621 | — | — | — |
| test_regex_traits.test | COMPILE | 0.323301 | 0.568705 | 1.067638 | — | — | — |
| test_sub_match.test | COMPILE | 0.257227 | 0.455322 | 0.803134 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.084× | 1.083×, 1.086× | 1.001× |
| clang | 0.923× | 0.918×, 0.927× | 1.006× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824563).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| c_traits.test | 2 | 0.010847s | 0.014729s | 0.009190s | byte equality observed |
| c_traits_u.test | 2 | 0.012129s | 0.015266s | 0.010339s | byte equality observed |
| misc1.test | 7 | 0.003275s | 0.003237s | 0.003147s | byte equality observed |
| misc2.test | 7 | 0.002958s | 0.002801s | 0.002742s | byte equality observed |
| regress.test | 2 | 0.014024s | 0.017416s | 0.011803s | byte equality observed |
| regress_u.test | 2 | 0.014642s | 0.017778s | 0.011997s | byte equality observed |
| test1.test | 9 | 0.002647s | 0.002592s | 0.002595s | byte equality observed |
| test10.test | 9 | 0.002616s | 0.002545s | 0.002465s | byte equality observed |
| test10u.test | 8 | 0.002643s | 0.002567s | 0.002491s | byte equality observed |
| test11.test | 9 | 0.002631s | 0.002590s | 0.002476s | byte equality observed |
| test11u.test | 8 | 0.002594s | 0.002518s | 0.002544s | byte equality observed |
| test1u.test | 8 | 0.002574s | 0.002624s | 0.002553s | byte equality observed |
| test2.test | 8 | 0.002636s | 0.002714s | 0.002555s | byte equality observed |
| test2u.test | 8 | 0.002703s | 0.002721s | 0.002567s | byte equality observed |
| test3.test | 7 | 0.002667s | 0.002695s | 0.002609s | byte equality observed |
| test3u.test | 8 | 0.002708s | 0.002727s | 0.002579s | byte equality observed |
| test4.test | 9 | 0.002642s | 0.002645s | 0.002536s | byte equality observed |
| test4u.test | 9 | 0.002660s | 0.002658s | 0.002529s | byte equality observed |
| test5.test | 8 | 0.002692s | 0.002701s | 0.002611s | byte equality observed |
| test5u.test | 9 | 0.002711s | 0.002648s | 0.002565s | byte equality observed |
| test6.test | 8 | 0.002644s | 0.002645s | 0.002470s | byte equality observed |
| test6u.test | 8 | 0.002647s | 0.002591s | 0.002533s | byte equality observed |
| test7.test | 8 | 0.002628s | 0.002703s | 0.002495s | byte equality observed |
| test7u.test | 9 | 0.002660s | 0.002642s | 0.002550s | byte equality observed |
| test8.test | 8 | 0.002585s | 0.002635s | 0.002533s | byte equality observed |
| test8u.test | 8 | 0.002638s | 0.002607s | 0.002517s | byte equality observed |
| test9.test | 7 | 0.002613s | 0.002620s | 0.002496s | byte equality observed |
| test9u.test | 8 | 0.002592s | 0.002633s | 0.002549s | byte equality observed |
| test_actions.test | 7 | 0.003329s | 0.003379s | 0.003203s | byte equality observed |
| test_assert.test | 8 | 0.002753s | 0.002717s | 0.002671s | byte equality observed |
| test_assert_with_placeholder.test | 9 | 0.002580s | 0.002566s | 0.002521s | byte equality observed |
| test_cycles.test | 8 | 0.002656s | 0.002611s | 0.002559s | byte equality observed |
| test_dynamic.test | 8 | 0.002619s | 0.002564s | 0.002570s | byte equality observed |
| test_dynamic_grammar.test | 8 | 0.002802s | 0.002751s | 0.002760s | byte equality observed |
| test_format.test | 8 | 0.002813s | 0.002700s | 0.002609s | byte equality observed |
| test_non_char.test | 8 | 0.002730s | 0.002627s | 0.002616s | byte equality observed |
| test_skip.test | 8 | 0.002583s | 0.002499s | 0.002447s | byte equality observed |
| test_static.test | 9 | 0.002337s | 0.002240s | 0.002292s | byte equality observed |
| test_symbols.test | 5 | 0.003226s | 0.003093s | 0.002912s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824563).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
