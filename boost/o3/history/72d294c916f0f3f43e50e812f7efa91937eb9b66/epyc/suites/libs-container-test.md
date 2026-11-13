# libs/container/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| vector_test.test | RUN | 8.774107 | 41.549177 | 38.312583 | 0.006643 | 0.006728 | 0.009545 |
| vector_options_test.test | RUN | 0.432701 | 1.965380 | 1.751274 | 0.002329 | 0.002239 | 0.002296 |
| uses_allocator_test.test | RUN | 0.077764 | 0.168588 | 0.249225 | 0.000834 | 0.000712 | 0.002024 |
| uses_allocator_construction_test.test | RUN | 0.240036 | 0.629094 | 0.873436 | 0.001983 | 0.002038 | 0.001928 |
| unsynchronized_pool_resource_test.test | RUN | 0.300630 | 1.810189 | 1.141994 | 0.002010 | 0.002108 | 0.002220 |
| tree_test.test | RUN | 1.187423 | 6.261076 | 5.631078 | 0.001583 | 0.001928 | 0.001926 |
| throw_exception_test.test | RUN | 0.166489 | 0.396411 | 0.598477 | 0.001610 | 0.002060 | 0.002057 |
| synchronized_pool_resource_test.test | RUN | 0.296787 | 1.814298 | 1.137431 | 0.001830 | 0.002064 | 0.002130 |
| string_view_compat_test.test | RUN | 0.496358 | 3.604965 | 2.147935 | 0.001813 | 0.002040 | 0.002036 |
| string_test.test | RUN | 2.019577 | 13.611977 | 10.476032 | 0.024525 | 0.021388 | 0.022811 |
| string_options_test.test | RUN | 0.380877 | 2.987182 | 1.787796 | 0.002091 | 0.002134 | 0.001930 |
| static_vector_test.test | RUN | 2.245329 | 11.016742 | 10.627441 | 0.002755 | 0.002853 | 0.002752 |
| static_vector_options_test.test | RUN | 0.298173 | 0.778546 | 1.049189 | 0.002168 | 0.002107 | 0.002227 |
| stable_vector_test.test | RUN | 4.897254 | 22.488708 | 19.791643 | 0.037460 | 0.036682 | 0.039492 |
| small_vector_test.test | RUN | 3.315175 | 15.957322 | 14.386574 | 0.004202 | 0.004091 | 0.007310 |
| small_vector_options_test.test | RUN | 0.469400 | 1.971343 | 2.002860 | 0.002048 | 0.002268 | 0.001892 |
| slist_test.test | RUN | 3.053054 | 14.046961 | 12.801331 | 0.033964 | 0.033248 | 0.034144 |
| set_test.test | RUN | 4.625053 | 26.850018 | 23.035917 | 0.020012 | 0.018203 | 0.019670 |
| segtor_test.test | RUN | 6.124651 | 29.133142 | 26.461246 | 1.036891 | 0.903205 | 0.932885 |
| segtor_options_test.test | RUN | 0.316489 | 0.886151 | 1.337872 | 0.002183 | 0.002190 | 0.002190 |
| scoped_allocator_usage_test.test | RUN | 1.050360 | 4.421292 | 4.631052 | 0.001626 | 0.001973 | 0.002120 |
| scoped_allocator_adaptor_test.test | RUN | 0.481707 | 1.316802 | 2.121617 | 0.001959 | 0.002152 | 0.001708 |
| resource_adaptor_test.test | RUN | 0.247336 | 0.775914 | 0.981771 | 0.001953 | 0.001931 | 0.002065 |
| polymorphic_allocator_test.test | RUN | 0.206008 | 0.622971 | 0.791546 | 0.001898 | 0.001956 | 0.001809 |
| pmr_vector_test.test | RUN | 0.120731 | 0.237051 | 0.442004 | 0.000717 | 0.000870 | 0.001868 |
| pmr_string_test.test | RUN | 0.239583 | 0.507649 | 0.864028 | 0.000770 | 0.000839 | 0.002013 |
| pmr_stable_vector_test.test | RUN | 0.280248 | 0.783176 | 1.021880 | 0.001983 | 0.001850 | 0.001980 |
| pmr_small_vector_test.test | RUN | 0.142898 | 0.340280 | 0.540256 | 0.001926 | 0.002031 | 0.001805 |
| pmr_slist_test.test | RUN | 0.200760 | 1.033909 | 1.040888 | 0.002026 | 0.001853 | 0.001939 |
| pmr_set_test.test | RUN | 0.224489 | 0.560258 | 0.786085 | 0.001844 | 0.002086 | 0.001933 |
| pmr_nest_test.test | RUN | 0.175074 | 0.384734 | 0.626397 | 0.002002 | 0.001996 | 0.002042 |
| pmr_map_test.test | RUN | 0.232552 | 0.575144 | 0.843664 | 0.001800 | 0.001843 | 0.002058 |
| pmr_list_test.test | RUN | 0.152897 | 0.366155 | 0.494613 | 0.001802 | 0.001743 | 0.002124 |
| pmr_hub_test.test | RUN | 0.220129 | 0.488875 | 0.680286 | 0.001835 | 0.001751 | 0.001772 |
| pmr_flat_set_test.test | RUN | 0.171757 | 0.386278 | 0.650699 | 0.001973 | 0.001896 | 0.002012 |
| pmr_flat_map_test.test | RUN | 0.195804 | 0.446835 | 0.643664 | 0.001764 | 0.001794 | 0.001807 |
| pmr_devector_test.test | RUN | 0.136188 | 0.382265 | 0.547327 | 0.001854 | 0.001740 | 0.002007 |
| pmr_deque_test.test | RUN | 0.132170 | 0.318394 | 0.463133 | 0.002446 | 0.002118 | 0.001833 |
| pair_test.test | RUN | 0.329577 | 0.768274 | 1.188469 | 0.001788 | 0.002073 | 0.001736 |
| null_iterators_test.test | RUN | 0.527446 | 1.474567 | 1.846821 | 0.001972 | 0.002058 | 0.002024 |
| node_handle_test.test | RUN | 0.224954 | 1.829206 | 0.801864 | 0.001931 | 0.001968 | 0.001635 |
| node_allocator_test.test | RUN | 3.580524 | 17.288710 | 16.193228 | 0.013293 | 0.013557 | 0.011775 |
| new_delete_resource_test.test | RUN | 0.296558 | 1.113435 | 1.056463 | 0.002673 | 0.002716 | 0.002767 |
| nest_test.test | RUN | 0.702598 | 4.942148 | 3.483925 | 0.002023 | 0.002224 | 0.002054 |
| nest_api_test.test | RUN | 1.197881 | 9.440924 | 6.782918 | 0.003134 | 0.003105 | 0.003376 |
| monotonic_buffer_resource_test.test | RUN | 0.272712 | 1.963341 | 1.056347 | 0.001938 | 0.002029 | 0.002086 |
| memory_resource_test.test | RUN | 0.169955 | 0.566564 | 0.655482 | 0.001781 | 0.002035 | 0.001929 |
| map_test.test | RUN | 6.320151 | 30.207474 | 34.044380 | 0.014857 | 0.014850 | 0.014670 |
| list_test.test | RUN | 3.332328 | 15.327362 | 13.727960 | 0.033378 | 0.034583 | 0.035741 |
| is_constructible_test.test | RUN | 0.102588 | 0.217091 | 0.397603 | 0.000845 | 0.000927 | 0.001969 |
| insert_vs_emplace_test.test | RUN | 0.339329 | 1.308569 | 2.081525 | 0.002153 | 0.002670 | 0.002092 |
| hub_uses_allocator_test.test | RUN | 0.348947 | 1.111104 | 1.523292 | 0.001808 | 0.002164 | 0.002010 |
| hub_stability_test.test | RUN | 0.760535 | 5.000535 | 4.630806 | 0.002426 | 0.002450 | 0.002457 |
| hub_sort_test.test | RUN | 0.483530 | 2.291157 | 2.370865 | 0.662481 | 0.642505 | 0.612150 |
| hub_pocxx_test.test | RUN | 0.469800 | 2.702622 | 1.784587 | 0.001962 | 0.002036 | 0.001950 |
| hub_new_extended_alignment_test.test | RUN | 0.258061 | 0.854091 | 0.980882 | 0.001904 | 0.001973 | 0.001884 |
| hub_exception_safety_test.test | RUN | 0.908596 | 6.153265 | 4.509105 | 0.103581 | 0.117747 | 0.109087 |
| hub_empty_sort_test.test | RUN | 0.298019 | 0.780459 | 1.104153 | 0.001565 | 0.001961 | 0.002034 |
| hub_api_test.test | RUN | 2.967676 | 25.293891 | 15.479693 | 0.004849 | 0.004324 | 0.004980 |
| global_resource_test.test | RUN | 0.179334 | 0.541786 | 0.638811 | 0.002130 | 0.002166 | 0.002140 |
| flat_tree_test.test | RUN | 5.535387 | 38.014826 | 27.855846 | 0.001933 | 0.002094 | 0.002054 |
| flat_set_test.test | RUN | 7.048580 | 48.199667 | 35.709003 | 0.010816 | 0.009468 | 0.010764 |
| flat_set_adaptor_test.test | RUN | 5.317484 | 42.984034 | 28.107402 | 0.010648 | 0.009893 | 0.010773 |
| flat_map_test.test | RUN | 11.517130 | 71.453675 | 59.583843 | 0.011946 | 0.011433 | 0.010880 |
| flat_map_adaptor_test.test | RUN | 7.237697 | 52.003303 | 42.290422 | 0.010941 | 0.010388 | 0.010540 |
| explicit_inst_vector_test.test | RUN | 1.158291 | 7.165509 | 6.383039 | 0.001948 | 0.002078 | 0.001825 |
| explicit_inst_string_test.test | RUN | 1.483485 | 12.755976 | 8.449809 | 0.002003 | 0.002035 | 0.001996 |
| explicit_inst_static_vector_test.test | RUN | 0.157016 | 0.500930 | 0.632500 | 0.001951 | 0.002000 | 0.001982 |
| explicit_inst_stable_vector_test.test | RUN | 0.899755 | 4.599422 | 4.123236 | 0.001810 | 0.002066 | 0.002018 |
| explicit_inst_small_vector_test.test | RUN | 0.757816 | 4.162426 | 3.726499 | 0.001773 | 0.002004 | 0.001377 |
| explicit_inst_slist_test.test | RUN | 0.562805 | 3.810391 | 2.908393 | 0.001936 | 0.002024 | 0.001651 |
| explicit_inst_set_test.test | RUN | 0.801543 | 4.438621 | 3.627656 | 0.001947 | 0.001963 | 0.001681 |
| explicit_inst_map_test.test | RUN | 0.895988 | 4.568688 | 3.882878 | 0.001865 | 0.002162 | 0.002089 |
| explicit_inst_list_test.test | RUN | 0.203861 | 1.060419 | 1.037918 | 0.001876 | 0.001956 | 0.002100 |
| explicit_inst_flat_set_test.test | RUN | 4.640718 | 32.471644 | 21.730300 | 0.002042 | 0.002010 | 0.001873 |
| explicit_inst_flat_map_test.test | RUN | 5.026056 | 33.840909 | 24.052721 | 0.001873 | 0.002154 | 0.001664 |
| explicit_inst_devector_test.test | RUN | 0.623897 | 4.177271 | 3.159699 | 0.001744 | 0.001961 | 0.002017 |
| explicit_inst_deque_test.test | RUN | 0.651160 | 4.457073 | 3.300525 | 0.001948 | 0.001984 | 0.002030 |
| devector_test.test | RUN | 8.001810 | 51.797474 | 36.906268 | 0.005071 | 0.004012 | 0.005140 |
| devector_options_test.test | RUN | 0.387659 | 2.486803 | 1.840997 | 0.002335 | 0.002254 | 0.002393 |
| deque_test.test | RUN | 6.537869 | 31.199838 | 28.233990 | 0.484175 | 0.446418 | 0.539586 |
| deque_options_test.test | RUN | 0.315316 | 0.920381 | 1.412178 | 0.002028 | 0.002202 | 0.002238 |
| copy_move_algo_test.test | RUN | 0.295338 | 1.135013 | 1.324051 | 0.002076 | 0.002260 | 0.002144 |
| common_iterator_test.test | RUN | 0.023146 | 0.060649 | 0.062089 | 0.000690 | 0.000949 | 0.001975 |
| boost_iterator_comp_test.test | RUN | 0.423317 | 1.378753 | 1.666462 | 0.001913 | 0.001993 | 0.001964 |
| allocator_traits_test.test | RUN | 0.251531 | 0.633894 | 0.973776 | 0.001799 | 0.002002 | 0.001904 |
| alloc_full_test.test | RUN | 0.277970 | 1.198690 | 1.215809 | 0.006036 | 0.005967 | 0.006088 |
| alloc_basic_test.test | RUN | 0.233317 | 0.898551 | 0.973885 | 0.002391 | 0.002229 | 0.002078 |
| adaptive_pool_test.test | RUN | 3.543248 | 12.362669 | 16.245358 | 0.012235 | 0.011445 | 0.019725 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.941× | 0.952×, 0.929× | 1.013× |
| clang | 0.979× | 0.986×, 0.972× | 0.979× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824563).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adaptive_pool_test.test | 2 | 0.013401s | 0.012194s | 0.013487s | byte equality observed |
| alloc_basic_test.test | 10 | 0.001257s | 0.001308s | 0.001240s | byte equality observed |
| alloc_full_test.test | 6 | 0.003889s | 0.003850s | 0.003891s | byte equality observed |
| allocator_traits_test.test | 10 | 0.001165s | 0.001183s | 0.001158s | byte equality observed |
| boost_iterator_comp_test.test | 10 | 0.001266s | 0.001267s | 0.001226s | byte equality observed |
| common_iterator_test.test | 10 | 0.000581s | 0.000593s | 0.001247s | byte equality observed |
| copy_move_algo_test.test | 10 | 0.001445s | 0.001379s | 0.001344s | byte equality observed |
| deque_options_test.test | 10 | 0.001303s | 0.001344s | 0.001320s | byte equality observed |
| deque_test.test | 1 | 0.521123s | 0.478369s | 0.594975s | byte equality observed |
| devector_options_test.test | 10 | 0.001455s | 0.001315s | 0.001340s | byte equality observed |
| devector_test.test | 4 | 0.003701s | 0.003574s | 0.004897s | byte equality observed |
| explicit_inst_deque_test.test | 10 | 0.001175s | 0.001130s | 0.001156s | byte equality observed |
| explicit_inst_devector_test.test | 10 | 0.001137s | 0.001130s | 0.001134s | byte equality observed |
| explicit_inst_flat_map_test.test | 10 | 0.001146s | 0.001134s | 0.001167s | byte equality observed |
| explicit_inst_flat_set_test.test | 10 | 0.001147s | 0.001136s | 0.001129s | byte equality observed |
| explicit_inst_list_test.test | 10 | 0.001157s | 0.001115s | 0.001119s | byte equality observed |
| explicit_inst_map_test.test | 10 | 0.001180s | 0.001124s | 0.001130s | byte equality observed |
| explicit_inst_set_test.test | 10 | 0.001164s | 0.001128s | 0.001149s | byte equality observed |
| explicit_inst_slist_test.test | 10 | 0.001158s | 0.001129s | 0.001145s | byte equality observed |
| explicit_inst_small_vector_test.test | 10 | 0.001148s | 0.001164s | 0.001148s | byte equality observed |
| explicit_inst_stable_vector_test.test | 10 | 0.001163s | 0.001138s | 0.001172s | byte equality observed |
| explicit_inst_static_vector_test.test | 10 | 0.001135s | 0.001138s | 0.001128s | byte equality observed |
| explicit_inst_string_test.test | 10 | 0.001143s | 0.001149s | 0.001143s | byte equality observed |
| explicit_inst_vector_test.test | 10 | 0.001141s | 0.001144s | 0.001155s | byte equality observed |
| flat_map_adaptor_test.test | 3 | 0.007212s | 0.006812s | 0.006999s | byte equality observed |
| flat_map_test.test | 3 | 0.007747s | 0.007295s | 0.007059s | byte equality observed |
| flat_set_adaptor_test.test | 3 | 0.007208s | 0.006481s | 0.007170s | byte equality observed |
| flat_set_test.test | 3 | 0.007058s | 0.006273s | 0.007019s | byte equality observed |
| flat_tree_test.test | 10 | 0.001149s | 0.001161s | 0.001126s | byte equality observed |
| global_resource_test.test | 10 | 0.001193s | 0.001188s | 0.001155s | byte equality observed |
| hub_api_test.test | 6 | 0.003349s | 0.002723s | 0.002659s | byte equality observed |
| hub_empty_sort_test.test | 10 | 0.001148s | 0.001164s | 0.001125s | byte equality observed |
| hub_exception_safety_test.test | 1 | 0.073817s | 0.080868s | 0.074040s | byte equality observed |
| hub_new_extended_alignment_test.test | 10 | 0.001208s | 0.001288s | 0.001217s | byte equality observed |
| hub_pocxx_test.test | 10 | 0.001223s | 0.001250s | 0.001193s | byte equality observed |
| hub_sort_test.test | 1 | 0.529740s | 0.517212s | 0.494305s | byte equality observed |
| hub_stability_test.test | 9 | 0.001443s | 0.001450s | 0.001499s | byte equality observed |
| hub_uses_allocator_test.test | 10 | 0.001154s | 0.001143s | 0.001173s | byte equality observed |
| insert_vs_emplace_test.test | 10 | 0.001183s | 0.001191s | 0.001208s | byte equality observed |
| is_constructible_test.test | 10 | 0.000549s | 0.000544s | 0.001123s | byte equality observed |
| list_test.test | 1 | 0.021792s | 0.021116s | 0.021372s | byte equality observed |
| map_test.test | 3 | 0.009617s | 0.009485s | 0.009330s | byte equality observed |
| memory_resource_test.test | 10 | 0.001169s | 0.001188s | 0.001173s | byte equality observed |
| monotonic_buffer_resource_test.test | 10 | 0.001182s | 0.001193s | 0.001159s | byte equality observed |
| nest_api_test.test | 9 | 0.001960s | 0.002037s | 0.002004s | byte equality observed |
| nest_test.test | 10 | 0.001197s | 0.001191s | 0.001174s | byte equality observed |
| new_delete_resource_test.test | 10 | 0.001751s | 0.001741s | 0.001715s | byte equality observed |
| node_allocator_test.test | 3 | 0.008925s | 0.008604s | 0.008619s | byte equality observed |
| node_handle_test.test | 10 | 0.001172s | 0.001188s | 0.001145s | byte equality observed |
| null_iterators_test.test | 10 | 0.001177s | 0.001153s | 0.001152s | byte equality observed |
| pair_test.test | 10 | 0.001165s | 0.001157s | 0.001142s | byte equality observed |
| pmr_deque_test.test | 10 | 0.001145s | 0.001140s | 0.001140s | byte equality observed |
| pmr_devector_test.test | 10 | 0.001139s | 0.001114s | 0.001151s | byte equality observed |
| pmr_flat_map_test.test | 10 | 0.001149s | 0.001154s | 0.001158s | byte equality observed |
| pmr_flat_set_test.test | 10 | 0.001142s | 0.001133s | 0.001149s | byte equality observed |
| pmr_hub_test.test | 10 | 0.001162s | 0.001126s | 0.001138s | byte equality observed |
| pmr_list_test.test | 10 | 0.001147s | 0.001138s | 0.001135s | byte equality observed |
| pmr_map_test.test | 10 | 0.001143s | 0.001143s | 0.001136s | byte equality observed |
| pmr_nest_test.test | 10 | 0.001147s | 0.001123s | 0.001168s | byte equality observed |
| pmr_set_test.test | 10 | 0.001170s | 0.001133s | 0.001195s | byte equality observed |
| pmr_slist_test.test | 10 | 0.001140s | 0.001115s | 0.001231s | byte equality observed |
| pmr_small_vector_test.test | 10 | 0.001155s | 0.001106s | 0.001219s | byte equality observed |
| pmr_stable_vector_test.test | 10 | 0.001153s | 0.001159s | 0.001216s | byte equality observed |
| pmr_string_test.test | 10 | 0.000560s | 0.000539s | 0.001215s | byte equality observed |
| pmr_vector_test.test | 10 | 0.000552s | 0.000535s | 0.001237s | byte equality observed |
| polymorphic_allocator_test.test | 10 | 0.001230s | 0.001192s | 0.001241s | byte equality observed |
| resource_adaptor_test.test | 10 | 0.001234s | 0.001275s | 0.001267s | byte equality observed |
| scoped_allocator_adaptor_test.test | 10 | 0.001256s | 0.001229s | 0.001241s | byte equality observed |
| scoped_allocator_usage_test.test | 10 | 0.001300s | 0.001288s | 0.001254s | byte equality observed |
| segtor_options_test.test | 10 | 0.001268s | 0.001251s | 0.001237s | byte equality observed |
| segtor_test.test | 1 | 0.524402s | 0.456522s | 0.445024s | byte equality observed |
| set_test.test | 2 | 0.013473s | 0.012098s | 0.013148s | byte equality observed |
| slist_test.test | 1 | 0.020438s | 0.020357s | 0.020315s | byte equality observed |
| small_vector_options_test.test | 10 | 0.001294s | 0.001242s | 0.001244s | byte equality observed |
| small_vector_test.test | 5 | 0.002405s | 0.002226s | 0.004088s | byte equality observed |
| stable_vector_test.test | 1 | 0.029393s | 0.029660s | 0.030860s | byte equality observed |
| static_vector_options_test.test | 10 | 0.001260s | 0.001191s | 0.001231s | byte equality observed |
| static_vector_test.test | 10 | 0.001589s | 0.001493s | 0.001558s | byte equality observed |
| string_options_test.test | 10 | 0.001232s | 0.001219s | 0.001189s | byte equality observed |
| string_test.test | 2 | 0.015306s | 0.015313s | 0.014988s | byte equality observed |
| string_view_compat_test.test | 10 | 0.001175s | 0.001167s | 0.001136s | byte equality observed |
| synchronized_pool_resource_test.test | 10 | 0.001233s | 0.001207s | 0.001221s | byte equality observed |
| throw_exception_test.test | 10 | 0.001148s | 0.001130s | 0.001148s | byte equality observed |
| tree_test.test | 10 | 0.001147s | 0.001144s | 0.001162s | byte equality observed |
| unsynchronized_pool_resource_test.test | 10 | 0.001204s | 0.001204s | 0.001202s | byte equality observed |
| uses_allocator_construction_test.test | 10 | 0.001129s | 0.001117s | 0.001157s | byte equality observed |
| uses_allocator_test.test | 10 | 0.000532s | 0.000534s | 0.001112s | byte equality observed |
| vector_options_test.test | 10 | 0.001346s | 0.001302s | 0.001303s | byte equality observed |
| vector_test.test | 4 | 0.003960s | 0.003867s | 0.005547s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824563).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
