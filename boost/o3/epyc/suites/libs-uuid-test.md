# libs/uuid/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_include1.test | RUN | 0.602243 | 1.330049 | 2.411996 | 0.000891 | 0.000853 | 0.002256 |
| test_uuid.test | RUN | 0.363837 | 1.049831 | 1.579239 | 0.002442 | 0.002132 | 0.002196 |
| test_uuid_no_simd.test | RUN | 0.361107 | 1.011358 | 1.430134 | 0.001950 | 0.002044 | 0.001880 |
| test_uuid_2.test | RUN | 0.243131 | 0.776599 | 1.053814 | 0.001780 | 0.001976 | 0.001884 |
| test_uuid_3.test | RUN | 0.228740 | 0.523974 | 0.823718 | 0.002072 | 0.002128 | 0.001436 |
| test_alignment.test | RUN | 0.202304 | 0.503942 | 0.758459 | 0.002106 | 0.001642 | 0.002207 |
| test_alignment_2.test | RUN | 0.220885 | 0.463364 | 0.790342 | 0.001990 | 0.002064 | 0.002012 |
| test_attribute_packed.test | RUN | 0.207344 | 0.493896 | 0.809601 | 0.002618 | 0.002154 | 0.001824 |
| test_pragma_pack.test | RUN | 0.201078 | 0.486105 | 0.789464 | 0.002028 | 0.002262 | 0.002306 |
| test_data.test | RUN | 0.231440 | 0.869860 | 1.096697 | 0.001705 | 0.001879 | 0.002106 |
| test_comparison.test | RUN | 0.361175 | 0.998903 | 1.522239 | 0.002032 | 0.002299 | 0.002150 |
| test_comparison_no_simd.test | RUN | 0.319415 | 1.169277 | 1.281216 | 0.001976 | 0.002080 | 0.002244 |
| test_io.test | RUN | 0.414630 | 1.880997 | 2.121901 | 0.002604 | 0.002160 | 0.002412 |
| test_io_no_simd.test | RUN | 0.395860 | 1.838865 | 2.010163 | 0.001904 | 0.002004 | 0.002389 |
| test_io_2.test | RUN | 0.318066 | 1.942128 | 1.528177 | 0.002016 | 0.001853 | 0.002474 |
| test_io_2_no_simd.test | RUN | 0.283498 | 1.839877 | 1.313872 | 0.002378 | 0.002400 | 0.002280 |
| test_to_chars.test | RUN | 0.292157 | 1.228543 | 1.407045 | 0.002184 | 0.002376 | 0.002065 |
| test_to_chars_no_simd.test | RUN | 0.268937 | 1.255738 | 1.318348 | 0.002345 | 0.001721 | 0.002171 |
| test_to_chars_2.test | RUN | 0.332721 | 2.200421 | 1.605637 | 0.002014 | 0.002038 | 0.001860 |
| test_to_chars_2_no_simd.test | RUN | 0.336473 | 2.345659 | 1.538823 | 0.001891 | 0.001944 | 0.002266 |
| test_to_chars_3.test | RUN | 0.249986 | 0.789060 | 1.011515 | 0.002032 | 0.002229 | 0.001992 |
| test_from_chars.test | RUN | 0.275699 | 1.149731 | 1.202144 | 0.001880 | 0.001887 | 0.002122 |
| test_from_chars_no_simd.test | RUN | 0.244668 | 0.927450 | 0.947037 | 0.001804 | 0.001780 | 0.002112 |
| test_from_chars_2.test | RUN | 0.273024 | 1.112834 | 1.144151 | 0.002060 | 0.002113 | 0.001984 |
| test_from_chars_2_no_simd.test | RUN | 0.236574 | 0.832184 | 0.898923 | 0.001862 | 0.002127 | 0.002006 |
| test_uuid_from_string.test | RUN | 0.313969 | 1.195589 | 1.360853 | 0.002132 | 0.002499 | 0.002219 |
| test_uuid_from_string_2.test | RUN | 0.276876 | 1.091105 | 1.139625 | 0.002691 | 0.002520 | 0.002279 |
| test_uuid_from_string_3.test | RUN | 0.285269 | 1.107877 | 1.203565 | 0.002060 | 0.002270 | 0.002424 |
| test_uuid_clock.test | RUN | 0.183065 | 0.456871 | 0.747492 | 0.001741 | 0.002125 | 0.001956 |
| test_nil_generator.test | RUN | 0.211958 | 0.533701 | 0.888025 | 0.001980 | 0.002278 | 0.002107 |
| test_string_generator.test | RUN | 0.245410 | 0.896136 | 1.155942 | 0.002002 | 0.002304 | 0.002248 |
| test_string_generator_2.test | RUN | 0.286421 | 1.217426 | 1.271221 | 0.001862 | 0.001982 | 0.002180 |
| test_random_generator.test | RUN | 0.428850 | 2.367143 | 1.944710 | 0.003397 | 0.004389 | 0.004065 |
| test_random_generator_2.test | RUN | 0.321036 | 0.903602 | 1.363838 | 0.001852 | 0.002209 | 0.002056 |
| test_name_generator.test | RUN | 0.276334 | 3.075941 | 1.427965 | 0.001701 | 0.002200 | 0.001878 |
| test_namespaces.test | RUN | 0.219727 | 0.652462 | 0.945848 | 0.002043 | 0.002195 | 0.002040 |
| test_name_generator_md5.test | RUN | 0.276124 | 1.441892 | 1.347735 | 0.002118 | 0.002320 | 0.002530 |
| test_name_generator_sha1.test | RUN | 0.271780 | 4.509395 | 1.349854 | 0.002830 | 0.002268 | 0.002361 |
| test_time_generator.test | RUN | 0.321650 | 0.928239 | 1.328007 | 0.001869 | 0.002032 | 0.002346 |
| test_time_generator_v1.test | RUN | 0.346049 | 0.986050 | 1.461917 | 0.002737 | 0.002910 | 0.003078 |
| test_time_generator_v1_2.test | RUN | 0.344763 | 0.937889 | 1.415261 | 0.002638 | 0.002698 | 0.002655 |
| test_time_generator_v1_3.test | RUN | 0.389499 | 1.000552 | 1.582705 | 0.006482 | 0.005465 | 0.005698 |
| test_time_generator_v6.test | RUN | 0.357573 | 0.987696 | 1.500444 | 0.002566 | 0.002667 | 0.002801 |
| test_time_generator_v6_2.test | RUN | 0.346992 | 0.961693 | 1.460905 | 0.001967 | 0.002640 | 0.002616 |
| test_time_generator_v6_3.test | RUN | 0.391698 | 0.993275 | 1.603139 | 0.006203 | 0.005845 | 0.006665 |
| test_time_generator_v7.test | RUN | 0.326173 | 0.995732 | 1.478351 | 0.002156 | 0.002342 | 0.002086 |
| test_time_generator_v7_2.test | RUN | 0.315196 | 0.905103 | 1.310418 | 0.001921 | 0.002104 | 0.002178 |
| test_time_generator_v7_3.test | RUN | 0.313242 | 1.139299 | 1.386036 | 0.002962 | 0.003237 | 0.003208 |
| test_serialization.test | RUN | 0.570139 | 1.689997 | 2.279191 | 0.002310 | 0.001872 | 0.002286 |
| test_tagging.test | RUN | 0.320719 | 1.080490 | 1.325379 | 0.002160 | 0.002076 | 0.002294 |
| test_uuid_class.test | RUN | 0.328981 | 0.957843 | 1.471656 | 0.001936 | 0.002064 | 0.001813 |
| test_uuid_in_map.test | RUN | 0.363522 | 1.114757 | 1.573022 | 0.001976 | 0.002182 | 0.002173 |
| test_hash.test | RUN | 0.369810 | 1.006142 | 1.468882 | 0.001934 | 0.002412 | 0.002134 |
| test_hash_value.test | RUN | 0.285592 | 0.695843 | 1.086662 | 0.006369 | 0.006105 | 0.006023 |
| test_std_unordered.test | RUN | 0.353051 | 1.102264 | 1.418410 | 0.002042 | 0.002053 | 0.001911 |
| test_boost_unordered.test | RUN | 0.481871 | 1.465770 | 1.892256 | 0.002198 | 0.002058 | 0.001961 |
| test_detail_md5.test | RUN | 0.233767 | 0.725297 | 1.043095 | 0.002102 | 0.002588 | 0.001888 |
| test_detail_sha1.test | RUN | 0.206968 | 0.891572 | 0.978106 | 0.012794 | 0.011390 | 0.011500 |
| test_detail_endian.test | RUN | 0.158625 | 0.537832 | 0.733467 | 0.001698 | 0.002034 | 0.001968 |
| test_detail_chacha20.test | RUN | 0.181225 | 0.628662 | 0.754414 | 0.002328 | 0.002072 | 0.001778 |
| random_provider_no_copy_ctor.test | COMPILE_FAIL | 0.172968 | 0.369621 | 0.629765 | — | — | — |
| random_provider_no_copy_assign.test | COMPILE_FAIL | 0.165435 | 0.365821 | 0.635419 | — | — | — |
| test_msvc_simd_bug981648.test | RUN | 0.287842 | 0.725367 | 1.083089 | 0.001940 | 0.002226 | 0.002205 |
| test_bench_random.test | RUN | 0.321717 | 0.921545 | 1.392358 | 0.008917 | 0.008959 | 0.009089 |
| test_entropy_error.test | RUN | 0.157505 | 0.431537 | 0.621143 | 0.001850 | 0.002579 | 0.002224 |
| test_detail_random_provider.test | RUN | 0.301938 | 0.724108 | 1.228572 | 0.001902 | 0.002160 | 0.002198 |
| test_constants.test | RUN | 0.223451 | 0.662284 | 0.931242 | 0.001878 | 0.002675 | 0.002238 |
| test_uuid_cx.test | COMPILE | 0.139442 | 0.331770 | 0.528921 | — | — | — |
| test_uuid_cx2.test | RUN | 0.239969 | 0.763121 | 1.039839 | 0.001844 | 0.001792 | 0.002058 |
| test_uuid_cx3.test | RUN | 0.221704 | 0.601655 | 0.924672 | 0.002090 | 0.001876 | 0.002189 |
| test_uuid_cx3_no_simd.test | RUN | 0.193333 | 0.576509 | 0.801772 | 0.001753 | 0.001829 | 0.001583 |
| test_hash_value_cx.test | RUN | 0.213159 | 0.556099 | 0.828591 | 0.001774 | 0.001654 | 0.002647 |
| test_string_generator_cx.test | RUN | 0.224419 | 0.661808 | 0.959213 | 0.002310 | 0.001895 | 0.002126 |
| test_string_generator_cx2.test | RUN | 0.253543 | 0.820883 | 1.097541 | 0.001998 | 0.002276 | 0.002075 |
| test_constants_cx.test | RUN | 0.201472 | 0.455828 | 0.804435 | 0.001918 | 0.001966 | 0.001879 |
| test_from_chars_cx.test | RUN | 0.261817 | 0.888193 | 1.042599 | 0.001850 | 0.002244 | 0.002354 |
| test_from_chars_cx2.test | RUN | 0.262935 | 1.092928 | 1.147984 | 0.002076 | 0.001818 | 0.001769 |
| test_to_chars_cx.test | RUN | 0.254359 | 1.001050 | 1.181071 | 0.002288 | 0.002576 | 0.001548 |
| test_uuid_from_string_cx.test | RUN | 0.266453 | 0.989475 | 1.202514 | 0.001837 | 0.001504 | 0.002050 |
| quick.test | RUN | 0.297683 | 0.803763 | 1.144928 | 0.001403 | 0.001559 | 0.001648 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.986× | 0.983×, 0.989× | 0.989× |
| clang | 0.999× | 0.996×, 1.003× | 1.006× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898266).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| quick.test | 10 | 0.001531s | 0.001553s | 0.001510s | byte equality observed |
| test_alignment.test | 10 | 0.001478s | 0.001563s | 0.001481s | byte equality observed |
| test_alignment_2.test | 10 | 0.001524s | 0.001525s | 0.001477s | byte equality observed |
| test_attribute_packed.test | 10 | 0.001490s | 0.001482s | 0.001467s | byte equality observed |
| test_bench_random.test | 3 | 0.008520s | 0.008556s | 0.008445s | native test oracle; output varies |
| test_boost_unordered.test | 10 | 0.001544s | 0.001535s | 0.001559s | byte equality observed |
| test_comparison.test | 10 | 0.001539s | 0.001498s | 0.001580s | byte equality observed |
| test_comparison_no_simd.test | 10 | 0.001564s | 0.001544s | 0.001561s | byte equality observed |
| test_constants.test | 10 | 0.001537s | 0.001531s | 0.001536s | byte equality observed |
| test_constants_cx.test | 10 | 0.001518s | 0.001519s | 0.001519s | byte equality observed |
| test_data.test | 10 | 0.001523s | 0.001573s | 0.001505s | byte equality observed |
| test_detail_chacha20.test | 10 | 0.001522s | 0.001526s | 0.001551s | byte equality observed |
| test_detail_endian.test | 10 | 0.001500s | 0.001519s | 0.001500s | byte equality observed |
| test_detail_md5.test | 10 | 0.001536s | 0.001554s | 0.001517s | byte equality observed |
| test_detail_random_provider.test | 10 | 0.001541s | 0.001543s | 0.001519s | byte equality observed |
| test_detail_sha1.test | 3 | 0.011610s | 0.010537s | 0.010902s | byte equality observed |
| test_entropy_error.test | 10 | 0.001500s | 0.001475s | 0.001494s | byte equality observed |
| test_from_chars.test | 10 | 0.001510s | 0.001481s | 0.001520s | byte equality observed |
| test_from_chars_2.test | 10 | 0.001496s | 0.001503s | 0.001499s | byte equality observed |
| test_from_chars_2_no_simd.test | 10 | 0.001531s | 0.001520s | 0.001520s | byte equality observed |
| test_from_chars_cx.test | 10 | 0.001544s | 0.001559s | 0.001551s | byte equality observed |
| test_from_chars_cx2.test | 10 | 0.001554s | 0.001540s | 0.001537s | byte equality observed |
| test_from_chars_no_simd.test | 10 | 0.001540s | 0.001552s | 0.001533s | byte equality observed |
| test_hash.test | 10 | 0.001546s | 0.001543s | 0.001510s | byte equality observed |
| test_hash_value.test | 4 | 0.005731s | 0.005430s | 0.005187s | byte equality observed |
| test_hash_value_cx.test | 10 | 0.001511s | 0.001531s | 0.001517s | byte equality observed |
| test_include1.test | 10 | 0.000672s | 0.000692s | 0.001529s | byte equality observed |
| test_io.test | 10 | 0.001610s | 0.001630s | 0.001631s | byte equality observed |
| test_io_2.test | 10 | 0.001735s | 0.001745s | 0.001718s | byte equality observed |
| test_io_2_no_simd.test | 10 | 0.001748s | 0.001753s | 0.001738s | byte equality observed |
| test_io_no_simd.test | 10 | 0.001641s | 0.001645s | 0.001610s | byte equality observed |
| test_msvc_simd_bug981648.test | 9 | 0.001552s | 0.001563s | 0.001556s | native test oracle; output varies |
| test_name_generator.test | 10 | 0.001566s | 0.001624s | 0.001581s | byte equality observed |
| test_name_generator_md5.test | 8 | 0.001600s | 0.001574s | 0.001571s | byte equality observed |
| test_name_generator_sha1.test | 10 | 0.001602s | 0.001600s | 0.001607s | byte equality observed |
| test_namespaces.test | 7 | 0.001567s | 0.001578s | 0.001542s | byte equality observed |
| test_nil_generator.test | 10 | 0.001518s | 0.001514s | 0.001540s | byte equality observed |
| test_pragma_pack.test | 10 | 0.001522s | 0.001544s | 0.001494s | byte equality observed |
| test_random_generator.test | 7 | 0.002984s | 0.002963s | 0.002919s | native test oracle; output varies |
| test_random_generator_2.test | 10 | 0.001528s | 0.001502s | 0.001535s | byte equality observed |
| test_serialization.test | 10 | 0.001691s | 0.001672s | 0.001679s | byte equality observed |
| test_std_unordered.test | 10 | 0.001617s | 0.001619s | 0.001647s | byte equality observed |
| test_string_generator.test | 8 | 0.001701s | 0.001707s | 0.001675s | byte equality observed |
| test_string_generator_2.test | 8 | 0.001601s | 0.001619s | 0.001542s | byte equality observed |
| test_string_generator_cx.test | 10 | 0.001566s | 0.001545s | 0.001557s | byte equality observed |
| test_string_generator_cx2.test | 10 | 0.001570s | 0.001559s | 0.001533s | byte equality observed |
| test_tagging.test | 10 | 0.001561s | 0.001591s | 0.001520s | byte equality observed |
| test_time_generator.test | 10 | 0.001578s | 0.001558s | 0.001558s | byte equality observed |
| test_time_generator_v1.test | 9 | 0.002119s | 0.002187s | 0.002161s | byte equality observed |
| test_time_generator_v1_2.test | 10 | 0.001891s | 0.001900s | 0.001887s | byte equality observed |
| test_time_generator_v1_3.test | 5 | 0.004714s | 0.004586s | 0.004873s | byte equality observed |
| test_time_generator_v6.test | 8 | 0.002119s | 0.002175s | 0.002249s | byte equality observed |
| test_time_generator_v6_2.test | 10 | 0.001918s | 0.001934s | 0.001932s | byte equality observed |
| test_time_generator_v6_3.test | 4 | 0.004910s | 0.004600s | 0.004653s | byte equality observed |
| test_time_generator_v7.test | 10 | 0.001853s | 0.001910s | 0.001861s | byte equality observed |
| test_time_generator_v7_2.test | 10 | 0.001629s | 0.001645s | 0.001641s | byte equality observed |
| test_time_generator_v7_3.test | 8 | 0.002580s | 0.002626s | 0.002582s | byte equality observed |
| test_to_chars.test | 10 | 0.001664s | 0.001631s | 0.001579s | byte equality observed |
| test_to_chars_2.test | 10 | 0.001577s | 0.001583s | 0.001570s | byte equality observed |
| test_to_chars_2_no_simd.test | 10 | 0.001581s | 0.001543s | 0.001517s | byte equality observed |
| test_to_chars_3.test | 10 | 0.001533s | 0.001560s | 0.001600s | byte equality observed |
| test_to_chars_cx.test | 10 | 0.001541s | 0.001562s | 0.001542s | byte equality observed |
| test_to_chars_no_simd.test | 10 | 0.001658s | 0.001623s | 0.001624s | byte equality observed |
| test_uuid.test | 10 | 0.001539s | 0.001548s | 0.001540s | byte equality observed |
| test_uuid_2.test | 10 | 0.001520s | 0.001511s | 0.001546s | byte equality observed |
| test_uuid_3.test | 10 | 0.001561s | 0.001558s | 0.001534s | byte equality observed |
| test_uuid_class.test | 10 | 0.001573s | 0.001570s | 0.001548s | byte equality observed |
| test_uuid_clock.test | 10 | 0.001540s | 0.001550s | 0.001509s | byte equality observed |
| test_uuid_cx2.test | 10 | 0.001562s | 0.001502s | 0.001547s | byte equality observed |
| test_uuid_cx3.test | 10 | 0.001538s | 0.001569s | 0.001537s | byte equality observed |
| test_uuid_cx3_no_simd.test | 10 | 0.001508s | 0.001560s | 0.001521s | byte equality observed |
| test_uuid_from_string.test | 10 | 0.001559s | 0.001580s | 0.001543s | byte equality observed |
| test_uuid_from_string_2.test | 10 | 0.001950s | 0.001940s | 0.001932s | byte equality observed |
| test_uuid_from_string_3.test | 10 | 0.001685s | 0.001681s | 0.001678s | byte equality observed |
| test_uuid_from_string_cx.test | 10 | 0.001541s | 0.001510s | 0.001504s | byte equality observed |
| test_uuid_in_map.test | 10 | 0.001560s | 0.001551s | 0.001546s | byte equality observed |
| test_uuid_no_simd.test | 10 | 0.001517s | 0.001550s | 0.001538s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898266).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
