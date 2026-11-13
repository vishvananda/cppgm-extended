# libs/json/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| array.test | RUN | 0.860237 | 4.738557 | 4.462241 | 0.002839 | 0.002423 | 0.002768 |
| basic_parser.test | RUN | 5.197315 | 114.382095 | 71.773633 | 0.555411 | 0.198448 | 0.707153 |
| conversion.test | RUN | 0.646442 | 1.801140 | 2.272300 | 0.001613 | 0.001734 | 0.001791 |
| doc_background.test | RUN | 0.652509 | 1.803394 | 2.282536 | 0.001774 | 0.001806 | 0.001777 |
| doc_forward_conversion.test | RUN | 0.682275 | 2.101499 | 2.577828 | 0.001608 | 0.001978 | 0.001894 |
| doc_parsing.test | RUN | 0.657700 | 1.927993 | 2.395648 | 0.001881 | 0.001769 | 0.001882 |
| doc_quick_look.test | RUN | 0.851536 | 2.370042 | 3.164065 | 0.001769 | 0.001689 | 0.001309 |
| doc_serializing.test | RUN | 0.597072 | 1.702878 | 2.221144 | 0.001591 | 0.001542 | 0.001696 |
| doc_storage_ptr.test | RUN | 0.607680 | 1.822471 | 2.180477 | 0.001514 | 0.001805 | 0.001798 |
| doc_uses_allocator.test | RUN | 0.629434 | 1.756059 | 2.202022 | 0.001858 | 0.001915 | 0.001824 |
| doc_using_numbers.test | RUN | 0.653800 | 1.738643 | 2.423271 | 0.001714 | 0.001840 | 0.001759 |
| double.test | RUN | 0.701456 | 3.004871 | 2.841818 | 1.439478 | 1.256898 | 1.423859 |
| error.test | RUN | 0.506035 | 1.646398 | 1.706338 | 0.001814 | 0.001794 | 0.001651 |
| fwd.test | RUN | 0.586753 | 1.799544 | 2.034459 | 0.001734 | 0.001809 | 0.001633 |
| json.test | RUN | 0.641255 | 1.993809 | 2.497643 | 0.001778 | 0.001748 | 0.001671 |
| kind.test | RUN | 0.479588 | 1.620986 | 1.798250 | 0.001685 | 0.001954 | 0.001893 |
| memory_resource.test | RUN | 0.666535 | 2.028693 | 2.342415 | 0.001564 | 0.001948 | 0.001949 |
| monotonic_resource.test | RUN | 0.610567 | 1.947248 | 2.126446 | 0.002560 | 0.002448 | 0.002575 |
| natvis.test | RUN | 0.614631 | 1.830860 | 2.120881 | 0.001761 | 0.001824 | 0.001810 |
| null_resource.test | RUN | 0.509404 | 1.613961 | 1.865423 | 0.001786 | 0.002084 | 0.001980 |
| object.test | RUN | 1.007256 | 5.582374 | 4.802202 | 0.006266 | 0.005622 | 0.006161 |
| parse.test | RUN | 0.666120 | 2.815344 | 2.666688 | 0.002283 | 0.002572 | 0.002515 |
| parser.test | RUN | 0.646018 | 2.397249 | 2.484340 | 0.002067 | 0.001694 | 0.001918 |
| pilfer.test | RUN | 0.342145 | 1.090551 | 1.173549 | 0.001724 | 0.001804 | 0.001544 |
| pointer.test | RUN | 0.725252 | 3.255380 | 2.886683 | 0.002116 | 0.002173 | 0.001944 |
| result_for.test | RUN | 0.623166 | 1.944690 | 2.413738 | 0.001680 | 0.001932 | 0.001882 |
| serialize.test | RUN | 0.639299 | 2.019996 | 2.143248 | 0.001716 | 0.001996 | 0.001840 |
| serializer.test | RUN | 0.966187 | 5.773430 | 5.562814 | 0.002794 | 0.002848 | 0.002696 |
| snippets.test | RUN | 1.527154 | 8.048184 | 7.034803 | 0.002144 | 0.002194 | 0.001995 |
| static_resource.test | RUN | 0.622820 | 1.779610 | 1.988182 | 0.002042 | 0.001882 | 0.001664 |
| storage_ptr.test | RUN | 0.637668 | 1.884414 | 2.386097 | 0.001952 | 0.001936 | 0.002022 |
| stream_parser.test | RUN | 0.754552 | 3.327342 | 3.387239 | 0.096428 | 0.105269 | 0.088937 |
| string.test | RUN | 1.251528 | 8.634226 | 7.145996 | 0.002971 | 0.002397 | 0.002837 |
| string_view.test | RUN | 0.376336 | 1.119813 | 1.330641 | 0.001837 | 0.001707 | 0.001865 |
| value.test | RUN | 1.236772 | 9.332612 | 7.031329 | 0.003439 | 0.003472 | 0.003922 |
| value_from.test | RUN | 1.153225 | 5.236719 | 5.638697 | 0.002184 | 0.002052 | 0.002150 |
| value_stack.test | RUN | 0.607765 | 1.701337 | 2.034830 | 0.001737 | 0.001885 | 0.001862 |
| value_to.test | RUN | 2.817256 | 17.164589 | 13.530560 | 0.003206 | 0.003010 | 0.002776 |
| value_ref.test | RUN | 0.692064 | 2.399534 | 2.811030 | 0.001990 | 0.002106 | 0.001922 |
| utf8.test | RUN | 0.516329 | 1.724469 | 1.866678 | 0.001773 | 0.002009 | 0.001870 |
| visit.test | RUN | 0.675453 | 2.506016 | 2.614050 | 0.001914 | 0.001756 | 0.001786 |
| d2s_intrinsics_test.test | RUN | 0.502208 | 1.508112 | 1.818337 | 0.001854 | 0.001817 | 0.001888 |
| d2s_table_test.test | RUN | 0.511706 | 1.639941 | 1.884436 | 0.001834 | 0.001798 | 0.001884 |
| d2s_test.test | RUN | 0.539508 | 1.990624 | 2.126268 | 0.001901 | 0.001816 | 0.001934 |
| parse_into.test | RUN | 16.829936 | 144.584244 | 89.210259 | 0.003046 | 0.002940 | 0.002733 |
| limits.test | RUN | 2.034039 | 12.556963 | 9.295344 | 0.002059 | 0.001966 | 0.002132 |
| header_only.test | RUN | 1.265372 | 9.486267 | 7.187079 | 0.001810 | 0.001791 | 0.001700 |
| no_exceptions.test | RUN | 1.888192 | 10.075064 | 8.926296 | 0.001686 | 0.001817 | 0.001880 |
| intrusive_macros.test | RUN | 1.583052 | 10.391678 | 7.416751 | 0.001587 | 0.001889 | 0.001808 |
| _gdb-printers.test | RUN | 0.288899 | 0.799468 | 0.949384 | 0.202476 | 0.287838 | 0.209797 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.836× | 0.837×, 0.835× | 1.022× |
| clang | 1.021× | 1.034×, 1.008× | 1.021× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224918).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| array.test | 10 | 0.001677s | 0.001484s | 0.001649s | byte equality observed |
| basic_parser.test | 1 | 0.364088s | 0.211137s | 0.442302s | native test oracle; output varies |
| conversion.test | 10 | 0.000989s | 0.000978s | 0.000978s | byte equality observed |
| d2s_intrinsics_test.test | 10 | 0.000982s | 0.000972s | 0.000979s | byte equality observed |
| d2s_table_test.test | 10 | 0.001007s | 0.000991s | 0.001011s | byte equality observed |
| d2s_test.test | 10 | 0.001066s | 0.001036s | 0.001040s | byte equality observed |
| doc_background.test | 10 | 0.001012s | 0.000968s | 0.000999s | byte equality observed |
| doc_forward_conversion.test | 10 | 0.001030s | 0.001018s | 0.001025s | byte equality observed |
| doc_parsing.test | 10 | 0.001046s | 0.001031s | 0.001022s | byte equality observed |
| doc_quick_look.test | 10 | 0.001018s | 0.000996s | 0.000998s | byte equality observed |
| doc_serializing.test | 10 | 0.001016s | 0.000970s | 0.000979s | byte equality observed |
| doc_storage_ptr.test | 10 | 0.001012s | 0.000998s | 0.000996s | byte equality observed |
| doc_uses_allocator.test | 10 | 0.001027s | 0.000981s | 0.000993s | byte equality observed |
| doc_using_numbers.test | 10 | 0.001017s | 0.000982s | 0.000979s | byte equality observed |
| double.test | 1 | 0.906879s | 0.835568s | 0.870536s | native test oracle; output varies |
| error.test | 10 | 0.001070s | 0.001012s | 0.001055s | byte equality observed |
| fwd.test | 10 | 0.000974s | 0.000962s | 0.000989s | byte equality observed |
| header_only.test | 10 | 0.000962s | 0.000949s | 0.000954s | byte equality observed |
| intrusive_macros.test | 10 | 0.001020s | 0.000988s | 0.001016s | byte equality observed |
| json.test | 10 | 0.001004s | 0.000976s | 0.001004s | byte equality observed |
| kind.test | 10 | 0.001022s | 0.001015s | 0.001005s | byte equality observed |
| limits.test | 10 | 0.001200s | 0.001168s | 0.001173s | byte equality observed |
| memory_resource.test | 10 | 0.001025s | 0.001012s | 0.000995s | byte equality observed |
| monotonic_resource.test | 10 | 0.001461s | 0.001326s | 0.001380s | byte equality observed |
| natvis.test | 10 | 0.001037s | 0.001021s | 0.001011s | byte equality observed |
| no_exceptions.test | 10 | 0.001028s | 0.001004s | 0.001006s | byte equality observed |
| null_resource.test | 10 | 0.001057s | 0.001024s | 0.001022s | byte equality observed |
| object.test | 5 | 0.003948s | 0.003205s | 0.003780s | native test oracle; output varies |
| parse.test | 10 | 0.001469s | 0.001391s | 0.001415s | byte equality observed |
| parse_into.test | 7 | 0.002368s | 0.002168s | 0.002097s | native test oracle; output varies |
| parser.test | 10 | 0.001149s | 0.001099s | 0.001113s | byte equality observed |
| pilfer.test | 10 | 0.000981s | 0.000961s | 0.000980s | byte equality observed |
| pointer.test | 10 | 0.001200s | 0.001151s | 0.001173s | byte equality observed |
| result_for.test | 10 | 0.000995s | 0.000969s | 0.000980s | byte equality observed |
| serialize.test | 10 | 0.001049s | 0.001025s | 0.001034s | byte equality observed |
| serializer.test | 10 | 0.001699s | 0.001654s | 0.001647s | native test oracle; output varies |
| snippets.test | 10 | 0.001207s | 0.001166s | 0.001173s | byte equality observed |
| static_resource.test | 10 | 0.001078s | 0.001048s | 0.001063s | byte equality observed |
| storage_ptr.test | 10 | 0.001052s | 0.001035s | 0.001045s | byte equality observed |
| stream_parser.test | 1 | 0.053525s | 0.065297s | 0.046060s | native test oracle; output varies |
| string.test | 10 | 0.001682s | 0.001461s | 0.001642s | byte equality observed |
| string_view.test | 10 | 0.000991s | 0.000968s | 0.000990s | byte equality observed |
| utf8.test | 10 | 0.000991s | 0.000987s | 0.000993s | byte equality observed |
| value.test | 8 | 0.002116s | 0.001894s | 0.002317s | native test oracle; output varies |
| value_from.test | 10 | 0.001227s | 0.001132s | 0.001151s | byte equality observed |
| value_ref.test | 10 | 0.001055s | 0.001022s | 0.001040s | byte equality observed |
| value_stack.test | 10 | 0.001036s | 0.001012s | 0.001032s | byte equality observed |
| value_to.test | 10 | 0.001871s | 0.001752s | 0.001747s | native test oracle; output varies |
| visit.test | 10 | 0.001036s | 0.001001s | 0.001042s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224918).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
