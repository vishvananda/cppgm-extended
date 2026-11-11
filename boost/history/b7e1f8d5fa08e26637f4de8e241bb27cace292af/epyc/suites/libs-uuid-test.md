# libs/uuid/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_include1.test | RUN | 0.469997 | 0.957096 | 1.445863 | 0.001520 | 0.000800 | 0.001768 |
| test_uuid.test | RUN | 0.260715 | 0.580702 | 0.839267 | 0.001662 | 0.001836 | 0.001801 |
| test_uuid_no_simd.test | RUN | 0.241421 | 0.543116 | 0.791226 | 0.001654 | 0.001762 | 0.001844 |
| test_uuid_2.test | RUN | 0.171570 | 0.417498 | 0.569006 | 0.001624 | 0.001372 | 0.001850 |
| test_uuid_3.test | RUN | 0.169617 | 0.352760 | 0.511914 | 0.001421 | 0.001873 | 0.001762 |
| test_alignment.test | RUN | 0.155022 | 0.310574 | 0.489545 | 0.001703 | 0.001920 | 0.001747 |
| test_alignment_2.test | RUN | 0.154860 | 0.301512 | 0.430301 | 0.001736 | 0.001714 | 0.001673 |
| test_attribute_packed.test | RUN | 0.155844 | 0.304886 | 0.451825 | 0.001584 | 0.001509 | 0.001797 |
| test_pragma_pack.test | RUN | 0.158581 | 0.312630 | 0.455600 | 0.001421 | 0.001964 | 0.001765 |
| test_data.test | RUN | 0.170052 | 0.407777 | 0.557079 | 0.001738 | 0.001876 | 0.001828 |
| test_comparison.test | RUN | 0.243266 | 0.582347 | 0.796103 | 0.001766 | 0.002060 | 0.001563 |
| test_comparison_no_simd.test | RUN | 0.225053 | 0.546774 | 0.709264 | 0.001920 | 0.001872 | 0.001958 |
| test_io.test | RUN | 0.262587 | 0.835351 | 0.956723 | 0.001992 | 0.002067 | 0.001942 |
| test_io_no_simd.test | RUN | 0.232834 | 0.734388 | 0.857175 | 0.001578 | 0.001735 | 0.001913 |
| test_io_2.test | RUN | 0.203287 | 0.629699 | 0.739668 | 0.001730 | 0.001983 | 0.001705 |
| test_io_2_no_simd.test | RUN | 0.182539 | 0.536310 | 0.674674 | 0.002036 | 0.001882 | 0.001911 |
| test_to_chars.test | RUN | 0.194830 | 0.593725 | 0.675613 | 0.002181 | 0.002386 | 0.002124 |
| test_to_chars_no_simd.test | RUN | 0.175966 | 0.533226 | 0.636864 | 0.002004 | 0.002232 | 0.001837 |
| test_to_chars_2.test | RUN | 0.196413 | 0.629490 | 0.748837 | 0.001657 | 0.001539 | 0.001548 |
| test_to_chars_2_no_simd.test | RUN | 0.177445 | 0.596577 | 0.637109 | 0.001579 | 0.001515 | 0.001856 |
| test_to_chars_3.test | RUN | 0.181910 | 0.522190 | 0.638870 | 0.001837 | 0.002036 | 0.001807 |
| test_from_chars.test | RUN | 0.181578 | 0.559418 | 0.671144 | 0.001600 | 0.001634 | 0.001876 |
| test_from_chars_no_simd.test | RUN | 0.152781 | 0.380073 | 0.522886 | 0.001786 | 0.001851 | 0.001726 |
| test_from_chars_2.test | RUN | 0.183186 | 0.539917 | 0.637049 | 0.001588 | 0.001682 | 0.001690 |
| test_from_chars_2_no_simd.test | RUN | 0.157160 | 0.374153 | 0.511806 | 0.001828 | 0.001784 | 0.001822 |
| test_uuid_from_string.test | RUN | 0.211278 | 0.720071 | 0.792987 | 0.001606 | 0.001957 | 0.001704 |
| test_uuid_from_string_2.test | RUN | 0.183025 | 0.565837 | 0.645017 | 0.002088 | 0.001962 | 0.002130 |
| test_uuid_from_string_3.test | RUN | 0.184370 | 0.582313 | 0.689033 | 0.001748 | 0.001664 | 0.001767 |
| test_uuid_clock.test | RUN | 0.141779 | 0.317302 | 0.444143 | 0.001708 | 0.001720 | 0.001331 |
| test_nil_generator.test | RUN | 0.164080 | 0.355980 | 0.536582 | 0.001710 | 0.001668 | 0.001658 |
| test_string_generator.test | RUN | 0.173991 | 0.491413 | 0.632204 | 0.001906 | 0.001988 | 0.001982 |
| test_string_generator_2.test | RUN | 0.196596 | 0.594664 | 0.715637 | 0.001629 | 0.001879 | 0.001786 |
| test_random_generator.test | RUN | 0.302173 | 0.776667 | 0.936833 | 0.002813 | 0.002884 | 0.002764 |
| test_random_generator_2.test | RUN | 0.244886 | 0.548457 | 0.755407 | 0.001602 | 0.001716 | 0.001663 |
| test_name_generator.test | RUN | 0.188556 | 0.554954 | 0.684625 | 0.001464 | 0.001845 | 0.001423 |
| test_namespaces.test | RUN | 0.177315 | 0.406368 | 0.587912 | 0.001529 | 0.001692 | 0.001572 |
| test_name_generator_md5.test | RUN | 0.184620 | 0.609288 | 0.686515 | 0.001664 | 0.001770 | 0.001745 |
| test_name_generator_sha1.test | RUN | 0.183446 | 0.562606 | 0.660670 | 0.001707 | 0.002004 | 0.001908 |
| test_time_generator.test | RUN | 0.236921 | 0.565514 | 0.798090 | 0.001498 | 0.001908 | 0.001789 |
| test_time_generator_v1.test | RUN | 0.258033 | 0.661928 | 0.868470 | 0.005193 | 0.005216 | 0.005107 |
| test_time_generator_v1_2.test | RUN | 0.258594 | 0.603832 | 0.824845 | 0.003413 | 0.003722 | 0.003742 |
| test_time_generator_v1_3.test | RUN | 0.280108 | 0.710609 | 0.941553 | 0.013657 | 0.009634 | 0.011836 |
| test_time_generator_v6.test | RUN | 0.261990 | 0.648240 | 0.878607 | 0.005373 | 0.005410 | 0.005549 |
| test_time_generator_v6_2.test | RUN | 0.264361 | 0.608198 | 0.844781 | 0.003461 | 0.003735 | 0.003645 |
| test_time_generator_v6_3.test | RUN | 0.284164 | 0.734358 | 0.944428 | 0.010910 | 0.013862 | 0.012470 |
| test_time_generator_v7.test | RUN | 0.247478 | 0.602459 | 0.846786 | 0.003486 | 0.003492 | 0.003852 |
| test_time_generator_v7_2.test | RUN | 0.241507 | 0.567934 | 0.808347 | 0.002030 | 0.002046 | 0.002020 |
| test_time_generator_v7_3.test | RUN | 0.245136 | 0.537985 | 0.739274 | 0.002430 | 0.002588 | 0.002384 |
| test_serialization.test | RUN | 0.441829 | 1.078734 | 1.298533 | 0.001841 | 0.001944 | 0.002128 |
| test_tagging.test | RUN | 0.244253 | 0.548736 | 0.754633 | 0.001796 | 0.001812 | 0.001775 |
| test_uuid_class.test | RUN | 0.250621 | 0.592755 | 0.857219 | 0.001740 | 0.001731 | 0.001581 |
| test_uuid_in_map.test | RUN | 0.274813 | 0.687218 | 0.933160 | 0.001538 | 0.002022 | 0.001798 |
| test_hash.test | RUN | 0.282845 | 0.592327 | 0.865761 | 0.001720 | 0.002076 | 0.002006 |
| test_hash_value.test | RUN | 0.200292 | 0.467371 | 0.622743 | 0.011955 | 0.013627 | 0.012940 |
| test_std_unordered.test | RUN | 0.255899 | 0.643078 | 0.868356 | 0.001814 | 0.002289 | 0.002100 |
| test_boost_unordered.test | RUN | 0.352187 | 0.825756 | 1.071571 | 0.002131 | 0.002066 | 0.002140 |
| test_detail_md5.test | RUN | 0.168366 | 0.431901 | 0.538468 | 0.001804 | 0.001540 | 0.001775 |
| test_detail_sha1.test | RUN | 0.158472 | 0.390286 | 0.506279 | 0.023162 | 0.023674 | 0.025593 |
| test_detail_endian.test | RUN | 0.127593 | 0.303477 | 0.410521 | 0.001569 | 0.001919 | 0.001572 |
| test_detail_chacha20.test | RUN | 0.140865 | 0.291040 | 0.406214 | 0.001630 | 0.001584 | 0.001701 |
| random_provider_no_copy_ctor.test | COMPILE_FAIL | 0.130011 | 0.251949 | 0.381998 | — | — | — |
| random_provider_no_copy_assign.test | COMPILE_FAIL | 0.130231 | 0.251514 | 0.390954 | — | — | — |
| test_msvc_simd_bug981648.test | RUN | 0.224852 | 0.515334 | 0.685362 | 0.001857 | 0.001820 | 0.001644 |
| test_bench_random.test | RUN | 0.240332 | 0.524721 | 0.749341 | 0.005905 | 0.006168 | 0.005988 |
| test_entropy_error.test | RUN | 0.118471 | 0.266331 | 0.369846 | 0.001721 | 0.001846 | 0.001681 |
| test_detail_random_provider.test | RUN | 0.235122 | 0.490227 | 0.751048 | 0.001828 | 0.001811 | 0.001840 |
| test_constants.test | RUN | 0.166703 | 0.400372 | 0.579972 | 0.001697 | 0.001475 | 0.001604 |
| test_uuid_cx.test | COMPILE | 0.110704 | 0.214388 | 0.310693 | — | — | — |
| test_uuid_cx2.test | RUN | 0.170025 | 0.416022 | 0.561238 | 0.001640 | 0.001751 | 0.001768 |
| test_uuid_cx3.test | RUN | 0.168637 | 0.376980 | 0.529568 | 0.001741 | 0.001541 | 0.001492 |
| test_uuid_cx3_no_simd.test | RUN | 0.152331 | 0.347793 | 0.453167 | 0.001688 | 0.001668 | 0.001628 |
| test_hash_value_cx.test | RUN | 0.167494 | 0.345898 | 0.537614 | 0.001530 | 0.001788 | 0.001748 |
| test_string_generator_cx.test | RUN | 0.169155 | 0.396721 | 0.540495 | 0.001652 | 0.001627 | 0.001742 |
| test_string_generator_cx2.test | RUN | 0.172422 | 0.435983 | 0.606626 | 0.001767 | 0.001569 | 0.001740 |
| test_constants_cx.test | RUN | 0.166067 | 0.327719 | 0.528824 | 0.001507 | 0.001553 | 0.001370 |
| test_from_chars_cx.test | RUN | 0.184575 | 0.525690 | 0.617920 | 0.001784 | 0.001888 | 0.001782 |
| test_from_chars_cx2.test | RUN | 0.185959 | 0.536951 | 0.628935 | 0.001665 | 0.001743 | 0.001886 |
| test_to_chars_cx.test | RUN | 0.190091 | 0.575600 | 0.697299 | 0.001850 | 0.001225 | 0.001212 |
| test_uuid_from_string_cx.test | RUN | 0.187353 | 0.561594 | 0.673296 | 0.001499 | 0.001444 | 0.001680 |
| quick.test | RUN | 0.211030 | 0.498003 | 0.580165 | 0.001232 | 0.001292 | 0.001219 |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497688) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497688) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497688).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
