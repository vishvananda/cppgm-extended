# libs/json/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| array.test | RUN | 0.965326 | 4.793032 | 4.636789 | 0.003065 | 0.002871 | 0.003066 |
| basic_parser.test | RUN | 4.406001 | 120.375303 | 73.828277 | 0.781851 | 0.198010 | 0.695171 |
| conversion.test | RUN | 0.743418 | 1.814647 | 2.360474 | 0.001784 | 0.001982 | 0.001623 |
| doc_background.test | RUN | 0.724814 | 1.823601 | 2.327201 | 0.001882 | 0.001940 | 0.001746 |
| doc_forward_conversion.test | RUN | 0.736741 | 2.051598 | 2.652877 | 0.002016 | 0.001986 | 0.001606 |
| doc_parsing.test | RUN | 0.732468 | 1.954145 | 2.410665 | 0.001984 | 0.001874 | 0.002044 |
| doc_quick_look.test | RUN | 0.918406 | 2.444994 | 3.254919 | 0.001730 | 0.001651 | 0.001884 |
| doc_serializing.test | RUN | 0.662796 | 1.726657 | 2.241074 | 0.001741 | 0.001707 | 0.001695 |
| doc_storage_ptr.test | RUN | 0.676695 | 1.737921 | 2.256831 | 0.001714 | 0.001871 | 0.001948 |
| doc_uses_allocator.test | RUN | 0.697104 | 1.786665 | 2.202205 | 0.001942 | 0.002016 | 0.002050 |
| doc_using_numbers.test | RUN | 0.728155 | 1.761374 | 2.465980 | 0.001793 | 0.001954 | 0.001857 |
| double.test | RUN | 0.799997 | 2.991014 | 2.788160 | 1.473891 | 1.341129 | 1.336314 |
| error.test | RUN | 0.546943 | 1.684187 | 1.895939 | 0.001786 | 0.001592 | 0.001892 |
| fwd.test | RUN | 0.660709 | 1.664969 | 2.391021 | 0.001631 | 0.001856 | 0.001864 |
| json.test | RUN | 0.713766 | 1.958965 | 2.539064 | 0.001664 | 0.001974 | 0.001853 |
| kind.test | RUN | 0.542805 | 1.591100 | 1.770089 | 0.001708 | 0.002056 | 0.001800 |
| memory_resource.test | RUN | 0.736438 | 2.022263 | 2.406924 | 0.001888 | 0.002067 | 0.002011 |
| monotonic_resource.test | RUN | 0.675707 | 1.953939 | 2.061712 | 0.002730 | 0.002549 | 0.002504 |
| natvis.test | RUN | 0.679105 | 1.938226 | 2.314350 | 0.001779 | 0.001923 | 0.001960 |
| null_resource.test | RUN | 0.570426 | 1.574854 | 1.735518 | 0.001978 | 0.002060 | 0.002044 |
| object.test | RUN | 1.070350 | 5.589132 | 5.467384 | 0.006897 | 0.005956 | 0.006828 |
| parse.test | RUN | 0.736844 | 2.854054 | 2.509691 | 0.002608 | 0.002688 | 0.002632 |
| parser.test | RUN | 0.721679 | 2.430056 | 2.252275 | 0.002124 | 0.002162 | 0.002138 |
| pilfer.test | RUN | 0.377697 | 1.114327 | 1.038704 | 0.001864 | 0.001741 | 0.001778 |
| pointer.test | RUN | 0.804595 | 3.257698 | 3.050843 | 0.002221 | 0.002332 | 0.001674 |
| result_for.test | RUN | 0.689523 | 1.983199 | 2.244071 | 0.001543 | 0.001925 | 0.001663 |
| serialize.test | RUN | 0.691385 | 2.060093 | 2.331184 | 0.001828 | 0.002052 | 0.001554 |
| serializer.test | RUN | 1.077175 | 5.711617 | 5.187128 | 0.002851 | 0.002721 | 0.002817 |
| snippets.test | RUN | 1.555912 | 8.173086 | 7.922608 | 0.002170 | 0.002246 | 0.002205 |
| static_resource.test | RUN | 0.681363 | 1.876346 | 2.210472 | 0.002037 | 0.001988 | 0.001742 |
| storage_ptr.test | RUN | 0.716662 | 1.906698 | 2.155733 | 0.002240 | 0.001877 | 0.001878 |
| stream_parser.test | RUN | 0.873256 | 3.541220 | 2.952924 | 0.087511 | 0.118519 | 0.085478 |
| string.test | RUN | 1.326465 | 8.135600 | 7.967694 | 0.003030 | 0.002620 | 0.002970 |
| string_view.test | RUN | 0.415670 | 1.192445 | 1.376863 | 0.001844 | 0.001974 | 0.001868 |
| value.test | RUN | 1.316322 | 9.517161 | 6.353625 | 0.003613 | 0.003519 | 0.004180 |
| value_from.test | RUN | 1.251386 | 5.482237 | 5.729107 | 0.002222 | 0.002028 | 0.002130 |
| value_stack.test | RUN | 0.692791 | 1.701799 | 2.052901 | 0.001940 | 0.001824 | 0.001891 |
| value_to.test | RUN | 2.661903 | 17.692888 | 13.947042 | 0.003223 | 0.003139 | 0.002842 |
| value_ref.test | RUN | 0.759238 | 2.408770 | 2.834919 | 0.002000 | 0.002119 | 0.002122 |
| utf8.test | RUN | 0.570741 | 1.483032 | 1.892601 | 0.001892 | 0.001806 | 0.001855 |
| visit.test | RUN | 0.778311 | 2.611080 | 2.652608 | 0.001998 | 0.002030 | 0.001897 |
| d2s_intrinsics_test.test | RUN | 0.576136 | 1.539885 | 1.825408 | 0.001863 | 0.002124 | 0.001874 |
| d2s_table_test.test | RUN | 0.582779 | 1.710071 | 1.713292 | 0.001901 | 0.002013 | 0.002014 |
| d2s_test.test | RUN | 0.614242 | 2.026994 | 2.159052 | 0.002036 | 0.002113 | 0.002332 |
| parse_into.test | RUN | 15.539443 | 147.299728 | 91.323912 | 0.003319 | 0.002988 | 0.002862 |
| limits.test | RUN | 2.097208 | 12.499559 | 9.336347 | 0.002373 | 0.002156 | 0.002208 |
| header_only.test | RUN | 1.356516 | 9.317655 | 7.353793 | 0.001896 | 0.001664 | 0.001538 |
| no_exceptions.test | RUN | 1.958655 | 10.479615 | 8.662926 | 0.001993 | 0.001934 | 0.001870 |
| intrusive_macros.test | RUN | 1.624281 | 10.593864 | 8.080605 | 0.001810 | 0.001964 | 0.001954 |
| _gdb-printers.test | RUN | 0.317324 | 0.824830 | 1.060171 | 0.184450 | 0.284221 | 0.208877 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.730× | 0.731×, 0.730× | 1.008× |
| clang | 0.885× | 0.885×, 0.885× | 1.001× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| array.test | 9 | 0.001742s | 0.001527s | 0.001698s | native test oracle; output varies |
| basic_parser.test | 1 | 0.499398s | 0.211251s | 0.449857s | native test oracle; output varies |
| conversion.test | 10 | 0.001009s | 0.001020s | 0.000994s | byte equality observed |
| d2s_intrinsics_test.test | 10 | 0.000999s | 0.001020s | 0.000991s | byte equality observed |
| d2s_table_test.test | 10 | 0.001008s | 0.001032s | 0.001007s | byte equality observed |
| d2s_test.test | 10 | 0.001077s | 0.001083s | 0.001072s | byte equality observed |
| doc_background.test | 10 | 0.001035s | 0.001015s | 0.000991s | byte equality observed |
| doc_forward_conversion.test | 10 | 0.001061s | 0.001065s | 0.001046s | byte equality observed |
| doc_parsing.test | 10 | 0.001084s | 0.001065s | 0.001039s | byte equality observed |
| doc_quick_look.test | 10 | 0.001046s | 0.001031s | 0.001027s | byte equality observed |
| doc_serializing.test | 10 | 0.001040s | 0.001000s | 0.000999s | byte equality observed |
| doc_storage_ptr.test | 10 | 0.001045s | 0.001038s | 0.001021s | byte equality observed |
| doc_uses_allocator.test | 10 | 0.001032s | 0.001052s | 0.000991s | byte equality observed |
| doc_using_numbers.test | 10 | 0.001037s | 0.001010s | 0.000991s | byte equality observed |
| double.test | 1 | 0.991904s | 0.837785s | 0.870457s | native test oracle; output varies |
| error.test | 8 | 0.001128s | 0.001040s | 0.001108s | byte equality observed |
| fwd.test | 10 | 0.001025s | 0.000985s | 0.001031s | byte equality observed |
| header_only.test | 10 | 0.001013s | 0.000974s | 0.001002s | byte equality observed |
| intrusive_macros.test | 10 | 0.001063s | 0.001017s | 0.001066s | byte equality observed |
| json.test | 10 | 0.001036s | 0.001024s | 0.001040s | byte equality observed |
| kind.test | 10 | 0.001063s | 0.001029s | 0.001056s | byte equality observed |
| limits.test | 10 | 0.001252s | 0.001181s | 0.001222s | byte equality observed |
| memory_resource.test | 10 | 0.001056s | 0.001023s | 0.001055s | byte equality observed |
| monotonic_resource.test | 10 | 0.001517s | 0.001336s | 0.001475s | byte equality observed |
| natvis.test | 10 | 0.001074s | 0.001043s | 0.001066s | byte equality observed |
| no_exceptions.test | 10 | 0.001065s | 0.001028s | 0.001051s | byte equality observed |
| null_resource.test | 10 | 0.001089s | 0.001042s | 0.001083s | byte equality observed |
| object.test | 5 | 0.003991s | 0.003228s | 0.004025s | native test oracle; output varies |
| parse.test | 10 | 0.001486s | 0.001411s | 0.001446s | byte equality observed |
| parse_into.test | 7 | 0.002499s | 0.002195s | 0.002167s | native test oracle; output varies |
| parser.test | 10 | 0.001193s | 0.001134s | 0.001170s | byte equality observed |
| pilfer.test | 10 | 0.001021s | 0.000983s | 0.001028s | byte equality observed |
| pointer.test | 10 | 0.001248s | 0.001177s | 0.001211s | byte equality observed |
| result_for.test | 10 | 0.001025s | 0.000990s | 0.001024s | byte equality observed |
| serialize.test | 10 | 0.001082s | 0.001044s | 0.001061s | byte equality observed |
| serializer.test | 10 | 0.001733s | 0.001663s | 0.001692s | byte equality observed |
| snippets.test | 10 | 0.001246s | 0.001194s | 0.001229s | byte equality observed |
| static_resource.test | 10 | 0.001124s | 0.001075s | 0.001101s | byte equality observed |
| storage_ptr.test | 10 | 0.001102s | 0.001049s | 0.001090s | byte equality observed |
| stream_parser.test | 1 | 0.052388s | 0.064870s | 0.046358s | native test oracle; output varies |
| string.test | 9 | 0.001806s | 0.001512s | 0.001683s | byte equality observed |
| string_view.test | 10 | 0.001025s | 0.001003s | 0.001021s | byte equality observed |
| utf8.test | 10 | 0.001027s | 0.000993s | 0.001019s | byte equality observed |
| value.test | 8 | 0.002161s | 0.001925s | 0.002363s | native test oracle; output varies |
| value_from.test | 10 | 0.001221s | 0.001157s | 0.001179s | byte equality observed |
| value_ref.test | 10 | 0.001096s | 0.001071s | 0.001081s | byte equality observed |
| value_stack.test | 10 | 0.001071s | 0.001044s | 0.001061s | byte equality observed |
| value_to.test | 8 | 0.001862s | 0.001759s | 0.001787s | native test oracle; output varies |
| visit.test | 10 | 0.001077s | 0.001031s | 0.001055s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
