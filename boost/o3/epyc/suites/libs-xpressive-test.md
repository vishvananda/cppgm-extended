# libs/xpressive/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| regress.test | RUN | 2.941277 | 17.777329 | 14.095513 | 0.010768 | 0.013732 | 0.009674 |
| regress_u.test | RUN | 3.102297 | 18.269728 | 14.117974 | 0.011081 | 0.013303 | 0.010057 |
| c_traits.test | RUN | 2.899371 | 17.361000 | 13.430921 | 0.007508 | 0.011332 | 0.008941 |
| c_traits_u.test | RUN | 3.115061 | 18.009878 | 14.061378 | 0.009183 | 0.012177 | 0.008415 |
| test1.test | RUN | 1.068281 | 4.522557 | 4.188419 | 0.003034 | 0.003274 | 0.003214 |
| test2.test | RUN | 1.397833 | 6.463946 | 6.085528 | 0.003224 | 0.003320 | 0.003205 |
| test3.test | RUN | 1.440953 | 6.463991 | 6.156293 | 0.002948 | 0.003233 | 0.003130 |
| test4.test | RUN | 1.257733 | 5.317582 | 5.069545 | 0.002955 | 0.003221 | 0.003056 |
| test5.test | RUN | 1.308034 | 5.756865 | 5.502504 | 0.003157 | 0.003300 | 0.002947 |
| test6.test | RUN | 1.177659 | 4.767016 | 4.664391 | 0.003171 | 0.003081 | 0.003022 |
| test7.test | RUN | 1.152965 | 4.982704 | 4.684864 | 0.002928 | 0.003160 | 0.003180 |
| test8.test | RUN | 1.142318 | 4.743459 | 4.548277 | 0.002793 | 0.003072 | 0.003202 |
| test9.test | RUN | 1.227366 | 5.239675 | 5.110962 | 0.003219 | 0.003369 | 0.003162 |
| test10.test | RUN | 1.216778 | 4.970546 | 4.819779 | 0.002822 | 0.003180 | 0.003056 |
| test11.test | RUN | 1.168471 | 4.685828 | 4.741123 | 0.002886 | 0.003140 | 0.003191 |
| test1u.test | RUN | 1.085335 | 4.483130 | 4.080312 | 0.002942 | 0.003161 | 0.003132 |
| test2u.test | RUN | 1.438810 | 6.656179 | 6.337302 | 0.003240 | 0.003120 | 0.003001 |
| test3u.test | RUN | 1.453925 | 6.823942 | 6.545347 | 0.003184 | 0.003260 | 0.003381 |
| test4u.test | RUN | 1.251199 | 5.305027 | 5.127447 | 0.003394 | 0.003184 | 0.003129 |
| test5u.test | RUN | 1.335822 | 5.774748 | 5.678320 | 0.003036 | 0.003365 | 0.003252 |
| test6u.test | RUN | 1.227985 | 4.934253 | 4.865557 | 0.003129 | 0.003248 | 0.003186 |
| test7u.test | RUN | 1.197639 | 5.162632 | 4.883736 | 0.003042 | 0.003279 | 0.003131 |
| test8u.test | RUN | 1.140250 | 4.743439 | 4.552788 | 0.002810 | 0.003306 | 0.003176 |
| test9u.test | RUN | 1.238399 | 5.184552 | 5.153658 | 0.003141 | 0.003378 | 0.003118 |
| test10u.test | RUN | 1.222855 | 4.865697 | 4.747646 | 0.003142 | 0.003392 | 0.003166 |
| test11u.test | RUN | 1.208527 | 4.844548 | 4.930852 | 0.003231 | 0.003329 | 0.003131 |
| misc1.test | RUN | 2.172273 | 12.266989 | 10.517706 | 0.004034 | 0.004302 | 0.003991 |
| misc2.test | RUN | 2.273020 | 11.660624 | 10.195503 | 0.003361 | 0.004311 | 0.003590 |
| test_format.test | RUN | 2.426095 | 14.246710 | 11.478327 | 0.003243 | 0.003365 | 0.003174 |
| test_cycles.test | RUN | 1.053867 | 4.387425 | 3.892646 | 0.003059 | 0.003359 | 0.003188 |
| test_non_char.test | RUN | 1.943993 | 9.828524 | 8.661356 | 0.003331 | 0.003384 | 0.002966 |
| test_static.test | RUN | 0.958781 | 3.991507 | 3.372792 | 0.003241 | 0.003094 | 0.003165 |
| test_actions.test | RUN | 2.256583 | 11.968063 | 10.809228 | 0.004097 | 0.004353 | 0.004052 |
| test_assert.test | RUN | 1.335261 | 5.113221 | 4.974044 | 0.003422 | 0.003494 | 0.003385 |
| test_assert_with_placeholder.test | RUN | 1.251859 | 4.756027 | 4.599400 | 0.003111 | 0.003156 | 0.003131 |
| test_symbols.test | RUN | 2.413182 | 11.884210 | 12.075609 | 0.003971 | 0.003952 | 0.004381 |
| test_dynamic.test | RUN | 1.549699 | 8.049058 | 6.942248 | 0.003162 | 0.003158 | 0.003061 |
| test_dynamic_grammar.test | RUN | 1.630262 | 8.274113 | 7.220805 | 0.003423 | 0.003480 | 0.003077 |
| test_skip.test | RUN | 1.849322 | 8.485660 | 8.322812 | 0.003114 | 0.003234 | 0.003094 |
| multiple_defs.test | LINK | 3.142617 | 15.582053 | 13.827829 | — | — | — |
| test_basic_regex.test | COMPILE | 0.660579 | 1.308279 | 1.947519 | — | — | — |
| test_match_results.test | COMPILE | 0.441348 | 0.885281 | 1.324430 | — | — | — |
| test_regex_algorithms.test | COMPILE | 0.433128 | 0.869843 | 1.280991 | — | — | — |
| test_regex_compiler.test | COMPILE | 0.655641 | 1.388683 | 1.915403 | — | — | — |
| test_regex_constants.test | COMPILE | 0.038027 | 0.059807 | 0.067434 | — | — | — |
| test_regex_error.test | COMPILE | 0.241532 | 0.450826 | 0.683020 | — | — | — |
| test_regex_iterator.test | COMPILE | 0.453735 | 0.860411 | 1.270025 | — | — | — |
| test_regex_primitives.test | COMPILE | 0.649656 | 1.387377 | 1.764008 | — | — | — |
| test_regex_token_iterator.test | COMPILE | 0.426361 | 0.883466 | 1.079584 | — | — | — |
| test_regex_traits.test | COMPILE | 0.248400 | 0.455952 | 0.697424 | — | — | — |
| test_sub_match.test | COMPILE | 0.192585 | 0.339492 | 0.521140 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.084× | 1.087×, 1.080× | 0.996× |
| clang | 0.935× | 0.932×, 0.939× | 0.991× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224918).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| c_traits.test | 3 | 0.007399s | 0.010272s | 0.006744s | byte equality observed |
| c_traits_u.test | 3 | 0.007949s | 0.010604s | 0.006847s | byte equality observed |
| misc1.test | 8 | 0.002602s | 0.002495s | 0.002476s | byte equality observed |
| misc2.test | 10 | 0.002310s | 0.002207s | 0.002247s | byte equality observed |
| regress.test | 3 | 0.009375s | 0.011620s | 0.007901s | byte equality observed |
| regress_u.test | 3 | 0.009372s | 0.011753s | 0.007786s | byte equality observed |
| test1.test | 10 | 0.002055s | 0.002029s | 0.001967s | byte equality observed |
| test10.test | 10 | 0.002033s | 0.001977s | 0.001940s | byte equality observed |
| test10u.test | 10 | 0.002020s | 0.002021s | 0.001992s | byte equality observed |
| test11.test | 10 | 0.001998s | 0.001999s | 0.001943s | byte equality observed |
| test11u.test | 10 | 0.002021s | 0.002020s | 0.001969s | byte equality observed |
| test1u.test | 10 | 0.002028s | 0.002041s | 0.001997s | byte equality observed |
| test2.test | 10 | 0.002093s | 0.002085s | 0.001997s | byte equality observed |
| test2u.test | 10 | 0.002080s | 0.002085s | 0.002024s | byte equality observed |
| test3.test | 10 | 0.002081s | 0.002116s | 0.002028s | byte equality observed |
| test3u.test | 10 | 0.002029s | 0.002120s | 0.002047s | byte equality observed |
| test4.test | 10 | 0.002055s | 0.002036s | 0.001994s | byte equality observed |
| test4u.test | 10 | 0.002064s | 0.002069s | 0.001978s | byte equality observed |
| test5.test | 10 | 0.002056s | 0.002067s | 0.001955s | byte equality observed |
| test5u.test | 10 | 0.002063s | 0.002077s | 0.002052s | byte equality observed |
| test6.test | 10 | 0.002029s | 0.002051s | 0.001975s | byte equality observed |
| test6u.test | 10 | 0.002041s | 0.002011s | 0.001980s | byte equality observed |
| test7.test | 10 | 0.002040s | 0.002012s | 0.001978s | byte equality observed |
| test7u.test | 10 | 0.002023s | 0.002012s | 0.001985s | byte equality observed |
| test8.test | 10 | 0.002032s | 0.001989s | 0.001989s | byte equality observed |
| test8u.test | 10 | 0.002020s | 0.001978s | 0.001984s | byte equality observed |
| test9.test | 10 | 0.002045s | 0.002026s | 0.001995s | byte equality observed |
| test9u.test | 10 | 0.002030s | 0.002015s | 0.001989s | byte equality observed |
| test_actions.test | 8 | 0.002588s | 0.002533s | 0.002526s | byte equality observed |
| test_assert.test | 9 | 0.002151s | 0.002119s | 0.002074s | byte equality observed |
| test_assert_with_placeholder.test | 10 | 0.002001s | 0.001975s | 0.001948s | byte equality observed |
| test_cycles.test | 10 | 0.002073s | 0.002040s | 0.001983s | byte equality observed |
| test_dynamic.test | 10 | 0.002035s | 0.002002s | 0.002002s | byte equality observed |
| test_dynamic_grammar.test | 9 | 0.002180s | 0.002143s | 0.002115s | byte equality observed |
| test_format.test | 10 | 0.002117s | 0.002084s | 0.002056s | byte equality observed |
| test_non_char.test | 10 | 0.002108s | 0.002030s | 0.002011s | byte equality observed |
| test_skip.test | 10 | 0.002002s | 0.001973s | 0.002003s | byte equality observed |
| test_static.test | 9 | 0.001973s | 0.001869s | 0.001907s | byte equality observed |
| test_symbols.test | 9 | 0.002371s | 0.002303s | 0.002226s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224918).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
