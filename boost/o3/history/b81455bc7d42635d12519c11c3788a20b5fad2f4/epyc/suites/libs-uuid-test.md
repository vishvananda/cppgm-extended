# libs/uuid/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_include1.test | RUN | 0.305342 | 0.668703 | 0.878228 | 0.000537 | 0.000653 | 0.001433 |
| test_uuid.test | RUN | 0.184484 | 0.555548 | 0.623422 | 0.001293 | 0.001517 | 0.001414 |
| test_uuid_no_simd.test | RUN | 0.187028 | 0.547814 | 0.591910 | 0.001428 | 0.001208 | 0.001233 |
| test_uuid_2.test | RUN | 0.122726 | 0.401115 | 0.409903 | 0.001269 | 0.001563 | 0.001391 |
| test_uuid_3.test | RUN | 0.113023 | 0.282187 | 0.342290 | 0.001279 | 0.001684 | 0.001115 |
| test_alignment.test | RUN | 0.099561 | 0.278412 | 0.294699 | 0.001365 | 0.001348 | 0.001456 |
| test_alignment_2.test | RUN | 0.097832 | 0.253732 | 0.305597 | 0.001218 | 0.001482 | 0.001336 |
| test_attribute_packed.test | RUN | 0.108957 | 0.265249 | 0.298951 | 0.001504 | 0.001652 | 0.001283 |
| test_pragma_pack.test | RUN | 0.099927 | 0.264646 | 0.305649 | 0.001341 | 0.001617 | 0.001277 |
| test_data.test | RUN | 0.116916 | 0.468864 | 0.438615 | 0.001314 | 0.001525 | 0.001405 |
| test_comparison.test | RUN | 0.178730 | 0.551677 | 0.596697 | 0.001675 | 0.001662 | 0.001126 |
| test_comparison_no_simd.test | RUN | 0.159292 | 0.636858 | 0.548809 | 0.001178 | 0.001393 | 0.001460 |
| test_io.test | RUN | 0.218078 | 1.047704 | 0.887512 | 0.001639 | 0.001626 | 0.001476 |
| test_io_no_simd.test | RUN | 0.209214 | 1.022392 | 0.859886 | 0.001336 | 0.001610 | 0.001448 |
| test_io_2.test | RUN | 0.172034 | 1.062048 | 0.647445 | 0.001221 | 0.001696 | 0.001473 |
| test_io_2_no_simd.test | RUN | 0.152418 | 0.996913 | 0.596010 | 0.001694 | 0.001747 | 0.001399 |
| test_to_chars.test | RUN | 0.155487 | 0.685596 | 0.596618 | 0.001600 | 0.001711 | 0.001488 |
| test_to_chars_no_simd.test | RUN | 0.143293 | 0.703974 | 0.592209 | 0.001457 | 0.001710 | 0.001517 |
| test_to_chars_2.test | RUN | 0.182781 | 1.279241 | 0.711243 | 0.001386 | 0.001544 | 0.001361 |
| test_to_chars_2_no_simd.test | RUN | 0.179915 | 1.331312 | 0.655720 | 0.001357 | 0.001306 | 0.001461 |
| test_to_chars_3.test | RUN | 0.120400 | 0.460591 | 0.437954 | 0.001169 | 0.001508 | 0.001355 |
| test_from_chars.test | RUN | 0.147144 | 0.645304 | 0.478765 | 0.001397 | 0.001497 | 0.001343 |
| test_from_chars_no_simd.test | RUN | 0.118797 | 0.512406 | 0.411576 | 0.001182 | 0.001262 | 0.001277 |
| test_from_chars_2.test | RUN | 0.137741 | 0.593561 | 0.470809 | 0.001313 | 0.001462 | 0.001439 |
| test_from_chars_2_no_simd.test | RUN | 0.121642 | 0.462201 | 0.384042 | 0.001289 | 0.001692 | 0.001325 |
| test_uuid_from_string.test | RUN | 0.164080 | 0.665031 | 0.593833 | 0.001308 | 0.001661 | 0.001313 |
| test_uuid_from_string_2.test | RUN | 0.146285 | 0.615012 | 0.486708 | 0.001507 | 0.001713 | 0.001766 |
| test_uuid_from_string_3.test | RUN | 0.149702 | 0.612346 | 0.513443 | 0.001153 | 0.001475 | 0.001346 |
| test_uuid_clock.test | RUN | 0.091007 | 0.248633 | 0.297528 | 0.001388 | 0.001643 | 0.001323 |
| test_nil_generator.test | RUN | 0.106489 | 0.306341 | 0.348237 | 0.001372 | 0.001429 | 0.001458 |
| test_string_generator.test | RUN | 0.129412 | 0.485524 | 0.484678 | 0.001496 | 0.001720 | 0.001335 |
| test_string_generator_2.test | RUN | 0.156122 | 0.683720 | 0.523497 | 0.001472 | 0.001506 | 0.001492 |
| test_random_generator.test | RUN | 0.229177 | 1.292442 | 0.832180 | 0.059060 | 0.059465 | 0.059028 |
| test_random_generator_2.test | RUN | 0.161506 | 0.498823 | 0.544795 | 0.001290 | 0.001570 | 0.001433 |
| test_name_generator.test | RUN | 0.144278 | 1.565738 | 0.597983 | 0.001420 | 0.001628 | 0.001321 |
| test_namespaces.test | RUN | 0.120817 | 0.351984 | 0.363491 | 0.001323 | 0.001558 | 0.001538 |
| test_name_generator_md5.test | RUN | 0.145887 | 0.788874 | 0.584981 | 0.001414 | 0.001598 | 0.001544 |
| test_name_generator_sha1.test | RUN | 0.140885 | 2.352435 | 0.571501 | 0.001456 | 0.001616 | 0.001550 |
| test_time_generator.test | RUN | 0.161982 | 0.496539 | 0.528055 | 0.001408 | 0.001553 | 0.001382 |
| test_time_generator_v1.test | RUN | 0.180137 | 0.547311 | 0.595186 | 0.001666 | 0.002054 | 0.001882 |
| test_time_generator_v1_2.test | RUN | 0.169503 | 0.509774 | 0.576584 | 0.001543 | 0.001842 | 0.001745 |
| test_time_generator_v1_3.test | RUN | 0.187646 | 0.530998 | 0.610271 | 0.003273 | 0.003796 | 0.003680 |
| test_time_generator_v6.test | RUN | 0.183090 | 0.547802 | 0.599443 | 0.001782 | 0.002134 | 0.001794 |
| test_time_generator_v6_2.test | RUN | 0.185007 | 0.526986 | 0.577719 | 0.001458 | 0.001764 | 0.001443 |
| test_time_generator_v6_3.test | RUN | 0.191485 | 0.561550 | 0.622836 | 0.002494 | 0.004241 | 0.004543 |
| test_time_generator_v7.test | RUN | 0.170476 | 0.569462 | 0.567018 | 0.001605 | 0.001928 | 0.001484 |
| test_time_generator_v7_2.test | RUN | 0.164376 | 0.494200 | 0.524939 | 0.001221 | 0.001740 | 0.001281 |
| test_time_generator_v7_3.test | RUN | 0.163588 | 0.639273 | 0.541105 | 0.030035 | 0.030376 | 0.030197 |
| test_serialization.test | RUN | 0.298614 | 0.952740 | 0.990643 | 0.001731 | 0.001486 | 0.001560 |
| test_tagging.test | RUN | 0.166695 | 0.570943 | 0.531510 | 0.001367 | 0.001438 | 0.001435 |
| test_uuid_class.test | RUN | 0.172651 | 0.497947 | 0.573845 | 0.001416 | 0.001644 | 0.001300 |
| test_uuid_in_map.test | RUN | 0.183984 | 0.582576 | 0.625637 | 0.001302 | 0.001661 | 0.001299 |
| test_hash.test | RUN | 0.190001 | 0.522976 | 0.577427 | 0.001432 | 0.001651 | 0.001449 |
| test_hash_value.test | RUN | 0.135341 | 0.354946 | 0.419451 | 0.003709 | 0.003855 | 0.003728 |
| test_std_unordered.test | RUN | 0.185465 | 0.584664 | 0.584274 | 0.001263 | 0.001704 | 0.001553 |
| test_boost_unordered.test | RUN | 0.258017 | 0.757195 | 0.774804 | 0.001403 | 0.001372 | 0.001273 |
| test_detail_md5.test | RUN | 0.113377 | 0.364836 | 0.417986 | 0.001283 | 0.001476 | 0.001306 |
| test_detail_sha1.test | RUN | 0.112020 | 0.461654 | 0.386011 | 0.006903 | 0.006883 | 0.005824 |
| test_detail_endian.test | RUN | 0.087662 | 0.266826 | 0.297216 | 0.001379 | 0.001600 | 0.001427 |
| test_detail_chacha20.test | RUN | 0.095845 | 0.327923 | 0.302167 | 0.001506 | 0.001323 | 0.001381 |
| random_provider_no_copy_ctor.test | COMPILE_FAIL | 0.084929 | 0.185061 | 0.230404 | — | — | — |
| random_provider_no_copy_assign.test | COMPILE_FAIL | 0.083908 | 0.188812 | 0.230176 | — | — | — |
| test_msvc_simd_bug981648.test | RUN | 0.142734 | 0.371188 | 0.411354 | 0.001474 | 0.001530 | 0.001413 |
| test_bench_random.test | RUN | 0.159344 | 0.478585 | 0.557438 | 0.311430 | 0.312238 | 0.312435 |
| test_entropy_error.test | RUN | 0.081467 | 0.225296 | 0.256505 | 0.001329 | 0.001409 | 0.001209 |
| test_detail_random_provider.test | RUN | 0.154068 | 0.379076 | 0.471846 | 0.002766 | 0.002906 | 0.002669 |
| test_constants.test | RUN | 0.118362 | 0.356726 | 0.345378 | 0.001133 | 0.001401 | 0.001248 |
| test_uuid_cx.test | COMPILE | 0.071610 | 0.138516 | 0.199356 | — | — | — |
| test_uuid_cx2.test | RUN | 0.121055 | 0.370322 | 0.428884 | 0.001466 | 0.001422 | 0.001292 |
| test_uuid_cx3.test | RUN | 0.110897 | 0.310886 | 0.385884 | 0.001228 | 0.001387 | 0.001239 |
| test_uuid_cx3_no_simd.test | RUN | 0.085235 | 0.291249 | 0.342515 | 0.001349 | 0.001243 | 0.001454 |
| test_hash_value_cx.test | RUN | 0.106628 | 0.285142 | 0.341488 | 0.001498 | 0.001516 | 0.001196 |
| test_string_generator_cx.test | RUN | 0.115095 | 0.355408 | 0.390307 | 0.001253 | 0.001409 | 0.001202 |
| test_string_generator_cx2.test | RUN | 0.128079 | 0.427770 | 0.445650 | 0.001204 | 0.001462 | 0.001373 |
| test_constants_cx.test | RUN | 0.103843 | 0.244932 | 0.319767 | 0.001318 | 0.001327 | 0.001231 |
| test_from_chars_cx.test | RUN | 0.129903 | 0.470523 | 0.417921 | 0.001456 | 0.001534 | 0.001360 |
| test_from_chars_cx2.test | RUN | 0.135854 | 0.583861 | 0.473506 | 0.001396 | 0.001062 | 0.001352 |
| test_to_chars_cx.test | RUN | 0.134461 | 0.545277 | 0.504691 | 0.001369 | 0.001386 | 0.001103 |
| test_uuid_from_string_cx.test | RUN | 0.143673 | 0.550875 | 0.494056 | 0.000958 | 0.001177 | 0.001097 |
| quick.test | RUN | 0.104264 | 0.446678 | 0.487758 | 0.001064 | 0.001010 | 0.000963 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.002× | 1.008×, 0.996× | 1.001× |
| clang | 0.999× | 0.999×, 0.999× | 1.006× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409407).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| quick.test | 10 | 0.000847s | 0.000842s | 0.000844s | byte equality observed |
| test_alignment.test | 10 | 0.000844s | 0.000880s | 0.000859s | byte equality observed |
| test_alignment_2.test | 10 | 0.000851s | 0.000867s | 0.000869s | byte equality observed |
| test_attribute_packed.test | 10 | 0.000829s | 0.000857s | 0.000831s | byte equality observed |
| test_bench_random.test | 1 | 0.323188s | 0.323171s | 0.323111s | native test oracle; output varies |
| test_boost_unordered.test | 10 | 0.001391s | 0.001426s | 0.001386s | byte equality observed |
| test_comparison.test | 10 | 0.000829s | 0.000860s | 0.000864s | byte equality observed |
| test_comparison_no_simd.test | 10 | 0.000838s | 0.000858s | 0.000849s | byte equality observed |
| test_constants.test | 10 | 0.000831s | 0.000837s | 0.000841s | byte equality observed |
| test_constants_cx.test | 10 | 0.000787s | 0.000792s | 0.000817s | byte equality observed |
| test_data.test | 10 | 0.000824s | 0.000833s | 0.000830s | byte equality observed |
| test_detail_chacha20.test | 10 | 0.000837s | 0.000828s | 0.000847s | byte equality observed |
| test_detail_endian.test | 10 | 0.000833s | 0.000833s | 0.000845s | byte equality observed |
| test_detail_md5.test | 10 | 0.000847s | 0.000858s | 0.000855s | byte equality observed |
| test_detail_random_provider.test | 8 | 0.002416s | 0.002430s | 0.002407s | byte equality observed |
| test_detail_sha1.test | 4 | 0.004696s | 0.005313s | 0.003972s | byte equality observed |
| test_entropy_error.test | 10 | 0.000858s | 0.000849s | 0.000857s | byte equality observed |
| test_from_chars.test | 10 | 0.000842s | 0.000854s | 0.000827s | byte equality observed |
| test_from_chars_2.test | 10 | 0.000845s | 0.000851s | 0.000815s | byte equality observed |
| test_from_chars_2_no_simd.test | 10 | 0.000837s | 0.000849s | 0.000826s | byte equality observed |
| test_from_chars_cx.test | 10 | 0.000832s | 0.000835s | 0.000826s | byte equality observed |
| test_from_chars_cx2.test | 10 | 0.000832s | 0.000845s | 0.000821s | byte equality observed |
| test_from_chars_no_simd.test | 10 | 0.000836s | 0.000839s | 0.000838s | byte equality observed |
| test_hash.test | 10 | 0.001388s | 0.001405s | 0.001396s | byte equality observed |
| test_hash_value.test | 8 | 0.002734s | 0.002733s | 0.002695s | byte equality observed |
| test_hash_value_cx.test | 10 | 0.000841s | 0.000857s | 0.000840s | byte equality observed |
| test_include1.test | 10 | 0.000384s | 0.000390s | 0.000818s | byte equality observed |
| test_io.test | 10 | 0.000896s | 0.000921s | 0.000891s | byte equality observed |
| test_io_2.test | 10 | 0.000919s | 0.000958s | 0.000940s | byte equality observed |
| test_io_2_no_simd.test | 10 | 0.000949s | 0.000942s | 0.000951s | byte equality observed |
| test_io_no_simd.test | 10 | 0.000909s | 0.000943s | 0.000901s | byte equality observed |
| test_msvc_simd_bug981648.test | 10 | 0.000858s | 0.000854s | 0.000859s | native test oracle; output varies |
| test_name_generator.test | 10 | 0.000883s | 0.000889s | 0.000872s | byte equality observed |
| test_name_generator_md5.test | 10 | 0.000890s | 0.000881s | 0.000869s | byte equality observed |
| test_name_generator_sha1.test | 10 | 0.000877s | 0.000902s | 0.000872s | byte equality observed |
| test_namespaces.test | 10 | 0.000869s | 0.000874s | 0.000885s | byte equality observed |
| test_nil_generator.test | 10 | 0.000854s | 0.000859s | 0.000857s | byte equality observed |
| test_pragma_pack.test | 10 | 0.000847s | 0.000857s | 0.000862s | byte equality observed |
| test_random_generator.test | 1 | 0.061431s | 0.061424s | 0.061442s | native test oracle; output varies |
| test_random_generator_2.test | 10 | 0.000838s | 0.000840s | 0.000861s | byte equality observed |
| test_serialization.test | 10 | 0.000908s | 0.000901s | 0.000914s | byte equality observed |
| test_std_unordered.test | 10 | 0.001422s | 0.001420s | 0.001413s | byte equality observed |
| test_string_generator.test | 10 | 0.000894s | 0.000897s | 0.000916s | byte equality observed |
| test_string_generator_2.test | 10 | 0.000867s | 0.000855s | 0.000866s | byte equality observed |
| test_string_generator_cx.test | 10 | 0.000851s | 0.000867s | 0.000847s | byte equality observed |
| test_string_generator_cx2.test | 10 | 0.000855s | 0.000869s | 0.000859s | byte equality observed |
| test_tagging.test | 10 | 0.000885s | 0.000889s | 0.000865s | byte equality observed |
| test_time_generator.test | 10 | 0.000871s | 0.000900s | 0.000876s | byte equality observed |
| test_time_generator_v1.test | 10 | 0.001146s | 0.001246s | 0.001249s | byte equality observed |
| test_time_generator_v1_2.test | 10 | 0.001031s | 0.001067s | 0.001079s | byte equality observed |
| test_time_generator_v1_3.test | 7 | 0.003075s | 0.002880s | 0.002898s | byte equality observed |
| test_time_generator_v6.test | 10 | 0.001171s | 0.001253s | 0.001254s | byte equality observed |
| test_time_generator_v6_2.test | 10 | 0.001045s | 0.001112s | 0.001059s | byte equality observed |
| test_time_generator_v6_3.test | 7 | 0.003045s | 0.002927s | 0.002899s | byte equality observed |
| test_time_generator_v7.test | 10 | 0.001043s | 0.001090s | 0.001064s | byte equality observed |
| test_time_generator_v7_2.test | 10 | 0.001144s | 0.001067s | 0.001150s | byte equality observed |
| test_time_generator_v7_3.test | 1 | 0.030825s | 0.030394s | 0.031158s | byte equality observed |
| test_to_chars.test | 10 | 0.000841s | 0.000869s | 0.000843s | byte equality observed |
| test_to_chars_2.test | 10 | 0.000823s | 0.000856s | 0.000829s | byte equality observed |
| test_to_chars_2_no_simd.test | 10 | 0.000874s | 0.000888s | 0.000873s | byte equality observed |
| test_to_chars_3.test | 10 | 0.000860s | 0.000890s | 0.000861s | byte equality observed |
| test_to_chars_cx.test | 10 | 0.000864s | 0.000875s | 0.000860s | byte equality observed |
| test_to_chars_no_simd.test | 10 | 0.000907s | 0.000900s | 0.000897s | byte equality observed |
| test_uuid.test | 10 | 0.000851s | 0.000853s | 0.000851s | byte equality observed |
| test_uuid_2.test | 10 | 0.000856s | 0.000862s | 0.000838s | byte equality observed |
| test_uuid_3.test | 10 | 0.000864s | 0.000858s | 0.000840s | byte equality observed |
| test_uuid_class.test | 10 | 0.000903s | 0.000905s | 0.000890s | byte equality observed |
| test_uuid_clock.test | 10 | 0.000850s | 0.000876s | 0.000852s | byte equality observed |
| test_uuid_cx2.test | 10 | 0.000862s | 0.000842s | 0.000866s | byte equality observed |
| test_uuid_cx3.test | 10 | 0.000839s | 0.000862s | 0.000836s | byte equality observed |
| test_uuid_cx3_no_simd.test | 10 | 0.000847s | 0.000877s | 0.000831s | byte equality observed |
| test_uuid_from_string.test | 10 | 0.000871s | 0.000862s | 0.000881s | byte equality observed |
| test_uuid_from_string_2.test | 10 | 0.001009s | 0.000992s | 0.001004s | byte equality observed |
| test_uuid_from_string_3.test | 10 | 0.000914s | 0.000914s | 0.000929s | byte equality observed |
| test_uuid_from_string_cx.test | 10 | 0.000866s | 0.000857s | 0.000860s | byte equality observed |
| test_uuid_in_map.test | 10 | 0.000859s | 0.000858s | 0.000862s | byte equality observed |
| test_uuid_no_simd.test | 10 | 0.000851s | 0.000864s | 0.000853s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409407).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
