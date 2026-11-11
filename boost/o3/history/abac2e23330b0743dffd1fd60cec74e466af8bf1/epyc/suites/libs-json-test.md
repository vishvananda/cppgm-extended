# libs/json/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| array.test | RUN | 1.374242 | 6.752118 | 7.126203 | 0.003790 | 0.002953 | 0.003796 |
| basic_parser.test | RUN | 6.962664 | 162.414966 | 113.181315 | 0.967526 | 0.270467 | 1.114455 |
| conversion.test | RUN | 1.057781 | 2.626163 | 3.731226 | 0.002157 | 0.002178 | 0.002310 |
| doc_background.test | RUN | 1.014522 | 2.633352 | 3.676449 | 0.002092 | 0.002250 | 0.002273 |
| doc_forward_conversion.test | RUN | 1.011036 | 3.033506 | 3.979299 | 0.002259 | 0.002145 | 0.002364 |
| doc_parsing.test | RUN | 0.992690 | 2.719312 | 3.686572 | 0.002312 | 0.002158 | 0.002269 |
| doc_quick_look.test | RUN | 1.266475 | 3.415517 | 4.964319 | 0.002014 | 0.002154 | 0.002341 |
| doc_serializing.test | RUN | 0.952340 | 2.449091 | 3.520597 | 0.002102 | 0.002257 | 0.001874 |
| doc_storage_ptr.test | RUN | 0.924651 | 2.594327 | 3.477548 | 0.002214 | 0.002382 | 0.002326 |
| doc_uses_allocator.test | RUN | 0.982406 | 2.390906 | 3.534890 | 0.002280 | 0.002378 | 0.002200 |
| doc_using_numbers.test | RUN | 0.999318 | 2.498259 | 3.877800 | 0.002216 | 0.002104 | 0.002337 |
| double.test | RUN | 1.107079 | 4.446075 | 4.556942 | 2.179770 | 1.996603 | 2.129468 |
| error.test | RUN | 0.784993 | 2.407424 | 2.788888 | 0.002170 | 0.002201 | 0.002060 |
| fwd.test | RUN | 0.942821 | 2.306809 | 3.352318 | 0.002184 | 0.002227 | 0.002246 |
| json.test | RUN | 1.012269 | 2.821472 | 3.950640 | 0.002051 | 0.002188 | 0.002312 |
| kind.test | RUN | 0.742829 | 2.320114 | 2.862792 | 0.002292 | 0.002260 | 0.002370 |
| memory_resource.test | RUN | 1.051373 | 3.038209 | 3.736484 | 0.002082 | 0.002415 | 0.001926 |
| monotonic_resource.test | RUN | 0.971762 | 2.799161 | 3.604689 | 0.003243 | 0.002934 | 0.003241 |
| natvis.test | RUN | 0.957361 | 2.723401 | 3.341570 | 0.002273 | 0.002275 | 0.002189 |
| null_resource.test | RUN | 0.804416 | 2.245644 | 2.690064 | 0.002312 | 0.002415 | 0.002073 |
| object.test | RUN | 1.519305 | 8.082821 | 8.669788 | 0.010263 | 0.007374 | 0.008714 |
| parse.test | RUN | 1.053639 | 4.083242 | 3.943638 | 0.003109 | 0.003322 | 0.003187 |
| parser.test | RUN | 1.024751 | 3.516684 | 3.334377 | 0.002486 | 0.002655 | 0.002575 |
| pilfer.test | RUN | 0.533679 | 1.518341 | 1.569192 | 0.002171 | 0.002264 | 0.001964 |
| pointer.test | RUN | 1.141174 | 4.696535 | 4.555109 | 0.002602 | 0.002609 | 0.002668 |
| result_for.test | RUN | 0.983506 | 2.865784 | 3.391109 | 0.002210 | 0.002311 | 0.002297 |
| serialize.test | RUN | 0.988781 | 2.940405 | 3.476380 | 0.002474 | 0.002670 | 0.002565 |
| serializer.test | RUN | 1.503189 | 8.359765 | 8.014145 | 0.003552 | 0.003586 | 0.003648 |
| snippets.test | RUN | 2.116785 | 11.345074 | 11.216275 | 0.002575 | 0.002803 | 0.002693 |
| static_resource.test | RUN | 0.960746 | 2.508982 | 3.189314 | 0.002388 | 0.002264 | 0.002396 |
| storage_ptr.test | RUN | 0.995615 | 2.672681 | 3.628004 | 0.002392 | 0.002571 | 0.002337 |
| stream_parser.test | RUN | 1.149028 | 4.628738 | 4.653199 | 0.119070 | 0.128176 | 0.103765 |
| string.test | RUN | 1.820156 | 12.226838 | 12.147488 | 0.003877 | 0.003298 | 0.003303 |
| string_view.test | RUN | 0.601299 | 1.733057 | 1.750428 | 0.002188 | 0.002205 | 0.001942 |
| value.test | RUN | 1.870050 | 12.264947 | 10.138339 | 0.004669 | 0.003936 | 0.005857 |
| value_from.test | RUN | 1.660465 | 8.078788 | 7.478901 | 0.002601 | 0.002493 | 0.002303 |
| value_stack.test | RUN | 0.943015 | 2.625289 | 3.427713 | 0.002234 | 0.002350 | 0.002456 |
| value_to.test | RUN | 3.733195 | 25.263787 | 20.864728 | 0.004036 | 0.003702 | 0.003353 |
| value_ref.test | RUN | 1.081886 | 3.534152 | 4.399559 | 0.002485 | 0.002498 | 0.002354 |
| utf8.test | RUN | 0.814457 | 2.146143 | 2.798327 | 0.002272 | 0.002496 | 0.002130 |
| visit.test | RUN | 1.081127 | 3.952233 | 4.053644 | 0.002288 | 0.002124 | 0.002404 |
| d2s_intrinsics_test.test | RUN | 0.812635 | 2.192247 | 2.850593 | 0.002244 | 0.002370 | 0.002328 |
| d2s_table_test.test | RUN | 0.819345 | 2.189595 | 3.014440 | 0.002365 | 0.002103 | 0.002334 |
| d2s_test.test | RUN | 0.836458 | 2.848066 | 3.372236 | 0.002358 | 0.002234 | 0.002496 |
| parse_into.test | RUN | 22.150409 | 194.813679 | 130.324891 | 0.003603 | 0.003559 | 0.003372 |
| limits.test | RUN | 2.931545 | 16.769746 | 14.057304 | 0.002486 | 0.002359 | 0.002359 |
| header_only.test | RUN | 1.815557 | 12.671041 | 10.837343 | 0.002175 | 0.002141 | 0.002210 |
| no_exceptions.test | RUN | 2.895145 | 13.746019 | 12.558333 | 0.002142 | 0.002252 | 0.002300 |
| intrusive_macros.test | RUN | 2.259873 | 15.050021 | 12.643295 | 0.001976 | 0.002094 | 0.002284 |
| _gdb-printers.test | RUN | 0.466535 | 1.002359 | 1.603677 | 0.245952 | 0.395801 | 0.281327 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.782× | 0.796×, 0.768× | 0.987× |
| clang | 0.943× | 0.937×, 0.948× | 0.977× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| array.test | 8 | 0.002193s | 0.001915s | 0.002111s | native test oracle; output varies |
| basic_parser.test | 1 | 0.569893s | 0.300319s | 0.649954s | native test oracle; output varies |
| conversion.test | 10 | 0.001251s | 0.001257s | 0.001234s | byte equality observed |
| d2s_intrinsics_test.test | 10 | 0.001238s | 0.001253s | 0.001246s | byte equality observed |
| d2s_table_test.test | 10 | 0.001262s | 0.001279s | 0.001267s | byte equality observed |
| d2s_test.test | 10 | 0.001315s | 0.001326s | 0.001301s | byte equality observed |
| doc_background.test | 10 | 0.001233s | 0.001255s | 0.001217s | byte equality observed |
| doc_forward_conversion.test | 10 | 0.001304s | 0.001298s | 0.001264s | byte equality observed |
| doc_parsing.test | 10 | 0.001310s | 0.001304s | 0.001265s | byte equality observed |
| doc_quick_look.test | 10 | 0.001280s | 0.001289s | 0.001241s | byte equality observed |
| doc_serializing.test | 10 | 0.001248s | 0.001261s | 0.001217s | byte equality observed |
| doc_storage_ptr.test | 10 | 0.001284s | 0.001285s | 0.001251s | byte equality observed |
| doc_uses_allocator.test | 10 | 0.001278s | 0.001265s | 0.001220s | byte equality observed |
| doc_using_numbers.test | 10 | 0.001253s | 0.001265s | 0.001226s | byte equality observed |
| double.test | 1 | 1.351283s | 1.167053s | 1.169581s | native test oracle; output varies |
| error.test | 10 | 0.001345s | 0.001332s | 0.001324s | byte equality observed |
| fwd.test | 10 | 0.001237s | 0.001220s | 0.001230s | byte equality observed |
| header_only.test | 10 | 0.001224s | 0.001210s | 0.001219s | byte equality observed |
| intrusive_macros.test | 10 | 0.001290s | 0.001273s | 0.001279s | byte equality observed |
| json.test | 10 | 0.001261s | 0.001269s | 0.001258s | byte equality observed |
| kind.test | 10 | 0.001301s | 0.001281s | 0.001284s | byte equality observed |
| limits.test | 10 | 0.001536s | 0.001456s | 0.001477s | byte equality observed |
| memory_resource.test | 10 | 0.001307s | 0.001274s | 0.001265s | byte equality observed |
| monotonic_resource.test | 10 | 0.001875s | 0.001668s | 0.001783s | native test oracle; output varies |
| natvis.test | 10 | 0.001305s | 0.001278s | 0.001288s | byte equality observed |
| no_exceptions.test | 10 | 0.001318s | 0.001256s | 0.001258s | byte equality observed |
| null_resource.test | 10 | 0.001338s | 0.001299s | 0.001317s | byte equality observed |
| object.test | 4 | 0.005263s | 0.004192s | 0.004972s | native test oracle; output varies |
| parse.test | 10 | 0.001809s | 0.001796s | 0.001791s | byte equality observed |
| parse_into.test | 6 | 0.002927s | 0.002731s | 0.002647s | native test oracle; output varies |
| parser.test | 10 | 0.001460s | 0.001419s | 0.001448s | byte equality observed |
| pilfer.test | 10 | 0.001261s | 0.001253s | 0.001240s | byte equality observed |
| pointer.test | 10 | 0.001503s | 0.001458s | 0.001468s | byte equality observed |
| result_for.test | 10 | 0.001248s | 0.001256s | 0.001254s | byte equality observed |
| serialize.test | 10 | 0.001330s | 0.001302s | 0.001299s | byte equality observed |
| serializer.test | 9 | 0.002166s | 0.002069s | 0.002058s | native test oracle; output varies |
| snippets.test | 10 | 0.001504s | 0.001502s | 0.001500s | byte equality observed |
| static_resource.test | 10 | 0.001368s | 0.001335s | 0.001330s | byte equality observed |
| storage_ptr.test | 10 | 0.001334s | 0.001308s | 0.001315s | byte equality observed |
| stream_parser.test | 1 | 0.067709s | 0.078536s | 0.057775s | native test oracle; output varies |
| string.test | 8 | 0.002199s | 0.001842s | 0.002077s | native test oracle; output varies |
| string_view.test | 10 | 0.001253s | 0.001238s | 0.001238s | byte equality observed |
| utf8.test | 10 | 0.001251s | 0.001260s | 0.001274s | byte equality observed |
| value.test | 7 | 0.002747s | 0.002467s | 0.003034s | native test oracle; output varies |
| value_from.test | 10 | 0.001470s | 0.001441s | 0.001441s | byte equality observed |
| value_ref.test | 10 | 0.001334s | 0.001325s | 0.001327s | byte equality observed |
| value_stack.test | 10 | 0.001308s | 0.001285s | 0.001284s | byte equality observed |
| value_to.test | 8 | 0.002251s | 0.002244s | 0.002276s | native test oracle; output varies |
| visit.test | 10 | 0.001317s | 0.001295s | 0.001295s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
