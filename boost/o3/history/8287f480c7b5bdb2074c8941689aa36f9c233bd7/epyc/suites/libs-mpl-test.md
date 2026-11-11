# libs/mpl/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| increased_arity.test | COMPILE | 0.437337 | 0.999760 | 1.025819 | — | — | — |
| largest_int.test | COMPILE | 0.143609 | 0.273667 | 0.367257 | — | — | — |
| msvc_is_class.test | COMPILE | 0.017010 | 0.036511 | 0.041326 | — | — | — |
| template_arity.test | COMPILE | 0.145212 | 0.274522 | 0.357816 | — | — | — |
| is_seq.test | COMPILE | 0.005716 | 0.013571 | 0.027733 | — | — | — |
| token_equal.test | COMPILE | 0.005914 | 0.013978 | 0.026829 | — | — | — |
| advance.test | COMPILE | 0.160336 | 0.301971 | 0.394744 | — | — | — |
| always.test | COMPILE | 0.159027 | 0.323459 | 0.437952 | — | — | — |
| apply.test | COMPILE | 0.173187 | 0.340318 | 0.450031 | — | — | — |
| apply_wrap.test | COMPILE | 0.150450 | 0.285438 | 0.389077 | — | — | — |
| arithmetic.test | COMPILE | 0.162071 | 0.326768 | 0.422394 | — | — | — |
| as_sequence.test | COMPILE | 0.176721 | 0.351314 | 0.420086 | — | — | — |
| assert.test | COMPILE | 0.045419 | 0.071493 | 0.086270 | — | — | — |
| at.test | COMPILE | 0.171270 | 0.345850 | 0.463643 | — | — | — |
| back.test | COMPILE | 0.165699 | 0.334125 | 0.442696 | — | — | — |
| bind.test | COMPILE | 0.164250 | 0.314821 | 0.412167 | — | — | — |
| bitwise.test | COMPILE | 0.166116 | 0.321704 | 0.423246 | — | — | — |
| bool.test | RUN | 0.146862 | 0.280920 | 0.399601 | 0.001360 | 0.001658 | 0.001638 |
| comparison.test | COMPILE | 0.160486 | 0.317139 | 0.431283 | — | — | — |
| contains.test | COMPILE | 0.192262 | 0.388257 | 0.478312 | — | — | — |
| copy.test | COMPILE | 0.229542 | 0.468762 | 0.594924 | — | — | — |
| copy_if.test | COMPILE | 0.203269 | 0.410377 | 0.522822 | — | — | — |
| count.test | COMPILE | 0.193196 | 0.382908 | 0.519671 | — | — | — |
| count_if.test | COMPILE | 0.204396 | 0.408476 | 0.539664 | — | — | — |
| deque.test | COMPILE | 0.199386 | 0.391725 | 0.498986 | — | — | — |
| distance.test | COMPILE | 0.182481 | 0.350506 | 0.459159 | — | — | — |
| empty.test | COMPILE | 0.160549 | 0.313655 | 0.413236 | — | — | — |
| equal.test | COMPILE | 0.184678 | 0.374328 | 0.485012 | — | — | — |
| erase.test | COMPILE | 0.211593 | 0.435664 | 0.553056 | — | — | — |
| erase_range.test | COMPILE | 0.199285 | 0.404922 | 0.522508 | — | — | — |
| eval_if.test | COMPILE | 0.150754 | 0.299784 | 0.375170 | — | — | — |
| filter_view.test | COMPILE | 0.199607 | 0.417009 | 0.519439 | — | — | — |
| find.test | COMPILE | 0.193342 | 0.385186 | 0.505329 | — | — | — |
| find_if.test | COMPILE | 0.199520 | 0.408209 | 0.514772 | — | — | — |
| fold.test | COMPILE | 0.206998 | 0.414969 | 0.553409 | — | — | — |
| for_each.test | RUN | 0.290634 | 0.614208 | 0.849761 | 0.001582 | 0.001806 | 0.001692 |
| front.test | COMPILE | 0.164561 | 0.327383 | 0.428932 | — | — | — |
| get_tag_def.test | COMPILE | 0.147246 | 0.288281 | 0.395224 | — | — | — |
| has_xxx.test | COMPILE | 0.162450 | 0.321898 | 0.417245 | — | — | — |
| identity.test | COMPILE | 0.165051 | 0.327661 | 0.418986 | — | — | — |
| if.test | COMPILE | 0.148912 | 0.291350 | 0.353827 | — | — | — |
| index_of.test | COMPILE | 0.187268 | 0.375007 | 0.471408 | — | — | — |
| inherit.test | COMPILE | 0.153297 | 0.297699 | 0.380499 | — | — | — |
| insert.test | COMPILE | 0.215231 | 0.436448 | 0.581173 | — | — | — |
| insert_range.test | COMPILE | 0.284597 | 0.589105 | 0.834609 | — | — | — |
| int.test | RUN | 0.152650 | 0.324966 | 0.413419 | 0.003828 | 0.001705 | 0.001636 |
| char.test | RUN | 0.159694 | 0.322203 | 0.437096 | 0.001602 | 0.001371 | 0.001509 |
| char_unsigned.test | RUN | 0.201575 | 0.321908 | 0.396778 | 0.001854 | 0.001854 | 0.001684 |
| integral_c.test | RUN | 0.209261 | 0.406393 | 0.511755 | 0.001688 | 0.001746 | 0.001643 |
| integral_c_unsigned.test | RUN | 0.171822 | 0.404682 | 0.480728 | 0.001777 | 0.001832 | 0.001664 |
| is_placeholder.test | COMPILE | 0.151816 | 0.292406 | 0.400975 | — | — | — |
| is_sequence.test | COMPILE | 0.177442 | 0.350181 | 0.435633 | — | — | — |
| iterator_tags.test | COMPILE | 0.157875 | 0.296427 | 0.379323 | — | — | — |
| joint_view.test | COMPILE | 0.198455 | 0.399193 | 0.522816 | — | — | — |
| lambda.test | COMPILE | 0.184043 | 0.366148 | 0.472878 | — | — | — |
| lambda_args.test | COMPILE | 0.172310 | 0.355724 | 0.491801 | — | — | — |
| list.test | COMPILE | 0.182878 | 0.380562 | 0.474431 | — | — | — |
| list_c.test | COMPILE | 0.183726 | 0.359941 | 0.484846 | — | — | — |
| logical.test | COMPILE | 0.149691 | 0.290994 | 0.380573 | — | — | — |
| lower_bound.test | COMPILE | 0.190818 | 0.351256 | 0.492645 | — | — | — |
| map.test | COMPILE | 0.218463 | 0.467205 | 0.602700 | — | — | — |
| max_element.test | COMPILE | 0.181363 | 0.354611 | 0.458510 | — | — | — |
| min_max.test | COMPILE | 0.160584 | 0.303748 | 0.405173 | — | — | — |
| multiset.test | COMPILE | 0.194098 | 0.404399 | 0.508265 | — | — | — |
| next.test | COMPILE | 0.149703 | 0.297424 | 0.388658 | — | — | — |
| no_has_xxx.test | COMPILE | 0.159764 | 0.304079 | 0.426832 | — | — | — |
| numeric_ops.test | COMPILE | 0.180221 | 0.361532 | 0.437506 | — | — | — |
| pair_view.test | COMPILE | 0.200595 | 0.395610 | 0.524402 | — | — | — |
| partition.test | COMPILE | 0.215957 | 0.449991 | 0.572998 | — | — | — |
| pop_front.test | COMPILE | 0.180387 | 0.352139 | 0.471928 | — | — | — |
| push_back.test | COMPILE | 0.156460 | 0.307509 | 0.393352 | — | — | — |
| push_front.test | COMPILE | 0.180114 | 0.355557 | 0.475572 | — | — | — |
| quote.test | COMPILE | 0.150207 | 0.282510 | 0.394396 | — | — | — |
| range_c.test | COMPILE | 0.189398 | 0.396330 | 0.497856 | — | — | — |
| remove.test | COMPILE | 0.197124 | 0.406841 | 0.518528 | — | — | — |
| remove_if.test | COMPILE | 0.201380 | 0.417537 | 0.535292 | — | — | — |
| replace.test | COMPILE | 0.216798 | 0.452503 | 0.574981 | — | — | — |
| replace_if.test | COMPILE | 0.222280 | 0.453293 | 0.583512 | — | — | — |
| reverse.test | COMPILE | 0.207642 | 0.419149 | 0.549920 | — | — | — |
| same_as.test | COMPILE | 0.161242 | 0.303963 | 0.403204 | — | — | — |
| set.test | COMPILE | 0.280196 | 0.594419 | 0.831543 | — | — | — |
| set_c.test | COMPILE | 0.207510 | 0.410080 | 0.544375 | — | — | — |
| single_view.test | COMPILE | 0.194370 | 0.372806 | 0.485328 | — | — | — |
| size.test | COMPILE | 0.179525 | 0.358841 | 0.466539 | — | — | — |
| size_t.test | RUN | 0.161080 | 0.317938 | 0.438694 | 0.001568 | 0.001787 | 0.001641 |
| sizeof.test | COMPILE | 0.149077 | 0.281240 | 0.398360 | — | — | — |
| sort.test | COMPILE | 0.230264 | 0.469322 | 0.613429 | — | — | — |
| stable_partition.test | COMPILE | 0.217279 | 0.441455 | 0.582199 | — | — | — |
| transform.test | COMPILE | 0.228600 | 0.475795 | 0.615768 | — | — | — |
| transform_view.test | COMPILE | 0.187111 | 0.374262 | 0.504110 | — | — | — |
| unique.test | COMPILE | 0.199555 | 0.406099 | 0.520548 | — | — | — |
| unpack_args.test | COMPILE | 0.185577 | 0.372277 | 0.484080 | — | — | — |
| upper_bound.test | COMPILE | 0.188102 | 0.367721 | 0.491062 | — | — | — |
| vector.test | COMPILE | 0.210837 | 0.438170 | 0.581101 | — | — | — |
| vector_c.test | COMPILE | 0.189991 | 0.377783 | 0.491390 | — | — | — |
| zip_view.test | COMPILE | 0.256615 | 0.554490 | 0.707351 | — | — | — |
| string.test | RUN | 0.398566 | 1.078951 | 0.986248 | 0.001290 | 0.001302 | 0.001208 |
| issue_69.test | RUN | 0.019753 | 0.044694 | 0.051048 | 0.000603 | 0.000647 | 0.001624 |
| issue_84.test | RUN | 0.052992 | 0.088280 | 0.092826 | 0.000582 | 0.000561 | 0.001248 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.989× | 0.992×, 0.986× | 0.980× |
| clang | 1.140×† | 1.099×, 1.181× | 0.988× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| bool.test | 10 | 0.001030s | 0.000999s | 0.001069s | byte equality observed |
| char.test | 10 | 0.001043s | 0.001017s | 0.001058s | byte equality observed |
| char_unsigned.test | 10 | 0.001044s | 0.001018s | 0.001046s | byte equality observed |
| for_each.test | 10 | 0.001041s | 0.001034s | 0.001045s | byte equality observed |
| int.test | 10 | 0.001037s | 0.001021s | 0.001030s | byte equality observed |
| integral_c.test | 10 | 0.001047s | 0.001039s | 0.001021s | byte equality observed |
| integral_c_unsigned.test | 10 | 0.001043s | 0.001037s | 0.001050s | byte equality observed |
| issue_69.test | 10 | 0.000486s | 0.000500s | 0.001012s | byte equality observed |
| issue_84.test | 10 | 0.000481s | 0.000488s | 0.001008s | byte equality observed |
| size_t.test | 10 | 0.001028s | 0.001029s | 0.001039s | byte equality observed |
| string.test | 10 | 0.001039s | 0.001048s | 0.001069s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
