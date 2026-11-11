# libs/json/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| array.test | RUN | 0.714183 | 3.697176 | 3.013526 | 0.002481 | 0.002260 | 0.002375 |
| basic_parser.test | RUN | 3.671213 | 89.549657 | 52.437889 | 0.408932 | 0.140891 | 0.522804 |
| conversion.test | RUN | 0.540688 | 1.336718 | 1.489342 | 0.001366 | 0.001602 | 0.001498 |
| doc_background.test | RUN | 0.527473 | 1.332989 | 1.447463 | 0.001508 | 0.001478 | 0.001394 |
| doc_forward_conversion.test | RUN | 0.536567 | 1.511547 | 1.706272 | 0.001736 | 0.001562 | 0.001557 |
| doc_parsing.test | RUN | 0.551670 | 1.456918 | 1.543805 | 0.001707 | 0.001506 | 0.001512 |
| doc_quick_look.test | RUN | 0.665317 | 1.756727 | 2.055065 | 0.001382 | 0.001563 | 0.001587 |
| doc_serializing.test | RUN | 0.491084 | 1.288598 | 1.415685 | 0.001453 | 0.001379 | 0.001618 |
| doc_storage_ptr.test | RUN | 0.494768 | 1.294403 | 1.432865 | 0.001480 | 0.001413 | 0.001643 |
| doc_uses_allocator.test | RUN | 0.503801 | 1.300380 | 1.428779 | 0.001722 | 0.001681 | 0.001572 |
| doc_using_numbers.test | RUN | 0.536697 | 1.300357 | 1.515509 | 0.001583 | 0.001613 | 0.001746 |
| double.test | RUN | 0.577735 | 2.204543 | 1.938112 | 1.085940 | 0.956981 | 1.042946 |
| error.test | RUN | 0.422330 | 1.244832 | 1.115801 | 0.001550 | 0.001648 | 0.001674 |
| fwd.test | RUN | 0.494517 | 1.232262 | 1.495444 | 0.001409 | 0.001579 | 0.001698 |
| json.test | RUN | 0.495735 | 1.472425 | 1.676039 | 0.001452 | 0.001657 | 0.001531 |
| kind.test | RUN | 0.386818 | 1.162184 | 1.236006 | 0.001358 | 0.001648 | 0.001658 |
| memory_resource.test | RUN | 0.528793 | 1.555895 | 1.582390 | 0.001646 | 0.001826 | 0.001694 |
| monotonic_resource.test | RUN | 0.496634 | 1.460701 | 1.447933 | 0.001856 | 0.002106 | 0.002066 |
| natvis.test | RUN | 0.513983 | 1.373873 | 1.423671 | 0.001537 | 0.001572 | 0.001743 |
| null_resource.test | RUN | 0.419406 | 1.187865 | 1.147500 | 0.001586 | 0.001682 | 0.001816 |
| object.test | RUN | 0.806934 | 4.158931 | 3.803190 | 0.004309 | 0.004453 | 0.004961 |
| parse.test | RUN | 0.545819 | 2.044401 | 1.638079 | 0.002022 | 0.002216 | 0.002118 |
| parser.test | RUN | 0.532441 | 1.750587 | 1.584257 | 0.001772 | 0.001811 | 0.001830 |
| pilfer.test | RUN | 0.276394 | 0.809586 | 0.772945 | 0.001471 | 0.001644 | 0.001466 |
| pointer.test | RUN | 0.590888 | 2.418556 | 2.143214 | 0.001797 | 0.001918 | 0.001852 |
| result_for.test | RUN | 0.507228 | 1.452858 | 1.452851 | 0.001661 | 0.001640 | 0.001658 |
| serialize.test | RUN | 0.516985 | 1.482269 | 1.452757 | 0.001712 | 0.001771 | 0.001582 |
| serializer.test | RUN | 0.785313 | 4.250607 | 3.689389 | 0.002448 | 0.002566 | 0.002343 |
| snippets.test | RUN | 1.200399 | 6.055526 | 5.237821 | 0.001694 | 0.001638 | 0.002014 |
| static_resource.test | RUN | 0.499405 | 1.351308 | 1.407902 | 0.003942 | 0.001769 | 0.001545 |
| storage_ptr.test | RUN | 0.684993 | 1.432538 | 1.425232 | 0.001603 | 0.001759 | 0.001649 |
| stream_parser.test | RUN | 0.606614 | 2.495567 | 2.257781 | 0.059436 | 0.062764 | 0.062237 |
| string.test | RUN | 0.956741 | 6.717303 | 5.630630 | 0.002413 | 0.002376 | 0.002624 |
| string_view.test | RUN | 0.310803 | 0.887722 | 0.898276 | 0.001636 | 0.001700 | 0.001703 |
| value.test | RUN | 0.972495 | 6.413653 | 4.883877 | 0.002846 | 0.002857 | 0.004313 |
| value_from.test | RUN | 0.897747 | 4.346268 | 4.144004 | 0.001795 | 0.001764 | 0.001626 |
| value_stack.test | RUN | 0.502712 | 1.338295 | 1.558406 | 0.001547 | 0.001644 | 0.001725 |
| value_to.test | RUN | 2.055194 | 13.199400 | 10.729906 | 0.002706 | 0.002739 | 0.002882 |
| value_ref.test | RUN | 0.593003 | 1.790320 | 1.902620 | 0.001763 | 0.001602 | 0.001818 |
| utf8.test | RUN | 0.429098 | 1.243503 | 1.325356 | 0.001592 | 0.001730 | 0.001634 |
| visit.test | RUN | 0.559073 | 1.839194 | 1.749311 | 0.001658 | 0.001818 | 0.001720 |
| d2s_intrinsics_test.test | RUN | 0.423367 | 1.160952 | 1.200180 | 0.001616 | 0.001807 | 0.001714 |
| d2s_table_test.test | RUN | 0.420375 | 1.275954 | 1.275257 | 0.001554 | 0.001622 | 0.001768 |
| d2s_test.test | RUN | 0.438713 | 1.564663 | 1.403993 | 0.001674 | 0.001681 | 0.001823 |
| parse_into.test | RUN | 12.933642 | 111.170950 | 70.029349 | 0.002901 | 0.002813 | 0.002806 |
| limits.test | RUN | 1.620640 | 9.198969 | 7.377611 | 0.001956 | 0.001828 | 0.001914 |
| header_only.test | RUN | 0.987021 | 7.322669 | 5.289108 | 0.001439 | 0.001588 | 0.001592 |
| no_exceptions.test | RUN | 1.508825 | 7.874285 | 6.893143 | 0.001765 | 0.001684 | 0.001772 |
| intrusive_macros.test | RUN | 1.256917 | 8.695920 | 5.719281 | 0.001505 | 0.001776 | 0.001739 |
| _gdb-printers.test | RUN | 0.229507 | 0.574055 | 0.694628 | 0.144670 | 0.171329 | 0.171365 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.781×† | 0.753×, 0.808× | 0.988× |
| clang | 0.956× | 0.951×, 0.961× | 0.996× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| array.test | 10 | 0.001617s | 0.001342s | 0.001521s | byte equality observed |
| basic_parser.test | 1 | 0.293955s | 0.158394s | 0.345940s | native test oracle; output varies |
| conversion.test | 10 | 0.000958s | 0.000907s | 0.000976s | byte equality observed |
| d2s_intrinsics_test.test | 10 | 0.000963s | 0.000919s | 0.000955s | byte equality observed |
| d2s_table_test.test | 10 | 0.000991s | 0.000943s | 0.000977s | byte equality observed |
| d2s_test.test | 10 | 0.001034s | 0.000997s | 0.001039s | byte equality observed |
| doc_background.test | 10 | 0.000959s | 0.000932s | 0.000974s | byte equality observed |
| doc_forward_conversion.test | 10 | 0.001006s | 0.000976s | 0.001076s | byte equality observed |
| doc_parsing.test | 10 | 0.000997s | 0.000980s | 0.001072s | byte equality observed |
| doc_quick_look.test | 10 | 0.000976s | 0.000934s | 0.001018s | byte equality observed |
| doc_serializing.test | 10 | 0.000955s | 0.000923s | 0.001017s | byte equality observed |
| doc_storage_ptr.test | 10 | 0.000994s | 0.000964s | 0.001018s | byte equality observed |
| doc_uses_allocator.test | 10 | 0.000988s | 0.000956s | 0.000991s | byte equality observed |
| doc_using_numbers.test | 10 | 0.000967s | 0.000910s | 0.000994s | byte equality observed |
| double.test | 1 | 0.799141s | 0.656038s | 0.717105s | native test oracle; output varies |
| error.test | 10 | 0.001094s | 0.000931s | 0.001150s | byte equality observed |
| fwd.test | 10 | 0.000984s | 0.000911s | 0.001017s | byte equality observed |
| header_only.test | 10 | 0.000920s | 0.000898s | 0.001020s | byte equality observed |
| intrusive_macros.test | 10 | 0.000998s | 0.000959s | 0.001017s | byte equality observed |
| json.test | 10 | 0.000968s | 0.000949s | 0.001038s | byte equality observed |
| kind.test | 10 | 0.000960s | 0.000966s | 0.001044s | byte equality observed |
| limits.test | 10 | 0.001122s | 0.001077s | 0.001230s | byte equality observed |
| memory_resource.test | 10 | 0.000980s | 0.000954s | 0.001046s | byte equality observed |
| monotonic_resource.test | 10 | 0.001356s | 0.001243s | 0.001486s | native test oracle; output varies |
| natvis.test | 10 | 0.001002s | 0.000952s | 0.001073s | byte equality observed |
| no_exceptions.test | 10 | 0.001002s | 0.000941s | 0.001025s | byte equality observed |
| null_resource.test | 10 | 0.001035s | 0.000966s | 0.001060s | byte equality observed |
| object.test | 7 | 0.003123s | 0.002591s | 0.003185s | native test oracle; output varies |
| parse.test | 10 | 0.001386s | 0.001293s | 0.001438s | byte equality observed |
| parse_into.test | 5 | 0.002526s | 0.002388s | 0.002577s | native test oracle; output varies |
| parser.test | 9 | 0.001150s | 0.001046s | 0.001346s | byte equality observed |
| pilfer.test | 10 | 0.000972s | 0.000924s | 0.000998s | byte equality observed |
| pointer.test | 10 | 0.001174s | 0.001096s | 0.001232s | byte equality observed |
| result_for.test | 10 | 0.000977s | 0.000931s | 0.001048s | byte equality observed |
| serialize.test | 10 | 0.001003s | 0.001011s | 0.001084s | byte equality observed |
| serializer.test | 10 | 0.001575s | 0.001522s | 0.001632s | byte equality observed |
| snippets.test | 10 | 0.001147s | 0.001133s | 0.001313s | byte equality observed |
| static_resource.test | 10 | 0.001062s | 0.001028s | 0.001128s | byte equality observed |
| storage_ptr.test | 10 | 0.001032s | 0.000988s | 0.001097s | byte equality observed |
| stream_parser.test | 1 | 0.042238s | 0.046347s | 0.040232s | native test oracle; output varies |
| string.test | 10 | 0.001613s | 0.001387s | 0.001715s | native test oracle; output varies |
| string_view.test | 10 | 0.000987s | 0.000959s | 0.001154s | byte equality observed |
| utf8.test | 10 | 0.000979s | 0.000945s | 0.001069s | byte equality observed |
| value.test | 10 | 0.001855s | 0.001751s | 0.002305s | native test oracle; output varies |
| value_from.test | 10 | 0.001117s | 0.001106s | 0.001179s | byte equality observed |
| value_ref.test | 10 | 0.001006s | 0.001013s | 0.001138s | byte equality observed |
| value_stack.test | 10 | 0.001013s | 0.000975s | 0.001123s | byte equality observed |
| value_to.test | 10 | 0.001642s | 0.001613s | 0.001781s | native test oracle; output varies |
| visit.test | 10 | 0.001000s | 0.000994s | 0.001070s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
