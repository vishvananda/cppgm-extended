# libs/uuid/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_include1.test | RUN | 0.585533 | 1.225757 | 1.729838 | 0.000755 | 0.000755 | 0.001654 |
| test_uuid.test | RUN | 0.364631 | 0.990708 | 1.225551 | 0.001743 | 0.001726 | 0.001647 |
| test_uuid_no_simd.test | RUN | 0.334661 | 0.964136 | 1.103779 | 0.001659 | 0.001600 | 0.001367 |
| test_uuid_2.test | RUN | 0.237849 | 0.723831 | 0.794611 | 0.001463 | 0.001602 | 0.001536 |
| test_uuid_3.test | RUN | 0.215335 | 0.502659 | 0.635332 | 0.001749 | 0.001696 | 0.001515 |
| test_alignment.test | RUN | 0.194664 | 0.463072 | 0.582596 | 0.001770 | 0.001442 | 0.001679 |
| test_alignment_2.test | RUN | 0.193696 | 0.433918 | 0.586606 | 0.001483 | 0.001492 | 0.001568 |
| test_attribute_packed.test | RUN | 0.200203 | 0.455354 | 0.574166 | 0.001725 | 0.001783 | 0.001485 |
| test_pragma_pack.test | RUN | 0.196070 | 0.457199 | 0.587621 | 0.001820 | 0.001698 | 0.001368 |
| test_data.test | RUN | 0.248356 | 0.833006 | 0.839300 | 0.001799 | 0.001754 | 0.001670 |
| test_comparison.test | RUN | 0.332990 | 0.960339 | 1.137219 | 0.001779 | 0.001748 | 0.001487 |
| test_comparison_no_simd.test | RUN | 0.300815 | 1.115854 | 1.000291 | 0.001756 | 0.001765 | 0.001465 |
| test_io.test | RUN | 0.398315 | 1.799171 | 1.698955 | 0.001862 | 0.001776 | 0.001754 |
| test_io_no_simd.test | RUN | 0.374953 | 1.781206 | 1.595954 | 0.001578 | 0.001879 | 0.001747 |
| test_io_2.test | RUN | 0.318878 | 1.864477 | 1.172173 | 0.001724 | 0.001643 | 0.001850 |
| test_io_2_no_simd.test | RUN | 0.283898 | 1.768939 | 1.062420 | 0.001806 | 0.001972 | 0.001668 |
| test_to_chars.test | RUN | 0.320227 | 1.169971 | 1.105709 | 0.001820 | 0.001864 | 0.001737 |
| test_to_chars_no_simd.test | RUN | 0.297765 | 1.190794 | 1.059550 | 0.001806 | 0.001687 | 0.001730 |
| test_to_chars_2.test | RUN | 0.392073 | 2.124498 | 1.305102 | 0.001786 | 0.001660 | 0.001597 |
| test_to_chars_2_no_simd.test | RUN | 0.376008 | 2.302078 | 1.176321 | 0.001640 | 0.001550 | 0.001652 |
| test_to_chars_3.test | RUN | 0.243261 | 0.773004 | 0.800704 | 0.001702 | 0.001816 | 0.001423 |
| test_from_chars.test | RUN | 0.282868 | 1.115667 | 0.905020 | 0.001580 | 0.001489 | 0.001688 |
| test_from_chars_no_simd.test | RUN | 0.242096 | 0.891102 | 0.743526 | 0.001662 | 0.001605 | 0.001639 |
| test_from_chars_2.test | RUN | 0.273315 | 1.049650 | 0.884429 | 0.001564 | 0.001632 | 0.001524 |
| test_from_chars_2_no_simd.test | RUN | 0.232668 | 0.787020 | 0.720192 | 0.001524 | 0.001680 | 0.001522 |
| test_uuid_from_string.test | RUN | 0.303041 | 1.135923 | 1.076181 | 0.001470 | 0.001686 | 0.001648 |
| test_uuid_from_string_2.test | RUN | 0.266306 | 1.039739 | 0.917740 | 0.002008 | 0.002083 | 0.001962 |
| test_uuid_from_string_3.test | RUN | 0.267509 | 1.020275 | 0.933081 | 0.001622 | 0.001809 | 0.001784 |
| test_uuid_clock.test | RUN | 0.179358 | 0.417763 | 0.550383 | 0.001732 | 0.001710 | 0.001653 |
| test_nil_generator.test | RUN | 0.211681 | 0.506401 | 0.647404 | 0.001436 | 0.001540 | 0.001671 |
| test_string_generator.test | RUN | 0.246986 | 0.820888 | 0.898404 | 0.001804 | 0.001857 | 0.001681 |
| test_string_generator_2.test | RUN | 0.281145 | 1.161343 | 1.005225 | 0.001557 | 0.001480 | 0.001716 |
| test_random_generator.test | RUN | 0.439000 | 2.267654 | 1.522784 | 0.004384 | 0.004493 | 0.004436 |
| test_random_generator_2.test | RUN | 0.309619 | 0.870283 | 1.043692 | 0.001508 | 0.001743 | 0.001499 |
| test_name_generator.test | RUN | 0.286742 | 3.027176 | 1.119258 | 0.001534 | 0.001643 | 0.001698 |
| test_namespaces.test | RUN | 0.219651 | 0.623077 | 0.739112 | 0.001705 | 0.001653 | 0.001424 |
| test_name_generator_md5.test | RUN | 0.288584 | 1.384466 | 1.084865 | 0.001672 | 0.001711 | 0.001660 |
| test_name_generator_sha1.test | RUN | 0.306425 | 4.445869 | 1.050314 | 0.001734 | 0.001810 | 0.001475 |
| test_time_generator.test | RUN | 0.316449 | 0.865672 | 1.010798 | 0.001560 | 0.001730 | 0.001538 |
| test_time_generator_v1.test | RUN | 0.356286 | 0.947809 | 1.156244 | 0.002076 | 0.002059 | 0.002330 |
| test_time_generator_v1_2.test | RUN | 0.349830 | 0.904255 | 1.072006 | 0.001982 | 0.002100 | 0.002052 |
| test_time_generator_v1_3.test | RUN | 0.375624 | 0.955643 | 1.200787 | 0.004065 | 0.004773 | 0.003739 |
| test_time_generator_v6.test | RUN | 0.361037 | 0.968776 | 1.146779 | 0.002298 | 0.002074 | 0.002252 |
| test_time_generator_v6_2.test | RUN | 0.352410 | 0.907710 | 1.076390 | 0.001823 | 0.002082 | 0.002022 |
| test_time_generator_v6_3.test | RUN | 0.375460 | 0.968204 | 1.200875 | 0.005119 | 0.004738 | 0.004641 |
| test_time_generator_v7.test | RUN | 0.343523 | 0.976308 | 1.094641 | 0.001881 | 0.001794 | 0.001976 |
| test_time_generator_v7_2.test | RUN | 0.320584 | 0.848274 | 1.007454 | 0.001865 | 0.001768 | 0.001861 |
| test_time_generator_v7_3.test | RUN | 0.311535 | 1.112165 | 1.030420 | 0.002862 | 0.003142 | 0.003072 |
| test_serialization.test | RUN | 0.561822 | 1.600548 | 1.831441 | 0.001888 | 0.001642 | 0.001753 |
| test_tagging.test | RUN | 0.317078 | 1.035562 | 1.026663 | 0.001709 | 0.001569 | 0.001592 |
| test_uuid_class.test | RUN | 0.328304 | 0.906890 | 1.081395 | 0.001518 | 0.001610 | 0.001565 |
| test_uuid_in_map.test | RUN | 0.364133 | 1.041376 | 1.201860 | 0.001790 | 0.001703 | 0.001688 |
| test_hash.test | RUN | 0.359318 | 0.948073 | 1.095537 | 0.001716 | 0.001659 | 0.001688 |
| test_hash_value.test | RUN | 0.261619 | 0.654784 | 0.807635 | 0.005714 | 0.005719 | 0.005498 |
| test_std_unordered.test | RUN | 0.342162 | 1.077113 | 1.108369 | 0.001780 | 0.001820 | 0.001778 |
| test_boost_unordered.test | RUN | 0.456354 | 1.362351 | 1.404931 | 0.001786 | 0.001530 | 0.001626 |
| test_detail_md5.test | RUN | 0.224716 | 0.668302 | 0.794731 | 0.001696 | 0.001681 | 0.001628 |
| test_detail_sha1.test | RUN | 0.212507 | 0.850449 | 0.720876 | 0.010719 | 0.011506 | 0.010126 |
| test_detail_endian.test | RUN | 0.165493 | 0.489466 | 0.533113 | 0.001519 | 0.001700 | 0.001670 |
| test_detail_chacha20.test | RUN | 0.185772 | 0.606584 | 0.536629 | 0.001479 | 0.001601 | 0.001604 |
| random_provider_no_copy_ctor.test | COMPILE_FAIL | 0.163902 | 0.338475 | 0.455013 | — | — | — |
| random_provider_no_copy_assign.test | COMPILE_FAIL | 0.162561 | 0.336573 | 0.462986 | — | — | — |
| test_msvc_simd_bug981648.test | RUN | 0.276438 | 0.659180 | 0.776955 | 0.001652 | 0.001725 | 0.001488 |
| test_bench_random.test | RUN | 0.316583 | 0.877403 | 1.044362 | 0.016101 | 0.015904 | 0.015755 |
| test_entropy_error.test | RUN | 0.155338 | 0.385592 | 0.476148 | 0.001423 | 0.001732 | 0.001563 |
| test_detail_random_provider.test | RUN | 0.307071 | 0.680480 | 0.932321 | 0.001670 | 0.001738 | 0.001569 |
| test_constants.test | RUN | 0.222022 | 0.626183 | 0.728985 | 0.001656 | 0.001712 | 0.001427 |
| test_uuid_cx.test | COMPILE | 0.135812 | 0.279978 | 0.372119 | — | — | — |
| test_uuid_cx2.test | RUN | 0.229264 | 0.710025 | 0.805069 | 0.001485 | 0.001520 | 0.001782 |
| test_uuid_cx3.test | RUN | 0.217680 | 0.561515 | 0.733685 | 0.001676 | 0.001548 | 0.001671 |
| test_uuid_cx3_no_simd.test | RUN | 0.195214 | 0.519904 | 0.636316 | 0.001662 | 0.001682 | 0.001576 |
| test_hash_value_cx.test | RUN | 0.210303 | 0.503210 | 0.656748 | 0.001443 | 0.001716 | 0.001668 |
| test_string_generator_cx.test | RUN | 0.223441 | 0.618561 | 0.745588 | 0.001506 | 0.001487 | 0.001651 |
| test_string_generator_cx2.test | RUN | 0.230683 | 0.767169 | 0.858300 | 0.001482 | 0.001710 | 0.001671 |
| test_constants_cx.test | RUN | 0.201054 | 0.440194 | 0.602031 | 0.001621 | 0.001759 | 0.001458 |
| test_from_chars_cx.test | RUN | 0.250050 | 0.845898 | 0.795188 | 0.001584 | 0.001712 | 0.001524 |
| test_from_chars_cx2.test | RUN | 0.253847 | 1.038142 | 0.889205 | 0.001653 | 0.001682 | 0.001646 |
| test_to_chars_cx.test | RUN | 0.262683 | 0.947552 | 0.945120 | 0.001653 | 0.001387 | 0.001092 |
| test_uuid_from_string_cx.test | RUN | 0.261485 | 0.957883 | 0.883809 | 0.001243 | 0.001145 | 0.001077 |
| quick.test | RUN | 0.286448 | 0.710422 | 0.880505 | 0.001203 | 0.001202 | 0.000979 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.000× | 1.011×, 0.989× | 1.001× |
| clang | 0.992× | 1.001×, 0.983× | 0.990× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| quick.test | 10 | 0.001186s | 0.001171s | 0.001191s | byte equality observed |
| test_alignment.test | 10 | 0.001183s | 0.001158s | 0.001184s | byte equality observed |
| test_alignment_2.test | 10 | 0.001155s | 0.001151s | 0.001175s | byte equality observed |
| test_attribute_packed.test | 10 | 0.001155s | 0.001128s | 0.001143s | byte equality observed |
| test_bench_random.test | 2 | 0.015466s | 0.015483s | 0.015441s | native test oracle; output varies |
| test_boost_unordered.test | 10 | 0.001227s | 0.001247s | 0.001189s | byte equality observed |
| test_comparison.test | 10 | 0.001189s | 0.001194s | 0.001186s | byte equality observed |
| test_comparison_no_simd.test | 10 | 0.001204s | 0.001233s | 0.001233s | byte equality observed |
| test_constants.test | 10 | 0.001185s | 0.001198s | 0.001167s | byte equality observed |
| test_constants_cx.test | 10 | 0.001179s | 0.001158s | 0.001193s | byte equality observed |
| test_data.test | 10 | 0.001181s | 0.001187s | 0.001184s | byte equality observed |
| test_detail_chacha20.test | 10 | 0.001184s | 0.001187s | 0.001182s | byte equality observed |
| test_detail_endian.test | 10 | 0.001206s | 0.001176s | 0.001187s | byte equality observed |
| test_detail_md5.test | 10 | 0.001246s | 0.001213s | 0.001225s | byte equality observed |
| test_detail_random_provider.test | 10 | 0.001231s | 0.001237s | 0.001232s | byte equality observed |
| test_detail_sha1.test | 3 | 0.009695s | 0.009852s | 0.009291s | byte equality observed |
| test_entropy_error.test | 10 | 0.001215s | 0.001180s | 0.001190s | byte equality observed |
| test_from_chars.test | 10 | 0.001187s | 0.001175s | 0.001217s | byte equality observed |
| test_from_chars_2.test | 10 | 0.001199s | 0.001199s | 0.001203s | byte equality observed |
| test_from_chars_2_no_simd.test | 10 | 0.001214s | 0.001197s | 0.001230s | byte equality observed |
| test_from_chars_cx.test | 10 | 0.001205s | 0.001190s | 0.001202s | byte equality observed |
| test_from_chars_cx2.test | 10 | 0.001210s | 0.001186s | 0.001207s | byte equality observed |
| test_from_chars_no_simd.test | 10 | 0.001221s | 0.001189s | 0.001213s | byte equality observed |
| test_hash.test | 10 | 0.001203s | 0.001202s | 0.001205s | byte equality observed |
| test_hash_value.test | 5 | 0.005225s | 0.004875s | 0.004764s | byte equality observed |
| test_hash_value_cx.test | 10 | 0.001209s | 0.001210s | 0.001179s | byte equality observed |
| test_include1.test | 10 | 0.000510s | 0.000524s | 0.001183s | byte equality observed |
| test_io.test | 10 | 0.001299s | 0.001290s | 0.001261s | byte equality observed |
| test_io_2.test | 10 | 0.001364s | 0.001376s | 0.001374s | byte equality observed |
| test_io_2_no_simd.test | 10 | 0.001387s | 0.001407s | 0.001373s | byte equality observed |
| test_io_no_simd.test | 10 | 0.001285s | 0.001292s | 0.001268s | byte equality observed |
| test_msvc_simd_bug981648.test | 10 | 0.001217s | 0.001209s | 0.001233s | native test oracle; output varies |
| test_name_generator.test | 10 | 0.001220s | 0.001232s | 0.001207s | byte equality observed |
| test_name_generator_md5.test | 10 | 0.001246s | 0.001226s | 0.001251s | byte equality observed |
| test_name_generator_sha1.test | 10 | 0.001264s | 0.001263s | 0.001256s | byte equality observed |
| test_namespaces.test | 10 | 0.001199s | 0.001190s | 0.001169s | byte equality observed |
| test_nil_generator.test | 10 | 0.001167s | 0.001193s | 0.001160s | byte equality observed |
| test_pragma_pack.test | 10 | 0.001166s | 0.001152s | 0.001167s | byte equality observed |
| test_random_generator.test | 5 | 0.004020s | 0.003967s | 0.003973s | native test oracle; output varies |
| test_random_generator_2.test | 10 | 0.001195s | 0.001187s | 0.001181s | byte equality observed |
| test_serialization.test | 10 | 0.001292s | 0.001305s | 0.001292s | byte equality observed |
| test_std_unordered.test | 10 | 0.001297s | 0.001255s | 0.001272s | byte equality observed |
| test_string_generator.test | 10 | 0.001325s | 0.001340s | 0.001352s | byte equality observed |
| test_string_generator_2.test | 10 | 0.001214s | 0.001238s | 0.001202s | byte equality observed |
| test_string_generator_cx.test | 10 | 0.001207s | 0.001192s | 0.001207s | byte equality observed |
| test_string_generator_cx2.test | 10 | 0.001216s | 0.001222s | 0.001235s | byte equality observed |
| test_tagging.test | 10 | 0.001224s | 0.001219s | 0.001209s | byte equality observed |
| test_time_generator.test | 10 | 0.001233s | 0.001223s | 0.001228s | byte equality observed |
| test_time_generator_v1.test | 10 | 0.001704s | 0.001768s | 0.001847s | byte equality observed |
| test_time_generator_v1_2.test | 10 | 0.001531s | 0.001539s | 0.001579s | byte equality observed |
| test_time_generator_v1_3.test | 4 | 0.004459s | 0.004584s | 0.004321s | byte equality observed |
| test_time_generator_v6.test | 9 | 0.001684s | 0.001797s | 0.001834s | byte equality observed |
| test_time_generator_v6_2.test | 10 | 0.001517s | 0.001556s | 0.001572s | byte equality observed |
| test_time_generator_v6_3.test | 5 | 0.004621s | 0.004262s | 0.004186s | byte equality observed |
| test_time_generator_v7.test | 10 | 0.001489s | 0.001504s | 0.001550s | byte equality observed |
| test_time_generator_v7_2.test | 10 | 0.001284s | 0.001237s | 0.001252s | byte equality observed |
| test_time_generator_v7_3.test | 7 | 0.002657s | 0.002647s | 0.002617s | byte equality observed |
| test_to_chars.test | 10 | 0.001256s | 0.001286s | 0.001285s | byte equality observed |
| test_to_chars_2.test | 10 | 0.001235s | 0.001205s | 0.001168s | byte equality observed |
| test_to_chars_2_no_simd.test | 10 | 0.001233s | 0.001213s | 0.001217s | byte equality observed |
| test_to_chars_3.test | 10 | 0.001209s | 0.001210s | 0.001213s | byte equality observed |
| test_to_chars_cx.test | 10 | 0.001231s | 0.001224s | 0.001227s | byte equality observed |
| test_to_chars_no_simd.test | 10 | 0.001317s | 0.001288s | 0.001271s | byte equality observed |
| test_uuid.test | 10 | 0.001210s | 0.001186s | 0.001217s | byte equality observed |
| test_uuid_2.test | 10 | 0.001221s | 0.001212s | 0.001195s | byte equality observed |
| test_uuid_3.test | 10 | 0.001204s | 0.001175s | 0.001206s | byte equality observed |
| test_uuid_class.test | 10 | 0.001216s | 0.001228s | 0.001226s | byte equality observed |
| test_uuid_clock.test | 10 | 0.001200s | 0.001194s | 0.001188s | byte equality observed |
| test_uuid_cx2.test | 10 | 0.001230s | 0.001227s | 0.001222s | byte equality observed |
| test_uuid_cx3.test | 10 | 0.001184s | 0.001192s | 0.001217s | byte equality observed |
| test_uuid_cx3_no_simd.test | 10 | 0.001224s | 0.001218s | 0.001202s | byte equality observed |
| test_uuid_from_string.test | 10 | 0.001204s | 0.001221s | 0.001223s | byte equality observed |
| test_uuid_from_string_2.test | 10 | 0.001589s | 0.001536s | 0.001632s | byte equality observed |
| test_uuid_from_string_3.test | 10 | 0.001319s | 0.001317s | 0.001323s | byte equality observed |
| test_uuid_from_string_cx.test | 10 | 0.001233s | 0.001228s | 0.001200s | byte equality observed |
| test_uuid_in_map.test | 10 | 0.001209s | 0.001205s | 0.001211s | byte equality observed |
| test_uuid_no_simd.test | 10 | 0.001209s | 0.001205s | 0.001210s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
