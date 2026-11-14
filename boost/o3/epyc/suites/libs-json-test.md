# libs/json/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| array.test | RUN | 0.738498 | 3.697086 | 3.022112 | 0.002676 | 0.002227 | 0.002272 |
| basic_parser.test | RUN | 4.533647 | 94.083128 | 50.490245 | 0.392583 | 0.134651 | 0.484469 |
| conversion.test | RUN | 0.581914 | 1.360703 | 1.494316 | 0.001607 | 0.001626 | 0.001610 |
| doc_background.test | RUN | 0.576188 | 1.372564 | 1.458091 | 0.001986 | 0.001667 | 0.001349 |
| doc_forward_conversion.test | RUN | 0.602414 | 1.615188 | 1.689010 | 0.001938 | 0.001574 | 0.001616 |
| doc_parsing.test | RUN | 0.596367 | 1.547973 | 1.564263 | 0.002065 | 0.001470 | 0.001491 |
| doc_quick_look.test | RUN | 0.779704 | 1.795301 | 2.034831 | 0.001779 | 0.001467 | 0.001649 |
| doc_serializing.test | RUN | 0.548520 | 1.313181 | 1.433535 | 0.001645 | 0.001494 | 0.001398 |
| doc_storage_ptr.test | RUN | 0.538302 | 1.387649 | 1.428398 | 0.001543 | 0.001408 | 0.001527 |
| doc_uses_allocator.test | RUN | 0.534052 | 1.274624 | 1.418728 | 0.001914 | 0.001635 | 0.001514 |
| doc_using_numbers.test | RUN | 0.580602 | 1.370685 | 1.572540 | 0.001642 | 0.001625 | 0.001501 |
| double.test | RUN | 0.611203 | 2.284540 | 1.734642 | 1.183292 | 0.945290 | 1.021342 |
| error.test | RUN | 0.450695 | 1.296279 | 1.231740 | 0.001840 | 0.001663 | 0.001543 |
| fwd.test | RUN | 0.537441 | 1.270456 | 1.439775 | 0.001684 | 0.001629 | 0.001577 |
| json.test | RUN | 0.539367 | 1.528765 | 1.589215 | 0.001754 | 0.001552 | 0.001557 |
| kind.test | RUN | 0.403891 | 1.197394 | 1.189266 | 0.001911 | 0.001661 | 0.001610 |
| memory_resource.test | RUN | 0.579612 | 1.613882 | 1.516715 | 0.001591 | 0.001804 | 0.001560 |
| monotonic_resource.test | RUN | 0.546919 | 1.465395 | 1.502409 | 0.002152 | 0.002210 | 0.002132 |
| natvis.test | RUN | 0.534651 | 1.429509 | 1.412593 | 0.001913 | 0.001604 | 0.001585 |
| null_resource.test | RUN | 0.445349 | 1.219124 | 1.159401 | 0.001932 | 0.001755 | 0.001594 |
| object.test | RUN | 0.877984 | 4.261568 | 3.711621 | 0.005520 | 0.004229 | 0.004812 |
| parse.test | RUN | 0.588172 | 2.147417 | 1.637094 | 0.002460 | 0.002396 | 0.002170 |
| parser.test | RUN | 0.564430 | 1.834592 | 1.584297 | 0.002084 | 0.001809 | 0.001762 |
| pilfer.test | RUN | 0.295937 | 0.836881 | 0.769274 | 0.001778 | 0.001730 | 0.001401 |
| pointer.test | RUN | 0.638696 | 2.474443 | 2.078900 | 0.002068 | 0.001981 | 0.001620 |
| result_for.test | RUN | 0.529182 | 1.503527 | 1.470002 | 0.001693 | 0.001676 | 0.001434 |
| serialize.test | RUN | 0.540468 | 1.544515 | 1.615900 | 0.001726 | 0.001784 | 0.001666 |
| serializer.test | RUN | 0.838900 | 4.421543 | 3.460616 | 0.002719 | 0.002418 | 0.002384 |
| snippets.test | RUN | 1.332801 | 6.288392 | 5.295097 | 0.002300 | 0.001990 | 0.001941 |
| static_resource.test | RUN | 0.538762 | 1.414587 | 1.433351 | 0.002040 | 0.001757 | 0.001738 |
| storage_ptr.test | RUN | 0.575058 | 1.525838 | 1.445667 | 0.002100 | 0.001678 | 0.001629 |
| stream_parser.test | RUN | 0.693655 | 2.690116 | 2.121314 | 0.067643 | 0.075297 | 0.062418 |
| string.test | RUN | 1.079401 | 6.364074 | 5.673731 | 0.002862 | 0.002332 | 0.002292 |
| string_view.test | RUN | 0.332530 | 0.904951 | 0.837605 | 0.001907 | 0.001672 | 0.001536 |
| value.test | RUN | 1.086476 | 7.169457 | 4.459002 | 0.002940 | 0.002922 | 0.003075 |
| value_from.test | RUN | 1.006678 | 4.273951 | 3.798641 | 0.002086 | 0.001914 | 0.001843 |
| value_stack.test | RUN | 0.523312 | 1.266652 | 1.379507 | 0.001850 | 0.001683 | 0.001626 |
| value_to.test | RUN | 2.444924 | 13.578154 | 10.497054 | 0.003072 | 0.002544 | 0.002709 |
| value_ref.test | RUN | 0.575327 | 1.869293 | 1.722203 | 0.001918 | 0.001904 | 0.001542 |
| utf8.test | RUN | 0.448764 | 1.282233 | 1.165287 | 0.001834 | 0.001748 | 0.001552 |
| visit.test | RUN | 0.577805 | 1.878873 | 1.698063 | 0.001940 | 0.001629 | 0.001641 |
| d2s_intrinsics_test.test | RUN | 0.437927 | 1.146124 | 1.148136 | 0.001849 | 0.001625 | 0.001628 |
| d2s_table_test.test | RUN | 0.464174 | 1.253638 | 1.165022 | 0.001886 | 0.001720 | 0.001610 |
| d2s_test.test | RUN | 0.475698 | 1.577783 | 1.402111 | 0.001888 | 0.001794 | 0.001814 |
| parse_into.test | RUN | 14.310252 | 115.422815 | 66.185212 | 0.003378 | 0.002803 | 0.002526 |
| limits.test | RUN | 1.705259 | 9.624704 | 6.497811 | 0.002316 | 0.001998 | 0.001857 |
| header_only.test | RUN | 1.116757 | 7.553625 | 5.106845 | 0.001862 | 0.001672 | 0.001540 |
| no_exceptions.test | RUN | 1.646639 | 8.393549 | 6.278974 | 0.001647 | 0.001605 | 0.001617 |
| intrusive_macros.test | RUN | 1.397017 | 8.383853 | 5.814572 | 0.001542 | 0.001742 | 0.001499 |
| _gdb-printers.test | RUN | 0.246665 | 0.625332 | 0.644943 | 0.160363 | 0.227611 | 0.153109 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.831× | 0.823×, 0.839× | 0.997× |
| clang | 1.063× | 1.054×, 1.072× | 0.987× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026855).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| array.test | 10 | 0.001382s | 0.001250s | 0.001326s | byte equality observed |
| basic_parser.test | 1 | 0.225460s | 0.147441s | 0.328458s | native test oracle; output varies |
| conversion.test | 10 | 0.000834s | 0.000862s | 0.000849s | byte equality observed |
| d2s_intrinsics_test.test | 10 | 0.000833s | 0.000873s | 0.000851s | byte equality observed |
| d2s_table_test.test | 10 | 0.000861s | 0.000877s | 0.000879s | byte equality observed |
| d2s_test.test | 10 | 0.000909s | 0.000956s | 0.000909s | byte equality observed |
| doc_background.test | 10 | 0.000856s | 0.000862s | 0.000862s | byte equality observed |
| doc_forward_conversion.test | 10 | 0.000886s | 0.000897s | 0.000889s | byte equality observed |
| doc_parsing.test | 10 | 0.000902s | 0.000925s | 0.000881s | byte equality observed |
| doc_quick_look.test | 10 | 0.000871s | 0.000876s | 0.000868s | byte equality observed |
| doc_serializing.test | 10 | 0.000861s | 0.000860s | 0.000862s | byte equality observed |
| doc_storage_ptr.test | 10 | 0.000861s | 0.000873s | 0.000878s | byte equality observed |
| doc_uses_allocator.test | 10 | 0.000872s | 0.000881s | 0.000852s | byte equality observed |
| doc_using_numbers.test | 10 | 0.000870s | 0.000873s | 0.000847s | byte equality observed |
| double.test | 1 | 0.709369s | 0.618059s | 0.671453s | native test oracle; output varies |
| error.test | 10 | 0.000883s | 0.000881s | 0.000891s | byte equality observed |
| fwd.test | 10 | 0.000838s | 0.000837s | 0.000842s | byte equality observed |
| header_only.test | 10 | 0.000837s | 0.000810s | 0.000822s | byte equality observed |
| intrusive_macros.test | 10 | 0.000899s | 0.000873s | 0.000871s | byte equality observed |
| json.test | 10 | 0.000860s | 0.000857s | 0.000866s | byte equality observed |
| kind.test | 10 | 0.000881s | 0.000872s | 0.000875s | byte equality observed |
| limits.test | 10 | 0.001058s | 0.000996s | 0.000996s | byte equality observed |
| memory_resource.test | 10 | 0.000878s | 0.000868s | 0.000866s | byte equality observed |
| monotonic_resource.test | 10 | 0.001210s | 0.001142s | 0.001156s | byte equality observed |
| natvis.test | 10 | 0.000898s | 0.000893s | 0.000877s | byte equality observed |
| no_exceptions.test | 10 | 0.000893s | 0.000884s | 0.000874s | byte equality observed |
| null_resource.test | 10 | 0.000909s | 0.000911s | 0.000889s | byte equality observed |
| object.test | 7 | 0.002856s | 0.002407s | 0.002774s | native test oracle; output varies |
| parse.test | 10 | 0.001253s | 0.001191s | 0.001190s | byte equality observed |
| parse_into.test | 7 | 0.002226s | 0.002075s | 0.001954s | native test oracle; output varies |
| parser.test | 10 | 0.001000s | 0.000962s | 0.000980s | byte equality observed |
| pilfer.test | 10 | 0.000856s | 0.000850s | 0.000859s | byte equality observed |
| pointer.test | 10 | 0.001046s | 0.001011s | 0.001026s | byte equality observed |
| result_for.test | 10 | 0.000871s | 0.000858s | 0.000847s | byte equality observed |
| serialize.test | 10 | 0.000923s | 0.000893s | 0.000900s | byte equality observed |
| serializer.test | 10 | 0.001421s | 0.001385s | 0.001384s | byte equality observed |
| snippets.test | 10 | 0.001053s | 0.001026s | 0.001038s | byte equality observed |
| static_resource.test | 10 | 0.000953s | 0.000913s | 0.000928s | byte equality observed |
| storage_ptr.test | 10 | 0.000920s | 0.000902s | 0.000888s | byte equality observed |
| stream_parser.test | 1 | 0.040570s | 0.043542s | 0.036556s | native test oracle; output varies |
| string.test | 10 | 0.001388s | 0.001227s | 0.001348s | byte equality observed |
| string_view.test | 10 | 0.000855s | 0.000838s | 0.000842s | byte equality observed |
| utf8.test | 10 | 0.000852s | 0.000841s | 0.000861s | byte equality observed |
| value.test | 10 | 0.001681s | 0.001532s | 0.001856s | native test oracle; output varies |
| value_from.test | 10 | 0.001067s | 0.001003s | 0.000998s | byte equality observed |
| value_ref.test | 10 | 0.000915s | 0.000902s | 0.000908s | byte equality observed |
| value_stack.test | 10 | 0.000900s | 0.000879s | 0.000897s | byte equality observed |
| value_to.test | 10 | 0.001542s | 0.001451s | 0.001462s | byte equality observed |
| visit.test | 10 | 0.000885s | 0.000874s | 0.000894s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026855).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
