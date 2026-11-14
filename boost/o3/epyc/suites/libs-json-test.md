# libs/json/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| array.test | RUN | 0.896844 | 4.841430 | 4.499459 | 0.002677 | 0.002880 | 0.002844 |
| basic_parser.test | RUN | 5.368522 | 116.670812 | 71.220709 | 0.544938 | 0.197914 | 0.696001 |
| conversion.test | RUN | 0.673818 | 1.812031 | 2.355085 | 0.001808 | 0.001931 | 0.001546 |
| doc_background.test | RUN | 0.656954 | 1.835854 | 2.294086 | 0.001862 | 0.001936 | 0.001830 |
| doc_forward_conversion.test | RUN | 0.675876 | 2.136854 | 2.567663 | 0.001730 | 0.001946 | 0.001738 |
| doc_parsing.test | RUN | 0.674264 | 2.003167 | 2.251532 | 0.001917 | 0.001750 | 0.001878 |
| doc_quick_look.test | RUN | 0.874936 | 2.390910 | 3.196068 | 0.001839 | 0.001686 | 0.001875 |
| doc_serializing.test | RUN | 0.616680 | 1.741047 | 2.177271 | 0.001800 | 0.001823 | 0.001745 |
| doc_storage_ptr.test | RUN | 0.623639 | 1.732888 | 2.195510 | 0.001664 | 0.001716 | 0.001874 |
| doc_uses_allocator.test | RUN | 0.626964 | 1.784521 | 2.179424 | 0.001874 | 0.001943 | 0.001728 |
| doc_using_numbers.test | RUN | 0.671144 | 1.766325 | 2.441631 | 0.001653 | 0.002011 | 0.001768 |
| double.test | RUN | 0.709313 | 2.942550 | 2.941386 | 1.405652 | 1.132643 | 1.437655 |
| error.test | RUN | 0.520343 | 1.763846 | 1.678200 | 0.001871 | 0.002041 | 0.001699 |
| fwd.test | RUN | 0.618215 | 1.818681 | 2.068282 | 0.001826 | 0.001842 | 0.001767 |
| json.test | RUN | 0.664180 | 2.058875 | 2.508203 | 0.001824 | 0.001818 | 0.001876 |
| kind.test | RUN | 0.499822 | 1.546353 | 1.802147 | 0.001838 | 0.001914 | 0.001915 |
| memory_resource.test | RUN | 0.692298 | 2.135675 | 2.306970 | 0.001954 | 0.002060 | 0.001897 |
| monotonic_resource.test | RUN | 0.626454 | 1.950089 | 2.106919 | 0.002628 | 0.002487 | 0.002466 |
| natvis.test | RUN | 0.625046 | 1.870359 | 2.140490 | 0.001795 | 0.001824 | 0.001833 |
| null_resource.test | RUN | 0.531651 | 1.607106 | 1.679083 | 0.001534 | 0.002057 | 0.001838 |
| object.test | RUN | 1.044227 | 5.637620 | 5.542420 | 0.006292 | 0.005535 | 0.005265 |
| parse.test | RUN | 0.709060 | 2.855285 | 2.427048 | 0.002284 | 0.002704 | 0.002322 |
| parser.test | RUN | 0.664400 | 2.439056 | 2.221567 | 0.002100 | 0.002278 | 0.001948 |
| pilfer.test | RUN | 0.348441 | 1.106239 | 1.155511 | 0.001886 | 0.001790 | 0.001828 |
| pointer.test | RUN | 0.749185 | 3.253373 | 2.852153 | 0.002280 | 0.002200 | 0.002162 |
| result_for.test | RUN | 0.644124 | 1.965184 | 2.168466 | 0.001724 | 0.001914 | 0.001703 |
| serialize.test | RUN | 0.649813 | 2.058767 | 2.336481 | 0.001938 | 0.002048 | 0.001937 |
| serializer.test | RUN | 1.003481 | 5.810592 | 5.177151 | 0.002836 | 0.002894 | 0.002832 |
| snippets.test | RUN | 1.567802 | 8.047707 | 7.718113 | 0.002206 | 0.002014 | 0.001878 |
| static_resource.test | RUN | 0.638706 | 1.823539 | 2.188896 | 0.002002 | 0.001944 | 0.001908 |
| storage_ptr.test | RUN | 0.660082 | 1.901689 | 2.050450 | 0.001692 | 0.001928 | 0.001810 |
| stream_parser.test | RUN | 0.779900 | 3.401321 | 2.936728 | 0.088594 | 0.121799 | 0.087575 |
| string.test | RUN | 1.286044 | 8.146686 | 7.193403 | 0.002962 | 0.002617 | 0.002622 |
| string_view.test | RUN | 0.379266 | 1.195780 | 1.361929 | 0.001846 | 0.001888 | 0.001794 |
| value.test | RUN | 1.274659 | 9.188579 | 6.896192 | 0.003741 | 0.003405 | 0.003865 |
| value_from.test | RUN | 1.166313 | 5.543108 | 5.675162 | 0.002220 | 0.002202 | 0.002045 |
| value_stack.test | RUN | 0.612020 | 1.808602 | 2.232100 | 0.001892 | 0.002047 | 0.001842 |
| value_to.test | RUN | 2.910287 | 17.403246 | 14.588564 | 0.003274 | 0.003466 | 0.003172 |
| value_ref.test | RUN | 0.713515 | 2.612434 | 2.810325 | 0.001937 | 0.002058 | 0.001885 |
| utf8.test | RUN | 0.543536 | 1.646364 | 1.782450 | 0.001919 | 0.001812 | 0.001780 |
| visit.test | RUN | 0.695115 | 2.772766 | 2.584984 | 0.001702 | 0.001860 | 0.001750 |
| d2s_intrinsics_test.test | RUN | 0.520710 | 1.469410 | 1.708275 | 0.001795 | 0.001795 | 0.001897 |
| d2s_table_test.test | RUN | 0.532380 | 1.672798 | 1.806546 | 0.001847 | 0.001730 | 0.001912 |
| d2s_test.test | RUN | 0.556860 | 2.011046 | 1.928539 | 0.002056 | 0.001763 | 0.001886 |
| parse_into.test | RUN | 17.341168 | 146.297536 | 87.678744 | 0.003276 | 0.002935 | 0.002808 |
| limits.test | RUN | 2.114399 | 11.941399 | 9.834888 | 0.001992 | 0.002168 | 0.002154 |
| header_only.test | RUN | 1.338346 | 9.429799 | 6.998307 | 0.001700 | 0.001676 | 0.001780 |
| no_exceptions.test | RUN | 2.032355 | 10.417879 | 9.370948 | 0.001874 | 0.001736 | 0.001941 |
| intrusive_macros.test | RUN | 1.601034 | 9.985853 | 8.183049 | 0.001744 | 0.001950 | 0.001936 |
| _gdb-printers.test | RUN | 0.287861 | 0.794651 | 0.981267 | 0.188149 | 0.261315 | 0.210008 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.820× | 0.837×, 0.803× | 0.995× |
| clang | 1.010× | 1.001×, 1.019× | 1.005× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267198).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| array.test | 10 | 0.001670s | 0.001534s | 0.001635s | byte equality observed |
| basic_parser.test | 1 | 0.357526s | 0.210579s | 0.440104s | native test oracle; output varies |
| conversion.test | 10 | 0.000979s | 0.000990s | 0.000970s | byte equality observed |
| d2s_intrinsics_test.test | 10 | 0.000983s | 0.001002s | 0.000974s | byte equality observed |
| d2s_table_test.test | 10 | 0.001004s | 0.001022s | 0.000980s | byte equality observed |
| d2s_test.test | 10 | 0.001074s | 0.001063s | 0.001031s | byte equality observed |
| doc_background.test | 10 | 0.001017s | 0.000999s | 0.000974s | byte equality observed |
| doc_forward_conversion.test | 10 | 0.001055s | 0.001042s | 0.001013s | byte equality observed |
| doc_parsing.test | 10 | 0.001063s | 0.001040s | 0.001030s | byte equality observed |
| doc_quick_look.test | 10 | 0.001031s | 0.001032s | 0.001007s | byte equality observed |
| doc_serializing.test | 10 | 0.001022s | 0.000985s | 0.000978s | byte equality observed |
| doc_storage_ptr.test | 10 | 0.001027s | 0.001016s | 0.001017s | byte equality observed |
| doc_uses_allocator.test | 10 | 0.001021s | 0.000988s | 0.000988s | byte equality observed |
| doc_using_numbers.test | 10 | 0.001032s | 0.000984s | 0.000988s | byte equality observed |
| double.test | 1 | 0.941631s | 0.822155s | 0.881269s | native test oracle; output varies |
| error.test | 10 | 0.001114s | 0.001058s | 0.001097s | byte equality observed |
| fwd.test | 10 | 0.000989s | 0.000975s | 0.000999s | byte equality observed |
| header_only.test | 10 | 0.000971s | 0.000959s | 0.000986s | byte equality observed |
| intrusive_macros.test | 10 | 0.001047s | 0.001021s | 0.001041s | byte equality observed |
| json.test | 10 | 0.001002s | 0.001003s | 0.001019s | byte equality observed |
| kind.test | 10 | 0.001030s | 0.001029s | 0.001032s | byte equality observed |
| limits.test | 10 | 0.001204s | 0.001174s | 0.001179s | byte equality observed |
| memory_resource.test | 10 | 0.001024s | 0.001019s | 0.001021s | byte equality observed |
| monotonic_resource.test | 10 | 0.001456s | 0.001354s | 0.001386s | byte equality observed |
| natvis.test | 10 | 0.001051s | 0.001029s | 0.001019s | byte equality observed |
| no_exceptions.test | 10 | 0.001015s | 0.001003s | 0.001011s | byte equality observed |
| null_resource.test | 10 | 0.001043s | 0.001043s | 0.001030s | byte equality observed |
| object.test | 5 | 0.003917s | 0.003232s | 0.003781s | native test oracle; output varies |
| parse.test | 10 | 0.001463s | 0.001443s | 0.001405s | byte equality observed |
| parse_into.test | 7 | 0.002354s | 0.002250s | 0.002134s | native test oracle; output varies |
| parser.test | 10 | 0.001157s | 0.001133s | 0.001122s | byte equality observed |
| pilfer.test | 10 | 0.000992s | 0.000986s | 0.000979s | byte equality observed |
| pointer.test | 10 | 0.001196s | 0.001174s | 0.001190s | byte equality observed |
| result_for.test | 10 | 0.000981s | 0.000994s | 0.000994s | byte equality observed |
| serialize.test | 10 | 0.001033s | 0.001051s | 0.001030s | byte equality observed |
| serializer.test | 10 | 0.001683s | 0.001662s | 0.001662s | byte equality observed |
| snippets.test | 10 | 0.001206s | 0.001183s | 0.001205s | byte equality observed |
| static_resource.test | 10 | 0.001079s | 0.001076s | 0.001083s | byte equality observed |
| storage_ptr.test | 10 | 0.001053s | 0.001057s | 0.001061s | byte equality observed |
| stream_parser.test | 1 | 0.050810s | 0.065110s | 0.046240s | native test oracle; output varies |
| string.test | 9 | 0.001677s | 0.001486s | 0.001675s | byte equality observed |
| string_view.test | 10 | 0.000998s | 0.000994s | 0.001000s | byte equality observed |
| utf8.test | 10 | 0.001008s | 0.001006s | 0.000994s | byte equality observed |
| value.test | 8 | 0.002138s | 0.001914s | 0.002346s | native test oracle; output varies |
| value_from.test | 10 | 0.001221s | 0.001159s | 0.001144s | byte equality observed |
| value_ref.test | 10 | 0.001060s | 0.001049s | 0.001058s | byte equality observed |
| value_stack.test | 10 | 0.001050s | 0.001037s | 0.001028s | byte equality observed |
| value_to.test | 7 | 0.001842s | 0.001746s | 0.001757s | native test oracle; output varies |
| visit.test | 10 | 0.001033s | 0.001022s | 0.001047s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267198).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
