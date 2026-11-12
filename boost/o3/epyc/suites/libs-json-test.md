# libs/json/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| array.test | RUN | 1.184660 | 6.684091 | 6.747578 | 0.003399 | 0.003689 | 0.003607 |
| basic_parser.test | RUN | 7.281878 | 150.829855 | 98.544858 | 0.864576 | 0.273898 | 1.130865 |
| conversion.test | RUN | 0.880164 | 2.592028 | 3.489954 | 0.002102 | 0.002350 | 0.002082 |
| doc_background.test | RUN | 0.918285 | 2.562962 | 3.503210 | 0.002224 | 0.002370 | 0.002067 |
| doc_forward_conversion.test | RUN | 0.928984 | 2.987918 | 3.967398 | 0.002390 | 0.002484 | 0.002366 |
| doc_parsing.test | RUN | 0.994169 | 2.643455 | 3.732468 | 0.002486 | 0.002360 | 0.002196 |
| doc_quick_look.test | RUN | 1.224588 | 3.270880 | 4.923943 | 0.002024 | 0.002275 | 0.001972 |
| doc_serializing.test | RUN | 0.854609 | 2.484192 | 3.363759 | 0.002065 | 0.002053 | 0.002113 |
| doc_storage_ptr.test | RUN | 0.864968 | 2.393995 | 3.337201 | 0.002248 | 0.002226 | 0.002092 |
| doc_uses_allocator.test | RUN | 0.836628 | 2.396407 | 3.391691 | 0.002504 | 0.002234 | 0.002212 |
| doc_using_numbers.test | RUN | 0.921145 | 2.508451 | 3.727203 | 0.002995 | 0.002046 | 0.002058 |
| double.test | RUN | 1.041885 | 4.212874 | 4.429662 | 2.241017 | 2.009203 | 2.197477 |
| error.test | RUN | 0.699178 | 2.379702 | 2.766413 | 0.002132 | 0.002360 | 0.001916 |
| fwd.test | RUN | 0.827172 | 2.387140 | 3.051700 | 0.002006 | 0.002044 | 0.001772 |
| json.test | RUN | 0.900154 | 2.741592 | 3.861215 | 0.001719 | 0.002274 | 0.002174 |
| kind.test | RUN | 0.623062 | 2.252378 | 2.644506 | 0.002447 | 0.002510 | 0.002197 |
| memory_resource.test | RUN | 0.873786 | 2.809696 | 3.652681 | 0.002284 | 0.002322 | 0.002444 |
| monotonic_resource.test | RUN | 0.845043 | 2.730663 | 3.239354 | 0.003348 | 0.003231 | 0.003068 |
| natvis.test | RUN | 0.881310 | 2.605059 | 3.344736 | 0.002037 | 0.002380 | 0.002220 |
| null_resource.test | RUN | 0.708756 | 2.209780 | 2.737980 | 0.002312 | 0.002452 | 0.002212 |
| object.test | RUN | 1.408842 | 7.826468 | 8.117753 | 0.009740 | 0.007691 | 0.008540 |
| parse.test | RUN | 0.961251 | 3.987815 | 3.883990 | 0.003138 | 0.003761 | 0.002974 |
| parser.test | RUN | 0.923018 | 3.455522 | 3.422894 | 0.002467 | 0.002500 | 0.002218 |
| pilfer.test | RUN | 0.489313 | 1.547869 | 1.577374 | 0.003001 | 0.002178 | 0.002540 |
| pointer.test | RUN | 1.045465 | 4.618366 | 4.438766 | 0.002634 | 0.002778 | 0.002595 |
| result_for.test | RUN | 0.882052 | 2.742448 | 3.679256 | 0.001957 | 0.002150 | 0.002141 |
| serialize.test | RUN | 0.879210 | 2.858669 | 3.332996 | 0.002197 | 0.002418 | 0.002244 |
| serializer.test | RUN | 1.358270 | 8.350919 | 7.944740 | 0.003856 | 0.003536 | 0.003348 |
| snippets.test | RUN | 2.134143 | 10.551094 | 11.021304 | 0.002668 | 0.002651 | 0.002662 |
| static_resource.test | RUN | 0.889173 | 2.508212 | 3.058641 | 0.002835 | 0.002248 | 0.002430 |
| storage_ptr.test | RUN | 0.894879 | 2.794294 | 3.203725 | 0.002115 | 0.002270 | 0.002348 |
| stream_parser.test | RUN | 1.058035 | 5.207653 | 4.333356 | 0.140921 | 0.170622 | 0.111765 |
| string.test | RUN | 1.700338 | 10.820085 | 11.563331 | 0.003781 | 0.003236 | 0.004204 |
| string_view.test | RUN | 0.535278 | 1.694117 | 1.807674 | 0.002134 | 0.002347 | 0.002166 |
| value.test | RUN | 1.799022 | 13.210598 | 9.840445 | 0.004760 | 0.004014 | 0.005656 |
| value_from.test | RUN | 1.680783 | 7.485305 | 8.193909 | 0.002967 | 0.002142 | 0.002424 |
| value_stack.test | RUN | 0.868074 | 2.330910 | 3.220640 | 0.002172 | 0.002053 | 0.002245 |
| value_to.test | RUN | 3.815541 | 23.570951 | 21.759110 | 0.004101 | 0.004562 | 0.003974 |
| value_ref.test | RUN | 0.955530 | 3.343695 | 4.216256 | 0.002302 | 0.002913 | 0.001850 |
| utf8.test | RUN | 0.750020 | 2.362018 | 2.766303 | 0.002138 | 0.002478 | 0.002041 |
| visit.test | RUN | 0.955745 | 3.594396 | 3.762907 | 0.002282 | 0.002381 | 0.002257 |
| d2s_intrinsics_test.test | RUN | 0.707497 | 2.205715 | 2.710287 | 0.002146 | 0.002178 | 0.002020 |
| d2s_table_test.test | RUN | 0.730160 | 2.418432 | 2.581454 | 0.002363 | 0.003306 | 0.002176 |
| d2s_test.test | RUN | 0.785642 | 2.966158 | 3.229373 | 0.002341 | 0.002493 | 0.002392 |
| parse_into.test | RUN | 21.378711 | 186.341150 | 120.192041 | 0.004232 | 0.003242 | 0.003262 |
| limits.test | RUN | 2.763083 | 16.537941 | 14.459209 | 0.002536 | 0.002370 | 0.002542 |
| header_only.test | RUN | 1.800310 | 12.400006 | 9.919495 | 0.002160 | 0.002102 | 0.001952 |
| no_exceptions.test | RUN | 2.585432 | 14.597692 | 13.899619 | 0.002425 | 0.002320 | 0.002054 |
| intrusive_macros.test | RUN | 2.172670 | 13.828938 | 11.278513 | 0.002088 | 0.003235 | 0.002160 |
| _gdb-printers.test | RUN | 0.410803 | 1.088035 | 1.609902 | 0.249872 | 0.291977 | 0.263810 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.841× | 0.849×, 0.833× | 0.982× |
| clang | 1.010× | 1.012×, 1.009× | 0.993× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898266).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| array.test | 8 | 0.002120s | 0.001915s | 0.002050s | native test oracle; output varies |
| basic_parser.test | 1 | 0.492783s | 0.297169s | 0.649663s | native test oracle; output varies |
| conversion.test | 10 | 0.001221s | 0.001243s | 0.001201s | byte equality observed |
| d2s_intrinsics_test.test | 10 | 0.001221s | 0.001259s | 0.001227s | byte equality observed |
| d2s_table_test.test | 10 | 0.001241s | 0.001251s | 0.001220s | byte equality observed |
| d2s_test.test | 10 | 0.001308s | 0.001308s | 0.001264s | byte equality observed |
| doc_background.test | 10 | 0.001266s | 0.001253s | 0.001199s | byte equality observed |
| doc_forward_conversion.test | 10 | 0.001282s | 0.001301s | 0.001257s | byte equality observed |
| doc_parsing.test | 10 | 0.001297s | 0.001309s | 0.001271s | byte equality observed |
| doc_quick_look.test | 10 | 0.001258s | 0.001278s | 0.001228s | byte equality observed |
| doc_serializing.test | 10 | 0.001261s | 0.001225s | 0.001205s | byte equality observed |
| doc_storage_ptr.test | 10 | 0.001255s | 0.001245s | 0.001245s | byte equality observed |
| doc_uses_allocator.test | 10 | 0.001256s | 0.001233s | 0.001208s | byte equality observed |
| doc_using_numbers.test | 10 | 0.001262s | 0.001232s | 0.001208s | byte equality observed |
| double.test | 1 | 1.289255s | 1.184500s | 1.179050s | native test oracle; output varies |
| error.test | 10 | 0.001343s | 0.001367s | 0.001295s | byte equality observed |
| fwd.test | 10 | 0.001227s | 0.001211s | 0.001212s | byte equality observed |
| header_only.test | 10 | 0.001219s | 0.001191s | 0.001202s | byte equality observed |
| intrusive_macros.test | 10 | 0.001263s | 0.001257s | 0.001242s | byte equality observed |
| json.test | 10 | 0.001249s | 0.001229s | 0.001227s | byte equality observed |
| kind.test | 10 | 0.001270s | 0.001255s | 0.001265s | byte equality observed |
| limits.test | 10 | 0.001479s | 0.001447s | 0.001469s | byte equality observed |
| memory_resource.test | 10 | 0.001265s | 0.001257s | 0.001247s | byte equality observed |
| monotonic_resource.test | 10 | 0.001817s | 0.001620s | 0.001724s | native test oracle; output varies |
| natvis.test | 10 | 0.001279s | 0.001261s | 0.001239s | byte equality observed |
| no_exceptions.test | 10 | 0.001269s | 0.001258s | 0.001247s | byte equality observed |
| null_resource.test | 10 | 0.001302s | 0.001289s | 0.001256s | byte equality observed |
| object.test | 4 | 0.005161s | 0.004150s | 0.004897s | native test oracle; output varies |
| parse.test | 9 | 0.001812s | 0.001756s | 0.001714s | byte equality observed |
| parse_into.test | 6 | 0.002896s | 0.002757s | 0.002613s | native test oracle; output varies |
| parser.test | 10 | 0.001445s | 0.001424s | 0.001376s | byte equality observed |
| pilfer.test | 10 | 0.001240s | 0.001208s | 0.001208s | byte equality observed |
| pointer.test | 10 | 0.001487s | 0.001440s | 0.001469s | byte equality observed |
| result_for.test | 10 | 0.001246s | 0.001222s | 0.001211s | byte equality observed |
| serialize.test | 10 | 0.001295s | 0.001282s | 0.001299s | byte equality observed |
| serializer.test | 9 | 0.002114s | 0.002054s | 0.002023s | native test oracle; output varies |
| snippets.test | 10 | 0.001486s | 0.001445s | 0.001428s | byte equality observed |
| static_resource.test | 10 | 0.001338s | 0.001283s | 0.001279s | byte equality observed |
| storage_ptr.test | 10 | 0.001306s | 0.001261s | 0.001261s | byte equality observed |
| stream_parser.test | 1 | 0.083077s | 0.078460s | 0.056831s | native test oracle; output varies |
| string.test | 6 | 0.002058s | 0.001966s | 0.002027s | byte equality observed |
| string_view.test | 9 | 0.001218s | 0.001217s | 0.001239s | byte equality observed |
| utf8.test | 10 | 0.001225s | 0.001205s | 0.001230s | byte equality observed |
| value.test | 6 | 0.002731s | 0.002461s | 0.003001s | native test oracle; output varies |
| value_from.test | 10 | 0.001488s | 0.001460s | 0.001431s | byte equality observed |
| value_ref.test | 10 | 0.001311s | 0.001319s | 0.001291s | byte equality observed |
| value_stack.test | 10 | 0.001293s | 0.001292s | 0.001288s | byte equality observed |
| value_to.test | 8 | 0.002306s | 0.002252s | 0.002188s | native test oracle; output varies |
| visit.test | 10 | 0.001274s | 0.001289s | 0.001261s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898266).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
