# libs/json/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| array.test | RUN | 1.201490 | 6.571100 | 6.811111 | 0.003465 | 0.003474 | 0.003589 |
| basic_parser.test | RUN | 7.112139 | 140.752505 | 98.431153 | 0.845424 | 0.274914 | 1.128759 |
| conversion.test | RUN | 0.900045 | 2.508948 | 3.488251 | 0.002026 | 0.002288 | 0.002158 |
| doc_background.test | RUN | 0.892269 | 2.463373 | 3.558432 | 0.002134 | 0.002282 | 0.002067 |
| doc_forward_conversion.test | RUN | 0.894128 | 2.923878 | 4.007410 | 0.002072 | 0.002268 | 0.002152 |
| doc_parsing.test | RUN | 0.899387 | 2.616107 | 3.732917 | 0.002252 | 0.002377 | 0.002090 |
| doc_quick_look.test | RUN | 1.120732 | 3.252989 | 4.846616 | 0.001826 | 0.001924 | 0.001925 |
| doc_serializing.test | RUN | 0.809433 | 2.372559 | 3.443413 | 0.002145 | 0.001795 | 0.001764 |
| doc_storage_ptr.test | RUN | 0.811807 | 2.393377 | 3.474184 | 0.001948 | 0.002075 | 0.002116 |
| doc_uses_allocator.test | RUN | 0.825012 | 2.336056 | 3.404476 | 0.002088 | 0.002188 | 0.002117 |
| doc_using_numbers.test | RUN | 0.911602 | 2.485931 | 3.784935 | 0.001927 | 0.002166 | 0.002096 |
| double.test | RUN | 0.929871 | 3.992509 | 4.429827 | 2.251871 | 1.784322 | 2.195205 |
| error.test | RUN | 0.681998 | 2.397041 | 2.800094 | 0.002070 | 0.002211 | 0.002182 |
| fwd.test | RUN | 0.795700 | 2.495476 | 3.175647 | 0.001908 | 0.002320 | 0.002070 |
| json.test | RUN | 0.841680 | 2.871357 | 3.788932 | 0.001976 | 0.002419 | 0.002210 |
| kind.test | RUN | 0.622187 | 2.144263 | 2.742905 | 0.001964 | 0.002171 | 0.002270 |
| memory_resource.test | RUN | 0.869244 | 2.798048 | 3.581957 | 0.002170 | 0.002090 | 0.002012 |
| monotonic_resource.test | RUN | 0.828682 | 2.709947 | 3.606284 | 0.002940 | 0.002992 | 0.002950 |
| natvis.test | RUN | 0.849761 | 2.536284 | 3.174884 | 0.002217 | 0.002134 | 0.002190 |
| null_resource.test | RUN | 0.699695 | 2.232398 | 2.697950 | 0.002174 | 0.002340 | 0.002162 |
| object.test | RUN | 1.366142 | 7.528685 | 8.384104 | 0.010035 | 0.007997 | 0.008697 |
| parse.test | RUN | 0.910802 | 3.909249 | 3.785578 | 0.003118 | 0.003123 | 0.003087 |
| parser.test | RUN | 0.874326 | 3.309521 | 3.397573 | 0.002412 | 0.002538 | 0.002522 |
| pilfer.test | RUN | 0.474398 | 1.511792 | 1.739230 | 0.001954 | 0.001886 | 0.002069 |
| pointer.test | RUN | 0.996071 | 4.501600 | 4.461490 | 0.002421 | 0.002551 | 0.002323 |
| result_for.test | RUN | 0.851119 | 2.644257 | 3.682918 | 0.001910 | 0.002059 | 0.002123 |
| serialize.test | RUN | 0.850158 | 2.782890 | 3.412375 | 0.002340 | 0.002322 | 0.002372 |
| serializer.test | RUN | 1.325983 | 7.683326 | 7.432884 | 0.003558 | 0.003190 | 0.003495 |
| snippets.test | RUN | 2.068084 | 11.119894 | 11.550883 | 0.002496 | 0.002476 | 0.002411 |
| static_resource.test | RUN | 0.841795 | 2.603889 | 3.328647 | 0.002274 | 0.002272 | 0.002146 |
| storage_ptr.test | RUN | 0.875909 | 2.567828 | 3.376825 | 0.002173 | 0.002145 | 0.001984 |
| stream_parser.test | RUN | 1.010276 | 4.789871 | 4.666795 | 0.128476 | 0.170980 | 0.128234 |
| string.test | RUN | 1.712408 | 11.536308 | 10.547103 | 0.003799 | 0.002961 | 0.003646 |
| string_view.test | RUN | 0.513928 | 1.647402 | 2.091031 | 0.001801 | 0.002193 | 0.002166 |
| value.test | RUN | 1.698138 | 11.850319 | 10.374207 | 0.004868 | 0.004646 | 0.005395 |
| value_from.test | RUN | 1.562505 | 7.706006 | 8.406001 | 0.002639 | 0.002349 | 0.002421 |
| value_stack.test | RUN | 0.842213 | 2.415505 | 3.182066 | 0.002134 | 0.002308 | 0.002367 |
| value_to.test | RUN | 3.836330 | 22.209456 | 20.394947 | 0.004212 | 0.003904 | 0.003387 |
| value_ref.test | RUN | 0.965907 | 3.349620 | 4.338443 | 0.002342 | 0.002290 | 0.002276 |
| utf8.test | RUN | 0.720974 | 2.263577 | 2.881955 | 0.002278 | 0.001938 | 0.002254 |
| visit.test | RUN | 0.932991 | 3.322706 | 4.062293 | 0.002134 | 0.002098 | 0.002503 |
| d2s_intrinsics_test.test | RUN | 0.684853 | 2.156973 | 2.874614 | 0.002048 | 0.001922 | 0.002110 |
| d2s_table_test.test | RUN | 0.693168 | 2.349287 | 2.931232 | 0.002208 | 0.002123 | 0.001879 |
| d2s_test.test | RUN | 0.734631 | 2.902652 | 2.947872 | 0.002166 | 0.002366 | 0.002136 |
| parse_into.test | RUN | 21.789805 | 181.340982 | 123.025794 | 0.003426 | 0.003351 | 0.003231 |
| limits.test | RUN | 2.754810 | 16.674655 | 14.513202 | 0.002309 | 0.002533 | 0.002676 |
| header_only.test | RUN | 1.809735 | 12.857698 | 10.106594 | 0.002038 | 0.002004 | 0.002162 |
| no_exceptions.test | RUN | 2.603531 | 14.387109 | 13.710957 | 0.002133 | 0.001978 | 0.002164 |
| intrusive_macros.test | RUN | 2.136499 | 13.998146 | 11.477427 | 0.002404 | 0.002259 | 0.002180 |
| _gdb-printers.test | RUN | 0.397562 | 1.077331 | 1.623472 | 0.237442 | 0.274009 | 0.258908 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.836× | 0.827×, 0.845× | 0.985× |
| clang | 0.992× | 0.992×, 0.991× | 0.998× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274616).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| array.test | 7 | 0.002101s | 0.001846s | 0.002027s | native test oracle; output varies |
| basic_parser.test | 1 | 0.491961s | 0.297354s | 0.657113s | native test oracle; output varies |
| conversion.test | 10 | 0.001182s | 0.001191s | 0.001169s | byte equality observed |
| d2s_intrinsics_test.test | 10 | 0.001183s | 0.001187s | 0.001194s | byte equality observed |
| d2s_table_test.test | 10 | 0.001208s | 0.001228s | 0.001191s | byte equality observed |
| d2s_test.test | 10 | 0.001281s | 0.001275s | 0.001245s | byte equality observed |
| doc_background.test | 10 | 0.001220s | 0.001187s | 0.001167s | byte equality observed |
| doc_forward_conversion.test | 10 | 0.001247s | 0.001244s | 0.001219s | byte equality observed |
| doc_parsing.test | 10 | 0.001271s | 0.001259s | 0.001223s | byte equality observed |
| doc_quick_look.test | 10 | 0.001236s | 0.001225s | 0.001204s | byte equality observed |
| doc_serializing.test | 10 | 0.001217s | 0.001190s | 0.001183s | byte equality observed |
| doc_storage_ptr.test | 10 | 0.001218s | 0.001234s | 0.001193s | byte equality observed |
| doc_uses_allocator.test | 10 | 0.001210s | 0.001207s | 0.001175s | byte equality observed |
| doc_using_numbers.test | 10 | 0.001214s | 0.001184s | 0.001170s | byte equality observed |
| double.test | 1 | 1.317248s | 1.188701s | 1.167434s | native test oracle; output varies |
| error.test | 10 | 0.001301s | 0.001236s | 0.001294s | byte equality observed |
| fwd.test | 10 | 0.001190s | 0.001178s | 0.001208s | byte equality observed |
| header_only.test | 10 | 0.001186s | 0.001173s | 0.001183s | byte equality observed |
| intrusive_macros.test | 10 | 0.001250s | 0.001214s | 0.001230s | byte equality observed |
| json.test | 10 | 0.001221s | 0.001193s | 0.001212s | byte equality observed |
| kind.test | 10 | 0.001241s | 0.001228s | 0.001246s | byte equality observed |
| limits.test | 10 | 0.001460s | 0.001410s | 0.001427s | byte equality observed |
| memory_resource.test | 10 | 0.001225s | 0.001232s | 0.001230s | byte equality observed |
| monotonic_resource.test | 10 | 0.001786s | 0.001610s | 0.001720s | native test oracle; output varies |
| natvis.test | 10 | 0.001249s | 0.001245s | 0.001234s | byte equality observed |
| no_exceptions.test | 10 | 0.001221s | 0.001200s | 0.001253s | byte equality observed |
| null_resource.test | 10 | 0.001268s | 0.001243s | 0.001257s | byte equality observed |
| object.test | 4 | 0.005132s | 0.004094s | 0.004888s | native test oracle; output varies |
| parse.test | 10 | 0.001770s | 0.001676s | 0.001709s | byte equality observed |
| parse_into.test | 6 | 0.002806s | 0.002596s | 0.002555s | native test oracle; output varies |
| parser.test | 10 | 0.001374s | 0.001322s | 0.001394s | byte equality observed |
| pilfer.test | 10 | 0.001170s | 0.001164s | 0.001202s | byte equality observed |
| pointer.test | 10 | 0.001421s | 0.001387s | 0.001441s | byte equality observed |
| result_for.test | 10 | 0.001186s | 0.001174s | 0.001202s | byte equality observed |
| serialize.test | 10 | 0.001240s | 0.001238s | 0.001247s | byte equality observed |
| serializer.test | 9 | 0.002070s | 0.001977s | 0.002000s | native test oracle; output varies |
| snippets.test | 10 | 0.001417s | 0.001401s | 0.001418s | byte equality observed |
| static_resource.test | 10 | 0.001270s | 0.001274s | 0.001279s | byte equality observed |
| storage_ptr.test | 10 | 0.001238s | 0.001253s | 0.001252s | byte equality observed |
| stream_parser.test | 1 | 0.082714s | 0.078136s | 0.057271s | native test oracle; output varies |
| string.test | 9 | 0.002014s | 0.001728s | 0.002022s | native test oracle; output varies |
| string_view.test | 10 | 0.001165s | 0.001181s | 0.001193s | byte equality observed |
| utf8.test | 10 | 0.001163s | 0.001178s | 0.001197s | byte equality observed |
| value.test | 7 | 0.002662s | 0.002390s | 0.002965s | byte equality observed |
| value_from.test | 10 | 0.001449s | 0.001402s | 0.001391s | byte equality observed |
| value_ref.test | 10 | 0.001248s | 0.001255s | 0.001266s | byte equality observed |
| value_stack.test | 10 | 0.001233s | 0.001239s | 0.001239s | byte equality observed |
| value_to.test | 6 | 0.002271s | 0.002162s | 0.002173s | native test oracle; output varies |
| visit.test | 10 | 0.001223s | 0.001227s | 0.001238s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274616).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
