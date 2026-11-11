# libs/uuid/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_include1.test | RUN | 0.503406 | 1.005114 | 1.444423 | 0.001518 | 0.000753 | 0.001728 |
| test_uuid.test | RUN | 0.267882 | 0.602576 | 0.867774 | 0.001629 | 0.001835 | 0.001621 |
| test_uuid_no_simd.test | RUN | 0.259100 | 0.547096 | 0.753681 | 0.001440 | 0.001710 | 0.001585 |
| test_uuid_2.test | RUN | 0.174979 | 0.420810 | 0.561610 | 0.001749 | 0.001780 | 0.001835 |
| test_uuid_3.test | RUN | 0.178023 | 0.356495 | 0.498619 | 0.001509 | 0.001792 | 0.001860 |
| test_alignment.test | RUN | 0.162100 | 0.320254 | 0.434609 | 0.001755 | 0.001922 | 0.001890 |
| test_alignment_2.test | RUN | 0.160879 | 0.312463 | 0.478483 | 0.001806 | 0.001778 | 0.001847 |
| test_attribute_packed.test | RUN | 0.159274 | 0.314424 | 0.460458 | 0.001395 | 0.001588 | 0.001810 |
| test_pragma_pack.test | RUN | 0.159286 | 0.319578 | 0.466091 | 0.001747 | 0.001928 | 0.001825 |
| test_data.test | RUN | 0.180772 | 0.412239 | 0.578761 | 0.001656 | 0.001869 | 0.001793 |
| test_comparison.test | RUN | 0.244892 | 0.585714 | 0.774798 | 0.002230 | 0.001656 | 0.002017 |
| test_comparison_no_simd.test | RUN | 0.229584 | 0.551047 | 0.713219 | 0.002494 | 0.001579 | 0.002130 |
| test_io.test | RUN | 0.264607 | 0.854478 | 0.957367 | 0.002110 | 0.002079 | 0.001958 |
| test_io_no_simd.test | RUN | 0.243609 | 0.737876 | 0.830545 | 0.001928 | 0.001922 | 0.001767 |
| test_io_2.test | RUN | 0.217389 | 0.640804 | 0.786764 | 0.001943 | 0.001548 | 0.001744 |
| test_io_2_no_simd.test | RUN | 0.192358 | 0.524465 | 0.675281 | 0.001895 | 0.002187 | 0.002104 |
| test_to_chars.test | RUN | 0.201352 | 0.600381 | 0.698861 | 0.002250 | 0.002476 | 0.002343 |
| test_to_chars_no_simd.test | RUN | 0.185568 | 0.538418 | 0.639066 | 0.002140 | 0.002290 | 0.001968 |
| test_to_chars_2.test | RUN | 0.207553 | 0.661614 | 0.744543 | 0.001607 | 0.001848 | 0.002024 |
| test_to_chars_2_no_simd.test | RUN | 0.188528 | 0.592213 | 0.668751 | 0.001623 | 0.002023 | 0.001946 |
| test_to_chars_3.test | RUN | 0.187039 | 0.525786 | 0.649031 | 0.001876 | 0.001915 | 0.001841 |
| test_from_chars.test | RUN | 0.190390 | 0.550504 | 0.672025 | 0.001526 | 0.001645 | 0.001628 |
| test_from_chars_no_simd.test | RUN | 0.156937 | 0.384541 | 0.529858 | 0.001444 | 0.001788 | 0.001848 |
| test_from_chars_2.test | RUN | 0.185500 | 0.528867 | 0.651802 | 0.001634 | 0.001366 | 0.001890 |
| test_from_chars_2_no_simd.test | RUN | 0.162464 | 0.378114 | 0.510930 | 0.001820 | 0.001842 | 0.001800 |
| test_uuid_from_string.test | RUN | 0.218855 | 0.717944 | 0.795304 | 0.001618 | 0.001849 | 0.001880 |
| test_uuid_from_string_2.test | RUN | 0.188996 | 0.568760 | 0.642889 | 0.002238 | 0.002144 | 0.002227 |
| test_uuid_from_string_3.test | RUN | 0.190649 | 0.579978 | 0.688756 | 0.001766 | 0.002126 | 0.002052 |
| test_uuid_clock.test | RUN | 0.150792 | 0.304685 | 0.439200 | 0.001882 | 0.001680 | 0.001521 |
| test_nil_generator.test | RUN | 0.172796 | 0.360386 | 0.518596 | 0.001790 | 0.001819 | 0.001930 |
| test_string_generator.test | RUN | 0.184915 | 0.490437 | 0.647697 | 0.001837 | 0.002032 | 0.002010 |
| test_string_generator_2.test | RUN | 0.207471 | 0.613044 | 0.729314 | 0.001948 | 0.002003 | 0.001918 |
| test_random_generator.test | RUN | 0.321663 | 0.768210 | 0.965522 | 0.002925 | 0.002923 | 0.002868 |
| test_random_generator_2.test | RUN | 0.245425 | 0.558601 | 0.768052 | 0.001563 | 0.001824 | 0.001685 |
| test_name_generator.test | RUN | 0.195503 | 0.557961 | 0.651907 | 0.001758 | 0.001903 | 0.001758 |
| test_namespaces.test | RUN | 0.175777 | 0.410580 | 0.577275 | 0.001348 | 0.001672 | 0.001756 |
| test_name_generator_md5.test | RUN | 0.198367 | 0.599235 | 0.680614 | 0.001785 | 0.001934 | 0.001850 |
| test_name_generator_sha1.test | RUN | 0.189318 | 0.573441 | 0.698481 | 0.002042 | 0.001965 | 0.002018 |
| test_time_generator.test | RUN | 0.244889 | 0.572717 | 0.788092 | 0.001669 | 0.001678 | 0.001801 |
| test_time_generator_v1.test | RUN | 0.272542 | 0.647883 | 0.845445 | 0.004922 | 0.005252 | 0.005732 |
| test_time_generator_v1_2.test | RUN | 0.268194 | 0.617657 | 0.853511 | 0.003750 | 0.003784 | 0.003766 |
| test_time_generator_v1_3.test | RUN | 0.300761 | 0.719961 | 0.929090 | 0.012202 | 0.012547 | 0.012284 |
| test_time_generator_v6.test | RUN | 0.270589 | 0.661693 | 0.860460 | 0.005051 | 0.005020 | 0.005851 |
| test_time_generator_v6_2.test | RUN | 0.267837 | 0.615395 | 0.847835 | 0.003452 | 0.003696 | 0.003910 |
| test_time_generator_v6_3.test | RUN | 0.296741 | 0.735518 | 0.941379 | 0.010290 | 0.013289 | 0.007840 |
| test_time_generator_v7.test | RUN | 0.261219 | 0.617220 | 0.817848 | 0.003522 | 0.003412 | 0.003990 |
| test_time_generator_v7_2.test | RUN | 0.241367 | 0.570401 | 0.770944 | 0.002036 | 0.001992 | 0.002042 |
| test_time_generator_v7_3.test | RUN | 0.239122 | 0.536022 | 0.774743 | 0.002207 | 0.002779 | 0.002856 |
| test_serialization.test | RUN | 0.466269 | 1.105481 | 1.283482 | 0.002004 | 0.001969 | 0.002168 |
| test_tagging.test | RUN | 0.260281 | 0.555239 | 0.753070 | 0.001678 | 0.001742 | 0.001666 |
| test_uuid_class.test | RUN | 0.252467 | 0.594963 | 0.864159 | 0.001661 | 0.001877 | 0.001650 |
| test_uuid_in_map.test | RUN | 0.294015 | 0.682166 | 0.925132 | 0.001785 | 0.001813 | 0.001818 |
| test_hash.test | RUN | 0.294429 | 0.601125 | 0.881478 | 0.001950 | 0.001745 | 0.002168 |
| test_hash_value.test | RUN | 0.209372 | 0.470525 | 0.631225 | 0.011732 | 0.013245 | 0.013237 |
| test_std_unordered.test | RUN | 0.278466 | 0.628559 | 0.863913 | 0.001944 | 0.002163 | 0.002252 |
| test_boost_unordered.test | RUN | 0.365428 | 0.831650 | 1.071735 | 0.002188 | 0.002180 | 0.002189 |
| test_detail_md5.test | RUN | 0.170035 | 0.437070 | 0.545930 | 0.001844 | 0.001520 | 0.001881 |
| test_detail_sha1.test | RUN | 0.171766 | 0.388567 | 0.514860 | 0.023844 | 0.023409 | 0.025818 |
| test_detail_endian.test | RUN | 0.136247 | 0.298315 | 0.388046 | 0.001589 | 0.001940 | 0.001861 |
| test_detail_chacha20.test | RUN | 0.143486 | 0.301241 | 0.399529 | 0.001552 | 0.001727 | 0.001960 |
| random_provider_no_copy_ctor.test | COMPILE_FAIL | 0.138637 | 0.251622 | 0.387216 | — | — | — |
| random_provider_no_copy_assign.test | COMPILE_FAIL | 0.132400 | 0.261646 | 0.345043 | — | — | — |
| test_msvc_simd_bug981648.test | RUN | 0.236784 | 0.529029 | 0.698025 | 0.001665 | 0.001893 | 0.001615 |
| test_bench_random.test | RUN | 0.244060 | 0.517841 | 0.753438 | 0.006194 | 0.006372 | 0.006323 |
| test_entropy_error.test | RUN | 0.129851 | 0.269499 | 0.384715 | 0.001845 | 0.001908 | 0.001814 |
| test_detail_random_provider.test | RUN | 0.251621 | 0.498044 | 0.741185 | 0.001884 | 0.001592 | 0.001795 |
| test_constants.test | RUN | 0.171360 | 0.411521 | 0.539627 | 0.001746 | 0.001684 | 0.001398 |
| test_uuid_cx.test | COMPILE | 0.121066 | 0.224880 | 0.325505 | — | — | — |
| test_uuid_cx2.test | RUN | 0.182853 | 0.437977 | 0.564251 | 0.001722 | 0.001756 | 0.001762 |
| test_uuid_cx3.test | RUN | 0.180041 | 0.399899 | 0.567355 | 0.001640 | 0.001603 | 0.001642 |
| test_uuid_cx3_no_simd.test | RUN | 0.156434 | 0.349374 | 0.494807 | 0.001938 | 0.001740 | 0.001558 |
| test_hash_value_cx.test | RUN | 0.173704 | 0.362722 | 0.495864 | 0.001740 | 0.001808 | 0.001683 |
| test_string_generator_cx.test | RUN | 0.174488 | 0.400383 | 0.559872 | 0.001640 | 0.001668 | 0.001874 |
| test_string_generator_cx2.test | RUN | 0.179637 | 0.433433 | 0.619357 | 0.001646 | 0.001604 | 0.001731 |
| test_constants_cx.test | RUN | 0.170610 | 0.337290 | 0.488180 | 0.001639 | 0.001528 | 0.001630 |
| test_from_chars_cx.test | RUN | 0.192447 | 0.520321 | 0.656170 | 0.001414 | 0.001926 | 0.001628 |
| test_from_chars_cx2.test | RUN | 0.191064 | 0.537012 | 0.652426 | 0.001865 | 0.001636 | 0.001892 |
| test_to_chars_cx.test | RUN | 0.193391 | 0.576973 | 0.631576 | 0.001798 | 0.001846 | 0.001442 |
| test_uuid_from_string_cx.test | RUN | 0.191243 | 0.559343 | 0.684387 | 0.001509 | 0.001412 | 0.001301 |
| quick.test | RUN | 0.249358 | 0.507391 | 0.718890 | 0.001394 | 0.001288 | 0.001226 |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42570601152) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42570601152) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42570601152).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
