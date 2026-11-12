# libs/mpl/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| increased_arity.test | COMPILE | 0.509423 | 1.417266 | 1.527927 | — | — | — |
| largest_int.test | COMPILE | 0.168921 | 0.392036 | 0.590316 | — | — | — |
| msvc_is_class.test | COMPILE | 0.020330 | 0.051025 | 0.057242 | — | — | — |
| template_arity.test | COMPILE | 0.165696 | 0.382924 | 0.578906 | — | — | — |
| is_seq.test | COMPILE | 0.005997 | 0.018020 | 0.029496 | — | — | — |
| token_equal.test | COMPILE | 0.006139 | 0.018014 | 0.031983 | — | — | — |
| advance.test | COMPILE | 0.191050 | 0.444860 | 0.652841 | — | — | — |
| always.test | COMPILE | 0.194343 | 0.471815 | 0.639037 | — | — | — |
| apply.test | COMPILE | 0.205222 | 0.461238 | 0.660121 | — | — | — |
| apply_wrap.test | COMPILE | 0.175356 | 0.400187 | 0.601460 | — | — | — |
| arithmetic.test | COMPILE | 0.189439 | 0.460571 | 0.661294 | — | — | — |
| as_sequence.test | COMPILE | 0.210907 | 0.498805 | 0.649033 | — | — | — |
| assert.test | COMPILE | 0.048431 | 0.096660 | 0.124868 | — | — | — |
| at.test | COMPILE | 0.208192 | 0.481953 | 0.693757 | — | — | — |
| back.test | COMPILE | 0.198927 | 0.460840 | 0.650396 | — | — | — |
| bind.test | COMPILE | 0.194356 | 0.446735 | 0.647194 | — | — | — |
| bitwise.test | COMPILE | 0.193981 | 0.444804 | 0.645433 | — | — | — |
| bool.test | RUN | 0.175535 | 0.401031 | 0.578275 | 0.002007 | 0.002029 | 0.002256 |
| comparison.test | COMPILE | 0.192850 | 0.450291 | 0.629033 | — | — | — |
| contains.test | COMPILE | 0.229073 | 0.556024 | 0.727695 | — | — | — |
| copy.test | COMPILE | 0.288921 | 0.666965 | 0.946302 | — | — | — |
| copy_if.test | COMPILE | 0.248783 | 0.572096 | 0.824674 | — | — | — |
| count.test | COMPILE | 0.250776 | 0.530878 | 0.799829 | — | — | — |
| count_if.test | COMPILE | 0.272598 | 0.574253 | 0.832136 | — | — | — |
| deque.test | COMPILE | 0.253089 | 0.556918 | 0.784350 | — | — | — |
| distance.test | COMPILE | 0.225137 | 0.496822 | 0.735991 | — | — | — |
| empty.test | COMPILE | 0.197943 | 0.440218 | 0.632886 | — | — | — |
| equal.test | COMPILE | 0.229982 | 0.534377 | 0.759460 | — | — | — |
| erase.test | COMPILE | 0.264489 | 0.628901 | 0.865198 | — | — | — |
| erase_range.test | COMPILE | 0.240060 | 0.571944 | 0.828515 | — | — | — |
| eval_if.test | COMPILE | 0.181657 | 0.424875 | 0.581740 | — | — | — |
| filter_view.test | COMPILE | 0.238701 | 0.583051 | 0.807133 | — | — | — |
| find.test | COMPILE | 0.230118 | 0.543021 | 0.795013 | — | — | — |
| find_if.test | COMPILE | 0.244567 | 0.575522 | 0.806379 | — | — | — |
| fold.test | COMPILE | 0.253987 | 0.576496 | 0.849802 | — | — | — |
| for_each.test | RUN | 0.362067 | 0.877221 | 1.321994 | 0.002127 | 0.002105 | 0.002150 |
| front.test | COMPILE | 0.200160 | 0.462932 | 0.694782 | — | — | — |
| get_tag_def.test | COMPILE | 0.179890 | 0.410012 | 0.578687 | — | — | — |
| has_xxx.test | COMPILE | 0.191881 | 0.456854 | 0.643395 | — | — | — |
| identity.test | COMPILE | 0.206796 | 0.449388 | 0.638078 | — | — | — |
| if.test | COMPILE | 0.182620 | 0.392243 | 0.567134 | — | — | — |
| index_of.test | COMPILE | 0.226022 | 0.540523 | 0.757143 | — | — | — |
| inherit.test | COMPILE | 0.180010 | 0.420986 | 0.591372 | — | — | — |
| insert.test | COMPILE | 0.268729 | 0.611211 | 0.906750 | — | — | — |
| insert_range.test | COMPILE | 0.364189 | 0.843454 | 1.312582 | — | — | — |
| int.test | RUN | 0.190159 | 0.443149 | 0.658183 | 0.001813 | 0.001967 | 0.002178 |
| char.test | RUN | 0.190331 | 0.445709 | 0.685811 | 0.001762 | 0.001685 | 0.001764 |
| char_unsigned.test | RUN | 0.185609 | 0.458136 | 0.617518 | 0.002414 | 0.002198 | 0.002002 |
| integral_c.test | RUN | 0.208793 | 0.578679 | 0.767072 | 0.001902 | 0.002260 | 0.002213 |
| integral_c_unsigned.test | RUN | 0.235191 | 0.560210 | 0.791039 | 0.001861 | 0.002043 | 0.002115 |
| is_placeholder.test | COMPILE | 0.186654 | 0.412184 | 0.622955 | — | — | — |
| is_sequence.test | COMPILE | 0.214299 | 0.498697 | 0.682223 | — | — | — |
| iterator_tags.test | COMPILE | 0.182515 | 0.409990 | 0.603179 | — | — | — |
| joint_view.test | COMPILE | 0.237356 | 0.568720 | 0.815226 | — | — | — |
| lambda.test | COMPILE | 0.220118 | 0.533996 | 0.757048 | — | — | — |
| lambda_args.test | COMPILE | 0.210486 | 0.517427 | 0.750802 | — | — | — |
| list.test | COMPILE | 0.220550 | 0.546018 | 0.738938 | — | — | — |
| list_c.test | COMPILE | 0.222022 | 0.495334 | 0.765952 | — | — | — |
| logical.test | COMPILE | 0.181627 | 0.394144 | 0.601096 | — | — | — |
| lower_bound.test | COMPILE | 0.257491 | 0.530219 | 0.754376 | — | — | — |
| map.test | COMPILE | 0.275694 | 0.663557 | 0.934530 | — | — | — |
| max_element.test | COMPILE | 0.218503 | 0.487540 | 0.742236 | — | — | — |
| min_max.test | COMPILE | 0.182529 | 0.443692 | 0.593206 | — | — | — |
| multiset.test | COMPILE | 0.235926 | 0.567055 | 0.796664 | — | — | — |
| next.test | COMPILE | 0.180092 | 0.399502 | 0.601739 | — | — | — |
| no_has_xxx.test | COMPILE | 0.190276 | 0.439166 | 0.627580 | — | — | — |
| numeric_ops.test | COMPILE | 0.208003 | 0.502751 | 0.715463 | — | — | — |
| pair_view.test | COMPILE | 0.242241 | 0.553830 | 0.797800 | — | — | — |
| partition.test | COMPILE | 0.257657 | 0.616041 | 0.876884 | — | — | — |
| pop_front.test | COMPILE | 0.207179 | 0.513798 | 0.703195 | — | — | — |
| push_back.test | COMPILE | 0.183464 | 0.431944 | 0.627314 | — | — | — |
| push_front.test | COMPILE | 0.209876 | 0.501133 | 0.732513 | — | — | — |
| quote.test | COMPILE | 0.177078 | 0.401053 | 0.606975 | — | — | — |
| range_c.test | COMPILE | 0.224144 | 0.548707 | 0.752110 | — | — | — |
| remove.test | COMPILE | 0.239217 | 0.565806 | 0.794108 | — | — | — |
| remove_if.test | COMPILE | 0.237333 | 0.572932 | 0.798927 | — | — | — |
| replace.test | COMPILE | 0.257154 | 0.634486 | 0.884048 | — | — | — |
| replace_if.test | COMPILE | 0.264564 | 0.654594 | 0.870055 | — | — | — |
| reverse.test | COMPILE | 0.249866 | 0.612212 | 0.845612 | — | — | — |
| same_as.test | COMPILE | 0.192172 | 0.444712 | 0.623972 | — | — | — |
| set.test | COMPILE | 0.345189 | 0.836029 | 1.281807 | — | — | — |
| set_c.test | COMPILE | 0.249096 | 0.585259 | 0.837273 | — | — | — |
| single_view.test | COMPILE | 0.223334 | 0.517075 | 0.758529 | — | — | — |
| size.test | COMPILE | 0.216529 | 0.503319 | 0.722134 | — | — | — |
| size_t.test | RUN | 0.181373 | 0.452584 | 0.663506 | 0.001954 | 0.002195 | 0.001964 |
| sizeof.test | COMPILE | 0.170171 | 0.391632 | 0.624479 | — | — | — |
| sort.test | COMPILE | 0.281292 | 0.667049 | 0.945860 | — | — | — |
| stable_partition.test | COMPILE | 0.265549 | 0.622242 | 0.893618 | — | — | — |
| transform.test | COMPILE | 0.271381 | 0.661838 | 0.948509 | — | — | — |
| transform_view.test | COMPILE | 0.223943 | 0.522130 | 0.771044 | — | — | — |
| unique.test | COMPILE | 0.235154 | 0.566792 | 0.780782 | — | — | — |
| unpack_args.test | COMPILE | 0.220296 | 0.531152 | 0.729444 | — | — | — |
| upper_bound.test | COMPILE | 0.225782 | 0.510519 | 0.761655 | — | — | — |
| vector.test | COMPILE | 0.255909 | 0.629165 | 0.925156 | — | — | — |
| vector_c.test | COMPILE | 0.229133 | 0.527691 | 0.767769 | — | — | — |
| zip_view.test | COMPILE | 0.317738 | 0.774481 | 1.094718 | — | — | — |
| string.test | RUN | 0.494504 | 1.423088 | 1.421051 | 0.001461 | 0.001984 | 0.001758 |
| issue_69.test | RUN | 0.021949 | 0.062388 | 0.070552 | 0.000797 | 0.000854 | 0.001835 |
| issue_84.test | RUN | 0.046207 | 0.114195 | 0.136872 | 0.000604 | 0.000570 | 0.001483 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.967× | 0.969×, 0.964× | 0.981× |
| clang | 1.094× | 1.098×, 1.090× | 1.011× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898266).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| bool.test | 10 | 0.001306s | 0.001266s | 0.001292s | byte equality observed |
| char.test | 10 | 0.001320s | 0.001253s | 0.001301s | byte equality observed |
| char_unsigned.test | 10 | 0.001282s | 0.001246s | 0.001292s | byte equality observed |
| for_each.test | 10 | 0.001310s | 0.001281s | 0.001296s | byte equality observed |
| int.test | 10 | 0.001358s | 0.001280s | 0.001258s | byte equality observed |
| integral_c.test | 10 | 0.001306s | 0.001282s | 0.001281s | byte equality observed |
| integral_c_unsigned.test | 10 | 0.001282s | 0.001337s | 0.001258s | byte equality observed |
| issue_69.test | 10 | 0.000614s | 0.000598s | 0.001221s | byte equality observed |
| issue_84.test | 10 | 0.000616s | 0.000600s | 0.001326s | byte equality observed |
| size_t.test | 10 | 0.001296s | 0.001282s | 0.001234s | byte equality observed |
| string.test | 10 | 0.001290s | 0.001326s | 0.001315s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898266).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
