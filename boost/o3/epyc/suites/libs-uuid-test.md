# libs/uuid/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_include1.test | RUN | 0.419383 | 0.942624 | 1.438004 | 0.000698 | 0.000722 | 0.001737 |
| test_uuid.test | RUN | 0.249149 | 0.751880 | 0.986252 | 0.001830 | 0.001744 | 0.001734 |
| test_uuid_no_simd.test | RUN | 0.243944 | 0.714800 | 0.914146 | 0.001700 | 0.001604 | 0.001326 |
| test_uuid_2.test | RUN | 0.176350 | 0.556832 | 0.627624 | 0.001829 | 0.001823 | 0.001501 |
| test_uuid_3.test | RUN | 0.149766 | 0.380596 | 0.556041 | 0.001695 | 0.001915 | 0.001497 |
| test_alignment.test | RUN | 0.136164 | 0.336915 | 0.488214 | 0.001699 | 0.001575 | 0.001730 |
| test_alignment_2.test | RUN | 0.142293 | 0.354079 | 0.501363 | 0.001674 | 0.001499 | 0.001659 |
| test_attribute_packed.test | RUN | 0.133371 | 0.346218 | 0.466032 | 0.001666 | 0.001860 | 0.001259 |
| test_pragma_pack.test | RUN | 0.145845 | 0.346209 | 0.506699 | 0.001744 | 0.001777 | 0.001496 |
| test_data.test | RUN | 0.163334 | 0.627971 | 0.682265 | 0.001456 | 0.001708 | 0.001721 |
| test_comparison.test | RUN | 0.230387 | 0.723191 | 0.959602 | 0.001745 | 0.001779 | 0.001656 |
| test_comparison_no_simd.test | RUN | 0.230460 | 0.824338 | 0.793317 | 0.001549 | 0.001783 | 0.001778 |
| test_io.test | RUN | 0.292730 | 1.359730 | 1.381144 | 0.001896 | 0.001872 | 0.001773 |
| test_io_no_simd.test | RUN | 0.281687 | 1.336799 | 1.280051 | 0.001656 | 0.001872 | 0.001893 |
| test_io_2.test | RUN | 0.227193 | 1.398349 | 0.975702 | 0.001531 | 0.001820 | 0.001780 |
| test_io_2_no_simd.test | RUN | 0.204558 | 1.328005 | 0.844800 | 0.001913 | 0.001537 | 0.001840 |
| test_to_chars.test | RUN | 0.210478 | 0.874928 | 0.892895 | 0.001826 | 0.001936 | 0.001750 |
| test_to_chars_no_simd.test | RUN | 0.201823 | 0.897911 | 0.844029 | 0.001669 | 0.001884 | 0.001837 |
| test_to_chars_2.test | RUN | 0.242471 | 1.595830 | 1.061891 | 0.001681 | 0.001484 | 0.001577 |
| test_to_chars_2_no_simd.test | RUN | 0.240229 | 1.727779 | 0.962495 | 0.001200 | 0.001603 | 0.001500 |
| test_to_chars_3.test | RUN | 0.170660 | 0.586623 | 0.621834 | 0.001622 | 0.001383 | 0.001661 |
| test_from_chars.test | RUN | 0.197254 | 0.833864 | 0.733470 | 0.001584 | 0.001837 | 0.001687 |
| test_from_chars_no_simd.test | RUN | 0.165028 | 0.660959 | 0.623316 | 0.001616 | 0.001704 | 0.001626 |
| test_from_chars_2.test | RUN | 0.191241 | 0.785808 | 0.703972 | 0.001669 | 0.001674 | 0.001646 |
| test_from_chars_2_no_simd.test | RUN | 0.163335 | 0.600290 | 0.605300 | 0.001672 | 0.001683 | 0.001608 |
| test_uuid_from_string.test | RUN | 0.223510 | 0.859108 | 0.882940 | 0.001512 | 0.001804 | 0.001462 |
| test_uuid_from_string_2.test | RUN | 0.199000 | 0.784720 | 0.767997 | 0.001919 | 0.002076 | 0.002048 |
| test_uuid_from_string_3.test | RUN | 0.198556 | 0.785278 | 0.756265 | 0.001629 | 0.001842 | 0.001585 |
| test_uuid_clock.test | RUN | 0.128398 | 0.321861 | 0.478084 | 0.001492 | 0.001843 | 0.001541 |
| test_nil_generator.test | RUN | 0.146691 | 0.396765 | 0.563705 | 0.001622 | 0.001540 | 0.001724 |
| test_string_generator.test | RUN | 0.176423 | 0.622461 | 0.740965 | 0.001794 | 0.001928 | 0.001919 |
| test_string_generator_2.test | RUN | 0.205015 | 0.887715 | 0.846310 | 0.001536 | 0.001622 | 0.001853 |
| test_random_generator.test | RUN | 0.303320 | 1.709903 | 1.246345 | 0.002432 | 0.002602 | 0.002721 |
| test_random_generator_2.test | RUN | 0.224539 | 0.654782 | 0.835170 | 0.001472 | 0.001802 | 0.001750 |
| test_name_generator.test | RUN | 0.196289 | 2.197137 | 0.909989 | 0.001366 | 0.001801 | 0.001506 |
| test_namespaces.test | RUN | 0.160084 | 0.467592 | 0.629447 | 0.001355 | 0.001813 | 0.001473 |
| test_name_generator_md5.test | RUN | 0.198913 | 1.035771 | 0.866158 | 0.001556 | 0.001798 | 0.001792 |
| test_name_generator_sha1.test | RUN | 0.191975 | 3.202694 | 0.845591 | 0.001739 | 0.001888 | 0.001671 |
| test_time_generator.test | RUN | 0.219009 | 0.662893 | 0.833062 | 0.001546 | 0.001876 | 0.001800 |
| test_time_generator_v1.test | RUN | 0.243044 | 0.710151 | 0.939197 | 0.002108 | 0.002288 | 0.002264 |
| test_time_generator_v1_2.test | RUN | 0.242524 | 0.688464 | 0.893847 | 0.001904 | 0.002076 | 0.002055 |
| test_time_generator_v1_3.test | RUN | 0.268252 | 0.718600 | 0.937508 | 0.004320 | 0.004600 | 0.005523 |
| test_time_generator_v6.test | RUN | 0.242560 | 0.737526 | 0.966445 | 0.001792 | 0.002250 | 0.002062 |
| test_time_generator_v6_2.test | RUN | 0.237865 | 0.689313 | 0.910906 | 0.001782 | 0.002097 | 0.002007 |
| test_time_generator_v6_3.test | RUN | 0.263734 | 0.732925 | 0.992813 | 0.003978 | 0.004650 | 0.004957 |
| test_time_generator_v7.test | RUN | 0.228830 | 0.744067 | 0.888195 | 0.001556 | 0.002010 | 0.002078 |
| test_time_generator_v7_2.test | RUN | 0.214605 | 0.634677 | 0.846931 | 0.001669 | 0.001859 | 0.001806 |
| test_time_generator_v7_3.test | RUN | 0.217498 | 0.812859 | 0.869957 | 0.002062 | 0.002424 | 0.002303 |
| test_serialization.test | RUN | 0.408260 | 1.246286 | 1.492891 | 0.002063 | 0.001902 | 0.001792 |
| test_tagging.test | RUN | 0.226349 | 0.773912 | 0.872191 | 0.001268 | 0.001843 | 0.001742 |
| test_uuid_class.test | RUN | 0.237792 | 0.684776 | 0.918246 | 0.001596 | 0.001862 | 0.001818 |
| test_uuid_in_map.test | RUN | 0.260270 | 0.806321 | 0.966233 | 0.001543 | 0.001901 | 0.001582 |
| test_hash.test | RUN | 0.256802 | 0.723924 | 0.924213 | 0.001725 | 0.001732 | 0.001713 |
| test_hash_value.test | RUN | 0.179873 | 0.484447 | 0.674322 | 0.004698 | 0.004901 | 0.004727 |
| test_std_unordered.test | RUN | 0.246688 | 0.806456 | 0.902556 | 0.001604 | 0.001873 | 0.001849 |
| test_boost_unordered.test | RUN | 0.351543 | 1.024312 | 1.159095 | 0.001812 | 0.001638 | 0.001829 |
| test_detail_md5.test | RUN | 0.158459 | 0.492009 | 0.666497 | 0.001560 | 0.001636 | 0.001611 |
| test_detail_sha1.test | RUN | 0.152868 | 0.651491 | 0.585408 | 0.009849 | 0.009136 | 0.008593 |
| test_detail_endian.test | RUN | 0.117905 | 0.371625 | 0.405775 | 0.001548 | 0.001776 | 0.001718 |
| test_detail_chacha20.test | RUN | 0.130074 | 0.449958 | 0.470018 | 0.001539 | 0.001806 | 0.001548 |
| random_provider_no_copy_ctor.test | COMPILE_FAIL | 0.115046 | 0.260626 | 0.368568 | — | — | — |
| random_provider_no_copy_assign.test | COMPILE_FAIL | 0.115322 | 0.260595 | 0.362121 | — | — | — |
| test_msvc_simd_bug981648.test | RUN | 0.191994 | 0.510638 | 0.682005 | 0.001710 | 0.001834 | 0.001744 |
| test_bench_random.test | RUN | 0.218647 | 0.660953 | 0.850456 | 0.005835 | 0.005721 | 0.005402 |
| test_entropy_error.test | RUN | 0.111781 | 0.300499 | 0.396843 | 0.001620 | 0.001612 | 0.001511 |
| test_detail_random_provider.test | RUN | 0.219192 | 0.517406 | 0.791881 | 0.001448 | 0.001591 | 0.001791 |
| test_constants.test | RUN | 0.162229 | 0.474693 | 0.631262 | 0.001535 | 0.001689 | 0.001667 |
| test_uuid_cx.test | COMPILE | 0.097313 | 0.219791 | 0.319556 | — | — | — |
| test_uuid_cx2.test | RUN | 0.166192 | 0.551908 | 0.645706 | 0.001826 | 0.001624 | 0.001723 |
| test_uuid_cx3.test | RUN | 0.151327 | 0.424326 | 0.598482 | 0.001560 | 0.001471 | 0.001631 |
| test_uuid_cx3_no_simd.test | RUN | 0.139151 | 0.396663 | 0.552701 | 0.001695 | 0.001565 | 0.001423 |
| test_hash_value_cx.test | RUN | 0.144054 | 0.390107 | 0.517712 | 0.001351 | 0.001680 | 0.001398 |
| test_string_generator_cx.test | RUN | 0.160508 | 0.479200 | 0.602015 | 0.001750 | 0.001560 | 0.001685 |
| test_string_generator_cx2.test | RUN | 0.175756 | 0.587189 | 0.689991 | 0.001562 | 0.001774 | 0.001400 |
| test_constants_cx.test | RUN | 0.141278 | 0.339369 | 0.523283 | 0.001595 | 0.001549 | 0.001654 |
| test_from_chars_cx.test | RUN | 0.181040 | 0.642125 | 0.673941 | 0.001581 | 0.001872 | 0.001331 |
| test_from_chars_cx2.test | RUN | 0.186292 | 0.761332 | 0.711383 | 0.001769 | 0.001794 | 0.001690 |
| test_to_chars_cx.test | RUN | 0.184241 | 0.698249 | 0.782670 | 0.001691 | 0.001887 | 0.001278 |
| test_uuid_from_string_cx.test | RUN | 0.194881 | 0.706090 | 0.666147 | 0.001326 | 0.001407 | 0.001692 |
| quick.test | RUN | 0.204352 | 0.558699 | 0.746502 | 0.001144 | 0.001236 | 0.001204 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.991× | 0.997×, 0.985× | 0.993× |
| clang | 1.000× | 0.999×, 1.000× | 0.999× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224918).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| quick.test | 10 | 0.001135s | 0.001140s | 0.001152s | byte equality observed |
| test_alignment.test | 10 | 0.001140s | 0.001142s | 0.001158s | byte equality observed |
| test_alignment_2.test | 10 | 0.001140s | 0.001151s | 0.001165s | byte equality observed |
| test_attribute_packed.test | 5 | 0.001158s | 0.001144s | 0.001162s | byte equality observed |
| test_bench_random.test | 4 | 0.005182s | 0.005174s | 0.005190s | native test oracle; output varies |
| test_boost_unordered.test | 10 | 0.001221s | 0.001183s | 0.001251s | byte equality observed |
| test_comparison.test | 10 | 0.001173s | 0.001177s | 0.001171s | byte equality observed |
| test_comparison_no_simd.test | 10 | 0.001187s | 0.001182s | 0.001172s | byte equality observed |
| test_constants.test | 10 | 0.001158s | 0.001152s | 0.001152s | byte equality observed |
| test_constants_cx.test | 10 | 0.001128s | 0.001133s | 0.001141s | byte equality observed |
| test_data.test | 10 | 0.001138s | 0.001161s | 0.001165s | byte equality observed |
| test_detail_chacha20.test | 10 | 0.001151s | 0.001137s | 0.001153s | byte equality observed |
| test_detail_endian.test | 10 | 0.001126s | 0.001139s | 0.001141s | byte equality observed |
| test_detail_md5.test | 10 | 0.001169s | 0.001137s | 0.001158s | byte equality observed |
| test_detail_random_provider.test | 10 | 0.001159s | 0.001143s | 0.001131s | byte equality observed |
| test_detail_sha1.test | 3 | 0.008897s | 0.008487s | 0.007565s | byte equality observed |
| test_entropy_error.test | 10 | 0.001149s | 0.001123s | 0.001135s | byte equality observed |
| test_from_chars.test | 10 | 0.001140s | 0.001125s | 0.001151s | byte equality observed |
| test_from_chars_2.test | 10 | 0.001130s | 0.001135s | 0.001145s | byte equality observed |
| test_from_chars_2_no_simd.test | 10 | 0.001144s | 0.001149s | 0.001144s | byte equality observed |
| test_from_chars_cx.test | 10 | 0.001151s | 0.001118s | 0.001141s | byte equality observed |
| test_from_chars_cx2.test | 10 | 0.001149s | 0.001155s | 0.001134s | byte equality observed |
| test_from_chars_no_simd.test | 10 | 0.001152s | 0.001147s | 0.001176s | byte equality observed |
| test_hash.test | 10 | 0.001163s | 0.001154s | 0.001156s | byte equality observed |
| test_hash_value.test | 6 | 0.004260s | 0.004161s | 0.004071s | byte equality observed |
| test_hash_value_cx.test | 10 | 0.001130s | 0.001121s | 0.001148s | byte equality observed |
| test_include1.test | 10 | 0.000504s | 0.000496s | 0.001128s | byte equality observed |
| test_io.test | 10 | 0.001236s | 0.001200s | 0.001231s | byte equality observed |
| test_io_2.test | 10 | 0.001279s | 0.001272s | 0.001270s | byte equality observed |
| test_io_2_no_simd.test | 10 | 0.001289s | 0.001286s | 0.001313s | byte equality observed |
| test_io_no_simd.test | 10 | 0.001231s | 0.001240s | 0.001247s | byte equality observed |
| test_msvc_simd_bug981648.test | 10 | 0.001167s | 0.001151s | 0.001156s | native test oracle; output varies |
| test_name_generator.test | 10 | 0.001188s | 0.001198s | 0.001199s | byte equality observed |
| test_name_generator_md5.test | 10 | 0.001184s | 0.001191s | 0.001172s | byte equality observed |
| test_name_generator_sha1.test | 10 | 0.001198s | 0.001190s | 0.001169s | byte equality observed |
| test_namespaces.test | 10 | 0.001171s | 0.001148s | 0.001168s | byte equality observed |
| test_nil_generator.test | 10 | 0.001148s | 0.001146s | 0.001154s | byte equality observed |
| test_pragma_pack.test | 10 | 0.001159s | 0.001143s | 0.001151s | byte equality observed |
| test_random_generator.test | 9 | 0.002019s | 0.001991s | 0.001990s | native test oracle; output varies |
| test_random_generator_2.test | 10 | 0.001182s | 0.001168s | 0.001156s | byte equality observed |
| test_serialization.test | 10 | 0.001266s | 0.001254s | 0.001266s | byte equality observed |
| test_std_unordered.test | 10 | 0.001236s | 0.001242s | 0.001248s | byte equality observed |
| test_string_generator.test | 10 | 0.001258s | 0.001271s | 0.001289s | byte equality observed |
| test_string_generator_2.test | 10 | 0.001171s | 0.001184s | 0.001150s | byte equality observed |
| test_string_generator_cx.test | 10 | 0.001170s | 0.001168s | 0.001150s | byte equality observed |
| test_string_generator_cx2.test | 10 | 0.001169s | 0.001176s | 0.001150s | byte equality observed |
| test_tagging.test | 10 | 0.001174s | 0.001155s | 0.001174s | byte equality observed |
| test_time_generator.test | 10 | 0.001167s | 0.001169s | 0.001191s | byte equality observed |
| test_time_generator_v1.test | 10 | 0.001599s | 0.001677s | 0.001706s | byte equality observed |
| test_time_generator_v1_2.test | 10 | 0.001428s | 0.001450s | 0.001456s | byte equality observed |
| test_time_generator_v1_3.test | 6 | 0.003584s | 0.003507s | 0.003626s | byte equality observed |
| test_time_generator_v6.test | 10 | 0.001580s | 0.001669s | 0.001695s | byte equality observed |
| test_time_generator_v6_2.test | 10 | 0.001432s | 0.001385s | 0.001454s | byte equality observed |
| test_time_generator_v6_3.test | 5 | 0.003715s | 0.003582s | 0.003488s | byte equality observed |
| test_time_generator_v7.test | 10 | 0.001436s | 0.001455s | 0.001446s | byte equality observed |
| test_time_generator_v7_2.test | 10 | 0.001256s | 0.001248s | 0.001219s | byte equality observed |
| test_time_generator_v7_3.test | 9 | 0.001807s | 0.001811s | 0.001809s | byte equality observed |
| test_to_chars.test | 10 | 0.001234s | 0.001225s | 0.001229s | byte equality observed |
| test_to_chars_2.test | 10 | 0.001164s | 0.001181s | 0.001187s | byte equality observed |
| test_to_chars_2_no_simd.test | 10 | 0.001185s | 0.001178s | 0.001156s | byte equality observed |
| test_to_chars_3.test | 10 | 0.001142s | 0.001173s | 0.001167s | byte equality observed |
| test_to_chars_cx.test | 10 | 0.001166s | 0.001185s | 0.001159s | byte equality observed |
| test_to_chars_no_simd.test | 10 | 0.001224s | 0.001212s | 0.001237s | byte equality observed |
| test_uuid.test | 10 | 0.001168s | 0.001153s | 0.001153s | byte equality observed |
| test_uuid_2.test | 10 | 0.001155s | 0.001162s | 0.001165s | byte equality observed |
| test_uuid_3.test | 10 | 0.001152s | 0.001143s | 0.001144s | byte equality observed |
| test_uuid_class.test | 10 | 0.001158s | 0.001197s | 0.001161s | byte equality observed |
| test_uuid_clock.test | 10 | 0.001145s | 0.001152s | 0.001153s | byte equality observed |
| test_uuid_cx2.test | 10 | 0.001162s | 0.001152s | 0.001141s | byte equality observed |
| test_uuid_cx3.test | 10 | 0.001152s | 0.001132s | 0.001166s | byte equality observed |
| test_uuid_cx3_no_simd.test | 10 | 0.001149s | 0.001191s | 0.001158s | byte equality observed |
| test_uuid_from_string.test | 10 | 0.001166s | 0.001172s | 0.001185s | byte equality observed |
| test_uuid_from_string_2.test | 10 | 0.001451s | 0.001401s | 0.001439s | byte equality observed |
| test_uuid_from_string_3.test | 10 | 0.001240s | 0.001237s | 0.001229s | byte equality observed |
| test_uuid_from_string_cx.test | 10 | 0.001150s | 0.001183s | 0.001159s | byte equality observed |
| test_uuid_in_map.test | 10 | 0.001165s | 0.001183s | 0.001139s | byte equality observed |
| test_uuid_no_simd.test | 10 | 0.001150s | 0.001185s | 0.001141s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224918).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
