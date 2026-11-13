# libs/xpressive/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| regress.test | RUN | 3.197982 | 18.948528 | 13.726791 | 0.010755 | 0.011852 | 0.009117 |
| regress_u.test | RUN | 3.420181 | 19.593346 | 13.907640 | 0.010979 | 0.012094 | 0.009957 |
| c_traits.test | RUN | 3.186054 | 18.897455 | 13.307701 | 0.008602 | 0.010004 | 0.007994 |
| c_traits_u.test | RUN | 3.411027 | 19.393281 | 13.859743 | 0.010006 | 0.011009 | 0.008443 |
| test1.test | RUN | 1.081498 | 4.721989 | 3.978466 | 0.002452 | 0.002363 | 0.002263 |
| test2.test | RUN | 1.478258 | 6.759729 | 5.901639 | 0.002544 | 0.002375 | 0.002241 |
| test3.test | RUN | 1.477811 | 6.820904 | 5.878651 | 0.002327 | 0.002481 | 0.002206 |
| test4.test | RUN | 1.262008 | 5.588428 | 4.839305 | 0.002352 | 0.002377 | 0.002200 |
| test5.test | RUN | 1.348021 | 6.047551 | 5.204830 | 0.002424 | 0.002440 | 0.002222 |
| test6.test | RUN | 1.207053 | 5.092907 | 4.385995 | 0.002458 | 0.002260 | 0.002241 |
| test7.test | RUN | 1.156288 | 5.238128 | 4.426933 | 0.002540 | 0.002427 | 0.002137 |
| test8.test | RUN | 1.149467 | 5.043984 | 4.296171 | 0.002264 | 0.002420 | 0.002132 |
| test9.test | RUN | 1.265544 | 5.532668 | 4.832790 | 0.002471 | 0.002472 | 0.002170 |
| test10.test | RUN | 1.233563 | 5.203587 | 4.464610 | 0.002438 | 0.002423 | 0.002210 |
| test11.test | RUN | 1.185240 | 5.001826 | 4.409736 | 0.002280 | 0.002405 | 0.002213 |
| test1u.test | RUN | 1.074141 | 4.650109 | 3.877952 | 0.002526 | 0.002339 | 0.002237 |
| test2u.test | RUN | 1.466696 | 6.999343 | 6.099420 | 0.002624 | 0.002799 | 0.002326 |
| test3u.test | RUN | 1.515105 | 7.204980 | 6.219895 | 0.002336 | 0.002523 | 0.002238 |
| test4u.test | RUN | 1.287087 | 5.589603 | 4.907434 | 0.002446 | 0.002389 | 0.002255 |
| test5u.test | RUN | 1.369050 | 6.174994 | 5.451910 | 0.002811 | 0.002381 | 0.002229 |
| test6u.test | RUN | 1.246107 | 5.142094 | 4.523336 | 0.002515 | 0.002459 | 0.002245 |
| test7u.test | RUN | 1.205585 | 5.422000 | 4.655750 | 0.002490 | 0.002356 | 0.002196 |
| test8u.test | RUN | 1.155149 | 5.031938 | 4.324192 | 0.002419 | 0.002354 | 0.002199 |
| test9u.test | RUN | 1.253144 | 5.397538 | 4.936626 | 0.002281 | 0.002435 | 0.002278 |
| test10u.test | RUN | 1.231994 | 5.123946 | 4.560366 | 0.002520 | 0.002428 | 0.002234 |
| test11u.test | RUN | 1.240788 | 5.146824 | 4.652896 | 0.002502 | 0.002540 | 0.002230 |
| misc1.test | RUN | 2.277267 | 13.222877 | 10.258471 | 0.002862 | 0.003064 | 0.002787 |
| misc2.test | RUN | 2.322176 | 12.431510 | 9.974888 | 0.002503 | 0.002643 | 0.002446 |
| test_format.test | RUN | 2.509005 | 15.337354 | 11.415200 | 0.002625 | 0.002415 | 0.002276 |
| test_cycles.test | RUN | 1.037727 | 4.548417 | 3.732939 | 0.002322 | 0.002368 | 0.002241 |
| test_non_char.test | RUN | 2.051489 | 10.615678 | 8.402154 | 0.002495 | 0.002599 | 0.002306 |
| test_static.test | RUN | 0.945715 | 4.209935 | 3.302216 | 0.002396 | 0.002415 | 0.002215 |
| test_actions.test | RUN | 2.342037 | 12.550541 | 10.323443 | 0.002938 | 0.002901 | 0.002906 |
| test_assert.test | RUN | 1.325606 | 5.424846 | 4.801279 | 0.002437 | 0.002500 | 0.002356 |
| test_assert_with_placeholder.test | RUN | 1.236538 | 4.922086 | 4.313407 | 0.002231 | 0.002168 | 0.002193 |
| test_symbols.test | RUN | 2.509587 | 12.339960 | 11.653829 | 0.003007 | 0.003004 | 0.002834 |
| test_dynamic.test | RUN | 1.632609 | 8.423636 | 6.710397 | 0.002346 | 0.002313 | 0.002267 |
| test_dynamic_grammar.test | RUN | 1.677597 | 9.024600 | 7.124106 | 0.002579 | 0.002526 | 0.002421 |
| test_skip.test | RUN | 1.913190 | 9.118011 | 8.127611 | 0.002397 | 0.002162 | 0.002258 |
| multiple_defs.test | LINK | 3.215742 | 16.351043 | 13.702286 | — | — | — |
| test_basic_regex.test | COMPILE | 0.618608 | 1.329123 | 1.746013 | — | — | — |
| test_match_results.test | COMPILE | 0.421579 | 0.891966 | 1.192211 | — | — | — |
| test_regex_algorithms.test | COMPILE | 0.411528 | 0.896156 | 1.228606 | — | — | — |
| test_regex_compiler.test | COMPILE | 0.634467 | 1.417696 | 1.723443 | — | — | — |
| test_regex_constants.test | COMPILE | 0.032880 | 0.056688 | 0.061326 | — | — | — |
| test_regex_error.test | COMPILE | 0.223588 | 0.454952 | 0.619994 | — | — | — |
| test_regex_iterator.test | COMPILE | 0.420899 | 0.903809 | 1.139184 | — | — | — |
| test_regex_primitives.test | COMPILE | 0.547000 | 1.409415 | 1.646941 | — | — | — |
| test_regex_token_iterator.test | COMPILE | 0.409664 | 0.900347 | 1.095698 | — | — | — |
| test_regex_traits.test | COMPILE | 0.242698 | 0.474197 | 0.597118 | — | — | — |
| test_sub_match.test | COMPILE | 0.190197 | 0.316493 | 0.446436 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.050× | 1.045×, 1.055× | 1.012× |
| clang | 0.954× | 0.962×, 0.946× | 0.999× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345325).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| c_traits.test | 3 | 0.007525s | 0.009187s | 0.007247s | byte equality observed |
| c_traits_u.test | 3 | 0.008135s | 0.009715s | 0.007782s | byte equality observed |
| misc1.test | 10 | 0.002173s | 0.002071s | 0.002072s | byte equality observed |
| misc2.test | 10 | 0.001931s | 0.001837s | 0.001860s | byte equality observed |
| regress.test | 3 | 0.009259s | 0.010636s | 0.008571s | byte equality observed |
| regress_u.test | 3 | 0.009624s | 0.010674s | 0.008922s | byte equality observed |
| test1.test | 10 | 0.001781s | 0.001746s | 0.001693s | byte equality observed |
| test10.test | 7 | 0.001750s | 0.001736s | 0.001665s | byte equality observed |
| test10u.test | 10 | 0.001723s | 0.001700s | 0.001683s | byte equality observed |
| test11.test | 10 | 0.001739s | 0.001723s | 0.001616s | byte equality observed |
| test11u.test | 10 | 0.001730s | 0.001680s | 0.001657s | byte equality observed |
| test1u.test | 10 | 0.001742s | 0.001726s | 0.001686s | byte equality observed |
| test2.test | 10 | 0.001791s | 0.001760s | 0.001690s | byte equality observed |
| test2u.test | 10 | 0.001799s | 0.001794s | 0.001753s | byte equality observed |
| test3.test | 10 | 0.001801s | 0.001767s | 0.001733s | byte equality observed |
| test3u.test | 10 | 0.001801s | 0.001770s | 0.001747s | byte equality observed |
| test4.test | 10 | 0.001769s | 0.001727s | 0.001684s | byte equality observed |
| test4u.test | 10 | 0.001741s | 0.001715s | 0.001719s | byte equality observed |
| test5.test | 10 | 0.001771s | 0.001751s | 0.001730s | byte equality observed |
| test5u.test | 10 | 0.001782s | 0.001789s | 0.001744s | byte equality observed |
| test6.test | 10 | 0.001745s | 0.001709s | 0.001666s | byte equality observed |
| test6u.test | 10 | 0.001751s | 0.001736s | 0.001712s | byte equality observed |
| test7.test | 10 | 0.001759s | 0.001738s | 0.001691s | byte equality observed |
| test7u.test | 10 | 0.001743s | 0.001735s | 0.001686s | byte equality observed |
| test8.test | 10 | 0.001721s | 0.001689s | 0.001696s | byte equality observed |
| test8u.test | 10 | 0.001742s | 0.001698s | 0.001669s | byte equality observed |
| test9.test | 10 | 0.001732s | 0.001705s | 0.001677s | byte equality observed |
| test9u.test | 10 | 0.001767s | 0.001700s | 0.001659s | byte equality observed |
| test_actions.test | 10 | 0.002057s | 0.002052s | 0.002008s | byte equality observed |
| test_assert.test | 10 | 0.001792s | 0.001749s | 0.001713s | byte equality observed |
| test_assert_with_placeholder.test | 10 | 0.001706s | 0.001705s | 0.001684s | byte equality observed |
| test_cycles.test | 10 | 0.001752s | 0.001734s | 0.001660s | byte equality observed |
| test_dynamic.test | 10 | 0.001748s | 0.001712s | 0.001693s | byte equality observed |
| test_dynamic_grammar.test | 10 | 0.001849s | 0.001812s | 0.001766s | byte equality observed |
| test_format.test | 10 | 0.001802s | 0.001721s | 0.001747s | byte equality observed |
| test_non_char.test | 10 | 0.001769s | 0.001722s | 0.001697s | byte equality observed |
| test_skip.test | 10 | 0.001680s | 0.001638s | 0.001612s | byte equality observed |
| test_static.test | 10 | 0.001626s | 0.001530s | 0.001562s | byte equality observed |
| test_symbols.test | 10 | 0.001884s | 0.001818s | 0.001739s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345325).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
