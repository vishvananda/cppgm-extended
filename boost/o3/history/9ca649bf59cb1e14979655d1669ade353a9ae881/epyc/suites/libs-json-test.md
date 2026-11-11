# libs/json/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| array.test | RUN | 0.983072 | 4.861011 | 4.435326 | 0.003033 | 0.002872 | 0.002844 |
| basic_parser.test | RUN | 5.023349 | 121.993492 | 70.807890 | 0.636712 | 0.193382 | 0.686324 |
| conversion.test | RUN | 0.765511 | 1.820055 | 2.341541 | 0.001824 | 0.001973 | 0.001868 |
| doc_background.test | RUN | 0.730906 | 1.829823 | 2.306397 | 0.001913 | 0.001786 | 0.001805 |
| doc_forward_conversion.test | RUN | 0.738324 | 2.062106 | 2.579697 | 0.002012 | 0.001758 | 0.001692 |
| doc_parsing.test | RUN | 0.732149 | 1.995383 | 2.265962 | 0.002005 | 0.001833 | 0.001856 |
| doc_quick_look.test | RUN | 0.913199 | 2.464471 | 3.208527 | 0.001789 | 0.001759 | 0.001899 |
| doc_serializing.test | RUN | 0.688831 | 1.847178 | 2.216740 | 0.001840 | 0.001812 | 0.001691 |
| doc_storage_ptr.test | RUN | 0.679259 | 1.725192 | 2.215407 | 0.001799 | 0.001867 | 0.001781 |
| doc_uses_allocator.test | RUN | 0.691082 | 1.794229 | 2.174057 | 0.001896 | 0.004034 | 0.001759 |
| doc_using_numbers.test | RUN | 0.720125 | 1.782549 | 2.431309 | 0.001886 | 0.002052 | 0.001854 |
| double.test | RUN | 0.795932 | 2.966931 | 2.781763 | 1.438318 | 1.339793 | 1.334985 |
| error.test | RUN | 0.565089 | 1.801303 | 1.875425 | 0.001740 | 0.002016 | 0.001958 |
| fwd.test | RUN | 0.677111 | 1.685233 | 2.375094 | 0.001748 | 0.001834 | 0.001847 |
| json.test | RUN | 0.713014 | 2.047437 | 2.447082 | 0.001813 | 0.001888 | 0.001872 |
| kind.test | RUN | 0.551182 | 1.623176 | 1.798197 | 0.001986 | 0.002210 | 0.001936 |
| memory_resource.test | RUN | 0.730893 | 2.040938 | 2.310282 | 0.001379 | 0.002044 | 0.001852 |
| monotonic_resource.test | RUN | 0.686670 | 1.974749 | 2.150236 | 0.002621 | 0.002512 | 0.002497 |
| natvis.test | RUN | 0.691157 | 1.963477 | 2.125088 | 0.001870 | 0.002018 | 0.002026 |
| null_resource.test | RUN | 0.573379 | 1.600641 | 1.739671 | 0.001988 | 0.002148 | 0.001822 |
| object.test | RUN | 1.098326 | 5.713999 | 5.592210 | 0.006972 | 0.005643 | 0.006782 |
| parse.test | RUN | 0.744883 | 2.875269 | 2.395096 | 0.002520 | 0.002762 | 0.002361 |
| parser.test | RUN | 0.730362 | 2.513036 | 2.309073 | 0.002035 | 0.002318 | 0.002106 |
| pilfer.test | RUN | 0.380967 | 1.131878 | 1.049493 | 0.001770 | 0.002022 | 0.001710 |
| pointer.test | RUN | 0.802923 | 3.412376 | 3.022953 | 0.002026 | 0.002379 | 0.002308 |
| result_for.test | RUN | 0.692651 | 2.035551 | 2.238146 | 0.001776 | 0.002033 | 0.001845 |
| serialize.test | RUN | 0.695889 | 2.087610 | 2.342992 | 0.001929 | 0.002051 | 0.002108 |
| serializer.test | RUN | 1.064548 | 5.874296 | 5.055838 | 0.002872 | 0.003057 | 0.002559 |
| snippets.test | RUN | 1.526236 | 8.280006 | 7.804619 | 0.002037 | 0.002064 | 0.002214 |
| static_resource.test | RUN | 0.677836 | 1.813182 | 2.205805 | 0.002027 | 0.001930 | 0.001975 |
| storage_ptr.test | RUN | 0.729123 | 1.912731 | 2.111107 | 0.001874 | 0.002088 | 0.002008 |
| stream_parser.test | RUN | 0.818412 | 3.406723 | 2.938791 | 0.086288 | 0.118310 | 0.086728 |
| string.test | RUN | 1.311861 | 8.786283 | 7.590844 | 0.002883 | 0.002872 | 0.002704 |
| string_view.test | RUN | 0.432196 | 1.206989 | 1.338910 | 0.001792 | 0.001956 | 0.001837 |
| value.test | RUN | 1.320149 | 8.845748 | 6.450529 | 0.003539 | 0.003282 | 0.004224 |
| value_from.test | RUN | 1.193794 | 5.805172 | 5.475968 | 0.001915 | 0.002020 | 0.002081 |
| value_stack.test | RUN | 0.695181 | 1.770704 | 2.103299 | 0.001880 | 0.002126 | 0.001942 |
| value_to.test | RUN | 2.734782 | 17.535432 | 13.469039 | 0.003319 | 0.003468 | 0.003028 |
| value_ref.test | RUN | 0.758404 | 2.445848 | 2.838981 | 0.001950 | 0.001794 | 0.002056 |
| utf8.test | RUN | 0.587175 | 1.486311 | 1.857853 | 0.001796 | 0.001894 | 0.001902 |
| visit.test | RUN | 0.765810 | 2.727570 | 2.639664 | 0.001903 | 0.001969 | 0.001934 |
| d2s_intrinsics_test.test | RUN | 0.578736 | 1.623281 | 1.824620 | 0.001870 | 0.001930 | 0.001930 |
| d2s_table_test.test | RUN | 0.591348 | 1.727528 | 1.878883 | 0.002988 | 0.002060 | 0.001965 |
| d2s_test.test | RUN | 0.633482 | 2.109739 | 2.141292 | 0.001942 | 0.001969 | 0.001908 |
| parse_into.test | RUN | 17.090410 | 150.236199 | 88.770935 | 0.003243 | 0.003055 | 0.002927 |
| limits.test | RUN | 2.127882 | 11.781421 | 9.286512 | 0.003265 | 0.002192 | 0.001966 |
| header_only.test | RUN | 1.342786 | 9.548537 | 7.224313 | 0.001768 | 0.001934 | 0.001731 |
| no_exceptions.test | RUN | 2.116396 | 9.975311 | 8.963672 | 0.001966 | 0.001876 | 0.001990 |
| intrusive_macros.test | RUN | 1.669117 | 10.765398 | 7.482420 | 0.001937 | 0.002098 | 0.001716 |
| _gdb-printers.test | RUN | 0.331130 | 0.717638 | 1.017177 | 0.180866 | 0.265977 | 0.203498 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.752× | 0.747×, 0.757× | 1.022× |
| clang | 0.960× | 0.962×, 0.958× | 1.000× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| array.test | 9 | 0.001751s | 0.001601s | 0.001667s | native test oracle; output varies |
| basic_parser.test | 1 | 0.411290s | 0.209333s | 0.446757s | native test oracle; output varies |
| conversion.test | 10 | 0.001043s | 0.001038s | 0.001003s | byte equality observed |
| d2s_intrinsics_test.test | 10 | 0.001048s | 0.001015s | 0.000999s | byte equality observed |
| d2s_table_test.test | 10 | 0.001049s | 0.001085s | 0.001025s | byte equality observed |
| d2s_test.test | 10 | 0.001084s | 0.001106s | 0.001068s | byte equality observed |
| doc_background.test | 10 | 0.001030s | 0.001040s | 0.001018s | byte equality observed |
| doc_forward_conversion.test | 10 | 0.001080s | 0.001076s | 0.001069s | byte equality observed |
| doc_parsing.test | 10 | 0.001093s | 0.001078s | 0.001046s | byte equality observed |
| doc_quick_look.test | 10 | 0.001060s | 0.001071s | 0.001027s | byte equality observed |
| doc_serializing.test | 10 | 0.001016s | 0.001041s | 0.000991s | byte equality observed |
| doc_storage_ptr.test | 10 | 0.001057s | 0.001079s | 0.001037s | byte equality observed |
| doc_uses_allocator.test | 10 | 0.001051s | 0.001046s | 0.001041s | byte equality observed |
| doc_using_numbers.test | 10 | 0.001018s | 0.001027s | 0.001016s | byte equality observed |
| double.test | 1 | 0.969829s | 0.798624s | 0.874503s | native test oracle; output varies |
| error.test | 10 | 0.001143s | 0.001042s | 0.001146s | byte equality observed |
| fwd.test | 10 | 0.001031s | 0.001010s | 0.001045s | byte equality observed |
| header_only.test | 10 | 0.001021s | 0.001003s | 0.000992s | byte equality observed |
| intrusive_macros.test | 10 | 0.001042s | 0.001111s | 0.001046s | byte equality observed |
| json.test | 10 | 0.001036s | 0.001057s | 0.001037s | byte equality observed |
| kind.test | 10 | 0.001060s | 0.001081s | 0.001045s | byte equality observed |
| limits.test | 10 | 0.001240s | 0.001234s | 0.001191s | byte equality observed |
| memory_resource.test | 10 | 0.001058s | 0.001040s | 0.001005s | byte equality observed |
| monotonic_resource.test | 10 | 0.001482s | 0.001349s | 0.001417s | byte equality observed |
| natvis.test | 10 | 0.001066s | 0.001058s | 0.001017s | byte equality observed |
| no_exceptions.test | 10 | 0.001061s | 0.001056s | 0.001050s | byte equality observed |
| null_resource.test | 10 | 0.001093s | 0.001105s | 0.001058s | byte equality observed |
| object.test | 5 | 0.003970s | 0.003334s | 0.003824s | native test oracle; output varies |
| parse.test | 10 | 0.001480s | 0.001520s | 0.001426s | byte equality observed |
| parse_into.test | 7 | 0.002470s | 0.002378s | 0.002172s | native test oracle; output varies |
| parser.test | 10 | 0.001184s | 0.001204s | 0.001148s | byte equality observed |
| pilfer.test | 10 | 0.001023s | 0.001044s | 0.000990s | byte equality observed |
| pointer.test | 10 | 0.001241s | 0.001249s | 0.001221s | byte equality observed |
| result_for.test | 10 | 0.001019s | 0.001035s | 0.001012s | byte equality observed |
| serialize.test | 10 | 0.001067s | 0.001088s | 0.001044s | byte equality observed |
| serializer.test | 10 | 0.001761s | 0.001706s | 0.001679s | native test oracle; output varies |
| snippets.test | 10 | 0.001258s | 0.001248s | 0.001224s | byte equality observed |
| static_resource.test | 10 | 0.001131s | 0.001132s | 0.001125s | byte equality observed |
| storage_ptr.test | 10 | 0.001093s | 0.001082s | 0.001087s | byte equality observed |
| stream_parser.test | 1 | 0.053354s | 0.065272s | 0.046106s | native test oracle; output varies |
| string.test | 9 | 0.001799s | 0.001569s | 0.001676s | byte equality observed |
| string_view.test | 10 | 0.001035s | 0.001030s | 0.001022s | byte equality observed |
| utf8.test | 10 | 0.001045s | 0.001028s | 0.000999s | byte equality observed |
| value.test | 8 | 0.002169s | 0.001967s | 0.002344s | native test oracle; output varies |
| value_from.test | 10 | 0.001212s | 0.001208s | 0.001185s | byte equality observed |
| value_ref.test | 10 | 0.001114s | 0.001094s | 0.001074s | byte equality observed |
| value_stack.test | 10 | 0.001078s | 0.001127s | 0.001059s | byte equality observed |
| value_to.test | 10 | 0.001849s | 0.001808s | 0.001837s | native test oracle; output varies |
| visit.test | 10 | 0.001096s | 0.001095s | 0.001061s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
