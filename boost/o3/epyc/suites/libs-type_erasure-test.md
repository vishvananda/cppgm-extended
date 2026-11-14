# libs/type_erasure/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_binding.test | RUN | 0.429251 | 1.604650 | 1.313003 | 0.003188 | 0.003002 | 0.003285 |
| test_increment.test | RUN | 0.448015 | 1.203845 | 1.327042 | 0.002863 | 0.003000 | 0.003061 |
| test_add.test | RUN | 0.536397 | 1.880573 | 1.912563 | 0.003603 | 0.003339 | 0.003015 |
| test_add_assign.test | RUN | 0.545745 | 2.055570 | 1.982731 | 0.003300 | 0.003286 | 0.003231 |
| test_callable.test | RUN | 0.924025 | 3.657767 | 4.253999 | 0.005341 | 0.005290 | 0.005331 |
| test_reference.test | RUN | 0.430523 | 1.450747 | 1.192288 | 0.002966 | 0.003121 | 0.003026 |
| test_construct.test | RUN | 1.886919 | 20.112693 | 12.325735 | 0.006802 | 0.006971 | 0.006685 |
| test_relaxed.test | RUN | 0.417534 | 1.069025 | 1.120372 | 0.003055 | 0.002953 | 0.002922 |
| test_assign.test | RUN | 4.395923 | 61.241732 | 45.580028 | 0.010332 | 0.006278 | 0.006509 |
| test_construct_ref.test | RUN | 0.523530 | 2.537898 | 1.788775 | 0.004116 | 0.004125 | 0.004084 |
| test_construct_cref.test | RUN | 0.562344 | 2.412348 | 2.306957 | 0.004450 | 0.004727 | 0.004705 |
| test_any_cast.test | RUN | 0.473608 | 2.442782 | 1.718388 | 0.003999 | 0.003778 | 0.004000 |
| test_binding_of.test | RUN | 0.402691 | 0.973008 | 0.984204 | 0.003014 | 0.002820 | 0.002827 |
| test_typeid_of.test | RUN | 0.463997 | 1.282638 | 1.107547 | 0.003412 | 0.003087 | 0.003070 |
| test_nested.test | RUN | 0.505292 | 1.330161 | 1.312541 | 0.003208 | 0.003056 | 0.002780 |
| test_less.test | RUN | 0.612278 | 2.707896 | 2.153696 | 0.004105 | 0.004108 | 0.004128 |
| test_equal.test | RUN | 0.572517 | 2.484198 | 1.777149 | 0.003972 | 0.003936 | 0.003923 |
| test_negate.test | RUN | 0.398047 | 1.033369 | 1.052122 | 0.002778 | 0.002832 | 0.002724 |
| test_dereference.test | RUN | 0.445147 | 1.375751 | 1.351733 | 0.003303 | 0.003171 | 0.003052 |
| test_subscript.test | RUN | 0.463437 | 1.552474 | 1.391615 | 0.003317 | 0.003489 | 0.003098 |
| test_forward_iterator.test | RUN | 0.553950 | 1.784615 | 1.555717 | 0.003285 | 0.003242 | 0.003007 |
| test_tuple.test | RUN | 0.486004 | 1.793314 | 1.425191 | 0.003480 | 0.003444 | 0.003025 |
| test_stream.test | RUN | 0.631694 | 2.860322 | 2.285867 | 0.004185 | 0.004260 | 0.004583 |
| test_deduced.test | RUN | 0.423566 | 1.063445 | 0.973235 | 0.002964 | 0.003037 | 0.002991 |
| test_same_type.test | RUN | 0.512517 | 1.352280 | 1.327509 | 0.003348 | 0.003270 | 0.003263 |
| test_member.test | RUN | 0.641276 | 2.755731 | 1.983889 | 0.004601 | 0.004880 | 0.004520 |
| test_null.test | RUN | 0.452609 | 1.251932 | 1.160089 | 0.003134 | 0.003114 | 0.002761 |
| test_free.test | RUN | 0.527163 | 2.277644 | 1.669575 | 0.003635 | 0.003912 | 0.003478 |
| test_is_empty.test | RUN | 0.389193 | 1.191288 | 1.089244 | 0.003063 | 0.003086 | 0.002902 |
| test_dynamic_any_cast.test | RUN | 0.733144 | 3.454600 | 2.905667 | 0.004467 | 0.004496 | 0.004403 |
| test_limits.test | RUN | 0.667779 | 2.281854 | 2.198571 | 0.003315 | 0.003385 | 0.003086 |
| test_param.test | COMPILE | 0.259795 | 0.645991 | 0.701159 | — | — | — |
| test_is_subconcept.test | COMPILE | 0.255606 | 0.510800 | 0.623185 | — | — | — |
| test_sfinae.test | COMPILE | 0.262233 | 0.514838 | 0.629420 | — | — | — |
| test_interface_order.test | COMPILE | 0.253690 | 0.494173 | 0.610882 | — | — | — |
| test_is_placeholder.test | COMPILE | 0.030914 | 0.048776 | 0.056906 | — | — | — |
| fail_default_construct.test | COMPILE_FAIL | 0.247662 | 0.462598 | 0.552389 | — | — | — |
| fail_construct_mismatch.test | COMPILE_FAIL | 0.257026 | 0.478960 | 0.513789 | — | — | — |
| fail_construct_mismatch_ref.test | COMPILE_FAIL | 0.245469 | 0.473163 | 0.545926 | — | — | — |
| fail_construct_mismatch_cref.test | COMPILE_FAIL | 0.247358 | 0.467974 | 0.520237 | — | — | — |
| fail_construct_mismatch_rref.test | COMPILE_FAIL | 0.250918 | 0.459257 | 0.548685 | — | — | — |
| fail_binding_convert_no_mapping.test | COMPILE_FAIL | 0.258040 | 0.457977 | 0.528803 | — | — | — |
| fail_increment_discard_const.test | COMPILE_FAIL | 0.254358 | 0.472437 | 0.484151 | — | — | — |
| fail_not_incrementable.test | COMPILE_FAIL | 0.266993 | 0.513670 | 0.569241 | — | — | — |
| fail_ref_assign.test | COMPILE_FAIL | 0.257821 | 0.498037 | 0.519217 | — | — | — |
| fail_cref_assign.test | COMPILE_FAIL | 0.254882 | 0.494462 | 0.469882 | — | — | — |
| fail_rref_assign.test | COMPILE_FAIL | 0.257623 | 0.462857 | 0.536318 | — | — | — |
| fail_ref_discard_const.test | COMPILE_FAIL | 0.254159 | 0.487155 | 0.477219 | — | — | — |
| fail_ref_discard_const_convert.test | COMPILE_FAIL | 0.251895 | 0.494944 | 0.554992 | — | — | — |
| fail_ref_discard_const_convert_ref.test | COMPILE_FAIL | 0.250371 | 0.503615 | 0.565298 | — | — | — |
| fail_ref_discard_const_convert_cref.test | COMPILE_FAIL | 0.253677 | 0.510757 | 0.492952 | — | — | — |
| fail_ref_discard_const_init.test | COMPILE_FAIL | 0.252611 | 0.447095 | 0.517094 | — | — | — |
| fail_any_cast_discard_const1.test | COMPILE_FAIL | 0.246005 | 0.477620 | 0.553828 | — | — | — |
| fail_any_cast_discard_const2.test | COMPILE_FAIL | 0.247200 | 0.490902 | 0.486144 | — | — | — |
| fail_any_cast_discard_const3.test | COMPILE_FAIL | 0.247044 | 0.451660 | 0.572102 | — | — | — |
| fail_any_cast_discard_const4.test | COMPILE_FAIL | 0.251347 | 0.482504 | 0.575890 | — | — | — |
| fail_any_cast_discard_const5.test | COMPILE_FAIL | 0.245956 | 0.493195 | 0.509661 | — | — | — |
| fail_any_cast_discard_const6.test | COMPILE_FAIL | 0.249508 | 0.468271 | 0.577858 | — | — | — |
| fail_any_cast_pointer_to_ref.test | COMPILE_FAIL | 0.239683 | 0.479540 | 0.545635 | — | — | — |
| fail_any_cast_pointer_to_val.test | COMPILE_FAIL | 0.204597 | 0.466310 | 0.523384 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.922×† | 0.870×, 0.974× | 1.068× |
| clang | 0.985× | 0.989×, 0.980× | 1.039× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051632).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| test_add.test | 9 | 0.001759s | 0.001746s | 0.001694s | byte equality observed |
| test_add_assign.test | 10 | 0.001714s | 0.001695s | 0.001650s | byte equality observed |
| test_any_cast.test | 9 | 0.001977s | 0.001940s | 0.001924s | byte equality observed |
| test_assign.test | 4 | 0.004913s | 0.004328s | 0.004704s | byte equality observed |
| test_binding.test | 10 | 0.001608s | 0.001590s | 0.001579s | byte equality observed |
| test_binding_of.test | 10 | 0.001539s | 0.001483s | 0.001494s | byte equality observed |
| test_callable.test | 7 | 0.002541s | 0.002505s | 0.002450s | byte equality observed |
| test_construct.test | 5 | 0.003387s | 0.003217s | 0.003152s | byte equality observed |
| test_construct_cref.test | 7 | 0.002254s | 0.002253s | 0.002200s | byte equality observed |
| test_construct_ref.test | 8 | 0.002073s | 0.002000s | 0.001972s | byte equality observed |
| test_deduced.test | 10 | 0.001545s | 0.001487s | 0.001483s | byte equality observed |
| test_dereference.test | 10 | 0.001639s | 0.001567s | 0.001622s | byte equality observed |
| test_dynamic_any_cast.test | 8 | 0.002272s | 0.002226s | 0.002212s | byte equality observed |
| test_equal.test | 9 | 0.001913s | 0.001902s | 0.001876s | byte equality observed |
| test_forward_iterator.test | 10 | 0.001660s | 0.001627s | 0.001533s | byte equality observed |
| test_free.test | 9 | 0.001940s | 0.001870s | 0.001845s | byte equality observed |
| test_increment.test | 10 | 0.001589s | 0.001553s | 0.001515s | byte equality observed |
| test_is_empty.test | 10 | 0.001569s | 0.001552s | 0.001484s | byte equality observed |
| test_less.test | 8 | 0.002069s | 0.002088s | 0.001998s | byte equality observed |
| test_limits.test | 10 | 0.001704s | 0.001722s | 0.001647s | byte equality observed |
| test_member.test | 7 | 0.002343s | 0.002386s | 0.002266s | byte equality observed |
| test_negate.test | 10 | 0.001530s | 0.001488s | 0.001499s | byte equality observed |
| test_nested.test | 10 | 0.001579s | 0.001552s | 0.001530s | byte equality observed |
| test_null.test | 10 | 0.001609s | 0.001588s | 0.001573s | byte equality observed |
| test_reference.test | 10 | 0.001676s | 0.001666s | 0.001617s | byte equality observed |
| test_relaxed.test | 10 | 0.001546s | 0.001555s | 0.001523s | byte equality observed |
| test_same_type.test | 10 | 0.001631s | 0.001650s | 0.001572s | byte equality observed |
| test_stream.test | 8 | 0.002200s | 0.002207s | 0.002174s | byte equality observed |
| test_subscript.test | 10 | 0.001664s | 0.001723s | 0.001667s | byte equality observed |
| test_tuple.test | 10 | 0.001667s | 0.001679s | 0.001653s | byte equality observed |
| test_typeid_of.test | 10 | 0.001665s | 0.001700s | 0.001675s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051632).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
