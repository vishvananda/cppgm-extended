# libs/type_erasure/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_binding.test | RUN | 0.535698 | 1.342566 | 1.610624 | 0.004395 | 0.004723 | 0.004621 |
| test_increment.test | RUN | 0.542748 | 1.229522 | 1.456324 | 0.004193 | 0.004620 | 0.004275 |
| test_add.test | RUN | 0.657452 | 1.996839 | 2.109237 | 0.004206 | 0.004877 | 0.004466 |
| test_add_assign.test | RUN | 0.688511 | 2.103689 | 2.162285 | 0.004728 | 0.005124 | 0.005101 |
| test_callable.test | RUN | 1.004921 | 4.501928 | 4.323152 | 0.007103 | 0.007874 | 0.007713 |
| test_reference.test | RUN | 0.561952 | 1.279474 | 1.569276 | 0.004205 | 0.004832 | 0.004736 |
| test_construct.test | RUN | 1.427795 | 9.375163 | 8.010936 | 0.010155 | 0.010869 | 0.010714 |
| test_relaxed.test | RUN | 0.529413 | 1.206001 | 1.459324 | 0.004047 | 0.004356 | 0.004330 |
| test_assign.test | RUN | 2.632027 | 28.674279 | 20.761753 | 0.017324 | 0.012351 | 0.016890 |
| test_construct_ref.test | RUN | 0.600213 | 1.635375 | 1.915995 | 0.005172 | 0.005667 | 0.006068 |
| test_construct_cref.test | RUN | 0.644683 | 1.930111 | 2.232572 | 0.006108 | 0.006853 | 0.007098 |
| test_any_cast.test | RUN | 0.591494 | 1.586740 | 1.811941 | 0.004938 | 0.005937 | 0.005198 |
| test_binding_of.test | RUN | 0.519047 | 1.072895 | 1.352318 | 0.004039 | 0.004255 | 0.004294 |
| test_typeid_of.test | RUN | 0.555012 | 1.234667 | 1.454088 | 0.004236 | 0.004191 | 0.004177 |
| test_nested.test | RUN | 0.587342 | 1.459447 | 1.708825 | 0.003988 | 0.004188 | 0.004348 |
| test_less.test | RUN | 0.692603 | 2.134269 | 2.410651 | 0.005328 | 0.006038 | 0.006264 |
| test_equal.test | RUN | 0.681223 | 2.019616 | 2.117248 | 0.005064 | 0.005729 | 0.005075 |
| test_negate.test | RUN | 0.518281 | 1.132991 | 1.430479 | 0.003852 | 0.003995 | 0.004194 |
| test_dereference.test | RUN | 0.565735 | 1.340668 | 1.651952 | 0.004057 | 0.004708 | 0.004594 |
| test_subscript.test | RUN | 0.602256 | 1.423061 | 1.740060 | 0.004343 | 0.004751 | 0.004685 |
| test_forward_iterator.test | RUN | 0.680527 | 1.904261 | 2.092383 | 0.004147 | 0.004600 | 0.004435 |
| test_tuple.test | RUN | 0.587118 | 1.465341 | 1.751571 | 0.004280 | 0.005014 | 0.004340 |
| test_stream.test | RUN | 0.697057 | 2.420368 | 2.429635 | 0.005410 | 0.006457 | 0.006235 |
| test_deduced.test | RUN | 0.530117 | 1.098699 | 1.407418 | 0.003864 | 0.004497 | 0.004127 |
| test_same_type.test | RUN | 0.586811 | 1.387281 | 1.672197 | 0.004186 | 0.004185 | 0.004541 |
| test_member.test | RUN | 0.684026 | 2.299638 | 2.406988 | 0.006387 | 0.006284 | 0.006993 |
| test_null.test | RUN | 0.531312 | 1.129960 | 1.426254 | 0.004085 | 0.004477 | 0.004335 |
| test_free.test | RUN | 0.643594 | 1.658193 | 1.919101 | 0.004899 | 0.005359 | 0.005417 |
| test_is_empty.test | RUN | 0.530365 | 1.078664 | 1.391630 | 0.003643 | 0.004206 | 0.004205 |
| test_dynamic_any_cast.test | RUN | 0.789229 | 3.080813 | 3.147457 | 0.006049 | 0.006304 | 0.006715 |
| test_limits.test | RUN | 0.842780 | 3.029352 | 2.713764 | 0.004405 | 0.004318 | 0.004699 |
| test_param.test | COMPILE | 0.367895 | 0.704609 | 0.969806 | — | — | — |
| test_is_subconcept.test | COMPILE | 0.361768 | 0.601718 | 0.692047 | — | — | — |
| test_sfinae.test | COMPILE | 0.358171 | 0.613567 | 0.742280 | — | — | — |
| test_interface_order.test | COMPILE | 0.369663 | 0.651619 | 0.804352 | — | — | — |
| test_is_placeholder.test | COMPILE | 0.048161 | 0.049594 | 0.066984 | — | — | — |
| fail_default_construct.test | COMPILE_FAIL | 0.341707 | 0.569828 | 0.750091 | — | — | — |
| fail_construct_mismatch.test | COMPILE_FAIL | 0.343014 | 0.608780 | 0.819286 | — | — | — |
| fail_construct_mismatch_ref.test | COMPILE_FAIL | 0.341044 | 0.579044 | 0.765097 | — | — | — |
| fail_construct_mismatch_cref.test | COMPILE_FAIL | 0.338803 | 0.576249 | 0.778971 | — | — | — |
| fail_construct_mismatch_rref.test | COMPILE_FAIL | 0.343047 | 0.570712 | 0.767963 | — | — | — |
| fail_binding_convert_no_mapping.test | COMPILE_FAIL | 0.330086 | 0.566372 | 0.764834 | — | — | — |
| fail_increment_discard_const.test | COMPILE_FAIL | 0.343209 | 0.575247 | 0.784130 | — | — | — |
| fail_not_incrementable.test | COMPILE_FAIL | 0.354458 | 0.527192 | 0.800298 | — | — | — |
| fail_ref_assign.test | COMPILE_FAIL | 0.341470 | 0.574686 | 0.762214 | — | — | — |
| fail_cref_assign.test | COMPILE_FAIL | 0.340740 | 0.566938 | 0.782317 | — | — | — |
| fail_rref_assign.test | COMPILE_FAIL | 0.342983 | 0.583992 | 0.775405 | — | — | — |
| fail_ref_discard_const.test | COMPILE_FAIL | 0.336449 | 0.583997 | 0.787853 | — | — | — |
| fail_ref_discard_const_convert.test | COMPILE_FAIL | 0.346207 | 0.617044 | 0.809664 | — | — | — |
| fail_ref_discard_const_convert_ref.test | COMPILE_FAIL | 0.344075 | 0.640942 | 0.811435 | — | — | — |
| fail_ref_discard_const_convert_cref.test | COMPILE_FAIL | 0.346848 | 0.618162 | 0.805937 | — | — | — |
| fail_ref_discard_const_init.test | COMPILE_FAIL | 0.336500 | 0.577627 | 0.763323 | — | — | — |
| fail_any_cast_discard_const1.test | COMPILE_FAIL | 0.345875 | 0.613319 | 0.809295 | — | — | — |
| fail_any_cast_discard_const2.test | COMPILE_FAIL | 0.342654 | 0.595216 | 0.815087 | — | — | — |
| fail_any_cast_discard_const3.test | COMPILE_FAIL | 0.349402 | 0.599995 | 0.810095 | — | — | — |
| fail_any_cast_discard_const4.test | COMPILE_FAIL | 0.348509 | 0.603271 | 0.824993 | — | — | — |
| fail_any_cast_discard_const5.test | COMPILE_FAIL | 0.350198 | 0.619908 | 0.815974 | — | — | — |
| fail_any_cast_discard_const6.test | COMPILE_FAIL | 0.340924 | 0.599819 | 0.817784 | — | — | — |
| fail_any_cast_pointer_to_ref.test | COMPILE_FAIL | 0.314971 | 0.580028 | 0.654620 | — | — | — |
| fail_any_cast_pointer_to_val.test | COMPILE_FAIL | 0.256105 | 0.546867 | 0.680502 | — | — | — |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497688) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497688) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497688).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
