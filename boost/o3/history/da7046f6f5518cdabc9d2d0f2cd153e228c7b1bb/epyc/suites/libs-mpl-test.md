# libs/mpl/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| increased_arity.test | COMPILE | 0.302829 | 0.735784 | 0.695542 | — | — | — |
| largest_int.test | COMPILE | 0.104699 | 0.205169 | 0.267421 | — | — | — |
| msvc_is_class.test | COMPILE | 0.012786 | 0.028604 | 0.033032 | — | — | — |
| template_arity.test | COMPILE | 0.103770 | 0.201276 | 0.265270 | — | — | — |
| is_seq.test | COMPILE | 0.004456 | 0.011027 | 0.022724 | — | — | — |
| token_equal.test | COMPILE | 0.004780 | 0.011031 | 0.023033 | — | — | — |
| advance.test | COMPILE | 0.118704 | 0.235680 | 0.284242 | — | — | — |
| always.test | COMPILE | 0.118354 | 0.237855 | 0.287737 | — | — | — |
| apply.test | COMPILE | 0.124733 | 0.240789 | 0.300603 | — | — | — |
| apply_wrap.test | COMPILE | 0.106205 | 0.209091 | 0.255741 | — | — | — |
| arithmetic.test | COMPILE | 0.114044 | 0.234260 | 0.298005 | — | — | — |
| as_sequence.test | COMPILE | 0.125436 | 0.258510 | 0.313386 | — | — | — |
| assert.test | COMPILE | 0.033882 | 0.052130 | 0.063494 | — | — | — |
| at.test | COMPILE | 0.119220 | 0.249626 | 0.318605 | — | — | — |
| back.test | COMPILE | 0.116377 | 0.238523 | 0.300236 | — | — | — |
| bind.test | COMPILE | 0.114001 | 0.231179 | 0.289038 | — | — | — |
| bitwise.test | COMPILE | 0.118498 | 0.236614 | 0.298144 | — | — | — |
| bool.test | RUN | 0.106190 | 0.204999 | 0.269342 | 0.001275 | 0.001537 | 0.001455 |
| comparison.test | COMPILE | 0.113284 | 0.230575 | 0.288220 | — | — | — |
| contains.test | COMPILE | 0.135311 | 0.284909 | 0.338073 | — | — | — |
| copy.test | COMPILE | 0.165245 | 0.342146 | 0.414739 | — | — | — |
| copy_if.test | COMPILE | 0.143666 | 0.297633 | 0.367756 | — | — | — |
| count.test | COMPILE | 0.138239 | 0.281938 | 0.351769 | — | — | — |
| count_if.test | COMPILE | 0.193844 | 0.288598 | 0.363444 | — | — | — |
| deque.test | COMPILE | 0.185859 | 0.288610 | 0.347800 | — | — | — |
| distance.test | COMPILE | 0.154321 | 0.260630 | 0.325044 | — | — | — |
| empty.test | COMPILE | 0.128013 | 0.233675 | 0.292776 | — | — | — |
| equal.test | COMPILE | 0.131642 | 0.262590 | 0.324383 | — | — | — |
| erase.test | COMPILE | 0.151083 | 0.313933 | 0.378603 | — | — | — |
| erase_range.test | COMPILE | 0.142566 | 0.298632 | 0.367887 | — | — | — |
| eval_if.test | COMPILE | 0.112288 | 0.214173 | 0.266830 | — | — | — |
| filter_view.test | COMPILE | 0.141410 | 0.291443 | 0.361093 | — | — | — |
| find.test | COMPILE | 0.138843 | 0.287714 | 0.342387 | — | — | — |
| find_if.test | COMPILE | 0.147240 | 0.293395 | 0.351819 | — | — | — |
| fold.test | COMPILE | 0.148451 | 0.307564 | 0.377058 | — | — | — |
| for_each.test | RUN | 0.213976 | 0.431762 | 0.572397 | 0.001465 | 0.001339 | 0.001549 |
| front.test | COMPILE | 0.119277 | 0.247893 | 0.299152 | — | — | — |
| get_tag_def.test | COMPILE | 0.111329 | 0.204821 | 0.259027 | — | — | — |
| has_xxx.test | COMPILE | 0.116642 | 0.227897 | 0.289422 | — | — | — |
| identity.test | COMPILE | 0.119987 | 0.233240 | 0.285500 | — | — | — |
| if.test | COMPILE | 0.107548 | 0.203220 | 0.250576 | — | — | — |
| index_of.test | COMPILE | 0.134266 | 0.270263 | 0.329563 | — | — | — |
| inherit.test | COMPILE | 0.110461 | 0.215099 | 0.269272 | — | — | — |
| insert.test | COMPILE | 0.151103 | 0.319546 | 0.403614 | — | — | — |
| insert_range.test | COMPILE | 0.199536 | 0.421843 | 0.573516 | — | — | — |
| int.test | RUN | 0.112513 | 0.232989 | 0.287196 | 0.001419 | 0.001460 | 0.001502 |
| char.test | RUN | 0.113836 | 0.237713 | 0.303200 | 0.001301 | 0.001617 | 0.001490 |
| char_unsigned.test | RUN | 0.113174 | 0.235958 | 0.280872 | 0.001340 | 0.001635 | 0.001556 |
| integral_c.test | RUN | 0.121918 | 0.292481 | 0.362397 | 0.001507 | 0.001558 | 0.001607 |
| integral_c_unsigned.test | RUN | 0.121145 | 0.298906 | 0.364410 | 0.001323 | 0.001421 | 0.001596 |
| is_placeholder.test | COMPILE | 0.110290 | 0.206780 | 0.252048 | — | — | — |
| is_sequence.test | COMPILE | 0.138732 | 0.257725 | 0.323307 | — | — | — |
| iterator_tags.test | COMPILE | 0.114846 | 0.213555 | 0.277160 | — | — | — |
| joint_view.test | COMPILE | 0.144042 | 0.290799 | 0.361437 | — | — | — |
| lambda.test | COMPILE | 0.138062 | 0.263547 | 0.342671 | — | — | — |
| lambda_args.test | COMPILE | 0.128387 | 0.256247 | 0.329914 | — | — | — |
| list.test | COMPILE | 0.133755 | 0.265500 | 0.331903 | — | — | — |
| list_c.test | COMPILE | 0.131124 | 0.261959 | 0.341674 | — | — | — |
| logical.test | COMPILE | 0.108989 | 0.207984 | 0.275843 | — | — | — |
| lower_bound.test | COMPILE | 0.140115 | 0.275375 | 0.331683 | — | — | — |
| map.test | COMPILE | 0.155695 | 0.333905 | 0.426011 | — | — | — |
| max_element.test | COMPILE | 0.129855 | 0.255651 | 0.328446 | — | — | — |
| min_max.test | COMPILE | 0.112870 | 0.226013 | 0.286603 | — | — | — |
| multiset.test | COMPILE | 0.139691 | 0.287879 | 0.356640 | — | — | — |
| next.test | COMPILE | 0.108143 | 0.207630 | 0.261157 | — | — | — |
| no_has_xxx.test | COMPILE | 0.114759 | 0.228871 | 0.298867 | — | — | — |
| numeric_ops.test | COMPILE | 0.124290 | 0.254946 | 0.320534 | — | — | — |
| pair_view.test | COMPILE | 0.143233 | 0.293394 | 0.357383 | — | — | — |
| partition.test | COMPILE | 0.153761 | 0.312542 | 0.404335 | — | — | — |
| pop_front.test | COMPILE | 0.127524 | 0.261878 | 0.308893 | — | — | — |
| push_back.test | COMPILE | 0.110689 | 0.222899 | 0.276622 | — | — | — |
| push_front.test | COMPILE | 0.128803 | 0.261264 | 0.329341 | — | — | — |
| quote.test | COMPILE | 0.110218 | 0.206745 | 0.273279 | — | — | — |
| range_c.test | COMPILE | 0.133467 | 0.278297 | 0.338851 | — | — | — |
| remove.test | COMPILE | 0.139326 | 0.295682 | 0.352735 | — | — | — |
| remove_if.test | COMPILE | 0.143153 | 0.294215 | 0.374289 | — | — | — |
| replace.test | COMPILE | 0.153449 | 0.322696 | 0.392137 | — | — | — |
| replace_if.test | COMPILE | 0.155116 | 0.331352 | 0.395395 | — | — | — |
| reverse.test | COMPILE | 0.148357 | 0.309791 | 0.369935 | — | — | — |
| same_as.test | COMPILE | 0.117954 | 0.228196 | 0.290141 | — | — | — |
| set.test | COMPILE | 0.188423 | 0.416641 | 0.553319 | — | — | — |
| set_c.test | COMPILE | 0.143830 | 0.298187 | 0.371891 | — | — | — |
| single_view.test | COMPILE | 0.132088 | 0.269929 | 0.337248 | — | — | — |
| size.test | COMPILE | 0.132001 | 0.253641 | 0.317602 | — | — | — |
| size_t.test | RUN | 0.114411 | 0.237270 | 0.287970 | 0.001438 | 0.001525 | 0.001605 |
| sizeof.test | COMPILE | 0.108440 | 0.203620 | 0.258215 | — | — | — |
| sort.test | COMPILE | 0.165566 | 0.340710 | 0.416495 | — | — | — |
| stable_partition.test | COMPILE | 0.154482 | 0.316849 | 0.392011 | — | — | — |
| transform.test | COMPILE | 0.161949 | 0.339316 | 0.425047 | — | — | — |
| transform_view.test | COMPILE | 0.135849 | 0.269429 | 0.336777 | — | — | — |
| unique.test | COMPILE | 0.140998 | 0.295976 | 0.363836 | — | — | — |
| unpack_args.test | COMPILE | 0.133293 | 0.276910 | 0.336868 | — | — | — |
| upper_bound.test | COMPILE | 0.135765 | 0.263826 | 0.331219 | — | — | — |
| vector.test | COMPILE | 0.151577 | 0.320631 | 0.388702 | — | — | — |
| vector_c.test | COMPILE | 0.136078 | 0.274530 | 0.343948 | — | — | — |
| zip_view.test | COMPILE | 0.184160 | 0.401338 | 0.482853 | — | — | — |
| string.test | RUN | 0.282795 | 0.805668 | 0.732350 | 0.001092 | 0.001124 | 0.001129 |
| issue_69.test | RUN | 0.014322 | 0.036660 | 0.039855 | 0.000549 | 0.000620 | 0.001333 |
| issue_84.test | RUN | 0.036915 | 0.064352 | 0.069610 | 0.000461 | 0.000481 | 0.001162 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.042×† | 1.005×, 1.079× | 0.936× |
| clang | 1.114×† | 1.132×, 1.096× | 0.912× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| bool.test | 10 | 0.000944s | 0.000964s | 0.000994s | byte equality observed |
| char.test | 10 | 0.000953s | 0.000936s | 0.001010s | byte equality observed |
| char_unsigned.test | 10 | 0.000951s | 0.000940s | 0.001019s | byte equality observed |
| for_each.test | 10 | 0.000959s | 0.000951s | 0.001017s | byte equality observed |
| int.test | 10 | 0.000958s | 0.000948s | 0.000988s | byte equality observed |
| integral_c.test | 10 | 0.000945s | 0.000982s | 0.000986s | byte equality observed |
| integral_c_unsigned.test | 10 | 0.000963s | 0.000986s | 0.001005s | byte equality observed |
| issue_69.test | 10 | 0.000445s | 0.000460s | 0.000970s | byte equality observed |
| issue_84.test | 10 | 0.000452s | 0.000452s | 0.000966s | byte equality observed |
| size_t.test | 10 | 0.000962s | 0.000982s | 0.001010s | byte equality observed |
| string.test | 10 | 0.000972s | 0.001003s | 0.001000s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
