# libs/type_erasure/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_binding.test | RUN | 0.607325 | 2.031190 | 1.899023 | 0.003484 | 0.003524 | 0.003396 |
| test_increment.test | RUN | 0.559413 | 1.575663 | 1.651389 | 0.003155 | 0.003328 | 0.003041 |
| test_add.test | RUN | 0.767109 | 2.477720 | 2.416052 | 0.003825 | 0.003801 | 0.003425 |
| test_add_assign.test | RUN | 0.713306 | 2.617333 | 2.487199 | 0.003441 | 0.003729 | 0.003691 |
| test_callable.test | RUN | 1.176377 | 4.585629 | 5.505683 | 0.005716 | 0.006211 | 0.005800 |
| test_reference.test | RUN | 0.594448 | 1.817004 | 1.721462 | 0.003290 | 0.003731 | 0.003700 |
| test_construct.test | RUN | 2.322603 | 23.078792 | 15.639254 | 0.007967 | 0.007484 | 0.006851 |
| test_relaxed.test | RUN | 0.568568 | 1.310578 | 1.520246 | 0.003359 | 0.003201 | 0.003178 |
| test_assign.test | RUN | 6.379314 | 73.372988 | 55.196574 | 0.010979 | 0.007205 | 0.007272 |
| test_construct_ref.test | RUN | 0.709452 | 3.123379 | 2.456936 | 0.004825 | 0.004800 | 0.004742 |
| test_construct_cref.test | RUN | 0.814730 | 2.965975 | 3.187798 | 0.005054 | 0.005425 | 0.005308 |
| test_any_cast.test | RUN | 0.670423 | 3.042061 | 2.353655 | 0.004261 | 0.004287 | 0.004060 |
| test_binding_of.test | RUN | 0.550593 | 1.231780 | 1.476156 | 0.003154 | 0.003078 | 0.003355 |
| test_typeid_of.test | RUN | 0.575453 | 1.588763 | 1.643457 | 0.003763 | 0.003672 | 0.003585 |
| test_nested.test | RUN | 0.644051 | 1.709102 | 1.954596 | 0.003381 | 0.003243 | 0.002806 |
| test_less.test | RUN | 0.819303 | 3.292024 | 2.936358 | 0.004817 | 0.004559 | 0.004560 |
| test_equal.test | RUN | 0.747805 | 3.100208 | 2.627850 | 0.004013 | 0.004081 | 0.004407 |
| test_negate.test | RUN | 0.565001 | 1.217284 | 1.497509 | 0.003185 | 0.003236 | 0.002830 |
| test_dereference.test | RUN | 0.617358 | 1.600065 | 1.812952 | 0.003573 | 0.003223 | 0.002859 |
| test_subscript.test | RUN | 0.640533 | 1.687133 | 1.928259 | 0.003590 | 0.003416 | 0.003590 |
| test_forward_iterator.test | RUN | 0.762657 | 2.004810 | 2.233193 | 0.003544 | 0.003144 | 0.003126 |
| test_tuple.test | RUN | 0.655825 | 1.966828 | 1.684638 | 0.003677 | 0.003623 | 0.003537 |
| test_stream.test | RUN | 0.841762 | 3.438730 | 3.070497 | 0.005059 | 0.004773 | 0.005215 |
| test_deduced.test | RUN | 0.564174 | 1.193408 | 1.513642 | 0.003232 | 0.003267 | 0.002952 |
| test_same_type.test | RUN | 0.618485 | 1.721185 | 1.629389 | 0.003658 | 0.003442 | 0.003052 |
| test_member.test | RUN | 0.777421 | 3.362629 | 2.990073 | 0.005097 | 0.005017 | 0.005436 |
| test_null.test | RUN | 0.566662 | 1.641863 | 1.605971 | 0.003303 | 0.003324 | 0.003556 |
| test_free.test | RUN | 0.690955 | 2.713681 | 1.899765 | 0.004069 | 0.004224 | 0.003713 |
| test_is_empty.test | RUN | 0.555741 | 1.397505 | 1.463352 | 0.003269 | 0.003231 | 0.003332 |
| test_dynamic_any_cast.test | RUN | 0.970743 | 4.038179 | 3.944547 | 0.004584 | 0.005272 | 0.004206 |
| test_limits.test | RUN | 0.901457 | 2.827472 | 2.954498 | 0.003359 | 0.003605 | 0.003593 |
| test_param.test | COMPILE | 0.386141 | 0.743141 | 0.986688 | — | — | — |
| test_is_subconcept.test | COMPILE | 0.370834 | 0.571750 | 0.808719 | — | — | — |
| test_sfinae.test | COMPILE | 0.364906 | 0.569484 | 0.815631 | — | — | — |
| test_interface_order.test | COMPILE | 0.352232 | 0.614517 | 0.820748 | — | — | — |
| test_is_placeholder.test | COMPILE | 0.050784 | 0.056086 | 0.059948 | — | — | — |
| fail_default_construct.test | COMPILE_FAIL | 0.350692 | 0.561324 | 0.798055 | — | — | — |
| fail_construct_mismatch.test | COMPILE_FAIL | 0.352194 | 0.586050 | 0.685388 | — | — | — |
| fail_construct_mismatch_ref.test | COMPILE_FAIL | 0.349237 | 0.570544 | 0.791712 | — | — | — |
| fail_construct_mismatch_cref.test | COMPILE_FAIL | 0.342580 | 0.583412 | 0.673759 | — | — | — |
| fail_construct_mismatch_rref.test | COMPILE_FAIL | 0.347115 | 0.593591 | 0.823615 | — | — | — |
| fail_binding_convert_no_mapping.test | COMPILE_FAIL | 0.335246 | 0.566177 | 0.777319 | — | — | — |
| fail_increment_discard_const.test | COMPILE_FAIL | 0.347989 | 0.592780 | 0.680889 | — | — | — |
| fail_not_incrementable.test | COMPILE_FAIL | 0.359693 | 0.611496 | 0.822714 | — | — | — |
| fail_ref_assign.test | COMPILE_FAIL | 0.348641 | 0.548556 | 0.785238 | — | — | — |
| fail_cref_assign.test | COMPILE_FAIL | 0.349451 | 0.574694 | 0.719944 | — | — | — |
| fail_rref_assign.test | COMPILE_FAIL | 0.350807 | 0.528115 | 0.800612 | — | — | — |
| fail_ref_discard_const.test | COMPILE_FAIL | 0.342361 | 0.592132 | 0.818625 | — | — | — |
| fail_ref_discard_const_convert.test | COMPILE_FAIL | 0.352085 | 0.636854 | 0.703074 | — | — | — |
| fail_ref_discard_const_convert_ref.test | COMPILE_FAIL | 0.350573 | 0.595172 | 0.830817 | — | — | — |
| fail_ref_discard_const_convert_cref.test | COMPILE_FAIL | 0.349907 | 0.579839 | 0.747787 | — | — | — |
| fail_ref_discard_const_init.test | COMPILE_FAIL | 0.342475 | 0.589240 | 0.782071 | — | — | — |
| fail_any_cast_discard_const1.test | COMPILE_FAIL | 0.356186 | 0.551046 | 0.814011 | — | — | — |
| fail_any_cast_discard_const2.test | COMPILE_FAIL | 0.354321 | 0.610121 | 0.718342 | — | — | — |
| fail_any_cast_discard_const3.test | COMPILE_FAIL | 0.356675 | 0.604679 | 0.835457 | — | — | — |
| fail_any_cast_discard_const4.test | COMPILE_FAIL | 0.351589 | 0.600881 | 0.827720 | — | — | — |
| fail_any_cast_discard_const5.test | COMPILE_FAIL | 0.352895 | 0.564230 | 0.802145 | — | — | — |
| fail_any_cast_discard_const6.test | COMPILE_FAIL | 0.348706 | 0.603565 | 0.841336 | — | — | — |
| fail_any_cast_pointer_to_ref.test | COMPILE_FAIL | 0.328909 | 0.603160 | 0.695788 | — | — | — |
| fail_any_cast_pointer_to_val.test | COMPILE_FAIL | 0.253696 | 0.552758 | 0.631653 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.972×† | 1.063×, 0.880× | 0.999× |
| clang | 0.991×† | 1.106×, 0.877× | 0.916× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| test_add.test | 9 | 0.002015s | 0.001991s | 0.002090s | byte equality observed |
| test_add_assign.test | 9 | 0.001973s | 0.002021s | 0.002042s | byte equality observed |
| test_any_cast.test | 8 | 0.002253s | 0.002216s | 0.002356s | byte equality observed |
| test_assign.test | 4 | 0.005925s | 0.005291s | 0.006058s | byte equality observed |
| test_binding.test | 10 | 0.001868s | 0.001804s | 0.001961s | byte equality observed |
| test_binding_of.test | 10 | 0.001753s | 0.001795s | 0.001860s | byte equality observed |
| test_callable.test | 7 | 0.002882s | 0.002899s | 0.003183s | byte equality observed |
| test_construct.test | 5 | 0.003795s | 0.003710s | 0.003999s | byte equality observed |
| test_construct_cref.test | 7 | 0.002622s | 0.002589s | 0.002817s | byte equality observed |
| test_construct_ref.test | 8 | 0.002378s | 0.002375s | 0.002494s | byte equality observed |
| test_deduced.test | 10 | 0.001742s | 0.001714s | 0.001820s | byte equality observed |
| test_dereference.test | 10 | 0.001873s | 0.001777s | 0.001951s | byte equality observed |
| test_dynamic_any_cast.test | 7 | 0.002601s | 0.002555s | 0.002793s | byte equality observed |
| test_equal.test | 9 | 0.002252s | 0.002124s | 0.002297s | byte equality observed |
| test_forward_iterator.test | 10 | 0.001940s | 0.001819s | 0.001993s | byte equality observed |
| test_free.test | 8 | 0.002282s | 0.002193s | 0.002339s | byte equality observed |
| test_increment.test | 10 | 0.001859s | 0.001793s | 0.001895s | byte equality observed |
| test_is_empty.test | 9 | 0.001853s | 0.001735s | 0.001898s | byte equality observed |
| test_less.test | 6 | 0.002459s | 0.002339s | 0.002544s | byte equality observed |
| test_limits.test | 7 | 0.002024s | 0.001900s | 0.002047s | byte equality observed |
| test_member.test | 7 | 0.002772s | 0.002638s | 0.002858s | byte equality observed |
| test_negate.test | 10 | 0.001747s | 0.001690s | 0.001806s | byte equality observed |
| test_nested.test | 10 | 0.001865s | 0.001794s | 0.001837s | byte equality observed |
| test_null.test | 10 | 0.001870s | 0.001837s | 0.001970s | byte equality observed |
| test_reference.test | 8 | 0.001948s | 0.001928s | 0.001986s | byte equality observed |
| test_relaxed.test | 10 | 0.001841s | 0.001769s | 0.001916s | byte equality observed |
| test_same_type.test | 9 | 0.001868s | 0.001858s | 0.001954s | byte equality observed |
| test_stream.test | 8 | 0.002549s | 0.002474s | 0.002651s | byte equality observed |
| test_subscript.test | 9 | 0.001930s | 0.001857s | 0.002028s | byte equality observed |
| test_tuple.test | 9 | 0.001916s | 0.001917s | 0.002058s | byte equality observed |
| test_typeid_of.test | 8 | 0.001999s | 0.001923s | 0.002100s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
