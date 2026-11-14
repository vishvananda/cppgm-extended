# libs/variant2/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| quick.test | RUN | 0.084040 | 0.176147 | 0.234780 | 0.001370 | 0.000703 | 0.001438 |
| variant_size.test | RUN | 0.151453 | 0.454573 | 0.440874 | 0.001952 | 0.001624 | 0.001488 |
| variant_alternative.test | RUN | 0.197903 | 1.449386 | 0.778144 | 0.001525 | 0.001656 | 0.001276 |
| variant_holds_alternative.test | RUN | 0.135405 | 0.348670 | 0.453661 | 0.001389 | 0.001556 | 0.001364 |
| variant_holds_alternative_cx.test | COMPILE | 0.090376 | 0.194088 | 0.245508 | — | — | — |
| variant_get_by_index.test | RUN | 0.226294 | 1.568850 | 0.960913 | 0.001537 | 0.001602 | 0.001550 |
| variant_get_by_index_cx.test | COMPILE | 0.094733 | 0.200049 | 0.254398 | — | — | — |
| variant_get_by_type.test | RUN | 0.237292 | 1.684775 | 1.025562 | 0.001615 | 0.001242 | 0.001406 |
| variant_get_by_type_cx.test | COMPILE | 0.082448 | 0.181190 | 0.224076 | — | — | — |
| variant_default_construct.test | RUN | 0.210346 | 1.224787 | 0.839329 | 0.001331 | 0.001499 | 0.001292 |
| variant_default_construct_cx.test | COMPILE | 0.092388 | 0.195555 | 0.261614 | — | — | — |
| variant_copy_construct.test | RUN | 0.324630 | 2.116629 | 1.584304 | 0.001540 | 0.001570 | 0.001540 |
| variant_copy_construct_cx.test | COMPILE | 0.099621 | 0.214620 | 0.296595 | — | — | — |
| variant_move_construct.test | RUN | 0.302177 | 1.752854 | 1.425453 | 0.001337 | 0.001398 | 0.001556 |
| variant_move_construct_cx.test | COMPILE | 0.082204 | 0.178796 | 0.216317 | — | — | — |
| variant_value_construct.test | RUN | 0.165286 | 0.460367 | 0.628623 | 0.001087 | 0.001546 | 0.001511 |
| variant_value_construct_cx.test | COMPILE | 0.094650 | 0.203520 | 0.281242 | — | — | — |
| variant_in_place_index_construct.test | RUN | 0.172835 | 0.441741 | 0.574339 | 0.001221 | 0.001466 | 0.001258 |
| variant_in_place_index_construct_cx.test | COMPILE | 0.092978 | 0.200709 | 0.260723 | — | — | — |
| variant_in_place_type_construct.test | RUN | 0.166668 | 0.451553 | 0.569185 | 0.001546 | 0.001468 | 0.001471 |
| variant_in_place_type_construct_cx.test | COMPILE | 0.093188 | 0.208207 | 0.266426 | — | — | — |
| variant_copy_assign.test | RUN | 0.223236 | 1.413160 | 0.892627 | 0.001260 | 0.001634 | 0.001347 |
| variant_copy_assign_cx.test | COMPILE | 0.081628 | 0.182342 | 0.210450 | — | — | — |
| variant_move_assign.test | RUN | 0.223255 | 1.411683 | 0.905505 | 0.002778 | 0.001632 | 0.001214 |
| variant_move_assign_cx.test | COMPILE | 0.084170 | 0.179851 | 0.222921 | — | — | — |
| variant_value_assign.test | RUN | 0.176397 | 0.493893 | 0.627410 | 0.003361 | 0.001603 | 0.001345 |
| variant_value_assign_cx.test | COMPILE | 0.086431 | 0.173839 | 0.229668 | — | — | — |
| variant_emplace_index.test | RUN | 0.342059 | 0.576370 | 0.670863 | 0.001560 | 0.003033 | 0.001500 |
| variant_emplace_index_cx.test | COMPILE | 0.145294 | 0.177773 | 0.218850 | — | — | — |
| variant_emplace_type.test | RUN | 0.190563 | 0.497396 | 0.578922 | 0.001604 | 0.001487 | 0.001490 |
| variant_emplace_type_cx.test | COMPILE | 0.084834 | 0.183542 | 0.213991 | — | — | — |
| variant_swap.test | RUN | 0.183780 | 0.622131 | 0.672252 | 0.001518 | 0.001415 | 0.001448 |
| variant_eq_ne.test | RUN | 0.155781 | 0.369662 | 0.481954 | 0.001293 | 0.001540 | 0.001493 |
| variant_eq_ne_cx.test | COMPILE | 0.090227 | 0.184794 | 0.207117 | — | — | — |
| variant_lt_gt.test | RUN | 0.159285 | 0.424875 | 0.497856 | 0.001475 | 0.001529 | 0.001457 |
| variant_lt_gt_cx.test | COMPILE | 0.084781 | 0.174578 | 0.213912 | — | — | — |
| variant_destroy.test | RUN | 0.156081 | 0.429765 | 0.517736 | 0.001650 | 0.001608 | 0.001548 |
| variant_visit.test | RUN | 0.881305 | 3.251861 | 8.598263 | 0.001294 | 0.001593 | 0.001327 |
| variant_convert_construct.test | RUN | 0.201305 | 0.555481 | 0.704380 | 0.001660 | 0.001637 | 0.001445 |
| variant_subset.test | RUN | 0.206718 | 0.602926 | 0.755594 | 0.001695 | 0.001632 | 0.001349 |
| variant_valueless.test | RUN | 0.150082 | 0.393209 | 0.496521 | 0.001387 | 0.001620 | 0.001456 |
| variant_copy_construct_throw.test | RUN | 0.118782 | 0.291493 | 0.345292 | 0.001810 | 0.001668 | 0.001538 |
| variant_move_construct_throw.test | RUN | 0.125449 | 0.293189 | 0.339747 | 0.001677 | 0.001530 | 0.001470 |
| variant_convert_construct_throw.test | RUN | 0.141484 | 0.295184 | 0.387100 | 0.001465 | 0.001666 | 0.001573 |
| variant_copy_assign_throw.test | RUN | 0.122207 | 0.277408 | 0.342237 | 0.001683 | 0.001642 | 0.001541 |
| variant_move_assign_throw.test | RUN | 0.119770 | 0.277169 | 0.364131 | 0.001615 | 0.001525 | 0.001483 |
| variant_get_by_index_nx.test | RUN | 0.277567 | 1.339558 | 0.908290 | 0.001520 | 0.001572 | 0.001467 |
| variant_get_by_index_cx_nx.test | COMPILE | 0.093001 | 0.193511 | 0.261788 | — | — | — |
| variant_get_by_type_nx.test | RUN | 0.275400 | 1.504892 | 1.000202 | 0.001486 | 0.001313 | 0.001503 |
| variant_get_by_type_cx_nx.test | COMPILE | 0.086070 | 0.183442 | 0.213431 | — | — | — |
| variant_subset_nx.test | RUN | 0.206413 | 0.603068 | 0.756677 | 0.001821 | 0.001528 | 0.001590 |
| variant_hash.test | RUN | 0.208991 | 0.693033 | 0.695278 | 0.001604 | 0.001656 | 0.001492 |
| variant_trivial.test | RUN | 1.136620 | 3.804054 | 5.126104 | 0.001472 | 0.001738 | 0.001499 |
| variant_special.test | RUN | 1.103634 | 4.222657 | 5.335080 | 0.001550 | 0.001682 | 0.001510 |
| variant_visit_derived.test | RUN | 0.136960 | 0.353824 | 0.515279 | 0.001646 | 0.001654 | 0.001570 |
| variant_many_types.test | RUN | 1.164807 | 5.201370 | 7.524742 | 0.001418 | 0.001605 | 0.001280 |
| variant_visit_r.test | RUN | 0.451108 | 1.577230 | 2.867941 | 0.001525 | 0.001466 | 0.001672 |
| variant_derived_construct.test | COMPILE | 0.086229 | 0.185968 | 0.233812 | — | — | — |
| variant_visit_by_index.test | RUN | 0.195197 | 0.459328 | 0.685581 | 0.001338 | 0.001624 | 0.001497 |
| variant_ostream_insert.test | RUN | 0.135699 | 0.399375 | 0.425355 | 0.001557 | 0.001529 | 0.001469 |
| is_output_streamable.test | RUN | 0.173413 | 0.764677 | 0.619765 | 0.001308 | 0.001467 | 0.001590 |
| variant_json_value_from.test | RUN | 0.263255 | 0.649895 | 0.824973 | 0.001629 | 0.001646 | 0.001575 |
| variant_json_value_to.test | RUN | 0.328531 | 0.850572 | 1.102831 | 0.001694 | 0.001566 | 0.001421 |
| variant_uses_double_storage.test | COMPILE | 0.088061 | 0.196639 | 0.264256 | — | — | — |
| variant_derived_construct2.test | RUN | 0.119144 | 0.306843 | 0.382510 | 0.001406 | 0.001630 | 0.001447 |
| variant_visit_cx.test | COMPILE | 0.086035 | 0.180995 | 0.213727 | — | — | — |
| variant_visit_cx_2.test | COMPILE | 0.081220 | 0.179987 | 0.210948 | — | — | — |
| variant_visit_r_cx.test | COMPILE | 0.079561 | 0.177544 | 0.212902 | — | — | — |
| variant_index_type_126.test | RUN | 0.799111 | 1.695997 | 3.172825 | 0.001413 | 0.001490 | 0.001563 |
| variant_index_type_127.test | RUN | 0.771881 | 1.709183 | 3.208052 | 0.001365 | 0.001448 | 0.001416 |
| variant_index_type_128.test | RUN | 0.784755 | 1.749279 | 3.288512 | 0.001291 | 0.001586 | 0.001387 |
| variant_index_type_129.test | RUN | 0.802758 | 1.723862 | 3.266303 | 0.001669 | 0.001485 | 0.001462 |
| variant_index_type_254.test | RUN | 2.927954 | 6.512136 | 10.314677 | 0.001541 | 0.001660 | 0.001418 |
| variant_index_type_255.test | RUN | 2.913941 | 6.548860 | 10.245096 | 0.001336 | 0.001618 | 0.001513 |
| variant_index_type_256.test | RUN | 2.895566 | 6.775428 | 10.283509 | 0.001449 | 0.001354 | 0.001489 |
| variant_index_type_257.test | RUN | 2.898214 | 6.730348 | 10.450059 | 0.001384 | 0.001260 | 0.001352 |
| variant_default_construct_cx_2.test | COMPILE | 0.081368 | 0.184685 | 0.203173 | — | — | — |
| variant_default_construct_cx_3.test | COMPILE | 0.081231 | 0.177570 | 0.194018 | — | — | — |
| variant_default_construct_cx_4.test | COMPILE | 0.080113 | 0.169792 | 0.214377 | — | — | — |
| variant_default_construct_cx_5.test | COMPILE | 0.080105 | 0.173108 | 0.218985 | — | — | — |
| variant_value_construct_cx_2.test | COMPILE | 0.080721 | 0.176216 | 0.205085 | — | — | — |
| variant_value_construct_cx_3.test | COMPILE | 0.080324 | 0.171703 | 0.169684 | — | — | — |
| variant_value_construct_cx_4.test | COMPILE | 0.074206 | 0.164435 | 0.193813 | — | — | — |
| variant_issue_55.test | RUN | 0.107928 | 0.302338 | 0.372912 | 0.001216 | 0.001078 | 0.001033 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.951× | 0.970×, 0.932× | 0.987× |
| clang | 1.005× | 0.996×, 1.015× | 0.980× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026855).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| is_output_streamable.test | 10 | 0.000862s | 0.000858s | 0.000862s | byte equality observed |
| quick.test | 10 | 0.000840s | 0.000387s | 0.000842s | byte equality observed |
| variant_alternative.test | 10 | 0.000869s | 0.000863s | 0.000863s | byte equality observed |
| variant_convert_construct.test | 10 | 0.000873s | 0.000844s | 0.000873s | byte equality observed |
| variant_convert_construct_throw.test | 10 | 0.000882s | 0.000876s | 0.000890s | byte equality observed |
| variant_copy_assign.test | 10 | 0.000868s | 0.000884s | 0.000860s | byte equality observed |
| variant_copy_assign_throw.test | 10 | 0.000880s | 0.000892s | 0.000877s | byte equality observed |
| variant_copy_construct.test | 10 | 0.000877s | 0.000875s | 0.000877s | byte equality observed |
| variant_copy_construct_throw.test | 10 | 0.000875s | 0.000882s | 0.000881s | byte equality observed |
| variant_default_construct.test | 10 | 0.000881s | 0.000860s | 0.000863s | byte equality observed |
| variant_derived_construct2.test | 10 | 0.000867s | 0.000857s | 0.000851s | byte equality observed |
| variant_destroy.test | 10 | 0.000869s | 0.000865s | 0.000871s | byte equality observed |
| variant_emplace_index.test | 10 | 0.000880s | 0.000857s | 0.000874s | byte equality observed |
| variant_emplace_type.test | 10 | 0.000878s | 0.000859s | 0.000860s | byte equality observed |
| variant_eq_ne.test | 10 | 0.000856s | 0.000842s | 0.000897s | byte equality observed |
| variant_get_by_index.test | 10 | 0.000898s | 0.000919s | 0.000905s | byte equality observed |
| variant_get_by_index_nx.test | 10 | 0.000908s | 0.000873s | 0.000863s | byte equality observed |
| variant_get_by_type.test | 10 | 0.000900s | 0.000911s | 0.000903s | byte equality observed |
| variant_get_by_type_nx.test | 10 | 0.000890s | 0.000860s | 0.000874s | byte equality observed |
| variant_hash.test | 10 | 0.000869s | 0.000886s | 0.000845s | byte equality observed |
| variant_holds_alternative.test | 10 | 0.000867s | 0.000856s | 0.000879s | byte equality observed |
| variant_in_place_index_construct.test | 10 | 0.000878s | 0.000887s | 0.000873s | byte equality observed |
| variant_in_place_type_construct.test | 10 | 0.000865s | 0.000868s | 0.000869s | byte equality observed |
| variant_index_type_126.test | 10 | 0.000869s | 0.000848s | 0.000891s | byte equality observed |
| variant_index_type_127.test | 10 | 0.000861s | 0.000859s | 0.000866s | byte equality observed |
| variant_index_type_128.test | 10 | 0.000872s | 0.000869s | 0.000866s | byte equality observed |
| variant_index_type_129.test | 10 | 0.000865s | 0.000841s | 0.000873s | byte equality observed |
| variant_index_type_254.test | 10 | 0.000857s | 0.000853s | 0.000861s | byte equality observed |
| variant_index_type_255.test | 10 | 0.000860s | 0.000850s | 0.000855s | byte equality observed |
| variant_index_type_256.test | 10 | 0.000862s | 0.000853s | 0.000847s | byte equality observed |
| variant_index_type_257.test | 10 | 0.000873s | 0.000863s | 0.000869s | byte equality observed |
| variant_issue_55.test | 10 | 0.000882s | 0.000876s | 0.000874s | byte equality observed |
| variant_json_value_from.test | 10 | 0.000892s | 0.000917s | 0.000901s | byte equality observed |
| variant_json_value_to.test | 10 | 0.000896s | 0.000884s | 0.000901s | byte equality observed |
| variant_lt_gt.test | 10 | 0.000866s | 0.000849s | 0.000883s | byte equality observed |
| variant_many_types.test | 10 | 0.000911s | 0.000893s | 0.000903s | byte equality observed |
| variant_move_assign.test | 10 | 0.000900s | 0.000880s | 0.000864s | byte equality observed |
| variant_move_assign_throw.test | 10 | 0.000881s | 0.000880s | 0.000894s | byte equality observed |
| variant_move_construct.test | 10 | 0.000879s | 0.000873s | 0.000863s | byte equality observed |
| variant_move_construct_throw.test | 10 | 0.000888s | 0.000904s | 0.000895s | byte equality observed |
| variant_ostream_insert.test | 10 | 0.000875s | 0.000871s | 0.000878s | byte equality observed |
| variant_size.test | 10 | 0.000859s | 0.000854s | 0.000851s | byte equality observed |
| variant_special.test | 10 | 0.000940s | 0.000926s | 0.000915s | byte equality observed |
| variant_subset.test | 10 | 0.000897s | 0.000882s | 0.000888s | byte equality observed |
| variant_subset_nx.test | 10 | 0.000877s | 0.000851s | 0.000872s | byte equality observed |
| variant_swap.test | 10 | 0.000881s | 0.000884s | 0.000876s | byte equality observed |
| variant_trivial.test | 10 | 0.000909s | 0.000911s | 0.000888s | byte equality observed |
| variant_value_assign.test | 10 | 0.000865s | 0.000883s | 0.000852s | byte equality observed |
| variant_value_construct.test | 10 | 0.000861s | 0.000850s | 0.000860s | byte equality observed |
| variant_valueless.test | 10 | 0.000889s | 0.000887s | 0.000885s | byte equality observed |
| variant_visit.test | 10 | 0.000869s | 0.000879s | 0.000871s | byte equality observed |
| variant_visit_by_index.test | 10 | 0.000872s | 0.000854s | 0.000847s | byte equality observed |
| variant_visit_derived.test | 10 | 0.000844s | 0.000852s | 0.000847s | byte equality observed |
| variant_visit_r.test | 10 | 0.000852s | 0.000862s | 0.000888s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026855).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
