# libs/type_erasure/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_binding.test | RUN | 0.814787 | 2.808183 | 2.857770 | 0.004108 | 0.003847 | 0.004036 |
| test_increment.test | RUN | 0.825160 | 2.124125 | 2.443799 | 0.004100 | 0.003768 | 0.004184 |
| test_add.test | RUN | 1.003772 | 3.368841 | 3.601449 | 0.004513 | 0.004644 | 0.004612 |
| test_add_assign.test | RUN | 0.997830 | 3.572273 | 3.691703 | 0.004660 | 0.004577 | 0.004384 |
| test_callable.test | RUN | 1.659478 | 6.075659 | 7.921912 | 0.007644 | 0.007789 | 0.007825 |
| test_reference.test | RUN | 0.787403 | 2.495796 | 2.575128 | 0.004278 | 0.004591 | 0.004447 |
| test_construct.test | RUN | 3.111840 | 29.051650 | 22.188106 | 0.009634 | 0.008819 | 0.010331 |
| test_relaxed.test | RUN | 0.746834 | 1.831318 | 2.307286 | 0.004060 | 0.003726 | 0.004082 |
| test_assign.test | RUN | 8.060066 | 88.076921 | 71.022601 | 0.013599 | 0.008190 | 0.008741 |
| test_construct_ref.test | RUN | 0.964956 | 4.311709 | 3.682958 | 0.005668 | 0.005549 | 0.006125 |
| test_construct_cref.test | RUN | 1.100336 | 4.032451 | 4.478601 | 0.007012 | 0.006162 | 0.006588 |
| test_any_cast.test | RUN | 0.893488 | 4.277427 | 3.616298 | 0.005507 | 0.005117 | 0.005541 |
| test_binding_of.test | RUN | 0.729787 | 1.720572 | 2.178469 | 0.003729 | 0.003811 | 0.003438 |
| test_typeid_of.test | RUN | 0.812682 | 2.339511 | 2.516482 | 0.004173 | 0.004379 | 0.004397 |
| test_nested.test | RUN | 0.862706 | 2.276444 | 2.748930 | 0.004432 | 0.003804 | 0.004365 |
| test_less.test | RUN | 1.086121 | 4.531790 | 4.612918 | 0.006019 | 0.005709 | 0.005882 |
| test_equal.test | RUN | 1.026985 | 4.147924 | 3.567081 | 0.004993 | 0.004603 | 0.005373 |
| test_negate.test | RUN | 0.769765 | 1.704423 | 2.237267 | 0.003461 | 0.003471 | 0.003856 |
| test_dereference.test | RUN | 0.821678 | 2.077630 | 2.506234 | 0.003914 | 0.004045 | 0.004495 |
| test_subscript.test | RUN | 0.842442 | 2.370739 | 2.743894 | 0.004399 | 0.003829 | 0.004456 |
| test_forward_iterator.test | RUN | 0.993190 | 2.793746 | 2.951790 | 0.004040 | 0.004357 | 0.003758 |
| test_tuple.test | RUN | 0.843342 | 2.708997 | 2.900753 | 0.004501 | 0.003804 | 0.003986 |
| test_stream.test | RUN | 1.094182 | 4.395414 | 4.336175 | 0.005986 | 0.005482 | 0.006218 |
| test_deduced.test | RUN | 0.746566 | 1.546550 | 2.329696 | 0.003793 | 0.003356 | 0.003782 |
| test_same_type.test | RUN | 0.851265 | 2.207250 | 2.455202 | 0.004173 | 0.003939 | 0.004014 |
| test_member.test | RUN | 1.052612 | 4.374208 | 3.813362 | 0.005932 | 0.005764 | 0.006292 |
| test_null.test | RUN | 0.749075 | 2.261688 | 2.383308 | 0.004194 | 0.003987 | 0.004062 |
| test_free.test | RUN | 0.930919 | 3.849823 | 3.377498 | 0.005150 | 0.005146 | 0.005035 |
| test_is_empty.test | RUN | 0.743623 | 1.896452 | 2.161112 | 0.003860 | 0.003786 | 0.004227 |
| test_dynamic_any_cast.test | RUN | 1.324284 | 5.067709 | 6.068013 | 0.006454 | 0.006513 | 0.006138 |
| test_limits.test | RUN | 1.199326 | 3.764734 | 3.963427 | 0.004115 | 0.004754 | 0.004526 |
| test_param.test | COMPILE | 0.540467 | 1.129807 | 1.509342 | — | — | — |
| test_is_subconcept.test | COMPILE | 0.499003 | 0.843260 | 1.351907 | — | — | — |
| test_sfinae.test | COMPILE | 0.502667 | 0.863626 | 1.145080 | — | — | — |
| test_interface_order.test | COMPILE | 0.504570 | 0.829698 | 1.276268 | — | — | — |
| test_is_placeholder.test | COMPILE | 0.068214 | 0.087370 | 0.093591 | — | — | — |
| fail_default_construct.test | COMPILE_FAIL | 0.468250 | 0.815929 | 1.136206 | — | — | — |
| fail_construct_mismatch.test | COMPILE_FAIL | 0.466133 | 0.841076 | 1.187020 | — | — | — |
| fail_construct_mismatch_ref.test | COMPILE_FAIL | 0.469968 | 0.734261 | 1.248379 | — | — | — |
| fail_construct_mismatch_cref.test | COMPILE_FAIL | 0.478111 | 0.815622 | 1.067337 | — | — | — |
| fail_construct_mismatch_rref.test | COMPILE_FAIL | 0.471005 | 0.824828 | 1.263914 | — | — | — |
| fail_binding_convert_no_mapping.test | COMPILE_FAIL | 0.453865 | 0.742846 | 1.218833 | — | — | — |
| fail_increment_discard_const.test | COMPILE_FAIL | 0.465256 | 0.822365 | 1.084045 | — | — | — |
| fail_not_incrementable.test | COMPILE_FAIL | 0.499080 | 0.855314 | 1.259544 | — | — | — |
| fail_ref_assign.test | COMPILE_FAIL | 0.467692 | 0.786230 | 1.216482 | — | — | — |
| fail_cref_assign.test | COMPILE_FAIL | 0.472144 | 0.806018 | 1.135919 | — | — | — |
| fail_rref_assign.test | COMPILE_FAIL | 0.466642 | 0.819327 | 1.227284 | — | — | — |
| fail_ref_discard_const.test | COMPILE_FAIL | 0.474779 | 0.784024 | 1.263773 | — | — | — |
| fail_ref_discard_const_convert.test | COMPILE_FAIL | 0.467016 | 0.863364 | 1.108523 | — | — | — |
| fail_ref_discard_const_convert_ref.test | COMPILE_FAIL | 0.468254 | 0.876216 | 1.289262 | — | — | — |
| fail_ref_discard_const_convert_cref.test | COMPILE_FAIL | 0.477584 | 0.809956 | 1.292598 | — | — | — |
| fail_ref_discard_const_init.test | COMPILE_FAIL | 0.472846 | 0.826749 | 1.016034 | — | — | — |
| fail_any_cast_discard_const1.test | COMPILE_FAIL | 0.474284 | 0.851721 | 1.248590 | — | — | — |
| fail_any_cast_discard_const2.test | COMPILE_FAIL | 0.467307 | 0.769264 | 1.097921 | — | — | — |
| fail_any_cast_discard_const3.test | COMPILE_FAIL | 0.482495 | 0.834628 | 1.295755 | — | — | — |
| fail_any_cast_discard_const4.test | COMPILE_FAIL | 0.480755 | 0.859292 | 1.245251 | — | — | — |
| fail_any_cast_discard_const5.test | COMPILE_FAIL | 0.471179 | 0.799880 | 1.127531 | — | — | — |
| fail_any_cast_discard_const6.test | COMPILE_FAIL | 0.469851 | 0.826244 | 1.298844 | — | — | — |
| fail_any_cast_pointer_to_ref.test | COMPILE_FAIL | 0.476234 | 0.799079 | 1.102110 | — | — | — |
| fail_any_cast_pointer_to_val.test | COMPILE_FAIL | 0.416911 | 0.740892 | 0.908556 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.956× | 0.947×, 0.965× | 1.004× |
| clang | 0.971× | 0.973×, 0.969× | 1.012× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| test_add.test | 8 | 0.002983s | 0.002930s | 0.002880s | byte equality observed |
| test_add_assign.test | 8 | 0.002997s | 0.002952s | 0.002854s | byte equality observed |
| test_any_cast.test | 7 | 0.003489s | 0.003389s | 0.003312s | byte equality observed |
| test_assign.test | 3 | 0.009927s | 0.008429s | 0.009248s | byte equality observed |
| test_binding.test | 9 | 0.002832s | 0.002752s | 0.002730s | byte equality observed |
| test_binding_of.test | 8 | 0.002632s | 0.002539s | 0.002544s | byte equality observed |
| test_callable.test | 6 | 0.004482s | 0.004394s | 0.004182s | byte equality observed |
| test_construct.test | 4 | 0.005865s | 0.005754s | 0.005657s | byte equality observed |
| test_construct_cref.test | 6 | 0.004093s | 0.004034s | 0.003859s | byte equality observed |
| test_construct_ref.test | 7 | 0.003538s | 0.003537s | 0.003506s | byte equality observed |
| test_deduced.test | 8 | 0.002635s | 0.002573s | 0.002546s | byte equality observed |
| test_dereference.test | 8 | 0.002824s | 0.002681s | 0.002735s | byte equality observed |
| test_dynamic_any_cast.test | 6 | 0.004034s | 0.003873s | 0.003826s | byte equality observed |
| test_equal.test | 7 | 0.003378s | 0.003283s | 0.003266s | byte equality observed |
| test_forward_iterator.test | 9 | 0.002862s | 0.002702s | 0.002733s | byte equality observed |
| test_free.test | 7 | 0.003293s | 0.003260s | 0.003219s | byte equality observed |
| test_increment.test | 8 | 0.002722s | 0.002610s | 0.002553s | byte equality observed |
| test_is_empty.test | 8 | 0.002752s | 0.002658s | 0.002640s | byte equality observed |
| test_less.test | 7 | 0.003539s | 0.003618s | 0.003554s | byte equality observed |
| test_limits.test | 8 | 0.002956s | 0.002877s | 0.002865s | byte equality observed |
| test_member.test | 6 | 0.004112s | 0.004006s | 0.003973s | byte equality observed |
| test_negate.test | 8 | 0.002676s | 0.002540s | 0.002540s | byte equality observed |
| test_nested.test | 9 | 0.002760s | 0.002626s | 0.002641s | byte equality observed |
| test_null.test | 7 | 0.002857s | 0.002748s | 0.002693s | byte equality observed |
| test_reference.test | 7 | 0.002913s | 0.002809s | 0.002746s | byte equality observed |
| test_relaxed.test | 6 | 0.002732s | 0.002545s | 0.002600s | byte equality observed |
| test_same_type.test | 8 | 0.002826s | 0.002722s | 0.002726s | byte equality observed |
| test_stream.test | 6 | 0.003815s | 0.003750s | 0.003718s | byte equality observed |
| test_subscript.test | 8 | 0.002899s | 0.002722s | 0.002818s | byte equality observed |
| test_tuple.test | 8 | 0.002871s | 0.002874s | 0.002797s | byte equality observed |
| test_typeid_of.test | 8 | 0.002945s | 0.002858s | 0.002823s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
