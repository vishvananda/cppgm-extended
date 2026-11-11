# libs/uuid/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_include1.test | RUN | 0.465278 | 0.988846 | 1.439827 | 0.000807 | 0.000824 | 0.001675 |
| test_uuid.test | RUN | 0.300329 | 0.776677 | 1.023083 | 0.001796 | 0.001870 | 0.001756 |
| test_uuid_no_simd.test | RUN | 0.277390 | 0.751958 | 0.870874 | 0.001786 | 0.001934 | 0.001593 |
| test_uuid_2.test | RUN | 0.201820 | 0.585044 | 0.665582 | 0.001544 | 0.001732 | 0.001728 |
| test_uuid_3.test | RUN | 0.168599 | 0.399635 | 0.560823 | 0.001888 | 0.001891 | 0.001199 |
| test_alignment.test | RUN | 0.157912 | 0.367391 | 0.465928 | 0.001771 | 0.001471 | 0.001713 |
| test_alignment_2.test | RUN | 0.157744 | 0.342247 | 0.505316 | 0.001570 | 0.001994 | 0.001542 |
| test_attribute_packed.test | RUN | 0.168676 | 0.324573 | 0.469448 | 0.001843 | 0.001946 | 0.001602 |
| test_pragma_pack.test | RUN | 0.161377 | 0.350447 | 0.507870 | 0.001859 | 0.001944 | 0.001518 |
| test_data.test | RUN | 0.201202 | 0.646359 | 0.668103 | 0.001816 | 0.001944 | 0.001762 |
| test_comparison.test | RUN | 0.260232 | 0.761785 | 0.964542 | 0.001678 | 0.002077 | 0.001601 |
| test_comparison_no_simd.test | RUN | 0.259118 | 0.820884 | 0.794237 | 0.001597 | 0.001576 | 0.001704 |
| test_io.test | RUN | 0.313853 | 1.263603 | 1.367760 | 0.001806 | 0.002152 | 0.001865 |
| test_io_no_simd.test | RUN | 0.302019 | 1.403038 | 1.280762 | 0.001772 | 0.001635 | 0.001758 |
| test_io_2.test | RUN | 0.252885 | 1.452622 | 0.972907 | 0.001764 | 0.002100 | 0.001911 |
| test_io_2_no_simd.test | RUN | 0.218615 | 1.360332 | 0.846138 | 0.001888 | 0.001663 | 0.001599 |
| test_to_chars.test | RUN | 0.258045 | 0.944881 | 0.866422 | 0.001928 | 0.001296 | 0.001881 |
| test_to_chars_no_simd.test | RUN | 0.235656 | 1.196396 | 0.872374 | 0.001895 | 0.003504 | 0.001846 |
| test_to_chars_2.test | RUN | 0.311266 | 2.054598 | 1.077731 | 0.001578 | 0.002019 | 0.001554 |
| test_to_chars_2_no_simd.test | RUN | 0.296364 | 2.174007 | 0.932987 | 0.001667 | 0.002033 | 0.001544 |
| test_to_chars_3.test | RUN | 0.193190 | 0.857168 | 0.680854 | 0.001730 | 0.001987 | 0.001486 |
| test_from_chars.test | RUN | 0.221709 | 0.968638 | 0.756655 | 0.001707 | 0.001642 | 0.001741 |
| test_from_chars_no_simd.test | RUN | 0.191899 | 0.720601 | 0.611045 | 0.001812 | 0.001822 | 0.001670 |
| test_from_chars_2.test | RUN | 0.214876 | 0.843548 | 0.738163 | 0.001688 | 0.002025 | 0.001535 |
| test_from_chars_2_no_simd.test | RUN | 0.185921 | 0.645323 | 0.607913 | 0.001471 | 0.001842 | 0.001733 |
| test_uuid_from_string.test | RUN | 0.258589 | 0.920592 | 0.883279 | 0.001708 | 0.002036 | 0.001428 |
| test_uuid_from_string_2.test | RUN | 0.219970 | 0.839628 | 0.726520 | 0.001924 | 0.002451 | 0.002078 |
| test_uuid_from_string_3.test | RUN | 0.231205 | 0.848296 | 0.790593 | 0.001741 | 0.002144 | 0.001591 |
| test_uuid_clock.test | RUN | 0.148590 | 0.348593 | 0.473540 | 0.001660 | 0.002144 | 0.001616 |
| test_nil_generator.test | RUN | 0.171474 | 0.425283 | 0.564106 | 0.001718 | 0.002114 | 0.001715 |
| test_string_generator.test | RUN | 0.196121 | 0.661278 | 0.738550 | 0.001626 | 0.002083 | 0.001844 |
| test_string_generator_2.test | RUN | 0.220682 | 0.972308 | 0.845621 | 0.001640 | 0.002041 | 0.001791 |
| test_random_generator.test | RUN | 0.355549 | 1.663991 | 1.242771 | 0.002424 | 0.002766 | 0.002544 |
| test_random_generator_2.test | RUN | 0.253821 | 0.705083 | 0.862022 | 0.001790 | 0.002011 | 0.001641 |
| test_name_generator.test | RUN | 0.231011 | 2.386816 | 0.885312 | 0.001541 | 0.001936 | 0.001487 |
| test_namespaces.test | RUN | 0.179807 | 0.516607 | 0.612106 | 0.001429 | 0.002016 | 0.002007 |
| test_name_generator_md5.test | RUN | 0.231432 | 1.100904 | 0.892824 | 0.001616 | 0.001998 | 0.001834 |
| test_name_generator_sha1.test | RUN | 0.240250 | 3.360776 | 0.850092 | 0.001927 | 0.002028 | 0.001807 |
| test_time_generator.test | RUN | 0.249039 | 0.697463 | 0.867617 | 0.001706 | 0.001926 | 0.001489 |
| test_time_generator_v1.test | RUN | 0.283817 | 0.735390 | 0.956315 | 0.002094 | 0.002684 | 0.002286 |
| test_time_generator_v1_2.test | RUN | 0.289865 | 0.710563 | 0.874867 | 0.002092 | 0.002175 | 0.002006 |
| test_time_generator_v1_3.test | RUN | 0.302322 | 0.776835 | 0.976871 | 0.004403 | 0.004479 | 0.004720 |
| test_time_generator_v6.test | RUN | 0.294104 | 0.779218 | 0.951466 | 0.001998 | 0.002651 | 0.002467 |
| test_time_generator_v6_2.test | RUN | 0.281877 | 0.712361 | 0.918502 | 0.001747 | 0.002309 | 0.002044 |
| test_time_generator_v6_3.test | RUN | 0.300921 | 0.781859 | 0.957062 | 0.004508 | 0.004999 | 0.004813 |
| test_time_generator_v7.test | RUN | 0.269728 | 0.794458 | 0.911688 | 0.001822 | 0.002239 | 0.002010 |
| test_time_generator_v7_2.test | RUN | 0.254613 | 0.666988 | 0.810302 | 0.001658 | 0.002163 | 0.001735 |
| test_time_generator_v7_3.test | RUN | 0.252634 | 0.861465 | 0.854819 | 0.002132 | 0.002544 | 0.002542 |
| test_serialization.test | RUN | 0.462198 | 1.298736 | 1.496123 | 0.002066 | 0.001649 | 0.001924 |
| test_tagging.test | RUN | 0.254168 | 0.792331 | 0.867780 | 0.001732 | 0.002080 | 0.001881 |
| test_uuid_class.test | RUN | 0.262928 | 0.708848 | 0.879084 | 0.001546 | 0.001816 | 0.001796 |
| test_uuid_in_map.test | RUN | 0.291909 | 0.808127 | 0.984371 | 0.001608 | 0.001794 | 0.001738 |
| test_hash.test | RUN | 0.294730 | 0.777788 | 0.907378 | 0.001721 | 0.001686 | 0.002016 |
| test_hash_value.test | RUN | 0.207715 | 0.532736 | 0.676089 | 0.004811 | 0.004973 | 0.004284 |
| test_std_unordered.test | RUN | 0.276414 | 0.824292 | 0.910691 | 0.001654 | 0.001636 | 0.001534 |
| test_boost_unordered.test | RUN | 0.368713 | 1.083963 | 1.201179 | 0.001825 | 0.001774 | 0.001555 |
| test_detail_md5.test | RUN | 0.183618 | 0.524870 | 0.675289 | 0.001718 | 0.001648 | 0.001495 |
| test_detail_sha1.test | RUN | 0.171959 | 0.670387 | 0.614614 | 0.009154 | 0.009064 | 0.008342 |
| test_detail_endian.test | RUN | 0.133285 | 0.388692 | 0.449108 | 0.001534 | 0.001941 | 0.001615 |
| test_detail_chacha20.test | RUN | 0.151409 | 0.462643 | 0.469704 | 0.001837 | 0.001528 | 0.001783 |
| random_provider_no_copy_ctor.test | COMPILE_FAIL | 0.130917 | 0.237302 | 0.343510 | — | — | — |
| random_provider_no_copy_assign.test | COMPILE_FAIL | 0.133476 | 0.269908 | 0.363888 | — | — | — |
| test_msvc_simd_bug981648.test | RUN | 0.229949 | 0.522486 | 0.681045 | 0.001782 | 0.001956 | 0.001709 |
| test_bench_random.test | RUN | 0.259418 | 0.689984 | 0.894751 | 0.005829 | 0.005966 | 0.005394 |
| test_entropy_error.test | RUN | 0.131497 | 0.323221 | 0.398420 | 0.001410 | 0.001892 | 0.001638 |
| test_detail_random_provider.test | RUN | 0.250379 | 0.543394 | 0.799741 | 0.001757 | 0.001744 | 0.001506 |
| test_constants.test | RUN | 0.182482 | 0.491005 | 0.632151 | 0.001392 | 0.001885 | 0.001444 |
| test_uuid_cx.test | COMPILE | 0.116081 | 0.235110 | 0.336648 | — | — | — |
| test_uuid_cx2.test | RUN | 0.197865 | 0.567568 | 0.683175 | 0.001731 | 0.001798 | 0.001760 |
| test_uuid_cx3.test | RUN | 0.175967 | 0.449191 | 0.584330 | 0.001233 | 0.001262 | 0.001428 |
| test_uuid_cx3_no_simd.test | RUN | 0.159182 | 0.412011 | 0.535011 | 0.001436 | 0.001701 | 0.001553 |
| test_hash_value_cx.test | RUN | 0.166248 | 0.408115 | 0.548183 | 0.001637 | 0.001812 | 0.001482 |
| test_string_generator_cx.test | RUN | 0.180832 | 0.498679 | 0.598498 | 0.001768 | 0.001503 | 0.001820 |
| test_string_generator_cx2.test | RUN | 0.187785 | 0.611467 | 0.689910 | 0.001539 | 0.001870 | 0.001920 |
| test_constants_cx.test | RUN | 0.164420 | 0.356095 | 0.510062 | 0.001223 | 0.001896 | 0.001782 |
| test_from_chars_cx.test | RUN | 0.194333 | 0.665303 | 0.659583 | 0.001496 | 0.001823 | 0.001752 |
| test_from_chars_cx2.test | RUN | 0.208929 | 0.800197 | 0.703477 | 0.001721 | 0.001462 | 0.001732 |
| test_to_chars_cx.test | RUN | 0.206541 | 0.739627 | 0.764086 | 0.001791 | 0.001620 | 0.001899 |
| test_uuid_from_string_cx.test | RUN | 0.208392 | 0.758135 | 0.648699 | 0.001299 | 0.001507 | 0.001250 |
| quick.test | RUN | 0.239958 | 0.621797 | 0.693505 | 0.001200 | 0.001366 | 0.001242 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.012× | 0.994×, 1.030× | 0.979× |
| clang | 1.004× | 1.016×, 0.992× | 0.984× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| quick.test | 10 | 0.001179s | 0.001186s | 0.001149s | byte equality observed |
| test_alignment.test | 10 | 0.001199s | 0.001209s | 0.001191s | byte equality observed |
| test_alignment_2.test | 10 | 0.001176s | 0.001192s | 0.001176s | byte equality observed |
| test_attribute_packed.test | 10 | 0.001174s | 0.001209s | 0.001206s | byte equality observed |
| test_bench_random.test | 4 | 0.005264s | 0.005183s | 0.005145s | native test oracle; output varies |
| test_boost_unordered.test | 10 | 0.001247s | 0.001261s | 0.001189s | byte equality observed |
| test_comparison.test | 10 | 0.001214s | 0.001240s | 0.001242s | byte equality observed |
| test_comparison_no_simd.test | 10 | 0.001214s | 0.001209s | 0.001233s | byte equality observed |
| test_constants.test | 10 | 0.001215s | 0.001201s | 0.001195s | byte equality observed |
| test_constants_cx.test | 10 | 0.001180s | 0.001169s | 0.001163s | byte equality observed |
| test_data.test | 10 | 0.001188s | 0.001182s | 0.001204s | byte equality observed |
| test_detail_chacha20.test | 10 | 0.001195s | 0.001188s | 0.001166s | byte equality observed |
| test_detail_endian.test | 10 | 0.001188s | 0.001196s | 0.001184s | byte equality observed |
| test_detail_md5.test | 10 | 0.001204s | 0.001229s | 0.001196s | byte equality observed |
| test_detail_random_provider.test | 10 | 0.001207s | 0.001193s | 0.001180s | byte equality observed |
| test_detail_sha1.test | 3 | 0.007907s | 0.008498s | 0.007782s | byte equality observed |
| test_entropy_error.test | 10 | 0.001167s | 0.001175s | 0.001174s | byte equality observed |
| test_from_chars.test | 10 | 0.001179s | 0.001156s | 0.001173s | byte equality observed |
| test_from_chars_2.test | 10 | 0.001183s | 0.001184s | 0.001207s | byte equality observed |
| test_from_chars_2_no_simd.test | 10 | 0.001193s | 0.001194s | 0.001194s | byte equality observed |
| test_from_chars_cx.test | 10 | 0.001175s | 0.001209s | 0.001185s | byte equality observed |
| test_from_chars_cx2.test | 10 | 0.001194s | 0.001190s | 0.001156s | byte equality observed |
| test_from_chars_no_simd.test | 10 | 0.001210s | 0.001196s | 0.001190s | byte equality observed |
| test_hash.test | 10 | 0.001205s | 0.001205s | 0.001153s | byte equality observed |
| test_hash_value.test | 5 | 0.004382s | 0.004120s | 0.004072s | byte equality observed |
| test_hash_value_cx.test | 10 | 0.001192s | 0.001186s | 0.001126s | byte equality observed |
| test_include1.test | 10 | 0.000538s | 0.000541s | 0.001177s | byte equality observed |
| test_io.test | 10 | 0.001273s | 0.001297s | 0.001286s | byte equality observed |
| test_io_2.test | 10 | 0.001321s | 0.001312s | 0.001349s | byte equality observed |
| test_io_2_no_simd.test | 10 | 0.001354s | 0.001341s | 0.001355s | byte equality observed |
| test_io_no_simd.test | 10 | 0.001290s | 0.001274s | 0.001262s | byte equality observed |
| test_msvc_simd_bug981648.test | 10 | 0.001213s | 0.001208s | 0.001177s | native test oracle; output varies |
| test_name_generator.test | 10 | 0.001211s | 0.001252s | 0.001230s | byte equality observed |
| test_name_generator_md5.test | 10 | 0.001223s | 0.001233s | 0.001228s | byte equality observed |
| test_name_generator_sha1.test | 10 | 0.001239s | 0.001265s | 0.001258s | byte equality observed |
| test_namespaces.test | 10 | 0.001183s | 0.001186s | 0.001206s | byte equality observed |
| test_nil_generator.test | 10 | 0.001157s | 0.001201s | 0.001198s | byte equality observed |
| test_pragma_pack.test | 10 | 0.001188s | 0.001239s | 0.001230s | byte equality observed |
| test_random_generator.test | 9 | 0.002051s | 0.002039s | 0.002027s | native test oracle; output varies |
| test_random_generator_2.test | 10 | 0.001212s | 0.001186s | 0.001201s | byte equality observed |
| test_serialization.test | 10 | 0.001339s | 0.001313s | 0.001295s | byte equality observed |
| test_std_unordered.test | 10 | 0.001289s | 0.001283s | 0.001285s | byte equality observed |
| test_string_generator.test | 10 | 0.001335s | 0.001306s | 0.001309s | byte equality observed |
| test_string_generator_2.test | 10 | 0.001229s | 0.001209s | 0.001239s | byte equality observed |
| test_string_generator_cx.test | 10 | 0.001196s | 0.001191s | 0.001220s | byte equality observed |
| test_string_generator_cx2.test | 10 | 0.001209s | 0.001215s | 0.001187s | byte equality observed |
| test_tagging.test | 10 | 0.001203s | 0.001202s | 0.001182s | byte equality observed |
| test_time_generator.test | 10 | 0.001226s | 0.001233s | 0.001228s | byte equality observed |
| test_time_generator_v1.test | 10 | 0.001625s | 0.001749s | 0.001765s | byte equality observed |
| test_time_generator_v1_2.test | 10 | 0.001427s | 0.001488s | 0.001515s | byte equality observed |
| test_time_generator_v1_3.test | 5 | 0.003777s | 0.003815s | 0.003592s | byte equality observed |
| test_time_generator_v6.test | 10 | 0.001643s | 0.001750s | 0.001777s | byte equality observed |
| test_time_generator_v6_2.test | 9 | 0.001436s | 0.001497s | 0.001504s | byte equality observed |
| test_time_generator_v6_3.test | 5 | 0.003745s | 0.003652s | 0.003836s | byte equality observed |
| test_time_generator_v7.test | 10 | 0.001480s | 0.001542s | 0.001482s | byte equality observed |
| test_time_generator_v7_2.test | 10 | 0.001308s | 0.001317s | 0.001256s | byte equality observed |
| test_time_generator_v7_3.test | 10 | 0.001859s | 0.001901s | 0.001815s | byte equality observed |
| test_to_chars.test | 10 | 0.001271s | 0.001293s | 0.001291s | byte equality observed |
| test_to_chars_2.test | 10 | 0.001232s | 0.001245s | 0.001221s | byte equality observed |
| test_to_chars_2_no_simd.test | 10 | 0.001235s | 0.001229s | 0.001234s | byte equality observed |
| test_to_chars_3.test | 10 | 0.001190s | 0.001203s | 0.001236s | byte equality observed |
| test_to_chars_cx.test | 10 | 0.001208s | 0.001154s | 0.001219s | byte equality observed |
| test_to_chars_no_simd.test | 10 | 0.001275s | 0.001228s | 0.001300s | byte equality observed |
| test_uuid.test | 10 | 0.001202s | 0.001157s | 0.001186s | byte equality observed |
| test_uuid_2.test | 10 | 0.001200s | 0.001211s | 0.001202s | byte equality observed |
| test_uuid_3.test | 10 | 0.001192s | 0.001205s | 0.001223s | byte equality observed |
| test_uuid_class.test | 10 | 0.001209s | 0.001223s | 0.001207s | byte equality observed |
| test_uuid_clock.test | 10 | 0.001190s | 0.001206s | 0.001201s | byte equality observed |
| test_uuid_cx2.test | 10 | 0.001202s | 0.001201s | 0.001211s | byte equality observed |
| test_uuid_cx3.test | 10 | 0.001189s | 0.001202s | 0.001196s | byte equality observed |
| test_uuid_cx3_no_simd.test | 10 | 0.001209s | 0.001191s | 0.001186s | byte equality observed |
| test_uuid_from_string.test | 10 | 0.001201s | 0.001216s | 0.001214s | byte equality observed |
| test_uuid_from_string_2.test | 10 | 0.001469s | 0.001483s | 0.001510s | byte equality observed |
| test_uuid_from_string_3.test | 10 | 0.001302s | 0.001279s | 0.001246s | byte equality observed |
| test_uuid_from_string_cx.test | 10 | 0.001186s | 0.001203s | 0.001184s | byte equality observed |
| test_uuid_in_map.test | 10 | 0.001207s | 0.001207s | 0.001189s | byte equality observed |
| test_uuid_no_simd.test | 10 | 0.001181s | 0.001180s | 0.001174s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
