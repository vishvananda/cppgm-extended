# libs/mpl/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| increased_arity.test | COMPILE | 0.599495 | 1.372336 | 1.597411 | — | — | — |
| largest_int.test | COMPILE | 0.195008 | 0.390997 | 0.555452 | — | — | — |
| msvc_is_class.test | COMPILE | 0.023594 | 0.048369 | 0.057810 | — | — | — |
| template_arity.test | COMPILE | 0.197427 | 0.383705 | 0.549767 | — | — | — |
| is_seq.test | COMPILE | 0.006772 | 0.017929 | 0.034211 | — | — | — |
| token_equal.test | COMPILE | 0.007477 | 0.016260 | 0.032254 | — | — | — |
| advance.test | COMPILE | 0.210638 | 0.413655 | 0.659859 | — | — | — |
| always.test | COMPILE | 0.219244 | 0.444190 | 0.651228 | — | — | — |
| apply.test | COMPILE | 0.228681 | 0.465606 | 0.712388 | — | — | — |
| apply_wrap.test | COMPILE | 0.202436 | 0.383598 | 0.606933 | — | — | — |
| arithmetic.test | COMPILE | 0.218014 | 0.441705 | 0.626502 | — | — | — |
| as_sequence.test | COMPILE | 0.236646 | 0.489939 | 0.713128 | — | — | — |
| assert.test | COMPILE | 0.060939 | 0.095305 | 0.125701 | — | — | — |
| at.test | COMPILE | 0.229425 | 0.477827 | 0.705495 | — | — | — |
| back.test | COMPILE | 0.222924 | 0.460344 | 0.665061 | — | — | — |
| bind.test | COMPILE | 0.217347 | 0.445520 | 0.669836 | — | — | — |
| bitwise.test | COMPILE | 0.224368 | 0.451368 | 0.693258 | — | — | — |
| bool.test | RUN | 0.197527 | 0.391037 | 0.609518 | 0.001511 | 0.002047 | 0.002030 |
| comparison.test | COMPILE | 0.220523 | 0.440861 | 0.651320 | — | — | — |
| contains.test | COMPILE | 0.258662 | 0.549800 | 0.801860 | — | — | — |
| copy.test | COMPILE | 0.302617 | 0.662409 | 0.940477 | — | — | — |
| copy_if.test | COMPILE | 0.272372 | 0.579941 | 0.839341 | — | — | — |
| count.test | COMPILE | 0.256614 | 0.528328 | 0.800822 | — | — | — |
| count_if.test | COMPILE | 0.277906 | 0.573379 | 0.848075 | — | — | — |
| deque.test | COMPILE | 0.255914 | 0.554295 | 0.795584 | — | — | — |
| distance.test | COMPILE | 0.243027 | 0.485830 | 0.724702 | — | — | — |
| empty.test | COMPILE | 0.214471 | 0.441010 | 0.664491 | — | — | — |
| equal.test | COMPILE | 0.247999 | 0.522950 | 0.749923 | — | — | — |
| erase.test | COMPILE | 0.280906 | 0.600238 | 0.902158 | — | — | — |
| erase_range.test | COMPILE | 0.267398 | 0.573168 | 0.823882 | — | — | — |
| eval_if.test | COMPILE | 0.197534 | 0.397313 | 0.647485 | — | — | — |
| filter_view.test | COMPILE | 0.270196 | 0.572763 | 0.818241 | — | — | — |
| find.test | COMPILE | 0.264862 | 0.543301 | 0.817931 | — | — | — |
| find_if.test | COMPILE | 0.265718 | 0.560953 | 0.839353 | — | — | — |
| fold.test | COMPILE | 0.280678 | 0.590153 | 0.868468 | — | — | — |
| for_each.test | RUN | 0.391799 | 0.865703 | 1.380396 | 0.002174 | 0.002056 | 0.002104 |
| front.test | COMPILE | 0.221957 | 0.469307 | 0.676795 | — | — | — |
| get_tag_def.test | COMPILE | 0.195513 | 0.421584 | 0.625240 | — | — | — |
| has_xxx.test | COMPILE | 0.221125 | 0.429751 | 0.698554 | — | — | — |
| identity.test | COMPILE | 0.224038 | 0.465683 | 0.689092 | — | — | — |
| if.test | COMPILE | 0.200595 | 0.387293 | 0.610604 | — | — | — |
| index_of.test | COMPILE | 0.254454 | 0.518736 | 0.762182 | — | — | — |
| inherit.test | COMPILE | 0.207450 | 0.412451 | 0.639735 | — | — | — |
| insert.test | COMPILE | 0.294121 | 0.621087 | 0.904469 | — | — | — |
| insert_range.test | COMPILE | 0.386655 | 0.842526 | 1.331210 | — | — | — |
| int.test | RUN | 0.209940 | 0.458182 | 0.665127 | 0.001787 | 0.001940 | 0.001996 |
| char.test | RUN | 0.215806 | 0.468568 | 0.674795 | 0.001802 | 0.001830 | 0.001894 |
| char_unsigned.test | RUN | 0.215028 | 0.440231 | 0.672687 | 0.001872 | 0.002298 | 0.002140 |
| integral_c.test | RUN | 0.224799 | 0.570317 | 0.793853 | 0.001896 | 0.002121 | 0.002445 |
| integral_c_unsigned.test | RUN | 0.224893 | 0.577197 | 0.777052 | 0.001933 | 0.002090 | 0.002004 |
| is_placeholder.test | COMPILE | 0.206595 | 0.406992 | 0.639758 | — | — | — |
| is_sequence.test | COMPILE | 0.247053 | 0.485783 | 0.699534 | — | — | — |
| iterator_tags.test | COMPILE | 0.208477 | 0.413893 | 0.573148 | — | — | — |
| joint_view.test | COMPILE | 0.273320 | 0.557142 | 0.810034 | — | — | — |
| lambda.test | COMPILE | 0.258112 | 0.516818 | 0.776476 | — | — | — |
| lambda_args.test | COMPILE | 0.240937 | 0.504006 | 0.796561 | — | — | — |
| list.test | COMPILE | 0.248747 | 0.534006 | 0.740728 | — | — | — |
| list_c.test | COMPILE | 0.246958 | 0.506955 | 0.752680 | — | — | — |
| logical.test | COMPILE | 0.204524 | 0.406592 | 0.613897 | — | — | — |
| lower_bound.test | COMPILE | 0.260793 | 0.509484 | 0.764445 | — | — | — |
| map.test | COMPILE | 0.296727 | 0.645851 | 0.966266 | — | — | — |
| max_element.test | COMPILE | 0.244720 | 0.490328 | 0.725788 | — | — | — |
| min_max.test | COMPILE | 0.208971 | 0.421912 | 0.639590 | — | — | — |
| multiset.test | COMPILE | 0.263583 | 0.564332 | 0.804225 | — | — | — |
| next.test | COMPILE | 0.198167 | 0.394451 | 0.616947 | — | — | — |
| no_has_xxx.test | COMPILE | 0.216210 | 0.440792 | 0.657154 | — | — | — |
| numeric_ops.test | COMPILE | 0.234954 | 0.511795 | 0.715763 | — | — | — |
| pair_view.test | COMPILE | 0.271184 | 0.551044 | 0.829603 | — | — | — |
| partition.test | COMPILE | 0.289643 | 0.619617 | 0.913384 | — | — | — |
| pop_front.test | COMPILE | 0.244037 | 0.490306 | 0.744815 | — | — | — |
| push_back.test | COMPILE | 0.210859 | 0.410774 | 0.650905 | — | — | — |
| push_front.test | COMPILE | 0.242868 | 0.510414 | 0.762708 | — | — | — |
| quote.test | COMPILE | 0.205704 | 0.411181 | 0.618855 | — | — | — |
| range_c.test | COMPILE | 0.254440 | 0.523272 | 0.767556 | — | — | — |
| remove.test | COMPILE | 0.269383 | 0.549580 | 0.816816 | — | — | — |
| remove_if.test | COMPILE | 0.270462 | 0.576747 | 0.846057 | — | — | — |
| replace.test | COMPILE | 0.296891 | 0.635950 | 0.899757 | — | — | — |
| replace_if.test | COMPILE | 0.291693 | 0.631357 | 0.903534 | — | — | — |
| reverse.test | COMPILE | 0.276496 | 0.584025 | 0.865370 | — | — | — |
| same_as.test | COMPILE | 0.219392 | 0.423375 | 0.655900 | — | — | — |
| set.test | COMPILE | 0.372924 | 0.850440 | 1.315951 | — | — | — |
| set_c.test | COMPILE | 0.270478 | 0.553558 | 0.837867 | — | — | — |
| single_view.test | COMPILE | 0.244981 | 0.512290 | 0.777481 | — | — | — |
| size.test | COMPILE | 0.247683 | 0.504749 | 0.737809 | — | — | — |
| size_t.test | RUN | 0.208388 | 0.463443 | 0.686387 | 0.001910 | 0.002131 | 0.002188 |
| sizeof.test | COMPILE | 0.195604 | 0.402601 | 0.614457 | — | — | — |
| sort.test | COMPILE | 0.305417 | 0.657374 | 0.971371 | — | — | — |
| stable_partition.test | COMPILE | 0.289483 | 0.625761 | 0.915813 | — | — | — |
| transform.test | COMPILE | 0.301918 | 0.655601 | 0.965064 | — | — | — |
| transform_view.test | COMPILE | 0.252502 | 0.543680 | 0.780683 | — | — | — |
| unique.test | COMPILE | 0.265756 | 0.553626 | 0.823703 | — | — | — |
| unpack_args.test | COMPILE | 0.246175 | 0.516776 | 0.770569 | — | — | — |
| upper_bound.test | COMPILE | 0.253992 | 0.510021 | 0.737652 | — | — | — |
| vector.test | COMPILE | 0.294444 | 0.635539 | 0.903362 | — | — | — |
| vector_c.test | COMPILE | 0.252413 | 0.507868 | 0.812265 | — | — | — |
| zip_view.test | COMPILE | 0.345145 | 0.781802 | 1.014235 | — | — | — |
| string.test | RUN | 0.519332 | 1.411984 | 1.519247 | 0.001591 | 0.001600 | 0.001442 |
| issue_69.test | RUN | 0.021069 | 0.058297 | 0.070641 | 0.000795 | 0.000747 | 0.002064 |
| issue_84.test | RUN | 0.063763 | 0.125932 | 0.126001 | 0.000586 | 0.000628 | 0.001531 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.008× | 1.001×, 1.016× | 0.986× |
| clang | 1.096× | 1.092×, 1.100× | 1.002× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| bool.test | 10 | 0.001281s | 0.001283s | 0.001261s | byte equality observed |
| char.test | 10 | 0.001274s | 0.001294s | 0.001240s | byte equality observed |
| char_unsigned.test | 10 | 0.001280s | 0.001297s | 0.001304s | byte equality observed |
| for_each.test | 10 | 0.001286s | 0.001351s | 0.001276s | byte equality observed |
| int.test | 10 | 0.001273s | 0.001270s | 0.001246s | byte equality observed |
| integral_c.test | 10 | 0.001256s | 0.001251s | 0.001242s | byte equality observed |
| integral_c_unsigned.test | 10 | 0.001292s | 0.001287s | 0.001260s | byte equality observed |
| issue_69.test | 10 | 0.000595s | 0.000571s | 0.001294s | byte equality observed |
| issue_84.test | 10 | 0.000610s | 0.000607s | 0.001264s | byte equality observed |
| size_t.test | 10 | 0.001280s | 0.001272s | 0.001286s | byte equality observed |
| string.test | 10 | 0.001250s | 0.001302s | 0.001289s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
