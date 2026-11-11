# libs/mpl/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| increased_arity.test | COMPILE | 0.344951 | 0.846578 | 0.782064 | — | — | — |
| largest_int.test | COMPILE | 0.131371 | 0.245951 | 0.274576 | — | — | — |
| msvc_is_class.test | COMPILE | 0.015470 | 0.033042 | 0.035685 | — | — | — |
| template_arity.test | COMPILE | 0.123786 | 0.243635 | 0.273138 | — | — | — |
| is_seq.test | COMPILE | 0.005168 | 0.013974 | 0.026192 | — | — | — |
| token_equal.test | COMPILE | 0.005814 | 0.012358 | 0.026846 | — | — | — |
| advance.test | COMPILE | 0.137714 | 0.268640 | 0.315476 | — | — | — |
| always.test | COMPILE | 0.138588 | 0.267763 | 0.312615 | — | — | — |
| apply.test | COMPILE | 0.149463 | 0.297073 | 0.334141 | — | — | — |
| apply_wrap.test | COMPILE | 0.131147 | 0.269972 | 0.300437 | — | — | — |
| arithmetic.test | COMPILE | 0.141287 | 0.301180 | 0.313257 | — | — | — |
| as_sequence.test | COMPILE | 0.148036 | 0.316105 | 0.357521 | — | — | — |
| assert.test | COMPILE | 0.038435 | 0.064555 | 0.071635 | — | — | — |
| at.test | COMPILE | 0.145038 | 0.298000 | 0.365803 | — | — | — |
| back.test | COMPILE | 0.140352 | 0.290106 | 0.341115 | — | — | — |
| bind.test | COMPILE | 0.137567 | 0.281300 | 0.329176 | — | — | — |
| bitwise.test | COMPILE | 0.137246 | 0.277325 | 0.352121 | — | — | — |
| bool.test | RUN | 0.126282 | 0.253607 | 0.310914 | 0.001558 | 0.001581 | 0.001498 |
| comparison.test | COMPILE | 0.135963 | 0.274429 | 0.322682 | — | — | — |
| contains.test | COMPILE | 0.161776 | 0.339076 | 0.379550 | — | — | — |
| copy.test | COMPILE | 0.192622 | 0.405194 | 0.478570 | — | — | — |
| copy_if.test | COMPILE | 0.170485 | 0.378729 | 0.421577 | — | — | — |
| count.test | COMPILE | 0.165005 | 0.354260 | 0.394949 | — | — | — |
| count_if.test | COMPILE | 0.173678 | 0.370556 | 0.410442 | — | — | — |
| deque.test | COMPILE | 0.166032 | 0.359453 | 0.401595 | — | — | — |
| distance.test | COMPILE | 0.154381 | 0.316931 | 0.366610 | — | — | — |
| empty.test | COMPILE | 0.135074 | 0.276194 | 0.324854 | — | — | — |
| equal.test | COMPILE | 0.158745 | 0.334370 | 0.369670 | — | — | — |
| erase.test | COMPILE | 0.177609 | 0.387749 | 0.447230 | — | — | — |
| erase_range.test | COMPILE | 0.178208 | 0.350301 | 0.405126 | — | — | — |
| eval_if.test | COMPILE | 0.131665 | 0.258632 | 0.291784 | — | — | — |
| filter_view.test | COMPILE | 0.178951 | 0.365810 | 0.418311 | — | — | — |
| find.test | COMPILE | 0.172636 | 0.358835 | 0.396870 | — | — | — |
| find_if.test | COMPILE | 0.169782 | 0.369314 | 0.407769 | — | — | — |
| fold.test | COMPILE | 0.178632 | 0.376910 | 0.426435 | — | — | — |
| for_each.test | RUN | 0.256390 | 0.547969 | 0.652860 | 0.001584 | 0.002007 | 0.001808 |
| front.test | COMPILE | 0.143859 | 0.299799 | 0.337333 | — | — | — |
| get_tag_def.test | COMPILE | 0.129914 | 0.257981 | 0.288427 | — | — | — |
| has_xxx.test | COMPILE | 0.147309 | 0.269435 | 0.329052 | — | — | — |
| identity.test | COMPILE | 0.145210 | 0.293832 | 0.305193 | — | — | — |
| if.test | COMPILE | 0.132716 | 0.255590 | 0.279332 | — | — | — |
| index_of.test | COMPILE | 0.159465 | 0.332319 | 0.370002 | — | — | — |
| inherit.test | COMPILE | 0.135321 | 0.264138 | 0.296688 | — | — | — |
| insert.test | COMPILE | 0.184759 | 0.383781 | 0.437882 | — | — | — |
| insert_range.test | COMPILE | 0.249923 | 0.509071 | 0.627052 | — | — | — |
| int.test | RUN | 0.134069 | 0.281473 | 0.308391 | 0.001594 | 0.001814 | 0.001463 |
| char.test | RUN | 0.135325 | 0.283196 | 0.318276 | 0.001621 | 0.001555 | 0.001584 |
| char_unsigned.test | RUN | 0.135506 | 0.279455 | 0.321798 | 0.001582 | 0.001856 | 0.001859 |
| integral_c.test | RUN | 0.145758 | 0.376775 | 0.381303 | 0.001696 | 0.001832 | 0.001844 |
| integral_c_unsigned.test | RUN | 0.142150 | 0.370683 | 0.384412 | 0.001694 | 0.001571 | 0.001899 |
| is_placeholder.test | COMPILE | 0.131797 | 0.266163 | 0.284584 | — | — | — |
| is_sequence.test | COMPILE | 0.156845 | 0.336191 | 0.327505 | — | — | — |
| iterator_tags.test | COMPILE | 0.135605 | 0.275209 | 0.293950 | — | — | — |
| joint_view.test | COMPILE | 0.172656 | 0.358107 | 0.404399 | — | — | — |
| lambda.test | COMPILE | 0.163740 | 0.332790 | 0.372239 | — | — | — |
| lambda_args.test | COMPILE | 0.151709 | 0.309514 | 0.373278 | — | — | — |
| list.test | COMPILE | 0.157212 | 0.330428 | 0.369442 | — | — | — |
| list_c.test | COMPILE | 0.153196 | 0.328592 | 0.395293 | — | — | — |
| logical.test | COMPILE | 0.133081 | 0.264717 | 0.316659 | — | — | — |
| lower_bound.test | COMPILE | 0.164109 | 0.334830 | 0.373172 | — | — | — |
| map.test | COMPILE | 0.188175 | 0.427030 | 0.488333 | — | — | — |
| max_element.test | COMPILE | 0.159446 | 0.307092 | 0.362756 | — | — | — |
| min_max.test | COMPILE | 0.142307 | 0.269399 | 0.324911 | — | — | — |
| multiset.test | COMPILE | 0.165670 | 0.358205 | 0.411010 | — | — | — |
| next.test | COMPILE | 0.127872 | 0.267091 | 0.332094 | — | — | — |
| no_has_xxx.test | COMPILE | 0.138752 | 0.269099 | 0.328506 | — | — | — |
| numeric_ops.test | COMPILE | 0.156334 | 0.316164 | 0.349453 | — | — | — |
| pair_view.test | COMPILE | 0.171256 | 0.342187 | 0.410668 | — | — | — |
| partition.test | COMPILE | 0.188877 | 0.381968 | 0.452427 | — | — | — |
| pop_front.test | COMPILE | 0.154998 | 0.310867 | 0.354974 | — | — | — |
| push_back.test | COMPILE | 0.139899 | 0.263033 | 0.313210 | — | — | — |
| push_front.test | COMPILE | 0.153552 | 0.314717 | 0.362776 | — | — | — |
| quote.test | COMPILE | 0.129271 | 0.261177 | 0.293489 | — | — | — |
| range_c.test | COMPILE | 0.156247 | 0.330008 | 0.374352 | — | — | — |
| remove.test | COMPILE | 0.172714 | 0.348582 | 0.388820 | — | — | — |
| remove_if.test | COMPILE | 0.169111 | 0.364258 | 0.412104 | — | — | — |
| replace.test | COMPILE | 0.176484 | 0.435062 | 0.430641 | — | — | — |
| replace_if.test | COMPILE | 0.178831 | 0.433691 | 0.440129 | — | — | — |
| reverse.test | COMPILE | 0.175232 | 0.408214 | 0.410096 | — | — | — |
| same_as.test | COMPILE | 0.140896 | 0.288299 | 0.312529 | — | — | — |
| set.test | COMPILE | 0.234412 | 0.545628 | 0.619228 | — | — | — |
| set_c.test | COMPILE | 0.172079 | 0.373886 | 0.418491 | — | — | — |
| single_view.test | COMPILE | 0.158485 | 0.344403 | 0.360067 | — | — | — |
| size.test | COMPILE | 0.150474 | 0.311622 | 0.349595 | — | — | — |
| size_t.test | RUN | 0.140265 | 0.287424 | 0.322940 | 0.001394 | 0.002176 | 0.001961 |
| sizeof.test | COMPILE | 0.128413 | 0.262565 | 0.286359 | — | — | — |
| sort.test | COMPILE | 0.193585 | 0.422109 | 0.470764 | — | — | — |
| stable_partition.test | COMPILE | 0.181132 | 0.399526 | 0.413728 | — | — | — |
| transform.test | COMPILE | 0.191493 | 0.419646 | 0.465973 | — | — | — |
| transform_view.test | COMPILE | 0.161223 | 0.342559 | 0.377388 | — | — | — |
| unique.test | COMPILE | 0.166638 | 0.343907 | 0.384910 | — | — | — |
| unpack_args.test | COMPILE | 0.156659 | 0.321571 | 0.370274 | — | — | — |
| upper_bound.test | COMPILE | 0.159244 | 0.331313 | 0.368567 | — | — | — |
| vector.test | COMPILE | 0.183889 | 0.404011 | 0.442265 | — | — | — |
| vector_c.test | COMPILE | 0.159510 | 0.326300 | 0.381696 | — | — | — |
| zip_view.test | COMPILE | 0.219200 | 0.500168 | 0.546981 | — | — | — |
| string.test | RUN | 0.316644 | 0.976512 | 0.834565 | 0.001405 | 0.001436 | 0.001450 |
| issue_69.test | RUN | 0.012673 | 0.042719 | 0.046449 | 0.000664 | 0.000674 | 0.001901 |
| issue_84.test | RUN | 0.040586 | 0.078524 | 0.076532 | 0.000594 | 0.000673 | 0.001399 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.011×† | 1.018×, 1.004× | 1.066× |
| clang | 1.108×† | 1.087×, 1.129× | 0.942× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| bool.test | 10 | 0.000964s | 0.000947s | 0.000956s | byte equality observed |
| char.test | 10 | 0.000939s | 0.000924s | 0.000954s | byte equality observed |
| char_unsigned.test | 10 | 0.000948s | 0.000952s | 0.000947s | byte equality observed |
| for_each.test | 10 | 0.000968s | 0.000982s | 0.000993s | byte equality observed |
| int.test | 10 | 0.000956s | 0.000964s | 0.000965s | byte equality observed |
| integral_c.test | 10 | 0.000949s | 0.000958s | 0.000996s | byte equality observed |
| integral_c_unsigned.test | 10 | 0.000957s | 0.000985s | 0.000984s | byte equality observed |
| issue_69.test | 10 | 0.000447s | 0.000454s | 0.000936s | byte equality observed |
| issue_84.test | 10 | 0.000442s | 0.000453s | 0.000944s | byte equality observed |
| size_t.test | 10 | 0.000959s | 0.000958s | 0.000971s | byte equality observed |
| string.test | 10 | 0.000950s | 0.000961s | 0.000984s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
