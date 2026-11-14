# libs/mpl/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| increased_arity.test | COMPILE | 0.353382 | 0.989231 | 1.031269 | — | — | — |
| largest_int.test | COMPILE | 0.123790 | 0.277650 | 0.352375 | — | — | — |
| msvc_is_class.test | COMPILE | 0.014479 | 0.037342 | 0.043144 | — | — | — |
| template_arity.test | COMPILE | 0.122945 | 0.277535 | 0.344646 | — | — | — |
| is_seq.test | COMPILE | 0.004980 | 0.012616 | 0.026132 | — | — | — |
| token_equal.test | COMPILE | 0.005140 | 0.013230 | 0.026182 | — | — | — |
| advance.test | COMPILE | 0.135197 | 0.314833 | 0.414046 | — | — | — |
| always.test | COMPILE | 0.138326 | 0.328469 | 0.430074 | — | — | — |
| apply.test | COMPILE | 0.146480 | 0.328191 | 0.427755 | — | — | — |
| apply_wrap.test | COMPILE | 0.126978 | 0.285497 | 0.380712 | — | — | — |
| arithmetic.test | COMPILE | 0.137947 | 0.330883 | 0.428733 | — | — | — |
| as_sequence.test | COMPILE | 0.148507 | 0.354962 | 0.421496 | — | — | — |
| assert.test | COMPILE | 0.035436 | 0.072248 | 0.087101 | — | — | — |
| at.test | COMPILE | 0.148920 | 0.343239 | 0.445812 | — | — | — |
| back.test | COMPILE | 0.141735 | 0.334564 | 0.427205 | — | — | — |
| bind.test | COMPILE | 0.137422 | 0.319453 | 0.420268 | — | — | — |
| bitwise.test | COMPILE | 0.139132 | 0.328593 | 0.425108 | — | — | — |
| bool.test | RUN | 0.125563 | 0.289280 | 0.373307 | 0.001520 | 0.001540 | 0.001744 |
| comparison.test | COMPILE | 0.136315 | 0.323363 | 0.406208 | — | — | — |
| contains.test | COMPILE | 0.165534 | 0.399729 | 0.492706 | — | — | — |
| copy.test | COMPILE | 0.201768 | 0.471914 | 0.604467 | — | — | — |
| copy_if.test | COMPILE | 0.174737 | 0.417394 | 0.535035 | — | — | — |
| count.test | COMPILE | 0.166265 | 0.379343 | 0.519319 | — | — | — |
| count_if.test | COMPILE | 0.174845 | 0.412273 | 0.543110 | — | — | — |
| deque.test | COMPILE | 0.167589 | 0.396821 | 0.499090 | — | — | — |
| distance.test | COMPILE | 0.157391 | 0.357770 | 0.467631 | — | — | — |
| empty.test | COMPILE | 0.136563 | 0.319986 | 0.408161 | — | — | — |
| equal.test | COMPILE | 0.158815 | 0.386491 | 0.492310 | — | — | — |
| erase.test | COMPILE | 0.183889 | 0.444836 | 0.560207 | — | — | — |
| erase_range.test | COMPILE | 0.173398 | 0.413173 | 0.529620 | — | — | — |
| eval_if.test | COMPILE | 0.125761 | 0.293307 | 0.380303 | — | — | — |
| filter_view.test | COMPILE | 0.172411 | 0.402473 | 0.522118 | — | — | — |
| find.test | COMPILE | 0.169738 | 0.389289 | 0.507146 | — | — | — |
| find_if.test | COMPILE | 0.172509 | 0.406787 | 0.522946 | — | — | — |
| fold.test | COMPILE | 0.181014 | 0.420626 | 0.558736 | — | — | — |
| for_each.test | RUN | 0.259259 | 0.619542 | 0.854904 | 0.001551 | 0.001828 | 0.001850 |
| front.test | COMPILE | 0.140733 | 0.335319 | 0.424751 | — | — | — |
| get_tag_def.test | COMPILE | 0.124739 | 0.299776 | 0.399465 | — | — | — |
| has_xxx.test | COMPILE | 0.139580 | 0.312936 | 0.421858 | — | — | — |
| identity.test | COMPILE | 0.142654 | 0.331339 | 0.418984 | — | — | — |
| if.test | COMPILE | 0.125250 | 0.285184 | 0.366359 | — | — | — |
| index_of.test | COMPILE | 0.161402 | 0.388409 | 0.478016 | — | — | — |
| inherit.test | COMPILE | 0.129202 | 0.291480 | 0.388054 | — | — | — |
| insert.test | COMPILE | 0.187102 | 0.450771 | 0.578413 | — | — | — |
| insert_range.test | COMPILE | 0.253234 | 0.610302 | 0.854368 | — | — | — |
| int.test | RUN | 0.131214 | 0.337979 | 0.417512 | 0.001572 | 0.001407 | 0.001587 |
| char.test | RUN | 0.131589 | 0.328768 | 0.429920 | 0.001641 | 0.001740 | 0.001678 |
| char_unsigned.test | RUN | 0.136110 | 0.328438 | 0.394290 | 0.001763 | 0.001818 | 0.001762 |
| integral_c.test | RUN | 0.152580 | 0.413778 | 0.508023 | 0.001558 | 0.001722 | 0.001423 |
| integral_c_unsigned.test | RUN | 0.154665 | 0.406601 | 0.501012 | 0.001606 | 0.001947 | 0.001540 |
| is_placeholder.test | COMPILE | 0.131681 | 0.295911 | 0.373986 | — | — | — |
| is_sequence.test | COMPILE | 0.154523 | 0.359562 | 0.470884 | — | — | — |
| iterator_tags.test | COMPILE | 0.130419 | 0.300083 | 0.395412 | — | — | — |
| joint_view.test | COMPILE | 0.170999 | 0.413598 | 0.526559 | — | — | — |
| lambda.test | COMPILE | 0.160623 | 0.370421 | 0.491823 | — | — | — |
| lambda_args.test | COMPILE | 0.151093 | 0.360570 | 0.477017 | — | — | — |
| list.test | COMPILE | 0.159232 | 0.377999 | 0.498731 | — | — | — |
| list_c.test | COMPILE | 0.155206 | 0.365580 | 0.489861 | — | — | — |
| logical.test | COMPILE | 0.129157 | 0.288278 | 0.384288 | — | — | — |
| lower_bound.test | COMPILE | 0.169490 | 0.387318 | 0.498621 | — | — | — |
| map.test | COMPILE | 0.190452 | 0.477810 | 0.597488 | — | — | — |
| max_element.test | COMPILE | 0.153554 | 0.364283 | 0.473388 | — | — | — |
| min_max.test | COMPILE | 0.131929 | 0.304353 | 0.415618 | — | — | — |
| multiset.test | COMPILE | 0.168872 | 0.405326 | 0.518130 | — | — | — |
| next.test | COMPILE | 0.125061 | 0.288968 | 0.387169 | — | — | — |
| no_has_xxx.test | COMPILE | 0.133758 | 0.310828 | 0.409343 | — | — | — |
| numeric_ops.test | COMPILE | 0.148446 | 0.356126 | 0.462697 | — | — | — |
| pair_view.test | COMPILE | 0.172455 | 0.411624 | 0.516895 | — | — | — |
| partition.test | COMPILE | 0.187626 | 0.446611 | 0.581843 | — | — | — |
| pop_front.test | COMPILE | 0.153774 | 0.370512 | 0.463602 | — | — | — |
| push_back.test | COMPILE | 0.130970 | 0.310643 | 0.411801 | — | — | — |
| push_front.test | COMPILE | 0.155968 | 0.369826 | 0.472643 | — | — | — |
| quote.test | COMPILE | 0.128240 | 0.296367 | 0.384982 | — | — | — |
| range_c.test | COMPILE | 0.161292 | 0.381054 | 0.489650 | — | — | — |
| remove.test | COMPILE | 0.170573 | 0.407302 | 0.519764 | — | — | — |
| remove_if.test | COMPILE | 0.176664 | 0.423584 | 0.533977 | — | — | — |
| replace.test | COMPILE | 0.187057 | 0.462571 | 0.577767 | — | — | — |
| replace_if.test | COMPILE | 0.190614 | 0.454180 | 0.580539 | — | — | — |
| reverse.test | COMPILE | 0.180583 | 0.431886 | 0.555184 | — | — | — |
| same_as.test | COMPILE | 0.140650 | 0.316845 | 0.401979 | — | — | — |
| set.test | COMPILE | 0.239944 | 0.623712 | 0.834322 | — | — | — |
| set_c.test | COMPILE | 0.175745 | 0.424286 | 0.542790 | — | — | — |
| single_view.test | COMPILE | 0.158834 | 0.408624 | 0.491838 | — | — | — |
| size.test | COMPILE | 0.152948 | 0.382630 | 0.468211 | — | — | — |
| size_t.test | RUN | 0.135348 | 0.330271 | 0.404566 | 0.001504 | 0.001873 | 0.001822 |
| sizeof.test | COMPILE | 0.124220 | 0.280587 | 0.365424 | — | — | — |
| sort.test | COMPILE | 0.199887 | 0.475360 | 0.596506 | — | — | — |
| stable_partition.test | COMPILE | 0.187109 | 0.441126 | 0.564441 | — | — | — |
| transform.test | COMPILE | 0.198572 | 0.485623 | 0.620191 | — | — | — |
| transform_view.test | COMPILE | 0.161903 | 0.391831 | 0.485003 | — | — | — |
| unique.test | COMPILE | 0.171084 | 0.399328 | 0.512120 | — | — | — |
| unpack_args.test | COMPILE | 0.158128 | 0.371483 | 0.471428 | — | — | — |
| upper_bound.test | COMPILE | 0.165181 | 0.385331 | 0.472152 | — | — | — |
| vector.test | COMPILE | 0.186386 | 0.463094 | 0.593627 | — | — | — |
| vector_c.test | COMPILE | 0.163826 | 0.361608 | 0.505590 | — | — | — |
| zip_view.test | COMPILE | 0.231942 | 0.573593 | 0.733851 | — | — | — |
| string.test | RUN | 0.361780 | 1.097936 | 1.065709 | 0.001322 | 0.001376 | 0.001309 |
| issue_69.test | RUN | 0.016064 | 0.049712 | 0.044651 | 0.000539 | 0.000621 | 0.001491 |
| issue_84.test | RUN | 0.038578 | 0.084305 | 0.083150 | 0.000591 | 0.000535 | 0.001458 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.000× | 0.996×, 1.004× | 0.980× |
| clang | 1.061×† | 1.032×, 1.090× | 1.006× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267198).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| bool.test | 10 | 0.001004s | 0.001035s | 0.001008s | byte equality observed |
| char.test | 10 | 0.001002s | 0.001000s | 0.000985s | byte equality observed |
| char_unsigned.test | 10 | 0.001028s | 0.000986s | 0.000981s | byte equality observed |
| for_each.test | 10 | 0.001037s | 0.001034s | 0.001022s | byte equality observed |
| int.test | 10 | 0.001007s | 0.001002s | 0.000983s | byte equality observed |
| integral_c.test | 10 | 0.001007s | 0.001008s | 0.001023s | byte equality observed |
| integral_c_unsigned.test | 10 | 0.001034s | 0.001032s | 0.001023s | byte equality observed |
| issue_69.test | 10 | 0.000471s | 0.000471s | 0.000985s | byte equality observed |
| issue_84.test | 10 | 0.000462s | 0.000458s | 0.000968s | byte equality observed |
| size_t.test | 10 | 0.001036s | 0.001028s | 0.001017s | byte equality observed |
| string.test | 10 | 0.001024s | 0.000996s | 0.001005s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267198).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
