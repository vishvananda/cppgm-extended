# libs/variant2/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| quick.test | RUN | 0.179529 | 0.382787 | 0.590552 | 0.002052 | 0.000917 | 0.002123 |
| variant_size.test | RUN | 0.294854 | 0.854848 | 1.079946 | 0.002174 | 0.002098 | 0.001926 |
| variant_alternative.test | RUN | 0.381725 | 2.684457 | 1.797153 | 0.002006 | 0.002334 | 0.002141 |
| variant_holds_alternative.test | RUN | 0.292111 | 0.654864 | 1.059525 | 0.002173 | 0.002284 | 0.002068 |
| variant_holds_alternative_cx.test | COMPILE | 0.179162 | 0.406379 | 0.631565 | — | — | — |
| variant_get_by_index.test | RUN | 0.433398 | 2.894281 | 2.136851 | 0.001958 | 0.002220 | 0.002170 |
| variant_get_by_index_cx.test | COMPILE | 0.179603 | 0.412189 | 0.663139 | — | — | — |
| variant_get_by_type.test | RUN | 0.422260 | 3.040116 | 2.303051 | 0.002154 | 0.002067 | 0.001936 |
| variant_get_by_type_cx.test | COMPILE | 0.179083 | 0.344241 | 0.531449 | — | — | — |
| variant_default_construct.test | RUN | 0.409677 | 2.286085 | 1.860243 | 0.001842 | 0.002100 | 0.002090 |
| variant_default_construct_cx.test | COMPILE | 0.198990 | 0.378929 | 0.646565 | — | — | — |
| variant_copy_construct.test | RUN | 0.621070 | 3.822212 | 3.300959 | 0.001779 | 0.002054 | 0.002064 |
| variant_copy_construct_cx.test | COMPILE | 0.211997 | 0.440508 | 0.765491 | — | — | — |
| variant_move_construct.test | RUN | 0.613166 | 3.312018 | 3.062984 | 0.002070 | 0.001898 | 0.001940 |
| variant_move_construct_cx.test | COMPILE | 0.180271 | 0.369603 | 0.546591 | — | — | — |
| variant_value_construct.test | RUN | 0.348329 | 0.848620 | 1.346879 | 0.001838 | 0.002050 | 0.001960 |
| variant_value_construct_cx.test | COMPILE | 0.188798 | 0.401140 | 0.647283 | — | — | — |
| variant_in_place_index_construct.test | RUN | 0.342307 | 0.856459 | 1.359600 | 0.002019 | 0.001891 | 0.002113 |
| variant_in_place_index_construct_cx.test | COMPILE | 0.192932 | 0.396120 | 0.626285 | — | — | — |
| variant_in_place_type_construct.test | RUN | 0.337053 | 0.880229 | 1.356565 | 0.002012 | 0.002230 | 0.001728 |
| variant_in_place_type_construct_cx.test | COMPILE | 0.189475 | 0.405312 | 0.637048 | — | — | — |
| variant_copy_assign.test | RUN | 0.426661 | 2.643045 | 2.154337 | 0.002140 | 0.002178 | 0.002022 |
| variant_copy_assign_cx.test | COMPILE | 0.180893 | 0.351297 | 0.521434 | — | — | — |
| variant_move_assign.test | RUN | 0.419295 | 2.687555 | 2.041907 | 0.002093 | 0.002322 | 0.001988 |
| variant_move_assign_cx.test | COMPILE | 0.178186 | 0.349483 | 0.545687 | — | — | — |
| variant_value_assign.test | RUN | 0.340626 | 0.971209 | 1.488451 | 0.002110 | 0.002092 | 0.002216 |
| variant_value_assign_cx.test | COMPILE | 0.161602 | 0.348229 | 0.547756 | — | — | — |
| variant_emplace_index.test | RUN | 0.373719 | 1.105878 | 1.626574 | 0.002066 | 0.002214 | 0.002175 |
| variant_emplace_index_cx.test | COMPILE | 0.173278 | 0.353632 | 0.483247 | — | — | — |
| variant_emplace_type.test | RUN | 0.357968 | 0.939825 | 1.453699 | 0.002096 | 0.002330 | 0.001969 |
| variant_emplace_type_cx.test | COMPILE | 0.177071 | 0.356068 | 0.530879 | — | — | — |
| variant_swap.test | RUN | 0.361927 | 1.184360 | 1.582012 | 0.002133 | 0.002116 | 0.001762 |
| variant_eq_ne.test | RUN | 0.306867 | 0.709673 | 1.150756 | 0.001858 | 0.001968 | 0.001964 |
| variant_eq_ne_cx.test | COMPILE | 0.165275 | 0.358744 | 0.545100 | — | — | — |
| variant_lt_gt.test | RUN | 0.314509 | 0.829040 | 1.165933 | 0.002058 | 0.002102 | 0.002240 |
| variant_lt_gt_cx.test | COMPILE | 0.169286 | 0.328943 | 0.554403 | — | — | — |
| variant_destroy.test | RUN | 0.313102 | 0.821861 | 1.258760 | 0.001998 | 0.002124 | 0.002027 |
| variant_visit.test | RUN | 1.708332 | 6.186270 | 18.206890 | 0.001943 | 0.002315 | 0.001966 |
| variant_convert_construct.test | RUN | 0.381719 | 1.063593 | 1.726729 | 0.002108 | 0.002332 | 0.002120 |
| variant_subset.test | RUN | 0.394976 | 1.152559 | 1.773249 | 0.002100 | 0.002260 | 0.002212 |
| variant_valueless.test | RUN | 0.297751 | 0.783118 | 1.226886 | 0.002107 | 0.002224 | 0.002104 |
| variant_copy_construct_throw.test | RUN | 0.244976 | 0.551275 | 0.845510 | 0.002303 | 0.002372 | 0.002074 |
| variant_move_construct_throw.test | RUN | 0.239172 | 0.562739 | 0.833666 | 0.002011 | 0.002142 | 0.002281 |
| variant_convert_construct_throw.test | RUN | 0.247537 | 0.565991 | 0.925840 | 0.002122 | 0.002128 | 0.002127 |
| variant_copy_assign_throw.test | RUN | 0.233570 | 0.525886 | 0.818800 | 0.002296 | 0.002400 | 0.002257 |
| variant_move_assign_throw.test | RUN | 0.235635 | 0.537848 | 0.840661 | 0.002240 | 0.002161 | 0.002157 |
| variant_get_by_index_nx.test | RUN | 0.499486 | 2.518675 | 2.165431 | 0.002056 | 0.002034 | 0.002140 |
| variant_get_by_index_cx_nx.test | COMPILE | 0.192779 | 0.383752 | 0.616452 | — | — | — |
| variant_get_by_type_nx.test | RUN | 0.511734 | 2.834117 | 2.293429 | 0.002146 | 0.001903 | 0.002102 |
| variant_get_by_type_cx_nx.test | COMPILE | 0.171922 | 0.365517 | 0.555959 | — | — | — |
| variant_subset_nx.test | RUN | 0.415727 | 1.138742 | 1.781671 | 0.002118 | 0.002090 | 0.002261 |
| variant_hash.test | RUN | 0.412185 | 1.272671 | 1.659053 | 0.002101 | 0.002415 | 0.002106 |
| variant_trivial.test | RUN | 2.251515 | 6.977201 | 12.365828 | 0.002200 | 0.002299 | 0.002013 |
| variant_special.test | RUN | 2.133623 | 7.401473 | 12.647923 | 0.002190 | 0.002422 | 0.002260 |
| variant_visit_derived.test | RUN | 0.302380 | 0.698979 | 1.160123 | 0.002196 | 0.002213 | 0.002042 |
| variant_many_types.test | RUN | 2.238606 | 9.831382 | 15.962795 | 0.001858 | 0.002424 | 0.002214 |
| variant_visit_r.test | RUN | 0.944154 | 3.067899 | 6.111713 | 0.001952 | 0.002089 | 0.002195 |
| variant_derived_construct.test | COMPILE | 0.173231 | 0.363567 | 0.538167 | — | — | — |
| variant_visit_by_index.test | RUN | 0.374055 | 0.936608 | 1.584041 | 0.001854 | 0.002343 | 0.001917 |
| variant_ostream_insert.test | RUN | 0.272902 | 0.703103 | 1.070360 | 0.002118 | 0.002238 | 0.001702 |
| is_output_streamable.test | RUN | 0.322661 | 1.449293 | 1.473057 | 0.001840 | 0.002216 | 0.002204 |
| variant_json_value_from.test | RUN | 0.501477 | 1.181833 | 1.862647 | 0.002040 | 0.002310 | 0.002250 |
| variant_json_value_to.test | RUN | 0.620091 | 1.614081 | 2.517687 | 0.002092 | 0.002372 | 0.002224 |
| variant_uses_double_storage.test | COMPILE | 0.179004 | 0.358693 | 0.609240 | — | — | — |
| variant_derived_construct2.test | RUN | 0.248276 | 0.592485 | 0.907814 | 0.001802 | 0.002206 | 0.002122 |
| variant_visit_cx.test | COMPILE | 0.168041 | 0.355222 | 0.512106 | — | — | — |
| variant_visit_cx_2.test | COMPILE | 0.165720 | 0.346228 | 0.550943 | — | — | — |
| variant_visit_r_cx.test | COMPILE | 0.164545 | 0.340283 | 0.513311 | — | — | — |
| variant_index_type_126.test | RUN | 1.426083 | 3.403731 | 7.789677 | 0.002050 | 0.002168 | 0.002048 |
| variant_index_type_127.test | RUN | 1.438125 | 3.442702 | 7.932220 | 0.002118 | 0.002508 | 0.001763 |
| variant_index_type_128.test | RUN | 1.463347 | 3.515643 | 7.895046 | 0.001795 | 0.002104 | 0.002042 |
| variant_index_type_129.test | RUN | 1.465628 | 3.485759 | 7.967394 | 0.001836 | 0.002318 | 0.002212 |
| variant_index_type_254.test | RUN | 4.822075 | 12.087053 | 22.412673 | 0.002152 | 0.002152 | 0.001649 |
| variant_index_type_255.test | RUN | 5.103280 | 12.159194 | 21.962858 | 0.001608 | 0.002228 | 0.002162 |
| variant_index_type_256.test | RUN | 5.070162 | 12.162693 | 22.593690 | 0.001802 | 0.002191 | 0.002046 |
| variant_index_type_257.test | RUN | 4.940757 | 12.456471 | 23.093770 | 0.001902 | 0.002129 | 0.001418 |
| variant_default_construct_cx_2.test | COMPILE | 0.154570 | 0.375482 | 0.541662 | — | — | — |
| variant_default_construct_cx_3.test | COMPILE | 0.172915 | 0.387493 | 0.548039 | — | — | — |
| variant_default_construct_cx_4.test | COMPILE | 0.166840 | 0.335501 | 0.538569 | — | — | — |
| variant_default_construct_cx_5.test | COMPILE | 0.164244 | 0.340930 | 0.589435 | — | — | — |
| variant_value_construct_cx_2.test | COMPILE | 0.164177 | 0.327611 | 0.532570 | — | — | — |
| variant_value_construct_cx_3.test | COMPILE | 0.163617 | 0.352939 | 0.528479 | — | — | — |
| variant_value_construct_cx_4.test | COMPILE | 0.103793 | 0.242263 | 0.491171 | — | — | — |
| variant_issue_55.test | RUN | 0.197837 | 0.539672 | 0.882209 | 0.001393 | 0.001598 | 0.001343 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.984× | 0.983×, 0.986× | 0.999× |
| clang | 0.994× | 0.984×, 1.004× | 1.010× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| is_output_streamable.test | 8 | 0.001549s | 0.001572s | 0.001526s | byte equality observed |
| quick.test | 10 | 0.001482s | 0.000680s | 0.001498s | byte equality observed |
| variant_alternative.test | 10 | 0.001530s | 0.001562s | 0.001558s | byte equality observed |
| variant_convert_construct.test | 10 | 0.001534s | 0.001557s | 0.001560s | byte equality observed |
| variant_convert_construct_throw.test | 10 | 0.001586s | 0.001574s | 0.001608s | byte equality observed |
| variant_copy_assign.test | 10 | 0.001563s | 0.001532s | 0.001521s | byte equality observed |
| variant_copy_assign_throw.test | 9 | 0.001600s | 0.001607s | 0.001581s | byte equality observed |
| variant_copy_construct.test | 10 | 0.001564s | 0.001564s | 0.001527s | byte equality observed |
| variant_copy_construct_throw.test | 10 | 0.001573s | 0.001575s | 0.001585s | byte equality observed |
| variant_default_construct.test | 10 | 0.001540s | 0.001538s | 0.001552s | byte equality observed |
| variant_derived_construct2.test | 10 | 0.001532s | 0.001521s | 0.001559s | byte equality observed |
| variant_destroy.test | 10 | 0.001542s | 0.001525s | 0.001565s | byte equality observed |
| variant_emplace_index.test | 10 | 0.001555s | 0.001568s | 0.001549s | byte equality observed |
| variant_emplace_type.test | 10 | 0.001577s | 0.001522s | 0.001549s | byte equality observed |
| variant_eq_ne.test | 10 | 0.001535s | 0.001555s | 0.001566s | byte equality observed |
| variant_get_by_index.test | 10 | 0.001658s | 0.001708s | 0.001635s | byte equality observed |
| variant_get_by_index_nx.test | 10 | 0.001565s | 0.001520s | 0.001546s | byte equality observed |
| variant_get_by_type.test | 10 | 0.001588s | 0.001625s | 0.001597s | byte equality observed |
| variant_get_by_type_nx.test | 10 | 0.001557s | 0.001554s | 0.001559s | byte equality observed |
| variant_hash.test | 10 | 0.001529s | 0.001530s | 0.001548s | byte equality observed |
| variant_holds_alternative.test | 10 | 0.001529s | 0.001524s | 0.001556s | byte equality observed |
| variant_in_place_index_construct.test | 10 | 0.001558s | 0.001588s | 0.001521s | byte equality observed |
| variant_in_place_type_construct.test | 10 | 0.001546s | 0.001565s | 0.001533s | byte equality observed |
| variant_index_type_126.test | 10 | 0.001546s | 0.001549s | 0.001526s | byte equality observed |
| variant_index_type_127.test | 10 | 0.001553s | 0.001532s | 0.001526s | byte equality observed |
| variant_index_type_128.test | 10 | 0.001534s | 0.001519s | 0.001576s | byte equality observed |
| variant_index_type_129.test | 10 | 0.001568s | 0.001534s | 0.001506s | byte equality observed |
| variant_index_type_254.test | 10 | 0.001542s | 0.001532s | 0.001533s | byte equality observed |
| variant_index_type_255.test | 10 | 0.001524s | 0.001501s | 0.001511s | byte equality observed |
| variant_index_type_256.test | 10 | 0.001535s | 0.001546s | 0.001544s | byte equality observed |
| variant_index_type_257.test | 10 | 0.001557s | 0.001500s | 0.001566s | byte equality observed |
| variant_issue_55.test | 10 | 0.001572s | 0.001580s | 0.001549s | byte equality observed |
| variant_json_value_from.test | 10 | 0.001586s | 0.001582s | 0.001562s | byte equality observed |
| variant_json_value_to.test | 10 | 0.001598s | 0.001564s | 0.001583s | byte equality observed |
| variant_lt_gt.test | 10 | 0.001532s | 0.001545s | 0.001544s | byte equality observed |
| variant_many_types.test | 10 | 0.001616s | 0.001577s | 0.001548s | byte equality observed |
| variant_move_assign.test | 10 | 0.001536s | 0.001577s | 0.001558s | byte equality observed |
| variant_move_assign_throw.test | 10 | 0.001587s | 0.001618s | 0.001549s | byte equality observed |
| variant_move_construct.test | 10 | 0.001551s | 0.001554s | 0.001570s | byte equality observed |
| variant_move_construct_throw.test | 10 | 0.001565s | 0.001565s | 0.001555s | byte equality observed |
| variant_ostream_insert.test | 10 | 0.001581s | 0.001584s | 0.001616s | byte equality observed |
| variant_size.test | 10 | 0.001578s | 0.001533s | 0.001527s | byte equality observed |
| variant_special.test | 10 | 0.001621s | 0.001599s | 0.001556s | byte equality observed |
| variant_subset.test | 9 | 0.001569s | 0.001588s | 0.001503s | byte equality observed |
| variant_subset_nx.test | 10 | 0.001548s | 0.001523s | 0.001521s | byte equality observed |
| variant_swap.test | 10 | 0.001559s | 0.001557s | 0.001590s | byte equality observed |
| variant_trivial.test | 10 | 0.001603s | 0.001576s | 0.001579s | byte equality observed |
| variant_value_assign.test | 10 | 0.001537s | 0.001546s | 0.001549s | byte equality observed |
| variant_value_construct.test | 10 | 0.001550s | 0.001529s | 0.001533s | byte equality observed |
| variant_valueless.test | 10 | 0.001611s | 0.001592s | 0.001613s | byte equality observed |
| variant_visit.test | 10 | 0.001577s | 0.001555s | 0.001532s | byte equality observed |
| variant_visit_by_index.test | 10 | 0.001539s | 0.001481s | 0.001565s | byte equality observed |
| variant_visit_derived.test | 10 | 0.001567s | 0.001523s | 0.001522s | byte equality observed |
| variant_visit_r.test | 10 | 0.001525s | 0.001511s | 0.001558s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
