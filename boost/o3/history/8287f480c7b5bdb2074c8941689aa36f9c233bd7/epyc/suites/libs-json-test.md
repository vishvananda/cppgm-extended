# libs/json/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| array.test | RUN | 0.962445 | 4.727948 | 4.453644 | 0.002943 | 0.002551 | 0.002842 |
| basic_parser.test | RUN | 4.964830 | 113.481622 | 69.519911 | 0.636158 | 0.188813 | 0.701089 |
| conversion.test | RUN | 0.728352 | 1.779776 | 2.327027 | 0.001659 | 0.001912 | 0.001730 |
| doc_background.test | RUN | 0.726706 | 1.814398 | 2.294930 | 0.001812 | 0.001912 | 0.001764 |
| doc_forward_conversion.test | RUN | 0.713119 | 2.110245 | 2.567564 | 0.001699 | 0.001880 | 0.001568 |
| doc_parsing.test | RUN | 0.726048 | 1.986979 | 2.265914 | 0.001865 | 0.001891 | 0.001832 |
| doc_quick_look.test | RUN | 0.881463 | 2.367577 | 3.181401 | 0.001738 | 0.001782 | 0.001844 |
| doc_serializing.test | RUN | 0.657070 | 1.741089 | 2.206584 | 0.001672 | 0.001571 | 0.001738 |
| doc_storage_ptr.test | RUN | 0.675055 | 1.813319 | 2.213998 | 0.001878 | 0.001980 | 0.001758 |
| doc_uses_allocator.test | RUN | 0.708691 | 1.672496 | 2.166306 | 0.001926 | 0.001866 | 0.001714 |
| doc_using_numbers.test | RUN | 0.715327 | 1.775918 | 2.441510 | 0.001817 | 0.001825 | 0.001801 |
| double.test | RUN | 0.778344 | 3.062244 | 2.831999 | 1.535456 | 1.340616 | 1.310535 |
| error.test | RUN | 0.554528 | 1.675955 | 1.899926 | 0.001741 | 0.001814 | 0.001808 |
| fwd.test | RUN | 0.661428 | 1.646305 | 2.381599 | 0.001507 | 0.001708 | 0.001811 |
| json.test | RUN | 0.665316 | 1.964032 | 2.462599 | 0.001590 | 0.001718 | 0.001803 |
| kind.test | RUN | 0.523066 | 1.546392 | 1.786812 | 0.001832 | 0.002000 | 0.001948 |
| memory_resource.test | RUN | 0.721603 | 2.029345 | 2.255550 | 0.001904 | 0.001984 | 0.001589 |
| monotonic_resource.test | RUN | 0.683922 | 1.951886 | 2.300490 | 0.002258 | 0.002288 | 0.002514 |
| natvis.test | RUN | 0.694418 | 1.898194 | 2.102133 | 0.001842 | 0.001938 | 0.001592 |
| null_resource.test | RUN | 0.560364 | 1.554725 | 1.704621 | 0.001899 | 0.002070 | 0.001617 |
| object.test | RUN | 1.080999 | 5.556982 | 5.245149 | 0.006251 | 0.005710 | 0.006007 |
| parse.test | RUN | 0.738703 | 2.811081 | 2.381935 | 0.002532 | 0.002530 | 0.002626 |
| parser.test | RUN | 0.719455 | 2.403816 | 2.453835 | 0.002070 | 0.002088 | 0.001955 |
| pilfer.test | RUN | 0.382423 | 1.077013 | 1.205822 | 0.001518 | 0.001779 | 0.001909 |
| pointer.test | RUN | 0.795677 | 3.250862 | 3.034715 | 0.002042 | 0.002191 | 0.002286 |
| result_for.test | RUN | 0.678741 | 1.943529 | 2.207417 | 0.002086 | 0.001879 | 0.001627 |
| serialize.test | RUN | 0.698913 | 2.015517 | 2.349965 | 0.001874 | 0.002069 | 0.001890 |
| serializer.test | RUN | 1.046982 | 5.799150 | 5.123602 | 0.002758 | 0.003034 | 0.002562 |
| snippets.test | RUN | 1.498585 | 7.580030 | 7.779843 | 0.002024 | 0.002068 | 0.002170 |
| static_resource.test | RUN | 0.671603 | 1.803652 | 2.209342 | 0.001966 | 0.001998 | 0.001980 |
| storage_ptr.test | RUN | 0.709524 | 1.953014 | 2.141989 | 0.001894 | 0.002040 | 0.001946 |
| stream_parser.test | RUN | 0.799622 | 3.658283 | 2.919546 | 0.090969 | 0.119889 | 0.080974 |
| string.test | RUN | 1.283227 | 8.578391 | 7.805445 | 0.002319 | 0.002595 | 0.002788 |
| string_view.test | RUN | 0.421212 | 1.114413 | 1.358991 | 0.001795 | 0.001798 | 0.001850 |
| value.test | RUN | 1.285192 | 9.247955 | 6.128848 | 0.003458 | 0.003578 | 0.003662 |
| value_from.test | RUN | 1.212780 | 5.154343 | 5.768845 | 0.002188 | 0.002002 | 0.001808 |
| value_stack.test | RUN | 0.684307 | 1.640882 | 2.042034 | 0.001825 | 0.001792 | 0.001906 |
| value_to.test | RUN | 2.757525 | 17.489077 | 14.538195 | 0.003191 | 0.003216 | 0.002810 |
| value_ref.test | RUN | 0.770034 | 2.518221 | 2.802571 | 0.002115 | 0.002191 | 0.001856 |
| utf8.test | RUN | 0.577303 | 1.659312 | 1.879520 | 0.001852 | 0.001498 | 0.001909 |
| visit.test | RUN | 0.773152 | 2.639636 | 2.636466 | 0.001878 | 0.002007 | 0.001845 |
| d2s_intrinsics_test.test | RUN | 0.570788 | 1.550126 | 1.653810 | 0.001837 | 0.001813 | 0.001613 |
| d2s_table_test.test | RUN | 0.580264 | 1.644805 | 1.707361 | 0.001797 | 0.001773 | 0.001692 |
| d2s_test.test | RUN | 0.601058 | 2.031746 | 1.999258 | 0.001936 | 0.001652 | 0.001818 |
| parse_into.test | RUN | 16.994621 | 142.950107 | 87.965194 | 0.003188 | 0.002904 | 0.002791 |
| limits.test | RUN | 2.112731 | 12.251258 | 9.230563 | 0.002135 | 0.002030 | 0.002231 |
| header_only.test | RUN | 1.315806 | 9.873662 | 6.185520 | 0.001850 | 0.001875 | 0.001625 |
| no_exceptions.test | RUN | 2.037043 | 11.258237 | 9.228729 | 0.001924 | 0.002100 | 0.001907 |
| intrusive_macros.test | RUN | 1.640429 | 10.755881 | 7.539666 | 0.001905 | 0.001982 | 0.001859 |
| _gdb-printers.test | RUN | 0.322868 | 0.798711 | 1.053670 | 0.180860 | 0.285244 | 0.204011 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.770× | 0.759×, 0.781× | 1.003× |
| clang | 1.016×† | 0.958×, 1.074× | 1.004× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| array.test | 10 | 0.001704s | 0.001555s | 0.001633s | byte equality observed |
| basic_parser.test | 1 | 0.409934s | 0.213983s | 0.454217s | native test oracle; output varies |
| conversion.test | 10 | 0.001027s | 0.001012s | 0.001015s | byte equality observed |
| d2s_intrinsics_test.test | 10 | 0.001030s | 0.001004s | 0.000992s | byte equality observed |
| d2s_table_test.test | 10 | 0.001034s | 0.001014s | 0.001027s | byte equality observed |
| d2s_test.test | 10 | 0.001076s | 0.001073s | 0.001083s | byte equality observed |
| doc_background.test | 10 | 0.001004s | 0.000998s | 0.000996s | byte equality observed |
| doc_forward_conversion.test | 10 | 0.001061s | 0.001048s | 0.001044s | byte equality observed |
| doc_parsing.test | 10 | 0.001068s | 0.001052s | 0.001048s | byte equality observed |
| doc_quick_look.test | 10 | 0.001044s | 0.001050s | 0.001025s | byte equality observed |
| doc_serializing.test | 10 | 0.001005s | 0.001008s | 0.001005s | byte equality observed |
| doc_storage_ptr.test | 10 | 0.001048s | 0.001044s | 0.001024s | byte equality observed |
| doc_uses_allocator.test | 10 | 0.001040s | 0.001008s | 0.000997s | byte equality observed |
| doc_using_numbers.test | 10 | 0.001017s | 0.001008s | 0.000992s | byte equality observed |
| double.test | 1 | 0.971153s | 0.814441s | 0.876537s | native test oracle; output varies |
| error.test | 10 | 0.001093s | 0.001065s | 0.001120s | byte equality observed |
| fwd.test | 10 | 0.001019s | 0.001008s | 0.001037s | byte equality observed |
| header_only.test | 10 | 0.000993s | 0.000982s | 0.000994s | byte equality observed |
| intrusive_macros.test | 10 | 0.001050s | 0.001024s | 0.001062s | byte equality observed |
| json.test | 10 | 0.001030s | 0.001016s | 0.001026s | byte equality observed |
| kind.test | 10 | 0.001050s | 0.001035s | 0.001051s | byte equality observed |
| limits.test | 10 | 0.001224s | 0.001207s | 0.001232s | byte equality observed |
| memory_resource.test | 10 | 0.001049s | 0.001043s | 0.001046s | byte equality observed |
| monotonic_resource.test | 10 | 0.001484s | 0.001347s | 0.001426s | byte equality observed |
| natvis.test | 10 | 0.001067s | 0.001036s | 0.001054s | byte equality observed |
| no_exceptions.test | 10 | 0.001055s | 0.001029s | 0.001056s | byte equality observed |
| null_resource.test | 10 | 0.001071s | 0.001077s | 0.001086s | byte equality observed |
| object.test | 5 | 0.003929s | 0.003243s | 0.003815s | native test oracle; output varies |
| parse.test | 10 | 0.001461s | 0.001426s | 0.001447s | byte equality observed |
| parse_into.test | 7 | 0.002417s | 0.002267s | 0.002194s | native test oracle; output varies |
| parser.test | 10 | 0.001177s | 0.001154s | 0.001173s | byte equality observed |
| pilfer.test | 10 | 0.001012s | 0.001013s | 0.001012s | byte equality observed |
| pointer.test | 10 | 0.001221s | 0.001195s | 0.001213s | byte equality observed |
| result_for.test | 10 | 0.001018s | 0.001033s | 0.001015s | byte equality observed |
| serialize.test | 10 | 0.001073s | 0.001065s | 0.001076s | byte equality observed |
| serializer.test | 10 | 0.001740s | 0.001683s | 0.001694s | byte equality observed |
| snippets.test | 10 | 0.001242s | 0.001205s | 0.001255s | byte equality observed |
| static_resource.test | 10 | 0.001121s | 0.001087s | 0.001112s | byte equality observed |
| storage_ptr.test | 10 | 0.001087s | 0.001082s | 0.001064s | byte equality observed |
| stream_parser.test | 1 | 0.052820s | 0.065316s | 0.046292s | native test oracle; output varies |
| string.test | 9 | 0.001730s | 0.001519s | 0.001736s | byte equality observed |
| string_view.test | 10 | 0.001005s | 0.001018s | 0.001005s | byte equality observed |
| utf8.test | 10 | 0.001018s | 0.001014s | 0.000998s | byte equality observed |
| value.test | 8 | 0.002108s | 0.001998s | 0.002365s | native test oracle; output varies |
| value_from.test | 10 | 0.001191s | 0.001186s | 0.001209s | byte equality observed |
| value_ref.test | 10 | 0.001090s | 0.001078s | 0.001073s | byte equality observed |
| value_stack.test | 10 | 0.001068s | 0.001061s | 0.001041s | byte equality observed |
| value_to.test | 10 | 0.001798s | 0.001776s | 0.001815s | native test oracle; output varies |
| visit.test | 10 | 0.001061s | 0.001068s | 0.001076s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
