# libs/mpl/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| increased_arity.test | COMPILE | 0.244577 | 0.718247 | 0.653445 | — | — | — |
| largest_int.test | COMPILE | 0.086182 | 0.199125 | 0.248894 | — | — | — |
| msvc_is_class.test | COMPILE | 0.011563 | 0.028670 | 0.030322 | — | — | — |
| template_arity.test | COMPILE | 0.088143 | 0.198243 | 0.244877 | — | — | — |
| is_seq.test | COMPILE | 0.003988 | 0.010562 | 0.021073 | — | — | — |
| token_equal.test | COMPILE | 0.004082 | 0.010727 | 0.021294 | — | — | — |
| advance.test | COMPILE | 0.094001 | 0.230863 | 0.265315 | — | — | — |
| always.test | COMPILE | 0.098636 | 0.225996 | 0.270493 | — | — | — |
| apply.test | COMPILE | 0.104710 | 0.245202 | 0.289586 | — | — | — |
| apply_wrap.test | COMPILE | 0.090938 | 0.207780 | 0.249709 | — | — | — |
| arithmetic.test | COMPILE | 0.098742 | 0.237120 | 0.264867 | — | — | — |
| as_sequence.test | COMPILE | 0.103961 | 0.259141 | 0.288798 | — | — | — |
| assert.test | COMPILE | 0.025273 | 0.052785 | 0.060223 | — | — | — |
| at.test | COMPILE | 0.104668 | 0.251162 | 0.289748 | — | — | — |
| back.test | COMPILE | 0.099136 | 0.242698 | 0.274302 | — | — | — |
| bind.test | COMPILE | 0.094871 | 0.227280 | 0.263170 | — | — | — |
| bitwise.test | COMPILE | 0.095907 | 0.231752 | 0.274773 | — | — | — |
| bool.test | RUN | 0.090863 | 0.203494 | 0.249384 | 0.001226 | 0.001379 | 0.001438 |
| comparison.test | COMPILE | 0.095814 | 0.231484 | 0.263885 | — | — | — |
| contains.test | COMPILE | 0.116662 | 0.285458 | 0.318937 | — | — | — |
| copy.test | COMPILE | 0.140400 | 0.335595 | 0.396028 | — | — | — |
| copy_if.test | COMPILE | 0.122498 | 0.294904 | 0.345874 | — | — | — |
| count.test | COMPILE | 0.119700 | 0.280505 | 0.330394 | — | — | — |
| count_if.test | COMPILE | 0.122810 | 0.296146 | 0.348090 | — | — | — |
| deque.test | COMPILE | 0.118848 | 0.287178 | 0.333257 | — | — | — |
| distance.test | COMPILE | 0.109661 | 0.255246 | 0.305118 | — | — | — |
| empty.test | COMPILE | 0.097951 | 0.234136 | 0.269449 | — | — | — |
| equal.test | COMPILE | 0.114110 | 0.275883 | 0.309247 | — | — | — |
| erase.test | COMPILE | 0.129881 | 0.314504 | 0.359354 | — | — | — |
| erase_range.test | COMPILE | 0.124730 | 0.294548 | 0.340521 | — | — | — |
| eval_if.test | COMPILE | 0.091260 | 0.212959 | 0.241503 | — | — | — |
| filter_view.test | COMPILE | 0.124091 | 0.304612 | 0.340707 | — | — | — |
| find.test | COMPILE | 0.115836 | 0.281669 | 0.329737 | — | — | — |
| find_if.test | COMPILE | 0.121122 | 0.299098 | 0.336114 | — | — | — |
| fold.test | COMPILE | 0.125338 | 0.303374 | 0.348731 | — | — | — |
| for_each.test | RUN | 0.187276 | 0.443285 | 0.557841 | 0.001244 | 0.001321 | 0.001477 |
| front.test | COMPILE | 0.104765 | 0.240568 | 0.284730 | — | — | — |
| get_tag_def.test | COMPILE | 0.091084 | 0.210393 | 0.246728 | — | — | — |
| has_xxx.test | COMPILE | 0.103218 | 0.230587 | 0.272506 | — | — | — |
| identity.test | COMPILE | 0.101922 | 0.238785 | 0.255509 | — | — | — |
| if.test | COMPILE | 0.088095 | 0.211980 | 0.243015 | — | — | — |
| index_of.test | COMPILE | 0.113618 | 0.262315 | 0.314132 | — | — | — |
| inherit.test | COMPILE | 0.093102 | 0.214768 | 0.252518 | — | — | — |
| insert.test | COMPILE | 0.133313 | 0.324810 | 0.371604 | — | — | — |
| insert_range.test | COMPILE | 0.179503 | 0.428361 | 0.531165 | — | — | — |
| int.test | RUN | 0.094791 | 0.228882 | 0.266530 | 0.001400 | 0.001159 | 0.001322 |
| char.test | RUN | 0.096613 | 0.234220 | 0.274562 | 0.001276 | 0.001452 | 0.001247 |
| char_unsigned.test | RUN | 0.095666 | 0.229137 | 0.266870 | 0.001239 | 0.001600 | 0.001393 |
| integral_c.test | RUN | 0.104331 | 0.294077 | 0.338165 | 0.001320 | 0.001364 | 0.001442 |
| integral_c_unsigned.test | RUN | 0.108109 | 0.291904 | 0.338728 | 0.001404 | 0.001493 | 0.001218 |
| is_placeholder.test | COMPILE | 0.092904 | 0.213774 | 0.233877 | — | — | — |
| is_sequence.test | COMPILE | 0.108382 | 0.262417 | 0.302441 | — | — | — |
| iterator_tags.test | COMPILE | 0.091465 | 0.216407 | 0.259660 | — | — | — |
| joint_view.test | COMPILE | 0.121002 | 0.288701 | 0.336383 | — | — | — |
| lambda.test | COMPILE | 0.109496 | 0.269038 | 0.318308 | — | — | — |
| lambda_args.test | COMPILE | 0.106034 | 0.254425 | 0.309625 | — | — | — |
| list.test | COMPILE | 0.111638 | 0.269211 | 0.316373 | — | — | — |
| list_c.test | COMPILE | 0.109286 | 0.265208 | 0.317013 | — | — | — |
| logical.test | COMPILE | 0.088835 | 0.211308 | 0.254235 | — | — | — |
| lower_bound.test | COMPILE | 0.117676 | 0.265126 | 0.310167 | — | — | — |
| map.test | COMPILE | 0.136633 | 0.339428 | 0.393825 | — | — | — |
| max_element.test | COMPILE | 0.110513 | 0.260539 | 0.304823 | — | — | — |
| min_max.test | COMPILE | 0.096797 | 0.219468 | 0.262927 | — | — | — |
| multiset.test | COMPILE | 0.118065 | 0.293451 | 0.328467 | — | — | — |
| next.test | COMPILE | 0.090298 | 0.219023 | 0.251673 | — | — | — |
| no_has_xxx.test | COMPILE | 0.096040 | 0.220549 | 0.272995 | — | — | — |
| numeric_ops.test | COMPILE | 0.106618 | 0.263606 | 0.294931 | — | — | — |
| pair_view.test | COMPILE | 0.122987 | 0.299921 | 0.336976 | — | — | — |
| partition.test | COMPILE | 0.132157 | 0.331614 | 0.366377 | — | — | — |
| pop_front.test | COMPILE | 0.107707 | 0.270343 | 0.297227 | — | — | — |
| push_back.test | COMPILE | 0.092588 | 0.232096 | 0.259284 | — | — | — |
| push_front.test | COMPILE | 0.110470 | 0.277573 | 0.295625 | — | — | — |
| quote.test | COMPILE | 0.091538 | 0.227719 | 0.247786 | — | — | — |
| range_c.test | COMPILE | 0.115962 | 0.290068 | 0.314943 | — | — | — |
| remove.test | COMPILE | 0.120723 | 0.311439 | 0.332811 | — | — | — |
| remove_if.test | COMPILE | 0.122823 | 0.318862 | 0.342672 | — | — | — |
| replace.test | COMPILE | 0.134689 | 0.352113 | 0.365777 | — | — | — |
| replace_if.test | COMPILE | 0.136979 | 0.345760 | 0.369699 | — | — | — |
| reverse.test | COMPILE | 0.127175 | 0.326074 | 0.346567 | — | — | — |
| same_as.test | COMPILE | 0.097203 | 0.238058 | 0.263249 | — | — | — |
| set.test | COMPILE | 0.171075 | 0.438898 | 0.514061 | — | — | — |
| set_c.test | COMPILE | 0.123669 | 0.315539 | 0.342947 | — | — | — |
| single_view.test | COMPILE | 0.111878 | 0.282998 | 0.315345 | — | — | — |
| size.test | COMPILE | 0.109481 | 0.271918 | 0.299407 | — | — | — |
| size_t.test | RUN | 0.095376 | 0.243898 | 0.270858 | 0.001317 | 0.001597 | 0.001435 |
| sizeof.test | COMPILE | 0.088951 | 0.213630 | 0.234980 | — | — | — |
| sort.test | COMPILE | 0.143824 | 0.357656 | 0.380578 | — | — | — |
| stable_partition.test | COMPILE | 0.133685 | 0.330954 | 0.369436 | — | — | — |
| transform.test | COMPILE | 0.140022 | 0.361138 | 0.399858 | — | — | — |
| transform_view.test | COMPILE | 0.117671 | 0.285900 | 0.316453 | — | — | — |
| unique.test | COMPILE | 0.117189 | 0.307029 | 0.325288 | — | — | — |
| unpack_args.test | COMPILE | 0.114692 | 0.286956 | 0.312618 | — | — | — |
| upper_bound.test | COMPILE | 0.113006 | 0.279471 | 0.309636 | — | — | — |
| vector.test | COMPILE | 0.130523 | 0.339083 | 0.366426 | — | — | — |
| vector_c.test | COMPILE | 0.115361 | 0.287098 | 0.323404 | — | — | — |
| zip_view.test | COMPILE | 0.161266 | 0.412910 | 0.458617 | — | — | — |
| string.test | RUN | 0.248318 | 0.839026 | 0.683631 | 0.001075 | 0.001137 | 0.001054 |
| issue_69.test | RUN | 0.013082 | 0.036653 | 0.036964 | 0.000501 | 0.000553 | 0.001321 |
| issue_84.test | RUN | 0.021796 | 0.067138 | 0.064249 | 0.000429 | 0.000464 | 0.001063 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.996× | 0.999×, 0.993× | 1.006× |
| clang | 1.096× | 1.094×, 1.099× | 1.000× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409407).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| bool.test | 10 | 0.000844s | 0.000842s | 0.000836s | byte equality observed |
| char.test | 10 | 0.000844s | 0.000843s | 0.000855s | byte equality observed |
| char_unsigned.test | 10 | 0.000853s | 0.000843s | 0.000849s | byte equality observed |
| for_each.test | 10 | 0.000862s | 0.000865s | 0.000839s | byte equality observed |
| int.test | 10 | 0.000845s | 0.000863s | 0.000852s | byte equality observed |
| integral_c.test | 10 | 0.000847s | 0.000838s | 0.000851s | byte equality observed |
| integral_c_unsigned.test | 10 | 0.000873s | 0.000849s | 0.000869s | byte equality observed |
| issue_69.test | 10 | 0.000389s | 0.000390s | 0.000828s | byte equality observed |
| issue_84.test | 10 | 0.000386s | 0.000397s | 0.000831s | byte equality observed |
| size_t.test | 10 | 0.000857s | 0.000860s | 0.000836s | byte equality observed |
| string.test | 10 | 0.000839s | 0.000836s | 0.000839s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409407).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
