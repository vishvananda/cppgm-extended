# libs/mpl/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| increased_arity.test | COMPILE | 0.600545 | 1.442507 | 1.589856 | — | — | — |
| largest_int.test | COMPILE | 0.208795 | 0.410485 | 0.552491 | — | — | — |
| msvc_is_class.test | COMPILE | 0.023127 | 0.049706 | 0.058709 | — | — | — |
| template_arity.test | COMPILE | 0.194491 | 0.387844 | 0.562005 | — | — | — |
| is_seq.test | COMPILE | 0.007285 | 0.018308 | 0.033343 | — | — | — |
| token_equal.test | COMPILE | 0.007778 | 0.019541 | 0.032860 | — | — | — |
| advance.test | COMPILE | 0.216874 | 0.451519 | 0.648578 | — | — | — |
| always.test | COMPILE | 0.229123 | 0.433085 | 0.647023 | — | — | — |
| apply.test | COMPILE | 0.242389 | 0.498835 | 0.713601 | — | — | — |
| apply_wrap.test | COMPILE | 0.210769 | 0.424286 | 0.605324 | — | — | — |
| arithmetic.test | COMPILE | 0.227378 | 0.467151 | 0.635583 | — | — | — |
| as_sequence.test | COMPILE | 0.244970 | 0.514686 | 0.715671 | — | — | — |
| assert.test | COMPILE | 0.062543 | 0.093308 | 0.123274 | — | — | — |
| at.test | COMPILE | 0.237383 | 0.494966 | 0.700186 | — | — | — |
| back.test | COMPILE | 0.231645 | 0.458032 | 0.663083 | — | — | — |
| bind.test | COMPILE | 0.223522 | 0.465963 | 0.664386 | — | — | — |
| bitwise.test | COMPILE | 0.228092 | 0.469121 | 0.678808 | — | — | — |
| bool.test | RUN | 0.206006 | 0.407402 | 0.597776 | 0.001858 | 0.002242 | 0.002232 |
| comparison.test | COMPILE | 0.229789 | 0.446045 | 0.644047 | — | — | — |
| contains.test | COMPILE | 0.264658 | 0.566588 | 0.754078 | — | — | — |
| copy.test | COMPILE | 0.320575 | 0.665751 | 0.970736 | — | — | — |
| copy_if.test | COMPILE | 0.279570 | 0.603092 | 0.860282 | — | — | — |
| count.test | COMPILE | 0.267226 | 0.549414 | 0.805392 | — | — | — |
| count_if.test | COMPILE | 0.286117 | 0.588564 | 0.851749 | — | — | — |
| deque.test | COMPILE | 0.271231 | 0.573863 | 0.782200 | — | — | — |
| distance.test | COMPILE | 0.250773 | 0.507691 | 0.750508 | — | — | — |
| empty.test | COMPILE | 0.222269 | 0.456488 | 0.632292 | — | — | — |
| equal.test | COMPILE | 0.258963 | 0.543873 | 0.766679 | — | — | — |
| erase.test | COMPILE | 0.292153 | 0.627854 | 0.897383 | — | — | — |
| erase_range.test | COMPILE | 0.277775 | 0.577671 | 0.823641 | — | — | — |
| eval_if.test | COMPILE | 0.212699 | 0.442200 | 0.645813 | — | — | — |
| filter_view.test | COMPILE | 0.279235 | 0.595186 | 0.834158 | — | — | — |
| find.test | COMPILE | 0.274219 | 0.557961 | 0.795857 | — | — | — |
| find_if.test | COMPILE | 0.276801 | 0.600717 | 0.819416 | — | — | — |
| fold.test | COMPILE | 0.292042 | 0.594723 | 0.876315 | — | — | — |
| for_each.test | RUN | 0.414575 | 0.902124 | 1.375613 | 0.002392 | 0.002140 | 0.002212 |
| front.test | COMPILE | 0.231280 | 0.472031 | 0.677470 | — | — | — |
| get_tag_def.test | COMPILE | 0.209088 | 0.427255 | 0.622284 | — | — | — |
| has_xxx.test | COMPILE | 0.230927 | 0.464220 | 0.692612 | — | — | — |
| identity.test | COMPILE | 0.236821 | 0.466793 | 0.663706 | — | — | — |
| if.test | COMPILE | 0.203685 | 0.426319 | 0.575207 | — | — | — |
| index_of.test | COMPILE | 0.265793 | 0.540941 | 0.768449 | — | — | — |
| inherit.test | COMPILE | 0.219800 | 0.439105 | 0.621186 | — | — | — |
| insert.test | COMPILE | 0.299652 | 0.626309 | 0.927609 | — | — | — |
| insert_range.test | COMPILE | 0.399515 | 0.880678 | 1.293347 | — | — | — |
| int.test | RUN | 0.212443 | 0.466977 | 0.658300 | 0.002044 | 0.002028 | 0.002282 |
| char.test | RUN | 0.220135 | 0.475625 | 0.695497 | 0.002024 | 0.001874 | 0.002192 |
| char_unsigned.test | RUN | 0.222532 | 0.468842 | 0.627762 | 0.001990 | 0.002343 | 0.002188 |
| integral_c.test | RUN | 0.235174 | 0.590309 | 0.769812 | 0.001855 | 0.002196 | 0.002186 |
| integral_c_unsigned.test | RUN | 0.235243 | 0.591424 | 0.798800 | 0.002113 | 0.002350 | 0.001950 |
| is_placeholder.test | COMPILE | 0.209826 | 0.421830 | 0.639914 | — | — | — |
| is_sequence.test | COMPILE | 0.250030 | 0.499360 | 0.693337 | — | — | — |
| iterator_tags.test | COMPILE | 0.215740 | 0.429540 | 0.613702 | — | — | — |
| joint_view.test | COMPILE | 0.283841 | 0.588433 | 0.830304 | — | — | — |
| lambda.test | COMPILE | 0.257638 | 0.520982 | 0.749223 | — | — | — |
| lambda_args.test | COMPILE | 0.260032 | 0.518514 | 0.783545 | — | — | — |
| list.test | COMPILE | 0.256838 | 0.540766 | 0.738842 | — | — | — |
| list_c.test | COMPILE | 0.251109 | 0.506901 | 0.772802 | — | — | — |
| logical.test | COMPILE | 0.209586 | 0.413106 | 0.615083 | — | — | — |
| lower_bound.test | COMPILE | 0.262228 | 0.556101 | 0.773516 | — | — | — |
| map.test | COMPILE | 0.305756 | 0.675665 | 0.958828 | — | — | — |
| max_element.test | COMPILE | 0.250851 | 0.496830 | 0.757772 | — | — | — |
| min_max.test | COMPILE | 0.217082 | 0.449831 | 0.626179 | — | — | — |
| multiset.test | COMPILE | 0.269685 | 0.585344 | 0.797670 | — | — | — |
| next.test | COMPILE | 0.211136 | 0.416170 | 0.610385 | — | — | — |
| no_has_xxx.test | COMPILE | 0.221389 | 0.456090 | 0.647620 | — | — | — |
| numeric_ops.test | COMPILE | 0.243873 | 0.502981 | 0.729912 | — | — | — |
| pair_view.test | COMPILE | 0.275063 | 0.587247 | 0.786577 | — | — | — |
| partition.test | COMPILE | 0.304228 | 0.621444 | 0.899416 | — | — | — |
| pop_front.test | COMPILE | 0.245855 | 0.520312 | 0.754331 | — | — | — |
| push_back.test | COMPILE | 0.211477 | 0.437438 | 0.618163 | — | — | — |
| push_front.test | COMPILE | 0.248945 | 0.513577 | 0.729528 | — | — | — |
| quote.test | COMPILE | 0.214316 | 0.408209 | 0.613342 | — | — | — |
| range_c.test | COMPILE | 0.255071 | 0.555612 | 0.751099 | — | — | — |
| remove.test | COMPILE | 0.269373 | 0.583708 | 0.818413 | — | — | — |
| remove_if.test | COMPILE | 0.275515 | 0.596706 | 0.838360 | — | — | — |
| replace.test | COMPILE | 0.305215 | 0.632542 | 0.901092 | — | — | — |
| replace_if.test | COMPILE | 0.296768 | 0.648923 | 0.921576 | — | — | — |
| reverse.test | COMPILE | 0.281086 | 0.601171 | 0.869728 | — | — | — |
| same_as.test | COMPILE | 0.224792 | 0.449893 | 0.658376 | — | — | — |
| set.test | COMPILE | 0.381208 | 0.854461 | 1.272578 | — | — | — |
| set_c.test | COMPILE | 0.283478 | 0.601136 | 0.856111 | — | — | — |
| single_view.test | COMPILE | 0.255545 | 0.538922 | 0.766699 | — | — | — |
| size.test | COMPILE | 0.250124 | 0.515171 | 0.710152 | — | — | — |
| size_t.test | RUN | 0.218665 | 0.462470 | 0.658601 | 0.002134 | 0.002270 | 0.002346 |
| sizeof.test | COMPILE | 0.203483 | 0.408061 | 0.562441 | — | — | — |
| sort.test | COMPILE | 0.318570 | 0.670749 | 0.939216 | — | — | — |
| stable_partition.test | COMPILE | 0.297788 | 0.637792 | 0.899004 | — | — | — |
| transform.test | COMPILE | 0.315332 | 0.684507 | 0.986775 | — | — | — |
| transform_view.test | COMPILE | 0.257783 | 0.547424 | 0.772894 | — | — | — |
| unique.test | COMPILE | 0.270570 | 0.577525 | 0.807714 | — | — | — |
| unpack_args.test | COMPILE | 0.258723 | 0.546753 | 0.735357 | — | — | — |
| upper_bound.test | COMPILE | 0.263255 | 0.532043 | 0.755518 | — | — | — |
| vector.test | COMPILE | 0.295222 | 0.649175 | 0.935897 | — | — | — |
| vector_c.test | COMPILE | 0.260675 | 0.524209 | 0.812841 | — | — | — |
| zip_view.test | COMPILE | 0.376302 | 0.807550 | 1.099719 | — | — | — |
| string.test | RUN | 0.538113 | 1.429037 | 1.431859 | 0.001786 | 0.001672 | 0.001714 |
| issue_69.test | RUN | 0.024267 | 0.067882 | 0.071634 | 0.000731 | 0.000752 | 0.002208 |
| issue_84.test | RUN | 0.067325 | 0.114165 | 0.132922 | 0.000736 | 0.000837 | 0.001597 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.995× | 0.988×, 1.002× | 0.995× |
| clang | 1.104× | 1.113×, 1.094× | 1.009× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| bool.test | 10 | 0.001284s | 0.001313s | 0.001266s | byte equality observed |
| char.test | 10 | 0.001323s | 0.001345s | 0.001331s | byte equality observed |
| char_unsigned.test | 10 | 0.001302s | 0.001264s | 0.001304s | byte equality observed |
| for_each.test | 10 | 0.001353s | 0.001288s | 0.001344s | byte equality observed |
| int.test | 10 | 0.001320s | 0.001316s | 0.001364s | byte equality observed |
| integral_c.test | 10 | 0.001320s | 0.001333s | 0.001276s | byte equality observed |
| integral_c_unsigned.test | 10 | 0.001313s | 0.001335s | 0.001282s | byte equality observed |
| issue_69.test | 10 | 0.000611s | 0.000606s | 0.001316s | byte equality observed |
| issue_84.test | 10 | 0.000610s | 0.000609s | 0.001269s | byte equality observed |
| size_t.test | 10 | 0.001309s | 0.001316s | 0.001279s | byte equality observed |
| string.test | 10 | 0.001296s | 0.001331s | 0.001282s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
