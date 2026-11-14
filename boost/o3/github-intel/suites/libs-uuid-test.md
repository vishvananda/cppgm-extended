# libs/uuid/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_include1.test | RUN | 0.328013 | 0.779479 | 1.132061 | 0.000486 | 0.000439 | 0.001396 |
| test_uuid.test | RUN | 0.199637 | 0.657369 | 0.813149 | 0.001092 | 0.001164 | 0.001572 |
| test_uuid_no_simd.test | RUN | 0.191937 | 0.629155 | 0.724345 | 0.001192 | 0.001265 | 0.001320 |
| test_uuid_2.test | RUN | 0.147366 | 0.483256 | 0.544824 | 0.000987 | 0.001387 | 0.001656 |
| test_uuid_3.test | RUN | 0.113782 | 0.322383 | 0.437945 | 0.001178 | 0.001208 | 0.001544 |
| test_alignment.test | RUN | 0.107468 | 0.289422 | 0.378088 | 0.001214 | 0.001179 | 0.001572 |
| test_alignment_2.test | RUN | 0.111179 | 0.291418 | 0.385951 | 0.001022 | 0.001219 | 0.001178 |
| test_attribute_packed.test | RUN | 0.110326 | 0.291439 | 0.393613 | 0.001067 | 0.001191 | 0.001253 |
| test_pragma_pack.test | RUN | 0.104762 | 0.296354 | 0.393785 | 0.001089 | 0.001150 | 0.001178 |
| test_data.test | RUN | 0.127893 | 0.549747 | 0.551665 | 0.000989 | 0.001263 | 0.001418 |
| test_comparison.test | RUN | 0.187063 | 0.634938 | 0.767660 | 0.001088 | 0.001266 | 0.001276 |
| test_comparison_no_simd.test | RUN | 0.175227 | 0.715642 | 0.667228 | 0.001085 | 0.001120 | 0.001582 |
| test_io.test | RUN | 0.247402 | 1.226553 | 1.152092 | 0.001465 | 0.001523 | 0.001584 |
| test_io_no_simd.test | RUN | 0.220467 | 1.211481 | 1.063205 | 0.001074 | 0.001205 | 0.001432 |
| test_io_2.test | RUN | 0.194763 | 1.286691 | 0.791190 | 0.001167 | 0.001272 | 0.001349 |
| test_io_2_no_simd.test | RUN | 0.174241 | 1.204393 | 0.719652 | 0.001305 | 0.001393 | 0.001440 |
| test_to_chars.test | RUN | 0.171621 | 0.792490 | 0.734835 | 0.001291 | 0.001361 | 0.001290 |
| test_to_chars_no_simd.test | RUN | 0.160515 | 0.806760 | 0.696272 | 0.001295 | 0.001358 | 0.001363 |
| test_to_chars_2.test | RUN | 0.208958 | 1.482678 | 0.858988 | 0.000983 | 0.001344 | 0.001193 |
| test_to_chars_2_no_simd.test | RUN | 0.195608 | 1.566886 | 0.781773 | 0.001087 | 0.001319 | 0.001180 |
| test_to_chars_3.test | RUN | 0.139909 | 0.507110 | 0.520471 | 0.001113 | 0.001174 | 0.001166 |
| test_from_chars.test | RUN | 0.165684 | 0.743182 | 0.594539 | 0.001225 | 0.001164 | 0.001350 |
| test_from_chars_no_simd.test | RUN | 0.143340 | 0.590019 | 0.496654 | 0.001119 | 0.001161 | 0.001316 |
| test_from_chars_2.test | RUN | 0.160823 | 0.697167 | 0.570589 | 0.001096 | 0.001162 | 0.001133 |
| test_from_chars_2_no_simd.test | RUN | 0.141576 | 0.527780 | 0.474441 | 0.001115 | 0.001146 | 0.001402 |
| test_uuid_from_string.test | RUN | 0.189300 | 0.756097 | 0.707397 | 0.001145 | 0.001170 | 0.001135 |
| test_uuid_from_string_2.test | RUN | 0.166907 | 0.691837 | 0.593751 | 0.001448 | 0.001496 | 0.001308 |
| test_uuid_from_string_3.test | RUN | 0.176071 | 0.702557 | 0.622295 | 0.001212 | 0.001467 | 0.001204 |
| test_uuid_clock.test | RUN | 0.103393 | 0.283953 | 0.361905 | 0.001192 | 0.001249 | 0.001234 |
| test_nil_generator.test | RUN | 0.118202 | 0.326002 | 0.428113 | 0.001091 | 0.001242 | 0.001510 |
| test_string_generator.test | RUN | 0.157870 | 0.557895 | 0.590910 | 0.001256 | 0.001351 | 0.001433 |
| test_string_generator_2.test | RUN | 0.201960 | 0.787608 | 0.672266 | 0.001224 | 0.001144 | 0.001305 |
| test_random_generator.test | RUN | 0.261545 | 1.505767 | 1.006863 | 0.004256 | 0.004355 | 0.004435 |
| test_random_generator_2.test | RUN | 0.191621 | 0.572151 | 0.679852 | 0.001145 | 0.001252 | 0.001392 |
| test_name_generator.test | RUN | 0.176731 | 1.986560 | 0.734166 | 0.001148 | 0.001324 | 0.001178 |
| test_namespaces.test | RUN | 0.130484 | 0.412226 | 0.487856 | 0.001145 | 0.001192 | 0.001148 |
| test_name_generator_md5.test | RUN | 0.174445 | 0.932607 | 0.708838 | 0.001177 | 0.001183 | 0.001279 |
| test_name_generator_sha1.test | RUN | 0.173661 | 2.911819 | 0.703040 | 0.001143 | 0.001408 | 0.001384 |
| test_time_generator.test | RUN | 0.182859 | 0.574256 | 0.655050 | 0.001101 | 0.001434 | 0.001204 |
| test_time_generator_v1.test | RUN | 0.198125 | 0.638099 | 0.742801 | 0.001535 | 0.001733 | 0.001873 |
| test_time_generator_v1_2.test | RUN | 0.200177 | 0.587228 | 0.697929 | 0.001554 | 0.001498 | 0.001638 |
| test_time_generator_v1_3.test | RUN | 0.219802 | 0.620126 | 0.769345 | 0.003780 | 0.004192 | 0.003432 |
| test_time_generator_v6.test | RUN | 0.202829 | 0.633451 | 0.734170 | 0.001577 | 0.001706 | 0.001740 |
| test_time_generator_v6_2.test | RUN | 0.200700 | 0.600137 | 0.695243 | 0.001410 | 0.001577 | 0.001484 |
| test_time_generator_v6_3.test | RUN | 0.220176 | 0.618705 | 0.778993 | 0.003655 | 0.003790 | 0.003880 |
| test_time_generator_v7.test | RUN | 0.191300 | 0.637589 | 0.703601 | 0.001496 | 0.001482 | 0.001513 |
| test_time_generator_v7_2.test | RUN | 0.179623 | 0.552342 | 0.656038 | 0.001092 | 0.001330 | 0.001228 |
| test_time_generator_v7_3.test | RUN | 0.178703 | 0.703452 | 0.671566 | 0.002659 | 0.002842 | 0.002707 |
| test_serialization.test | RUN | 0.329123 | 1.075310 | 1.208600 | 0.001305 | 0.001209 | 0.001378 |
| test_tagging.test | RUN | 0.185811 | 0.677216 | 0.669695 | 0.001129 | 0.001130 | 0.001277 |
| test_uuid_class.test | RUN | 0.191318 | 0.594186 | 0.701157 | 0.001080 | 0.001005 | 0.001174 |
| test_uuid_in_map.test | RUN | 0.214969 | 0.686137 | 0.765032 | 0.001108 | 0.001127 | 0.001304 |
| test_hash.test | RUN | 0.207664 | 0.611878 | 0.708523 | 0.001178 | 0.001156 | 0.001230 |
| test_hash_value.test | RUN | 0.145158 | 0.426633 | 0.527595 | 0.004054 | 0.004090 | 0.004149 |
| test_std_unordered.test | RUN | 0.194996 | 0.695629 | 0.715318 | 0.001157 | 0.001322 | 0.001343 |
| test_boost_unordered.test | RUN | 0.283139 | 0.899272 | 0.921072 | 0.001179 | 0.001333 | 0.001075 |
| test_detail_md5.test | RUN | 0.129494 | 0.432849 | 0.514825 | 0.001128 | 0.001142 | 0.001211 |
| test_detail_sha1.test | RUN | 0.123996 | 0.553188 | 0.460726 | 0.007482 | 0.008140 | 0.007177 |
| test_detail_endian.test | RUN | 0.093150 | 0.312626 | 0.328381 | 0.001114 | 0.001205 | 0.001158 |
| test_detail_chacha20.test | RUN | 0.105095 | 0.392109 | 0.364737 | 0.000985 | 0.001246 | 0.001216 |
| random_provider_no_copy_ctor.test | COMPILE_FAIL | 0.092809 | 0.214447 | 0.281252 | — | — | — |
| random_provider_no_copy_assign.test | COMPILE_FAIL | 0.091472 | 0.213015 | 0.280272 | — | — | — |
| test_msvc_simd_bug981648.test | RUN | 0.153190 | 0.433959 | 0.508503 | 0.001180 | 0.001288 | 0.001329 |
| test_bench_random.test | RUN | 0.178675 | 0.563768 | 0.678910 | 0.017058 | 0.017210 | 0.017018 |
| test_entropy_error.test | RUN | 0.089410 | 0.259317 | 0.309706 | 0.001168 | 0.001160 | 0.001083 |
| test_detail_random_provider.test | RUN | 0.178314 | 0.437714 | 0.584446 | 0.001203 | 0.001226 | 0.001189 |
| test_constants.test | RUN | 0.134852 | 0.416429 | 0.471683 | 0.001250 | 0.001216 | 0.001197 |
| test_uuid_cx.test | COMPILE | 0.078958 | 0.188027 | 0.233891 | — | — | — |
| test_uuid_cx2.test | RUN | 0.148518 | 0.469698 | 0.508943 | 0.001260 | 0.001349 | 0.001242 |
| test_uuid_cx3.test | RUN | 0.123639 | 0.363734 | 0.468855 | 0.001135 | 0.001174 | 0.001051 |
| test_uuid_cx3_no_simd.test | RUN | 0.118183 | 0.348274 | 0.403280 | 0.001268 | 0.001222 | 0.001088 |
| test_hash_value_cx.test | RUN | 0.117858 | 0.323040 | 0.415884 | 0.001136 | 0.001140 | 0.001223 |
| test_string_generator_cx.test | RUN | 0.140135 | 0.414465 | 0.478227 | 0.001144 | 0.001081 | 0.001113 |
| test_string_generator_cx2.test | RUN | 0.177385 | 0.509416 | 0.555433 | 0.001076 | 0.001114 | 0.001145 |
| test_constants_cx.test | RUN | 0.112890 | 0.277309 | 0.382136 | 0.001106 | 0.001194 | 0.001413 |
| test_from_chars_cx.test | RUN | 0.153860 | 0.548471 | 0.505659 | 0.001202 | 0.001196 | 0.001123 |
| test_from_chars_cx2.test | RUN | 0.160974 | 0.681285 | 0.566526 | 0.001178 | 0.001085 | 0.001253 |
| test_to_chars_cx.test | RUN | 0.155593 | 0.635034 | 0.620244 | 0.000912 | 0.001351 | 0.000842 |
| test_uuid_from_string_cx.test | RUN | 0.166266 | 0.642351 | 0.567964 | 0.000870 | 0.000798 | 0.000841 |
| quick.test | RUN | 0.176833 | 0.473466 | 0.566845 | 0.000823 | 0.000775 | 0.000809 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.009× | 1.009×, 1.010× | 0.997× |
| clang | 1.005× | 1.002×, 1.008× | 1.001× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814672).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| quick.test | 10 | 0.000798s | 0.000825s | 0.000804s | byte equality observed |
| test_alignment.test | 10 | 0.000799s | 0.000812s | 0.000781s | byte equality observed |
| test_alignment_2.test | 10 | 0.000815s | 0.000803s | 0.000792s | byte equality observed |
| test_attribute_packed.test | 10 | 0.000737s | 0.000743s | 0.000767s | byte equality observed |
| test_bench_random.test | 2 | 0.016694s | 0.016680s | 0.016659s | native test oracle; output varies |
| test_boost_unordered.test | 10 | 0.000799s | 0.000784s | 0.000798s | byte equality observed |
| test_comparison.test | 10 | 0.000760s | 0.000737s | 0.000768s | byte equality observed |
| test_comparison_no_simd.test | 10 | 0.000781s | 0.000780s | 0.000791s | byte equality observed |
| test_constants.test | 10 | 0.000808s | 0.000823s | 0.000801s | byte equality observed |
| test_constants_cx.test | 10 | 0.000801s | 0.000783s | 0.000764s | byte equality observed |
| test_data.test | 10 | 0.000813s | 0.000798s | 0.000796s | byte equality observed |
| test_detail_chacha20.test | 10 | 0.000799s | 0.000787s | 0.000810s | byte equality observed |
| test_detail_endian.test | 10 | 0.000822s | 0.000801s | 0.000820s | byte equality observed |
| test_detail_md5.test | 10 | 0.000869s | 0.000819s | 0.000812s | byte equality observed |
| test_detail_random_provider.test | 10 | 0.000824s | 0.000820s | 0.000790s | byte equality observed |
| test_detail_sha1.test | 3 | 0.005466s | 0.006432s | 0.005745s | byte equality observed |
| test_entropy_error.test | 10 | 0.000750s | 0.000754s | 0.000733s | byte equality observed |
| test_from_chars.test | 10 | 0.000790s | 0.000787s | 0.000792s | byte equality observed |
| test_from_chars_2.test | 10 | 0.000805s | 0.000816s | 0.000815s | byte equality observed |
| test_from_chars_2_no_simd.test | 10 | 0.000808s | 0.000800s | 0.000819s | byte equality observed |
| test_from_chars_cx.test | 10 | 0.000818s | 0.000818s | 0.000836s | byte equality observed |
| test_from_chars_cx2.test | 10 | 0.000810s | 0.000813s | 0.000840s | byte equality observed |
| test_from_chars_no_simd.test | 10 | 0.000818s | 0.000853s | 0.000851s | byte equality observed |
| test_hash.test | 9 | 0.000783s | 0.000803s | 0.000815s | byte equality observed |
| test_hash_value.test | 6 | 0.003251s | 0.003172s | 0.003148s | byte equality observed |
| test_hash_value_cx.test | 10 | 0.000758s | 0.000770s | 0.000753s | byte equality observed |
| test_include1.test | 10 | 0.000346s | 0.000359s | 0.000775s | byte equality observed |
| test_io.test | 10 | 0.000860s | 0.000870s | 0.000838s | byte equality observed |
| test_io_2.test | 10 | 0.000917s | 0.000901s | 0.000875s | byte equality observed |
| test_io_2_no_simd.test | 10 | 0.000909s | 0.000911s | 0.000878s | byte equality observed |
| test_io_no_simd.test | 10 | 0.000880s | 0.000902s | 0.000862s | byte equality observed |
| test_msvc_simd_bug981648.test | 10 | 0.000830s | 0.000834s | 0.000814s | native test oracle; output varies |
| test_name_generator.test | 10 | 0.000857s | 0.000843s | 0.000828s | byte equality observed |
| test_name_generator_md5.test | 10 | 0.000826s | 0.000863s | 0.000823s | byte equality observed |
| test_name_generator_sha1.test | 10 | 0.000845s | 0.000862s | 0.000822s | byte equality observed |
| test_namespaces.test | 10 | 0.000846s | 0.000824s | 0.000791s | byte equality observed |
| test_nil_generator.test | 10 | 0.000839s | 0.000805s | 0.000824s | byte equality observed |
| test_pragma_pack.test | 10 | 0.000782s | 0.000781s | 0.000791s | byte equality observed |
| test_random_generator.test | 5 | 0.003886s | 0.003870s | 0.003866s | native test oracle; output varies |
| test_random_generator_2.test | 10 | 0.000769s | 0.000765s | 0.000767s | byte equality observed |
| test_serialization.test | 10 | 0.000883s | 0.000870s | 0.000842s | byte equality observed |
| test_std_unordered.test | 10 | 0.000880s | 0.000848s | 0.000890s | byte equality observed |
| test_string_generator.test | 10 | 0.000872s | 0.000883s | 0.000876s | byte equality observed |
| test_string_generator_2.test | 10 | 0.000829s | 0.000847s | 0.000810s | byte equality observed |
| test_string_generator_cx.test | 10 | 0.000830s | 0.000820s | 0.000815s | byte equality observed |
| test_string_generator_cx2.test | 10 | 0.000848s | 0.000816s | 0.000842s | byte equality observed |
| test_tagging.test | 10 | 0.000831s | 0.000826s | 0.000813s | byte equality observed |
| test_time_generator.test | 10 | 0.000841s | 0.000847s | 0.000809s | byte equality observed |
| test_time_generator_v1.test | 10 | 0.001142s | 0.001213s | 0.001239s | byte equality observed |
| test_time_generator_v1_2.test | 10 | 0.001016s | 0.001069s | 0.001050s | byte equality observed |
| test_time_generator_v1_3.test | 5 | 0.004466s | 0.004386s | 0.004446s | byte equality observed |
| test_time_generator_v6.test | 10 | 0.001163s | 0.001260s | 0.001272s | byte equality observed |
| test_time_generator_v6_2.test | 10 | 0.001021s | 0.001048s | 0.001058s | byte equality observed |
| test_time_generator_v6_3.test | 5 | 0.004776s | 0.004567s | 0.004694s | byte equality observed |
| test_time_generator_v7.test | 10 | 0.001015s | 0.001038s | 0.001053s | byte equality observed |
| test_time_generator_v7_2.test | 10 | 0.000862s | 0.000868s | 0.000866s | byte equality observed |
| test_time_generator_v7_3.test | 7 | 0.002412s | 0.002409s | 0.002391s | byte equality observed |
| test_to_chars.test | 10 | 0.000828s | 0.000819s | 0.000811s | byte equality observed |
| test_to_chars_2.test | 10 | 0.000794s | 0.000832s | 0.000800s | byte equality observed |
| test_to_chars_2_no_simd.test | 10 | 0.000816s | 0.000851s | 0.000810s | byte equality observed |
| test_to_chars_3.test | 10 | 0.000818s | 0.000816s | 0.000801s | byte equality observed |
| test_to_chars_cx.test | 10 | 0.000810s | 0.000852s | 0.000790s | byte equality observed |
| test_to_chars_no_simd.test | 10 | 0.000855s | 0.000899s | 0.000829s | byte equality observed |
| test_uuid.test | 10 | 0.000798s | 0.000787s | 0.000826s | byte equality observed |
| test_uuid_2.test | 10 | 0.000793s | 0.000811s | 0.000788s | byte equality observed |
| test_uuid_3.test | 10 | 0.000823s | 0.000795s | 0.000798s | byte equality observed |
| test_uuid_class.test | 10 | 0.000817s | 0.000819s | 0.000839s | byte equality observed |
| test_uuid_clock.test | 10 | 0.000825s | 0.000792s | 0.000808s | byte equality observed |
| test_uuid_cx2.test | 10 | 0.000837s | 0.000814s | 0.000805s | byte equality observed |
| test_uuid_cx3.test | 10 | 0.000808s | 0.000797s | 0.000812s | byte equality observed |
| test_uuid_cx3_no_simd.test | 10 | 0.000805s | 0.000809s | 0.000809s | byte equality observed |
| test_uuid_from_string.test | 10 | 0.000828s | 0.000799s | 0.000816s | byte equality observed |
| test_uuid_from_string_2.test | 10 | 0.000957s | 0.000982s | 0.001007s | byte equality observed |
| test_uuid_from_string_3.test | 10 | 0.000891s | 0.000879s | 0.000867s | byte equality observed |
| test_uuid_from_string_cx.test | 10 | 0.000790s | 0.000791s | 0.000804s | byte equality observed |
| test_uuid_in_map.test | 10 | 0.000798s | 0.000843s | 0.000826s | byte equality observed |
| test_uuid_no_simd.test | 10 | 0.000793s | 0.000831s | 0.000812s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814672).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
