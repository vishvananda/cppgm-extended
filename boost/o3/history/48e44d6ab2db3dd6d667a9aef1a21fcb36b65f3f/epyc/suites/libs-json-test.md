# libs/json/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| array.test | RUN | 1.312385 | 6.630436 | 6.813537 | 0.003454 | 0.003687 | 0.003649 |
| basic_parser.test | RUN | 6.748893 | 144.042115 | 100.947800 | 0.977131 | 0.273541 | 1.127366 |
| conversion.test | RUN | 1.004989 | 2.531672 | 3.509095 | 0.001868 | 0.002334 | 0.002156 |
| doc_background.test | RUN | 0.974443 | 2.476817 | 3.553853 | 0.002082 | 0.002297 | 0.002335 |
| doc_forward_conversion.test | RUN | 0.957002 | 2.944020 | 3.984126 | 0.002248 | 0.002325 | 0.002269 |
| doc_parsing.test | RUN | 0.964689 | 2.611820 | 3.706494 | 0.002232 | 0.002216 | 0.002226 |
| doc_quick_look.test | RUN | 1.157351 | 3.285802 | 4.941002 | 0.001932 | 0.002042 | 0.002380 |
| doc_serializing.test | RUN | 0.904695 | 2.384055 | 3.513416 | 0.001900 | 0.001674 | 0.002158 |
| doc_storage_ptr.test | RUN | 0.876930 | 2.391973 | 3.462054 | 0.002082 | 0.002134 | 0.002521 |
| doc_uses_allocator.test | RUN | 0.922556 | 2.325255 | 3.432012 | 0.002312 | 0.001887 | 0.002150 |
| doc_using_numbers.test | RUN | 0.946705 | 2.487222 | 3.784670 | 0.002050 | 0.002175 | 0.002238 |
| double.test | RUN | 1.053811 | 4.229232 | 4.547671 | 2.066757 | 2.019194 | 2.172259 |
| error.test | RUN | 0.749731 | 2.292291 | 2.683276 | 0.002090 | 0.002318 | 0.002056 |
| fwd.test | RUN | 0.900431 | 2.313870 | 3.107005 | 0.002034 | 0.002254 | 0.002183 |
| json.test | RUN | 0.967622 | 2.665248 | 4.016280 | 0.002024 | 0.002120 | 0.002380 |
| kind.test | RUN | 0.730268 | 2.257861 | 2.741816 | 0.002131 | 0.002368 | 0.002540 |
| memory_resource.test | RUN | 0.996319 | 2.810868 | 3.524202 | 0.002014 | 0.002088 | 0.002179 |
| monotonic_resource.test | RUN | 0.938637 | 2.744781 | 3.538487 | 0.003216 | 0.002788 | 0.003210 |
| natvis.test | RUN | 0.943837 | 2.503047 | 3.225024 | 0.002082 | 0.002056 | 0.002016 |
| null_resource.test | RUN | 0.771243 | 2.260430 | 2.639187 | 0.002278 | 0.002434 | 0.002387 |
| object.test | RUN | 1.442934 | 7.688196 | 8.475269 | 0.009348 | 0.007354 | 0.009527 |
| parse.test | RUN | 0.999518 | 3.945984 | 3.735140 | 0.003023 | 0.003168 | 0.003064 |
| parser.test | RUN | 0.978224 | 3.341897 | 3.393107 | 0.002394 | 0.002512 | 0.002428 |
| pilfer.test | RUN | 0.514987 | 1.514487 | 1.599593 | 0.001998 | 0.001941 | 0.001955 |
| pointer.test | RUN | 1.062164 | 4.475549 | 4.537881 | 0.002415 | 0.002722 | 0.002402 |
| result_for.test | RUN | 0.919729 | 2.696721 | 3.738299 | 0.002092 | 0.002270 | 0.002270 |
| serialize.test | RUN | 0.940834 | 2.800608 | 3.369051 | 0.002364 | 0.002130 | 0.002203 |
| serializer.test | RUN | 1.442595 | 8.176879 | 7.946363 | 0.003740 | 0.003464 | 0.003813 |
| snippets.test | RUN | 2.063764 | 10.814114 | 11.175342 | 0.002895 | 0.002386 | 0.002777 |
| static_resource.test | RUN | 0.915434 | 2.530802 | 3.048853 | 0.002251 | 0.002125 | 0.002246 |
| storage_ptr.test | RUN | 0.941225 | 2.566675 | 3.467471 | 0.002030 | 0.002250 | 0.002324 |
| stream_parser.test | RUN | 1.093284 | 4.581246 | 4.434055 | 0.136769 | 0.151437 | 0.111238 |
| string.test | RUN | 1.714560 | 11.839166 | 11.748649 | 0.003652 | 0.003066 | 0.003687 |
| string_view.test | RUN | 0.576474 | 1.478551 | 1.811302 | 0.002042 | 0.002027 | 0.002313 |
| value.test | RUN | 1.742467 | 12.942636 | 9.926974 | 0.004437 | 0.004078 | 0.005749 |
| value_from.test | RUN | 1.608474 | 6.986930 | 8.235093 | 0.002446 | 0.002416 | 0.002743 |
| value_stack.test | RUN | 0.907651 | 2.307751 | 3.279521 | 0.001978 | 0.002334 | 0.001952 |
| value_to.test | RUN | 3.527128 | 23.576941 | 22.418582 | 0.004170 | 0.004213 | 0.004247 |
| value_ref.test | RUN | 1.032883 | 3.392801 | 4.255853 | 0.002332 | 0.002318 | 0.002300 |
| utf8.test | RUN | 0.803976 | 2.030364 | 2.775343 | 0.002187 | 0.002013 | 0.001928 |
| visit.test | RUN | 0.994596 | 3.456380 | 4.000949 | 0.002301 | 0.001976 | 0.002226 |
| d2s_intrinsics_test.test | RUN | 0.757415 | 2.024542 | 2.399164 | 0.002045 | 0.002290 | 0.001999 |
| d2s_table_test.test | RUN | 0.768927 | 2.350142 | 2.644512 | 0.002046 | 0.002088 | 0.002276 |
| d2s_test.test | RUN | 0.799513 | 2.698417 | 3.259253 | 0.002249 | 0.002213 | 0.002268 |
| parse_into.test | RUN | 21.122048 | 183.428123 | 122.305272 | 0.003328 | 0.003377 | 0.003264 |
| limits.test | RUN | 2.906549 | 16.187517 | 12.913491 | 0.002680 | 0.002182 | 0.002332 |
| header_only.test | RUN | 1.725724 | 12.537586 | 10.697013 | 0.001983 | 0.002094 | 0.002029 |
| no_exceptions.test | RUN | 2.635166 | 13.777770 | 13.181601 | 0.002037 | 0.002244 | 0.002170 |
| intrusive_macros.test | RUN | 2.206741 | 14.381373 | 11.501484 | 0.002035 | 0.001996 | 0.002298 |
| _gdb-printers.test | RUN | 0.419233 | 1.014804 | 1.629923 | 0.229520 | 0.392538 | 0.265521 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.784× | 0.772×, 0.796× | 1.013× |
| clang | 0.942× | 0.946×, 0.938× | 0.986× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| array.test | 7 | 0.002119s | 0.001851s | 0.002034s | native test oracle; output varies |
| basic_parser.test | 1 | 0.572438s | 0.298680s | 0.648479s | native test oracle; output varies |
| conversion.test | 10 | 0.001215s | 0.001229s | 0.001214s | byte equality observed |
| d2s_intrinsics_test.test | 10 | 0.001217s | 0.001222s | 0.001203s | byte equality observed |
| d2s_table_test.test | 10 | 0.001231s | 0.001246s | 0.001243s | byte equality observed |
| d2s_test.test | 10 | 0.001293s | 0.001299s | 0.001295s | byte equality observed |
| doc_background.test | 10 | 0.001209s | 0.001226s | 0.001220s | byte equality observed |
| doc_forward_conversion.test | 10 | 0.001270s | 0.001277s | 0.001275s | byte equality observed |
| doc_parsing.test | 10 | 0.001282s | 0.001269s | 0.001274s | byte equality observed |
| doc_quick_look.test | 10 | 0.001247s | 0.001234s | 0.001255s | byte equality observed |
| doc_serializing.test | 10 | 0.001227s | 0.001210s | 0.001217s | byte equality observed |
| doc_storage_ptr.test | 10 | 0.001250s | 0.001233s | 0.001236s | byte equality observed |
| doc_uses_allocator.test | 10 | 0.001251s | 0.001201s | 0.001219s | byte equality observed |
| doc_using_numbers.test | 10 | 0.001225s | 0.001208s | 0.001222s | byte equality observed |
| double.test | 1 | 1.334503s | 1.179714s | 1.153458s | native test oracle; output varies |
| error.test | 10 | 0.001335s | 0.001256s | 0.001368s | byte equality observed |
| fwd.test | 10 | 0.001216s | 0.001203s | 0.001225s | byte equality observed |
| header_only.test | 10 | 0.001206s | 0.001195s | 0.001208s | byte equality observed |
| intrusive_macros.test | 10 | 0.001255s | 0.001269s | 0.001240s | byte equality observed |
| json.test | 10 | 0.001225s | 0.001241s | 0.001244s | byte equality observed |
| kind.test | 10 | 0.001247s | 0.001268s | 0.001274s | byte equality observed |
| limits.test | 10 | 0.001473s | 0.001448s | 0.001458s | byte equality observed |
| memory_resource.test | 10 | 0.001251s | 0.001278s | 0.001265s | byte equality observed |
| monotonic_resource.test | 10 | 0.001796s | 0.001630s | 0.001729s | native test oracle; output varies |
| natvis.test | 10 | 0.001259s | 0.001279s | 0.001267s | byte equality observed |
| no_exceptions.test | 10 | 0.001255s | 0.001267s | 0.001256s | byte equality observed |
| null_resource.test | 10 | 0.001277s | 0.001293s | 0.001279s | byte equality observed |
| object.test | 4 | 0.005132s | 0.004146s | 0.004907s | native test oracle; output varies |
| parse.test | 10 | 0.001728s | 0.001728s | 0.001736s | byte equality observed |
| parse_into.test | 6 | 0.002807s | 0.002629s | 0.002561s | native test oracle; output varies |
| parser.test | 10 | 0.001406s | 0.001401s | 0.001397s | byte equality observed |
| pilfer.test | 10 | 0.001206s | 0.001187s | 0.001221s | byte equality observed |
| pointer.test | 10 | 0.001448s | 0.001419s | 0.001456s | byte equality observed |
| result_for.test | 10 | 0.001199s | 0.001199s | 0.001221s | byte equality observed |
| serialize.test | 10 | 0.001260s | 0.001264s | 0.001280s | byte equality observed |
| serializer.test | 9 | 0.002088s | 0.002008s | 0.002020s | native test oracle; output varies |
| snippets.test | 10 | 0.001437s | 0.001440s | 0.001460s | byte equality observed |
| static_resource.test | 10 | 0.001290s | 0.001296s | 0.001317s | byte equality observed |
| storage_ptr.test | 10 | 0.001269s | 0.001282s | 0.001273s | byte equality observed |
| stream_parser.test | 1 | 0.087003s | 0.078325s | 0.057387s | native test oracle; output varies |
| string.test | 6 | 0.002097s | 0.001768s | 0.002037s | native test oracle; output varies |
| string_view.test | 10 | 0.001216s | 0.001222s | 0.001210s | byte equality observed |
| utf8.test | 10 | 0.001201s | 0.001248s | 0.001211s | byte equality observed |
| value.test | 7 | 0.002701s | 0.002423s | 0.003010s | native test oracle; output varies |
| value_from.test | 10 | 0.001420s | 0.001426s | 0.001424s | byte equality observed |
| value_ref.test | 10 | 0.001279s | 0.001300s | 0.001297s | byte equality observed |
| value_stack.test | 10 | 0.001263s | 0.001266s | 0.001244s | byte equality observed |
| value_to.test | 9 | 0.002229s | 0.002195s | 0.002172s | native test oracle; output varies |
| visit.test | 10 | 0.001280s | 0.001267s | 0.001232s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
