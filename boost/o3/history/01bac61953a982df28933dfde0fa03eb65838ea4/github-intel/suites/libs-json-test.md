# libs/json/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| array.test | RUN | 0.736581 | 4.235793 | 3.684811 | 0.002058 | 0.001780 | 0.001994 |
| basic_parser.test | RUN | 5.187568 | 102.528597 | 62.785191 | 0.405017 | 0.186714 | 0.419984 |
| conversion.test | RUN | 0.511756 | 1.546096 | 1.796192 | 0.001330 | 0.001251 | 0.001172 |
| doc_background.test | RUN | 0.523603 | 1.558508 | 1.775177 | 0.001155 | 0.001156 | 0.001125 |
| doc_forward_conversion.test | RUN | 0.546211 | 1.830598 | 2.032047 | 0.001331 | 0.001270 | 0.001325 |
| doc_parsing.test | RUN | 0.581965 | 1.685328 | 1.880660 | 0.001331 | 0.001263 | 0.001267 |
| doc_quick_look.test | RUN | 0.719252 | 2.052480 | 2.493596 | 0.001148 | 0.001072 | 0.001236 |
| doc_serializing.test | RUN | 0.502729 | 1.496288 | 1.705184 | 0.001261 | 0.001041 | 0.001162 |
| doc_storage_ptr.test | RUN | 0.499132 | 1.547928 | 1.737014 | 0.001368 | 0.001076 | 0.001385 |
| doc_uses_allocator.test | RUN | 0.506463 | 1.519466 | 1.724561 | 0.001437 | 0.001342 | 0.001216 |
| doc_using_numbers.test | RUN | 0.568589 | 1.577096 | 1.868120 | 0.001402 | 0.001227 | 0.001325 |
| double.test | RUN | 0.597400 | 2.755767 | 2.208295 | 1.337603 | 1.152997 | 1.304569 |
| error.test | RUN | 0.439533 | 1.518877 | 1.438194 | 0.001400 | 0.001294 | 0.001264 |
| fwd.test | RUN | 0.475993 | 1.557509 | 1.659069 | 0.001174 | 0.001311 | 0.001265 |
| json.test | RUN | 0.513429 | 1.804279 | 1.969237 | 0.001264 | 0.001393 | 0.001282 |
| kind.test | RUN | 0.372531 | 1.378152 | 1.384477 | 0.001182 | 0.001440 | 0.001204 |
| memory_resource.test | RUN | 0.544531 | 1.827810 | 1.890760 | 0.001244 | 0.001396 | 0.001362 |
| monotonic_resource.test | RUN | 0.484100 | 1.711756 | 1.793306 | 0.001741 | 0.001881 | 0.001855 |
| natvis.test | RUN | 0.474536 | 1.629695 | 1.766750 | 0.001226 | 0.001534 | 0.001375 |
| null_resource.test | RUN | 0.414067 | 1.419123 | 1.410106 | 0.001344 | 0.001494 | 0.001276 |
| object.test | RUN | 0.874624 | 5.093274 | 4.462445 | 0.005433 | 0.004310 | 0.005171 |
| parse.test | RUN | 0.550917 | 2.532853 | 2.013377 | 0.001742 | 0.001785 | 0.001884 |
| parser.test | RUN | 0.533221 | 2.149826 | 1.930397 | 0.001370 | 0.001433 | 0.001497 |
| pilfer.test | RUN | 0.281171 | 0.965487 | 0.923273 | 0.001182 | 0.001186 | 0.001304 |
| pointer.test | RUN | 0.624226 | 2.895654 | 2.500745 | 0.001670 | 0.001526 | 0.001449 |
| result_for.test | RUN | 0.515403 | 1.721600 | 1.828056 | 0.001161 | 0.001369 | 0.001182 |
| serialize.test | RUN | 0.522017 | 1.779282 | 1.803098 | 0.001418 | 0.001303 | 0.001240 |
| serializer.test | RUN | 0.857433 | 5.204025 | 4.130034 | 0.002105 | 0.002052 | 0.002144 |
| snippets.test | RUN | 1.365663 | 7.385792 | 6.298252 | 0.001501 | 0.001615 | 0.001729 |
| static_resource.test | RUN | 0.499173 | 1.652975 | 1.681598 | 0.001286 | 0.001393 | 0.001446 |
| storage_ptr.test | RUN | 0.519831 | 1.750728 | 1.855076 | 0.001213 | 0.001407 | 0.001389 |
| stream_parser.test | RUN | 0.640703 | 3.316840 | 2.671301 | 0.075252 | 0.072048 | 0.072225 |
| string.test | RUN | 1.154629 | 8.408779 | 6.818865 | 0.002272 | 0.002003 | 0.002093 |
| string_view.test | RUN | 0.310486 | 1.034622 | 1.062473 | 0.001178 | 0.001379 | 0.001414 |
| value.test | RUN | 1.111933 | 8.478439 | 5.472728 | 0.002621 | 0.002571 | 0.003086 |
| value_from.test | RUN | 0.987329 | 5.306254 | 4.857611 | 0.001512 | 0.001609 | 0.001475 |
| value_stack.test | RUN | 0.500619 | 1.542465 | 1.709009 | 0.001466 | 0.001374 | 0.001370 |
| value_to.test | RUN | 2.608452 | 16.551562 | 12.023714 | 0.002500 | 0.002525 | 0.002644 |
| value_ref.test | RUN | 0.560426 | 2.264200 | 2.199435 | 0.001406 | 0.001390 | 0.001315 |
| utf8.test | RUN | 0.430979 | 1.448558 | 1.483168 | 0.001250 | 0.001336 | 0.001230 |
| visit.test | RUN | 0.557225 | 2.330420 | 2.084630 | 0.001232 | 0.001423 | 0.001390 |
| d2s_intrinsics_test.test | RUN | 0.416570 | 1.361912 | 1.417140 | 0.001126 | 0.001151 | 0.001431 |
| d2s_table_test.test | RUN | 0.423757 | 1.509103 | 1.494074 | 0.001257 | 0.001419 | 0.001422 |
| d2s_test.test | RUN | 0.446402 | 1.839694 | 1.689643 | 0.001338 | 0.001436 | 0.001419 |
| parse_into.test | RUN | 16.900530 | 134.022258 | 81.810483 | 0.002116 | 0.002176 | 0.002310 |
| limits.test | RUN | 1.809368 | 11.919732 | 8.401653 | 0.001367 | 0.001646 | 0.001637 |
| header_only.test | RUN | 1.201159 | 9.051733 | 5.652953 | 0.001304 | 0.001246 | 0.001188 |
| no_exceptions.test | RUN | 1.724756 | 10.233551 | 7.712017 | 0.001379 | 0.001356 | 0.001200 |
| intrusive_macros.test | RUN | 1.436842 | 9.772703 | 6.812572 | 0.001338 | 0.001338 | 0.001295 |
| _gdb-printers.test | RUN | 0.228146 | 0.657919 | 0.781629 | 0.149846 | 0.247579 | 0.176233 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.826× | 0.826×, 0.825× | 1.008× |
| clang | 1.071× | 1.072×, 1.070× | 0.993× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814672).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| array.test | 10 | 0.001417s | 0.001278s | 0.001359s | byte equality observed |
| basic_parser.test | 1 | 0.291608s | 0.189408s | 0.431926s | native test oracle; output varies |
| conversion.test | 10 | 0.000735s | 0.000740s | 0.000746s | byte equality observed |
| d2s_intrinsics_test.test | 10 | 0.000736s | 0.000753s | 0.000741s | byte equality observed |
| d2s_table_test.test | 10 | 0.000756s | 0.000762s | 0.000768s | byte equality observed |
| d2s_test.test | 10 | 0.000807s | 0.000795s | 0.000792s | byte equality observed |
| doc_background.test | 10 | 0.000757s | 0.000741s | 0.000738s | byte equality observed |
| doc_forward_conversion.test | 10 | 0.000782s | 0.000769s | 0.000768s | byte equality observed |
| doc_parsing.test | 10 | 0.000797s | 0.000790s | 0.000778s | byte equality observed |
| doc_quick_look.test | 10 | 0.000762s | 0.000773s | 0.000766s | byte equality observed |
| doc_serializing.test | 10 | 0.000766s | 0.000749s | 0.000742s | byte equality observed |
| doc_storage_ptr.test | 10 | 0.000757s | 0.000774s | 0.000763s | byte equality observed |
| doc_uses_allocator.test | 10 | 0.000774s | 0.000757s | 0.000758s | byte equality observed |
| doc_using_numbers.test | 10 | 0.000769s | 0.000756s | 0.000747s | byte equality observed |
| double.test | 1 | 0.922145s | 0.813600s | 0.865660s | native test oracle; output varies |
| error.test | 10 | 0.000831s | 0.000810s | 0.000840s | byte equality observed |
| fwd.test | 10 | 0.000766s | 0.000767s | 0.000769s | byte equality observed |
| header_only.test | 10 | 0.000758s | 0.000752s | 0.000745s | byte equality observed |
| intrusive_macros.test | 10 | 0.000790s | 0.000775s | 0.000771s | byte equality observed |
| json.test | 10 | 0.000774s | 0.000772s | 0.000767s | byte equality observed |
| kind.test | 10 | 0.000796s | 0.000782s | 0.000782s | byte equality observed |
| limits.test | 10 | 0.000947s | 0.000917s | 0.000921s | byte equality observed |
| memory_resource.test | 10 | 0.000793s | 0.000786s | 0.000781s | byte equality observed |
| monotonic_resource.test | 10 | 0.001179s | 0.001159s | 0.001152s | byte equality observed |
| natvis.test | 10 | 0.000803s | 0.000787s | 0.000793s | byte equality observed |
| no_exceptions.test | 10 | 0.000796s | 0.000770s | 0.000781s | byte equality observed |
| null_resource.test | 10 | 0.000815s | 0.000806s | 0.000804s | byte equality observed |
| object.test | 6 | 0.003480s | 0.002843s | 0.003334s | native test oracle; output varies |
| parse.test | 10 | 0.001144s | 0.001112s | 0.001101s | byte equality observed |
| parse_into.test | 8 | 0.001921s | 0.001823s | 0.001707s | native test oracle; output varies |
| parser.test | 10 | 0.000903s | 0.000879s | 0.000882s | byte equality observed |
| pilfer.test | 10 | 0.000763s | 0.000755s | 0.000754s | byte equality observed |
| pointer.test | 10 | 0.000933s | 0.000920s | 0.000922s | byte equality observed |
| result_for.test | 10 | 0.000773s | 0.000778s | 0.000760s | byte equality observed |
| serialize.test | 10 | 0.000801s | 0.000796s | 0.000800s | byte equality observed |
| serializer.test | 10 | 0.001420s | 0.001376s | 0.001371s | byte equality observed |
| snippets.test | 10 | 0.000948s | 0.000914s | 0.000906s | byte equality observed |
| static_resource.test | 10 | 0.000824s | 0.000826s | 0.000813s | byte equality observed |
| storage_ptr.test | 10 | 0.000817s | 0.000808s | 0.000797s | byte equality observed |
| stream_parser.test | 1 | 0.044489s | 0.040274s | 0.043363s | native test oracle; output varies |
| string.test | 10 | 0.001363s | 0.001172s | 0.001326s | byte equality observed |
| string_view.test | 10 | 0.000764s | 0.000757s | 0.000752s | byte equality observed |
| utf8.test | 10 | 0.000766s | 0.000768s | 0.000760s | byte equality observed |
| value.test | 10 | 0.001743s | 0.001604s | 0.001985s | native test oracle; output varies |
| value_from.test | 10 | 0.000956s | 0.000897s | 0.000883s | byte equality observed |
| value_ref.test | 10 | 0.000811s | 0.000817s | 0.000803s | byte equality observed |
| value_stack.test | 10 | 0.000801s | 0.000797s | 0.000785s | byte equality observed |
| value_to.test | 10 | 0.001498s | 0.001452s | 0.001439s | byte equality observed |
| visit.test | 10 | 0.000788s | 0.000789s | 0.000777s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814672).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
