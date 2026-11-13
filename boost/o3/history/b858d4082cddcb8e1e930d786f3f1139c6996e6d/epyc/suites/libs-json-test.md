# libs/json/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| array.test | RUN | 0.889923 | 4.960944 | 4.619034 | 0.003190 | 0.002925 | 0.003031 |
| basic_parser.test | RUN | 5.616134 | 124.240497 | 81.066322 | 0.555083 | 0.198771 | 0.440961 |
| conversion.test | RUN | 0.633837 | 1.843547 | 2.380575 | 0.001797 | 0.002054 | 0.003373 |
| doc_background.test | RUN | 0.661268 | 1.849820 | 2.342502 | 0.001581 | 0.001930 | 0.001832 |
| doc_forward_conversion.test | RUN | 0.729174 | 2.177392 | 2.601723 | 0.001854 | 0.001910 | 0.001716 |
| doc_parsing.test | RUN | 0.702181 | 1.946042 | 2.469174 | 0.001894 | 0.001879 | 0.001992 |
| doc_quick_look.test | RUN | 0.886348 | 2.435156 | 3.305362 | 0.002047 | 0.001818 | 0.001952 |
| doc_serializing.test | RUN | 0.656280 | 1.732630 | 2.250541 | 0.001942 | 0.001874 | 0.001701 |
| doc_storage_ptr.test | RUN | 0.656798 | 1.752906 | 2.250197 | 0.001910 | 0.001804 | 0.001971 |
| doc_uses_allocator.test | RUN | 0.658074 | 1.732817 | 2.229626 | 0.002001 | 0.001896 | 0.001976 |
| doc_using_numbers.test | RUN | 0.700947 | 1.821284 | 2.495352 | 0.001786 | 0.001840 | 0.001927 |
| double.test | RUN | 0.757056 | 3.187620 | 2.864398 | 1.489901 | 1.213229 | 1.444388 |
| error.test | RUN | 0.530588 | 1.750573 | 1.709106 | 0.002030 | 0.002010 | 0.002002 |
| fwd.test | RUN | 0.637130 | 1.697995 | 2.154003 | 0.001898 | 0.001918 | 0.001938 |
| json.test | RUN | 0.661277 | 2.075281 | 2.477953 | 0.001794 | 0.001792 | 0.001951 |
| kind.test | RUN | 0.487688 | 1.601198 | 1.817375 | 0.001902 | 0.002040 | 0.002056 |
| memory_resource.test | RUN | 0.690170 | 2.197683 | 2.295708 | 0.002038 | 0.001840 | 0.001869 |
| monotonic_resource.test | RUN | 0.637079 | 1.940648 | 2.401972 | 0.002690 | 0.002654 | 0.002424 |
| natvis.test | RUN | 0.649899 | 1.934344 | 2.157968 | 0.002074 | 0.001900 | 0.001988 |
| null_resource.test | RUN | 0.521972 | 1.666254 | 1.743771 | 0.001838 | 0.002109 | 0.002110 |
| object.test | RUN | 1.057737 | 5.765869 | 5.588950 | 0.006532 | 0.005691 | 0.006850 |
| parse.test | RUN | 0.700095 | 3.042189 | 2.705171 | 0.002546 | 0.002932 | 0.002476 |
| parser.test | RUN | 0.678302 | 2.524034 | 2.229711 | 0.002168 | 0.002240 | 0.001974 |
| pilfer.test | RUN | 0.358467 | 1.133145 | 1.009228 | 0.001876 | 0.001916 | 0.001731 |
| pointer.test | RUN | 0.772986 | 3.406064 | 3.016296 | 0.002193 | 0.002181 | 0.001881 |
| result_for.test | RUN | 0.656739 | 2.027253 | 2.206712 | 0.001696 | 0.001704 | 0.001782 |
| serialize.test | RUN | 0.655415 | 2.096205 | 2.250489 | 0.001843 | 0.001956 | 0.002036 |
| serializer.test | RUN | 1.013010 | 5.895967 | 5.212760 | 0.002156 | 0.002955 | 0.002673 |
| snippets.test | RUN | 1.553346 | 8.321845 | 8.195153 | 0.002320 | 0.002390 | 0.002295 |
| static_resource.test | RUN | 0.642801 | 1.868751 | 2.279617 | 0.002014 | 0.002106 | 0.002046 |
| storage_ptr.test | RUN | 0.662290 | 1.927873 | 2.364450 | 0.001963 | 0.001996 | 0.002007 |
| stream_parser.test | RUN | 0.796242 | 3.465526 | 3.279146 | 0.093902 | 0.101864 | 0.078237 |
| string.test | RUN | 1.300092 | 9.030505 | 8.157046 | 0.002958 | 0.002678 | 0.002664 |
| string_view.test | RUN | 0.404100 | 1.202952 | 1.378311 | 0.001920 | 0.001815 | 0.002092 |
| value.test | RUN | 1.315883 | 9.457228 | 6.061936 | 0.003900 | 0.003498 | 0.003664 |
| value_from.test | RUN | 1.190678 | 5.827298 | 5.915633 | 0.002036 | 0.002114 | 0.002278 |
| value_stack.test | RUN | 0.628974 | 1.727220 | 2.065251 | 0.001859 | 0.002034 | 0.002052 |
| value_to.test | RUN | 2.915028 | 17.780085 | 14.174797 | 0.003626 | 0.002896 | 0.002925 |
| value_ref.test | RUN | 0.715504 | 2.678021 | 2.905898 | 0.002034 | 0.002098 | 0.002052 |
| utf8.test | RUN | 0.547660 | 1.702684 | 1.950126 | 0.001972 | 0.002012 | 0.001842 |
| visit.test | RUN | 0.709639 | 2.453906 | 2.733368 | 0.001968 | 0.001818 | 0.002095 |
| d2s_intrinsics_test.test | RUN | 0.547945 | 1.539358 | 1.898481 | 0.001850 | 0.002172 | 0.001938 |
| d2s_table_test.test | RUN | 0.553971 | 1.623456 | 1.972352 | 0.001875 | 0.001906 | 0.002043 |
| d2s_test.test | RUN | 0.565651 | 2.136201 | 2.201782 | 0.002091 | 0.002000 | 0.002086 |
| parse_into.test | RUN | 17.339913 | 151.816203 | 94.569129 | 0.003007 | 0.002995 | 0.002985 |
| limits.test | RUN | 2.121122 | 13.037022 | 9.526524 | 0.002136 | 0.002292 | 0.002058 |
| header_only.test | RUN | 1.314272 | 9.796284 | 7.340290 | 0.001832 | 0.001833 | 0.001854 |
| no_exceptions.test | RUN | 1.961093 | 10.577045 | 8.634402 | 0.001872 | 0.001992 | 0.001789 |
| intrusive_macros.test | RUN | 1.601348 | 10.479447 | 8.321486 | 0.001672 | 0.001976 | 0.002030 |
| _gdb-printers.test | RUN | 0.297665 | 0.805196 | 0.846208 | 0.185207 | 0.271601 | 0.210775 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.818× | 0.813×, 0.822× | 1.005× |
| clang | 1.020× | 1.039×, 1.000× | 1.000× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737952).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| array.test | 10 | 0.001716s | 0.001505s | 0.001658s | byte equality observed |
| basic_parser.test | 1 | 0.373781s | 0.213523s | 0.443102s | native test oracle; output varies |
| conversion.test | 10 | 0.001004s | 0.000984s | 0.001021s | byte equality observed |
| d2s_intrinsics_test.test | 10 | 0.000986s | 0.000972s | 0.001013s | byte equality observed |
| d2s_table_test.test | 10 | 0.001009s | 0.000992s | 0.001009s | byte equality observed |
| d2s_test.test | 10 | 0.001071s | 0.001058s | 0.001063s | byte equality observed |
| doc_background.test | 10 | 0.001028s | 0.000985s | 0.001026s | byte equality observed |
| doc_forward_conversion.test | 10 | 0.001051s | 0.001023s | 0.001041s | byte equality observed |
| doc_parsing.test | 10 | 0.001063s | 0.001051s | 0.001036s | byte equality observed |
| doc_quick_look.test | 10 | 0.001013s | 0.001028s | 0.001047s | byte equality observed |
| doc_serializing.test | 10 | 0.001014s | 0.001011s | 0.001011s | byte equality observed |
| doc_storage_ptr.test | 10 | 0.001029s | 0.001025s | 0.001017s | byte equality observed |
| doc_uses_allocator.test | 10 | 0.001022s | 0.001022s | 0.000994s | byte equality observed |
| doc_using_numbers.test | 10 | 0.001043s | 0.001013s | 0.000993s | byte equality observed |
| double.test | 1 | 0.911570s | 0.805212s | 0.874692s | native test oracle; output varies |
| error.test | 10 | 0.001143s | 0.001034s | 0.001164s | byte equality observed |
| fwd.test | 10 | 0.001020s | 0.000995s | 0.001008s | byte equality observed |
| header_only.test | 10 | 0.001000s | 0.000989s | 0.000971s | byte equality observed |
| intrusive_macros.test | 10 | 0.001059s | 0.001080s | 0.001021s | byte equality observed |
| json.test | 10 | 0.001036s | 0.001027s | 0.001033s | byte equality observed |
| kind.test | 10 | 0.001053s | 0.001046s | 0.001051s | byte equality observed |
| limits.test | 10 | 0.001241s | 0.001199s | 0.001206s | byte equality observed |
| memory_resource.test | 10 | 0.001045s | 0.001059s | 0.001039s | byte equality observed |
| monotonic_resource.test | 10 | 0.001499s | 0.001363s | 0.001432s | byte equality observed |
| natvis.test | 10 | 0.001061s | 0.001069s | 0.001095s | byte equality observed |
| no_exceptions.test | 10 | 0.001039s | 0.001028s | 0.001056s | byte equality observed |
| null_resource.test | 10 | 0.001064s | 0.001063s | 0.001072s | byte equality observed |
| object.test | 5 | 0.003973s | 0.003276s | 0.003927s | native test oracle; output varies |
| parse.test | 10 | 0.001522s | 0.001451s | 0.001527s | byte equality observed |
| parse_into.test | 7 | 0.002456s | 0.002290s | 0.002307s | native test oracle; output varies |
| parser.test | 10 | 0.001196s | 0.001160s | 0.001171s | byte equality observed |
| pilfer.test | 10 | 0.000990s | 0.001009s | 0.001037s | byte equality observed |
| pointer.test | 10 | 0.001216s | 0.001191s | 0.001216s | byte equality observed |
| result_for.test | 10 | 0.001031s | 0.001027s | 0.001036s | byte equality observed |
| serialize.test | 10 | 0.001052s | 0.001073s | 0.001071s | byte equality observed |
| serializer.test | 10 | 0.001725s | 0.001680s | 0.001674s | byte equality observed |
| snippets.test | 10 | 0.001260s | 0.001233s | 0.001214s | byte equality observed |
| static_resource.test | 10 | 0.001099s | 0.001122s | 0.001070s | byte equality observed |
| storage_ptr.test | 10 | 0.001064s | 0.001099s | 0.001044s | byte equality observed |
| stream_parser.test | 1 | 0.051652s | 0.065108s | 0.046552s | native test oracle; output varies |
| string.test | 9 | 0.001777s | 0.001535s | 0.001762s | byte equality observed |
| string_view.test | 10 | 0.001011s | 0.001011s | 0.001004s | byte equality observed |
| utf8.test | 10 | 0.001023s | 0.000998s | 0.001040s | byte equality observed |
| value.test | 8 | 0.002170s | 0.001954s | 0.002423s | native test oracle; output varies |
| value_from.test | 10 | 0.001277s | 0.001168s | 0.001201s | byte equality observed |
| value_ref.test | 10 | 0.001087s | 0.001060s | 0.001054s | byte equality observed |
| value_stack.test | 10 | 0.001073s | 0.001071s | 0.001087s | byte equality observed |
| value_to.test | 10 | 0.001938s | 0.001787s | 0.001870s | native test oracle; output varies |
| visit.test | 10 | 0.001087s | 0.001077s | 0.001099s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737952).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
