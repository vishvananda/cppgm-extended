# libs/variant2/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| quick.test | RUN | 0.084331 | 0.181201 | 0.220365 | 0.001357 | 0.000662 | 0.001445 |
| variant_size.test | RUN | 0.138156 | 0.451526 | 0.429066 | 0.001457 | 0.001528 | 0.001529 |
| variant_alternative.test | RUN | 0.182663 | 1.453779 | 0.774494 | 0.001486 | 0.001630 | 0.001191 |
| variant_holds_alternative.test | RUN | 0.132375 | 0.331425 | 0.446713 | 0.001262 | 0.001639 | 0.001326 |
| variant_holds_alternative_cx.test | COMPILE | 0.086464 | 0.200310 | 0.241728 | — | — | — |
| variant_get_by_index.test | RUN | 0.207825 | 1.611245 | 0.951459 | 0.001291 | 0.001680 | 0.001493 |
| variant_get_by_index_cx.test | COMPILE | 0.090349 | 0.205992 | 0.251506 | — | — | — |
| variant_get_by_type.test | RUN | 0.226472 | 1.747173 | 1.019195 | 0.001614 | 0.001640 | 0.001433 |
| variant_get_by_type_cx.test | COMPILE | 0.079880 | 0.186139 | 0.217497 | — | — | — |
| variant_default_construct.test | RUN | 0.203412 | 1.278415 | 0.820456 | 0.001442 | 0.001562 | 0.001480 |
| variant_default_construct_cx.test | COMPILE | 0.090496 | 0.202005 | 0.254518 | — | — | — |
| variant_copy_construct.test | RUN | 0.324579 | 2.214647 | 1.520843 | 0.001220 | 0.001638 | 0.001530 |
| variant_copy_construct_cx.test | COMPILE | 0.097101 | 0.221242 | 0.287434 | — | — | — |
| variant_move_construct.test | RUN | 0.300058 | 1.827379 | 1.329756 | 0.001546 | 0.001577 | 0.001560 |
| variant_move_construct_cx.test | COMPILE | 0.078702 | 0.187582 | 0.212416 | — | — | — |
| variant_value_construct.test | RUN | 0.160128 | 0.480362 | 0.557212 | 0.001384 | 0.001562 | 0.001519 |
| variant_value_construct_cx.test | COMPILE | 0.091827 | 0.212465 | 0.270488 | — | — | — |
| variant_in_place_index_construct.test | RUN | 0.166251 | 0.462140 | 0.563061 | 0.001377 | 0.001380 | 0.001338 |
| variant_in_place_index_construct_cx.test | COMPILE | 0.090139 | 0.202976 | 0.258393 | — | — | — |
| variant_in_place_type_construct.test | RUN | 0.167281 | 0.448741 | 0.564849 | 0.001378 | 0.001622 | 0.001501 |
| variant_in_place_type_construct_cx.test | COMPILE | 0.094516 | 0.206913 | 0.267954 | — | — | — |
| variant_copy_assign.test | RUN | 0.217000 | 1.412906 | 0.889914 | 0.001370 | 0.001341 | 0.001498 |
| variant_copy_assign_cx.test | COMPILE | 0.079424 | 0.180425 | 0.217826 | — | — | — |
| variant_move_assign.test | RUN | 0.217511 | 1.394261 | 0.887772 | 0.001302 | 0.001620 | 0.001228 |
| variant_move_assign_cx.test | COMPILE | 0.081347 | 0.177006 | 0.213062 | — | — | — |
| variant_value_assign.test | RUN | 0.166555 | 0.495422 | 0.627238 | 0.001459 | 0.001563 | 0.001435 |
| variant_value_assign_cx.test | COMPILE | 0.081800 | 0.171394 | 0.217773 | — | — | — |
| variant_emplace_index.test | RUN | 0.186193 | 0.570817 | 0.663101 | 0.001455 | 0.001685 | 0.001510 |
| variant_emplace_index_cx.test | COMPILE | 0.081641 | 0.171775 | 0.214107 | — | — | — |
| variant_emplace_type.test | RUN | 0.175663 | 0.494182 | 0.607047 | 0.001541 | 0.001618 | 0.001448 |
| variant_emplace_type_cx.test | COMPILE | 0.078941 | 0.180940 | 0.217274 | — | — | — |
| variant_swap.test | RUN | 0.178324 | 0.631939 | 0.641060 | 0.001470 | 0.001374 | 0.001305 |
| variant_eq_ne.test | RUN | 0.143758 | 0.383230 | 0.455889 | 0.001307 | 0.001462 | 0.001454 |
| variant_eq_ne_cx.test | COMPILE | 0.078295 | 0.185556 | 0.221553 | — | — | — |
| variant_lt_gt.test | RUN | 0.147806 | 0.431319 | 0.495115 | 0.001561 | 0.001602 | 0.001532 |
| variant_lt_gt_cx.test | COMPILE | 0.080042 | 0.172465 | 0.221178 | — | — | — |
| variant_destroy.test | RUN | 0.151452 | 0.428798 | 0.511108 | 0.001539 | 0.001614 | 0.001461 |
| variant_visit.test | RUN | 0.838677 | 3.203099 | 8.419879 | 0.001466 | 0.001592 | 0.001564 |
| variant_convert_construct.test | RUN | 0.192196 | 0.561352 | 0.699147 | 0.001544 | 0.001582 | 0.001462 |
| variant_subset.test | RUN | 0.196546 | 0.597297 | 0.743926 | 0.001504 | 0.001668 | 0.001352 |
| variant_valueless.test | RUN | 0.144737 | 0.384627 | 0.488096 | 0.001494 | 0.001633 | 0.001441 |
| variant_copy_construct_throw.test | RUN | 0.114766 | 0.286998 | 0.350846 | 0.001344 | 0.001665 | 0.001464 |
| variant_move_construct_throw.test | RUN | 0.117239 | 0.289715 | 0.352544 | 0.001545 | 0.001690 | 0.001506 |
| variant_convert_construct_throw.test | RUN | 0.118002 | 0.295212 | 0.362803 | 0.001374 | 0.001510 | 0.001487 |
| variant_copy_assign_throw.test | RUN | 0.112604 | 0.272951 | 0.350635 | 0.001527 | 0.001632 | 0.001473 |
| variant_move_assign_throw.test | RUN | 0.113974 | 0.272669 | 0.333927 | 0.001578 | 0.001662 | 0.001519 |
| variant_get_by_index_nx.test | RUN | 0.259462 | 1.333368 | 0.919322 | 0.001465 | 0.001456 | 0.001492 |
| variant_get_by_index_cx_nx.test | COMPILE | 0.087019 | 0.205421 | 0.238691 | — | — | — |
| variant_get_by_type_nx.test | RUN | 0.269082 | 1.487626 | 0.976455 | 0.001504 | 0.001516 | 0.001464 |
| variant_get_by_type_cx_nx.test | COMPILE | 0.080445 | 0.176667 | 0.211054 | — | — | — |
| variant_subset_nx.test | RUN | 0.204179 | 0.575024 | 0.754652 | 0.001486 | 0.001526 | 0.001565 |
| variant_hash.test | RUN | 0.201311 | 0.663316 | 0.637062 | 0.001604 | 0.001626 | 0.001499 |
| variant_trivial.test | RUN | 1.148007 | 3.781017 | 5.025825 | 0.001445 | 0.001596 | 0.001487 |
| variant_special.test | RUN | 1.144488 | 4.185788 | 5.264837 | 0.001678 | 0.001713 | 0.001552 |
| variant_visit_derived.test | RUN | 0.137940 | 0.348667 | 0.514045 | 0.001541 | 0.001626 | 0.001406 |
| variant_many_types.test | RUN | 1.150578 | 5.176549 | 7.193896 | 0.001292 | 0.001247 | 0.001519 |
| variant_visit_r.test | RUN | 0.432077 | 1.547732 | 2.830481 | 0.001562 | 0.001557 | 0.001386 |
| variant_derived_construct.test | COMPILE | 0.087537 | 0.184073 | 0.226755 | — | — | — |
| variant_visit_by_index.test | RUN | 0.181845 | 0.460736 | 0.671918 | 0.001486 | 0.001606 | 0.001467 |
| variant_ostream_insert.test | RUN | 0.140403 | 0.392017 | 0.395484 | 0.001373 | 0.001552 | 0.001341 |
| is_output_streamable.test | RUN | 0.160527 | 0.790929 | 0.625038 | 0.001394 | 0.001529 | 0.001552 |
| variant_json_value_from.test | RUN | 0.252589 | 0.601361 | 0.776962 | 0.001601 | 0.001665 | 0.001628 |
| variant_json_value_to.test | RUN | 0.306013 | 0.846539 | 1.042049 | 0.001431 | 0.001608 | 0.001456 |
| variant_uses_double_storage.test | COMPILE | 0.085234 | 0.194548 | 0.255769 | — | — | — |
| variant_derived_construct2.test | RUN | 0.112847 | 0.300111 | 0.387216 | 0.001255 | 0.001532 | 0.001279 |
| variant_visit_cx.test | COMPILE | 0.082700 | 0.182866 | 0.206850 | — | — | — |
| variant_visit_cx_2.test | COMPILE | 0.079994 | 0.183643 | 0.202025 | — | — | — |
| variant_visit_r_cx.test | COMPILE | 0.078570 | 0.168030 | 0.211552 | — | — | — |
| variant_index_type_126.test | RUN | 0.752587 | 1.691526 | 3.055240 | 0.001406 | 0.001554 | 0.001421 |
| variant_index_type_127.test | RUN | 0.754234 | 1.684866 | 3.124644 | 0.001382 | 0.001518 | 0.001445 |
| variant_index_type_128.test | RUN | 0.801324 | 1.722690 | 3.174201 | 0.001520 | 0.001351 | 0.001534 |
| variant_index_type_129.test | RUN | 0.788721 | 1.733491 | 3.187046 | 0.001393 | 0.001553 | 0.001305 |
| variant_index_type_254.test | RUN | 2.781163 | 6.483542 | 10.085069 | 0.001468 | 0.001559 | 0.001546 |
| variant_index_type_255.test | RUN | 2.776114 | 6.331306 | 10.244414 | 0.001567 | 0.001536 | 0.001629 |
| variant_index_type_256.test | RUN | 2.828111 | 6.621531 | 10.284080 | 0.001504 | 0.001504 | 0.001281 |
| variant_index_type_257.test | RUN | 2.851841 | 6.605598 | 10.289149 | 0.001320 | 0.001365 | 0.001350 |
| variant_default_construct_cx_2.test | COMPILE | 0.079684 | 0.184121 | 0.222332 | — | — | — |
| variant_default_construct_cx_3.test | COMPILE | 0.079299 | 0.183682 | 0.196808 | — | — | — |
| variant_default_construct_cx_4.test | COMPILE | 0.080593 | 0.174196 | 0.213211 | — | — | — |
| variant_default_construct_cx_5.test | COMPILE | 0.080283 | 0.175558 | 0.208849 | — | — | — |
| variant_value_construct_cx_2.test | COMPILE | 0.077946 | 0.170311 | 0.213720 | — | — | — |
| variant_value_construct_cx_3.test | COMPILE | 0.077672 | 0.169243 | 0.208816 | — | — | — |
| variant_value_construct_cx_4.test | COMPILE | 0.073815 | 0.159369 | 0.191098 | — | — | — |
| variant_issue_55.test | RUN | 0.107357 | 0.320012 | 0.304460 | 0.001068 | 0.001105 | 0.001023 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.987× | 1.003×, 0.972× | 1.001× |
| clang | 0.992× | 0.990×, 0.993× | 1.003× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409407).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| is_output_streamable.test | 10 | 0.000841s | 0.000873s | 0.000871s | byte equality observed |
| quick.test | 10 | 0.000829s | 0.000393s | 0.000840s | byte equality observed |
| variant_alternative.test | 10 | 0.000852s | 0.000854s | 0.000854s | byte equality observed |
| variant_convert_construct.test | 10 | 0.000869s | 0.000857s | 0.000852s | byte equality observed |
| variant_convert_construct_throw.test | 10 | 0.000872s | 0.000883s | 0.000870s | byte equality observed |
| variant_copy_assign.test | 10 | 0.000877s | 0.000858s | 0.000853s | byte equality observed |
| variant_copy_assign_throw.test | 10 | 0.000875s | 0.000879s | 0.000865s | byte equality observed |
| variant_copy_construct.test | 10 | 0.000856s | 0.000872s | 0.000857s | byte equality observed |
| variant_copy_construct_throw.test | 10 | 0.000873s | 0.000879s | 0.000867s | byte equality observed |
| variant_default_construct.test | 10 | 0.000861s | 0.000875s | 0.000843s | byte equality observed |
| variant_derived_construct2.test | 10 | 0.000868s | 0.000876s | 0.000844s | byte equality observed |
| variant_destroy.test | 10 | 0.000857s | 0.000839s | 0.000845s | byte equality observed |
| variant_emplace_index.test | 10 | 0.000856s | 0.000848s | 0.000838s | byte equality observed |
| variant_emplace_type.test | 10 | 0.000854s | 0.000847s | 0.000875s | byte equality observed |
| variant_eq_ne.test | 10 | 0.000839s | 0.000861s | 0.000842s | byte equality observed |
| variant_get_by_index.test | 10 | 0.000896s | 0.000912s | 0.000894s | byte equality observed |
| variant_get_by_index_nx.test | 10 | 0.000857s | 0.000859s | 0.000843s | byte equality observed |
| variant_get_by_type.test | 10 | 0.000917s | 0.000888s | 0.000883s | byte equality observed |
| variant_get_by_type_nx.test | 10 | 0.000858s | 0.000885s | 0.000842s | byte equality observed |
| variant_hash.test | 10 | 0.000855s | 0.000862s | 0.000839s | byte equality observed |
| variant_holds_alternative.test | 10 | 0.000854s | 0.000891s | 0.000843s | byte equality observed |
| variant_in_place_index_construct.test | 10 | 0.000869s | 0.000880s | 0.000846s | byte equality observed |
| variant_in_place_type_construct.test | 10 | 0.000860s | 0.000867s | 0.000850s | byte equality observed |
| variant_index_type_126.test | 10 | 0.000853s | 0.000846s | 0.000840s | byte equality observed |
| variant_index_type_127.test | 10 | 0.000870s | 0.000850s | 0.000852s | byte equality observed |
| variant_index_type_128.test | 10 | 0.000860s | 0.000845s | 0.000876s | byte equality observed |
| variant_index_type_129.test | 10 | 0.000843s | 0.000849s | 0.000842s | byte equality observed |
| variant_index_type_254.test | 10 | 0.000857s | 0.000854s | 0.000842s | byte equality observed |
| variant_index_type_255.test | 10 | 0.000857s | 0.000865s | 0.000849s | byte equality observed |
| variant_index_type_256.test | 10 | 0.000869s | 0.000850s | 0.000851s | byte equality observed |
| variant_index_type_257.test | 10 | 0.000846s | 0.000859s | 0.000842s | byte equality observed |
| variant_issue_55.test | 10 | 0.000872s | 0.000849s | 0.000849s | byte equality observed |
| variant_json_value_from.test | 10 | 0.000892s | 0.000911s | 0.000866s | byte equality observed |
| variant_json_value_to.test | 10 | 0.000878s | 0.000890s | 0.000875s | byte equality observed |
| variant_lt_gt.test | 10 | 0.000847s | 0.000866s | 0.000833s | byte equality observed |
| variant_many_types.test | 10 | 0.000874s | 0.000896s | 0.000884s | byte equality observed |
| variant_move_assign.test | 10 | 0.000864s | 0.000858s | 0.000848s | byte equality observed |
| variant_move_assign_throw.test | 10 | 0.000897s | 0.000868s | 0.000869s | byte equality observed |
| variant_move_construct.test | 10 | 0.000847s | 0.000855s | 0.000846s | byte equality observed |
| variant_move_construct_throw.test | 10 | 0.000883s | 0.000894s | 0.000868s | byte equality observed |
| variant_ostream_insert.test | 10 | 0.000879s | 0.000880s | 0.000870s | byte equality observed |
| variant_size.test | 10 | 0.000868s | 0.000896s | 0.000864s | byte equality observed |
| variant_special.test | 10 | 0.000932s | 0.000935s | 0.000896s | byte equality observed |
| variant_subset.test | 10 | 0.000889s | 0.000877s | 0.000886s | byte equality observed |
| variant_subset_nx.test | 10 | 0.000855s | 0.000854s | 0.000844s | byte equality observed |
| variant_swap.test | 10 | 0.000859s | 0.000856s | 0.000860s | byte equality observed |
| variant_trivial.test | 10 | 0.000901s | 0.000916s | 0.000880s | byte equality observed |
| variant_value_assign.test | 10 | 0.000853s | 0.000869s | 0.000829s | byte equality observed |
| variant_value_construct.test | 10 | 0.000852s | 0.000853s | 0.000847s | byte equality observed |
| variant_valueless.test | 10 | 0.000885s | 0.000881s | 0.000860s | byte equality observed |
| variant_visit.test | 10 | 0.000877s | 0.000873s | 0.000850s | byte equality observed |
| variant_visit_by_index.test | 10 | 0.000856s | 0.000855s | 0.000850s | byte equality observed |
| variant_visit_derived.test | 10 | 0.000853s | 0.000870s | 0.000834s | byte equality observed |
| variant_visit_r.test | 10 | 0.000855s | 0.000843s | 0.000834s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409407).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
