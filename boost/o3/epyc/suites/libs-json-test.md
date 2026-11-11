# libs/json/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| array.test | RUN | 0.859125 | 4.541436 | 3.806764 | 0.002767 | 0.002684 | 0.002681 |
| basic_parser.test | RUN | 4.148476 | 149.031929 | 74.614236 | 0.535310 | 0.147188 | 0.339948 |
| conversion.test | RUN | 0.649019 | 1.681379 | 1.807437 | 0.001933 | 0.002146 | 0.001492 |
| doc_background.test | RUN | 0.651660 | 1.619455 | 1.804276 | 0.001959 | 0.001972 | 0.001656 |
| doc_forward_conversion.test | RUN | 0.652432 | 1.895494 | 2.071481 | 0.002309 | 0.002229 | 0.001610 |
| doc_parsing.test | RUN | 0.660173 | 1.818021 | 1.835781 | 0.002128 | 0.002008 | 0.001580 |
| doc_quick_look.test | RUN | 0.840891 | 2.253792 | 2.534694 | 0.002012 | 0.002066 | 0.001822 |
| doc_serializing.test | RUN | 0.613707 | 1.670477 | 1.728074 | 0.001862 | 0.001674 | 0.001912 |
| doc_storage_ptr.test | RUN | 0.605259 | 1.576803 | 1.730231 | 0.002077 | 0.001993 | 0.001876 |
| doc_uses_allocator.test | RUN | 0.622369 | 1.581299 | 1.705700 | 0.002010 | 0.001714 | 0.001902 |
| doc_using_numbers.test | RUN | 0.638493 | 1.681041 | 1.931497 | 0.001872 | 0.001938 | 0.002059 |
| double.test | RUN | 0.700140 | 3.044135 | 2.153136 | 1.168333 | 0.908229 | 1.121649 |
| error.test | RUN | 0.495446 | 1.554071 | 1.424409 | 0.001817 | 0.002114 | 0.001976 |
| fwd.test | RUN | 0.575549 | 1.669304 | 1.684795 | 0.001768 | 0.002190 | 0.001847 |
| json.test | RUN | 0.638525 | 1.989970 | 1.980587 | 0.001909 | 0.002198 | 0.002066 |
| kind.test | RUN | 0.499307 | 1.465679 | 1.328710 | 0.001944 | 0.002060 | 0.001991 |
| memory_resource.test | RUN | 0.643455 | 1.878326 | 1.811761 | 0.001599 | 0.002262 | 0.002042 |
| monotonic_resource.test | RUN | 0.604219 | 1.889807 | 1.729595 | 0.002322 | 0.002643 | 0.002543 |
| natvis.test | RUN | 0.637107 | 1.771307 | 1.813910 | 0.001976 | 0.002148 | 0.001946 |
| null_resource.test | RUN | 0.514815 | 1.438914 | 1.373779 | 0.001914 | 0.002360 | 0.002086 |
| object.test | RUN | 0.979248 | 5.385214 | 4.416522 | 0.005564 | 0.005180 | 0.005490 |
| parse.test | RUN | 0.672167 | 2.752390 | 2.032042 | 0.002278 | 0.002667 | 0.002494 |
| parser.test | RUN | 0.669830 | 2.462289 | 1.752508 | 0.002208 | 0.002118 | 0.002158 |
| pilfer.test | RUN | 0.336844 | 1.062041 | 0.819367 | 0.002012 | 0.001990 | 0.001954 |
| pointer.test | RUN | 0.719432 | 3.076540 | 2.358310 | 0.002015 | 0.002294 | 0.002344 |
| result_for.test | RUN | 0.652028 | 1.901772 | 2.027662 | 0.001838 | 0.002062 | 0.001946 |
| serialize.test | RUN | 0.620847 | 1.914306 | 1.703434 | 0.001965 | 0.002173 | 0.002074 |
| serializer.test | RUN | 0.934912 | 5.281173 | 4.364709 | 0.003061 | 0.002928 | 0.003114 |
| snippets.test | RUN | 1.438459 | 7.813033 | 6.497082 | 0.002303 | 0.002436 | 0.001968 |
| static_resource.test | RUN | 0.626826 | 1.790854 | 1.686541 | 0.002194 | 0.002166 | 0.001913 |
| storage_ptr.test | RUN | 0.652728 | 1.796417 | 1.770668 | 0.002118 | 0.002185 | 0.002102 |
| stream_parser.test | RUN | 0.768642 | 3.210602 | 2.604062 | 0.065903 | 0.078849 | 0.064096 |
| string.test | RUN | 1.167351 | 8.539479 | 6.672709 | 0.002660 | 0.002677 | 0.002965 |
| string_view.test | RUN | 0.376720 | 1.128202 | 1.108913 | 0.001879 | 0.002100 | 0.002065 |
| value.test | RUN | 1.197039 | 9.490092 | 5.877385 | 0.003214 | 0.003096 | 0.003862 |
| value_from.test | RUN | 1.129682 | 5.882275 | 4.719032 | 0.002216 | 0.002462 | 0.002072 |
| value_stack.test | RUN | 0.594602 | 1.644740 | 1.592460 | 0.001941 | 0.002165 | 0.001946 |
| value_to.test | RUN | 2.343976 | 18.183820 | 11.332091 | 0.003155 | 0.003190 | 0.003345 |
| value_ref.test | RUN | 0.688450 | 2.308822 | 2.184072 | 0.002004 | 0.002113 | 0.002186 |
| utf8.test | RUN | 0.507712 | 1.527994 | 1.508495 | 0.001902 | 0.001984 | 0.001940 |
| visit.test | RUN | 0.684392 | 2.499584 | 2.045295 | 0.001668 | 0.002254 | 0.001971 |
| d2s_intrinsics_test.test | RUN | 0.514837 | 1.329633 | 1.375836 | 0.002122 | 0.001896 | 0.001915 |
| d2s_table_test.test | RUN | 0.533275 | 1.625671 | 1.461591 | 0.002014 | 0.002111 | 0.001892 |
| d2s_test.test | RUN | 0.533116 | 1.851701 | 1.633946 | 0.002086 | 0.002471 | 0.001940 |
| parse_into.test | RUN | 13.877938 | 145.734635 | 83.578915 | 0.003663 | 0.003628 | 0.003079 |
| limits.test | RUN | 1.857515 | 11.700494 | 8.121852 | 0.002302 | 0.002418 | 0.001929 |
| header_only.test | RUN | 1.159099 | 9.051154 | 5.554575 | 0.002167 | 0.001898 | 0.001429 |
| no_exceptions.test | RUN | 1.736208 | 10.441144 | 7.305507 | 0.002010 | 0.002302 | 0.001776 |
| intrusive_macros.test | RUN | 1.416885 | 9.876700 | 6.918405 | 0.001920 | 0.002174 | 0.001981 |
| _gdb-printers.test | RUN | 0.285651 | 0.766167 | 0.843553 | 0.164155 | 0.228647 | 0.240905 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.701× | 0.699×, 0.702× | 1.009× |
| clang | 0.918× | 0.933×, 0.902× | 0.980× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| array.test | 10 | 0.001597s | 0.001360s | 0.001467s | native test oracle; output varies |
| basic_parser.test | 1 | 0.343532s | 0.168264s | 0.339386s | native test oracle; output varies |
| conversion.test | 10 | 0.000958s | 0.000941s | 0.000947s | byte equality observed |
| d2s_intrinsics_test.test | 10 | 0.000961s | 0.000914s | 0.000939s | byte equality observed |
| d2s_table_test.test | 10 | 0.000972s | 0.000943s | 0.000981s | byte equality observed |
| d2s_test.test | 10 | 0.001017s | 0.001016s | 0.001053s | byte equality observed |
| doc_background.test | 10 | 0.001000s | 0.000940s | 0.000966s | byte equality observed |
| doc_forward_conversion.test | 10 | 0.001013s | 0.000973s | 0.001050s | byte equality observed |
| doc_parsing.test | 10 | 0.001019s | 0.000982s | 0.001062s | byte equality observed |
| doc_quick_look.test | 10 | 0.000983s | 0.000962s | 0.000957s | byte equality observed |
| doc_serializing.test | 10 | 0.000990s | 0.000900s | 0.001018s | byte equality observed |
| doc_storage_ptr.test | 10 | 0.000996s | 0.000947s | 0.001051s | byte equality observed |
| doc_uses_allocator.test | 10 | 0.000996s | 0.000985s | 0.000950s | byte equality observed |
| doc_using_numbers.test | 10 | 0.000984s | 0.000964s | 0.000958s | byte equality observed |
| double.test | 1 | 0.811537s | 0.639854s | 0.717566s | native test oracle; output varies |
| error.test | 10 | 0.001142s | 0.000972s | 0.001079s | byte equality observed |
| fwd.test | 10 | 0.000952s | 0.000975s | 0.000979s | byte equality observed |
| header_only.test | 10 | 0.000938s | 0.000929s | 0.000931s | byte equality observed |
| intrusive_macros.test | 10 | 0.000975s | 0.000990s | 0.000984s | byte equality observed |
| json.test | 10 | 0.000937s | 0.000969s | 0.000972s | byte equality observed |
| kind.test | 10 | 0.000969s | 0.001008s | 0.001004s | byte equality observed |
| limits.test | 10 | 0.001145s | 0.001146s | 0.001117s | byte equality observed |
| memory_resource.test | 10 | 0.000967s | 0.000973s | 0.000965s | byte equality observed |
| monotonic_resource.test | 10 | 0.001362s | 0.001268s | 0.001321s | byte equality observed |
| natvis.test | 10 | 0.000998s | 0.000992s | 0.000969s | byte equality observed |
| no_exceptions.test | 10 | 0.000981s | 0.000999s | 0.000980s | byte equality observed |
| null_resource.test | 10 | 0.001001s | 0.001010s | 0.000992s | byte equality observed |
| object.test | 6 | 0.003171s | 0.002602s | 0.003366s | native test oracle; output varies |
| parse.test | 9 | 0.001417s | 0.001345s | 0.001334s | byte equality observed |
| parse_into.test | 5 | 0.002727s | 0.002502s | 0.002400s | native test oracle; output varies |
| parser.test | 10 | 0.001134s | 0.001118s | 0.001095s | byte equality observed |
| pilfer.test | 10 | 0.000959s | 0.000972s | 0.000932s | byte equality observed |
| pointer.test | 10 | 0.001146s | 0.001160s | 0.001124s | byte equality observed |
| result_for.test | 10 | 0.000937s | 0.000947s | 0.000927s | byte equality observed |
| serialize.test | 10 | 0.000996s | 0.001023s | 0.000985s | byte equality observed |
| serializer.test | 9 | 0.001577s | 0.001522s | 0.001529s | byte equality observed |
| snippets.test | 10 | 0.001172s | 0.001175s | 0.001194s | byte equality observed |
| static_resource.test | 10 | 0.001062s | 0.001050s | 0.000982s | byte equality observed |
| storage_ptr.test | 10 | 0.001029s | 0.001014s | 0.000968s | byte equality observed |
| stream_parser.test | 1 | 0.042068s | 0.046398s | 0.039703s | native test oracle; output varies |
| string.test | 9 | 0.001721s | 0.001566s | 0.001758s | native test oracle; output varies |
| string_view.test | 10 | 0.000975s | 0.000962s | 0.000963s | byte equality observed |
| utf8.test | 10 | 0.000966s | 0.000954s | 0.000959s | byte equality observed |
| value.test | 8 | 0.001928s | 0.001740s | 0.002114s | native test oracle; output varies |
| value_from.test | 10 | 0.001168s | 0.001129s | 0.001188s | byte equality observed |
| value_ref.test | 10 | 0.001061s | 0.001034s | 0.001017s | byte equality observed |
| value_stack.test | 10 | 0.001028s | 0.001036s | 0.000998s | byte equality observed |
| value_to.test | 9 | 0.001690s | 0.001633s | 0.001718s | native test oracle; output varies |
| visit.test | 10 | 0.000986s | 0.001023s | 0.001005s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
