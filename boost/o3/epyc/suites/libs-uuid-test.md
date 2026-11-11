# libs/uuid/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_include1.test | RUN | 0.435587 | 0.850790 | 1.003665 | 0.000861 | 0.000794 | 0.001595 |
| test_uuid.test | RUN | 0.251254 | 0.697088 | 0.709345 | 0.002022 | 0.001984 | 0.001782 |
| test_uuid_no_simd.test | RUN | 0.242899 | 0.656519 | 0.662492 | 0.001821 | 0.001864 | 0.001394 |
| test_uuid_2.test | RUN | 0.183527 | 0.498489 | 0.479312 | 0.002021 | 0.001267 | 0.001591 |
| test_uuid_3.test | RUN | 0.159765 | 0.366440 | 0.369528 | 0.001554 | 0.001831 | 0.001329 |
| test_alignment.test | RUN | 0.143594 | 0.302833 | 0.337700 | 0.001478 | 0.001664 | 0.001644 |
| test_alignment_2.test | RUN | 0.150971 | 0.294330 | 0.350609 | 0.001634 | 0.001932 | 0.001424 |
| test_attribute_packed.test | RUN | 0.144096 | 0.299971 | 0.346619 | 0.001838 | 0.001684 | 0.001484 |
| test_pragma_pack.test | RUN | 0.141558 | 0.294086 | 0.344939 | 0.001726 | 0.001845 | 0.001368 |
| test_data.test | RUN | 0.173435 | 0.536489 | 0.486725 | 0.001916 | 0.001738 | 0.001682 |
| test_comparison.test | RUN | 0.253909 | 0.634755 | 0.710803 | 0.001758 | 0.001986 | 0.001722 |
| test_comparison_no_simd.test | RUN | 0.223786 | 0.710473 | 0.639276 | 0.001724 | 0.001745 | 0.001884 |
| test_io.test | RUN | 0.294888 | 1.232969 | 1.082201 | 0.002176 | 0.001980 | 0.002062 |
| test_io_no_simd.test | RUN | 0.279119 | 1.281263 | 1.023767 | 0.002027 | 0.001745 | 0.001843 |
| test_io_2.test | RUN | 0.234847 | 1.290403 | 0.764939 | 0.001749 | 0.001661 | 0.001644 |
| test_io_2_no_simd.test | RUN | 0.213731 | 1.231593 | 0.700373 | 0.001890 | 0.001742 | 0.001952 |
| test_to_chars.test | RUN | 0.226432 | 0.860285 | 0.686117 | 0.002134 | 0.001838 | 0.001815 |
| test_to_chars_no_simd.test | RUN | 0.228079 | 0.801302 | 0.615923 | 0.002082 | 0.001928 | 0.001962 |
| test_to_chars_2.test | RUN | 0.277682 | 1.419494 | 0.808851 | 0.002050 | 0.001718 | 0.001865 |
| test_to_chars_2_no_simd.test | RUN | 0.269688 | 1.535558 | 0.779341 | 0.002240 | 0.001746 | 0.001618 |
| test_to_chars_3.test | RUN | 0.189952 | 0.529985 | 0.525164 | 0.001930 | 0.002024 | 0.001701 |
| test_from_chars.test | RUN | 0.199615 | 0.747013 | 0.559680 | 0.001956 | 0.001795 | 0.001866 |
| test_from_chars_no_simd.test | RUN | 0.165660 | 0.596339 | 0.493256 | 0.001481 | 0.001802 | 0.001598 |
| test_from_chars_2.test | RUN | 0.192325 | 0.713074 | 0.551248 | 0.001740 | 0.001847 | 0.001530 |
| test_from_chars_2_no_simd.test | RUN | 0.163286 | 0.536567 | 0.431702 | 0.001766 | 0.001784 | 0.001465 |
| test_uuid_from_string.test | RUN | 0.225152 | 0.795382 | 0.637426 | 0.001688 | 0.001887 | 0.001472 |
| test_uuid_from_string_2.test | RUN | 0.190442 | 0.714432 | 0.544575 | 0.002392 | 0.002206 | 0.001752 |
| test_uuid_from_string_3.test | RUN | 0.198647 | 0.703463 | 0.551937 | 0.001658 | 0.002116 | 0.001645 |
| test_uuid_clock.test | RUN | 0.142962 | 0.280137 | 0.334560 | 0.001830 | 0.002152 | 0.001551 |
| test_nil_generator.test | RUN | 0.157190 | 0.336890 | 0.378455 | 0.001795 | 0.001922 | 0.001596 |
| test_string_generator.test | RUN | 0.177190 | 0.582481 | 0.520350 | 0.001702 | 0.002174 | 0.001822 |
| test_string_generator_2.test | RUN | 0.197991 | 0.808312 | 0.586689 | 0.001602 | 0.001895 | 0.001639 |
| test_random_generator.test | RUN | 0.312165 | 1.544110 | 0.915248 | 0.056069 | 0.059592 | 0.058883 |
| test_random_generator_2.test | RUN | 0.222427 | 0.607773 | 0.610183 | 0.001645 | 0.001928 | 0.001720 |
| test_name_generator.test | RUN | 0.194276 | 2.068844 | 0.683520 | 0.001277 | 0.001853 | 0.001375 |
| test_namespaces.test | RUN | 0.163330 | 0.423495 | 0.448968 | 0.001647 | 0.001978 | 0.001471 |
| test_name_generator_md5.test | RUN | 0.199541 | 0.939831 | 0.635133 | 0.001756 | 0.001931 | 0.001721 |
| test_name_generator_sha1.test | RUN | 0.191568 | 2.949155 | 0.628885 | 0.001943 | 0.001848 | 0.001729 |
| test_time_generator.test | RUN | 0.224840 | 0.591240 | 0.580833 | 0.001628 | 0.001951 | 0.002758 |
| test_time_generator_v1.test | RUN | 0.256843 | 0.640029 | 0.645548 | 0.002437 | 0.002512 | 0.002073 |
| test_time_generator_v1_2.test | RUN | 0.259423 | 0.614633 | 0.635739 | 0.001948 | 0.002242 | 0.001949 |
| test_time_generator_v1_3.test | RUN | 0.285717 | 0.631242 | 0.695003 | 0.004245 | 0.004318 | 0.003779 |
| test_time_generator_v6.test | RUN | 0.266658 | 0.679129 | 0.646812 | 0.002054 | 0.002359 | 0.002277 |
| test_time_generator_v6_2.test | RUN | 0.259446 | 0.623195 | 0.621128 | 0.001985 | 0.002161 | 0.001779 |
| test_time_generator_v6_3.test | RUN | 0.269160 | 0.648779 | 0.669091 | 0.003432 | 0.004804 | 0.004021 |
| test_time_generator_v7.test | RUN | 0.239288 | 0.642993 | 0.631038 | 0.002118 | 0.002148 | 0.001717 |
| test_time_generator_v7_2.test | RUN | 0.222563 | 0.590215 | 0.580875 | 0.001813 | 0.001918 | 0.001715 |
| test_time_generator_v7_3.test | RUN | 0.225422 | 0.713125 | 0.582934 | 0.030369 | 0.030623 | 0.030405 |
| test_serialization.test | RUN | 0.424176 | 1.109501 | 1.072609 | 0.002192 | 0.001956 | 0.001881 |
| test_tagging.test | RUN | 0.237935 | 0.723344 | 0.604962 | 0.001464 | 0.001797 | 0.001849 |
| test_uuid_class.test | RUN | 0.241571 | 0.604661 | 0.638402 | 0.001747 | 0.001604 | 0.001697 |
| test_uuid_in_map.test | RUN | 0.267682 | 0.699859 | 0.694803 | 0.001568 | 0.001600 | 0.001805 |
| test_hash.test | RUN | 0.272411 | 0.613159 | 0.677393 | 0.001772 | 0.001948 | 0.001836 |
| test_hash_value.test | RUN | 0.193921 | 0.429920 | 0.503737 | 0.004672 | 0.006033 | 0.004342 |
| test_std_unordered.test | RUN | 0.255665 | 0.701690 | 0.675284 | 0.001850 | 0.002022 | 0.001853 |
| test_boost_unordered.test | RUN | 0.363227 | 0.925177 | 0.878185 | 0.002054 | 0.002190 | 0.001496 |
| test_detail_md5.test | RUN | 0.167892 | 0.472325 | 0.495450 | 0.001948 | 0.001938 | 0.001638 |
| test_detail_sha1.test | RUN | 0.157648 | 0.598400 | 0.435169 | 0.006786 | 0.007458 | 0.006793 |
| test_detail_endian.test | RUN | 0.119176 | 0.372652 | 0.336486 | 0.001996 | 0.001888 | 0.001792 |
| test_detail_chacha20.test | RUN | 0.135991 | 0.390668 | 0.333482 | 0.001588 | 0.001511 | 0.001528 |
| random_provider_no_copy_ctor.test | COMPILE_FAIL | 0.130434 | 0.225036 | 0.245620 | — | — | — |
| random_provider_no_copy_assign.test | COMPILE_FAIL | 0.126710 | 0.230112 | 0.264829 | — | — | — |
| test_msvc_simd_bug981648.test | RUN | 0.222875 | 0.469592 | 0.488268 | 0.001941 | 0.001904 | 0.001745 |
| test_bench_random.test | RUN | 0.230564 | 0.585313 | 0.616455 | 0.310212 | 0.310682 | 0.310301 |
| test_entropy_error.test | RUN | 0.114828 | 0.270459 | 0.305701 | 0.001904 | 0.001816 | 0.001717 |
| test_detail_random_provider.test | RUN | 0.228602 | 0.491557 | 0.538766 | 0.003315 | 0.003389 | 0.001380 |
| test_constants.test | RUN | 0.170209 | 0.420945 | 0.374159 | 0.001875 | 0.001695 | 0.001346 |
| test_uuid_cx.test | COMPILE | 0.104274 | 0.238214 | 0.214939 | — | — | — |
| test_uuid_cx2.test | RUN | 0.169911 | 0.480029 | 0.489308 | 0.001622 | 0.001477 | 0.001257 |
| test_uuid_cx3.test | RUN | 0.134435 | 0.380096 | 0.442773 | 0.001880 | 0.001681 | 0.001442 |
| test_uuid_cx3_no_simd.test | RUN | 0.145557 | 0.341208 | 0.398888 | 0.001852 | 0.001426 | 0.001434 |
| test_hash_value_cx.test | RUN | 0.156741 | 0.334567 | 0.402153 | 0.001909 | 0.001295 | 0.001560 |
| test_string_generator_cx.test | RUN | 0.165107 | 0.415716 | 0.491430 | 0.001830 | 0.001680 | 0.001630 |
| test_string_generator_cx2.test | RUN | 0.174266 | 0.514430 | 0.547886 | 0.001910 | 0.001926 | 0.001326 |
| test_constants_cx.test | RUN | 0.151533 | 0.290768 | 0.380997 | 0.001902 | 0.001752 | 0.001428 |
| test_from_chars_cx.test | RUN | 0.191205 | 0.557198 | 0.528129 | 0.001898 | 0.001627 | 0.001322 |
| test_from_chars_cx2.test | RUN | 0.197026 | 0.700929 | 0.567951 | 0.001856 | 0.001508 | 0.001580 |
| test_to_chars_cx.test | RUN | 0.198205 | 0.651557 | 0.624523 | 0.001826 | 0.001586 | 0.001357 |
| test_uuid_from_string_cx.test | RUN | 0.192871 | 0.650087 | 0.582114 | 0.001588 | 0.001497 | 0.001304 |
| quick.test | RUN | 0.176479 | 0.546773 | 0.599187 | 0.001562 | 0.001655 | 0.001279 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.970× | 0.950×, 0.989× | 1.003× |
| clang | 1.008×† | 1.038×, 0.978× | 0.885× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| quick.test | 10 | 0.000962s | 0.000938s | 0.001043s | byte equality observed |
| test_alignment.test | 10 | 0.000958s | 0.000973s | 0.000995s | byte equality observed |
| test_alignment_2.test | 10 | 0.000975s | 0.000961s | 0.001014s | byte equality observed |
| test_attribute_packed.test | 10 | 0.000967s | 0.000943s | 0.000974s | byte equality observed |
| test_bench_random.test | 1 | 0.338488s | 0.322526s | 0.363252s | native test oracle; output varies |
| test_boost_unordered.test | 10 | 0.001598s | 0.001475s | 0.001608s | byte equality observed |
| test_comparison.test | 10 | 0.000980s | 0.001040s | 0.001077s | byte equality observed |
| test_comparison_no_simd.test | 10 | 0.001019s | 0.000970s | 0.001007s | byte equality observed |
| test_constants.test | 10 | 0.000979s | 0.000923s | 0.000978s | byte equality observed |
| test_constants_cx.test | 10 | 0.000931s | 0.000926s | 0.000954s | byte equality observed |
| test_data.test | 10 | 0.000990s | 0.000994s | 0.000973s | byte equality observed |
| test_detail_chacha20.test | 10 | 0.001010s | 0.000937s | 0.000959s | byte equality observed |
| test_detail_endian.test | 10 | 0.001008s | 0.000946s | 0.000995s | byte equality observed |
| test_detail_md5.test | 10 | 0.001067s | 0.000967s | 0.001041s | byte equality observed |
| test_detail_random_provider.test | 7 | 0.002654s | 0.002595s | 0.002676s | byte equality observed |
| test_detail_sha1.test | 4 | 0.004879s | 0.006092s | 0.005335s | byte equality observed |
| test_entropy_error.test | 10 | 0.001047s | 0.001037s | 0.001069s | byte equality observed |
| test_from_chars.test | 10 | 0.001020s | 0.000965s | 0.001049s | byte equality observed |
| test_from_chars_2.test | 10 | 0.001072s | 0.000963s | 0.001064s | byte equality observed |
| test_from_chars_2_no_simd.test | 10 | 0.001027s | 0.001024s | 0.001054s | byte equality observed |
| test_from_chars_cx.test | 10 | 0.001025s | 0.000986s | 0.001045s | byte equality observed |
| test_from_chars_cx2.test | 10 | 0.001047s | 0.000988s | 0.001063s | byte equality observed |
| test_from_chars_no_simd.test | 10 | 0.001055s | 0.000987s | 0.001053s | byte equality observed |
| test_hash.test | 10 | 0.001591s | 0.001509s | 0.001647s | byte equality observed |
| test_hash_value.test | 6 | 0.003392s | 0.003293s | 0.003317s | byte equality observed |
| test_hash_value_cx.test | 10 | 0.001047s | 0.000996s | 0.001128s | byte equality observed |
| test_include1.test | 10 | 0.000474s | 0.000463s | 0.001039s | byte equality observed |
| test_io.test | 10 | 0.001124s | 0.001113s | 0.001096s | byte equality observed |
| test_io_2.test | 10 | 0.001142s | 0.001158s | 0.001155s | byte equality observed |
| test_io_2_no_simd.test | 10 | 0.001144s | 0.001129s | 0.001134s | byte equality observed |
| test_io_no_simd.test | 10 | 0.001108s | 0.001081s | 0.001090s | byte equality observed |
| test_msvc_simd_bug981648.test | 10 | 0.001033s | 0.001041s | 0.001041s | native test oracle; output varies |
| test_name_generator.test | 10 | 0.001077s | 0.001073s | 0.001053s | byte equality observed |
| test_name_generator_md5.test | 10 | 0.001066s | 0.001027s | 0.001042s | byte equality observed |
| test_name_generator_sha1.test | 10 | 0.001071s | 0.001048s | 0.001076s | byte equality observed |
| test_namespaces.test | 10 | 0.001040s | 0.001004s | 0.001040s | byte equality observed |
| test_nil_generator.test | 10 | 0.001051s | 0.001009s | 0.001035s | byte equality observed |
| test_pragma_pack.test | 10 | 0.001050s | 0.001002s | 0.001033s | byte equality observed |
| test_random_generator.test | 1 | 0.083944s | 0.062326s | 0.111124s | native test oracle; output varies |
| test_random_generator_2.test | 10 | 0.001034s | 0.000981s | 0.001043s | byte equality observed |
| test_serialization.test | 10 | 0.001138s | 0.001074s | 0.001114s | byte equality observed |
| test_std_unordered.test | 10 | 0.001642s | 0.001610s | 0.001934s | byte equality observed |
| test_string_generator.test | 10 | 0.001115s | 0.001084s | 0.001111s | byte equality observed |
| test_string_generator_2.test | 10 | 0.001067s | 0.001053s | 0.001027s | byte equality observed |
| test_string_generator_cx.test | 10 | 0.001056s | 0.001004s | 0.001023s | byte equality observed |
| test_string_generator_cx2.test | 10 | 0.001074s | 0.001025s | 0.001072s | byte equality observed |
| test_tagging.test | 8 | 0.002140s | 0.001339s | 0.002612s | byte equality observed |
| test_time_generator.test | 10 | 0.001624s | 0.001139s | 0.002236s | byte equality observed |
| test_time_generator_v1.test | 10 | 0.001536s | 0.001475s | 0.001714s | byte equality observed |
| test_time_generator_v1_2.test | 10 | 0.001295s | 0.001261s | 0.001400s | byte equality observed |
| test_time_generator_v1_3.test | 6 | 0.003253s | 0.003274s | 0.003174s | byte equality observed |
| test_time_generator_v6.test | 10 | 0.001493s | 0.001453s | 0.001611s | byte equality observed |
| test_time_generator_v6_2.test | 10 | 0.001341s | 0.001256s | 0.001368s | byte equality observed |
| test_time_generator_v6_3.test | 6 | 0.003329s | 0.003195s | 0.003302s | byte equality observed |
| test_time_generator_v7.test | 10 | 0.001457s | 0.001361s | 0.001534s | byte equality observed |
| test_time_generator_v7_2.test | 10 | 0.001539s | 0.001448s | 0.001451s | byte equality observed |
| test_time_generator_v7_3.test | 1 | 0.032031s | 0.031574s | 0.031385s | byte equality observed |
| test_to_chars.test | 10 | 0.001115s | 0.001058s | 0.001113s | byte equality observed |
| test_to_chars_2.test | 10 | 0.001083s | 0.001023s | 0.001119s | byte equality observed |
| test_to_chars_2_no_simd.test | 10 | 0.001097s | 0.001053s | 0.001076s | byte equality observed |
| test_to_chars_3.test | 10 | 0.001088s | 0.001066s | 0.001087s | byte equality observed |
| test_to_chars_cx.test | 10 | 0.001103s | 0.001050s | 0.001087s | byte equality observed |
| test_to_chars_no_simd.test | 10 | 0.001140s | 0.001128s | 0.001167s | byte equality observed |
| test_uuid.test | 10 | 0.001086s | 0.001049s | 0.001092s | byte equality observed |
| test_uuid_2.test | 10 | 0.001094s | 0.001074s | 0.001083s | byte equality observed |
| test_uuid_3.test | 10 | 0.001080s | 0.001061s | 0.001263s | byte equality observed |
| test_uuid_class.test | 9 | 0.001094s | 0.001072s | 0.001125s | byte equality observed |
| test_uuid_clock.test | 10 | 0.001091s | 0.001080s | 0.001251s | byte equality observed |
| test_uuid_cx2.test | 10 | 0.001096s | 0.001070s | 0.001164s | byte equality observed |
| test_uuid_cx3.test | 10 | 0.001075s | 0.001048s | 0.001194s | byte equality observed |
| test_uuid_cx3_no_simd.test | 10 | 0.001082s | 0.001039s | 0.001074s | byte equality observed |
| test_uuid_from_string.test | 10 | 0.001106s | 0.001068s | 0.001128s | byte equality observed |
| test_uuid_from_string_2.test | 10 | 0.001318s | 0.001266s | 0.001346s | byte equality observed |
| test_uuid_from_string_3.test | 10 | 0.001168s | 0.001150s | 0.001159s | byte equality observed |
| test_uuid_from_string_cx.test | 10 | 0.001095s | 0.001026s | 0.001089s | byte equality observed |
| test_uuid_in_map.test | 10 | 0.001085s | 0.001080s | 0.001097s | byte equality observed |
| test_uuid_no_simd.test | 10 | 0.001087s | 0.001083s | 0.001082s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
