# libs/mpl/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| increased_arity.test | COMPILE | 0.358702 | 1.002081 | 1.031639 | — | — | — |
| largest_int.test | COMPILE | 0.125145 | 0.276415 | 0.338038 | — | — | — |
| msvc_is_class.test | COMPILE | 0.015142 | 0.037349 | 0.043057 | — | — | — |
| template_arity.test | COMPILE | 0.120973 | 0.274475 | 0.345435 | — | — | — |
| is_seq.test | COMPILE | 0.004958 | 0.013282 | 0.027000 | — | — | — |
| token_equal.test | COMPILE | 0.004970 | 0.013177 | 0.027295 | — | — | — |
| advance.test | COMPILE | 0.135333 | 0.303182 | 0.418683 | — | — | — |
| always.test | COMPILE | 0.137235 | 0.321956 | 0.431676 | — | — | — |
| apply.test | COMPILE | 0.144031 | 0.338020 | 0.432675 | — | — | — |
| apply_wrap.test | COMPILE | 0.121262 | 0.289797 | 0.381814 | — | — | — |
| arithmetic.test | COMPILE | 0.134630 | 0.319734 | 0.397539 | — | — | — |
| as_sequence.test | COMPILE | 0.144508 | 0.353907 | 0.457117 | — | — | — |
| assert.test | COMPILE | 0.034965 | 0.070296 | 0.086583 | — | — | — |
| at.test | COMPILE | 0.145055 | 0.345006 | 0.449173 | — | — | — |
| back.test | COMPILE | 0.135211 | 0.334971 | 0.424435 | — | — | — |
| bind.test | COMPILE | 0.134155 | 0.312634 | 0.422035 | — | — | — |
| bitwise.test | COMPILE | 0.134362 | 0.326282 | 0.432587 | — | — | — |
| bool.test | RUN | 0.123031 | 0.289052 | 0.377071 | 0.001528 | 0.001649 | 0.001729 |
| comparison.test | COMPILE | 0.132194 | 0.329445 | 0.404458 | — | — | — |
| contains.test | COMPILE | 0.164635 | 0.378925 | 0.492337 | — | — | — |
| copy.test | COMPILE | 0.199633 | 0.460861 | 0.611523 | — | — | — |
| copy_if.test | COMPILE | 0.171928 | 0.413911 | 0.542691 | — | — | — |
| count.test | COMPILE | 0.166862 | 0.382729 | 0.500697 | — | — | — |
| count_if.test | COMPILE | 0.172258 | 0.404898 | 0.539945 | — | — | — |
| deque.test | COMPILE | 0.165305 | 0.397846 | 0.509514 | — | — | — |
| distance.test | COMPILE | 0.151995 | 0.357505 | 0.459114 | — | — | — |
| empty.test | COMPILE | 0.133995 | 0.308520 | 0.412733 | — | — | — |
| equal.test | COMPILE | 0.158863 | 0.369502 | 0.490029 | — | — | — |
| erase.test | COMPILE | 0.183433 | 0.433434 | 0.569144 | — | — | — |
| erase_range.test | COMPILE | 0.162847 | 0.407274 | 0.526269 | — | — | — |
| eval_if.test | COMPILE | 0.128724 | 0.294163 | 0.394763 | — | — | — |
| filter_view.test | COMPILE | 0.172285 | 0.413204 | 0.536186 | — | — | — |
| find.test | COMPILE | 0.161672 | 0.387348 | 0.512245 | — | — | — |
| find_if.test | COMPILE | 0.171654 | 0.409204 | 0.526415 | — | — | — |
| fold.test | COMPILE | 0.175480 | 0.417213 | 0.556668 | — | — | — |
| for_each.test | RUN | 0.256720 | 0.609222 | 0.882671 | 0.001552 | 0.001605 | 0.001702 |
| front.test | COMPILE | 0.137383 | 0.331256 | 0.436810 | — | — | — |
| get_tag_def.test | COMPILE | 0.121638 | 0.288558 | 0.391831 | — | — | — |
| has_xxx.test | COMPILE | 0.130376 | 0.317408 | 0.443577 | — | — | — |
| identity.test | COMPILE | 0.142715 | 0.330808 | 0.433324 | — | — | — |
| if.test | COMPILE | 0.120815 | 0.286416 | 0.397071 | — | — | — |
| index_of.test | COMPILE | 0.158810 | 0.366224 | 0.490454 | — | — | — |
| inherit.test | COMPILE | 0.126879 | 0.299140 | 0.411931 | — | — | — |
| insert.test | COMPILE | 0.184428 | 0.445677 | 0.577318 | — | — | — |
| insert_range.test | COMPILE | 0.256323 | 0.592128 | 0.828996 | — | — | — |
| int.test | RUN | 0.133232 | 0.323281 | 0.426410 | 0.001486 | 0.001681 | 0.001502 |
| char.test | RUN | 0.135001 | 0.334027 | 0.432157 | 0.001551 | 0.001686 | 0.001470 |
| char_unsigned.test | RUN | 0.129187 | 0.321220 | 0.422439 | 0.001697 | 0.001810 | 0.001781 |
| integral_c.test | RUN | 0.146848 | 0.411596 | 0.491130 | 0.001348 | 0.001772 | 0.001634 |
| integral_c_unsigned.test | RUN | 0.145205 | 0.405810 | 0.516598 | 0.001464 | 0.001675 | 0.001575 |
| is_placeholder.test | COMPILE | 0.123009 | 0.293422 | 0.404183 | — | — | — |
| is_sequence.test | COMPILE | 0.146314 | 0.351749 | 0.435383 | — | — | — |
| iterator_tags.test | COMPILE | 0.126725 | 0.294269 | 0.371875 | — | — | — |
| joint_view.test | COMPILE | 0.170166 | 0.396425 | 0.517576 | — | — | — |
| lambda.test | COMPILE | 0.155660 | 0.361639 | 0.485368 | — | — | — |
| lambda_args.test | COMPILE | 0.147310 | 0.340053 | 0.493903 | — | — | — |
| list.test | COMPILE | 0.153534 | 0.372268 | 0.472182 | — | — | — |
| list_c.test | COMPILE | 0.153209 | 0.357749 | 0.480401 | — | — | — |
| logical.test | COMPILE | 0.128192 | 0.284992 | 0.370109 | — | — | — |
| lower_bound.test | COMPILE | 0.159211 | 0.380906 | 0.486688 | — | — | — |
| map.test | COMPILE | 0.185493 | 0.469950 | 0.610542 | — | — | — |
| max_element.test | COMPILE | 0.151517 | 0.349457 | 0.465812 | — | — | — |
| min_max.test | COMPILE | 0.128750 | 0.314925 | 0.408304 | — | — | — |
| multiset.test | COMPILE | 0.168124 | 0.408477 | 0.514872 | — | — | — |
| next.test | COMPILE | 0.126557 | 0.298185 | 0.378651 | — | — | — |
| no_has_xxx.test | COMPILE | 0.134738 | 0.307699 | 0.430958 | — | — | — |
| numeric_ops.test | COMPILE | 0.141855 | 0.362018 | 0.467273 | — | — | — |
| pair_view.test | COMPILE | 0.169632 | 0.394153 | 0.517571 | — | — | — |
| partition.test | COMPILE | 0.186606 | 0.440194 | 0.573193 | — | — | — |
| pop_front.test | COMPILE | 0.153840 | 0.357512 | 0.459909 | — | — | — |
| push_back.test | COMPILE | 0.127429 | 0.304122 | 0.408689 | — | — | — |
| push_front.test | COMPILE | 0.150767 | 0.358421 | 0.467962 | — | — | — |
| quote.test | COMPILE | 0.127324 | 0.289454 | 0.374985 | — | — | — |
| range_c.test | COMPILE | 0.158299 | 0.380805 | 0.489007 | — | — | — |
| remove.test | COMPILE | 0.163631 | 0.404243 | 0.520919 | — | — | — |
| remove_if.test | COMPILE | 0.174703 | 0.413930 | 0.529566 | — | — | — |
| replace.test | COMPILE | 0.189119 | 0.454777 | 0.566943 | — | — | — |
| replace_if.test | COMPILE | 0.188065 | 0.454914 | 0.583825 | — | — | — |
| reverse.test | COMPILE | 0.178927 | 0.424424 | 0.557712 | — | — | — |
| same_as.test | COMPILE | 0.134385 | 0.306869 | 0.405560 | — | — | — |
| set.test | COMPILE | 0.242286 | 0.598466 | 0.816282 | — | — | — |
| set_c.test | COMPILE | 0.174684 | 0.402265 | 0.559048 | — | — | — |
| single_view.test | COMPILE | 0.160035 | 0.372742 | 0.480732 | — | — | — |
| size.test | COMPILE | 0.153907 | 0.358417 | 0.460079 | — | — | — |
| size_t.test | RUN | 0.132282 | 0.332380 | 0.416647 | 0.001536 | 0.001680 | 0.001784 |
| sizeof.test | COMPILE | 0.124551 | 0.291182 | 0.363396 | — | — | — |
| sort.test | COMPILE | 0.200870 | 0.469172 | 0.608837 | — | — | — |
| stable_partition.test | COMPILE | 0.189352 | 0.441865 | 0.550272 | — | — | — |
| transform.test | COMPILE | 0.200045 | 0.469169 | 0.617105 | — | — | — |
| transform_view.test | COMPILE | 0.161211 | 0.389142 | 0.493944 | — | — | — |
| unique.test | COMPILE | 0.169046 | 0.393616 | 0.507391 | — | — | — |
| unpack_args.test | COMPILE | 0.156810 | 0.367944 | 0.481347 | — | — | — |
| upper_bound.test | COMPILE | 0.165018 | 0.370015 | 0.481164 | — | — | — |
| vector.test | COMPILE | 0.186091 | 0.451701 | 0.573003 | — | — | — |
| vector_c.test | COMPILE | 0.163862 | 0.366831 | 0.511509 | — | — | — |
| zip_view.test | COMPILE | 0.228816 | 0.558379 | 0.708025 | — | — | — |
| string.test | RUN | 0.376680 | 1.084821 | 1.044139 | 0.001315 | 0.001293 | 0.001244 |
| issue_69.test | RUN | 0.017308 | 0.048077 | 0.050896 | 0.000584 | 0.000661 | 0.001501 |
| issue_84.test | RUN | 0.030810 | 0.084099 | 0.088742 | 0.000526 | 0.000576 | 0.001334 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.018×† | 1.029×, 1.008× | 0.942× |
| clang | 1.091× | 1.088×, 1.093× | 0.996× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224918).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| bool.test | 10 | 0.000987s | 0.001000s | 0.001029s | byte equality observed |
| char.test | 10 | 0.001003s | 0.001006s | 0.001013s | byte equality observed |
| char_unsigned.test | 10 | 0.000993s | 0.001000s | 0.001039s | byte equality observed |
| for_each.test | 10 | 0.001007s | 0.001029s | 0.001014s | byte equality observed |
| int.test | 10 | 0.000998s | 0.001023s | 0.000984s | byte equality observed |
| integral_c.test | 10 | 0.000990s | 0.001012s | 0.000991s | byte equality observed |
| integral_c_unsigned.test | 10 | 0.001015s | 0.001012s | 0.000993s | byte equality observed |
| issue_69.test | 10 | 0.000464s | 0.000449s | 0.000963s | byte equality observed |
| issue_84.test | 10 | 0.000458s | 0.000464s | 0.000985s | byte equality observed |
| size_t.test | 10 | 0.001021s | 0.001023s | 0.000970s | byte equality observed |
| string.test | 10 | 0.001004s | 0.000985s | 0.000976s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224918).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
