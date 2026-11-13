# libs/uuid/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_include1.test | RUN | 0.408977 | 0.950971 | 1.468016 | 0.000716 | 0.000748 | 0.001771 |
| test_uuid.test | RUN | 0.266338 | 0.796191 | 1.043438 | 0.001814 | 0.001922 | 0.001873 |
| test_uuid_no_simd.test | RUN | 0.242650 | 0.740737 | 0.892995 | 0.001828 | 0.001816 | 0.001703 |
| test_uuid_2.test | RUN | 0.170808 | 0.581382 | 0.682411 | 0.001608 | 0.001603 | 0.001309 |
| test_uuid_3.test | RUN | 0.147815 | 0.402771 | 0.570885 | 0.001836 | 0.001968 | 0.001640 |
| test_alignment.test | RUN | 0.139261 | 0.363853 | 0.471568 | 0.001646 | 0.001562 | 0.001836 |
| test_alignment_2.test | RUN | 0.140191 | 0.346688 | 0.510889 | 0.001692 | 0.001622 | 0.001424 |
| test_attribute_packed.test | RUN | 0.148331 | 0.363570 | 0.496368 | 0.001884 | 0.001704 | 0.001579 |
| test_pragma_pack.test | RUN | 0.137469 | 0.368737 | 0.488524 | 0.001766 | 0.001792 | 0.001683 |
| test_data.test | RUN | 0.174671 | 0.650610 | 0.688278 | 0.001658 | 0.001640 | 0.001834 |
| test_comparison.test | RUN | 0.231200 | 0.750985 | 0.959791 | 0.001728 | 0.001800 | 0.001622 |
| test_comparison_no_simd.test | RUN | 0.222323 | 0.852261 | 0.866354 | 0.001627 | 0.001909 | 0.001610 |
| test_io.test | RUN | 0.312260 | 1.428151 | 1.342367 | 0.001934 | 0.002044 | 0.001954 |
| test_io_no_simd.test | RUN | 0.282352 | 1.428269 | 1.309567 | 0.001625 | 0.001853 | 0.001730 |
| test_io_2.test | RUN | 0.229652 | 1.430063 | 0.990128 | 0.001945 | 0.001820 | 0.001786 |
| test_io_2_no_simd.test | RUN | 0.205857 | 1.393611 | 0.879183 | 0.001748 | 0.001946 | 0.001730 |
| test_to_chars.test | RUN | 0.214264 | 0.912743 | 0.922340 | 0.001796 | 0.002095 | 0.001978 |
| test_to_chars_no_simd.test | RUN | 0.196221 | 0.919393 | 0.869165 | 0.001727 | 0.001919 | 0.001886 |
| test_to_chars_2.test | RUN | 0.242031 | 1.649227 | 1.069134 | 0.001689 | 0.001785 | 0.001484 |
| test_to_chars_2_no_simd.test | RUN | 0.241625 | 1.754338 | 0.968962 | 0.001544 | 0.001847 | 0.001906 |
| test_to_chars_3.test | RUN | 0.176106 | 0.604315 | 0.678828 | 0.001556 | 0.001796 | 0.001649 |
| test_from_chars.test | RUN | 0.199095 | 0.878175 | 0.753011 | 0.001349 | 0.001964 | 0.001634 |
| test_from_chars_no_simd.test | RUN | 0.159992 | 0.690302 | 0.645479 | 0.001324 | 0.001642 | 0.001597 |
| test_from_chars_2.test | RUN | 0.185363 | 0.820167 | 0.748823 | 0.001409 | 0.001892 | 0.001464 |
| test_from_chars_2_no_simd.test | RUN | 0.140157 | 0.626826 | 0.588309 | 0.001802 | 0.001924 | 0.001483 |
| test_uuid_from_string.test | RUN | 0.225369 | 0.898809 | 0.897909 | 0.001414 | 0.001916 | 0.001503 |
| test_uuid_from_string_2.test | RUN | 0.197594 | 0.810236 | 0.777549 | 0.002049 | 0.001979 | 0.002046 |
| test_uuid_from_string_3.test | RUN | 0.198776 | 0.808045 | 0.776997 | 0.001462 | 0.002394 | 0.001928 |
| test_uuid_clock.test | RUN | 0.127855 | 0.331776 | 0.481837 | 0.001712 | 0.001894 | 0.001844 |
| test_nil_generator.test | RUN | 0.150133 | 0.386917 | 0.530428 | 0.001525 | 0.001835 | 0.001804 |
| test_string_generator.test | RUN | 0.180248 | 0.680822 | 0.766867 | 0.001832 | 0.002168 | 0.001797 |
| test_string_generator_2.test | RUN | 0.202431 | 0.978893 | 0.836539 | 0.001521 | 0.001647 | 0.001784 |
| test_random_generator.test | RUN | 0.316417 | 1.774928 | 1.250041 | 0.002416 | 0.002638 | 0.002740 |
| test_random_generator_2.test | RUN | 0.221539 | 0.688477 | 0.879739 | 0.001430 | 0.001940 | 0.001505 |
| test_name_generator.test | RUN | 0.195316 | 2.314691 | 0.928596 | 0.001502 | 0.001898 | 0.001556 |
| test_namespaces.test | RUN | 0.159139 | 0.502511 | 0.588731 | 0.001520 | 0.002005 | 0.001621 |
| test_name_generator_md5.test | RUN | 0.198187 | 1.099705 | 0.891119 | 0.001623 | 0.001893 | 0.001881 |
| test_name_generator_sha1.test | RUN | 0.190701 | 3.280450 | 0.858377 | 0.001489 | 0.002056 | 0.001824 |
| test_time_generator.test | RUN | 0.224457 | 0.678731 | 0.851080 | 0.001538 | 0.001929 | 0.001665 |
| test_time_generator_v1.test | RUN | 0.248852 | 0.750584 | 0.957823 | 0.002038 | 0.002415 | 0.002444 |
| test_time_generator_v1_2.test | RUN | 0.246275 | 0.697057 | 0.898307 | 0.001983 | 0.002272 | 0.002214 |
| test_time_generator_v1_3.test | RUN | 0.270818 | 0.743910 | 0.965982 | 0.003753 | 0.004792 | 0.004750 |
| test_time_generator_v6.test | RUN | 0.251503 | 0.756187 | 0.969409 | 0.002023 | 0.002434 | 0.001956 |
| test_time_generator_v6_2.test | RUN | 0.248659 | 0.739929 | 0.923095 | 0.001710 | 0.002265 | 0.002086 |
| test_time_generator_v6_3.test | RUN | 0.272286 | 0.773876 | 1.004800 | 0.003626 | 0.004792 | 0.005236 |
| test_time_generator_v7.test | RUN | 0.238651 | 0.781581 | 0.924161 | 0.001757 | 0.002136 | 0.001766 |
| test_time_generator_v7_2.test | RUN | 0.225991 | 0.671460 | 0.817975 | 0.001421 | 0.001885 | 0.001695 |
| test_time_generator_v7_3.test | RUN | 0.221591 | 0.846216 | 0.884960 | 0.002160 | 0.002697 | 0.002429 |
| test_serialization.test | RUN | 0.420607 | 1.335396 | 1.527652 | 0.001992 | 0.001862 | 0.001990 |
| test_tagging.test | RUN | 0.228957 | 0.813816 | 0.931319 | 0.001691 | 0.001605 | 0.001425 |
| test_uuid_class.test | RUN | 0.235813 | 0.699852 | 0.906882 | 0.001624 | 0.001612 | 0.001920 |
| test_uuid_in_map.test | RUN | 0.260662 | 0.845083 | 1.055718 | 0.001448 | 0.002104 | 0.001769 |
| test_hash.test | RUN | 0.268839 | 0.751470 | 0.936956 | 0.001684 | 0.001904 | 0.001886 |
| test_hash_value.test | RUN | 0.183462 | 0.512773 | 0.703106 | 0.004837 | 0.004882 | 0.004901 |
| test_std_unordered.test | RUN | 0.243666 | 0.845408 | 0.921140 | 0.001790 | 0.001888 | 0.001730 |
| test_boost_unordered.test | RUN | 0.350432 | 1.073726 | 1.201355 | 0.001786 | 0.001625 | 0.001834 |
| test_detail_md5.test | RUN | 0.159081 | 0.535241 | 0.650691 | 0.001594 | 0.001939 | 0.001696 |
| test_detail_sha1.test | RUN | 0.157505 | 0.652315 | 0.615963 | 0.009903 | 0.009277 | 0.008446 |
| test_detail_endian.test | RUN | 0.120663 | 0.379902 | 0.435357 | 0.001635 | 0.001745 | 0.001677 |
| test_detail_chacha20.test | RUN | 0.128216 | 0.558421 | 0.456919 | 0.001525 | 0.001753 | 0.001810 |
| random_provider_no_copy_ctor.test | COMPILE_FAIL | 0.117439 | 0.337849 | 0.378799 | — | — | — |
| random_provider_no_copy_assign.test | COMPILE_FAIL | 0.116883 | 0.321700 | 0.377522 | — | — | — |
| test_msvc_simd_bug981648.test | RUN | 0.194331 | 0.598098 | 0.666683 | 0.001713 | 0.001877 | 0.001835 |
| test_bench_random.test | RUN | 0.223940 | 0.679125 | 0.895105 | 0.005596 | 0.005937 | 0.005431 |
| test_entropy_error.test | RUN | 0.113696 | 0.320422 | 0.410395 | 0.001544 | 0.001702 | 0.001623 |
| test_detail_random_provider.test | RUN | 0.213161 | 0.537920 | 0.816538 | 0.001463 | 0.001636 | 0.001471 |
| test_constants.test | RUN | 0.157032 | 0.499953 | 0.642647 | 0.001475 | 0.001943 | 0.001622 |
| test_uuid_cx.test | COMPILE | 0.098297 | 0.226976 | 0.337788 | — | — | — |
| test_uuid_cx2.test | RUN | 0.168362 | 0.561921 | 0.692210 | 0.001726 | 0.001779 | 0.001974 |
| test_uuid_cx3.test | RUN | 0.153924 | 0.438681 | 0.637272 | 0.001674 | 0.001824 | 0.001476 |
| test_uuid_cx3_no_simd.test | RUN | 0.140678 | 0.406296 | 0.568235 | 0.001355 | 0.001812 | 0.001796 |
| test_hash_value_cx.test | RUN | 0.144958 | 0.396113 | 0.531065 | 0.001330 | 0.001908 | 0.001608 |
| test_string_generator_cx.test | RUN | 0.162071 | 0.506214 | 0.627738 | 0.001708 | 0.001542 | 0.001861 |
| test_string_generator_cx2.test | RUN | 0.174582 | 0.615901 | 0.743735 | 0.001586 | 0.001862 | 0.001486 |
| test_constants_cx.test | RUN | 0.139274 | 0.359584 | 0.509807 | 0.001512 | 0.001524 | 0.001882 |
| test_from_chars_cx.test | RUN | 0.177994 | 0.674191 | 0.704630 | 0.001452 | 0.001755 | 0.001752 |
| test_from_chars_cx2.test | RUN | 0.186748 | 0.798995 | 0.766852 | 0.001751 | 0.001768 | 0.001850 |
| test_to_chars_cx.test | RUN | 0.183044 | 0.763775 | 0.779185 | 0.001680 | 0.001618 | 0.001427 |
| test_uuid_from_string_cx.test | RUN | 0.201060 | 0.747788 | 0.787703 | 0.001379 | 0.001352 | 0.001252 |
| quick.test | RUN | 0.213999 | 0.566258 | 0.792331 | 0.001245 | 0.001329 | 0.001287 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.001× | 1.004×, 0.998× | 1.002× |
| clang | 0.991× | 0.988×, 0.994× | 0.983× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737952).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| quick.test | 10 | 0.001150s | 0.001157s | 0.001165s | byte equality observed |
| test_alignment.test | 10 | 0.001175s | 0.001191s | 0.001193s | byte equality observed |
| test_alignment_2.test | 10 | 0.001169s | 0.001168s | 0.001175s | byte equality observed |
| test_attribute_packed.test | 10 | 0.001142s | 0.001186s | 0.001179s | byte equality observed |
| test_bench_random.test | 4 | 0.005254s | 0.005203s | 0.005183s | native test oracle; output varies |
| test_boost_unordered.test | 10 | 0.001239s | 0.001225s | 0.001236s | byte equality observed |
| test_comparison.test | 10 | 0.001224s | 0.001186s | 0.001207s | byte equality observed |
| test_comparison_no_simd.test | 10 | 0.001222s | 0.001211s | 0.001197s | byte equality observed |
| test_constants.test | 10 | 0.001181s | 0.001193s | 0.001181s | byte equality observed |
| test_constants_cx.test | 10 | 0.001123s | 0.001155s | 0.001148s | byte equality observed |
| test_data.test | 10 | 0.001164s | 0.001177s | 0.001150s | byte equality observed |
| test_detail_chacha20.test | 10 | 0.001158s | 0.001185s | 0.001149s | byte equality observed |
| test_detail_endian.test | 10 | 0.001176s | 0.001133s | 0.001170s | byte equality observed |
| test_detail_md5.test | 10 | 0.001197s | 0.001170s | 0.001183s | byte equality observed |
| test_detail_random_provider.test | 10 | 0.001180s | 0.001179s | 0.001188s | byte equality observed |
| test_detail_sha1.test | 3 | 0.008854s | 0.008468s | 0.007314s | byte equality observed |
| test_entropy_error.test | 10 | 0.001180s | 0.001148s | 0.001127s | byte equality observed |
| test_from_chars.test | 10 | 0.001143s | 0.001136s | 0.001168s | byte equality observed |
| test_from_chars_2.test | 10 | 0.001144s | 0.001173s | 0.001167s | byte equality observed |
| test_from_chars_2_no_simd.test | 10 | 0.001191s | 0.001152s | 0.001195s | byte equality observed |
| test_from_chars_cx.test | 10 | 0.001181s | 0.001163s | 0.001169s | byte equality observed |
| test_from_chars_cx2.test | 10 | 0.001181s | 0.001157s | 0.001154s | byte equality observed |
| test_from_chars_no_simd.test | 10 | 0.001163s | 0.001177s | 0.001153s | byte equality observed |
| test_hash.test | 10 | 0.001205s | 0.001178s | 0.001180s | byte equality observed |
| test_hash_value.test | 5 | 0.004293s | 0.004114s | 0.004118s | byte equality observed |
| test_hash_value_cx.test | 10 | 0.001160s | 0.001169s | 0.001168s | byte equality observed |
| test_include1.test | 10 | 0.000512s | 0.000496s | 0.001132s | byte equality observed |
| test_io.test | 10 | 0.001256s | 0.001235s | 0.001276s | byte equality observed |
| test_io_2.test | 10 | 0.001307s | 0.001269s | 0.001315s | byte equality observed |
| test_io_2_no_simd.test | 10 | 0.001319s | 0.001277s | 0.001321s | byte equality observed |
| test_io_no_simd.test | 10 | 0.001253s | 0.001191s | 0.001278s | byte equality observed |
| test_msvc_simd_bug981648.test | 10 | 0.001178s | 0.001148s | 0.001194s | native test oracle; output varies |
| test_name_generator.test | 10 | 0.001220s | 0.001187s | 0.001213s | byte equality observed |
| test_name_generator_md5.test | 10 | 0.001215s | 0.001198s | 0.001190s | byte equality observed |
| test_name_generator_sha1.test | 10 | 0.001230s | 0.001224s | 0.001202s | byte equality observed |
| test_namespaces.test | 10 | 0.001160s | 0.001173s | 0.001170s | byte equality observed |
| test_nil_generator.test | 10 | 0.001164s | 0.001153s | 0.001160s | byte equality observed |
| test_pragma_pack.test | 10 | 0.001181s | 0.001190s | 0.001157s | byte equality observed |
| test_random_generator.test | 8 | 0.002036s | 0.002005s | 0.002014s | native test oracle; output varies |
| test_random_generator_2.test | 10 | 0.001184s | 0.001158s | 0.001155s | byte equality observed |
| test_serialization.test | 10 | 0.001288s | 0.001293s | 0.001270s | byte equality observed |
| test_std_unordered.test | 10 | 0.001233s | 0.001241s | 0.001263s | byte equality observed |
| test_string_generator.test | 10 | 0.001260s | 0.001287s | 0.001261s | byte equality observed |
| test_string_generator_2.test | 10 | 0.001200s | 0.001196s | 0.001175s | byte equality observed |
| test_string_generator_cx.test | 10 | 0.001178s | 0.001181s | 0.001175s | byte equality observed |
| test_string_generator_cx2.test | 10 | 0.001151s | 0.001208s | 0.001141s | byte equality observed |
| test_tagging.test | 10 | 0.001184s | 0.001175s | 0.001166s | byte equality observed |
| test_time_generator.test | 10 | 0.001208s | 0.001208s | 0.001191s | byte equality observed |
| test_time_generator_v1.test | 10 | 0.001593s | 0.001734s | 0.001729s | byte equality observed |
| test_time_generator_v1_2.test | 10 | 0.001421s | 0.001485s | 0.001492s | byte equality observed |
| test_time_generator_v1_3.test | 6 | 0.003616s | 0.003731s | 0.003677s | byte equality observed |
| test_time_generator_v6.test | 10 | 0.001609s | 0.001765s | 0.001751s | byte equality observed |
| test_time_generator_v6_2.test | 10 | 0.001424s | 0.001448s | 0.001473s | byte equality observed |
| test_time_generator_v6_3.test | 6 | 0.003779s | 0.003715s | 0.003792s | byte equality observed |
| test_time_generator_v7.test | 10 | 0.001433s | 0.001476s | 0.001472s | byte equality observed |
| test_time_generator_v7_2.test | 10 | 0.001240s | 0.001266s | 0.001224s | byte equality observed |
| test_time_generator_v7_3.test | 10 | 0.001839s | 0.001832s | 0.001832s | byte equality observed |
| test_to_chars.test | 10 | 0.001229s | 0.001250s | 0.001230s | byte equality observed |
| test_to_chars_2.test | 10 | 0.001197s | 0.001177s | 0.001217s | byte equality observed |
| test_to_chars_2_no_simd.test | 10 | 0.001192s | 0.001209s | 0.001201s | byte equality observed |
| test_to_chars_3.test | 10 | 0.001175s | 0.001209s | 0.001160s | byte equality observed |
| test_to_chars_cx.test | 10 | 0.001192s | 0.001182s | 0.001216s | byte equality observed |
| test_to_chars_no_simd.test | 10 | 0.001249s | 0.001234s | 0.001224s | byte equality observed |
| test_uuid.test | 10 | 0.001164s | 0.001168s | 0.001137s | byte equality observed |
| test_uuid_2.test | 10 | 0.001183s | 0.001178s | 0.001156s | byte equality observed |
| test_uuid_3.test | 10 | 0.001161s | 0.001191s | 0.001192s | byte equality observed |
| test_uuid_class.test | 10 | 0.001189s | 0.001214s | 0.001174s | byte equality observed |
| test_uuid_clock.test | 10 | 0.001151s | 0.001185s | 0.001152s | byte equality observed |
| test_uuid_cx2.test | 10 | 0.001177s | 0.001188s | 0.001184s | byte equality observed |
| test_uuid_cx3.test | 10 | 0.001142s | 0.001177s | 0.001122s | byte equality observed |
| test_uuid_cx3_no_simd.test | 10 | 0.001145s | 0.001177s | 0.001159s | byte equality observed |
| test_uuid_from_string.test | 10 | 0.001181s | 0.001166s | 0.001173s | byte equality observed |
| test_uuid_from_string_2.test | 10 | 0.001388s | 0.001432s | 0.001520s | byte equality observed |
| test_uuid_from_string_3.test | 10 | 0.001257s | 0.001242s | 0.001271s | byte equality observed |
| test_uuid_from_string_cx.test | 10 | 0.001156s | 0.001178s | 0.001165s | byte equality observed |
| test_uuid_in_map.test | 10 | 0.001163s | 0.001189s | 0.001161s | byte equality observed |
| test_uuid_no_simd.test | 10 | 0.001178s | 0.001151s | 0.001114s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737952).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
