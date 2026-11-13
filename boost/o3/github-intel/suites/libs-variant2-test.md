# libs/variant2/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| quick.test | RUN | 0.110705 | 0.256293 | 0.320558 | 0.001330 | 0.000572 | 0.001289 |
| variant_size.test | RUN | 0.192491 | 0.627959 | 0.636805 | 0.001414 | 0.001359 | 0.001370 |
| variant_alternative.test | RUN | 0.256961 | 2.023926 | 1.113856 | 0.001369 | 0.001380 | 0.001310 |
| variant_holds_alternative.test | RUN | 0.182043 | 0.455644 | 0.610917 | 0.001365 | 0.001353 | 0.001258 |
| variant_holds_alternative_cx.test | COMPILE | 0.114269 | 0.266144 | 0.340796 | — | — | — |
| variant_get_by_index.test | RUN | 0.298662 | 2.217388 | 1.329918 | 0.001531 | 0.001724 | 0.001640 |
| variant_get_by_index_cx.test | COMPILE | 0.117045 | 0.279869 | 0.361160 | — | — | — |
| variant_get_by_type.test | RUN | 0.303192 | 2.380365 | 1.491689 | 0.001405 | 0.001429 | 0.001388 |
| variant_get_by_type_cx.test | COMPILE | 0.110699 | 0.238000 | 0.290541 | — | — | — |
| variant_default_construct.test | RUN | 0.280806 | 1.714099 | 1.233993 | 0.001285 | 0.001274 | 0.001312 |
| variant_default_construct_cx.test | COMPILE | 0.119146 | 0.265549 | 0.377161 | — | — | — |
| variant_copy_construct.test | RUN | 0.450324 | 2.900327 | 2.089112 | 0.001530 | 0.001335 | 0.001361 |
| variant_copy_construct_cx.test | COMPILE | 0.137677 | 0.300787 | 0.421549 | — | — | — |
| variant_move_construct.test | RUN | 0.421433 | 2.499658 | 1.921203 | 0.001371 | 0.001296 | 0.001228 |
| variant_move_construct_cx.test | COMPILE | 0.105286 | 0.247539 | 0.299184 | — | — | — |
| variant_value_construct.test | RUN | 0.225788 | 0.618305 | 0.801043 | 0.001349 | 0.001360 | 0.001369 |
| variant_value_construct_cx.test | COMPILE | 0.121943 | 0.279989 | 0.347794 | — | — | — |
| variant_in_place_index_construct.test | RUN | 0.229737 | 0.630532 | 0.796177 | 0.001410 | 0.001396 | 0.001235 |
| variant_in_place_index_construct_cx.test | COMPILE | 0.120155 | 0.283425 | 0.359031 | — | — | — |
| variant_in_place_type_construct.test | RUN | 0.222777 | 0.616712 | 0.766566 | 0.001321 | 0.001268 | 0.001225 |
| variant_in_place_type_construct_cx.test | COMPILE | 0.119505 | 0.284585 | 0.359000 | — | — | — |
| variant_copy_assign.test | RUN | 0.299683 | 2.015744 | 1.272494 | 0.001293 | 0.001294 | 0.001239 |
| variant_copy_assign_cx.test | COMPILE | 0.105471 | 0.239216 | 0.295189 | — | — | — |
| variant_move_assign.test | RUN | 0.302606 | 2.006518 | 1.263709 | 0.001332 | 0.001393 | 0.001110 |
| variant_move_assign_cx.test | COMPILE | 0.109266 | 0.243007 | 0.291498 | — | — | — |
| variant_value_assign.test | RUN | 0.226145 | 0.708724 | 0.866573 | 0.001370 | 0.001338 | 0.001164 |
| variant_value_assign_cx.test | COMPILE | 0.106501 | 0.237652 | 0.301477 | — | — | — |
| variant_emplace_index.test | RUN | 0.261975 | 0.798053 | 0.942517 | 0.001357 | 0.001358 | 0.001262 |
| variant_emplace_index_cx.test | COMPILE | 0.105708 | 0.235093 | 0.298020 | — | — | — |
| variant_emplace_type.test | RUN | 0.235742 | 0.689592 | 0.843551 | 0.001221 | 0.001282 | 0.001551 |
| variant_emplace_type_cx.test | COMPILE | 0.107956 | 0.244598 | 0.295809 | — | — | — |
| variant_swap.test | RUN | 0.242847 | 0.858281 | 0.920971 | 0.001421 | 0.001316 | 0.001219 |
| variant_eq_ne.test | RUN | 0.192734 | 0.502430 | 0.662051 | 0.001364 | 0.001226 | 0.001260 |
| variant_eq_ne_cx.test | COMPILE | 0.105766 | 0.240082 | 0.319850 | — | — | — |
| variant_lt_gt.test | RUN | 0.204220 | 0.583078 | 0.688546 | 0.001454 | 0.001350 | 0.001285 |
| variant_lt_gt_cx.test | COMPILE | 0.107352 | 0.236430 | 0.300091 | — | — | — |
| variant_destroy.test | RUN | 0.208075 | 0.586731 | 0.709440 | 0.001266 | 0.001586 | 0.001262 |
| variant_visit.test | RUN | 1.160408 | 4.435355 | 11.914966 | 0.001398 | 0.001388 | 0.001364 |
| variant_convert_construct.test | RUN | 0.259101 | 0.774038 | 0.981757 | 0.001312 | 0.001470 | 0.001340 |
| variant_subset.test | RUN | 0.264379 | 0.859668 | 1.069451 | 0.001412 | 0.001388 | 0.001277 |
| variant_valueless.test | RUN | 0.192491 | 0.534360 | 0.684701 | 0.001526 | 0.001374 | 0.001365 |
| variant_copy_construct_throw.test | RUN | 0.151309 | 0.398510 | 0.506589 | 0.001291 | 0.001414 | 0.001280 |
| variant_move_construct_throw.test | RUN | 0.151177 | 0.385394 | 0.498893 | 0.001344 | 0.001386 | 0.001462 |
| variant_convert_construct_throw.test | RUN | 0.158300 | 0.426174 | 0.523613 | 0.001486 | 0.001417 | 0.001309 |
| variant_copy_assign_throw.test | RUN | 0.149146 | 0.376802 | 0.488993 | 0.001303 | 0.001428 | 0.001262 |
| variant_move_assign_throw.test | RUN | 0.154054 | 0.383288 | 0.491336 | 0.001414 | 0.001383 | 0.001443 |
| variant_get_by_index_nx.test | RUN | 0.379650 | 1.880979 | 1.275394 | 0.001348 | 0.001245 | 0.001340 |
| variant_get_by_index_cx_nx.test | COMPILE | 0.122696 | 0.280785 | 0.362013 | — | — | — |
| variant_get_by_type_nx.test | RUN | 0.374633 | 2.099788 | 1.383407 | 0.001427 | 0.001251 | 0.001244 |
| variant_get_by_type_cx_nx.test | COMPILE | 0.105217 | 0.239002 | 0.307257 | — | — | — |
| variant_subset_nx.test | RUN | 0.278689 | 0.805187 | 1.035542 | 0.001390 | 0.001279 | 0.001302 |
| variant_hash.test | RUN | 0.271943 | 0.937190 | 0.957271 | 0.001428 | 0.001425 | 0.001262 |
| variant_trivial.test | RUN | 1.516455 | 5.092251 | 6.856752 | 0.001404 | 0.001456 | 0.001403 |
| variant_special.test | RUN | 1.507307 | 5.609791 | 7.431786 | 0.001566 | 0.001450 | 0.001298 |
| variant_visit_derived.test | RUN | 0.187315 | 0.489033 | 0.709324 | 0.001625 | 0.001391 | 0.001284 |
| variant_many_types.test | RUN | 1.608008 | 6.948299 | 9.924530 | 0.001302 | 0.001254 | 0.001331 |
| variant_visit_r.test | RUN | 0.623005 | 2.252750 | 3.949370 | 0.001343 | 0.001302 | 0.001256 |
| variant_derived_construct.test | COMPILE | 0.114621 | 0.244107 | 0.318377 | — | — | — |
| variant_visit_by_index.test | RUN | 0.240414 | 0.631753 | 0.929750 | 0.001242 | 0.001308 | 0.001303 |
| variant_ostream_insert.test | RUN | 0.176060 | 0.522338 | 0.588340 | 0.001386 | 0.001299 | 0.001308 |
| is_output_streamable.test | RUN | 0.220965 | 1.077529 | 0.851032 | 0.001271 | 0.001367 | 0.001366 |
| variant_json_value_from.test | RUN | 0.334644 | 0.824622 | 1.092649 | 0.001342 | 0.001426 | 0.001322 |
| variant_json_value_to.test | RUN | 0.411113 | 1.169914 | 1.468099 | 0.001493 | 0.001438 | 0.001254 |
| variant_uses_double_storage.test | COMPILE | 0.113640 | 0.249694 | 0.346275 | — | — | — |
| variant_derived_construct2.test | RUN | 0.156477 | 0.408781 | 0.519435 | 0.001363 | 0.001372 | 0.001147 |
| variant_visit_cx.test | COMPILE | 0.105490 | 0.242729 | 0.298685 | — | — | — |
| variant_visit_cx_2.test | COMPILE | 0.104896 | 0.233661 | 0.292283 | — | — | — |
| variant_visit_r_cx.test | COMPILE | 0.106773 | 0.234153 | 0.293965 | — | — | — |
| variant_index_type_126.test | RUN | 0.932662 | 2.237016 | 4.353302 | 0.001235 | 0.001390 | 0.001309 |
| variant_index_type_127.test | RUN | 0.965729 | 2.236144 | 4.370399 | 0.001255 | 0.001374 | 0.001373 |
| variant_index_type_128.test | RUN | 0.963117 | 2.286548 | 4.508823 | 0.001264 | 0.001253 | 0.001260 |
| variant_index_type_129.test | RUN | 0.975384 | 2.295066 | 4.456425 | 0.001427 | 0.001228 | 0.001239 |
| variant_index_type_254.test | RUN | 3.588510 | 8.760975 | 14.025857 | 0.001207 | 0.001209 | 0.001362 |
| variant_index_type_255.test | RUN | 3.461352 | 8.717059 | 14.047460 | 0.001383 | 0.001418 | 0.001488 |
| variant_index_type_256.test | RUN | 3.645385 | 8.921218 | 14.225761 | 0.001275 | 0.001319 | 0.001240 |
| variant_index_type_257.test | RUN | 3.501190 | 9.101276 | 14.354303 | 0.001356 | 0.001400 | 0.001257 |
| variant_default_construct_cx_2.test | COMPILE | 0.107109 | 0.245834 | 0.334542 | — | — | — |
| variant_default_construct_cx_3.test | COMPILE | 0.098242 | 0.246917 | 0.280544 | — | — | — |
| variant_default_construct_cx_4.test | COMPILE | 0.104771 | 0.236684 | 0.278284 | — | — | — |
| variant_default_construct_cx_5.test | COMPILE | 0.107002 | 0.258903 | 0.290343 | — | — | — |
| variant_value_construct_cx_2.test | COMPILE | 0.104171 | 0.263138 | 0.289383 | — | — | — |
| variant_value_construct_cx_3.test | COMPILE | 0.102853 | 0.262646 | 0.272321 | — | — | — |
| variant_value_construct_cx_4.test | COMPILE | 0.066023 | 0.192458 | 0.275983 | — | — | — |
| variant_issue_55.test | RUN | 0.136548 | 0.370956 | 0.440559 | 0.000953 | 0.000924 | 0.000976 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.989× | 0.991×, 0.987× | 1.007× |
| clang | 0.998× | 0.994×, 1.002× | 1.000× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345325).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| is_output_streamable.test | 10 | 0.001019s | 0.001027s | 0.001028s | byte equality observed |
| quick.test | 10 | 0.001002s | 0.000435s | 0.001005s | byte equality observed |
| variant_alternative.test | 10 | 0.001014s | 0.001041s | 0.001023s | byte equality observed |
| variant_convert_construct.test | 10 | 0.001018s | 0.001011s | 0.001013s | byte equality observed |
| variant_convert_construct_throw.test | 10 | 0.001046s | 0.001050s | 0.001054s | byte equality observed |
| variant_copy_assign.test | 10 | 0.001024s | 0.001021s | 0.001030s | byte equality observed |
| variant_copy_assign_throw.test | 10 | 0.001045s | 0.001061s | 0.001047s | byte equality observed |
| variant_copy_construct.test | 10 | 0.001028s | 0.001037s | 0.001018s | byte equality observed |
| variant_copy_construct_throw.test | 10 | 0.001035s | 0.001032s | 0.001055s | byte equality observed |
| variant_default_construct.test | 10 | 0.001016s | 0.001020s | 0.001029s | byte equality observed |
| variant_derived_construct2.test | 10 | 0.001023s | 0.001001s | 0.001013s | byte equality observed |
| variant_destroy.test | 10 | 0.001009s | 0.001021s | 0.001012s | byte equality observed |
| variant_emplace_index.test | 10 | 0.001022s | 0.001027s | 0.001016s | byte equality observed |
| variant_emplace_type.test | 10 | 0.001016s | 0.000994s | 0.001006s | byte equality observed |
| variant_eq_ne.test | 10 | 0.001028s | 0.001012s | 0.001024s | byte equality observed |
| variant_get_by_index.test | 10 | 0.001077s | 0.001100s | 0.001078s | byte equality observed |
| variant_get_by_index_nx.test | 10 | 0.001037s | 0.001017s | 0.001004s | byte equality observed |
| variant_get_by_type.test | 10 | 0.001065s | 0.001093s | 0.001087s | byte equality observed |
| variant_get_by_type_nx.test | 10 | 0.001043s | 0.001023s | 0.001037s | byte equality observed |
| variant_hash.test | 10 | 0.001008s | 0.001013s | 0.001014s | byte equality observed |
| variant_holds_alternative.test | 10 | 0.001025s | 0.000995s | 0.001004s | byte equality observed |
| variant_in_place_index_construct.test | 10 | 0.001029s | 0.001011s | 0.001029s | byte equality observed |
| variant_in_place_type_construct.test | 10 | 0.001038s | 0.001021s | 0.001026s | byte equality observed |
| variant_index_type_126.test | 10 | 0.001043s | 0.000996s | 0.001007s | byte equality observed |
| variant_index_type_127.test | 10 | 0.001021s | 0.001002s | 0.001022s | byte equality observed |
| variant_index_type_128.test | 10 | 0.001026s | 0.001011s | 0.000994s | byte equality observed |
| variant_index_type_129.test | 10 | 0.001017s | 0.001019s | 0.001029s | byte equality observed |
| variant_index_type_254.test | 10 | 0.001022s | 0.001026s | 0.001032s | byte equality observed |
| variant_index_type_255.test | 10 | 0.001008s | 0.001005s | 0.001032s | byte equality observed |
| variant_index_type_256.test | 10 | 0.001017s | 0.001011s | 0.001006s | byte equality observed |
| variant_index_type_257.test | 10 | 0.001017s | 0.000995s | 0.001029s | byte equality observed |
| variant_issue_55.test | 10 | 0.001030s | 0.001023s | 0.000999s | byte equality observed |
| variant_json_value_from.test | 10 | 0.001058s | 0.001038s | 0.001050s | byte equality observed |
| variant_json_value_to.test | 10 | 0.001036s | 0.001066s | 0.001049s | byte equality observed |
| variant_lt_gt.test | 10 | 0.001004s | 0.001002s | 0.001005s | byte equality observed |
| variant_many_types.test | 10 | 0.001032s | 0.001029s | 0.001033s | byte equality observed |
| variant_move_assign.test | 10 | 0.001009s | 0.001040s | 0.001008s | byte equality observed |
| variant_move_assign_throw.test | 10 | 0.001038s | 0.001036s | 0.001036s | byte equality observed |
| variant_move_construct.test | 10 | 0.001025s | 0.001029s | 0.001032s | byte equality observed |
| variant_move_construct_throw.test | 10 | 0.001036s | 0.001038s | 0.001044s | byte equality observed |
| variant_ostream_insert.test | 10 | 0.001045s | 0.001042s | 0.001029s | byte equality observed |
| variant_size.test | 10 | 0.001002s | 0.001009s | 0.001003s | byte equality observed |
| variant_special.test | 10 | 0.001092s | 0.001066s | 0.001073s | byte equality observed |
| variant_subset.test | 10 | 0.001048s | 0.001051s | 0.001037s | byte equality observed |
| variant_subset_nx.test | 10 | 0.001022s | 0.001033s | 0.001027s | byte equality observed |
| variant_swap.test | 10 | 0.001013s | 0.001035s | 0.001018s | byte equality observed |
| variant_trivial.test | 10 | 0.001076s | 0.001053s | 0.001055s | byte equality observed |
| variant_value_assign.test | 10 | 0.001031s | 0.001014s | 0.001026s | byte equality observed |
| variant_value_construct.test | 10 | 0.001018s | 0.001005s | 0.001041s | byte equality observed |
| variant_valueless.test | 10 | 0.001049s | 0.001082s | 0.001055s | byte equality observed |
| variant_visit.test | 10 | 0.001028s | 0.001039s | 0.001014s | byte equality observed |
| variant_visit_by_index.test | 10 | 0.001022s | 0.000999s | 0.001017s | byte equality observed |
| variant_visit_derived.test | 10 | 0.001012s | 0.000987s | 0.001035s | byte equality observed |
| variant_visit_r.test | 10 | 0.001034s | 0.001018s | 0.001011s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345325).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
