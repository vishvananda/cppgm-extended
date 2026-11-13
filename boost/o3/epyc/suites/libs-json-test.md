# libs/json/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| array.test | RUN | 1.158590 | 6.496112 | 6.808562 | 0.003298 | 0.002958 | 0.003621 |
| basic_parser.test | RUN | 7.075146 | 141.696622 | 98.862161 | 0.860207 | 0.274263 | 1.132077 |
| conversion.test | RUN | 0.887986 | 2.471404 | 3.518194 | 0.001935 | 0.001916 | 0.002108 |
| doc_background.test | RUN | 0.887190 | 2.472662 | 3.550300 | 0.001983 | 0.001553 | 0.002142 |
| doc_forward_conversion.test | RUN | 0.866178 | 2.919885 | 3.988588 | 0.002104 | 0.001980 | 0.002136 |
| doc_parsing.test | RUN | 0.904972 | 2.593216 | 3.657364 | 0.002213 | 0.001948 | 0.002049 |
| doc_quick_look.test | RUN | 1.121428 | 3.239228 | 4.947608 | 0.002074 | 0.002001 | 0.002204 |
| doc_serializing.test | RUN | 0.797588 | 2.430663 | 3.362624 | 0.001911 | 0.001807 | 0.002120 |
| doc_storage_ptr.test | RUN | 0.789564 | 2.485821 | 3.440075 | 0.002191 | 0.002269 | 0.002166 |
| doc_uses_allocator.test | RUN | 0.808943 | 2.349908 | 3.408983 | 0.002046 | 0.001910 | 0.002294 |
| doc_using_numbers.test | RUN | 0.895839 | 2.419636 | 3.703581 | 0.001810 | 0.002216 | 0.002097 |
| double.test | RUN | 0.914606 | 3.963434 | 4.474706 | 2.040893 | 1.784899 | 1.892951 |
| error.test | RUN | 0.683369 | 2.415311 | 2.884115 | 0.002065 | 0.002179 | 0.002320 |
| fwd.test | RUN | 0.812407 | 2.525580 | 3.708545 | 0.002058 | 0.002058 | 0.002092 |
| json.test | RUN | 0.865364 | 2.870971 | 3.849850 | 0.002115 | 0.002077 | 0.002019 |
| kind.test | RUN | 0.657587 | 2.132712 | 2.757304 | 0.002194 | 0.002216 | 0.002290 |
| memory_resource.test | RUN | 0.858186 | 2.792229 | 3.579418 | 0.002095 | 0.002141 | 0.002241 |
| monotonic_resource.test | RUN | 0.822312 | 2.599606 | 3.315302 | 0.003180 | 0.002953 | 0.002907 |
| natvis.test | RUN | 0.844254 | 2.603221 | 3.541836 | 0.002146 | 0.002296 | 0.002192 |
| null_resource.test | RUN | 0.689264 | 2.226912 | 2.654572 | 0.002144 | 0.002195 | 0.002292 |
| object.test | RUN | 1.353670 | 7.634563 | 7.172089 | 0.009929 | 0.006966 | 0.008340 |
| parse.test | RUN | 0.900104 | 3.887620 | 4.149550 | 0.003090 | 0.003030 | 0.003064 |
| parser.test | RUN | 0.880827 | 3.335060 | 3.829767 | 0.002341 | 0.002315 | 0.002088 |
| pilfer.test | RUN | 0.455967 | 1.448559 | 1.803673 | 0.001950 | 0.001931 | 0.001777 |
| pointer.test | RUN | 0.994486 | 4.469193 | 4.397967 | 0.002380 | 0.002480 | 0.002498 |
| result_for.test | RUN | 0.835248 | 2.738492 | 3.699273 | 0.001871 | 0.002123 | 0.002236 |
| serialize.test | RUN | 0.846104 | 2.765671 | 3.318664 | 0.002135 | 0.002298 | 0.002077 |
| serializer.test | RUN | 1.309000 | 8.103779 | 8.082399 | 0.003491 | 0.003672 | 0.003183 |
| snippets.test | RUN | 2.041688 | 10.557948 | 10.152250 | 0.002484 | 0.002575 | 0.002250 |
| static_resource.test | RUN | 0.813432 | 2.425570 | 2.979253 | 0.002195 | 0.002213 | 0.002255 |
| storage_ptr.test | RUN | 0.870349 | 2.667408 | 3.678009 | 0.002587 | 0.002205 | 0.002098 |
| stream_parser.test | RUN | 1.008592 | 4.463929 | 5.183319 | 0.138984 | 0.151482 | 0.131514 |
| string.test | RUN | 1.652380 | 11.715820 | 10.454857 | 0.004382 | 0.002982 | 0.002985 |
| string_view.test | RUN | 0.511135 | 1.442337 | 1.997234 | 0.002027 | 0.001958 | 0.002240 |
| value.test | RUN | 1.703838 | 12.596074 | 10.400965 | 0.004763 | 0.004344 | 0.005346 |
| value_from.test | RUN | 1.557800 | 6.937712 | 8.347290 | 0.002442 | 0.002314 | 0.002486 |
| value_stack.test | RUN | 0.829214 | 2.298909 | 3.217874 | 0.002040 | 0.002158 | 0.002383 |
| value_to.test | RUN | 3.740479 | 23.576300 | 21.470768 | 0.004118 | 0.004033 | 0.004000 |
| value_ref.test | RUN | 0.921446 | 3.292089 | 4.291987 | 0.002350 | 0.002132 | 0.002212 |
| utf8.test | RUN | 0.680790 | 2.173872 | 2.842366 | 0.002122 | 0.002024 | 0.001766 |
| visit.test | RUN | 0.923671 | 3.530641 | 3.743918 | 0.002182 | 0.002072 | 0.002166 |
| d2s_intrinsics_test.test | RUN | 0.678603 | 1.995978 | 2.530321 | 0.002015 | 0.002058 | 0.002114 |
| d2s_table_test.test | RUN | 0.691421 | 2.253539 | 2.584790 | 0.002147 | 0.002210 | 0.002065 |
| d2s_test.test | RUN | 0.732919 | 2.673621 | 3.208530 | 0.002117 | 0.002198 | 0.002040 |
| parse_into.test | RUN | 21.316913 | 182.094602 | 120.297781 | 0.003342 | 0.003195 | 0.003121 |
| limits.test | RUN | 2.690719 | 15.615418 | 13.940268 | 0.002576 | 0.002350 | 0.002523 |
| header_only.test | RUN | 1.809543 | 12.680333 | 9.100359 | 0.002100 | 0.001803 | 0.002095 |
| no_exceptions.test | RUN | 2.554513 | 14.789093 | 13.594624 | 0.002124 | 0.002156 | 0.002230 |
| intrusive_macros.test | RUN | 2.106933 | 13.473806 | 12.102084 | 0.002148 | 0.002230 | 0.002166 |
| _gdb-printers.test | RUN | 0.389910 | 1.065558 | 1.546322 | 0.242735 | 0.385990 | 0.267452 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.830× | 0.835×, 0.825× | 1.001× |
| clang | 0.998× | 0.998×, 0.997× | 0.991× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824563).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| array.test | 8 | 0.002168s | 0.001917s | 0.002098s | native test oracle; output varies |
| basic_parser.test | 1 | 0.497216s | 0.305982s | 0.649518s | native test oracle; output varies |
| conversion.test | 10 | 0.001173s | 0.001181s | 0.001174s | byte equality observed |
| d2s_intrinsics_test.test | 10 | 0.001199s | 0.001152s | 0.001158s | byte equality observed |
| d2s_table_test.test | 10 | 0.001214s | 0.001193s | 0.001205s | byte equality observed |
| d2s_test.test | 10 | 0.001271s | 0.001240s | 0.001224s | byte equality observed |
| doc_background.test | 10 | 0.001227s | 0.001178s | 0.001161s | byte equality observed |
| doc_forward_conversion.test | 10 | 0.001246s | 0.001214s | 0.001193s | byte equality observed |
| doc_parsing.test | 10 | 0.001259s | 0.001243s | 0.001191s | byte equality observed |
| doc_quick_look.test | 10 | 0.001221s | 0.001207s | 0.001185s | byte equality observed |
| doc_serializing.test | 10 | 0.001231s | 0.001194s | 0.001162s | byte equality observed |
| doc_storage_ptr.test | 10 | 0.001219s | 0.001235s | 0.001211s | byte equality observed |
| doc_uses_allocator.test | 10 | 0.001223s | 0.001170s | 0.001166s | byte equality observed |
| doc_using_numbers.test | 10 | 0.001218s | 0.001172s | 0.001166s | byte equality observed |
| double.test | 1 | 1.310113s | 1.179043s | 1.166329s | native test oracle; output varies |
| error.test | 10 | 0.001280s | 0.001286s | 0.001288s | byte equality observed |
| fwd.test | 10 | 0.001168s | 0.001189s | 0.001197s | byte equality observed |
| header_only.test | 10 | 0.001155s | 0.001156s | 0.001180s | byte equality observed |
| intrusive_macros.test | 10 | 0.001213s | 0.001204s | 0.001213s | byte equality observed |
| json.test | 10 | 0.001197s | 0.001199s | 0.001197s | byte equality observed |
| kind.test | 10 | 0.001227s | 0.001230s | 0.001233s | byte equality observed |
| limits.test | 10 | 0.001434s | 0.001409s | 0.001402s | byte equality observed |
| memory_resource.test | 10 | 0.001213s | 0.001222s | 0.001217s | byte equality observed |
| monotonic_resource.test | 10 | 0.001756s | 0.001599s | 0.001720s | byte equality observed |
| natvis.test | 10 | 0.001206s | 0.001222s | 0.001230s | byte equality observed |
| no_exceptions.test | 10 | 0.001221s | 0.001222s | 0.001221s | byte equality observed |
| null_resource.test | 10 | 0.001259s | 0.001252s | 0.001248s | byte equality observed |
| object.test | 4 | 0.005159s | 0.004060s | 0.004850s | native test oracle; output varies |
| parse.test | 10 | 0.001773s | 0.001703s | 0.001730s | byte equality observed |
| parse_into.test | 7 | 0.002740s | 0.002599s | 0.002529s | native test oracle; output varies |
| parser.test | 9 | 0.001377s | 0.001360s | 0.001351s | byte equality observed |
| pilfer.test | 10 | 0.001174s | 0.001179s | 0.001186s | byte equality observed |
| pointer.test | 10 | 0.001412s | 0.001405s | 0.001407s | byte equality observed |
| result_for.test | 10 | 0.001178s | 0.001201s | 0.001184s | byte equality observed |
| serialize.test | 10 | 0.001246s | 0.001254s | 0.001227s | byte equality observed |
| serializer.test | 9 | 0.002074s | 0.002028s | 0.001992s | native test oracle; output varies |
| snippets.test | 10 | 0.001433s | 0.001442s | 0.001400s | byte equality observed |
| static_resource.test | 10 | 0.001295s | 0.001307s | 0.001265s | byte equality observed |
| storage_ptr.test | 10 | 0.001255s | 0.001266s | 0.001259s | byte equality observed |
| stream_parser.test | 1 | 0.084133s | 0.078945s | 0.057371s | native test oracle; output varies |
| string.test | 6 | 0.002045s | 0.001776s | 0.001974s | native test oracle; output varies |
| string_view.test | 10 | 0.001179s | 0.001181s | 0.001202s | byte equality observed |
| utf8.test | 10 | 0.001186s | 0.001220s | 0.001180s | byte equality observed |
| value.test | 6 | 0.002694s | 0.002392s | 0.002988s | native test oracle; output varies |
| value_from.test | 10 | 0.001474s | 0.001378s | 0.001384s | byte equality observed |
| value_ref.test | 10 | 0.001262s | 0.001256s | 0.001246s | byte equality observed |
| value_stack.test | 10 | 0.001237s | 0.001230s | 0.001224s | byte equality observed |
| value_to.test | 7 | 0.002283s | 0.002172s | 0.002168s | native test oracle; output varies |
| visit.test | 10 | 0.001231s | 0.001223s | 0.001213s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824563).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
