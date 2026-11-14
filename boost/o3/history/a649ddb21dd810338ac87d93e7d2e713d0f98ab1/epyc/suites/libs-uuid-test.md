# libs/uuid/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_include1.test | RUN | 0.586642 | 1.313925 | 2.363080 | 0.000742 | 0.000873 | 0.001980 |
| test_uuid.test | RUN | 0.346802 | 1.060667 | 1.565204 | 0.002014 | 0.002305 | 0.002106 |
| test_uuid_no_simd.test | RUN | 0.342092 | 1.053873 | 1.444865 | 0.002208 | 0.001884 | 0.001819 |
| test_uuid_2.test | RUN | 0.242466 | 0.735396 | 1.007221 | 0.001746 | 0.002226 | 0.001814 |
| test_uuid_3.test | RUN | 0.205261 | 0.530721 | 0.874675 | 0.001793 | 0.002148 | 0.001888 |
| test_alignment.test | RUN | 0.203016 | 0.512375 | 0.772584 | 0.002060 | 0.001886 | 0.002129 |
| test_alignment_2.test | RUN | 0.197882 | 0.463761 | 0.790154 | 0.001936 | 0.001738 | 0.001595 |
| test_attribute_packed.test | RUN | 0.187064 | 0.489829 | 0.786585 | 0.001942 | 0.002244 | 0.002084 |
| test_pragma_pack.test | RUN | 0.192135 | 0.480204 | 0.782753 | 0.002182 | 0.002140 | 0.001894 |
| test_data.test | RUN | 0.238412 | 0.854204 | 1.081319 | 0.001910 | 0.002114 | 0.001996 |
| test_comparison.test | RUN | 0.310443 | 1.033047 | 1.477394 | 0.002102 | 0.002130 | 0.002116 |
| test_comparison_no_simd.test | RUN | 0.312135 | 1.174205 | 1.255284 | 0.001704 | 0.001900 | 0.002156 |
| test_io.test | RUN | 0.410706 | 1.854951 | 2.120053 | 0.002232 | 0.002149 | 0.002310 |
| test_io_no_simd.test | RUN | 0.366242 | 1.817785 | 1.949236 | 0.001880 | 0.001876 | 0.002328 |
| test_io_2.test | RUN | 0.313827 | 1.912048 | 1.481026 | 0.001788 | 0.002250 | 0.002166 |
| test_io_2_no_simd.test | RUN | 0.283846 | 1.816819 | 1.291077 | 0.002000 | 0.002194 | 0.002392 |
| test_to_chars.test | RUN | 0.289878 | 1.207830 | 1.394713 | 0.002132 | 0.002287 | 0.002212 |
| test_to_chars_no_simd.test | RUN | 0.258587 | 1.244220 | 1.305875 | 0.002078 | 0.001890 | 0.002225 |
| test_to_chars_2.test | RUN | 0.330031 | 2.166266 | 1.608101 | 0.001746 | 0.001974 | 0.002091 |
| test_to_chars_2_no_simd.test | RUN | 0.306045 | 2.333660 | 1.463120 | 0.001958 | 0.001902 | 0.002009 |
| test_to_chars_3.test | RUN | 0.226437 | 0.796313 | 0.994711 | 0.001952 | 0.001767 | 0.002166 |
| test_from_chars.test | RUN | 0.265673 | 1.143226 | 1.170784 | 0.002070 | 0.001826 | 0.002211 |
| test_from_chars_no_simd.test | RUN | 0.245525 | 0.916613 | 0.944937 | 0.001853 | 0.002043 | 0.001910 |
| test_from_chars_2.test | RUN | 0.260351 | 1.104445 | 1.129362 | 0.001593 | 0.002156 | 0.002064 |
| test_from_chars_2_no_simd.test | RUN | 0.232922 | 0.837023 | 0.921776 | 0.001774 | 0.001956 | 0.002181 |
| test_uuid_from_string.test | RUN | 0.309487 | 1.199995 | 1.350130 | 0.001921 | 0.002167 | 0.001718 |
| test_uuid_from_string_2.test | RUN | 0.273102 | 1.084201 | 1.167658 | 0.002330 | 0.002395 | 0.002512 |
| test_uuid_from_string_3.test | RUN | 0.275886 | 1.090616 | 1.213827 | 0.001762 | 0.002200 | 0.002296 |
| test_uuid_clock.test | RUN | 0.174845 | 0.451974 | 0.728700 | 0.001700 | 0.002153 | 0.001939 |
| test_nil_generator.test | RUN | 0.202818 | 0.552932 | 0.884500 | 0.001858 | 0.001940 | 0.002008 |
| test_string_generator.test | RUN | 0.259183 | 0.871887 | 1.151866 | 0.002146 | 0.002334 | 0.002404 |
| test_string_generator_2.test | RUN | 0.322565 | 1.234576 | 1.288492 | 0.001801 | 0.002021 | 0.002198 |
| test_random_generator.test | RUN | 0.418901 | 2.353155 | 1.971888 | 0.003292 | 0.003555 | 0.003634 |
| test_random_generator_2.test | RUN | 0.315988 | 0.921261 | 1.385119 | 0.001636 | 0.002146 | 0.002022 |
| test_name_generator.test | RUN | 0.279973 | 3.069713 | 1.382889 | 0.001970 | 0.001787 | 0.001747 |
| test_namespaces.test | RUN | 0.227702 | 0.651955 | 0.950717 | 0.001629 | 0.002006 | 0.002012 |
| test_name_generator_md5.test | RUN | 0.284512 | 1.438076 | 1.361887 | 0.001923 | 0.002060 | 0.002211 |
| test_name_generator_sha1.test | RUN | 0.274028 | 4.494852 | 1.319002 | 0.001859 | 0.001869 | 0.002390 |
| test_time_generator.test | RUN | 0.299350 | 0.917070 | 1.366951 | 0.001932 | 0.002218 | 0.001888 |
| test_time_generator_v1.test | RUN | 0.331304 | 0.979156 | 1.473973 | 0.002213 | 0.002615 | 0.002568 |
| test_time_generator_v1_2.test | RUN | 0.329654 | 0.942931 | 1.371589 | 0.002289 | 0.002466 | 0.002682 |
| test_time_generator_v1_3.test | RUN | 0.365115 | 0.994074 | 1.628445 | 0.005644 | 0.005343 | 0.006086 |
| test_time_generator_v6.test | RUN | 0.335279 | 1.029324 | 1.519710 | 0.002594 | 0.002614 | 0.002712 |
| test_time_generator_v6_2.test | RUN | 0.327779 | 0.958413 | 1.391242 | 0.001650 | 0.002412 | 0.002208 |
| test_time_generator_v6_3.test | RUN | 0.370123 | 1.000424 | 1.594641 | 0.005646 | 0.004329 | 0.006277 |
| test_time_generator_v7.test | RUN | 0.333712 | 1.019858 | 1.441900 | 0.002094 | 0.002314 | 0.002499 |
| test_time_generator_v7_2.test | RUN | 0.310709 | 0.875230 | 1.319348 | 0.001558 | 0.002328 | 0.002313 |
| test_time_generator_v7_3.test | RUN | 0.314449 | 1.124684 | 1.383462 | 0.002916 | 0.003247 | 0.002911 |
| test_serialization.test | RUN | 0.555719 | 1.702890 | 2.321136 | 0.002248 | 0.002111 | 0.002276 |
| test_tagging.test | RUN | 0.314118 | 1.076129 | 1.350129 | 0.001874 | 0.001802 | 0.002283 |
| test_uuid_class.test | RUN | 0.319803 | 0.961172 | 1.468962 | 0.001710 | 0.001701 | 0.002181 |
| test_uuid_in_map.test | RUN | 0.357699 | 1.110421 | 1.594053 | 0.001877 | 0.002196 | 0.001729 |
| test_hash.test | RUN | 0.363395 | 1.011388 | 1.473431 | 0.001936 | 0.001890 | 0.001921 |
| test_hash_value.test | RUN | 0.254957 | 0.680514 | 1.078614 | 0.006277 | 0.006193 | 0.006063 |
| test_std_unordered.test | RUN | 0.330370 | 1.110306 | 1.435354 | 0.002016 | 0.002007 | 0.001662 |
| test_boost_unordered.test | RUN | 0.482152 | 1.455718 | 1.864680 | 0.001675 | 0.001954 | 0.001996 |
| test_detail_md5.test | RUN | 0.227730 | 0.724627 | 1.036840 | 0.002226 | 0.002034 | 0.001978 |
| test_detail_sha1.test | RUN | 0.202589 | 0.869882 | 0.961712 | 0.012716 | 0.011253 | 0.011465 |
| test_detail_endian.test | RUN | 0.158273 | 0.519176 | 0.718173 | 0.001927 | 0.001894 | 0.001766 |
| test_detail_chacha20.test | RUN | 0.176362 | 0.633231 | 0.739767 | 0.001767 | 0.002110 | 0.001705 |
| random_provider_no_copy_ctor.test | COMPILE_FAIL | 0.157587 | 0.381014 | 0.617045 | — | — | — |
| random_provider_no_copy_assign.test | COMPILE_FAIL | 0.159772 | 0.361059 | 0.635387 | — | — | — |
| test_msvc_simd_bug981648.test | RUN | 0.269347 | 0.718778 | 1.076848 | 0.001931 | 0.002123 | 0.002214 |
| test_bench_random.test | RUN | 0.309962 | 0.927188 | 1.397695 | 0.008438 | 0.008466 | 0.008908 |
| test_entropy_error.test | RUN | 0.156726 | 0.423044 | 0.624709 | 0.001784 | 0.002075 | 0.002136 |
| test_detail_random_provider.test | RUN | 0.290436 | 0.736500 | 1.204284 | 0.001538 | 0.002271 | 0.001787 |
| test_constants.test | RUN | 0.229530 | 0.660474 | 0.937733 | 0.001878 | 0.002078 | 0.001918 |
| test_uuid_cx.test | COMPILE | 0.129989 | 0.304836 | 0.515287 | — | — | — |
| test_uuid_cx2.test | RUN | 0.238061 | 0.740567 | 1.071626 | 0.002049 | 0.002066 | 0.001880 |
| test_uuid_cx3.test | RUN | 0.210156 | 0.595108 | 0.965693 | 0.001947 | 0.001942 | 0.001997 |
| test_uuid_cx3_no_simd.test | RUN | 0.189920 | 0.569873 | 0.855864 | 0.001622 | 0.002118 | 0.001949 |
| test_hash_value_cx.test | RUN | 0.200333 | 0.532685 | 0.877632 | 0.001764 | 0.002104 | 0.001686 |
| test_string_generator_cx.test | RUN | 0.231508 | 0.659369 | 0.984152 | 0.001847 | 0.001980 | 0.001886 |
| test_string_generator_cx2.test | RUN | 0.283675 | 0.815553 | 1.101564 | 0.001798 | 0.002183 | 0.001972 |
| test_constants_cx.test | RUN | 0.191332 | 0.473796 | 0.805375 | 0.001880 | 0.001816 | 0.001871 |
| test_from_chars_cx.test | RUN | 0.241445 | 0.916407 | 1.030926 | 0.001794 | 0.002198 | 0.001776 |
| test_from_chars_cx2.test | RUN | 0.257203 | 1.100415 | 1.136610 | 0.001842 | 0.002006 | 0.002130 |
| test_to_chars_cx.test | RUN | 0.255940 | 0.992832 | 1.205618 | 0.001516 | 0.002008 | 0.002008 |
| test_uuid_from_string_cx.test | RUN | 0.266700 | 1.014106 | 1.150739 | 0.001523 | 0.001489 | 0.001537 |
| quick.test | RUN | 0.264568 | 0.757094 | 1.180047 | 0.001397 | 0.001669 | 0.001388 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.991× | 0.987×, 0.995× | 1.008× |
| clang | 0.999× | 1.003×, 0.996× | 0.999× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274616).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| quick.test | 10 | 0.001490s | 0.001507s | 0.001505s | byte equality observed |
| test_alignment.test | 10 | 0.001475s | 0.001462s | 0.001459s | byte equality observed |
| test_alignment_2.test | 10 | 0.001460s | 0.001462s | 0.001453s | byte equality observed |
| test_attribute_packed.test | 10 | 0.001440s | 0.001460s | 0.001427s | byte equality observed |
| test_bench_random.test | 3 | 0.008249s | 0.008219s | 0.008245s | native test oracle; output varies |
| test_boost_unordered.test | 10 | 0.001527s | 0.001468s | 0.001512s | byte equality observed |
| test_comparison.test | 10 | 0.001495s | 0.001515s | 0.001490s | byte equality observed |
| test_comparison_no_simd.test | 10 | 0.001519s | 0.001506s | 0.001508s | byte equality observed |
| test_constants.test | 10 | 0.001503s | 0.001518s | 0.001507s | byte equality observed |
| test_constants_cx.test | 10 | 0.001458s | 0.001451s | 0.001460s | byte equality observed |
| test_data.test | 10 | 0.001495s | 0.001494s | 0.001507s | byte equality observed |
| test_detail_chacha20.test | 10 | 0.001501s | 0.001472s | 0.001478s | byte equality observed |
| test_detail_endian.test | 10 | 0.001474s | 0.001471s | 0.001479s | byte equality observed |
| test_detail_md5.test | 10 | 0.001504s | 0.001502s | 0.001507s | byte equality observed |
| test_detail_random_provider.test | 10 | 0.001485s | 0.001478s | 0.001513s | byte equality observed |
| test_detail_sha1.test | 3 | 0.011592s | 0.010188s | 0.010957s | byte equality observed |
| test_entropy_error.test | 10 | 0.001451s | 0.001439s | 0.001461s | byte equality observed |
| test_from_chars.test | 10 | 0.001445s | 0.001437s | 0.001455s | byte equality observed |
| test_from_chars_2.test | 10 | 0.001462s | 0.001476s | 0.001461s | byte equality observed |
| test_from_chars_2_no_simd.test | 10 | 0.001502s | 0.001481s | 0.001509s | byte equality observed |
| test_from_chars_cx.test | 10 | 0.001507s | 0.001444s | 0.001487s | byte equality observed |
| test_from_chars_cx2.test | 10 | 0.001483s | 0.001458s | 0.001493s | byte equality observed |
| test_from_chars_no_simd.test | 10 | 0.001483s | 0.001502s | 0.001477s | byte equality observed |
| test_hash.test | 10 | 0.001477s | 0.001510s | 0.001484s | byte equality observed |
| test_hash_value.test | 4 | 0.005671s | 0.005397s | 0.005326s | byte equality observed |
| test_hash_value_cx.test | 10 | 0.001482s | 0.001458s | 0.001493s | byte equality observed |
| test_include1.test | 10 | 0.000643s | 0.000646s | 0.001456s | byte equality observed |
| test_io.test | 10 | 0.001577s | 0.001565s | 0.001575s | byte equality observed |
| test_io_2.test | 10 | 0.001676s | 0.001710s | 0.001690s | byte equality observed |
| test_io_2_no_simd.test | 10 | 0.001689s | 0.001730s | 0.001703s | byte equality observed |
| test_io_no_simd.test | 10 | 0.001586s | 0.001549s | 0.001585s | byte equality observed |
| test_msvc_simd_bug981648.test | 10 | 0.001510s | 0.001514s | 0.001529s | native test oracle; output varies |
| test_name_generator.test | 10 | 0.001537s | 0.001554s | 0.001513s | byte equality observed |
| test_name_generator_md5.test | 10 | 0.001517s | 0.001533s | 0.001530s | byte equality observed |
| test_name_generator_sha1.test | 10 | 0.001559s | 0.001529s | 0.001605s | byte equality observed |
| test_namespaces.test | 10 | 0.001497s | 0.001492s | 0.001484s | byte equality observed |
| test_nil_generator.test | 10 | 0.001455s | 0.001485s | 0.001505s | byte equality observed |
| test_pragma_pack.test | 10 | 0.001465s | 0.001505s | 0.001508s | byte equality observed |
| test_random_generator.test | 8 | 0.002907s | 0.002875s | 0.002860s | native test oracle; output varies |
| test_random_generator_2.test | 10 | 0.001483s | 0.001489s | 0.001509s | byte equality observed |
| test_serialization.test | 10 | 0.001645s | 0.001647s | 0.001592s | byte equality observed |
| test_std_unordered.test | 10 | 0.001607s | 0.001591s | 0.001582s | byte equality observed |
| test_string_generator.test | 10 | 0.001643s | 0.001665s | 0.001628s | byte equality observed |
| test_string_generator_2.test | 10 | 0.001518s | 0.001574s | 0.001558s | byte equality observed |
| test_string_generator_cx.test | 10 | 0.001503s | 0.001521s | 0.001474s | byte equality observed |
| test_string_generator_cx2.test | 10 | 0.001532s | 0.001499s | 0.001525s | byte equality observed |
| test_tagging.test | 10 | 0.001503s | 0.001521s | 0.001505s | byte equality observed |
| test_time_generator.test | 8 | 0.001544s | 0.001525s | 0.001533s | byte equality observed |
| test_time_generator_v1.test | 10 | 0.002089s | 0.002195s | 0.002216s | byte equality observed |
| test_time_generator_v1_2.test | 10 | 0.001836s | 0.001877s | 0.001898s | byte equality observed |
| test_time_generator_v1_3.test | 5 | 0.004688s | 0.004662s | 0.004595s | byte equality observed |
| test_time_generator_v6.test | 10 | 0.002046s | 0.002138s | 0.002211s | byte equality observed |
| test_time_generator_v6_2.test | 9 | 0.001840s | 0.001880s | 0.001878s | byte equality observed |
| test_time_generator_v6_3.test | 4 | 0.004850s | 0.004615s | 0.004714s | byte equality observed |
| test_time_generator_v7.test | 10 | 0.001809s | 0.001883s | 0.001862s | byte equality observed |
| test_time_generator_v7_2.test | 10 | 0.001589s | 0.001579s | 0.001551s | byte equality observed |
| test_time_generator_v7_3.test | 6 | 0.002534s | 0.002507s | 0.002502s | byte equality observed |
| test_to_chars.test | 10 | 0.001596s | 0.001606s | 0.001584s | byte equality observed |
| test_to_chars_2.test | 10 | 0.001537s | 0.001516s | 0.001502s | byte equality observed |
| test_to_chars_2_no_simd.test | 10 | 0.001550s | 0.001534s | 0.001522s | byte equality observed |
| test_to_chars_3.test | 10 | 0.001541s | 0.001537s | 0.001534s | byte equality observed |
| test_to_chars_cx.test | 10 | 0.001526s | 0.001519s | 0.001510s | byte equality observed |
| test_to_chars_no_simd.test | 10 | 0.001633s | 0.001637s | 0.001597s | byte equality observed |
| test_uuid.test | 10 | 0.001491s | 0.001477s | 0.001490s | byte equality observed |
| test_uuid_2.test | 10 | 0.001492s | 0.001536s | 0.001510s | byte equality observed |
| test_uuid_3.test | 10 | 0.001494s | 0.001460s | 0.001521s | byte equality observed |
| test_uuid_class.test | 10 | 0.001522s | 0.001531s | 0.001557s | byte equality observed |
| test_uuid_clock.test | 10 | 0.001503s | 0.001479s | 0.001489s | byte equality observed |
| test_uuid_cx2.test | 10 | 0.001528s | 0.001488s | 0.001510s | byte equality observed |
| test_uuid_cx3.test | 10 | 0.001496s | 0.001475s | 0.001514s | byte equality observed |
| test_uuid_cx3_no_simd.test | 10 | 0.001506s | 0.001463s | 0.001509s | byte equality observed |
| test_uuid_from_string.test | 10 | 0.001549s | 0.001493s | 0.001506s | byte equality observed |
| test_uuid_from_string_2.test | 10 | 0.001913s | 0.001906s | 0.001934s | byte equality observed |
| test_uuid_from_string_3.test | 10 | 0.001631s | 0.001606s | 0.001647s | byte equality observed |
| test_uuid_from_string_cx.test | 10 | 0.001500s | 0.001498s | 0.001477s | byte equality observed |
| test_uuid_in_map.test | 10 | 0.001521s | 0.001508s | 0.001514s | byte equality observed |
| test_uuid_no_simd.test | 10 | 0.001512s | 0.001500s | 0.001483s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274616).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
