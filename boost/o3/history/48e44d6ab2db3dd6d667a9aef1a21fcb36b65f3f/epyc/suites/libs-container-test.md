# libs/container/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| vector_test.test | RUN | 7.418186 | 43.788879 | 39.059707 | 0.007670 | 0.007206 | 0.009947 |
| vector_options_test.test | RUN | 0.475264 | 1.987551 | 1.837122 | 0.002166 | 0.002390 | 0.002504 |
| uses_allocator_test.test | RUN | 0.091168 | 0.173218 | 0.251507 | 0.000947 | 0.000952 | 0.002112 |
| uses_allocator_construction_test.test | RUN | 0.260486 | 0.650745 | 0.794549 | 0.002046 | 0.002308 | 0.002086 |
| unsynchronized_pool_resource_test.test | RUN | 0.358133 | 1.826095 | 1.088047 | 0.002108 | 0.002124 | 0.002102 |
| tree_test.test | RUN | 1.229220 | 6.560385 | 5.803319 | 0.002154 | 0.002142 | 0.002173 |
| throw_exception_test.test | RUN | 0.179179 | 0.421259 | 0.617411 | 0.002075 | 0.002082 | 0.002203 |
| synchronized_pool_resource_test.test | RUN | 0.351201 | 1.919050 | 1.142160 | 0.002070 | 0.002378 | 0.002203 |
| string_view_compat_test.test | RUN | 0.563918 | 3.781220 | 2.210671 | 0.002023 | 0.001897 | 0.002241 |
| string_test.test | RUN | 2.332310 | 13.930002 | 10.720466 | 0.022012 | 0.021956 | 0.022787 |
| string_options_test.test | RUN | 0.392188 | 3.103950 | 1.649773 | 0.002162 | 0.002252 | 0.002106 |
| static_vector_test.test | RUN | 2.596568 | 11.675260 | 10.494947 | 0.002881 | 0.003381 | 0.002858 |
| static_vector_options_test.test | RUN | 0.303304 | 0.808896 | 1.072538 | 0.002291 | 0.002000 | 0.002306 |
| stable_vector_test.test | RUN | 5.534398 | 23.595823 | 19.733173 | 0.037437 | 0.037545 | 0.039167 |
| small_vector_test.test | RUN | 3.045954 | 16.867123 | 14.148241 | 0.004943 | 0.004204 | 0.007441 |
| small_vector_options_test.test | RUN | 0.480739 | 2.013800 | 2.027606 | 0.002152 | 0.002445 | 0.002173 |
| slist_test.test | RUN | 2.726389 | 14.873803 | 12.905319 | 0.033240 | 0.034267 | 0.032451 |
| set_test.test | RUN | 4.150725 | 28.544446 | 23.319659 | 0.019518 | 0.019032 | 0.020067 |
| segtor_test.test | RUN | 5.830241 | 30.430401 | 26.073286 | 0.957028 | 0.908064 | 0.904058 |
| segtor_options_test.test | RUN | 0.330087 | 0.968870 | 1.229090 | 0.002059 | 0.002224 | 0.002076 |
| scoped_allocator_usage_test.test | RUN | 1.092358 | 4.598363 | 4.584018 | 0.002104 | 0.002210 | 0.002126 |
| scoped_allocator_adaptor_test.test | RUN | 0.579789 | 1.374352 | 2.191086 | 0.002026 | 0.002212 | 0.002216 |
| resource_adaptor_test.test | RUN | 0.291774 | 0.771349 | 0.951030 | 0.002000 | 0.002205 | 0.002093 |
| polymorphic_allocator_test.test | RUN | 0.245834 | 0.664636 | 0.755037 | 0.001806 | 0.001992 | 0.002070 |
| pmr_vector_test.test | RUN | 0.141625 | 0.267670 | 0.434225 | 0.000862 | 0.000733 | 0.001644 |
| pmr_string_test.test | RUN | 0.274341 | 0.522890 | 0.821884 | 0.000804 | 0.000901 | 0.001962 |
| pmr_stable_vector_test.test | RUN | 0.263421 | 0.823277 | 1.041132 | 0.001892 | 0.001932 | 0.002026 |
| pmr_small_vector_test.test | RUN | 0.163134 | 0.332153 | 0.531237 | 0.001806 | 0.002184 | 0.001984 |
| pmr_slist_test.test | RUN | 0.213589 | 1.049231 | 1.044486 | 0.001950 | 0.002240 | 0.002109 |
| pmr_set_test.test | RUN | 0.237780 | 0.595761 | 0.748474 | 0.001839 | 0.002389 | 0.002259 |
| pmr_nest_test.test | RUN | 0.207498 | 0.385333 | 0.568511 | 0.002081 | 0.002130 | 0.002126 |
| pmr_map_test.test | RUN | 0.246727 | 0.579675 | 0.853694 | 0.001889 | 0.002203 | 0.002114 |
| pmr_list_test.test | RUN | 0.171131 | 0.357463 | 0.557309 | 0.001778 | 0.002147 | 0.002280 |
| pmr_hub_test.test | RUN | 0.248067 | 0.505736 | 0.721662 | 0.002022 | 0.001938 | 0.001892 |
| pmr_flat_set_test.test | RUN | 0.182309 | 0.399043 | 0.613724 | 0.001782 | 0.002056 | 0.002201 |
| pmr_flat_map_test.test | RUN | 0.229015 | 0.457100 | 0.716517 | 0.002137 | 0.002012 | 0.001791 |
| pmr_devector_test.test | RUN | 0.146654 | 0.395263 | 0.558040 | 0.001778 | 0.001933 | 0.001842 |
| pmr_deque_test.test | RUN | 0.142512 | 0.323921 | 0.532000 | 0.001764 | 0.002296 | 0.001895 |
| pair_test.test | RUN | 0.359049 | 0.798656 | 1.061930 | 0.001943 | 0.002289 | 0.002025 |
| null_iterators_test.test | RUN | 0.600325 | 1.508729 | 2.002436 | 0.001672 | 0.002204 | 0.002006 |
| node_handle_test.test | RUN | 0.244072 | 1.898717 | 0.794413 | 0.001821 | 0.002442 | 0.001948 |
| node_allocator_test.test | RUN | 3.043257 | 18.735164 | 16.700934 | 0.013072 | 0.012975 | 0.014019 |
| new_delete_resource_test.test | RUN | 0.329661 | 1.164430 | 1.067384 | 0.002796 | 0.003241 | 0.002735 |
| nest_test.test | RUN | 0.759610 | 5.166819 | 3.445520 | 0.002073 | 0.002358 | 0.002036 |
| nest_api_test.test | RUN | 1.468950 | 9.730460 | 7.250909 | 0.003544 | 0.003598 | 0.003252 |
| monotonic_buffer_resource_test.test | RUN | 0.354323 | 2.053194 | 1.053522 | 0.002137 | 0.002267 | 0.002006 |
| memory_resource_test.test | RUN | 0.216270 | 0.606366 | 0.669040 | 0.002034 | 0.002598 | 0.002148 |
| map_test.test | RUN | 6.195429 | 31.609231 | 35.042925 | 0.014080 | 0.015300 | 0.014218 |
| list_test.test | RUN | 2.919490 | 16.279923 | 14.358602 | 0.033427 | 0.032967 | 0.034259 |
| is_constructible_test.test | RUN | 0.130963 | 0.236804 | 0.335341 | 0.000875 | 0.000876 | 0.002049 |
| insert_vs_emplace_test.test | RUN | 0.338639 | 1.378819 | 2.057271 | 0.001937 | 0.002363 | 0.002380 |
| hub_uses_allocator_test.test | RUN | 0.415407 | 1.171916 | 1.471462 | 0.002002 | 0.002088 | 0.002148 |
| hub_stability_test.test | RUN | 0.735786 | 5.269857 | 4.965853 | 0.002370 | 0.002734 | 0.002444 |
| hub_sort_test.test | RUN | 0.465307 | 2.344095 | 2.466053 | 0.673702 | 0.638952 | 0.627993 |
| hub_pocxx_test.test | RUN | 0.472614 | 2.816392 | 1.771901 | 0.001938 | 0.002270 | 0.001938 |
| hub_new_extended_alignment_test.test | RUN | 0.282799 | 0.851929 | 1.050572 | 0.001946 | 0.002290 | 0.001757 |
| hub_exception_safety_test.test | RUN | 1.028613 | 6.463417 | 4.712127 | 0.110048 | 0.121579 | 0.101157 |
| hub_empty_sort_test.test | RUN | 0.320729 | 0.813809 | 1.236002 | 0.001990 | 0.002292 | 0.002168 |
| hub_api_test.test | RUN | 2.833087 | 26.697998 | 16.046944 | 0.005848 | 0.004610 | 0.004526 |
| global_resource_test.test | RUN | 0.190286 | 0.553287 | 0.675742 | 0.002038 | 0.002376 | 0.002324 |
| flat_tree_test.test | RUN | 4.987835 | 38.753112 | 28.476214 | 0.002019 | 0.001845 | 0.002248 |
| flat_set_test.test | RUN | 6.131418 | 49.153564 | 37.001501 | 0.010755 | 0.010184 | 0.010798 |
| flat_set_adaptor_test.test | RUN | 4.877358 | 43.777295 | 29.671312 | 0.010690 | 0.010179 | 0.010840 |
| flat_map_test.test | RUN | 10.433397 | 72.433998 | 61.303400 | 0.011884 | 0.011824 | 0.011551 |
| flat_map_adaptor_test.test | RUN | 6.645960 | 53.332717 | 43.317551 | 0.010800 | 0.010753 | 0.010659 |
| explicit_inst_vector_test.test | RUN | 1.071172 | 7.114018 | 6.555323 | 0.001911 | 0.001806 | 0.002242 |
| explicit_inst_string_test.test | RUN | 1.429918 | 12.772640 | 9.037913 | 0.001798 | 0.002108 | 0.001603 |
| explicit_inst_static_vector_test.test | RUN | 0.168082 | 0.492367 | 0.731434 | 0.002004 | 0.002175 | 0.002233 |
| explicit_inst_stable_vector_test.test | RUN | 0.936994 | 4.700395 | 4.377623 | 0.002044 | 0.001918 | 0.002109 |
| explicit_inst_small_vector_test.test | RUN | 0.754187 | 4.075929 | 3.938737 | 0.001666 | 0.002152 | 0.002014 |
| explicit_inst_slist_test.test | RUN | 0.610225 | 3.694571 | 3.189626 | 0.002063 | 0.002240 | 0.001784 |
| explicit_inst_set_test.test | RUN | 0.894190 | 4.341825 | 3.723827 | 0.002075 | 0.002184 | 0.002184 |
| explicit_inst_map_test.test | RUN | 0.975659 | 4.531040 | 4.053119 | 0.002038 | 0.002041 | 0.001991 |
| explicit_inst_list_test.test | RUN | 0.228539 | 1.051364 | 1.008125 | 0.001998 | 0.001927 | 0.002064 |
| explicit_inst_flat_set_test.test | RUN | 4.473898 | 32.674140 | 22.418009 | 0.001962 | 0.002145 | 0.002200 |
| explicit_inst_flat_map_test.test | RUN | 5.042098 | 34.282673 | 24.800609 | 0.001974 | 0.002100 | 0.001644 |
| explicit_inst_devector_test.test | RUN | 0.584603 | 4.198260 | 3.309859 | 0.001962 | 0.001947 | 0.002064 |
| explicit_inst_deque_test.test | RUN | 0.681070 | 4.467498 | 3.415609 | 0.002015 | 0.002198 | 0.002075 |
| devector_test.test | RUN | 8.788990 | 48.348150 | 37.483880 | 0.004597 | 0.004048 | 0.005120 |
| devector_options_test.test | RUN | 0.419897 | 2.511485 | 1.866647 | 0.002479 | 0.002448 | 0.002601 |
| deque_test.test | RUN | 6.286064 | 33.400471 | 28.532836 | 0.517645 | 0.429145 | 0.549071 |
| deque_options_test.test | RUN | 0.333689 | 0.912082 | 1.453437 | 0.002110 | 0.002192 | 0.002294 |
| copy_move_algo_test.test | RUN | 0.341862 | 1.208657 | 1.347140 | 0.002224 | 0.002062 | 0.002078 |
| common_iterator_test.test | RUN | 0.027798 | 0.061867 | 0.066776 | 0.000798 | 0.000928 | 0.001982 |
| boost_iterator_comp_test.test | RUN | 0.466635 | 1.406613 | 1.788043 | 0.001893 | 0.002147 | 0.002080 |
| allocator_traits_test.test | RUN | 0.283233 | 0.599968 | 0.868778 | 0.002068 | 0.002038 | 0.002051 |
| alloc_full_test.test | RUN | 0.295388 | 1.265869 | 1.305867 | 0.006312 | 0.006444 | 0.006270 |
| alloc_basic_test.test | RUN | 0.256229 | 0.914079 | 0.961348 | 0.002081 | 0.002166 | 0.002276 |
| adaptive_pool_test.test | RUN | 2.057229 | 19.032365 | 16.137582 | 0.012650 | 0.017030 | 0.020046 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.894× | 0.891×, 0.897× | 1.003× |
| clang | 0.954× | 0.949×, 0.958× | 0.988× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adaptive_pool_test.test | 2 | 0.013369s | 0.012126s | 0.013312s | byte equality observed |
| alloc_basic_test.test | 10 | 0.001315s | 0.001318s | 0.001332s | byte equality observed |
| alloc_full_test.test | 6 | 0.003880s | 0.003849s | 0.004019s | byte equality observed |
| allocator_traits_test.test | 10 | 0.001225s | 0.001238s | 0.001229s | byte equality observed |
| boost_iterator_comp_test.test | 10 | 0.001295s | 0.001275s | 0.001283s | byte equality observed |
| common_iterator_test.test | 10 | 0.000609s | 0.000598s | 0.001263s | byte equality observed |
| copy_move_algo_test.test | 10 | 0.001469s | 0.001403s | 0.001447s | byte equality observed |
| deque_options_test.test | 10 | 0.001337s | 0.001288s | 0.001316s | byte equality observed |
| deque_test.test | 1 | 0.568008s | 0.463724s | 0.587018s | byte equality observed |
| devector_options_test.test | 10 | 0.001472s | 0.001360s | 0.001334s | byte equality observed |
| devector_test.test | 4 | 0.004120s | 0.003636s | 0.004912s | byte equality observed |
| explicit_inst_deque_test.test | 10 | 0.001178s | 0.001184s | 0.001166s | byte equality observed |
| explicit_inst_devector_test.test | 10 | 0.001155s | 0.001201s | 0.001169s | byte equality observed |
| explicit_inst_flat_map_test.test | 10 | 0.001183s | 0.001195s | 0.001155s | byte equality observed |
| explicit_inst_flat_set_test.test | 10 | 0.001165s | 0.001206s | 0.001166s | byte equality observed |
| explicit_inst_list_test.test | 10 | 0.001170s | 0.001173s | 0.001149s | byte equality observed |
| explicit_inst_map_test.test | 10 | 0.001180s | 0.001184s | 0.001190s | byte equality observed |
| explicit_inst_set_test.test | 10 | 0.001172s | 0.001190s | 0.001199s | byte equality observed |
| explicit_inst_slist_test.test | 10 | 0.001173s | 0.001188s | 0.001184s | byte equality observed |
| explicit_inst_small_vector_test.test | 10 | 0.001173s | 0.001188s | 0.001195s | byte equality observed |
| explicit_inst_stable_vector_test.test | 10 | 0.001173s | 0.001183s | 0.001188s | byte equality observed |
| explicit_inst_static_vector_test.test | 10 | 0.001164s | 0.001160s | 0.001166s | byte equality observed |
| explicit_inst_string_test.test | 10 | 0.001171s | 0.001173s | 0.001177s | byte equality observed |
| explicit_inst_vector_test.test | 10 | 0.001183s | 0.001178s | 0.001181s | byte equality observed |
| flat_map_adaptor_test.test | 3 | 0.007133s | 0.006834s | 0.006909s | byte equality observed |
| flat_map_test.test | 3 | 0.007737s | 0.007310s | 0.007183s | byte equality observed |
| flat_set_adaptor_test.test | 3 | 0.007210s | 0.006520s | 0.007121s | byte equality observed |
| flat_set_test.test | 3 | 0.007254s | 0.006295s | 0.006999s | byte equality observed |
| flat_tree_test.test | 10 | 0.001191s | 0.001188s | 0.001180s | byte equality observed |
| global_resource_test.test | 10 | 0.001218s | 0.001220s | 0.001217s | byte equality observed |
| hub_api_test.test | 5 | 0.004094s | 0.002774s | 0.002762s | byte equality observed |
| hub_empty_sort_test.test | 10 | 0.001189s | 0.001203s | 0.001190s | byte equality observed |
| hub_exception_safety_test.test | 1 | 0.074405s | 0.080086s | 0.074331s | byte equality observed |
| hub_new_extended_alignment_test.test | 10 | 0.001241s | 0.001322s | 0.001247s | byte equality observed |
| hub_pocxx_test.test | 10 | 0.001218s | 0.001279s | 0.001222s | byte equality observed |
| hub_sort_test.test | 1 | 0.548029s | 0.504366s | 0.497563s | byte equality observed |
| hub_stability_test.test | 10 | 0.001534s | 0.001471s | 0.001468s | byte equality observed |
| hub_uses_allocator_test.test | 10 | 0.001203s | 0.001197s | 0.001186s | byte equality observed |
| insert_vs_emplace_test.test | 10 | 0.001232s | 0.001240s | 0.001219s | byte equality observed |
| is_constructible_test.test | 10 | 0.000575s | 0.000578s | 0.001196s | byte equality observed |
| list_test.test | 1 | 0.021624s | 0.021053s | 0.021426s | byte equality observed |
| map_test.test | 2 | 0.009414s | 0.009510s | 0.009325s | byte equality observed |
| memory_resource_test.test | 10 | 0.001215s | 0.001199s | 0.001184s | byte equality observed |
| monotonic_buffer_resource_test.test | 10 | 0.001231s | 0.001218s | 0.001201s | byte equality observed |
| nest_api_test.test | 8 | 0.002320s | 0.002019s | 0.002039s | byte equality observed |
| nest_test.test | 10 | 0.001253s | 0.001238s | 0.001214s | byte equality observed |
| new_delete_resource_test.test | 10 | 0.001765s | 0.001740s | 0.001721s | byte equality observed |
| node_allocator_test.test | 3 | 0.008694s | 0.008488s | 0.008765s | byte equality observed |
| node_handle_test.test | 10 | 0.001203s | 0.001209s | 0.001185s | byte equality observed |
| null_iterators_test.test | 10 | 0.001213s | 0.001192s | 0.001188s | byte equality observed |
| pair_test.test | 10 | 0.001228s | 0.001190s | 0.001149s | byte equality observed |
| pmr_deque_test.test | 10 | 0.001197s | 0.001175s | 0.001150s | byte equality observed |
| pmr_devector_test.test | 10 | 0.001180s | 0.001191s | 0.001162s | byte equality observed |
| pmr_flat_map_test.test | 10 | 0.001174s | 0.001181s | 0.001164s | byte equality observed |
| pmr_flat_set_test.test | 10 | 0.001182s | 0.001176s | 0.001149s | byte equality observed |
| pmr_hub_test.test | 10 | 0.001173s | 0.001185s | 0.001159s | byte equality observed |
| pmr_list_test.test | 10 | 0.001175s | 0.001193s | 0.001172s | byte equality observed |
| pmr_map_test.test | 10 | 0.001180s | 0.001181s | 0.001167s | byte equality observed |
| pmr_nest_test.test | 10 | 0.001201s | 0.001218s | 0.001184s | byte equality observed |
| pmr_set_test.test | 10 | 0.001190s | 0.001179s | 0.001239s | byte equality observed |
| pmr_slist_test.test | 10 | 0.001172s | 0.001197s | 0.001213s | byte equality observed |
| pmr_small_vector_test.test | 10 | 0.001178s | 0.001186s | 0.001257s | byte equality observed |
| pmr_stable_vector_test.test | 10 | 0.001195s | 0.001187s | 0.001291s | byte equality observed |
| pmr_string_test.test | 10 | 0.000579s | 0.000582s | 0.001233s | byte equality observed |
| pmr_vector_test.test | 10 | 0.000583s | 0.000568s | 0.001220s | byte equality observed |
| polymorphic_allocator_test.test | 10 | 0.001307s | 0.001231s | 0.001258s | byte equality observed |
| resource_adaptor_test.test | 10 | 0.001282s | 0.001301s | 0.001285s | byte equality observed |
| scoped_allocator_adaptor_test.test | 10 | 0.001259s | 0.001304s | 0.001365s | byte equality observed |
| scoped_allocator_usage_test.test | 10 | 0.001345s | 0.001286s | 0.001252s | byte equality observed |
| segtor_options_test.test | 10 | 0.001251s | 0.001285s | 0.001297s | byte equality observed |
| segtor_test.test | 1 | 0.512862s | 0.461630s | 0.442592s | byte equality observed |
| set_test.test | 2 | 0.013265s | 0.011784s | 0.013208s | byte equality observed |
| slist_test.test | 1 | 0.020762s | 0.020298s | 0.020618s | byte equality observed |
| small_vector_options_test.test | 10 | 0.001333s | 0.001258s | 0.001324s | byte equality observed |
| small_vector_test.test | 5 | 0.002772s | 0.002249s | 0.004170s | byte equality observed |
| stable_vector_test.test | 1 | 0.029627s | 0.029526s | 0.030910s | byte equality observed |
| static_vector_options_test.test | 10 | 0.001283s | 0.001205s | 0.001265s | byte equality observed |
| static_vector_test.test | 10 | 0.001677s | 0.001480s | 0.001570s | byte equality observed |
| string_options_test.test | 10 | 0.001264s | 0.001235s | 0.001247s | byte equality observed |
| string_test.test | 2 | 0.014500s | 0.015307s | 0.014936s | byte equality observed |
| string_view_compat_test.test | 10 | 0.001211s | 0.001168s | 0.001184s | byte equality observed |
| synchronized_pool_resource_test.test | 10 | 0.001245s | 0.001224s | 0.001281s | byte equality observed |
| throw_exception_test.test | 10 | 0.001163s | 0.001147s | 0.001196s | byte equality observed |
| tree_test.test | 10 | 0.001172s | 0.001140s | 0.001178s | byte equality observed |
| unsynchronized_pool_resource_test.test | 10 | 0.001233s | 0.001235s | 0.001266s | byte equality observed |
| uses_allocator_construction_test.test | 10 | 0.001163s | 0.001149s | 0.001188s | byte equality observed |
| uses_allocator_test.test | 10 | 0.000544s | 0.000543s | 0.001178s | byte equality observed |
| vector_options_test.test | 10 | 0.001352s | 0.001317s | 0.001352s | byte equality observed |
| vector_test.test | 4 | 0.004381s | 0.003894s | 0.005637s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
