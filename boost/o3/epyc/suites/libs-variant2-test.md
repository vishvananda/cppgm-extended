# libs/variant2/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| quick.test | RUN | 0.174433 | 0.384000 | 0.583617 | 0.002046 | 0.000949 | 0.002116 |
| variant_size.test | RUN | 0.271683 | 0.847014 | 1.078139 | 0.002320 | 0.002387 | 0.002315 |
| variant_alternative.test | RUN | 0.373916 | 2.649859 | 1.775031 | 0.001996 | 0.002080 | 0.001774 |
| variant_holds_alternative.test | RUN | 0.263026 | 0.645119 | 1.091058 | 0.002180 | 0.002032 | 0.001919 |
| variant_holds_alternative_cx.test | COMPILE | 0.167299 | 0.375279 | 0.647041 | — | — | — |
| variant_get_by_index.test | RUN | 0.425122 | 2.858371 | 2.207212 | 0.002058 | 0.002367 | 0.002628 |
| variant_get_by_index_cx.test | COMPILE | 0.170694 | 0.409066 | 0.661489 | — | — | — |
| variant_get_by_type.test | RUN | 0.426276 | 3.043696 | 2.341968 | 0.002784 | 0.002396 | 0.002518 |
| variant_get_by_type_cx.test | COMPILE | 0.167731 | 0.351915 | 0.572959 | — | — | — |
| variant_default_construct.test | RUN | 0.426244 | 2.244153 | 1.891953 | 0.002052 | 0.001933 | 0.002366 |
| variant_default_construct_cx.test | COMPILE | 0.197237 | 0.386175 | 0.647463 | — | — | — |
| variant_copy_construct.test | RUN | 0.618567 | 3.766685 | 3.392046 | 0.002179 | 0.002080 | 0.002127 |
| variant_copy_construct_cx.test | COMPILE | 0.191236 | 0.413507 | 0.741048 | — | — | — |
| variant_move_construct.test | RUN | 0.584244 | 3.271884 | 3.112050 | 0.002192 | 0.002808 | 0.002206 |
| variant_move_construct_cx.test | COMPILE | 0.164146 | 0.354652 | 0.573157 | — | — | — |
| variant_value_construct.test | RUN | 0.321485 | 0.874849 | 1.398440 | 0.001905 | 0.002162 | 0.002335 |
| variant_value_construct_cx.test | COMPILE | 0.176774 | 0.389628 | 0.644215 | — | — | — |
| variant_in_place_index_construct.test | RUN | 0.321868 | 0.838518 | 1.324069 | 0.002458 | 0.001856 | 0.002226 |
| variant_in_place_index_construct_cx.test | COMPILE | 0.177210 | 0.406169 | 0.648813 | — | — | — |
| variant_in_place_type_construct.test | RUN | 0.329981 | 0.851501 | 1.397614 | 0.002138 | 0.002129 | 0.002834 |
| variant_in_place_type_construct_cx.test | COMPILE | 0.178138 | 0.408109 | 0.689279 | — | — | — |
| variant_copy_assign.test | RUN | 0.442520 | 2.624343 | 2.092513 | 0.002206 | 0.002040 | 0.001934 |
| variant_copy_assign_cx.test | COMPILE | 0.162495 | 0.343010 | 0.559264 | — | — | — |
| variant_move_assign.test | RUN | 0.418828 | 2.628508 | 2.067714 | 0.001989 | 0.002146 | 0.002436 |
| variant_move_assign_cx.test | COMPILE | 0.151364 | 0.347762 | 0.582458 | — | — | — |
| variant_value_assign.test | RUN | 0.327619 | 0.964340 | 1.530882 | 0.002026 | 0.002060 | 0.002368 |
| variant_value_assign_cx.test | COMPILE | 0.163750 | 0.335264 | 0.519332 | — | — | — |
| variant_emplace_index.test | RUN | 0.379324 | 1.115964 | 1.679993 | 0.001911 | 0.002008 | 0.002231 |
| variant_emplace_index_cx.test | COMPILE | 0.159361 | 0.360494 | 0.583411 | — | — | — |
| variant_emplace_type.test | RUN | 0.346760 | 0.937819 | 1.491239 | 0.002059 | 0.002173 | 0.002128 |
| variant_emplace_type_cx.test | COMPILE | 0.163869 | 0.356614 | 0.589529 | — | — | — |
| variant_swap.test | RUN | 0.338957 | 1.150694 | 1.589839 | 0.002325 | 0.002206 | 0.001873 |
| variant_eq_ne.test | RUN | 0.275795 | 0.712259 | 1.130013 | 0.002153 | 0.002158 | 0.002157 |
| variant_eq_ne_cx.test | COMPILE | 0.158314 | 0.352675 | 0.569870 | — | — | — |
| variant_lt_gt.test | RUN | 0.296470 | 0.817956 | 1.218550 | 0.002192 | 0.002421 | 0.002322 |
| variant_lt_gt_cx.test | COMPILE | 0.157432 | 0.359742 | 0.515689 | — | — | — |
| variant_destroy.test | RUN | 0.295038 | 0.809824 | 1.223861 | 0.002052 | 0.002208 | 0.002406 |
| variant_visit.test | RUN | 1.701162 | 6.218495 | 18.625360 | 0.002650 | 0.001902 | 0.002206 |
| variant_convert_construct.test | RUN | 0.363462 | 1.071448 | 1.734011 | 0.003205 | 0.002278 | 0.002320 |
| variant_subset.test | RUN | 0.385479 | 1.138975 | 1.756022 | 0.002171 | 0.002517 | 0.002278 |
| variant_valueless.test | RUN | 0.307959 | 0.777207 | 1.277973 | 0.002265 | 0.002212 | 0.002352 |
| variant_copy_construct_throw.test | RUN | 0.233801 | 0.538086 | 0.835402 | 0.002238 | 0.002279 | 0.002248 |
| variant_move_construct_throw.test | RUN | 0.223342 | 0.558715 | 0.883631 | 0.002206 | 0.002188 | 0.002124 |
| variant_convert_construct_throw.test | RUN | 0.223648 | 0.550871 | 0.895501 | 0.002178 | 0.002176 | 0.001812 |
| variant_copy_assign_throw.test | RUN | 0.217246 | 0.520319 | 0.871931 | 0.002074 | 0.002259 | 0.002396 |
| variant_move_assign_throw.test | RUN | 0.210724 | 0.555212 | 0.903372 | 0.002120 | 0.002073 | 0.002572 |
| variant_get_by_index_nx.test | RUN | 0.519871 | 2.474347 | 2.164000 | 0.002194 | 0.002178 | 0.001850 |
| variant_get_by_index_cx_nx.test | COMPILE | 0.187145 | 0.418758 | 0.666263 | — | — | — |
| variant_get_by_type_nx.test | RUN | 0.492550 | 2.756841 | 2.252519 | 0.002109 | 0.002137 | 0.002199 |
| variant_get_by_type_cx_nx.test | COMPILE | 0.155191 | 0.365804 | 0.582191 | — | — | — |
| variant_subset_nx.test | RUN | 0.400524 | 1.096002 | 1.891615 | 0.002082 | 0.002182 | 0.003168 |
| variant_hash.test | RUN | 0.389466 | 1.268484 | 1.646234 | 0.002295 | 0.002232 | 0.002496 |
| variant_trivial.test | RUN | 2.216017 | 6.674707 | 12.697451 | 0.002272 | 0.002284 | 0.002259 |
| variant_special.test | RUN | 2.169619 | 7.345435 | 13.208291 | 0.002053 | 0.002165 | 0.002393 |
| variant_visit_derived.test | RUN | 0.259700 | 0.647461 | 1.156223 | 0.002207 | 0.002222 | 0.002358 |
| variant_many_types.test | RUN | 2.212027 | 10.150530 | 16.642951 | 0.002092 | 0.002306 | 0.002294 |
| variant_visit_r.test | RUN | 0.891721 | 3.129578 | 6.209245 | 0.002558 | 0.002384 | 0.002480 |
| variant_derived_construct.test | COMPILE | 0.169133 | 0.318484 | 0.547499 | — | — | — |
| variant_visit_by_index.test | RUN | 0.372939 | 0.837315 | 1.594907 | 0.001895 | 0.001932 | 0.002176 |
| variant_ostream_insert.test | RUN | 0.261973 | 0.718779 | 0.953620 | 0.002104 | 0.002260 | 0.002374 |
| is_output_streamable.test | RUN | 0.317771 | 1.413448 | 1.472465 | 0.001760 | 0.002221 | 0.002194 |
| variant_json_value_from.test | RUN | 0.490858 | 1.282890 | 1.890499 | 0.001627 | 0.002071 | 0.002294 |
| variant_json_value_to.test | RUN | 0.601053 | 1.594988 | 2.553777 | 0.002711 | 0.002282 | 0.002286 |
| variant_uses_double_storage.test | COMPILE | 0.175219 | 0.402671 | 0.634140 | — | — | — |
| variant_derived_construct2.test | RUN | 0.222018 | 0.603680 | 0.947541 | 0.002006 | 0.002132 | 0.002177 |
| variant_visit_cx.test | COMPILE | 0.164519 | 0.342367 | 0.497188 | — | — | — |
| variant_visit_cx_2.test | COMPILE | 0.155981 | 0.355245 | 0.553071 | — | — | — |
| variant_visit_r_cx.test | COMPILE | 0.157773 | 0.354187 | 0.543415 | — | — | — |
| variant_index_type_126.test | RUN | 1.410559 | 3.461755 | 7.909597 | 0.002126 | 0.002190 | 0.002204 |
| variant_index_type_127.test | RUN | 1.418831 | 3.366942 | 7.962240 | 0.002172 | 0.001691 | 0.001936 |
| variant_index_type_128.test | RUN | 1.489633 | 3.447415 | 7.968498 | 0.002158 | 0.002262 | 0.001996 |
| variant_index_type_129.test | RUN | 1.475191 | 3.477638 | 8.068540 | 0.002025 | 0.002275 | 0.002186 |
| variant_index_type_254.test | RUN | 4.966951 | 12.194775 | 23.021026 | 0.002068 | 0.002092 | 0.002166 |
| variant_index_type_255.test | RUN | 5.156051 | 12.084423 | 23.146732 | 0.001889 | 0.002082 | 0.002130 |
| variant_index_type_256.test | RUN | 5.153149 | 12.288066 | 23.308438 | 0.001884 | 0.001872 | 0.002322 |
| variant_index_type_257.test | RUN | 5.111198 | 12.481993 | 23.641080 | 0.001651 | 0.001475 | 0.002200 |
| variant_default_construct_cx_2.test | COMPILE | 0.149728 | 0.349463 | 0.529477 | — | — | — |
| variant_default_construct_cx_3.test | COMPILE | 0.135977 | 0.390981 | 0.478334 | — | — | — |
| variant_default_construct_cx_4.test | COMPILE | 0.159475 | 0.341087 | 0.595403 | — | — | — |
| variant_default_construct_cx_5.test | COMPILE | 0.158002 | 0.375576 | 0.566439 | — | — | — |
| variant_value_construct_cx_2.test | COMPILE | 0.156513 | 0.349919 | 0.572349 | — | — | — |
| variant_value_construct_cx_3.test | COMPILE | 0.152080 | 0.339223 | 0.512268 | — | — | — |
| variant_value_construct_cx_4.test | COMPILE | 0.153758 | 0.280037 | 0.403707 | — | — | — |
| variant_issue_55.test | RUN | 0.216571 | 0.643389 | 0.758208 | 0.001658 | 0.001460 | 0.001698 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.985× | 0.987×, 0.983× | 0.992× |
| clang | 1.002× | 0.996×, 1.007× | 0.998× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898266).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| is_output_streamable.test | 9 | 0.001511s | 0.001534s | 0.001529s | byte equality observed |
| quick.test | 10 | 0.001491s | 0.000687s | 0.001507s | byte equality observed |
| variant_alternative.test | 10 | 0.001526s | 0.001570s | 0.001523s | byte equality observed |
| variant_convert_construct.test | 9 | 0.001542s | 0.001556s | 0.001540s | byte equality observed |
| variant_convert_construct_throw.test | 10 | 0.001588s | 0.001591s | 0.001559s | byte equality observed |
| variant_copy_assign.test | 9 | 0.001536s | 0.001535s | 0.001510s | byte equality observed |
| variant_copy_assign_throw.test | 10 | 0.001586s | 0.001588s | 0.001541s | byte equality observed |
| variant_copy_construct.test | 10 | 0.001570s | 0.001564s | 0.001529s | byte equality observed |
| variant_copy_construct_throw.test | 10 | 0.001568s | 0.001552s | 0.001594s | byte equality observed |
| variant_default_construct.test | 10 | 0.001535s | 0.001539s | 0.001538s | byte equality observed |
| variant_derived_construct2.test | 8 | 0.001542s | 0.001549s | 0.001542s | byte equality observed |
| variant_destroy.test | 10 | 0.001516s | 0.001517s | 0.001537s | byte equality observed |
| variant_emplace_index.test | 10 | 0.001555s | 0.001555s | 0.001549s | byte equality observed |
| variant_emplace_type.test | 10 | 0.001536s | 0.001530s | 0.001528s | byte equality observed |
| variant_eq_ne.test | 10 | 0.001537s | 0.001558s | 0.001508s | byte equality observed |
| variant_get_by_index.test | 10 | 0.001642s | 0.001668s | 0.001671s | byte equality observed |
| variant_get_by_index_nx.test | 10 | 0.001529s | 0.001528s | 0.001499s | byte equality observed |
| variant_get_by_type.test | 10 | 0.001605s | 0.001659s | 0.001590s | byte equality observed |
| variant_get_by_type_nx.test | 10 | 0.001568s | 0.001575s | 0.001507s | byte equality observed |
| variant_hash.test | 10 | 0.001533s | 0.001566s | 0.001531s | byte equality observed |
| variant_holds_alternative.test | 10 | 0.001527s | 0.001530s | 0.001508s | byte equality observed |
| variant_in_place_index_construct.test | 10 | 0.001558s | 0.001543s | 0.001558s | byte equality observed |
| variant_in_place_type_construct.test | 10 | 0.001554s | 0.001570s | 0.001545s | byte equality observed |
| variant_index_type_126.test | 8 | 0.001558s | 0.001541s | 0.001545s | byte equality observed |
| variant_index_type_127.test | 10 | 0.001547s | 0.001532s | 0.001523s | byte equality observed |
| variant_index_type_128.test | 10 | 0.001544s | 0.001487s | 0.001544s | byte equality observed |
| variant_index_type_129.test | 10 | 0.001537s | 0.001502s | 0.001567s | byte equality observed |
| variant_index_type_254.test | 10 | 0.001550s | 0.001550s | 0.001552s | byte equality observed |
| variant_index_type_255.test | 10 | 0.001534s | 0.001552s | 0.001535s | byte equality observed |
| variant_index_type_256.test | 10 | 0.001502s | 0.001530s | 0.001506s | byte equality observed |
| variant_index_type_257.test | 10 | 0.001538s | 0.001533s | 0.001525s | byte equality observed |
| variant_issue_55.test | 10 | 0.001583s | 0.001569s | 0.001521s | byte equality observed |
| variant_json_value_from.test | 10 | 0.001609s | 0.001579s | 0.001586s | byte equality observed |
| variant_json_value_to.test | 10 | 0.001597s | 0.001603s | 0.001577s | byte equality observed |
| variant_lt_gt.test | 10 | 0.001534s | 0.001545s | 0.001545s | byte equality observed |
| variant_many_types.test | 10 | 0.001560s | 0.001551s | 0.001553s | byte equality observed |
| variant_move_assign.test | 10 | 0.001556s | 0.001574s | 0.001528s | byte equality observed |
| variant_move_assign_throw.test | 10 | 0.001575s | 0.001622s | 0.001569s | byte equality observed |
| variant_move_construct.test | 10 | 0.001551s | 0.001585s | 0.001561s | byte equality observed |
| variant_move_construct_throw.test | 10 | 0.001568s | 0.001584s | 0.001592s | byte equality observed |
| variant_ostream_insert.test | 10 | 0.001573s | 0.001564s | 0.001558s | byte equality observed |
| variant_size.test | 10 | 0.001547s | 0.001558s | 0.001550s | byte equality observed |
| variant_special.test | 9 | 0.001632s | 0.001621s | 0.001595s | byte equality observed |
| variant_subset.test | 10 | 0.001581s | 0.001569s | 0.001604s | byte equality observed |
| variant_subset_nx.test | 10 | 0.001533s | 0.001547s | 0.001549s | byte equality observed |
| variant_swap.test | 10 | 0.001585s | 0.001526s | 0.001535s | byte equality observed |
| variant_trivial.test | 10 | 0.001566s | 0.001602s | 0.001576s | byte equality observed |
| variant_value_assign.test | 10 | 0.001571s | 0.001589s | 0.001549s | byte equality observed |
| variant_value_construct.test | 10 | 0.001550s | 0.001559s | 0.001523s | byte equality observed |
| variant_valueless.test | 10 | 0.001599s | 0.001557s | 0.001593s | byte equality observed |
| variant_visit.test | 10 | 0.001567s | 0.001552s | 0.001564s | byte equality observed |
| variant_visit_by_index.test | 10 | 0.001540s | 0.001523s | 0.001516s | byte equality observed |
| variant_visit_derived.test | 10 | 0.001552s | 0.001480s | 0.001504s | byte equality observed |
| variant_visit_r.test | 10 | 0.001562s | 0.001514s | 0.001548s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898266).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
