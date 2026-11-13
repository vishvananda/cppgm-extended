# libs/variant2/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| quick.test | RUN | 0.164514 | 0.343641 | 0.586759 | 0.002020 | 0.000888 | 0.001901 |
| variant_size.test | RUN | 0.256693 | 0.831601 | 1.093426 | 0.002030 | 0.002152 | 0.002035 |
| variant_alternative.test | RUN | 0.362249 | 2.601219 | 1.750834 | 0.001760 | 0.001948 | 0.001883 |
| variant_holds_alternative.test | RUN | 0.260314 | 0.626764 | 1.091012 | 0.001737 | 0.002117 | 0.001989 |
| variant_holds_alternative_cx.test | COMPILE | 0.160239 | 0.366444 | 0.627151 | — | — | — |
| variant_get_by_index.test | RUN | 0.399364 | 2.849115 | 2.130769 | 0.002247 | 0.002136 | 0.002197 |
| variant_get_by_index_cx.test | COMPILE | 0.169065 | 0.407579 | 0.645026 | — | — | — |
| variant_get_by_type.test | RUN | 0.408321 | 3.032321 | 2.252245 | 0.002130 | 0.002222 | 0.002200 |
| variant_get_by_type_cx.test | COMPILE | 0.155966 | 0.338557 | 0.541227 | — | — | — |
| variant_default_construct.test | RUN | 0.384618 | 2.257972 | 1.863488 | 0.002066 | 0.001907 | 0.001850 |
| variant_default_construct_cx.test | COMPILE | 0.177167 | 0.367952 | 0.643449 | — | — | — |
| variant_copy_construct.test | RUN | 0.618589 | 3.736548 | 3.312028 | 0.001804 | 0.001737 | 0.002040 |
| variant_copy_construct_cx.test | COMPILE | 0.186819 | 0.432709 | 0.733748 | — | — | — |
| variant_move_construct.test | RUN | 0.573093 | 3.249497 | 3.067508 | 0.002079 | 0.001855 | 0.002114 |
| variant_move_construct_cx.test | COMPILE | 0.151411 | 0.346996 | 0.560927 | — | — | — |
| variant_value_construct.test | RUN | 0.303545 | 0.862043 | 1.335461 | 0.001942 | 0.002095 | 0.002147 |
| variant_value_construct_cx.test | COMPILE | 0.174398 | 0.408750 | 0.646560 | — | — | — |
| variant_in_place_index_construct.test | RUN | 0.313946 | 0.872239 | 1.306182 | 0.001735 | 0.001703 | 0.002111 |
| variant_in_place_index_construct_cx.test | COMPILE | 0.169445 | 0.395179 | 0.622293 | — | — | — |
| variant_in_place_type_construct.test | RUN | 0.311183 | 0.863463 | 1.394958 | 0.001786 | 0.002098 | 0.002069 |
| variant_in_place_type_construct_cx.test | COMPILE | 0.172600 | 0.400141 | 0.617757 | — | — | — |
| variant_copy_assign.test | RUN | 0.411343 | 2.617287 | 2.081757 | 0.001929 | 0.002128 | 0.002112 |
| variant_copy_assign_cx.test | COMPILE | 0.150642 | 0.337459 | 0.538309 | — | — | — |
| variant_move_assign.test | RUN | 0.417769 | 2.598138 | 2.052769 | 0.002100 | 0.002054 | 0.001816 |
| variant_move_assign_cx.test | COMPILE | 0.149482 | 0.343557 | 0.554931 | — | — | — |
| variant_value_assign.test | RUN | 0.313843 | 0.947904 | 1.465581 | 0.001774 | 0.001953 | 0.002031 |
| variant_value_assign_cx.test | COMPILE | 0.147733 | 0.332025 | 0.552381 | — | — | — |
| variant_emplace_index.test | RUN | 0.354963 | 1.108021 | 1.584367 | 0.001807 | 0.002116 | 0.002071 |
| variant_emplace_index_cx.test | COMPILE | 0.146315 | 0.333158 | 0.485707 | — | — | — |
| variant_emplace_type.test | RUN | 0.320546 | 0.943787 | 1.481850 | 0.001947 | 0.002158 | 0.001948 |
| variant_emplace_type_cx.test | COMPILE | 0.147895 | 0.336265 | 0.533295 | — | — | — |
| variant_swap.test | RUN | 0.335691 | 1.170312 | 1.535694 | 0.001795 | 0.002082 | 0.001624 |
| variant_eq_ne.test | RUN | 0.271817 | 0.700702 | 1.131475 | 0.001862 | 0.002049 | 0.001793 |
| variant_eq_ne_cx.test | COMPILE | 0.147431 | 0.356933 | 0.551018 | — | — | — |
| variant_lt_gt.test | RUN | 0.278744 | 0.802755 | 1.196380 | 0.002011 | 0.001996 | 0.001981 |
| variant_lt_gt_cx.test | COMPILE | 0.146118 | 0.341027 | 0.524390 | — | — | — |
| variant_destroy.test | RUN | 0.281796 | 0.772838 | 1.209328 | 0.002058 | 0.002195 | 0.002170 |
| variant_visit.test | RUN | 1.668010 | 6.100072 | 18.330276 | 0.001890 | 0.002581 | 0.002129 |
| variant_convert_construct.test | RUN | 0.348690 | 1.076758 | 1.663500 | 0.002198 | 0.002132 | 0.002392 |
| variant_subset.test | RUN | 0.369026 | 1.124628 | 1.723303 | 0.002069 | 0.001944 | 0.002112 |
| variant_valueless.test | RUN | 0.276372 | 0.773045 | 1.239120 | 0.001968 | 0.001832 | 0.001917 |
| variant_copy_construct_throw.test | RUN | 0.207835 | 0.565088 | 0.874474 | 0.002076 | 0.001857 | 0.002108 |
| variant_move_construct_throw.test | RUN | 0.217489 | 0.554169 | 0.839781 | 0.002012 | 0.001901 | 0.002139 |
| variant_convert_construct_throw.test | RUN | 0.219524 | 0.595556 | 0.887453 | 0.001752 | 0.001876 | 0.002108 |
| variant_copy_assign_throw.test | RUN | 0.208381 | 0.533481 | 0.884615 | 0.002028 | 0.002140 | 0.002174 |
| variant_move_assign_throw.test | RUN | 0.210539 | 0.523529 | 0.798251 | 0.001848 | 0.001943 | 0.002088 |
| variant_get_by_index_nx.test | RUN | 0.492814 | 2.491192 | 2.102328 | 0.001784 | 0.002027 | 0.002130 |
| variant_get_by_index_cx_nx.test | COMPILE | 0.166570 | 0.416492 | 0.643074 | — | — | — |
| variant_get_by_type_nx.test | RUN | 0.483081 | 2.749953 | 2.247834 | 0.002132 | 0.002197 | 0.002050 |
| variant_get_by_type_cx_nx.test | COMPILE | 0.150069 | 0.353500 | 0.561152 | — | — | — |
| variant_subset_nx.test | RUN | 0.377545 | 1.089762 | 1.732939 | 0.001889 | 0.002039 | 0.002122 |
| variant_hash.test | RUN | 0.364462 | 1.245071 | 1.667621 | 0.002062 | 0.002182 | 0.002091 |
| variant_trivial.test | RUN | 2.185767 | 6.748909 | 12.144702 | 0.002204 | 0.002214 | 0.002028 |
| variant_special.test | RUN | 2.129054 | 7.381280 | 12.003650 | 0.002118 | 0.002298 | 0.002198 |
| variant_visit_derived.test | RUN | 0.273479 | 0.696330 | 1.227839 | 0.002177 | 0.002152 | 0.002086 |
| variant_many_types.test | RUN | 2.223558 | 9.622438 | 16.637046 | 0.001967 | 0.001984 | 0.002120 |
| variant_visit_r.test | RUN | 0.935752 | 3.070686 | 5.937083 | 0.002032 | 0.002124 | 0.002092 |
| variant_derived_construct.test | COMPILE | 0.151916 | 0.352656 | 0.523438 | — | — | — |
| variant_visit_by_index.test | RUN | 0.342878 | 0.899658 | 1.572988 | 0.001868 | 0.001807 | 0.002596 |
| variant_ostream_insert.test | RUN | 0.243329 | 0.734267 | 1.081169 | 0.001992 | 0.002133 | 0.002152 |
| is_output_streamable.test | RUN | 0.309297 | 1.513219 | 1.407385 | 0.001996 | 0.002054 | 0.002044 |
| variant_json_value_from.test | RUN | 0.467163 | 1.147938 | 1.865811 | 0.002100 | 0.002028 | 0.002176 |
| variant_json_value_to.test | RUN | 0.572386 | 1.592122 | 2.463794 | 0.002107 | 0.002234 | 0.002186 |
| variant_uses_double_storage.test | COMPILE | 0.159160 | 0.361430 | 0.665906 | — | — | — |
| variant_derived_construct2.test | RUN | 0.213740 | 0.591415 | 0.949814 | 0.002015 | 0.001999 | 0.002028 |
| variant_visit_cx.test | COMPILE | 0.153082 | 0.346946 | 0.536706 | — | — | — |
| variant_visit_cx_2.test | COMPILE | 0.148042 | 0.349645 | 0.507434 | — | — | — |
| variant_visit_r_cx.test | COMPILE | 0.151036 | 0.348377 | 0.559041 | — | — | — |
| variant_index_type_126.test | RUN | 1.355088 | 3.297316 | 7.498899 | 0.001687 | 0.001889 | 0.002001 |
| variant_index_type_127.test | RUN | 1.420098 | 3.317069 | 7.787738 | 0.002064 | 0.002228 | 0.002094 |
| variant_index_type_128.test | RUN | 1.398586 | 3.379592 | 7.942273 | 0.002044 | 0.002125 | 0.001951 |
| variant_index_type_129.test | RUN | 1.411941 | 3.394252 | 7.853463 | 0.001997 | 0.002199 | 0.002102 |
| variant_index_type_254.test | RUN | 4.741511 | 11.926921 | 22.223466 | 0.002010 | 0.002418 | 0.002064 |
| variant_index_type_255.test | RUN | 4.762208 | 11.915119 | 22.602878 | 0.001934 | 0.002184 | 0.001779 |
| variant_index_type_256.test | RUN | 4.801450 | 12.231135 | 22.480819 | 0.001778 | 0.001780 | 0.001962 |
| variant_index_type_257.test | RUN | 4.862657 | 12.168316 | 22.210035 | 0.001699 | 0.001700 | 0.002111 |
| variant_default_construct_cx_2.test | COMPILE | 0.148790 | 0.377970 | 0.585710 | — | — | — |
| variant_default_construct_cx_3.test | COMPILE | 0.139428 | 0.383721 | 0.467444 | — | — | — |
| variant_default_construct_cx_4.test | COMPILE | 0.145062 | 0.321483 | 0.487482 | — | — | — |
| variant_default_construct_cx_5.test | COMPILE | 0.143215 | 0.318144 | 0.524316 | — | — | — |
| variant_value_construct_cx_2.test | COMPILE | 0.149981 | 0.334214 | 0.538107 | — | — | — |
| variant_value_construct_cx_3.test | COMPILE | 0.143161 | 0.333597 | 0.529907 | — | — | — |
| variant_value_construct_cx_4.test | COMPILE | 0.113510 | 0.294662 | 0.457044 | — | — | — |
| variant_issue_55.test | RUN | 0.230252 | 0.595752 | 0.709163 | 0.001418 | 0.001440 | 0.001439 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.990× | 0.988×, 0.993× | 1.006× |
| clang | 0.993× | 0.997×, 0.988× | 0.998× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824563).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| is_output_streamable.test | 10 | 0.001500s | 0.001511s | 0.001454s | byte equality observed |
| quick.test | 10 | 0.001458s | 0.000660s | 0.001440s | byte equality observed |
| variant_alternative.test | 10 | 0.001477s | 0.001501s | 0.001466s | byte equality observed |
| variant_convert_construct.test | 10 | 0.001517s | 0.001510s | 0.001504s | byte equality observed |
| variant_convert_construct_throw.test | 10 | 0.001522s | 0.001551s | 0.001542s | byte equality observed |
| variant_copy_assign.test | 10 | 0.001521s | 0.001531s | 0.001503s | byte equality observed |
| variant_copy_assign_throw.test | 10 | 0.001520s | 0.001553s | 0.001494s | byte equality observed |
| variant_copy_construct.test | 10 | 0.001545s | 0.001528s | 0.001478s | byte equality observed |
| variant_copy_construct_throw.test | 10 | 0.001538s | 0.001515s | 0.001533s | byte equality observed |
| variant_default_construct.test | 8 | 0.001519s | 0.001515s | 0.001535s | byte equality observed |
| variant_derived_construct2.test | 10 | 0.001496s | 0.001498s | 0.001493s | byte equality observed |
| variant_destroy.test | 10 | 0.001450s | 0.001471s | 0.001461s | byte equality observed |
| variant_emplace_index.test | 10 | 0.001488s | 0.001479s | 0.001426s | byte equality observed |
| variant_emplace_type.test | 10 | 0.001495s | 0.001483s | 0.001513s | byte equality observed |
| variant_eq_ne.test | 10 | 0.001482s | 0.001453s | 0.001493s | byte equality observed |
| variant_get_by_index.test | 10 | 0.001595s | 0.001625s | 0.001627s | byte equality observed |
| variant_get_by_index_nx.test | 10 | 0.001507s | 0.001459s | 0.001455s | byte equality observed |
| variant_get_by_type.test | 10 | 0.001530s | 0.001590s | 0.001539s | byte equality observed |
| variant_get_by_type_nx.test | 10 | 0.001503s | 0.001496s | 0.001493s | byte equality observed |
| variant_hash.test | 10 | 0.001522s | 0.001528s | 0.001457s | byte equality observed |
| variant_holds_alternative.test | 8 | 0.001480s | 0.001484s | 0.001497s | byte equality observed |
| variant_in_place_index_construct.test | 10 | 0.001528s | 0.001512s | 0.001480s | byte equality observed |
| variant_in_place_type_construct.test | 10 | 0.001494s | 0.001501s | 0.001488s | byte equality observed |
| variant_index_type_126.test | 9 | 0.001475s | 0.001501s | 0.001444s | byte equality observed |
| variant_index_type_127.test | 10 | 0.001488s | 0.001517s | 0.001457s | byte equality observed |
| variant_index_type_128.test | 10 | 0.001467s | 0.001482s | 0.001499s | byte equality observed |
| variant_index_type_129.test | 10 | 0.001499s | 0.001481s | 0.001454s | byte equality observed |
| variant_index_type_254.test | 10 | 0.001487s | 0.001504s | 0.001462s | byte equality observed |
| variant_index_type_255.test | 10 | 0.001496s | 0.001439s | 0.001490s | byte equality observed |
| variant_index_type_256.test | 10 | 0.001485s | 0.001499s | 0.001510s | byte equality observed |
| variant_index_type_257.test | 10 | 0.001496s | 0.001462s | 0.001500s | byte equality observed |
| variant_issue_55.test | 10 | 0.001513s | 0.001487s | 0.001525s | byte equality observed |
| variant_json_value_from.test | 10 | 0.001544s | 0.001583s | 0.001531s | byte equality observed |
| variant_json_value_to.test | 10 | 0.001535s | 0.001579s | 0.001545s | byte equality observed |
| variant_lt_gt.test | 10 | 0.001487s | 0.001498s | 0.001533s | byte equality observed |
| variant_many_types.test | 10 | 0.001540s | 0.001527s | 0.001519s | byte equality observed |
| variant_move_assign.test | 10 | 0.001520s | 0.001550s | 0.001506s | byte equality observed |
| variant_move_assign_throw.test | 10 | 0.001528s | 0.001535s | 0.001526s | byte equality observed |
| variant_move_construct.test | 10 | 0.001523s | 0.001494s | 0.001491s | byte equality observed |
| variant_move_construct_throw.test | 7 | 0.001532s | 0.001571s | 0.001517s | byte equality observed |
| variant_ostream_insert.test | 9 | 0.001553s | 0.001524s | 0.001542s | byte equality observed |
| variant_size.test | 10 | 0.001474s | 0.001510s | 0.001463s | byte equality observed |
| variant_special.test | 10 | 0.001581s | 0.001534s | 0.001564s | byte equality observed |
| variant_subset.test | 10 | 0.001534s | 0.001542s | 0.001566s | byte equality observed |
| variant_subset_nx.test | 10 | 0.001491s | 0.001513s | 0.001556s | byte equality observed |
| variant_swap.test | 10 | 0.001530s | 0.001523s | 0.001470s | byte equality observed |
| variant_trivial.test | 10 | 0.001585s | 0.001546s | 0.001503s | byte equality observed |
| variant_value_assign.test | 10 | 0.001492s | 0.001493s | 0.001492s | byte equality observed |
| variant_value_construct.test | 10 | 0.001515s | 0.001519s | 0.001474s | byte equality observed |
| variant_valueless.test | 10 | 0.001523s | 0.001526s | 0.001520s | byte equality observed |
| variant_visit.test | 10 | 0.001513s | 0.001520s | 0.001498s | byte equality observed |
| variant_visit_by_index.test | 10 | 0.001488s | 0.001497s | 0.001446s | byte equality observed |
| variant_visit_derived.test | 10 | 0.001488s | 0.001481s | 0.001477s | byte equality observed |
| variant_visit_r.test | 10 | 0.001488s | 0.001430s | 0.001479s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824563).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
