# libs/variant2/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| quick.test | RUN | 0.114020 | 0.250979 | 0.381213 | 0.001860 | 0.000848 | 0.001767 |
| variant_size.test | RUN | 0.208591 | 0.632517 | 0.727391 | 0.003985 | 0.002025 | 0.001771 |
| variant_alternative.test | RUN | 0.307666 | 1.993813 | 1.166991 | 0.001631 | 0.001934 | 0.001319 |
| variant_holds_alternative.test | RUN | 0.208183 | 0.478575 | 0.701843 | 0.001659 | 0.001904 | 0.001579 |
| variant_holds_alternative_cx.test | COMPILE | 0.123200 | 0.297914 | 0.405939 | — | — | — |
| variant_get_by_index.test | RUN | 0.304840 | 2.138681 | 1.442971 | 0.001702 | 0.001957 | 0.001842 |
| variant_get_by_index_cx.test | COMPILE | 0.125452 | 0.294732 | 0.418192 | — | — | — |
| variant_get_by_type.test | RUN | 0.299603 | 2.305836 | 1.535210 | 0.001805 | 0.001978 | 0.001670 |
| variant_get_by_type_cx.test | COMPILE | 0.111659 | 0.250525 | 0.363465 | — | — | — |
| variant_default_construct.test | RUN | 0.298437 | 1.723585 | 1.230253 | 0.001757 | 0.001778 | 0.001671 |
| variant_default_construct_cx.test | COMPILE | 0.119897 | 0.291440 | 0.412136 | — | — | — |
| variant_copy_construct.test | RUN | 0.443573 | 2.773253 | 2.176612 | 0.001644 | 0.001934 | 0.001798 |
| variant_copy_construct_cx.test | COMPILE | 0.139612 | 0.319757 | 0.485823 | — | — | — |
| variant_move_construct.test | RUN | 0.418782 | 2.488372 | 2.016433 | 0.001568 | 0.001660 | 0.001689 |
| variant_move_construct_cx.test | COMPILE | 0.111460 | 0.261944 | 0.358085 | — | — | — |
| variant_value_construct.test | RUN | 0.228427 | 0.611949 | 0.881991 | 0.001662 | 0.001829 | 0.001798 |
| variant_value_construct_cx.test | COMPILE | 0.122016 | 0.287801 | 0.403477 | — | — | — |
| variant_in_place_index_construct.test | RUN | 0.235460 | 0.625321 | 0.928206 | 0.001732 | 0.001771 | 0.001790 |
| variant_in_place_index_construct_cx.test | COMPILE | 0.127841 | 0.300225 | 0.451287 | — | — | — |
| variant_in_place_type_construct.test | RUN | 0.231327 | 0.621030 | 0.888616 | 0.001618 | 0.001606 | 0.001738 |
| variant_in_place_type_construct_cx.test | COMPILE | 0.128318 | 0.295779 | 0.437557 | — | — | — |
| variant_copy_assign.test | RUN | 0.308620 | 1.993022 | 1.349391 | 0.001591 | 0.002122 | 0.001774 |
| variant_copy_assign_cx.test | COMPILE | 0.111347 | 0.237825 | 0.361718 | — | — | — |
| variant_move_assign.test | RUN | 0.296094 | 1.997967 | 1.338892 | 0.001766 | 0.001881 | 0.001706 |
| variant_move_assign_cx.test | COMPILE | 0.110538 | 0.244657 | 0.356683 | — | — | — |
| variant_value_assign.test | RUN | 0.228490 | 0.723600 | 0.942252 | 0.001424 | 0.001891 | 0.001687 |
| variant_value_assign_cx.test | COMPILE | 0.111244 | 0.291648 | 0.352180 | — | — | — |
| variant_emplace_index.test | RUN | 0.256394 | 0.867287 | 1.039328 | 0.001742 | 0.001954 | 0.001804 |
| variant_emplace_index_cx.test | COMPILE | 0.110057 | 0.258079 | 0.313926 | — | — | — |
| variant_emplace_type.test | RUN | 0.238605 | 0.736171 | 0.948498 | 0.001816 | 0.001952 | 0.001806 |
| variant_emplace_type_cx.test | COMPILE | 0.108140 | 0.250096 | 0.358045 | — | — | — |
| variant_swap.test | RUN | 0.251502 | 0.879653 | 1.007009 | 0.001644 | 0.001804 | 0.001475 |
| variant_eq_ne.test | RUN | 0.194238 | 0.529359 | 0.769571 | 0.001509 | 0.001891 | 0.001738 |
| variant_eq_ne_cx.test | COMPILE | 0.114016 | 0.269437 | 0.326299 | — | — | — |
| variant_lt_gt.test | RUN | 0.210983 | 0.606366 | 0.769212 | 0.001764 | 0.001972 | 0.001677 |
| variant_lt_gt_cx.test | COMPILE | 0.107637 | 0.250260 | 0.345048 | — | — | — |
| variant_destroy.test | RUN | 0.218446 | 0.604601 | 0.806303 | 0.001563 | 0.001927 | 0.001786 |
| variant_visit.test | RUN | 1.201634 | 4.680518 | 12.439057 | 0.001562 | 0.002040 | 0.001636 |
| variant_convert_construct.test | RUN | 0.266730 | 0.833454 | 1.100237 | 0.001733 | 0.002066 | 0.001663 |
| variant_subset.test | RUN | 0.277565 | 0.858609 | 1.132118 | 0.001804 | 0.001612 | 0.001868 |
| variant_valueless.test | RUN | 0.196611 | 0.583235 | 0.814872 | 0.001670 | 0.001754 | 0.001606 |
| variant_copy_construct_throw.test | RUN | 0.161169 | 0.425124 | 0.561964 | 0.001820 | 0.001762 | 0.001847 |
| variant_move_construct_throw.test | RUN | 0.155841 | 0.422559 | 0.552428 | 0.001758 | 0.001844 | 0.001491 |
| variant_convert_construct_throw.test | RUN | 0.160304 | 0.459398 | 0.598057 | 0.001615 | 0.001722 | 0.001722 |
| variant_copy_assign_throw.test | RUN | 0.152164 | 0.396707 | 0.559114 | 0.001653 | 0.001932 | 0.001882 |
| variant_move_assign_throw.test | RUN | 0.153750 | 0.385562 | 0.547688 | 0.001631 | 0.001859 | 0.001856 |
| variant_get_by_index_nx.test | RUN | 0.358732 | 1.810780 | 1.411525 | 0.001648 | 0.001752 | 0.001778 |
| variant_get_by_index_cx_nx.test | COMPILE | 0.125573 | 0.305628 | 0.405044 | — | — | — |
| variant_get_by_type_nx.test | RUN | 0.365594 | 1.993373 | 1.516626 | 0.001718 | 0.001768 | 0.001804 |
| variant_get_by_type_cx_nx.test | COMPILE | 0.114261 | 0.257213 | 0.357917 | — | — | — |
| variant_subset_nx.test | RUN | 0.278963 | 0.801980 | 1.169358 | 0.001616 | 0.001870 | 0.001895 |
| variant_hash.test | RUN | 0.269438 | 0.934369 | 1.090540 | 0.001762 | 0.001972 | 0.001709 |
| variant_trivial.test | RUN | 1.619618 | 5.354121 | 8.184199 | 0.001586 | 0.001928 | 0.001767 |
| variant_special.test | RUN | 1.516591 | 5.755429 | 8.520714 | 0.001838 | 0.002020 | 0.001724 |
| variant_visit_derived.test | RUN | 0.199606 | 0.518343 | 0.783339 | 0.001782 | 0.002002 | 0.001850 |
| variant_many_types.test | RUN | 1.581384 | 7.378980 | 10.965992 | 0.001530 | 0.001851 | 0.001599 |
| variant_visit_r.test | RUN | 0.655756 | 2.233396 | 4.371335 | 0.001775 | 0.001815 | 0.001783 |
| variant_derived_construct.test | COMPILE | 0.118385 | 0.268893 | 0.385190 | — | — | — |
| variant_visit_by_index.test | RUN | 0.258859 | 0.656055 | 1.104352 | 0.001468 | 0.001881 | 0.001842 |
| variant_ostream_insert.test | RUN | 0.187002 | 0.544678 | 0.613038 | 0.001656 | 0.001724 | 0.001797 |
| is_output_streamable.test | RUN | 0.222294 | 1.099842 | 0.969358 | 0.001445 | 0.001860 | 0.001754 |
| variant_json_value_from.test | RUN | 0.350253 | 0.851034 | 1.261606 | 0.001734 | 0.001940 | 0.001758 |
| variant_json_value_to.test | RUN | 0.434377 | 1.199793 | 1.656767 | 0.001669 | 0.001868 | 0.001684 |
| variant_uses_double_storage.test | COMPILE | 0.123268 | 0.276573 | 0.422674 | — | — | — |
| variant_derived_construct2.test | RUN | 0.163309 | 0.439245 | 0.543186 | 0.001377 | 0.001924 | 0.001738 |
| variant_visit_cx.test | COMPILE | 0.105922 | 0.259515 | 0.357166 | — | — | — |
| variant_visit_cx_2.test | COMPILE | 0.112013 | 0.260973 | 0.329908 | — | — | — |
| variant_visit_r_cx.test | COMPILE | 0.107569 | 0.255502 | 0.356759 | — | — | — |
| variant_index_type_126.test | RUN | 1.078082 | 2.493728 | 5.128670 | 0.001520 | 0.001897 | 0.001824 |
| variant_index_type_127.test | RUN | 1.092204 | 2.482935 | 5.209886 | 0.001658 | 0.001936 | 0.001734 |
| variant_index_type_128.test | RUN | 1.069555 | 2.568661 | 5.239438 | 0.001582 | 0.001920 | 0.001365 |
| variant_index_type_129.test | RUN | 1.054473 | 2.534030 | 5.283648 | 0.001789 | 0.001620 | 0.001689 |
| variant_index_type_254.test | RUN | 3.624190 | 9.393499 | 15.773464 | 0.001681 | 0.001906 | 0.001577 |
| variant_index_type_255.test | RUN | 3.628420 | 9.346409 | 15.830447 | 0.001687 | 0.001917 | 0.001622 |
| variant_index_type_256.test | RUN | 3.701195 | 9.543053 | 16.132067 | 0.001519 | 0.001685 | 0.001849 |
| variant_index_type_257.test | RUN | 3.744457 | 9.627594 | 16.287592 | 0.001495 | 0.001734 | 0.001360 |
| variant_default_construct_cx_2.test | COMPILE | 0.108152 | 0.287189 | 0.368398 | — | — | — |
| variant_default_construct_cx_3.test | COMPILE | 0.113119 | 0.307172 | 0.424543 | — | — | — |
| variant_default_construct_cx_4.test | COMPILE | 0.109641 | 0.264363 | 0.348714 | — | — | — |
| variant_default_construct_cx_5.test | COMPILE | 0.107582 | 0.249184 | 0.351843 | — | — | — |
| variant_value_construct_cx_2.test | COMPILE | 0.109684 | 0.254199 | 0.382994 | — | — | — |
| variant_value_construct_cx_3.test | COMPILE | 0.101871 | 0.249343 | 0.348624 | — | — | — |
| variant_value_construct_cx_4.test | COMPILE | 0.096516 | 0.184863 | 0.332361 | — | — | — |
| variant_issue_55.test | RUN | 0.168509 | 0.431995 | 0.515359 | 0.001271 | 0.001469 | 0.001230 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.991× | 0.983×, 0.999× | 1.016× |
| clang | 1.003× | 0.990×, 1.015× | 0.992× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737952).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| is_output_streamable.test | 10 | 0.001163s | 0.001218s | 0.001159s | byte equality observed |
| quick.test | 10 | 0.001147s | 0.000514s | 0.001126s | byte equality observed |
| variant_alternative.test | 10 | 0.001175s | 0.001202s | 0.001173s | byte equality observed |
| variant_convert_construct.test | 10 | 0.001175s | 0.001168s | 0.001143s | byte equality observed |
| variant_convert_construct_throw.test | 10 | 0.001198s | 0.001209s | 0.001194s | byte equality observed |
| variant_copy_assign.test | 10 | 0.001175s | 0.001171s | 0.001163s | byte equality observed |
| variant_copy_assign_throw.test | 10 | 0.001191s | 0.001234s | 0.001204s | byte equality observed |
| variant_copy_construct.test | 10 | 0.001192s | 0.001172s | 0.001161s | byte equality observed |
| variant_copy_construct_throw.test | 10 | 0.001193s | 0.001204s | 0.001211s | byte equality observed |
| variant_default_construct.test | 10 | 0.001174s | 0.001193s | 0.001152s | byte equality observed |
| variant_derived_construct2.test | 10 | 0.001169s | 0.001170s | 0.001152s | byte equality observed |
| variant_destroy.test | 10 | 0.001182s | 0.001163s | 0.001181s | byte equality observed |
| variant_emplace_index.test | 10 | 0.001174s | 0.001177s | 0.001151s | byte equality observed |
| variant_emplace_type.test | 10 | 0.001178s | 0.001168s | 0.001145s | byte equality observed |
| variant_eq_ne.test | 10 | 0.001159s | 0.001170s | 0.001170s | byte equality observed |
| variant_get_by_index.test | 10 | 0.001232s | 0.001273s | 0.001228s | byte equality observed |
| variant_get_by_index_nx.test | 10 | 0.001192s | 0.001162s | 0.001130s | byte equality observed |
| variant_get_by_type.test | 10 | 0.001231s | 0.001244s | 0.001213s | byte equality observed |
| variant_get_by_type_nx.test | 10 | 0.001201s | 0.001190s | 0.001148s | byte equality observed |
| variant_hash.test | 10 | 0.001170s | 0.001207s | 0.001166s | byte equality observed |
| variant_holds_alternative.test | 10 | 0.001177s | 0.001197s | 0.001177s | byte equality observed |
| variant_in_place_index_construct.test | 10 | 0.001189s | 0.001202s | 0.001196s | byte equality observed |
| variant_in_place_type_construct.test | 10 | 0.001194s | 0.001204s | 0.001175s | byte equality observed |
| variant_index_type_126.test | 10 | 0.001184s | 0.001158s | 0.001175s | byte equality observed |
| variant_index_type_127.test | 10 | 0.001175s | 0.001143s | 0.001154s | byte equality observed |
| variant_index_type_128.test | 10 | 0.001176s | 0.001173s | 0.001137s | byte equality observed |
| variant_index_type_129.test | 10 | 0.001173s | 0.001198s | 0.001152s | byte equality observed |
| variant_index_type_254.test | 10 | 0.001173s | 0.001158s | 0.001159s | byte equality observed |
| variant_index_type_255.test | 10 | 0.001178s | 0.001186s | 0.001177s | byte equality observed |
| variant_index_type_256.test | 10 | 0.001172s | 0.001171s | 0.001212s | byte equality observed |
| variant_index_type_257.test | 10 | 0.001169s | 0.001161s | 0.001198s | byte equality observed |
| variant_issue_55.test | 10 | 0.001186s | 0.001190s | 0.001204s | byte equality observed |
| variant_json_value_from.test | 10 | 0.001206s | 0.001235s | 0.001182s | byte equality observed |
| variant_json_value_to.test | 10 | 0.001245s | 0.001236s | 0.001234s | byte equality observed |
| variant_lt_gt.test | 10 | 0.001169s | 0.001186s | 0.001159s | byte equality observed |
| variant_many_types.test | 10 | 0.001211s | 0.001191s | 0.001186s | byte equality observed |
| variant_move_assign.test | 10 | 0.001188s | 0.001158s | 0.001159s | byte equality observed |
| variant_move_assign_throw.test | 10 | 0.001190s | 0.001203s | 0.001204s | byte equality observed |
| variant_move_construct.test | 10 | 0.001183s | 0.001189s | 0.001152s | byte equality observed |
| variant_move_construct_throw.test | 10 | 0.001192s | 0.001215s | 0.001216s | byte equality observed |
| variant_ostream_insert.test | 10 | 0.001181s | 0.001206s | 0.001192s | byte equality observed |
| variant_size.test | 10 | 0.001169s | 0.001152s | 0.001151s | byte equality observed |
| variant_special.test | 10 | 0.001264s | 0.001240s | 0.001250s | byte equality observed |
| variant_subset.test | 10 | 0.001227s | 0.001200s | 0.001219s | byte equality observed |
| variant_subset_nx.test | 10 | 0.001190s | 0.001171s | 0.001147s | byte equality observed |
| variant_swap.test | 10 | 0.001174s | 0.001178s | 0.001191s | byte equality observed |
| variant_trivial.test | 10 | 0.001190s | 0.001245s | 0.001198s | byte equality observed |
| variant_value_assign.test | 10 | 0.001167s | 0.001175s | 0.001189s | byte equality observed |
| variant_value_construct.test | 10 | 0.001170s | 0.001193s | 0.001172s | byte equality observed |
| variant_valueless.test | 10 | 0.001225s | 0.001244s | 0.001239s | byte equality observed |
| variant_visit.test | 10 | 0.001185s | 0.001200s | 0.001165s | byte equality observed |
| variant_visit_by_index.test | 10 | 0.001160s | 0.001163s | 0.001169s | byte equality observed |
| variant_visit_derived.test | 10 | 0.001174s | 0.001166s | 0.001179s | byte equality observed |
| variant_visit_r.test | 10 | 0.001188s | 0.001147s | 0.001170s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737952).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
