# libs/container/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| vector_test.test | RUN | 8.936756 | 41.901634 | 38.541532 | 0.007294 | 0.006899 | 0.009732 |
| vector_options_test.test | RUN | 0.414031 | 1.921536 | 1.779714 | 0.002342 | 0.002353 | 0.002322 |
| uses_allocator_test.test | RUN | 0.075739 | 0.168501 | 0.249148 | 0.000797 | 0.000868 | 0.002022 |
| uses_allocator_construction_test.test | RUN | 0.228677 | 0.640212 | 0.891449 | 0.001888 | 0.001868 | 0.002149 |
| unsynchronized_pool_resource_test.test | RUN | 0.282614 | 1.820780 | 1.084975 | 0.001912 | 0.002336 | 0.002132 |
| tree_test.test | RUN | 1.211339 | 6.193797 | 5.700588 | 0.001801 | 0.002009 | 0.002187 |
| throw_exception_test.test | RUN | 0.170400 | 0.381308 | 0.623369 | 0.001763 | 0.002122 | 0.001879 |
| synchronized_pool_resource_test.test | RUN | 0.285417 | 1.853291 | 1.152055 | 0.001944 | 0.002302 | 0.002256 |
| string_view_compat_test.test | RUN | 0.471965 | 3.659034 | 2.172256 | 0.001870 | 0.002144 | 0.001957 |
| string_test.test | RUN | 2.057212 | 13.628186 | 10.594913 | 0.023699 | 0.021363 | 0.022913 |
| string_options_test.test | RUN | 0.389608 | 2.980450 | 1.794583 | 0.001924 | 0.002242 | 0.002186 |
| static_vector_test.test | RUN | 2.235675 | 11.107427 | 10.635986 | 0.002608 | 0.002674 | 0.002846 |
| static_vector_options_test.test | RUN | 0.275626 | 0.804840 | 1.058858 | 0.002102 | 0.002142 | 0.002358 |
| stable_vector_test.test | RUN | 4.852019 | 22.439662 | 20.167282 | 0.036807 | 0.037354 | 0.038631 |
| small_vector_test.test | RUN | 3.404438 | 15.901072 | 14.335903 | 0.004281 | 0.003918 | 0.006886 |
| small_vector_options_test.test | RUN | 0.482586 | 1.994218 | 2.022387 | 0.002046 | 0.002269 | 0.002248 |
| slist_test.test | RUN | 3.111805 | 14.127685 | 13.067766 | 0.037192 | 0.033988 | 0.034253 |
| set_test.test | RUN | 4.697651 | 26.589606 | 22.797437 | 0.020534 | 0.018086 | 0.019344 |
| segtor_test.test | RUN | 6.044432 | 29.057397 | 25.947344 | 1.004802 | 0.921875 | 0.895459 |
| segtor_options_test.test | RUN | 0.315338 | 0.892497 | 1.284616 | 0.002257 | 0.001968 | 0.002085 |
| scoped_allocator_usage_test.test | RUN | 1.068319 | 4.474014 | 4.661519 | 0.002004 | 0.002016 | 0.001980 |
| scoped_allocator_adaptor_test.test | RUN | 0.499850 | 1.342054 | 2.116072 | 0.001825 | 0.001868 | 0.002092 |
| resource_adaptor_test.test | RUN | 0.252465 | 0.769950 | 0.920433 | 0.002008 | 0.002097 | 0.001918 |
| polymorphic_allocator_test.test | RUN | 0.212772 | 0.638074 | 0.832853 | 0.001828 | 0.002031 | 0.001838 |
| pmr_vector_test.test | RUN | 0.119270 | 0.258277 | 0.395015 | 0.000784 | 0.000772 | 0.001950 |
| pmr_string_test.test | RUN | 0.230295 | 0.528090 | 0.846383 | 0.000776 | 0.000896 | 0.002054 |
| pmr_stable_vector_test.test | RUN | 0.265320 | 0.804746 | 1.023531 | 0.001738 | 0.002034 | 0.001927 |
| pmr_small_vector_test.test | RUN | 0.143750 | 0.340299 | 0.546258 | 0.001802 | 0.002046 | 0.002052 |
| pmr_slist_test.test | RUN | 0.200914 | 1.036807 | 1.005208 | 0.001931 | 0.002123 | 0.002044 |
| pmr_set_test.test | RUN | 0.224985 | 0.565608 | 0.856503 | 0.001917 | 0.002027 | 0.001967 |
| pmr_nest_test.test | RUN | 0.175277 | 0.393150 | 0.561452 | 0.001967 | 0.001778 | 0.002047 |
| pmr_map_test.test | RUN | 0.236936 | 0.587155 | 0.808570 | 0.001703 | 0.001940 | 0.001987 |
| pmr_list_test.test | RUN | 0.161381 | 0.367877 | 0.603024 | 0.002019 | 0.002016 | 0.002026 |
| pmr_hub_test.test | RUN | 0.210043 | 0.503102 | 0.678366 | 0.001774 | 0.002082 | 0.001862 |
| pmr_flat_set_test.test | RUN | 0.169049 | 0.397461 | 0.651652 | 0.001878 | 0.002010 | 0.001832 |
| pmr_flat_map_test.test | RUN | 0.203283 | 0.449152 | 0.766284 | 0.001833 | 0.001946 | 0.001639 |
| pmr_devector_test.test | RUN | 0.135456 | 0.391275 | 0.481166 | 0.001906 | 0.001903 | 0.001742 |
| pmr_deque_test.test | RUN | 0.136910 | 0.315669 | 0.547656 | 0.001932 | 0.002136 | 0.002083 |
| pair_test.test | RUN | 0.343929 | 0.765476 | 1.218494 | 0.001936 | 0.002103 | 0.002012 |
| null_iterators_test.test | RUN | 0.516077 | 1.461717 | 2.031896 | 0.001926 | 0.001910 | 0.002152 |
| node_handle_test.test | RUN | 0.228411 | 1.839692 | 0.855779 | 0.001761 | 0.002159 | 0.002148 |
| node_allocator_test.test | RUN | 3.680935 | 17.563868 | 15.903820 | 0.012997 | 0.011865 | 0.014142 |
| new_delete_resource_test.test | RUN | 0.301081 | 1.138663 | 1.160911 | 0.002739 | 0.002965 | 0.003074 |
| nest_test.test | RUN | 0.683142 | 4.945775 | 3.424591 | 0.001858 | 0.002094 | 0.001718 |
| nest_api_test.test | RUN | 1.280713 | 9.418095 | 6.896923 | 0.003126 | 0.003382 | 0.003235 |
| monotonic_buffer_resource_test.test | RUN | 0.250238 | 1.943006 | 0.924909 | 0.001677 | 0.002248 | 0.002076 |
| memory_resource_test.test | RUN | 0.151344 | 0.551256 | 0.630160 | 0.001885 | 0.002176 | 0.002088 |
| map_test.test | RUN | 6.475860 | 29.705123 | 33.743657 | 0.014425 | 0.013898 | 0.013471 |
| list_test.test | RUN | 3.370931 | 15.447588 | 13.916669 | 0.033224 | 0.033816 | 0.034539 |
| is_constructible_test.test | RUN | 0.109949 | 0.234205 | 0.371474 | 0.000860 | 0.000769 | 0.001892 |
| insert_vs_emplace_test.test | RUN | 0.348454 | 1.321901 | 2.003662 | 0.002027 | 0.002143 | 0.002186 |
| hub_uses_allocator_test.test | RUN | 0.354307 | 1.103457 | 1.556231 | 0.001834 | 0.002076 | 0.002134 |
| hub_stability_test.test | RUN | 0.778114 | 5.058209 | 4.817810 | 0.002412 | 0.002470 | 0.002102 |
| hub_sort_test.test | RUN | 0.501459 | 2.280962 | 2.302576 | 0.632314 | 0.622545 | 0.614335 |
| hub_pocxx_test.test | RUN | 0.480914 | 2.687187 | 1.854120 | 0.001981 | 0.002091 | 0.002132 |
| hub_new_extended_alignment_test.test | RUN | 0.268421 | 0.822684 | 1.040715 | 0.001959 | 0.002074 | 0.001826 |
| hub_exception_safety_test.test | RUN | 0.944441 | 6.354841 | 4.856448 | 0.109109 | 0.116539 | 0.115634 |
| hub_empty_sort_test.test | RUN | 0.302628 | 0.757938 | 1.227056 | 0.001884 | 0.002013 | 0.002085 |
| hub_api_test.test | RUN | 2.984763 | 24.884529 | 14.891389 | 0.004977 | 0.004536 | 0.004282 |
| global_resource_test.test | RUN | 0.166796 | 0.533558 | 0.632625 | 0.001892 | 0.002140 | 0.002010 |
| flat_tree_test.test | RUN | 5.698088 | 38.165041 | 27.368386 | 0.001888 | 0.002038 | 0.001826 |
| flat_set_test.test | RUN | 7.125599 | 48.489953 | 36.456121 | 0.011384 | 0.009598 | 0.010652 |
| flat_set_adaptor_test.test | RUN | 5.391266 | 42.719436 | 28.147991 | 0.010458 | 0.010346 | 0.010763 |
| flat_map_test.test | RUN | 11.639299 | 72.559181 | 58.839466 | 0.011929 | 0.011581 | 0.010929 |
| flat_map_adaptor_test.test | RUN | 7.390788 | 53.164217 | 41.781669 | 0.010875 | 0.011106 | 0.010345 |
| explicit_inst_vector_test.test | RUN | 1.153477 | 7.241198 | 6.266232 | 0.001893 | 0.002106 | 0.001914 |
| explicit_inst_string_test.test | RUN | 1.489533 | 12.587167 | 8.385192 | 0.001820 | 0.002105 | 0.001913 |
| explicit_inst_static_vector_test.test | RUN | 0.154121 | 0.494066 | 0.658989 | 0.001720 | 0.001859 | 0.001860 |
| explicit_inst_stable_vector_test.test | RUN | 0.934696 | 4.541999 | 4.042232 | 0.002064 | 0.002240 | 0.001906 |
| explicit_inst_small_vector_test.test | RUN | 0.790551 | 4.129431 | 3.899423 | 0.001785 | 0.002093 | 0.001854 |
| explicit_inst_slist_test.test | RUN | 0.612014 | 3.804020 | 3.044830 | 0.001957 | 0.002014 | 0.001653 |
| explicit_inst_set_test.test | RUN | 0.854442 | 4.471251 | 3.603157 | 0.002286 | 0.002013 | 0.001870 |
| explicit_inst_map_test.test | RUN | 0.940567 | 4.541952 | 3.889093 | 0.002054 | 0.002149 | 0.002108 |
| explicit_inst_list_test.test | RUN | 0.217323 | 1.034491 | 1.042926 | 0.001905 | 0.002133 | 0.002080 |
| explicit_inst_flat_set_test.test | RUN | 4.847202 | 33.310836 | 22.037323 | 0.001847 | 0.002312 | 0.002000 |
| explicit_inst_flat_map_test.test | RUN | 5.182693 | 34.780558 | 24.153176 | 0.001948 | 0.002196 | 0.002170 |
| explicit_inst_devector_test.test | RUN | 0.601559 | 4.250728 | 3.110523 | 0.001759 | 0.002104 | 0.002050 |
| explicit_inst_deque_test.test | RUN | 0.649045 | 4.570795 | 3.466078 | 0.002026 | 0.002260 | 0.002068 |
| devector_test.test | RUN | 9.113727 | 52.797430 | 41.381129 | 0.004190 | 0.004149 | 0.005353 |
| devector_options_test.test | RUN | 0.392490 | 2.572804 | 2.011536 | 0.002231 | 0.002401 | 0.002412 |
| deque_test.test | RUN | 6.402175 | 32.108024 | 26.392123 | 0.558285 | 0.430846 | 0.548648 |
| deque_options_test.test | RUN | 0.317093 | 0.985604 | 1.307761 | 0.002055 | 0.002434 | 0.002036 |
| copy_move_algo_test.test | RUN | 0.304550 | 1.200946 | 1.181936 | 0.002056 | 0.002384 | 0.002086 |
| common_iterator_test.test | RUN | 0.025273 | 0.063280 | 0.052121 | 0.000792 | 0.000915 | 0.002118 |
| boost_iterator_comp_test.test | RUN | 0.440451 | 1.382389 | 1.717982 | 0.001739 | 0.002122 | 0.002023 |
| allocator_traits_test.test | RUN | 0.261439 | 0.651479 | 0.874066 | 0.001852 | 0.002058 | 0.002008 |
| alloc_full_test.test | RUN | 0.284663 | 1.290450 | 1.291373 | 0.005725 | 0.006422 | 0.006192 |
| alloc_basic_test.test | RUN | 0.237909 | 0.943848 | 0.951408 | 0.002152 | 0.002271 | 0.002248 |
| adaptive_pool_test.test | RUN | 2.422656 | 13.167358 | 10.330980 | 0.012450 | 0.011674 | 0.012549 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.906× | 0.896×, 0.916× | 1.016× |
| clang | 0.967× | 0.968×, 0.966× | 0.992× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274616).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adaptive_pool_test.test | 2 | 0.013154s | 0.012023s | 0.013929s | byte equality observed |
| alloc_basic_test.test | 10 | 0.001278s | 0.001305s | 0.001275s | byte equality observed |
| alloc_full_test.test | 6 | 0.003786s | 0.003942s | 0.003960s | byte equality observed |
| allocator_traits_test.test | 10 | 0.001177s | 0.001164s | 0.001193s | byte equality observed |
| boost_iterator_comp_test.test | 10 | 0.001257s | 0.001231s | 0.001215s | byte equality observed |
| common_iterator_test.test | 10 | 0.000587s | 0.000581s | 0.001228s | byte equality observed |
| copy_move_algo_test.test | 10 | 0.001442s | 0.001351s | 0.001405s | byte equality observed |
| deque_options_test.test | 10 | 0.001291s | 0.001266s | 0.001266s | byte equality observed |
| deque_test.test | 1 | 0.618826s | 0.464424s | 0.596521s | byte equality observed |
| devector_options_test.test | 10 | 0.001436s | 0.001330s | 0.001292s | byte equality observed |
| devector_test.test | 4 | 0.003704s | 0.003661s | 0.004880s | byte equality observed |
| explicit_inst_deque_test.test | 10 | 0.001154s | 0.001147s | 0.001112s | byte equality observed |
| explicit_inst_devector_test.test | 10 | 0.001138s | 0.001146s | 0.001135s | byte equality observed |
| explicit_inst_flat_map_test.test | 10 | 0.001151s | 0.001156s | 0.001138s | byte equality observed |
| explicit_inst_flat_set_test.test | 10 | 0.001152s | 0.001154s | 0.001124s | byte equality observed |
| explicit_inst_list_test.test | 10 | 0.001137s | 0.001134s | 0.001130s | byte equality observed |
| explicit_inst_map_test.test | 10 | 0.001152s | 0.001174s | 0.001147s | byte equality observed |
| explicit_inst_set_test.test | 10 | 0.001156s | 0.001151s | 0.001132s | byte equality observed |
| explicit_inst_slist_test.test | 10 | 0.001149s | 0.001141s | 0.001142s | byte equality observed |
| explicit_inst_small_vector_test.test | 10 | 0.001153s | 0.001145s | 0.001150s | byte equality observed |
| explicit_inst_stable_vector_test.test | 10 | 0.001145s | 0.001159s | 0.001142s | byte equality observed |
| explicit_inst_static_vector_test.test | 10 | 0.001146s | 0.001153s | 0.001144s | byte equality observed |
| explicit_inst_string_test.test | 10 | 0.001157s | 0.001152s | 0.001143s | byte equality observed |
| explicit_inst_vector_test.test | 10 | 0.001142s | 0.001154s | 0.001139s | byte equality observed |
| flat_map_adaptor_test.test | 3 | 0.007206s | 0.006847s | 0.006930s | byte equality observed |
| flat_map_test.test | 3 | 0.007692s | 0.007287s | 0.007163s | byte equality observed |
| flat_set_adaptor_test.test | 3 | 0.007277s | 0.006515s | 0.007120s | byte equality observed |
| flat_set_test.test | 3 | 0.007272s | 0.006290s | 0.007019s | byte equality observed |
| flat_tree_test.test | 10 | 0.001153s | 0.001148s | 0.001151s | byte equality observed |
| global_resource_test.test | 10 | 0.001171s | 0.001184s | 0.001188s | byte equality observed |
| hub_api_test.test | 6 | 0.003289s | 0.002743s | 0.002775s | byte equality observed |
| hub_empty_sort_test.test | 10 | 0.001166s | 0.001160s | 0.001154s | byte equality observed |
| hub_exception_safety_test.test | 1 | 0.076786s | 0.084664s | 0.078180s | byte equality observed |
| hub_new_extended_alignment_test.test | 10 | 0.001210s | 0.001261s | 0.001256s | byte equality observed |
| hub_pocxx_test.test | 10 | 0.001192s | 0.001219s | 0.001192s | byte equality observed |
| hub_sort_test.test | 1 | 0.511203s | 0.512873s | 0.496117s | byte equality observed |
| hub_stability_test.test | 10 | 0.001474s | 0.001440s | 0.001498s | byte equality observed |
| hub_uses_allocator_test.test | 10 | 0.001172s | 0.001143s | 0.001157s | byte equality observed |
| insert_vs_emplace_test.test | 10 | 0.001208s | 0.001190s | 0.001171s | byte equality observed |
| is_constructible_test.test | 10 | 0.000557s | 0.000549s | 0.001128s | byte equality observed |
| list_test.test | 1 | 0.021610s | 0.020977s | 0.021653s | byte equality observed |
| map_test.test | 3 | 0.009613s | 0.009456s | 0.009310s | byte equality observed |
| memory_resource_test.test | 10 | 0.001189s | 0.001181s | 0.001178s | byte equality observed |
| monotonic_buffer_resource_test.test | 10 | 0.001213s | 0.001191s | 0.001194s | byte equality observed |
| nest_api_test.test | 10 | 0.001972s | 0.001993s | 0.002020s | byte equality observed |
| nest_test.test | 10 | 0.001218s | 0.001206s | 0.001197s | byte equality observed |
| new_delete_resource_test.test | 10 | 0.001741s | 0.001754s | 0.001730s | byte equality observed |
| node_allocator_test.test | 3 | 0.008987s | 0.008831s | 0.008914s | byte equality observed |
| node_handle_test.test | 10 | 0.001195s | 0.001186s | 0.001188s | byte equality observed |
| null_iterators_test.test | 10 | 0.001179s | 0.001163s | 0.001165s | byte equality observed |
| pair_test.test | 10 | 0.001179s | 0.001164s | 0.001166s | byte equality observed |
| pmr_deque_test.test | 10 | 0.001155s | 0.001153s | 0.001164s | byte equality observed |
| pmr_devector_test.test | 10 | 0.001167s | 0.001158s | 0.001158s | byte equality observed |
| pmr_flat_map_test.test | 10 | 0.001160s | 0.001146s | 0.001175s | byte equality observed |
| pmr_flat_set_test.test | 10 | 0.001158s | 0.001184s | 0.001151s | byte equality observed |
| pmr_hub_test.test | 10 | 0.001166s | 0.001197s | 0.001158s | byte equality observed |
| pmr_list_test.test | 10 | 0.001165s | 0.001150s | 0.001163s | byte equality observed |
| pmr_map_test.test | 10 | 0.001156s | 0.001180s | 0.001167s | byte equality observed |
| pmr_nest_test.test | 10 | 0.001231s | 0.001162s | 0.001222s | byte equality observed |
| pmr_set_test.test | 10 | 0.001259s | 0.001179s | 0.001246s | byte equality observed |
| pmr_slist_test.test | 10 | 0.001269s | 0.001170s | 0.001198s | byte equality observed |
| pmr_small_vector_test.test | 10 | 0.001275s | 0.001162s | 0.001229s | byte equality observed |
| pmr_stable_vector_test.test | 10 | 0.001241s | 0.001170s | 0.001202s | byte equality observed |
| pmr_string_test.test | 10 | 0.000585s | 0.000558s | 0.001233s | byte equality observed |
| pmr_vector_test.test | 10 | 0.000596s | 0.000557s | 0.001217s | byte equality observed |
| polymorphic_allocator_test.test | 10 | 0.001241s | 0.001251s | 0.001216s | byte equality observed |
| resource_adaptor_test.test | 10 | 0.001258s | 0.001280s | 0.001275s | byte equality observed |
| scoped_allocator_adaptor_test.test | 10 | 0.001237s | 0.001256s | 0.001320s | byte equality observed |
| scoped_allocator_usage_test.test | 10 | 0.001255s | 0.001272s | 0.001235s | byte equality observed |
| segtor_options_test.test | 10 | 0.001236s | 0.001258s | 0.001246s | byte equality observed |
| segtor_test.test | 1 | 0.493098s | 0.459066s | 0.451340s | byte equality observed |
| set_test.test | 2 | 0.013757s | 0.011817s | 0.013363s | byte equality observed |
| slist_test.test | 1 | 0.020664s | 0.020275s | 0.020528s | byte equality observed |
| small_vector_options_test.test | 10 | 0.001293s | 0.001249s | 0.001239s | byte equality observed |
| small_vector_test.test | 5 | 0.002494s | 0.002268s | 0.004107s | byte equality observed |
| stable_vector_test.test | 1 | 0.030237s | 0.030323s | 0.031486s | byte equality observed |
| static_vector_options_test.test | 10 | 0.001235s | 0.001182s | 0.001242s | byte equality observed |
| static_vector_test.test | 10 | 0.001598s | 0.001462s | 0.001523s | byte equality observed |
| string_options_test.test | 10 | 0.001234s | 0.001192s | 0.001197s | byte equality observed |
| string_test.test | 2 | 0.015081s | 0.015436s | 0.015056s | byte equality observed |
| string_view_compat_test.test | 10 | 0.001174s | 0.001163s | 0.001175s | byte equality observed |
| synchronized_pool_resource_test.test | 10 | 0.001224s | 0.001211s | 0.001219s | byte equality observed |
| throw_exception_test.test | 10 | 0.001157s | 0.001140s | 0.001149s | byte equality observed |
| tree_test.test | 10 | 0.001158s | 0.001139s | 0.001136s | byte equality observed |
| unsynchronized_pool_resource_test.test | 10 | 0.001224s | 0.001214s | 0.001206s | byte equality observed |
| uses_allocator_construction_test.test | 10 | 0.001144s | 0.001131s | 0.001146s | byte equality observed |
| uses_allocator_test.test | 10 | 0.000545s | 0.000535s | 0.001117s | byte equality observed |
| vector_options_test.test | 10 | 0.001333s | 0.001309s | 0.001326s | byte equality observed |
| vector_test.test | 4 | 0.004250s | 0.004019s | 0.005662s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274616).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
