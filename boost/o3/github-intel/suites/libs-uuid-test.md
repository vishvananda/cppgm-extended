# libs/uuid/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_include1.test | RUN | 0.561257 | 1.114706 | 1.525755 | 0.000796 | 0.000734 | 0.001744 |
| test_uuid.test | RUN | 0.345728 | 0.923413 | 1.113905 | 0.001790 | 0.001804 | 0.001774 |
| test_uuid_no_simd.test | RUN | 0.331579 | 0.889168 | 0.989485 | 0.001717 | 0.001457 | 0.001730 |
| test_uuid_2.test | RUN | 0.228289 | 0.665836 | 0.735043 | 0.001517 | 0.001638 | 0.001622 |
| test_uuid_3.test | RUN | 0.202865 | 0.466825 | 0.578197 | 0.001796 | 0.001771 | 0.001701 |
| test_alignment.test | RUN | 0.182489 | 0.418935 | 0.523037 | 0.001758 | 0.001677 | 0.001774 |
| test_alignment_2.test | RUN | 0.182489 | 0.408770 | 0.527831 | 0.001502 | 0.001449 | 0.001580 |
| test_attribute_packed.test | RUN | 0.190292 | 0.422250 | 0.511453 | 0.001822 | 0.001914 | 0.001480 |
| test_pragma_pack.test | RUN | 0.184153 | 0.421449 | 0.523020 | 0.001796 | 0.001784 | 0.001755 |
| test_data.test | RUN | 0.244700 | 0.779222 | 0.751588 | 0.001772 | 0.001774 | 0.001667 |
| test_comparison.test | RUN | 0.307095 | 0.906827 | 1.008200 | 0.001835 | 0.001854 | 0.001752 |
| test_comparison_no_simd.test | RUN | 0.295806 | 1.028527 | 0.928675 | 0.001765 | 0.001701 | 0.003111 |
| test_io.test | RUN | 0.390490 | 1.740713 | 1.592598 | 0.002103 | 0.001930 | 0.001849 |
| test_io_no_simd.test | RUN | 0.369875 | 1.725537 | 1.492477 | 0.001684 | 0.001775 | 0.001867 |
| test_io_2.test | RUN | 0.319213 | 1.795649 | 1.088931 | 0.001592 | 0.001998 | 0.001811 |
| test_io_2_no_simd.test | RUN | 0.281093 | 1.701730 | 1.003322 | 0.001895 | 0.001837 | 0.001939 |
| test_to_chars.test | RUN | 0.318605 | 1.123350 | 1.024285 | 0.001921 | 0.002038 | 0.001866 |
| test_to_chars_no_simd.test | RUN | 0.288954 | 1.130160 | 0.984575 | 0.001664 | 0.001830 | 0.001899 |
| test_to_chars_2.test | RUN | 0.386245 | 1.984462 | 1.230389 | 0.001753 | 0.001955 | 0.001460 |
| test_to_chars_2_no_simd.test | RUN | 0.375519 | 2.182237 | 1.120558 | 0.001690 | 0.001561 | 0.001788 |
| test_to_chars_3.test | RUN | 0.235213 | 0.708581 | 0.732144 | 0.001614 | 0.001550 | 0.001789 |
| test_from_chars.test | RUN | 0.271842 | 1.043897 | 0.855457 | 0.001565 | 0.001709 | 0.001789 |
| test_from_chars_no_simd.test | RUN | 0.233070 | 0.817964 | 0.708134 | 0.001679 | 0.001736 | 0.001626 |
| test_from_chars_2.test | RUN | 0.261597 | 0.976108 | 0.827243 | 0.001437 | 0.001633 | 0.001512 |
| test_from_chars_2_no_simd.test | RUN | 0.220914 | 0.743039 | 0.680108 | 0.001616 | 0.001471 | 0.001754 |
| test_uuid_from_string.test | RUN | 0.286041 | 1.071354 | 0.996578 | 0.001346 | 0.001605 | 0.001564 |
| test_uuid_from_string_2.test | RUN | 0.255016 | 0.984357 | 0.841209 | 0.002043 | 0.001403 | 0.002262 |
| test_uuid_from_string_3.test | RUN | 0.255978 | 0.978649 | 0.871327 | 0.001774 | 0.001893 | 0.001996 |
| test_uuid_clock.test | RUN | 0.167986 | 0.394547 | 0.508929 | 0.001634 | 0.001747 | 0.001656 |
| test_nil_generator.test | RUN | 0.195893 | 0.462098 | 0.579872 | 0.001736 | 0.001631 | 0.001752 |
| test_string_generator.test | RUN | 0.234664 | 0.780690 | 0.840330 | 0.001715 | 0.001934 | 0.001836 |
| test_string_generator_2.test | RUN | 0.267056 | 1.094658 | 0.933517 | 0.001528 | 0.001890 | 0.001818 |
| test_random_generator.test | RUN | 0.424902 | 2.141768 | 1.388189 | 0.004098 | 0.003893 | 0.003956 |
| test_random_generator_2.test | RUN | 0.296823 | 0.803652 | 0.943558 | 0.001608 | 0.001764 | 0.001645 |
| test_name_generator.test | RUN | 0.286037 | 2.795836 | 1.039226 | 0.001632 | 0.001885 | 0.001907 |
| test_namespaces.test | RUN | 0.210886 | 0.574016 | 0.661773 | 0.001745 | 0.001748 | 0.001784 |
| test_name_generator_md5.test | RUN | 0.282490 | 1.309405 | 1.007334 | 0.001724 | 0.001795 | 0.001761 |
| test_name_generator_sha1.test | RUN | 0.299329 | 4.092117 | 0.973748 | 0.001948 | 0.001641 | 0.001819 |
| test_time_generator.test | RUN | 0.298212 | 0.812931 | 0.921419 | 0.001649 | 0.002052 | 0.001670 |
| test_time_generator_v1.test | RUN | 0.342874 | 0.905034 | 1.032584 | 0.002233 | 0.002368 | 0.002500 |
| test_time_generator_v1_2.test | RUN | 0.328975 | 0.834460 | 0.962881 | 0.002090 | 0.002226 | 0.002096 |
| test_time_generator_v1_3.test | RUN | 0.355247 | 0.872391 | 1.076211 | 0.005852 | 0.004882 | 0.005421 |
| test_time_generator_v6.test | RUN | 0.337893 | 0.891683 | 1.029593 | 0.002499 | 0.002190 | 0.002177 |
| test_time_generator_v6_2.test | RUN | 0.332144 | 0.814597 | 0.993688 | 0.002106 | 0.002079 | 0.002148 |
| test_time_generator_v6_3.test | RUN | 0.352853 | 0.876027 | 1.068030 | 0.004851 | 0.004345 | 0.004662 |
| test_time_generator_v7.test | RUN | 0.320508 | 0.878272 | 1.003589 | 0.002104 | 0.002222 | 0.002140 |
| test_time_generator_v7_2.test | RUN | 0.294742 | 0.770344 | 0.925160 | 0.001785 | 0.001850 | 0.001799 |
| test_time_generator_v7_3.test | RUN | 0.304997 | 0.993549 | 0.920479 | 0.002750 | 0.002901 | 0.002900 |
| test_serialization.test | RUN | 0.547809 | 1.501632 | 1.699588 | 0.001969 | 0.001663 | 0.001900 |
| test_tagging.test | RUN | 0.300678 | 0.959930 | 0.933593 | 0.001894 | 0.001528 | 0.001804 |
| test_uuid_class.test | RUN | 0.313891 | 0.843464 | 0.987814 | 0.001632 | 0.001716 | 0.001801 |
| test_uuid_in_map.test | RUN | 0.345011 | 0.958831 | 1.065546 | 0.001815 | 0.001555 | 0.001744 |
| test_hash.test | RUN | 0.342831 | 0.864233 | 0.980836 | 0.001695 | 0.001596 | 0.001740 |
| test_hash_value.test | RUN | 0.249201 | 0.596289 | 0.732556 | 0.005850 | 0.005684 | 0.005564 |
| test_std_unordered.test | RUN | 0.327647 | 0.985933 | 1.006108 | 0.001892 | 0.001883 | 0.001904 |
| test_boost_unordered.test | RUN | 0.455742 | 1.267649 | 1.290273 | 0.001588 | 0.001831 | 0.001644 |
| test_detail_md5.test | RUN | 0.220407 | 0.603466 | 0.733460 | 0.001632 | 0.001851 | 0.001682 |
| test_detail_sha1.test | RUN | 0.204532 | 0.796111 | 0.649128 | 0.010602 | 0.010885 | 0.010035 |
| test_detail_endian.test | RUN | 0.157669 | 0.448478 | 0.480674 | 0.001627 | 0.001859 | 0.001791 |
| test_detail_chacha20.test | RUN | 0.174650 | 0.550098 | 0.503571 | 0.001897 | 0.001488 | 0.001629 |
| random_provider_no_copy_ctor.test | COMPILE_FAIL | 0.148991 | 0.305098 | 0.400588 | — | — | — |
| random_provider_no_copy_assign.test | COMPILE_FAIL | 0.152898 | 0.309731 | 0.385614 | — | — | — |
| test_msvc_simd_bug981648.test | RUN | 0.263115 | 0.608398 | 0.708656 | 0.001782 | 0.001834 | 0.001775 |
| test_bench_random.test | RUN | 0.302489 | 0.815199 | 0.952237 | 0.012667 | 0.012760 | 0.012743 |
| test_entropy_error.test | RUN | 0.146809 | 0.369062 | 0.432209 | 0.001572 | 0.001760 | 0.001599 |
| test_detail_random_provider.test | RUN | 0.288876 | 0.624571 | 0.820290 | 0.001798 | 0.001785 | 0.001562 |
| test_constants.test | RUN | 0.211703 | 0.581464 | 0.661955 | 0.001786 | 0.002032 | 0.001661 |
| test_uuid_cx.test | COMPILE | 0.127969 | 0.271453 | 0.320162 | — | — | — |
| test_uuid_cx2.test | RUN | 0.225457 | 0.663797 | 0.728363 | 0.001612 | 0.001517 | 0.001751 |
| test_uuid_cx3.test | RUN | 0.203263 | 0.507719 | 0.668745 | 0.001627 | 0.001231 | 0.001761 |
| test_uuid_cx3_no_simd.test | RUN | 0.187866 | 0.488649 | 0.567880 | 0.001706 | 0.001582 | 0.001720 |
| test_hash_value_cx.test | RUN | 0.196075 | 0.463540 | 0.592855 | 0.001798 | 0.001772 | 0.001653 |
| test_string_generator_cx.test | RUN | 0.209688 | 0.578995 | 0.669448 | 0.001664 | 0.001627 | 0.001560 |
| test_string_generator_cx2.test | RUN | 0.223758 | 0.711611 | 0.764418 | 0.001689 | 0.001694 | 0.001708 |
| test_constants_cx.test | RUN | 0.196633 | 0.399451 | 0.538837 | 0.001648 | 0.001756 | 0.001625 |
| test_from_chars_cx.test | RUN | 0.237871 | 0.769259 | 0.723584 | 0.001802 | 0.001728 | 0.001504 |
| test_from_chars_cx2.test | RUN | 0.247556 | 0.957842 | 0.805921 | 0.001673 | 0.001514 | 0.001867 |
| test_to_chars_cx.test | RUN | 0.245868 | 0.885796 | 0.864809 | 0.001245 | 0.001593 | 0.001622 |
| test_uuid_from_string_cx.test | RUN | 0.252612 | 0.891974 | 0.802452 | 0.001254 | 0.001263 | 0.001316 |
| quick.test | RUN | 0.260917 | 0.691324 | 0.808345 | 0.001271 | 0.001271 | 0.001214 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.997× | 0.995×, 0.998× | 1.014× |
| clang | 0.998× | 0.999×, 0.997× | 0.992× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| quick.test | 10 | 0.001202s | 0.001200s | 0.001182s | byte equality observed |
| test_alignment.test | 10 | 0.001198s | 0.001219s | 0.001181s | byte equality observed |
| test_alignment_2.test | 10 | 0.001190s | 0.001182s | 0.001175s | byte equality observed |
| test_attribute_packed.test | 10 | 0.001172s | 0.001165s | 0.001148s | byte equality observed |
| test_bench_random.test | 2 | 0.012291s | 0.012232s | 0.012146s | native test oracle; output varies |
| test_boost_unordered.test | 10 | 0.001253s | 0.001224s | 0.001249s | byte equality observed |
| test_comparison.test | 10 | 0.001191s | 0.001207s | 0.001206s | byte equality observed |
| test_comparison_no_simd.test | 10 | 0.001265s | 0.001282s | 0.001246s | byte equality observed |
| test_constants.test | 10 | 0.001227s | 0.001217s | 0.001218s | byte equality observed |
| test_constants_cx.test | 10 | 0.001198s | 0.001196s | 0.001176s | byte equality observed |
| test_data.test | 10 | 0.001212s | 0.001222s | 0.001220s | byte equality observed |
| test_detail_chacha20.test | 10 | 0.001220s | 0.001206s | 0.001217s | byte equality observed |
| test_detail_endian.test | 10 | 0.001208s | 0.001219s | 0.001212s | byte equality observed |
| test_detail_md5.test | 10 | 0.001262s | 0.001266s | 0.001251s | byte equality observed |
| test_detail_random_provider.test | 10 | 0.001259s | 0.001252s | 0.001249s | byte equality observed |
| test_detail_sha1.test | 3 | 0.009081s | 0.009512s | 0.008661s | byte equality observed |
| test_entropy_error.test | 10 | 0.001226s | 0.001212s | 0.001247s | byte equality observed |
| test_from_chars.test | 10 | 0.001212s | 0.001209s | 0.001212s | byte equality observed |
| test_from_chars_2.test | 10 | 0.001222s | 0.001208s | 0.001217s | byte equality observed |
| test_from_chars_2_no_simd.test | 10 | 0.001225s | 0.001221s | 0.001214s | byte equality observed |
| test_from_chars_cx.test | 10 | 0.001210s | 0.001209s | 0.001197s | byte equality observed |
| test_from_chars_cx2.test | 10 | 0.001192s | 0.001237s | 0.001225s | byte equality observed |
| test_from_chars_no_simd.test | 10 | 0.001218s | 0.001237s | 0.001231s | byte equality observed |
| test_hash.test | 10 | 0.001226s | 0.001201s | 0.001213s | byte equality observed |
| test_hash_value.test | 4 | 0.004891s | 0.004844s | 0.004758s | byte equality observed |
| test_hash_value_cx.test | 10 | 0.001216s | 0.001208s | 0.001206s | byte equality observed |
| test_include1.test | 10 | 0.000535s | 0.000535s | 0.001192s | byte equality observed |
| test_io.test | 10 | 0.001284s | 0.001316s | 0.001287s | byte equality observed |
| test_io_2.test | 10 | 0.001355s | 0.001363s | 0.001387s | byte equality observed |
| test_io_2_no_simd.test | 10 | 0.001380s | 0.001381s | 0.001345s | byte equality observed |
| test_io_no_simd.test | 10 | 0.001311s | 0.001306s | 0.001316s | byte equality observed |
| test_msvc_simd_bug981648.test | 10 | 0.001213s | 0.001193s | 0.001237s | native test oracle; output varies |
| test_name_generator.test | 10 | 0.001257s | 0.001256s | 0.001209s | byte equality observed |
| test_name_generator_md5.test | 10 | 0.001244s | 0.001261s | 0.001256s | byte equality observed |
| test_name_generator_sha1.test | 10 | 0.001254s | 0.001276s | 0.001242s | byte equality observed |
| test_namespaces.test | 10 | 0.001233s | 0.001207s | 0.001217s | byte equality observed |
| test_nil_generator.test | 10 | 0.001177s | 0.001211s | 0.001175s | byte equality observed |
| test_pragma_pack.test | 10 | 0.001197s | 0.001176s | 0.001201s | byte equality observed |
| test_random_generator.test | 6 | 0.003396s | 0.003365s | 0.003370s | native test oracle; output varies |
| test_random_generator_2.test | 10 | 0.001206s | 0.001219s | 0.001184s | byte equality observed |
| test_serialization.test | 10 | 0.001334s | 0.001345s | 0.001348s | byte equality observed |
| test_std_unordered.test | 10 | 0.001311s | 0.001306s | 0.001282s | byte equality observed |
| test_string_generator.test | 10 | 0.001342s | 0.001337s | 0.001341s | byte equality observed |
| test_string_generator_2.test | 10 | 0.001245s | 0.001248s | 0.001220s | byte equality observed |
| test_string_generator_cx.test | 10 | 0.001233s | 0.001201s | 0.001208s | byte equality observed |
| test_string_generator_cx2.test | 10 | 0.001252s | 0.001215s | 0.001188s | byte equality observed |
| test_tagging.test | 10 | 0.001232s | 0.001247s | 0.001238s | byte equality observed |
| test_time_generator.test | 10 | 0.001269s | 0.001243s | 0.001220s | byte equality observed |
| test_time_generator_v1.test | 10 | 0.001700s | 0.001777s | 0.001850s | byte equality observed |
| test_time_generator_v1_2.test | 10 | 0.001516s | 0.001551s | 0.001598s | byte equality observed |
| test_time_generator_v1_3.test | 4 | 0.004534s | 0.004073s | 0.004606s | byte equality observed |
| test_time_generator_v6.test | 10 | 0.001710s | 0.001802s | 0.001817s | byte equality observed |
| test_time_generator_v6_2.test | 10 | 0.001494s | 0.001565s | 0.001528s | byte equality observed |
| test_time_generator_v6_3.test | 4 | 0.004805s | 0.004411s | 0.004474s | byte equality observed |
| test_time_generator_v7.test | 9 | 0.001511s | 0.001541s | 0.001527s | byte equality observed |
| test_time_generator_v7_2.test | 10 | 0.001282s | 0.001298s | 0.001269s | byte equality observed |
| test_time_generator_v7_3.test | 8 | 0.002332s | 0.002313s | 0.002310s | byte equality observed |
| test_to_chars.test | 10 | 0.001300s | 0.001281s | 0.001286s | byte equality observed |
| test_to_chars_2.test | 10 | 0.001242s | 0.001233s | 0.001223s | byte equality observed |
| test_to_chars_2_no_simd.test | 10 | 0.001222s | 0.001243s | 0.001201s | byte equality observed |
| test_to_chars_3.test | 10 | 0.001246s | 0.001234s | 0.001213s | byte equality observed |
| test_to_chars_cx.test | 10 | 0.001245s | 0.001241s | 0.001216s | byte equality observed |
| test_to_chars_no_simd.test | 10 | 0.001325s | 0.001308s | 0.001263s | byte equality observed |
| test_uuid.test | 10 | 0.001205s | 0.001229s | 0.001217s | byte equality observed |
| test_uuid_2.test | 10 | 0.001235s | 0.001214s | 0.001228s | byte equality observed |
| test_uuid_3.test | 10 | 0.001203s | 0.001216s | 0.001197s | byte equality observed |
| test_uuid_class.test | 10 | 0.001227s | 0.001254s | 0.001218s | byte equality observed |
| test_uuid_clock.test | 10 | 0.001199s | 0.001224s | 0.001198s | byte equality observed |
| test_uuid_cx2.test | 10 | 0.001227s | 0.001255s | 0.001209s | byte equality observed |
| test_uuid_cx3.test | 10 | 0.001204s | 0.001218s | 0.001207s | byte equality observed |
| test_uuid_cx3_no_simd.test | 10 | 0.001215s | 0.001202s | 0.001203s | byte equality observed |
| test_uuid_from_string.test | 10 | 0.001249s | 0.001213s | 0.001215s | byte equality observed |
| test_uuid_from_string_2.test | 10 | 0.001551s | 0.001532s | 0.001564s | byte equality observed |
| test_uuid_from_string_3.test | 10 | 0.001331s | 0.001319s | 0.001303s | byte equality observed |
| test_uuid_from_string_cx.test | 10 | 0.001223s | 0.001219s | 0.001210s | byte equality observed |
| test_uuid_in_map.test | 10 | 0.001228s | 0.001241s | 0.001205s | byte equality observed |
| test_uuid_no_simd.test | 10 | 0.001207s | 0.001212s | 0.001200s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
