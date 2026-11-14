# libs/json/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| array.test | RUN | 0.768746 | 3.935038 | 3.438175 | 0.002684 | 0.002398 | 0.002503 |
| basic_parser.test | RUN | 4.506402 | 98.159705 | 65.995260 | 0.386866 | 0.147554 | 0.336644 |
| conversion.test | RUN | 0.541773 | 1.455292 | 1.701751 | 0.001420 | 0.001742 | 0.001780 |
| doc_background.test | RUN | 0.570376 | 1.461931 | 1.694070 | 0.001813 | 0.001642 | 0.001687 |
| doc_forward_conversion.test | RUN | 0.574622 | 1.722180 | 1.934783 | 0.001846 | 0.001648 | 0.001482 |
| doc_parsing.test | RUN | 0.576711 | 1.616959 | 1.721895 | 0.001862 | 0.001644 | 0.001825 |
| doc_quick_look.test | RUN | 0.740959 | 1.980349 | 2.393866 | 0.001920 | 0.001619 | 0.001896 |
| doc_serializing.test | RUN | 0.531878 | 1.394448 | 1.629863 | 0.001570 | 0.001619 | 0.001799 |
| doc_storage_ptr.test | RUN | 0.541369 | 1.467649 | 1.663421 | 0.001970 | 0.001501 | 0.001491 |
| doc_uses_allocator.test | RUN | 0.532194 | 1.378606 | 1.686169 | 0.001868 | 0.001676 | 0.001842 |
| doc_using_numbers.test | RUN | 0.591820 | 1.469009 | 1.827696 | 0.001886 | 0.001749 | 0.001701 |
| double.test | RUN | 0.631087 | 2.444057 | 2.060345 | 1.173748 | 1.019479 | 1.089295 |
| error.test | RUN | 0.421464 | 1.372470 | 1.298364 | 0.001886 | 0.001576 | 0.001726 |
| fwd.test | RUN | 0.516084 | 1.368578 | 1.676432 | 0.001776 | 0.001656 | 0.002004 |
| json.test | RUN | 0.538074 | 1.635918 | 1.909328 | 0.001848 | 0.001888 | 0.001972 |
| kind.test | RUN | 0.437265 | 1.276020 | 1.399010 | 0.001690 | 0.001831 | 0.002010 |
| memory_resource.test | RUN | 0.568431 | 1.786972 | 1.773410 | 0.001840 | 0.001865 | 0.001985 |
| monotonic_resource.test | RUN | 0.519658 | 1.621750 | 1.782650 | 0.002370 | 0.002098 | 0.002482 |
| natvis.test | RUN | 0.517595 | 1.527736 | 1.633466 | 0.001845 | 0.001886 | 0.001712 |
| null_resource.test | RUN | 0.433972 | 1.276917 | 1.287080 | 0.001932 | 0.001945 | 0.002058 |
| object.test | RUN | 0.895391 | 4.559080 | 4.334350 | 0.005553 | 0.004449 | 0.005365 |
| parse.test | RUN | 0.581575 | 2.256109 | 1.973872 | 0.002546 | 0.002275 | 0.002461 |
| parser.test | RUN | 0.599250 | 1.930558 | 1.687309 | 0.002086 | 0.001856 | 0.001886 |
| pilfer.test | RUN | 0.286529 | 0.869070 | 0.813598 | 0.001494 | 0.001547 | 0.001688 |
| pointer.test | RUN | 0.630775 | 2.625714 | 2.248049 | 0.001970 | 0.001990 | 0.002082 |
| result_for.test | RUN | 0.547925 | 1.604602 | 1.642689 | 0.001584 | 0.001792 | 0.001761 |
| serialize.test | RUN | 0.556323 | 1.625663 | 1.784283 | 0.002000 | 0.001924 | 0.001930 |
| serializer.test | RUN | 0.835725 | 4.626552 | 3.844425 | 0.002841 | 0.002688 | 0.002553 |
| snippets.test | RUN | 1.335708 | 6.650137 | 5.981418 | 0.002156 | 0.002039 | 0.002106 |
| static_resource.test | RUN | 0.527737 | 1.467977 | 1.573172 | 0.002051 | 0.001870 | 0.001911 |
| storage_ptr.test | RUN | 0.573048 | 1.560060 | 1.596592 | 0.001957 | 0.001877 | 0.001880 |
| stream_parser.test | RUN | 0.671950 | 2.752483 | 2.455679 | 0.065623 | 0.079356 | 0.067922 |
| string.test | RUN | 1.064538 | 6.853248 | 6.494689 | 0.002844 | 0.002536 | 0.002746 |
| string_view.test | RUN | 0.329557 | 0.964939 | 0.989498 | 0.001836 | 0.001841 | 0.001795 |
| value.test | RUN | 1.067018 | 7.604768 | 4.831866 | 0.003299 | 0.002806 | 0.003426 |
| value_from.test | RUN | 1.015189 | 4.610712 | 4.743198 | 0.002198 | 0.001974 | 0.001899 |
| value_stack.test | RUN | 0.528260 | 1.391126 | 1.686776 | 0.001927 | 0.001798 | 0.001901 |
| value_to.test | RUN | 2.479415 | 14.547601 | 11.705982 | 0.003088 | 0.002895 | 0.002815 |
| value_ref.test | RUN | 0.586694 | 1.963430 | 1.948285 | 0.001964 | 0.001812 | 0.001829 |
| utf8.test | RUN | 0.435853 | 1.291437 | 1.349494 | 0.001782 | 0.001682 | 0.001690 |
| visit.test | RUN | 0.570891 | 2.121880 | 1.797345 | 0.001529 | 0.001881 | 0.001753 |
| d2s_intrinsics_test.test | RUN | 0.437540 | 1.302153 | 1.235729 | 0.001866 | 0.001870 | 0.001690 |
| d2s_table_test.test | RUN | 0.445702 | 1.493506 | 1.403456 | 0.001878 | 0.001728 | 0.001764 |
| d2s_test.test | RUN | 0.469242 | 1.678065 | 1.444037 | 0.002130 | 0.001926 | 0.001717 |
| parse_into.test | RUN | 14.785759 | 120.268469 | 78.996580 | 0.003733 | 0.002877 | 0.003198 |
| limits.test | RUN | 1.757585 | 10.488188 | 7.689423 | 0.002306 | 0.002176 | 0.002160 |
| header_only.test | RUN | 1.127214 | 7.859483 | 5.720267 | 0.001798 | 0.001641 | 0.001760 |
| no_exceptions.test | RUN | 1.675035 | 9.151260 | 7.393841 | 0.002698 | 0.001766 | 0.001811 |
| intrusive_macros.test | RUN | 1.397388 | 8.633517 | 6.666250 | 0.001828 | 0.001939 | 0.001818 |
| _gdb-printers.test | RUN | 0.248763 | 0.604303 | 0.737757 | 0.161761 | 0.191622 | 0.225721 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.801×† | 0.760×, 0.842× | 0.998× |
| clang | 1.016× | 1.023×, 1.008× | 0.983× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051632).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| array.test | 10 | 0.001499s | 0.001296s | 0.001382s | byte equality observed |
| basic_parser.test | 1 | 0.232950s | 0.152537s | 0.349899s | native test oracle; output varies |
| conversion.test | 10 | 0.000881s | 0.000878s | 0.000866s | byte equality observed |
| d2s_intrinsics_test.test | 10 | 0.000871s | 0.000866s | 0.000869s | byte equality observed |
| d2s_table_test.test | 10 | 0.000898s | 0.000891s | 0.000892s | byte equality observed |
| d2s_test.test | 10 | 0.000957s | 0.000936s | 0.000936s | byte equality observed |
| doc_background.test | 10 | 0.000947s | 0.000857s | 0.000863s | byte equality observed |
| doc_forward_conversion.test | 10 | 0.000965s | 0.000928s | 0.000903s | byte equality observed |
| doc_parsing.test | 10 | 0.000976s | 0.000955s | 0.000914s | byte equality observed |
| doc_quick_look.test | 10 | 0.000936s | 0.000945s | 0.000886s | byte equality observed |
| doc_serializing.test | 10 | 0.000939s | 0.000902s | 0.000867s | byte equality observed |
| doc_storage_ptr.test | 10 | 0.000933s | 0.000926s | 0.000917s | byte equality observed |
| doc_uses_allocator.test | 10 | 0.000939s | 0.000918s | 0.000891s | byte equality observed |
| doc_using_numbers.test | 10 | 0.000947s | 0.000905s | 0.000887s | byte equality observed |
| double.test | 1 | 0.780077s | 0.638594s | 0.704125s | native test oracle; output varies |
| error.test | 10 | 0.001067s | 0.000901s | 0.001065s | byte equality observed |
| fwd.test | 10 | 0.000921s | 0.000872s | 0.000928s | byte equality observed |
| header_only.test | 10 | 0.000900s | 0.000845s | 0.000931s | byte equality observed |
| intrusive_macros.test | 10 | 0.000941s | 0.000923s | 0.000957s | byte equality observed |
| json.test | 10 | 0.000926s | 0.000887s | 0.000923s | byte equality observed |
| kind.test | 10 | 0.000949s | 0.000907s | 0.000944s | byte equality observed |
| limits.test | 10 | 0.001115s | 0.001037s | 0.001082s | native test oracle; output varies |
| memory_resource.test | 10 | 0.000934s | 0.000915s | 0.000936s | byte equality observed |
| monotonic_resource.test | 10 | 0.001285s | 0.001177s | 0.001244s | native test oracle; output varies |
| natvis.test | 10 | 0.000939s | 0.000905s | 0.000935s | byte equality observed |
| no_exceptions.test | 10 | 0.000941s | 0.000901s | 0.000931s | byte equality observed |
| null_resource.test | 10 | 0.000962s | 0.000948s | 0.000967s | byte equality observed |
| object.test | 6 | 0.003071s | 0.002585s | 0.003013s | native test oracle; output varies |
| parse.test | 10 | 0.001303s | 0.001274s | 0.001262s | native test oracle; output varies |
| parse_into.test | 6 | 0.002433s | 0.002319s | 0.002220s | native test oracle; output varies |
| parser.test | 10 | 0.001044s | 0.001051s | 0.001032s | byte equality observed |
| pilfer.test | 10 | 0.000889s | 0.000917s | 0.000896s | byte equality observed |
| pointer.test | 10 | 0.001096s | 0.001064s | 0.001077s | byte equality observed |
| result_for.test | 10 | 0.000885s | 0.000907s | 0.000897s | byte equality observed |
| serialize.test | 10 | 0.000989s | 0.000962s | 0.000945s | byte equality observed |
| serializer.test | 10 | 0.001534s | 0.001476s | 0.001483s | native test oracle; output varies |
| snippets.test | 10 | 0.001161s | 0.001097s | 0.001086s | byte equality observed |
| static_resource.test | 10 | 0.001020s | 0.000977s | 0.000976s | byte equality observed |
| storage_ptr.test | 10 | 0.000956s | 0.000965s | 0.000951s | byte equality observed |
| stream_parser.test | 1 | 0.042566s | 0.045825s | 0.039498s | native test oracle; output varies |
| string.test | 8 | 0.001545s | 0.001421s | 0.001533s | byte equality observed |
| string_view.test | 10 | 0.000899s | 0.000942s | 0.000892s | byte equality observed |
| utf8.test | 10 | 0.000888s | 0.000925s | 0.000908s | byte equality observed |
| value.test | 9 | 0.001811s | 0.001646s | 0.001920s | native test oracle; output varies |
| value_from.test | 10 | 0.001121s | 0.001060s | 0.001056s | byte equality observed |
| value_ref.test | 10 | 0.000994s | 0.000968s | 0.000960s | byte equality observed |
| value_stack.test | 10 | 0.000984s | 0.000946s | 0.000938s | byte equality observed |
| value_to.test | 10 | 0.001636s | 0.001530s | 0.001547s | native test oracle; output varies |
| visit.test | 10 | 0.000959s | 0.000916s | 0.000952s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051632).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
