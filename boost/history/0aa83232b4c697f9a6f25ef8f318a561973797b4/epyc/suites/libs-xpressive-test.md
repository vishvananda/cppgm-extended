# libs/xpressive/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| regress.test | RUN | 1.246783 | 8.210151 | 5.651187 | 0.033079 | 0.033025 | 0.035896 |
| regress_u.test | RUN | 1.421140 | 8.388906 | 5.793409 | 0.037768 | 0.041362 | 0.040695 |
| c_traits.test | RUN | 1.319020 | 7.602628 | 5.431121 | 0.026164 | 0.029404 | 0.031418 |
| c_traits_u.test | RUN | 1.379869 | 8.584064 | 5.796958 | 0.034808 | 0.036239 | 0.035710 |
| test1.test | RUN | 0.840078 | 2.723504 | 2.519037 | 0.003514 | 0.004178 | 0.004401 |
| test2.test | RUN | 1.000913 | 4.224259 | 3.573532 | 0.003793 | 0.004446 | 0.004378 |
| test3.test | RUN | 1.000532 | 4.294799 | 3.587749 | 0.003585 | 0.004228 | 0.003710 |
| test4.test | RUN | 0.902732 | 3.498716 | 2.984823 | 0.003506 | 0.003987 | 0.004244 |
| test5.test | RUN | 0.942840 | 3.702380 | 3.187679 | 0.003724 | 0.004392 | 0.003935 |
| test6.test | RUN | 0.962914 | 3.327725 | 2.902662 | 0.003496 | 0.004131 | 0.004169 |
| test7.test | RUN | 0.947820 | 3.298387 | 2.883271 | 0.003626 | 0.004065 | 0.004323 |
| test8.test | RUN | 0.940316 | 3.246221 | 2.851584 | 0.003471 | 0.003819 | 0.003882 |
| test9.test | RUN | 0.985965 | 3.658181 | 3.119965 | 0.003494 | 0.004141 | 0.004279 |
| test10.test | RUN | 0.892776 | 3.458584 | 3.146651 | 0.003623 | 0.004105 | 0.004183 |
| test11.test | RUN | 0.877286 | 3.189564 | 2.935941 | 0.003676 | 0.003703 | 0.004039 |
| test1u.test | RUN | 0.824730 | 2.767789 | 2.598842 | 0.003705 | 0.003829 | 0.004011 |
| test2u.test | RUN | 1.024483 | 4.458658 | 3.708148 | 0.003930 | 0.004121 | 0.004137 |
| test3u.test | RUN | 1.039059 | 4.841098 | 3.733880 | 0.003625 | 0.004086 | 0.004188 |
| test4u.test | RUN | 0.929976 | 3.588568 | 2.948078 | 0.003904 | 0.003932 | 0.004067 |
| test5u.test | RUN | 0.948762 | 3.859345 | 3.224683 | 0.003588 | 0.005047 | 0.004000 |
| test6u.test | RUN | 0.941273 | 3.507223 | 2.931792 | 0.003849 | 0.003938 | 0.004031 |
| test7u.test | RUN | 0.865784 | 3.384500 | 2.911253 | 0.003762 | 0.004187 | 0.004160 |
| test8u.test | RUN | 0.862264 | 3.161740 | 2.771248 | 0.003305 | 0.003944 | 0.003998 |
| test9u.test | RUN | 0.910321 | 3.674949 | 3.068790 | 0.003831 | 0.004327 | 0.004150 |
| test10u.test | RUN | 0.927724 | 3.429726 | 2.992964 | 0.003840 | 0.004026 | 0.004043 |
| test11u.test | RUN | 0.944575 | 3.495600 | 3.016843 | 0.003888 | 0.003855 | 0.004089 |
| misc1.test | RUN | 1.239701 | 7.062255 | 5.059190 | 0.004801 | 0.005615 | 0.005463 |
| misc2.test | RUN | 1.324926 | 6.952111 | 5.151027 | 0.004269 | 0.004829 | 0.004401 |
| test_format.test | RUN | 1.145069 | 6.770747 | 4.624585 | 0.003911 | 0.004293 | 0.004411 |
| test_cycles.test | RUN | 0.813282 | 2.672104 | 2.435083 | 0.004098 | 0.004194 | 0.003996 |
| test_non_char.test | RUN | 1.114646 | 5.642762 | 4.085817 | 0.004081 | 0.004152 | 0.004011 |
| test_static.test | RUN | 0.819452 | 2.333852 | 2.240188 | 0.003844 | 0.003949 | 0.003853 |
| test_actions.test | RUN | 1.598280 | 8.425781 | 6.625910 | 0.006004 | 0.005500 | 0.005514 |
| test_assert.test | RUN | 1.063362 | 3.572660 | 3.330603 | 0.004365 | 0.004176 | 0.004239 |
| test_assert_with_placeholder.test | RUN | 1.054478 | 3.125445 | 2.943720 | 0.003722 | 0.004365 | 0.004602 |
| test_symbols.test | RUN | 1.768031 | 9.382407 | 7.237445 | 0.005145 | 0.005267 | 0.005432 |
| test_dynamic.test | RUN | 0.977828 | 4.198580 | 3.174932 | 0.003452 | 0.003964 | 0.003821 |
| test_dynamic_grammar.test | RUN | 0.983855 | 4.268311 | 3.410519 | 0.003800 | 0.004384 | 0.004362 |
| test_skip.test | RUN | 1.300738 | 6.335379 | 5.069946 | 0.003791 | 0.004063 | 0.004103 |
| multiple_defs.test | LINK | 1.825705 | 8.097680 | 6.460741 | — | — | — |
| test_basic_regex.test | COMPILE | 0.607626 | 1.178520 | 1.332038 | — | — | — |
| test_match_results.test | COMPILE | 0.417209 | 0.752854 | 0.923814 | — | — | — |
| test_regex_algorithms.test | COMPILE | 0.420783 | 0.729116 | 0.922296 | — | — | — |
| test_regex_compiler.test | COMPILE | 0.629417 | 1.131152 | 1.354465 | — | — | — |
| test_regex_constants.test | COMPILE | 0.038561 | 0.040118 | 0.061463 | — | — | — |
| test_regex_error.test | COMPILE | 0.233115 | 0.381029 | 0.501481 | — | — | — |
| test_regex_iterator.test | COMPILE | 0.412835 | 0.726631 | 0.893140 | — | — | — |
| test_regex_primitives.test | COMPILE | 0.625623 | 1.086659 | 1.220023 | — | — | — |
| test_regex_token_iterator.test | COMPILE | 0.395491 | 0.661170 | 0.856495 | — | — | — |
| test_regex_traits.test | COMPILE | 0.247375 | 0.394080 | 0.488005 | — | — | — |
| test_sub_match.test | COMPILE | 0.207263 | 0.314309 | 0.334087 | — | — | — |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114108) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114108) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114108).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
