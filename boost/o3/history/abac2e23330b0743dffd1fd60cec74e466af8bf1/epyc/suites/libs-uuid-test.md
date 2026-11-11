# libs/uuid/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_include1.test | RUN | 0.693176 | 1.382078 | 2.346686 | 0.000819 | 0.000976 | 0.002142 |
| test_uuid.test | RUN | 0.404312 | 1.115242 | 1.610696 | 0.002189 | 0.002454 | 0.002228 |
| test_uuid_no_simd.test | RUN | 0.379563 | 1.079726 | 1.450825 | 0.002364 | 0.002178 | 0.001670 |
| test_uuid_2.test | RUN | 0.289804 | 0.816488 | 1.022572 | 0.002007 | 0.002307 | 0.002216 |
| test_uuid_3.test | RUN | 0.242026 | 0.567970 | 0.875749 | 0.002262 | 0.002443 | 0.001780 |
| test_alignment.test | RUN | 0.227429 | 0.501815 | 0.761384 | 0.002122 | 0.002306 | 0.002284 |
| test_alignment_2.test | RUN | 0.224408 | 0.505051 | 0.791258 | 0.001589 | 0.002110 | 0.001928 |
| test_attribute_packed.test | RUN | 0.239768 | 0.500635 | 0.777353 | 0.002177 | 0.002186 | 0.002128 |
| test_pragma_pack.test | RUN | 0.227987 | 0.508076 | 0.774837 | 0.002192 | 0.002339 | 0.001920 |
| test_data.test | RUN | 0.282132 | 0.898662 | 1.086106 | 0.002161 | 0.002225 | 0.001622 |
| test_comparison.test | RUN | 0.390220 | 1.072556 | 1.521386 | 0.002036 | 0.002440 | 0.001849 |
| test_comparison_no_simd.test | RUN | 0.343129 | 1.244067 | 1.308314 | 0.002212 | 0.002266 | 0.002349 |
| test_io.test | RUN | 0.456043 | 1.929171 | 2.141643 | 0.002178 | 0.002416 | 0.002306 |
| test_io_no_simd.test | RUN | 0.380630 | 1.912241 | 2.048497 | 0.002286 | 0.001988 | 0.002413 |
| test_io_2.test | RUN | 0.356660 | 2.011594 | 1.545692 | 0.002330 | 0.002268 | 0.002248 |
| test_io_2_no_simd.test | RUN | 0.309882 | 1.910616 | 1.309644 | 0.002059 | 0.002112 | 0.002324 |
| test_to_chars.test | RUN | 0.322541 | 1.256748 | 1.378271 | 0.002172 | 0.002186 | 0.002276 |
| test_to_chars_no_simd.test | RUN | 0.318413 | 1.304276 | 1.320496 | 0.002092 | 0.002246 | 0.002148 |
| test_to_chars_2.test | RUN | 0.417158 | 2.341225 | 1.667848 | 0.002118 | 0.002156 | 0.002206 |
| test_to_chars_2_no_simd.test | RUN | 0.393990 | 2.423115 | 1.502192 | 0.001931 | 0.002199 | 0.002296 |
| test_to_chars_3.test | RUN | 0.275032 | 0.842139 | 0.990601 | 0.002192 | 0.002186 | 0.002212 |
| test_from_chars.test | RUN | 0.312498 | 1.217772 | 1.108761 | 0.001986 | 0.001944 | 0.002232 |
| test_from_chars_no_simd.test | RUN | 0.274753 | 0.956617 | 0.987224 | 0.002286 | 0.001940 | 0.002138 |
| test_from_chars_2.test | RUN | 0.300553 | 1.158476 | 1.179029 | 0.002134 | 0.002344 | 0.001970 |
| test_from_chars_2_no_simd.test | RUN | 0.260579 | 0.852733 | 0.881704 | 0.002137 | 0.002098 | 0.002284 |
| test_uuid_from_string.test | RUN | 0.339507 | 1.254356 | 1.407351 | 0.001954 | 0.001899 | 0.001930 |
| test_uuid_from_string_2.test | RUN | 0.295496 | 1.119868 | 1.144771 | 0.002539 | 0.002456 | 0.002418 |
| test_uuid_from_string_3.test | RUN | 0.301368 | 1.165853 | 1.203906 | 0.002304 | 0.002354 | 0.002482 |
| test_uuid_clock.test | RUN | 0.201459 | 0.512480 | 0.731481 | 0.002085 | 0.001965 | 0.001886 |
| test_nil_generator.test | RUN | 0.242146 | 0.544808 | 0.854693 | 0.002252 | 0.002468 | 0.002280 |
| test_string_generator.test | RUN | 0.278343 | 0.927093 | 1.185014 | 0.002374 | 0.002547 | 0.002078 |
| test_string_generator_2.test | RUN | 0.325954 | 1.280758 | 1.308882 | 0.002236 | 0.002282 | 0.002300 |
| test_random_generator.test | RUN | 0.497582 | 2.472067 | 1.940303 | 0.003820 | 0.003642 | 0.003782 |
| test_random_generator_2.test | RUN | 0.354014 | 0.969315 | 1.398330 | 0.002028 | 0.002328 | 0.002086 |
| test_name_generator.test | RUN | 0.324748 | 3.293931 | 1.450740 | 0.002046 | 0.002266 | 0.001943 |
| test_namespaces.test | RUN | 0.253541 | 0.678530 | 0.951706 | 0.001877 | 0.002160 | 0.002038 |
| test_name_generator_md5.test | RUN | 0.322845 | 1.503831 | 1.440044 | 0.002034 | 0.002342 | 0.002301 |
| test_name_generator_sha1.test | RUN | 0.338153 | 4.779942 | 1.342000 | 0.002119 | 0.002146 | 0.002228 |
| test_time_generator.test | RUN | 0.359518 | 0.949452 | 1.382614 | 0.002272 | 0.002310 | 0.001648 |
| test_time_generator_v1.test | RUN | 0.402601 | 1.050112 | 1.441366 | 0.002784 | 0.002887 | 0.003099 |
| test_time_generator_v1_2.test | RUN | 0.390037 | 0.993688 | 1.503655 | 0.002490 | 0.002758 | 0.002600 |
| test_time_generator_v1_3.test | RUN | 0.416286 | 1.029104 | 1.599835 | 0.005348 | 0.005533 | 0.005551 |
| test_time_generator_v6.test | RUN | 0.401882 | 1.067446 | 1.464672 | 0.002524 | 0.002959 | 0.002768 |
| test_time_generator_v6_2.test | RUN | 0.395714 | 0.991992 | 1.470624 | 0.002454 | 0.002704 | 0.002559 |
| test_time_generator_v6_3.test | RUN | 0.430732 | 1.045123 | 1.561043 | 0.005637 | 0.006400 | 0.005724 |
| test_time_generator_v7.test | RUN | 0.384490 | 1.058625 | 1.426304 | 0.002385 | 0.002630 | 0.002206 |
| test_time_generator_v7_2.test | RUN | 0.354436 | 0.949343 | 1.384832 | 0.002122 | 0.002412 | 0.002126 |
| test_time_generator_v7_3.test | RUN | 0.365124 | 1.218852 | 1.378095 | 0.003075 | 0.003504 | 0.003328 |
| test_serialization.test | RUN | 0.646225 | 1.789303 | 2.319945 | 0.002329 | 0.002524 | 0.002276 |
| test_tagging.test | RUN | 0.354652 | 1.109612 | 1.380579 | 0.002068 | 0.001918 | 0.002144 |
| test_uuid_class.test | RUN | 0.376380 | 0.993065 | 1.522554 | 0.001914 | 0.001880 | 0.002226 |
| test_uuid_in_map.test | RUN | 0.422752 | 1.175092 | 1.590659 | 0.002044 | 0.002411 | 0.002325 |
| test_hash.test | RUN | 0.421078 | 1.047548 | 1.435891 | 0.002088 | 0.002191 | 0.002208 |
| test_hash_value.test | RUN | 0.295093 | 0.716650 | 1.058825 | 0.006505 | 0.006460 | 0.006297 |
| test_std_unordered.test | RUN | 0.395473 | 1.155730 | 1.458447 | 0.002457 | 0.002524 | 0.001819 |
| test_boost_unordered.test | RUN | 0.531886 | 1.491160 | 1.887826 | 0.002014 | 0.002412 | 0.002238 |
| test_detail_md5.test | RUN | 0.266965 | 0.761358 | 1.012589 | 0.001840 | 0.002402 | 0.002100 |
| test_detail_sha1.test | RUN | 0.245379 | 0.937035 | 0.993147 | 0.012250 | 0.010989 | 0.012285 |
| test_detail_endian.test | RUN | 0.191628 | 0.540350 | 0.705130 | 0.002200 | 0.001796 | 0.002010 |
| test_detail_chacha20.test | RUN | 0.214688 | 0.683014 | 0.713616 | 0.002130 | 0.002152 | 0.002280 |
| random_provider_no_copy_ctor.test | COMPILE_FAIL | 0.185381 | 0.395177 | 0.629743 | — | — | — |
| random_provider_no_copy_assign.test | COMPILE_FAIL | 0.183494 | 0.383852 | 0.655364 | — | — | — |
| test_msvc_simd_bug981648.test | RUN | 0.314458 | 0.747700 | 1.073320 | 0.002040 | 0.002371 | 0.002287 |
| test_bench_random.test | RUN | 0.359080 | 0.987991 | 1.373302 | 0.009744 | 0.009138 | 0.009402 |
| test_entropy_error.test | RUN | 0.181065 | 0.460736 | 0.596056 | 0.002039 | 0.002244 | 0.002348 |
| test_detail_random_provider.test | RUN | 0.341535 | 0.756733 | 1.224737 | 0.001934 | 0.002161 | 0.002062 |
| test_constants.test | RUN | 0.252253 | 0.694616 | 0.973243 | 0.002034 | 0.001836 | 0.002244 |
| test_uuid_cx.test | COMPILE | 0.157008 | 0.330825 | 0.532304 | — | — | — |
| test_uuid_cx2.test | RUN | 0.269991 | 0.813281 | 1.036131 | 0.001937 | 0.002250 | 0.002227 |
| test_uuid_cx3.test | RUN | 0.244731 | 0.612339 | 0.942330 | 0.001826 | 0.001886 | 0.002248 |
| test_uuid_cx3_no_simd.test | RUN | 0.221631 | 0.589822 | 0.821042 | 0.002136 | 0.001847 | 0.002290 |
| test_hash_value_cx.test | RUN | 0.236220 | 0.566899 | 0.847339 | 0.001629 | 0.001927 | 0.002052 |
| test_string_generator_cx.test | RUN | 0.257909 | 0.683916 | 0.936002 | 0.002150 | 0.002257 | 0.002296 |
| test_string_generator_cx2.test | RUN | 0.259417 | 0.834921 | 1.079395 | 0.001930 | 0.002078 | 0.001798 |
| test_constants_cx.test | RUN | 0.228118 | 0.498034 | 0.828143 | 0.001916 | 0.002056 | 0.001802 |
| test_from_chars_cx.test | RUN | 0.282117 | 0.925331 | 1.031950 | 0.001835 | 0.002336 | 0.002272 |
| test_from_chars_cx2.test | RUN | 0.289814 | 1.153001 | 1.146439 | 0.001899 | 0.001614 | 0.002324 |
| test_to_chars_cx.test | RUN | 0.289465 | 1.038090 | 1.216167 | 0.001680 | 0.002122 | 0.001907 |
| test_uuid_from_string_cx.test | RUN | 0.291097 | 1.063657 | 1.199725 | 0.001658 | 0.001742 | 0.001236 |
| quick.test | RUN | 0.334117 | 0.876312 | 1.070525 | 0.001674 | 0.001641 | 0.001666 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.990× | 0.989×, 0.991× | 0.999× |
| clang | 0.991× | 0.984×, 0.998× | 0.996× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| quick.test | 10 | 0.001548s | 0.001560s | 0.001560s | byte equality observed |
| test_alignment.test | 10 | 0.001578s | 0.001520s | 0.001486s | byte equality observed |
| test_alignment_2.test | 10 | 0.001545s | 0.001546s | 0.001519s | byte equality observed |
| test_attribute_packed.test | 10 | 0.001508s | 0.001478s | 0.001482s | byte equality observed |
| test_bench_random.test | 3 | 0.008990s | 0.008942s | 0.008930s | native test oracle; output varies |
| test_boost_unordered.test | 10 | 0.001584s | 0.001553s | 0.001560s | byte equality observed |
| test_comparison.test | 10 | 0.001523s | 0.001536s | 0.001561s | byte equality observed |
| test_comparison_no_simd.test | 10 | 0.001574s | 0.001622s | 0.001553s | byte equality observed |
| test_constants.test | 10 | 0.001556s | 0.001560s | 0.001524s | byte equality observed |
| test_constants_cx.test | 10 | 0.001528s | 0.001552s | 0.001508s | byte equality observed |
| test_data.test | 10 | 0.001553s | 0.001543s | 0.001530s | byte equality observed |
| test_detail_chacha20.test | 10 | 0.001568s | 0.001554s | 0.001578s | byte equality observed |
| test_detail_endian.test | 10 | 0.001572s | 0.001520s | 0.001551s | byte equality observed |
| test_detail_md5.test | 9 | 0.001576s | 0.001606s | 0.001561s | byte equality observed |
| test_detail_random_provider.test | 10 | 0.001545s | 0.001592s | 0.001557s | byte equality observed |
| test_detail_sha1.test | 3 | 0.011770s | 0.010536s | 0.010740s | byte equality observed |
| test_entropy_error.test | 10 | 0.001519s | 0.001501s | 0.001553s | byte equality observed |
| test_from_chars.test | 10 | 0.001512s | 0.001490s | 0.001498s | byte equality observed |
| test_from_chars_2.test | 10 | 0.001525s | 0.001526s | 0.001545s | byte equality observed |
| test_from_chars_2_no_simd.test | 10 | 0.001561s | 0.001554s | 0.001538s | byte equality observed |
| test_from_chars_cx.test | 10 | 0.001546s | 0.001556s | 0.001583s | byte equality observed |
| test_from_chars_cx2.test | 10 | 0.001498s | 0.001574s | 0.001562s | byte equality observed |
| test_from_chars_no_simd.test | 10 | 0.001583s | 0.001586s | 0.001582s | byte equality observed |
| test_hash.test | 10 | 0.001566s | 0.001552s | 0.001549s | byte equality observed |
| test_hash_value.test | 4 | 0.005747s | 0.005408s | 0.005462s | byte equality observed |
| test_hash_value_cx.test | 10 | 0.001497s | 0.001564s | 0.001486s | byte equality observed |
| test_include1.test | 10 | 0.000691s | 0.000697s | 0.001515s | byte equality observed |
| test_io.test | 10 | 0.001661s | 0.001658s | 0.001666s | byte equality observed |
| test_io_2.test | 10 | 0.001760s | 0.001746s | 0.001718s | byte equality observed |
| test_io_2_no_simd.test | 10 | 0.001730s | 0.001762s | 0.001768s | byte equality observed |
| test_io_no_simd.test | 10 | 0.001664s | 0.001657s | 0.001603s | byte equality observed |
| test_msvc_simd_bug981648.test | 10 | 0.001579s | 0.001576s | 0.001566s | native test oracle; output varies |
| test_name_generator.test | 10 | 0.001635s | 0.001569s | 0.001567s | byte equality observed |
| test_name_generator_md5.test | 10 | 0.001632s | 0.001641s | 0.001585s | byte equality observed |
| test_name_generator_sha1.test | 10 | 0.001620s | 0.001634s | 0.001635s | byte equality observed |
| test_namespaces.test | 10 | 0.001556s | 0.001507s | 0.001505s | byte equality observed |
| test_nil_generator.test | 10 | 0.001546s | 0.001558s | 0.001492s | byte equality observed |
| test_pragma_pack.test | 10 | 0.001552s | 0.001511s | 0.001538s | byte equality observed |
| test_random_generator.test | 7 | 0.003070s | 0.003074s | 0.003090s | native test oracle; output varies |
| test_random_generator_2.test | 10 | 0.001558s | 0.001530s | 0.001541s | byte equality observed |
| test_serialization.test | 10 | 0.001682s | 0.001689s | 0.001661s | byte equality observed |
| test_std_unordered.test | 10 | 0.001664s | 0.001651s | 0.001629s | byte equality observed |
| test_string_generator.test | 10 | 0.001726s | 0.001707s | 0.001724s | byte equality observed |
| test_string_generator_2.test | 10 | 0.001611s | 0.001594s | 0.001573s | byte equality observed |
| test_string_generator_cx.test | 9 | 0.001558s | 0.001592s | 0.001527s | byte equality observed |
| test_string_generator_cx2.test | 10 | 0.001590s | 0.001592s | 0.001568s | byte equality observed |
| test_tagging.test | 10 | 0.001605s | 0.001566s | 0.001559s | byte equality observed |
| test_time_generator.test | 9 | 0.001574s | 0.001618s | 0.001552s | byte equality observed |
| test_time_generator_v1.test | 9 | 0.002135s | 0.002200s | 0.002275s | byte equality observed |
| test_time_generator_v1_2.test | 10 | 0.001894s | 0.001907s | 0.001923s | byte equality observed |
| test_time_generator_v1_3.test | 5 | 0.004866s | 0.004771s | 0.004767s | byte equality observed |
| test_time_generator_v6.test | 9 | 0.002156s | 0.002183s | 0.002207s | byte equality observed |
| test_time_generator_v6_2.test | 10 | 0.001882s | 0.001922s | 0.001916s | byte equality observed |
| test_time_generator_v6_3.test | 5 | 0.004802s | 0.004630s | 0.004840s | byte equality observed |
| test_time_generator_v7.test | 10 | 0.001881s | 0.001923s | 0.001947s | byte equality observed |
| test_time_generator_v7_2.test | 10 | 0.001657s | 0.001634s | 0.001624s | byte equality observed |
| test_time_generator_v7_3.test | 8 | 0.002686s | 0.002673s | 0.002629s | byte equality observed |
| test_to_chars.test | 10 | 0.001652s | 0.001594s | 0.001606s | byte equality observed |
| test_to_chars_2.test | 10 | 0.001575s | 0.001589s | 0.001557s | byte equality observed |
| test_to_chars_2_no_simd.test | 10 | 0.001592s | 0.001581s | 0.001619s | byte equality observed |
| test_to_chars_3.test | 10 | 0.001606s | 0.001596s | 0.001550s | byte equality observed |
| test_to_chars_cx.test | 10 | 0.001601s | 0.001565s | 0.001596s | byte equality observed |
| test_to_chars_no_simd.test | 10 | 0.001675s | 0.001619s | 0.001648s | byte equality observed |
| test_uuid.test | 10 | 0.001570s | 0.001572s | 0.001571s | byte equality observed |
| test_uuid_2.test | 10 | 0.001597s | 0.001573s | 0.001584s | byte equality observed |
| test_uuid_3.test | 10 | 0.001566s | 0.001578s | 0.001590s | byte equality observed |
| test_uuid_class.test | 10 | 0.001576s | 0.001560s | 0.001592s | byte equality observed |
| test_uuid_clock.test | 10 | 0.001572s | 0.001562s | 0.001576s | byte equality observed |
| test_uuid_cx2.test | 10 | 0.001578s | 0.001554s | 0.001547s | byte equality observed |
| test_uuid_cx3.test | 10 | 0.001571s | 0.001541s | 0.001558s | byte equality observed |
| test_uuid_cx3_no_simd.test | 10 | 0.001563s | 0.001571s | 0.001564s | byte equality observed |
| test_uuid_from_string.test | 10 | 0.001570s | 0.001588s | 0.001599s | byte equality observed |
| test_uuid_from_string_2.test | 10 | 0.001993s | 0.002003s | 0.002083s | byte equality observed |
| test_uuid_from_string_3.test | 10 | 0.001679s | 0.001678s | 0.001683s | byte equality observed |
| test_uuid_from_string_cx.test | 10 | 0.001574s | 0.001573s | 0.001585s | byte equality observed |
| test_uuid_in_map.test | 10 | 0.001573s | 0.001595s | 0.001543s | byte equality observed |
| test_uuid_no_simd.test | 10 | 0.001536s | 0.001562s | 0.001549s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
