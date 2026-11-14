# libs/mpl/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| increased_arity.test | COMPILE | 0.282368 | 0.764923 | 0.752243 | — | — | — |
| largest_int.test | COMPILE | 0.108644 | 0.216021 | 0.253522 | — | — | — |
| msvc_is_class.test | COMPILE | 0.013618 | 0.030243 | 0.032152 | — | — | — |
| template_arity.test | COMPILE | 0.108496 | 0.212820 | 0.250957 | — | — | — |
| is_seq.test | COMPILE | 0.004743 | 0.011675 | 0.023859 | — | — | — |
| token_equal.test | COMPILE | 0.005017 | 0.011512 | 0.024698 | — | — | — |
| advance.test | COMPILE | 0.111010 | 0.245171 | 0.290564 | — | — | — |
| always.test | COMPILE | 0.122613 | 0.238017 | 0.301995 | — | — | — |
| apply.test | COMPILE | 0.124000 | 0.258062 | 0.318923 | — | — | — |
| apply_wrap.test | COMPILE | 0.107645 | 0.223193 | 0.279681 | — | — | — |
| arithmetic.test | COMPILE | 0.116169 | 0.248780 | 0.300475 | — | — | — |
| as_sequence.test | COMPILE | 0.123884 | 0.272463 | 0.324256 | — | — | — |
| assert.test | COMPILE | 0.029291 | 0.055968 | 0.066791 | — | — | — |
| at.test | COMPILE | 0.125443 | 0.262436 | 0.325998 | — | — | — |
| back.test | COMPILE | 0.117900 | 0.252569 | 0.315319 | — | — | — |
| bind.test | COMPILE | 0.116227 | 0.242437 | 0.302407 | — | — | — |
| bitwise.test | COMPILE | 0.115036 | 0.247589 | 0.314340 | — | — | — |
| bool.test | RUN | 0.105059 | 0.218218 | 0.289481 | 0.001517 | 0.001442 | 0.001397 |
| comparison.test | COMPILE | 0.111550 | 0.242385 | 0.305103 | — | — | — |
| contains.test | COMPILE | 0.135452 | 0.299393 | 0.356941 | — | — | — |
| copy.test | COMPILE | 0.164564 | 0.357480 | 0.440987 | — | — | — |
| copy_if.test | COMPILE | 0.147704 | 0.320182 | 0.389788 | — | — | — |
| count.test | COMPILE | 0.144057 | 0.295211 | 0.366601 | — | — | — |
| count_if.test | COMPILE | 0.149075 | 0.304996 | 0.386055 | — | — | — |
| deque.test | COMPILE | 0.139727 | 0.303433 | 0.369197 | — | — | — |
| distance.test | COMPILE | 0.126675 | 0.272202 | 0.388971 | — | — | — |
| empty.test | COMPILE | 0.114144 | 0.237079 | 0.351283 | — | — | — |
| equal.test | COMPILE | 0.135735 | 0.284150 | 0.385073 | — | — | — |
| erase.test | COMPILE | 0.151692 | 0.333893 | 0.407033 | — | — | — |
| erase_range.test | COMPILE | 0.147741 | 0.308714 | 0.380068 | — | — | — |
| eval_if.test | COMPILE | 0.106606 | 0.227005 | 0.277728 | — | — | — |
| filter_view.test | COMPILE | 0.144280 | 0.314833 | 0.374505 | — | — | — |
| find.test | COMPILE | 0.140221 | 0.299650 | 0.366540 | — | — | — |
| find_if.test | COMPILE | 0.140192 | 0.301960 | 0.378880 | — | — | — |
| fold.test | COMPILE | 0.150975 | 0.316172 | 0.399150 | — | — | — |
| for_each.test | RUN | 0.220315 | 0.497681 | 0.600218 | 0.001726 | 0.001436 | 0.001513 |
| front.test | COMPILE | 0.117772 | 0.257843 | 0.314530 | — | — | — |
| get_tag_def.test | COMPILE | 0.112248 | 0.263898 | 0.271854 | — | — | — |
| has_xxx.test | COMPILE | 0.113326 | 0.269692 | 0.304865 | — | — | — |
| identity.test | COMPILE | 0.116462 | 0.295021 | 0.292012 | — | — | — |
| if.test | COMPILE | 0.102646 | 0.270724 | 0.272422 | — | — | — |
| index_of.test | COMPILE | 0.130053 | 0.333791 | 0.343550 | — | — | — |
| inherit.test | COMPILE | 0.109894 | 0.278771 | 0.279395 | — | — | — |
| insert.test | COMPILE | 0.152267 | 0.336704 | 0.414003 | — | — | — |
| insert_range.test | COMPILE | 0.210986 | 0.439628 | 0.598805 | — | — | — |
| int.test | RUN | 0.108288 | 0.246684 | 0.295282 | 0.001725 | 0.001351 | 0.001587 |
| char.test | RUN | 0.110897 | 0.248403 | 0.305716 | 0.001470 | 0.001410 | 0.001397 |
| char_unsigned.test | RUN | 0.111192 | 0.248717 | 0.293425 | 0.001287 | 0.001617 | 0.001737 |
| integral_c.test | RUN | 0.123245 | 0.300189 | 0.368592 | 0.001628 | 0.001585 | 0.001796 |
| integral_c_unsigned.test | RUN | 0.125522 | 0.300747 | 0.381454 | 0.001587 | 0.001443 | 0.001538 |
| is_placeholder.test | COMPILE | 0.107173 | 0.216786 | 0.289737 | — | — | — |
| is_sequence.test | COMPILE | 0.131169 | 0.270027 | 0.324777 | — | — | — |
| iterator_tags.test | COMPILE | 0.106579 | 0.223565 | 0.281058 | — | — | — |
| joint_view.test | COMPILE | 0.140653 | 0.301241 | 0.376579 | — | — | — |
| lambda.test | COMPILE | 0.132665 | 0.276132 | 0.348564 | — | — | — |
| lambda_args.test | COMPILE | 0.124229 | 0.267533 | 0.346672 | — | — | — |
| list.test | COMPILE | 0.129312 | 0.284890 | 0.342816 | — | — | — |
| list_c.test | COMPILE | 0.129142 | 0.271181 | 0.345924 | — | — | — |
| logical.test | COMPILE | 0.107703 | 0.217102 | 0.280275 | — | — | — |
| lower_bound.test | COMPILE | 0.139762 | 0.291602 | 0.354412 | — | — | — |
| map.test | COMPILE | 0.163595 | 0.360297 | 0.435507 | — | — | — |
| max_element.test | COMPILE | 0.130568 | 0.269909 | 0.342821 | — | — | — |
| min_max.test | COMPILE | 0.111946 | 0.233954 | 0.298307 | — | — | — |
| multiset.test | COMPILE | 0.134248 | 0.311716 | 0.367680 | — | — | — |
| next.test | COMPILE | 0.105213 | 0.230196 | 0.283482 | — | — | — |
| no_has_xxx.test | COMPILE | 0.113533 | 0.236557 | 0.295961 | — | — | — |
| numeric_ops.test | COMPILE | 0.124389 | 0.276023 | 0.338408 | — | — | — |
| pair_view.test | COMPILE | 0.142029 | 0.274898 | 0.378715 | — | — | — |
| partition.test | COMPILE | 0.154618 | 0.332329 | 0.421641 | — | — | — |
| pop_front.test | COMPILE | 0.125878 | 0.271276 | 0.335722 | — | — | — |
| push_back.test | COMPILE | 0.112877 | 0.226612 | 0.294102 | — | — | — |
| push_front.test | COMPILE | 0.130685 | 0.273911 | 0.341378 | — | — | — |
| quote.test | COMPILE | 0.105657 | 0.220653 | 0.293933 | — | — | — |
| range_c.test | COMPILE | 0.129818 | 0.289740 | 0.365566 | — | — | — |
| remove.test | COMPILE | 0.138065 | 0.298358 | 0.377039 | — | — | — |
| remove_if.test | COMPILE | 0.148843 | 0.316483 | 0.399963 | — | — | — |
| replace.test | COMPILE | 0.153356 | 0.340967 | 0.414623 | — | — | — |
| replace_if.test | COMPILE | 0.153487 | 0.349532 | 0.422511 | — | — | — |
| reverse.test | COMPILE | 0.147732 | 0.318287 | 0.394367 | — | — | — |
| same_as.test | COMPILE | 0.113950 | 0.242917 | 0.297684 | — | — | — |
| set.test | COMPILE | 0.195259 | 0.439750 | 0.577584 | — | — | — |
| set_c.test | COMPILE | 0.146564 | 0.314249 | 0.393213 | — | — | — |
| single_view.test | COMPILE | 0.134778 | 0.282417 | 0.337077 | — | — | — |
| size.test | COMPILE | 0.125321 | 0.273831 | 0.323828 | — | — | — |
| size_t.test | RUN | 0.108802 | 0.246643 | 0.303908 | 0.001772 | 0.001698 | 0.001784 |
| sizeof.test | COMPILE | 0.104171 | 0.220617 | 0.276454 | — | — | — |
| sort.test | COMPILE | 0.164340 | 0.360066 | 0.444396 | — | — | — |
| stable_partition.test | COMPILE | 0.155642 | 0.336228 | 0.415257 | — | — | — |
| transform.test | COMPILE | 0.160153 | 0.364066 | 0.454905 | — | — | — |
| transform_view.test | COMPILE | 0.133689 | 0.289463 | 0.360574 | — | — | — |
| unique.test | COMPILE | 0.139003 | 0.312850 | 0.372163 | — | — | — |
| unpack_args.test | COMPILE | 0.129152 | 0.289088 | 0.345068 | — | — | — |
| upper_bound.test | COMPILE | 0.131692 | 0.283347 | 0.354932 | — | — | — |
| vector.test | COMPILE | 0.150641 | 0.343920 | 0.420145 | — | — | — |
| vector_c.test | COMPILE | 0.129123 | 0.292947 | 0.361080 | — | — | — |
| zip_view.test | COMPILE | 0.189050 | 0.416886 | 0.516810 | — | — | — |
| string.test | RUN | 0.279952 | 0.855719 | 0.744463 | 0.001347 | 0.001114 | 0.001168 |
| issue_69.test | RUN | 0.014495 | 0.036907 | 0.040960 | 0.000521 | 0.000480 | 0.001380 |
| issue_84.test | RUN | 0.028002 | 0.066448 | 0.073525 | 0.000532 | 0.000466 | 0.001193 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.007×† | 0.986×, 1.028× | 1.069× |
| clang | 1.108×† | 1.132×, 1.084× | 1.113× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051632).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| bool.test | 10 | 0.000904s | 0.000893s | 0.000925s | byte equality observed |
| char.test | 10 | 0.000890s | 0.000892s | 0.000906s | byte equality observed |
| char_unsigned.test | 10 | 0.000896s | 0.000897s | 0.000910s | byte equality observed |
| for_each.test | 10 | 0.000906s | 0.000899s | 0.000923s | byte equality observed |
| int.test | 10 | 0.000883s | 0.000893s | 0.000900s | byte equality observed |
| integral_c.test | 10 | 0.000876s | 0.000902s | 0.000916s | byte equality observed |
| integral_c_unsigned.test | 10 | 0.000906s | 0.000891s | 0.000889s | byte equality observed |
| issue_69.test | 10 | 0.000404s | 0.000402s | 0.000880s | byte equality observed |
| issue_84.test | 10 | 0.000408s | 0.000405s | 0.000883s | byte equality observed |
| size_t.test | 10 | 0.000926s | 0.000906s | 0.000907s | byte equality observed |
| string.test | 10 | 0.000906s | 0.000905s | 0.000917s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051632).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
