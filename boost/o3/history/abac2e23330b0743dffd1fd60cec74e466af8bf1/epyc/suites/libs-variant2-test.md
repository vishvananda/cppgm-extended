# libs/variant2/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| quick.test | RUN | 0.194912 | 0.371973 | 0.588537 | 0.002217 | 0.001074 | 0.002195 |
| variant_size.test | RUN | 0.298499 | 0.897354 | 1.100339 | 0.002156 | 0.002361 | 0.002280 |
| variant_alternative.test | RUN | 0.400452 | 2.725943 | 1.826184 | 0.002138 | 0.002319 | 0.001764 |
| variant_holds_alternative.test | RUN | 0.294272 | 0.683736 | 1.110742 | 0.001935 | 0.002287 | 0.001891 |
| variant_holds_alternative_cx.test | COMPILE | 0.198480 | 0.397340 | 0.647278 | — | — | — |
| variant_get_by_index.test | RUN | 0.431562 | 3.003108 | 2.226422 | 0.002012 | 0.002557 | 0.002332 |
| variant_get_by_index_cx.test | COMPILE | 0.207407 | 0.412402 | 0.687197 | — | — | — |
| variant_get_by_type.test | RUN | 0.476264 | 3.208127 | 2.403647 | 0.002253 | 0.002356 | 0.002224 |
| variant_get_by_type_cx.test | COMPILE | 0.174620 | 0.374462 | 0.585536 | — | — | — |
| variant_default_construct.test | RUN | 0.427786 | 2.406778 | 1.926637 | 0.001940 | 0.002105 | 0.001878 |
| variant_default_construct_cx.test | COMPILE | 0.201557 | 0.386624 | 0.649643 | — | — | — |
| variant_copy_construct.test | RUN | 0.677887 | 3.980507 | 3.378689 | 0.002172 | 0.002312 | 0.002137 |
| variant_copy_construct_cx.test | COMPILE | 0.213195 | 0.450862 | 0.758708 | — | — | — |
| variant_move_construct.test | RUN | 0.645363 | 3.486958 | 3.224072 | 0.001646 | 0.001990 | 0.002291 |
| variant_move_construct_cx.test | COMPILE | 0.192815 | 0.369922 | 0.574417 | — | — | — |
| variant_value_construct.test | RUN | 0.351938 | 0.909050 | 1.385381 | 0.001971 | 0.002284 | 0.002331 |
| variant_value_construct_cx.test | COMPILE | 0.199214 | 0.430962 | 0.698616 | — | — | — |
| variant_in_place_index_construct.test | RUN | 0.358452 | 0.933059 | 1.338476 | 0.002158 | 0.001895 | 0.002246 |
| variant_in_place_index_construct_cx.test | COMPILE | 0.199398 | 0.427809 | 0.626249 | — | — | — |
| variant_in_place_type_construct.test | RUN | 0.355580 | 0.902309 | 1.456105 | 0.002030 | 0.002132 | 0.002236 |
| variant_in_place_type_construct_cx.test | COMPILE | 0.210119 | 0.408446 | 0.607244 | — | — | — |
| variant_copy_assign.test | RUN | 0.448120 | 2.806502 | 2.146165 | 0.002006 | 0.002153 | 0.002210 |
| variant_copy_assign_cx.test | COMPILE | 0.176225 | 0.369295 | 0.536776 | — | — | — |
| variant_move_assign.test | RUN | 0.450342 | 2.728977 | 2.151073 | 0.002138 | 0.002483 | 0.001788 |
| variant_move_assign_cx.test | COMPILE | 0.175112 | 0.364488 | 0.571125 | — | — | — |
| variant_value_assign.test | RUN | 0.363288 | 0.993530 | 1.513130 | 0.002162 | 0.002212 | 0.001743 |
| variant_value_assign_cx.test | COMPILE | 0.176210 | 0.358102 | 0.571529 | — | — | — |
| variant_emplace_index.test | RUN | 0.400637 | 1.145625 | 1.681395 | 0.001782 | 0.002328 | 0.002319 |
| variant_emplace_index_cx.test | COMPILE | 0.185855 | 0.346510 | 0.519500 | — | — | — |
| variant_emplace_type.test | RUN | 0.362241 | 1.008741 | 1.478768 | 0.002226 | 0.002480 | 0.002202 |
| variant_emplace_type_cx.test | COMPILE | 0.182169 | 0.356282 | 0.570727 | — | — | — |
| variant_swap.test | RUN | 0.384345 | 1.245365 | 1.621351 | 0.002043 | 0.002322 | 0.001974 |
| variant_eq_ne.test | RUN | 0.311256 | 0.726318 | 1.138209 | 0.002211 | 0.002481 | 0.002042 |
| variant_eq_ne_cx.test | COMPILE | 0.182944 | 0.364110 | 0.560352 | — | — | — |
| variant_lt_gt.test | RUN | 0.331627 | 0.874151 | 1.232571 | 0.002182 | 0.002380 | 0.002250 |
| variant_lt_gt_cx.test | COMPILE | 0.175985 | 0.339088 | 0.556915 | — | — | — |
| variant_destroy.test | RUN | 0.325731 | 0.872435 | 1.235449 | 0.002074 | 0.002304 | 0.002284 |
| variant_visit.test | RUN | 1.828634 | 6.492253 | 18.993993 | 0.002197 | 0.002478 | 0.002268 |
| variant_convert_construct.test | RUN | 0.396331 | 1.116899 | 1.746559 | 0.002230 | 0.002284 | 0.002270 |
| variant_subset.test | RUN | 0.403648 | 1.207267 | 1.826014 | 0.001905 | 0.002430 | 0.002377 |
| variant_valueless.test | RUN | 0.307624 | 0.808800 | 1.235178 | 0.002266 | 0.002224 | 0.002290 |
| variant_copy_construct_throw.test | RUN | 0.265292 | 0.567943 | 0.813093 | 0.002282 | 0.002420 | 0.002270 |
| variant_move_construct_throw.test | RUN | 0.253321 | 0.569776 | 0.897303 | 0.002126 | 0.002216 | 0.002315 |
| variant_convert_construct_throw.test | RUN | 0.255102 | 0.569822 | 0.903483 | 0.002168 | 0.002172 | 0.002338 |
| variant_copy_assign_throw.test | RUN | 0.250998 | 0.541920 | 0.876399 | 0.002168 | 0.002386 | 0.002210 |
| variant_move_assign_throw.test | RUN | 0.248332 | 0.534483 | 0.908326 | 0.002212 | 0.002379 | 0.002265 |
| variant_get_by_index_nx.test | RUN | 0.529055 | 2.670658 | 2.181848 | 0.002246 | 0.002085 | 0.002236 |
| variant_get_by_index_cx_nx.test | COMPILE | 0.191387 | 0.386144 | 0.682339 | — | — | — |
| variant_get_by_type_nx.test | RUN | 0.527069 | 2.865244 | 2.325860 | 0.001886 | 0.002436 | 0.002272 |
| variant_get_by_type_cx_nx.test | COMPILE | 0.181825 | 0.367831 | 0.561411 | — | — | — |
| variant_subset_nx.test | RUN | 0.438429 | 1.178176 | 1.841151 | 0.002298 | 0.002242 | 0.001978 |
| variant_hash.test | RUN | 0.420550 | 1.321808 | 1.719915 | 0.002127 | 0.002384 | 0.002271 |
| variant_trivial.test | RUN | 2.333599 | 7.452735 | 12.109992 | 0.002220 | 0.002212 | 0.002067 |
| variant_special.test | RUN | 2.216795 | 7.970984 | 13.504258 | 0.002164 | 0.002606 | 0.002263 |
| variant_visit_derived.test | RUN | 0.292320 | 0.687476 | 1.264883 | 0.002206 | 0.002416 | 0.002164 |
| variant_many_types.test | RUN | 2.252876 | 10.385713 | 17.082002 | 0.001742 | 0.002192 | 0.002282 |
| variant_visit_r.test | RUN | 0.911859 | 3.175988 | 6.474833 | 0.002334 | 0.002403 | 0.001803 |
| variant_derived_construct.test | COMPILE | 0.190407 | 0.356254 | 0.541807 | — | — | — |
| variant_visit_by_index.test | RUN | 0.390262 | 0.990964 | 1.658144 | 0.001855 | 0.002373 | 0.001912 |
| variant_ostream_insert.test | RUN | 0.264343 | 0.730708 | 1.103714 | 0.001900 | 0.002108 | 0.002090 |
| is_output_streamable.test | RUN | 0.344410 | 1.490157 | 1.400626 | 0.002104 | 0.002451 | 0.002169 |
| variant_json_value_from.test | RUN | 0.533324 | 1.253172 | 2.060805 | 0.001960 | 0.002066 | 0.002378 |
| variant_json_value_to.test | RUN | 0.628490 | 1.667716 | 2.596349 | 0.002244 | 0.002316 | 0.002188 |
| variant_uses_double_storage.test | COMPILE | 0.186588 | 0.372727 | 0.652381 | — | — | — |
| variant_derived_construct2.test | RUN | 0.259021 | 0.632903 | 0.943196 | 0.001854 | 0.002258 | 0.002297 |
| variant_visit_cx.test | COMPILE | 0.175788 | 0.362186 | 0.570744 | — | — | — |
| variant_visit_cx_2.test | COMPILE | 0.181158 | 0.379913 | 0.575143 | — | — | — |
| variant_visit_r_cx.test | COMPILE | 0.171257 | 0.366266 | 0.486074 | — | — | — |
| variant_index_type_126.test | RUN | 1.463197 | 3.460449 | 7.907091 | 0.002156 | 0.002544 | 0.002128 |
| variant_index_type_127.test | RUN | 1.476005 | 3.590216 | 8.176272 | 0.002023 | 0.002312 | 0.002003 |
| variant_index_type_128.test | RUN | 1.492373 | 3.538517 | 8.009846 | 0.002172 | 0.002493 | 0.002156 |
| variant_index_type_129.test | RUN | 1.491965 | 3.597438 | 8.047026 | 0.002196 | 0.002144 | 0.002262 |
| variant_index_type_254.test | RUN | 4.930068 | 12.622768 | 22.823409 | 0.002260 | 0.002410 | 0.002196 |
| variant_index_type_255.test | RUN | 4.968657 | 12.461548 | 23.050166 | 0.001816 | 0.002286 | 0.002264 |
| variant_index_type_256.test | RUN | 5.023001 | 12.602753 | 23.840204 | 0.002134 | 0.002336 | 0.001934 |
| variant_index_type_257.test | RUN | 5.041171 | 12.920606 | 23.436227 | 0.002049 | 0.002206 | 0.002254 |
| variant_default_construct_cx_2.test | COMPILE | 0.164192 | 0.352315 | 0.602618 | — | — | — |
| variant_default_construct_cx_3.test | COMPILE | 0.174264 | 0.354020 | 0.512832 | — | — | — |
| variant_default_construct_cx_4.test | COMPILE | 0.178943 | 0.366367 | 0.524501 | — | — | — |
| variant_default_construct_cx_5.test | COMPILE | 0.185215 | 0.389547 | 0.537232 | — | — | — |
| variant_value_construct_cx_2.test | COMPILE | 0.174760 | 0.357450 | 0.552859 | — | — | — |
| variant_value_construct_cx_3.test | COMPILE | 0.156367 | 0.370765 | 0.560966 | — | — | — |
| variant_value_construct_cx_4.test | COMPILE | 0.159196 | 0.278078 | 0.504132 | — | — | — |
| variant_issue_55.test | RUN | 0.240291 | 0.580593 | 0.750315 | 0.001583 | 0.001720 | 0.001631 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.990× | 0.990×, 0.990× | 0.993× |
| clang | 0.987× | 0.981×, 0.994× | 0.998× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| is_output_streamable.test | 10 | 0.001527s | 0.001560s | 0.001555s | byte equality observed |
| quick.test | 10 | 0.001518s | 0.000690s | 0.001522s | byte equality observed |
| variant_alternative.test | 10 | 0.001531s | 0.001581s | 0.001594s | byte equality observed |
| variant_convert_construct.test | 10 | 0.001570s | 0.001562s | 0.001579s | byte equality observed |
| variant_convert_construct_throw.test | 10 | 0.001623s | 0.001620s | 0.001612s | byte equality observed |
| variant_copy_assign.test | 10 | 0.001575s | 0.001568s | 0.001566s | byte equality observed |
| variant_copy_assign_throw.test | 10 | 0.001625s | 0.001593s | 0.001576s | byte equality observed |
| variant_copy_construct.test | 10 | 0.001571s | 0.001588s | 0.001593s | byte equality observed |
| variant_copy_construct_throw.test | 10 | 0.001620s | 0.001639s | 0.001600s | byte equality observed |
| variant_default_construct.test | 10 | 0.001564s | 0.001574s | 0.001543s | byte equality observed |
| variant_derived_construct2.test | 10 | 0.001568s | 0.001559s | 0.001545s | byte equality observed |
| variant_destroy.test | 10 | 0.001554s | 0.001558s | 0.001522s | byte equality observed |
| variant_emplace_index.test | 10 | 0.001560s | 0.001540s | 0.001559s | byte equality observed |
| variant_emplace_type.test | 10 | 0.001564s | 0.001563s | 0.001552s | byte equality observed |
| variant_eq_ne.test | 10 | 0.001555s | 0.001530s | 0.001547s | byte equality observed |
| variant_get_by_index.test | 10 | 0.001698s | 0.001645s | 0.001641s | byte equality observed |
| variant_get_by_index_nx.test | 10 | 0.001571s | 0.001542s | 0.001524s | byte equality observed |
| variant_get_by_type.test | 10 | 0.001625s | 0.001646s | 0.001667s | byte equality observed |
| variant_get_by_type_nx.test | 10 | 0.001595s | 0.001541s | 0.001577s | byte equality observed |
| variant_hash.test | 10 | 0.001565s | 0.001569s | 0.001581s | byte equality observed |
| variant_holds_alternative.test | 10 | 0.001594s | 0.001543s | 0.001568s | byte equality observed |
| variant_in_place_index_construct.test | 10 | 0.001571s | 0.001579s | 0.001566s | byte equality observed |
| variant_in_place_type_construct.test | 10 | 0.001585s | 0.001591s | 0.001539s | byte equality observed |
| variant_index_type_126.test | 10 | 0.001568s | 0.001528s | 0.001545s | byte equality observed |
| variant_index_type_127.test | 8 | 0.001585s | 0.001569s | 0.001543s | byte equality observed |
| variant_index_type_128.test | 10 | 0.001578s | 0.001545s | 0.001552s | byte equality observed |
| variant_index_type_129.test | 10 | 0.001575s | 0.001534s | 0.001546s | byte equality observed |
| variant_index_type_254.test | 10 | 0.001566s | 0.001583s | 0.001548s | byte equality observed |
| variant_index_type_255.test | 10 | 0.001570s | 0.001531s | 0.001580s | byte equality observed |
| variant_index_type_256.test | 10 | 0.001562s | 0.001580s | 0.001544s | byte equality observed |
| variant_index_type_257.test | 10 | 0.001576s | 0.001549s | 0.001528s | byte equality observed |
| variant_issue_55.test | 10 | 0.001568s | 0.001613s | 0.001567s | byte equality observed |
| variant_json_value_from.test | 10 | 0.001632s | 0.001625s | 0.001612s | byte equality observed |
| variant_json_value_to.test | 10 | 0.001621s | 0.001656s | 0.001589s | byte equality observed |
| variant_lt_gt.test | 10 | 0.001563s | 0.001567s | 0.001549s | byte equality observed |
| variant_many_types.test | 10 | 0.001624s | 0.001559s | 0.001587s | byte equality observed |
| variant_move_assign.test | 8 | 0.001575s | 0.001600s | 0.001569s | byte equality observed |
| variant_move_assign_throw.test | 10 | 0.001615s | 0.001627s | 0.001603s | byte equality observed |
| variant_move_construct.test | 9 | 0.001583s | 0.001607s | 0.001562s | byte equality observed |
| variant_move_construct_throw.test | 10 | 0.001628s | 0.001622s | 0.001556s | byte equality observed |
| variant_ostream_insert.test | 10 | 0.001581s | 0.001633s | 0.001570s | byte equality observed |
| variant_size.test | 10 | 0.001589s | 0.001596s | 0.001583s | byte equality observed |
| variant_special.test | 10 | 0.001687s | 0.001654s | 0.001620s | byte equality observed |
| variant_subset.test | 10 | 0.001630s | 0.001624s | 0.001611s | byte equality observed |
| variant_subset_nx.test | 10 | 0.001543s | 0.001573s | 0.001549s | byte equality observed |
| variant_swap.test | 10 | 0.001578s | 0.001585s | 0.001575s | byte equality observed |
| variant_trivial.test | 10 | 0.001658s | 0.001638s | 0.001578s | byte equality observed |
| variant_value_assign.test | 10 | 0.001591s | 0.001597s | 0.001563s | byte equality observed |
| variant_value_construct.test | 10 | 0.001563s | 0.001568s | 0.001553s | byte equality observed |
| variant_valueless.test | 10 | 0.001633s | 0.001641s | 0.001608s | byte equality observed |
| variant_visit.test | 10 | 0.001620s | 0.001591s | 0.001543s | byte equality observed |
| variant_visit_by_index.test | 10 | 0.001557s | 0.001554s | 0.001575s | byte equality observed |
| variant_visit_derived.test | 10 | 0.001538s | 0.001539s | 0.001544s | byte equality observed |
| variant_visit_r.test | 10 | 0.001559s | 0.001555s | 0.001539s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
