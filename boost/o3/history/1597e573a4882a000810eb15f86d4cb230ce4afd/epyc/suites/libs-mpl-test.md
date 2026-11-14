# libs/mpl/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| increased_arity.test | COMPILE | 0.250557 | 0.730979 | 0.665678 | — | — | — |
| largest_int.test | COMPILE | 0.090875 | 0.205655 | 0.234641 | — | — | — |
| msvc_is_class.test | COMPILE | 0.011315 | 0.028723 | 0.030126 | — | — | — |
| template_arity.test | COMPILE | 0.089579 | 0.195482 | 0.231901 | — | — | — |
| is_seq.test | COMPILE | 0.004030 | 0.011112 | 0.021962 | — | — | — |
| token_equal.test | COMPILE | 0.004172 | 0.010149 | 0.021695 | — | — | — |
| advance.test | COMPILE | 0.097941 | 0.232503 | 0.264433 | — | — | — |
| always.test | COMPILE | 0.100104 | 0.233325 | 0.280094 | — | — | — |
| apply.test | COMPILE | 0.106408 | 0.238163 | 0.296077 | — | — | — |
| apply_wrap.test | COMPILE | 0.093054 | 0.210397 | 0.251389 | — | — | — |
| arithmetic.test | COMPILE | 0.099536 | 0.236843 | 0.281284 | — | — | — |
| as_sequence.test | COMPILE | 0.108300 | 0.259835 | 0.276682 | — | — | — |
| assert.test | COMPILE | 0.027145 | 0.052242 | 0.060185 | — | — | — |
| at.test | COMPILE | 0.119743 | 0.252756 | 0.293995 | — | — | — |
| back.test | COMPILE | 0.112349 | 0.244614 | 0.277227 | — | — | — |
| bind.test | COMPILE | 0.108976 | 0.228912 | 0.264585 | — | — | — |
| bitwise.test | COMPILE | 0.111882 | 0.236697 | 0.277684 | — | — | — |
| bool.test | RUN | 0.095253 | 0.208017 | 0.249922 | 0.001408 | 0.001433 | 0.001436 |
| comparison.test | COMPILE | 0.103231 | 0.231907 | 0.273682 | — | — | — |
| contains.test | COMPILE | 0.126913 | 0.284798 | 0.320190 | — | — | — |
| copy.test | COMPILE | 0.157606 | 0.345183 | 0.396898 | — | — | — |
| copy_if.test | COMPILE | 0.128356 | 0.302807 | 0.346789 | — | — | — |
| count.test | COMPILE | 0.125074 | 0.283146 | 0.335687 | — | — | — |
| count_if.test | COMPILE | 0.129444 | 0.288459 | 0.346311 | — | — | — |
| deque.test | COMPILE | 0.124005 | 0.290425 | 0.331629 | — | — | — |
| distance.test | COMPILE | 0.113874 | 0.263850 | 0.298818 | — | — | — |
| empty.test | COMPILE | 0.098539 | 0.225873 | 0.259298 | — | — | — |
| equal.test | COMPILE | 0.114805 | 0.265639 | 0.311258 | — | — | — |
| erase.test | COMPILE | 0.131578 | 0.310499 | 0.372193 | — | — | — |
| erase_range.test | COMPILE | 0.125411 | 0.300724 | 0.337799 | — | — | — |
| eval_if.test | COMPILE | 0.096202 | 0.215470 | 0.251802 | — | — | — |
| filter_view.test | COMPILE | 0.128500 | 0.303528 | 0.341222 | — | — | — |
| find.test | COMPILE | 0.124659 | 0.282644 | 0.328175 | — | — | — |
| find_if.test | COMPILE | 0.121242 | 0.292241 | 0.343601 | — | — | — |
| fold.test | COMPILE | 0.134492 | 0.309041 | 0.350374 | — | — | — |
| for_each.test | RUN | 0.195555 | 0.441401 | 0.562635 | 0.001501 | 0.001445 | 0.001481 |
| front.test | COMPILE | 0.105691 | 0.238818 | 0.277523 | — | — | — |
| get_tag_def.test | COMPILE | 0.094955 | 0.211225 | 0.245757 | — | — | — |
| has_xxx.test | COMPILE | 0.105693 | 0.227134 | 0.287517 | — | — | — |
| identity.test | COMPILE | 0.106898 | 0.237596 | 0.281370 | — | — | — |
| if.test | COMPILE | 0.095402 | 0.211689 | 0.255416 | — | — | — |
| index_of.test | COMPILE | 0.124333 | 0.277226 | 0.307846 | — | — | — |
| inherit.test | COMPILE | 0.098652 | 0.221915 | 0.262828 | — | — | — |
| insert.test | COMPILE | 0.137724 | 0.319327 | 0.377377 | — | — | — |
| insert_range.test | COMPILE | 0.182544 | 0.423473 | 0.540022 | — | — | — |
| int.test | RUN | 0.098712 | 0.233325 | 0.268796 | 0.001430 | 0.001314 | 0.001386 |
| char.test | RUN | 0.099408 | 0.238248 | 0.276791 | 0.001508 | 0.001475 | 0.001366 |
| char_unsigned.test | RUN | 0.100182 | 0.236795 | 0.272741 | 0.001411 | 0.001609 | 0.001447 |
| integral_c.test | RUN | 0.114937 | 0.294406 | 0.340097 | 0.001382 | 0.001591 | 0.001334 |
| integral_c_unsigned.test | RUN | 0.110658 | 0.295550 | 0.319945 | 0.001356 | 0.001352 | 0.001409 |
| is_placeholder.test | COMPILE | 0.095176 | 0.213773 | 0.253292 | — | — | — |
| is_sequence.test | COMPILE | 0.115474 | 0.259597 | 0.295237 | — | — | — |
| iterator_tags.test | COMPILE | 0.094814 | 0.215542 | 0.254734 | — | — | — |
| joint_view.test | COMPILE | 0.122066 | 0.289590 | 0.335585 | — | — | — |
| lambda.test | COMPILE | 0.119641 | 0.267329 | 0.305374 | — | — | — |
| lambda_args.test | COMPILE | 0.114109 | 0.257470 | 0.317762 | — | — | — |
| list.test | COMPILE | 0.121176 | 0.267845 | 0.315132 | — | — | — |
| list_c.test | COMPILE | 0.115538 | 0.264956 | 0.326445 | — | — | — |
| logical.test | COMPILE | 0.094621 | 0.208466 | 0.248243 | — | — | — |
| lower_bound.test | COMPILE | 0.113792 | 0.277779 | 0.320680 | — | — | — |
| map.test | COMPILE | 0.135557 | 0.340443 | 0.392657 | — | — | — |
| max_element.test | COMPILE | 0.113776 | 0.257288 | 0.308944 | — | — | — |
| min_max.test | COMPILE | 0.095961 | 0.228877 | 0.256618 | — | — | — |
| multiset.test | COMPILE | 0.121033 | 0.292393 | 0.334512 | — | — | — |
| next.test | COMPILE | 0.092686 | 0.208941 | 0.257153 | — | — | — |
| no_has_xxx.test | COMPILE | 0.099562 | 0.224583 | 0.286154 | — | — | — |
| numeric_ops.test | COMPILE | 0.113751 | 0.261539 | 0.302936 | — | — | — |
| pair_view.test | COMPILE | 0.134494 | 0.289570 | 0.338385 | — | — | — |
| partition.test | COMPILE | 0.148743 | 0.319170 | 0.374801 | — | — | — |
| pop_front.test | COMPILE | 0.122256 | 0.264576 | 0.300284 | — | — | — |
| push_back.test | COMPILE | 0.103897 | 0.221829 | 0.259006 | — | — | — |
| push_front.test | COMPILE | 0.117016 | 0.262718 | 0.311480 | — | — | — |
| quote.test | COMPILE | 0.093172 | 0.209872 | 0.251188 | — | — | — |
| range_c.test | COMPILE | 0.120099 | 0.282333 | 0.319103 | — | — | — |
| remove.test | COMPILE | 0.127315 | 0.296893 | 0.335341 | — | — | — |
| remove_if.test | COMPILE | 0.131053 | 0.298167 | 0.350759 | — | — | — |
| replace.test | COMPILE | 0.141164 | 0.324218 | 0.381009 | — | — | — |
| replace_if.test | COMPILE | 0.143568 | 0.334167 | 0.375446 | — | — | — |
| reverse.test | COMPILE | 0.135108 | 0.311848 | 0.356911 | — | — | — |
| same_as.test | COMPILE | 0.106680 | 0.229008 | 0.265744 | — | — | — |
| set.test | COMPILE | 0.175873 | 0.421038 | 0.539715 | — | — | — |
| set_c.test | COMPILE | 0.128719 | 0.294886 | 0.354476 | — | — | — |
| single_view.test | COMPILE | 0.120951 | 0.278001 | 0.322891 | — | — | — |
| size.test | COMPILE | 0.108929 | 0.260613 | 0.302788 | — | — | — |
| size_t.test | RUN | 0.097618 | 0.238915 | 0.283436 | 0.001273 | 0.001557 | 0.001673 |
| sizeof.test | COMPILE | 0.090801 | 0.204619 | 0.256623 | — | — | — |
| sort.test | COMPILE | 0.146966 | 0.344462 | 0.398539 | — | — | — |
| stable_partition.test | COMPILE | 0.141267 | 0.320628 | 0.377835 | — | — | — |
| transform.test | COMPILE | 0.142946 | 0.344692 | 0.404116 | — | — | — |
| transform_view.test | COMPILE | 0.122133 | 0.284323 | 0.329163 | — | — | — |
| unique.test | COMPILE | 0.122059 | 0.287746 | 0.337279 | — | — | — |
| unpack_args.test | COMPILE | 0.118742 | 0.275471 | 0.311859 | — | — | — |
| upper_bound.test | COMPILE | 0.118359 | 0.271324 | 0.322499 | — | — | — |
| vector.test | COMPILE | 0.134299 | 0.326796 | 0.386420 | — | — | — |
| vector_c.test | COMPILE | 0.119049 | 0.265640 | 0.330706 | — | — | — |
| zip_view.test | COMPILE | 0.160861 | 0.397673 | 0.470692 | — | — | — |
| string.test | RUN | 0.252409 | 0.817378 | 0.696696 | 0.001107 | 0.001113 | 0.001079 |
| issue_69.test | RUN | 0.012664 | 0.036858 | 0.038112 | 0.000462 | 0.000533 | 0.001372 |
| issue_84.test | RUN | 0.025847 | 0.067562 | 0.068619 | 0.000454 | 0.000469 | 0.001089 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.986×† | 0.984×, 0.988× | 0.793× |
| clang | 1.094× | 1.079×, 1.109× | 0.995× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026855).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| bool.test | 10 | 0.000874s | 0.000853s | 0.000864s | byte equality observed |
| char.test | 10 | 0.000860s | 0.000849s | 0.000847s | byte equality observed |
| char_unsigned.test | 10 | 0.000853s | 0.000844s | 0.000851s | byte equality observed |
| for_each.test | 10 | 0.000866s | 0.000859s | 0.000868s | byte equality observed |
| int.test | 10 | 0.000865s | 0.000854s | 0.000845s | byte equality observed |
| integral_c.test | 10 | 0.000865s | 0.000848s | 0.000856s | byte equality observed |
| integral_c_unsigned.test | 10 | 0.000870s | 0.000854s | 0.000855s | byte equality observed |
| issue_69.test | 10 | 0.000394s | 0.000394s | 0.000830s | byte equality observed |
| issue_84.test | 10 | 0.000405s | 0.000390s | 0.000856s | byte equality observed |
| size_t.test | 10 | 0.000879s | 0.000861s | 0.000849s | byte equality observed |
| string.test | 10 | 0.000877s | 0.000869s | 0.000873s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026855).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
