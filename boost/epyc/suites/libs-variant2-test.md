# libs/variant2/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| quick.test | RUN | 0.134339 | 0.260002 | 0.349419 | 0.001554 | 0.001578 | 0.001808 |
| variant_size.test | RUN | 0.215508 | 0.485298 | 0.685893 | 0.001843 | 0.001931 | 0.001900 |
| variant_alternative.test | RUN | 0.244541 | 0.792935 | 0.901591 | 0.001637 | 0.001783 | 0.001765 |
| variant_holds_alternative.test | RUN | 0.194508 | 0.457359 | 0.692555 | 0.001826 | 0.001672 | 0.001715 |
| variant_holds_alternative_cx.test | COMPILE | 0.141124 | 0.266995 | 0.421022 | — | — | — |
| variant_get_by_index.test | RUN | 0.266287 | 0.969743 | 1.134970 | 0.001874 | 0.002024 | 0.001776 |
| variant_get_by_index_cx.test | COMPILE | 0.140892 | 0.230210 | 0.428131 | — | — | — |
| variant_get_by_type.test | RUN | 0.270438 | 1.027184 | 1.156474 | 0.002067 | 0.001690 | 0.001678 |
| variant_get_by_type_cx.test | COMPILE | 0.131390 | 0.233627 | 0.362165 | — | — | — |
| variant_default_construct.test | RUN | 0.262869 | 0.883561 | 1.070511 | 0.001844 | 0.001838 | 0.001941 |
| variant_default_construct_cx.test | COMPILE | 0.143465 | 0.269401 | 0.412909 | — | — | — |
| variant_copy_construct.test | RUN | 0.363096 | 1.841814 | 1.799646 | 0.001764 | 0.001846 | 0.001784 |
| variant_copy_construct_cx.test | COMPILE | 0.158618 | 0.308917 | 0.509734 | — | — | — |
| variant_move_construct.test | RUN | 0.357786 | 1.671067 | 1.680121 | 0.001587 | 0.001385 | 0.001759 |
| variant_move_construct_cx.test | COMPILE | 0.134953 | 0.237590 | 0.363598 | — | — | — |
| variant_value_construct.test | RUN | 0.244516 | 0.582319 | 0.823438 | 0.001681 | 0.001912 | 0.001860 |
| variant_value_construct_cx.test | COMPILE | 0.148495 | 0.288871 | 0.445467 | — | — | — |
| variant_in_place_index_construct.test | RUN | 0.239207 | 0.574488 | 0.806794 | 0.001731 | 0.002034 | 0.001893 |
| variant_in_place_index_construct_cx.test | COMPILE | 0.145952 | 0.267241 | 0.442420 | — | — | — |
| variant_in_place_type_construct.test | RUN | 0.241155 | 0.598685 | 0.772290 | 0.001676 | 0.001728 | 0.001791 |
| variant_in_place_type_construct_cx.test | COMPILE | 0.147742 | 0.270446 | 0.422775 | — | — | — |
| variant_copy_assign.test | RUN | 0.280073 | 0.968877 | 1.143301 | 0.001843 | 0.001832 | 0.002132 |
| variant_copy_assign_cx.test | COMPILE | 0.139292 | 0.226425 | 0.320022 | — | — | — |
| variant_move_assign.test | RUN | 0.283330 | 0.953508 | 1.148362 | 0.001787 | 0.001638 | 0.001966 |
| variant_move_assign_cx.test | COMPILE | 0.132767 | 0.236855 | 0.363403 | — | — | — |
| variant_value_assign.test | RUN | 0.240849 | 0.679007 | 0.918425 | 0.001816 | 0.001948 | 0.001790 |
| variant_value_assign_cx.test | COMPILE | 0.136535 | 0.223514 | 0.345984 | — | — | — |
| variant_emplace_index.test | RUN | 0.260671 | 0.741414 | 0.974445 | 0.001902 | 0.001873 | 0.001904 |
| variant_emplace_index_cx.test | COMPILE | 0.128685 | 0.236618 | 0.373414 | — | — | — |
| variant_emplace_type.test | RUN | 0.242523 | 0.647942 | 0.887256 | 0.001578 | 0.001657 | 0.001890 |
| variant_emplace_type_cx.test | COMPILE | 0.130245 | 0.225896 | 0.363941 | — | — | — |
| variant_swap.test | RUN | 0.247550 | 0.678539 | 0.887414 | 0.001928 | 0.001578 | 0.001629 |
| variant_eq_ne.test | RUN | 0.231529 | 0.499370 | 0.724379 | 0.001872 | 0.001795 | 0.001914 |
| variant_eq_ne_cx.test | COMPILE | 0.126433 | 0.220732 | 0.346100 | — | — | — |
| variant_lt_gt.test | RUN | 0.237050 | 0.519767 | 0.736112 | 0.002026 | 0.001947 | 0.001896 |
| variant_lt_gt_cx.test | COMPILE | 0.124610 | 0.227986 | 0.353423 | — | — | — |
| variant_destroy.test | RUN | 0.223479 | 0.545001 | 0.769686 | 0.001840 | 0.001950 | 0.001860 |
| variant_visit.test | RUN | 0.955335 | 6.856552 | 7.060723 | 0.001932 | 0.001805 | 0.001898 |
| variant_convert_construct.test | RUN | 0.268573 | 0.819663 | 1.016880 | 0.001890 | 0.002060 | 0.001885 |
| variant_subset.test | RUN | 0.291243 | 0.961801 | 1.063374 | 0.001978 | 0.001772 | 0.001954 |
| variant_valueless.test | RUN | 0.233213 | 0.586985 | 0.770384 | 0.002036 | 0.001807 | 0.001916 |
| variant_copy_construct_throw.test | RUN | 0.178463 | 0.361839 | 0.516609 | 0.001852 | 0.001898 | 0.001886 |
| variant_move_construct_throw.test | RUN | 0.185757 | 0.354973 | 0.515330 | 0.001808 | 0.001640 | 0.001958 |
| variant_convert_construct_throw.test | RUN | 0.183783 | 0.392968 | 0.590899 | 0.001927 | 0.001991 | 0.001766 |
| variant_copy_assign_throw.test | RUN | 0.186205 | 0.346103 | 0.508368 | 0.001894 | 0.001686 | 0.001974 |
| variant_move_assign_throw.test | RUN | 0.181478 | 0.346899 | 0.560725 | 0.001859 | 0.001760 | 0.002036 |
| variant_get_by_index_nx.test | RUN | 0.307408 | 0.941442 | 1.099041 | 0.001950 | 0.001872 | 0.001802 |
| variant_get_by_index_cx_nx.test | COMPILE | 0.153968 | 0.277035 | 0.432938 | — | — | — |
| variant_get_by_type_nx.test | RUN | 0.305957 | 0.923136 | 1.142389 | 0.001936 | 0.001876 | 0.001806 |
| variant_get_by_type_cx_nx.test | COMPILE | 0.135234 | 0.237621 | 0.323492 | — | — | — |
| variant_subset_nx.test | RUN | 0.308655 | 0.878397 | 1.095335 | 0.001973 | 0.001857 | 0.001710 |
| variant_hash.test | RUN | 0.305549 | 0.782261 | 1.049128 | 0.002031 | 0.001846 | 0.001628 |
| variant_trivial.test | RUN | 1.687922 | 4.728046 | 8.030027 | 0.001884 | 0.001902 | 0.001986 |
| variant_special.test | RUN | 1.579104 | 4.867435 | 8.068902 | 0.001752 | 0.001986 | 0.001992 |
| variant_visit_derived.test | RUN | 0.214054 | 0.517344 | 0.741813 | 0.001958 | 0.001890 | 0.001888 |
| variant_many_types.test | RUN | 1.337526 | 14.598306 | 11.725763 | 0.001950 | 0.002058 | 0.002252 |
| variant_visit_r.test | RUN | 0.509542 | 3.267787 | 3.579597 | 0.001966 | 0.001868 | 0.002238 |
| variant_derived_construct.test | COMPILE | 0.137374 | 0.233649 | 0.375860 | — | — | — |
| variant_visit_by_index.test | RUN | 0.263678 | 0.772816 | 1.023135 | 0.001750 | 0.001844 | 0.001686 |
| variant_ostream_insert.test | RUN | 0.193245 | 0.459256 | 0.648875 | 0.001878 | 0.001877 | 0.001988 |
| is_output_streamable.test | RUN | 0.231709 | 0.674859 | 0.824152 | 0.001758 | 0.002016 | 0.001868 |
| variant_json_value_from.test | RUN | 0.387357 | 0.891345 | 1.255059 | 0.001698 | 0.001664 | 0.002108 |
| variant_json_value_to.test | RUN | 0.406746 | 1.135780 | 1.557421 | 0.002023 | 0.001964 | 0.002018 |
| variant_uses_double_storage.test | COMPILE | 0.141245 | 0.242613 | 0.437402 | — | — | — |
| variant_derived_construct2.test | RUN | 0.184862 | 0.364209 | 0.602239 | 0.001541 | 0.001950 | 0.001769 |
| variant_visit_cx.test | COMPILE | 0.130705 | 0.228819 | 0.380742 | — | — | — |
| variant_visit_cx_2.test | COMPILE | 0.125905 | 0.225595 | 0.390216 | — | — | — |
| variant_visit_r_cx.test | COMPILE | 0.124192 | 0.224665 | 0.310443 | — | — | — |
| variant_index_type_126.test | RUN | 1.142953 | 2.421749 | 5.222443 | 0.001833 | 0.001895 | 0.001992 |
| variant_index_type_127.test | RUN | 1.109918 | 2.464090 | 5.454268 | 0.001445 | 0.001904 | 0.001706 |
| variant_index_type_128.test | RUN | 1.126005 | 2.717421 | 5.518432 | 0.001760 | 0.001898 | 0.001555 |
| variant_index_type_129.test | RUN | 1.137789 | 2.453874 | 5.438312 | 0.001770 | 0.001874 | 0.001971 |
| variant_index_type_254.test | RUN | 3.774758 | 9.096727 | 16.260226 | 0.001638 | 0.002086 | 0.001958 |
| variant_index_type_255.test | RUN | 3.808685 | 8.857646 | 16.562987 | 0.001767 | 0.001768 | 0.001826 |
| variant_index_type_256.test | RUN | 3.852797 | 9.141060 | 16.895525 | 0.001592 | 0.001460 | 0.001929 |
| variant_index_type_257.test | RUN | 3.877920 | 7.614861 | 15.972317 | 0.001758 | 0.001376 | 0.001328 |
| variant_default_construct_cx_2.test | COMPILE | 0.131778 | 0.249127 | 0.395418 | — | — | — |
| variant_default_construct_cx_3.test | COMPILE | 0.135034 | 0.219909 | 0.362975 | — | — | — |
| variant_default_construct_cx_4.test | COMPILE | 0.130907 | 0.225756 | 0.335278 | — | — | — |
| variant_default_construct_cx_5.test | COMPILE | 0.128560 | 0.222906 | 0.327367 | — | — | — |
| variant_value_construct_cx_2.test | COMPILE | 0.124079 | 0.199040 | 0.340696 | — | — | — |
| variant_value_construct_cx_3.test | COMPILE | 0.124529 | 0.230408 | 0.353151 | — | — | — |
| variant_value_construct_cx_4.test | COMPILE | 0.109848 | 0.235374 | 0.308989 | — | — | — |
| variant_issue_55.test | RUN | 0.178265 | 0.640682 | 0.563290 | 0.001326 | 0.002070 | 0.001562 |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42570601152) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42570601152) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42570601152).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
