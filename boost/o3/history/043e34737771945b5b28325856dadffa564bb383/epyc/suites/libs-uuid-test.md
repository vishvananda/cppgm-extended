# libs/uuid/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_include1.test | RUN | 0.378958 | 0.789744 | 0.926282 | 0.000746 | 0.000769 | 0.001610 |
| test_uuid.test | RUN | 0.227261 | 0.666995 | 0.697031 | 0.001830 | 0.001927 | 0.001590 |
| test_uuid_no_simd.test | RUN | 0.211835 | 0.610089 | 0.612850 | 0.001892 | 0.001650 | 0.001467 |
| test_uuid_2.test | RUN | 0.153790 | 0.470296 | 0.454120 | 0.001643 | 0.001627 | 0.001395 |
| test_uuid_3.test | RUN | 0.179316 | 0.336421 | 0.350516 | 0.001620 | 0.001867 | 0.001502 |
| test_alignment.test | RUN | 0.136300 | 0.274504 | 0.330937 | 0.001773 | 0.001709 | 0.001631 |
| test_alignment_2.test | RUN | 0.132432 | 0.265972 | 0.336319 | 0.001609 | 0.001780 | 0.001566 |
| test_attribute_packed.test | RUN | 0.124753 | 0.290494 | 0.334139 | 0.003009 | 0.001960 | 0.001293 |
| test_pragma_pack.test | RUN | 0.129074 | 0.287624 | 0.315874 | 0.001631 | 0.001924 | 0.001420 |
| test_data.test | RUN | 0.163499 | 0.544476 | 0.464061 | 0.001571 | 0.001897 | 0.001606 |
| test_comparison.test | RUN | 0.211716 | 0.616110 | 0.655015 | 0.001792 | 0.001992 | 0.001342 |
| test_comparison_no_simd.test | RUN | 0.235358 | 0.703212 | 0.581696 | 0.001520 | 0.001666 | 0.001807 |
| test_io.test | RUN | 0.259674 | 1.216177 | 0.969589 | 0.001818 | 0.001962 | 0.001760 |
| test_io_no_simd.test | RUN | 0.233455 | 1.219835 | 0.952153 | 0.001416 | 0.002008 | 0.001846 |
| test_io_2.test | RUN | 0.198355 | 1.251109 | 0.712943 | 0.001637 | 0.001928 | 0.001835 |
| test_io_2_no_simd.test | RUN | 0.177363 | 1.185522 | 0.647070 | 0.001845 | 0.002141 | 0.001648 |
| test_to_chars.test | RUN | 0.202302 | 0.858593 | 0.653951 | 0.001882 | 0.002162 | 0.001774 |
| test_to_chars_no_simd.test | RUN | 0.186477 | 0.812113 | 0.623658 | 0.001776 | 0.001742 | 0.001777 |
| test_to_chars_2.test | RUN | 0.244611 | 1.376620 | 0.768122 | 0.001475 | 0.001662 | 0.001276 |
| test_to_chars_2_no_simd.test | RUN | 0.233914 | 1.447549 | 0.711018 | 0.001597 | 0.001650 | 0.001726 |
| test_to_chars_3.test | RUN | 0.159419 | 0.506342 | 0.476506 | 0.001560 | 0.001760 | 0.001624 |
| test_from_chars.test | RUN | 0.180041 | 0.701303 | 0.517062 | 0.001462 | 0.001673 | 0.001658 |
| test_from_chars_no_simd.test | RUN | 0.151643 | 0.555775 | 0.441238 | 0.001523 | 0.001512 | 0.001515 |
| test_from_chars_2.test | RUN | 0.170617 | 0.673230 | 0.505962 | 0.001532 | 0.001742 | 0.001675 |
| test_from_chars_2_no_simd.test | RUN | 0.150342 | 0.506605 | 0.416376 | 0.001649 | 0.001950 | 0.001511 |
| test_uuid_from_string.test | RUN | 0.189931 | 0.770792 | 0.627863 | 0.001387 | 0.001811 | 0.001374 |
| test_uuid_from_string_2.test | RUN | 0.168004 | 0.690671 | 0.527919 | 0.001794 | 0.002346 | 0.001780 |
| test_uuid_from_string_3.test | RUN | 0.168263 | 0.737592 | 0.564970 | 0.001533 | 0.001900 | 0.001738 |
| test_uuid_clock.test | RUN | 0.114300 | 0.303976 | 0.327894 | 0.001114 | 0.002091 | 0.001549 |
| test_nil_generator.test | RUN | 0.140380 | 0.386027 | 0.412722 | 0.001677 | 0.001873 | 0.001745 |
| test_string_generator.test | RUN | 0.161024 | 0.573726 | 0.556680 | 0.001739 | 0.002208 | 0.001850 |
| test_string_generator_2.test | RUN | 0.182171 | 0.807515 | 0.605921 | 0.001551 | 0.001954 | 0.001697 |
| test_random_generator.test | RUN | 0.291349 | 1.500210 | 0.907849 | 0.058838 | 0.058876 | 0.059410 |
| test_random_generator_2.test | RUN | 0.193530 | 0.587274 | 0.569938 | 0.001452 | 0.001868 | 0.001785 |
| test_name_generator.test | RUN | 0.181869 | 1.920737 | 0.647254 | 0.001366 | 0.001569 | 0.001421 |
| test_namespaces.test | RUN | 0.140217 | 0.398703 | 0.435620 | 0.001562 | 0.002038 | 0.001539 |
| test_name_generator_md5.test | RUN | 0.193459 | 0.914491 | 0.604052 | 0.001694 | 0.001871 | 0.001688 |
| test_name_generator_sha1.test | RUN | 0.203024 | 2.702922 | 0.611635 | 0.001604 | 0.001587 | 0.001726 |
| test_time_generator.test | RUN | 0.215722 | 0.583430 | 0.563912 | 0.001526 | 0.001982 | 0.001623 |
| test_time_generator_v1.test | RUN | 0.244143 | 0.606610 | 0.648862 | 0.001830 | 0.002202 | 0.002025 |
| test_time_generator_v1_2.test | RUN | 0.226995 | 0.574125 | 0.604399 | 0.001830 | 0.003727 | 0.001828 |
| test_time_generator_v1_3.test | RUN | 0.240543 | 0.622561 | 0.660165 | 0.003884 | 0.004398 | 0.003825 |
| test_time_generator_v6.test | RUN | 0.231034 | 0.653178 | 0.634957 | 0.001764 | 0.002236 | 0.002110 |
| test_time_generator_v6_2.test | RUN | 0.225136 | 0.579244 | 0.602835 | 0.001738 | 0.001932 | 0.001697 |
| test_time_generator_v6_3.test | RUN | 0.249372 | 0.604392 | 0.680791 | 0.005305 | 0.004714 | 0.003938 |
| test_time_generator_v7.test | RUN | 0.223585 | 0.608624 | 0.625927 | 0.001914 | 0.002128 | 0.001877 |
| test_time_generator_v7_2.test | RUN | 0.209005 | 0.515802 | 0.575174 | 0.001426 | 0.001892 | 0.001386 |
| test_time_generator_v7_3.test | RUN | 0.209480 | 0.651629 | 0.598517 | 0.031724 | 0.030567 | 0.030562 |
| test_serialization.test | RUN | 0.376581 | 1.016887 | 1.131899 | 0.002048 | 0.001725 | 0.001906 |
| test_tagging.test | RUN | 0.205055 | 0.618681 | 0.580620 | 0.001713 | 0.001373 | 0.001730 |
| test_uuid_class.test | RUN | 0.217705 | 0.560020 | 0.626899 | 0.001609 | 0.001352 | 0.001554 |
| test_uuid_in_map.test | RUN | 0.264655 | 0.657517 | 0.694382 | 0.001737 | 0.001677 | 0.001488 |
| test_hash.test | RUN | 0.270825 | 0.577409 | 0.632643 | 0.001655 | 0.001844 | 0.001778 |
| test_hash_value.test | RUN | 0.181376 | 0.408938 | 0.478221 | 0.004435 | 0.004358 | 0.004218 |
| test_std_unordered.test | RUN | 0.237105 | 0.648009 | 0.636738 | 0.001747 | 0.001750 | 0.001817 |
| test_boost_unordered.test | RUN | 0.324526 | 0.840361 | 0.844591 | 0.001608 | 0.001746 | 0.001616 |
| test_detail_md5.test | RUN | 0.152381 | 0.402958 | 0.454633 | 0.001535 | 0.001647 | 0.001635 |
| test_detail_sha1.test | RUN | 0.145923 | 0.508359 | 0.405152 | 0.007037 | 0.007508 | 0.006572 |
| test_detail_endian.test | RUN | 0.112196 | 0.283788 | 0.306396 | 0.001570 | 0.001404 | 0.001631 |
| test_detail_chacha20.test | RUN | 0.126310 | 0.354131 | 0.314561 | 0.001468 | 0.001511 | 0.001533 |
| random_provider_no_copy_ctor.test | COMPILE_FAIL | 0.107504 | 0.204168 | 0.252717 | — | — | — |
| random_provider_no_copy_assign.test | COMPILE_FAIL | 0.107977 | 0.199386 | 0.257823 | — | — | — |
| test_msvc_simd_bug981648.test | RUN | 0.190863 | 0.406959 | 0.441634 | 0.001830 | 0.001765 | 0.001788 |
| test_bench_random.test | RUN | 0.209701 | 0.530494 | 0.650832 | 0.308248 | 0.311479 | 0.312397 |
| test_entropy_error.test | RUN | 0.105357 | 0.235114 | 0.291426 | 0.001534 | 0.001720 | 0.001741 |
| test_detail_random_provider.test | RUN | 0.197762 | 0.405006 | 0.502399 | 0.003040 | 0.002926 | 0.002959 |
| test_constants.test | RUN | 0.150326 | 0.396891 | 0.424707 | 0.001558 | 0.001497 | 0.001331 |
| test_uuid_cx.test | COMPILE | 0.091304 | 0.181741 | 0.208443 | — | — | — |
| test_uuid_cx2.test | RUN | 0.152002 | 0.427330 | 0.456548 | 0.001558 | 0.001619 | 0.001736 |
| test_uuid_cx3.test | RUN | 0.124038 | 0.318473 | 0.424064 | 0.001614 | 0.001537 | 0.001469 |
| test_uuid_cx3_no_simd.test | RUN | 0.133405 | 0.320933 | 0.376579 | 0.001483 | 0.001566 | 0.001351 |
| test_hash_value_cx.test | RUN | 0.143965 | 0.308445 | 0.372194 | 0.001763 | 0.001470 | 0.001334 |
| test_string_generator_cx.test | RUN | 0.146659 | 0.407501 | 0.432616 | 0.001844 | 0.001737 | 0.001388 |
| test_string_generator_cx2.test | RUN | 0.158200 | 0.500859 | 0.492247 | 0.001599 | 0.001600 | 0.001546 |
| test_constants_cx.test | RUN | 0.142830 | 0.296672 | 0.339640 | 0.001588 | 0.001450 | 0.001576 |
| test_from_chars_cx.test | RUN | 0.173732 | 0.541835 | 0.464065 | 0.001716 | 0.001718 | 0.001602 |
| test_from_chars_cx2.test | RUN | 0.181206 | 0.629848 | 0.499168 | 0.001695 | 0.001336 | 0.001418 |
| test_to_chars_cx.test | RUN | 0.180441 | 0.591063 | 0.555545 | 0.001576 | 0.001379 | 0.001225 |
| test_uuid_from_string_cx.test | RUN | 0.179297 | 0.587684 | 0.532162 | 0.001308 | 0.001269 | 0.001229 |
| quick.test | RUN | 0.175613 | 0.487168 | 0.531153 | 0.001262 | 0.001097 | 0.001144 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.985×† | 1.089×, 0.881× | 0.993× |
| clang | 0.967×† | 0.804×, 1.131× | 1.173× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| quick.test | 10 | 0.000963s | 0.000967s | 0.000965s | byte equality observed |
| test_alignment.test | 10 | 0.000960s | 0.000994s | 0.000941s | byte equality observed |
| test_alignment_2.test | 10 | 0.000974s | 0.000958s | 0.001012s | byte equality observed |
| test_attribute_packed.test | 10 | 0.000916s | 0.000931s | 0.000935s | byte equality observed |
| test_bench_random.test | 1 | 0.323431s | 0.323360s | 0.323306s | native test oracle; output varies |
| test_boost_unordered.test | 10 | 0.001533s | 0.001504s | 0.001494s | byte equality observed |
| test_comparison.test | 10 | 0.000941s | 0.000963s | 0.001029s | byte equality observed |
| test_comparison_no_simd.test | 10 | 0.000944s | 0.000955s | 0.000969s | byte equality observed |
| test_constants.test | 10 | 0.000948s | 0.000936s | 0.000974s | byte equality observed |
| test_constants_cx.test | 10 | 0.000910s | 0.000930s | 0.000953s | byte equality observed |
| test_data.test | 10 | 0.000946s | 0.000952s | 0.000973s | byte equality observed |
| test_detail_chacha20.test | 10 | 0.000990s | 0.000937s | 0.000988s | byte equality observed |
| test_detail_endian.test | 10 | 0.000974s | 0.000940s | 0.000978s | byte equality observed |
| test_detail_md5.test | 10 | 0.001012s | 0.000997s | 0.001015s | byte equality observed |
| test_detail_random_provider.test | 8 | 0.002600s | 0.002607s | 0.002625s | byte equality observed |
| test_detail_sha1.test | 4 | 0.004983s | 0.006036s | 0.004850s | byte equality observed |
| test_entropy_error.test | 10 | 0.001025s | 0.001042s | 0.000994s | byte equality observed |
| test_from_chars.test | 10 | 0.001014s | 0.000970s | 0.000995s | byte equality observed |
| test_from_chars_2.test | 10 | 0.001033s | 0.000977s | 0.001002s | byte equality observed |
| test_from_chars_2_no_simd.test | 10 | 0.001040s | 0.000969s | 0.001014s | byte equality observed |
| test_from_chars_cx.test | 10 | 0.001010s | 0.000989s | 0.001001s | byte equality observed |
| test_from_chars_cx2.test | 10 | 0.001019s | 0.000993s | 0.001022s | byte equality observed |
| test_from_chars_no_simd.test | 10 | 0.001033s | 0.000992s | 0.001032s | byte equality observed |
| test_hash.test | 10 | 0.001501s | 0.001562s | 0.001540s | byte equality observed |
| test_hash_value.test | 6 | 0.003496s | 0.003246s | 0.003190s | byte equality observed |
| test_hash_value_cx.test | 10 | 0.000997s | 0.001022s | 0.001000s | byte equality observed |
| test_include1.test | 10 | 0.000470s | 0.000485s | 0.001021s | byte equality observed |
| test_io.test | 10 | 0.001098s | 0.001132s | 0.001123s | byte equality observed |
| test_io_2.test | 10 | 0.001129s | 0.001173s | 0.001140s | byte equality observed |
| test_io_2_no_simd.test | 10 | 0.001131s | 0.001195s | 0.001136s | byte equality observed |
| test_io_no_simd.test | 10 | 0.001107s | 0.001164s | 0.001106s | byte equality observed |
| test_msvc_simd_bug981648.test | 10 | 0.001042s | 0.001091s | 0.001053s | native test oracle; output varies |
| test_name_generator.test | 10 | 0.001073s | 0.001097s | 0.001076s | byte equality observed |
| test_name_generator_md5.test | 10 | 0.001037s | 0.001112s | 0.001046s | byte equality observed |
| test_name_generator_sha1.test | 10 | 0.001069s | 0.001107s | 0.001097s | byte equality observed |
| test_namespaces.test | 10 | 0.001034s | 0.001055s | 0.001058s | byte equality observed |
| test_nil_generator.test | 10 | 0.001017s | 0.001074s | 0.001055s | byte equality observed |
| test_pragma_pack.test | 10 | 0.001013s | 0.001059s | 0.001074s | byte equality observed |
| test_random_generator.test | 1 | 0.068587s | 0.064412s | 0.062916s | native test oracle; output varies |
| test_random_generator_2.test | 10 | 0.001019s | 0.001047s | 0.001061s | byte equality observed |
| test_serialization.test | 10 | 0.001126s | 0.001153s | 0.001154s | byte equality observed |
| test_std_unordered.test | 10 | 0.001675s | 0.001631s | 0.001682s | byte equality observed |
| test_string_generator.test | 10 | 0.001120s | 0.001128s | 0.001119s | byte equality observed |
| test_string_generator_2.test | 10 | 0.001064s | 0.001074s | 0.001062s | byte equality observed |
| test_string_generator_cx.test | 10 | 0.001033s | 0.001062s | 0.001056s | byte equality observed |
| test_string_generator_cx2.test | 10 | 0.001031s | 0.001045s | 0.001066s | byte equality observed |
| test_tagging.test | 6 | 0.002347s | 0.002206s | 0.001859s | byte equality observed |
| test_time_generator.test | 10 | 0.001537s | 0.001438s | 0.001236s | byte equality observed |
| test_time_generator_v1.test | 10 | 0.001467s | 0.001603s | 0.001575s | byte equality observed |
| test_time_generator_v1_2.test | 7 | 0.001284s | 0.001337s | 0.001318s | byte equality observed |
| test_time_generator_v1_3.test | 3 | 0.003146s | 0.003167s | 0.003261s | byte equality observed |
| test_time_generator_v6.test | 10 | 0.001439s | 0.001569s | 0.001587s | byte equality observed |
| test_time_generator_v6_2.test | 10 | 0.001286s | 0.001370s | 0.001372s | byte equality observed |
| test_time_generator_v6_3.test | 6 | 0.003214s | 0.003382s | 0.003323s | byte equality observed |
| test_time_generator_v7.test | 10 | 0.001403s | 0.001378s | 0.001504s | byte equality observed |
| test_time_generator_v7_2.test | 10 | 0.001537s | 0.001576s | 0.001607s | byte equality observed |
| test_time_generator_v7_3.test | 1 | 0.032031s | 0.032508s | 0.032762s | byte equality observed |
| test_to_chars.test | 10 | 0.001103s | 0.001110s | 0.001092s | byte equality observed |
| test_to_chars_2.test | 10 | 0.001071s | 0.001086s | 0.001069s | byte equality observed |
| test_to_chars_2_no_simd.test | 10 | 0.001079s | 0.001081s | 0.001098s | byte equality observed |
| test_to_chars_3.test | 10 | 0.001075s | 0.001095s | 0.001084s | byte equality observed |
| test_to_chars_cx.test | 10 | 0.001079s | 0.001101s | 0.001095s | byte equality observed |
| test_to_chars_no_simd.test | 10 | 0.001158s | 0.001126s | 0.001139s | byte equality observed |
| test_uuid.test | 10 | 0.001061s | 0.001068s | 0.001079s | byte equality observed |
| test_uuid_2.test | 10 | 0.001093s | 0.001092s | 0.001094s | byte equality observed |
| test_uuid_3.test | 10 | 0.001063s | 0.001049s | 0.001081s | byte equality observed |
| test_uuid_class.test | 9 | 0.001103s | 0.001131s | 0.001117s | byte equality observed |
| test_uuid_clock.test | 10 | 0.001089s | 0.001089s | 0.001074s | byte equality observed |
| test_uuid_cx2.test | 10 | 0.001083s | 0.001111s | 0.001092s | byte equality observed |
| test_uuid_cx3.test | 10 | 0.001078s | 0.001092s | 0.001082s | byte equality observed |
| test_uuid_cx3_no_simd.test | 10 | 0.001074s | 0.001078s | 0.001070s | byte equality observed |
| test_uuid_from_string.test | 10 | 0.001095s | 0.001105s | 0.001101s | byte equality observed |
| test_uuid_from_string_2.test | 10 | 0.001302s | 0.001289s | 0.001319s | byte equality observed |
| test_uuid_from_string_3.test | 10 | 0.001157s | 0.001194s | 0.001177s | byte equality observed |
| test_uuid_from_string_cx.test | 10 | 0.001084s | 0.001106s | 0.001081s | byte equality observed |
| test_uuid_in_map.test | 10 | 0.001081s | 0.001131s | 0.001121s | byte equality observed |
| test_uuid_no_simd.test | 10 | 0.001071s | 0.001104s | 0.001083s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
