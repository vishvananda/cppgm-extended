# libs/xpressive/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| regress.test | RUN | 1.385522 | 7.973049 | 5.295289 | 0.031824 | 0.032509 | 0.035075 |
| regress_u.test | RUN | 1.376530 | 8.211989 | 5.592420 | 0.033680 | 0.036616 | 0.038436 |
| c_traits.test | RUN | 1.330249 | 7.354108 | 5.240531 | 0.027611 | 0.031426 | 0.030046 |
| c_traits_u.test | RUN | 1.401140 | 7.773631 | 5.582715 | 0.031528 | 0.034907 | 0.037464 |
| test1.test | RUN | 0.863105 | 2.726054 | 2.471150 | 0.003627 | 0.003816 | 0.003706 |
| test2.test | RUN | 1.025234 | 3.607803 | 3.356180 | 0.003789 | 0.004030 | 0.004293 |
| test3.test | RUN | 1.022905 | 3.944251 | 3.447642 | 0.003719 | 0.004038 | 0.003932 |
| test4.test | RUN | 0.935609 | 3.244780 | 2.878195 | 0.003827 | 0.003810 | 0.004096 |
| test5.test | RUN | 0.955589 | 3.407501 | 3.028512 | 0.003651 | 0.004174 | 0.003820 |
| test6.test | RUN | 0.951285 | 3.086539 | 2.819188 | 0.003754 | 0.003629 | 0.003912 |
| test7.test | RUN | 0.930438 | 3.050857 | 2.755417 | 0.004263 | 0.003529 | 0.003751 |
| test8.test | RUN | 0.930097 | 2.941456 | 2.689390 | 0.003645 | 0.003777 | 0.003979 |
| test9.test | RUN | 1.001483 | 3.366627 | 2.922733 | 0.004015 | 0.004072 | 0.004047 |
| test10.test | RUN | 0.947058 | 3.280704 | 2.929545 | 0.003561 | 0.003893 | 0.004117 |
| test11.test | RUN | 0.929737 | 3.023103 | 2.855016 | 0.003530 | 0.003756 | 0.004132 |
| test1u.test | RUN | 0.870309 | 2.647079 | 2.488471 | 0.003464 | 0.003817 | 0.004185 |
| test2u.test | RUN | 1.077349 | 4.148801 | 3.649896 | 0.004076 | 0.004259 | 0.004284 |
| test3u.test | RUN | 1.071320 | 4.515506 | 3.747594 | 0.003824 | 0.004262 | 0.004085 |
| test4u.test | RUN | 0.968061 | 3.367168 | 2.981990 | 0.003785 | 0.003793 | 0.004022 |
| test5u.test | RUN | 1.006198 | 3.630695 | 3.116372 | 0.003871 | 0.003996 | 0.004170 |
| test6u.test | RUN | 0.973586 | 3.277232 | 2.914030 | 0.003972 | 0.004092 | 0.003995 |
| test7u.test | RUN | 0.916355 | 3.250327 | 2.848466 | 0.003959 | 0.004087 | 0.004051 |
| test8u.test | RUN | 0.904394 | 3.005859 | 2.694896 | 0.004141 | 0.003735 | 0.003907 |
| test9u.test | RUN | 0.956481 | 3.439708 | 2.989142 | 0.003918 | 0.004091 | 0.004088 |
| test10u.test | RUN | 0.997229 | 3.325588 | 2.910044 | 0.003702 | 0.003958 | 0.003894 |
| test11u.test | RUN | 0.959636 | 3.433407 | 2.877684 | 0.003567 | 0.004132 | 0.003939 |
| misc1.test | RUN | 1.246539 | 6.686199 | 4.900692 | 0.004841 | 0.005350 | 0.005318 |
| misc2.test | RUN | 1.355930 | 6.625555 | 4.904444 | 0.004211 | 0.004598 | 0.004199 |
| test_format.test | RUN | 1.188028 | 6.579707 | 4.427644 | 0.004132 | 0.004267 | 0.004291 |
| test_cycles.test | RUN | 0.838066 | 2.503137 | 2.378774 | 0.003921 | 0.003825 | 0.004121 |
| test_non_char.test | RUN | 1.115023 | 5.556783 | 3.950354 | 0.003893 | 0.004152 | 0.003699 |
| test_static.test | RUN | 0.802325 | 2.187570 | 2.127257 | 0.003705 | 0.003947 | 0.003752 |
| test_actions.test | RUN | 1.591350 | 8.366866 | 6.586202 | 0.005215 | 0.005067 | 0.005160 |
| test_assert.test | RUN | 1.069572 | 3.598510 | 3.158402 | 0.004318 | 0.004251 | 0.004120 |
| test_assert_with_placeholder.test | RUN | 1.013352 | 3.049783 | 2.899882 | 0.003874 | 0.004049 | 0.003752 |
| test_symbols.test | RUN | 1.752660 | 9.417920 | 7.055044 | 0.005279 | 0.005285 | 0.005598 |
| test_dynamic.test | RUN | 0.974032 | 4.037784 | 3.094608 | 0.003925 | 0.004221 | 0.003821 |
| test_dynamic_grammar.test | RUN | 0.999637 | 4.086039 | 3.382270 | 0.003641 | 0.004059 | 0.004247 |
| test_skip.test | RUN | 1.351358 | 5.846721 | 5.034370 | 0.003831 | 0.003837 | 0.004150 |
| multiple_defs.test | LINK | 1.886403 | 7.907097 | 6.305451 | — | — | — |
| test_basic_regex.test | COMPILE | 0.627180 | 1.127524 | 1.353742 | — | — | — |
| test_match_results.test | COMPILE | 0.426014 | 0.663417 | 0.912459 | — | — | — |
| test_regex_algorithms.test | COMPILE | 0.431390 | 0.700437 | 0.893035 | — | — | — |
| test_regex_compiler.test | COMPILE | 0.639028 | 1.121124 | 1.388852 | — | — | — |
| test_regex_constants.test | COMPILE | 0.039618 | 0.044445 | 0.059115 | — | — | — |
| test_regex_error.test | COMPILE | 0.234780 | 0.368612 | 0.508574 | — | — | — |
| test_regex_iterator.test | COMPILE | 0.433145 | 0.697518 | 0.930076 | — | — | — |
| test_regex_primitives.test | COMPILE | 0.636843 | 1.071479 | 1.278062 | — | — | — |
| test_regex_token_iterator.test | COMPILE | 0.427316 | 0.651546 | 0.951111 | — | — | — |
| test_regex_traits.test | COMPILE | 0.254492 | 0.376240 | 0.483810 | — | — | — |
| test_sub_match.test | COMPILE | 0.201350 | 0.306765 | 0.398230 | — | — | — |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42562778965) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42562778965) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42562778965).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
