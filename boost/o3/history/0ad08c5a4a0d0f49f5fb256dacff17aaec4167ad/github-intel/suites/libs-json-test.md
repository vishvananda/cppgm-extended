# libs/json/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| array.test | RUN | 1.165372 | 6.101817 | 5.157239 | 0.003132 | 0.002768 | 0.003231 |
| basic_parser.test | RUN | 5.920015 | 151.643630 | 90.311993 | 0.743485 | 0.275501 | 0.868055 |
| conversion.test | RUN | 0.826333 | 2.235804 | 2.508151 | 0.001797 | 0.001744 | 0.001713 |
| doc_background.test | RUN | 0.845000 | 2.238639 | 2.474710 | 0.001962 | 0.001735 | 0.001744 |
| doc_forward_conversion.test | RUN | 0.854757 | 2.615949 | 2.840435 | 0.001904 | 0.001814 | 0.001837 |
| doc_parsing.test | RUN | 0.861871 | 2.446666 | 2.641574 | 0.002182 | 0.002083 | 0.001822 |
| doc_quick_look.test | RUN | 1.084020 | 2.923726 | 3.465582 | 0.001754 | 0.001868 | 0.001720 |
| doc_serializing.test | RUN | 0.777390 | 2.143923 | 2.348128 | 0.001833 | 0.001709 | 0.001649 |
| doc_storage_ptr.test | RUN | 0.792208 | 2.227338 | 2.415608 | 0.001835 | 0.001872 | 0.001823 |
| doc_uses_allocator.test | RUN | 0.802495 | 2.173215 | 2.382943 | 0.001958 | 0.002301 | 0.001831 |
| doc_using_numbers.test | RUN | 0.880597 | 2.190692 | 2.650478 | 0.002012 | 0.001855 | 0.001806 |
| double.test | RUN | 0.954869 | 3.830765 | 3.038222 | 1.968393 | 1.653763 | 1.856113 |
| error.test | RUN | 0.686378 | 2.168163 | 2.095018 | 0.001782 | 0.001844 | 0.001833 |
| fwd.test | RUN | 0.786713 | 2.218626 | 2.592948 | 0.001738 | 0.001834 | 0.001792 |
| json.test | RUN | 0.809830 | 2.574747 | 2.756914 | 0.001729 | 0.001840 | 0.001906 |
| kind.test | RUN | 0.644308 | 1.955717 | 1.907815 | 0.001917 | 0.001912 | 0.001866 |
| memory_resource.test | RUN | 0.874896 | 2.591841 | 2.693325 | 0.001904 | 0.001794 | 0.002018 |
| monotonic_resource.test | RUN | 0.801182 | 2.454643 | 2.415515 | 0.002733 | 0.002720 | 0.002614 |
| natvis.test | RUN | 0.813015 | 2.297435 | 2.430601 | 0.001977 | 0.001825 | 0.001858 |
| null_resource.test | RUN | 0.673623 | 2.018073 | 1.916271 | 0.001936 | 0.002018 | 0.001802 |
| object.test | RUN | 1.341874 | 7.127051 | 6.324221 | 0.008332 | 0.006590 | 0.007867 |
| parse.test | RUN | 0.883506 | 3.582068 | 2.732352 | 0.002376 | 0.002863 | 0.002438 |
| parser.test | RUN | 0.850483 | 3.012387 | 2.654447 | 0.002120 | 0.002184 | 0.002364 |
| pilfer.test | RUN | 0.459548 | 1.369438 | 1.202671 | 0.001902 | 0.001910 | 0.001853 |
| pointer.test | RUN | 0.982549 | 4.129589 | 3.444865 | 0.002068 | 0.002146 | 0.002212 |
| result_for.test | RUN | 0.812692 | 2.447295 | 2.548841 | 0.001734 | 0.001883 | 0.001924 |
| serialize.test | RUN | 0.819853 | 2.528474 | 2.551215 | 0.001917 | 0.002022 | 0.001883 |
| serializer.test | RUN | 1.348143 | 7.246998 | 5.713595 | 0.003265 | 0.003064 | 0.002990 |
| snippets.test | RUN | 1.973028 | 10.241299 | 8.929207 | 0.002196 | 0.002258 | 0.002078 |
| static_resource.test | RUN | 0.790553 | 2.302630 | 2.400469 | 0.001914 | 0.001940 | 0.002034 |
| storage_ptr.test | RUN | 0.834957 | 2.436226 | 2.552434 | 0.002042 | 0.001992 | 0.001824 |
| stream_parser.test | RUN | 1.011955 | 4.734656 | 3.697778 | 0.116012 | 0.108736 | 0.099042 |
| string.test | RUN | 1.720274 | 11.439036 | 9.561942 | 0.003438 | 0.002584 | 0.003078 |
| string_view.test | RUN | 0.498046 | 1.491080 | 1.353306 | 0.001924 | 0.001973 | 0.001724 |
| value.test | RUN | 1.652405 | 11.687543 | 7.810653 | 0.003797 | 0.003269 | 0.004670 |
| value_from.test | RUN | 1.504811 | 7.224127 | 6.343329 | 0.002157 | 0.002115 | 0.002218 |
| value_stack.test | RUN | 0.789475 | 2.229523 | 2.322197 | 0.002064 | 0.001918 | 0.001890 |
| value_to.test | RUN | 3.415168 | 22.288720 | 17.363941 | 0.003594 | 0.003308 | 0.003382 |
| value_ref.test | RUN | 0.902845 | 3.227707 | 3.027598 | 0.002083 | 0.002068 | 0.001917 |
| utf8.test | RUN | 0.676789 | 2.118953 | 2.123312 | 0.001890 | 0.001841 | 0.001815 |
| visit.test | RUN | 0.899169 | 3.298017 | 2.808789 | 0.001863 | 0.001924 | 0.001873 |
| d2s_intrinsics_test.test | RUN | 0.668369 | 1.945099 | 1.931869 | 0.001851 | 0.001836 | 0.001758 |
| d2s_table_test.test | RUN | 0.688746 | 2.150969 | 2.013277 | 0.001864 | 0.001822 | 0.001930 |
| d2s_test.test | RUN | 0.708350 | 2.617008 | 2.324341 | 0.002029 | 0.002008 | 0.001939 |
| parse_into.test | RUN | 21.122450 | 179.112021 | 112.042719 | 0.004004 | 0.003055 | 0.003018 |
| limits.test | RUN | 2.625296 | 16.522915 | 11.712819 | 0.002014 | 0.002406 | 0.002307 |
| header_only.test | RUN | 1.730111 | 12.491717 | 8.526405 | 0.001684 | 0.001978 | 0.001751 |
| no_exceptions.test | RUN | 2.440003 | 13.894918 | 10.758858 | 0.002144 | 0.001937 | 0.001872 |
| intrusive_macros.test | RUN | 2.083028 | 13.562168 | 9.337890 | 0.001926 | 0.001842 | 0.001771 |
| _gdb-printers.test | RUN | 0.377436 | 0.901788 | 1.150318 | 0.223345 | 0.245423 | 0.234507 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.815× | 0.810×, 0.820× | 1.025× |
| clang | 1.029× | 1.027×, 1.031× | 0.989× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| array.test | 9 | 0.001814s | 0.001651s | 0.001783s | native test oracle; output varies |
| basic_parser.test | 1 | 0.510655s | 0.284613s | 0.634136s | native test oracle; output varies |
| conversion.test | 10 | 0.001001s | 0.000997s | 0.000997s | byte equality observed |
| d2s_intrinsics_test.test | 10 | 0.000988s | 0.000998s | 0.000998s | byte equality observed |
| d2s_table_test.test | 10 | 0.001009s | 0.001032s | 0.001036s | byte equality observed |
| d2s_test.test | 10 | 0.001047s | 0.001069s | 0.001040s | byte equality observed |
| doc_background.test | 10 | 0.001004s | 0.000998s | 0.000989s | byte equality observed |
| doc_forward_conversion.test | 10 | 0.001034s | 0.001043s | 0.001050s | byte equality observed |
| doc_parsing.test | 10 | 0.001045s | 0.001036s | 0.001026s | byte equality observed |
| doc_quick_look.test | 10 | 0.001002s | 0.001029s | 0.000995s | byte equality observed |
| doc_serializing.test | 10 | 0.001007s | 0.001006s | 0.000975s | byte equality observed |
| doc_storage_ptr.test | 10 | 0.001008s | 0.001032s | 0.001019s | byte equality observed |
| doc_uses_allocator.test | 10 | 0.001011s | 0.001007s | 0.001004s | byte equality observed |
| doc_using_numbers.test | 10 | 0.001017s | 0.000997s | 0.001004s | byte equality observed |
| double.test | 1 | 1.207690s | 1.083765s | 1.149487s | native test oracle; output varies |
| error.test | 10 | 0.001149s | 0.001133s | 0.001111s | byte equality observed |
| fwd.test | 10 | 0.001027s | 0.001009s | 0.001007s | byte equality observed |
| header_only.test | 10 | 0.001011s | 0.001017s | 0.000971s | byte equality observed |
| intrusive_macros.test | 10 | 0.001062s | 0.001079s | 0.001030s | byte equality observed |
| json.test | 10 | 0.001040s | 0.001038s | 0.001035s | byte equality observed |
| kind.test | 10 | 0.001069s | 0.001068s | 0.001041s | byte equality observed |
| limits.test | 10 | 0.001301s | 0.001226s | 0.001216s | byte equality observed |
| memory_resource.test | 10 | 0.001066s | 0.001041s | 0.001038s | byte equality observed |
| monotonic_resource.test | 10 | 0.001563s | 0.001523s | 0.001491s | byte equality observed |
| natvis.test | 10 | 0.001082s | 0.001068s | 0.001057s | byte equality observed |
| no_exceptions.test | 10 | 0.001063s | 0.001069s | 0.001030s | byte equality observed |
| null_resource.test | 10 | 0.001084s | 0.001092s | 0.001054s | byte equality observed |
| object.test | 5 | 0.004556s | 0.003755s | 0.004361s | native test oracle; output varies |
| parse.test | 10 | 0.001508s | 0.001494s | 0.001438s | byte equality observed |
| parse_into.test | 6 | 0.002680s | 0.002369s | 0.002289s | native test oracle; output varies |
| parser.test | 10 | 0.001194s | 0.001182s | 0.001148s | byte equality observed |
| pilfer.test | 10 | 0.001019s | 0.001023s | 0.000986s | byte equality observed |
| pointer.test | 10 | 0.001268s | 0.001232s | 0.001201s | byte equality observed |
| result_for.test | 10 | 0.001025s | 0.001033s | 0.000985s | byte equality observed |
| serialize.test | 10 | 0.001086s | 0.001079s | 0.001047s | byte equality observed |
| serializer.test | 10 | 0.001881s | 0.001846s | 0.001784s | native test oracle; output varies |
| snippets.test | 10 | 0.001257s | 0.001229s | 0.001194s | byte equality observed |
| static_resource.test | 10 | 0.001118s | 0.001104s | 0.001062s | byte equality observed |
| storage_ptr.test | 10 | 0.001082s | 0.001107s | 0.001033s | byte equality observed |
| stream_parser.test | 1 | 0.061624s | 0.079181s | 0.057023s | native test oracle; output varies |
| string.test | 8 | 0.001907s | 0.001615s | 0.001855s | native test oracle; output varies |
| string_view.test | 10 | 0.001028s | 0.001026s | 0.001005s | byte equality observed |
| utf8.test | 10 | 0.001030s | 0.001025s | 0.001018s | byte equality observed |
| value.test | 7 | 0.002392s | 0.002179s | 0.002746s | native test oracle; output varies |
| value_from.test | 10 | 0.001240s | 0.001208s | 0.001225s | byte equality observed |
| value_ref.test | 10 | 0.001105s | 0.001122s | 0.001065s | byte equality observed |
| value_stack.test | 10 | 0.001086s | 0.001084s | 0.001042s | byte equality observed |
| value_to.test | 9 | 0.002011s | 0.001950s | 0.001957s | native test oracle; output varies |
| visit.test | 10 | 0.001069s | 0.001066s | 0.001074s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
