# libs/variant2/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| quick.test | RUN | 0.107175 | 0.214162 | 0.253944 | 0.001610 | 0.001704 | 0.001683 |
| variant_size.test | RUN | 0.172624 | 0.398957 | 0.450192 | 0.001816 | 0.001972 | 0.001678 |
| variant_alternative.test | RUN | 0.188101 | 0.688585 | 0.651810 | 0.001389 | 0.001967 | 0.001544 |
| variant_holds_alternative.test | RUN | 0.158075 | 0.383552 | 0.452498 | 0.001717 | 0.001779 | 0.001349 |
| variant_holds_alternative_cx.test | COMPILE | 0.114130 | 0.219249 | 0.264441 | — | — | — |
| variant_get_by_index.test | RUN | 0.208281 | 0.838159 | 0.777565 | 0.001955 | 0.001864 | 0.001815 |
| variant_get_by_index_cx.test | COMPILE | 0.112789 | 0.238780 | 0.292702 | — | — | — |
| variant_get_by_type.test | RUN | 0.213964 | 0.886179 | 0.819803 | 0.001911 | 0.001466 | 0.001862 |
| variant_get_by_type_cx.test | COMPILE | 0.102190 | 0.192630 | 0.240301 | — | — | — |
| variant_default_construct.test | RUN | 0.225471 | 0.741927 | 0.750654 | 0.001682 | 0.001814 | 0.001787 |
| variant_default_construct_cx.test | COMPILE | 0.110466 | 0.221186 | 0.287746 | — | — | — |
| variant_copy_construct.test | RUN | 0.279488 | 1.553915 | 1.276696 | 0.001783 | 0.001680 | 0.001895 |
| variant_copy_construct_cx.test | COMPILE | 0.124675 | 0.249835 | 0.328901 | — | — | — |
| variant_move_construct.test | RUN | 0.274984 | 1.473395 | 1.244371 | 0.001877 | 0.001784 | 0.001586 |
| variant_move_construct_cx.test | COMPILE | 0.104386 | 0.182298 | 0.232361 | — | — | — |
| variant_value_construct.test | RUN | 0.188202 | 0.446612 | 0.573745 | 0.001758 | 0.002020 | 0.001874 |
| variant_value_construct_cx.test | COMPILE | 0.116124 | 0.231963 | 0.328331 | — | — | — |
| variant_in_place_index_construct.test | RUN | 0.189213 | 0.485950 | 0.569424 | 0.001517 | 0.001466 | 0.001801 |
| variant_in_place_index_construct_cx.test | COMPILE | 0.121348 | 0.212277 | 0.306315 | — | — | — |
| variant_in_place_type_construct.test | RUN | 0.195425 | 0.489836 | 0.537464 | 0.001758 | 0.001802 | 0.001745 |
| variant_in_place_type_construct_cx.test | COMPILE | 0.115354 | 0.228887 | 0.302647 | — | — | — |
| variant_copy_assign.test | RUN | 0.232302 | 0.819587 | 0.845179 | 0.001631 | 0.001865 | 0.001868 |
| variant_copy_assign_cx.test | COMPILE | 0.107554 | 0.187664 | 0.235393 | — | — | — |
| variant_move_assign.test | RUN | 0.225196 | 0.786748 | 0.839453 | 0.001775 | 0.001990 | 0.001666 |
| variant_move_assign_cx.test | COMPILE | 0.106933 | 0.186767 | 0.267985 | — | — | — |
| variant_value_assign.test | RUN | 0.199385 | 0.552892 | 0.630878 | 0.001743 | 0.001828 | 0.001680 |
| variant_value_assign_cx.test | COMPILE | 0.106668 | 0.192478 | 0.250728 | — | — | — |
| variant_emplace_index.test | RUN | 0.212228 | 0.640206 | 0.697299 | 0.001636 | 0.001850 | 0.001800 |
| variant_emplace_index_cx.test | COMPILE | 0.103979 | 0.185140 | 0.255956 | — | — | — |
| variant_emplace_type.test | RUN | 0.194907 | 0.573124 | 0.632896 | 0.001804 | 0.001520 | 0.001882 |
| variant_emplace_type_cx.test | COMPILE | 0.101737 | 0.221308 | 0.255653 | — | — | — |
| variant_swap.test | RUN | 0.196286 | 0.621345 | 0.638629 | 0.001801 | 0.001822 | 0.001588 |
| variant_eq_ne.test | RUN | 0.178808 | 0.413148 | 0.513314 | 0.001689 | 0.001874 | 0.002082 |
| variant_eq_ne_cx.test | COMPILE | 0.101652 | 0.186012 | 0.247420 | — | — | — |
| variant_lt_gt.test | RUN | 0.176839 | 0.433621 | 0.530807 | 0.001870 | 0.001865 | 0.002115 |
| variant_lt_gt_cx.test | COMPILE | 0.100668 | 0.198818 | 0.246780 | — | — | — |
| variant_destroy.test | RUN | 0.172253 | 0.459100 | 0.544977 | 0.001677 | 0.001923 | 0.001874 |
| variant_visit.test | RUN | 0.714367 | 5.775270 | 5.080263 | 0.001688 | 0.001808 | 0.001824 |
| variant_convert_construct.test | RUN | 0.209697 | 0.695851 | 0.754405 | 0.001775 | 0.001928 | 0.001866 |
| variant_subset.test | RUN | 0.212798 | 0.756218 | 0.779456 | 0.001858 | 0.001800 | 0.001746 |
| variant_valueless.test | RUN | 0.166992 | 0.501108 | 0.550151 | 0.001780 | 0.001684 | 0.002082 |
| variant_copy_construct_throw.test | RUN | 0.142408 | 0.286617 | 0.369372 | 0.001735 | 0.001962 | 0.001826 |
| variant_move_construct_throw.test | RUN | 0.140172 | 0.293662 | 0.384656 | 0.001006 | 0.001622 | 0.001733 |
| variant_convert_construct_throw.test | RUN | 0.145606 | 0.322546 | 0.407186 | 0.001828 | 0.001549 | 0.001686 |
| variant_copy_assign_throw.test | RUN | 0.138327 | 0.303229 | 0.387130 | 0.001796 | 0.001892 | 0.001766 |
| variant_move_assign_throw.test | RUN | 0.136698 | 0.291953 | 0.382489 | 0.001735 | 0.001780 | 0.001890 |
| variant_get_by_index_nx.test | RUN | 0.224767 | 0.767155 | 0.796020 | 0.001493 | 0.002026 | 0.001859 |
| variant_get_by_index_cx_nx.test | COMPILE | 0.111568 | 0.214440 | 0.288353 | — | — | — |
| variant_get_by_type_nx.test | RUN | 0.226805 | 0.851352 | 0.843628 | 0.001555 | 0.001750 | 0.001808 |
| variant_get_by_type_cx_nx.test | COMPILE | 0.100994 | 0.161363 | 0.266458 | — | — | — |
| variant_subset_nx.test | RUN | 0.229975 | 0.761753 | 0.773652 | 0.001879 | 0.001909 | 0.001742 |
| variant_hash.test | RUN | 0.238446 | 0.670152 | 0.735517 | 0.001686 | 0.001892 | 0.001869 |
| variant_trivial.test | RUN | 1.259314 | 4.320761 | 5.511126 | 0.001753 | 0.001855 | 0.001523 |
| variant_special.test | RUN | 1.261464 | 4.381594 | 5.424045 | 0.001798 | 0.001602 | 0.001944 |
| variant_visit_derived.test | RUN | 0.158438 | 0.414765 | 0.560626 | 0.001850 | 0.001848 | 0.001736 |
| variant_many_types.test | RUN | 1.061991 | 12.758009 | 7.997104 | 0.002090 | 0.002311 | 0.001920 |
| variant_visit_r.test | RUN | 0.454211 | 2.863821 | 2.518110 | 0.001708 | 0.002042 | 0.001764 |
| variant_derived_construct.test | COMPILE | 0.108798 | 0.189788 | 0.267428 | — | — | — |
| variant_visit_by_index.test | RUN | 0.215822 | 0.639052 | 0.719915 | 0.001527 | 0.002034 | 0.001644 |
| variant_ostream_insert.test | RUN | 0.158200 | 0.375923 | 0.427702 | 0.001837 | 0.001762 | 0.001599 |
| is_output_streamable.test | RUN | 0.189070 | 0.536626 | 0.540744 | 0.001852 | 0.001860 | 0.001878 |
| variant_json_value_from.test | RUN | 0.312712 | 0.688384 | 0.833223 | 0.001584 | 0.001894 | 0.001470 |
| variant_json_value_to.test | RUN | 0.338474 | 0.960385 | 1.065134 | 0.001640 | 0.001886 | 0.001912 |
| variant_uses_double_storage.test | COMPILE | 0.113670 | 0.215767 | 0.263531 | — | — | — |
| variant_derived_construct2.test | RUN | 0.148206 | 0.318319 | 0.374173 | 0.001670 | 0.001950 | 0.001713 |
| variant_visit_cx.test | COMPILE | 0.107692 | 0.194513 | 0.237673 | — | — | — |
| variant_visit_cx_2.test | COMPILE | 0.105248 | 0.200926 | 0.230855 | — | — | — |
| variant_visit_r_cx.test | COMPILE | 0.101143 | 0.191071 | 0.244228 | — | — | — |
| variant_index_type_126.test | RUN | 0.885871 | 1.991291 | 3.471999 | 0.001732 | 0.002036 | 0.001720 |
| variant_index_type_127.test | RUN | 0.897499 | 2.205676 | 3.491596 | 0.001798 | 0.001885 | 0.001700 |
| variant_index_type_128.test | RUN | 0.905015 | 2.068245 | 3.536944 | 0.001510 | 0.002045 | 0.001541 |
| variant_index_type_129.test | RUN | 0.911136 | 2.066303 | 3.558997 | 0.001768 | 0.001463 | 0.001765 |
| variant_index_type_254.test | RUN | 3.127893 | 7.668450 | 11.371033 | 0.001848 | 0.001800 | 0.001589 |
| variant_index_type_255.test | RUN | 3.146018 | 7.498150 | 11.420397 | 0.001847 | 0.001558 | 0.001809 |
| variant_index_type_256.test | RUN | 3.177284 | 7.328150 | 11.365908 | 0.001754 | 0.001512 | 0.001702 |
| variant_index_type_257.test | RUN | 3.200260 | 6.325769 | 10.990919 | 0.001715 | 0.001385 | 0.001213 |
| variant_default_construct_cx_2.test | COMPILE | 0.101246 | 0.181626 | 0.235262 | — | — | — |
| variant_default_construct_cx_3.test | COMPILE | 0.102701 | 0.184698 | 0.238973 | — | — | — |
| variant_default_construct_cx_4.test | COMPILE | 0.102259 | 0.186338 | 0.236000 | — | — | — |
| variant_default_construct_cx_5.test | COMPILE | 0.107545 | 0.179669 | 0.244687 | — | — | — |
| variant_value_construct_cx_2.test | COMPILE | 0.102895 | 0.191955 | 0.248041 | — | — | — |
| variant_value_construct_cx_3.test | COMPILE | 0.101420 | 0.187559 | 0.238842 | — | — | — |
| variant_value_construct_cx_4.test | COMPILE | 0.099828 | 0.182966 | 0.191490 | — | — | — |
| variant_issue_55.test | RUN | 0.138741 | 0.369020 | 0.372580 | 0.001405 | 0.001450 | 0.001303 |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114108) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114108) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114108).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
