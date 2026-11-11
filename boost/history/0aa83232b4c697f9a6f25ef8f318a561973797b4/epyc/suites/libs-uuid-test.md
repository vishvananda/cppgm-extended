# libs/uuid/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_include1.test | RUN | 0.400659 | 0.876723 | 1.003226 | 0.001766 | 0.000789 | 0.001771 |
| test_uuid.test | RUN | 0.221590 | 0.533704 | 0.577987 | 0.001794 | 0.001784 | 0.001872 |
| test_uuid_no_simd.test | RUN | 0.211403 | 0.483956 | 0.540282 | 0.001634 | 0.001750 | 0.001764 |
| test_uuid_2.test | RUN | 0.144680 | 0.350800 | 0.400334 | 0.001694 | 0.001512 | 0.001750 |
| test_uuid_3.test | RUN | 0.145781 | 0.299609 | 0.370425 | 0.001879 | 0.001844 | 0.002363 |
| test_alignment.test | RUN | 0.132933 | 0.266687 | 0.365889 | 0.001886 | 0.001860 | 0.001971 |
| test_alignment_2.test | RUN | 0.134933 | 0.267396 | 0.401899 | 0.001458 | 0.001654 | 0.001860 |
| test_attribute_packed.test | RUN | 0.129341 | 0.270046 | 0.405646 | 0.001799 | 0.001710 | 0.002048 |
| test_pragma_pack.test | RUN | 0.129800 | 0.260209 | 0.344362 | 0.001794 | 0.001860 | 0.001852 |
| test_data.test | RUN | 0.145213 | 0.352695 | 0.427606 | 0.001644 | 0.001894 | 0.001792 |
| test_comparison.test | RUN | 0.205642 | 0.499223 | 0.576654 | 0.001695 | 0.001580 | 0.001944 |
| test_comparison_no_simd.test | RUN | 0.186383 | 0.454333 | 0.493252 | 0.002088 | 0.001797 | 0.001838 |
| test_io.test | RUN | 0.227202 | 0.724934 | 0.688223 | 0.001862 | 0.002075 | 0.001871 |
| test_io_no_simd.test | RUN | 0.196683 | 0.614132 | 0.604073 | 0.001737 | 0.001953 | 0.001648 |
| test_io_2.test | RUN | 0.174662 | 0.533333 | 0.543021 | 0.001510 | 0.002026 | 0.001738 |
| test_io_2_no_simd.test | RUN | 0.153271 | 0.444533 | 0.476338 | 0.002012 | 0.001920 | 0.001883 |
| test_to_chars.test | RUN | 0.167593 | 0.518249 | 0.508146 | 0.002111 | 0.002276 | 0.002227 |
| test_to_chars_no_simd.test | RUN | 0.153090 | 0.489686 | 0.459160 | 0.001983 | 0.001776 | 0.002010 |
| test_to_chars_2.test | RUN | 0.167615 | 0.604203 | 0.530576 | 0.001670 | 0.001625 | 0.001472 |
| test_to_chars_2_no_simd.test | RUN | 0.152789 | 0.542058 | 0.470648 | 0.001534 | 0.001820 | 0.001615 |
| test_to_chars_3.test | RUN | 0.158602 | 0.460348 | 0.468349 | 0.001839 | 0.001907 | 0.001910 |
| test_from_chars.test | RUN | 0.154530 | 0.445257 | 0.483402 | 0.001720 | 0.001522 | 0.001617 |
| test_from_chars_no_simd.test | RUN | 0.131805 | 0.315476 | 0.399101 | 0.001378 | 0.001641 | 0.001444 |
| test_from_chars_2.test | RUN | 0.155600 | 0.444936 | 0.501264 | 0.001452 | 0.003232 | 0.001940 |
| test_from_chars_2_no_simd.test | RUN | 0.133267 | 0.303671 | 0.367482 | 0.001728 | 0.001660 | 0.001782 |
| test_uuid_from_string.test | RUN | 0.174772 | 0.747871 | 0.591002 | 0.001451 | 0.002110 | 0.001641 |
| test_uuid_from_string_2.test | RUN | 0.155058 | 0.604161 | 0.467052 | 0.002038 | 0.002290 | 0.001925 |
| test_uuid_from_string_3.test | RUN | 0.155025 | 0.629541 | 0.494540 | 0.001529 | 0.002265 | 0.001583 |
| test_uuid_clock.test | RUN | 0.117452 | 0.376042 | 0.313610 | 0.001843 | 0.001692 | 0.001540 |
| test_nil_generator.test | RUN | 0.140411 | 0.315571 | 0.387585 | 0.001827 | 0.002020 | 0.001912 |
| test_string_generator.test | RUN | 0.145613 | 0.436515 | 0.457201 | 0.001796 | 0.002012 | 0.001873 |
| test_string_generator_2.test | RUN | 0.168307 | 0.526154 | 0.508996 | 0.001548 | 0.002030 | 0.001589 |
| test_random_generator.test | RUN | 0.254277 | 0.675371 | 0.693548 | 0.058988 | 0.059687 | 0.052338 |
| test_random_generator_2.test | RUN | 0.198191 | 0.482310 | 0.519192 | 0.001492 | 0.001850 | 0.001395 |
| test_name_generator.test | RUN | 0.155375 | 0.491355 | 0.470160 | 0.001621 | 0.001926 | 0.001621 |
| test_namespaces.test | RUN | 0.149998 | 0.337809 | 0.412297 | 0.001779 | 0.001902 | 0.001459 |
| test_name_generator_md5.test | RUN | 0.158052 | 0.537051 | 0.514302 | 0.001695 | 0.001797 | 0.001908 |
| test_name_generator_sha1.test | RUN | 0.153746 | 0.497830 | 0.498174 | 0.001704 | 0.001889 | 0.001832 |
| test_time_generator.test | RUN | 0.201977 | 0.489296 | 0.530810 | 0.001630 | 0.001787 | 0.002000 |
| test_time_generator_v1.test | RUN | 0.222007 | 0.569899 | 0.610226 | 0.003890 | 0.004016 | 0.004041 |
| test_time_generator_v1_2.test | RUN | 0.216874 | 0.515001 | 0.583593 | 0.002827 | 0.003039 | 0.003019 |
| test_time_generator_v1_3.test | RUN | 0.236691 | 0.607890 | 0.666284 | 0.005978 | 0.009020 | 0.008396 |
| test_time_generator_v6.test | RUN | 0.215739 | 0.544518 | 0.594189 | 0.003678 | 0.002864 | 0.004109 |
| test_time_generator_v6_2.test | RUN | 0.216702 | 0.559096 | 0.591353 | 0.003018 | 0.003055 | 0.002818 |
| test_time_generator_v6_3.test | RUN | 0.241620 | 0.670591 | 0.665805 | 0.008374 | 0.009606 | 0.008299 |
| test_time_generator_v7.test | RUN | 0.207157 | 0.515531 | 0.582166 | 0.002994 | 0.003144 | 0.002907 |
| test_time_generator_v7_2.test | RUN | 0.202470 | 0.472831 | 0.543292 | 0.001778 | 0.002018 | 0.002052 |
| test_time_generator_v7_3.test | RUN | 0.204457 | 0.470152 | 0.548061 | 0.026351 | 0.030597 | 0.030597 |
| test_serialization.test | RUN | 0.366219 | 0.922259 | 0.914032 | 0.002004 | 0.001778 | 0.002032 |
| test_tagging.test | RUN | 0.208415 | 0.486295 | 0.568659 | 0.001559 | 0.001908 | 0.001618 |
| test_uuid_class.test | RUN | 0.214656 | 0.525520 | 0.578890 | 0.001504 | 0.001604 | 0.001561 |
| test_uuid_in_map.test | RUN | 0.231440 | 0.601043 | 0.608601 | 0.001614 | 0.002003 | 0.001821 |
| test_hash.test | RUN | 0.244046 | 0.511988 | 0.594749 | 0.001517 | 0.002042 | 0.001816 |
| test_hash_value.test | RUN | 0.168866 | 0.395882 | 0.446191 | 0.009423 | 0.009219 | 0.009376 |
| test_std_unordered.test | RUN | 0.222549 | 0.547355 | 0.581612 | 0.001685 | 0.002226 | 0.003180 |
| test_boost_unordered.test | RUN | 0.307407 | 0.705993 | 0.734155 | 0.002016 | 0.002424 | 0.001952 |
| test_detail_md5.test | RUN | 0.139731 | 0.386245 | 0.407829 | 0.001759 | 0.001387 | 0.001892 |
| test_detail_sha1.test | RUN | 0.134382 | 0.334244 | 0.363724 | 0.017240 | 0.016978 | 0.017573 |
| test_detail_endian.test | RUN | 0.106804 | 0.299332 | 0.279660 | 0.001565 | 0.001907 | 0.001681 |
| test_detail_chacha20.test | RUN | 0.133668 | 0.249771 | 0.276370 | 0.002471 | 0.001535 | 0.001782 |
| random_provider_no_copy_ctor.test | COMPILE_FAIL | 0.109034 | 0.214929 | 0.245182 | — | — | — |
| random_provider_no_copy_assign.test | COMPILE_FAIL | 0.108853 | 0.212786 | 0.250390 | — | — | — |
| test_msvc_simd_bug981648.test | RUN | 0.191060 | 0.432660 | 0.484618 | 0.001730 | 0.002058 | 0.001702 |
| test_bench_random.test | RUN | 0.200943 | 0.453546 | 0.533943 | 0.313349 | 0.312220 | 0.312326 |
| test_entropy_error.test | RUN | 0.104517 | 0.243657 | 0.279677 | 0.001814 | 0.001725 | 0.001570 |
| test_detail_random_provider.test | RUN | 0.193194 | 0.409208 | 0.445040 | 0.003040 | 0.001706 | 0.002921 |
| test_constants.test | RUN | 0.145539 | 0.354809 | 0.382408 | 0.001420 | 0.001704 | 0.001444 |
| test_uuid_cx.test | COMPILE | 0.093898 | 0.177471 | 0.215915 | — | — | — |
| test_uuid_cx2.test | RUN | 0.141372 | 0.352817 | 0.411292 | 0.001527 | 0.001801 | 0.001209 |
| test_uuid_cx3.test | RUN | 0.140256 | 0.315954 | 0.387887 | 0.001480 | 0.001768 | 0.001412 |
| test_uuid_cx3_no_simd.test | RUN | 0.121477 | 0.287759 | 0.354483 | 0.001365 | 0.001508 | 0.001570 |
| test_hash_value_cx.test | RUN | 0.155248 | 0.303185 | 0.373944 | 0.002114 | 0.001767 | 0.001409 |
| test_string_generator_cx.test | RUN | 0.142874 | 0.340605 | 0.403126 | 0.001704 | 0.001759 | 0.001606 |
| test_string_generator_cx2.test | RUN | 0.143132 | 0.365320 | 0.416239 | 0.001436 | 0.001577 | 0.001622 |
| test_constants_cx.test | RUN | 0.139792 | 0.272079 | 0.356046 | 0.001549 | 0.001750 | 0.001724 |
| test_from_chars_cx.test | RUN | 0.159849 | 0.447684 | 0.452153 | 0.001839 | 0.001890 | 0.001488 |
| test_from_chars_cx2.test | RUN | 0.151265 | 0.462372 | 0.461963 | 0.001826 | 0.002214 | 0.002106 |
| test_to_chars_cx.test | RUN | 0.153221 | 0.522780 | 0.493196 | 0.001366 | 0.001562 | 0.001295 |
| test_uuid_from_string_cx.test | RUN | 0.154934 | 0.498045 | 0.472538 | 0.001510 | 0.001616 | 0.001391 |
| quick.test | RUN | 0.198835 | 0.469608 | 0.526261 | 0.001427 | 0.001383 | 0.001330 |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114108) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114108) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114108).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
