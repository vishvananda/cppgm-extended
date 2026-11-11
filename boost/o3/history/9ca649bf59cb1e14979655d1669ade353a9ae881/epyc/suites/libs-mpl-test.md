# libs/mpl/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| increased_arity.test | COMPILE | 0.436830 | 1.016858 | 1.039802 | — | — | — |
| largest_int.test | COMPILE | 0.145976 | 0.280546 | 0.354966 | — | — | — |
| msvc_is_class.test | COMPILE | 0.017483 | 0.037776 | 0.044347 | — | — | — |
| template_arity.test | COMPILE | 0.147814 | 0.284647 | 0.354836 | — | — | — |
| is_seq.test | COMPILE | 0.005915 | 0.014496 | 0.028793 | — | — | — |
| token_equal.test | COMPILE | 0.005700 | 0.014432 | 0.029087 | — | — | — |
| advance.test | COMPILE | 0.159989 | 0.316478 | 0.422942 | — | — | — |
| always.test | COMPILE | 0.159898 | 0.325980 | 0.430481 | — | — | — |
| apply.test | COMPILE | 0.170907 | 0.333460 | 0.438166 | — | — | — |
| apply_wrap.test | COMPILE | 0.149671 | 0.284877 | 0.389148 | — | — | — |
| arithmetic.test | COMPILE | 0.161450 | 0.329994 | 0.397724 | — | — | — |
| as_sequence.test | COMPILE | 0.176869 | 0.353024 | 0.453252 | — | — | — |
| assert.test | COMPILE | 0.045472 | 0.070343 | 0.086045 | — | — | — |
| at.test | COMPILE | 0.171043 | 0.349455 | 0.449737 | — | — | — |
| back.test | COMPILE | 0.165960 | 0.332772 | 0.426702 | — | — | — |
| bind.test | COMPILE | 0.161461 | 0.314614 | 0.420676 | — | — | — |
| bitwise.test | COMPILE | 0.165123 | 0.333085 | 0.431299 | — | — | — |
| bool.test | RUN | 0.151742 | 0.289663 | 0.372566 | 0.001676 | 0.001510 | 0.001734 |
| comparison.test | COMPILE | 0.167880 | 0.326375 | 0.403503 | — | — | — |
| contains.test | COMPILE | 0.196297 | 0.383924 | 0.502455 | — | — | — |
| copy.test | COMPILE | 0.227698 | 0.467418 | 0.623139 | — | — | — |
| copy_if.test | COMPILE | 0.201397 | 0.420038 | 0.539407 | — | — | — |
| count.test | COMPILE | 0.193930 | 0.381137 | 0.515193 | — | — | — |
| count_if.test | COMPILE | 0.205477 | 0.407298 | 0.537973 | — | — | — |
| deque.test | COMPILE | 0.194517 | 0.403534 | 0.502166 | — | — | — |
| distance.test | COMPILE | 0.180722 | 0.364275 | 0.471047 | — | — | — |
| empty.test | COMPILE | 0.159251 | 0.322690 | 0.408061 | — | — | — |
| equal.test | COMPILE | 0.186519 | 0.364915 | 0.484278 | — | — | — |
| erase.test | COMPILE | 0.211594 | 0.438687 | 0.564594 | — | — | — |
| erase_range.test | COMPILE | 0.199009 | 0.413624 | 0.516887 | — | — | — |
| eval_if.test | COMPILE | 0.153326 | 0.298413 | 0.394250 | — | — | — |
| filter_view.test | COMPILE | 0.200127 | 0.406279 | 0.524153 | — | — | — |
| find.test | COMPILE | 0.196315 | 0.392013 | 0.511088 | — | — | — |
| find_if.test | COMPILE | 0.198850 | 0.408039 | 0.529520 | — | — | — |
| fold.test | COMPILE | 0.210882 | 0.413818 | 0.549907 | — | — | — |
| for_each.test | RUN | 0.288310 | 0.621428 | 0.867682 | 0.001760 | 0.001589 | 0.001730 |
| front.test | COMPILE | 0.168203 | 0.335259 | 0.432797 | — | — | — |
| get_tag_def.test | COMPILE | 0.145316 | 0.289792 | 0.392400 | — | — | — |
| has_xxx.test | COMPILE | 0.162380 | 0.319523 | 0.436000 | — | — | — |
| identity.test | COMPILE | 0.168981 | 0.329000 | 0.431834 | — | — | — |
| if.test | COMPILE | 0.148021 | 0.294073 | 0.399261 | — | — | — |
| index_of.test | COMPILE | 0.187083 | 0.369520 | 0.487461 | — | — | — |
| inherit.test | COMPILE | 0.156404 | 0.304600 | 0.408084 | — | — | — |
| insert.test | COMPILE | 0.222650 | 0.434726 | 0.576536 | — | — | — |
| insert_range.test | COMPILE | 0.284773 | 0.609085 | 0.831693 | — | — | — |
| int.test | RUN | 0.157929 | 0.327454 | 0.424858 | 0.001801 | 0.001275 | 0.001525 |
| char.test | RUN | 0.158570 | 0.328738 | 0.428076 | 0.001627 | 0.001744 | 0.001553 |
| char_unsigned.test | RUN | 0.158481 | 0.357388 | 0.410378 | 0.001678 | 0.001896 | 0.001826 |
| integral_c.test | RUN | 0.171355 | 0.418926 | 0.526237 | 0.001718 | 0.001741 | 0.001469 |
| integral_c_unsigned.test | RUN | 0.162072 | 0.396118 | 0.521371 | 0.001698 | 0.001648 | 0.001347 |
| is_placeholder.test | COMPILE | 0.152355 | 0.295673 | 0.356856 | — | — | — |
| is_sequence.test | COMPILE | 0.182344 | 0.356833 | 0.466405 | — | — | — |
| iterator_tags.test | COMPILE | 0.157392 | 0.297639 | 0.410264 | — | — | — |
| joint_view.test | COMPILE | 0.196958 | 0.395746 | 0.525374 | — | — | — |
| lambda.test | COMPILE | 0.186655 | 0.369289 | 0.494183 | — | — | — |
| lambda_args.test | COMPILE | 0.172493 | 0.353252 | 0.487665 | — | — | — |
| list.test | COMPILE | 0.179820 | 0.377177 | 0.488148 | — | — | — |
| list_c.test | COMPILE | 0.181376 | 0.315882 | 0.494211 | — | — | — |
| logical.test | COMPILE | 0.147166 | 0.282996 | 0.390805 | — | — | — |
| lower_bound.test | COMPILE | 0.189511 | 0.377214 | 0.483697 | — | — | — |
| map.test | COMPILE | 0.216489 | 0.472482 | 0.607664 | — | — | — |
| max_element.test | COMPILE | 0.177099 | 0.353617 | 0.468787 | — | — | — |
| min_max.test | COMPILE | 0.157063 | 0.319143 | 0.412983 | — | — | — |
| multiset.test | COMPILE | 0.198183 | 0.402865 | 0.509991 | — | — | — |
| next.test | COMPILE | 0.146545 | 0.289329 | 0.388350 | — | — | — |
| no_has_xxx.test | COMPILE | 0.156133 | 0.299912 | 0.423308 | — | — | — |
| numeric_ops.test | COMPILE | 0.176425 | 0.346116 | 0.465871 | — | — | — |
| pair_view.test | COMPILE | 0.202250 | 0.403699 | 0.515244 | — | — | — |
| partition.test | COMPILE | 0.216785 | 0.429107 | 0.573216 | — | — | — |
| pop_front.test | COMPILE | 0.179887 | 0.361282 | 0.463597 | — | — | — |
| push_back.test | COMPILE | 0.154340 | 0.306030 | 0.405664 | — | — | — |
| push_front.test | COMPILE | 0.178923 | 0.365432 | 0.458406 | — | — | — |
| quote.test | COMPILE | 0.151636 | 0.287546 | 0.373821 | — | — | — |
| range_c.test | COMPILE | 0.188704 | 0.382992 | 0.492080 | — | — | — |
| remove.test | COMPILE | 0.199809 | 0.401732 | 0.512422 | — | — | — |
| remove_if.test | COMPILE | 0.201274 | 0.411455 | 0.531870 | — | — | — |
| replace.test | COMPILE | 0.216190 | 0.454967 | 0.567025 | — | — | — |
| replace_if.test | COMPILE | 0.218219 | 0.457075 | 0.583635 | — | — | — |
| reverse.test | COMPILE | 0.209011 | 0.421410 | 0.547611 | — | — | — |
| same_as.test | COMPILE | 0.162661 | 0.312934 | 0.402033 | — | — | — |
| set.test | COMPILE | 0.268656 | 0.598132 | 0.814565 | — | — | — |
| set_c.test | COMPILE | 0.204109 | 0.407956 | 0.558860 | — | — | — |
| single_view.test | COMPILE | 0.184010 | 0.378126 | 0.472898 | — | — | — |
| size.test | COMPILE | 0.180265 | 0.354358 | 0.457260 | — | — | — |
| size_t.test | RUN | 0.156976 | 0.342036 | 0.416124 | 0.001565 | 0.001827 | 0.001729 |
| sizeof.test | COMPILE | 0.145331 | 0.297519 | 0.365958 | — | — | — |
| sort.test | COMPILE | 0.227212 | 0.475806 | 0.608393 | — | — | — |
| stable_partition.test | COMPILE | 0.217536 | 0.450003 | 0.545518 | — | — | — |
| transform.test | COMPILE | 0.228515 | 0.481966 | 0.613178 | — | — | — |
| transform_view.test | COMPILE | 0.195257 | 0.392972 | 0.487149 | — | — | — |
| unique.test | COMPILE | 0.196778 | 0.402142 | 0.508141 | — | — | — |
| unpack_args.test | COMPILE | 0.182074 | 0.374481 | 0.476231 | — | — | — |
| upper_bound.test | COMPILE | 0.189014 | 0.376298 | 0.486040 | — | — | — |
| vector.test | COMPILE | 0.212460 | 0.462337 | 0.575652 | — | — | — |
| vector_c.test | COMPILE | 0.191571 | 0.370176 | 0.510422 | — | — | — |
| zip_view.test | COMPILE | 0.262663 | 0.562013 | 0.706234 | — | — | — |
| string.test | RUN | 0.406679 | 1.114113 | 0.989523 | 0.001340 | 0.001360 | 0.001300 |
| issue_69.test | RUN | 0.018794 | 0.045380 | 0.051379 | 0.000635 | 0.000696 | 0.001641 |
| issue_84.test | RUN | 0.047963 | 0.089127 | 0.093121 | 0.000561 | 0.000618 | 0.001295 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.050×† | 1.081×, 1.018× | 1.051× |
| clang | 1.098×† | 1.086×, 1.111× | 1.072× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| bool.test | 10 | 0.001020s | 0.001102s | 0.001071s | byte equality observed |
| char.test | 10 | 0.001034s | 0.001072s | 0.001022s | byte equality observed |
| char_unsigned.test | 10 | 0.001040s | 0.001089s | 0.001033s | byte equality observed |
| for_each.test | 10 | 0.001032s | 0.001067s | 0.001047s | byte equality observed |
| int.test | 10 | 0.001039s | 0.001047s | 0.001028s | byte equality observed |
| integral_c.test | 10 | 0.001017s | 0.001053s | 0.001045s | byte equality observed |
| integral_c_unsigned.test | 10 | 0.001048s | 0.001056s | 0.001067s | byte equality observed |
| issue_69.test | 10 | 0.000486s | 0.000484s | 0.001018s | byte equality observed |
| issue_84.test | 10 | 0.000490s | 0.000485s | 0.001007s | byte equality observed |
| size_t.test | 10 | 0.001054s | 0.001038s | 0.001072s | byte equality observed |
| string.test | 10 | 0.001050s | 0.001085s | 0.001040s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
