# libs/type_erasure/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_binding.test | RUN | 0.374248 | 1.456402 | 1.207528 | 0.002950 | 0.002836 | 0.002632 |
| test_increment.test | RUN | 0.382051 | 1.103260 | 1.046408 | 0.002680 | 0.002808 | 0.002638 |
| test_add.test | RUN | 0.459021 | 1.772086 | 1.568208 | 0.003220 | 0.002935 | 0.002893 |
| test_add_assign.test | RUN | 0.467945 | 1.885491 | 1.628019 | 0.002678 | 0.002964 | 0.002863 |
| test_callable.test | RUN | 0.802737 | 3.340455 | 3.669916 | 0.004940 | 0.004884 | 0.004707 |
| test_reference.test | RUN | 0.367773 | 1.331677 | 1.114180 | 0.003014 | 0.003059 | 0.002872 |
| test_construct.test | RUN | 1.653697 | 17.574328 | 10.983552 | 0.006101 | 0.005702 | 0.005819 |
| test_relaxed.test | RUN | 0.373474 | 0.978663 | 0.981425 | 0.002605 | 0.002731 | 0.002568 |
| test_assign.test | RUN | 3.819596 | 50.960692 | 36.686401 | 0.008504 | 0.005693 | 0.006120 |
| test_construct_ref.test | RUN | 0.451074 | 2.284298 | 1.642165 | 0.003799 | 0.003829 | 0.003668 |
| test_construct_cref.test | RUN | 0.498485 | 2.172211 | 2.011015 | 0.004069 | 0.004425 | 0.004236 |
| test_any_cast.test | RUN | 0.413368 | 2.243056 | 1.586163 | 0.003504 | 0.003579 | 0.003158 |
| test_binding_of.test | RUN | 0.345710 | 0.883300 | 0.951557 | 0.002368 | 0.002537 | 0.002627 |
| test_typeid_of.test | RUN | 0.375924 | 1.162525 | 1.076190 | 0.002677 | 0.003004 | 0.002862 |
| test_nested.test | RUN | 0.404930 | 1.237875 | 1.196270 | 0.002592 | 0.002721 | 0.002647 |
| test_less.test | RUN | 0.511924 | 2.481683 | 2.028961 | 0.003826 | 0.003777 | 0.003646 |
| test_equal.test | RUN | 0.498918 | 2.277116 | 1.654124 | 0.003350 | 0.003441 | 0.003369 |
| test_negate.test | RUN | 0.346968 | 0.914827 | 0.963534 | 0.002748 | 0.002564 | 0.002506 |
| test_dereference.test | RUN | 0.400293 | 1.195430 | 1.127318 | 0.002802 | 0.002934 | 0.002852 |
| test_subscript.test | RUN | 0.408131 | 1.301772 | 1.129368 | 0.003019 | 0.002939 | 0.002802 |
| test_forward_iterator.test | RUN | 0.483257 | 1.593419 | 1.350735 | 0.002826 | 0.002977 | 0.002748 |
| test_tuple.test | RUN | 0.421707 | 1.588114 | 1.293728 | 0.002865 | 0.003109 | 0.003027 |
| test_stream.test | RUN | 0.545328 | 2.685216 | 2.061201 | 0.004188 | 0.003850 | 0.004022 |
| test_deduced.test | RUN | 0.366016 | 0.968208 | 0.898513 | 0.002494 | 0.002751 | 0.002570 |
| test_same_type.test | RUN | 0.395241 | 1.268415 | 1.206737 | 0.002680 | 0.002772 | 0.002892 |
| test_member.test | RUN | 0.511605 | 2.488054 | 1.807200 | 0.004298 | 0.004234 | 0.004280 |
| test_null.test | RUN | 0.364943 | 1.152839 | 1.037074 | 0.002613 | 0.002860 | 0.002856 |
| test_free.test | RUN | 0.432073 | 1.972691 | 1.377532 | 0.003390 | 0.003498 | 0.003359 |
| test_is_empty.test | RUN | 0.355845 | 0.997751 | 0.954723 | 0.002523 | 0.002378 | 0.002655 |
| test_dynamic_any_cast.test | RUN | 0.630887 | 3.036926 | 2.684927 | 0.004340 | 0.004345 | 0.003873 |
| test_limits.test | RUN | 0.610076 | 2.060440 | 1.975538 | 0.002846 | 0.003140 | 0.002877 |
| test_param.test | COMPILE | 0.238480 | 0.549139 | 0.637384 | — | — | — |
| test_is_subconcept.test | COMPILE | 0.225920 | 0.418580 | 0.536958 | — | — | — |
| test_sfinae.test | COMPILE | 0.234905 | 0.430700 | 0.558108 | — | — | — |
| test_interface_order.test | COMPILE | 0.223137 | 0.441689 | 0.541527 | — | — | — |
| test_is_placeholder.test | COMPILE | 0.029546 | 0.045638 | 0.051723 | — | — | — |
| fail_default_construct.test | COMPILE_FAIL | 0.211837 | 0.420037 | 0.502226 | — | — | — |
| fail_construct_mismatch.test | COMPILE_FAIL | 0.217416 | 0.424635 | 0.454850 | — | — | — |
| fail_construct_mismatch_ref.test | COMPILE_FAIL | 0.214695 | 0.418285 | 0.495276 | — | — | — |
| fail_construct_mismatch_cref.test | COMPILE_FAIL | 0.214173 | 0.429518 | 0.504803 | — | — | — |
| fail_construct_mismatch_rref.test | COMPILE_FAIL | 0.211004 | 0.425861 | 0.460146 | — | — | — |
| fail_binding_convert_no_mapping.test | COMPILE_FAIL | 0.204808 | 0.413272 | 0.484926 | — | — | — |
| fail_increment_discard_const.test | COMPILE_FAIL | 0.214344 | 0.416207 | 0.509386 | — | — | — |
| fail_not_incrementable.test | COMPILE_FAIL | 0.223362 | 0.430904 | 0.476472 | — | — | — |
| fail_ref_assign.test | COMPILE_FAIL | 0.208818 | 0.402173 | 0.524788 | — | — | — |
| fail_cref_assign.test | COMPILE_FAIL | 0.212099 | 0.417000 | 0.597211 | — | — | — |
| fail_rref_assign.test | COMPILE_FAIL | 0.211897 | 0.384909 | 0.569382 | — | — | — |
| fail_ref_discard_const.test | COMPILE_FAIL | 0.212010 | 0.418564 | 0.506899 | — | — | — |
| fail_ref_discard_const_convert.test | COMPILE_FAIL | 0.213915 | 0.433509 | 0.523085 | — | — | — |
| fail_ref_discard_const_convert_ref.test | COMPILE_FAIL | 0.212020 | 0.453097 | 0.472191 | — | — | — |
| fail_ref_discard_const_convert_cref.test | COMPILE_FAIL | 0.221326 | 0.450827 | 0.525534 | — | — | — |
| fail_ref_discard_const_init.test | COMPILE_FAIL | 0.212016 | 0.417855 | 0.462182 | — | — | — |
| fail_any_cast_discard_const1.test | COMPILE_FAIL | 0.217282 | 0.440632 | 0.514337 | — | — | — |
| fail_any_cast_discard_const2.test | COMPILE_FAIL | 0.214855 | 0.426681 | 0.476049 | — | — | — |
| fail_any_cast_discard_const3.test | COMPILE_FAIL | 0.225138 | 0.436921 | 0.518265 | — | — | — |
| fail_any_cast_discard_const4.test | COMPILE_FAIL | 0.216452 | 0.437957 | 0.516166 | — | — | — |
| fail_any_cast_discard_const5.test | COMPILE_FAIL | 0.215915 | 0.428046 | 0.494974 | — | — | — |
| fail_any_cast_discard_const6.test | COMPILE_FAIL | 0.217017 | 0.428691 | 0.521251 | — | — | — |
| fail_any_cast_pointer_to_ref.test | COMPILE_FAIL | 0.199920 | 0.433051 | 0.506202 | — | — | — |
| fail_any_cast_pointer_to_val.test | COMPILE_FAIL | 0.178475 | 0.351394 | 0.382517 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.962× | 0.971×, 0.954× | 1.003× |
| clang | 0.963× | 0.965×, 0.962× | 0.995× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409407).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| test_add.test | 10 | 0.001644s | 0.001586s | 0.001602s | byte equality observed |
| test_add_assign.test | 10 | 0.001641s | 0.001574s | 0.001576s | byte equality observed |
| test_any_cast.test | 10 | 0.001830s | 0.001784s | 0.001783s | byte equality observed |
| test_assign.test | 5 | 0.004555s | 0.003991s | 0.004314s | byte equality observed |
| test_binding.test | 10 | 0.001539s | 0.001493s | 0.001455s | byte equality observed |
| test_binding_of.test | 10 | 0.001461s | 0.001408s | 0.001372s | byte equality observed |
| test_callable.test | 8 | 0.002362s | 0.002315s | 0.002288s | byte equality observed |
| test_construct.test | 6 | 0.003090s | 0.002997s | 0.002966s | byte equality observed |
| test_construct_cref.test | 9 | 0.002165s | 0.002130s | 0.002103s | byte equality observed |
| test_construct_ref.test | 10 | 0.001934s | 0.001912s | 0.001896s | byte equality observed |
| test_deduced.test | 10 | 0.001448s | 0.001408s | 0.001382s | byte equality observed |
| test_dereference.test | 10 | 0.001540s | 0.001510s | 0.001479s | byte equality observed |
| test_dynamic_any_cast.test | 8 | 0.002133s | 0.002108s | 0.002067s | byte equality observed |
| test_equal.test | 10 | 0.001804s | 0.001778s | 0.001737s | byte equality observed |
| test_forward_iterator.test | 10 | 0.001553s | 0.001491s | 0.001489s | byte equality observed |
| test_free.test | 10 | 0.001817s | 0.001749s | 0.001705s | byte equality observed |
| test_increment.test | 10 | 0.001495s | 0.001449s | 0.001423s | byte equality observed |
| test_is_empty.test | 10 | 0.001493s | 0.001460s | 0.001418s | byte equality observed |
| test_less.test | 10 | 0.001940s | 0.001893s | 0.001874s | byte equality observed |
| test_limits.test | 10 | 0.001594s | 0.001558s | 0.001538s | byte equality observed |
| test_member.test | 7 | 0.002204s | 0.002164s | 0.002134s | byte equality observed |
| test_negate.test | 10 | 0.001435s | 0.001382s | 0.001375s | byte equality observed |
| test_nested.test | 8 | 0.001504s | 0.001454s | 0.001439s | byte equality observed |
| test_null.test | 8 | 0.001543s | 0.001497s | 0.001486s | byte equality observed |
| test_reference.test | 10 | 0.001569s | 0.001572s | 0.001521s | byte equality observed |
| test_relaxed.test | 10 | 0.001482s | 0.001429s | 0.001397s | byte equality observed |
| test_same_type.test | 10 | 0.001520s | 0.001464s | 0.001481s | byte equality observed |
| test_stream.test | 9 | 0.002064s | 0.002008s | 0.001994s | byte equality observed |
| test_subscript.test | 10 | 0.001572s | 0.001527s | 0.001499s | byte equality observed |
| test_tuple.test | 10 | 0.001568s | 0.001533s | 0.001522s | byte equality observed |
| test_typeid_of.test | 10 | 0.001570s | 0.001569s | 0.001511s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409407).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
