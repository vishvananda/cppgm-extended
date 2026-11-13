# libs/json/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| array.test | RUN | 0.644803 | 3.995850 | 2.982710 | 0.002271 | 0.002482 | 0.002136 |
| basic_parser.test | RUN | 3.881634 | 91.813036 | 50.370742 | 0.365946 | 0.135378 | 0.479772 |
| conversion.test | RUN | 0.470118 | 1.494988 | 1.483447 | 0.001213 | 0.001606 | 0.001418 |
| doc_background.test | RUN | 0.489298 | 1.509179 | 1.471772 | 0.001578 | 0.001647 | 0.001244 |
| doc_forward_conversion.test | RUN | 0.491483 | 1.740729 | 1.666066 | 0.001415 | 0.001668 | 0.001510 |
| doc_parsing.test | RUN | 0.505353 | 1.624808 | 1.492957 | 0.001638 | 0.001698 | 0.001317 |
| doc_quick_look.test | RUN | 0.649793 | 1.971477 | 2.026372 | 0.001434 | 0.001575 | 0.001597 |
| doc_serializing.test | RUN | 0.449998 | 1.439230 | 1.396267 | 0.001722 | 0.001618 | 0.001463 |
| doc_storage_ptr.test | RUN | 0.463072 | 1.476895 | 1.399220 | 0.001354 | 0.001603 | 0.001570 |
| doc_uses_allocator.test | RUN | 0.456060 | 1.423251 | 1.413921 | 0.001593 | 0.001750 | 0.001554 |
| doc_using_numbers.test | RUN | 0.492620 | 1.510909 | 1.552829 | 0.001413 | 0.001744 | 0.001619 |
| double.test | RUN | 0.527351 | 2.524930 | 1.717494 | 1.017407 | 1.072600 | 1.020773 |
| error.test | RUN | 0.376400 | 1.401427 | 1.162406 | 0.001480 | 0.001853 | 0.001536 |
| fwd.test | RUN | 0.433599 | 1.410690 | 1.383509 | 0.001544 | 0.001646 | 0.001507 |
| json.test | RUN | 0.472973 | 1.682340 | 1.590225 | 0.001514 | 0.001622 | 0.001451 |
| kind.test | RUN | 0.355567 | 1.315995 | 1.170003 | 0.001581 | 0.001915 | 0.001590 |
| memory_resource.test | RUN | 0.506546 | 1.768712 | 1.493819 | 0.001564 | 0.001922 | 0.001508 |
| monotonic_resource.test | RUN | 0.456907 | 1.626109 | 1.411033 | 0.002086 | 0.002307 | 0.001968 |
| natvis.test | RUN | 0.456118 | 1.573280 | 1.497784 | 0.001646 | 0.001832 | 0.001709 |
| null_resource.test | RUN | 0.379012 | 1.365857 | 1.148656 | 0.001516 | 0.001864 | 0.001663 |
| object.test | RUN | 0.760013 | 4.672076 | 3.401965 | 0.005051 | 0.004663 | 0.004677 |
| parse.test | RUN | 0.496159 | 2.357524 | 1.745394 | 0.002040 | 0.002317 | 0.002136 |
| parser.test | RUN | 0.478761 | 1.993166 | 1.617678 | 0.001735 | 0.001982 | 0.001646 |
| pilfer.test | RUN | 0.259522 | 0.930510 | 0.800366 | 0.001433 | 0.001520 | 0.001205 |
| pointer.test | RUN | 0.545373 | 2.699323 | 2.014862 | 0.001910 | 0.002088 | 0.001880 |
| result_for.test | RUN | 0.461079 | 1.622601 | 1.435233 | 0.001448 | 0.001784 | 0.001566 |
| serialize.test | RUN | 0.473473 | 1.721782 | 1.421911 | 0.001732 | 0.001937 | 0.001624 |
| serializer.test | RUN | 0.725538 | 4.768623 | 3.521905 | 0.002472 | 0.002607 | 0.002441 |
| snippets.test | RUN | 1.148378 | 6.798648 | 5.155288 | 0.001826 | 0.001903 | 0.001948 |
| static_resource.test | RUN | 0.462580 | 1.504897 | 1.384041 | 0.001716 | 0.001776 | 0.001636 |
| storage_ptr.test | RUN | 0.482357 | 1.593891 | 1.449685 | 0.001611 | 0.001838 | 0.001518 |
| stream_parser.test | RUN | 0.564983 | 2.845529 | 2.068724 | 0.063257 | 0.070750 | 0.061212 |
| string.test | RUN | 0.931638 | 6.715241 | 5.597219 | 0.003260 | 0.002266 | 0.002431 |
| string_view.test | RUN | 0.285265 | 0.834855 | 0.805605 | 0.001538 | 0.001578 | 0.001451 |
| value.test | RUN | 0.916501 | 6.788554 | 4.484254 | 0.002835 | 0.002722 | 0.003007 |
| value_from.test | RUN | 0.877791 | 3.967421 | 3.740453 | 0.001938 | 0.001938 | 0.001766 |
| value_stack.test | RUN | 0.455799 | 1.276351 | 1.361410 | 0.001538 | 0.001681 | 0.001590 |
| value_to.test | RUN | 2.115255 | 14.117075 | 10.174838 | 0.002655 | 0.002830 | 0.002662 |
| value_ref.test | RUN | 0.500386 | 1.790116 | 1.738280 | 0.001686 | 0.001749 | 0.001622 |
| utf8.test | RUN | 0.381908 | 1.199034 | 1.194671 | 0.001603 | 0.001533 | 0.001571 |
| visit.test | RUN | 0.492193 | 1.978663 | 1.656191 | 0.001448 | 0.001804 | 0.001572 |
| d2s_intrinsics_test.test | RUN | 0.378029 | 1.170200 | 1.190997 | 0.001656 | 0.001582 | 0.001542 |
| d2s_table_test.test | RUN | 0.389909 | 1.232315 | 1.207278 | 0.001532 | 0.001570 | 0.001518 |
| d2s_test.test | RUN | 0.398370 | 1.503908 | 1.336672 | 0.001747 | 0.001622 | 0.001694 |
| parse_into.test | RUN | 12.980002 | 111.383539 | 65.061963 | 0.002911 | 0.002692 | 0.002535 |
| limits.test | RUN | 1.532867 | 10.077959 | 6.707922 | 0.001934 | 0.001874 | 0.001780 |
| header_only.test | RUN | 1.001308 | 7.263258 | 4.915880 | 0.001372 | 0.001551 | 0.001450 |
| no_exceptions.test | RUN | 1.446102 | 8.001010 | 6.281159 | 0.001676 | 0.001743 | 0.001508 |
| intrusive_macros.test | RUN | 1.228017 | 8.097762 | 5.770316 | 0.001419 | 0.001746 | 0.001494 |
| _gdb-printers.test | RUN | 0.219988 | 0.552184 | 0.650000 | 0.145093 | 0.221347 | 0.156006 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.841× | 0.842×, 0.840× | 0.994× |
| clang | 1.060× | 1.054×, 1.067× | 1.021× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409407).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| array.test | 10 | 0.001385s | 0.001231s | 0.001319s | byte equality observed |
| basic_parser.test | 1 | 0.229491s | 0.148410s | 0.326055s | native test oracle; output varies |
| conversion.test | 10 | 0.000834s | 0.000829s | 0.000825s | byte equality observed |
| d2s_intrinsics_test.test | 10 | 0.000836s | 0.000840s | 0.000844s | byte equality observed |
| d2s_table_test.test | 10 | 0.000857s | 0.000862s | 0.000844s | byte equality observed |
| d2s_test.test | 10 | 0.000900s | 0.000901s | 0.000883s | byte equality observed |
| doc_background.test | 10 | 0.000860s | 0.000829s | 0.000830s | byte equality observed |
| doc_forward_conversion.test | 10 | 0.000883s | 0.000867s | 0.000884s | byte equality observed |
| doc_parsing.test | 10 | 0.000895s | 0.000880s | 0.000879s | byte equality observed |
| doc_quick_look.test | 10 | 0.000867s | 0.000868s | 0.000854s | byte equality observed |
| doc_serializing.test | 10 | 0.000861s | 0.000840s | 0.000838s | byte equality observed |
| doc_storage_ptr.test | 10 | 0.000864s | 0.000880s | 0.000871s | byte equality observed |
| doc_uses_allocator.test | 10 | 0.000865s | 0.000858s | 0.000844s | byte equality observed |
| doc_using_numbers.test | 10 | 0.000862s | 0.000874s | 0.000839s | byte equality observed |
| double.test | 1 | 0.694866s | 0.607924s | 0.666903s | native test oracle; output varies |
| error.test | 10 | 0.000887s | 0.000879s | 0.000852s | byte equality observed |
| fwd.test | 10 | 0.000853s | 0.000847s | 0.000811s | byte equality observed |
| header_only.test | 10 | 0.000833s | 0.000825s | 0.000812s | byte equality observed |
| intrusive_macros.test | 10 | 0.000878s | 0.000883s | 0.000855s | byte equality observed |
| json.test | 10 | 0.000854s | 0.000856s | 0.000842s | byte equality observed |
| kind.test | 10 | 0.000885s | 0.000888s | 0.000860s | byte equality observed |
| limits.test | 10 | 0.001025s | 0.001009s | 0.001000s | byte equality observed |
| memory_resource.test | 10 | 0.000868s | 0.000863s | 0.000861s | byte equality observed |
| monotonic_resource.test | 10 | 0.001195s | 0.001129s | 0.001132s | byte equality observed |
| natvis.test | 10 | 0.000877s | 0.000891s | 0.000875s | byte equality observed |
| no_exceptions.test | 10 | 0.000868s | 0.000890s | 0.000870s | byte equality observed |
| null_resource.test | 10 | 0.000900s | 0.000898s | 0.000870s | byte equality observed |
| object.test | 7 | 0.002853s | 0.002389s | 0.002739s | native test oracle; output varies |
| parse.test | 10 | 0.001212s | 0.001204s | 0.001180s | byte equality observed |
| parse_into.test | 8 | 0.002153s | 0.002038s | 0.001938s | native test oracle; output varies |
| parser.test | 10 | 0.000978s | 0.000964s | 0.000987s | byte equality observed |
| pilfer.test | 10 | 0.000841s | 0.000848s | 0.000851s | byte equality observed |
| pointer.test | 10 | 0.001030s | 0.001012s | 0.001019s | byte equality observed |
| result_for.test | 10 | 0.000853s | 0.000858s | 0.000841s | byte equality observed |
| serialize.test | 10 | 0.000909s | 0.000900s | 0.000894s | byte equality observed |
| serializer.test | 10 | 0.001413s | 0.001424s | 0.001414s | byte equality observed |
| snippets.test | 10 | 0.001052s | 0.001017s | 0.001028s | byte equality observed |
| static_resource.test | 10 | 0.000928s | 0.000917s | 0.000932s | byte equality observed |
| storage_ptr.test | 10 | 0.000906s | 0.000901s | 0.000893s | byte equality observed |
| stream_parser.test | 1 | 0.037483s | 0.043522s | 0.036590s | native test oracle; output varies |
| string.test | 10 | 0.001368s | 0.001223s | 0.001337s | byte equality observed |
| string_view.test | 10 | 0.000838s | 0.000844s | 0.000840s | byte equality observed |
| utf8.test | 10 | 0.000840s | 0.000852s | 0.000862s | byte equality observed |
| value.test | 5 | 0.001680s | 0.001536s | 0.001816s | native test oracle; output varies |
| value_from.test | 10 | 0.001052s | 0.001000s | 0.000998s | byte equality observed |
| value_ref.test | 10 | 0.000927s | 0.000919s | 0.000901s | byte equality observed |
| value_stack.test | 10 | 0.000895s | 0.000911s | 0.000873s | byte equality observed |
| value_to.test | 10 | 0.001526s | 0.001454s | 0.001429s | byte equality observed |
| visit.test | 10 | 0.000882s | 0.000876s | 0.000878s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409407).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
