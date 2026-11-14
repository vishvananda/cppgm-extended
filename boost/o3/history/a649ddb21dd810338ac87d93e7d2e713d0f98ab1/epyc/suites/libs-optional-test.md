# libs/optional/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| optional_test.test | RUN | 0.323174 | 2.044073 | 1.394697 | 0.002094 | 0.002337 | 0.002181 |
| optional_test_assign.test | RUN | 0.173633 | 0.450420 | 0.691713 | 0.001947 | 0.002262 | 0.002023 |
| optional_test_swap.test | RUN | 0.184070 | 0.547101 | 0.726852 | 0.001729 | 0.002046 | 0.001832 |
| optional_test_constexpr.test | COMPILE | 0.122794 | 0.303318 | 0.451818 | — | — | — |
| optional_test_wuninitialized.test | COMPILE | 0.184192 | 0.730739 | 0.691818 | — | — | — |
| optional_test_fwd_header.test | COMPILE | 0.125192 | 0.307786 | 0.428844 | — | — | — |
| optional_test_conversions_from_U.test | RUN | 0.204767 | 0.570905 | 0.778646 | 0.002011 | 0.002032 | 0.002000 |
| optional_test_constructors.test | RUN | 0.407776 | 2.072298 | 2.023076 | 0.001897 | 0.001940 | 0.002038 |
| optional_test_convert_from_T.test | RUN | 0.187832 | 0.483711 | 0.751504 | 0.001818 | 0.002160 | 0.002150 |
| optional_test_convert_assign.test | RUN | 0.184644 | 0.445278 | 0.666880 | 0.001926 | 0.002130 | 0.001882 |
| optional_test_empty_braces.test | RUN | 0.177029 | 0.451037 | 0.664596 | 0.001964 | 0.002072 | 0.002042 |
| optional_test_make_optional.test | RUN | 0.250382 | 0.670964 | 1.081074 | 0.001838 | 0.001818 | 0.001972 |
| optional_test_flat_map.test | RUN | 0.267228 | 0.807535 | 1.144390 | 0.001899 | 0.001850 | 0.001864 |
| optional_test_hash.test | RUN | 0.224173 | 0.667092 | 0.840030 | 0.001878 | 0.001947 | 0.001956 |
| optional_test_map.test | RUN | 0.267897 | 0.771856 | 1.210876 | 0.001965 | 0.001914 | 0.001562 |
| optional_test_tie.test | RUN | 0.192236 | 0.523074 | 0.725829 | 0.002024 | 0.001872 | 0.001970 |
| optional_test_ranges_find.test | RUN | 0.178520 | 0.428371 | 0.674619 | 0.001784 | 0.002142 | 0.001688 |
| optional_test_ref_assign_portable_minimum.test | RUN | 0.215581 | 1.259324 | 0.938292 | 0.002042 | 0.001823 | 0.002062 |
| optional_test_ref_assign_mutable_int.test | RUN | 0.179739 | 0.544549 | 0.757071 | 0.001886 | 0.001966 | 0.001880 |
| optional_test_ref_assign_const_int.test | RUN | 0.195927 | 0.550123 | 0.763570 | 0.002116 | 0.002007 | 0.001720 |
| optional_test_ref_converting_ctor.test | RUN | 0.198706 | 0.777241 | 0.814182 | 0.001832 | 0.001942 | 0.002172 |
| optional_test_ref_convert_assign_non_int.test | RUN | 0.182056 | 0.517887 | 0.733157 | 0.001830 | 0.001860 | 0.002046 |
| optional_test_ref_convert_assign_mutable_int.test | RUN | 0.184178 | 0.471627 | 0.683061 | 0.001873 | 0.001862 | 0.002068 |
| optional_test_ref_convert_assign_const_int.test | RUN | 0.182349 | 0.459424 | 0.631208 | 0.001801 | 0.001941 | 0.001749 |
| optional_test_ref_portable_minimum.test | RUN | 0.803152 | 2.505150 | 2.407288 | 0.001864 | 0.001860 | 0.001829 |
| optional_test_ref_move.test | RUN | 0.193128 | 0.483789 | 0.689138 | 0.002040 | 0.002068 | 0.002036 |
| optional_test_ref_to_val.test | RUN | 0.233917 | 0.990667 | 0.970128 | 0.001861 | 0.002174 | 0.001787 |
| optional_test_inplace_factory.test | RUN | 0.223747 | 0.916524 | 0.933741 | 0.001684 | 0.001764 | 0.001958 |
| optional_test_io.test | RUN | 0.214806 | 0.981416 | 0.857174 | 0.001944 | 0.001954 | 0.002122 |
| optional_test_move.test | RUN | 0.208644 | 0.681685 | 0.758267 | 0.001960 | 0.002082 | 0.001928 |
| optional_test_noexcept_move.test | RUN | 0.132627 | 0.307235 | 0.391406 | 0.000690 | 0.000791 | 0.002039 |
| optional_test_cmp_none.test | RUN | 0.182657 | 0.492035 | 0.678500 | 0.001986 | 0.002242 | 0.001950 |
| optional_test_value_access.test | RUN | 0.213572 | 0.808254 | 0.869880 | 0.001956 | 0.002248 | 0.002182 |
| optional_test_emplace.test | RUN | 0.214670 | 0.775804 | 0.878629 | 0.002136 | 0.002147 | 0.001908 |
| optional_test_minimum_requirements.test | RUN | 0.197216 | 0.633952 | 0.792714 | 0.001934 | 0.002114 | 0.002004 |
| optional_test_msvc_bug_workaround.test | RUN | 0.172002 | 0.441494 | 0.659159 | 0.002122 | 0.001917 | 0.002047 |
| optional_test_member_T.test | RUN | 0.176652 | 0.434556 | 0.671606 | 0.001756 | 0.002074 | 0.001644 |
| optional_test_tc_base.test | RUN | 0.180698 | 0.448390 | 0.628857 | 0.001737 | 0.002068 | 0.001687 |
| optional_test_sfinae_friendly_ctor.test | COMPILE | 0.127360 | 0.303965 | 0.467480 | — | — | — |
| optional_test_path_assignment.test | COMPILE | 0.129914 | 0.306154 | 0.415961 | — | — | — |
| optional_test_fail_const_swap.test | COMPILE_FAIL | 0.113085 | 0.272894 | 0.396019 | — | — | — |
| optional_test_ref_convert_assign_const_int_prevented.test | COMPILE_FAIL | 0.164334 | 0.390773 | 0.633857 | — | — | — |
| optional_test_fail1.test | COMPILE_FAIL | 0.115523 | 0.283955 | 0.410216 | — | — | — |
| optional_test_fail3a.test | COMPILE_FAIL | 0.116506 | 0.301078 | 0.425579 | — | — | — |
| optional_test_fail3b.test | COMPILE_FAIL | 0.122909 | 0.290294 | 0.415230 | — | — | — |
| optional_test_ref_fail1.test | COMPILE_FAIL | 0.121211 | 0.277180 | 0.420508 | — | — | — |
| optional_test_ref_fail3.test | COMPILE_FAIL | 0.114688 | 0.276800 | 0.418025 | — | — | — |
| optional_test_ref_fail4.test | COMPILE_FAIL | 0.115473 | 0.275288 | 0.397013 | — | — | — |
| optional_test_ref_fail_convert_from_temporary_optional_T.test | COMPILE_FAIL | 0.120897 | 0.288453 | 0.408787 | — | — | — |
| optional_test_inplace_fail.test | COMPILE_FAIL | 0.199327 | 0.438228 | 0.705748 | — | — | — |
| optional_test_inplace_fail2.test | COMPILE_FAIL | 0.195832 | 0.442951 | 0.707130 | — | — | — |
| optional_test_fail_implicit_bool_convert.test | COMPILE_FAIL | 0.115675 | 0.274902 | 0.405053 | — | — | — |
| optional_test_fail_copying_a_moveable_type.test | COMPILE_FAIL | 0.120335 | 0.290704 | 0.407378 | — | — | — |
| optional_test_fail_optional_rvalue_ref.test | COMPILE_FAIL | 0.119185 | 0.275398 | 0.398990 | — | — | — |
| optional_test_ref_fail_init_from_Trefref.test | COMPILE_FAIL | 0.122602 | 0.278675 | 0.419282 | — | — | — |
| optional_test_ref_fail_init_from_Urefref.test | COMPILE_FAIL | 0.119723 | 0.274783 | 0.424266 | — | — | — |
| optional_test_ref_fail_assign_from_Trefref.test | COMPILE_FAIL | 0.121131 | 0.283680 | 0.399221 | — | — | — |
| optional_test_ref_fail_assign_from_Urefref.test | COMPILE_FAIL | 0.120988 | 0.279969 | 0.407412 | — | — | — |
| optional_test_fail_convert_from_null.test | COMPILE_FAIL | 0.119606 | 0.281503 | 0.416258 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or.test | COMPILE_FAIL | 0.118157 | 0.275769 | 0.419012 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or_call.test | COMPILE_FAIL | 0.115953 | 0.286751 | 0.396705 | — | — | — |
| optional_test_fail_io_without_io.test | COMPILE_FAIL | 0.161918 | 0.363895 | 0.596067 | — | — | — |
| optional_test_fail_none_io_without_io.test | COMPILE_FAIL | 0.131614 | 0.300905 | 0.492937 | — | — | — |
| optional_test_fail_convert_assign_of_enums.test | COMPILE_FAIL | 0.119734 | 0.287090 | 0.433701 | — | — | — |
| optional_test_static_properties.test | RUN | 0.282543 | 0.643877 | 1.082972 | 0.002003 | 0.001933 | 0.002032 |
| optional_test_maybe_uninitialized_warning.test | COMPILE | 0.126274 | 0.311833 | 0.450695 | — | — | — |
| optional_test_deleted_default_ctor.test | COMPILE | 0.123741 | 0.307578 | 0.447352 | — | — | — |
| optional_test_constructible_from_other.test | COMPILE | 0.119181 | 0.293790 | 0.431859 | — | — | — |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | RUN | 0.142960 | 0.366866 | 0.582160 | 0.002008 | 0.002084 | 0.001693 |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_fail.test | RUN_FAIL | 0.142011 | 0.370462 | 0.536304 | 0.001782 | 0.001604 | 0.001711 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | RUN | 0.143951 | 0.364442 | 0.581850 | 0.001622 | 0.001664 | 0.001596 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_fail.test | RUN_FAIL | 0.144022 | 0.384734 | 0.515469 | 0.001459 | 0.002268 | 0.002018 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | RUN | 0.117134 | 0.221983 | 0.359022 | 0.001384 | 0.001579 | 0.001502 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_fail.test | COMPILE_FAIL | 0.086465 | 0.251400 | 0.335849 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.003× | 1.005×, 1.000× | 1.025× |
| clang | 1.009× | 1.007×, 1.011× | 1.005× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274616).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| optional_test.test | 10 | 0.001308s | 0.001380s | 0.001328s | byte equality observed |
| optional_test_assign.test | 10 | 0.001244s | 0.001237s | 0.001302s | byte equality observed |
| optional_test_cmp_none.test | 10 | 0.001225s | 0.001270s | 0.001244s | byte equality observed |
| optional_test_constructors.test | 10 | 0.001281s | 0.001316s | 0.001249s | byte equality observed |
| optional_test_conversions_from_U.test | 10 | 0.001261s | 0.001235s | 0.001271s | byte equality observed |
| optional_test_convert_assign.test | 10 | 0.001256s | 0.001232s | 0.001205s | byte equality observed |
| optional_test_convert_from_T.test | 10 | 0.001230s | 0.001295s | 0.001207s | byte equality observed |
| optional_test_emplace.test | 10 | 0.001258s | 0.001298s | 0.001268s | byte equality observed |
| optional_test_empty_braces.test | 10 | 0.001305s | 0.001343s | 0.001300s | byte equality observed |
| optional_test_flat_map.test | 10 | 0.001254s | 0.001257s | 0.001245s | byte equality observed |
| optional_test_hash.test | 10 | 0.001278s | 0.001291s | 0.001230s | byte equality observed |
| optional_test_inplace_factory.test | 10 | 0.001278s | 0.001289s | 0.001289s | byte equality observed |
| optional_test_io.test | 10 | 0.001276s | 0.001286s | 0.001265s | byte equality observed |
| optional_test_make_optional.test | 10 | 0.001242s | 0.001300s | 0.001224s | byte equality observed |
| optional_test_map.test | 10 | 0.001255s | 0.001239s | 0.001230s | byte equality observed |
| optional_test_member_T.test | 10 | 0.001246s | 0.001238s | 0.001189s | byte equality observed |
| optional_test_minimum_requirements.test | 10 | 0.001313s | 0.001316s | 0.001310s | byte equality observed |
| optional_test_move.test | 10 | 0.001250s | 0.001250s | 0.001232s | byte equality observed |
| optional_test_msvc_bug_workaround.test | 10 | 0.001256s | 0.001258s | 0.001235s | byte equality observed |
| optional_test_noexcept_move.test | 10 | 0.000572s | 0.000582s | 0.001254s | byte equality observed |
| optional_test_ranges_find.test | 10 | 0.001240s | 0.001248s | 0.001243s | byte equality observed |
| optional_test_ref_assign_const_int.test | 10 | 0.001243s | 0.001266s | 0.001291s | byte equality observed |
| optional_test_ref_assign_mutable_int.test | 10 | 0.001257s | 0.001246s | 0.001226s | byte equality observed |
| optional_test_ref_assign_portable_minimum.test | 10 | 0.001271s | 0.001284s | 0.001227s | byte equality observed |
| optional_test_ref_convert_assign_const_int.test | 10 | 0.001299s | 0.001260s | 0.001267s | byte equality observed |
| optional_test_ref_convert_assign_mutable_int.test | 8 | 0.001275s | 0.001265s | 0.001203s | byte equality observed |
| optional_test_ref_convert_assign_non_int.test | 9 | 0.001261s | 0.001244s | 0.001253s | byte equality observed |
| optional_test_ref_converting_ctor.test | 10 | 0.001253s | 0.001236s | 0.001214s | byte equality observed |
| optional_test_ref_move.test | 10 | 0.001269s | 0.001226s | 0.001224s | byte equality observed |
| optional_test_ref_portable_minimum.test | 10 | 0.001274s | 0.001311s | 0.001267s | byte equality observed |
| optional_test_ref_to_val.test | 10 | 0.001261s | 0.001267s | 0.001302s | byte equality observed |
| optional_test_static_properties.test | 10 | 0.001262s | 0.001246s | 0.001301s | byte equality observed |
| optional_test_swap.test | 10 | 0.001261s | 0.001252s | 0.001296s | byte equality observed |
| optional_test_tc_base.test | 10 | 0.001242s | 0.001258s | 0.001247s | byte equality observed |
| optional_test_tie.test | 10 | 0.001249s | 0.001236s | 0.001224s | byte equality observed |
| optional_test_value_access.test | 10 | 0.001319s | 0.001287s | 0.001289s | byte equality observed |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | 10 | 0.001267s | 0.001231s | 0.001194s | byte equality observed |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | 10 | 0.001224s | 0.001269s | 0.001253s | byte equality observed |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | 10 | 0.001226s | 0.001235s | 0.001240s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274616).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
