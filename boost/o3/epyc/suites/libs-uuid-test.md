# libs/uuid/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_include1.test | RUN | 0.548813 | 1.290017 | 2.293627 | 0.000925 | 0.000980 | 0.001918 |
| test_uuid.test | RUN | 0.357564 | 1.060125 | 1.574854 | 0.002220 | 0.002056 | 0.002047 |
| test_uuid_no_simd.test | RUN | 0.341440 | 1.004147 | 1.398201 | 0.001936 | 0.001768 | 0.002008 |
| test_uuid_2.test | RUN | 0.241660 | 0.756442 | 1.031552 | 0.001769 | 0.001752 | 0.001948 |
| test_uuid_3.test | RUN | 0.199511 | 0.558008 | 0.781782 | 0.001984 | 0.001970 | 0.001582 |
| test_alignment.test | RUN | 0.184269 | 0.487519 | 0.720235 | 0.002176 | 0.001997 | 0.002036 |
| test_alignment_2.test | RUN | 0.195023 | 0.473527 | 0.758578 | 0.001975 | 0.001896 | 0.001713 |
| test_attribute_packed.test | RUN | 0.188041 | 0.477735 | 0.773941 | 0.002008 | 0.002092 | 0.001706 |
| test_pragma_pack.test | RUN | 0.187123 | 0.484205 | 0.752965 | 0.001979 | 0.001885 | 0.001880 |
| test_data.test | RUN | 0.237288 | 0.873121 | 1.061669 | 0.001944 | 0.002068 | 0.001946 |
| test_comparison.test | RUN | 0.313667 | 1.010928 | 1.443659 | 0.001972 | 0.002154 | 0.002014 |
| test_comparison_no_simd.test | RUN | 0.292280 | 1.169300 | 1.326598 | 0.002093 | 0.001816 | 0.001929 |
| test_io.test | RUN | 0.419392 | 1.854330 | 2.027158 | 0.002117 | 0.002161 | 0.002174 |
| test_io_no_simd.test | RUN | 0.382723 | 1.828651 | 1.946549 | 0.001896 | 0.002154 | 0.002235 |
| test_io_2.test | RUN | 0.314066 | 1.955439 | 1.435095 | 0.002009 | 0.002329 | 0.001966 |
| test_io_2_no_simd.test | RUN | 0.282920 | 1.786249 | 1.354552 | 0.002110 | 0.002118 | 0.002210 |
| test_to_chars.test | RUN | 0.287429 | 1.203532 | 1.349248 | 0.002094 | 0.002163 | 0.002206 |
| test_to_chars_no_simd.test | RUN | 0.267918 | 1.236061 | 1.272782 | 0.001873 | 0.001892 | 0.002020 |
| test_to_chars_2.test | RUN | 0.327291 | 2.191029 | 1.521012 | 0.001862 | 0.002013 | 0.002096 |
| test_to_chars_2_no_simd.test | RUN | 0.325133 | 2.359608 | 1.444453 | 0.001918 | 0.001767 | 0.001958 |
| test_to_chars_3.test | RUN | 0.234538 | 0.796299 | 1.031386 | 0.001724 | 0.001746 | 0.001933 |
| test_from_chars.test | RUN | 0.266896 | 1.140951 | 1.145141 | 0.001807 | 0.001944 | 0.001774 |
| test_from_chars_no_simd.test | RUN | 0.226198 | 0.918497 | 0.941178 | 0.001778 | 0.001711 | 0.001676 |
| test_from_chars_2.test | RUN | 0.258185 | 1.083817 | 1.124507 | 0.001757 | 0.001874 | 0.001740 |
| test_from_chars_2_no_simd.test | RUN | 0.217935 | 0.840106 | 0.906849 | 0.001602 | 0.001962 | 0.001918 |
| test_uuid_from_string.test | RUN | 0.305218 | 1.208810 | 1.351722 | 0.001834 | 0.002014 | 0.002119 |
| test_uuid_from_string_2.test | RUN | 0.270780 | 1.085074 | 1.157126 | 0.002158 | 0.002558 | 0.002428 |
| test_uuid_from_string_3.test | RUN | 0.271621 | 1.088341 | 1.177901 | 0.001964 | 0.002084 | 0.002110 |
| test_uuid_clock.test | RUN | 0.171177 | 0.451171 | 0.741076 | 0.002056 | 0.002091 | 0.001757 |
| test_nil_generator.test | RUN | 0.198339 | 0.529392 | 0.827730 | 0.001953 | 0.002114 | 0.002121 |
| test_string_generator.test | RUN | 0.235261 | 0.866066 | 1.128654 | 0.002110 | 0.002278 | 0.002126 |
| test_string_generator_2.test | RUN | 0.275700 | 1.234678 | 1.259794 | 0.001852 | 0.002104 | 0.002072 |
| test_random_generator.test | RUN | 0.420875 | 2.365742 | 1.910614 | 0.003357 | 0.003348 | 0.003498 |
| test_random_generator_2.test | RUN | 0.298300 | 0.927675 | 1.324356 | 0.001850 | 0.002100 | 0.001986 |
| test_name_generator.test | RUN | 0.261117 | 3.092869 | 1.385561 | 0.001702 | 0.002133 | 0.001956 |
| test_namespaces.test | RUN | 0.214027 | 0.650136 | 0.926719 | 0.001878 | 0.002103 | 0.001932 |
| test_name_generator_md5.test | RUN | 0.269221 | 1.440015 | 1.321383 | 0.002074 | 0.002024 | 0.002162 |
| test_name_generator_sha1.test | RUN | 0.259025 | 4.572250 | 1.277794 | 0.002112 | 0.001987 | 0.002188 |
| test_time_generator.test | RUN | 0.301982 | 0.916988 | 1.295117 | 0.001938 | 0.002169 | 0.002050 |
| test_time_generator_v1.test | RUN | 0.332438 | 0.965770 | 1.452857 | 0.002311 | 0.002626 | 0.002906 |
| test_time_generator_v1_2.test | RUN | 0.331793 | 0.955312 | 1.367227 | 0.002302 | 0.002519 | 0.002376 |
| test_time_generator_v1_3.test | RUN | 0.363776 | 0.972194 | 1.475782 | 0.005125 | 0.004613 | 0.006492 |
| test_time_generator_v6.test | RUN | 0.329723 | 1.001423 | 1.505237 | 0.002317 | 0.002786 | 0.002610 |
| test_time_generator_v6_2.test | RUN | 0.329074 | 0.943451 | 1.390417 | 0.002074 | 0.002541 | 0.002268 |
| test_time_generator_v6_3.test | RUN | 0.358024 | 0.987777 | 1.587815 | 0.005678 | 0.005508 | 0.005770 |
| test_time_generator_v7.test | RUN | 0.318264 | 1.016322 | 1.425146 | 0.002015 | 0.002339 | 0.002506 |
| test_time_generator_v7_2.test | RUN | 0.298660 | 0.903742 | 1.300253 | 0.001890 | 0.002120 | 0.001861 |
| test_time_generator_v7_3.test | RUN | 0.298392 | 1.139793 | 1.322576 | 0.002809 | 0.002991 | 0.002877 |
| test_serialization.test | RUN | 0.553502 | 1.674882 | 2.298468 | 0.002162 | 0.001972 | 0.002208 |
| test_tagging.test | RUN | 0.311949 | 1.079681 | 1.302926 | 0.001785 | 0.001899 | 0.002110 |
| test_uuid_class.test | RUN | 0.311856 | 0.960183 | 1.417783 | 0.001743 | 0.001928 | 0.002073 |
| test_uuid_in_map.test | RUN | 0.353137 | 1.104952 | 1.565818 | 0.001891 | 0.002124 | 0.001932 |
| test_hash.test | RUN | 0.351110 | 0.997628 | 1.439689 | 0.002001 | 0.002133 | 0.002154 |
| test_hash_value.test | RUN | 0.246930 | 0.688550 | 1.053946 | 0.006076 | 0.005979 | 0.006154 |
| test_std_unordered.test | RUN | 0.329126 | 1.112338 | 1.407699 | 0.001630 | 0.002169 | 0.001705 |
| test_boost_unordered.test | RUN | 0.469861 | 1.427792 | 1.796118 | 0.001672 | 0.001838 | 0.001680 |
| test_detail_md5.test | RUN | 0.220155 | 0.693446 | 1.018411 | 0.001874 | 0.001871 | 0.001400 |
| test_detail_sha1.test | RUN | 0.208987 | 0.893423 | 0.929517 | 0.012410 | 0.011360 | 0.011253 |
| test_detail_endian.test | RUN | 0.166297 | 0.504026 | 0.668258 | 0.001472 | 0.001837 | 0.001904 |
| test_detail_chacha20.test | RUN | 0.172744 | 0.629448 | 0.684710 | 0.001806 | 0.001854 | 0.002040 |
| random_provider_no_copy_ctor.test | COMPILE_FAIL | 0.155531 | 0.365503 | 0.573799 | — | — | — |
| random_provider_no_copy_assign.test | COMPILE_FAIL | 0.160987 | 0.354182 | 0.601616 | — | — | — |
| test_msvc_simd_bug981648.test | RUN | 0.261246 | 0.710494 | 1.053885 | 0.001971 | 0.002288 | 0.002074 |
| test_bench_random.test | RUN | 0.291764 | 0.908377 | 1.326376 | 0.008702 | 0.008811 | 0.008856 |
| test_entropy_error.test | RUN | 0.148271 | 0.423996 | 0.606981 | 0.001655 | 0.001733 | 0.002092 |
| test_detail_random_provider.test | RUN | 0.296851 | 0.714842 | 1.216603 | 0.002046 | 0.002084 | 0.002162 |
| test_constants.test | RUN | 0.216963 | 0.670503 | 0.927037 | 0.001846 | 0.001998 | 0.002087 |
| test_uuid_cx.test | COMPILE | 0.132963 | 0.319409 | 0.515176 | — | — | — |
| test_uuid_cx2.test | RUN | 0.227994 | 0.764367 | 1.013357 | 0.001814 | 0.001953 | 0.001899 |
| test_uuid_cx3.test | RUN | 0.211334 | 0.589446 | 0.904188 | 0.001847 | 0.001729 | 0.002135 |
| test_uuid_cx3_no_simd.test | RUN | 0.192158 | 0.544029 | 0.803624 | 0.002015 | 0.001760 | 0.001694 |
| test_hash_value_cx.test | RUN | 0.194768 | 0.535460 | 0.835916 | 0.001834 | 0.002113 | 0.001954 |
| test_string_generator_cx.test | RUN | 0.219252 | 0.652765 | 0.906367 | 0.001930 | 0.001648 | 0.002157 |
| test_string_generator_cx2.test | RUN | 0.244088 | 0.791896 | 1.079021 | 0.001844 | 0.001942 | 0.001925 |
| test_constants_cx.test | RUN | 0.195614 | 0.479172 | 0.801647 | 0.001706 | 0.001896 | 0.001970 |
| test_from_chars_cx.test | RUN | 0.246090 | 0.892345 | 0.988791 | 0.002017 | 0.002116 | 0.001893 |
| test_from_chars_cx2.test | RUN | 0.248977 | 1.083376 | 1.096364 | 0.001808 | 0.001877 | 0.002104 |
| test_to_chars_cx.test | RUN | 0.254865 | 0.990788 | 1.113399 | 0.001800 | 0.001914 | 0.001440 |
| test_uuid_from_string_cx.test | RUN | 0.270729 | 1.014595 | 1.155053 | 0.001446 | 0.001426 | 0.001360 |
| quick.test | RUN | 0.265902 | 0.768699 | 1.135158 | 0.001489 | 0.001560 | 0.001447 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.990× | 0.986×, 0.995× | 1.005× |
| clang | 0.990× | 0.991×, 0.989× | 0.996× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824563).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| quick.test | 10 | 0.001464s | 0.001443s | 0.001449s | byte equality observed |
| test_alignment.test | 10 | 0.001461s | 0.001490s | 0.001466s | byte equality observed |
| test_alignment_2.test | 10 | 0.001452s | 0.001438s | 0.001444s | byte equality observed |
| test_attribute_packed.test | 10 | 0.001448s | 0.001416s | 0.001426s | byte equality observed |
| test_bench_random.test | 3 | 0.008281s | 0.008302s | 0.008152s | native test oracle; output varies |
| test_boost_unordered.test | 10 | 0.001485s | 0.001541s | 0.001481s | byte equality observed |
| test_comparison.test | 10 | 0.001481s | 0.001491s | 0.001528s | byte equality observed |
| test_comparison_no_simd.test | 10 | 0.001502s | 0.001541s | 0.001535s | byte equality observed |
| test_constants.test | 10 | 0.001498s | 0.001510s | 0.001488s | byte equality observed |
| test_constants_cx.test | 10 | 0.001463s | 0.001427s | 0.001455s | byte equality observed |
| test_data.test | 10 | 0.001483s | 0.001472s | 0.001500s | byte equality observed |
| test_detail_chacha20.test | 10 | 0.001479s | 0.001485s | 0.001484s | byte equality observed |
| test_detail_endian.test | 10 | 0.001477s | 0.001476s | 0.001468s | byte equality observed |
| test_detail_md5.test | 10 | 0.001523s | 0.001508s | 0.001498s | byte equality observed |
| test_detail_random_provider.test | 10 | 0.001459s | 0.001521s | 0.001459s | byte equality observed |
| test_detail_sha1.test | 3 | 0.012190s | 0.010355s | 0.010435s | byte equality observed |
| test_entropy_error.test | 10 | 0.001441s | 0.001436s | 0.001478s | byte equality observed |
| test_from_chars.test | 10 | 0.001441s | 0.001459s | 0.001440s | byte equality observed |
| test_from_chars_2.test | 10 | 0.001461s | 0.001450s | 0.001475s | byte equality observed |
| test_from_chars_2_no_simd.test | 10 | 0.001492s | 0.001465s | 0.001531s | byte equality observed |
| test_from_chars_cx.test | 10 | 0.001480s | 0.001480s | 0.001480s | byte equality observed |
| test_from_chars_cx2.test | 10 | 0.001485s | 0.001479s | 0.001487s | byte equality observed |
| test_from_chars_no_simd.test | 10 | 0.001490s | 0.001495s | 0.001462s | byte equality observed |
| test_hash.test | 10 | 0.001488s | 0.001499s | 0.001470s | byte equality observed |
| test_hash_value.test | 4 | 0.005602s | 0.005374s | 0.005273s | byte equality observed |
| test_hash_value_cx.test | 10 | 0.001462s | 0.001485s | 0.001448s | byte equality observed |
| test_include1.test | 10 | 0.000635s | 0.000680s | 0.001433s | byte equality observed |
| test_io.test | 10 | 0.001559s | 0.001572s | 0.001570s | byte equality observed |
| test_io_2.test | 10 | 0.001686s | 0.001654s | 0.001669s | byte equality observed |
| test_io_2_no_simd.test | 10 | 0.001695s | 0.001661s | 0.001681s | byte equality observed |
| test_io_no_simd.test | 10 | 0.001572s | 0.001576s | 0.001587s | byte equality observed |
| test_msvc_simd_bug981648.test | 10 | 0.001513s | 0.001471s | 0.001519s | native test oracle; output varies |
| test_name_generator.test | 10 | 0.001522s | 0.001560s | 0.001523s | byte equality observed |
| test_name_generator_md5.test | 10 | 0.001513s | 0.001496s | 0.001540s | byte equality observed |
| test_name_generator_sha1.test | 10 | 0.001534s | 0.001583s | 0.001565s | byte equality observed |
| test_namespaces.test | 10 | 0.001497s | 0.001487s | 0.001497s | byte equality observed |
| test_nil_generator.test | 10 | 0.001459s | 0.001479s | 0.001446s | byte equality observed |
| test_pragma_pack.test | 8 | 0.001489s | 0.001485s | 0.001459s | byte equality observed |
| test_random_generator.test | 7 | 0.002933s | 0.002901s | 0.002872s | native test oracle; output varies |
| test_random_generator_2.test | 10 | 0.001501s | 0.001492s | 0.001478s | byte equality observed |
| test_serialization.test | 10 | 0.001641s | 0.001635s | 0.001571s | byte equality observed |
| test_std_unordered.test | 10 | 0.001610s | 0.001573s | 0.001554s | byte equality observed |
| test_string_generator.test | 10 | 0.001622s | 0.001650s | 0.001637s | byte equality observed |
| test_string_generator_2.test | 10 | 0.001509s | 0.001534s | 0.001487s | byte equality observed |
| test_string_generator_cx.test | 10 | 0.001495s | 0.001498s | 0.001477s | byte equality observed |
| test_string_generator_cx2.test | 10 | 0.001507s | 0.001504s | 0.001481s | byte equality observed |
| test_tagging.test | 10 | 0.001512s | 0.001480s | 0.001528s | byte equality observed |
| test_time_generator.test | 10 | 0.001527s | 0.001497s | 0.001548s | byte equality observed |
| test_time_generator_v1.test | 9 | 0.002087s | 0.002176s | 0.002229s | byte equality observed |
| test_time_generator_v1_2.test | 10 | 0.001832s | 0.001876s | 0.001837s | byte equality observed |
| test_time_generator_v1_3.test | 5 | 0.004669s | 0.004509s | 0.004761s | byte equality observed |
| test_time_generator_v6.test | 10 | 0.002034s | 0.002163s | 0.002161s | byte equality observed |
| test_time_generator_v6_2.test | 10 | 0.001869s | 0.001853s | 0.001899s | byte equality observed |
| test_time_generator_v6_3.test | 5 | 0.004700s | 0.004800s | 0.004686s | byte equality observed |
| test_time_generator_v7.test | 8 | 0.001852s | 0.001860s | 0.001864s | byte equality observed |
| test_time_generator_v7_2.test | 10 | 0.001599s | 0.001562s | 0.001518s | byte equality observed |
| test_time_generator_v7_3.test | 8 | 0.002542s | 0.002540s | 0.002513s | byte equality observed |
| test_to_chars.test | 10 | 0.001570s | 0.001618s | 0.001567s | byte equality observed |
| test_to_chars_2.test | 10 | 0.001533s | 0.001482s | 0.001506s | byte equality observed |
| test_to_chars_2_no_simd.test | 10 | 0.001508s | 0.001543s | 0.001503s | byte equality observed |
| test_to_chars_3.test | 10 | 0.001512s | 0.001545s | 0.001478s | byte equality observed |
| test_to_chars_cx.test | 10 | 0.001491s | 0.001542s | 0.001515s | byte equality observed |
| test_to_chars_no_simd.test | 10 | 0.001589s | 0.001598s | 0.001568s | byte equality observed |
| test_uuid.test | 10 | 0.001473s | 0.001447s | 0.001461s | byte equality observed |
| test_uuid_2.test | 7 | 0.001532s | 0.001487s | 0.001451s | byte equality observed |
| test_uuid_3.test | 9 | 0.001464s | 0.001514s | 0.001436s | byte equality observed |
| test_uuid_class.test | 10 | 0.001513s | 0.001523s | 0.001463s | byte equality observed |
| test_uuid_clock.test | 8 | 0.001477s | 0.001511s | 0.001474s | byte equality observed |
| test_uuid_cx2.test | 10 | 0.001506s | 0.001503s | 0.001493s | byte equality observed |
| test_uuid_cx3.test | 10 | 0.001516s | 0.001535s | 0.001528s | byte equality observed |
| test_uuid_cx3_no_simd.test | 10 | 0.001495s | 0.001444s | 0.001497s | byte equality observed |
| test_uuid_from_string.test | 9 | 0.001508s | 0.001534s | 0.001515s | byte equality observed |
| test_uuid_from_string_2.test | 10 | 0.001898s | 0.001898s | 0.001933s | byte equality observed |
| test_uuid_from_string_3.test | 10 | 0.001581s | 0.001606s | 0.001614s | byte equality observed |
| test_uuid_from_string_cx.test | 10 | 0.001510s | 0.001496s | 0.001472s | byte equality observed |
| test_uuid_in_map.test | 10 | 0.001506s | 0.001519s | 0.001468s | byte equality observed |
| test_uuid_no_simd.test | 10 | 0.001490s | 0.001504s | 0.001480s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824563).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
