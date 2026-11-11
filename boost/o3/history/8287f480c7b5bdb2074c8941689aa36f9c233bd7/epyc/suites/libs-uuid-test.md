# libs/uuid/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_include1.test | RUN | 0.450859 | 0.934457 | 1.454600 | 0.000697 | 0.000715 | 0.001724 |
| test_uuid.test | RUN | 0.300689 | 0.768221 | 1.005997 | 0.001886 | 0.001826 | 0.001798 |
| test_uuid_no_simd.test | RUN | 0.268076 | 0.727370 | 0.905690 | 0.001618 | 0.001557 | 0.001532 |
| test_uuid_2.test | RUN | 0.190823 | 0.553511 | 0.639084 | 0.001701 | 0.001736 | 0.001707 |
| test_uuid_3.test | RUN | 0.168608 | 0.399334 | 0.560922 | 0.001891 | 0.001828 | 0.001475 |
| test_alignment.test | RUN | 0.155837 | 0.342983 | 0.475498 | 0.001642 | 0.001766 | 0.001743 |
| test_alignment_2.test | RUN | 0.156321 | 0.347649 | 0.471966 | 0.001572 | 0.001582 | 0.001668 |
| test_attribute_packed.test | RUN | 0.166531 | 0.343184 | 0.509273 | 0.001675 | 0.001733 | 0.001442 |
| test_pragma_pack.test | RUN | 0.158502 | 0.343837 | 0.508794 | 0.001660 | 0.001774 | 0.001453 |
| test_data.test | RUN | 0.198031 | 0.620849 | 0.680450 | 0.001612 | 0.001642 | 0.001742 |
| test_comparison.test | RUN | 0.257945 | 0.721539 | 0.893094 | 0.001642 | 0.001873 | 0.001827 |
| test_comparison_no_simd.test | RUN | 0.240080 | 0.838802 | 0.864219 | 0.001708 | 0.001547 | 0.001526 |
| test_io.test | RUN | 0.329464 | 1.365004 | 1.374371 | 0.002146 | 0.001906 | 0.001796 |
| test_io_no_simd.test | RUN | 0.318401 | 1.338471 | 1.294361 | 0.001641 | 0.001782 | 0.001634 |
| test_io_2.test | RUN | 0.259690 | 1.389783 | 0.988965 | 0.001565 | 0.001646 | 0.001966 |
| test_io_2_no_simd.test | RUN | 0.232486 | 1.322421 | 0.850321 | 0.001554 | 0.001854 | 0.002102 |
| test_to_chars.test | RUN | 0.254771 | 0.882357 | 0.880732 | 0.002084 | 0.001924 | 0.001779 |
| test_to_chars_no_simd.test | RUN | 0.236854 | 0.892433 | 0.891865 | 0.002028 | 0.001842 | 0.001808 |
| test_to_chars_2.test | RUN | 0.304117 | 1.602127 | 1.078274 | 0.001679 | 0.001806 | 0.001695 |
| test_to_chars_2_no_simd.test | RUN | 0.288575 | 1.705329 | 0.947923 | 0.001629 | 0.001548 | 0.001725 |
| test_to_chars_3.test | RUN | 0.199768 | 0.578467 | 0.670133 | 0.001719 | 0.001535 | 0.001330 |
| test_from_chars.test | RUN | 0.223418 | 0.831425 | 0.736658 | 0.001690 | 0.001698 | 0.001806 |
| test_from_chars_no_simd.test | RUN | 0.190087 | 0.666065 | 0.639266 | 0.001579 | 0.001608 | 0.001427 |
| test_from_chars_2.test | RUN | 0.214072 | 0.784305 | 0.743143 | 0.001570 | 0.001730 | 0.001558 |
| test_from_chars_2_no_simd.test | RUN | 0.186458 | 0.608427 | 0.573318 | 0.001515 | 0.001590 | 0.001861 |
| test_uuid_from_string.test | RUN | 0.237375 | 0.860616 | 0.895601 | 0.001514 | 0.001622 | 0.001542 |
| test_uuid_from_string_2.test | RUN | 0.209569 | 0.795085 | 0.749195 | 0.001996 | 0.001850 | 0.001780 |
| test_uuid_from_string_3.test | RUN | 0.210280 | 0.778851 | 0.792664 | 0.001685 | 0.001786 | 0.001959 |
| test_uuid_clock.test | RUN | 0.144227 | 0.342295 | 0.481885 | 0.001420 | 0.001770 | 0.001518 |
| test_nil_generator.test | RUN | 0.170255 | 0.389169 | 0.551073 | 0.001752 | 0.001688 | 0.001850 |
| test_string_generator.test | RUN | 0.195348 | 0.624479 | 0.739648 | 0.001753 | 0.002003 | 0.001820 |
| test_string_generator_2.test | RUN | 0.221513 | 0.893481 | 0.834284 | 0.001581 | 0.001894 | 0.001780 |
| test_random_generator.test | RUN | 0.355082 | 1.709801 | 1.292769 | 0.002560 | 0.002623 | 0.002697 |
| test_random_generator_2.test | RUN | 0.248196 | 0.662859 | 0.898103 | 0.001615 | 0.001830 | 0.001630 |
| test_name_generator.test | RUN | 0.227646 | 2.205773 | 0.921753 | 0.001524 | 0.001776 | 0.001711 |
| test_namespaces.test | RUN | 0.176120 | 0.477821 | 0.621374 | 0.001638 | 0.001741 | 0.001451 |
| test_name_generator_md5.test | RUN | 0.230228 | 1.037226 | 0.884407 | 0.001799 | 0.001874 | 0.001881 |
| test_name_generator_sha1.test | RUN | 0.244497 | 3.200692 | 0.896993 | 0.001824 | 0.001380 | 0.001679 |
| test_time_generator.test | RUN | 0.258204 | 0.653183 | 0.825409 | 0.001652 | 0.001926 | 0.001626 |
| test_time_generator_v1.test | RUN | 0.287754 | 0.734478 | 0.968142 | 0.001860 | 0.002334 | 0.002419 |
| test_time_generator_v1_2.test | RUN | 0.274476 | 0.696688 | 0.904968 | 0.001870 | 0.002172 | 0.002140 |
| test_time_generator_v1_3.test | RUN | 0.294151 | 0.721001 | 0.992634 | 0.004001 | 0.004726 | 0.004098 |
| test_time_generator_v6.test | RUN | 0.284099 | 0.732284 | 0.960174 | 0.001952 | 0.002436 | 0.002328 |
| test_time_generator_v6_2.test | RUN | 0.276265 | 0.675244 | 0.929697 | 0.001822 | 0.002126 | 0.002200 |
| test_time_generator_v6_3.test | RUN | 0.293667 | 0.742883 | 1.011174 | 0.004765 | 0.003704 | 0.004966 |
| test_time_generator_v7.test | RUN | 0.267076 | 0.723775 | 0.908884 | 0.001764 | 0.002132 | 0.001992 |
| test_time_generator_v7_2.test | RUN | 0.247094 | 0.628524 | 0.864487 | 0.001938 | 0.001869 | 0.001696 |
| test_time_generator_v7_3.test | RUN | 0.254389 | 0.826706 | 0.888187 | 0.002158 | 0.002522 | 0.002504 |
| test_serialization.test | RUN | 0.464155 | 1.220196 | 1.528409 | 0.001917 | 0.001655 | 0.002034 |
| test_tagging.test | RUN | 0.252029 | 0.770444 | 0.915404 | 0.001724 | 0.001690 | 0.001740 |
| test_uuid_class.test | RUN | 0.261817 | 0.694943 | 0.955381 | 0.001553 | 0.001503 | 0.001559 |
| test_uuid_in_map.test | RUN | 0.283741 | 0.784363 | 0.989156 | 0.001446 | 0.001853 | 0.001882 |
| test_hash.test | RUN | 0.301051 | 0.716346 | 0.968073 | 0.001720 | 0.001882 | 0.001930 |
| test_hash_value.test | RUN | 0.214690 | 0.498264 | 0.684550 | 0.004866 | 0.004904 | 0.004812 |
| test_std_unordered.test | RUN | 0.274196 | 0.795961 | 0.929868 | 0.001790 | 0.001871 | 0.001917 |
| test_boost_unordered.test | RUN | 0.369909 | 1.028030 | 1.227556 | 0.001844 | 0.001714 | 0.001753 |
| test_detail_md5.test | RUN | 0.179770 | 0.495869 | 0.662979 | 0.001676 | 0.001841 | 0.001938 |
| test_detail_sha1.test | RUN | 0.172786 | 0.635238 | 0.609759 | 0.009352 | 0.009328 | 0.008447 |
| test_detail_endian.test | RUN | 0.133028 | 0.361706 | 0.468709 | 0.001796 | 0.001729 | 0.001847 |
| test_detail_chacha20.test | RUN | 0.152187 | 0.458889 | 0.454634 | 0.001782 | 0.001562 | 0.001450 |
| random_provider_no_copy_ctor.test | COMPILE_FAIL | 0.132368 | 0.245912 | 0.383827 | — | — | — |
| random_provider_no_copy_assign.test | COMPILE_FAIL | 0.128904 | 0.264194 | 0.389503 | — | — | — |
| test_msvc_simd_bug981648.test | RUN | 0.225807 | 0.491824 | 0.658672 | 0.001826 | 0.001736 | 0.001513 |
| test_bench_random.test | RUN | 0.251767 | 0.639996 | 0.890106 | 0.005838 | 0.005821 | 0.005898 |
| test_entropy_error.test | RUN | 0.126963 | 0.304496 | 0.424610 | 0.001575 | 0.001862 | 0.001676 |
| test_detail_random_provider.test | RUN | 0.241898 | 0.499589 | 0.781588 | 0.001419 | 0.001749 | 0.001783 |
| test_constants.test | RUN | 0.180079 | 0.492949 | 0.596082 | 0.001714 | 0.001879 | 0.001672 |
| test_uuid_cx.test | COMPILE | 0.113033 | 0.227454 | 0.304578 | — | — | — |
| test_uuid_cx2.test | RUN | 0.187957 | 0.585690 | 0.660427 | 0.001900 | 0.001693 | 0.001864 |
| test_uuid_cx3.test | RUN | 0.175849 | 0.455552 | 0.624972 | 0.001268 | 0.001572 | 0.001475 |
| test_uuid_cx3_no_simd.test | RUN | 0.155298 | 0.417024 | 0.564063 | 0.001644 | 0.001609 | 0.001435 |
| test_hash_value_cx.test | RUN | 0.163988 | 0.379399 | 0.528474 | 0.001352 | 0.001861 | 0.001654 |
| test_string_generator_cx.test | RUN | 0.179258 | 0.478678 | 0.613274 | 0.001642 | 0.001536 | 0.001834 |
| test_string_generator_cx2.test | RUN | 0.187282 | 0.583528 | 0.719161 | 0.001670 | 0.001897 | 0.001617 |
| test_constants_cx.test | RUN | 0.159842 | 0.345797 | 0.537720 | 0.001368 | 0.001533 | 0.001682 |
| test_from_chars_cx.test | RUN | 0.192195 | 0.688885 | 0.654179 | 0.001437 | 0.001850 | 0.001736 |
| test_from_chars_cx2.test | RUN | 0.205537 | 0.818278 | 0.728468 | 0.001706 | 0.001746 | 0.001921 |
| test_to_chars_cx.test | RUN | 0.204947 | 0.741294 | 0.803242 | 0.001760 | 0.001881 | 0.001496 |
| test_uuid_from_string_cx.test | RUN | 0.205408 | 0.733278 | 0.780168 | 0.001262 | 0.001320 | 0.001353 |
| quick.test | RUN | 0.230858 | 0.527337 | 0.682776 | 0.001267 | 0.001284 | 0.001320 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.003× | 0.998×, 1.008× | 0.998× |
| clang | 1.001× | 0.999×, 1.003× | 1.010× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| quick.test | 10 | 0.001177s | 0.001178s | 0.001175s | byte equality observed |
| test_alignment.test | 10 | 0.001196s | 0.001201s | 0.001162s | byte equality observed |
| test_alignment_2.test | 10 | 0.001186s | 0.001184s | 0.001225s | byte equality observed |
| test_attribute_packed.test | 10 | 0.001172s | 0.001179s | 0.001202s | byte equality observed |
| test_bench_random.test | 4 | 0.005269s | 0.005201s | 0.005248s | native test oracle; output varies |
| test_boost_unordered.test | 10 | 0.001246s | 0.001233s | 0.001250s | byte equality observed |
| test_comparison.test | 10 | 0.001221s | 0.001224s | 0.001218s | byte equality observed |
| test_comparison_no_simd.test | 10 | 0.001230s | 0.001213s | 0.001253s | byte equality observed |
| test_constants.test | 10 | 0.001193s | 0.001197s | 0.001203s | byte equality observed |
| test_constants_cx.test | 10 | 0.001168s | 0.001154s | 0.001170s | byte equality observed |
| test_data.test | 10 | 0.001203s | 0.001182s | 0.001191s | byte equality observed |
| test_detail_chacha20.test | 10 | 0.001196s | 0.001202s | 0.001220s | byte equality observed |
| test_detail_endian.test | 10 | 0.001179s | 0.001162s | 0.001194s | byte equality observed |
| test_detail_md5.test | 10 | 0.001194s | 0.001183s | 0.001235s | byte equality observed |
| test_detail_random_provider.test | 10 | 0.001197s | 0.001164s | 0.001211s | byte equality observed |
| test_detail_sha1.test | 3 | 0.008247s | 0.008519s | 0.007533s | byte equality observed |
| test_entropy_error.test | 10 | 0.001184s | 0.001172s | 0.001179s | byte equality observed |
| test_from_chars.test | 10 | 0.001183s | 0.001154s | 0.001185s | byte equality observed |
| test_from_chars_2.test | 10 | 0.001175s | 0.001144s | 0.001186s | byte equality observed |
| test_from_chars_2_no_simd.test | 10 | 0.001208s | 0.001174s | 0.001200s | byte equality observed |
| test_from_chars_cx.test | 10 | 0.001184s | 0.001168s | 0.001192s | byte equality observed |
| test_from_chars_cx2.test | 10 | 0.001184s | 0.001189s | 0.001177s | byte equality observed |
| test_from_chars_no_simd.test | 10 | 0.001184s | 0.001191s | 0.001190s | byte equality observed |
| test_hash.test | 10 | 0.001219s | 0.001196s | 0.001190s | byte equality observed |
| test_hash_value.test | 5 | 0.004343s | 0.004214s | 0.004076s | byte equality observed |
| test_hash_value_cx.test | 10 | 0.001185s | 0.001182s | 0.001164s | byte equality observed |
| test_include1.test | 10 | 0.000521s | 0.000514s | 0.001159s | byte equality observed |
| test_io.test | 10 | 0.001256s | 0.001239s | 0.001282s | byte equality observed |
| test_io_2.test | 10 | 0.001347s | 0.001337s | 0.001318s | byte equality observed |
| test_io_2_no_simd.test | 10 | 0.001351s | 0.001338s | 0.001327s | byte equality observed |
| test_io_no_simd.test | 10 | 0.001267s | 0.001270s | 0.001258s | byte equality observed |
| test_msvc_simd_bug981648.test | 10 | 0.001191s | 0.001172s | 0.001192s | native test oracle; output varies |
| test_name_generator.test | 10 | 0.001222s | 0.001233s | 0.001234s | byte equality observed |
| test_name_generator_md5.test | 10 | 0.001227s | 0.001197s | 0.001166s | byte equality observed |
| test_name_generator_sha1.test | 10 | 0.001246s | 0.001247s | 0.001207s | byte equality observed |
| test_namespaces.test | 10 | 0.001183s | 0.001182s | 0.001184s | byte equality observed |
| test_nil_generator.test | 10 | 0.001163s | 0.001185s | 0.001193s | byte equality observed |
| test_pragma_pack.test | 10 | 0.001174s | 0.001185s | 0.001194s | byte equality observed |
| test_random_generator.test | 9 | 0.002051s | 0.002038s | 0.002015s | native test oracle; output varies |
| test_random_generator_2.test | 10 | 0.001207s | 0.001180s | 0.001211s | byte equality observed |
| test_serialization.test | 10 | 0.001308s | 0.001309s | 0.001286s | byte equality observed |
| test_std_unordered.test | 10 | 0.001255s | 0.001277s | 0.001250s | byte equality observed |
| test_string_generator.test | 10 | 0.001305s | 0.001303s | 0.001303s | byte equality observed |
| test_string_generator_2.test | 10 | 0.001204s | 0.001206s | 0.001201s | byte equality observed |
| test_string_generator_cx.test | 10 | 0.001214s | 0.001195s | 0.001191s | byte equality observed |
| test_string_generator_cx2.test | 10 | 0.001212s | 0.001206s | 0.001192s | byte equality observed |
| test_tagging.test | 10 | 0.001182s | 0.001183s | 0.001209s | byte equality observed |
| test_time_generator.test | 10 | 0.001202s | 0.001217s | 0.001243s | byte equality observed |
| test_time_generator_v1.test | 10 | 0.001633s | 0.001711s | 0.001776s | byte equality observed |
| test_time_generator_v1_2.test | 10 | 0.001458s | 0.001464s | 0.001492s | byte equality observed |
| test_time_generator_v1_3.test | 6 | 0.003699s | 0.003455s | 0.003562s | byte equality observed |
| test_time_generator_v6.test | 10 | 0.001610s | 0.001706s | 0.001740s | byte equality observed |
| test_time_generator_v6_2.test | 10 | 0.001432s | 0.001495s | 0.001522s | byte equality observed |
| test_time_generator_v6_3.test | 6 | 0.003652s | 0.003660s | 0.003657s | byte equality observed |
| test_time_generator_v7.test | 10 | 0.001440s | 0.001474s | 0.001506s | byte equality observed |
| test_time_generator_v7_2.test | 10 | 0.001265s | 0.001268s | 0.001260s | byte equality observed |
| test_time_generator_v7_3.test | 10 | 0.001872s | 0.001859s | 0.001889s | byte equality observed |
| test_to_chars.test | 10 | 0.001276s | 0.001258s | 0.001283s | byte equality observed |
| test_to_chars_2.test | 10 | 0.001202s | 0.001205s | 0.001222s | byte equality observed |
| test_to_chars_2_no_simd.test | 10 | 0.001210s | 0.001218s | 0.001212s | byte equality observed |
| test_to_chars_3.test | 10 | 0.001201s | 0.001198s | 0.001206s | byte equality observed |
| test_to_chars_cx.test | 10 | 0.001196s | 0.001205s | 0.001183s | byte equality observed |
| test_to_chars_no_simd.test | 10 | 0.001265s | 0.001267s | 0.001285s | byte equality observed |
| test_uuid.test | 10 | 0.001178s | 0.001207s | 0.001197s | byte equality observed |
| test_uuid_2.test | 10 | 0.001198s | 0.001182s | 0.001193s | byte equality observed |
| test_uuid_3.test | 10 | 0.001192s | 0.001183s | 0.001191s | byte equality observed |
| test_uuid_class.test | 10 | 0.001214s | 0.001192s | 0.001199s | byte equality observed |
| test_uuid_clock.test | 10 | 0.001175s | 0.001197s | 0.001189s | byte equality observed |
| test_uuid_cx2.test | 10 | 0.001207s | 0.001191s | 0.001206s | byte equality observed |
| test_uuid_cx3.test | 10 | 0.001187s | 0.001203s | 0.001190s | byte equality observed |
| test_uuid_cx3_no_simd.test | 10 | 0.001177s | 0.001198s | 0.001202s | byte equality observed |
| test_uuid_from_string.test | 10 | 0.001214s | 0.001198s | 0.001201s | byte equality observed |
| test_uuid_from_string_2.test | 10 | 0.001482s | 0.001445s | 0.001511s | byte equality observed |
| test_uuid_from_string_3.test | 10 | 0.001282s | 0.001279s | 0.001292s | byte equality observed |
| test_uuid_from_string_cx.test | 10 | 0.001199s | 0.001190s | 0.001209s | byte equality observed |
| test_uuid_in_map.test | 10 | 0.001193s | 0.001217s | 0.001191s | byte equality observed |
| test_uuid_no_simd.test | 10 | 0.001190s | 0.001186s | 0.001192s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
