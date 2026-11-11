# libs/uuid/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_include1.test | RUN | 0.385116 | 0.785495 | 0.944469 | 0.001541 | 0.000822 | 0.001650 |
| test_uuid.test | RUN | 0.215501 | 0.496249 | 0.553048 | 0.001493 | 0.001791 | 0.001642 |
| test_uuid_no_simd.test | RUN | 0.201079 | 0.452921 | 0.510425 | 0.001635 | 0.001848 | 0.001587 |
| test_uuid_2.test | RUN | 0.140519 | 0.365739 | 0.382892 | 0.001597 | 0.001804 | 0.001522 |
| test_uuid_3.test | RUN | 0.140605 | 0.296103 | 0.345775 | 0.001780 | 0.001888 | 0.001564 |
| test_alignment.test | RUN | 0.129783 | 0.270402 | 0.319940 | 0.001654 | 0.001762 | 0.001538 |
| test_alignment_2.test | RUN | 0.129092 | 0.260374 | 0.318701 | 0.001317 | 0.001704 | 0.001552 |
| test_attribute_packed.test | RUN | 0.128886 | 0.258800 | 0.311243 | 0.001569 | 0.001899 | 0.001427 |
| test_pragma_pack.test | RUN | 0.130998 | 0.254065 | 0.309589 | 0.001816 | 0.001796 | 0.001614 |
| test_data.test | RUN | 0.139585 | 0.336292 | 0.387081 | 0.001848 | 0.001850 | 0.001688 |
| test_comparison.test | RUN | 0.213748 | 0.492611 | 0.533578 | 0.001511 | 0.001957 | 0.001942 |
| test_comparison_no_simd.test | RUN | 0.193658 | 0.461937 | 0.485177 | 0.001844 | 0.001940 | 0.001678 |
| test_io.test | RUN | 0.232204 | 0.736589 | 0.652532 | 0.002020 | 0.002183 | 0.001760 |
| test_io_no_simd.test | RUN | 0.205768 | 0.631893 | 0.576503 | 0.001747 | 0.001738 | 0.001438 |
| test_io_2.test | RUN | 0.176739 | 0.542764 | 0.520839 | 0.002016 | 0.001902 | 0.001588 |
| test_io_2_no_simd.test | RUN | 0.155948 | 0.452599 | 0.456208 | 0.001964 | 0.002218 | 0.001717 |
| test_to_chars.test | RUN | 0.169261 | 0.525230 | 0.482914 | 0.002032 | 0.002238 | 0.001980 |
| test_to_chars_no_simd.test | RUN | 0.154167 | 0.459639 | 0.437070 | 0.001887 | 0.002113 | 0.001757 |
| test_to_chars_2.test | RUN | 0.166740 | 0.553343 | 0.504612 | 0.001710 | 0.001682 | 0.001572 |
| test_to_chars_2_no_simd.test | RUN | 0.153128 | 0.509016 | 0.453145 | 0.001620 | 0.001660 | 0.001537 |
| test_to_chars_3.test | RUN | 0.153693 | 0.450592 | 0.423341 | 0.001825 | 0.001994 | 0.001679 |
| test_from_chars.test | RUN | 0.155243 | 0.475831 | 0.454365 | 0.001542 | 0.001606 | 0.001291 |
| test_from_chars_no_simd.test | RUN | 0.130286 | 0.327354 | 0.356223 | 0.001776 | 0.001767 | 0.001428 |
| test_from_chars_2.test | RUN | 0.153582 | 0.456037 | 0.439036 | 0.001696 | 0.001802 | 0.001742 |
| test_from_chars_2_no_simd.test | RUN | 0.137594 | 0.326713 | 0.347933 | 0.001850 | 0.001629 | 0.001404 |
| test_uuid_from_string.test | RUN | 0.182315 | 0.590119 | 0.538478 | 0.001644 | 0.001887 | 0.001417 |
| test_uuid_from_string_2.test | RUN | 0.157786 | 0.461927 | 0.455589 | 0.001913 | 0.002092 | 0.001736 |
| test_uuid_from_string_3.test | RUN | 0.159815 | 0.486339 | 0.459587 | 0.001736 | 0.001888 | 0.001628 |
| test_uuid_clock.test | RUN | 0.122358 | 0.252641 | 0.308574 | 0.001802 | 0.001804 | 0.001380 |
| test_nil_generator.test | RUN | 0.141179 | 0.296505 | 0.356918 | 0.001808 | 0.001833 | 0.001644 |
| test_string_generator.test | RUN | 0.146451 | 0.403977 | 0.419196 | 0.001737 | 0.001867 | 0.001693 |
| test_string_generator_2.test | RUN | 0.169127 | 0.492539 | 0.474928 | 0.001591 | 0.001930 | 0.001734 |
| test_random_generator.test | RUN | 0.256203 | 0.634941 | 0.640237 | 0.059162 | 0.059763 | 0.059537 |
| test_random_generator_2.test | RUN | 0.201911 | 0.456533 | 0.535323 | 0.001422 | 0.001774 | 0.001492 |
| test_name_generator.test | RUN | 0.157269 | 0.456130 | 0.478255 | 0.001487 | 0.001939 | 0.001462 |
| test_namespaces.test | RUN | 0.142563 | 0.333772 | 0.350404 | 0.001410 | 0.001760 | 0.001625 |
| test_name_generator_md5.test | RUN | 0.156044 | 0.505673 | 0.508517 | 0.001632 | 0.001814 | 0.001591 |
| test_name_generator_sha1.test | RUN | 0.149659 | 0.472528 | 0.491691 | 0.001645 | 0.001786 | 0.001901 |
| test_time_generator.test | RUN | 0.199205 | 0.456260 | 0.564756 | 0.001724 | 0.001754 | 0.001767 |
| test_time_generator_v1.test | RUN | 0.221456 | 0.545823 | 0.621805 | 0.003571 | 0.003894 | 0.004037 |
| test_time_generator_v1_2.test | RUN | 0.216268 | 0.505039 | 0.572550 | 0.002773 | 0.002973 | 0.002964 |
| test_time_generator_v1_3.test | RUN | 0.242567 | 0.594851 | 0.639669 | 0.005674 | 0.008515 | 0.007850 |
| test_time_generator_v6.test | RUN | 0.222717 | 0.536950 | 0.583509 | 0.003756 | 0.003774 | 0.003507 |
| test_time_generator_v6_2.test | RUN | 0.216060 | 0.506540 | 0.572032 | 0.002730 | 0.003061 | 0.002717 |
| test_time_generator_v6_3.test | RUN | 0.239225 | 0.618389 | 0.625532 | 0.009644 | 0.007425 | 0.007299 |
| test_time_generator_v7.test | RUN | 0.208250 | 0.512497 | 0.542929 | 0.002770 | 0.002887 | 0.002694 |
| test_time_generator_v7_2.test | RUN | 0.198642 | 0.471555 | 0.503102 | 0.001650 | 0.001936 | 0.001798 |
| test_time_generator_v7_3.test | RUN | 0.201213 | 0.457677 | 0.504840 | 0.029475 | 0.030471 | 0.030413 |
| test_serialization.test | RUN | 0.370964 | 0.950128 | 0.891440 | 0.002088 | 0.002163 | 0.002034 |
| test_tagging.test | RUN | 0.210256 | 0.483148 | 0.565909 | 0.001320 | 0.001653 | 0.001556 |
| test_uuid_class.test | RUN | 0.215053 | 0.484097 | 0.549816 | 0.001403 | 0.001552 | 0.001661 |
| test_uuid_in_map.test | RUN | 0.238682 | 0.593608 | 0.636751 | 0.001528 | 0.001878 | 0.001818 |
| test_hash.test | RUN | 0.239302 | 0.509798 | 0.608511 | 0.001886 | 0.001578 | 0.001820 |
| test_hash_value.test | RUN | 0.166042 | 0.402768 | 0.435457 | 0.009228 | 0.009946 | 0.010065 |
| test_std_unordered.test | RUN | 0.217513 | 0.540922 | 0.601683 | 0.001877 | 0.002238 | 0.002048 |
| test_boost_unordered.test | RUN | 0.302787 | 0.719532 | 0.743999 | 0.002241 | 0.002060 | 0.001995 |
| test_detail_md5.test | RUN | 0.145627 | 0.363323 | 0.378617 | 0.001901 | 0.001638 | 0.001665 |
| test_detail_sha1.test | RUN | 0.136817 | 0.327419 | 0.355688 | 0.016738 | 0.017240 | 0.017843 |
| test_detail_endian.test | RUN | 0.108270 | 0.256399 | 0.273009 | 0.001544 | 0.001490 | 0.001698 |
| test_detail_chacha20.test | RUN | 0.122392 | 0.242998 | 0.283016 | 0.001810 | 0.001652 | 0.001511 |
| random_provider_no_copy_ctor.test | COMPILE_FAIL | 0.108420 | 0.206327 | 0.246120 | — | — | — |
| random_provider_no_copy_assign.test | COMPILE_FAIL | 0.108199 | 0.203736 | 0.263515 | — | — | — |
| test_msvc_simd_bug981648.test | RUN | 0.194008 | 0.420608 | 0.465333 | 0.001871 | 0.001858 | 0.001724 |
| test_bench_random.test | RUN | 0.202724 | 0.430246 | 0.530988 | 0.307332 | 0.312375 | 0.315695 |
| test_entropy_error.test | RUN | 0.100947 | 0.225896 | 0.267686 | 0.001572 | 0.001738 | 0.001456 |
| test_detail_random_provider.test | RUN | 0.200798 | 0.387508 | 0.510639 | 0.002828 | 0.001784 | 0.001540 |
| test_constants.test | RUN | 0.143936 | 0.328592 | 0.403252 | 0.001636 | 0.001893 | 0.001380 |
| test_uuid_cx.test | COMPILE | 0.092603 | 0.177882 | 0.173698 | — | — | — |
| test_uuid_cx2.test | RUN | 0.144879 | 0.350822 | 0.400052 | 0.001644 | 0.001544 | 0.001669 |
| test_uuid_cx3.test | RUN | 0.119543 | 0.313331 | 0.378335 | 0.001862 | 0.001586 | 0.001291 |
| test_uuid_cx3_no_simd.test | RUN | 0.127373 | 0.280554 | 0.338572 | 0.001630 | 0.001841 | 0.001171 |
| test_hash_value_cx.test | RUN | 0.139551 | 0.298417 | 0.363157 | 0.001630 | 0.001629 | 0.001425 |
| test_string_generator_cx.test | RUN | 0.142648 | 0.332775 | 0.396583 | 0.001558 | 0.001556 | 0.001380 |
| test_string_generator_cx2.test | RUN | 0.143607 | 0.361991 | 0.413610 | 0.001836 | 0.001739 | 0.001403 |
| test_constants_cx.test | RUN | 0.135107 | 0.270482 | 0.348791 | 0.001736 | 0.001719 | 0.001548 |
| test_from_chars_cx.test | RUN | 0.154054 | 0.439196 | 0.452784 | 0.001751 | 0.001845 | 0.001608 |
| test_from_chars_cx2.test | RUN | 0.155693 | 0.455400 | 0.466623 | 0.001578 | 0.001836 | 0.001674 |
| test_to_chars_cx.test | RUN | 0.161178 | 0.492280 | 0.495149 | 0.001527 | 0.001580 | 0.001279 |
| test_uuid_from_string_cx.test | RUN | 0.155162 | 0.481114 | 0.467046 | 0.001441 | 0.001451 | 0.001198 |
| quick.test | RUN | 0.169062 | 0.458100 | 0.517403 | 0.001352 | 0.001456 | 0.001202 |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42562778965) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42562778965) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42562778965).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
