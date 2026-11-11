# libs/mpl/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| increased_arity.test | COMPILE | 0.549102 | 1.309499 | 1.284368 | — | — | — |
| largest_int.test | COMPILE | 0.174941 | 0.370986 | 0.458936 | — | — | — |
| msvc_is_class.test | COMPILE | 0.020412 | 0.045868 | 0.046799 | — | — | — |
| template_arity.test | COMPILE | 0.175859 | 0.363875 | 0.456707 | — | — | — |
| is_seq.test | COMPILE | 0.006253 | 0.014485 | 0.026996 | — | — | — |
| token_equal.test | COMPILE | 0.007200 | 0.014991 | 0.027538 | — | — | — |
| advance.test | COMPILE | 0.197874 | 0.402975 | 0.485978 | — | — | — |
| always.test | COMPILE | 0.201167 | 0.409488 | 0.518332 | — | — | — |
| apply.test | COMPILE | 0.204946 | 0.440182 | 0.542947 | — | — | — |
| apply_wrap.test | COMPILE | 0.184162 | 0.364503 | 0.455443 | — | — | — |
| arithmetic.test | COMPILE | 0.200505 | 0.421899 | 0.518157 | — | — | — |
| as_sequence.test | COMPILE | 0.211312 | 0.450577 | 0.551725 | — | — | — |
| assert.test | COMPILE | 0.053287 | 0.085570 | 0.090097 | — | — | — |
| at.test | COMPILE | 0.208428 | 0.440941 | 0.541367 | — | — | — |
| back.test | COMPILE | 0.203740 | 0.423113 | 0.518881 | — | — | — |
| bind.test | COMPILE | 0.199528 | 0.407830 | 0.500942 | — | — | — |
| bitwise.test | COMPILE | 0.197595 | 0.421580 | 0.516930 | — | — | — |
| bool.test | RUN | 0.178589 | 0.368493 | 0.460022 | 0.001569 | 0.001561 | 0.001655 |
| comparison.test | COMPILE | 0.201464 | 0.412785 | 0.505603 | — | — | — |
| contains.test | COMPILE | 0.238302 | 0.501925 | 0.601476 | — | — | — |
| copy.test | COMPILE | 0.276614 | 0.608327 | 0.735712 | — | — | — |
| copy_if.test | COMPILE | 0.251296 | 0.535833 | 0.646589 | — | — | — |
| count.test | COMPILE | 0.241382 | 0.496683 | 0.619278 | — | — | — |
| count_if.test | COMPILE | 0.251875 | 0.525145 | 0.637864 | — | — | — |
| deque.test | COMPILE | 0.239771 | 0.507873 | 0.607688 | — | — | — |
| distance.test | COMPILE | 0.223350 | 0.460851 | 0.556226 | — | — | — |
| empty.test | COMPILE | 0.198277 | 0.410384 | 0.493576 | — | — | — |
| equal.test | COMPILE | 0.230501 | 0.475458 | 0.579123 | — | — | — |
| erase.test | COMPILE | 0.259755 | 0.564546 | 0.671696 | — | — | — |
| erase_range.test | COMPILE | 0.245413 | 0.527387 | 0.635410 | — | — | — |
| eval_if.test | COMPILE | 0.183492 | 0.388577 | 0.463710 | — | — | — |
| filter_view.test | COMPILE | 0.246386 | 0.525775 | 0.629474 | — | — | — |
| find.test | COMPILE | 0.239700 | 0.505158 | 0.616932 | — | — | — |
| find_if.test | COMPILE | 0.245623 | 0.528146 | 0.617971 | — | — | — |
| fold.test | COMPILE | 0.253964 | 0.536398 | 0.664413 | — | — | — |
| for_each.test | RUN | 0.366055 | 0.784583 | 1.031649 | 0.001519 | 0.001421 | 0.001629 |
| front.test | COMPILE | 0.202244 | 0.420249 | 0.519807 | — | — | — |
| get_tag_def.test | COMPILE | 0.180659 | 0.373075 | 0.442965 | — | — | — |
| has_xxx.test | COMPILE | 0.197230 | 0.397728 | 0.498251 | — | — | — |
| identity.test | COMPILE | 0.202211 | 0.416973 | 0.498078 | — | — | — |
| if.test | COMPILE | 0.178708 | 0.374067 | 0.460500 | — | — | — |
| index_of.test | COMPILE | 0.231202 | 0.481481 | 0.577336 | — | — | — |
| inherit.test | COMPILE | 0.193730 | 0.384072 | 0.466738 | — | — | — |
| insert.test | COMPILE | 0.271637 | 0.567697 | 0.682983 | — | — | — |
| insert_range.test | COMPILE | 0.350250 | 0.774359 | 1.006375 | — | — | — |
| int.test | RUN | 0.189068 | 0.403369 | 0.491827 | 0.001657 | 0.001619 | 0.001609 |
| char.test | RUN | 0.191654 | 0.426826 | 0.502833 | 0.001825 | 0.001489 | 0.001658 |
| char_unsigned.test | RUN | 0.192109 | 0.408017 | 0.487415 | 0.001721 | 0.001682 | 0.001649 |
| integral_c.test | RUN | 0.202995 | 0.535765 | 0.615883 | 0.001635 | 0.001650 | 0.001653 |
| integral_c_unsigned.test | RUN | 0.204154 | 0.532903 | 0.599552 | 0.001502 | 0.001545 | 0.001444 |
| is_placeholder.test | COMPILE | 0.179942 | 0.379353 | 0.459306 | — | — | — |
| is_sequence.test | COMPILE | 0.214945 | 0.453072 | 0.537097 | — | — | — |
| iterator_tags.test | COMPILE | 0.189831 | 0.380595 | 0.466713 | — | — | — |
| joint_view.test | COMPILE | 0.242591 | 0.515060 | 0.622463 | — | — | — |
| lambda.test | COMPILE | 0.227270 | 0.476942 | 0.577791 | — | — | — |
| lambda_args.test | COMPILE | 0.217435 | 0.450872 | 0.579109 | — | — | — |
| list.test | COMPILE | 0.221983 | 0.472978 | 0.569229 | — | — | — |
| list_c.test | COMPILE | 0.223949 | 0.463451 | 0.586781 | — | — | — |
| logical.test | COMPILE | 0.183528 | 0.370020 | 0.463801 | — | — | — |
| lower_bound.test | COMPILE | 0.233668 | 0.482580 | 0.586414 | — | — | — |
| map.test | COMPILE | 0.270162 | 0.602846 | 0.729095 | — | — | — |
| max_element.test | COMPILE | 0.222191 | 0.463411 | 0.561863 | — | — | — |
| min_max.test | COMPILE | 0.190426 | 0.384347 | 0.470382 | — | — | — |
| multiset.test | COMPILE | 0.238107 | 0.509147 | 0.612461 | — | — | — |
| next.test | COMPILE | 0.180380 | 0.366673 | 0.449511 | — | — | — |
| no_has_xxx.test | COMPILE | 0.198259 | 0.402577 | 0.504818 | — | — | — |
| numeric_ops.test | COMPILE | 0.216645 | 0.443664 | 0.552349 | — | — | — |
| pair_view.test | COMPILE | 0.243090 | 0.510851 | 0.611138 | — | — | — |
| partition.test | COMPILE | 0.264982 | 0.563726 | 0.680283 | — | — | — |
| pop_front.test | COMPILE | 0.216551 | 0.468664 | 0.557978 | — | — | — |
| push_back.test | COMPILE | 0.188600 | 0.386214 | 0.477954 | — | — | — |
| push_front.test | COMPILE | 0.216544 | 0.456364 | 0.550964 | — | — | — |
| quote.test | COMPILE | 0.186575 | 0.377075 | 0.458124 | — | — | — |
| range_c.test | COMPILE | 0.231954 | 0.496606 | 0.588135 | — | — | — |
| remove.test | COMPILE | 0.238431 | 0.513711 | 0.606972 | — | — | — |
| remove_if.test | COMPILE | 0.247110 | 0.528677 | 0.640436 | — | — | — |
| replace.test | COMPILE | 0.267598 | 0.577606 | 0.690985 | — | — | — |
| replace_if.test | COMPILE | 0.270877 | 0.595269 | 0.698908 | — | — | — |
| reverse.test | COMPILE | 0.255492 | 0.542573 | 0.653206 | — | — | — |
| same_as.test | COMPILE | 0.198200 | 0.400378 | 0.492670 | — | — | — |
| set.test | COMPILE | 0.337166 | 0.778062 | 0.981285 | — | — | — |
| set_c.test | COMPILE | 0.253288 | 0.519951 | 0.653221 | — | — | — |
| single_view.test | COMPILE | 0.230363 | 0.493764 | 0.569729 | — | — | — |
| size.test | COMPILE | 0.217886 | 0.452102 | 0.540314 | — | — | — |
| size_t.test | RUN | 0.196487 | 0.419277 | 0.496688 | 0.001501 | 0.001725 | 0.001582 |
| sizeof.test | COMPILE | 0.183312 | 0.360864 | 0.446266 | — | — | — |
| sort.test | COMPILE | 0.282624 | 0.604015 | 0.721770 | — | — | — |
| stable_partition.test | COMPILE | 0.268704 | 0.567336 | 0.687287 | — | — | — |
| transform.test | COMPILE | 0.283147 | 0.614641 | 0.747699 | — | — | — |
| transform_view.test | COMPILE | 0.235703 | 0.483532 | 0.581322 | — | — | — |
| unique.test | COMPILE | 0.245036 | 0.518110 | 0.619834 | — | — | — |
| unpack_args.test | COMPILE | 0.232740 | 0.476323 | 0.566518 | — | — | — |
| upper_bound.test | COMPILE | 0.238281 | 0.480265 | 0.577073 | — | — | — |
| vector.test | COMPILE | 0.263266 | 0.583470 | 0.695143 | — | — | — |
| vector_c.test | COMPILE | 0.233074 | 0.483251 | 0.596615 | — | — | — |
| zip_view.test | COMPILE | 0.326046 | 0.705402 | 0.870746 | — | — | — |
| string.test | RUN | 0.466536 | 1.303206 | 1.233926 | 0.001217 | 0.001133 | 0.001134 |
| issue_69.test | RUN | 0.021662 | 0.053557 | 0.050368 | 0.000527 | 0.000684 | 0.001450 |
| issue_84.test | RUN | 0.051327 | 0.103524 | 0.101071 | 0.000583 | 0.000541 | 0.001127 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.998× | 1.022×, 0.974× | 1.006× |
| clang | 1.108× | 1.096×, 1.119× | 1.030× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| bool.test | 10 | 0.000998s | 0.000961s | 0.000997s | byte equality observed |
| char.test | 10 | 0.000996s | 0.001040s | 0.001002s | byte equality observed |
| char_unsigned.test | 10 | 0.000987s | 0.000972s | 0.000985s | byte equality observed |
| for_each.test | 10 | 0.000977s | 0.001006s | 0.000977s | byte equality observed |
| int.test | 10 | 0.001008s | 0.000971s | 0.000989s | byte equality observed |
| integral_c.test | 10 | 0.000985s | 0.001007s | 0.000988s | byte equality observed |
| integral_c_unsigned.test | 10 | 0.000998s | 0.001024s | 0.000984s | byte equality observed |
| issue_69.test | 10 | 0.000454s | 0.000455s | 0.001030s | byte equality observed |
| issue_84.test | 10 | 0.000447s | 0.000449s | 0.000957s | byte equality observed |
| size_t.test | 10 | 0.001020s | 0.000972s | 0.001000s | byte equality observed |
| string.test | 10 | 0.000988s | 0.000975s | 0.001015s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
