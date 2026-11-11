# libs/uuid/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_include1.test | RUN | 0.594300 | 1.325939 | 2.085622 | 0.000639 | 0.000970 | 0.002751 |
| test_uuid.test | RUN | 0.334199 | 1.052463 | 1.312912 | 0.001726 | 0.002050 | 0.001916 |
| test_uuid_no_simd.test | RUN | 0.310557 | 1.094146 | 1.190091 | 0.001529 | 0.002625 | 0.003469 |
| test_uuid_2.test | RUN | 0.225362 | 0.760329 | 0.833974 | 0.001966 | 0.002082 | 0.002739 |
| test_uuid_3.test | RUN | 0.201851 | 0.524240 | 0.684031 | 0.002117 | 0.002190 | 0.001987 |
| test_alignment.test | RUN | 0.179343 | 0.462353 | 0.644653 | 0.001870 | 0.001790 | 0.001630 |
| test_alignment_2.test | RUN | 0.172593 | 0.456036 | 0.638967 | 0.001926 | 0.001483 | 0.001376 |
| test_attribute_packed.test | RUN | 0.182716 | 0.452614 | 0.642396 | 0.001606 | 0.001990 | 0.001634 |
| test_pragma_pack.test | RUN | 0.196947 | 0.450703 | 0.607053 | 0.001773 | 0.001904 | 0.002189 |
| test_data.test | RUN | 0.238749 | 0.823279 | 0.891380 | 0.001684 | 0.001782 | 0.001675 |
| test_comparison.test | RUN | 0.322463 | 0.954111 | 1.167289 | 0.002072 | 0.003325 | 0.002285 |
| test_comparison_no_simd.test | RUN | 0.303585 | 1.075815 | 1.095843 | 0.002108 | 0.002596 | 0.001916 |
| test_io.test | RUN | 0.370414 | 2.099758 | 1.699045 | 0.002004 | 0.002952 | 0.001956 |
| test_io_no_simd.test | RUN | 0.361496 | 2.391714 | 1.621352 | 0.001948 | 0.002146 | 0.002656 |
| test_io_2.test | RUN | 0.315519 | 2.389753 | 1.205985 | 0.001940 | 0.001908 | 0.002124 |
| test_io_2_no_simd.test | RUN | 0.300041 | 2.055153 | 1.164332 | 0.001918 | 0.002950 | 0.002000 |
| test_to_chars.test | RUN | 0.322400 | 1.346051 | 1.112789 | 0.001898 | 0.003172 | 0.001549 |
| test_to_chars_no_simd.test | RUN | 0.306938 | 1.187367 | 1.095561 | 0.001634 | 0.002830 | 0.001507 |
| test_to_chars_2.test | RUN | 0.399630 | 2.674242 | 1.352139 | 0.001910 | 0.002350 | 0.001726 |
| test_to_chars_2_no_simd.test | RUN | 0.385165 | 2.947006 | 1.185569 | 0.001621 | 0.002046 | 0.001677 |
| test_to_chars_3.test | RUN | 0.248063 | 0.966448 | 0.865154 | 0.001816 | 0.002140 | 0.002403 |
| test_from_chars.test | RUN | 0.277280 | 1.231756 | 0.954922 | 0.001746 | 0.003541 | 0.001475 |
| test_from_chars_no_simd.test | RUN | 0.252157 | 0.900989 | 0.799798 | 0.001403 | 0.003127 | 0.001744 |
| test_from_chars_2.test | RUN | 0.272450 | 1.221765 | 0.960073 | 0.002100 | 0.002845 | 0.001950 |
| test_from_chars_2_no_simd.test | RUN | 0.236853 | 0.894685 | 0.778313 | 0.001782 | 0.002692 | 0.001642 |
| test_uuid_from_string.test | RUN | 0.298856 | 1.608419 | 1.117767 | 0.001760 | 0.001692 | 0.001896 |
| test_uuid_from_string_2.test | RUN | 0.257366 | 1.577066 | 0.949198 | 0.002089 | 0.003192 | 0.002346 |
| test_uuid_from_string_3.test | RUN | 0.263974 | 1.677568 | 1.006844 | 0.001930 | 0.003394 | 0.001765 |
| test_uuid_clock.test | RUN | 0.180161 | 0.724329 | 0.590832 | 0.002038 | 0.001688 | 0.002178 |
| test_nil_generator.test | RUN | 0.202107 | 0.863299 | 0.704705 | 0.002412 | 0.001980 | 0.002122 |
| test_string_generator.test | RUN | 0.241292 | 1.107782 | 1.019270 | 0.002224 | 0.002001 | 0.002565 |
| test_string_generator_2.test | RUN | 0.274946 | 1.602971 | 1.070846 | 0.001765 | 0.002918 | 0.002004 |
| test_random_generator.test | RUN | 0.456091 | 2.929341 | 1.559888 | 0.005605 | 0.005574 | 0.005044 |
| test_random_generator_2.test | RUN | 0.300945 | 1.233919 | 1.092627 | 0.001630 | 0.002962 | 0.002212 |
| test_name_generator.test | RUN | 0.287500 | 3.545449 | 1.156558 | 0.001814 | 0.002967 | 0.001746 |
| test_namespaces.test | RUN | 0.213935 | 0.691160 | 0.790311 | 0.001812 | 0.002522 | 0.002130 |
| test_name_generator_md5.test | RUN | 0.291022 | 1.654092 | 1.081776 | 0.002221 | 0.002293 | 0.001552 |
| test_name_generator_sha1.test | RUN | 0.310534 | 4.521292 | 1.073844 | 0.001982 | 0.002407 | 0.002114 |
| test_time_generator.test | RUN | 0.302536 | 1.125254 | 1.098442 | 0.001652 | 0.002872 | 0.001647 |
| test_time_generator_v1.test | RUN | 0.354674 | 1.325119 | 1.207146 | 0.002176 | 0.002677 | 0.002468 |
| test_time_generator_v1_2.test | RUN | 0.337767 | 1.189664 | 1.114797 | 0.002758 | 0.002458 | 0.002232 |
| test_time_generator_v1_3.test | RUN | 0.373266 | 1.233024 | 1.253704 | 0.016335 | 0.015324 | 0.017049 |
| test_time_generator_v6.test | RUN | 0.373964 | 1.331785 | 1.248892 | 0.002274 | 0.002979 | 0.002764 |
| test_time_generator_v6_2.test | RUN | 0.361190 | 1.365225 | 1.132245 | 0.001746 | 0.003249 | 0.001968 |
| test_time_generator_v6_3.test | RUN | 0.366613 | 1.423571 | 1.252678 | 0.015294 | 0.014355 | 0.019933 |
| test_time_generator_v7.test | RUN | 0.324175 | 1.349226 | 1.142087 | 0.002352 | 0.002429 | 0.002539 |
| test_time_generator_v7_2.test | RUN | 0.308492 | 1.084031 | 1.105138 | 0.002066 | 0.001877 | 0.002050 |
| test_time_generator_v7_3.test | RUN | 0.303949 | 1.272808 | 1.206781 | 0.003936 | 0.003898 | 0.004402 |
| test_serialization.test | RUN | 0.550456 | 1.776720 | 1.919610 | 0.002196 | 0.002162 | 0.002774 |
| test_tagging.test | RUN | 0.301223 | 1.107095 | 1.086006 | 0.001760 | 0.002044 | 0.001840 |
| test_uuid_class.test | RUN | 0.300614 | 0.916908 | 1.161977 | 0.001557 | 0.001824 | 0.002163 |
| test_uuid_in_map.test | RUN | 0.335492 | 1.029953 | 1.230053 | 0.001694 | 0.002172 | 0.001922 |
| test_hash.test | RUN | 0.345983 | 0.926007 | 1.189812 | 0.001745 | 0.001563 | 0.001593 |
| test_hash_value.test | RUN | 0.252603 | 0.656781 | 0.875798 | 0.006513 | 0.006400 | 0.006475 |
| test_std_unordered.test | RUN | 0.322415 | 1.034668 | 1.200202 | 0.001758 | 0.001852 | 0.001619 |
| test_boost_unordered.test | RUN | 0.426324 | 1.366978 | 1.515451 | 0.001821 | 0.002994 | 0.002000 |
| test_detail_md5.test | RUN | 0.215186 | 0.654053 | 0.817957 | 0.001871 | 0.001928 | 0.001830 |
| test_detail_sha1.test | RUN | 0.198123 | 0.851100 | 0.777535 | 0.009993 | 0.010448 | 0.009748 |
| test_detail_endian.test | RUN | 0.151423 | 0.481135 | 0.595636 | 0.001932 | 0.001686 | 0.001488 |
| test_detail_chacha20.test | RUN | 0.186735 | 0.617837 | 0.589133 | 0.001726 | 0.001851 | 0.001660 |
| random_provider_no_copy_ctor.test | COMPILE_FAIL | 0.147377 | 0.354483 | 0.506466 | — | — | — |
| random_provider_no_copy_assign.test | COMPILE_FAIL | 0.169929 | 0.340175 | 0.491655 | — | — | — |
| test_msvc_simd_bug981648.test | RUN | 0.276364 | 0.695372 | 0.894366 | 0.001701 | 0.001901 | 0.002236 |
| test_bench_random.test | RUN | 0.291248 | 0.862445 | 1.149175 | 0.017976 | 0.017834 | 0.017736 |
| test_entropy_error.test | RUN | 0.173653 | 0.388981 | 0.515069 | 0.001748 | 0.001923 | 0.002102 |
| test_detail_random_provider.test | RUN | 0.279732 | 0.669046 | 1.208621 | 0.002344 | 0.001956 | 0.001641 |
| test_constants.test | RUN | 0.205024 | 0.610915 | 0.813662 | 0.002002 | 0.001956 | 0.001838 |
| test_uuid_cx.test | COMPILE | 0.132071 | 0.288436 | 0.424535 | — | — | — |
| test_uuid_cx2.test | RUN | 0.230439 | 0.699444 | 0.884839 | 0.001732 | 0.002156 | 0.001419 |
| test_uuid_cx3.test | RUN | 0.201073 | 0.555821 | 0.753857 | 0.002058 | 0.001693 | 0.001499 |
| test_uuid_cx3_no_simd.test | RUN | 0.195332 | 0.540699 | 0.688274 | 0.001778 | 0.002032 | 0.001822 |
| test_hash_value_cx.test | RUN | 0.192522 | 0.505295 | 0.725121 | 0.002018 | 0.001683 | 0.001850 |
| test_string_generator_cx.test | RUN | 0.204728 | 0.611151 | 0.773427 | 0.001864 | 0.001882 | 0.001818 |
| test_string_generator_cx2.test | RUN | 0.219692 | 0.742273 | 0.877979 | 0.001724 | 0.003554 | 0.002764 |
| test_constants_cx.test | RUN | 0.202198 | 0.435715 | 0.672618 | 0.001745 | 0.002010 | 0.001644 |
| test_from_chars_cx.test | RUN | 0.234699 | 0.814283 | 0.835762 | 0.001946 | 0.002656 | 0.002062 |
| test_from_chars_cx2.test | RUN | 0.251131 | 1.016436 | 0.934441 | 0.001998 | 0.002823 | 0.002196 |
| test_to_chars_cx.test | RUN | 0.253217 | 0.964876 | 1.011495 | 0.001716 | 0.002416 | 0.001696 |
| test_uuid_from_string_cx.test | RUN | 0.254645 | 0.986339 | 0.963993 | 0.001626 | 0.001525 | 0.002370 |
| quick.test | RUN | 0.289333 | 0.773159 | 1.050821 | 0.002230 | 0.001622 | 0.002094 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.036×† | 1.017×, 1.055× | 0.927× |
| clang | 0.963×† | 0.917×, 1.010× | 1.016× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1353 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| quick.test | 7 | 0.001618s | 0.001656s | 0.001615s | byte equality observed |
| test_alignment.test | 8 | 0.001640s | 0.001633s | 0.001536s | byte equality observed |
| test_alignment_2.test | 7 | 0.001683s | 0.001641s | 0.001637s | byte equality observed |
| test_attribute_packed.test | 7 | 0.001668s | 0.001621s | 0.001575s | byte equality observed |
| test_bench_random.test | 2 | 0.018062s | 0.017977s | 0.018201s | native test oracle; output varies |
| test_boost_unordered.test | 8 | 0.001648s | 0.001698s | 0.001790s | byte equality observed |
| test_comparison.test | 8 | 0.001685s | 0.001693s | 0.001703s | byte equality observed |
| test_comparison_no_simd.test | 8 | 0.001689s | 0.001774s | 0.001620s | byte equality observed |
| test_constants.test | 8 | 0.001684s | 0.001698s | 0.001728s | byte equality observed |
| test_constants_cx.test | 8 | 0.001599s | 0.001585s | 0.001683s | byte equality observed |
| test_data.test | 8 | 0.001595s | 0.001795s | 0.001592s | byte equality observed |
| test_detail_chacha20.test | 6 | 0.001614s | 0.001655s | 0.001638s | byte equality observed |
| test_detail_endian.test | 8 | 0.001610s | 0.001586s | 0.001597s | byte equality observed |
| test_detail_md5.test | 8 | 0.001669s | 0.001679s | 0.001610s | byte equality observed |
| test_detail_random_provider.test | 9 | 0.001683s | 0.001687s | 0.001692s | byte equality observed |
| test_detail_sha1.test | 2 | 0.010135s | 0.010293s | 0.009671s | byte equality observed |
| test_entropy_error.test | 7 | 0.001620s | 0.001855s | 0.001572s | byte equality observed |
| test_from_chars.test | 8 | 0.001583s | 0.001699s | 0.001681s | byte equality observed |
| test_from_chars_2.test | 8 | 0.001693s | 0.001633s | 0.001582s | byte equality observed |
| test_from_chars_2_no_simd.test | 6 | 0.001597s | 0.001620s | 0.001682s | byte equality observed |
| test_from_chars_cx.test | 7 | 0.001649s | 0.001637s | 0.001596s | byte equality observed |
| test_from_chars_cx2.test | 7 | 0.001585s | 0.001686s | 0.001628s | byte equality observed |
| test_from_chars_no_simd.test | 8 | 0.001626s | 0.001592s | 0.001720s | byte equality observed |
| test_hash.test | 9 | 0.001599s | 0.001684s | 0.001716s | byte equality observed |
| test_hash_value.test | 3 | 0.006659s | 0.006803s | 0.006346s | byte equality observed |
| test_hash_value_cx.test | 10 | 0.001597s | 0.001691s | 0.001657s | byte equality observed |
| test_include1.test | 10 | 0.000809s | 0.000859s | 0.001581s | byte equality observed |
| test_io.test | 5 | 0.001730s | 0.001755s | 0.001703s | byte equality observed |
| test_io_2.test | 5 | 0.001789s | 0.001831s | 0.001750s | byte equality observed |
| test_io_2_no_simd.test | 6 | 0.001759s | 0.001842s | 0.001853s | byte equality observed |
| test_io_no_simd.test | 6 | 0.001671s | 0.001813s | 0.001713s | byte equality observed |
| test_msvc_simd_bug981648.test | 7 | 0.001701s | 0.001690s | 0.001615s | native test oracle; output varies |
| test_name_generator.test | 8 | 0.001667s | 0.001739s | 0.001602s | byte equality observed |
| test_name_generator_md5.test | 7 | 0.001617s | 0.001638s | 0.001629s | byte equality observed |
| test_name_generator_sha1.test | 8 | 0.001718s | 0.001707s | 0.001670s | byte equality observed |
| test_namespaces.test | 7 | 0.001611s | 0.001627s | 0.001595s | byte equality observed |
| test_nil_generator.test | 7 | 0.001650s | 0.001606s | 0.001614s | byte equality observed |
| test_pragma_pack.test | 7 | 0.001569s | 0.001664s | 0.001593s | byte equality observed |
| test_random_generator.test | 4 | 0.004838s | 0.004776s | 0.004858s | native test oracle; output varies |
| test_random_generator_2.test | 7 | 0.001607s | 0.001666s | 0.001680s | byte equality observed |
| test_serialization.test | 7 | 0.001745s | 0.001819s | 0.001749s | byte equality observed |
| test_std_unordered.test | 8 | 0.001700s | 0.001839s | 0.001716s | byte equality observed |
| test_string_generator.test | 5 | 0.001717s | 0.001948s | 0.001874s | byte equality observed |
| test_string_generator_2.test | 6 | 0.001601s | 0.001690s | 0.001607s | byte equality observed |
| test_string_generator_cx.test | 8 | 0.001596s | 0.001626s | 0.001630s | byte equality observed |
| test_string_generator_cx2.test | 6 | 0.001602s | 0.001626s | 0.001601s | byte equality observed |
| test_tagging.test | 7 | 0.001673s | 0.001685s | 0.001570s | byte equality observed |
| test_time_generator.test | 8 | 0.001638s | 0.001787s | 0.001584s | byte equality observed |
| test_time_generator_v1.test | 5 | 0.002190s | 0.002271s | 0.002226s | byte equality observed |
| test_time_generator_v1_2.test | 8 | 0.001920s | 0.002095s | 0.002081s | byte equality observed |
| test_time_generator_v1_3.test | 2 | 0.016240s | 0.015973s | 0.016518s | byte equality observed |
| test_time_generator_v6.test | 4 | 0.002237s | 0.002362s | 0.002285s | byte equality observed |
| test_time_generator_v6_2.test | 5 | 0.001910s | 0.002047s | 0.002137s | byte equality observed |
| test_time_generator_v6_3.test | 2 | 0.015548s | 0.015441s | 0.015137s | byte equality observed |
| test_time_generator_v7.test | 6 | 0.001903s | 0.002004s | 0.001921s | byte equality observed |
| test_time_generator_v7_2.test | 8 | 0.001721s | 0.001704s | 0.001652s | byte equality observed |
| test_time_generator_v7_3.test | 5 | 0.003272s | 0.003311s | 0.003281s | byte equality observed |
| test_to_chars.test | 7 | 0.001689s | 0.001677s | 0.001698s | byte equality observed |
| test_to_chars_2.test | 7 | 0.001680s | 0.001682s | 0.001645s | byte equality observed |
| test_to_chars_2_no_simd.test | 9 | 0.001634s | 0.001714s | 0.001692s | byte equality observed |
| test_to_chars_3.test | 5 | 0.001592s | 0.001801s | 0.001698s | byte equality observed |
| test_to_chars_cx.test | 7 | 0.001697s | 0.001855s | 0.001633s | byte equality observed |
| test_to_chars_no_simd.test | 7 | 0.001710s | 0.001920s | 0.001761s | byte equality observed |
| test_uuid.test | 8 | 0.001613s | 0.001623s | 0.001618s | byte equality observed |
| test_uuid_2.test | 9 | 0.001614s | 0.001698s | 0.001584s | byte equality observed |
| test_uuid_3.test | 8 | 0.001671s | 0.001640s | 0.001587s | byte equality observed |
| test_uuid_class.test | 6 | 0.001634s | 0.001700s | 0.001574s | byte equality observed |
| test_uuid_clock.test | 8 | 0.001602s | 0.001675s | 0.001608s | byte equality observed |
| test_uuid_cx2.test | 8 | 0.001670s | 0.001717s | 0.001580s | byte equality observed |
| test_uuid_cx3.test | 7 | 0.001672s | 0.001667s | 0.001523s | byte equality observed |
| test_uuid_cx3_no_simd.test | 8 | 0.001624s | 0.001832s | 0.001647s | byte equality observed |
| test_uuid_from_string.test | 7 | 0.001770s | 0.001821s | 0.001554s | byte equality observed |
| test_uuid_from_string_2.test | 7 | 0.002172s | 0.001982s | 0.002020s | byte equality observed |
| test_uuid_from_string_3.test | 7 | 0.001827s | 0.001743s | 0.001688s | byte equality observed |
| test_uuid_from_string_cx.test | 7 | 0.001643s | 0.001714s | 0.001655s | byte equality observed |
| test_uuid_in_map.test | 8 | 0.001632s | 0.001635s | 0.001588s | byte equality observed |
| test_uuid_no_simd.test | 8 | 0.001627s | 0.001724s | 0.001599s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
