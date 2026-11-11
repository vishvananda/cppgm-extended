# libs/json/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| array.test | RUN | 1.852088 | 6.552386 | 5.713180 | 0.003387 | 0.003138 | 0.004826 |
| basic_parser.test | RUN | 8.801991 | 184.039327 | 80.277529 | 0.730114 | 0.351736 | 0.848077 |
| conversion.test | RUN | 1.281935 | 2.436152 | 2.897955 | 0.002104 | 0.002010 | 0.003543 |
| doc_background.test | RUN | 1.409810 | 2.685853 | 2.803912 | 0.002144 | 0.002002 | 0.001978 |
| doc_forward_conversion.test | RUN | 1.276400 | 2.956713 | 3.217435 | 0.002875 | 0.003136 | 0.003096 |
| doc_parsing.test | RUN | 1.511704 | 2.858863 | 2.978731 | 0.002315 | 0.002221 | 0.002565 |
| doc_quick_look.test | RUN | 1.543674 | 3.308313 | 3.945890 | 0.002100 | 0.003076 | 0.002616 |
| doc_serializing.test | RUN | 1.100753 | 2.416636 | 2.754699 | 0.003011 | 0.002160 | 0.002138 |
| doc_storage_ptr.test | RUN | 1.142185 | 2.424455 | 2.872321 | 0.002546 | 0.003572 | 0.002056 |
| doc_uses_allocator.test | RUN | 1.214863 | 2.295368 | 2.708044 | 0.002030 | 0.002197 | 0.002365 |
| doc_using_numbers.test | RUN | 1.480561 | 2.446987 | 3.040656 | 0.002228 | 0.002295 | 0.001720 |
| double.test | RUN | 1.480228 | 4.142825 | 3.745630 | 3.069703 | 1.744926 | 1.666528 |
| error.test | RUN | 0.954279 | 2.419424 | 2.314090 | 0.002030 | 0.001894 | 0.002350 |
| fwd.test | RUN | 1.445138 | 2.426165 | 3.030743 | 0.001853 | 0.002036 | 0.002206 |
| json.test | RUN | 1.129837 | 2.737828 | 3.259105 | 0.001968 | 0.002248 | 0.001708 |
| kind.test | RUN | 0.835156 | 2.140414 | 2.201806 | 0.002226 | 0.002062 | 0.001722 |
| memory_resource.test | RUN | 1.277066 | 2.946034 | 3.141586 | 0.002270 | 0.001736 | 0.002366 |
| monotonic_resource.test | RUN | 1.147886 | 2.602840 | 3.007174 | 0.004643 | 0.003385 | 0.002665 |
| natvis.test | RUN | 1.039266 | 2.717990 | 2.842062 | 0.002218 | 0.001951 | 0.002229 |
| null_resource.test | RUN | 1.060950 | 2.103180 | 2.300140 | 0.002058 | 0.002818 | 0.002324 |
| object.test | RUN | 1.764967 | 7.704649 | 7.122165 | 0.007521 | 0.006186 | 0.007782 |
| parse.test | RUN | 1.292515 | 3.804679 | 3.328829 | 0.002640 | 0.002434 | 0.002715 |
| parser.test | RUN | 1.388407 | 3.389651 | 3.166508 | 0.001888 | 0.002216 | 0.002565 |
| pilfer.test | RUN | 0.633338 | 1.479226 | 1.592884 | 0.002219 | 0.002074 | 0.002146 |
| pointer.test | RUN | 1.361017 | 4.561113 | 3.994253 | 0.002244 | 0.002508 | 0.002206 |
| result_for.test | RUN | 0.935461 | 2.598055 | 3.009140 | 0.001956 | 0.002284 | 0.002050 |
| serialize.test | RUN | 0.962702 | 2.769573 | 2.927709 | 0.002623 | 0.002173 | 0.002132 |
| serializer.test | RUN | 1.771127 | 8.031847 | 6.374501 | 0.004120 | 0.004763 | 0.004196 |
| snippets.test | RUN | 2.517168 | 11.284330 | 9.769072 | 0.002549 | 0.002543 | 0.002709 |
| static_resource.test | RUN | 0.935947 | 2.657351 | 2.837477 | 0.001920 | 0.001960 | 0.001748 |
| storage_ptr.test | RUN | 1.005631 | 2.673481 | 3.006645 | 0.001951 | 0.002169 | 0.002400 |
| stream_parser.test | RUN | 1.177922 | 5.119180 | 4.232293 | 0.113579 | 0.121736 | 0.092768 |
| string.test | RUN | 2.393872 | 12.636430 | 9.897867 | 0.005380 | 0.002908 | 0.003613 |
| string_view.test | RUN | 0.567562 | 1.667896 | 1.683451 | 0.002526 | 0.001858 | 0.001593 |
| value.test | RUN | 1.905337 | 13.845063 | 8.720787 | 0.005896 | 0.004492 | 0.004813 |
| value_from.test | RUN | 1.915997 | 8.632322 | 7.693824 | 0.002332 | 0.002481 | 0.003576 |
| value_stack.test | RUN | 0.869162 | 2.498455 | 2.866912 | 0.002718 | 0.002020 | 0.002044 |
| value_to.test | RUN | 4.443335 | 25.475960 | 17.708632 | 0.003670 | 0.004294 | 0.004466 |
| value_ref.test | RUN | 1.049505 | 3.511802 | 3.619375 | 0.002198 | 0.003316 | 0.002122 |
| utf8.test | RUN | 0.834425 | 2.396435 | 2.452286 | 0.001845 | 0.002370 | 0.002090 |
| visit.test | RUN | 1.068528 | 3.660501 | 3.294365 | 0.002310 | 0.002220 | 0.002396 |
| d2s_intrinsics_test.test | RUN | 0.991420 | 2.283342 | 2.280821 | 0.002092 | 0.001985 | 0.001910 |
| d2s_table_test.test | RUN | 0.853027 | 2.608297 | 2.407355 | 0.002971 | 0.001934 | 0.002057 |
| d2s_test.test | RUN | 0.996991 | 3.104077 | 2.676500 | 0.002529 | 0.002496 | 0.002173 |
| parse_into.test | RUN | 37.141632 | 252.807691 | 152.275149 | 0.004860 | 0.005432 | 0.004218 |
| limits.test | RUN | 3.033213 | 19.328111 | 12.522282 | 0.002506 | 0.002427 | 0.002432 |
| header_only.test | RUN | 1.888777 | 14.023561 | 8.802649 | 0.002050 | 0.001935 | 0.002955 |
| no_exceptions.test | RUN | 3.347233 | 16.797388 | 11.533478 | 0.002114 | 0.002224 | 0.002380 |
| intrusive_macros.test | RUN | 3.169339 | 15.456035 | 10.170288 | 0.002240 | 0.003402 | 0.002604 |
| _gdb-printers.test | RUN | 0.378320 | 1.138871 | 1.297752 | 0.336695 | 0.406113 | 0.390259 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.818×† | 0.797×, 0.839× | 1.011× |
| clang | 0.907×† | 0.822×, 0.992× | 0.996× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1353 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| array.test | 4 | 0.003103s | 0.002745s | 0.003038s | native test oracle; output varies |
| basic_parser.test | 1 | 0.700037s | 0.410576s | 0.794785s | native test oracle; output varies |
| conversion.test | 8 | 0.001747s | 0.001921s | 0.001705s | byte equality observed |
| d2s_intrinsics_test.test | 8 | 0.001674s | 0.001635s | 0.001733s | byte equality observed |
| d2s_table_test.test | 7 | 0.001783s | 0.001700s | 0.001704s | byte equality observed |
| d2s_test.test | 7 | 0.001818s | 0.001998s | 0.001903s | byte equality observed |
| doc_background.test | 7 | 0.001698s | 0.001749s | 0.001718s | byte equality observed |
| doc_forward_conversion.test | 5 | 0.001845s | 0.001818s | 0.001722s | byte equality observed |
| doc_parsing.test | 7 | 0.001770s | 0.001760s | 0.001769s | byte equality observed |
| doc_quick_look.test | 7 | 0.001708s | 0.001764s | 0.001791s | byte equality observed |
| doc_serializing.test | 8 | 0.001678s | 0.001830s | 0.001718s | byte equality observed |
| doc_storage_ptr.test | 6 | 0.001742s | 0.001698s | 0.001805s | byte equality observed |
| doc_uses_allocator.test | 8 | 0.001744s | 0.001783s | 0.001617s | byte equality observed |
| doc_using_numbers.test | 8 | 0.001659s | 0.001725s | 0.001679s | byte equality observed |
| double.test | 1 | 1.800706s | 1.596635s | 1.669963s | native test oracle; output varies |
| error.test | 7 | 0.001803s | 0.001820s | 0.001785s | byte equality observed |
| fwd.test | 8 | 0.001734s | 0.001677s | 0.001735s | byte equality observed |
| header_only.test | 6 | 0.001763s | 0.001660s | 0.001659s | byte equality observed |
| intrusive_macros.test | 7 | 0.001768s | 0.001758s | 0.001725s | byte equality observed |
| json.test | 6 | 0.001691s | 0.001858s | 0.001777s | byte equality observed |
| kind.test | 6 | 0.001797s | 0.001794s | 0.001721s | byte equality observed |
| limits.test | 7 | 0.002183s | 0.002016s | 0.002035s | byte equality observed |
| memory_resource.test | 8 | 0.001773s | 0.001777s | 0.001727s | byte equality observed |
| monotonic_resource.test | 6 | 0.002698s | 0.002510s | 0.002408s | native test oracle; output varies |
| natvis.test | 7 | 0.001790s | 0.001796s | 0.001769s | byte equality observed |
| no_exceptions.test | 7 | 0.001807s | 0.001839s | 0.001742s | byte equality observed |
| null_resource.test | 7 | 0.001793s | 0.001760s | 0.001717s | byte equality observed |
| object.test | 3 | 0.008147s | 0.006147s | 0.007230s | native test oracle; output varies |
| parse.test | 6 | 0.002439s | 0.002407s | 0.002422s | native test oracle; output varies |
| parse_into.test | 4 | 0.003964s | 0.003768s | 0.003339s | native test oracle; output varies |
| parser.test | 7 | 0.001979s | 0.002048s | 0.001905s | byte equality observed |
| pilfer.test | 7 | 0.001775s | 0.001648s | 0.001690s | byte equality observed |
| pointer.test | 7 | 0.002048s | 0.002015s | 0.002088s | native test oracle; output varies |
| result_for.test | 7 | 0.001738s | 0.001845s | 0.001772s | byte equality observed |
| serialize.test | 7 | 0.001910s | 0.001935s | 0.001686s | byte equality observed |
| serializer.test | 6 | 0.002896s | 0.002879s | 0.002766s | native test oracle; output varies |
| snippets.test | 7 | 0.002031s | 0.001994s | 0.001989s | native test oracle; output varies |
| static_resource.test | 7 | 0.001952s | 0.001909s | 0.001822s | native test oracle; output varies |
| storage_ptr.test | 8 | 0.001816s | 0.001799s | 0.001713s | byte equality observed |
| stream_parser.test | 1 | 0.106716s | 0.102120s | 0.100627s | native test oracle; output varies |
| string.test | 6 | 0.003136s | 0.002527s | 0.002818s | native test oracle; output varies |
| string_view.test | 8 | 0.001755s | 0.001679s | 0.001680s | byte equality observed |
| utf8.test | 9 | 0.001743s | 0.001682s | 0.001629s | byte equality observed |
| value.test | 3 | 0.004158s | 0.003647s | 0.004277s | native test oracle; output varies |
| value_from.test | 7 | 0.001966s | 0.001985s | 0.001943s | native test oracle; output varies |
| value_ref.test | 9 | 0.001820s | 0.001747s | 0.001805s | byte equality observed |
| value_stack.test | 7 | 0.001885s | 0.001725s | 0.001717s | native test oracle; output varies |
| value_to.test | 5 | 0.003382s | 0.003089s | 0.003097s | native test oracle; output varies |
| visit.test | 7 | 0.001905s | 0.001684s | 0.001822s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
