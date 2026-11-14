# libs/uuid/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_include1.test | RUN | 0.327647 | 0.735663 | 0.956773 | 0.000712 | 0.000748 | 0.001698 |
| test_uuid.test | RUN | 0.202219 | 0.628365 | 0.679970 | 0.001532 | 0.001673 | 0.001756 |
| test_uuid_no_simd.test | RUN | 0.186868 | 0.588392 | 0.663227 | 0.001646 | 0.001674 | 0.001515 |
| test_uuid_2.test | RUN | 0.147737 | 0.443549 | 0.463538 | 0.001591 | 0.001385 | 0.001546 |
| test_uuid_3.test | RUN | 0.121121 | 0.323170 | 0.388921 | 0.001658 | 0.001768 | 0.001691 |
| test_alignment.test | RUN | 0.110025 | 0.271388 | 0.335460 | 0.001830 | 0.001277 | 0.001746 |
| test_alignment_2.test | RUN | 0.115240 | 0.263195 | 0.336627 | 0.001662 | 0.001579 | 0.001765 |
| test_attribute_packed.test | RUN | 0.114566 | 0.277365 | 0.350880 | 0.001556 | 0.001664 | 0.001586 |
| test_pragma_pack.test | RUN | 0.113303 | 0.275845 | 0.350797 | 0.001598 | 0.001604 | 0.001524 |
| test_data.test | RUN | 0.137563 | 0.495358 | 0.487637 | 0.001614 | 0.001350 | 0.001694 |
| test_comparison.test | RUN | 0.183327 | 0.577884 | 0.675627 | 0.001640 | 0.001809 | 0.001437 |
| test_comparison_no_simd.test | RUN | 0.171916 | 0.648654 | 0.585185 | 0.001530 | 0.001609 | 0.001660 |
| test_io.test | RUN | 0.246046 | 1.131213 | 1.020870 | 0.002008 | 0.001860 | 0.001818 |
| test_io_no_simd.test | RUN | 0.213065 | 1.161396 | 1.011176 | 0.001528 | 0.001861 | 0.001963 |
| test_io_2.test | RUN | 0.184048 | 1.198751 | 0.715458 | 0.001517 | 0.001876 | 0.001794 |
| test_io_2_no_simd.test | RUN | 0.162953 | 1.116574 | 0.650262 | 0.001457 | 0.001934 | 0.001830 |
| test_to_chars.test | RUN | 0.165410 | 0.755074 | 0.727290 | 0.001746 | 0.001866 | 0.001856 |
| test_to_chars_no_simd.test | RUN | 0.148496 | 0.742301 | 0.664635 | 0.001725 | 0.001604 | 0.001752 |
| test_to_chars_2.test | RUN | 0.188503 | 1.326989 | 0.780350 | 0.001426 | 0.001616 | 0.001689 |
| test_to_chars_2_no_simd.test | RUN | 0.174893 | 1.391375 | 0.735089 | 0.001540 | 0.001408 | 0.001573 |
| test_to_chars_3.test | RUN | 0.137138 | 0.480229 | 0.480284 | 0.001720 | 0.001594 | 0.001718 |
| test_from_chars.test | RUN | 0.163385 | 0.668574 | 0.538691 | 0.001544 | 0.001719 | 0.001713 |
| test_from_chars_no_simd.test | RUN | 0.144083 | 0.531010 | 0.456831 | 0.001481 | 0.001482 | 0.001434 |
| test_from_chars_2.test | RUN | 0.160914 | 0.625986 | 0.528634 | 0.001471 | 0.001710 | 0.001696 |
| test_from_chars_2_no_simd.test | RUN | 0.137244 | 0.475257 | 0.427742 | 0.001632 | 0.001706 | 0.001772 |
| test_uuid_from_string.test | RUN | 0.186267 | 0.692209 | 0.662349 | 0.001396 | 0.001666 | 0.001620 |
| test_uuid_from_string_2.test | RUN | 0.162159 | 0.632241 | 0.532989 | 0.001693 | 0.001754 | 0.001656 |
| test_uuid_from_string_3.test | RUN | 0.165632 | 0.640417 | 0.582812 | 0.001300 | 0.001902 | 0.001554 |
| test_uuid_clock.test | RUN | 0.101305 | 0.272732 | 0.342800 | 0.001348 | 0.001816 | 0.001231 |
| test_nil_generator.test | RUN | 0.119275 | 0.308351 | 0.391326 | 0.001502 | 0.001720 | 0.001523 |
| test_string_generator.test | RUN | 0.149785 | 0.531316 | 0.520476 | 0.001546 | 0.001990 | 0.001760 |
| test_string_generator_2.test | RUN | 0.182762 | 0.754137 | 0.608881 | 0.001530 | 0.001708 | 0.002014 |
| test_random_generator.test | RUN | 0.245064 | 1.425362 | 0.935870 | 0.059072 | 0.059561 | 0.059598 |
| test_random_generator_2.test | RUN | 0.187123 | 0.571484 | 0.646244 | 0.001311 | 0.001834 | 0.001809 |
| test_name_generator.test | RUN | 0.165351 | 1.849261 | 0.643991 | 0.001468 | 0.001863 | 0.001432 |
| test_namespaces.test | RUN | 0.138315 | 0.416797 | 0.436076 | 0.001206 | 0.001675 | 0.001656 |
| test_name_generator_md5.test | RUN | 0.162724 | 0.855799 | 0.657594 | 0.001531 | 0.001870 | 0.001794 |
| test_name_generator_sha1.test | RUN | 0.156945 | 2.608295 | 0.646723 | 0.001654 | 0.001843 | 0.001827 |
| test_time_generator.test | RUN | 0.173718 | 0.547345 | 0.591300 | 0.001415 | 0.001778 | 0.001502 |
| test_time_generator_v1.test | RUN | 0.193648 | 0.609509 | 0.665094 | 0.001877 | 0.002265 | 0.002190 |
| test_time_generator_v1_2.test | RUN | 0.193082 | 0.573728 | 0.626715 | 0.001640 | 0.002028 | 0.002004 |
| test_time_generator_v1_3.test | RUN | 0.213777 | 0.597518 | 0.699177 | 0.003591 | 0.004344 | 0.004339 |
| test_time_generator_v6.test | RUN | 0.197663 | 0.595913 | 0.664238 | 0.001806 | 0.002268 | 0.002486 |
| test_time_generator_v6_2.test | RUN | 0.193756 | 0.567374 | 0.637006 | 0.001634 | 0.002048 | 0.001992 |
| test_time_generator_v6_3.test | RUN | 0.213973 | 0.590133 | 0.705546 | 0.003514 | 0.004366 | 0.003386 |
| test_time_generator_v7.test | RUN | 0.186559 | 0.615950 | 0.658595 | 0.001576 | 0.001914 | 0.001903 |
| test_time_generator_v7_2.test | RUN | 0.176613 | 0.548747 | 0.598785 | 0.001531 | 0.002140 | 0.001694 |
| test_time_generator_v7_3.test | RUN | 0.174107 | 0.672663 | 0.613040 | 0.030050 | 0.030620 | 0.030474 |
| test_serialization.test | RUN | 0.331636 | 1.042759 | 1.150374 | 0.001884 | 0.001744 | 0.001863 |
| test_tagging.test | RUN | 0.183273 | 0.638113 | 0.625250 | 0.001647 | 0.001630 | 0.001585 |
| test_uuid_class.test | RUN | 0.187033 | 0.578863 | 0.642071 | 0.001468 | 0.001476 | 0.001598 |
| test_uuid_in_map.test | RUN | 0.217142 | 0.695006 | 0.722451 | 0.001410 | 0.001738 | 0.001740 |
| test_hash.test | RUN | 0.215844 | 0.580291 | 0.687850 | 0.001588 | 0.001820 | 0.001893 |
| test_hash_value.test | RUN | 0.154229 | 0.407332 | 0.520586 | 0.003897 | 0.004258 | 0.004284 |
| test_std_unordered.test | RUN | 0.198669 | 0.654456 | 0.695772 | 0.001426 | 0.001929 | 0.001866 |
| test_boost_unordered.test | RUN | 0.281402 | 0.862573 | 0.926885 | 0.001702 | 0.001765 | 0.001498 |
| test_detail_md5.test | RUN | 0.130495 | 0.416658 | 0.470785 | 0.001631 | 0.001814 | 0.001623 |
| test_detail_sha1.test | RUN | 0.118608 | 0.525700 | 0.458290 | 0.007381 | 0.007470 | 0.006942 |
| test_detail_endian.test | RUN | 0.094999 | 0.302940 | 0.329354 | 0.001421 | 0.001539 | 0.001841 |
| test_detail_chacha20.test | RUN | 0.104612 | 0.367734 | 0.356504 | 0.001558 | 0.001337 | 0.001489 |
| random_provider_no_copy_ctor.test | COMPILE_FAIL | 0.093875 | 0.209342 | 0.258982 | — | — | — |
| random_provider_no_copy_assign.test | COMPILE_FAIL | 0.094511 | 0.208335 | 0.270859 | — | — | — |
| test_msvc_simd_bug981648.test | RUN | 0.158837 | 0.417059 | 0.502001 | 0.001569 | 0.001771 | 0.001831 |
| test_bench_random.test | RUN | 0.173899 | 0.536890 | 0.659046 | 0.311987 | 0.311427 | 0.311992 |
| test_entropy_error.test | RUN | 0.085569 | 0.253769 | 0.310062 | 0.001421 | 0.001712 | 0.001796 |
| test_detail_random_provider.test | RUN | 0.166714 | 0.428145 | 0.557825 | 0.003039 | 0.002982 | 0.001586 |
| test_constants.test | RUN | 0.129265 | 0.361918 | 0.394409 | 0.001258 | 0.001553 | 0.001380 |
| test_uuid_cx.test | COMPILE | 0.072424 | 0.175157 | 0.230445 | — | — | — |
| test_uuid_cx2.test | RUN | 0.133232 | 0.429370 | 0.502280 | 0.001375 | 0.001438 | 0.001493 |
| test_uuid_cx3.test | RUN | 0.097101 | 0.344657 | 0.442613 | 0.001343 | 0.003016 | 0.001448 |
| test_uuid_cx3_no_simd.test | RUN | 0.105947 | 0.329484 | 0.391736 | 0.001368 | 0.001519 | 0.001453 |
| test_hash_value_cx.test | RUN | 0.114548 | 0.321315 | 0.402312 | 0.001466 | 0.001498 | 0.001440 |
| test_string_generator_cx.test | RUN | 0.129933 | 0.392097 | 0.459674 | 0.001426 | 0.001587 | 0.001713 |
| test_string_generator_cx2.test | RUN | 0.162226 | 0.476145 | 0.518601 | 0.001367 | 0.001744 | 0.001562 |
| test_constants_cx.test | RUN | 0.110940 | 0.262348 | 0.366751 | 0.001313 | 0.001664 | 0.001658 |
| test_from_chars_cx.test | RUN | 0.142280 | 0.500330 | 0.490815 | 0.001534 | 0.001861 | 0.001858 |
| test_from_chars_cx2.test | RUN | 0.145464 | 0.649060 | 0.534604 | 0.001389 | 0.001207 | 0.001359 |
| test_to_chars_cx.test | RUN | 0.148810 | 0.601745 | 0.579661 | 0.001491 | 0.001162 | 0.001554 |
| test_uuid_from_string_cx.test | RUN | 0.156370 | 0.606452 | 0.544484 | 0.001160 | 0.001103 | 0.001430 |
| quick.test | RUN | 0.119405 | 0.505033 | 0.558634 | 0.001157 | 0.001196 | 0.001197 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.021×† | 0.886×, 1.155× | 1.123× |
| clang | 1.059×† | 1.006×, 1.113× | 1.130× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051632).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| quick.test | 10 | 0.000899s | 0.000931s | 0.000896s | byte equality observed |
| test_alignment.test | 10 | 0.000910s | 0.000934s | 0.000900s | byte equality observed |
| test_alignment_2.test | 10 | 0.000904s | 0.000922s | 0.000892s | byte equality observed |
| test_attribute_packed.test | 10 | 0.000883s | 0.000908s | 0.000892s | byte equality observed |
| test_bench_random.test | 1 | 0.323160s | 0.323244s | 0.323441s | native test oracle; output varies |
| test_boost_unordered.test | 10 | 0.001449s | 0.001469s | 0.001460s | byte equality observed |
| test_comparison.test | 10 | 0.000886s | 0.000915s | 0.000894s | byte equality observed |
| test_comparison_no_simd.test | 10 | 0.000867s | 0.000940s | 0.000895s | byte equality observed |
| test_constants.test | 10 | 0.000845s | 0.000924s | 0.000872s | byte equality observed |
| test_constants_cx.test | 10 | 0.000841s | 0.000889s | 0.000837s | byte equality observed |
| test_data.test | 10 | 0.000848s | 0.000918s | 0.000869s | byte equality observed |
| test_detail_chacha20.test | 10 | 0.000855s | 0.000917s | 0.000867s | byte equality observed |
| test_detail_endian.test | 10 | 0.000863s | 0.000881s | 0.000868s | byte equality observed |
| test_detail_md5.test | 10 | 0.000883s | 0.000901s | 0.000878s | byte equality observed |
| test_detail_random_provider.test | 8 | 0.002460s | 0.002516s | 0.002433s | byte equality observed |
| test_detail_sha1.test | 4 | 0.005009s | 0.005833s | 0.004303s | byte equality observed |
| test_entropy_error.test | 10 | 0.000924s | 0.001020s | 0.000881s | byte equality observed |
| test_from_chars.test | 10 | 0.000870s | 0.000930s | 0.000861s | byte equality observed |
| test_from_chars_2.test | 10 | 0.000863s | 0.000910s | 0.000883s | byte equality observed |
| test_from_chars_2_no_simd.test | 10 | 0.000862s | 0.000891s | 0.000897s | byte equality observed |
| test_from_chars_cx.test | 10 | 0.000877s | 0.000920s | 0.000870s | byte equality observed |
| test_from_chars_cx2.test | 10 | 0.000875s | 0.000915s | 0.000871s | byte equality observed |
| test_from_chars_no_simd.test | 10 | 0.000876s | 0.000910s | 0.000887s | byte equality observed |
| test_hash.test | 10 | 0.001419s | 0.001493s | 0.001406s | byte equality observed |
| test_hash_value.test | 7 | 0.002847s | 0.003051s | 0.002820s | byte equality observed |
| test_hash_value_cx.test | 10 | 0.000877s | 0.000939s | 0.000869s | byte equality observed |
| test_include1.test | 10 | 0.000403s | 0.000415s | 0.000873s | byte equality observed |
| test_io.test | 10 | 0.000987s | 0.001021s | 0.000937s | byte equality observed |
| test_io_2.test | 10 | 0.000968s | 0.001013s | 0.001015s | byte equality observed |
| test_io_2_no_simd.test | 10 | 0.000981s | 0.001016s | 0.001004s | byte equality observed |
| test_io_no_simd.test | 10 | 0.000961s | 0.000985s | 0.000978s | byte equality observed |
| test_msvc_simd_bug981648.test | 10 | 0.000900s | 0.000923s | 0.000946s | native test oracle; output varies |
| test_name_generator.test | 10 | 0.000917s | 0.000968s | 0.000939s | byte equality observed |
| test_name_generator_md5.test | 10 | 0.000914s | 0.000975s | 0.000939s | byte equality observed |
| test_name_generator_sha1.test | 10 | 0.000961s | 0.000971s | 0.000951s | byte equality observed |
| test_namespaces.test | 10 | 0.000887s | 0.000927s | 0.000919s | byte equality observed |
| test_nil_generator.test | 10 | 0.000889s | 0.000915s | 0.000911s | byte equality observed |
| test_pragma_pack.test | 10 | 0.000883s | 0.000914s | 0.000918s | byte equality observed |
| test_random_generator.test | 1 | 0.061475s | 0.061526s | 0.061593s | native test oracle; output varies |
| test_random_generator_2.test | 10 | 0.000892s | 0.000904s | 0.000861s | byte equality observed |
| test_serialization.test | 10 | 0.000965s | 0.000983s | 0.000979s | byte equality observed |
| test_std_unordered.test | 10 | 0.001449s | 0.001480s | 0.001471s | byte equality observed |
| test_string_generator.test | 10 | 0.000943s | 0.000980s | 0.000966s | byte equality observed |
| test_string_generator_2.test | 10 | 0.000924s | 0.000957s | 0.000918s | byte equality observed |
| test_string_generator_cx.test | 10 | 0.000895s | 0.000905s | 0.000909s | byte equality observed |
| test_string_generator_cx2.test | 10 | 0.000916s | 0.000943s | 0.000925s | byte equality observed |
| test_tagging.test | 9 | 0.000933s | 0.000937s | 0.000935s | byte equality observed |
| test_time_generator.test | 10 | 0.000925s | 0.000980s | 0.000935s | byte equality observed |
| test_time_generator_v1.test | 10 | 0.001191s | 0.001376s | 0.001360s | byte equality observed |
| test_time_generator_v1_2.test | 10 | 0.001088s | 0.001162s | 0.001140s | byte equality observed |
| test_time_generator_v1_3.test | 6 | 0.003048s | 0.003036s | 0.003155s | byte equality observed |
| test_time_generator_v6.test | 10 | 0.001206s | 0.001361s | 0.001343s | byte equality observed |
| test_time_generator_v6_2.test | 10 | 0.001088s | 0.001185s | 0.001136s | byte equality observed |
| test_time_generator_v6_3.test | 6 | 0.003078s | 0.003099s | 0.003190s | byte equality observed |
| test_time_generator_v7.test | 10 | 0.001120s | 0.001378s | 0.001151s | byte equality observed |
| test_time_generator_v7_2.test | 10 | 0.001299s | 0.001310s | 0.001356s | byte equality observed |
| test_time_generator_v7_3.test | 1 | 0.031506s | 0.031769s | 0.031769s | byte equality observed |
| test_to_chars.test | 10 | 0.000934s | 0.000996s | 0.000909s | byte equality observed |
| test_to_chars_2.test | 10 | 0.000935s | 0.001012s | 0.000929s | byte equality observed |
| test_to_chars_2_no_simd.test | 10 | 0.000951s | 0.000979s | 0.000959s | byte equality observed |
| test_to_chars_3.test | 10 | 0.000919s | 0.000977s | 0.000955s | byte equality observed |
| test_to_chars_cx.test | 10 | 0.000943s | 0.001007s | 0.000937s | byte equality observed |
| test_to_chars_no_simd.test | 10 | 0.001011s | 0.001021s | 0.000982s | byte equality observed |
| test_uuid.test | 10 | 0.000938s | 0.001000s | 0.000976s | byte equality observed |
| test_uuid_2.test | 10 | 0.000926s | 0.000983s | 0.000975s | byte equality observed |
| test_uuid_3.test | 10 | 0.000944s | 0.000967s | 0.000981s | byte equality observed |
| test_uuid_class.test | 9 | 0.000926s | 0.000988s | 0.001007s | byte equality observed |
| test_uuid_clock.test | 10 | 0.000890s | 0.000958s | 0.001022s | byte equality observed |
| test_uuid_cx2.test | 10 | 0.000930s | 0.001009s | 0.000965s | byte equality observed |
| test_uuid_cx3.test | 10 | 0.000923s | 0.000985s | 0.000944s | byte equality observed |
| test_uuid_cx3_no_simd.test | 10 | 0.000889s | 0.000988s | 0.000998s | byte equality observed |
| test_uuid_from_string.test | 10 | 0.000929s | 0.000990s | 0.000943s | byte equality observed |
| test_uuid_from_string_2.test | 10 | 0.001056s | 0.001171s | 0.001171s | byte equality observed |
| test_uuid_from_string_3.test | 10 | 0.001002s | 0.001039s | 0.001021s | byte equality observed |
| test_uuid_from_string_cx.test | 10 | 0.000925s | 0.000978s | 0.000983s | byte equality observed |
| test_uuid_in_map.test | 8 | 0.000960s | 0.001013s | 0.000993s | byte equality observed |
| test_uuid_no_simd.test | 10 | 0.000971s | 0.000971s | 0.000983s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051632).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
