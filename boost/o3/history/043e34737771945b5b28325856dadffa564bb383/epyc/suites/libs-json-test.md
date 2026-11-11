# libs/json/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| array.test | RUN | 0.861253 | 4.049625 | 3.711333 | 0.002733 | 0.002477 | 0.002655 |
| basic_parser.test | RUN | 4.244673 | 104.361883 | 67.470312 | 0.450055 | 0.151725 | 0.333492 |
| conversion.test | RUN | 0.642955 | 1.478168 | 1.736895 | 0.001458 | 0.001651 | 0.001440 |
| doc_background.test | RUN | 0.620081 | 1.446809 | 1.726121 | 0.001818 | 0.001908 | 0.001931 |
| doc_forward_conversion.test | RUN | 0.633295 | 1.687084 | 1.966888 | 0.002168 | 0.001771 | 0.001804 |
| doc_parsing.test | RUN | 0.626661 | 1.603162 | 1.770373 | 0.001972 | 0.001657 | 0.001715 |
| doc_quick_look.test | RUN | 0.778057 | 1.936312 | 2.512093 | 0.001681 | 0.001820 | 0.001791 |
| doc_serializing.test | RUN | 0.588064 | 1.518981 | 1.684176 | 0.001798 | 0.001553 | 0.001769 |
| doc_storage_ptr.test | RUN | 0.586668 | 1.438639 | 1.729985 | 0.001626 | 0.001741 | 0.001494 |
| doc_uses_allocator.test | RUN | 0.590060 | 1.386985 | 1.700997 | 0.002016 | 0.001666 | 0.001717 |
| doc_using_numbers.test | RUN | 0.628038 | 1.536389 | 1.795356 | 0.001894 | 0.001898 | 0.001842 |
| double.test | RUN | 0.696594 | 2.483117 | 1.991490 | 1.191554 | 0.896806 | 1.090615 |
| error.test | RUN | 0.492318 | 1.515249 | 1.325346 | 0.002090 | 0.001732 | 0.001922 |
| fwd.test | RUN | 0.575961 | 1.444322 | 1.633116 | 0.001785 | 0.001746 | 0.001768 |
| json.test | RUN | 0.628574 | 1.666277 | 1.887450 | 0.001930 | 0.001930 | 0.001780 |
| kind.test | RUN | 0.474872 | 1.349902 | 1.259514 | 0.001887 | 0.001937 | 0.001998 |
| memory_resource.test | RUN | 0.643298 | 1.697483 | 1.719916 | 0.001563 | 0.001984 | 0.002005 |
| monotonic_resource.test | RUN | 0.608378 | 1.641462 | 1.810770 | 0.002478 | 0.002324 | 0.002508 |
| natvis.test | RUN | 0.607915 | 1.541510 | 1.730395 | 0.001933 | 0.001922 | 0.001552 |
| null_resource.test | RUN | 0.499529 | 1.274258 | 1.322357 | 0.002166 | 0.001957 | 0.001990 |
| object.test | RUN | 1.052978 | 4.640743 | 4.433036 | 0.005548 | 0.004729 | 0.005691 |
| parse.test | RUN | 0.876015 | 2.360327 | 1.833685 | 0.002626 | 0.002467 | 0.002539 |
| parser.test | RUN | 0.717459 | 1.972378 | 1.779334 | 0.002244 | 0.002144 | 0.002259 |
| pilfer.test | RUN | 0.334874 | 0.912770 | 0.857240 | 0.001920 | 0.001848 | 0.001714 |
| pointer.test | RUN | 0.712364 | 2.793703 | 2.384542 | 0.002252 | 0.002358 | 0.002294 |
| result_for.test | RUN | 0.612687 | 1.728903 | 1.680763 | 0.001932 | 0.001933 | 0.001815 |
| serialize.test | RUN | 0.619619 | 1.812893 | 1.794717 | 0.002098 | 0.002000 | 0.002181 |
| serializer.test | RUN | 0.930086 | 4.927104 | 4.002095 | 0.002618 | 0.002755 | 0.002664 |
| snippets.test | RUN | 1.349293 | 6.999055 | 6.472066 | 0.002418 | 0.002240 | 0.002433 |
| static_resource.test | RUN | 0.598487 | 1.552489 | 1.705808 | 0.001998 | 0.003378 | 0.002132 |
| storage_ptr.test | RUN | 0.637747 | 1.603314 | 1.716495 | 0.001822 | 0.001996 | 0.002006 |
| stream_parser.test | RUN | 0.702715 | 2.816501 | 2.491419 | 0.063660 | 0.067351 | 0.068283 |
| string.test | RUN | 1.177466 | 7.460941 | 6.741712 | 0.002176 | 0.002554 | 0.002893 |
| string_view.test | RUN | 0.415871 | 0.852384 | 1.049679 | 0.001844 | 0.001798 | 0.001905 |
| value.test | RUN | 1.132919 | 7.896832 | 5.117733 | 0.002983 | 0.002859 | 0.003514 |
| value_from.test | RUN | 1.049871 | 4.400639 | 4.644394 | 0.002182 | 0.002116 | 0.002074 |
| value_stack.test | RUN | 0.600034 | 1.380922 | 1.519888 | 0.001970 | 0.002068 | 0.001964 |
| value_to.test | RUN | 2.338376 | 14.910420 | 10.835648 | 0.003175 | 0.003111 | 0.002634 |
| value_ref.test | RUN | 0.643459 | 2.094956 | 2.075796 | 0.002007 | 0.003028 | 0.001990 |
| utf8.test | RUN | 0.495470 | 1.435230 | 1.394445 | 0.001966 | 0.001874 | 0.001878 |
| visit.test | RUN | 0.653907 | 2.194001 | 2.008564 | 0.002048 | 0.001845 | 0.002028 |
| d2s_intrinsics_test.test | RUN | 0.490625 | 1.231202 | 1.381795 | 0.001926 | 0.001878 | 0.001544 |
| d2s_table_test.test | RUN | 0.521893 | 1.400713 | 1.468049 | 0.001944 | 0.001925 | 0.001912 |
| d2s_test.test | RUN | 0.522922 | 1.699305 | 1.606849 | 0.002012 | 0.001954 | 0.001949 |
| parse_into.test | RUN | 14.511986 | 126.560378 | 81.109322 | 0.003529 | 0.003015 | 0.003198 |
| limits.test | RUN | 1.853254 | 11.112186 | 7.990881 | 0.002082 | 0.002304 | 0.002200 |
| header_only.test | RUN | 1.148692 | 8.549340 | 5.156162 | 0.001855 | 0.001944 | 0.001686 |
| no_exceptions.test | RUN | 1.722117 | 9.967871 | 7.377237 | 0.001602 | 0.002051 | 0.001940 |
| intrusive_macros.test | RUN | 1.460943 | 9.373203 | 6.673779 | 0.001670 | 0.001933 | 0.001997 |
| _gdb-printers.test | RUN | 0.265786 | 0.586542 | 0.754799 | 0.166053 | 0.246655 | 0.231732 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.771×† | 0.773×, 0.768× | 0.945× |
| clang | 0.990× | 0.995×, 0.985× | 1.008× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| array.test | 7 | 0.001568s | 0.001490s | 0.001509s | byte equality observed |
| basic_parser.test | 1 | 0.285652s | 0.159517s | 0.362593s | native test oracle; output varies |
| conversion.test | 10 | 0.000961s | 0.001024s | 0.000964s | byte equality observed |
| d2s_intrinsics_test.test | 10 | 0.000959s | 0.001004s | 0.000956s | byte equality observed |
| d2s_table_test.test | 10 | 0.000981s | 0.000981s | 0.000977s | byte equality observed |
| d2s_test.test | 10 | 0.001042s | 0.001045s | 0.001025s | byte equality observed |
| doc_background.test | 10 | 0.000945s | 0.000966s | 0.000966s | byte equality observed |
| doc_forward_conversion.test | 10 | 0.001004s | 0.001013s | 0.001008s | byte equality observed |
| doc_parsing.test | 10 | 0.001003s | 0.001012s | 0.001017s | byte equality observed |
| doc_quick_look.test | 10 | 0.000974s | 0.001015s | 0.000992s | byte equality observed |
| doc_serializing.test | 10 | 0.000953s | 0.000976s | 0.000973s | byte equality observed |
| doc_storage_ptr.test | 10 | 0.000972s | 0.000980s | 0.000985s | byte equality observed |
| doc_uses_allocator.test | 10 | 0.000969s | 0.000966s | 0.000967s | byte equality observed |
| doc_using_numbers.test | 10 | 0.000963s | 0.000987s | 0.000968s | byte equality observed |
| double.test | 1 | 0.795660s | 0.649931s | 0.725218s | native test oracle; output varies |
| error.test | 10 | 0.001081s | 0.001001s | 0.001128s | byte equality observed |
| fwd.test | 10 | 0.000965s | 0.000972s | 0.000952s | byte equality observed |
| header_only.test | 10 | 0.000951s | 0.000941s | 0.000964s | byte equality observed |
| intrusive_macros.test | 10 | 0.001005s | 0.001032s | 0.001003s | byte equality observed |
| json.test | 10 | 0.000989s | 0.000975s | 0.000983s | byte equality observed |
| kind.test | 10 | 0.000999s | 0.001009s | 0.001012s | byte equality observed |
| limits.test | 10 | 0.001147s | 0.001172s | 0.001145s | byte equality observed |
| memory_resource.test | 10 | 0.000985s | 0.001008s | 0.001011s | byte equality observed |
| monotonic_resource.test | 10 | 0.001403s | 0.001296s | 0.001316s | byte equality observed |
| natvis.test | 10 | 0.001017s | 0.001043s | 0.001001s | byte equality observed |
| no_exceptions.test | 10 | 0.000987s | 0.001018s | 0.000991s | byte equality observed |
| null_resource.test | 10 | 0.001035s | 0.001089s | 0.001016s | byte equality observed |
| object.test | 7 | 0.003112s | 0.002708s | 0.003047s | native test oracle; output varies |
| parse.test | 10 | 0.001354s | 0.001395s | 0.001350s | byte equality observed |
| parse_into.test | 6 | 0.002495s | 0.002498s | 0.002231s | native test oracle; output varies |
| parser.test | 10 | 0.001116s | 0.001104s | 0.001094s | byte equality observed |
| pilfer.test | 10 | 0.000956s | 0.000953s | 0.000976s | byte equality observed |
| pointer.test | 10 | 0.001129s | 0.001145s | 0.001139s | byte equality observed |
| result_for.test | 10 | 0.000950s | 0.000975s | 0.000963s | byte equality observed |
| serialize.test | 10 | 0.001005s | 0.001036s | 0.001038s | byte equality observed |
| serializer.test | 10 | 0.001558s | 0.001563s | 0.001538s | native test oracle; output varies |
| snippets.test | 10 | 0.001179s | 0.001170s | 0.001153s | byte equality observed |
| static_resource.test | 10 | 0.001047s | 0.001058s | 0.001035s | byte equality observed |
| storage_ptr.test | 10 | 0.001013s | 0.001042s | 0.001038s | byte equality observed |
| stream_parser.test | 1 | 0.041839s | 0.046896s | 0.040420s | native test oracle; output varies |
| string.test | 10 | 0.001580s | 0.001512s | 0.001550s | byte equality observed |
| string_view.test | 10 | 0.000963s | 0.001011s | 0.000986s | byte equality observed |
| utf8.test | 10 | 0.000954s | 0.000978s | 0.000959s | byte equality observed |
| value.test | 9 | 0.001815s | 0.001748s | 0.002037s | native test oracle; output varies |
| value_from.test | 10 | 0.001129s | 0.001131s | 0.001123s | byte equality observed |
| value_ref.test | 10 | 0.001019s | 0.001027s | 0.001029s | byte equality observed |
| value_stack.test | 10 | 0.001009s | 0.001039s | 0.001035s | byte equality observed |
| value_to.test | 10 | 0.001626s | 0.001634s | 0.001686s | native test oracle; output varies |
| visit.test | 10 | 0.001003s | 0.001012s | 0.001020s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
