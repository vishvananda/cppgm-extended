# libs/mpl/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| increased_arity.test | COMPILE | 0.485656 | 1.378666 | 1.509053 | — | — | — |
| largest_int.test | COMPILE | 0.163932 | 0.391055 | 0.570061 | — | — | — |
| msvc_is_class.test | COMPILE | 0.019971 | 0.047619 | 0.056567 | — | — | — |
| template_arity.test | COMPILE | 0.156771 | 0.382911 | 0.573831 | — | — | — |
| is_seq.test | COMPILE | 0.006354 | 0.017105 | 0.031757 | — | — | — |
| token_equal.test | COMPILE | 0.006310 | 0.015891 | 0.029776 | — | — | — |
| advance.test | COMPILE | 0.177037 | 0.411607 | 0.610509 | — | — | — |
| always.test | COMPILE | 0.188717 | 0.440616 | 0.656170 | — | — | — |
| apply.test | COMPILE | 0.199906 | 0.462773 | 0.656524 | — | — | — |
| apply_wrap.test | COMPILE | 0.167603 | 0.400245 | 0.581184 | — | — | — |
| arithmetic.test | COMPILE | 0.183714 | 0.445550 | 0.650470 | — | — | — |
| as_sequence.test | COMPILE | 0.199259 | 0.490764 | 0.648083 | — | — | — |
| assert.test | COMPILE | 0.046216 | 0.094723 | 0.120592 | — | — | — |
| at.test | COMPILE | 0.196716 | 0.471810 | 0.679100 | — | — | — |
| back.test | COMPILE | 0.185021 | 0.451496 | 0.654607 | — | — | — |
| bind.test | COMPILE | 0.183641 | 0.435701 | 0.644998 | — | — | — |
| bitwise.test | COMPILE | 0.184859 | 0.454909 | 0.646150 | — | — | — |
| bool.test | RUN | 0.167862 | 0.397512 | 0.581265 | 0.001844 | 0.001776 | 0.002172 |
| comparison.test | COMPILE | 0.179665 | 0.450469 | 0.618003 | — | — | — |
| contains.test | COMPILE | 0.219688 | 0.524742 | 0.741917 | — | — | — |
| copy.test | COMPILE | 0.265146 | 0.654422 | 0.911960 | — | — | — |
| copy_if.test | COMPILE | 0.229649 | 0.574087 | 0.826389 | — | — | — |
| count.test | COMPILE | 0.225665 | 0.535061 | 0.780576 | — | — | — |
| count_if.test | COMPILE | 0.228347 | 0.547018 | 0.812835 | — | — | — |
| deque.test | COMPILE | 0.218187 | 0.552995 | 0.773583 | — | — | — |
| distance.test | COMPILE | 0.207527 | 0.493087 | 0.696933 | — | — | — |
| empty.test | COMPILE | 0.182631 | 0.450550 | 0.635544 | — | — | — |
| equal.test | COMPILE | 0.213195 | 0.487868 | 0.760964 | — | — | — |
| erase.test | COMPILE | 0.244045 | 0.597732 | 0.860083 | — | — | — |
| erase_range.test | COMPILE | 0.228449 | 0.584620 | 0.823806 | — | — | — |
| eval_if.test | COMPILE | 0.167286 | 0.398884 | 0.595254 | — | — | — |
| filter_view.test | COMPILE | 0.233912 | 0.559520 | 0.791299 | — | — | — |
| find.test | COMPILE | 0.230144 | 0.533710 | 0.772201 | — | — | — |
| find_if.test | COMPILE | 0.237978 | 0.539224 | 0.775676 | — | — | — |
| fold.test | COMPILE | 0.246528 | 0.598087 | 0.862448 | — | — | — |
| for_each.test | RUN | 0.361509 | 0.852831 | 1.310173 | 0.001867 | 0.002064 | 0.001919 |
| front.test | COMPILE | 0.193386 | 0.459876 | 0.662207 | — | — | — |
| get_tag_def.test | COMPILE | 0.172672 | 0.384597 | 0.606183 | — | — | — |
| has_xxx.test | COMPILE | 0.190039 | 0.430413 | 0.644066 | — | — | — |
| identity.test | COMPILE | 0.197525 | 0.440887 | 0.639876 | — | — | — |
| if.test | COMPILE | 0.172285 | 0.385384 | 0.572450 | — | — | — |
| index_of.test | COMPILE | 0.221721 | 0.501056 | 0.734640 | — | — | — |
| inherit.test | COMPILE | 0.183279 | 0.409078 | 0.572324 | — | — | — |
| insert.test | COMPILE | 0.252482 | 0.620734 | 0.912990 | — | — | — |
| insert_range.test | COMPILE | 0.343906 | 0.842283 | 1.294815 | — | — | — |
| int.test | RUN | 0.177903 | 0.454136 | 0.643612 | 0.001888 | 0.001813 | 0.001842 |
| char.test | RUN | 0.178346 | 0.469686 | 0.641156 | 0.001964 | 0.001895 | 0.001824 |
| char_unsigned.test | RUN | 0.179682 | 0.436569 | 0.649563 | 0.001862 | 0.002158 | 0.001976 |
| integral_c.test | RUN | 0.208588 | 0.574564 | 0.747716 | 0.001874 | 0.001939 | 0.002058 |
| integral_c_unsigned.test | RUN | 0.205849 | 0.564291 | 0.785604 | 0.002015 | 0.001895 | 0.001812 |
| is_placeholder.test | COMPILE | 0.172478 | 0.414851 | 0.605074 | — | — | — |
| is_sequence.test | COMPILE | 0.213107 | 0.483598 | 0.666674 | — | — | — |
| iterator_tags.test | COMPILE | 0.173041 | 0.402939 | 0.571573 | — | — | — |
| joint_view.test | COMPILE | 0.229072 | 0.549509 | 0.795190 | — | — | — |
| lambda.test | COMPILE | 0.215748 | 0.506894 | 0.724307 | — | — | — |
| lambda_args.test | COMPILE | 0.203941 | 0.499218 | 0.749752 | — | — | — |
| list.test | COMPILE | 0.211319 | 0.513235 | 0.734916 | — | — | — |
| list_c.test | COMPILE | 0.207751 | 0.497233 | 0.747532 | — | — | — |
| logical.test | COMPILE | 0.171343 | 0.387639 | 0.584362 | — | — | — |
| lower_bound.test | COMPILE | 0.219550 | 0.523512 | 0.747100 | — | — | — |
| map.test | COMPILE | 0.256081 | 0.655514 | 0.927389 | — | — | — |
| max_element.test | COMPILE | 0.206826 | 0.483056 | 0.730012 | — | — | — |
| min_max.test | COMPILE | 0.175536 | 0.437959 | 0.602719 | — | — | — |
| multiset.test | COMPILE | 0.231297 | 0.550809 | 0.780244 | — | — | — |
| next.test | COMPILE | 0.170455 | 0.397754 | 0.599793 | — | — | — |
| no_has_xxx.test | COMPILE | 0.180062 | 0.433255 | 0.644095 | — | — | — |
| numeric_ops.test | COMPILE | 0.196315 | 0.480873 | 0.666262 | — | — | — |
| pair_view.test | COMPILE | 0.236082 | 0.550569 | 0.790952 | — | — | — |
| partition.test | COMPILE | 0.246362 | 0.601609 | 0.877283 | — | — | — |
| pop_front.test | COMPILE | 0.209754 | 0.497833 | 0.692612 | — | — | — |
| push_back.test | COMPILE | 0.170184 | 0.421028 | 0.629319 | — | — | — |
| push_front.test | COMPILE | 0.207750 | 0.502118 | 0.707264 | — | — | — |
| quote.test | COMPILE | 0.169749 | 0.406455 | 0.589679 | — | — | — |
| range_c.test | COMPILE | 0.220949 | 0.522336 | 0.738927 | — | — | — |
| remove.test | COMPILE | 0.228895 | 0.552194 | 0.781113 | — | — | — |
| remove_if.test | COMPILE | 0.234522 | 0.569573 | 0.814072 | — | — | — |
| replace.test | COMPILE | 0.249994 | 0.626897 | 0.845509 | — | — | — |
| replace_if.test | COMPILE | 0.260540 | 0.623752 | 0.884058 | — | — | — |
| reverse.test | COMPILE | 0.245178 | 0.579790 | 0.841256 | — | — | — |
| same_as.test | COMPILE | 0.189702 | 0.414142 | 0.615681 | — | — | — |
| set.test | COMPILE | 0.324572 | 0.834337 | 1.237065 | — | — | — |
| set_c.test | COMPILE | 0.239321 | 0.551896 | 0.822500 | — | — | — |
| single_view.test | COMPILE | 0.209141 | 0.510814 | 0.744972 | — | — | — |
| size.test | COMPILE | 0.206976 | 0.498265 | 0.703711 | — | — | — |
| size_t.test | RUN | 0.176565 | 0.450967 | 0.615670 | 0.001865 | 0.001944 | 0.002046 |
| sizeof.test | COMPILE | 0.163641 | 0.395993 | 0.551157 | — | — | — |
| sort.test | COMPILE | 0.267793 | 0.638621 | 0.891230 | — | — | — |
| stable_partition.test | COMPILE | 0.257019 | 0.605452 | 0.856340 | — | — | — |
| transform.test | COMPILE | 0.262796 | 0.653315 | 0.950602 | — | — | — |
| transform_view.test | COMPILE | 0.214983 | 0.532615 | 0.752834 | — | — | — |
| unique.test | COMPILE | 0.228842 | 0.542405 | 0.786244 | — | — | — |
| unpack_args.test | COMPILE | 0.213185 | 0.503833 | 0.733643 | — | — | — |
| upper_bound.test | COMPILE | 0.220708 | 0.515631 | 0.733746 | — | — | — |
| vector.test | COMPILE | 0.245456 | 0.632906 | 0.867176 | — | — | — |
| vector_c.test | COMPILE | 0.221967 | 0.501304 | 0.790327 | — | — | — |
| zip_view.test | COMPILE | 0.311586 | 0.758453 | 1.019935 | — | — | — |
| string.test | RUN | 0.493524 | 1.381560 | 1.417443 | 0.001625 | 0.001366 | 0.001417 |
| issue_69.test | RUN | 0.022759 | 0.058120 | 0.068274 | 0.000658 | 0.000794 | 0.001910 |
| issue_84.test | RUN | 0.050216 | 0.121458 | 0.128028 | 0.000924 | 0.000694 | 0.001402 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.001× | 0.994×, 1.007× | 0.988× |
| clang | 1.099× | 1.091×, 1.107× | 0.996× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824563).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| bool.test | 10 | 0.001282s | 0.001233s | 0.001216s | byte equality observed |
| char.test | 10 | 0.001253s | 0.001283s | 0.001215s | byte equality observed |
| char_unsigned.test | 10 | 0.001226s | 0.001233s | 0.001216s | byte equality observed |
| for_each.test | 10 | 0.001280s | 0.001300s | 0.001303s | byte equality observed |
| int.test | 10 | 0.001255s | 0.001286s | 0.001275s | byte equality observed |
| integral_c.test | 10 | 0.001261s | 0.001210s | 0.001298s | byte equality observed |
| integral_c_unsigned.test | 10 | 0.001297s | 0.001293s | 0.001212s | byte equality observed |
| issue_69.test | 10 | 0.000578s | 0.000584s | 0.001290s | byte equality observed |
| issue_84.test | 10 | 0.000566s | 0.000592s | 0.001211s | byte equality observed |
| size_t.test | 10 | 0.001241s | 0.001260s | 0.001186s | byte equality observed |
| string.test | 10 | 0.001248s | 0.001287s | 0.001220s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824563).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
