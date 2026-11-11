# libs/xpressive/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| regress.test | RUN | 3.187493 | 18.834995 | 14.222178 | 0.010943 | 0.014282 | 0.009573 |
| regress_u.test | RUN | 3.549730 | 19.406746 | 14.163887 | 0.010925 | 0.013048 | 0.010093 |
| c_traits.test | RUN | 3.143188 | 18.685008 | 13.661950 | 0.008778 | 0.011775 | 0.008034 |
| c_traits_u.test | RUN | 3.430998 | 19.311266 | 14.042698 | 0.008947 | 0.012678 | 0.008337 |
| test1.test | RUN | 1.190219 | 4.817718 | 4.213416 | 0.003289 | 0.003391 | 0.003274 |
| test2.test | RUN | 1.510582 | 6.738390 | 6.116025 | 0.003162 | 0.003437 | 0.003011 |
| test3.test | RUN | 1.499519 | 6.707200 | 6.224648 | 0.003239 | 0.003154 | 0.003008 |
| test4.test | RUN | 1.379780 | 5.304938 | 5.007690 | 0.003022 | 0.003571 | 0.003108 |
| test5.test | RUN | 1.405722 | 5.951782 | 5.498962 | 0.003102 | 0.003376 | 0.003127 |
| test6.test | RUN | 1.278493 | 4.949805 | 4.671505 | 0.003275 | 0.003462 | 0.003183 |
| test7.test | RUN | 1.262823 | 5.128388 | 4.632445 | 0.003214 | 0.003550 | 0.003135 |
| test8.test | RUN | 1.269208 | 4.924695 | 4.546570 | 0.003218 | 0.003379 | 0.003176 |
| test9.test | RUN | 1.361511 | 5.473943 | 5.115145 | 0.003250 | 0.003321 | 0.003068 |
| test10.test | RUN | 1.300612 | 5.206151 | 4.794423 | 0.003190 | 0.003145 | 0.002946 |
| test11.test | RUN | 1.294567 | 4.927139 | 4.646797 | 0.003298 | 0.003515 | 0.003105 |
| test1u.test | RUN | 1.198377 | 4.720706 | 4.176689 | 0.002932 | 0.003511 | 0.003117 |
| test2u.test | RUN | 1.564002 | 7.018959 | 6.315953 | 0.003199 | 0.003485 | 0.003389 |
| test3u.test | RUN | 1.542558 | 7.021579 | 6.451172 | 0.003181 | 0.003344 | 0.003357 |
| test4u.test | RUN | 1.329451 | 5.524727 | 5.046445 | 0.002927 | 0.003473 | 0.003342 |
| test5u.test | RUN | 1.427415 | 6.153713 | 5.646149 | 0.003180 | 0.003566 | 0.003138 |
| test6u.test | RUN | 1.305335 | 5.142521 | 4.867081 | 0.003060 | 0.003479 | 0.002812 |
| test7u.test | RUN | 1.282924 | 5.326651 | 4.868698 | 0.003015 | 0.003489 | 0.003224 |
| test8u.test | RUN | 1.225936 | 5.004196 | 4.515953 | 0.003208 | 0.003475 | 0.003164 |
| test9u.test | RUN | 1.357557 | 5.248241 | 5.145777 | 0.003194 | 0.003262 | 0.003133 |
| test10u.test | RUN | 1.358351 | 4.977820 | 4.652650 | 0.003266 | 0.003590 | 0.003238 |
| test11u.test | RUN | 1.319222 | 5.045246 | 4.940783 | 0.003404 | 0.003409 | 0.003102 |
| misc1.test | RUN | 2.414665 | 13.175976 | 10.473975 | 0.004250 | 0.004599 | 0.004060 |
| misc2.test | RUN | 2.395031 | 12.202752 | 10.155757 | 0.003465 | 0.003951 | 0.003644 |
| test_format.test | RUN | 2.587720 | 14.997961 | 11.541273 | 0.003263 | 0.003519 | 0.003173 |
| test_cycles.test | RUN | 1.172074 | 4.528891 | 3.894636 | 0.003054 | 0.003459 | 0.003093 |
| test_non_char.test | RUN | 2.042336 | 10.392710 | 8.574913 | 0.003246 | 0.003445 | 0.003472 |
| test_static.test | RUN | 1.059474 | 4.084480 | 3.374767 | 0.003225 | 0.003358 | 0.003064 |
| test_actions.test | RUN | 2.459267 | 12.355825 | 10.662940 | 0.004146 | 0.004530 | 0.004105 |
| test_assert.test | RUN | 1.505400 | 5.311729 | 5.040703 | 0.003564 | 0.003503 | 0.003481 |
| test_assert_with_placeholder.test | RUN | 1.384254 | 4.854118 | 4.512264 | 0.003201 | 0.003178 | 0.003075 |
| test_symbols.test | RUN | 2.617946 | 12.166388 | 12.110474 | 0.004520 | 0.004476 | 0.004351 |
| test_dynamic.test | RUN | 1.722123 | 8.093464 | 6.892837 | 0.003353 | 0.003404 | 0.002936 |
| test_dynamic_grammar.test | RUN | 1.778088 | 8.662273 | 7.023415 | 0.003422 | 0.003589 | 0.003526 |
| test_skip.test | RUN | 1.921489 | 8.950434 | 8.259275 | 0.003340 | 0.003550 | 0.002723 |
| multiple_defs.test | LINK | 3.333967 | 16.172058 | 13.716529 | — | — | — |
| test_basic_regex.test | COMPILE | 0.766663 | 1.316145 | 1.836728 | — | — | — |
| test_match_results.test | COMPILE | 0.522908 | 0.945276 | 1.261366 | — | — | — |
| test_regex_algorithms.test | COMPILE | 0.527253 | 0.956257 | 1.283848 | — | — | — |
| test_regex_compiler.test | COMPILE | 0.778877 | 1.500568 | 1.943170 | — | — | — |
| test_regex_constants.test | COMPILE | 0.052226 | 0.065573 | 0.071462 | — | — | — |
| test_regex_error.test | COMPILE | 0.275644 | 0.501094 | 0.708710 | — | — | — |
| test_regex_iterator.test | COMPILE | 0.506254 | 0.913445 | 1.247615 | — | — | — |
| test_regex_primitives.test | COMPILE | 0.685894 | 1.477330 | 1.737252 | — | — | — |
| test_regex_token_iterator.test | COMPILE | 0.437295 | 0.909156 | 1.195350 | — | — | — |
| test_regex_traits.test | COMPILE | 0.282455 | 0.496166 | 0.671464 | — | — | — |
| test_sub_match.test | COMPILE | 0.217835 | 0.400580 | 0.459464 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.097×† | 1.074×, 1.120× | 0.850× |
| clang | 0.938× | 0.955×, 0.921× | 1.000× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| c_traits.test | 3 | 0.007405s | 0.010513s | 0.006599s | byte equality observed |
| c_traits_u.test | 3 | 0.007817s | 0.010604s | 0.007166s | byte equality observed |
| misc1.test | 8 | 0.002718s | 0.002635s | 0.002630s | byte equality observed |
| misc2.test | 9 | 0.002396s | 0.002333s | 0.002326s | byte equality observed |
| regress.test | 3 | 0.009108s | 0.012073s | 0.007967s | byte equality observed |
| regress_u.test | 3 | 0.009690s | 0.012031s | 0.007766s | byte equality observed |
| test1.test | 9 | 0.002166s | 0.002104s | 0.002075s | byte equality observed |
| test10.test | 9 | 0.002081s | 0.002071s | 0.002047s | byte equality observed |
| test10u.test | 9 | 0.002097s | 0.002061s | 0.002058s | byte equality observed |
| test11.test | 10 | 0.002110s | 0.002071s | 0.002009s | byte equality observed |
| test11u.test | 10 | 0.002133s | 0.002110s | 0.002049s | byte equality observed |
| test1u.test | 10 | 0.002090s | 0.002119s | 0.002053s | byte equality observed |
| test2.test | 10 | 0.002157s | 0.002185s | 0.002107s | byte equality observed |
| test2u.test | 9 | 0.002164s | 0.002181s | 0.002169s | byte equality observed |
| test3.test | 10 | 0.002101s | 0.002217s | 0.002128s | byte equality observed |
| test3u.test | 10 | 0.002184s | 0.002146s | 0.002116s | byte equality observed |
| test4.test | 10 | 0.002126s | 0.002105s | 0.002048s | byte equality observed |
| test4u.test | 10 | 0.002120s | 0.002086s | 0.002086s | byte equality observed |
| test5.test | 10 | 0.002122s | 0.002131s | 0.002024s | byte equality observed |
| test5u.test | 9 | 0.002150s | 0.002195s | 0.002121s | byte equality observed |
| test6.test | 10 | 0.002067s | 0.002080s | 0.002067s | byte equality observed |
| test6u.test | 9 | 0.002070s | 0.002156s | 0.002074s | byte equality observed |
| test7.test | 10 | 0.002094s | 0.002123s | 0.002075s | byte equality observed |
| test7u.test | 9 | 0.002099s | 0.002110s | 0.002063s | byte equality observed |
| test8.test | 10 | 0.002124s | 0.002100s | 0.002038s | byte equality observed |
| test8u.test | 10 | 0.002105s | 0.002073s | 0.002063s | byte equality observed |
| test9.test | 10 | 0.002099s | 0.002101s | 0.002056s | byte equality observed |
| test9u.test | 10 | 0.002140s | 0.002090s | 0.002059s | byte equality observed |
| test_actions.test | 8 | 0.002670s | 0.002685s | 0.002591s | byte equality observed |
| test_assert.test | 10 | 0.002229s | 0.002193s | 0.002131s | byte equality observed |
| test_assert_with_placeholder.test | 10 | 0.002110s | 0.002005s | 0.002029s | byte equality observed |
| test_cycles.test | 8 | 0.002148s | 0.002081s | 0.002072s | byte equality observed |
| test_dynamic.test | 10 | 0.002133s | 0.002088s | 0.002057s | byte equality observed |
| test_dynamic_grammar.test | 9 | 0.002268s | 0.002209s | 0.002187s | byte equality observed |
| test_format.test | 9 | 0.002218s | 0.002202s | 0.002112s | byte equality observed |
| test_non_char.test | 10 | 0.002219s | 0.002181s | 0.002057s | byte equality observed |
| test_skip.test | 10 | 0.002066s | 0.002016s | 0.002021s | byte equality observed |
| test_static.test | 10 | 0.001997s | 0.001975s | 0.001883s | byte equality observed |
| test_symbols.test | 8 | 0.002589s | 0.002590s | 0.002339s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
