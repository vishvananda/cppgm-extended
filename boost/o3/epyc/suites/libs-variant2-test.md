# libs/variant2/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| quick.test | RUN | 0.153226 | 0.341323 | 0.594868 | 0.001846 | 0.000858 | 0.002332 |
| variant_size.test | RUN | 0.257965 | 0.840110 | 1.101285 | 0.001965 | 0.002232 | 0.002232 |
| variant_alternative.test | RUN | 0.345297 | 2.602575 | 1.809269 | 0.001932 | 0.002211 | 0.001699 |
| variant_holds_alternative.test | RUN | 0.254043 | 0.630315 | 1.116210 | 0.002113 | 0.002086 | 0.001822 |
| variant_holds_alternative_cx.test | COMPILE | 0.165608 | 0.370647 | 0.628483 | — | — | — |
| variant_get_by_index.test | RUN | 0.393619 | 2.841814 | 2.163968 | 0.002119 | 0.002097 | 0.002210 |
| variant_get_by_index_cx.test | COMPILE | 0.171890 | 0.413776 | 0.665578 | — | — | — |
| variant_get_by_type.test | RUN | 0.418853 | 3.066656 | 2.376525 | 0.002206 | 0.002128 | 0.002020 |
| variant_get_by_type_cx.test | COMPILE | 0.157415 | 0.344179 | 0.577547 | — | — | — |
| variant_default_construct.test | RUN | 0.408582 | 2.213996 | 1.905150 | 0.002040 | 0.001940 | 0.001922 |
| variant_default_construct_cx.test | COMPILE | 0.162838 | 0.375151 | 0.662593 | — | — | — |
| variant_copy_construct.test | RUN | 0.606401 | 3.775595 | 3.367549 | 0.001918 | 0.002114 | 0.002158 |
| variant_copy_construct_cx.test | COMPILE | 0.186991 | 0.418311 | 0.759294 | — | — | — |
| variant_move_construct.test | RUN | 0.570319 | 3.292034 | 3.144786 | 0.002134 | 0.001939 | 0.002210 |
| variant_move_construct_cx.test | COMPILE | 0.156710 | 0.356200 | 0.572267 | — | — | — |
| variant_value_construct.test | RUN | 0.308266 | 0.879128 | 1.359331 | 0.001870 | 0.002080 | 0.002232 |
| variant_value_construct_cx.test | COMPILE | 0.182518 | 0.392755 | 0.663424 | — | — | — |
| variant_in_place_index_construct.test | RUN | 0.313406 | 0.846608 | 1.326773 | 0.001730 | 0.001891 | 0.002204 |
| variant_in_place_index_construct_cx.test | COMPILE | 0.175140 | 0.394935 | 0.658179 | — | — | — |
| variant_in_place_type_construct.test | RUN | 0.312545 | 0.855555 | 1.414847 | 0.001971 | 0.002016 | 0.002348 |
| variant_in_place_type_construct_cx.test | COMPILE | 0.171026 | 0.398120 | 0.655231 | — | — | — |
| variant_copy_assign.test | RUN | 0.427608 | 2.623272 | 2.177514 | 0.001870 | 0.002076 | 0.001952 |
| variant_copy_assign_cx.test | COMPILE | 0.151104 | 0.356282 | 0.565423 | — | — | — |
| variant_move_assign.test | RUN | 0.399091 | 2.613010 | 2.128291 | 0.001764 | 0.002214 | 0.002156 |
| variant_move_assign_cx.test | COMPILE | 0.157226 | 0.357091 | 0.547268 | — | — | — |
| variant_value_assign.test | RUN | 0.325269 | 0.934293 | 1.490729 | 0.002028 | 0.002066 | 0.001772 |
| variant_value_assign_cx.test | COMPILE | 0.144067 | 0.341677 | 0.557197 | — | — | — |
| variant_emplace_index.test | RUN | 0.358128 | 1.104762 | 1.667305 | 0.002042 | 0.001894 | 0.002320 |
| variant_emplace_index_cx.test | COMPILE | 0.155482 | 0.362549 | 0.574654 | — | — | — |
| variant_emplace_type.test | RUN | 0.318081 | 0.935200 | 1.492231 | 0.002020 | 0.002152 | 0.002010 |
| variant_emplace_type_cx.test | COMPILE | 0.145647 | 0.337320 | 0.557490 | — | — | — |
| variant_swap.test | RUN | 0.349288 | 1.179430 | 1.588085 | 0.001739 | 0.001966 | 0.001945 |
| variant_eq_ne.test | RUN | 0.270791 | 0.700681 | 1.129615 | 0.001848 | 0.001884 | 0.002259 |
| variant_eq_ne_cx.test | COMPILE | 0.150234 | 0.353870 | 0.563098 | — | — | — |
| variant_lt_gt.test | RUN | 0.288920 | 0.815740 | 1.175628 | 0.002281 | 0.001923 | 0.002235 |
| variant_lt_gt_cx.test | COMPILE | 0.146390 | 0.333977 | 0.562791 | — | — | — |
| variant_destroy.test | RUN | 0.292568 | 0.781989 | 1.274501 | 0.002138 | 0.002186 | 0.002302 |
| variant_visit.test | RUN | 1.661152 | 6.066813 | 18.656494 | 0.001800 | 0.001570 | 0.002194 |
| variant_convert_construct.test | RUN | 0.354391 | 1.051774 | 1.707316 | 0.001777 | 0.002192 | 0.002500 |
| variant_subset.test | RUN | 0.382235 | 1.143354 | 1.831432 | 0.001890 | 0.002038 | 0.002266 |
| variant_valueless.test | RUN | 0.266845 | 0.759031 | 1.197676 | 0.002231 | 0.002012 | 0.002162 |
| variant_copy_construct_throw.test | RUN | 0.229134 | 0.564415 | 0.844679 | 0.002234 | 0.001940 | 0.002241 |
| variant_move_construct_throw.test | RUN | 0.218504 | 0.550715 | 0.843208 | 0.001853 | 0.002064 | 0.002175 |
| variant_convert_construct_throw.test | RUN | 0.223005 | 0.603343 | 0.952864 | 0.001978 | 0.002170 | 0.002188 |
| variant_copy_assign_throw.test | RUN | 0.208256 | 0.537236 | 0.848385 | 0.001946 | 0.002260 | 0.002218 |
| variant_move_assign_throw.test | RUN | 0.214819 | 0.523986 | 0.844762 | 0.001938 | 0.002170 | 0.002336 |
| variant_get_by_index_nx.test | RUN | 0.492364 | 2.459661 | 2.195534 | 0.001920 | 0.002029 | 0.002262 |
| variant_get_by_index_cx_nx.test | COMPILE | 0.174094 | 0.426291 | 0.618292 | — | — | — |
| variant_get_by_type_nx.test | RUN | 0.498376 | 2.715301 | 2.299648 | 0.001896 | 0.001935 | 0.002262 |
| variant_get_by_type_cx_nx.test | COMPILE | 0.146683 | 0.354861 | 0.557677 | — | — | — |
| variant_subset_nx.test | RUN | 0.390816 | 1.090846 | 1.807143 | 0.001801 | 0.002202 | 0.001968 |
| variant_hash.test | RUN | 0.374504 | 1.280849 | 1.799242 | 0.002054 | 0.002185 | 0.002122 |
| variant_trivial.test | RUN | 2.159091 | 6.747595 | 12.699308 | 0.002184 | 0.002350 | 0.002150 |
| variant_special.test | RUN | 2.214890 | 7.568765 | 12.796453 | 0.002325 | 0.002358 | 0.002252 |
| variant_visit_derived.test | RUN | 0.252406 | 0.690030 | 1.261660 | 0.001986 | 0.002171 | 0.002187 |
| variant_many_types.test | RUN | 2.230415 | 9.717340 | 16.913332 | 0.002316 | 0.002030 | 0.002110 |
| variant_visit_r.test | RUN | 0.883546 | 3.023860 | 6.525889 | 0.002128 | 0.002146 | 0.002377 |
| variant_derived_construct.test | COMPILE | 0.149525 | 0.368721 | 0.573386 | — | — | — |
| variant_visit_by_index.test | RUN | 0.337691 | 0.888084 | 1.713367 | 0.001879 | 0.002174 | 0.002170 |
| variant_ostream_insert.test | RUN | 0.249089 | 0.744489 | 1.034456 | 0.002142 | 0.002086 | 0.002117 |
| is_output_streamable.test | RUN | 0.301663 | 1.550384 | 1.458985 | 0.001926 | 0.002300 | 0.002124 |
| variant_json_value_from.test | RUN | 0.477573 | 1.174053 | 1.982958 | 0.001992 | 0.002271 | 0.002282 |
| variant_json_value_to.test | RUN | 0.582711 | 1.625035 | 2.547210 | 0.001831 | 0.002330 | 0.002282 |
| variant_uses_double_storage.test | COMPILE | 0.163519 | 0.366992 | 0.626111 | — | — | — |
| variant_derived_construct2.test | RUN | 0.215704 | 0.589328 | 0.967976 | 0.002005 | 0.002099 | 0.002270 |
| variant_visit_cx.test | COMPILE | 0.154822 | 0.358651 | 0.568910 | — | — | — |
| variant_visit_cx_2.test | COMPILE | 0.145988 | 0.373369 | 0.512558 | — | — | — |
| variant_visit_r_cx.test | COMPILE | 0.151656 | 0.359263 | 0.581850 | — | — | — |
| variant_index_type_126.test | RUN | 1.392945 | 3.340122 | 7.878622 | 0.002070 | 0.002191 | 0.001916 |
| variant_index_type_127.test | RUN | 1.403579 | 3.370746 | 7.851295 | 0.001850 | 0.001923 | 0.002157 |
| variant_index_type_128.test | RUN | 1.480653 | 3.427802 | 8.040294 | 0.001992 | 0.002266 | 0.002294 |
| variant_index_type_129.test | RUN | 1.439711 | 3.391905 | 8.083589 | 0.001952 | 0.001902 | 0.002332 |
| variant_index_type_254.test | RUN | 4.801097 | 12.002254 | 22.366394 | 0.001805 | 0.002298 | 0.001826 |
| variant_index_type_255.test | RUN | 4.787286 | 12.014141 | 22.997293 | 0.001846 | 0.002218 | 0.002119 |
| variant_index_type_256.test | RUN | 4.840767 | 12.142506 | 23.050396 | 0.002053 | 0.002130 | 0.001988 |
| variant_index_type_257.test | RUN | 4.896427 | 12.480285 | 23.309743 | 0.001662 | 0.001994 | 0.001750 |
| variant_default_construct_cx_2.test | COMPILE | 0.155171 | 0.377382 | 0.568685 | — | — | — |
| variant_default_construct_cx_3.test | COMPILE | 0.132536 | 0.394309 | 0.517369 | — | — | — |
| variant_default_construct_cx_4.test | COMPILE | 0.149266 | 0.331630 | 0.572702 | — | — | — |
| variant_default_construct_cx_5.test | COMPILE | 0.147253 | 0.341848 | 0.526732 | — | — | — |
| variant_value_construct_cx_2.test | COMPILE | 0.150040 | 0.354473 | 0.476032 | — | — | — |
| variant_value_construct_cx_3.test | COMPILE | 0.141583 | 0.350761 | 0.543040 | — | — | — |
| variant_value_construct_cx_4.test | COMPILE | 0.140901 | 0.258085 | 0.371186 | — | — | — |
| variant_issue_55.test | RUN | 0.202088 | 0.530419 | 0.713498 | 0.001478 | 0.001449 | 0.001412 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.988× | 0.993×, 0.984× | 1.001× |
| clang | 0.992× | 0.995×, 0.989× | 0.995× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274616).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| is_output_streamable.test | 10 | 0.001517s | 0.001525s | 0.001518s | byte equality observed |
| quick.test | 10 | 0.001473s | 0.000643s | 0.001479s | byte equality observed |
| variant_alternative.test | 10 | 0.001495s | 0.001527s | 0.001529s | byte equality observed |
| variant_convert_construct.test | 10 | 0.001515s | 0.001483s | 0.001490s | byte equality observed |
| variant_convert_construct_throw.test | 10 | 0.001549s | 0.001510s | 0.001543s | byte equality observed |
| variant_copy_assign.test | 10 | 0.001521s | 0.001488s | 0.001475s | byte equality observed |
| variant_copy_assign_throw.test | 10 | 0.001563s | 0.001583s | 0.001576s | byte equality observed |
| variant_copy_construct.test | 10 | 0.001505s | 0.001527s | 0.001486s | byte equality observed |
| variant_copy_construct_throw.test | 10 | 0.001540s | 0.001477s | 0.001535s | byte equality observed |
| variant_default_construct.test | 10 | 0.001509s | 0.001494s | 0.001524s | byte equality observed |
| variant_derived_construct2.test | 10 | 0.001484s | 0.001538s | 0.001521s | byte equality observed |
| variant_destroy.test | 10 | 0.001500s | 0.001479s | 0.001517s | byte equality observed |
| variant_emplace_index.test | 10 | 0.001530s | 0.001501s | 0.001520s | byte equality observed |
| variant_emplace_type.test | 10 | 0.001516s | 0.001523s | 0.001521s | byte equality observed |
| variant_eq_ne.test | 10 | 0.001482s | 0.001514s | 0.001523s | byte equality observed |
| variant_get_by_index.test | 10 | 0.001581s | 0.001631s | 0.001603s | byte equality observed |
| variant_get_by_index_nx.test | 10 | 0.001520s | 0.001495s | 0.001452s | byte equality observed |
| variant_get_by_type.test | 10 | 0.001582s | 0.001622s | 0.001537s | byte equality observed |
| variant_get_by_type_nx.test | 10 | 0.001517s | 0.001493s | 0.001509s | byte equality observed |
| variant_hash.test | 10 | 0.001507s | 0.001491s | 0.001494s | byte equality observed |
| variant_holds_alternative.test | 10 | 0.001532s | 0.001501s | 0.001474s | byte equality observed |
| variant_in_place_index_construct.test | 10 | 0.001522s | 0.001504s | 0.001509s | byte equality observed |
| variant_in_place_type_construct.test | 10 | 0.001486s | 0.001488s | 0.001473s | byte equality observed |
| variant_index_type_126.test | 10 | 0.001516s | 0.001502s | 0.001494s | byte equality observed |
| variant_index_type_127.test | 10 | 0.001534s | 0.001502s | 0.001491s | byte equality observed |
| variant_index_type_128.test | 10 | 0.001501s | 0.001491s | 0.001499s | byte equality observed |
| variant_index_type_129.test | 10 | 0.001493s | 0.001525s | 0.001513s | byte equality observed |
| variant_index_type_254.test | 10 | 0.001528s | 0.001504s | 0.001507s | byte equality observed |
| variant_index_type_255.test | 10 | 0.001505s | 0.001475s | 0.001502s | byte equality observed |
| variant_index_type_256.test | 10 | 0.001502s | 0.001491s | 0.001498s | byte equality observed |
| variant_index_type_257.test | 10 | 0.001507s | 0.001503s | 0.001485s | byte equality observed |
| variant_issue_55.test | 10 | 0.001504s | 0.001542s | 0.001510s | byte equality observed |
| variant_json_value_from.test | 10 | 0.001571s | 0.001582s | 0.001535s | byte equality observed |
| variant_json_value_to.test | 10 | 0.001561s | 0.001538s | 0.001531s | byte equality observed |
| variant_lt_gt.test | 10 | 0.001502s | 0.001512s | 0.001502s | byte equality observed |
| variant_many_types.test | 10 | 0.001524s | 0.001550s | 0.001525s | byte equality observed |
| variant_move_assign.test | 10 | 0.001527s | 0.001534s | 0.001478s | byte equality observed |
| variant_move_assign_throw.test | 9 | 0.001547s | 0.001506s | 0.001550s | byte equality observed |
| variant_move_construct.test | 10 | 0.001528s | 0.001505s | 0.001493s | byte equality observed |
| variant_move_construct_throw.test | 10 | 0.001508s | 0.001510s | 0.001534s | byte equality observed |
| variant_ostream_insert.test | 10 | 0.001541s | 0.001506s | 0.001523s | byte equality observed |
| variant_size.test | 8 | 0.001504s | 0.001520s | 0.001483s | byte equality observed |
| variant_special.test | 10 | 0.001588s | 0.001590s | 0.001551s | byte equality observed |
| variant_subset.test | 10 | 0.001525s | 0.001510s | 0.001538s | byte equality observed |
| variant_subset_nx.test | 10 | 0.001532s | 0.001464s | 0.001480s | byte equality observed |
| variant_swap.test | 10 | 0.001515s | 0.001548s | 0.001500s | byte equality observed |
| variant_trivial.test | 10 | 0.001572s | 0.001549s | 0.001533s | byte equality observed |
| variant_value_assign.test | 10 | 0.001527s | 0.001481s | 0.001534s | byte equality observed |
| variant_value_construct.test | 10 | 0.001467s | 0.001513s | 0.001489s | byte equality observed |
| variant_valueless.test | 10 | 0.001556s | 0.001559s | 0.001558s | byte equality observed |
| variant_visit.test | 10 | 0.001558s | 0.001515s | 0.001528s | byte equality observed |
| variant_visit_by_index.test | 10 | 0.001465s | 0.001469s | 0.001448s | byte equality observed |
| variant_visit_derived.test | 10 | 0.001473s | 0.001501s | 0.001472s | byte equality observed |
| variant_visit_r.test | 9 | 0.001526s | 0.001450s | 0.001469s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274616).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
