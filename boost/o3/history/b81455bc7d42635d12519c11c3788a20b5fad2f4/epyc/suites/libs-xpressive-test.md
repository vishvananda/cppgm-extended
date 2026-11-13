# libs/xpressive/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| regress.test | RUN | 2.077957 | 13.703572 | 10.180537 | 0.008058 | 0.010416 | 0.007310 |
| regress_u.test | RUN | 2.423442 | 14.303388 | 10.359639 | 0.008022 | 0.010134 | 0.007799 |
| c_traits.test | RUN | 2.296776 | 13.530387 | 9.927903 | 0.006926 | 0.008785 | 0.006456 |
| c_traits_u.test | RUN | 2.432331 | 14.138220 | 10.229640 | 0.006967 | 0.009376 | 0.006707 |
| test1.test | RUN | 0.793587 | 3.464332 | 2.843038 | 0.002628 | 0.002694 | 0.002616 |
| test2.test | RUN | 1.035687 | 4.830812 | 4.305229 | 0.002555 | 0.002960 | 0.002595 |
| test3.test | RUN | 1.034763 | 4.856560 | 4.339260 | 0.002368 | 0.002563 | 0.002606 |
| test4.test | RUN | 0.913640 | 4.024381 | 3.576004 | 0.002499 | 0.002672 | 0.002645 |
| test5.test | RUN | 0.962718 | 4.331745 | 3.776885 | 0.002525 | 0.002769 | 0.002562 |
| test6.test | RUN | 0.840612 | 3.694582 | 3.176910 | 0.002750 | 0.002666 | 0.002597 |
| test7.test | RUN | 0.852258 | 3.723993 | 3.201621 | 0.002406 | 0.002732 | 0.002673 |
| test8.test | RUN | 0.850209 | 3.658594 | 3.104821 | 0.002590 | 0.002718 | 0.002563 |
| test9.test | RUN | 0.898016 | 3.906206 | 3.565962 | 0.002534 | 0.002719 | 0.002506 |
| test10.test | RUN | 0.887651 | 3.672239 | 3.302470 | 0.002354 | 0.002400 | 0.002436 |
| test11.test | RUN | 0.851235 | 3.636530 | 3.246300 | 0.002099 | 0.002780 | 0.002573 |
| test1u.test | RUN | 0.786608 | 3.459309 | 2.770725 | 0.002424 | 0.002734 | 0.002616 |
| test2u.test | RUN | 1.058821 | 4.939255 | 4.470653 | 0.002678 | 0.002878 | 0.002722 |
| test3u.test | RUN | 1.079050 | 5.092631 | 4.529880 | 0.002759 | 0.002847 | 0.002618 |
| test4u.test | RUN | 0.946165 | 4.023915 | 3.453772 | 0.002877 | 0.002844 | 0.002649 |
| test5u.test | RUN | 0.983901 | 4.408575 | 3.952273 | 0.002484 | 0.002768 | 0.002541 |
| test6u.test | RUN | 0.905995 | 3.697989 | 3.363911 | 0.002618 | 0.002803 | 0.002523 |
| test7u.test | RUN | 0.861187 | 3.912835 | 3.344968 | 0.002561 | 0.002656 | 0.002604 |
| test8u.test | RUN | 0.861350 | 3.652330 | 3.188183 | 0.002430 | 0.002756 | 0.002580 |
| test9u.test | RUN | 0.937841 | 3.946044 | 3.554492 | 0.002570 | 0.002851 | 0.002561 |
| test10u.test | RUN | 0.889662 | 3.695923 | 3.233215 | 0.002629 | 0.002773 | 0.002579 |
| test11u.test | RUN | 0.897899 | 3.780536 | 3.421332 | 0.002775 | 0.002844 | 0.002611 |
| misc1.test | RUN | 1.533778 | 9.425523 | 7.548049 | 0.003565 | 0.003430 | 0.003371 |
| misc2.test | RUN | 1.636531 | 8.962373 | 7.371173 | 0.002956 | 0.003084 | 0.003000 |
| test_format.test | RUN | 1.701585 | 11.008107 | 8.463990 | 0.002851 | 0.002686 | 0.002708 |
| test_cycles.test | RUN | 0.742094 | 3.345453 | 2.671401 | 0.002611 | 0.002752 | 0.002419 |
| test_non_char.test | RUN | 1.368772 | 7.675810 | 6.186116 | 0.002783 | 0.002844 | 0.002695 |
| test_static.test | RUN | 0.697761 | 3.028693 | 2.404321 | 0.002734 | 0.002789 | 0.002616 |
| test_actions.test | RUN | 1.698821 | 9.381866 | 7.630520 | 0.003602 | 0.003708 | 0.003372 |
| test_assert.test | RUN | 0.996114 | 3.954625 | 3.503447 | 0.002816 | 0.002982 | 0.002756 |
| test_assert_with_placeholder.test | RUN | 0.919769 | 3.737083 | 3.172561 | 0.002581 | 0.002918 | 0.002584 |
| test_symbols.test | RUN | 1.735574 | 9.267492 | 8.554479 | 0.003523 | 0.003943 | 0.003571 |
| test_dynamic.test | RUN | 1.097894 | 6.451329 | 4.940498 | 0.002716 | 0.002846 | 0.002738 |
| test_dynamic_grammar.test | RUN | 1.158908 | 6.815312 | 5.263376 | 0.002466 | 0.003013 | 0.002779 |
| test_skip.test | RUN | 1.378766 | 6.863936 | 5.949544 | 0.002612 | 0.002626 | 0.002601 |
| multiple_defs.test | LINK | 2.173651 | 12.502053 | 10.041301 | — | — | — |
| test_basic_regex.test | COMPILE | 0.500381 | 0.990796 | 1.254671 | — | — | — |
| test_match_results.test | COMPILE | 0.330278 | 0.706325 | 0.777414 | — | — | — |
| test_regex_algorithms.test | COMPILE | 0.311316 | 0.689716 | 0.817311 | — | — | — |
| test_regex_compiler.test | COMPILE | 0.496923 | 1.077131 | 1.236883 | — | — | — |
| test_regex_constants.test | COMPILE | 0.027542 | 0.046769 | 0.050581 | — | — | — |
| test_regex_error.test | COMPILE | 0.173031 | 0.345219 | 0.433839 | — | — | — |
| test_regex_iterator.test | COMPILE | 0.321735 | 0.675370 | 0.841548 | — | — | — |
| test_regex_primitives.test | COMPILE | 0.478086 | 1.128194 | 1.167716 | — | — | — |
| test_regex_token_iterator.test | COMPILE | 0.314469 | 0.688280 | 0.801826 | — | — | — |
| test_regex_traits.test | COMPILE | 0.181440 | 0.360931 | 0.402304 | — | — | — |
| test_sub_match.test | COMPILE | 0.125727 | 0.297347 | 0.283225 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.077×† | 1.109×, 1.046× | 0.996× |
| clang | 0.955× | 0.948×, 0.962× | 0.999× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409407).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| c_traits.test | 4 | 0.004441s | 0.006299s | 0.004258s | byte equality observed |
| c_traits_u.test | 4 | 0.004863s | 0.006447s | 0.004370s | byte equality observed |
| misc1.test | 10 | 0.001840s | 0.001815s | 0.001769s | byte equality observed |
| misc2.test | 10 | 0.001625s | 0.001604s | 0.001564s | byte equality observed |
| regress.test | 3 | 0.005324s | 0.007198s | 0.004758s | byte equality observed |
| regress_u.test | 3 | 0.005472s | 0.007290s | 0.004842s | byte equality observed |
| test1.test | 10 | 0.001456s | 0.001466s | 0.001394s | byte equality observed |
| test10.test | 10 | 0.001442s | 0.001402s | 0.001387s | byte equality observed |
| test10u.test | 10 | 0.001453s | 0.001442s | 0.001417s | byte equality observed |
| test11.test | 10 | 0.001435s | 0.001452s | 0.001410s | byte equality observed |
| test11u.test | 10 | 0.001436s | 0.001444s | 0.001416s | byte equality observed |
| test1u.test | 10 | 0.001443s | 0.001460s | 0.001409s | byte equality observed |
| test2.test | 10 | 0.001479s | 0.001470s | 0.001425s | byte equality observed |
| test2u.test | 10 | 0.001475s | 0.001493s | 0.001437s | byte equality observed |
| test3.test | 10 | 0.001463s | 0.001496s | 0.001443s | byte equality observed |
| test3u.test | 10 | 0.001478s | 0.001490s | 0.001444s | byte equality observed |
| test4.test | 10 | 0.001465s | 0.001448s | 0.001401s | byte equality observed |
| test4u.test | 10 | 0.001471s | 0.001457s | 0.001413s | byte equality observed |
| test5.test | 10 | 0.001462s | 0.001481s | 0.001406s | byte equality observed |
| test5u.test | 10 | 0.001469s | 0.001505s | 0.001413s | byte equality observed |
| test6.test | 10 | 0.001443s | 0.001446s | 0.001419s | byte equality observed |
| test6u.test | 10 | 0.001455s | 0.001461s | 0.001409s | byte equality observed |
| test7.test | 10 | 0.001452s | 0.001449s | 0.001385s | byte equality observed |
| test7u.test | 10 | 0.001468s | 0.001443s | 0.001411s | byte equality observed |
| test8.test | 10 | 0.001442s | 0.001449s | 0.001412s | byte equality observed |
| test8u.test | 10 | 0.001439s | 0.001451s | 0.001423s | byte equality observed |
| test9.test | 10 | 0.001454s | 0.001462s | 0.001401s | byte equality observed |
| test9u.test | 10 | 0.001474s | 0.001461s | 0.001407s | byte equality observed |
| test_actions.test | 10 | 0.001847s | 0.001843s | 0.001780s | byte equality observed |
| test_assert.test | 10 | 0.001556s | 0.001528s | 0.001499s | byte equality observed |
| test_assert_with_placeholder.test | 10 | 0.001453s | 0.001448s | 0.001436s | byte equality observed |
| test_cycles.test | 10 | 0.001464s | 0.001443s | 0.001406s | byte equality observed |
| test_dynamic.test | 10 | 0.001465s | 0.001435s | 0.001429s | byte equality observed |
| test_dynamic_grammar.test | 10 | 0.001555s | 0.001522s | 0.001508s | byte equality observed |
| test_format.test | 10 | 0.001562s | 0.001514s | 0.001471s | byte equality observed |
| test_non_char.test | 10 | 0.001518s | 0.001485s | 0.001470s | byte equality observed |
| test_skip.test | 10 | 0.001462s | 0.001432s | 0.001418s | byte equality observed |
| test_static.test | 10 | 0.001432s | 0.001393s | 0.001379s | byte equality observed |
| test_symbols.test | 10 | 0.001819s | 0.001766s | 0.001751s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409407).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
