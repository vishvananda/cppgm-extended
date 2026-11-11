# libs/variant2/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| quick.test | RUN | 0.156595 | 0.342723 | 0.505100 | 0.001720 | 0.000652 | 0.002893 |
| variant_size.test | RUN | 0.253973 | 0.767457 | 0.921290 | 0.001958 | 0.001750 | 0.001975 |
| variant_alternative.test | RUN | 0.320859 | 2.585717 | 1.526022 | 0.001714 | 0.001771 | 0.001676 |
| variant_holds_alternative.test | RUN | 0.285920 | 0.644744 | 0.930261 | 0.002021 | 0.002040 | 0.002026 |
| variant_holds_alternative_cx.test | COMPILE | 0.155722 | 0.346770 | 0.546722 | — | — | — |
| variant_get_by_index.test | RUN | 0.382687 | 2.766072 | 1.847080 | 0.001848 | 0.002422 | 0.002024 |
| variant_get_by_index_cx.test | COMPILE | 0.162805 | 0.354271 | 0.535635 | — | — | — |
| variant_get_by_type.test | RUN | 0.386979 | 2.905587 | 1.948362 | 0.002140 | 0.002398 | 0.001795 |
| variant_get_by_type_cx.test | COMPILE | 0.160270 | 0.314219 | 0.472875 | — | — | — |
| variant_default_construct.test | RUN | 0.366316 | 2.166384 | 1.684120 | 0.002118 | 0.002200 | 0.002360 |
| variant_default_construct_cx.test | COMPILE | 0.206056 | 0.349960 | 0.551655 | — | — | — |
| variant_copy_construct.test | RUN | 0.617637 | 3.680099 | 2.991945 | 0.001726 | 0.001683 | 0.001941 |
| variant_copy_construct_cx.test | COMPILE | 0.181845 | 0.406441 | 0.651379 | — | — | — |
| variant_move_construct.test | RUN | 0.615786 | 3.189108 | 2.563150 | 0.001806 | 0.001760 | 0.001672 |
| variant_move_construct_cx.test | COMPILE | 0.144711 | 0.309077 | 0.506060 | — | — | — |
| variant_value_construct.test | RUN | 0.287821 | 0.825607 | 1.107971 | 0.001722 | 0.001772 | 0.002271 |
| variant_value_construct_cx.test | COMPILE | 0.160638 | 0.360797 | 0.549235 | — | — | — |
| variant_in_place_index_construct.test | RUN | 0.304817 | 0.818769 | 1.227530 | 0.001862 | 0.002020 | 0.002636 |
| variant_in_place_index_construct_cx.test | COMPILE | 0.172624 | 0.367422 | 0.580844 | — | — | — |
| variant_in_place_type_construct.test | RUN | 0.308204 | 0.812463 | 1.137209 | 0.001689 | 0.002120 | 0.002694 |
| variant_in_place_type_construct_cx.test | COMPILE | 0.180020 | 0.375088 | 0.552473 | — | — | — |
| variant_copy_assign.test | RUN | 0.397933 | 2.476403 | 1.866755 | 0.001744 | 0.001996 | 0.001865 |
| variant_copy_assign_cx.test | COMPILE | 0.152652 | 0.340396 | 0.470948 | — | — | — |
| variant_move_assign.test | RUN | 0.427316 | 2.503509 | 1.735165 | 0.001942 | 0.001726 | 0.001924 |
| variant_move_assign_cx.test | COMPILE | 0.143121 | 0.319134 | 0.485483 | — | — | — |
| variant_value_assign.test | RUN | 0.313725 | 0.943118 | 1.295824 | 0.001428 | 0.001819 | 0.002117 |
| variant_value_assign_cx.test | COMPILE | 0.146158 | 0.311173 | 0.491757 | — | — | — |
| variant_emplace_index.test | RUN | 0.343886 | 1.101181 | 1.449456 | 0.002248 | 0.001716 | 0.001770 |
| variant_emplace_index_cx.test | COMPILE | 0.147875 | 0.316226 | 0.472641 | — | — | — |
| variant_emplace_type.test | RUN | 0.306316 | 0.898250 | 1.264974 | 0.001668 | 0.002036 | 0.001812 |
| variant_emplace_type_cx.test | COMPILE | 0.149257 | 0.314593 | 0.472925 | — | — | — |
| variant_swap.test | RUN | 0.320059 | 1.100063 | 1.276108 | 0.001618 | 0.001499 | 0.001744 |
| variant_eq_ne.test | RUN | 0.258642 | 0.668253 | 0.943968 | 0.001758 | 0.001792 | 0.001856 |
| variant_eq_ne_cx.test | COMPILE | 0.143947 | 0.327402 | 0.453982 | — | — | — |
| variant_lt_gt.test | RUN | 0.274609 | 0.754840 | 0.994287 | 0.001693 | 0.002136 | 0.001702 |
| variant_lt_gt_cx.test | COMPILE | 0.148818 | 0.320609 | 0.453135 | — | — | — |
| variant_destroy.test | RUN | 0.272972 | 0.765223 | 1.020558 | 0.001709 | 0.001736 | 0.001883 |
| variant_visit.test | RUN | 1.622022 | 5.726983 | 15.953830 | 0.001846 | 0.001996 | 0.002474 |
| variant_convert_construct.test | RUN | 0.342469 | 1.005298 | 1.466133 | 0.001942 | 0.002024 | 0.002781 |
| variant_subset.test | RUN | 0.363306 | 1.079881 | 1.476884 | 0.001538 | 0.001882 | 0.001777 |
| variant_valueless.test | RUN | 0.261719 | 0.733674 | 1.004144 | 0.001451 | 0.001889 | 0.001756 |
| variant_copy_construct_throw.test | RUN | 0.205262 | 0.498771 | 0.721284 | 0.001731 | 0.001766 | 0.001626 |
| variant_move_construct_throw.test | RUN | 0.213751 | 0.506446 | 0.705668 | 0.001774 | 0.001998 | 0.001619 |
| variant_convert_construct_throw.test | RUN | 0.225951 | 0.564295 | 0.773840 | 0.001872 | 0.002161 | 0.001636 |
| variant_copy_assign_throw.test | RUN | 0.202369 | 0.503344 | 0.711032 | 0.001735 | 0.001732 | 0.001716 |
| variant_move_assign_throw.test | RUN | 0.208892 | 0.489028 | 0.732740 | 0.001435 | 0.001699 | 0.001758 |
| variant_get_by_index_nx.test | RUN | 0.459706 | 2.294399 | 1.732784 | 0.001788 | 0.002809 | 0.002100 |
| variant_get_by_index_cx_nx.test | COMPILE | 0.167807 | 0.366899 | 0.547995 | — | — | — |
| variant_get_by_type_nx.test | RUN | 0.465978 | 2.591843 | 1.912579 | 0.001716 | 0.002130 | 0.001806 |
| variant_get_by_type_cx_nx.test | COMPILE | 0.143756 | 0.314071 | 0.446231 | — | — | — |
| variant_subset_nx.test | RUN | 0.361534 | 1.103471 | 1.448117 | 0.001924 | 0.001786 | 0.001543 |
| variant_hash.test | RUN | 0.359385 | 1.159653 | 1.392141 | 0.001619 | 0.001997 | 0.001621 |
| variant_trivial.test | RUN | 1.966278 | 6.631937 | 10.577231 | 0.002074 | 0.002176 | 0.002359 |
| variant_special.test | RUN | 1.967384 | 8.084688 | 10.771167 | 0.002191 | 0.002509 | 0.003459 |
| variant_visit_derived.test | RUN | 0.236002 | 0.668273 | 0.978900 | 0.002014 | 0.002332 | 0.001712 |
| variant_many_types.test | RUN | 2.083583 | 9.454293 | 14.530849 | 0.002301 | 0.002187 | 0.002825 |
| variant_visit_r.test | RUN | 0.775505 | 2.969032 | 5.273921 | 0.002068 | 0.002175 | 0.002120 |
| variant_derived_construct.test | COMPILE | 0.145693 | 0.321133 | 0.465229 | — | — | — |
| variant_visit_by_index.test | RUN | 0.329809 | 0.849725 | 1.370408 | 0.001675 | 0.001592 | 0.001655 |
| variant_ostream_insert.test | RUN | 0.254847 | 0.685608 | 0.896536 | 0.002086 | 0.002922 | 0.002090 |
| is_output_streamable.test | RUN | 0.289429 | 1.388297 | 1.192292 | 0.001726 | 0.002226 | 0.001740 |
| variant_json_value_from.test | RUN | 0.448833 | 1.110423 | 1.643863 | 0.002334 | 0.002923 | 0.001834 |
| variant_json_value_to.test | RUN | 0.550772 | 1.711187 | 2.061786 | 0.003048 | 0.002403 | 0.002046 |
| variant_uses_double_storage.test | COMPILE | 0.163607 | 0.332621 | 0.535053 | — | — | — |
| variant_derived_construct2.test | RUN | 0.217912 | 0.528394 | 0.785387 | 0.002162 | 0.002821 | 0.002260 |
| variant_visit_cx.test | COMPILE | 0.148417 | 0.323819 | 0.448870 | — | — | — |
| variant_visit_cx_2.test | COMPILE | 0.147065 | 0.313404 | 0.452909 | — | — | — |
| variant_visit_r_cx.test | COMPILE | 0.141519 | 0.312090 | 0.455830 | — | — | — |
| variant_index_type_126.test | RUN | 1.316710 | 3.466872 | 7.146919 | 0.002350 | 0.002290 | 0.002652 |
| variant_index_type_127.test | RUN | 1.292456 | 3.372467 | 7.046967 | 0.001499 | 0.003199 | 0.002149 |
| variant_index_type_128.test | RUN | 1.344785 | 3.330703 | 7.264827 | 0.001872 | 0.001978 | 0.002290 |
| variant_index_type_129.test | RUN | 1.320501 | 3.333405 | 7.095194 | 0.002093 | 0.002392 | 0.002054 |
| variant_index_type_254.test | RUN | 4.514168 | 11.903737 | 20.413331 | 0.002128 | 0.001991 | 0.002926 |
| variant_index_type_255.test | RUN | 4.595375 | 12.777446 | 19.952167 | 0.002156 | 0.002158 | 0.002204 |
| variant_index_type_256.test | RUN | 4.710406 | 12.322206 | 20.289140 | 0.002010 | 0.002263 | 0.002275 |
| variant_index_type_257.test | RUN | 4.636779 | 12.258301 | 20.919099 | 0.002180 | 0.001976 | 0.002215 |
| variant_default_construct_cx_2.test | COMPILE | 0.150725 | 0.316116 | 0.460561 | — | — | — |
| variant_default_construct_cx_3.test | COMPILE | 0.166786 | 0.318127 | 0.477369 | — | — | — |
| variant_default_construct_cx_4.test | COMPILE | 0.147504 | 0.328945 | 0.452797 | — | — | — |
| variant_default_construct_cx_5.test | COMPILE | 0.147858 | 0.323739 | 0.441880 | — | — | — |
| variant_value_construct_cx_2.test | COMPILE | 0.155579 | 0.318644 | 0.594680 | — | — | — |
| variant_value_construct_cx_3.test | COMPILE | 0.149913 | 0.325862 | 0.449435 | — | — | — |
| variant_value_construct_cx_4.test | COMPILE | 0.144513 | 0.329205 | 0.474095 | — | — | — |
| variant_issue_55.test | RUN | 0.287297 | 0.772101 | 1.040211 | 0.002348 | 0.001415 | 0.001961 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.009×† | 0.992×, 1.027× | 0.901× |
| clang | 0.972×† | 0.932×, 1.012× | 1.004× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1353 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| is_output_streamable.test | 8 | 0.001609s | 0.001700s | 0.001598s | byte equality observed |
| quick.test | 7 | 0.001562s | 0.000837s | 0.001673s | byte equality observed |
| variant_alternative.test | 6 | 0.001642s | 0.001704s | 0.001743s | byte equality observed |
| variant_convert_construct.test | 8 | 0.001623s | 0.001649s | 0.001592s | byte equality observed |
| variant_convert_construct_throw.test | 7 | 0.001641s | 0.001739s | 0.001693s | byte equality observed |
| variant_copy_assign.test | 8 | 0.001634s | 0.001714s | 0.001544s | byte equality observed |
| variant_copy_assign_throw.test | 8 | 0.001676s | 0.001667s | 0.001606s | byte equality observed |
| variant_copy_construct.test | 7 | 0.001601s | 0.001595s | 0.001619s | byte equality observed |
| variant_copy_construct_throw.test | 7 | 0.001772s | 0.001760s | 0.001722s | byte equality observed |
| variant_default_construct.test | 9 | 0.001585s | 0.001695s | 0.001532s | byte equality observed |
| variant_derived_construct2.test | 9 | 0.001596s | 0.001674s | 0.001606s | byte equality observed |
| variant_destroy.test | 8 | 0.001586s | 0.001808s | 0.001590s | byte equality observed |
| variant_emplace_index.test | 9 | 0.001662s | 0.001721s | 0.001726s | byte equality observed |
| variant_emplace_type.test | 7 | 0.001572s | 0.001695s | 0.001591s | byte equality observed |
| variant_eq_ne.test | 8 | 0.001620s | 0.001703s | 0.001527s | byte equality observed |
| variant_get_by_index.test | 8 | 0.001702s | 0.001898s | 0.001723s | byte equality observed |
| variant_get_by_index_nx.test | 7 | 0.001629s | 0.001638s | 0.001590s | byte equality observed |
| variant_get_by_type.test | 9 | 0.001658s | 0.001775s | 0.001652s | byte equality observed |
| variant_get_by_type_nx.test | 8 | 0.001642s | 0.001646s | 0.001636s | byte equality observed |
| variant_hash.test | 7 | 0.001558s | 0.001784s | 0.001593s | byte equality observed |
| variant_holds_alternative.test | 8 | 0.001735s | 0.001668s | 0.001633s | byte equality observed |
| variant_in_place_index_construct.test | 9 | 0.001671s | 0.001729s | 0.001677s | byte equality observed |
| variant_in_place_type_construct.test | 6 | 0.001658s | 0.001739s | 0.001582s | byte equality observed |
| variant_index_type_126.test | 8 | 0.001615s | 0.001845s | 0.001557s | byte equality observed |
| variant_index_type_127.test | 8 | 0.001594s | 0.001677s | 0.001693s | byte equality observed |
| variant_index_type_128.test | 8 | 0.001589s | 0.001608s | 0.001650s | byte equality observed |
| variant_index_type_129.test | 8 | 0.001609s | 0.001718s | 0.001557s | byte equality observed |
| variant_index_type_254.test | 7 | 0.001693s | 0.001558s | 0.001687s | byte equality observed |
| variant_index_type_255.test | 7 | 0.001649s | 0.001643s | 0.001579s | byte equality observed |
| variant_index_type_256.test | 8 | 0.001606s | 0.001640s | 0.001601s | byte equality observed |
| variant_index_type_257.test | 8 | 0.001609s | 0.001796s | 0.001570s | byte equality observed |
| variant_issue_55.test | 7 | 0.001678s | 0.001665s | 0.001647s | byte equality observed |
| variant_json_value_from.test | 8 | 0.001688s | 0.001770s | 0.001678s | byte equality observed |
| variant_json_value_to.test | 9 | 0.001713s | 0.001754s | 0.001602s | byte equality observed |
| variant_lt_gt.test | 6 | 0.001621s | 0.001677s | 0.001615s | byte equality observed |
| variant_many_types.test | 8 | 0.001711s | 0.001649s | 0.001717s | byte equality observed |
| variant_move_assign.test | 8 | 0.001651s | 0.001727s | 0.001593s | byte equality observed |
| variant_move_assign_throw.test | 7 | 0.001677s | 0.001800s | 0.001582s | byte equality observed |
| variant_move_construct.test | 7 | 0.001681s | 0.001637s | 0.001600s | byte equality observed |
| variant_move_construct_throw.test | 7 | 0.001646s | 0.001643s | 0.001623s | byte equality observed |
| variant_ostream_insert.test | 7 | 0.001635s | 0.001751s | 0.001645s | byte equality observed |
| variant_size.test | 7 | 0.001604s | 0.001604s | 0.001583s | byte equality observed |
| variant_special.test | 7 | 0.001761s | 0.001685s | 0.001626s | byte equality observed |
| variant_subset.test | 8 | 0.001729s | 0.001677s | 0.001618s | byte equality observed |
| variant_subset_nx.test | 7 | 0.001587s | 0.001645s | 0.001654s | byte equality observed |
| variant_swap.test | 6 | 0.001634s | 0.001618s | 0.001663s | byte equality observed |
| variant_trivial.test | 9 | 0.001691s | 0.001706s | 0.001706s | byte equality observed |
| variant_value_assign.test | 6 | 0.001663s | 0.001615s | 0.001644s | byte equality observed |
| variant_value_construct.test | 8 | 0.001664s | 0.001652s | 0.001637s | byte equality observed |
| variant_valueless.test | 7 | 0.001696s | 0.001662s | 0.001636s | byte equality observed |
| variant_visit.test | 8 | 0.001664s | 0.001697s | 0.001634s | byte equality observed |
| variant_visit_by_index.test | 7 | 0.001658s | 0.001574s | 0.001768s | byte equality observed |
| variant_visit_derived.test | 8 | 0.001697s | 0.001824s | 0.001648s | byte equality observed |
| variant_visit_r.test | 6 | 0.001726s | 0.001671s | 0.001699s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
