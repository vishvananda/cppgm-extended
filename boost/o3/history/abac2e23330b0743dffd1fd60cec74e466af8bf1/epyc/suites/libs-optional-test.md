# libs/optional/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| optional_test.test | RUN | 0.388654 | 2.113332 | 1.457028 | 0.001994 | 0.002472 | 0.002388 |
| optional_test_assign.test | RUN | 0.200286 | 0.452878 | 0.691234 | 0.002141 | 0.002272 | 0.002386 |
| optional_test_swap.test | RUN | 0.222473 | 0.558678 | 0.776393 | 0.001818 | 0.002299 | 0.002178 |
| optional_test_constexpr.test | COMPILE | 0.147352 | 0.305218 | 0.462996 | — | — | — |
| optional_test_wuninitialized.test | COMPILE | 0.227283 | 0.751005 | 0.703172 | — | — | — |
| optional_test_fwd_header.test | COMPILE | 0.149040 | 0.314179 | 0.436652 | — | — | — |
| optional_test_conversions_from_U.test | RUN | 0.239704 | 0.586591 | 0.812036 | 0.002021 | 0.002358 | 0.002125 |
| optional_test_constructors.test | RUN | 0.533526 | 2.142422 | 2.102964 | 0.002006 | 0.002168 | 0.002134 |
| optional_test_convert_from_T.test | RUN | 0.225347 | 0.478981 | 0.735751 | 0.001990 | 0.002338 | 0.002117 |
| optional_test_convert_assign.test | RUN | 0.208768 | 0.456737 | 0.671441 | 0.001987 | 0.002124 | 0.002271 |
| optional_test_empty_braces.test | RUN | 0.210891 | 0.473712 | 0.746140 | 0.002154 | 0.002280 | 0.002226 |
| optional_test_make_optional.test | RUN | 0.289010 | 0.707466 | 1.061817 | 0.002042 | 0.002043 | 0.002184 |
| optional_test_flat_map.test | RUN | 0.310744 | 0.802218 | 1.210356 | 0.001994 | 0.001870 | 0.002304 |
| optional_test_hash.test | RUN | 0.262542 | 0.664276 | 0.912612 | 0.001888 | 0.002136 | 0.002244 |
| optional_test_map.test | RUN | 0.312179 | 0.807682 | 1.243330 | 0.001864 | 0.002304 | 0.002038 |
| optional_test_tie.test | RUN | 0.229391 | 0.538739 | 0.740894 | 0.002124 | 0.001865 | 0.002258 |
| optional_test_ranges_find.test | RUN | 0.200120 | 0.447926 | 0.652168 | 0.001893 | 0.002106 | 0.002246 |
| optional_test_ref_assign_portable_minimum.test | RUN | 0.324727 | 1.290609 | 0.925149 | 0.002355 | 0.001874 | 0.002334 |
| optional_test_ref_assign_mutable_int.test | RUN | 0.222370 | 0.571612 | 0.734588 | 0.002151 | 0.002076 | 0.002118 |
| optional_test_ref_assign_const_int.test | RUN | 0.238760 | 0.587183 | 0.742569 | 0.002162 | 0.001917 | 0.002160 |
| optional_test_ref_converting_ctor.test | RUN | 0.292386 | 0.788753 | 0.821939 | 0.002050 | 0.002368 | 0.002297 |
| optional_test_ref_convert_assign_non_int.test | RUN | 0.228057 | 0.540687 | 0.788306 | 0.002229 | 0.001858 | 0.001852 |
| optional_test_ref_convert_assign_mutable_int.test | RUN | 0.211314 | 0.515448 | 0.658854 | 0.001989 | 0.002113 | 0.002232 |
| optional_test_ref_convert_assign_const_int.test | RUN | 0.199998 | 0.471296 | 0.682796 | 0.002038 | 0.002125 | 0.002338 |
| optional_test_ref_portable_minimum.test | RUN | 0.757889 | 2.585352 | 2.481647 | 0.002193 | 0.002210 | 0.001988 |
| optional_test_ref_move.test | RUN | 0.224730 | 0.511121 | 0.748831 | 0.002136 | 0.002210 | 0.001944 |
| optional_test_ref_to_val.test | RUN | 0.309619 | 1.041371 | 1.016451 | 0.001980 | 0.001924 | 0.002096 |
| optional_test_inplace_factory.test | RUN | 0.273579 | 0.935100 | 0.937597 | 0.002162 | 0.002109 | 0.002141 |
| optional_test_io.test | RUN | 0.258442 | 1.007586 | 0.878181 | 0.002080 | 0.002157 | 0.002024 |
| optional_test_move.test | RUN | 0.251888 | 0.700881 | 0.787565 | 0.001854 | 0.002268 | 0.001474 |
| optional_test_noexcept_move.test | RUN | 0.155005 | 0.328006 | 0.419869 | 0.000919 | 0.000939 | 0.001965 |
| optional_test_cmp_none.test | RUN | 0.215053 | 0.494366 | 0.696356 | 0.002070 | 0.002364 | 0.002200 |
| optional_test_value_access.test | RUN | 0.252814 | 0.823178 | 0.894705 | 0.001951 | 0.002206 | 0.002064 |
| optional_test_emplace.test | RUN | 0.248786 | 0.810515 | 0.964081 | 0.001906 | 0.001924 | 0.001924 |
| optional_test_minimum_requirements.test | RUN | 0.230116 | 0.652939 | 0.837623 | 0.001965 | 0.002171 | 0.002192 |
| optional_test_msvc_bug_workaround.test | RUN | 0.208436 | 0.458727 | 0.671317 | 0.002134 | 0.002070 | 0.002189 |
| optional_test_member_T.test | RUN | 0.207923 | 0.452153 | 0.701156 | 0.002114 | 0.001927 | 0.002378 |
| optional_test_tc_base.test | RUN | 0.214408 | 0.469692 | 0.707160 | 0.001944 | 0.002174 | 0.002072 |
| optional_test_sfinae_friendly_ctor.test | COMPILE | 0.159309 | 0.329578 | 0.439413 | — | — | — |
| optional_test_path_assignment.test | COMPILE | 0.156756 | 0.325696 | 0.412616 | — | — | — |
| optional_test_fail_const_swap.test | COMPILE_FAIL | 0.145150 | 0.286248 | 0.390242 | — | — | — |
| optional_test_ref_convert_assign_const_int_prevented.test | COMPILE_FAIL | 0.191970 | 0.406725 | 0.648384 | — | — | — |
| optional_test_fail1.test | COMPILE_FAIL | 0.139553 | 0.283854 | 0.414567 | — | — | — |
| optional_test_fail3a.test | COMPILE_FAIL | 0.138566 | 0.298408 | 0.439790 | — | — | — |
| optional_test_fail3b.test | COMPILE_FAIL | 0.139389 | 0.300306 | 0.439271 | — | — | — |
| optional_test_ref_fail1.test | COMPILE_FAIL | 0.139164 | 0.295437 | 0.422807 | — | — | — |
| optional_test_ref_fail3.test | COMPILE_FAIL | 0.138799 | 0.286581 | 0.406816 | — | — | — |
| optional_test_ref_fail4.test | COMPILE_FAIL | 0.140415 | 0.289795 | 0.424575 | — | — | — |
| optional_test_ref_fail_convert_from_temporary_optional_T.test | COMPILE_FAIL | 0.142252 | 0.294806 | 0.411252 | — | — | — |
| optional_test_inplace_fail.test | COMPILE_FAIL | 0.231095 | 0.461223 | 0.719288 | — | — | — |
| optional_test_inplace_fail2.test | COMPILE_FAIL | 0.232000 | 0.454798 | 0.698114 | — | — | — |
| optional_test_fail_implicit_bool_convert.test | COMPILE_FAIL | 0.138501 | 0.299211 | 0.429649 | — | — | — |
| optional_test_fail_copying_a_moveable_type.test | COMPILE_FAIL | 0.139467 | 0.299646 | 0.436012 | — | — | — |
| optional_test_fail_optional_rvalue_ref.test | COMPILE_FAIL | 0.137649 | 0.284343 | 0.405837 | — | — | — |
| optional_test_ref_fail_init_from_Trefref.test | COMPILE_FAIL | 0.133145 | 0.283822 | 0.412160 | — | — | — |
| optional_test_ref_fail_init_from_Urefref.test | COMPILE_FAIL | 0.132727 | 0.299352 | 0.419761 | — | — | — |
| optional_test_ref_fail_assign_from_Trefref.test | COMPILE_FAIL | 0.138323 | 0.292238 | 0.419070 | — | — | — |
| optional_test_ref_fail_assign_from_Urefref.test | COMPILE_FAIL | 0.140137 | 0.284908 | 0.420320 | — | — | — |
| optional_test_fail_convert_from_null.test | COMPILE_FAIL | 0.135060 | 0.283670 | 0.405604 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or.test | COMPILE_FAIL | 0.135741 | 0.298416 | 0.416354 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or_call.test | COMPILE_FAIL | 0.138423 | 0.293450 | 0.419240 | — | — | — |
| optional_test_fail_io_without_io.test | COMPILE_FAIL | 0.187012 | 0.380553 | 0.621625 | — | — | — |
| optional_test_fail_none_io_without_io.test | COMPILE_FAIL | 0.146939 | 0.322842 | 0.502611 | — | — | — |
| optional_test_fail_convert_assign_of_enums.test | COMPILE_FAIL | 0.140815 | 0.284711 | 0.434147 | — | — | — |
| optional_test_static_properties.test | RUN | 0.328709 | 0.658483 | 1.101132 | 0.002060 | 0.002134 | 0.002206 |
| optional_test_maybe_uninitialized_warning.test | COMPILE | 0.147927 | 0.311555 | 0.441641 | — | — | — |
| optional_test_deleted_default_ctor.test | COMPILE | 0.153455 | 0.322106 | 0.480543 | — | — | — |
| optional_test_constructible_from_other.test | COMPILE | 0.147067 | 0.303846 | 0.432317 | — | — | — |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | RUN | 0.170552 | 0.384171 | 0.562474 | 0.001962 | 0.002308 | 0.002033 |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_fail.test | RUN_FAIL | 0.171191 | 0.390154 | 0.581699 | 0.001657 | 0.001889 | 0.002158 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | RUN | 0.171567 | 0.390570 | 0.588011 | 0.001844 | 0.001928 | 0.002026 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_fail.test | RUN_FAIL | 0.175197 | 0.398192 | 0.525455 | 0.001668 | 0.002358 | 0.002436 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | RUN | 0.118957 | 0.240024 | 0.398079 | 0.001724 | 0.001635 | 0.001522 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_fail.test | COMPILE_FAIL | 0.088160 | 0.237665 | 0.354698 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.003× | 1.002×, 1.003× | 0.993× |
| clang | 1.010× | 1.016×, 1.004× | 0.997× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| optional_test.test | 10 | 0.001419s | 0.001429s | 0.001408s | byte equality observed |
| optional_test_assign.test | 10 | 0.001282s | 0.001326s | 0.001294s | byte equality observed |
| optional_test_cmp_none.test | 10 | 0.001320s | 0.001333s | 0.001309s | byte equality observed |
| optional_test_constructors.test | 10 | 0.001318s | 0.001308s | 0.001307s | byte equality observed |
| optional_test_conversions_from_U.test | 10 | 0.001337s | 0.001320s | 0.001303s | byte equality observed |
| optional_test_convert_assign.test | 10 | 0.001280s | 0.001337s | 0.001327s | byte equality observed |
| optional_test_convert_from_T.test | 10 | 0.001352s | 0.001324s | 0.001273s | byte equality observed |
| optional_test_emplace.test | 10 | 0.001339s | 0.001311s | 0.001313s | byte equality observed |
| optional_test_empty_braces.test | 10 | 0.001294s | 0.001339s | 0.001348s | byte equality observed |
| optional_test_flat_map.test | 10 | 0.001295s | 0.001281s | 0.001278s | byte equality observed |
| optional_test_hash.test | 10 | 0.001283s | 0.001331s | 0.001325s | byte equality observed |
| optional_test_inplace_factory.test | 10 | 0.001298s | 0.001386s | 0.001314s | byte equality observed |
| optional_test_io.test | 10 | 0.001316s | 0.001329s | 0.001370s | byte equality observed |
| optional_test_make_optional.test | 10 | 0.001377s | 0.001373s | 0.001343s | byte equality observed |
| optional_test_map.test | 10 | 0.001355s | 0.001339s | 0.001344s | byte equality observed |
| optional_test_member_T.test | 10 | 0.001303s | 0.001328s | 0.001271s | byte equality observed |
| optional_test_minimum_requirements.test | 10 | 0.001353s | 0.001317s | 0.001316s | byte equality observed |
| optional_test_move.test | 10 | 0.001295s | 0.001289s | 0.001314s | byte equality observed |
| optional_test_msvc_bug_workaround.test | 10 | 0.001325s | 0.001327s | 0.001294s | byte equality observed |
| optional_test_noexcept_move.test | 10 | 0.000598s | 0.000607s | 0.001272s | byte equality observed |
| optional_test_ranges_find.test | 10 | 0.001278s | 0.001275s | 0.001270s | byte equality observed |
| optional_test_ref_assign_const_int.test | 10 | 0.001315s | 0.001315s | 0.001336s | byte equality observed |
| optional_test_ref_assign_mutable_int.test | 7 | 0.001283s | 0.001395s | 0.001280s | byte equality observed |
| optional_test_ref_assign_portable_minimum.test | 8 | 0.001331s | 0.001299s | 0.001283s | byte equality observed |
| optional_test_ref_convert_assign_const_int.test | 10 | 0.001317s | 0.001317s | 0.001317s | byte equality observed |
| optional_test_ref_convert_assign_mutable_int.test | 10 | 0.001315s | 0.001326s | 0.001365s | byte equality observed |
| optional_test_ref_convert_assign_non_int.test | 10 | 0.001280s | 0.001325s | 0.001267s | byte equality observed |
| optional_test_ref_converting_ctor.test | 10 | 0.001317s | 0.001296s | 0.001311s | byte equality observed |
| optional_test_ref_move.test | 10 | 0.001339s | 0.001336s | 0.001307s | byte equality observed |
| optional_test_ref_portable_minimum.test | 10 | 0.001351s | 0.001370s | 0.001375s | byte equality observed |
| optional_test_ref_to_val.test | 10 | 0.001314s | 0.001377s | 0.001266s | byte equality observed |
| optional_test_static_properties.test | 10 | 0.001334s | 0.001322s | 0.001281s | byte equality observed |
| optional_test_swap.test | 10 | 0.001315s | 0.001349s | 0.001325s | byte equality observed |
| optional_test_tc_base.test | 10 | 0.001300s | 0.001309s | 0.001320s | byte equality observed |
| optional_test_tie.test | 10 | 0.001336s | 0.001394s | 0.001338s | byte equality observed |
| optional_test_value_access.test | 10 | 0.001372s | 0.001343s | 0.001314s | byte equality observed |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | 10 | 0.001261s | 0.001251s | 0.001309s | byte equality observed |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | 10 | 0.001273s | 0.001263s | 0.001310s | byte equality observed |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | 10 | 0.001265s | 0.001258s | 0.001275s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
