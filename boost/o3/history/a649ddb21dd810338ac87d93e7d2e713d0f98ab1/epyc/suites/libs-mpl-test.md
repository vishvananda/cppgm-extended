# libs/mpl/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| increased_arity.test | COMPILE | 0.496396 | 1.401030 | 1.549249 | — | — | — |
| largest_int.test | COMPILE | 0.167098 | 0.386335 | 0.574646 | — | — | — |
| msvc_is_class.test | COMPILE | 0.019537 | 0.046969 | 0.057921 | — | — | — |
| template_arity.test | COMPILE | 0.159173 | 0.385655 | 0.550562 | — | — | — |
| is_seq.test | COMPILE | 0.006609 | 0.017931 | 0.029395 | — | — | — |
| token_equal.test | COMPILE | 0.006195 | 0.018542 | 0.034009 | — | — | — |
| advance.test | COMPILE | 0.183514 | 0.413697 | 0.642671 | — | — | — |
| always.test | COMPILE | 0.183978 | 0.442126 | 0.646914 | — | — | — |
| apply.test | COMPILE | 0.202073 | 0.468157 | 0.678389 | — | — | — |
| apply_wrap.test | COMPILE | 0.173817 | 0.403719 | 0.589900 | — | — | — |
| arithmetic.test | COMPILE | 0.186655 | 0.447696 | 0.654213 | — | — | — |
| as_sequence.test | COMPILE | 0.209375 | 0.490266 | 0.669543 | — | — | — |
| assert.test | COMPILE | 0.047377 | 0.094663 | 0.121496 | — | — | — |
| at.test | COMPILE | 0.199848 | 0.478774 | 0.677328 | — | — | — |
| back.test | COMPILE | 0.189909 | 0.458651 | 0.661669 | — | — | — |
| bind.test | COMPILE | 0.184430 | 0.441775 | 0.640471 | — | — | — |
| bitwise.test | COMPILE | 0.190262 | 0.451187 | 0.648550 | — | — | — |
| bool.test | RUN | 0.170484 | 0.399922 | 0.579581 | 0.002042 | 0.002182 | 0.001733 |
| comparison.test | COMPILE | 0.182968 | 0.449987 | 0.638578 | — | — | — |
| contains.test | COMPILE | 0.223164 | 0.522421 | 0.768716 | — | — | — |
| copy.test | COMPILE | 0.277888 | 0.648157 | 0.887345 | — | — | — |
| copy_if.test | COMPILE | 0.243966 | 0.563163 | 0.828274 | — | — | — |
| count.test | COMPILE | 0.230632 | 0.535554 | 0.777476 | — | — | — |
| count_if.test | COMPILE | 0.238988 | 0.565454 | 0.820120 | — | — | — |
| deque.test | COMPILE | 0.234221 | 0.558757 | 0.769901 | — | — | — |
| distance.test | COMPILE | 0.212717 | 0.490445 | 0.721090 | — | — | — |
| empty.test | COMPILE | 0.187214 | 0.437405 | 0.638963 | — | — | — |
| equal.test | COMPILE | 0.216286 | 0.503436 | 0.754432 | — | — | — |
| erase.test | COMPILE | 0.251218 | 0.608325 | 0.854415 | — | — | — |
| erase_range.test | COMPILE | 0.245228 | 0.578834 | 0.839981 | — | — | — |
| eval_if.test | COMPILE | 0.174364 | 0.400961 | 0.588138 | — | — | — |
| filter_view.test | COMPILE | 0.238575 | 0.558010 | 0.800653 | — | — | — |
| find.test | COMPILE | 0.230594 | 0.558216 | 0.794251 | — | — | — |
| find_if.test | COMPILE | 0.239901 | 0.550167 | 0.812222 | — | — | — |
| fold.test | COMPILE | 0.251802 | 0.592963 | 0.844725 | — | — | — |
| for_each.test | RUN | 0.361657 | 0.833799 | 1.315468 | 0.001856 | 0.002088 | 0.001942 |
| front.test | COMPILE | 0.192212 | 0.479766 | 0.656936 | — | — | — |
| get_tag_def.test | COMPILE | 0.177245 | 0.391505 | 0.601714 | — | — | — |
| has_xxx.test | COMPILE | 0.192812 | 0.437442 | 0.682212 | — | — | — |
| identity.test | COMPILE | 0.188549 | 0.458390 | 0.667878 | — | — | — |
| if.test | COMPILE | 0.167089 | 0.413201 | 0.606373 | — | — | — |
| index_of.test | COMPILE | 0.218642 | 0.526916 | 0.744410 | — | — | — |
| inherit.test | COMPILE | 0.177389 | 0.413061 | 0.616978 | — | — | — |
| insert.test | COMPILE | 0.249893 | 0.611215 | 0.879504 | — | — | — |
| insert_range.test | COMPILE | 0.339809 | 0.833262 | 1.272885 | — | — | — |
| int.test | RUN | 0.178538 | 0.447975 | 0.651164 | 0.002016 | 0.001947 | 0.001739 |
| char.test | RUN | 0.181750 | 0.462438 | 0.652908 | 0.001854 | 0.001962 | 0.001850 |
| char_unsigned.test | RUN | 0.179383 | 0.455928 | 0.654141 | 0.001930 | 0.002340 | 0.001923 |
| integral_c.test | RUN | 0.202969 | 0.583638 | 0.787904 | 0.002136 | 0.001933 | 0.001954 |
| integral_c_unsigned.test | RUN | 0.208410 | 0.582509 | 0.756914 | 0.001871 | 0.001832 | 0.001898 |
| is_placeholder.test | COMPILE | 0.172096 | 0.418599 | 0.623797 | — | — | — |
| is_sequence.test | COMPILE | 0.204798 | 0.492096 | 0.674786 | — | — | — |
| iterator_tags.test | COMPILE | 0.173065 | 0.435336 | 0.585212 | — | — | — |
| joint_view.test | COMPILE | 0.233288 | 0.554260 | 0.802164 | — | — | — |
| lambda.test | COMPILE | 0.213940 | 0.513038 | 0.744247 | — | — | — |
| lambda_args.test | COMPILE | 0.201902 | 0.483753 | 0.764854 | — | — | — |
| list.test | COMPILE | 0.211194 | 0.528987 | 0.733511 | — | — | — |
| list_c.test | COMPILE | 0.212790 | 0.514216 | 0.747359 | — | — | — |
| logical.test | COMPILE | 0.170474 | 0.410223 | 0.586344 | — | — | — |
| lower_bound.test | COMPILE | 0.220349 | 0.511423 | 0.757989 | — | — | — |
| map.test | COMPILE | 0.261080 | 0.663186 | 0.935557 | — | — | — |
| max_element.test | COMPILE | 0.210188 | 0.497114 | 0.712975 | — | — | — |
| min_max.test | COMPILE | 0.181407 | 0.429332 | 0.617842 | — | — | — |
| multiset.test | COMPILE | 0.230138 | 0.556715 | 0.798861 | — | — | — |
| next.test | COMPILE | 0.169839 | 0.406634 | 0.592016 | — | — | — |
| no_has_xxx.test | COMPILE | 0.183339 | 0.438319 | 0.631022 | — | — | — |
| numeric_ops.test | COMPILE | 0.202057 | 0.494936 | 0.697333 | — | — | — |
| pair_view.test | COMPILE | 0.235131 | 0.569087 | 0.799811 | — | — | — |
| partition.test | COMPILE | 0.250619 | 0.613857 | 0.874065 | — | — | — |
| pop_front.test | COMPILE | 0.208517 | 0.520559 | 0.722174 | — | — | — |
| push_back.test | COMPILE | 0.175826 | 0.429215 | 0.631443 | — | — | — |
| push_front.test | COMPILE | 0.211029 | 0.507774 | 0.711965 | — | — | — |
| quote.test | COMPILE | 0.173734 | 0.415229 | 0.588870 | — | — | — |
| range_c.test | COMPILE | 0.213216 | 0.528543 | 0.757203 | — | — | — |
| remove.test | COMPILE | 0.227514 | 0.560487 | 0.782433 | — | — | — |
| remove_if.test | COMPILE | 0.234971 | 0.578268 | 0.809337 | — | — | — |
| replace.test | COMPILE | 0.257046 | 0.638356 | 0.863408 | — | — | — |
| replace_if.test | COMPILE | 0.249788 | 0.605689 | 0.883171 | — | — | — |
| reverse.test | COMPILE | 0.235310 | 0.563259 | 0.841276 | — | — | — |
| same_as.test | COMPILE | 0.183701 | 0.427273 | 0.631525 | — | — | — |
| set.test | COMPILE | 0.329950 | 0.867929 | 1.234635 | — | — | — |
| set_c.test | COMPILE | 0.231197 | 0.556015 | 0.836942 | — | — | — |
| single_view.test | COMPILE | 0.211706 | 0.522681 | 0.754350 | — | — | — |
| size.test | COMPILE | 0.210931 | 0.504371 | 0.693524 | — | — | — |
| size_t.test | RUN | 0.178796 | 0.467970 | 0.630643 | 0.001914 | 0.002080 | 0.001980 |
| sizeof.test | COMPILE | 0.167531 | 0.409955 | 0.583043 | — | — | — |
| sort.test | COMPILE | 0.268359 | 0.662562 | 0.940139 | — | — | — |
| stable_partition.test | COMPILE | 0.252997 | 0.625470 | 0.869702 | — | — | — |
| transform.test | COMPILE | 0.267733 | 0.664411 | 0.944722 | — | — | — |
| transform_view.test | COMPILE | 0.222205 | 0.545957 | 0.758514 | — | — | — |
| unique.test | COMPILE | 0.226917 | 0.557308 | 0.779188 | — | — | — |
| unpack_args.test | COMPILE | 0.210508 | 0.513964 | 0.745908 | — | — | — |
| upper_bound.test | COMPILE | 0.218034 | 0.515897 | 0.751075 | — | — | — |
| vector.test | COMPILE | 0.248993 | 0.638387 | 0.897567 | — | — | — |
| vector_c.test | COMPILE | 0.220918 | 0.506029 | 0.767460 | — | — | — |
| zip_view.test | COMPILE | 0.308098 | 0.764475 | 1.075002 | — | — | — |
| string.test | RUN | 0.474783 | 1.412935 | 1.422774 | 0.001526 | 0.001680 | 0.001492 |
| issue_69.test | RUN | 0.015130 | 0.057971 | 0.069320 | 0.000715 | 0.000780 | 0.001837 |
| issue_84.test | RUN | 0.039822 | 0.125179 | 0.137714 | 0.000684 | 0.000616 | 0.001352 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.014× | 1.026×, 1.002× | 1.012× |
| clang | 1.085× | 1.088×, 1.083× | 0.999× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274616).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| bool.test | 10 | 0.001248s | 0.001329s | 0.001262s | byte equality observed |
| char.test | 10 | 0.001226s | 0.001277s | 0.001291s | byte equality observed |
| char_unsigned.test | 10 | 0.001236s | 0.001246s | 0.001264s | byte equality observed |
| for_each.test | 10 | 0.001272s | 0.001263s | 0.001259s | byte equality observed |
| int.test | 10 | 0.001261s | 0.001257s | 0.001276s | byte equality observed |
| integral_c.test | 10 | 0.001262s | 0.001232s | 0.001214s | byte equality observed |
| integral_c_unsigned.test | 10 | 0.001277s | 0.001224s | 0.001223s | byte equality observed |
| issue_69.test | 10 | 0.000590s | 0.000582s | 0.001196s | byte equality observed |
| issue_84.test | 10 | 0.000582s | 0.000575s | 0.001183s | byte equality observed |
| size_t.test | 10 | 0.001258s | 0.001274s | 0.001211s | byte equality observed |
| string.test | 10 | 0.001237s | 0.001271s | 0.001265s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274616).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
