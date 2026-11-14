# libs/mpl/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| increased_arity.test | COMPILE | 0.298811 | 0.855260 | 0.797957 | — | — | — |
| largest_int.test | COMPILE | 0.091939 | 0.226282 | 0.292720 | — | — | — |
| msvc_is_class.test | COMPILE | 0.010773 | 0.028583 | 0.032808 | — | — | — |
| template_arity.test | COMPILE | 0.094288 | 0.233151 | 0.293052 | — | — | — |
| is_seq.test | COMPILE | 0.003566 | 0.010009 | 0.021212 | — | — | — |
| token_equal.test | COMPILE | 0.003624 | 0.010194 | 0.021176 | — | — | — |
| advance.test | COMPILE | 0.103844 | 0.261893 | 0.314837 | — | — | — |
| always.test | COMPILE | 0.106206 | 0.273110 | 0.334322 | — | — | — |
| apply.test | COMPILE | 0.112271 | 0.288089 | 0.349961 | — | — | — |
| apply_wrap.test | COMPILE | 0.096869 | 0.241770 | 0.290909 | — | — | — |
| arithmetic.test | COMPILE | 0.107029 | 0.273577 | 0.337042 | — | — | — |
| as_sequence.test | COMPILE | 0.119128 | 0.290014 | 0.351410 | — | — | — |
| assert.test | COMPILE | 0.026285 | 0.054109 | 0.064971 | — | — | — |
| at.test | COMPILE | 0.116483 | 0.291096 | 0.353958 | — | — | — |
| back.test | COMPILE | 0.112836 | 0.283098 | 0.335820 | — | — | — |
| bind.test | COMPILE | 0.108615 | 0.266719 | 0.326928 | — | — | — |
| bitwise.test | COMPILE | 0.107301 | 0.264861 | 0.333467 | — | — | — |
| bool.test | RUN | 0.093530 | 0.240813 | 0.301505 | 0.001007 | 0.001320 | 0.001198 |
| comparison.test | COMPILE | 0.105085 | 0.269054 | 0.328446 | — | — | — |
| contains.test | COMPILE | 0.127596 | 0.327846 | 0.394268 | — | — | — |
| copy.test | COMPILE | 0.154672 | 0.394733 | 0.479463 | — | — | — |
| copy_if.test | COMPILE | 0.134253 | 0.351724 | 0.425451 | — | — | — |
| count.test | COMPILE | 0.132163 | 0.329166 | 0.404397 | — | — | — |
| count_if.test | COMPILE | 0.132897 | 0.338395 | 0.419654 | — | — | — |
| deque.test | COMPILE | 0.128262 | 0.329880 | 0.402668 | — | — | — |
| distance.test | COMPILE | 0.118681 | 0.302336 | 0.366939 | — | — | — |
| empty.test | COMPILE | 0.106227 | 0.269274 | 0.324065 | — | — | — |
| equal.test | COMPILE | 0.119449 | 0.308467 | 0.377808 | — | — | — |
| erase.test | COMPILE | 0.140563 | 0.363343 | 0.438212 | — | — | — |
| erase_range.test | COMPILE | 0.134201 | 0.347471 | 0.415628 | — | — | — |
| eval_if.test | COMPILE | 0.095805 | 0.249138 | 0.301365 | — | — | — |
| filter_view.test | COMPILE | 0.132214 | 0.337469 | 0.406849 | — | — | — |
| find.test | COMPILE | 0.130899 | 0.336005 | 0.391811 | — | — | — |
| find_if.test | COMPILE | 0.130395 | 0.339108 | 0.408800 | — | — | — |
| fold.test | COMPILE | 0.141254 | 0.354914 | 0.435368 | — | — | — |
| for_each.test | RUN | 0.203393 | 0.511734 | 0.670253 | 0.000901 | 0.001356 | 0.001256 |
| front.test | COMPILE | 0.105878 | 0.287518 | 0.337028 | — | — | — |
| get_tag_def.test | COMPILE | 0.097416 | 0.237652 | 0.298376 | — | — | — |
| has_xxx.test | COMPILE | 0.101258 | 0.258328 | 0.338851 | — | — | — |
| identity.test | COMPILE | 0.110073 | 0.274797 | 0.323988 | — | — | — |
| if.test | COMPILE | 0.092433 | 0.236572 | 0.289513 | — | — | — |
| index_of.test | COMPILE | 0.125512 | 0.315464 | 0.381279 | — | — | — |
| inherit.test | COMPILE | 0.097844 | 0.246578 | 0.309909 | — | — | — |
| insert.test | COMPILE | 0.145885 | 0.378350 | 0.450536 | — | — | — |
| insert_range.test | COMPILE | 0.193965 | 0.506069 | 0.636212 | — | — | — |
| int.test | RUN | 0.104009 | 0.272839 | 0.323248 | 0.001021 | 0.001355 | 0.001157 |
| char.test | RUN | 0.099760 | 0.277602 | 0.321928 | 0.001059 | 0.001088 | 0.001322 |
| char_unsigned.test | RUN | 0.106251 | 0.267870 | 0.320773 | 0.001049 | 0.001366 | 0.001244 |
| integral_c.test | RUN | 0.115980 | 0.350238 | 0.404684 | 0.001214 | 0.001393 | 0.001277 |
| integral_c_unsigned.test | RUN | 0.113076 | 0.347194 | 0.404008 | 0.001067 | 0.001116 | 0.001139 |
| is_placeholder.test | COMPILE | 0.098888 | 0.246714 | 0.290513 | — | — | — |
| is_sequence.test | COMPILE | 0.117283 | 0.301742 | 0.358288 | — | — | — |
| iterator_tags.test | COMPILE | 0.098553 | 0.255471 | 0.306628 | — | — | — |
| joint_view.test | COMPILE | 0.129830 | 0.339191 | 0.407370 | — | — | — |
| lambda.test | COMPILE | 0.118616 | 0.314414 | 0.373505 | — | — | — |
| lambda_args.test | COMPILE | 0.112157 | 0.291857 | 0.373188 | — | — | — |
| list.test | COMPILE | 0.117085 | 0.312919 | 0.366423 | — | — | — |
| list_c.test | COMPILE | 0.118047 | 0.305469 | 0.376757 | — | — | — |
| logical.test | COMPILE | 0.094330 | 0.246609 | 0.299415 | — | — | — |
| lower_bound.test | COMPILE | 0.121950 | 0.309983 | 0.383812 | — | — | — |
| map.test | COMPILE | 0.147465 | 0.392055 | 0.474273 | — | — | — |
| max_element.test | COMPILE | 0.120187 | 0.300549 | 0.358883 | — | — | — |
| min_max.test | COMPILE | 0.098012 | 0.258403 | 0.313284 | — | — | — |
| multiset.test | COMPILE | 0.127968 | 0.335427 | 0.400771 | — | — | — |
| next.test | COMPILE | 0.090252 | 0.240666 | 0.300067 | — | — | — |
| no_has_xxx.test | COMPILE | 0.103286 | 0.259727 | 0.328566 | — | — | — |
| numeric_ops.test | COMPILE | 0.113024 | 0.294985 | 0.346486 | — | — | — |
| pair_view.test | COMPILE | 0.127262 | 0.335989 | 0.401035 | — | — | — |
| partition.test | COMPILE | 0.143578 | 0.372569 | 0.446650 | — | — | — |
| pop_front.test | COMPILE | 0.118534 | 0.298503 | 0.357056 | — | — | — |
| push_back.test | COMPILE | 0.098890 | 0.253342 | 0.308881 | — | — | — |
| push_front.test | COMPILE | 0.116828 | 0.306987 | 0.366531 | — | — | — |
| quote.test | COMPILE | 0.098640 | 0.246705 | 0.303666 | — | — | — |
| range_c.test | COMPILE | 0.127916 | 0.321906 | 0.388756 | — | — | — |
| remove.test | COMPILE | 0.130809 | 0.336818 | 0.404553 | — | — | — |
| remove_if.test | COMPILE | 0.140492 | 0.351725 | 0.423554 | — | — | — |
| replace.test | COMPILE | 0.144926 | 0.387960 | 0.455859 | — | — | — |
| replace_if.test | COMPILE | 0.151317 | 0.374865 | 0.464821 | — | — | — |
| reverse.test | COMPILE | 0.136805 | 0.352886 | 0.430502 | — | — | — |
| same_as.test | COMPILE | 0.102344 | 0.260545 | 0.316373 | — | — | — |
| set.test | COMPILE | 0.184941 | 0.513237 | 0.657971 | — | — | — |
| set_c.test | COMPILE | 0.134817 | 0.348105 | 0.430020 | — | — | — |
| single_view.test | COMPILE | 0.126339 | 0.322721 | 0.377133 | — | — | — |
| size.test | COMPILE | 0.117758 | 0.302905 | 0.363339 | — | — | — |
| size_t.test | RUN | 0.105690 | 0.278122 | 0.343581 | 0.001111 | 0.001118 | 0.001372 |
| sizeof.test | COMPILE | 0.094932 | 0.243795 | 0.302402 | — | — | — |
| sort.test | COMPILE | 0.150039 | 0.396494 | 0.481424 | — | — | — |
| stable_partition.test | COMPILE | 0.141145 | 0.374319 | 0.447800 | — | — | — |
| transform.test | COMPILE | 0.150544 | 0.404184 | 0.491591 | — | — | — |
| transform_view.test | COMPILE | 0.121270 | 0.325028 | 0.387544 | — | — | — |
| unique.test | COMPILE | 0.132940 | 0.339146 | 0.402067 | — | — | — |
| unpack_args.test | COMPILE | 0.120754 | 0.316706 | 0.373515 | — | — | — |
| upper_bound.test | COMPILE | 0.124446 | 0.318588 | 0.377600 | — | — | — |
| vector.test | COMPILE | 0.138902 | 0.380994 | 0.444816 | — | — | — |
| vector_c.test | COMPILE | 0.127368 | 0.313568 | 0.390190 | — | — | — |
| zip_view.test | COMPILE | 0.176418 | 0.471687 | 0.560394 | — | — | — |
| string.test | RUN | 0.299648 | 0.939208 | 0.839740 | 0.000800 | 0.000943 | 0.000834 |
| issue_69.test | RUN | 0.009990 | 0.034531 | 0.037913 | 0.000445 | 0.000449 | 0.001119 |
| issue_84.test | RUN | 0.029847 | 0.070274 | 0.070718 | 0.000384 | 0.000368 | 0.000858 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.007× | 1.009×, 1.004× | 0.985× |
| clang | 1.100× | 1.101×, 1.098× | 0.996× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814672).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| bool.test | 10 | 0.000785s | 0.000821s | 0.000796s | byte equality observed |
| char.test | 10 | 0.000826s | 0.000817s | 0.000811s | byte equality observed |
| char_unsigned.test | 10 | 0.000828s | 0.000809s | 0.000809s | byte equality observed |
| for_each.test | 10 | 0.000812s | 0.000838s | 0.000831s | byte equality observed |
| int.test | 10 | 0.000786s | 0.000814s | 0.000807s | byte equality observed |
| integral_c.test | 10 | 0.000813s | 0.000810s | 0.000790s | byte equality observed |
| integral_c_unsigned.test | 10 | 0.000800s | 0.000785s | 0.000814s | byte equality observed |
| issue_69.test | 10 | 0.000367s | 0.000364s | 0.000795s | byte equality observed |
| issue_84.test | 10 | 0.000363s | 0.000363s | 0.000799s | byte equality observed |
| size_t.test | 10 | 0.000814s | 0.000816s | 0.000799s | byte equality observed |
| string.test | 10 | 0.000816s | 0.000831s | 0.000823s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814672).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
