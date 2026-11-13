# libs/mpl/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| increased_arity.test | COMPILE | 0.367029 | 1.025718 | 1.063921 | — | — | — |
| largest_int.test | COMPILE | 0.123745 | 0.271900 | 0.361707 | — | — | — |
| msvc_is_class.test | COMPILE | 0.015084 | 0.038334 | 0.044104 | — | — | — |
| template_arity.test | COMPILE | 0.123840 | 0.279656 | 0.363376 | — | — | — |
| is_seq.test | COMPILE | 0.005022 | 0.013866 | 0.028558 | — | — | — |
| token_equal.test | COMPILE | 0.005618 | 0.013248 | 0.028506 | — | — | — |
| advance.test | COMPILE | 0.137551 | 0.311995 | 0.428162 | — | — | — |
| always.test | COMPILE | 0.138343 | 0.326005 | 0.441553 | — | — | — |
| apply.test | COMPILE | 0.155857 | 0.341088 | 0.449843 | — | — | — |
| apply_wrap.test | COMPILE | 0.129224 | 0.288452 | 0.395175 | — | — | — |
| arithmetic.test | COMPILE | 0.138621 | 0.327590 | 0.414820 | — | — | — |
| as_sequence.test | COMPILE | 0.151136 | 0.357791 | 0.466613 | — | — | — |
| assert.test | COMPILE | 0.036704 | 0.070448 | 0.089660 | — | — | — |
| at.test | COMPILE | 0.149245 | 0.348900 | 0.465576 | — | — | — |
| back.test | COMPILE | 0.144930 | 0.332418 | 0.437665 | — | — | — |
| bind.test | COMPILE | 0.137665 | 0.317484 | 0.435300 | — | — | — |
| bitwise.test | COMPILE | 0.140069 | 0.334118 | 0.441470 | — | — | — |
| bool.test | RUN | 0.127732 | 0.282750 | 0.387553 | 0.001536 | 0.001731 | 0.001725 |
| comparison.test | COMPILE | 0.136549 | 0.321332 | 0.413945 | — | — | — |
| contains.test | COMPILE | 0.164121 | 0.396341 | 0.542841 | — | — | — |
| copy.test | COMPILE | 0.204431 | 0.468998 | 0.635114 | — | — | — |
| copy_if.test | COMPILE | 0.181974 | 0.415458 | 0.552636 | — | — | — |
| count.test | COMPILE | 0.175209 | 0.393582 | 0.537142 | — | — | — |
| count_if.test | COMPILE | 0.179198 | 0.389913 | 0.547613 | — | — | — |
| deque.test | COMPILE | 0.169132 | 0.394877 | 0.526006 | — | — | — |
| distance.test | COMPILE | 0.163378 | 0.360262 | 0.481272 | — | — | — |
| empty.test | COMPILE | 0.141059 | 0.325709 | 0.432757 | — | — | — |
| equal.test | COMPILE | 0.159337 | 0.376395 | 0.489943 | — | — | — |
| erase.test | COMPILE | 0.182311 | 0.421318 | 0.578003 | — | — | — |
| erase_range.test | COMPILE | 0.177698 | 0.375922 | 0.541945 | — | — | — |
| eval_if.test | COMPILE | 0.131820 | 0.294153 | 0.410476 | — | — | — |
| filter_view.test | COMPILE | 0.181600 | 0.406821 | 0.539922 | — | — | — |
| find.test | COMPILE | 0.178738 | 0.393094 | 0.526398 | — | — | — |
| find_if.test | COMPILE | 0.181204 | 0.404240 | 0.534269 | — | — | — |
| fold.test | COMPILE | 0.189988 | 0.428327 | 0.565481 | — | — | — |
| for_each.test | RUN | 0.265720 | 0.633080 | 0.865723 | 0.001449 | 0.001626 | 0.001939 |
| front.test | COMPILE | 0.144213 | 0.332555 | 0.456676 | — | — | — |
| get_tag_def.test | COMPILE | 0.128571 | 0.300505 | 0.388339 | — | — | — |
| has_xxx.test | COMPILE | 0.143709 | 0.319168 | 0.440091 | — | — | — |
| identity.test | COMPILE | 0.150307 | 0.322277 | 0.436978 | — | — | — |
| if.test | COMPILE | 0.134352 | 0.288593 | 0.373538 | — | — | — |
| index_of.test | COMPILE | 0.172529 | 0.385876 | 0.520631 | — | — | — |
| inherit.test | COMPILE | 0.135537 | 0.313824 | 0.415110 | — | — | — |
| insert.test | COMPILE | 0.197669 | 0.444313 | 0.601166 | — | — | — |
| insert_range.test | COMPILE | 0.269235 | 0.594198 | 0.849994 | — | — | — |
| int.test | RUN | 0.134451 | 0.323762 | 0.443750 | 0.001583 | 0.001656 | 0.001524 |
| char.test | RUN | 0.135700 | 0.325435 | 0.428166 | 0.001386 | 0.001469 | 0.001918 |
| char_unsigned.test | RUN | 0.141446 | 0.323993 | 0.409147 | 0.001750 | 0.001607 | 0.001881 |
| integral_c.test | RUN | 0.158602 | 0.408668 | 0.523865 | 0.001609 | 0.001822 | 0.001580 |
| integral_c_unsigned.test | RUN | 0.151896 | 0.407398 | 0.520449 | 0.001277 | 0.001762 | 0.001463 |
| is_placeholder.test | COMPILE | 0.131094 | 0.289976 | 0.360805 | — | — | — |
| is_sequence.test | COMPILE | 0.154451 | 0.352312 | 0.478780 | — | — | — |
| iterator_tags.test | COMPILE | 0.134537 | 0.299574 | 0.408189 | — | — | — |
| joint_view.test | COMPILE | 0.176994 | 0.420344 | 0.540115 | — | — | — |
| lambda.test | COMPILE | 0.160987 | 0.389348 | 0.507751 | — | — | — |
| lambda_args.test | COMPILE | 0.158321 | 0.391290 | 0.498545 | — | — | — |
| list.test | COMPILE | 0.160460 | 0.416786 | 0.494302 | — | — | — |
| list_c.test | COMPILE | 0.163351 | 0.389490 | 0.499902 | — | — | — |
| logical.test | COMPILE | 0.128954 | 0.317574 | 0.402141 | — | — | — |
| lower_bound.test | COMPILE | 0.169275 | 0.381784 | 0.488297 | — | — | — |
| map.test | COMPILE | 0.197527 | 0.475568 | 0.608953 | — | — | — |
| max_element.test | COMPILE | 0.155982 | 0.360724 | 0.479950 | — | — | — |
| min_max.test | COMPILE | 0.132721 | 0.312320 | 0.425842 | — | — | — |
| multiset.test | COMPILE | 0.171213 | 0.399865 | 0.523767 | — | — | — |
| next.test | COMPILE | 0.130956 | 0.297130 | 0.397772 | — | — | — |
| no_has_xxx.test | COMPILE | 0.138791 | 0.311539 | 0.436132 | — | — | — |
| numeric_ops.test | COMPILE | 0.150052 | 0.360900 | 0.478294 | — | — | — |
| pair_view.test | COMPILE | 0.180085 | 0.395254 | 0.530663 | — | — | — |
| partition.test | COMPILE | 0.194786 | 0.438937 | 0.585210 | — | — | — |
| pop_front.test | COMPILE | 0.156825 | 0.358786 | 0.478020 | — | — | — |
| push_back.test | COMPILE | 0.133710 | 0.302480 | 0.417401 | — | — | — |
| push_front.test | COMPILE | 0.156533 | 0.369684 | 0.480262 | — | — | — |
| quote.test | COMPILE | 0.134251 | 0.298329 | 0.394687 | — | — | — |
| range_c.test | COMPILE | 0.169083 | 0.385488 | 0.497917 | — | — | — |
| remove.test | COMPILE | 0.171193 | 0.411427 | 0.528712 | — | — | — |
| remove_if.test | COMPILE | 0.184275 | 0.428807 | 0.546003 | — | — | — |
| replace.test | COMPILE | 0.188800 | 0.459784 | 0.583029 | — | — | — |
| replace_if.test | COMPILE | 0.192929 | 0.462561 | 0.593911 | — | — | — |
| reverse.test | COMPILE | 0.182079 | 0.432762 | 0.567577 | — | — | — |
| same_as.test | COMPILE | 0.136111 | 0.323255 | 0.418145 | — | — | — |
| set.test | COMPILE | 0.243037 | 0.605653 | 0.834099 | — | — | — |
| set_c.test | COMPILE | 0.175163 | 0.410666 | 0.549187 | — | — | — |
| single_view.test | COMPILE | 0.161493 | 0.380478 | 0.501070 | — | — | — |
| size.test | COMPILE | 0.154646 | 0.365022 | 0.470595 | — | — | — |
| size_t.test | RUN | 0.133013 | 0.323151 | 0.418896 | 0.001595 | 0.001784 | 0.001856 |
| sizeof.test | COMPILE | 0.122277 | 0.297866 | 0.390066 | — | — | — |
| sort.test | COMPILE | 0.196985 | 0.493765 | 0.611843 | — | — | — |
| stable_partition.test | COMPILE | 0.185566 | 0.462541 | 0.588312 | — | — | — |
| transform.test | COMPILE | 0.200077 | 0.484974 | 0.643806 | — | — | — |
| transform_view.test | COMPILE | 0.160660 | 0.391969 | 0.499532 | — | — | — |
| unique.test | COMPILE | 0.169418 | 0.409956 | 0.556786 | — | — | — |
| unpack_args.test | COMPILE | 0.157382 | 0.389381 | 0.505971 | — | — | — |
| upper_bound.test | COMPILE | 0.159188 | 0.387013 | 0.510430 | — | — | — |
| vector.test | COMPILE | 0.186709 | 0.470755 | 0.620121 | — | — | — |
| vector_c.test | COMPILE | 0.161635 | 0.377534 | 0.513529 | — | — | — |
| zip_view.test | COMPILE | 0.223679 | 0.566545 | 0.759862 | — | — | — |
| string.test | RUN | 0.395065 | 1.113005 | 1.107806 | 0.001410 | 0.001340 | 0.001301 |
| issue_69.test | RUN | 0.017153 | 0.045398 | 0.052589 | 0.000746 | 0.000701 | 0.001612 |
| issue_84.test | RUN | 0.029414 | 0.091799 | 0.069963 | 0.000640 | 0.000574 | 0.001243 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.964×† | 1.003×, 0.925× | 0.993× |
| clang | 1.151×† | 1.182×, 1.120× | 1.036× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737952).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| bool.test | 10 | 0.001009s | 0.000996s | 0.001022s | byte equality observed |
| char.test | 10 | 0.000988s | 0.000979s | 0.001050s | byte equality observed |
| char_unsigned.test | 10 | 0.001040s | 0.001001s | 0.001023s | byte equality observed |
| for_each.test | 10 | 0.001031s | 0.001051s | 0.001103s | byte equality observed |
| int.test | 10 | 0.001005s | 0.001017s | 0.001069s | byte equality observed |
| integral_c.test | 10 | 0.001053s | 0.001011s | 0.001065s | byte equality observed |
| integral_c_unsigned.test | 10 | 0.001032s | 0.001049s | 0.001024s | byte equality observed |
| issue_69.test | 10 | 0.000483s | 0.000480s | 0.001025s | byte equality observed |
| issue_84.test | 10 | 0.000478s | 0.000480s | 0.001018s | byte equality observed |
| size_t.test | 10 | 0.001039s | 0.001069s | 0.001022s | byte equality observed |
| string.test | 10 | 0.001002s | 0.001039s | 0.001040s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737952).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
