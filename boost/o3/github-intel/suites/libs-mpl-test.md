# libs/mpl/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| increased_arity.test | COMPILE | 0.352930 | 1.043020 | 0.972674 | — | — | — |
| largest_int.test | COMPILE | 0.121024 | 0.286623 | 0.335901 | — | — | — |
| msvc_is_class.test | COMPILE | 0.015432 | 0.038706 | 0.035866 | — | — | — |
| template_arity.test | COMPILE | 0.121064 | 0.281774 | 0.319319 | — | — | — |
| is_seq.test | COMPILE | 0.003976 | 0.011995 | 0.023764 | — | — | — |
| token_equal.test | COMPILE | 0.004392 | 0.012418 | 0.023200 | — | — | — |
| advance.test | COMPILE | 0.129533 | 0.309514 | 0.358061 | — | — | — |
| always.test | COMPILE | 0.132843 | 0.322490 | 0.379299 | — | — | — |
| apply.test | COMPILE | 0.139678 | 0.340368 | 0.400692 | — | — | — |
| apply_wrap.test | COMPILE | 0.121504 | 0.287652 | 0.343977 | — | — | — |
| arithmetic.test | COMPILE | 0.129038 | 0.329308 | 0.387313 | — | — | — |
| as_sequence.test | COMPILE | 0.137168 | 0.351357 | 0.410395 | — | — | — |
| assert.test | COMPILE | 0.031346 | 0.068259 | 0.074096 | — | — | — |
| at.test | COMPILE | 0.140248 | 0.348131 | 0.417387 | — | — | — |
| back.test | COMPILE | 0.132721 | 0.333023 | 0.400348 | — | — | — |
| bind.test | COMPILE | 0.131773 | 0.317180 | 0.376198 | — | — | — |
| bitwise.test | COMPILE | 0.130876 | 0.323914 | 0.383840 | — | — | — |
| bool.test | RUN | 0.120909 | 0.292321 | 0.348798 | 0.001215 | 0.001300 | 0.001282 |
| comparison.test | COMPILE | 0.129483 | 0.322306 | 0.377423 | — | — | — |
| contains.test | COMPILE | 0.156120 | 0.397874 | 0.449419 | — | — | — |
| copy.test | COMPILE | 0.186479 | 0.474256 | 0.544116 | — | — | — |
| copy_if.test | COMPILE | 0.167598 | 0.423903 | 0.482692 | — | — | — |
| count.test | COMPILE | 0.159105 | 0.393830 | 0.461394 | — | — | — |
| count_if.test | COMPILE | 0.165060 | 0.404245 | 0.477465 | — | — | — |
| deque.test | COMPILE | 0.156562 | 0.400874 | 0.463319 | — | — | — |
| distance.test | COMPILE | 0.146174 | 0.365665 | 0.428000 | — | — | — |
| empty.test | COMPILE | 0.127685 | 0.312579 | 0.375923 | — | — | — |
| equal.test | COMPILE | 0.149112 | 0.377957 | 0.430745 | — | — | — |
| erase.test | COMPILE | 0.172630 | 0.444407 | 0.505871 | — | — | — |
| erase_range.test | COMPILE | 0.164854 | 0.401461 | 0.482932 | — | — | — |
| eval_if.test | COMPILE | 0.120304 | 0.297038 | 0.349761 | — | — | — |
| filter_view.test | COMPILE | 0.164699 | 0.418348 | 0.476058 | — | — | — |
| find.test | COMPILE | 0.162127 | 0.400746 | 0.461225 | — | — | — |
| find_if.test | COMPILE | 0.165196 | 0.414020 | 0.468950 | — | — | — |
| fold.test | COMPILE | 0.172490 | 0.438544 | 0.504056 | — | — | — |
| for_each.test | RUN | 0.251472 | 0.608593 | 0.765309 | 0.001503 | 0.001388 | 0.001253 |
| front.test | COMPILE | 0.134783 | 0.347479 | 0.394199 | — | — | — |
| get_tag_def.test | COMPILE | 0.117702 | 0.278782 | 0.335311 | — | — | — |
| has_xxx.test | COMPILE | 0.128931 | 0.310604 | 0.378330 | — | — | — |
| identity.test | COMPILE | 0.137441 | 0.323712 | 0.370304 | — | — | — |
| if.test | COMPILE | 0.117729 | 0.283290 | 0.330662 | — | — | — |
| index_of.test | COMPILE | 0.151313 | 0.372276 | 0.427765 | — | — | — |
| inherit.test | COMPILE | 0.122210 | 0.297243 | 0.344950 | — | — | — |
| insert.test | COMPILE | 0.176012 | 0.451833 | 0.522930 | — | — | — |
| insert_range.test | COMPILE | 0.241730 | 0.607058 | 0.728137 | — | — | — |
| int.test | RUN | 0.126282 | 0.327043 | 0.382797 | 0.001308 | 0.001360 | 0.001232 |
| char.test | RUN | 0.125861 | 0.322618 | 0.368033 | 0.001330 | 0.001368 | 0.001244 |
| char_unsigned.test | RUN | 0.127305 | 0.323497 | 0.368815 | 0.001274 | 0.001428 | 0.001211 |
| integral_c.test | RUN | 0.139253 | 0.416105 | 0.456265 | 0.001330 | 0.001429 | 0.001267 |
| integral_c_unsigned.test | RUN | 0.146205 | 0.416342 | 0.451126 | 0.001326 | 0.001325 | 0.001152 |
| is_placeholder.test | COMPILE | 0.111830 | 0.285237 | 0.342625 | — | — | — |
| is_sequence.test | COMPILE | 0.144006 | 0.358666 | 0.416954 | — | — | — |
| iterator_tags.test | COMPILE | 0.121341 | 0.305868 | 0.350242 | — | — | — |
| joint_view.test | COMPILE | 0.158374 | 0.405617 | 0.470585 | — | — | — |
| lambda.test | COMPILE | 0.149940 | 0.375918 | 0.436120 | — | — | — |
| lambda_args.test | COMPILE | 0.142419 | 0.351389 | 0.426784 | — | — | — |
| list.test | COMPILE | 0.150343 | 0.374141 | 0.429921 | — | — | — |
| list_c.test | COMPILE | 0.139714 | 0.370865 | 0.446027 | — | — | — |
| logical.test | COMPILE | 0.121435 | 0.296167 | 0.345176 | — | — | — |
| lower_bound.test | COMPILE | 0.152227 | 0.379683 | 0.430143 | — | — | — |
| map.test | COMPILE | 0.183858 | 0.476607 | 0.545680 | — | — | — |
| max_element.test | COMPILE | 0.148420 | 0.362076 | 0.418899 | — | — | — |
| min_max.test | COMPILE | 0.122177 | 0.312541 | 0.362437 | — | — | — |
| multiset.test | COMPILE | 0.153812 | 0.402573 | 0.458203 | — | — | — |
| next.test | COMPILE | 0.119948 | 0.292167 | 0.340460 | — | — | — |
| no_has_xxx.test | COMPILE | 0.133633 | 0.316538 | 0.375472 | — | — | — |
| numeric_ops.test | COMPILE | 0.140121 | 0.359923 | 0.413366 | — | — | — |
| pair_view.test | COMPILE | 0.160586 | 0.410906 | 0.461526 | — | — | — |
| partition.test | COMPILE | 0.177710 | 0.446976 | 0.512407 | — | — | — |
| pop_front.test | COMPILE | 0.145618 | 0.354794 | 0.418819 | — | — | — |
| push_back.test | COMPILE | 0.119662 | 0.297599 | 0.356601 | — | — | — |
| push_front.test | COMPILE | 0.144625 | 0.369220 | 0.420464 | — | — | — |
| quote.test | COMPILE | 0.120025 | 0.294914 | 0.344545 | — | — | — |
| range_c.test | COMPILE | 0.155236 | 0.379877 | 0.443245 | — | — | — |
| remove.test | COMPILE | 0.161038 | 0.411808 | 0.460556 | — | — | — |
| remove_if.test | COMPILE | 0.167549 | 0.422503 | 0.481044 | — | — | — |
| replace.test | COMPILE | 0.182771 | 0.457479 | 0.518792 | — | — | — |
| replace_if.test | COMPILE | 0.179516 | 0.468218 | 0.526604 | — | — | — |
| reverse.test | COMPILE | 0.170283 | 0.429085 | 0.490423 | — | — | — |
| same_as.test | COMPILE | 0.125267 | 0.315473 | 0.363299 | — | — | — |
| set.test | COMPILE | 0.228861 | 0.615409 | 0.741445 | — | — | — |
| set_c.test | COMPILE | 0.165183 | 0.420915 | 0.495532 | — | — | — |
| single_view.test | COMPILE | 0.150863 | 0.378295 | 0.442012 | — | — | — |
| size.test | COMPILE | 0.143099 | 0.378559 | 0.414866 | — | — | — |
| size_t.test | RUN | 0.125036 | 0.341629 | 0.391740 | 0.001258 | 0.001445 | 0.001321 |
| sizeof.test | COMPILE | 0.117197 | 0.298012 | 0.349535 | — | — | — |
| sort.test | COMPILE | 0.189622 | 0.478336 | 0.555464 | — | — | — |
| stable_partition.test | COMPILE | 0.175991 | 0.445124 | 0.522140 | — | — | — |
| transform.test | COMPILE | 0.189341 | 0.485404 | 0.563543 | — | — | — |
| transform_view.test | COMPILE | 0.154504 | 0.376606 | 0.442587 | — | — | — |
| unique.test | COMPILE | 0.162094 | 0.402653 | 0.476622 | — | — | — |
| unpack_args.test | COMPILE | 0.151408 | 0.380431 | 0.435032 | — | — | — |
| upper_bound.test | COMPILE | 0.154854 | 0.379652 | 0.441165 | — | — | — |
| vector.test | COMPILE | 0.175786 | 0.450381 | 0.528703 | — | — | — |
| vector_c.test | COMPILE | 0.153574 | 0.376962 | 0.446259 | — | — | — |
| zip_view.test | COMPILE | 0.217936 | 0.569549 | 0.651624 | — | — | — |
| string.test | RUN | 0.347874 | 1.082515 | 0.938558 | 0.001020 | 0.001013 | 0.000913 |
| issue_69.test | RUN | 0.014273 | 0.044156 | 0.043473 | 0.000520 | 0.000571 | 0.001271 |
| issue_84.test | RUN | 0.035728 | 0.082391 | 0.083250 | 0.000395 | 0.000424 | 0.000940 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.004× | 1.014×, 0.995× | 1.017× |
| clang | 1.115× | 1.109×, 1.122× | 0.976× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345325).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| bool.test | 10 | 0.000879s | 0.000881s | 0.000913s | byte equality observed |
| char.test | 10 | 0.000873s | 0.000875s | 0.000903s | byte equality observed |
| char_unsigned.test | 10 | 0.000894s | 0.000857s | 0.000880s | byte equality observed |
| for_each.test | 10 | 0.000895s | 0.000908s | 0.000905s | byte equality observed |
| int.test | 10 | 0.000867s | 0.000922s | 0.000924s | byte equality observed |
| integral_c.test | 10 | 0.000877s | 0.000891s | 0.000881s | byte equality observed |
| integral_c_unsigned.test | 10 | 0.000902s | 0.000888s | 0.000910s | byte equality observed |
| issue_69.test | 10 | 0.000392s | 0.000386s | 0.000907s | byte equality observed |
| issue_84.test | 10 | 0.000399s | 0.000387s | 0.000848s | byte equality observed |
| size_t.test | 10 | 0.000874s | 0.000874s | 0.000890s | byte equality observed |
| string.test | 10 | 0.000924s | 0.000864s | 0.000900s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345325).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
