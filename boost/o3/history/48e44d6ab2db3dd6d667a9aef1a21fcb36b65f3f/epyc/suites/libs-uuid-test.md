# libs/uuid/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_include1.test | RUN | 0.679309 | 1.336800 | 2.322146 | 0.000795 | 0.000965 | 0.002120 |
| test_uuid.test | RUN | 0.401158 | 1.059338 | 1.549053 | 0.002212 | 0.002043 | 0.002106 |
| test_uuid_no_simd.test | RUN | 0.382731 | 1.017397 | 1.408863 | 0.002056 | 0.002310 | 0.001806 |
| test_uuid_2.test | RUN | 0.274087 | 0.777475 | 1.050870 | 0.002250 | 0.001872 | 0.001950 |
| test_uuid_3.test | RUN | 0.254551 | 0.531539 | 0.811711 | 0.002129 | 0.002226 | 0.001992 |
| test_alignment.test | RUN | 0.213910 | 0.483125 | 0.751332 | 0.001999 | 0.001812 | 0.002128 |
| test_alignment_2.test | RUN | 0.226738 | 0.484128 | 0.789633 | 0.001863 | 0.001948 | 0.001950 |
| test_attribute_packed.test | RUN | 0.236019 | 0.492706 | 0.787731 | 0.002072 | 0.002197 | 0.001634 |
| test_pragma_pack.test | RUN | 0.220453 | 0.484586 | 0.745806 | 0.002088 | 0.002272 | 0.002013 |
| test_data.test | RUN | 0.274681 | 0.880007 | 1.069375 | 0.002012 | 0.002153 | 0.002173 |
| test_comparison.test | RUN | 0.376826 | 1.031357 | 1.482677 | 0.002016 | 0.002102 | 0.002096 |
| test_comparison_no_simd.test | RUN | 0.329077 | 1.153677 | 1.237379 | 0.002022 | 0.002032 | 0.001966 |
| test_io.test | RUN | 0.429114 | 1.889843 | 2.099446 | 0.002234 | 0.002228 | 0.002182 |
| test_io_no_simd.test | RUN | 0.393784 | 1.836890 | 1.917462 | 0.001993 | 0.002018 | 0.002372 |
| test_io_2.test | RUN | 0.333573 | 1.903081 | 1.445874 | 0.001941 | 0.002453 | 0.002190 |
| test_io_2_no_simd.test | RUN | 0.299653 | 1.870881 | 1.336906 | 0.002318 | 0.002372 | 0.002138 |
| test_to_chars.test | RUN | 0.340116 | 1.209332 | 1.356676 | 0.002048 | 0.002298 | 0.002116 |
| test_to_chars_no_simd.test | RUN | 0.319885 | 1.254038 | 1.307644 | 0.002048 | 0.002159 | 0.002134 |
| test_to_chars_2.test | RUN | 0.406460 | 2.225528 | 1.609883 | 0.001952 | 0.002057 | 0.001779 |
| test_to_chars_2_no_simd.test | RUN | 0.380932 | 2.400395 | 1.453774 | 0.001889 | 0.001905 | 0.001754 |
| test_to_chars_3.test | RUN | 0.265815 | 0.821149 | 1.021676 | 0.001909 | 0.001870 | 0.002014 |
| test_from_chars.test | RUN | 0.303454 | 1.154841 | 1.166030 | 0.001924 | 0.001832 | 0.002046 |
| test_from_chars_no_simd.test | RUN | 0.259309 | 0.934061 | 0.944906 | 0.001851 | 0.001845 | 0.001894 |
| test_from_chars_2.test | RUN | 0.293413 | 1.114584 | 1.096151 | 0.001909 | 0.002019 | 0.001922 |
| test_from_chars_2_no_simd.test | RUN | 0.255670 | 0.840120 | 0.907597 | 0.001797 | 0.002097 | 0.001990 |
| test_uuid_from_string.test | RUN | 0.323734 | 1.206272 | 1.342020 | 0.001924 | 0.001971 | 0.001432 |
| test_uuid_from_string_2.test | RUN | 0.285057 | 1.100098 | 1.167850 | 0.002426 | 0.002721 | 0.002395 |
| test_uuid_from_string_3.test | RUN | 0.285896 | 1.092902 | 1.155809 | 0.002186 | 0.001938 | 0.002394 |
| test_uuid_clock.test | RUN | 0.196032 | 0.466034 | 0.747490 | 0.001792 | 0.002327 | 0.001694 |
| test_nil_generator.test | RUN | 0.225744 | 0.553441 | 0.861154 | 0.002516 | 0.001956 | 0.002154 |
| test_string_generator.test | RUN | 0.262477 | 0.871727 | 1.146587 | 0.002037 | 0.002366 | 0.002487 |
| test_string_generator_2.test | RUN | 0.292963 | 1.249181 | 1.277847 | 0.001851 | 0.002060 | 0.002146 |
| test_random_generator.test | RUN | 0.488731 | 2.378127 | 1.957057 | 0.003250 | 0.003450 | 0.003596 |
| test_random_generator_2.test | RUN | 0.334386 | 0.940446 | 1.333939 | 0.001898 | 0.002008 | 0.002200 |
| test_name_generator.test | RUN | 0.304651 | 3.068306 | 1.402698 | 0.001795 | 0.002129 | 0.002116 |
| test_namespaces.test | RUN | 0.241097 | 0.688273 | 0.968140 | 0.001730 | 0.002157 | 0.002157 |
| test_name_generator_md5.test | RUN | 0.312351 | 1.476790 | 1.312921 | 0.002120 | 0.002090 | 0.002184 |
| test_name_generator_sha1.test | RUN | 0.323187 | 4.597189 | 1.293162 | 0.002253 | 0.001994 | 0.002185 |
| test_time_generator.test | RUN | 0.340642 | 0.931494 | 1.306616 | 0.001727 | 0.002250 | 0.002258 |
| test_time_generator_v1.test | RUN | 0.388557 | 0.985433 | 1.459814 | 0.002384 | 0.003012 | 0.002887 |
| test_time_generator_v1_2.test | RUN | 0.380362 | 0.942075 | 1.362102 | 0.002242 | 0.002532 | 0.002542 |
| test_time_generator_v1_3.test | RUN | 0.417140 | 1.013344 | 1.581969 | 0.004933 | 0.005308 | 0.004666 |
| test_time_generator_v6.test | RUN | 0.389486 | 1.013369 | 1.495882 | 0.002574 | 0.002860 | 0.002974 |
| test_time_generator_v6_2.test | RUN | 0.383295 | 0.969840 | 1.391572 | 0.002178 | 0.002700 | 0.002552 |
| test_time_generator_v6_3.test | RUN | 0.405169 | 1.024092 | 1.592408 | 0.004654 | 0.005178 | 0.006148 |
| test_time_generator_v7.test | RUN | 0.361144 | 1.021686 | 1.447923 | 0.002028 | 0.002519 | 0.002322 |
| test_time_generator_v7_2.test | RUN | 0.335695 | 0.911826 | 1.355789 | 0.001756 | 0.002404 | 0.001993 |
| test_time_generator_v7_3.test | RUN | 0.350109 | 1.164466 | 1.348547 | 0.002943 | 0.003169 | 0.003234 |
| test_serialization.test | RUN | 0.620749 | 1.745138 | 2.335234 | 0.002412 | 0.002203 | 0.002360 |
| test_tagging.test | RUN | 0.342815 | 1.084758 | 1.345517 | 0.001986 | 0.001791 | 0.001896 |
| test_uuid_class.test | RUN | 0.354475 | 0.974799 | 1.378575 | 0.001892 | 0.001898 | 0.001824 |
| test_uuid_in_map.test | RUN | 0.388888 | 1.102648 | 1.618499 | 0.001825 | 0.002280 | 0.002076 |
| test_hash.test | RUN | 0.396067 | 1.015512 | 1.429115 | 0.002128 | 0.002051 | 0.002119 |
| test_hash_value.test | RUN | 0.282521 | 0.683900 | 1.076237 | 0.006192 | 0.006253 | 0.006064 |
| test_std_unordered.test | RUN | 0.368224 | 1.119649 | 1.434576 | 0.001951 | 0.002109 | 0.002050 |
| test_boost_unordered.test | RUN | 0.497398 | 1.452044 | 1.785022 | 0.001986 | 0.002049 | 0.002186 |
| test_detail_md5.test | RUN | 0.251129 | 0.730727 | 1.046193 | 0.001892 | 0.001851 | 0.002184 |
| test_detail_sha1.test | RUN | 0.234605 | 0.894644 | 0.925940 | 0.012767 | 0.011430 | 0.011094 |
| test_detail_endian.test | RUN | 0.181650 | 0.535408 | 0.677430 | 0.001942 | 0.001924 | 0.002009 |
| test_detail_chacha20.test | RUN | 0.205007 | 0.639618 | 0.685238 | 0.002282 | 0.002155 | 0.001968 |
| random_provider_no_copy_ctor.test | COMPILE_FAIL | 0.176668 | 0.387707 | 0.586108 | — | — | — |
| random_provider_no_copy_assign.test | COMPILE_FAIL | 0.177167 | 0.365401 | 0.593413 | — | — | — |
| test_msvc_simd_bug981648.test | RUN | 0.310725 | 0.730799 | 1.076127 | 0.001937 | 0.002250 | 0.002070 |
| test_bench_random.test | RUN | 0.344802 | 0.933002 | 1.339515 | 0.008907 | 0.008991 | 0.008631 |
| test_entropy_error.test | RUN | 0.170623 | 0.440142 | 0.632763 | 0.002035 | 0.002214 | 0.001932 |
| test_detail_random_provider.test | RUN | 0.330124 | 0.731336 | 1.232187 | 0.002042 | 0.002053 | 0.001916 |
| test_constants.test | RUN | 0.245902 | 0.655723 | 0.980353 | 0.002223 | 0.002268 | 0.001630 |
| test_uuid_cx.test | COMPILE | 0.149851 | 0.323824 | 0.512395 | — | — | — |
| test_uuid_cx2.test | RUN | 0.254915 | 0.780382 | 1.044125 | 0.001763 | 0.001924 | 0.002042 |
| test_uuid_cx3.test | RUN | 0.234477 | 0.587931 | 0.900807 | 0.001682 | 0.002047 | 0.002054 |
| test_uuid_cx3_no_simd.test | RUN | 0.219644 | 0.577858 | 0.852390 | 0.001924 | 0.001908 | 0.002028 |
| test_hash_value_cx.test | RUN | 0.232096 | 0.551921 | 0.879317 | 0.001730 | 0.001664 | 0.001958 |
| test_string_generator_cx.test | RUN | 0.238543 | 0.673859 | 0.905684 | 0.001948 | 0.002085 | 0.001528 |
| test_string_generator_cx2.test | RUN | 0.249739 | 0.830183 | 1.113188 | 0.001524 | 0.002018 | 0.002244 |
| test_constants_cx.test | RUN | 0.229086 | 0.465773 | 0.767105 | 0.001928 | 0.001760 | 0.001998 |
| test_from_chars_cx.test | RUN | 0.270299 | 0.887831 | 1.062551 | 0.002020 | 0.002181 | 0.002148 |
| test_from_chars_cx2.test | RUN | 0.276923 | 1.105753 | 1.067085 | 0.001913 | 0.001484 | 0.001981 |
| test_to_chars_cx.test | RUN | 0.275006 | 0.993326 | 1.203177 | 0.001675 | 0.001861 | 0.001832 |
| test_uuid_from_string_cx.test | RUN | 0.283045 | 1.023136 | 1.150140 | 0.001628 | 0.001510 | 0.002665 |
| quick.test | RUN | 0.284090 | 0.795783 | 0.990548 | 0.001513 | 0.001519 | 0.001602 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.992× | 0.986×, 0.998× | 0.996× |
| clang | 1.003× | 1.001×, 1.005× | 1.003× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| quick.test | 10 | 0.001522s | 0.001546s | 0.001498s | byte equality observed |
| test_alignment.test | 10 | 0.001510s | 0.001519s | 0.001531s | byte equality observed |
| test_alignment_2.test | 10 | 0.001491s | 0.001536s | 0.001513s | byte equality observed |
| test_attribute_packed.test | 10 | 0.001482s | 0.001452s | 0.001467s | byte equality observed |
| test_bench_random.test | 3 | 0.008417s | 0.008361s | 0.008430s | native test oracle; output varies |
| test_boost_unordered.test | 10 | 0.001538s | 0.001564s | 0.001508s | byte equality observed |
| test_comparison.test | 10 | 0.001519s | 0.001546s | 0.001588s | byte equality observed |
| test_comparison_no_simd.test | 10 | 0.001544s | 0.001584s | 0.001553s | byte equality observed |
| test_constants.test | 10 | 0.001531s | 0.001541s | 0.001543s | byte equality observed |
| test_constants_cx.test | 10 | 0.001515s | 0.001485s | 0.001517s | byte equality observed |
| test_data.test | 10 | 0.001519s | 0.001555s | 0.001515s | byte equality observed |
| test_detail_chacha20.test | 10 | 0.001532s | 0.001522s | 0.001537s | byte equality observed |
| test_detail_endian.test | 10 | 0.001525s | 0.001517s | 0.001536s | byte equality observed |
| test_detail_md5.test | 10 | 0.001555s | 0.001550s | 0.001530s | byte equality observed |
| test_detail_random_provider.test | 10 | 0.001546s | 0.001538s | 0.001537s | byte equality observed |
| test_detail_sha1.test | 3 | 0.010787s | 0.010427s | 0.010584s | byte equality observed |
| test_entropy_error.test | 10 | 0.001504s | 0.001508s | 0.001486s | byte equality observed |
| test_from_chars.test | 10 | 0.001503s | 0.001476s | 0.001482s | byte equality observed |
| test_from_chars_2.test | 10 | 0.001508s | 0.001472s | 0.001480s | byte equality observed |
| test_from_chars_2_no_simd.test | 10 | 0.001547s | 0.001558s | 0.001537s | byte equality observed |
| test_from_chars_cx.test | 10 | 0.001536s | 0.001548s | 0.001512s | byte equality observed |
| test_from_chars_cx2.test | 10 | 0.001526s | 0.001550s | 0.001534s | byte equality observed |
| test_from_chars_no_simd.test | 10 | 0.001533s | 0.001520s | 0.001571s | byte equality observed |
| test_hash.test | 10 | 0.001541s | 0.001551s | 0.001543s | byte equality observed |
| test_hash_value.test | 5 | 0.005670s | 0.005380s | 0.005302s | byte equality observed |
| test_hash_value_cx.test | 10 | 0.001476s | 0.001508s | 0.001526s | byte equality observed |
| test_include1.test | 10 | 0.000667s | 0.000681s | 0.001456s | byte equality observed |
| test_io.test | 10 | 0.001603s | 0.001644s | 0.001664s | byte equality observed |
| test_io_2.test | 7 | 0.001713s | 0.001689s | 0.001734s | byte equality observed |
| test_io_2_no_simd.test | 10 | 0.001731s | 0.001769s | 0.001731s | byte equality observed |
| test_io_no_simd.test | 10 | 0.001641s | 0.001672s | 0.001601s | byte equality observed |
| test_msvc_simd_bug981648.test | 8 | 0.001534s | 0.001497s | 0.001558s | native test oracle; output varies |
| test_name_generator.test | 10 | 0.001589s | 0.001619s | 0.001573s | byte equality observed |
| test_name_generator_md5.test | 10 | 0.001584s | 0.001532s | 0.001554s | byte equality observed |
| test_name_generator_sha1.test | 10 | 0.001593s | 0.001594s | 0.001569s | byte equality observed |
| test_namespaces.test | 10 | 0.001535s | 0.001554s | 0.001511s | byte equality observed |
| test_nil_generator.test | 10 | 0.001486s | 0.001510s | 0.001527s | byte equality observed |
| test_pragma_pack.test | 10 | 0.001506s | 0.001512s | 0.001483s | byte equality observed |
| test_random_generator.test | 7 | 0.002967s | 0.002903s | 0.002954s | native test oracle; output varies |
| test_random_generator_2.test | 10 | 0.001538s | 0.001492s | 0.001546s | byte equality observed |
| test_serialization.test | 10 | 0.001669s | 0.001698s | 0.001637s | byte equality observed |
| test_std_unordered.test | 10 | 0.001637s | 0.001613s | 0.001633s | byte equality observed |
| test_string_generator.test | 10 | 0.001710s | 0.001705s | 0.001699s | byte equality observed |
| test_string_generator_2.test | 10 | 0.001567s | 0.001561s | 0.001542s | byte equality observed |
| test_string_generator_cx.test | 10 | 0.001545s | 0.001519s | 0.001531s | byte equality observed |
| test_string_generator_cx2.test | 10 | 0.001578s | 0.001558s | 0.001535s | byte equality observed |
| test_tagging.test | 10 | 0.001543s | 0.001572s | 0.001512s | byte equality observed |
| test_time_generator.test | 10 | 0.001563s | 0.001557s | 0.001563s | byte equality observed |
| test_time_generator_v1.test | 9 | 0.002080s | 0.002234s | 0.002255s | byte equality observed |
| test_time_generator_v1_2.test | 10 | 0.001857s | 0.001875s | 0.001897s | byte equality observed |
| test_time_generator_v1_3.test | 5 | 0.004741s | 0.004505s | 0.004818s | byte equality observed |
| test_time_generator_v6.test | 9 | 0.002107s | 0.002196s | 0.002231s | byte equality observed |
| test_time_generator_v6_2.test | 9 | 0.001823s | 0.001870s | 0.001945s | byte equality observed |
| test_time_generator_v6_3.test | 4 | 0.004843s | 0.004773s | 0.004747s | byte equality observed |
| test_time_generator_v7.test | 10 | 0.001898s | 0.001889s | 0.001939s | byte equality observed |
| test_time_generator_v7_2.test | 10 | 0.001619s | 0.001577s | 0.001628s | byte equality observed |
| test_time_generator_v7_3.test | 8 | 0.002612s | 0.002535s | 0.002541s | byte equality observed |
| test_to_chars.test | 10 | 0.001646s | 0.001598s | 0.001597s | byte equality observed |
| test_to_chars_2.test | 10 | 0.001569s | 0.001598s | 0.001564s | byte equality observed |
| test_to_chars_2_no_simd.test | 10 | 0.001594s | 0.001567s | 0.001573s | byte equality observed |
| test_to_chars_3.test | 10 | 0.001521s | 0.001546s | 0.001570s | byte equality observed |
| test_to_chars_cx.test | 10 | 0.001560s | 0.001549s | 0.001527s | byte equality observed |
| test_to_chars_no_simd.test | 10 | 0.001626s | 0.001620s | 0.001639s | byte equality observed |
| test_uuid.test | 10 | 0.001526s | 0.001511s | 0.001547s | byte equality observed |
| test_uuid_2.test | 10 | 0.001559s | 0.001576s | 0.001548s | byte equality observed |
| test_uuid_3.test | 10 | 0.001529s | 0.001529s | 0.001542s | byte equality observed |
| test_uuid_class.test | 10 | 0.001526s | 0.001532s | 0.001571s | byte equality observed |
| test_uuid_clock.test | 10 | 0.001520s | 0.001510s | 0.001549s | byte equality observed |
| test_uuid_cx2.test | 10 | 0.001533s | 0.001533s | 0.001571s | byte equality observed |
| test_uuid_cx3.test | 10 | 0.001542s | 0.001557s | 0.001574s | byte equality observed |
| test_uuid_cx3_no_simd.test | 10 | 0.001549s | 0.001524s | 0.001521s | byte equality observed |
| test_uuid_from_string.test | 10 | 0.001551s | 0.001546s | 0.001538s | byte equality observed |
| test_uuid_from_string_2.test | 10 | 0.001987s | 0.001971s | 0.002015s | byte equality observed |
| test_uuid_from_string_3.test | 10 | 0.001695s | 0.001659s | 0.001640s | byte equality observed |
| test_uuid_from_string_cx.test | 10 | 0.001551s | 0.001554s | 0.001526s | byte equality observed |
| test_uuid_in_map.test | 10 | 0.001580s | 0.001578s | 0.001531s | byte equality observed |
| test_uuid_no_simd.test | 10 | 0.001529s | 0.001539s | 0.001497s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
