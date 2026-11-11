# libs/mpl/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| increased_arity.test | COMPILE | 0.437284 | 1.012293 | 1.009092 | — | — | — |
| largest_int.test | COMPILE | 0.145321 | 0.280449 | 0.399602 | — | — | — |
| msvc_is_class.test | COMPILE | 0.017526 | 0.036588 | 0.043905 | — | — | — |
| template_arity.test | COMPILE | 0.143759 | 0.276813 | 0.399117 | — | — | — |
| is_seq.test | COMPILE | 0.005598 | 0.014136 | 0.025532 | — | — | — |
| token_equal.test | COMPILE | 0.005956 | 0.014287 | 0.027619 | — | — | — |
| advance.test | COMPILE | 0.159739 | 0.318913 | 0.404320 | — | — | — |
| always.test | COMPILE | 0.160878 | 0.324257 | 0.434812 | — | — | — |
| apply.test | COMPILE | 0.167848 | 0.325518 | 0.459206 | — | — | — |
| apply_wrap.test | COMPILE | 0.147704 | 0.289241 | 0.385726 | — | — | — |
| arithmetic.test | COMPILE | 0.164312 | 0.330610 | 0.406314 | — | — | — |
| as_sequence.test | COMPILE | 0.175238 | 0.354387 | 0.456112 | — | — | — |
| assert.test | COMPILE | 0.045616 | 0.071023 | 0.085828 | — | — | — |
| at.test | COMPILE | 0.173032 | 0.340202 | 0.454888 | — | — | — |
| back.test | COMPILE | 0.163989 | 0.327324 | 0.431604 | — | — | — |
| bind.test | COMPILE | 0.160759 | 0.324619 | 0.423292 | — | — | — |
| bitwise.test | COMPILE | 0.164106 | 0.329864 | 0.432412 | — | — | — |
| bool.test | RUN | 0.146532 | 0.290835 | 0.378641 | 0.001596 | 0.001745 | 0.001563 |
| comparison.test | COMPILE | 0.158604 | 0.330935 | 0.418098 | — | — | — |
| contains.test | COMPILE | 0.195167 | 0.387704 | 0.481783 | — | — | — |
| copy.test | COMPILE | 0.230082 | 0.465372 | 0.622383 | — | — | — |
| copy_if.test | COMPILE | 0.202853 | 0.419114 | 0.547992 | — | — | — |
| count.test | COMPILE | 0.194779 | 0.380700 | 0.517024 | — | — | — |
| count_if.test | COMPILE | 0.206187 | 0.408971 | 0.545002 | — | — | — |
| deque.test | COMPILE | 0.195212 | 0.399156 | 0.507197 | — | — | — |
| distance.test | COMPILE | 0.181496 | 0.358308 | 0.477332 | — | — | — |
| empty.test | COMPILE | 0.160307 | 0.313956 | 0.409473 | — | — | — |
| equal.test | COMPILE | 0.188584 | 0.374671 | 0.491158 | — | — | — |
| erase.test | COMPILE | 0.212416 | 0.433093 | 0.571896 | — | — | — |
| erase_range.test | COMPILE | 0.199845 | 0.408865 | 0.546009 | — | — | — |
| eval_if.test | COMPILE | 0.151974 | 0.300954 | 0.377078 | — | — | — |
| filter_view.test | COMPILE | 0.193926 | 0.416062 | 0.525802 | — | — | — |
| find.test | COMPILE | 0.193631 | 0.391758 | 0.516530 | — | — | — |
| find_if.test | COMPILE | 0.197996 | 0.412524 | 0.524790 | — | — | — |
| fold.test | COMPILE | 0.208603 | 0.419194 | 0.564709 | — | — | — |
| for_each.test | RUN | 0.291849 | 0.620462 | 0.863215 | 0.001722 | 0.001622 | 0.001886 |
| front.test | COMPILE | 0.167120 | 0.332849 | 0.448998 | — | — | — |
| get_tag_def.test | COMPILE | 0.149568 | 0.293184 | 0.379659 | — | — | — |
| has_xxx.test | COMPILE | 0.162964 | 0.318049 | 0.426607 | — | — | — |
| identity.test | COMPILE | 0.167699 | 0.331076 | 0.410503 | — | — | — |
| if.test | COMPILE | 0.149981 | 0.293429 | 0.378363 | — | — | — |
| index_of.test | COMPILE | 0.190097 | 0.378641 | 0.485533 | — | — | — |
| inherit.test | COMPILE | 0.155420 | 0.304492 | 0.381070 | — | — | — |
| insert.test | COMPILE | 0.217642 | 0.444211 | 0.588280 | — | — | — |
| insert_range.test | COMPILE | 0.282592 | 0.597166 | 0.842457 | — | — | — |
| int.test | RUN | 0.154863 | 0.322650 | 0.440136 | 0.001677 | 0.001674 | 0.001501 |
| char.test | RUN | 0.161134 | 0.328683 | 0.420932 | 0.001639 | 0.001552 | 0.001823 |
| char_unsigned.test | RUN | 0.159042 | 0.325811 | 0.413408 | 0.001658 | 0.001830 | 0.001839 |
| integral_c.test | RUN | 0.169665 | 0.412686 | 0.531165 | 0.001606 | 0.001797 | 0.001791 |
| integral_c_unsigned.test | RUN | 0.166770 | 0.415605 | 0.529614 | 0.001409 | 0.001728 | 0.001614 |
| is_placeholder.test | COMPILE | 0.149056 | 0.295292 | 0.352668 | — | — | — |
| is_sequence.test | COMPILE | 0.179772 | 0.357654 | 0.439451 | — | — | — |
| iterator_tags.test | COMPILE | 0.153978 | 0.293719 | 0.370422 | — | — | — |
| joint_view.test | COMPILE | 0.198134 | 0.399846 | 0.541877 | — | — | — |
| lambda.test | COMPILE | 0.186836 | 0.374890 | 0.511771 | — | — | — |
| lambda_args.test | COMPILE | 0.180318 | 0.365560 | 0.490460 | — | — | — |
| list.test | COMPILE | 0.182568 | 0.379683 | 0.492909 | — | — | — |
| list_c.test | COMPILE | 0.180972 | 0.370398 | 0.489614 | — | — | — |
| logical.test | COMPILE | 0.149442 | 0.296874 | 0.381774 | — | — | — |
| lower_bound.test | COMPILE | 0.193365 | 0.372561 | 0.497552 | — | — | — |
| map.test | COMPILE | 0.222907 | 0.469040 | 0.612857 | — | — | — |
| max_element.test | COMPILE | 0.181931 | 0.359835 | 0.480659 | — | — | — |
| min_max.test | COMPILE | 0.157157 | 0.310768 | 0.398940 | — | — | — |
| multiset.test | COMPILE | 0.195060 | 0.403628 | 0.520667 | — | — | — |
| next.test | COMPILE | 0.147309 | 0.294152 | 0.383099 | — | — | — |
| no_has_xxx.test | COMPILE | 0.159984 | 0.310759 | 0.417289 | — | — | — |
| numeric_ops.test | COMPILE | 0.174567 | 0.351451 | 0.458190 | — | — | — |
| pair_view.test | COMPILE | 0.199795 | 0.402474 | 0.521764 | — | — | — |
| partition.test | COMPILE | 0.215680 | 0.435658 | 0.585769 | — | — | — |
| pop_front.test | COMPILE | 0.179500 | 0.360217 | 0.477012 | — | — | — |
| push_back.test | COMPILE | 0.153862 | 0.303073 | 0.410090 | — | — | — |
| push_front.test | COMPILE | 0.180879 | 0.367137 | 0.474374 | — | — | — |
| quote.test | COMPILE | 0.152360 | 0.296732 | 0.378186 | — | — | — |
| range_c.test | COMPILE | 0.186164 | 0.385946 | 0.495776 | — | — | — |
| remove.test | COMPILE | 0.194821 | 0.408204 | 0.520876 | — | — | — |
| remove_if.test | COMPILE | 0.200095 | 0.422217 | 0.522911 | — | — | — |
| replace.test | COMPILE | 0.216008 | 0.454734 | 0.576215 | — | — | — |
| replace_if.test | COMPILE | 0.218245 | 0.461247 | 0.588383 | — | — | — |
| reverse.test | COMPILE | 0.207389 | 0.428612 | 0.554143 | — | — | — |
| same_as.test | COMPILE | 0.161794 | 0.316566 | 0.411308 | — | — | — |
| set.test | COMPILE | 0.271348 | 0.595155 | 0.821476 | — | — | — |
| set_c.test | COMPILE | 0.202250 | 0.412238 | 0.569559 | — | — | — |
| single_view.test | COMPILE | 0.186785 | 0.378625 | 0.487424 | — | — | — |
| size.test | COMPILE | 0.179241 | 0.358538 | 0.461754 | — | — | — |
| size_t.test | RUN | 0.158338 | 0.320981 | 0.416329 | 0.001810 | 0.001886 | 0.001778 |
| sizeof.test | COMPILE | 0.146676 | 0.286738 | 0.373382 | — | — | — |
| sort.test | COMPILE | 0.228755 | 0.478640 | 0.604632 | — | — | — |
| stable_partition.test | COMPILE | 0.219295 | 0.444136 | 0.573734 | — | — | — |
| transform.test | COMPILE | 0.229659 | 0.480171 | 0.631044 | — | — | — |
| transform_view.test | COMPILE | 0.192492 | 0.390967 | 0.490938 | — | — | — |
| unique.test | COMPILE | 0.198824 | 0.399859 | 0.520924 | — | — | — |
| unpack_args.test | COMPILE | 0.189650 | 0.377639 | 0.477222 | — | — | — |
| upper_bound.test | COMPILE | 0.191447 | 0.375643 | 0.492708 | — | — | — |
| vector.test | COMPILE | 0.214550 | 0.457584 | 0.583114 | — | — | — |
| vector_c.test | COMPILE | 0.188733 | 0.364660 | 0.510569 | — | — | — |
| zip_view.test | COMPILE | 0.260541 | 0.564496 | 0.723635 | — | — | — |
| string.test | RUN | 0.407346 | 1.104480 | 0.993908 | 0.001331 | 0.001416 | 0.001290 |
| issue_69.test | RUN | 0.018537 | 0.044943 | 0.050755 | 0.000651 | 0.000723 | 0.001571 |
| issue_84.test | RUN | 0.053352 | 0.090818 | 0.096810 | 0.000591 | 0.000570 | 0.001382 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.981×† | 0.969×, 0.993× | 1.070× |
| clang | 1.060×† | 1.104×, 1.015× | 0.991× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| bool.test | 10 | 0.001044s | 0.001025s | 0.001041s | byte equality observed |
| char.test | 10 | 0.001038s | 0.001030s | 0.001023s | byte equality observed |
| char_unsigned.test | 10 | 0.001052s | 0.001023s | 0.001029s | byte equality observed |
| for_each.test | 10 | 0.001049s | 0.001047s | 0.001026s | byte equality observed |
| int.test | 10 | 0.001047s | 0.001025s | 0.001026s | byte equality observed |
| integral_c.test | 10 | 0.001037s | 0.001013s | 0.001060s | byte equality observed |
| integral_c_unsigned.test | 10 | 0.001041s | 0.001043s | 0.001030s | byte equality observed |
| issue_69.test | 10 | 0.000493s | 0.000487s | 0.001024s | byte equality observed |
| issue_84.test | 10 | 0.000492s | 0.000492s | 0.001036s | byte equality observed |
| size_t.test | 10 | 0.001054s | 0.001038s | 0.001034s | byte equality observed |
| string.test | 10 | 0.001053s | 0.001026s | 0.001027s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
