# libs/uuid/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_include1.test | RUN | 0.402704 | 0.929452 | 1.308674 | 0.000593 | 0.000554 | 0.001320 |
| test_uuid.test | RUN | 0.246230 | 0.767602 | 0.914676 | 0.001373 | 0.001297 | 0.001273 |
| test_uuid_no_simd.test | RUN | 0.242738 | 0.754472 | 0.806423 | 0.001366 | 0.001325 | 0.001300 |
| test_uuid_2.test | RUN | 0.170921 | 0.548777 | 0.603921 | 0.001674 | 0.001364 | 0.001099 |
| test_uuid_3.test | RUN | 0.146640 | 0.369165 | 0.465390 | 0.001379 | 0.001446 | 0.001179 |
| test_alignment.test | RUN | 0.136038 | 0.352586 | 0.426710 | 0.001466 | 0.001264 | 0.001352 |
| test_alignment_2.test | RUN | 0.134181 | 0.330981 | 0.435400 | 0.001338 | 0.001253 | 0.001244 |
| test_attribute_packed.test | RUN | 0.138170 | 0.349108 | 0.431005 | 0.001388 | 0.001339 | 0.001119 |
| test_pragma_pack.test | RUN | 0.135374 | 0.347255 | 0.425301 | 0.001647 | 0.001370 | 0.001195 |
| test_data.test | RUN | 0.162095 | 0.639728 | 0.622192 | 0.001266 | 0.001365 | 0.001275 |
| test_comparison.test | RUN | 0.240581 | 0.744995 | 0.850267 | 0.001419 | 0.001405 | 0.001214 |
| test_comparison_no_simd.test | RUN | 0.221850 | 0.845322 | 0.764582 | 0.001378 | 0.001399 | 0.001268 |
| test_io.test | RUN | 0.306503 | 1.410609 | 1.273271 | 0.001457 | 0.001457 | 0.001421 |
| test_io_no_simd.test | RUN | 0.289331 | 1.412383 | 1.235560 | 0.001327 | 0.001423 | 0.001399 |
| test_io_2.test | RUN | 0.239252 | 1.474544 | 0.915790 | 0.001298 | 0.001439 | 0.001315 |
| test_io_2_no_simd.test | RUN | 0.211419 | 1.399577 | 0.834398 | 0.001520 | 0.001493 | 0.001404 |
| test_to_chars.test | RUN | 0.212046 | 0.928040 | 0.830778 | 0.001382 | 0.001450 | 0.001405 |
| test_to_chars_no_simd.test | RUN | 0.201710 | 0.941441 | 0.813131 | 0.001540 | 0.001423 | 0.001334 |
| test_to_chars_2.test | RUN | 0.256723 | 1.715155 | 0.999437 | 0.001264 | 0.001430 | 0.001250 |
| test_to_chars_2_no_simd.test | RUN | 0.256275 | 1.818887 | 0.901163 | 0.001262 | 0.001416 | 0.001286 |
| test_to_chars_3.test | RUN | 0.172130 | 0.600054 | 0.606913 | 0.001402 | 0.001288 | 0.001182 |
| test_from_chars.test | RUN | 0.201908 | 0.867076 | 0.686497 | 0.001448 | 0.001273 | 0.001202 |
| test_from_chars_no_simd.test | RUN | 0.167547 | 0.686876 | 0.569477 | 0.001271 | 0.001293 | 0.001243 |
| test_from_chars_2.test | RUN | 0.199051 | 0.818875 | 0.667791 | 0.001256 | 0.001346 | 0.001234 |
| test_from_chars_2_no_simd.test | RUN | 0.161497 | 0.618065 | 0.545259 | 0.001412 | 0.001489 | 0.001173 |
| test_uuid_from_string.test | RUN | 0.234572 | 0.901646 | 0.805680 | 0.001410 | 0.001290 | 0.001129 |
| test_uuid_from_string_2.test | RUN | 0.206445 | 0.814575 | 0.691715 | 0.001725 | 0.001673 | 0.001598 |
| test_uuid_from_string_3.test | RUN | 0.205483 | 0.816894 | 0.711286 | 0.001119 | 0.001369 | 0.001311 |
| test_uuid_clock.test | RUN | 0.121402 | 0.329620 | 0.412319 | 0.001192 | 0.001398 | 0.001287 |
| test_nil_generator.test | RUN | 0.145216 | 0.391429 | 0.485969 | 0.001366 | 0.001304 | 0.001268 |
| test_string_generator.test | RUN | 0.175680 | 0.649489 | 0.677879 | 0.001592 | 0.001433 | 0.001403 |
| test_string_generator_2.test | RUN | 0.208550 | 0.919651 | 0.760847 | 0.001340 | 0.001298 | 0.001271 |
| test_random_generator.test | RUN | 0.320061 | 1.786898 | 1.154027 | 0.003548 | 0.003556 | 0.003562 |
| test_random_generator_2.test | RUN | 0.223220 | 0.676356 | 0.777388 | 0.001299 | 0.001371 | 0.001205 |
| test_name_generator.test | RUN | 0.199595 | 2.305690 | 0.839482 | 0.001286 | 0.001432 | 0.001356 |
| test_namespaces.test | RUN | 0.157972 | 0.483194 | 0.554102 | 0.001299 | 0.001432 | 0.001137 |
| test_name_generator_md5.test | RUN | 0.209019 | 1.100139 | 0.815374 | 0.001392 | 0.001407 | 0.001354 |
| test_name_generator_sha1.test | RUN | 0.198717 | 3.431815 | 0.798118 | 0.001303 | 0.001270 | 0.001274 |
| test_time_generator.test | RUN | 0.221204 | 0.681444 | 0.740348 | 0.001418 | 0.001288 | 0.001285 |
| test_time_generator_v1.test | RUN | 0.244225 | 0.753945 | 0.837140 | 0.001784 | 0.001914 | 0.001775 |
| test_time_generator_v1_2.test | RUN | 0.241857 | 0.691232 | 0.805227 | 0.001210 | 0.001706 | 0.001637 |
| test_time_generator_v1_3.test | RUN | 0.265947 | 0.736415 | 0.892805 | 0.004997 | 0.004233 | 0.003562 |
| test_time_generator_v6.test | RUN | 0.246144 | 0.740208 | 0.853481 | 0.001882 | 0.001840 | 0.001829 |
| test_time_generator_v6_2.test | RUN | 0.241874 | 0.698454 | 0.821878 | 0.001652 | 0.001681 | 0.001546 |
| test_time_generator_v6_3.test | RUN | 0.271114 | 0.742752 | 0.879285 | 0.005388 | 0.004180 | 0.004845 |
| test_time_generator_v7.test | RUN | 0.241916 | 0.742073 | 0.802021 | 0.001568 | 0.001576 | 0.001461 |
| test_time_generator_v7_2.test | RUN | 0.224540 | 0.657928 | 0.744224 | 0.001391 | 0.001473 | 0.001351 |
| test_time_generator_v7_3.test | RUN | 0.219471 | 0.846130 | 0.759229 | 0.002283 | 0.002473 | 0.002310 |
| test_serialization.test | RUN | 0.397889 | 1.274004 | 1.384274 | 0.001658 | 0.001351 | 0.001460 |
| test_tagging.test | RUN | 0.222232 | 0.793691 | 0.756754 | 0.001338 | 0.001321 | 0.001309 |
| test_uuid_class.test | RUN | 0.235433 | 0.703609 | 0.818892 | 0.001393 | 0.001314 | 0.001264 |
| test_uuid_in_map.test | RUN | 0.256826 | 0.808516 | 0.880981 | 0.001320 | 0.001353 | 0.001248 |
| test_hash.test | RUN | 0.259346 | 0.729713 | 0.811313 | 0.001371 | 0.001264 | 0.001344 |
| test_hash_value.test | RUN | 0.184006 | 0.499879 | 0.593777 | 0.004752 | 0.004766 | 0.004472 |
| test_std_unordered.test | RUN | 0.239587 | 0.824542 | 0.830876 | 0.001638 | 0.001394 | 0.001461 |
| test_boost_unordered.test | RUN | 0.346624 | 1.057460 | 1.072979 | 0.001428 | 0.001343 | 0.001285 |
| test_detail_md5.test | RUN | 0.156996 | 0.515002 | 0.586118 | 0.001414 | 0.001408 | 0.001330 |
| test_detail_sha1.test | RUN | 0.148074 | 0.660558 | 0.537566 | 0.009612 | 0.009029 | 0.008197 |
| test_detail_endian.test | RUN | 0.111754 | 0.379177 | 0.393747 | 0.001320 | 0.001648 | 0.001308 |
| test_detail_chacha20.test | RUN | 0.127265 | 0.495663 | 0.413551 | 0.001395 | 0.001297 | 0.001219 |
| random_provider_no_copy_ctor.test | COMPILE_FAIL | 0.114822 | 0.286979 | 0.314534 | — | — | — |
| random_provider_no_copy_assign.test | COMPILE_FAIL | 0.114699 | 0.255845 | 0.323915 | — | — | — |
| test_msvc_simd_bug981648.test | RUN | 0.192327 | 0.508205 | 0.590377 | 0.001355 | 0.001317 | 0.001309 |
| test_bench_random.test | RUN | 0.215801 | 0.667660 | 0.784060 | 0.012674 | 0.012472 | 0.012296 |
| test_entropy_error.test | RUN | 0.111446 | 0.297526 | 0.358718 | 0.001425 | 0.001378 | 0.001208 |
| test_detail_random_provider.test | RUN | 0.216413 | 0.513908 | 0.671513 | 0.001339 | 0.001378 | 0.001238 |
| test_constants.test | RUN | 0.167049 | 0.488043 | 0.542043 | 0.001341 | 0.001343 | 0.001274 |
| test_uuid_cx.test | COMPILE | 0.103344 | 0.226127 | 0.267272 | — | — | — |
| test_uuid_cx2.test | RUN | 0.168455 | 0.553363 | 0.586784 | 0.001385 | 0.001254 | 0.001360 |
| test_uuid_cx3.test | RUN | 0.148930 | 0.420460 | 0.543939 | 0.001400 | 0.001396 | 0.001101 |
| test_uuid_cx3_no_simd.test | RUN | 0.141419 | 0.407797 | 0.482500 | 0.001280 | 0.001188 | 0.001246 |
| test_hash_value_cx.test | RUN | 0.143886 | 0.385078 | 0.461426 | 0.001219 | 0.001307 | 0.001264 |
| test_string_generator_cx.test | RUN | 0.158256 | 0.485028 | 0.545819 | 0.001365 | 0.001361 | 0.001281 |
| test_string_generator_cx2.test | RUN | 0.176975 | 0.594638 | 0.644242 | 0.001444 | 0.001343 | 0.001230 |
| test_constants_cx.test | RUN | 0.139217 | 0.337766 | 0.435845 | 0.001199 | 0.001248 | 0.001267 |
| test_from_chars_cx.test | RUN | 0.179879 | 0.650527 | 0.600122 | 0.001273 | 0.001362 | 0.001230 |
| test_from_chars_cx2.test | RUN | 0.187209 | 0.809997 | 0.665447 | 0.001269 | 0.001233 | 0.001283 |
| test_to_chars_cx.test | RUN | 0.184044 | 0.748158 | 0.689018 | 0.001440 | 0.001375 | 0.001011 |
| test_uuid_from_string_cx.test | RUN | 0.197874 | 0.760444 | 0.686826 | 0.001039 | 0.001155 | 0.000935 |
| quick.test | RUN | 0.212251 | 0.566316 | 0.675689 | 0.000963 | 0.000917 | 0.000847 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.951× | 0.947×, 0.955× | 1.025× |
| clang | 1.009× | 1.000×, 1.019× | 1.013× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345325).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| quick.test | 10 | 0.000918s | 0.000851s | 0.000952s | byte equality observed |
| test_alignment.test | 10 | 0.000956s | 0.000856s | 0.000988s | byte equality observed |
| test_alignment_2.test | 10 | 0.000944s | 0.000856s | 0.000986s | byte equality observed |
| test_attribute_packed.test | 10 | 0.000938s | 0.000838s | 0.000971s | byte equality observed |
| test_bench_random.test | 2 | 0.012214s | 0.011922s | 0.012154s | native test oracle; output varies |
| test_boost_unordered.test | 10 | 0.001006s | 0.000880s | 0.001041s | byte equality observed |
| test_comparison.test | 10 | 0.000981s | 0.000895s | 0.001020s | byte equality observed |
| test_comparison_no_simd.test | 10 | 0.001026s | 0.000947s | 0.001034s | byte equality observed |
| test_constants.test | 10 | 0.001020s | 0.000878s | 0.001037s | byte equality observed |
| test_constants_cx.test | 10 | 0.000951s | 0.000888s | 0.001018s | byte equality observed |
| test_data.test | 10 | 0.000988s | 0.000900s | 0.001021s | byte equality observed |
| test_detail_chacha20.test | 10 | 0.001023s | 0.000848s | 0.001034s | byte equality observed |
| test_detail_endian.test | 10 | 0.000979s | 0.000903s | 0.001014s | byte equality observed |
| test_detail_md5.test | 10 | 0.001037s | 0.000905s | 0.001044s | byte equality observed |
| test_detail_random_provider.test | 10 | 0.001015s | 0.000849s | 0.001042s | byte equality observed |
| test_detail_sha1.test | 3 | 0.007653s | 0.006928s | 0.007430s | byte equality observed |
| test_entropy_error.test | 10 | 0.001009s | 0.000827s | 0.001034s | byte equality observed |
| test_from_chars.test | 10 | 0.001000s | 0.000895s | 0.001009s | byte equality observed |
| test_from_chars_2.test | 10 | 0.001013s | 0.000879s | 0.001029s | byte equality observed |
| test_from_chars_2_no_simd.test | 10 | 0.001021s | 0.000855s | 0.001024s | byte equality observed |
| test_from_chars_cx.test | 10 | 0.001005s | 0.000871s | 0.001005s | byte equality observed |
| test_from_chars_cx2.test | 10 | 0.001013s | 0.000909s | 0.001013s | byte equality observed |
| test_from_chars_no_simd.test | 10 | 0.001026s | 0.000911s | 0.001016s | byte equality observed |
| test_hash.test | 10 | 0.001019s | 0.000871s | 0.001043s | byte equality observed |
| test_hash_value.test | 5 | 0.004169s | 0.003512s | 0.004068s | byte equality observed |
| test_hash_value_cx.test | 10 | 0.001006s | 0.000870s | 0.001011s | byte equality observed |
| test_include1.test | 10 | 0.000424s | 0.000378s | 0.001011s | byte equality observed |
| test_io.test | 10 | 0.001085s | 0.000944s | 0.001085s | byte equality observed |
| test_io_2.test | 10 | 0.001147s | 0.000986s | 0.001156s | byte equality observed |
| test_io_2_no_simd.test | 10 | 0.001156s | 0.001018s | 0.001160s | byte equality observed |
| test_io_no_simd.test | 10 | 0.001076s | 0.000963s | 0.001101s | byte equality observed |
| test_msvc_simd_bug981648.test | 10 | 0.001015s | 0.000880s | 0.001021s | native test oracle; output varies |
| test_name_generator.test | 10 | 0.001038s | 0.000933s | 0.001054s | byte equality observed |
| test_name_generator_md5.test | 10 | 0.001042s | 0.000891s | 0.001053s | byte equality observed |
| test_name_generator_sha1.test | 10 | 0.001058s | 0.000904s | 0.001065s | byte equality observed |
| test_namespaces.test | 10 | 0.001014s | 0.000872s | 0.001017s | byte equality observed |
| test_nil_generator.test | 10 | 0.001003s | 0.000894s | 0.000972s | byte equality observed |
| test_pragma_pack.test | 10 | 0.000996s | 0.000880s | 0.000986s | byte equality observed |
| test_random_generator.test | 6 | 0.003240s | 0.003154s | 0.003202s | native test oracle; output varies |
| test_random_generator_2.test | 10 | 0.001015s | 0.000898s | 0.000998s | byte equality observed |
| test_serialization.test | 10 | 0.001131s | 0.001048s | 0.001096s | byte equality observed |
| test_std_unordered.test | 10 | 0.001087s | 0.000995s | 0.001100s | byte equality observed |
| test_string_generator.test | 10 | 0.001130s | 0.001079s | 0.001128s | byte equality observed |
| test_string_generator_2.test | 10 | 0.001050s | 0.001000s | 0.001034s | byte equality observed |
| test_string_generator_cx.test | 10 | 0.001047s | 0.000979s | 0.001012s | byte equality observed |
| test_string_generator_cx2.test | 10 | 0.001065s | 0.001008s | 0.001023s | byte equality observed |
| test_tagging.test | 10 | 0.001051s | 0.001037s | 0.001032s | byte equality observed |
| test_time_generator.test | 10 | 0.001015s | 0.001043s | 0.001043s | byte equality observed |
| test_time_generator_v1.test | 10 | 0.001470s | 0.001491s | 0.001522s | byte equality observed |
| test_time_generator_v1_2.test | 10 | 0.001282s | 0.001282s | 0.001327s | byte equality observed |
| test_time_generator_v1_3.test | 5 | 0.003846s | 0.004259s | 0.004400s | byte equality observed |
| test_time_generator_v6.test | 10 | 0.001452s | 0.001466s | 0.001556s | byte equality observed |
| test_time_generator_v6_2.test | 10 | 0.001298s | 0.001304s | 0.001314s | byte equality observed |
| test_time_generator_v6_3.test | 4 | 0.003946s | 0.004057s | 0.004070s | byte equality observed |
| test_time_generator_v7.test | 10 | 0.001258s | 0.001260s | 0.001309s | byte equality observed |
| test_time_generator_v7_2.test | 10 | 0.001090s | 0.001078s | 0.001074s | byte equality observed |
| test_time_generator_v7_3.test | 9 | 0.002157s | 0.002137s | 0.002154s | byte equality observed |
| test_to_chars.test | 10 | 0.001082s | 0.001064s | 0.001068s | byte equality observed |
| test_to_chars_2.test | 10 | 0.001043s | 0.001039s | 0.001034s | byte equality observed |
| test_to_chars_2_no_simd.test | 10 | 0.001035s | 0.001022s | 0.001038s | byte equality observed |
| test_to_chars_3.test | 10 | 0.001053s | 0.001013s | 0.001027s | byte equality observed |
| test_to_chars_cx.test | 10 | 0.001037s | 0.001020s | 0.001010s | byte equality observed |
| test_to_chars_no_simd.test | 10 | 0.001094s | 0.001074s | 0.001081s | byte equality observed |
| test_uuid.test | 10 | 0.001016s | 0.001010s | 0.001005s | byte equality observed |
| test_uuid_2.test | 10 | 0.001030s | 0.001025s | 0.001025s | byte equality observed |
| test_uuid_3.test | 10 | 0.001008s | 0.001005s | 0.001036s | byte equality observed |
| test_uuid_class.test | 10 | 0.001042s | 0.001047s | 0.001036s | byte equality observed |
| test_uuid_clock.test | 10 | 0.001015s | 0.001002s | 0.001000s | byte equality observed |
| test_uuid_cx2.test | 10 | 0.001023s | 0.001029s | 0.001013s | byte equality observed |
| test_uuid_cx3.test | 10 | 0.001008s | 0.001017s | 0.000995s | byte equality observed |
| test_uuid_cx3_no_simd.test | 10 | 0.001025s | 0.001004s | 0.000991s | byte equality observed |
| test_uuid_from_string.test | 10 | 0.001040s | 0.001029s | 0.001029s | byte equality observed |
| test_uuid_from_string_2.test | 10 | 0.001296s | 0.001274s | 0.001306s | byte equality observed |
| test_uuid_from_string_3.test | 10 | 0.001125s | 0.001119s | 0.001120s | byte equality observed |
| test_uuid_from_string_cx.test | 10 | 0.001035s | 0.001003s | 0.001015s | byte equality observed |
| test_uuid_in_map.test | 10 | 0.001034s | 0.001021s | 0.001012s | byte equality observed |
| test_uuid_no_simd.test | 10 | 0.001024s | 0.001011s | 0.001012s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345325).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
