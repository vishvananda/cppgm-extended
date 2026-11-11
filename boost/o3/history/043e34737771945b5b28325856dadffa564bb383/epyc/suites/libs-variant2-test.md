# libs/variant2/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| quick.test | RUN | 0.106782 | 0.189510 | 0.255083 | 0.001739 | 0.000774 | 0.001755 |
| variant_size.test | RUN | 0.189727 | 0.478821 | 0.474605 | 0.001792 | 0.001833 | 0.001680 |
| variant_alternative.test | RUN | 0.230362 | 1.572330 | 0.814078 | 0.001791 | 0.001884 | 0.001372 |
| variant_holds_alternative.test | RUN | 0.168625 | 0.374873 | 0.479799 | 0.001544 | 0.001756 | 0.001748 |
| variant_holds_alternative_cx.test | COMPILE | 0.112160 | 0.224729 | 0.253778 | — | — | — |
| variant_get_by_index.test | RUN | 0.258491 | 1.757368 | 1.036025 | 0.002298 | 0.001803 | 0.001882 |
| variant_get_by_index_cx.test | COMPILE | 0.119125 | 0.228315 | 0.268891 | — | — | — |
| variant_get_by_type.test | RUN | 0.281085 | 1.861885 | 1.098703 | 0.001605 | 0.001604 | 0.001802 |
| variant_get_by_type_cx.test | COMPILE | 0.105557 | 0.199809 | 0.237347 | — | — | — |
| variant_default_construct.test | RUN | 0.243772 | 1.365365 | 0.904749 | 0.001873 | 0.001737 | 0.001785 |
| variant_default_construct_cx.test | COMPILE | 0.111651 | 0.204558 | 0.280208 | — | — | — |
| variant_copy_construct.test | RUN | 0.393565 | 2.382452 | 1.600557 | 0.001571 | 0.001902 | 0.001744 |
| variant_copy_construct_cx.test | COMPILE | 0.125663 | 0.232911 | 0.351209 | — | — | — |
| variant_move_construct.test | RUN | 0.369611 | 1.952112 | 1.485877 | 0.001514 | 0.001662 | 0.001656 |
| variant_move_construct_cx.test | COMPILE | 0.101031 | 0.221892 | 0.233750 | — | — | — |
| variant_value_construct.test | RUN | 0.195242 | 0.502620 | 0.614492 | 0.001450 | 0.001720 | 0.001736 |
| variant_value_construct_cx.test | COMPILE | 0.116290 | 0.218291 | 0.270622 | — | — | — |
| variant_in_place_index_construct.test | RUN | 0.208255 | 0.499495 | 0.623334 | 0.002097 | 0.001659 | 0.001506 |
| variant_in_place_index_construct_cx.test | COMPILE | 0.116653 | 0.225691 | 0.289337 | — | — | — |
| variant_in_place_type_construct.test | RUN | 0.211223 | 0.493130 | 0.586520 | 0.001490 | 0.001722 | 0.001419 |
| variant_in_place_type_construct_cx.test | COMPILE | 0.119789 | 0.228285 | 0.275997 | — | — | — |
| variant_copy_assign.test | RUN | 0.299057 | 1.602042 | 0.960843 | 0.001620 | 0.001598 | 0.001184 |
| variant_copy_assign_cx.test | COMPILE | 0.105689 | 0.185941 | 0.233767 | — | — | — |
| variant_move_assign.test | RUN | 0.297125 | 1.565841 | 0.918467 | 0.001632 | 0.003582 | 0.001337 |
| variant_move_assign_cx.test | COMPILE | 0.112657 | 0.191526 | 0.214992 | — | — | — |
| variant_value_assign.test | RUN | 0.215926 | 0.563276 | 0.623610 | 0.001562 | 0.001765 | 0.001474 |
| variant_value_assign_cx.test | COMPILE | 0.104709 | 0.192653 | 0.226478 | — | — | — |
| variant_emplace_index.test | RUN | 0.230163 | 0.653920 | 0.702286 | 0.001706 | 0.001549 | 0.001647 |
| variant_emplace_index_cx.test | COMPILE | 0.102499 | 0.189240 | 0.222216 | — | — | — |
| variant_emplace_type.test | RUN | 0.212605 | 0.569598 | 0.630171 | 0.001752 | 0.001828 | 0.001599 |
| variant_emplace_type_cx.test | COMPILE | 0.100062 | 0.227054 | 0.221060 | — | — | — |
| variant_swap.test | RUN | 0.219247 | 0.654594 | 0.649607 | 0.001738 | 0.001441 | 0.001584 |
| variant_eq_ne.test | RUN | 0.182575 | 0.391267 | 0.494937 | 0.001554 | 0.001556 | 0.001536 |
| variant_eq_ne_cx.test | COMPILE | 0.103734 | 0.195406 | 0.237793 | — | — | — |
| variant_lt_gt.test | RUN | 0.193501 | 0.457319 | 0.504302 | 0.001777 | 0.001848 | 0.001558 |
| variant_lt_gt_cx.test | COMPILE | 0.101935 | 0.186451 | 0.203951 | — | — | — |
| variant_destroy.test | RUN | 0.189523 | 0.460931 | 0.513208 | 0.001852 | 0.001841 | 0.001660 |
| variant_visit.test | RUN | 0.998709 | 3.587293 | 9.206567 | 0.001534 | 0.001671 | 0.001649 |
| variant_convert_construct.test | RUN | 0.232979 | 0.610638 | 0.740051 | 0.001822 | 0.001848 | 0.001596 |
| variant_subset.test | RUN | 0.237853 | 0.646680 | 0.764148 | 0.001838 | 0.002060 | 0.001627 |
| variant_valueless.test | RUN | 0.173707 | 0.441277 | 0.511062 | 0.001560 | 0.001770 | 0.001703 |
| variant_copy_construct_throw.test | RUN | 0.145253 | 0.312559 | 0.368507 | 0.001596 | 0.001718 | 0.001560 |
| variant_move_construct_throw.test | RUN | 0.143209 | 0.325172 | 0.362203 | 0.001656 | 0.001618 | 0.001786 |
| variant_convert_construct_throw.test | RUN | 0.150375 | 0.351031 | 0.384254 | 0.001606 | 0.001815 | 0.001753 |
| variant_copy_assign_throw.test | RUN | 0.146171 | 0.304213 | 0.385424 | 0.001554 | 0.001914 | 0.001701 |
| variant_move_assign_throw.test | RUN | 0.144664 | 0.299359 | 0.383082 | 0.001874 | 0.001772 | 0.001783 |
| variant_get_by_index_nx.test | RUN | 0.300600 | 1.494878 | 0.987451 | 0.001898 | 0.001864 | 0.001783 |
| variant_get_by_index_cx_nx.test | COMPILE | 0.112144 | 0.237664 | 0.273729 | — | — | — |
| variant_get_by_type_nx.test | RUN | 0.317005 | 1.692951 | 1.025766 | 0.001888 | 0.001662 | 0.001612 |
| variant_get_by_type_cx_nx.test | COMPILE | 0.110461 | 0.203988 | 0.234773 | — | — | — |
| variant_subset_nx.test | RUN | 0.281811 | 0.672398 | 0.800902 | 0.001914 | 0.001798 | 0.001732 |
| variant_hash.test | RUN | 0.267339 | 0.731677 | 0.693788 | 0.001923 | 0.001850 | 0.001605 |
| variant_trivial.test | RUN | 1.418607 | 4.163415 | 5.616628 | 0.001877 | 0.001858 | 0.001363 |
| variant_special.test | RUN | 1.389001 | 4.611161 | 5.776294 | 0.001729 | 0.002026 | 0.001844 |
| variant_visit_derived.test | RUN | 0.244285 | 0.390117 | 0.556027 | 0.002257 | 0.001820 | 0.001660 |
| variant_many_types.test | RUN | 1.379003 | 5.806714 | 8.212525 | 0.001482 | 0.001665 | 0.001694 |
| variant_visit_r.test | RUN | 0.537024 | 1.728705 | 3.134083 | 0.001746 | 0.001819 | 0.001750 |
| variant_derived_construct.test | COMPILE | 0.111671 | 0.203731 | 0.248104 | — | — | — |
| variant_visit_by_index.test | RUN | 0.232264 | 0.536999 | 0.721671 | 0.001525 | 0.001973 | 0.001639 |
| variant_ostream_insert.test | RUN | 0.170852 | 0.426812 | 0.525553 | 0.001932 | 0.001754 | 0.001722 |
| is_output_streamable.test | RUN | 0.209248 | 0.860415 | 0.681479 | 0.001543 | 0.001898 | 0.001719 |
| variant_json_value_from.test | RUN | 0.323769 | 0.671444 | 0.862986 | 0.001904 | 0.001974 | 0.001758 |
| variant_json_value_to.test | RUN | 0.381716 | 0.984140 | 1.180999 | 0.001891 | 0.001846 | 0.001640 |
| variant_uses_double_storage.test | COMPILE | 0.111367 | 0.227204 | 0.281977 | — | — | — |
| variant_derived_construct2.test | RUN | 0.155078 | 0.347437 | 0.417704 | 0.001757 | 0.001941 | 0.001578 |
| variant_visit_cx.test | COMPILE | 0.105532 | 0.206470 | 0.227179 | — | — | — |
| variant_visit_cx_2.test | COMPILE | 0.105062 | 0.200295 | 0.223112 | — | — | — |
| variant_visit_r_cx.test | COMPILE | 0.101841 | 0.200779 | 0.231444 | — | — | — |
| variant_index_type_126.test | RUN | 0.902123 | 2.021178 | 3.343042 | 0.001565 | 0.002121 | 0.001506 |
| variant_index_type_127.test | RUN | 0.902365 | 2.057408 | 3.387361 | 0.001643 | 0.001966 | 0.001510 |
| variant_index_type_128.test | RUN | 0.909602 | 2.096218 | 3.427047 | 0.001585 | 0.001617 | 0.001593 |
| variant_index_type_129.test | RUN | 0.918975 | 2.089585 | 3.395218 | 0.001678 | 0.001911 | 0.001723 |
| variant_index_type_254.test | RUN | 3.176852 | 7.284971 | 11.153513 | 0.001667 | 0.001740 | 0.001632 |
| variant_index_type_255.test | RUN | 3.189480 | 7.139755 | 10.929913 | 0.001634 | 0.001633 | 0.001632 |
| variant_index_type_256.test | RUN | 3.230296 | 7.269899 | 11.196538 | 0.001534 | 0.001375 | 0.001613 |
| variant_index_type_257.test | RUN | 3.244753 | 7.516709 | 11.159799 | 0.001551 | 0.001567 | 0.001496 |
| variant_default_construct_cx_2.test | COMPILE | 0.108634 | 0.193579 | 0.224892 | — | — | — |
| variant_default_construct_cx_3.test | COMPILE | 0.104984 | 0.215060 | 0.228644 | — | — | — |
| variant_default_construct_cx_4.test | COMPILE | 0.102675 | 0.192848 | 0.232342 | — | — | — |
| variant_default_construct_cx_5.test | COMPILE | 0.102509 | 0.191696 | 0.237767 | — | — | — |
| variant_value_construct_cx_2.test | COMPILE | 0.103402 | 0.194338 | 0.234669 | — | — | — |
| variant_value_construct_cx_3.test | COMPILE | 0.098767 | 0.136565 | 0.231779 | — | — | — |
| variant_value_construct_cx_4.test | COMPILE | 0.082941 | 0.166971 | 0.184895 | — | — | — |
| variant_issue_55.test | RUN | 0.148695 | 0.364566 | 0.398622 | 0.001389 | 0.001379 | 0.001305 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.009× | 1.016×, 1.001× | 0.964× |
| clang | 0.992× | 0.988×, 0.996× | 1.016× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| is_output_streamable.test | 10 | 0.001077s | 0.001119s | 0.001097s | byte equality observed |
| quick.test | 10 | 0.001057s | 0.000482s | 0.001080s | byte equality observed |
| variant_alternative.test | 10 | 0.001082s | 0.001113s | 0.001093s | byte equality observed |
| variant_convert_construct.test | 10 | 0.001078s | 0.001135s | 0.001079s | byte equality observed |
| variant_convert_construct_throw.test | 10 | 0.001118s | 0.001147s | 0.001123s | byte equality observed |
| variant_copy_assign.test | 10 | 0.001076s | 0.001134s | 0.001088s | byte equality observed |
| variant_copy_assign_throw.test | 10 | 0.001111s | 0.001112s | 0.001130s | byte equality observed |
| variant_copy_construct.test | 10 | 0.001100s | 0.001162s | 0.001078s | byte equality observed |
| variant_copy_construct_throw.test | 10 | 0.001103s | 0.001153s | 0.001109s | byte equality observed |
| variant_default_construct.test | 10 | 0.001083s | 0.001130s | 0.001100s | byte equality observed |
| variant_derived_construct2.test | 10 | 0.001084s | 0.001146s | 0.001079s | byte equality observed |
| variant_destroy.test | 10 | 0.001066s | 0.001148s | 0.001090s | byte equality observed |
| variant_emplace_index.test | 10 | 0.001083s | 0.001140s | 0.001086s | byte equality observed |
| variant_emplace_type.test | 10 | 0.001088s | 0.001134s | 0.001083s | byte equality observed |
| variant_eq_ne.test | 10 | 0.001085s | 0.001120s | 0.001083s | byte equality observed |
| variant_get_by_index.test | 10 | 0.001166s | 0.001194s | 0.001149s | byte equality observed |
| variant_get_by_index_nx.test | 10 | 0.001103s | 0.001118s | 0.001092s | byte equality observed |
| variant_get_by_type.test | 10 | 0.001150s | 0.001186s | 0.001150s | byte equality observed |
| variant_get_by_type_nx.test | 10 | 0.001109s | 0.001134s | 0.001091s | byte equality observed |
| variant_hash.test | 10 | 0.001095s | 0.001133s | 0.001096s | byte equality observed |
| variant_holds_alternative.test | 10 | 0.001103s | 0.001120s | 0.001084s | byte equality observed |
| variant_in_place_index_construct.test | 10 | 0.001068s | 0.001090s | 0.001099s | byte equality observed |
| variant_in_place_type_construct.test | 10 | 0.001095s | 0.001118s | 0.001097s | byte equality observed |
| variant_index_type_126.test | 10 | 0.001082s | 0.001104s | 0.001110s | byte equality observed |
| variant_index_type_127.test | 10 | 0.001099s | 0.001113s | 0.001075s | byte equality observed |
| variant_index_type_128.test | 10 | 0.001089s | 0.001117s | 0.001097s | byte equality observed |
| variant_index_type_129.test | 10 | 0.001086s | 0.001091s | 0.001076s | byte equality observed |
| variant_index_type_254.test | 10 | 0.001090s | 0.001104s | 0.001084s | byte equality observed |
| variant_index_type_255.test | 10 | 0.001076s | 0.001095s | 0.001087s | byte equality observed |
| variant_index_type_256.test | 10 | 0.001096s | 0.001094s | 0.001080s | byte equality observed |
| variant_index_type_257.test | 10 | 0.001121s | 0.001099s | 0.001081s | byte equality observed |
| variant_issue_55.test | 10 | 0.001114s | 0.001099s | 0.001085s | byte equality observed |
| variant_json_value_from.test | 10 | 0.001136s | 0.001164s | 0.001078s | byte equality observed |
| variant_json_value_to.test | 10 | 0.001142s | 0.001147s | 0.001091s | byte equality observed |
| variant_lt_gt.test | 10 | 0.001094s | 0.001117s | 0.001070s | byte equality observed |
| variant_many_types.test | 10 | 0.001149s | 0.001165s | 0.001113s | byte equality observed |
| variant_move_assign.test | 10 | 0.001091s | 0.001115s | 0.001080s | byte equality observed |
| variant_move_assign_throw.test | 10 | 0.001131s | 0.001128s | 0.001110s | byte equality observed |
| variant_move_construct.test | 10 | 0.001110s | 0.001137s | 0.001089s | byte equality observed |
| variant_move_construct_throw.test | 10 | 0.001139s | 0.001131s | 0.001115s | byte equality observed |
| variant_ostream_insert.test | 10 | 0.001119s | 0.001152s | 0.001104s | byte equality observed |
| variant_size.test | 10 | 0.001100s | 0.001113s | 0.001099s | byte equality observed |
| variant_special.test | 8 | 0.001169s | 0.001230s | 0.001163s | byte equality observed |
| variant_subset.test | 9 | 0.001125s | 0.001131s | 0.001164s | byte equality observed |
| variant_subset_nx.test | 10 | 0.001066s | 0.001112s | 0.001092s | byte equality observed |
| variant_swap.test | 10 | 0.001094s | 0.001119s | 0.001117s | byte equality observed |
| variant_trivial.test | 10 | 0.001138s | 0.001198s | 0.001123s | byte equality observed |
| variant_value_assign.test | 9 | 0.001098s | 0.001102s | 0.001095s | byte equality observed |
| variant_value_construct.test | 10 | 0.001090s | 0.001128s | 0.001109s | byte equality observed |
| variant_valueless.test | 10 | 0.001127s | 0.001181s | 0.001123s | byte equality observed |
| variant_visit.test | 10 | 0.001102s | 0.001131s | 0.001125s | byte equality observed |
| variant_visit_by_index.test | 10 | 0.001090s | 0.001107s | 0.001089s | byte equality observed |
| variant_visit_derived.test | 10 | 0.001061s | 0.001116s | 0.001097s | byte equality observed |
| variant_visit_r.test | 10 | 0.001115s | 0.001127s | 0.001116s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
