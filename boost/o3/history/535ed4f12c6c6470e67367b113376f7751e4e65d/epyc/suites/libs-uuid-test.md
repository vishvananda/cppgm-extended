# libs/uuid/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_include1.test | RUN | 0.480313 | 0.937854 | 1.468081 | 0.000734 | 0.000736 | 0.001698 |
| test_uuid.test | RUN | 0.286732 | 0.777288 | 1.032557 | 0.001666 | 0.001924 | 0.001628 |
| test_uuid_no_simd.test | RUN | 0.275674 | 0.728599 | 0.893271 | 0.001588 | 0.001492 | 0.001479 |
| test_uuid_2.test | RUN | 0.205446 | 0.552873 | 0.669916 | 0.001795 | 0.001584 | 0.001387 |
| test_uuid_3.test | RUN | 0.173814 | 0.405020 | 0.567591 | 0.001519 | 0.001928 | 0.001601 |
| test_alignment.test | RUN | 0.159708 | 0.352098 | 0.480011 | 0.001626 | 0.001864 | 0.001834 |
| test_alignment_2.test | RUN | 0.167331 | 0.344091 | 0.509848 | 0.001424 | 0.001459 | 0.001633 |
| test_attribute_packed.test | RUN | 0.156731 | 0.356076 | 0.506490 | 0.001840 | 0.001648 | 0.001459 |
| test_pragma_pack.test | RUN | 0.161036 | 0.353749 | 0.476592 | 0.001737 | 0.001797 | 0.001595 |
| test_data.test | RUN | 0.198782 | 0.634099 | 0.689771 | 0.001817 | 0.001693 | 0.001844 |
| test_comparison.test | RUN | 0.275774 | 0.731757 | 0.956324 | 0.001808 | 0.001953 | 0.001698 |
| test_comparison_no_simd.test | RUN | 0.256989 | 0.834713 | 0.865932 | 0.001533 | 0.001671 | 0.001613 |
| test_io.test | RUN | 0.318977 | 1.392521 | 1.334700 | 0.001820 | 0.001972 | 0.001982 |
| test_io_no_simd.test | RUN | 0.300731 | 1.359881 | 1.297802 | 0.001442 | 0.001854 | 0.001814 |
| test_io_2.test | RUN | 0.252146 | 1.410229 | 0.952473 | 0.001802 | 0.002046 | 0.001758 |
| test_io_2_no_simd.test | RUN | 0.211714 | 1.325467 | 0.900249 | 0.001874 | 0.002078 | 0.001934 |
| test_to_chars.test | RUN | 0.263981 | 0.893543 | 0.915312 | 0.001968 | 0.002030 | 0.001765 |
| test_to_chars_no_simd.test | RUN | 0.245260 | 0.908271 | 0.840180 | 0.001929 | 0.001774 | 0.001950 |
| test_to_chars_2.test | RUN | 0.293626 | 1.632880 | 1.076319 | 0.001618 | 0.001747 | 0.001368 |
| test_to_chars_2_no_simd.test | RUN | 0.297008 | 1.725548 | 0.970510 | 0.001540 | 0.001692 | 0.001864 |
| test_to_chars_3.test | RUN | 0.200930 | 0.592981 | 0.633380 | 0.001831 | 0.001807 | 0.001463 |
| test_from_chars.test | RUN | 0.213637 | 0.845120 | 0.762130 | 0.001826 | 0.001754 | 0.001756 |
| test_from_chars_no_simd.test | RUN | 0.185601 | 0.669519 | 0.632602 | 0.001684 | 0.001790 | 0.001775 |
| test_from_chars_2.test | RUN | 0.214507 | 0.792801 | 0.743423 | 0.001599 | 0.001671 | 0.001715 |
| test_from_chars_2_no_simd.test | RUN | 0.184986 | 0.608510 | 0.613696 | 0.001821 | 0.001704 | 0.001630 |
| test_uuid_from_string.test | RUN | 0.238972 | 0.867017 | 0.884921 | 0.001692 | 0.001725 | 0.001552 |
| test_uuid_from_string_2.test | RUN | 0.213820 | 0.789844 | 0.735051 | 0.001954 | 0.002170 | 0.001873 |
| test_uuid_from_string_3.test | RUN | 0.212870 | 0.795093 | 0.789439 | 0.001768 | 0.001922 | 0.001909 |
| test_uuid_clock.test | RUN | 0.146035 | 0.326618 | 0.474675 | 0.001639 | 0.001882 | 0.001684 |
| test_nil_generator.test | RUN | 0.170466 | 0.380770 | 0.570475 | 0.001775 | 0.001838 | 0.001744 |
| test_string_generator.test | RUN | 0.199321 | 0.643305 | 0.748345 | 0.001950 | 0.002029 | 0.002016 |
| test_string_generator_2.test | RUN | 0.225981 | 0.886331 | 0.846087 | 0.001637 | 0.001773 | 0.001755 |
| test_random_generator.test | RUN | 0.352348 | 1.717017 | 1.271583 | 0.002390 | 0.002701 | 0.002680 |
| test_random_generator_2.test | RUN | 0.253976 | 0.686184 | 0.860937 | 0.001892 | 0.001885 | 0.001746 |
| test_name_generator.test | RUN | 0.227792 | 2.220017 | 0.896502 | 0.001619 | 0.001894 | 0.001704 |
| test_namespaces.test | RUN | 0.178711 | 0.480085 | 0.609272 | 0.001707 | 0.001899 | 0.001717 |
| test_name_generator_md5.test | RUN | 0.230564 | 1.054835 | 0.868206 | 0.001913 | 0.001932 | 0.001909 |
| test_name_generator_sha1.test | RUN | 0.255960 | 3.238913 | 0.873305 | 0.001730 | 0.002038 | 0.002010 |
| test_time_generator.test | RUN | 0.253344 | 0.663299 | 0.858489 | 0.001598 | 0.001951 | 0.001799 |
| test_time_generator_v1.test | RUN | 0.291025 | 0.735314 | 0.920089 | 0.001956 | 0.002346 | 0.002200 |
| test_time_generator_v1_2.test | RUN | 0.272686 | 0.700308 | 0.887789 | 0.002092 | 0.002171 | 0.002126 |
| test_time_generator_v1_3.test | RUN | 0.298467 | 0.736974 | 1.001165 | 0.004452 | 0.004735 | 0.004276 |
| test_time_generator_v6.test | RUN | 0.288886 | 0.753742 | 0.952959 | 0.002072 | 0.002331 | 0.002341 |
| test_time_generator_v6_2.test | RUN | 0.275882 | 0.697807 | 0.901947 | 0.001832 | 0.002316 | 0.001824 |
| test_time_generator_v6_3.test | RUN | 0.299706 | 0.744491 | 1.034544 | 0.004488 | 0.004965 | 0.004636 |
| test_time_generator_v7.test | RUN | 0.266828 | 0.742927 | 0.908164 | 0.001830 | 0.002128 | 0.002112 |
| test_time_generator_v7_2.test | RUN | 0.247574 | 0.637055 | 0.866716 | 0.001437 | 0.001981 | 0.001833 |
| test_time_generator_v7_3.test | RUN | 0.251431 | 0.833332 | 0.879535 | 0.002431 | 0.002708 | 0.002224 |
| test_serialization.test | RUN | 0.461284 | 1.245503 | 1.499544 | 0.002030 | 0.001935 | 0.001974 |
| test_tagging.test | RUN | 0.263451 | 0.783763 | 0.888209 | 0.001652 | 0.001644 | 0.001676 |
| test_uuid_class.test | RUN | 0.266045 | 0.690846 | 0.934271 | 0.001587 | 0.001676 | 0.001928 |
| test_uuid_in_map.test | RUN | 0.289040 | 0.808758 | 0.990663 | 0.001668 | 0.001811 | 0.001864 |
| test_hash.test | RUN | 0.297613 | 0.731266 | 0.929547 | 0.001888 | 0.001800 | 0.001837 |
| test_hash_value.test | RUN | 0.210804 | 0.502387 | 0.703071 | 0.004968 | 0.004978 | 0.004842 |
| test_std_unordered.test | RUN | 0.280644 | 0.795466 | 0.927386 | 0.001560 | 0.002022 | 0.001914 |
| test_boost_unordered.test | RUN | 0.391525 | 1.039044 | 1.168836 | 0.001937 | 0.001770 | 0.001596 |
| test_detail_md5.test | RUN | 0.180965 | 0.513144 | 0.685433 | 0.001586 | 0.001680 | 0.001895 |
| test_detail_sha1.test | RUN | 0.172538 | 0.642027 | 0.589914 | 0.008760 | 0.009316 | 0.008358 |
| test_detail_endian.test | RUN | 0.134983 | 0.365814 | 0.452741 | 0.001648 | 0.001795 | 0.001879 |
| test_detail_chacha20.test | RUN | 0.149424 | 0.462544 | 0.446235 | 0.001571 | 0.001613 | 0.001771 |
| random_provider_no_copy_ctor.test | COMPILE_FAIL | 0.132053 | 0.258481 | 0.387643 | — | — | — |
| random_provider_no_copy_assign.test | COMPILE_FAIL | 0.130760 | 0.263615 | 0.397531 | — | — | — |
| test_msvc_simd_bug981648.test | RUN | 0.226272 | 0.500081 | 0.661838 | 0.001812 | 0.001846 | 0.001543 |
| test_bench_random.test | RUN | 0.254526 | 0.669491 | 0.875698 | 0.005624 | 0.005825 | 0.005622 |
| test_entropy_error.test | RUN | 0.128551 | 0.315530 | 0.396438 | 0.001788 | 0.001874 | 0.001689 |
| test_detail_random_provider.test | RUN | 0.241848 | 0.514021 | 0.757977 | 0.001308 | 0.001752 | 0.001501 |
| test_constants.test | RUN | 0.178988 | 0.478483 | 0.596542 | 0.001614 | 0.001867 | 0.001564 |
| test_uuid_cx.test | COMPILE | 0.113389 | 0.234724 | 0.337455 | — | — | — |
| test_uuid_cx2.test | RUN | 0.195337 | 0.535458 | 0.677354 | 0.001785 | 0.001838 | 0.001872 |
| test_uuid_cx3.test | RUN | 0.174540 | 0.428599 | 0.627532 | 0.001460 | 0.001771 | 0.001498 |
| test_uuid_cx3_no_simd.test | RUN | 0.157680 | 0.414494 | 0.521360 | 0.001787 | 0.001808 | 0.001767 |
| test_hash_value_cx.test | RUN | 0.167793 | 0.375012 | 0.552697 | 0.001408 | 0.001709 | 0.001559 |
| test_string_generator_cx.test | RUN | 0.177987 | 0.488513 | 0.612039 | 0.001770 | 0.001896 | 0.001800 |
| test_string_generator_cx2.test | RUN | 0.192137 | 0.586128 | 0.692743 | 0.001610 | 0.001810 | 0.001630 |
| test_constants_cx.test | RUN | 0.159874 | 0.337440 | 0.523627 | 0.001815 | 0.001625 | 0.001763 |
| test_from_chars_cx.test | RUN | 0.194720 | 0.645234 | 0.676126 | 0.001383 | 0.001850 | 0.001342 |
| test_from_chars_cx2.test | RUN | 0.208659 | 0.780150 | 0.751461 | 0.001742 | 0.001416 | 0.001904 |
| test_to_chars_cx.test | RUN | 0.208557 | 0.733618 | 0.771300 | 0.001489 | 0.001881 | 0.001438 |
| test_uuid_from_string_cx.test | RUN | 0.205314 | 0.739363 | 0.783666 | 0.001415 | 0.001442 | 0.001286 |
| quick.test | RUN | 0.234785 | 0.573271 | 0.789735 | 0.001258 | 0.001298 | 0.001103 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.003× | 0.999×, 1.007× | 0.991× |
| clang | 1.018× | 1.010×, 1.027× | 1.008× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| quick.test | 10 | 0.001188s | 0.001203s | 0.001200s | byte equality observed |
| test_alignment.test | 10 | 0.001189s | 0.001193s | 0.001193s | byte equality observed |
| test_alignment_2.test | 10 | 0.001198s | 0.001204s | 0.001201s | byte equality observed |
| test_attribute_packed.test | 10 | 0.001181s | 0.001199s | 0.001211s | byte equality observed |
| test_bench_random.test | 4 | 0.005241s | 0.005202s | 0.005211s | native test oracle; output varies |
| test_boost_unordered.test | 10 | 0.001243s | 0.001247s | 0.001243s | byte equality observed |
| test_comparison.test | 10 | 0.001224s | 0.001227s | 0.001246s | byte equality observed |
| test_comparison_no_simd.test | 10 | 0.001239s | 0.001216s | 0.001241s | byte equality observed |
| test_constants.test | 10 | 0.001220s | 0.001196s | 0.001196s | byte equality observed |
| test_constants_cx.test | 10 | 0.001167s | 0.001165s | 0.001167s | byte equality observed |
| test_data.test | 10 | 0.001197s | 0.001176s | 0.001196s | byte equality observed |
| test_detail_chacha20.test | 10 | 0.001194s | 0.001191s | 0.001202s | byte equality observed |
| test_detail_endian.test | 10 | 0.001193s | 0.001175s | 0.001193s | byte equality observed |
| test_detail_md5.test | 10 | 0.001242s | 0.001216s | 0.001228s | byte equality observed |
| test_detail_random_provider.test | 10 | 0.001206s | 0.001184s | 0.001198s | byte equality observed |
| test_detail_sha1.test | 3 | 0.007384s | 0.008404s | 0.007608s | byte equality observed |
| test_entropy_error.test | 10 | 0.001183s | 0.001168s | 0.001171s | byte equality observed |
| test_from_chars.test | 10 | 0.001163s | 0.001139s | 0.001176s | byte equality observed |
| test_from_chars_2.test | 10 | 0.001201s | 0.001181s | 0.001204s | byte equality observed |
| test_from_chars_2_no_simd.test | 10 | 0.001163s | 0.001190s | 0.001195s | byte equality observed |
| test_from_chars_cx.test | 10 | 0.001186s | 0.001196s | 0.001186s | byte equality observed |
| test_from_chars_cx2.test | 10 | 0.001184s | 0.001212s | 0.001194s | byte equality observed |
| test_from_chars_no_simd.test | 10 | 0.001190s | 0.001181s | 0.001194s | byte equality observed |
| test_hash.test | 10 | 0.001196s | 0.001208s | 0.001225s | byte equality observed |
| test_hash_value.test | 5 | 0.004383s | 0.004186s | 0.004165s | byte equality observed |
| test_hash_value_cx.test | 10 | 0.001192s | 0.001150s | 0.001173s | byte equality observed |
| test_include1.test | 10 | 0.000526s | 0.000536s | 0.001153s | byte equality observed |
| test_io.test | 10 | 0.001276s | 0.001270s | 0.001268s | byte equality observed |
| test_io_2.test | 10 | 0.001345s | 0.001323s | 0.001335s | byte equality observed |
| test_io_2_no_simd.test | 10 | 0.001345s | 0.001357s | 0.001337s | byte equality observed |
| test_io_no_simd.test | 10 | 0.001292s | 0.001260s | 0.001270s | byte equality observed |
| test_msvc_simd_bug981648.test | 10 | 0.001207s | 0.001203s | 0.001192s | native test oracle; output varies |
| test_name_generator.test | 10 | 0.001237s | 0.001235s | 0.001233s | byte equality observed |
| test_name_generator_md5.test | 10 | 0.001231s | 0.001245s | 0.001216s | byte equality observed |
| test_name_generator_sha1.test | 10 | 0.001243s | 0.001254s | 0.001230s | byte equality observed |
| test_namespaces.test | 10 | 0.001200s | 0.001212s | 0.001193s | byte equality observed |
| test_nil_generator.test | 10 | 0.001185s | 0.001194s | 0.001197s | byte equality observed |
| test_pragma_pack.test | 10 | 0.001184s | 0.001182s | 0.001203s | byte equality observed |
| test_random_generator.test | 9 | 0.002070s | 0.002026s | 0.002041s | native test oracle; output varies |
| test_random_generator_2.test | 10 | 0.001210s | 0.001194s | 0.001202s | byte equality observed |
| test_serialization.test | 10 | 0.001303s | 0.001291s | 0.001315s | byte equality observed |
| test_std_unordered.test | 10 | 0.001270s | 0.001277s | 0.001281s | byte equality observed |
| test_string_generator.test | 10 | 0.001306s | 0.001326s | 0.001313s | byte equality observed |
| test_string_generator_2.test | 10 | 0.001232s | 0.001238s | 0.001232s | byte equality observed |
| test_string_generator_cx.test | 10 | 0.001197s | 0.001195s | 0.001211s | byte equality observed |
| test_string_generator_cx2.test | 10 | 0.001210s | 0.001206s | 0.001214s | byte equality observed |
| test_tagging.test | 10 | 0.001212s | 0.001218s | 0.001180s | byte equality observed |
| test_time_generator.test | 10 | 0.001239s | 0.001230s | 0.001228s | byte equality observed |
| test_time_generator_v1.test | 10 | 0.001625s | 0.001744s | 0.001755s | byte equality observed |
| test_time_generator_v1_2.test | 10 | 0.001464s | 0.001484s | 0.001487s | byte equality observed |
| test_time_generator_v1_3.test | 6 | 0.003711s | 0.003651s | 0.003580s | byte equality observed |
| test_time_generator_v6.test | 10 | 0.001625s | 0.001733s | 0.001736s | byte equality observed |
| test_time_generator_v6_2.test | 10 | 0.001461s | 0.001487s | 0.001499s | byte equality observed |
| test_time_generator_v6_3.test | 6 | 0.003753s | 0.003583s | 0.003789s | byte equality observed |
| test_time_generator_v7.test | 10 | 0.001489s | 0.001484s | 0.001515s | byte equality observed |
| test_time_generator_v7_2.test | 10 | 0.001277s | 0.001261s | 0.001286s | byte equality observed |
| test_time_generator_v7_3.test | 9 | 0.001889s | 0.001828s | 0.001862s | byte equality observed |
| test_to_chars.test | 10 | 0.001275s | 0.001282s | 0.001276s | byte equality observed |
| test_to_chars_2.test | 10 | 0.001227s | 0.001231s | 0.001217s | byte equality observed |
| test_to_chars_2_no_simd.test | 10 | 0.001233s | 0.001212s | 0.001240s | byte equality observed |
| test_to_chars_3.test | 10 | 0.001219s | 0.001191s | 0.001219s | byte equality observed |
| test_to_chars_cx.test | 10 | 0.001210s | 0.001221s | 0.001218s | byte equality observed |
| test_to_chars_no_simd.test | 10 | 0.001292s | 0.001267s | 0.001279s | byte equality observed |
| test_uuid.test | 10 | 0.001176s | 0.001196s | 0.001192s | byte equality observed |
| test_uuid_2.test | 10 | 0.001214s | 0.001199s | 0.001224s | byte equality observed |
| test_uuid_3.test | 10 | 0.001193s | 0.001197s | 0.001179s | byte equality observed |
| test_uuid_class.test | 10 | 0.001214s | 0.001221s | 0.001215s | byte equality observed |
| test_uuid_clock.test | 10 | 0.001179s | 0.001186s | 0.001195s | byte equality observed |
| test_uuid_cx2.test | 10 | 0.001213s | 0.001223s | 0.001205s | byte equality observed |
| test_uuid_cx3.test | 10 | 0.001195s | 0.001204s | 0.001177s | byte equality observed |
| test_uuid_cx3_no_simd.test | 10 | 0.001174s | 0.001186s | 0.001186s | byte equality observed |
| test_uuid_from_string.test | 10 | 0.001221s | 0.001195s | 0.001201s | byte equality observed |
| test_uuid_from_string_2.test | 10 | 0.001486s | 0.001471s | 0.001508s | byte equality observed |
| test_uuid_from_string_3.test | 10 | 0.001293s | 0.001314s | 0.001299s | byte equality observed |
| test_uuid_from_string_cx.test | 10 | 0.001183s | 0.001206s | 0.001210s | byte equality observed |
| test_uuid_in_map.test | 10 | 0.001208s | 0.001247s | 0.001218s | byte equality observed |
| test_uuid_no_simd.test | 10 | 0.001169s | 0.001220s | 0.001186s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
