# libs/uuid/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_include1.test | RUN | 0.354263 | 0.666258 | 0.956846 | 0.000664 | 0.000702 | 0.001698 |
| test_uuid.test | RUN | 0.217365 | 0.564827 | 0.680672 | 0.001952 | 0.001575 | 0.001644 |
| test_uuid_no_simd.test | RUN | 0.209472 | 0.528973 | 0.623109 | 0.001765 | 0.001830 | 0.001597 |
| test_uuid_2.test | RUN | 0.157775 | 0.406043 | 0.471640 | 0.001411 | 0.002900 | 0.001337 |
| test_uuid_3.test | RUN | 0.131132 | 0.343050 | 0.374354 | 0.001306 | 0.001590 | 0.001299 |
| test_alignment.test | RUN | 0.124621 | 0.256040 | 0.321497 | 0.001608 | 0.001493 | 0.001642 |
| test_alignment_2.test | RUN | 0.118581 | 0.249714 | 0.334232 | 0.001407 | 0.001297 | 0.001510 |
| test_attribute_packed.test | RUN | 0.125997 | 0.253599 | 0.329790 | 0.001570 | 0.001514 | 0.001401 |
| test_pragma_pack.test | RUN | 0.121012 | 0.247338 | 0.320645 | 0.001518 | 0.001506 | 0.001110 |
| test_data.test | RUN | 0.150652 | 0.451715 | 0.469987 | 0.001268 | 0.001614 | 0.001730 |
| test_comparison.test | RUN | 0.210712 | 0.535006 | 0.627894 | 0.001720 | 0.001666 | 0.001492 |
| test_comparison_no_simd.test | RUN | 0.182627 | 0.597738 | 0.588149 | 0.001455 | 0.001367 | 0.001667 |
| test_io.test | RUN | 0.244501 | 1.025150 | 0.974179 | 0.001881 | 0.001634 | 0.001717 |
| test_io_no_simd.test | RUN | 0.231841 | 1.015224 | 0.942853 | 0.001478 | 0.001533 | 0.001592 |
| test_io_2.test | RUN | 0.198403 | 1.046245 | 0.693082 | 0.001622 | 0.001507 | 0.001642 |
| test_io_2_no_simd.test | RUN | 0.174843 | 0.985653 | 0.645364 | 0.001674 | 0.001762 | 0.001460 |
| test_to_chars.test | RUN | 0.199617 | 0.672005 | 0.654643 | 0.001684 | 0.001703 | 0.001735 |
| test_to_chars_no_simd.test | RUN | 0.180967 | 0.660520 | 0.643755 | 0.001632 | 0.001696 | 0.001711 |
| test_to_chars_2.test | RUN | 0.229223 | 1.203657 | 0.774738 | 0.001476 | 0.001460 | 0.001632 |
| test_to_chars_2_no_simd.test | RUN | 0.220498 | 1.275546 | 0.711894 | 0.001377 | 0.001375 | 0.001682 |
| test_to_chars_3.test | RUN | 0.156439 | 0.433021 | 0.476851 | 0.001320 | 0.001421 | 0.001537 |
| test_from_chars.test | RUN | 0.166825 | 0.619500 | 0.518640 | 0.001500 | 0.001438 | 0.001544 |
| test_from_chars_no_simd.test | RUN | 0.144664 | 0.494623 | 0.446486 | 0.001376 | 0.001490 | 0.001372 |
| test_from_chars_2.test | RUN | 0.157860 | 0.572190 | 0.516884 | 0.001379 | 0.001419 | 0.001602 |
| test_from_chars_2_no_simd.test | RUN | 0.140831 | 0.447969 | 0.418203 | 0.001313 | 0.001624 | 0.001455 |
| test_uuid_from_string.test | RUN | 0.183006 | 0.636528 | 0.604521 | 0.001582 | 0.001499 | 0.001197 |
| test_uuid_from_string_2.test | RUN | 0.161220 | 0.581082 | 0.525482 | 0.001651 | 0.001458 | 0.001535 |
| test_uuid_from_string_3.test | RUN | 0.163819 | 0.575596 | 0.541904 | 0.001479 | 0.001527 | 0.001710 |
| test_uuid_clock.test | RUN | 0.109194 | 0.243736 | 0.329861 | 0.001319 | 0.001612 | 0.001666 |
| test_nil_generator.test | RUN | 0.128137 | 0.280338 | 0.367080 | 0.001460 | 0.001366 | 0.001689 |
| test_string_generator.test | RUN | 0.147677 | 0.465417 | 0.525916 | 0.001495 | 0.001790 | 0.001576 |
| test_string_generator_2.test | RUN | 0.167174 | 0.660335 | 0.581880 | 0.001391 | 0.001466 | 0.001611 |
| test_random_generator.test | RUN | 0.269550 | 1.246582 | 0.887597 | 0.059047 | 0.059232 | 0.059372 |
| test_random_generator_2.test | RUN | 0.187017 | 0.484241 | 0.590201 | 0.001222 | 0.001606 | 0.001391 |
| test_name_generator.test | RUN | 0.176569 | 1.583482 | 0.645395 | 0.001324 | 0.001720 | 0.001519 |
| test_namespaces.test | RUN | 0.136454 | 0.360266 | 0.403329 | 0.001441 | 0.001602 | 0.001568 |
| test_name_generator_md5.test | RUN | 0.173504 | 0.782747 | 0.632455 | 0.001535 | 0.001668 | 0.001716 |
| test_name_generator_sha1.test | RUN | 0.179486 | 2.306215 | 0.628068 | 0.001473 | 0.001689 | 0.001404 |
| test_time_generator.test | RUN | 0.186648 | 0.492712 | 0.587044 | 0.001414 | 0.001683 | 0.001706 |
| test_time_generator_v1.test | RUN | 0.215037 | 0.551950 | 0.632083 | 0.001868 | 0.002087 | 0.001970 |
| test_time_generator_v1_2.test | RUN | 0.210762 | 0.511664 | 0.598741 | 0.001821 | 0.001922 | 0.001856 |
| test_time_generator_v1_3.test | RUN | 0.223436 | 0.533034 | 0.673095 | 0.003634 | 0.003860 | 0.004046 |
| test_time_generator_v6.test | RUN | 0.215893 | 0.542436 | 0.645830 | 0.001802 | 0.002084 | 0.001966 |
| test_time_generator_v6_2.test | RUN | 0.213380 | 0.500905 | 0.603982 | 0.001734 | 0.001868 | 0.001559 |
| test_time_generator_v6_3.test | RUN | 0.227816 | 0.541453 | 0.672327 | 0.003507 | 0.003339 | 0.004468 |
| test_time_generator_v7.test | RUN | 0.210452 | 0.527870 | 0.621468 | 0.001659 | 0.001808 | 0.001504 |
| test_time_generator_v7_2.test | RUN | 0.189699 | 0.461930 | 0.551974 | 0.001286 | 0.001648 | 0.001560 |
| test_time_generator_v7_3.test | RUN | 0.201307 | 0.598703 | 0.575058 | 0.030072 | 0.030333 | 0.030473 |
| test_serialization.test | RUN | 0.358375 | 0.909227 | 1.245327 | 0.001884 | 0.001631 | 0.001888 |
| test_tagging.test | RUN | 0.194252 | 0.558632 | 0.591208 | 0.001543 | 0.001578 | 0.001596 |
| test_uuid_class.test | RUN | 0.205036 | 0.505594 | 0.620013 | 0.001477 | 0.001347 | 0.001533 |
| test_uuid_in_map.test | RUN | 0.224850 | 0.589163 | 0.675316 | 0.001664 | 0.001589 | 0.001578 |
| test_hash.test | RUN | 0.224412 | 0.518230 | 0.620491 | 0.001616 | 0.001644 | 0.001644 |
| test_hash_value.test | RUN | 0.166086 | 0.369249 | 0.461371 | 0.004153 | 0.003069 | 0.004134 |
| test_std_unordered.test | RUN | 0.217591 | 0.587505 | 0.618284 | 0.001831 | 0.001675 | 0.001710 |
| test_boost_unordered.test | RUN | 0.287906 | 0.771026 | 0.812434 | 0.001515 | 0.001540 | 0.001515 |
| test_detail_md5.test | RUN | 0.141849 | 0.365888 | 0.456152 | 0.001531 | 0.001444 | 0.001686 |
| test_detail_sha1.test | RUN | 0.135792 | 0.466537 | 0.419224 | 0.006313 | 0.006867 | 0.006452 |
| test_detail_endian.test | RUN | 0.106019 | 0.275716 | 0.321156 | 0.001428 | 0.001485 | 0.001544 |
| test_detail_chacha20.test | RUN | 0.112945 | 0.326389 | 0.329541 | 0.001364 | 0.001634 | 0.001425 |
| random_provider_no_copy_ctor.test | COMPILE_FAIL | 0.097134 | 0.186040 | 0.257242 | — | — | — |
| random_provider_no_copy_assign.test | COMPILE_FAIL | 0.099459 | 0.185850 | 0.244881 | — | — | — |
| test_msvc_simd_bug981648.test | RUN | 0.171875 | 0.377070 | 0.434388 | 0.001579 | 0.001711 | 0.001551 |
| test_bench_random.test | RUN | 0.191406 | 0.485330 | 0.602487 | 0.311770 | 0.311882 | 0.311883 |
| test_entropy_error.test | RUN | 0.096657 | 0.217242 | 0.276395 | 0.001510 | 0.001361 | 0.001629 |
| test_detail_random_provider.test | RUN | 0.185820 | 0.383761 | 0.493222 | 0.003078 | 0.003068 | 0.003280 |
| test_constants.test | RUN | 0.136065 | 0.357158 | 0.424186 | 0.001411 | 0.001452 | 0.001404 |
| test_uuid_cx.test | COMPILE | 0.084152 | 0.138285 | 0.216032 | — | — | — |
| test_uuid_cx2.test | RUN | 0.124324 | 0.367864 | 0.455223 | 0.001406 | 0.001616 | 0.001468 |
| test_uuid_cx3.test | RUN | 0.130909 | 0.308321 | 0.412016 | 0.001534 | 0.001385 | 0.001580 |
| test_uuid_cx3_no_simd.test | RUN | 0.121577 | 0.295345 | 0.378211 | 0.001434 | 0.001249 | 0.001335 |
| test_hash_value_cx.test | RUN | 0.130652 | 0.283672 | 0.365280 | 0.001635 | 0.001522 | 0.000974 |
| test_string_generator_cx.test | RUN | 0.140215 | 0.355351 | 0.453652 | 0.001385 | 0.001515 | 0.001659 |
| test_string_generator_cx2.test | RUN | 0.147611 | 0.432394 | 0.498356 | 0.001473 | 0.001545 | 0.001200 |
| test_constants_cx.test | RUN | 0.127043 | 0.250458 | 0.352620 | 0.001222 | 0.001326 | 0.001449 |
| test_from_chars_cx.test | RUN | 0.147137 | 0.467043 | 0.470116 | 0.001731 | 0.001628 | 0.001607 |
| test_from_chars_cx2.test | RUN | 0.152323 | 0.572453 | 0.517473 | 0.001370 | 0.001148 | 0.001501 |
| test_to_chars_cx.test | RUN | 0.158180 | 0.531318 | 0.557395 | 0.001245 | 0.001278 | 0.001657 |
| test_uuid_from_string_cx.test | RUN | 0.161108 | 0.540432 | 0.531005 | 0.001203 | 0.001214 | 0.001232 |
| quick.test | RUN | 0.126813 | 0.441093 | 0.539116 | 0.001094 | 0.001120 | 0.001084 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.940×† | 1.002×, 0.879× | 0.993× |
| clang | 1.061×† | 1.007×, 1.114× | 0.812× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| quick.test | 10 | 0.001007s | 0.000964s | 0.001120s | byte equality observed |
| test_alignment.test | 10 | 0.001020s | 0.000949s | 0.001134s | byte equality observed |
| test_alignment_2.test | 10 | 0.001025s | 0.000971s | 0.001095s | byte equality observed |
| test_attribute_packed.test | 10 | 0.000965s | 0.000911s | 0.001097s | byte equality observed |
| test_bench_random.test | 1 | 0.323483s | 0.322282s | 0.388559s | native test oracle; output varies |
| test_boost_unordered.test | 10 | 0.001622s | 0.001463s | 0.001621s | byte equality observed |
| test_comparison.test | 10 | 0.001048s | 0.000915s | 0.001096s | byte equality observed |
| test_comparison_no_simd.test | 10 | 0.001030s | 0.000909s | 0.001090s | byte equality observed |
| test_constants.test | 10 | 0.000984s | 0.000895s | 0.001079s | byte equality observed |
| test_constants_cx.test | 10 | 0.000957s | 0.000863s | 0.001033s | byte equality observed |
| test_data.test | 10 | 0.000977s | 0.000899s | 0.001085s | byte equality observed |
| test_detail_chacha20.test | 10 | 0.000982s | 0.000910s | 0.001067s | byte equality observed |
| test_detail_endian.test | 10 | 0.001009s | 0.000908s | 0.001079s | byte equality observed |
| test_detail_md5.test | 10 | 0.001051s | 0.000953s | 0.001112s | byte equality observed |
| test_detail_random_provider.test | 8 | 0.002654s | 0.002507s | 0.002675s | byte equality observed |
| test_detail_sha1.test | 4 | 0.005339s | 0.005828s | 0.005523s | byte equality observed |
| test_entropy_error.test | 10 | 0.001079s | 0.000984s | 0.001111s | byte equality observed |
| test_from_chars.test | 10 | 0.001032s | 0.000983s | 0.001066s | byte equality observed |
| test_from_chars_2.test | 10 | 0.001041s | 0.000951s | 0.001076s | byte equality observed |
| test_from_chars_2_no_simd.test | 10 | 0.001018s | 0.000982s | 0.001065s | byte equality observed |
| test_from_chars_cx.test | 10 | 0.001003s | 0.000974s | 0.001061s | byte equality observed |
| test_from_chars_cx2.test | 10 | 0.001015s | 0.000924s | 0.001071s | byte equality observed |
| test_from_chars_no_simd.test | 10 | 0.001049s | 0.000975s | 0.001080s | byte equality observed |
| test_hash.test | 10 | 0.001562s | 0.001414s | 0.001608s | byte equality observed |
| test_hash_value.test | 7 | 0.003441s | 0.003172s | 0.003403s | byte equality observed |
| test_hash_value_cx.test | 10 | 0.001047s | 0.001032s | 0.001108s | byte equality observed |
| test_include1.test | 10 | 0.000489s | 0.000452s | 0.001070s | byte equality observed |
| test_io.test | 10 | 0.001117s | 0.000999s | 0.001174s | byte equality observed |
| test_io_2.test | 10 | 0.001178s | 0.001105s | 0.001203s | byte equality observed |
| test_io_2_no_simd.test | 10 | 0.001155s | 0.001137s | 0.001181s | byte equality observed |
| test_io_no_simd.test | 10 | 0.001114s | 0.001107s | 0.001142s | byte equality observed |
| test_msvc_simd_bug981648.test | 10 | 0.001063s | 0.001056s | 0.001123s | native test oracle; output varies |
| test_name_generator.test | 10 | 0.001073s | 0.001050s | 0.001115s | byte equality observed |
| test_name_generator_md5.test | 10 | 0.001085s | 0.000994s | 0.001108s | byte equality observed |
| test_name_generator_sha1.test | 10 | 0.001116s | 0.001067s | 0.001127s | byte equality observed |
| test_namespaces.test | 10 | 0.001077s | 0.001005s | 0.001100s | byte equality observed |
| test_nil_generator.test | 10 | 0.001045s | 0.001036s | 0.001093s | byte equality observed |
| test_pragma_pack.test | 10 | 0.001025s | 0.001001s | 0.001068s | byte equality observed |
| test_random_generator.test | 1 | 0.062738s | 0.061504s | 0.129634s | native test oracle; output varies |
| test_random_generator_2.test | 10 | 0.001040s | 0.000994s | 0.001055s | byte equality observed |
| test_serialization.test | 10 | 0.001124s | 0.001104s | 0.001156s | byte equality observed |
| test_std_unordered.test | 10 | 0.001654s | 0.001487s | 0.001943s | byte equality observed |
| test_string_generator.test | 10 | 0.001130s | 0.001100s | 0.001166s | byte equality observed |
| test_string_generator_2.test | 10 | 0.001078s | 0.001067s | 0.001114s | byte equality observed |
| test_string_generator_cx.test | 10 | 0.001026s | 0.001012s | 0.001067s | byte equality observed |
| test_string_generator_cx2.test | 10 | 0.001041s | 0.001054s | 0.001094s | byte equality observed |
| test_tagging.test | 10 | 0.001544s | 0.001085s | 0.002766s | byte equality observed |
| test_time_generator.test | 10 | 0.001312s | 0.001093s | 0.002420s | byte equality observed |
| test_time_generator_v1.test | 10 | 0.001468s | 0.001500s | 0.001681s | byte equality observed |
| test_time_generator_v1_2.test | 10 | 0.001266s | 0.001294s | 0.001419s | byte equality observed |
| test_time_generator_v1_3.test | 7 | 0.003224s | 0.003225s | 0.003204s | byte equality observed |
| test_time_generator_v6.test | 10 | 0.001474s | 0.001520s | 0.001648s | byte equality observed |
| test_time_generator_v6_2.test | 10 | 0.001287s | 0.001288s | 0.001403s | byte equality observed |
| test_time_generator_v6_3.test | 7 | 0.003189s | 0.003188s | 0.003366s | byte equality observed |
| test_time_generator_v7.test | 10 | 0.001559s | 0.001526s | 0.001815s | byte equality observed |
| test_time_generator_v7_2.test | 10 | 0.001540s | 0.001496s | 0.001646s | byte equality observed |
| test_time_generator_v7_3.test | 1 | 0.032635s | 0.032442s | 0.033114s | byte equality observed |
| test_to_chars.test | 10 | 0.001111s | 0.001071s | 0.001161s | byte equality observed |
| test_to_chars_2.test | 10 | 0.001079s | 0.001057s | 0.001140s | byte equality observed |
| test_to_chars_2_no_simd.test | 10 | 0.001113s | 0.001050s | 0.001155s | byte equality observed |
| test_to_chars_3.test | 10 | 0.001089s | 0.001066s | 0.001146s | byte equality observed |
| test_to_chars_cx.test | 10 | 0.001114s | 0.001068s | 0.001162s | byte equality observed |
| test_to_chars_no_simd.test | 10 | 0.001177s | 0.001114s | 0.001198s | byte equality observed |
| test_uuid.test | 10 | 0.001089s | 0.001054s | 0.001105s | byte equality observed |
| test_uuid_2.test | 10 | 0.001098s | 0.001057s | 0.001129s | byte equality observed |
| test_uuid_3.test | 10 | 0.001103s | 0.001060s | 0.001131s | byte equality observed |
| test_uuid_class.test | 10 | 0.001133s | 0.001079s | 0.001172s | byte equality observed |
| test_uuid_clock.test | 10 | 0.001098s | 0.001054s | 0.001118s | byte equality observed |
| test_uuid_cx2.test | 10 | 0.001111s | 0.001046s | 0.001162s | byte equality observed |
| test_uuid_cx3.test | 10 | 0.001085s | 0.001027s | 0.001131s | byte equality observed |
| test_uuid_cx3_no_simd.test | 10 | 0.001099s | 0.001061s | 0.001145s | byte equality observed |
| test_uuid_from_string.test | 10 | 0.001126s | 0.001067s | 0.001130s | byte equality observed |
| test_uuid_from_string_2.test | 10 | 0.001323s | 0.001248s | 0.001380s | byte equality observed |
| test_uuid_from_string_3.test | 10 | 0.001186s | 0.001118s | 0.001210s | byte equality observed |
| test_uuid_from_string_cx.test | 10 | 0.001102s | 0.001060s | 0.001132s | byte equality observed |
| test_uuid_in_map.test | 10 | 0.001130s | 0.001077s | 0.001135s | byte equality observed |
| test_uuid_no_simd.test | 10 | 0.001133s | 0.001074s | 0.001133s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
