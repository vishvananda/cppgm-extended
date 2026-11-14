# libs/uuid/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_include1.test | RUN | 0.315905 | 0.679315 | 0.909183 | 0.000667 | 0.000673 | 0.001505 |
| test_uuid.test | RUN | 0.186313 | 0.549818 | 0.623664 | 0.001508 | 0.001619 | 0.001425 |
| test_uuid_no_simd.test | RUN | 0.187053 | 0.554102 | 0.598967 | 0.003418 | 0.001515 | 0.001409 |
| test_uuid_2.test | RUN | 0.284795 | 0.399482 | 0.426298 | 0.001533 | 0.001748 | 0.001290 |
| test_uuid_3.test | RUN | 0.351269 | 0.282997 | 0.341040 | 0.003862 | 0.001588 | 0.001214 |
| test_alignment.test | RUN | 0.187000 | 0.268641 | 0.308368 | 0.003360 | 0.001386 | 0.001438 |
| test_alignment_2.test | RUN | 0.106362 | 0.242344 | 0.314256 | 0.003696 | 0.001283 | 0.001168 |
| test_attribute_packed.test | RUN | 0.137076 | 0.256723 | 0.304238 | 0.004468 | 0.001599 | 0.001371 |
| test_pragma_pack.test | RUN | 0.239557 | 0.253623 | 0.315337 | 0.001517 | 0.001594 | 0.001148 |
| test_data.test | RUN | 0.236512 | 0.449086 | 0.447634 | 0.001431 | 0.001496 | 0.001437 |
| test_comparison.test | RUN | 0.235893 | 0.538891 | 0.603050 | 0.001742 | 0.001583 | 0.001556 |
| test_comparison_no_simd.test | RUN | 0.171119 | 0.599541 | 0.541941 | 0.001506 | 0.001326 | 0.001374 |
| test_io.test | RUN | 0.233904 | 1.032837 | 0.884934 | 0.001730 | 0.001630 | 0.001475 |
| test_io_no_simd.test | RUN | 0.207960 | 1.001151 | 0.883357 | 0.001331 | 0.001473 | 0.001302 |
| test_io_2.test | RUN | 0.179731 | 1.037768 | 0.644765 | 0.001670 | 0.001541 | 0.001681 |
| test_io_2_no_simd.test | RUN | 0.156870 | 0.989027 | 0.613858 | 0.001296 | 0.001460 | 0.001530 |
| test_to_chars.test | RUN | 0.163044 | 0.660109 | 0.618609 | 0.001639 | 0.001832 | 0.001502 |
| test_to_chars_no_simd.test | RUN | 0.147803 | 0.664862 | 0.593452 | 0.001491 | 0.001525 | 0.001303 |
| test_to_chars_2.test | RUN | 0.185498 | 1.218746 | 0.715685 | 0.001413 | 0.001525 | 0.001393 |
| test_to_chars_2_no_simd.test | RUN | 0.172240 | 1.282065 | 0.660606 | 0.001298 | 0.001445 | 0.001354 |
| test_to_chars_3.test | RUN | 0.136833 | 0.435197 | 0.433974 | 0.001267 | 0.001530 | 0.001522 |
| test_from_chars.test | RUN | 0.151956 | 0.616373 | 0.490328 | 0.001412 | 0.001350 | 0.001240 |
| test_from_chars_no_simd.test | RUN | 0.132228 | 0.497599 | 0.403114 | 0.001476 | 0.001330 | 0.001421 |
| test_from_chars_2.test | RUN | 0.147975 | 0.582353 | 0.474103 | 0.001359 | 0.001495 | 0.001148 |
| test_from_chars_2_no_simd.test | RUN | 0.129573 | 0.439950 | 0.396412 | 0.001294 | 0.001586 | 0.001155 |
| test_uuid_from_string.test | RUN | 0.172048 | 0.649932 | 0.577413 | 0.001356 | 0.001460 | 0.001465 |
| test_uuid_from_string_2.test | RUN | 0.152872 | 0.585664 | 0.496889 | 0.001433 | 0.001800 | 0.001591 |
| test_uuid_from_string_3.test | RUN | 0.153750 | 0.594776 | 0.518989 | 0.001246 | 0.001633 | 0.001614 |
| test_uuid_clock.test | RUN | 0.098767 | 0.238203 | 0.300542 | 0.001296 | 0.001623 | 0.001578 |
| test_nil_generator.test | RUN | 0.109876 | 0.281548 | 0.343713 | 0.001464 | 0.001563 | 0.001511 |
| test_string_generator.test | RUN | 0.140872 | 0.480947 | 0.485779 | 0.001750 | 0.001708 | 0.001555 |
| test_string_generator_2.test | RUN | 0.176471 | 0.661613 | 0.546840 | 0.001351 | 0.001380 | 0.001522 |
| test_random_generator.test | RUN | 0.239018 | 1.259747 | 0.810362 | 0.059057 | 0.059088 | 0.059122 |
| test_random_generator_2.test | RUN | 0.175948 | 0.503722 | 0.565396 | 0.001227 | 0.001616 | 0.001339 |
| test_name_generator.test | RUN | 0.160513 | 1.556490 | 0.573721 | 0.001318 | 0.001678 | 0.001258 |
| test_namespaces.test | RUN | 0.129332 | 0.355654 | 0.404665 | 0.001444 | 0.001587 | 0.001262 |
| test_name_generator_md5.test | RUN | 0.157057 | 0.792373 | 0.585988 | 0.001186 | 0.001645 | 0.001518 |
| test_name_generator_sha1.test | RUN | 0.152212 | 2.342038 | 0.577285 | 0.001486 | 0.001660 | 0.001343 |
| test_time_generator.test | RUN | 0.163090 | 0.489687 | 0.537347 | 0.001211 | 0.001548 | 0.001321 |
| test_time_generator_v1.test | RUN | 0.181852 | 0.545067 | 0.610983 | 0.001693 | 0.002038 | 0.001936 |
| test_time_generator_v1_2.test | RUN | 0.179921 | 0.505783 | 0.583742 | 0.001659 | 0.001828 | 0.001770 |
| test_time_generator_v1_3.test | RUN | 0.198447 | 0.544828 | 0.632798 | 0.003463 | 0.003531 | 0.002910 |
| test_time_generator_v6.test | RUN | 0.183258 | 0.541454 | 0.595327 | 0.001579 | 0.002014 | 0.001704 |
| test_time_generator_v6_2.test | RUN | 0.181244 | 0.473407 | 0.573735 | 0.001403 | 0.001880 | 0.001676 |
| test_time_generator_v6_3.test | RUN | 0.197936 | 0.541905 | 0.631974 | 0.002708 | 0.004038 | 0.003538 |
| test_time_generator_v7.test | RUN | 0.173988 | 0.533367 | 0.577552 | 0.001462 | 0.001786 | 0.001412 |
| test_time_generator_v7_2.test | RUN | 0.163206 | 0.479724 | 0.532616 | 0.001507 | 0.001663 | 0.001562 |
| test_time_generator_v7_3.test | RUN | 0.163539 | 0.601594 | 0.546732 | 0.030123 | 0.030271 | 0.030088 |
| test_serialization.test | RUN | 0.321748 | 0.918390 | 0.996139 | 0.001702 | 0.001351 | 0.001575 |
| test_tagging.test | RUN | 0.167977 | 0.691174 | 0.558747 | 0.001581 | 0.001498 | 0.001546 |
| test_uuid_class.test | RUN | 0.177060 | 0.648794 | 0.577433 | 0.001334 | 0.001498 | 0.001366 |
| test_uuid_in_map.test | RUN | 0.196649 | 0.732616 | 0.625019 | 0.001358 | 0.001620 | 0.001643 |
| test_hash.test | RUN | 0.195469 | 0.521272 | 0.596177 | 0.001333 | 0.001530 | 0.001499 |
| test_hash_value.test | RUN | 0.139159 | 0.371388 | 0.435048 | 0.003940 | 0.003712 | 0.003807 |
| test_std_unordered.test | RUN | 0.184111 | 0.591823 | 0.594341 | 0.001463 | 0.001635 | 0.001614 |
| test_boost_unordered.test | RUN | 0.259187 | 0.775704 | 0.760632 | 0.001486 | 0.001557 | 0.001165 |
| test_detail_md5.test | RUN | 0.120308 | 0.369478 | 0.416809 | 0.001258 | 0.001587 | 0.001637 |
| test_detail_sha1.test | RUN | 0.118000 | 0.470055 | 0.394446 | 0.006805 | 0.006505 | 0.006060 |
| test_detail_endian.test | RUN | 0.088058 | 0.271102 | 0.286331 | 0.001428 | 0.001440 | 0.001474 |
| test_detail_chacha20.test | RUN | 0.097585 | 0.359963 | 0.295251 | 0.001682 | 0.001330 | 0.001353 |
| random_provider_no_copy_ctor.test | COMPILE_FAIL | 0.087333 | 0.215217 | 0.222887 | — | — | — |
| random_provider_no_copy_assign.test | COMPILE_FAIL | 0.087416 | 0.214806 | 0.241875 | — | — | — |
| test_msvc_simd_bug981648.test | RUN | 0.147999 | 0.391494 | 0.428592 | 0.001584 | 0.001433 | 0.001442 |
| test_bench_random.test | RUN | 0.167493 | 0.498192 | 0.545426 | 0.311399 | 0.310696 | 0.311846 |
| test_entropy_error.test | RUN | 0.082519 | 0.222474 | 0.262068 | 0.001457 | 0.001555 | 0.001435 |
| test_detail_random_provider.test | RUN | 0.154548 | 0.376343 | 0.507059 | 0.003000 | 0.002782 | 0.002968 |
| test_constants.test | RUN | 0.127804 | 0.364929 | 0.396588 | 0.001306 | 0.001434 | 0.001146 |
| test_uuid_cx.test | COMPILE | 0.061048 | 0.162778 | 0.150178 | — | — | — |
| test_uuid_cx2.test | RUN | 0.128809 | 0.348842 | 0.404410 | 0.001283 | 0.001644 | 0.001344 |
| test_uuid_cx3.test | RUN | 0.095117 | 0.312383 | 0.391247 | 0.001483 | 0.001376 | 0.001333 |
| test_uuid_cx3_no_simd.test | RUN | 0.101040 | 0.299520 | 0.352234 | 0.001381 | 0.001452 | 0.001427 |
| test_hash_value_cx.test | RUN | 0.112621 | 0.282667 | 0.343613 | 0.001439 | 0.001603 | 0.001170 |
| test_string_generator_cx.test | RUN | 0.126511 | 0.358829 | 0.398577 | 0.001430 | 0.001409 | 0.001438 |
| test_string_generator_cx2.test | RUN | 0.158044 | 0.431898 | 0.462968 | 0.001311 | 0.001512 | 0.001545 |
| test_constants_cx.test | RUN | 0.106869 | 0.248467 | 0.317031 | 0.001354 | 0.001249 | 0.001104 |
| test_from_chars_cx.test | RUN | 0.136610 | 0.477211 | 0.426920 | 0.001593 | 0.001580 | 0.001172 |
| test_from_chars_cx2.test | RUN | 0.139091 | 0.572516 | 0.465656 | 0.001425 | 0.001158 | 0.001610 |
| test_to_chars_cx.test | RUN | 0.139502 | 0.538212 | 0.505223 | 0.001271 | 0.001380 | 0.001355 |
| test_uuid_from_string_cx.test | RUN | 0.148568 | 0.552047 | 0.492177 | 0.001156 | 0.001150 | 0.000913 |
| quick.test | RUN | 0.123786 | 0.399958 | 0.469238 | 0.001111 | 0.001090 | 0.001106 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.001× | 1.003×, 1.000× | 0.996× |
| clang | 0.998× | 0.998×, 0.997× | 0.999× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026855).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| quick.test | 10 | 0.000849s | 0.000847s | 0.000860s | byte equality observed |
| test_alignment.test | 10 | 0.000868s | 0.000869s | 0.000871s | byte equality observed |
| test_alignment_2.test | 10 | 0.000872s | 0.000873s | 0.000881s | byte equality observed |
| test_attribute_packed.test | 10 | 0.000834s | 0.000875s | 0.000829s | byte equality observed |
| test_bench_random.test | 1 | 0.323095s | 0.322987s | 0.323188s | native test oracle; output varies |
| test_boost_unordered.test | 10 | 0.001394s | 0.001423s | 0.001389s | byte equality observed |
| test_comparison.test | 10 | 0.000828s | 0.000869s | 0.000825s | byte equality observed |
| test_comparison_no_simd.test | 10 | 0.000834s | 0.000869s | 0.000830s | byte equality observed |
| test_constants.test | 10 | 0.000837s | 0.000853s | 0.000812s | byte equality observed |
| test_constants_cx.test | 10 | 0.000813s | 0.000796s | 0.000790s | byte equality observed |
| test_data.test | 10 | 0.000832s | 0.000842s | 0.000802s | byte equality observed |
| test_detail_chacha20.test | 10 | 0.000839s | 0.000839s | 0.000798s | byte equality observed |
| test_detail_endian.test | 10 | 0.000822s | 0.000868s | 0.000796s | byte equality observed |
| test_detail_md5.test | 10 | 0.000848s | 0.000842s | 0.000814s | byte equality observed |
| test_detail_random_provider.test | 8 | 0.002407s | 0.002411s | 0.002367s | byte equality observed |
| test_detail_sha1.test | 4 | 0.004786s | 0.005290s | 0.003905s | byte equality observed |
| test_entropy_error.test | 10 | 0.000847s | 0.000835s | 0.000803s | byte equality observed |
| test_from_chars.test | 10 | 0.000838s | 0.000820s | 0.000800s | byte equality observed |
| test_from_chars_2.test | 10 | 0.000841s | 0.000812s | 0.000811s | byte equality observed |
| test_from_chars_2_no_simd.test | 10 | 0.000839s | 0.000841s | 0.000808s | byte equality observed |
| test_from_chars_cx.test | 10 | 0.000845s | 0.000828s | 0.000794s | byte equality observed |
| test_from_chars_cx2.test | 10 | 0.000837s | 0.000823s | 0.000807s | byte equality observed |
| test_from_chars_no_simd.test | 10 | 0.000843s | 0.000825s | 0.000815s | byte equality observed |
| test_hash.test | 10 | 0.001373s | 0.001363s | 0.001354s | byte equality observed |
| test_hash_value.test | 8 | 0.002751s | 0.002750s | 0.002661s | byte equality observed |
| test_hash_value_cx.test | 10 | 0.000834s | 0.000830s | 0.000828s | byte equality observed |
| test_include1.test | 10 | 0.000386s | 0.000389s | 0.000805s | byte equality observed |
| test_io.test | 10 | 0.000911s | 0.000913s | 0.000869s | byte equality observed |
| test_io_2.test | 10 | 0.000936s | 0.000959s | 0.000932s | byte equality observed |
| test_io_2_no_simd.test | 10 | 0.000968s | 0.000969s | 0.000954s | byte equality observed |
| test_io_no_simd.test | 10 | 0.000906s | 0.000929s | 0.000908s | byte equality observed |
| test_msvc_simd_bug981648.test | 10 | 0.000867s | 0.000875s | 0.000866s | native test oracle; output varies |
| test_name_generator.test | 10 | 0.000899s | 0.000896s | 0.000904s | byte equality observed |
| test_name_generator_md5.test | 10 | 0.000885s | 0.000903s | 0.000900s | byte equality observed |
| test_name_generator_sha1.test | 10 | 0.000903s | 0.000912s | 0.000901s | byte equality observed |
| test_namespaces.test | 10 | 0.000875s | 0.000857s | 0.000877s | byte equality observed |
| test_nil_generator.test | 10 | 0.000855s | 0.000853s | 0.000852s | byte equality observed |
| test_pragma_pack.test | 10 | 0.000856s | 0.000854s | 0.000843s | byte equality observed |
| test_random_generator.test | 1 | 0.061352s | 0.061385s | 0.061455s | native test oracle; output varies |
| test_random_generator_2.test | 10 | 0.000853s | 0.000836s | 0.000830s | byte equality observed |
| test_serialization.test | 10 | 0.000937s | 0.000911s | 0.000921s | byte equality observed |
| test_std_unordered.test | 10 | 0.001432s | 0.001427s | 0.001413s | byte equality observed |
| test_string_generator.test | 10 | 0.000929s | 0.000903s | 0.000910s | byte equality observed |
| test_string_generator_2.test | 10 | 0.000882s | 0.000878s | 0.000867s | byte equality observed |
| test_string_generator_cx.test | 10 | 0.000868s | 0.000886s | 0.000859s | byte equality observed |
| test_string_generator_cx2.test | 10 | 0.000880s | 0.000870s | 0.000873s | byte equality observed |
| test_tagging.test | 10 | 0.000905s | 0.000872s | 0.000890s | byte equality observed |
| test_time_generator.test | 10 | 0.000896s | 0.000882s | 0.000876s | byte equality observed |
| test_time_generator_v1.test | 10 | 0.001162s | 0.001239s | 0.001262s | byte equality observed |
| test_time_generator_v1_2.test | 10 | 0.001043s | 0.001078s | 0.001072s | byte equality observed |
| test_time_generator_v1_3.test | 7 | 0.003042s | 0.002894s | 0.002898s | byte equality observed |
| test_time_generator_v6.test | 10 | 0.001172s | 0.001261s | 0.001270s | byte equality observed |
| test_time_generator_v6_2.test | 10 | 0.001075s | 0.001088s | 0.001090s | byte equality observed |
| test_time_generator_v6_3.test | 7 | 0.003044s | 0.002921s | 0.002977s | byte equality observed |
| test_time_generator_v7.test | 10 | 0.001064s | 0.001106s | 0.001090s | byte equality observed |
| test_time_generator_v7_2.test | 10 | 0.001181s | 0.001291s | 0.001177s | byte equality observed |
| test_time_generator_v7_3.test | 1 | 0.030989s | 0.031827s | 0.031161s | byte equality observed |
| test_to_chars.test | 10 | 0.000875s | 0.000886s | 0.000863s | byte equality observed |
| test_to_chars_2.test | 10 | 0.000860s | 0.000881s | 0.000855s | byte equality observed |
| test_to_chars_2_no_simd.test | 10 | 0.000894s | 0.000882s | 0.000891s | byte equality observed |
| test_to_chars_3.test | 10 | 0.000878s | 0.000867s | 0.000895s | byte equality observed |
| test_to_chars_cx.test | 10 | 0.000871s | 0.000854s | 0.000881s | byte equality observed |
| test_to_chars_no_simd.test | 10 | 0.000911s | 0.000903s | 0.000912s | byte equality observed |
| test_uuid.test | 10 | 0.000870s | 0.000872s | 0.000857s | byte equality observed |
| test_uuid_2.test | 10 | 0.000865s | 0.000860s | 0.000879s | byte equality observed |
| test_uuid_3.test | 10 | 0.000854s | 0.000883s | 0.000865s | byte equality observed |
| test_uuid_class.test | 10 | 0.000887s | 0.000884s | 0.000893s | byte equality observed |
| test_uuid_clock.test | 10 | 0.000857s | 0.000854s | 0.000892s | byte equality observed |
| test_uuid_cx2.test | 10 | 0.000874s | 0.000890s | 0.000865s | byte equality observed |
| test_uuid_cx3.test | 10 | 0.000858s | 0.000857s | 0.000862s | byte equality observed |
| test_uuid_cx3_no_simd.test | 10 | 0.000859s | 0.000849s | 0.000867s | byte equality observed |
| test_uuid_from_string.test | 10 | 0.000883s | 0.000874s | 0.000873s | byte equality observed |
| test_uuid_from_string_2.test | 10 | 0.001018s | 0.000995s | 0.001034s | byte equality observed |
| test_uuid_from_string_3.test | 10 | 0.000935s | 0.000930s | 0.000925s | byte equality observed |
| test_uuid_from_string_cx.test | 10 | 0.000879s | 0.000852s | 0.000888s | byte equality observed |
| test_uuid_in_map.test | 10 | 0.000877s | 0.000856s | 0.000868s | byte equality observed |
| test_uuid_no_simd.test | 10 | 0.000854s | 0.000856s | 0.000873s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026855).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
