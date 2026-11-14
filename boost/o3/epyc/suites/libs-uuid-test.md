# libs/uuid/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_include1.test | RUN | 0.419969 | 0.951769 | 1.442756 | 0.000703 | 0.000751 | 0.001680 |
| test_uuid.test | RUN | 0.242903 | 0.759083 | 1.032100 | 0.001807 | 0.001827 | 0.001766 |
| test_uuid_no_simd.test | RUN | 0.250153 | 0.725830 | 0.880552 | 0.001619 | 0.001643 | 0.001520 |
| test_uuid_2.test | RUN | 0.176004 | 0.559244 | 0.671456 | 0.001704 | 0.001830 | 0.001510 |
| test_uuid_3.test | RUN | 0.150059 | 0.365813 | 0.563322 | 0.001421 | 0.001900 | 0.001589 |
| test_alignment.test | RUN | 0.136397 | 0.344623 | 0.468486 | 0.001686 | 0.001656 | 0.001834 |
| test_alignment_2.test | RUN | 0.141484 | 0.354954 | 0.507088 | 0.001668 | 0.001690 | 0.001688 |
| test_attribute_packed.test | RUN | 0.138136 | 0.359702 | 0.478673 | 0.001486 | 0.001854 | 0.001395 |
| test_pragma_pack.test | RUN | 0.139501 | 0.350748 | 0.509936 | 0.001767 | 0.001961 | 0.001411 |
| test_data.test | RUN | 0.172698 | 0.633670 | 0.681121 | 0.001554 | 0.001588 | 0.001760 |
| test_comparison.test | RUN | 0.232076 | 0.737853 | 0.958204 | 0.001683 | 0.001817 | 0.001318 |
| test_comparison_no_simd.test | RUN | 0.220794 | 0.833216 | 0.870241 | 0.001737 | 0.001571 | 0.001840 |
| test_io.test | RUN | 0.297517 | 1.390305 | 1.316708 | 0.001990 | 0.001962 | 0.001802 |
| test_io_no_simd.test | RUN | 0.269455 | 1.349260 | 1.287817 | 0.001626 | 0.001802 | 0.001679 |
| test_io_2.test | RUN | 0.235783 | 1.401747 | 0.937262 | 0.001787 | 0.001710 | 0.001930 |
| test_io_2_no_simd.test | RUN | 0.208651 | 1.323425 | 0.895213 | 0.001862 | 0.001754 | 0.001992 |
| test_to_chars.test | RUN | 0.210903 | 0.896486 | 0.906450 | 0.001874 | 0.001953 | 0.001734 |
| test_to_chars_no_simd.test | RUN | 0.191447 | 0.903890 | 0.845590 | 0.001996 | 0.001789 | 0.001904 |
| test_to_chars_2.test | RUN | 0.244946 | 1.610863 | 1.060505 | 0.001685 | 0.001782 | 0.001285 |
| test_to_chars_2_no_simd.test | RUN | 0.228122 | 1.737234 | 0.958650 | 0.001224 | 0.001728 | 0.001764 |
| test_to_chars_3.test | RUN | 0.176593 | 0.599902 | 0.675952 | 0.001589 | 0.001630 | 0.001680 |
| test_from_chars.test | RUN | 0.195393 | 0.850888 | 0.738806 | 0.001700 | 0.001930 | 0.001828 |
| test_from_chars_no_simd.test | RUN | 0.176399 | 0.674375 | 0.637185 | 0.001585 | 0.001546 | 0.001489 |
| test_from_chars_2.test | RUN | 0.190520 | 0.804915 | 0.746243 | 0.001547 | 0.001629 | 0.001787 |
| test_from_chars_2_no_simd.test | RUN | 0.174171 | 0.608144 | 0.572508 | 0.001446 | 0.001676 | 0.001419 |
| test_uuid_from_string.test | RUN | 0.224978 | 0.879144 | 0.881696 | 0.001769 | 0.001693 | 0.001466 |
| test_uuid_from_string_2.test | RUN | 0.199090 | 0.795618 | 0.737919 | 0.001726 | 0.002020 | 0.002035 |
| test_uuid_from_string_3.test | RUN | 0.200326 | 0.799457 | 0.790771 | 0.001360 | 0.001750 | 0.001946 |
| test_uuid_clock.test | RUN | 0.125168 | 0.330938 | 0.476959 | 0.001451 | 0.001836 | 0.001591 |
| test_nil_generator.test | RUN | 0.147779 | 0.401241 | 0.530417 | 0.001741 | 0.001784 | 0.001844 |
| test_string_generator.test | RUN | 0.191648 | 0.612171 | 0.760419 | 0.001656 | 0.002042 | 0.001854 |
| test_string_generator_2.test | RUN | 0.233500 | 0.902409 | 0.830100 | 0.001583 | 0.001704 | 0.001746 |
| test_random_generator.test | RUN | 0.311960 | 1.726375 | 1.234208 | 0.002500 | 0.002599 | 0.002662 |
| test_random_generator_2.test | RUN | 0.230891 | 0.681996 | 0.879647 | 0.001362 | 0.001966 | 0.001607 |
| test_name_generator.test | RUN | 0.202539 | 2.225107 | 0.930812 | 0.001377 | 0.001910 | 0.001593 |
| test_namespaces.test | RUN | 0.166756 | 0.487075 | 0.589226 | 0.001579 | 0.001848 | 0.001835 |
| test_name_generator_md5.test | RUN | 0.207790 | 1.068758 | 0.868874 | 0.001657 | 0.001848 | 0.001977 |
| test_name_generator_sha1.test | RUN | 0.201290 | 3.253298 | 0.871129 | 0.001595 | 0.001982 | 0.001424 |
| test_time_generator.test | RUN | 0.222578 | 0.667366 | 0.820702 | 0.001514 | 0.001816 | 0.001425 |
| test_time_generator_v1.test | RUN | 0.250330 | 0.738062 | 0.954922 | 0.002042 | 0.002441 | 0.002347 |
| test_time_generator_v1_2.test | RUN | 0.238407 | 0.681512 | 0.862074 | 0.001826 | 0.002226 | 0.002160 |
| test_time_generator_v1_3.test | RUN | 0.265246 | 0.732198 | 1.009308 | 0.003897 | 0.005458 | 0.005193 |
| test_time_generator_v6.test | RUN | 0.246272 | 0.751444 | 0.922691 | 0.001886 | 0.002362 | 0.002310 |
| test_time_generator_v6_2.test | RUN | 0.239266 | 0.690192 | 0.918527 | 0.001730 | 0.002154 | 0.002075 |
| test_time_generator_v6_3.test | RUN | 0.266586 | 0.739210 | 0.998798 | 0.004230 | 0.003754 | 0.004730 |
| test_time_generator_v7.test | RUN | 0.235313 | 0.729482 | 0.891146 | 0.001479 | 0.002094 | 0.002041 |
| test_time_generator_v7_2.test | RUN | 0.219967 | 0.657493 | 0.852482 | 0.001582 | 0.001908 | 0.001732 |
| test_time_generator_v7_3.test | RUN | 0.218845 | 0.833171 | 0.876563 | 0.002190 | 0.002509 | 0.002566 |
| test_serialization.test | RUN | 0.412490 | 1.242925 | 1.509666 | 0.002009 | 0.001729 | 0.001949 |
| test_tagging.test | RUN | 0.227330 | 0.800206 | 0.892580 | 0.001681 | 0.001739 | 0.001452 |
| test_uuid_class.test | RUN | 0.234385 | 0.698734 | 0.932138 | 0.001452 | 0.001648 | 0.001555 |
| test_uuid_in_map.test | RUN | 0.257326 | 0.802415 | 0.979459 | 0.001665 | 0.001725 | 0.001846 |
| test_hash.test | RUN | 0.260156 | 0.735538 | 0.925478 | 0.001810 | 0.001762 | 0.001796 |
| test_hash_value.test | RUN | 0.185057 | 0.495987 | 0.688180 | 0.004642 | 0.004901 | 0.004763 |
| test_std_unordered.test | RUN | 0.247068 | 0.816140 | 0.921668 | 0.001525 | 0.001793 | 0.001894 |
| test_boost_unordered.test | RUN | 0.344219 | 1.063527 | 1.195005 | 0.001586 | 0.001839 | 0.001570 |
| test_detail_md5.test | RUN | 0.157783 | 0.515752 | 0.684926 | 0.001617 | 0.001618 | 0.001820 |
| test_detail_sha1.test | RUN | 0.150912 | 0.642499 | 0.611813 | 0.008970 | 0.009273 | 0.008590 |
| test_detail_endian.test | RUN | 0.117180 | 0.379134 | 0.451268 | 0.001583 | 0.001830 | 0.001782 |
| test_detail_chacha20.test | RUN | 0.129104 | 0.448903 | 0.451950 | 0.001546 | 0.001730 | 0.001670 |
| random_provider_no_copy_ctor.test | COMPILE_FAIL | 0.114608 | 0.263613 | 0.369445 | — | — | — |
| random_provider_no_copy_assign.test | COMPILE_FAIL | 0.118106 | 0.266378 | 0.368492 | — | — | — |
| test_msvc_simd_bug981648.test | RUN | 0.200721 | 0.528072 | 0.673669 | 0.001755 | 0.001951 | 0.001560 |
| test_bench_random.test | RUN | 0.217613 | 0.682985 | 0.871632 | 0.005575 | 0.005643 | 0.005505 |
| test_entropy_error.test | RUN | 0.111588 | 0.308746 | 0.406044 | 0.001749 | 0.001680 | 0.001646 |
| test_detail_random_provider.test | RUN | 0.213559 | 0.525520 | 0.769967 | 0.001723 | 0.001763 | 0.001667 |
| test_constants.test | RUN | 0.168913 | 0.488086 | 0.586552 | 0.001507 | 0.001914 | 0.001614 |
| test_uuid_cx.test | COMPILE | 0.097106 | 0.217829 | 0.302473 | — | — | — |
| test_uuid_cx2.test | RUN | 0.173952 | 0.541256 | 0.664678 | 0.001562 | 0.001702 | 0.001872 |
| test_uuid_cx3.test | RUN | 0.155359 | 0.437137 | 0.589011 | 0.001432 | 0.001612 | 0.001512 |
| test_uuid_cx3_no_simd.test | RUN | 0.143825 | 0.430545 | 0.540034 | 0.001698 | 0.001678 | 0.001848 |
| test_hash_value_cx.test | RUN | 0.151337 | 0.398626 | 0.550611 | 0.001513 | 0.001852 | 0.001539 |
| test_string_generator_cx.test | RUN | 0.174541 | 0.475325 | 0.607177 | 0.001788 | 0.001682 | 0.001843 |
| test_string_generator_cx2.test | RUN | 0.207391 | 0.596698 | 0.733120 | 0.001661 | 0.001861 | 0.001725 |
| test_constants_cx.test | RUN | 0.144379 | 0.353314 | 0.503798 | 0.001466 | 0.001572 | 0.001708 |
| test_from_chars_cx.test | RUN | 0.181313 | 0.641949 | 0.696282 | 0.001645 | 0.001850 | 0.001532 |
| test_from_chars_cx2.test | RUN | 0.187961 | 0.807357 | 0.685260 | 0.001814 | 0.001541 | 0.001837 |
| test_to_chars_cx.test | RUN | 0.188144 | 0.731200 | 0.786627 | 0.001659 | 0.001676 | 0.001198 |
| test_uuid_from_string_cx.test | RUN | 0.196598 | 0.751101 | 0.728990 | 0.001276 | 0.001387 | 0.001311 |
| quick.test | RUN | 0.213734 | 0.560121 | 0.658884 | 0.001270 | 0.001252 | 0.001263 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.008× | 1.008×, 1.008× | 0.999× |
| clang | 0.997× | 0.990×, 1.005× | 1.013× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267198).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| quick.test | 10 | 0.001169s | 0.001168s | 0.001155s | byte equality observed |
| test_alignment.test | 10 | 0.001182s | 0.001161s | 0.001167s | byte equality observed |
| test_alignment_2.test | 10 | 0.001174s | 0.001162s | 0.001183s | byte equality observed |
| test_attribute_packed.test | 10 | 0.001149s | 0.001183s | 0.001170s | byte equality observed |
| test_bench_random.test | 4 | 0.005188s | 0.005161s | 0.005136s | native test oracle; output varies |
| test_boost_unordered.test | 10 | 0.001240s | 0.001213s | 0.001207s | byte equality observed |
| test_comparison.test | 8 | 0.001234s | 0.001187s | 0.001197s | byte equality observed |
| test_comparison_no_simd.test | 10 | 0.001210s | 0.001212s | 0.001194s | byte equality observed |
| test_constants.test | 10 | 0.001185s | 0.001177s | 0.001178s | byte equality observed |
| test_constants_cx.test | 10 | 0.001146s | 0.001174s | 0.001137s | byte equality observed |
| test_data.test | 10 | 0.001174s | 0.001182s | 0.001170s | byte equality observed |
| test_detail_chacha20.test | 10 | 0.001159s | 0.001174s | 0.001182s | byte equality observed |
| test_detail_endian.test | 10 | 0.001167s | 0.001158s | 0.001128s | byte equality observed |
| test_detail_md5.test | 10 | 0.001203s | 0.001203s | 0.001182s | byte equality observed |
| test_detail_random_provider.test | 10 | 0.001162s | 0.001190s | 0.001199s | byte equality observed |
| test_detail_sha1.test | 3 | 0.008270s | 0.008417s | 0.007621s | byte equality observed |
| test_entropy_error.test | 10 | 0.001171s | 0.001127s | 0.001177s | byte equality observed |
| test_from_chars.test | 10 | 0.001165s | 0.001133s | 0.001157s | byte equality observed |
| test_from_chars_2.test | 10 | 0.001161s | 0.001151s | 0.001137s | byte equality observed |
| test_from_chars_2_no_simd.test | 10 | 0.001170s | 0.001193s | 0.001180s | byte equality observed |
| test_from_chars_cx.test | 10 | 0.001163s | 0.001145s | 0.001185s | byte equality observed |
| test_from_chars_cx2.test | 10 | 0.001171s | 0.001193s | 0.001153s | byte equality observed |
| test_from_chars_no_simd.test | 10 | 0.001172s | 0.001172s | 0.001176s | byte equality observed |
| test_hash.test | 10 | 0.001199s | 0.001176s | 0.001182s | byte equality observed |
| test_hash_value.test | 5 | 0.004188s | 0.004107s | 0.004145s | byte equality observed |
| test_hash_value_cx.test | 10 | 0.001155s | 0.001140s | 0.001129s | byte equality observed |
| test_include1.test | 10 | 0.000507s | 0.000496s | 0.001140s | byte equality observed |
| test_io.test | 10 | 0.001243s | 0.001238s | 0.001264s | byte equality observed |
| test_io_2.test | 10 | 0.001298s | 0.001325s | 0.001281s | byte equality observed |
| test_io_2_no_simd.test | 10 | 0.001328s | 0.001327s | 0.001335s | byte equality observed |
| test_io_no_simd.test | 10 | 0.001243s | 0.001253s | 0.001238s | byte equality observed |
| test_msvc_simd_bug981648.test | 10 | 0.001198s | 0.001175s | 0.001189s | native test oracle; output varies |
| test_name_generator.test | 10 | 0.001212s | 0.001188s | 0.001220s | byte equality observed |
| test_name_generator_md5.test | 10 | 0.001214s | 0.001182s | 0.001202s | byte equality observed |
| test_name_generator_sha1.test | 10 | 0.001215s | 0.001220s | 0.001183s | byte equality observed |
| test_namespaces.test | 10 | 0.001172s | 0.001195s | 0.001161s | byte equality observed |
| test_nil_generator.test | 10 | 0.001185s | 0.001187s | 0.001172s | byte equality observed |
| test_pragma_pack.test | 10 | 0.001167s | 0.001148s | 0.001160s | byte equality observed |
| test_random_generator.test | 10 | 0.002020s | 0.002001s | 0.002010s | native test oracle; output varies |
| test_random_generator_2.test | 10 | 0.001194s | 0.001179s | 0.001169s | byte equality observed |
| test_serialization.test | 10 | 0.001300s | 0.001286s | 0.001275s | byte equality observed |
| test_std_unordered.test | 10 | 0.001244s | 0.001247s | 0.001269s | byte equality observed |
| test_string_generator.test | 10 | 0.001286s | 0.001284s | 0.001269s | byte equality observed |
| test_string_generator_2.test | 10 | 0.001203s | 0.001196s | 0.001188s | byte equality observed |
| test_string_generator_cx.test | 10 | 0.001181s | 0.001175s | 0.001164s | byte equality observed |
| test_string_generator_cx2.test | 10 | 0.001177s | 0.001187s | 0.001149s | byte equality observed |
| test_tagging.test | 10 | 0.001152s | 0.001175s | 0.001185s | byte equality observed |
| test_time_generator.test | 10 | 0.001185s | 0.001214s | 0.001169s | byte equality observed |
| test_time_generator_v1.test | 10 | 0.001598s | 0.001701s | 0.001720s | byte equality observed |
| test_time_generator_v1_2.test | 10 | 0.001451s | 0.001470s | 0.001473s | byte equality observed |
| test_time_generator_v1_3.test | 6 | 0.003734s | 0.003594s | 0.003743s | byte equality observed |
| test_time_generator_v6.test | 10 | 0.001630s | 0.001707s | 0.001722s | byte equality observed |
| test_time_generator_v6_2.test | 10 | 0.001437s | 0.001457s | 0.001504s | byte equality observed |
| test_time_generator_v6_3.test | 6 | 0.003738s | 0.003642s | 0.003722s | byte equality observed |
| test_time_generator_v7.test | 10 | 0.001446s | 0.001474s | 0.001480s | byte equality observed |
| test_time_generator_v7_2.test | 10 | 0.001274s | 0.001234s | 0.001229s | byte equality observed |
| test_time_generator_v7_3.test | 10 | 0.001851s | 0.001826s | 0.001826s | byte equality observed |
| test_to_chars.test | 10 | 0.001237s | 0.001254s | 0.001266s | byte equality observed |
| test_to_chars_2.test | 10 | 0.001205s | 0.001192s | 0.001182s | byte equality observed |
| test_to_chars_2_no_simd.test | 10 | 0.001202s | 0.001175s | 0.001198s | byte equality observed |
| test_to_chars_3.test | 10 | 0.001192s | 0.001169s | 0.001163s | byte equality observed |
| test_to_chars_cx.test | 10 | 0.001188s | 0.001184s | 0.001194s | byte equality observed |
| test_to_chars_no_simd.test | 10 | 0.001258s | 0.001233s | 0.001221s | byte equality observed |
| test_uuid.test | 10 | 0.001142s | 0.001163s | 0.001165s | byte equality observed |
| test_uuid_2.test | 10 | 0.001181s | 0.001176s | 0.001177s | byte equality observed |
| test_uuid_3.test | 10 | 0.001174s | 0.001164s | 0.001167s | byte equality observed |
| test_uuid_class.test | 10 | 0.001177s | 0.001182s | 0.001177s | byte equality observed |
| test_uuid_clock.test | 10 | 0.001130s | 0.001188s | 0.001162s | byte equality observed |
| test_uuid_cx2.test | 10 | 0.001176s | 0.001172s | 0.001182s | byte equality observed |
| test_uuid_cx3.test | 10 | 0.001156s | 0.001159s | 0.001162s | byte equality observed |
| test_uuid_cx3_no_simd.test | 10 | 0.001160s | 0.001167s | 0.001162s | byte equality observed |
| test_uuid_from_string.test | 10 | 0.001198s | 0.001165s | 0.001189s | byte equality observed |
| test_uuid_from_string_2.test | 10 | 0.001454s | 0.001445s | 0.001472s | byte equality observed |
| test_uuid_from_string_3.test | 10 | 0.001272s | 0.001259s | 0.001272s | byte equality observed |
| test_uuid_from_string_cx.test | 10 | 0.001174s | 0.001175s | 0.001171s | byte equality observed |
| test_uuid_in_map.test | 10 | 0.001178s | 0.001174s | 0.001156s | byte equality observed |
| test_uuid_no_simd.test | 10 | 0.001170s | 0.001176s | 0.001178s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267198).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
