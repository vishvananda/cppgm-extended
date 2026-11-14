# libs/xpressive/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| regress.test | RUN | 2.172225 | 13.855291 | 10.254906 | 0.008186 | 0.009783 | 0.007429 |
| regress_u.test | RUN | 2.305555 | 14.167893 | 10.568378 | 0.008336 | 0.010536 | 0.007704 |
| c_traits.test | RUN | 2.145112 | 13.632540 | 10.100278 | 0.006723 | 0.009010 | 0.006682 |
| c_traits_u.test | RUN | 2.303135 | 14.150602 | 10.439523 | 0.007236 | 0.008962 | 0.006787 |
| test1.test | RUN | 0.830605 | 3.477303 | 3.030891 | 0.002468 | 0.002744 | 0.002718 |
| test2.test | RUN | 1.098302 | 4.904617 | 4.316930 | 0.001971 | 0.002875 | 0.002564 |
| test3.test | RUN | 1.069566 | 4.905657 | 4.421967 | 0.002975 | 0.002820 | 0.002700 |
| test4.test | RUN | 0.956621 | 4.073171 | 3.557089 | 0.002575 | 0.002709 | 0.002686 |
| test5.test | RUN | 1.019379 | 4.356659 | 3.902524 | 0.002737 | 0.002868 | 0.002642 |
| test6.test | RUN | 0.903022 | 3.722906 | 3.261969 | 0.002376 | 0.002578 | 0.002639 |
| test7.test | RUN | 0.878798 | 3.763672 | 3.326601 | 0.002523 | 0.002759 | 0.002590 |
| test8.test | RUN | 0.876713 | 3.638569 | 3.135532 | 0.002379 | 0.002640 | 0.002569 |
| test9.test | RUN | 0.946804 | 3.984617 | 3.586335 | 0.002772 | 0.002830 | 0.002602 |
| test10.test | RUN | 0.961325 | 3.776242 | 3.369387 | 0.002462 | 0.002770 | 0.002573 |
| test11.test | RUN | 0.931486 | 3.626329 | 3.317081 | 0.002516 | 0.002812 | 0.002578 |
| test1u.test | RUN | 0.858338 | 3.410068 | 2.807439 | 0.002880 | 0.002825 | 0.002612 |
| test2u.test | RUN | 1.116664 | 5.081213 | 4.478944 | 0.002761 | 0.002864 | 0.002765 |
| test3u.test | RUN | 1.149960 | 5.225450 | 4.646940 | 0.002542 | 0.002742 | 0.002690 |
| test4u.test | RUN | 0.946811 | 4.066370 | 3.574498 | 0.002746 | 0.002823 | 0.002669 |
| test5u.test | RUN | 1.045224 | 4.443043 | 4.055447 | 0.002462 | 0.002924 | 0.002714 |
| test6u.test | RUN | 0.946368 | 3.768733 | 3.376502 | 0.002833 | 0.002714 | 0.002710 |
| test7u.test | RUN | 0.922617 | 3.976772 | 3.489159 | 0.002880 | 0.002707 | 0.002660 |
| test8u.test | RUN | 0.901756 | 3.640606 | 3.138292 | 0.002643 | 0.002883 | 0.002604 |
| test9u.test | RUN | 0.978096 | 3.919308 | 3.898903 | 0.002507 | 0.002736 | 0.002837 |
| test10u.test | RUN | 0.935515 | 3.743155 | 3.877671 | 0.002593 | 0.002814 | 0.002767 |
| test11u.test | RUN | 0.919136 | 3.691312 | 4.089464 | 0.002858 | 0.002739 | 0.002773 |
| misc1.test | RUN | 1.640313 | 9.578706 | 8.330441 | 0.003491 | 0.003499 | 0.003433 |
| misc2.test | RUN | 1.720192 | 9.098815 | 7.715067 | 0.003183 | 0.003059 | 0.003059 |
| test_format.test | RUN | 1.695265 | 11.198040 | 8.521121 | 0.002907 | 0.002934 | 0.002752 |
| test_cycles.test | RUN | 0.787052 | 3.338530 | 2.651124 | 0.002655 | 0.002710 | 0.002675 |
| test_non_char.test | RUN | 1.450856 | 7.749419 | 6.426169 | 0.002903 | 0.002952 | 0.002707 |
| test_static.test | RUN | 0.751925 | 3.058687 | 2.357522 | 0.002783 | 0.002540 | 0.002676 |
| test_actions.test | RUN | 1.834275 | 9.227833 | 7.769833 | 0.003758 | 0.003745 | 0.003473 |
| test_assert.test | RUN | 1.048528 | 4.039176 | 3.466807 | 0.003131 | 0.002735 | 0.003040 |
| test_assert_with_placeholder.test | RUN | 0.981267 | 3.550693 | 3.164649 | 0.002898 | 0.002719 | 0.003131 |
| test_symbols.test | RUN | 1.911914 | 9.032149 | 8.621346 | 0.003746 | 0.003631 | 0.003705 |
| test_dynamic.test | RUN | 1.159317 | 6.154124 | 5.091559 | 0.003929 | 0.002766 | 0.002737 |
| test_dynamic_grammar.test | RUN | 1.245805 | 6.441212 | 5.234932 | 0.003105 | 0.002938 | 0.002720 |
| test_skip.test | RUN | 1.473397 | 6.668864 | 5.928934 | 0.002941 | 0.002793 | 0.002661 |
| multiple_defs.test | LINK | 2.318698 | 12.015737 | 10.219828 | — | — | — |
| test_basic_regex.test | COMPILE | 0.513396 | 0.952824 | 1.225490 | — | — | — |
| test_match_results.test | COMPILE | 0.346637 | 0.649651 | 0.802500 | — | — | — |
| test_regex_algorithms.test | COMPILE | 0.338373 | 0.671988 | 0.808204 | — | — | — |
| test_regex_compiler.test | COMPILE | 0.536671 | 1.075738 | 1.224278 | — | — | — |
| test_regex_constants.test | COMPILE | 0.028583 | 0.047723 | 0.051180 | — | — | — |
| test_regex_error.test | COMPILE | 0.190285 | 0.341448 | 0.443437 | — | — | — |
| test_regex_iterator.test | COMPILE | 0.339919 | 0.656521 | 0.788460 | — | — | — |
| test_regex_primitives.test | COMPILE | 0.500777 | 1.086055 | 1.142227 | — | — | — |
| test_regex_token_iterator.test | COMPILE | 0.313313 | 0.654603 | 0.779629 | — | — | — |
| test_regex_traits.test | COMPILE | 0.195873 | 0.351503 | 0.420380 | — | — | — |
| test_sub_match.test | COMPILE | 0.162063 | 0.285647 | 0.325116 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.059×† | 1.091×, 1.027× | 0.947× |
| clang | 1.012×† | 0.959×, 1.066× | 0.993× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026855).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| c_traits.test | 4 | 0.004522s | 0.006306s | 0.004695s | byte equality observed |
| c_traits_u.test | 4 | 0.004816s | 0.006537s | 0.004734s | byte equality observed |
| misc1.test | 10 | 0.001846s | 0.001809s | 0.001861s | byte equality observed |
| misc2.test | 10 | 0.001637s | 0.001632s | 0.001680s | byte equality observed |
| regress.test | 3 | 0.005488s | 0.007089s | 0.005240s | byte equality observed |
| regress_u.test | 3 | 0.005394s | 0.007379s | 0.005635s | byte equality observed |
| test1.test | 10 | 0.001480s | 0.001494s | 0.001524s | byte equality observed |
| test10.test | 10 | 0.001466s | 0.001485s | 0.001544s | byte equality observed |
| test10u.test | 10 | 0.001482s | 0.001464s | 0.001552s | byte equality observed |
| test11.test | 10 | 0.001480s | 0.001447s | 0.001521s | byte equality observed |
| test11u.test | 10 | 0.001479s | 0.001483s | 0.001538s | byte equality observed |
| test1u.test | 10 | 0.001477s | 0.001512s | 0.001570s | byte equality observed |
| test2.test | 10 | 0.001526s | 0.001542s | 0.001550s | byte equality observed |
| test2u.test | 10 | 0.001511s | 0.001581s | 0.001567s | byte equality observed |
| test3.test | 10 | 0.001495s | 0.001537s | 0.001519s | byte equality observed |
| test3u.test | 10 | 0.001498s | 0.001560s | 0.001571s | byte equality observed |
| test4.test | 10 | 0.001477s | 0.001469s | 0.001569s | byte equality observed |
| test4u.test | 10 | 0.001476s | 0.001499s | 0.001564s | byte equality observed |
| test5.test | 10 | 0.001519s | 0.001495s | 0.001610s | byte equality observed |
| test5u.test | 10 | 0.001524s | 0.001482s | 0.001671s | byte equality observed |
| test6.test | 10 | 0.001494s | 0.001458s | 0.001605s | byte equality observed |
| test6u.test | 10 | 0.001496s | 0.001481s | 0.001630s | byte equality observed |
| test7.test | 10 | 0.001547s | 0.001468s | 0.001608s | byte equality observed |
| test7u.test | 10 | 0.001575s | 0.001507s | 0.001654s | byte equality observed |
| test8.test | 10 | 0.001622s | 0.001473s | 0.001658s | byte equality observed |
| test8u.test | 10 | 0.001652s | 0.001488s | 0.001648s | byte equality observed |
| test9.test | 10 | 0.001668s | 0.001608s | 0.001647s | byte equality observed |
| test9u.test | 10 | 0.001640s | 0.001584s | 0.001636s | byte equality observed |
| test_actions.test | 9 | 0.002092s | 0.002009s | 0.002030s | byte equality observed |
| test_assert.test | 10 | 0.001772s | 0.001703s | 0.001698s | byte equality observed |
| test_assert_with_placeholder.test | 10 | 0.001658s | 0.001642s | 0.001636s | byte equality observed |
| test_cycles.test | 10 | 0.001723s | 0.001650s | 0.001649s | byte equality observed |
| test_dynamic.test | 10 | 0.001698s | 0.001628s | 0.001626s | byte equality observed |
| test_dynamic_grammar.test | 10 | 0.001789s | 0.001771s | 0.001754s | byte equality observed |
| test_format.test | 10 | 0.001758s | 0.001752s | 0.001707s | byte equality observed |
| test_non_char.test | 10 | 0.001737s | 0.001717s | 0.001687s | byte equality observed |
| test_skip.test | 10 | 0.001696s | 0.001649s | 0.001601s | byte equality observed |
| test_static.test | 10 | 0.001580s | 0.001481s | 0.001578s | byte equality observed |
| test_symbols.test | 10 | 0.001956s | 0.001890s | 0.001932s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026855).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
