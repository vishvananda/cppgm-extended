# libs/type_erasure/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_binding.test | RUN | 0.708088 | 2.861226 | 2.558516 | 0.003396 | 0.002976 | 0.004837 |
| test_increment.test | RUN | 0.732607 | 1.913863 | 3.459781 | 0.003235 | 0.003730 | 0.004939 |
| test_add.test | RUN | 0.909297 | 3.172600 | 4.851598 | 0.004500 | 0.004442 | 0.004927 |
| test_add_assign.test | RUN | 0.870707 | 3.296236 | 3.235334 | 0.005690 | 0.003719 | 0.005339 |
| test_callable.test | RUN | 1.592134 | 5.742524 | 8.559121 | 0.008566 | 0.005557 | 0.005215 |
| test_reference.test | RUN | 0.707108 | 2.396547 | 2.465189 | 0.003828 | 0.003511 | 0.003358 |
| test_construct.test | RUN | 3.284871 | 35.610891 | 20.165359 | 0.007031 | 0.007131 | 0.006732 |
| test_relaxed.test | RUN | 0.823269 | 1.669314 | 2.509257 | 0.003400 | 0.003735 | 0.004800 |
| test_assign.test | RUN | 8.917975 | 132.550440 | 76.853902 | 0.011031 | 0.010150 | 0.012297 |
| test_construct_ref.test | RUN | 0.849559 | 4.022624 | 3.190534 | 0.004628 | 0.004611 | 0.004559 |
| test_construct_cref.test | RUN | 1.134215 | 3.795511 | 4.749332 | 0.004983 | 0.005185 | 0.004923 |
| test_any_cast.test | RUN | 0.814127 | 4.196373 | 3.270225 | 0.004345 | 0.004541 | 0.004096 |
| test_binding_of.test | RUN | 0.654310 | 1.563211 | 2.793126 | 0.003376 | 0.003586 | 0.003138 |
| test_typeid_of.test | RUN | 0.671760 | 2.307784 | 2.434610 | 0.003475 | 0.003346 | 0.003168 |
| test_nested.test | RUN | 0.760920 | 2.161474 | 3.059520 | 0.003703 | 0.003116 | 0.003870 |
| test_less.test | RUN | 0.972073 | 4.187572 | 4.563094 | 0.004801 | 0.004782 | 0.004547 |
| test_equal.test | RUN | 0.914256 | 4.105881 | 4.242521 | 0.004263 | 0.003849 | 0.004110 |
| test_negate.test | RUN | 0.657399 | 1.657012 | 2.922232 | 0.005006 | 0.003357 | 0.003092 |
| test_dereference.test | RUN | 0.690317 | 2.194770 | 2.588885 | 0.003513 | 0.003177 | 0.003312 |
| test_subscript.test | RUN | 0.802194 | 2.426903 | 2.505207 | 0.003147 | 0.003305 | 0.003723 |
| test_forward_iterator.test | RUN | 1.282748 | 2.740167 | 3.068468 | 0.003817 | 0.003071 | 0.003890 |
| test_tuple.test | RUN | 0.780180 | 2.778897 | 2.885030 | 0.003605 | 0.003387 | 0.004368 |
| test_stream.test | RUN | 1.029770 | 5.675010 | 3.866204 | 0.004484 | 0.007352 | 0.006307 |
| test_deduced.test | RUN | 1.058143 | 1.644681 | 2.021140 | 0.002929 | 0.003035 | 0.003777 |
| test_same_type.test | RUN | 0.747652 | 2.379310 | 3.030261 | 0.003319 | 0.004078 | 0.003166 |
| test_member.test | RUN | 0.991819 | 5.428108 | 4.441852 | 0.005169 | 0.009614 | 0.004848 |
| test_null.test | RUN | 0.701308 | 2.135634 | 1.951014 | 0.003187 | 0.003335 | 0.004053 |
| test_free.test | RUN | 0.831849 | 5.557208 | 2.740140 | 0.003857 | 0.006511 | 0.003770 |
| test_is_empty.test | RUN | 0.688076 | 2.086948 | 1.888602 | 0.003960 | 0.005349 | 0.003183 |
| test_dynamic_any_cast.test | RUN | 1.131025 | 8.102089 | 5.130271 | 0.005613 | 0.007534 | 0.005776 |
| test_limits.test | RUN | 1.099802 | 5.782299 | 3.593731 | 0.004088 | 0.005899 | 0.003621 |
| test_param.test | COMPILE | 0.440500 | 1.020097 | 1.204829 | — | — | — |
| test_is_subconcept.test | COMPILE | 0.404534 | 0.803605 | 1.046404 | — | — | — |
| test_sfinae.test | COMPILE | 0.455334 | 0.796098 | 1.099292 | — | — | — |
| test_interface_order.test | COMPILE | 0.445917 | 0.786454 | 1.031664 | — | — | — |
| test_is_placeholder.test | COMPILE | 0.055428 | 0.073582 | 0.086904 | — | — | — |
| fail_default_construct.test | COMPILE_FAIL | 0.417901 | 0.767038 | 0.963107 | — | — | — |
| fail_construct_mismatch.test | COMPILE_FAIL | 0.445015 | 1.020847 | 0.989867 | — | — | — |
| fail_construct_mismatch_ref.test | COMPILE_FAIL | 0.408417 | 0.984638 | 1.004939 | — | — | — |
| fail_construct_mismatch_cref.test | COMPILE_FAIL | 0.668366 | 1.050157 | 0.988059 | — | — | — |
| fail_construct_mismatch_rref.test | COMPILE_FAIL | 0.668352 | 1.055988 | 1.042720 | — | — | — |
| fail_binding_convert_no_mapping.test | COMPILE_FAIL | 0.432844 | 1.038794 | 0.967953 | — | — | — |
| fail_increment_discard_const.test | COMPILE_FAIL | 0.430926 | 1.207310 | 0.958851 | — | — | — |
| fail_not_incrementable.test | COMPILE_FAIL | 0.649795 | 1.345093 | 1.063536 | — | — | — |
| fail_ref_assign.test | COMPILE_FAIL | 0.425077 | 1.319604 | 1.009187 | — | — | — |
| fail_cref_assign.test | COMPILE_FAIL | 0.630361 | 1.379425 | 0.953695 | — | — | — |
| fail_rref_assign.test | COMPILE_FAIL | 0.427695 | 1.535350 | 0.949578 | — | — | — |
| fail_ref_discard_const.test | COMPILE_FAIL | 0.426549 | 1.425265 | 1.008408 | — | — | — |
| fail_ref_discard_const_convert.test | COMPILE_FAIL | 0.427588 | 1.472047 | 1.011499 | — | — | — |
| fail_ref_discard_const_convert_ref.test | COMPILE_FAIL | 0.558648 | 1.506710 | 1.007979 | — | — | — |
| fail_ref_discard_const_convert_cref.test | COMPILE_FAIL | 0.463533 | 1.480004 | 0.989716 | — | — | — |
| fail_ref_discard_const_init.test | COMPILE_FAIL | 0.411499 | 1.381556 | 0.936093 | — | — | — |
| fail_any_cast_discard_const1.test | COMPILE_FAIL | 0.394429 | 1.445743 | 1.007585 | — | — | — |
| fail_any_cast_discard_const2.test | COMPILE_FAIL | 0.398033 | 1.279835 | 1.008533 | — | — | — |
| fail_any_cast_discard_const3.test | COMPILE_FAIL | 0.410463 | 1.367823 | 1.050262 | — | — | — |
| fail_any_cast_discard_const4.test | COMPILE_FAIL | 0.393631 | 1.422355 | 1.054260 | — | — | — |
| fail_any_cast_discard_const5.test | COMPILE_FAIL | 0.393449 | 1.451657 | 1.050106 | — | — | — |
| fail_any_cast_discard_const6.test | COMPILE_FAIL | 0.396889 | 1.132755 | 1.012716 | — | — | — |
| fail_any_cast_pointer_to_ref.test | COMPILE_FAIL | 0.390579 | 1.351012 | 1.143917 | — | — | — |
| fail_any_cast_pointer_to_val.test | COMPILE_FAIL | 0.406872 | 1.415310 | 1.130745 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.909×† | 0.861×, 0.958× | 1.010× |
| clang | 0.912× | 0.897×, 0.928× | 1.021× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1353 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| test_add.test | 5 | 0.003257s | 0.002909s | 0.002859s | byte equality observed |
| test_add_assign.test | 5 | 0.003367s | 0.003081s | 0.002834s | byte equality observed |
| test_any_cast.test | 4 | 0.003358s | 0.003206s | 0.003323s | byte equality observed |
| test_assign.test | 2 | 0.009314s | 0.008559s | 0.008238s | byte equality observed |
| test_binding.test | 5 | 0.002989s | 0.002753s | 0.002874s | byte equality observed |
| test_binding_of.test | 6 | 0.002699s | 0.002654s | 0.002548s | byte equality observed |
| test_callable.test | 3 | 0.004287s | 0.004212s | 0.004193s | byte equality observed |
| test_construct.test | 3 | 0.005890s | 0.005404s | 0.005422s | byte equality observed |
| test_construct_cref.test | 4 | 0.004256s | 0.003943s | 0.003760s | byte equality observed |
| test_construct_ref.test | 4 | 0.003742s | 0.003564s | 0.003232s | byte equality observed |
| test_deduced.test | 4 | 0.002826s | 0.002766s | 0.002583s | byte equality observed |
| test_dereference.test | 5 | 0.003009s | 0.002821s | 0.002701s | byte equality observed |
| test_dynamic_any_cast.test | 4 | 0.003982s | 0.003843s | 0.003687s | byte equality observed |
| test_equal.test | 4 | 0.003313s | 0.003172s | 0.003204s | byte equality observed |
| test_forward_iterator.test | 4 | 0.002825s | 0.002863s | 0.002870s | byte equality observed |
| test_free.test | 4 | 0.003357s | 0.003153s | 0.003184s | byte equality observed |
| test_increment.test | 5 | 0.002857s | 0.002791s | 0.002636s | byte equality observed |
| test_is_empty.test | 5 | 0.002936s | 0.002710s | 0.002694s | byte equality observed |
| test_less.test | 4 | 0.004172s | 0.003493s | 0.003307s | byte equality observed |
| test_limits.test | 5 | 0.003025s | 0.002934s | 0.002776s | byte equality observed |
| test_member.test | 3 | 0.004198s | 0.003941s | 0.003704s | byte equality observed |
| test_negate.test | 4 | 0.002659s | 0.002718s | 0.002863s | byte equality observed |
| test_nested.test | 5 | 0.002791s | 0.002704s | 0.002607s | byte equality observed |
| test_null.test | 5 | 0.002851s | 0.002818s | 0.002676s | byte equality observed |
| test_reference.test | 4 | 0.002969s | 0.002856s | 0.002729s | byte equality observed |
| test_relaxed.test | 5 | 0.002752s | 0.002631s | 0.002520s | byte equality observed |
| test_same_type.test | 5 | 0.002873s | 0.002829s | 0.002733s | byte equality observed |
| test_stream.test | 4 | 0.003866s | 0.003615s | 0.003509s | byte equality observed |
| test_subscript.test | 4 | 0.003218s | 0.002804s | 0.002852s | byte equality observed |
| test_tuple.test | 5 | 0.003121s | 0.002913s | 0.002689s | byte equality observed |
| test_typeid_of.test | 5 | 0.002914s | 0.002807s | 0.002726s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
