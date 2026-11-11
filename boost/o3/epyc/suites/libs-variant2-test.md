# libs/variant2/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| quick.test | RUN | 0.123850 | 0.220297 | 0.282137 | 0.001820 | 0.000847 | 0.001789 |
| variant_size.test | RUN | 0.199564 | 0.537077 | 0.542946 | 0.001970 | 0.001987 | 0.001810 |
| variant_alternative.test | RUN | 0.257905 | 1.726972 | 0.922789 | 0.001966 | 0.001861 | 0.001548 |
| variant_holds_alternative.test | RUN | 0.197316 | 0.424672 | 0.515740 | 0.001688 | 0.002067 | 0.001860 |
| variant_holds_alternative_cx.test | COMPILE | 0.133117 | 0.263344 | 0.289518 | — | — | — |
| variant_get_by_index.test | RUN | 0.282930 | 1.867181 | 1.128046 | 0.002150 | 0.002095 | 0.001980 |
| variant_get_by_index_cx.test | COMPILE | 0.136775 | 0.247354 | 0.339275 | — | — | — |
| variant_get_by_type.test | RUN | 0.305882 | 2.026481 | 1.231437 | 0.001678 | 0.002106 | 0.001572 |
| variant_get_by_type_cx.test | COMPILE | 0.128734 | 0.216855 | 0.276828 | — | — | — |
| variant_default_construct.test | RUN | 0.282745 | 1.525243 | 0.985416 | 0.001780 | 0.002104 | 0.001731 |
| variant_default_construct_cx.test | COMPILE | 0.124949 | 0.228839 | 0.316282 | — | — | — |
| variant_copy_construct.test | RUN | 0.418328 | 2.560084 | 1.898386 | 0.001878 | 0.001996 | 0.001868 |
| variant_copy_construct_cx.test | COMPILE | 0.140079 | 0.262351 | 0.401192 | — | — | — |
| variant_move_construct.test | RUN | 0.399936 | 2.116129 | 1.705922 | 0.001760 | 0.001779 | 0.001935 |
| variant_move_construct_cx.test | COMPILE | 0.114605 | 0.209700 | 0.282046 | — | — | — |
| variant_value_construct.test | RUN | 0.221639 | 0.554531 | 0.704532 | 0.001731 | 0.001898 | 0.001874 |
| variant_value_construct_cx.test | COMPILE | 0.129331 | 0.253789 | 0.313124 | — | — | — |
| variant_in_place_index_construct.test | RUN | 0.228303 | 0.541991 | 0.710904 | 0.001874 | 0.001726 | 0.001922 |
| variant_in_place_index_construct_cx.test | COMPILE | 0.128648 | 0.262206 | 0.340361 | — | — | — |
| variant_in_place_type_construct.test | RUN | 0.230565 | 0.561791 | 0.700715 | 0.001880 | 0.001994 | 0.001682 |
| variant_in_place_type_construct_cx.test | COMPILE | 0.132868 | 0.266546 | 0.329224 | — | — | — |
| variant_copy_assign.test | RUN | 0.288237 | 1.657464 | 1.090565 | 0.001519 | 0.001918 | 0.001510 |
| variant_copy_assign_cx.test | COMPILE | 0.118125 | 0.207635 | 0.250975 | — | — | — |
| variant_move_assign.test | RUN | 0.291028 | 1.646720 | 1.074907 | 0.001758 | 0.002364 | 0.001763 |
| variant_move_assign_cx.test | COMPILE | 0.112818 | 0.219560 | 0.249930 | — | — | — |
| variant_value_assign.test | RUN | 0.235882 | 0.599576 | 0.704165 | 0.001870 | 0.001807 | 0.001663 |
| variant_value_assign_cx.test | COMPILE | 0.115338 | 0.197969 | 0.255096 | — | — | — |
| variant_emplace_index.test | RUN | 0.255283 | 0.701964 | 0.780453 | 0.001626 | 0.001720 | 0.001781 |
| variant_emplace_index_cx.test | COMPILE | 0.120337 | 0.211927 | 0.243113 | — | — | — |
| variant_emplace_type.test | RUN | 0.243354 | 0.600436 | 0.691730 | 0.001428 | 0.001888 | 0.001861 |
| variant_emplace_type_cx.test | COMPILE | 0.115259 | 0.223430 | 0.244695 | — | — | — |
| variant_swap.test | RUN | 0.248548 | 0.749761 | 0.760770 | 0.001758 | 0.001903 | 0.001784 |
| variant_eq_ne.test | RUN | 0.210331 | 0.448318 | 0.538023 | 0.001768 | 0.001820 | 0.001811 |
| variant_eq_ne_cx.test | COMPILE | 0.117953 | 0.222359 | 0.259695 | — | — | — |
| variant_lt_gt.test | RUN | 0.232053 | 0.509864 | 0.564645 | 0.001845 | 0.001834 | 0.001746 |
| variant_lt_gt_cx.test | COMPILE | 0.116880 | 0.201937 | 0.228368 | — | — | — |
| variant_destroy.test | RUN | 0.235482 | 0.492811 | 0.558988 | 0.001772 | 0.001919 | 0.001784 |
| variant_visit.test | RUN | 1.146906 | 3.990987 | 9.937475 | 0.001657 | 0.001825 | 0.001818 |
| variant_convert_construct.test | RUN | 0.282763 | 0.667833 | 0.804585 | 0.002122 | 0.001974 | 0.001734 |
| variant_subset.test | RUN | 0.268022 | 0.746319 | 0.845717 | 0.001879 | 0.001935 | 0.001394 |
| variant_valueless.test | RUN | 0.199106 | 0.520848 | 0.585566 | 0.002008 | 0.002080 | 0.001539 |
| variant_copy_construct_throw.test | RUN | 0.165381 | 0.336781 | 0.390087 | 0.002112 | 0.002068 | 0.001776 |
| variant_move_construct_throw.test | RUN | 0.164470 | 0.342421 | 0.406414 | 0.001891 | 0.002023 | 0.001608 |
| variant_convert_construct_throw.test | RUN | 0.172181 | 0.383933 | 0.437564 | 0.001972 | 0.001760 | 0.001232 |
| variant_copy_assign_throw.test | RUN | 0.163964 | 0.330369 | 0.412472 | 0.002230 | 0.001839 | 0.001836 |
| variant_move_assign_throw.test | RUN | 0.159364 | 0.339463 | 0.444381 | 0.002100 | 0.002302 | 0.001922 |
| variant_get_by_index_nx.test | RUN | 0.320572 | 1.600870 | 1.107842 | 0.002020 | 0.001888 | 0.001862 |
| variant_get_by_index_cx_nx.test | COMPILE | 0.128494 | 0.255924 | 0.291433 | — | — | — |
| variant_get_by_type_nx.test | RUN | 0.320808 | 1.788031 | 1.118117 | 0.001978 | 0.001622 | 0.001770 |
| variant_get_by_type_cx_nx.test | COMPILE | 0.113168 | 0.203640 | 0.246148 | — | — | — |
| variant_subset_nx.test | RUN | 0.284638 | 0.713190 | 0.877881 | 0.001876 | 0.002038 | 0.001982 |
| variant_hash.test | RUN | 0.282693 | 0.805932 | 0.797815 | 0.002280 | 0.001737 | 0.002036 |
| variant_trivial.test | RUN | 1.507927 | 4.536444 | 6.165014 | 0.002280 | 0.002136 | 0.001800 |
| variant_special.test | RUN | 1.467659 | 4.670811 | 6.485706 | 0.001914 | 0.002136 | 0.002014 |
| variant_visit_derived.test | RUN | 0.188330 | 0.413347 | 0.582274 | 0.001956 | 0.001800 | 0.001738 |
| variant_many_types.test | RUN | 1.482189 | 6.415283 | 8.576118 | 0.001840 | 0.002018 | 0.002150 |
| variant_visit_r.test | RUN | 0.575298 | 1.956707 | 3.271661 | 0.002157 | 0.001780 | 0.001792 |
| variant_derived_construct.test | COMPILE | 0.119722 | 0.202756 | 0.283944 | — | — | — |
| variant_visit_by_index.test | RUN | 0.252476 | 0.533639 | 0.776131 | 0.001762 | 0.001945 | 0.001827 |
| variant_ostream_insert.test | RUN | 0.186255 | 0.435499 | 0.518110 | 0.002054 | 0.001859 | 0.001724 |
| is_output_streamable.test | RUN | 0.219496 | 0.902487 | 0.709422 | 0.001501 | 0.001794 | 0.001968 |
| variant_json_value_from.test | RUN | 0.357693 | 0.758992 | 0.893350 | 0.002025 | 0.001948 | 0.001991 |
| variant_json_value_to.test | RUN | 0.433766 | 1.006155 | 1.172927 | 0.001820 | 0.001977 | 0.001836 |
| variant_uses_double_storage.test | COMPILE | 0.124946 | 0.215697 | 0.298386 | — | — | — |
| variant_derived_construct2.test | RUN | 0.170919 | 0.358986 | 0.454812 | 0.002170 | 0.002044 | 0.001747 |
| variant_visit_cx.test | COMPILE | 0.117893 | 0.223006 | 0.245726 | — | — | — |
| variant_visit_cx_2.test | COMPILE | 0.114459 | 0.230168 | 0.235282 | — | — | — |
| variant_visit_r_cx.test | COMPILE | 0.116213 | 0.222490 | 0.251923 | — | — | — |
| variant_index_type_126.test | RUN | 0.978222 | 2.106846 | 3.811827 | 0.001820 | 0.001917 | 0.001985 |
| variant_index_type_127.test | RUN | 1.041535 | 2.081932 | 3.840094 | 0.002026 | 0.001759 | 0.001766 |
| variant_index_type_128.test | RUN | 1.056538 | 2.140177 | 3.869705 | 0.001650 | 0.001726 | 0.001825 |
| variant_index_type_129.test | RUN | 0.991235 | 2.110447 | 3.869694 | 0.001888 | 0.001842 | 0.001749 |
| variant_index_type_254.test | RUN | 3.417454 | 7.776814 | 12.086963 | 0.002134 | 0.001770 | 0.001762 |
| variant_index_type_255.test | RUN | 3.578946 | 7.768832 | 12.095049 | 0.001765 | 0.001485 | 0.001739 |
| variant_index_type_256.test | RUN | 3.449233 | 7.963170 | 12.284118 | 0.002058 | 0.001801 | 0.001582 |
| variant_index_type_257.test | RUN | 3.602399 | 8.025642 | 12.421938 | 0.001602 | 0.001969 | 0.001766 |
| variant_default_construct_cx_2.test | COMPILE | 0.136663 | 0.201607 | 0.237872 | — | — | — |
| variant_default_construct_cx_3.test | COMPILE | 0.114258 | 0.200079 | 0.241948 | — | — | — |
| variant_default_construct_cx_4.test | COMPILE | 0.115291 | 0.205141 | 0.312162 | — | — | — |
| variant_default_construct_cx_5.test | COMPILE | 0.112625 | 0.193793 | 0.310678 | — | — | — |
| variant_value_construct_cx_2.test | COMPILE | 0.110135 | 0.206932 | 0.331965 | — | — | — |
| variant_value_construct_cx_3.test | COMPILE | 0.109710 | 0.214170 | 0.282158 | — | — | — |
| variant_value_construct_cx_4.test | COMPILE | 0.095132 | 0.215266 | 0.230833 | — | — | — |
| variant_issue_55.test | RUN | 0.153942 | 0.364138 | 0.403324 | 0.001487 | 0.001513 | 0.001495 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.976× | 0.986×, 0.967× | 0.985× |
| clang | 0.960× | 0.961×, 0.958× | 0.994× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| is_output_streamable.test | 10 | 0.001083s | 0.001082s | 0.001084s | byte equality observed |
| quick.test | 10 | 0.001070s | 0.000488s | 0.001030s | byte equality observed |
| variant_alternative.test | 10 | 0.001087s | 0.001065s | 0.001090s | byte equality observed |
| variant_convert_construct.test | 10 | 0.001080s | 0.001074s | 0.001066s | byte equality observed |
| variant_convert_construct_throw.test | 10 | 0.001115s | 0.001077s | 0.001148s | byte equality observed |
| variant_copy_assign.test | 10 | 0.001089s | 0.001061s | 0.001130s | byte equality observed |
| variant_copy_assign_throw.test | 10 | 0.001116s | 0.001079s | 0.001118s | byte equality observed |
| variant_copy_construct.test | 10 | 0.001109s | 0.001051s | 0.001086s | byte equality observed |
| variant_copy_construct_throw.test | 10 | 0.001108s | 0.001093s | 0.001105s | byte equality observed |
| variant_default_construct.test | 10 | 0.001083s | 0.001078s | 0.001100s | byte equality observed |
| variant_derived_construct2.test | 10 | 0.001086s | 0.001048s | 0.001104s | byte equality observed |
| variant_destroy.test | 10 | 0.001103s | 0.001068s | 0.001077s | byte equality observed |
| variant_emplace_index.test | 10 | 0.001097s | 0.001083s | 0.001096s | byte equality observed |
| variant_emplace_type.test | 10 | 0.001110s | 0.001061s | 0.001095s | byte equality observed |
| variant_eq_ne.test | 10 | 0.001084s | 0.001050s | 0.001074s | byte equality observed |
| variant_get_by_index.test | 10 | 0.001138s | 0.001159s | 0.001167s | byte equality observed |
| variant_get_by_index_nx.test | 10 | 0.001112s | 0.001074s | 0.001078s | byte equality observed |
| variant_get_by_type.test | 10 | 0.001149s | 0.001117s | 0.001149s | byte equality observed |
| variant_get_by_type_nx.test | 10 | 0.001157s | 0.001059s | 0.001123s | byte equality observed |
| variant_hash.test | 10 | 0.001087s | 0.001063s | 0.001114s | byte equality observed |
| variant_holds_alternative.test | 10 | 0.001064s | 0.001066s | 0.001095s | byte equality observed |
| variant_in_place_index_construct.test | 10 | 0.001128s | 0.001048s | 0.001093s | byte equality observed |
| variant_in_place_type_construct.test | 10 | 0.001107s | 0.001059s | 0.001093s | byte equality observed |
| variant_index_type_126.test | 10 | 0.001085s | 0.001080s | 0.001091s | byte equality observed |
| variant_index_type_127.test | 10 | 0.001094s | 0.001032s | 0.001091s | byte equality observed |
| variant_index_type_128.test | 10 | 0.001110s | 0.001062s | 0.001081s | byte equality observed |
| variant_index_type_129.test | 10 | 0.001073s | 0.001056s | 0.001086s | byte equality observed |
| variant_index_type_254.test | 10 | 0.001087s | 0.001062s | 0.001100s | byte equality observed |
| variant_index_type_255.test | 10 | 0.001095s | 0.001040s | 0.001119s | byte equality observed |
| variant_index_type_256.test | 10 | 0.001083s | 0.001043s | 0.001070s | byte equality observed |
| variant_index_type_257.test | 10 | 0.001079s | 0.001065s | 0.001097s | byte equality observed |
| variant_issue_55.test | 10 | 0.001100s | 0.001066s | 0.001101s | byte equality observed |
| variant_json_value_from.test | 10 | 0.001134s | 0.001112s | 0.001173s | byte equality observed |
| variant_json_value_to.test | 10 | 0.001123s | 0.001084s | 0.001131s | byte equality observed |
| variant_lt_gt.test | 10 | 0.001097s | 0.001064s | 0.001101s | byte equality observed |
| variant_many_types.test | 10 | 0.001161s | 0.001108s | 0.001108s | byte equality observed |
| variant_move_assign.test | 10 | 0.001099s | 0.001073s | 0.001073s | byte equality observed |
| variant_move_assign_throw.test | 10 | 0.001128s | 0.001116s | 0.001114s | byte equality observed |
| variant_move_construct.test | 10 | 0.001126s | 0.001087s | 0.001080s | byte equality observed |
| variant_move_construct_throw.test | 10 | 0.001127s | 0.001090s | 0.001101s | byte equality observed |
| variant_ostream_insert.test | 10 | 0.001123s | 0.001076s | 0.001117s | byte equality observed |
| variant_size.test | 10 | 0.001079s | 0.001070s | 0.001069s | byte equality observed |
| variant_special.test | 10 | 0.001194s | 0.001140s | 0.001137s | byte equality observed |
| variant_subset.test | 10 | 0.001125s | 0.001113s | 0.001107s | byte equality observed |
| variant_subset_nx.test | 10 | 0.001091s | 0.001069s | 0.001090s | byte equality observed |
| variant_swap.test | 10 | 0.001086s | 0.001059s | 0.001099s | byte equality observed |
| variant_trivial.test | 10 | 0.001163s | 0.001133s | 0.001116s | byte equality observed |
| variant_value_assign.test | 10 | 0.001086s | 0.001095s | 0.001075s | byte equality observed |
| variant_value_construct.test | 10 | 0.001084s | 0.001097s | 0.001095s | byte equality observed |
| variant_valueless.test | 10 | 0.001137s | 0.001103s | 0.001123s | byte equality observed |
| variant_visit.test | 10 | 0.001115s | 0.001057s | 0.001104s | byte equality observed |
| variant_visit_by_index.test | 10 | 0.001088s | 0.001054s | 0.001096s | byte equality observed |
| variant_visit_derived.test | 10 | 0.001081s | 0.001066s | 0.001117s | byte equality observed |
| variant_visit_r.test | 10 | 0.001122s | 0.001075s | 0.001131s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
