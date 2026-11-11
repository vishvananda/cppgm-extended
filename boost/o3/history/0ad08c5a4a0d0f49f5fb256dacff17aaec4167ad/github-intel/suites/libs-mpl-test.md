# libs/mpl/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| increased_arity.test | COMPILE | 0.512506 | 1.241414 | 1.179576 | — | — | — |
| largest_int.test | COMPILE | 0.172144 | 0.338513 | 0.413742 | — | — | — |
| msvc_is_class.test | COMPILE | 0.019329 | 0.042768 | 0.046737 | — | — | — |
| template_arity.test | COMPILE | 0.170260 | 0.333248 | 0.411923 | — | — | — |
| is_seq.test | COMPILE | 0.005952 | 0.015044 | 0.027860 | — | — | — |
| token_equal.test | COMPILE | 0.006216 | 0.015693 | 0.028095 | — | — | — |
| advance.test | COMPILE | 0.184757 | 0.381100 | 0.443578 | — | — | — |
| always.test | COMPILE | 0.188674 | 0.388967 | 0.478657 | — | — | — |
| apply.test | COMPILE | 0.198790 | 0.394830 | 0.502033 | — | — | — |
| apply_wrap.test | COMPILE | 0.172709 | 0.345614 | 0.415248 | — | — | — |
| arithmetic.test | COMPILE | 0.189295 | 0.396524 | 0.451933 | — | — | — |
| as_sequence.test | COMPILE | 0.200556 | 0.429672 | 0.493264 | — | — | — |
| assert.test | COMPILE | 0.049275 | 0.079298 | 0.089426 | — | — | — |
| at.test | COMPILE | 0.195553 | 0.399260 | 0.501382 | — | — | — |
| back.test | COMPILE | 0.192620 | 0.399395 | 0.482705 | — | — | — |
| bind.test | COMPILE | 0.186087 | 0.379838 | 0.447864 | — | — | — |
| bitwise.test | COMPILE | 0.186349 | 0.390352 | 0.471207 | — | — | — |
| bool.test | RUN | 0.166169 | 0.351015 | 0.412263 | 0.001459 | 0.001615 | 0.001754 |
| comparison.test | COMPILE | 0.181589 | 0.400171 | 0.458550 | — | — | — |
| contains.test | COMPILE | 0.224075 | 0.470893 | 0.550648 | — | — | — |
| copy.test | COMPILE | 0.255832 | 0.569760 | 0.659307 | — | — | — |
| copy_if.test | COMPILE | 0.231908 | 0.509900 | 0.609494 | — | — | — |
| count.test | COMPILE | 0.220670 | 0.474282 | 0.578774 | — | — | — |
| count_if.test | COMPILE | 0.233825 | 0.499075 | 0.594220 | — | — | — |
| deque.test | COMPILE | 0.220611 | 0.475074 | 0.583526 | — | — | — |
| distance.test | COMPILE | 0.207093 | 0.438381 | 0.516806 | — | — | — |
| empty.test | COMPILE | 0.179693 | 0.383425 | 0.456150 | — | — | — |
| equal.test | COMPILE | 0.210612 | 0.461546 | 0.520857 | — | — | — |
| erase.test | COMPILE | 0.237818 | 0.518464 | 0.618142 | — | — | — |
| erase_range.test | COMPILE | 0.226520 | 0.499785 | 0.581654 | — | — | — |
| eval_if.test | COMPILE | 0.170040 | 0.352207 | 0.423508 | — | — | — |
| filter_view.test | COMPILE | 0.230825 | 0.505397 | 0.573164 | — | — | — |
| find.test | COMPILE | 0.222902 | 0.470208 | 0.563370 | — | — | — |
| find_if.test | COMPILE | 0.229840 | 0.487780 | 0.567919 | — | — | — |
| fold.test | COMPILE | 0.241740 | 0.510810 | 0.609872 | — | — | — |
| for_each.test | RUN | 0.345062 | 0.741218 | 0.941663 | 0.001808 | 0.001763 | 0.001771 |
| front.test | COMPILE | 0.188827 | 0.402545 | 0.474451 | — | — | — |
| get_tag_def.test | COMPILE | 0.170720 | 0.346041 | 0.409936 | — | — | — |
| has_xxx.test | COMPILE | 0.189775 | 0.373002 | 0.456907 | — | — | — |
| identity.test | COMPILE | 0.193421 | 0.386514 | 0.452154 | — | — | — |
| if.test | COMPILE | 0.171515 | 0.339057 | 0.406652 | — | — | — |
| index_of.test | COMPILE | 0.214699 | 0.450742 | 0.528723 | — | — | — |
| inherit.test | COMPILE | 0.180258 | 0.350503 | 0.425341 | — | — | — |
| insert.test | COMPILE | 0.248026 | 0.535253 | 0.632305 | — | — | — |
| insert_range.test | COMPILE | 0.332543 | 0.721788 | 0.912835 | — | — | — |
| int.test | RUN | 0.179902 | 0.387378 | 0.454259 | 0.001546 | 0.001673 | 0.001707 |
| char.test | RUN | 0.181618 | 0.393919 | 0.465392 | 0.001601 | 0.001876 | 0.001792 |
| char_unsigned.test | RUN | 0.183497 | 0.384317 | 0.447943 | 0.001731 | 0.001788 | 0.001727 |
| integral_c.test | RUN | 0.193562 | 0.497691 | 0.560473 | 0.001671 | 0.001677 | 0.001585 |
| integral_c_unsigned.test | RUN | 0.190220 | 0.498632 | 0.566285 | 0.001505 | 0.001754 | 0.001770 |
| is_placeholder.test | COMPILE | 0.172114 | 0.340244 | 0.428343 | — | — | — |
| is_sequence.test | COMPILE | 0.205792 | 0.417471 | 0.497734 | — | — | — |
| iterator_tags.test | COMPILE | 0.175327 | 0.361980 | 0.419823 | — | — | — |
| joint_view.test | COMPILE | 0.233908 | 0.481908 | 0.572005 | — | — | — |
| lambda.test | COMPILE | 0.213802 | 0.447106 | 0.537384 | — | — | — |
| lambda_args.test | COMPILE | 0.201428 | 0.422797 | 0.529007 | — | — | — |
| list.test | COMPILE | 0.207379 | 0.452329 | 0.528132 | — | — | — |
| list_c.test | COMPILE | 0.212458 | 0.436216 | 0.536945 | — | — | — |
| logical.test | COMPILE | 0.169346 | 0.353469 | 0.420042 | — | — | — |
| lower_bound.test | COMPILE | 0.221121 | 0.457512 | 0.534770 | — | — | — |
| map.test | COMPILE | 0.252313 | 0.571671 | 0.684933 | — | — | — |
| max_element.test | COMPILE | 0.211513 | 0.439159 | 0.525193 | — | — | — |
| min_max.test | COMPILE | 0.177487 | 0.368435 | 0.445250 | — | — | — |
| multiset.test | COMPILE | 0.226733 | 0.487492 | 0.573462 | — | — | — |
| next.test | COMPILE | 0.171983 | 0.347764 | 0.418616 | — | — | — |
| no_has_xxx.test | COMPILE | 0.183343 | 0.368795 | 0.452294 | — | — | — |
| numeric_ops.test | COMPILE | 0.203440 | 0.426285 | 0.504424 | — | — | — |
| pair_view.test | COMPILE | 0.229923 | 0.494071 | 0.569449 | — | — | — |
| partition.test | COMPILE | 0.246496 | 0.531585 | 0.637723 | — | — | — |
| pop_front.test | COMPILE | 0.202506 | 0.440598 | 0.510398 | — | — | — |
| push_back.test | COMPILE | 0.176223 | 0.357758 | 0.439246 | — | — | — |
| push_front.test | COMPILE | 0.204718 | 0.436191 | 0.513497 | — | — | — |
| quote.test | COMPILE | 0.169763 | 0.345099 | 0.426988 | — | — | — |
| range_c.test | COMPILE | 0.215913 | 0.461597 | 0.535985 | — | — | — |
| remove.test | COMPILE | 0.223555 | 0.481460 | 0.570516 | — | — | — |
| remove_if.test | COMPILE | 0.231263 | 0.499419 | 0.585233 | — | — | — |
| replace.test | COMPILE | 0.247639 | 0.546639 | 0.636935 | — | — | — |
| replace_if.test | COMPILE | 0.250068 | 0.560972 | 0.642204 | — | — | — |
| reverse.test | COMPILE | 0.240461 | 0.513921 | 0.605167 | — | — | — |
| same_as.test | COMPILE | 0.184319 | 0.376762 | 0.450240 | — | — | — |
| set.test | COMPILE | 0.308143 | 0.720814 | 0.908767 | — | — | — |
| set_c.test | COMPILE | 0.231003 | 0.493097 | 0.606189 | — | — | — |
| single_view.test | COMPILE | 0.214120 | 0.466662 | 0.548942 | — | — | — |
| size.test | COMPILE | 0.204525 | 0.427160 | 0.503539 | — | — | — |
| size_t.test | RUN | 0.182149 | 0.390552 | 0.461086 | 0.001508 | 0.001791 | 0.001768 |
| sizeof.test | COMPILE | 0.168247 | 0.338667 | 0.404025 | — | — | — |
| sort.test | COMPILE | 0.263641 | 0.569845 | 0.662064 | — | — | — |
| stable_partition.test | COMPILE | 0.246157 | 0.533569 | 0.619683 | — | — | — |
| transform.test | COMPILE | 0.260295 | 0.586764 | 0.676944 | — | — | — |
| transform_view.test | COMPILE | 0.218253 | 0.466112 | 0.543885 | — | — | — |
| unique.test | COMPILE | 0.228154 | 0.489540 | 0.554806 | — | — | — |
| unpack_args.test | COMPILE | 0.213233 | 0.454576 | 0.525904 | — | — | — |
| upper_bound.test | COMPILE | 0.217257 | 0.455714 | 0.534595 | — | — | — |
| vector.test | COMPILE | 0.246176 | 0.555070 | 0.634229 | — | — | — |
| vector_c.test | COMPILE | 0.221319 | 0.456617 | 0.556840 | — | — | — |
| zip_view.test | COMPILE | 0.302119 | 0.684511 | 0.793863 | — | — | — |
| string.test | RUN | 0.463375 | 1.290560 | 1.181597 | 0.001377 | 0.001312 | 0.001254 |
| issue_69.test | RUN | 0.019745 | 0.051844 | 0.052989 | 0.000779 | 0.000752 | 0.001734 |
| issue_84.test | RUN | 0.039049 | 0.100600 | 0.097045 | 0.000632 | 0.000602 | 0.001296 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.995× | 1.015×, 0.974× | 0.989× |
| clang | 1.110× | 1.107×, 1.114× | 0.989× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| bool.test | 10 | 0.001054s | 0.001025s | 0.001044s | byte equality observed |
| char.test | 10 | 0.001031s | 0.001025s | 0.001052s | byte equality observed |
| char_unsigned.test | 10 | 0.001048s | 0.001071s | 0.001061s | byte equality observed |
| for_each.test | 10 | 0.001056s | 0.001059s | 0.001067s | byte equality observed |
| int.test | 10 | 0.001049s | 0.001023s | 0.001051s | byte equality observed |
| integral_c.test | 10 | 0.001034s | 0.001040s | 0.001042s | byte equality observed |
| integral_c_unsigned.test | 10 | 0.001075s | 0.001058s | 0.001067s | byte equality observed |
| issue_69.test | 10 | 0.000473s | 0.000493s | 0.000987s | byte equality observed |
| issue_84.test | 10 | 0.000470s | 0.000477s | 0.001041s | byte equality observed |
| size_t.test | 10 | 0.001076s | 0.001018s | 0.001036s | byte equality observed |
| string.test | 10 | 0.001043s | 0.001069s | 0.001074s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
