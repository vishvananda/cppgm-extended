# libs/mpl/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| increased_arity.test | COMPILE | 0.614233 | 1.258474 | 1.321331 | — | — | — |
| largest_int.test | COMPILE | 0.178981 | 0.377255 | 0.501457 | — | — | — |
| msvc_is_class.test | COMPILE | 0.020958 | 0.045508 | 0.053458 | — | — | — |
| template_arity.test | COMPILE | 0.178309 | 0.381079 | 0.493896 | — | — | — |
| is_seq.test | COMPILE | 0.005463 | 0.013756 | 0.031200 | — | — | — |
| token_equal.test | COMPILE | 0.006310 | 0.015921 | 0.032641 | — | — | — |
| advance.test | COMPILE | 0.187275 | 0.437121 | 0.537482 | — | — | — |
| always.test | COMPILE | 0.192443 | 0.435574 | 0.540099 | — | — | — |
| apply.test | COMPILE | 0.194417 | 0.453123 | 0.598859 | — | — | — |
| apply_wrap.test | COMPILE | 0.184942 | 0.386498 | 0.511859 | — | — | — |
| arithmetic.test | COMPILE | 0.196365 | 0.461995 | 0.536905 | — | — | — |
| as_sequence.test | COMPILE | 0.206697 | 0.464189 | 0.597988 | — | — | — |
| assert.test | COMPILE | 0.048978 | 0.085821 | 0.102201 | — | — | — |
| at.test | COMPILE | 0.208673 | 0.464459 | 0.606532 | — | — | — |
| back.test | COMPILE | 0.207527 | 0.464135 | 0.556946 | — | — | — |
| bind.test | COMPILE | 0.190122 | 0.419495 | 0.584237 | — | — | — |
| bitwise.test | COMPILE | 0.218771 | 0.425046 | 0.544154 | — | — | — |
| bool.test | RUN | 0.190159 | 0.396053 | 0.499708 | 0.002902 | 0.002077 | 0.002747 |
| comparison.test | COMPILE | 0.193225 | 0.442477 | 0.541732 | — | — | — |
| contains.test | COMPILE | 0.258886 | 0.533084 | 0.657672 | — | — | — |
| copy.test | COMPILE | 0.351501 | 0.588332 | 0.790452 | — | — | — |
| copy_if.test | COMPILE | 0.315654 | 0.551300 | 0.702796 | — | — | — |
| count.test | COMPILE | 0.317760 | 0.496453 | 0.666824 | — | — | — |
| count_if.test | COMPILE | 0.326844 | 0.535460 | 0.693924 | — | — | — |
| deque.test | COMPILE | 0.297825 | 0.541497 | 0.634151 | — | — | — |
| distance.test | COMPILE | 0.296802 | 0.459005 | 0.617140 | — | — | — |
| empty.test | COMPILE | 0.222282 | 0.426512 | 0.531300 | — | — | — |
| equal.test | COMPILE | 0.334942 | 0.477276 | 0.644056 | — | — | — |
| erase.test | COMPILE | 0.356773 | 0.569113 | 0.717076 | — | — | — |
| erase_range.test | COMPILE | 0.338604 | 0.540880 | 0.849496 | — | — | — |
| eval_if.test | COMPILE | 0.244402 | 0.534310 | 0.530336 | — | — | — |
| filter_view.test | COMPILE | 0.341240 | 0.531209 | 0.701682 | — | — | — |
| find.test | COMPILE | 0.359275 | 0.507559 | 0.679491 | — | — | — |
| find_if.test | COMPILE | 0.308615 | 0.529467 | 0.693452 | — | — | — |
| fold.test | COMPILE | 0.270922 | 0.644593 | 0.729668 | — | — | — |
| for_each.test | RUN | 0.350967 | 0.805246 | 1.154219 | 0.001980 | 0.001715 | 0.001613 |
| front.test | COMPILE | 0.188016 | 0.436177 | 0.565366 | — | — | — |
| get_tag_def.test | COMPILE | 0.166679 | 0.395348 | 0.538042 | — | — | — |
| has_xxx.test | COMPILE | 0.182725 | 0.450654 | 0.543200 | — | — | — |
| identity.test | COMPILE | 0.194454 | 0.428584 | 0.605925 | — | — | — |
| if.test | COMPILE | 0.183773 | 0.383164 | 0.495341 | — | — | — |
| index_of.test | COMPILE | 0.211360 | 0.476920 | 0.618506 | — | — | — |
| inherit.test | COMPILE | 0.172444 | 0.399946 | 0.524831 | — | — | — |
| insert.test | COMPILE | 0.257404 | 0.612254 | 0.720538 | — | — | — |
| insert_range.test | COMPILE | 0.328410 | 0.783940 | 1.052975 | — | — | — |
| int.test | RUN | 0.187361 | 0.417426 | 0.557135 | 0.001698 | 0.001821 | 0.001847 |
| char.test | RUN | 0.184866 | 0.443257 | 0.550668 | 0.001720 | 0.001787 | 0.001696 |
| char_unsigned.test | RUN | 0.179154 | 0.433182 | 0.560769 | 0.001796 | 0.001938 | 0.001667 |
| integral_c.test | RUN | 0.194334 | 0.547015 | 0.710938 | 0.001737 | 0.001639 | 0.001732 |
| integral_c_unsigned.test | RUN | 0.192786 | 0.548369 | 0.739211 | 0.001399 | 0.001750 | 0.001547 |
| is_placeholder.test | COMPILE | 0.184862 | 0.420222 | 0.516635 | — | — | — |
| is_sequence.test | COMPILE | 0.208860 | 0.472265 | 0.592316 | — | — | — |
| iterator_tags.test | COMPILE | 0.183485 | 0.401331 | 0.536954 | — | — | — |
| joint_view.test | COMPILE | 0.228273 | 0.517494 | 0.667279 | — | — | — |
| lambda.test | COMPILE | 0.224403 | 0.501388 | 0.630069 | — | — | — |
| lambda_args.test | COMPILE | 0.210676 | 0.469128 | 0.636925 | — | — | — |
| list.test | COMPILE | 0.215380 | 0.486408 | 0.617265 | — | — | — |
| list_c.test | COMPILE | 0.212101 | 0.667964 | 0.639635 | — | — | — |
| logical.test | COMPILE | 0.183204 | 0.390443 | 0.499787 | — | — | — |
| lower_bound.test | COMPILE | 0.228237 | 0.485618 | 0.635719 | — | — | — |
| map.test | COMPILE | 0.252547 | 0.650708 | 0.795541 | — | — | — |
| max_element.test | COMPILE | 0.207460 | 0.475602 | 0.618469 | — | — | — |
| min_max.test | COMPILE | 0.177612 | 0.394631 | 0.523374 | — | — | — |
| multiset.test | COMPILE | 0.221225 | 0.575097 | 0.629903 | — | — | — |
| next.test | COMPILE | 0.167497 | 0.382121 | 0.511622 | — | — | — |
| no_has_xxx.test | COMPILE | 0.181935 | 0.415094 | 0.536597 | — | — | — |
| numeric_ops.test | COMPILE | 0.201994 | 0.464778 | 0.563570 | — | — | — |
| pair_view.test | COMPILE | 0.229475 | 0.518043 | 0.671728 | — | — | — |
| partition.test | COMPILE | 0.248781 | 0.592647 | 0.706829 | — | — | — |
| pop_front.test | COMPILE | 0.205130 | 0.473268 | 0.578541 | — | — | — |
| push_back.test | COMPILE | 0.172942 | 0.417900 | 0.525184 | — | — | — |
| push_front.test | COMPILE | 0.210359 | 0.515809 | 0.611158 | — | — | — |
| quote.test | COMPILE | 0.176273 | 0.391262 | 0.488061 | — | — | — |
| range_c.test | COMPILE | 0.215649 | 0.578902 | 0.637901 | — | — | — |
| remove.test | COMPILE | 0.222465 | 0.535520 | 0.663107 | — | — | — |
| remove_if.test | COMPILE | 0.228613 | 0.569869 | 0.678303 | — | — | — |
| replace.test | COMPILE | 0.244303 | 0.630244 | 0.719138 | — | — | — |
| replace_if.test | COMPILE | 0.256730 | 0.924965 | 0.717134 | — | — | — |
| reverse.test | COMPILE | 0.240715 | 0.557469 | 0.679940 | — | — | — |
| same_as.test | COMPILE | 0.184610 | 0.433137 | 0.526538 | — | — | — |
| set.test | COMPILE | 0.319859 | 0.790014 | 1.061706 | — | — | — |
| set_c.test | COMPILE | 0.227874 | 0.536557 | 0.660585 | — | — | — |
| single_view.test | COMPILE | 0.210844 | 0.517841 | 0.627584 | — | — | — |
| size.test | COMPILE | 0.211774 | 0.468815 | 0.592878 | — | — | — |
| size_t.test | RUN | 0.184934 | 0.470276 | 0.556477 | 0.002260 | 0.002066 | 0.001818 |
| sizeof.test | COMPILE | 0.169120 | 0.382289 | 0.503559 | — | — | — |
| sort.test | COMPILE | 0.264998 | 0.618373 | 0.751223 | — | — | — |
| stable_partition.test | COMPILE | 0.257201 | 0.587417 | 0.721618 | — | — | — |
| transform.test | COMPILE | 0.263868 | 0.633228 | 0.797501 | — | — | — |
| transform_view.test | COMPILE | 0.217008 | 0.495221 | 0.621384 | — | — | — |
| unique.test | COMPILE | 0.238490 | 0.551443 | 0.653099 | — | — | — |
| unpack_args.test | COMPILE | 0.210532 | 0.481909 | 0.588316 | — | — | — |
| upper_bound.test | COMPILE | 0.214934 | 0.501475 | 0.601524 | — | — | — |
| vector.test | COMPILE | 0.238801 | 0.588435 | 0.717344 | — | — | — |
| vector_c.test | COMPILE | 0.211157 | 0.485381 | 0.655124 | — | — | — |
| zip_view.test | COMPILE | 0.295272 | 0.711397 | 0.913411 | — | — | — |
| string.test | RUN | 0.567303 | 1.878527 | 1.782286 | 0.002151 | 0.002673 | 0.002102 |
| issue_69.test | RUN | 0.020252 | 0.056242 | 0.060370 | 0.000702 | 0.000759 | 0.013065 |
| issue_84.test | RUN | 0.059842 | 0.102284 | 0.112984 | 0.000722 | 0.000849 | 0.001946 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.005× | 1.006×, 1.004× | 0.963× |
| clang | 1.067×† | 1.053×, 1.080× | 0.940× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1353 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| bool.test | 8 | 0.001662s | 0.001610s | 0.001589s | byte equality observed |
| char.test | 7 | 0.001611s | 0.001620s | 0.001586s | byte equality observed |
| char_unsigned.test | 7 | 0.001665s | 0.001622s | 0.001605s | byte equality observed |
| for_each.test | 8 | 0.001577s | 0.001642s | 0.001564s | byte equality observed |
| int.test | 8 | 0.001630s | 0.001620s | 0.001589s | byte equality observed |
| integral_c.test | 9 | 0.001557s | 0.001601s | 0.001643s | byte equality observed |
| integral_c_unsigned.test | 6 | 0.001599s | 0.001581s | 0.001564s | byte equality observed |
| issue_69.test | 8 | 0.000818s | 0.000835s | 0.001594s | byte equality observed |
| issue_84.test | 8 | 0.000823s | 0.000798s | 0.001606s | byte equality observed |
| size_t.test | 8 | 0.001705s | 0.001702s | 0.001597s | byte equality observed |
| string.test | 8 | 0.001617s | 0.001636s | 0.001621s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
