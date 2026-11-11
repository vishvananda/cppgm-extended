# libs/mpl/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| increased_arity.test | COMPILE | 0.345206 | 0.832551 | 0.789855 | — | — | — |
| largest_int.test | COMPILE | 0.122397 | 0.268483 | 0.278496 | — | — | — |
| msvc_is_class.test | COMPILE | 0.014647 | 0.033105 | 0.039076 | — | — | — |
| template_arity.test | COMPILE | 0.122591 | 0.266004 | 0.275133 | — | — | — |
| is_seq.test | COMPILE | 0.005403 | 0.014256 | 0.027782 | — | — | — |
| token_equal.test | COMPILE | 0.005530 | 0.012868 | 0.026566 | — | — | — |
| advance.test | COMPILE | 0.134158 | 0.255106 | 0.318947 | — | — | — |
| always.test | COMPILE | 0.137202 | 0.257359 | 0.318319 | — | — | — |
| apply.test | COMPILE | 0.145226 | 0.264805 | 0.336900 | — | — | — |
| apply_wrap.test | COMPILE | 0.125173 | 0.235165 | 0.296494 | — | — | — |
| arithmetic.test | COMPILE | 0.135054 | 0.263384 | 0.306753 | — | — | — |
| as_sequence.test | COMPILE | 0.144866 | 0.286836 | 0.352282 | — | — | — |
| assert.test | COMPILE | 0.038568 | 0.060626 | 0.070988 | — | — | — |
| at.test | COMPILE | 0.142648 | 0.288372 | 0.348274 | — | — | — |
| back.test | COMPILE | 0.136998 | 0.270624 | 0.326168 | — | — | — |
| bind.test | COMPILE | 0.132929 | 0.247431 | 0.312111 | — | — | — |
| bitwise.test | COMPILE | 0.135079 | 0.250516 | 0.325620 | — | — | — |
| bool.test | RUN | 0.125256 | 0.238587 | 0.298645 | 0.001623 | 0.001608 | 0.001498 |
| comparison.test | COMPILE | 0.134818 | 0.263442 | 0.315641 | — | — | — |
| contains.test | COMPILE | 0.158986 | 0.317896 | 0.371288 | — | — | — |
| copy.test | COMPILE | 0.193937 | 0.387964 | 0.476197 | — | — | — |
| copy_if.test | COMPILE | 0.169303 | 0.338240 | 0.408811 | — | — | — |
| count.test | COMPILE | 0.162904 | 0.312660 | 0.393843 | — | — | — |
| count_if.test | COMPILE | 0.171965 | 0.331290 | 0.407139 | — | — | — |
| deque.test | COMPILE | 0.161250 | 0.326966 | 0.420011 | — | — | — |
| distance.test | COMPILE | 0.147946 | 0.294068 | 0.395337 | — | — | — |
| empty.test | COMPILE | 0.133246 | 0.263826 | 0.358340 | — | — | — |
| equal.test | COMPILE | 0.157316 | 0.308164 | 0.392219 | — | — | — |
| erase.test | COMPILE | 0.177543 | 0.354542 | 0.419745 | — | — | — |
| erase_range.test | COMPILE | 0.174349 | 0.341248 | 0.395556 | — | — | — |
| eval_if.test | COMPILE | 0.126457 | 0.245047 | 0.298966 | — | — | — |
| filter_view.test | COMPILE | 0.161762 | 0.342287 | 0.392320 | — | — | — |
| find.test | COMPILE | 0.160807 | 0.322433 | 0.377891 | — | — | — |
| find_if.test | COMPILE | 0.163137 | 0.332963 | 0.395020 | — | — | — |
| fold.test | COMPILE | 0.175795 | 0.340913 | 0.411186 | — | — | — |
| for_each.test | RUN | 0.248977 | 0.511573 | 0.638601 | 0.001787 | 0.001849 | 0.001928 |
| front.test | COMPILE | 0.142918 | 0.281141 | 0.324384 | — | — | — |
| get_tag_def.test | COMPILE | 0.126680 | 0.244898 | 0.277187 | — | — | — |
| has_xxx.test | COMPILE | 0.139130 | 0.253690 | 0.316460 | — | — | — |
| identity.test | COMPILE | 0.135252 | 0.281153 | 0.304747 | — | — | — |
| if.test | COMPILE | 0.122182 | 0.247729 | 0.285334 | — | — | — |
| index_of.test | COMPILE | 0.152687 | 0.311153 | 0.374992 | — | — | — |
| inherit.test | COMPILE | 0.130852 | 0.250946 | 0.296772 | — | — | — |
| insert.test | COMPILE | 0.180143 | 0.376845 | 0.430958 | — | — | — |
| insert_range.test | COMPILE | 0.237544 | 0.487505 | 0.631285 | — | — | — |
| int.test | RUN | 0.129166 | 0.268450 | 0.307548 | 0.001928 | 0.001486 | 0.001693 |
| char.test | RUN | 0.133181 | 0.273637 | 0.325136 | 0.001670 | 0.001764 | 0.001391 |
| char_unsigned.test | RUN | 0.132778 | 0.270184 | 0.311851 | 0.001878 | 0.001974 | 0.001938 |
| integral_c.test | RUN | 0.141260 | 0.341949 | 0.368461 | 0.001512 | 0.001684 | 0.001818 |
| integral_c_unsigned.test | RUN | 0.140649 | 0.340260 | 0.397007 | 0.001876 | 0.001824 | 0.001864 |
| is_placeholder.test | COMPILE | 0.127049 | 0.237861 | 0.297744 | — | — | — |
| is_sequence.test | COMPILE | 0.149700 | 0.301398 | 0.333860 | — | — | — |
| iterator_tags.test | COMPILE | 0.132140 | 0.259492 | 0.279942 | — | — | — |
| joint_view.test | COMPILE | 0.167221 | 0.330607 | 0.403482 | — | — | — |
| lambda.test | COMPILE | 0.154272 | 0.313175 | 0.374800 | — | — | — |
| lambda_args.test | COMPILE | 0.146185 | 0.287070 | 0.365138 | — | — | — |
| list.test | COMPILE | 0.153708 | 0.327322 | 0.365921 | — | — | — |
| list_c.test | COMPILE | 0.153305 | 0.315288 | 0.378642 | — | — | — |
| logical.test | COMPILE | 0.125599 | 0.256429 | 0.296975 | — | — | — |
| lower_bound.test | COMPILE | 0.164352 | 0.330476 | 0.374785 | — | — | — |
| map.test | COMPILE | 0.187558 | 0.407215 | 0.457654 | — | — | — |
| max_element.test | COMPILE | 0.158691 | 0.316180 | 0.350196 | — | — | — |
| min_max.test | COMPILE | 0.139779 | 0.283091 | 0.304496 | — | — | — |
| multiset.test | COMPILE | 0.171631 | 0.393855 | 0.382516 | — | — | — |
| next.test | COMPILE | 0.132861 | 0.302529 | 0.294764 | — | — | — |
| no_has_xxx.test | COMPILE | 0.144765 | 0.318477 | 0.326564 | — | — | — |
| numeric_ops.test | COMPILE | 0.153323 | 0.344552 | 0.349146 | — | — | — |
| pair_view.test | COMPILE | 0.170841 | 0.366501 | 0.384619 | — | — | — |
| partition.test | COMPILE | 0.179325 | 0.386161 | 0.434150 | — | — | — |
| pop_front.test | COMPILE | 0.148854 | 0.325905 | 0.362272 | — | — | — |
| push_back.test | COMPILE | 0.130071 | 0.272831 | 0.314197 | — | — | — |
| push_front.test | COMPILE | 0.153832 | 0.331094 | 0.365749 | — | — | — |
| quote.test | COMPILE | 0.127905 | 0.262875 | 0.300739 | — | — | — |
| range_c.test | COMPILE | 0.156412 | 0.366531 | 0.370184 | — | — | — |
| remove.test | COMPILE | 0.161178 | 0.382125 | 0.387540 | — | — | — |
| remove_if.test | COMPILE | 0.169895 | 0.418706 | 0.411980 | — | — | — |
| replace.test | COMPILE | 0.177340 | 0.428112 | 0.435863 | — | — | — |
| replace_if.test | COMPILE | 0.179335 | 0.414731 | 0.442447 | — | — | — |
| reverse.test | COMPILE | 0.175308 | 0.378607 | 0.416543 | — | — | — |
| same_as.test | COMPILE | 0.137323 | 0.285849 | 0.310656 | — | — | — |
| set.test | COMPILE | 0.225256 | 0.524555 | 0.643062 | — | — | — |
| set_c.test | COMPILE | 0.169491 | 0.359671 | 0.420974 | — | — | — |
| single_view.test | COMPILE | 0.153368 | 0.334168 | 0.367237 | — | — | — |
| size.test | COMPILE | 0.150117 | 0.304355 | 0.349346 | — | — | — |
| size_t.test | RUN | 0.135213 | 0.267522 | 0.322537 | 0.001551 | 0.001850 | 0.001756 |
| sizeof.test | COMPILE | 0.126550 | 0.233529 | 0.287570 | — | — | — |
| sort.test | COMPILE | 0.194145 | 0.393216 | 0.454269 | — | — | — |
| stable_partition.test | COMPILE | 0.183466 | 0.368265 | 0.424544 | — | — | — |
| transform.test | COMPILE | 0.189754 | 0.395064 | 0.467326 | — | — | — |
| transform_view.test | COMPILE | 0.157199 | 0.319316 | 0.374440 | — | — | — |
| unique.test | COMPILE | 0.165721 | 0.343044 | 0.393701 | — | — | — |
| unpack_args.test | COMPILE | 0.153783 | 0.305807 | 0.365296 | — | — | — |
| upper_bound.test | COMPILE | 0.160249 | 0.308443 | 0.364277 | — | — | — |
| vector.test | COMPILE | 0.179220 | 0.361947 | 0.438181 | — | — | — |
| vector_c.test | COMPILE | 0.162476 | 0.304024 | 0.383034 | — | — | — |
| zip_view.test | COMPILE | 0.217426 | 0.467621 | 0.548494 | — | — | — |
| string.test | RUN | 0.314914 | 0.908637 | 0.790268 | 0.001358 | 0.001462 | 0.001408 |
| issue_69.test | RUN | 0.016488 | 0.038945 | 0.043357 | 0.000611 | 0.000680 | 0.001693 |
| issue_84.test | RUN | 0.033881 | 0.074890 | 0.076701 | 0.000613 | 0.000607 | 0.001390 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.004× | 1.004×, 1.004× | 0.984× |
| clang | 1.093× | 1.072×, 1.114× | 1.013× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| bool.test | 10 | 0.000950s | 0.000948s | 0.000942s | byte equality observed |
| char.test | 10 | 0.000962s | 0.000949s | 0.000941s | byte equality observed |
| char_unsigned.test | 10 | 0.000962s | 0.000961s | 0.000950s | byte equality observed |
| for_each.test | 10 | 0.000966s | 0.000973s | 0.000950s | byte equality observed |
| int.test | 10 | 0.000955s | 0.000956s | 0.000948s | byte equality observed |
| integral_c.test | 10 | 0.000963s | 0.001000s | 0.000956s | byte equality observed |
| integral_c_unsigned.test | 10 | 0.000966s | 0.000965s | 0.000957s | byte equality observed |
| issue_69.test | 10 | 0.000454s | 0.000454s | 0.000938s | byte equality observed |
| issue_84.test | 10 | 0.000456s | 0.000451s | 0.000934s | byte equality observed |
| size_t.test | 10 | 0.000983s | 0.000984s | 0.000951s | byte equality observed |
| string.test | 10 | 0.000957s | 0.000950s | 0.000951s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
