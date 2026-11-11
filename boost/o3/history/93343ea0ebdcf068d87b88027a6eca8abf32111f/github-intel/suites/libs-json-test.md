# libs/json/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| array.test | RUN | 1.194196 | 6.202487 | 5.470298 | 0.002980 | 0.002840 | 0.002919 |
| basic_parser.test | RUN | 6.780661 | 152.403220 | 90.076342 | 0.792717 | 0.241893 | 0.941824 |
| conversion.test | RUN | 0.882579 | 2.327311 | 2.712995 | 0.001770 | 0.001773 | 0.001685 |
| doc_background.test | RUN | 0.882803 | 2.363435 | 2.693644 | 0.001718 | 0.001747 | 0.001588 |
| doc_forward_conversion.test | RUN | 0.879417 | 2.673002 | 3.054369 | 0.001754 | 0.001767 | 0.001719 |
| doc_parsing.test | RUN | 0.880751 | 2.554844 | 2.829961 | 0.001827 | 0.001754 | 0.001758 |
| doc_quick_look.test | RUN | 1.083299 | 3.141876 | 3.767730 | 0.001612 | 0.001698 | 0.001700 |
| doc_serializing.test | RUN | 0.808940 | 2.256932 | 2.561380 | 0.001740 | 0.001254 | 0.001540 |
| doc_storage_ptr.test | RUN | 0.814244 | 2.241838 | 2.617000 | 0.001601 | 0.001759 | 0.001812 |
| doc_uses_allocator.test | RUN | 0.820010 | 2.171637 | 2.645040 | 0.001830 | 0.001437 | 0.001667 |
| doc_using_numbers.test | RUN | 0.873345 | 2.298732 | 2.874931 | 0.001728 | 0.001825 | 0.001546 |
| double.test | RUN | 0.966359 | 3.978451 | 3.460850 | 2.023533 | 1.883391 | 1.899287 |
| error.test | RUN | 0.685268 | 2.224369 | 2.130753 | 0.001822 | 0.001786 | 0.001607 |
| fwd.test | RUN | 0.847614 | 2.199065 | 2.617545 | 0.001765 | 0.001652 | 0.001700 |
| json.test | RUN | 0.892388 | 2.584600 | 2.997030 | 0.001702 | 0.001796 | 0.001732 |
| kind.test | RUN | 0.675556 | 2.014446 | 2.111303 | 0.001798 | 0.001826 | 0.001740 |
| memory_resource.test | RUN | 0.892014 | 2.662729 | 2.778712 | 0.001794 | 0.001812 | 0.001742 |
| monotonic_resource.test | RUN | 0.846490 | 2.569071 | 2.716080 | 0.002875 | 0.002737 | 0.002745 |
| natvis.test | RUN | 0.842270 | 2.479119 | 2.560161 | 0.001912 | 0.001839 | 0.001708 |
| null_resource.test | RUN | 0.711511 | 2.054836 | 2.092606 | 0.001872 | 0.001842 | 0.001806 |
| object.test | RUN | 1.350359 | 7.448901 | 6.634430 | 0.008115 | 0.007099 | 0.007973 |
| parse.test | RUN | 0.897338 | 3.684068 | 3.030761 | 0.002558 | 0.002583 | 0.002351 |
| parser.test | RUN | 0.879157 | 3.176870 | 2.836774 | 0.001934 | 0.001938 | 0.001914 |
| pilfer.test | RUN | 0.468906 | 1.410870 | 1.280047 | 0.001722 | 0.001565 | 0.001649 |
| pointer.test | RUN | 0.985458 | 4.295519 | 3.676304 | 0.002010 | 0.002096 | 0.001750 |
| result_for.test | RUN | 0.838291 | 2.559171 | 2.705414 | 0.001779 | 0.001802 | 0.001675 |
| serialize.test | RUN | 0.855710 | 2.636365 | 2.743924 | 0.001863 | 0.001818 | 0.001770 |
| serializer.test | RUN | 1.361691 | 7.570732 | 6.107328 | 0.003300 | 0.003052 | 0.002981 |
| snippets.test | RUN | 1.950759 | 10.522628 | 9.267735 | 0.002176 | 0.002096 | 0.002016 |
| static_resource.test | RUN | 0.833726 | 2.430509 | 2.585771 | 0.001975 | 0.001905 | 0.001822 |
| storage_ptr.test | RUN | 0.889572 | 2.468286 | 2.801010 | 0.001856 | 0.001866 | 0.001778 |
| stream_parser.test | RUN | 1.023497 | 4.753012 | 3.913168 | 0.128919 | 0.143971 | 0.110898 |
| string.test | RUN | 1.678882 | 11.901412 | 9.655994 | 0.003336 | 0.002759 | 0.002896 |
| string_view.test | RUN | 0.523355 | 1.517319 | 1.402729 | 0.001770 | 0.001821 | 0.001747 |
| value.test | RUN | 1.715853 | 12.242459 | 8.285075 | 0.004130 | 0.003669 | 0.004551 |
| value_from.test | RUN | 1.495636 | 7.399429 | 6.713465 | 0.002153 | 0.002241 | 0.001934 |
| value_stack.test | RUN | 0.825632 | 2.248818 | 2.561042 | 0.001667 | 0.001643 | 0.001724 |
| value_to.test | RUN | 3.494560 | 23.098023 | 17.955784 | 0.003330 | 0.003480 | 0.003298 |
| value_ref.test | RUN | 0.934321 | 3.362816 | 3.207890 | 0.001585 | 0.001852 | 0.001700 |
| utf8.test | RUN | 0.699043 | 2.157629 | 2.164368 | 0.001746 | 0.001795 | 0.001688 |
| visit.test | RUN | 0.914539 | 3.252282 | 3.039378 | 0.001585 | 0.001859 | 0.001775 |
| d2s_intrinsics_test.test | RUN | 0.700524 | 1.957678 | 2.141745 | 0.001743 | 0.001659 | 0.001672 |
| d2s_table_test.test | RUN | 0.715037 | 2.075690 | 2.210967 | 0.001759 | 0.001672 | 0.001708 |
| d2s_test.test | RUN | 0.733090 | 2.679544 | 2.592416 | 0.001832 | 0.002026 | 0.001738 |
| parse_into.test | RUN | 22.322042 | 179.219438 | 109.863409 | 0.003186 | 0.002705 | 0.002671 |
| limits.test | RUN | 2.674329 | 15.916106 | 12.006641 | 0.002166 | 0.002143 | 0.002052 |
| header_only.test | RUN | 1.712099 | 12.469276 | 8.548694 | 0.001748 | 0.001758 | 0.001636 |
| no_exceptions.test | RUN | 2.541634 | 13.576457 | 11.384257 | 0.001818 | 0.001782 | 0.001785 |
| intrusive_macros.test | RUN | 2.102296 | 13.954300 | 10.047456 | 0.001750 | 0.001666 | 0.001474 |
| _gdb-printers.test | RUN | 0.383469 | 0.921567 | 1.207043 | 0.207263 | 0.342929 | 0.233374 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.804× | 0.802×, 0.806× | 1.010× |
| clang | 0.985× | 0.988×, 0.983× | 1.001× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| array.test | 10 | 0.001794s | 0.001553s | 0.001701s | native test oracle; output varies |
| basic_parser.test | 1 | 0.469496s | 0.257730s | 0.569914s | native test oracle; output varies |
| conversion.test | 10 | 0.000942s | 0.000945s | 0.000944s | byte equality observed |
| d2s_intrinsics_test.test | 10 | 0.000941s | 0.000949s | 0.000934s | byte equality observed |
| d2s_table_test.test | 10 | 0.000963s | 0.000962s | 0.000968s | byte equality observed |
| d2s_test.test | 10 | 0.001008s | 0.001001s | 0.000993s | byte equality observed |
| doc_background.test | 10 | 0.000941s | 0.000952s | 0.000964s | byte equality observed |
| doc_forward_conversion.test | 10 | 0.000986s | 0.001009s | 0.000995s | byte equality observed |
| doc_parsing.test | 10 | 0.001012s | 0.001010s | 0.000996s | byte equality observed |
| doc_quick_look.test | 10 | 0.000977s | 0.000982s | 0.000973s | byte equality observed |
| doc_serializing.test | 10 | 0.000954s | 0.000961s | 0.000946s | byte equality observed |
| doc_storage_ptr.test | 10 | 0.000980s | 0.000977s | 0.000961s | byte equality observed |
| doc_uses_allocator.test | 10 | 0.000977s | 0.000960s | 0.000939s | byte equality observed |
| doc_using_numbers.test | 10 | 0.000948s | 0.000962s | 0.000944s | byte equality observed |
| double.test | 1 | 1.284677s | 1.126322s | 1.161031s | native test oracle; output varies |
| error.test | 10 | 0.001025s | 0.001017s | 0.001032s | byte equality observed |
| fwd.test | 10 | 0.000943s | 0.000939s | 0.000966s | byte equality observed |
| header_only.test | 10 | 0.000954s | 0.000926s | 0.000947s | byte equality observed |
| intrusive_macros.test | 10 | 0.000992s | 0.000986s | 0.000973s | byte equality observed |
| json.test | 10 | 0.000958s | 0.000958s | 0.000974s | byte equality observed |
| kind.test | 10 | 0.000990s | 0.000984s | 0.000999s | byte equality observed |
| limits.test | 10 | 0.001177s | 0.001137s | 0.001164s | byte equality observed |
| memory_resource.test | 10 | 0.000976s | 0.000981s | 0.000979s | byte equality observed |
| monotonic_resource.test | 10 | 0.001634s | 0.001598s | 0.001610s | native test oracle; output varies |
| natvis.test | 10 | 0.000988s | 0.000996s | 0.000987s | byte equality observed |
| no_exceptions.test | 10 | 0.000990s | 0.000981s | 0.000990s | byte equality observed |
| null_resource.test | 10 | 0.001005s | 0.001020s | 0.001002s | byte equality observed |
| object.test | 5 | 0.004553s | 0.003664s | 0.004374s | native test oracle; output varies |
| parse.test | 10 | 0.001433s | 0.001395s | 0.001387s | byte equality observed |
| parse_into.test | 7 | 0.002329s | 0.002150s | 0.002120s | native test oracle; output varies |
| parser.test | 10 | 0.001115s | 0.001095s | 0.001111s | byte equality observed |
| pilfer.test | 10 | 0.000948s | 0.000954s | 0.000964s | byte equality observed |
| pointer.test | 10 | 0.001162s | 0.001150s | 0.001157s | byte equality observed |
| result_for.test | 10 | 0.000954s | 0.000976s | 0.000957s | byte equality observed |
| serialize.test | 10 | 0.001016s | 0.001011s | 0.001002s | byte equality observed |
| serializer.test | 10 | 0.001814s | 0.001789s | 0.001756s | native test oracle; output varies |
| snippets.test | 10 | 0.001154s | 0.001151s | 0.001161s | byte equality observed |
| static_resource.test | 10 | 0.001049s | 0.001032s | 0.001029s | byte equality observed |
| storage_ptr.test | 10 | 0.001017s | 0.001011s | 0.001008s | byte equality observed |
| stream_parser.test | 1 | 0.063887s | 0.075643s | 0.057983s | native test oracle; output varies |
| string.test | 9 | 0.001794s | 0.001544s | 0.001738s | native test oracle; output varies |
| string_view.test | 10 | 0.000957s | 0.000965s | 0.000956s | byte equality observed |
| utf8.test | 10 | 0.000960s | 0.000957s | 0.000970s | byte equality observed |
| value.test | 8 | 0.002233s | 0.002087s | 0.002579s | native test oracle; output varies |
| value_from.test | 10 | 0.001145s | 0.001120s | 0.001129s | byte equality observed |
| value_ref.test | 10 | 0.001034s | 0.001038s | 0.001020s | byte equality observed |
| value_stack.test | 10 | 0.001001s | 0.001013s | 0.000991s | byte equality observed |
| value_to.test | 10 | 0.001855s | 0.001859s | 0.001836s | native test oracle; output varies |
| visit.test | 10 | 0.000995s | 0.000987s | 0.000986s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
