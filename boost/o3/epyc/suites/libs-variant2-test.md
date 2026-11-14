# libs/variant2/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| quick.test | RUN | 0.084470 | 0.215339 | 0.267580 | 0.001561 | 0.000763 | 0.001722 |
| variant_size.test | RUN | 0.153534 | 0.497639 | 0.487677 | 0.001591 | 0.001830 | 0.001856 |
| variant_alternative.test | RUN | 0.192587 | 1.642414 | 0.885705 | 0.001687 | 0.001716 | 0.001760 |
| variant_holds_alternative.test | RUN | 0.137801 | 0.368654 | 0.518300 | 0.001470 | 0.001786 | 0.001799 |
| variant_holds_alternative_cx.test | COMPILE | 0.094985 | 0.242570 | 0.286394 | — | — | — |
| variant_get_by_index.test | RUN | 0.228749 | 1.747555 | 1.095786 | 0.001368 | 0.001813 | 0.001840 |
| variant_get_by_index_cx.test | COMPILE | 0.096867 | 0.228319 | 0.294406 | — | — | — |
| variant_get_by_type.test | RUN | 0.244252 | 1.848943 | 1.189743 | 0.001463 | 0.001415 | 0.001606 |
| variant_get_by_type_cx.test | COMPILE | 0.081800 | 0.200720 | 0.258129 | — | — | — |
| variant_default_construct.test | RUN | 0.209275 | 1.322347 | 0.953986 | 0.001394 | 0.001726 | 0.001574 |
| variant_default_construct_cx.test | COMPILE | 0.094645 | 0.207687 | 0.296055 | — | — | — |
| variant_copy_construct.test | RUN | 0.351168 | 2.235054 | 1.767427 | 0.001685 | 0.001854 | 0.001739 |
| variant_copy_construct_cx.test | COMPILE | 0.106518 | 0.245009 | 0.339373 | — | — | — |
| variant_move_construct.test | RUN | 0.332798 | 1.920654 | 1.561326 | 0.001394 | 0.001583 | 0.001769 |
| variant_move_construct_cx.test | COMPILE | 0.088049 | 0.201617 | 0.244987 | — | — | — |
| variant_value_construct.test | RUN | 0.169935 | 0.509797 | 0.646761 | 0.001503 | 0.001790 | 0.001757 |
| variant_value_construct_cx.test | COMPILE | 0.097501 | 0.235020 | 0.312858 | — | — | — |
| variant_in_place_index_construct.test | RUN | 0.182214 | 0.511206 | 0.649898 | 0.001405 | 0.001420 | 0.001680 |
| variant_in_place_index_construct_cx.test | COMPILE | 0.096630 | 0.220573 | 0.281024 | — | — | — |
| variant_in_place_type_construct.test | RUN | 0.177849 | 0.480154 | 0.634513 | 0.001381 | 0.001614 | 0.001816 |
| variant_in_place_type_construct_cx.test | COMPILE | 0.097536 | 0.221451 | 0.290815 | — | — | — |
| variant_copy_assign.test | RUN | 0.235395 | 1.565395 | 1.018984 | 0.001600 | 0.001755 | 0.001644 |
| variant_copy_assign_cx.test | COMPILE | 0.084776 | 0.190118 | 0.254018 | — | — | — |
| variant_move_assign.test | RUN | 0.233339 | 1.554980 | 0.976424 | 0.001530 | 0.001905 | 0.001425 |
| variant_move_assign_cx.test | COMPILE | 0.083856 | 0.195483 | 0.236459 | — | — | — |
| variant_value_assign.test | RUN | 0.179747 | 0.562209 | 0.676938 | 0.001505 | 0.001731 | 0.001643 |
| variant_value_assign_cx.test | COMPILE | 0.085223 | 0.196621 | 0.242485 | — | — | — |
| variant_emplace_index.test | RUN | 0.199992 | 0.634867 | 0.736143 | 0.001755 | 0.001674 | 0.001703 |
| variant_emplace_index_cx.test | COMPILE | 0.087785 | 0.194970 | 0.223994 | — | — | — |
| variant_emplace_type.test | RUN | 0.183779 | 0.555475 | 0.691231 | 0.001460 | 0.001824 | 0.001753 |
| variant_emplace_type_cx.test | COMPILE | 0.085078 | 0.201681 | 0.234307 | — | — | — |
| variant_swap.test | RUN | 0.192355 | 0.696969 | 0.730774 | 0.001515 | 0.001700 | 0.001696 |
| variant_eq_ne.test | RUN | 0.153979 | 0.413359 | 0.502965 | 0.001452 | 0.001768 | 0.001551 |
| variant_eq_ne_cx.test | COMPILE | 0.084231 | 0.201797 | 0.244609 | — | — | — |
| variant_lt_gt.test | RUN | 0.162111 | 0.484547 | 0.529224 | 0.001648 | 0.001856 | 0.001544 |
| variant_lt_gt_cx.test | COMPILE | 0.085861 | 0.196143 | 0.222070 | — | — | — |
| variant_destroy.test | RUN | 0.162640 | 0.478573 | 0.558475 | 0.001973 | 0.001803 | 0.001656 |
| variant_visit.test | RUN | 0.916679 | 3.744283 | 9.495645 | 0.001558 | 0.001793 | 0.001732 |
| variant_convert_construct.test | RUN | 0.197113 | 0.635245 | 0.804717 | 0.001532 | 0.001852 | 0.001771 |
| variant_subset.test | RUN | 0.201811 | 0.678400 | 0.832869 | 0.001621 | 0.001630 | 0.001428 |
| variant_valueless.test | RUN | 0.148334 | 0.451165 | 0.583990 | 0.001655 | 0.001765 | 0.001748 |
| variant_copy_construct_throw.test | RUN | 0.162972 | 0.322087 | 0.385548 | 0.001546 | 0.001710 | 0.001716 |
| variant_move_construct_throw.test | RUN | 0.126251 | 0.357270 | 0.425627 | 0.001484 | 0.001556 | 0.001770 |
| variant_convert_construct_throw.test | RUN | 0.122930 | 0.396749 | 0.437202 | 0.001666 | 0.001754 | 0.001514 |
| variant_copy_assign_throw.test | RUN | 0.123834 | 0.341675 | 0.380055 | 0.001366 | 0.001864 | 0.001755 |
| variant_move_assign_throw.test | RUN | 0.119147 | 0.310102 | 0.370892 | 0.001370 | 0.001897 | 0.001743 |
| variant_get_by_index_nx.test | RUN | 0.278090 | 1.560368 | 0.998091 | 0.001527 | 0.001827 | 0.001677 |
| variant_get_by_index_cx_nx.test | COMPILE | 0.091149 | 0.218567 | 0.268943 | — | — | — |
| variant_get_by_type_nx.test | RUN | 0.268551 | 1.691532 | 1.057705 | 0.001576 | 0.001674 | 0.001702 |
| variant_get_by_type_cx_nx.test | COMPILE | 0.085868 | 0.212494 | 0.234450 | — | — | — |
| variant_subset_nx.test | RUN | 0.219456 | 0.662728 | 0.830927 | 0.001521 | 0.001798 | 0.001706 |
| variant_hash.test | RUN | 0.208206 | 0.773332 | 0.737390 | 0.001480 | 0.001474 | 0.001652 |
| variant_trivial.test | RUN | 1.237887 | 4.204708 | 5.948393 | 0.001821 | 0.001892 | 0.001684 |
| variant_special.test | RUN | 1.223088 | 4.318069 | 6.087315 | 0.001556 | 0.002012 | 0.001782 |
| variant_visit_derived.test | RUN | 0.150373 | 0.391928 | 0.538705 | 0.002067 | 0.001709 | 0.001662 |
| variant_many_types.test | RUN | 1.261349 | 5.977135 | 8.391102 | 0.001572 | 0.001954 | 0.001444 |
| variant_visit_r.test | RUN | 0.504059 | 1.813608 | 3.150769 | 0.001590 | 0.001737 | 0.001673 |
| variant_derived_construct.test | COMPILE | 0.096381 | 0.192245 | 0.247385 | — | — | — |
| variant_visit_by_index.test | RUN | 0.196256 | 0.493516 | 0.740352 | 0.001350 | 0.001742 | 0.001677 |
| variant_ostream_insert.test | RUN | 0.146391 | 0.422954 | 0.501469 | 0.001393 | 0.001977 | 0.001548 |
| is_output_streamable.test | RUN | 0.180681 | 0.841344 | 0.675382 | 0.001663 | 0.001500 | 0.001679 |
| variant_json_value_from.test | RUN | 0.285284 | 0.745765 | 0.858380 | 0.001778 | 0.001634 | 0.001797 |
| variant_json_value_to.test | RUN | 0.339694 | 0.993893 | 1.172450 | 0.001684 | 0.001798 | 0.001782 |
| variant_uses_double_storage.test | COMPILE | 0.094114 | 0.206292 | 0.295325 | — | — | — |
| variant_derived_construct2.test | RUN | 0.126963 | 0.357260 | 0.427199 | 0.001458 | 0.001885 | 0.001446 |
| variant_visit_cx.test | COMPILE | 0.085496 | 0.214574 | 0.238196 | — | — | — |
| variant_visit_cx_2.test | COMPILE | 0.084047 | 0.225176 | 0.229631 | — | — | — |
| variant_visit_r_cx.test | COMPILE | 0.082095 | 0.201704 | 0.241771 | — | — | — |
| variant_index_type_126.test | RUN | 0.805441 | 1.976820 | 3.638083 | 0.001657 | 0.001651 | 0.001833 |
| variant_index_type_127.test | RUN | 0.836448 | 1.956004 | 3.636881 | 0.001574 | 0.001762 | 0.001650 |
| variant_index_type_128.test | RUN | 0.827854 | 2.007704 | 3.699819 | 0.001364 | 0.001726 | 0.001557 |
| variant_index_type_129.test | RUN | 0.842071 | 2.000681 | 3.686965 | 0.001649 | 0.001720 | 0.001722 |
| variant_index_type_254.test | RUN | 3.054284 | 7.293362 | 11.566896 | 0.001460 | 0.001931 | 0.001665 |
| variant_index_type_255.test | RUN | 3.036824 | 7.373163 | 11.708277 | 0.001473 | 0.001599 | 0.001742 |
| variant_index_type_256.test | RUN | 3.003600 | 7.472127 | 11.815989 | 0.001219 | 0.001889 | 0.001230 |
| variant_index_type_257.test | RUN | 3.010218 | 7.531413 | 11.832487 | 0.001490 | 0.001594 | 0.001427 |
| variant_default_construct_cx_2.test | COMPILE | 0.080974 | 0.212008 | 0.255050 | — | — | — |
| variant_default_construct_cx_3.test | COMPILE | 0.081748 | 0.199149 | 0.265611 | — | — | — |
| variant_default_construct_cx_4.test | COMPILE | 0.077546 | 0.199983 | 0.238122 | — | — | — |
| variant_default_construct_cx_5.test | COMPILE | 0.081358 | 0.199378 | 0.232134 | — | — | — |
| variant_value_construct_cx_2.test | COMPILE | 0.083553 | 0.198830 | 0.229937 | — | — | — |
| variant_value_construct_cx_3.test | COMPILE | 0.076921 | 0.198492 | 0.228475 | — | — | — |
| variant_value_construct_cx_4.test | COMPILE | 0.082614 | 0.149927 | 0.225035 | — | — | — |
| variant_issue_55.test | RUN | 0.130214 | 0.336692 | 0.356676 | 0.001225 | 0.001291 | 0.001339 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.056×† | 1.011×, 1.101× | 1.045× |
| clang | 0.999×† | 1.041×, 0.957× | 0.997× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051632).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| is_output_streamable.test | 10 | 0.000998s | 0.001022s | 0.000971s | byte equality observed |
| quick.test | 10 | 0.000971s | 0.000452s | 0.000921s | byte equality observed |
| variant_alternative.test | 10 | 0.000980s | 0.001024s | 0.000955s | byte equality observed |
| variant_convert_construct.test | 10 | 0.001017s | 0.001019s | 0.000966s | byte equality observed |
| variant_convert_construct_throw.test | 10 | 0.001011s | 0.001033s | 0.001000s | byte equality observed |
| variant_copy_assign.test | 10 | 0.001011s | 0.001007s | 0.000964s | byte equality observed |
| variant_copy_assign_throw.test | 10 | 0.001033s | 0.001027s | 0.000988s | byte equality observed |
| variant_copy_construct.test | 10 | 0.001006s | 0.001013s | 0.000997s | byte equality observed |
| variant_copy_construct_throw.test | 10 | 0.001017s | 0.001020s | 0.001017s | byte equality observed |
| variant_default_construct.test | 10 | 0.001010s | 0.001022s | 0.000993s | byte equality observed |
| variant_derived_construct2.test | 10 | 0.000991s | 0.000979s | 0.001015s | byte equality observed |
| variant_destroy.test | 10 | 0.000985s | 0.000985s | 0.000978s | byte equality observed |
| variant_emplace_index.test | 10 | 0.001024s | 0.000976s | 0.001029s | byte equality observed |
| variant_emplace_type.test | 10 | 0.001019s | 0.000998s | 0.001017s | byte equality observed |
| variant_eq_ne.test | 10 | 0.001015s | 0.000985s | 0.001011s | byte equality observed |
| variant_get_by_index.test | 10 | 0.001087s | 0.001074s | 0.001075s | byte equality observed |
| variant_get_by_index_nx.test | 10 | 0.001011s | 0.001003s | 0.000994s | byte equality observed |
| variant_get_by_type.test | 10 | 0.001053s | 0.001057s | 0.001037s | byte equality observed |
| variant_get_by_type_nx.test | 10 | 0.001015s | 0.000998s | 0.001005s | byte equality observed |
| variant_hash.test | 10 | 0.001003s | 0.000994s | 0.000998s | byte equality observed |
| variant_holds_alternative.test | 10 | 0.001010s | 0.001008s | 0.001012s | byte equality observed |
| variant_in_place_index_construct.test | 10 | 0.001005s | 0.001022s | 0.001005s | byte equality observed |
| variant_in_place_type_construct.test | 10 | 0.001013s | 0.001001s | 0.000988s | byte equality observed |
| variant_index_type_126.test | 10 | 0.000998s | 0.001011s | 0.001002s | byte equality observed |
| variant_index_type_127.test | 10 | 0.001015s | 0.001004s | 0.001009s | byte equality observed |
| variant_index_type_128.test | 10 | 0.000988s | 0.001010s | 0.001007s | byte equality observed |
| variant_index_type_129.test | 10 | 0.000972s | 0.001004s | 0.001013s | byte equality observed |
| variant_index_type_254.test | 10 | 0.000990s | 0.001018s | 0.000999s | byte equality observed |
| variant_index_type_255.test | 10 | 0.000969s | 0.001032s | 0.000986s | byte equality observed |
| variant_index_type_256.test | 10 | 0.000982s | 0.001026s | 0.001011s | byte equality observed |
| variant_index_type_257.test | 10 | 0.000989s | 0.000986s | 0.001012s | byte equality observed |
| variant_issue_55.test | 10 | 0.001015s | 0.001024s | 0.001001s | byte equality observed |
| variant_json_value_from.test | 10 | 0.001061s | 0.001043s | 0.001026s | byte equality observed |
| variant_json_value_to.test | 10 | 0.001028s | 0.001075s | 0.001011s | byte equality observed |
| variant_lt_gt.test | 10 | 0.001034s | 0.001021s | 0.000997s | byte equality observed |
| variant_many_types.test | 10 | 0.001062s | 0.001040s | 0.001031s | byte equality observed |
| variant_move_assign.test | 10 | 0.001021s | 0.001045s | 0.001003s | byte equality observed |
| variant_move_assign_throw.test | 10 | 0.001030s | 0.001083s | 0.001008s | byte equality observed |
| variant_move_construct.test | 10 | 0.001003s | 0.001061s | 0.000990s | byte equality observed |
| variant_move_construct_throw.test | 10 | 0.001038s | 0.001084s | 0.001047s | byte equality observed |
| variant_ostream_insert.test | 10 | 0.001021s | 0.001074s | 0.000978s | byte equality observed |
| variant_size.test | 10 | 0.001014s | 0.001060s | 0.000975s | byte equality observed |
| variant_special.test | 10 | 0.001099s | 0.001155s | 0.001075s | byte equality observed |
| variant_subset.test | 10 | 0.001033s | 0.001072s | 0.001060s | byte equality observed |
| variant_subset_nx.test | 10 | 0.001005s | 0.001035s | 0.001033s | byte equality observed |
| variant_swap.test | 10 | 0.001038s | 0.001069s | 0.001007s | byte equality observed |
| variant_trivial.test | 10 | 0.001087s | 0.001113s | 0.001060s | byte equality observed |
| variant_value_assign.test | 10 | 0.001020s | 0.001064s | 0.001000s | byte equality observed |
| variant_value_construct.test | 10 | 0.001031s | 0.001064s | 0.001034s | byte equality observed |
| variant_valueless.test | 10 | 0.001084s | 0.001085s | 0.001045s | byte equality observed |
| variant_visit.test | 10 | 0.001051s | 0.001061s | 0.001047s | byte equality observed |
| variant_visit_by_index.test | 10 | 0.001018s | 0.001019s | 0.001026s | byte equality observed |
| variant_visit_derived.test | 10 | 0.001002s | 0.001013s | 0.001039s | byte equality observed |
| variant_visit_r.test | 10 | 0.001013s | 0.001050s | 0.001037s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051632).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
