# libs/uuid/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_include1.test | RUN | 0.341202 | 0.681317 | 0.876120 | 0.001391 | 0.000637 | 0.001511 |
| test_uuid.test | RUN | 0.188960 | 0.424715 | 0.501508 | 0.001541 | 0.001636 | 0.001439 |
| test_uuid_no_simd.test | RUN | 0.170772 | 0.399748 | 0.481143 | 0.001638 | 0.001196 | 0.001476 |
| test_uuid_2.test | RUN | 0.109743 | 0.303390 | 0.350564 | 0.001602 | 0.001583 | 0.001398 |
| test_uuid_3.test | RUN | 0.122331 | 0.257966 | 0.314258 | 0.001598 | 0.001367 | 0.001447 |
| test_alignment.test | RUN | 0.113105 | 0.228411 | 0.286386 | 0.001421 | 0.001632 | 0.001368 |
| test_alignment_2.test | RUN | 0.115483 | 0.223729 | 0.285656 | 0.001597 | 0.001521 | 0.001484 |
| test_attribute_packed.test | RUN | 0.113910 | 0.226659 | 0.290115 | 0.001452 | 0.001382 | 0.001439 |
| test_pragma_pack.test | RUN | 0.115184 | 0.233931 | 0.297124 | 0.001419 | 0.001473 | 0.001466 |
| test_data.test | RUN | 0.129378 | 0.298238 | 0.356705 | 0.001562 | 0.001600 | 0.001530 |
| test_comparison.test | RUN | 0.181085 | 0.425253 | 0.508394 | 0.001514 | 0.001584 | 0.001496 |
| test_comparison_no_simd.test | RUN | 0.163154 | 0.398715 | 0.445998 | 0.001665 | 0.001667 | 0.001631 |
| test_io.test | RUN | 0.196969 | 0.637346 | 0.624226 | 0.001888 | 0.001810 | 0.001619 |
| test_io_no_simd.test | RUN | 0.172774 | 0.539285 | 0.550431 | 0.001372 | 0.001562 | 0.001486 |
| test_io_2.test | RUN | 0.152696 | 0.460762 | 0.493559 | 0.001501 | 0.001782 | 0.001489 |
| test_io_2_no_simd.test | RUN | 0.136357 | 0.388160 | 0.412601 | 0.001669 | 0.001627 | 0.001662 |
| test_to_chars.test | RUN | 0.145840 | 0.440986 | 0.445267 | 0.001894 | 0.001934 | 0.001771 |
| test_to_chars_no_simd.test | RUN | 0.132647 | 0.393459 | 0.411801 | 0.001732 | 0.001748 | 0.001694 |
| test_to_chars_2.test | RUN | 0.144259 | 0.485172 | 0.485333 | 0.001169 | 0.001298 | 0.001549 |
| test_to_chars_2_no_simd.test | RUN | 0.131156 | 0.438327 | 0.415457 | 0.001357 | 0.001520 | 0.001538 |
| test_to_chars_3.test | RUN | 0.133562 | 0.385397 | 0.395938 | 0.001610 | 0.001619 | 0.001484 |
| test_from_chars.test | RUN | 0.135342 | 0.403438 | 0.424239 | 0.001333 | 0.001398 | 0.001332 |
| test_from_chars_no_simd.test | RUN | 0.113608 | 0.274834 | 0.334139 | 0.001405 | 0.001343 | 0.001510 |
| test_from_chars_2.test | RUN | 0.131732 | 0.383786 | 0.407613 | 0.001302 | 0.001536 | 0.001618 |
| test_from_chars_2_no_simd.test | RUN | 0.116891 | 0.271820 | 0.325573 | 0.001514 | 0.001450 | 0.001556 |
| test_uuid_from_string.test | RUN | 0.154846 | 0.517377 | 0.508933 | 0.001184 | 0.001406 | 0.001410 |
| test_uuid_from_string_2.test | RUN | 0.134313 | 0.408196 | 0.414764 | 0.001560 | 0.001758 | 0.001506 |
| test_uuid_from_string_3.test | RUN | 0.134212 | 0.424772 | 0.435979 | 0.001594 | 0.001486 | 0.001540 |
| test_uuid_clock.test | RUN | 0.106506 | 0.226115 | 0.278730 | 0.001450 | 0.001431 | 0.001324 |
| test_nil_generator.test | RUN | 0.121808 | 0.260952 | 0.334491 | 0.001428 | 0.001542 | 0.001575 |
| test_string_generator.test | RUN | 0.127537 | 0.357575 | 0.393815 | 0.001512 | 0.001744 | 0.001541 |
| test_string_generator_2.test | RUN | 0.145168 | 0.443153 | 0.454309 | 0.001252 | 0.001478 | 0.001488 |
| test_random_generator.test | RUN | 0.225641 | 0.573452 | 0.608585 | 0.059063 | 0.059465 | 0.059329 |
| test_random_generator_2.test | RUN | 0.169848 | 0.383170 | 0.464425 | 0.001160 | 0.001618 | 0.001422 |
| test_name_generator.test | RUN | 0.137186 | 0.411318 | 0.445052 | 0.001551 | 0.001604 | 0.001432 |
| test_namespaces.test | RUN | 0.131332 | 0.295899 | 0.355440 | 0.001407 | 0.001538 | 0.001003 |
| test_name_generator_md5.test | RUN | 0.136186 | 0.442317 | 0.457147 | 0.001308 | 0.001505 | 0.001377 |
| test_name_generator_sha1.test | RUN | 0.132054 | 0.412440 | 0.434245 | 0.001558 | 0.001684 | 0.001627 |
| test_time_generator.test | RUN | 0.172271 | 0.416665 | 0.488764 | 0.001499 | 0.001500 | 0.001526 |
| test_time_generator_v1.test | RUN | 0.198905 | 0.469022 | 0.537898 | 0.003334 | 0.003392 | 0.003600 |
| test_time_generator_v1_2.test | RUN | 0.184251 | 0.449636 | 0.528806 | 0.002568 | 0.002638 | 0.002467 |
| test_time_generator_v1_3.test | RUN | 0.203095 | 0.539019 | 0.584749 | 0.009195 | 0.004763 | 0.008848 |
| test_time_generator_v6.test | RUN | 0.186780 | 0.471323 | 0.547937 | 0.003253 | 0.003423 | 0.003873 |
| test_time_generator_v6_2.test | RUN | 0.189778 | 0.454541 | 0.531911 | 0.002388 | 0.002595 | 0.002619 |
| test_time_generator_v6_3.test | RUN | 0.202301 | 0.539872 | 0.585321 | 0.007196 | 0.005682 | 0.008744 |
| test_time_generator_v7.test | RUN | 0.178756 | 0.456060 | 0.514988 | 0.002457 | 0.002735 | 0.002446 |
| test_time_generator_v7_2.test | RUN | 0.170937 | 0.412805 | 0.489607 | 0.001364 | 0.001689 | 0.001696 |
| test_time_generator_v7_3.test | RUN | 0.177795 | 0.405656 | 0.471179 | 0.029944 | 0.030247 | 0.029868 |
| test_serialization.test | RUN | 0.320209 | 0.799075 | 0.834802 | 0.001661 | 0.001764 | 0.001722 |
| test_tagging.test | RUN | 0.179772 | 0.403098 | 0.513313 | 0.001456 | 0.001460 | 0.001340 |
| test_uuid_class.test | RUN | 0.185822 | 0.429906 | 0.520280 | 0.001385 | 0.001427 | 0.001198 |
| test_uuid_in_map.test | RUN | 0.201453 | 0.495223 | 0.556083 | 0.001379 | 0.001651 | 0.001542 |
| test_hash.test | RUN | 0.203621 | 0.426074 | 0.537666 | 0.001566 | 0.001692 | 0.001604 |
| test_hash_value.test | RUN | 0.143541 | 0.332731 | 0.403792 | 0.008671 | 0.008979 | 0.008526 |
| test_std_unordered.test | RUN | 0.188147 | 0.468941 | 0.517623 | 0.001544 | 0.001824 | 0.001754 |
| test_boost_unordered.test | RUN | 0.257095 | 0.603521 | 0.663959 | 0.001673 | 0.001522 | 0.001700 |
| test_detail_md5.test | RUN | 0.120996 | 0.314302 | 0.362635 | 0.001368 | 0.001225 | 0.001503 |
| test_detail_sha1.test | RUN | 0.116279 | 0.278694 | 0.331335 | 0.016477 | 0.015718 | 0.016015 |
| test_detail_endian.test | RUN | 0.093317 | 0.214017 | 0.266603 | 0.001362 | 0.001662 | 0.001325 |
| test_detail_chacha20.test | RUN | 0.102277 | 0.208433 | 0.271013 | 0.001420 | 0.001646 | 0.001473 |
| random_provider_no_copy_ctor.test | COMPILE_FAIL | 0.094287 | 0.178926 | 0.238043 | — | — | — |
| random_provider_no_copy_assign.test | COMPILE_FAIL | 0.093746 | 0.176941 | 0.240102 | — | — | — |
| test_msvc_simd_bug981648.test | RUN | 0.165972 | 0.372287 | 0.423498 | 0.001450 | 0.001653 | 0.001489 |
| test_bench_random.test | RUN | 0.174931 | 0.380515 | 0.471751 | 0.312423 | 0.311266 | 0.308258 |
| test_entropy_error.test | RUN | 0.087222 | 0.201827 | 0.250801 | 0.001578 | 0.001574 | 0.001209 |
| test_detail_random_provider.test | RUN | 0.164048 | 0.326138 | 0.454144 | 0.003047 | 0.002991 | 0.001293 |
| test_constants.test | RUN | 0.123521 | 0.287187 | 0.314940 | 0.001198 | 0.001354 | 0.001292 |
| test_uuid_cx.test | COMPILE | 0.070584 | 0.152583 | 0.176112 | — | — | — |
| test_uuid_cx2.test | RUN | 0.103047 | 0.299085 | 0.360422 | 0.001500 | 0.001255 | 0.001214 |
| test_uuid_cx3.test | RUN | 0.120344 | 0.266228 | 0.347653 | 0.001321 | 0.001270 | 0.001263 |
| test_uuid_cx3_no_simd.test | RUN | 0.108764 | 0.241795 | 0.308030 | 0.001312 | 0.001374 | 0.001273 |
| test_hash_value_cx.test | RUN | 0.122111 | 0.245719 | 0.329439 | 0.001556 | 0.001368 | 0.001269 |
| test_string_generator_cx.test | RUN | 0.124050 | 0.291775 | 0.355477 | 0.001410 | 0.001361 | 0.001429 |
| test_string_generator_cx2.test | RUN | 0.124788 | 0.313725 | 0.378392 | 0.001234 | 0.001361 | 0.001458 |
| test_constants_cx.test | RUN | 0.118430 | 0.233140 | 0.318816 | 0.001321 | 0.001462 | 0.001087 |
| test_from_chars_cx.test | RUN | 0.134567 | 0.381119 | 0.410123 | 0.001509 | 0.001637 | 0.001325 |
| test_from_chars_cx2.test | RUN | 0.136996 | 0.390749 | 0.418881 | 0.001537 | 0.001629 | 0.001376 |
| test_to_chars_cx.test | RUN | 0.135703 | 0.419277 | 0.438246 | 0.001082 | 0.001223 | 0.001477 |
| test_uuid_from_string_cx.test | RUN | 0.131739 | 0.411098 | 0.425843 | 0.001165 | 0.001180 | 0.001108 |
| quick.test | RUN | 0.140392 | 0.382729 | 0.456304 | 0.001109 | 0.001085 | 0.001052 |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872153) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872153) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872153).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
