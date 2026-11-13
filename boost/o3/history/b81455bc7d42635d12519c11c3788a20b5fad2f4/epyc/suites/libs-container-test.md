# libs/container/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| vector_test.test | RUN | 5.034308 | 28.199633 | 20.116339 | 0.004239 | 0.005468 | 0.005468 |
| vector_options_test.test | RUN | 0.233081 | 1.329067 | 0.825232 | 0.001730 | 0.001966 | 0.001664 |
| uses_allocator_test.test | RUN | 0.043704 | 0.108904 | 0.106021 | 0.000540 | 0.000652 | 0.001427 |
| uses_allocator_construction_test.test | RUN | 0.127543 | 0.393689 | 0.334854 | 0.001592 | 0.001702 | 0.001532 |
| unsynchronized_pool_resource_test.test | RUN | 0.154726 | 1.181979 | 0.463227 | 0.001538 | 0.001694 | 0.001501 |
| tree_test.test | RUN | 0.725149 | 4.532088 | 2.820344 | 0.001477 | 0.001929 | 0.001507 |
| throw_exception_test.test | RUN | 0.091148 | 0.255522 | 0.254027 | 0.001373 | 0.001842 | 0.001458 |
| synchronized_pool_resource_test.test | RUN | 0.157048 | 1.208016 | 0.487210 | 0.001572 | 0.001963 | 0.001533 |
| string_view_compat_test.test | RUN | 0.266996 | 2.431869 | 0.910922 | 0.001393 | 0.001942 | 0.001462 |
| string_test.test | RUN | 1.188737 | 9.274217 | 5.295193 | 0.011750 | 0.015155 | 0.011374 |
| string_options_test.test | RUN | 0.210458 | 1.921588 | 0.712578 | 0.001588 | 0.002075 | 0.001645 |
| static_vector_test.test | RUN | 1.271141 | 7.711565 | 5.436459 | 0.002012 | 0.002337 | 0.001876 |
| static_vector_options_test.test | RUN | 0.153447 | 0.514368 | 0.461385 | 0.001558 | 0.002090 | 0.001574 |
| stable_vector_test.test | RUN | 2.893812 | 15.269125 | 10.402339 | 0.022760 | 0.024533 | 0.024320 |
| small_vector_test.test | RUN | 1.940717 | 10.265751 | 7.502437 | 0.002689 | 0.002739 | 0.003864 |
| small_vector_options_test.test | RUN | 0.268285 | 1.319580 | 0.926862 | 0.001528 | 0.002058 | 0.001603 |
| slist_test.test | RUN | 1.807634 | 9.154216 | 6.612748 | 0.017490 | 0.022217 | 0.017908 |
| set_test.test | RUN | 2.699048 | 18.593119 | 11.861333 | 0.012177 | 0.013459 | 0.011390 |
| segtor_test.test | RUN | 3.594454 | 20.618410 | 13.462668 | 0.449760 | 0.538274 | 0.381893 |
| segtor_options_test.test | RUN | 0.175996 | 0.560233 | 0.577443 | 0.001618 | 0.002067 | 0.001604 |
| scoped_allocator_usage_test.test | RUN | 0.633048 | 3.240060 | 2.208755 | 0.001292 | 0.001959 | 0.001430 |
| scoped_allocator_adaptor_test.test | RUN | 0.262701 | 0.864756 | 0.974005 | 0.001471 | 0.001879 | 0.001498 |
| resource_adaptor_test.test | RUN | 0.139062 | 0.542000 | 0.410383 | 0.001415 | 0.001894 | 0.001492 |
| polymorphic_allocator_test.test | RUN | 0.121061 | 0.424696 | 0.353983 | 0.001292 | 0.001722 | 0.001492 |
| pmr_vector_test.test | RUN | 0.067898 | 0.163701 | 0.168136 | 0.000523 | 0.000787 | 0.001382 |
| pmr_string_test.test | RUN | 0.132376 | 0.337039 | 0.330715 | 0.000617 | 0.000816 | 0.001434 |
| pmr_stable_vector_test.test | RUN | 0.156814 | 0.543107 | 0.475907 | 0.001320 | 0.001842 | 0.001219 |
| pmr_small_vector_test.test | RUN | 0.080259 | 0.220542 | 0.214207 | 0.001253 | 0.001770 | 0.001478 |
| pmr_slist_test.test | RUN | 0.113785 | 0.736488 | 0.490379 | 0.001271 | 0.001830 | 0.001181 |
| pmr_set_test.test | RUN | 0.122274 | 0.365168 | 0.366919 | 0.001446 | 0.001849 | 0.001124 |
| pmr_nest_test.test | RUN | 0.100507 | 0.250124 | 0.243572 | 0.001428 | 0.001845 | 0.001086 |
| pmr_map_test.test | RUN | 0.128391 | 0.383331 | 0.354542 | 0.001499 | 0.001691 | 0.001403 |
| pmr_list_test.test | RUN | 0.088056 | 0.240785 | 0.231783 | 0.001300 | 0.001799 | 0.001329 |
| pmr_hub_test.test | RUN | 0.123201 | 0.326942 | 0.287367 | 0.001417 | 0.001726 | 0.001518 |
| pmr_flat_set_test.test | RUN | 0.096758 | 0.250198 | 0.252642 | 0.001542 | 0.001622 | 0.001486 |
| pmr_flat_map_test.test | RUN | 0.112314 | 0.292054 | 0.296868 | 0.001261 | 0.001689 | 0.001556 |
| pmr_devector_test.test | RUN | 0.075193 | 0.255490 | 0.216832 | 0.001379 | 0.001478 | 0.001466 |
| pmr_deque_test.test | RUN | 0.074546 | 0.209753 | 0.219467 | 0.001753 | 0.001905 | 0.001410 |
| pair_test.test | RUN | 0.192077 | 0.502020 | 0.474566 | 0.001567 | 0.001868 | 0.001551 |
| null_iterators_test.test | RUN | 0.293023 | 0.973695 | 0.845269 | 0.001165 | 0.001856 | 0.001351 |
| node_handle_test.test | RUN | 0.128568 | 1.187025 | 0.359020 | 0.001421 | 0.001919 | 0.001578 |
| node_allocator_test.test | RUN | 2.093401 | 12.467775 | 8.305675 | 0.007772 | 0.009595 | 0.007855 |
| new_delete_resource_test.test | RUN | 0.149386 | 0.765221 | 0.482805 | 0.001886 | 0.002617 | 0.002078 |
| nest_test.test | RUN | 0.416483 | 3.463096 | 1.725200 | 0.001490 | 0.001953 | 0.001564 |
| nest_api_test.test | RUN | 0.702833 | 6.484105 | 3.496652 | 0.002152 | 0.002842 | 0.002102 |
| monotonic_buffer_resource_test.test | RUN | 0.164053 | 1.300624 | 0.507737 | 0.001452 | 0.001854 | 0.001219 |
| memory_resource_test.test | RUN | 0.098094 | 0.378455 | 0.280783 | 0.001447 | 0.001922 | 0.001439 |
| map_test.test | RUN | 3.680229 | 21.452224 | 17.155210 | 0.009315 | 0.011230 | 0.008665 |
| list_test.test | RUN | 1.907730 | 10.890606 | 6.952338 | 0.018357 | 0.023043 | 0.018034 |
| is_constructible_test.test | RUN | 0.062343 | 0.138234 | 0.156729 | 0.000682 | 0.000776 | 0.001513 |
| insert_vs_emplace_test.test | RUN | 0.197502 | 0.855257 | 0.915889 | 0.001627 | 0.001973 | 0.001584 |
| hub_uses_allocator_test.test | RUN | 0.208581 | 0.741770 | 0.664948 | 0.001449 | 0.001976 | 0.001577 |
| hub_stability_test.test | RUN | 0.451429 | 3.489412 | 2.372301 | 0.001679 | 0.002224 | 0.001701 |
| hub_sort_test.test | RUN | 0.282254 | 1.607101 | 1.141269 | 0.449544 | 0.533073 | 0.422354 |
| hub_pocxx_test.test | RUN | 0.271713 | 1.796701 | 0.832382 | 0.001173 | 0.001882 | 0.001290 |
| hub_new_extended_alignment_test.test | RUN | 0.150362 | 0.560875 | 0.440409 | 0.001493 | 0.001890 | 0.001285 |
| hub_exception_safety_test.test | RUN | 0.516190 | 4.396928 | 2.404700 | 0.069419 | 0.086388 | 0.072888 |
| hub_empty_sort_test.test | RUN | 0.165091 | 0.519167 | 0.518603 | 0.001245 | 0.001851 | 0.001490 |
| hub_api_test.test | RUN | 1.772759 | 17.285028 | 7.439128 | 0.003177 | 0.003636 | 0.003124 |
| global_resource_test.test | RUN | 0.093177 | 0.360100 | 0.266843 | 0.001535 | 0.001924 | 0.001495 |
| flat_tree_test.test | RUN | 3.310375 | 27.490205 | 14.829531 | 0.001443 | 0.001887 | 0.001527 |
| flat_set_test.test | RUN | 4.129748 | 33.743642 | 19.338810 | 0.006898 | 0.007116 | 0.006403 |
| flat_set_adaptor_test.test | RUN | 3.091354 | 29.947208 | 14.991949 | 0.006769 | 0.007602 | 0.006830 |
| flat_map_test.test | RUN | 6.764872 | 50.736593 | 31.418120 | 0.007524 | 0.008877 | 0.006855 |
| flat_map_adaptor_test.test | RUN | 4.290413 | 36.701816 | 22.237133 | 0.007068 | 0.008251 | 0.006070 |
| explicit_inst_vector_test.test | RUN | 0.653017 | 5.162321 | 3.499344 | 0.001435 | 0.001857 | 0.001507 |
| explicit_inst_string_test.test | RUN | 0.862166 | 9.224479 | 4.829563 | 0.001364 | 0.001920 | 0.001559 |
| explicit_inst_static_vector_test.test | RUN | 0.086044 | 0.338828 | 0.314146 | 0.001481 | 0.001849 | 0.001528 |
| explicit_inst_stable_vector_test.test | RUN | 0.544198 | 3.330298 | 2.167030 | 0.001317 | 0.001881 | 0.001475 |
| explicit_inst_small_vector_test.test | RUN | 0.433409 | 2.983425 | 1.954680 | 0.001317 | 0.001828 | 0.001503 |
| explicit_inst_slist_test.test | RUN | 0.327635 | 2.666770 | 1.509007 | 0.001452 | 0.001846 | 0.001458 |
| explicit_inst_set_test.test | RUN | 0.464701 | 3.106513 | 1.702096 | 0.001426 | 0.001780 | 0.001486 |
| explicit_inst_map_test.test | RUN | 0.521137 | 3.268710 | 1.862400 | 0.001497 | 0.001874 | 0.001475 |
| explicit_inst_list_test.test | RUN | 0.118669 | 0.743520 | 0.473190 | 0.001389 | 0.001862 | 0.001477 |
| explicit_inst_flat_set_test.test | RUN | 2.744090 | 22.778319 | 11.559951 | 0.001546 | 0.001904 | 0.001539 |
| explicit_inst_flat_map_test.test | RUN | 2.934915 | 23.814802 | 12.784804 | 0.001356 | 0.001787 | 0.001401 |
| explicit_inst_devector_test.test | RUN | 0.364707 | 2.913184 | 1.596919 | 0.001451 | 0.001778 | 0.001470 |
| explicit_inst_deque_test.test | RUN | 0.380909 | 3.131554 | 1.699056 | 0.001552 | 0.001761 | 0.001452 |
| devector_test.test | RUN | 5.338108 | 33.551408 | 20.657377 | 0.002982 | 0.003348 | 0.003498 |
| devector_options_test.test | RUN | 0.221562 | 1.686617 | 0.867601 | 0.001665 | 0.002074 | 0.001694 |
| deque_test.test | RUN | 3.329086 | 23.574669 | 14.434636 | 0.320764 | 0.329626 | 0.305987 |
| deque_options_test.test | RUN | 0.182849 | 0.638599 | 0.607697 | 0.001603 | 0.001980 | 0.001670 |
| copy_move_algo_test.test | RUN | 0.169183 | 0.761613 | 0.550056 | 0.001594 | 0.002028 | 0.001430 |
| common_iterator_test.test | RUN | 0.014160 | 0.042495 | 0.033326 | 0.000552 | 0.000749 | 0.001285 |
| boost_iterator_comp_test.test | RUN | 0.242695 | 0.883633 | 0.837294 | 0.001454 | 0.001891 | 0.001496 |
| allocator_traits_test.test | RUN | 0.142515 | 0.408308 | 0.387744 | 0.001384 | 0.001797 | 0.001544 |
| alloc_full_test.test | RUN | 0.161404 | 0.854008 | 0.584060 | 0.003488 | 0.004206 | 0.003590 |
| alloc_basic_test.test | RUN | 0.133369 | 0.594097 | 0.442799 | 0.001440 | 0.001931 | 0.001603 |
| adaptive_pool_test.test | RUN | 2.050693 | 13.288302 | 8.262674 | 0.007159 | 0.011113 | 0.010505 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.989× | 0.993×, 0.985× | 0.999× |
| clang | 0.969× | 0.988×, 0.951× | 1.008× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409407).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adaptive_pool_test.test | 3 | 0.007222s | 0.006651s | 0.007492s | byte equality observed |
| alloc_basic_test.test | 10 | 0.000895s | 0.000907s | 0.000859s | byte equality observed |
| alloc_full_test.test | 8 | 0.002452s | 0.002444s | 0.002520s | byte equality observed |
| allocator_traits_test.test | 10 | 0.000842s | 0.000846s | 0.000841s | byte equality observed |
| boost_iterator_comp_test.test | 10 | 0.000861s | 0.000878s | 0.000850s | byte equality observed |
| common_iterator_test.test | 10 | 0.000387s | 0.000392s | 0.000837s | byte equality observed |
| copy_move_algo_test.test | 10 | 0.000993s | 0.000932s | 0.000953s | byte equality observed |
| deque_options_test.test | 10 | 0.000901s | 0.000903s | 0.000872s | byte equality observed |
| deque_test.test | 1 | 0.322304s | 0.319922s | 0.322926s | byte equality observed |
| devector_options_test.test | 10 | 0.000955s | 0.000927s | 0.000918s | byte equality observed |
| devector_test.test | 6 | 0.002517s | 0.002571s | 0.003283s | byte equality observed |
| explicit_inst_deque_test.test | 10 | 0.000797s | 0.000796s | 0.000794s | byte equality observed |
| explicit_inst_devector_test.test | 10 | 0.000791s | 0.000793s | 0.000787s | byte equality observed |
| explicit_inst_flat_map_test.test | 10 | 0.000795s | 0.000797s | 0.000789s | byte equality observed |
| explicit_inst_flat_set_test.test | 10 | 0.000797s | 0.000800s | 0.000797s | byte equality observed |
| explicit_inst_list_test.test | 10 | 0.000793s | 0.000778s | 0.000775s | byte equality observed |
| explicit_inst_map_test.test | 10 | 0.000804s | 0.000803s | 0.000789s | byte equality observed |
| explicit_inst_set_test.test | 10 | 0.000798s | 0.000795s | 0.000786s | byte equality observed |
| explicit_inst_slist_test.test | 10 | 0.000822s | 0.000793s | 0.000785s | byte equality observed |
| explicit_inst_small_vector_test.test | 10 | 0.000810s | 0.000794s | 0.000788s | byte equality observed |
| explicit_inst_stable_vector_test.test | 10 | 0.000802s | 0.000797s | 0.000793s | byte equality observed |
| explicit_inst_static_vector_test.test | 10 | 0.000793s | 0.000791s | 0.000793s | byte equality observed |
| explicit_inst_string_test.test | 10 | 0.000805s | 0.000798s | 0.000812s | byte equality observed |
| explicit_inst_vector_test.test | 10 | 0.000793s | 0.000795s | 0.000816s | byte equality observed |
| flat_map_adaptor_test.test | 5 | 0.004661s | 0.004391s | 0.004473s | byte equality observed |
| flat_map_test.test | 4 | 0.005076s | 0.004689s | 0.004552s | byte equality observed |
| flat_set_adaptor_test.test | 5 | 0.004481s | 0.004007s | 0.004253s | byte equality observed |
| flat_set_test.test | 5 | 0.004511s | 0.003898s | 0.004161s | byte equality observed |
| flat_tree_test.test | 10 | 0.000799s | 0.000799s | 0.000804s | byte equality observed |
| global_resource_test.test | 10 | 0.000840s | 0.000846s | 0.000855s | byte equality observed |
| hub_api_test.test | 9 | 0.002124s | 0.001811s | 0.001814s | byte equality observed |
| hub_empty_sort_test.test | 10 | 0.000827s | 0.000805s | 0.000834s | byte equality observed |
| hub_exception_safety_test.test | 1 | 0.051512s | 0.051695s | 0.050265s | byte equality observed |
| hub_new_extended_alignment_test.test | 10 | 0.000817s | 0.000852s | 0.000878s | byte equality observed |
| hub_pocxx_test.test | 10 | 0.000844s | 0.000863s | 0.000877s | byte equality observed |
| hub_sort_test.test | 1 | 0.387004s | 0.381779s | 0.370694s | byte equality observed |
| hub_stability_test.test | 10 | 0.001071s | 0.001058s | 0.001038s | byte equality observed |
| hub_uses_allocator_test.test | 10 | 0.000823s | 0.000841s | 0.000829s | byte equality observed |
| insert_vs_emplace_test.test | 10 | 0.000850s | 0.000870s | 0.000853s | byte equality observed |
| is_constructible_test.test | 10 | 0.000374s | 0.000379s | 0.000794s | byte equality observed |
| list_test.test | 2 | 0.012178s | 0.012052s | 0.012090s | byte equality observed |
| map_test.test | 4 | 0.005964s | 0.005878s | 0.005783s | byte equality observed |
| memory_resource_test.test | 10 | 0.000814s | 0.000821s | 0.000816s | byte equality observed |
| monotonic_buffer_resource_test.test | 10 | 0.000832s | 0.000836s | 0.000840s | byte equality observed |
| nest_api_test.test | 10 | 0.001330s | 0.001371s | 0.001372s | byte equality observed |
| nest_test.test | 10 | 0.000857s | 0.000870s | 0.000848s | byte equality observed |
| new_delete_resource_test.test | 10 | 0.001218s | 0.001230s | 0.001214s | byte equality observed |
| node_allocator_test.test | 4 | 0.005538s | 0.005350s | 0.005411s | byte equality observed |
| node_handle_test.test | 10 | 0.000833s | 0.000842s | 0.000827s | byte equality observed |
| null_iterators_test.test | 10 | 0.000829s | 0.000826s | 0.000815s | byte equality observed |
| pair_test.test | 10 | 0.000816s | 0.000829s | 0.000820s | byte equality observed |
| pmr_deque_test.test | 10 | 0.000800s | 0.000821s | 0.000797s | byte equality observed |
| pmr_devector_test.test | 10 | 0.000802s | 0.000826s | 0.000805s | byte equality observed |
| pmr_flat_map_test.test | 10 | 0.000807s | 0.000811s | 0.000807s | byte equality observed |
| pmr_flat_set_test.test | 10 | 0.000812s | 0.000821s | 0.000793s | byte equality observed |
| pmr_hub_test.test | 10 | 0.000809s | 0.000819s | 0.000803s | byte equality observed |
| pmr_list_test.test | 10 | 0.000804s | 0.000811s | 0.000805s | byte equality observed |
| pmr_map_test.test | 10 | 0.000808s | 0.000831s | 0.000804s | byte equality observed |
| pmr_nest_test.test | 10 | 0.000810s | 0.000826s | 0.000813s | byte equality observed |
| pmr_set_test.test | 10 | 0.000811s | 0.000822s | 0.000810s | byte equality observed |
| pmr_slist_test.test | 10 | 0.000812s | 0.000827s | 0.000811s | byte equality observed |
| pmr_small_vector_test.test | 10 | 0.000812s | 0.000808s | 0.000811s | byte equality observed |
| pmr_stable_vector_test.test | 10 | 0.000820s | 0.000816s | 0.000811s | byte equality observed |
| pmr_string_test.test | 10 | 0.000389s | 0.000386s | 0.000820s | byte equality observed |
| pmr_vector_test.test | 10 | 0.000384s | 0.000389s | 0.000837s | byte equality observed |
| polymorphic_allocator_test.test | 10 | 0.000861s | 0.000880s | 0.000855s | byte equality observed |
| resource_adaptor_test.test | 10 | 0.000868s | 0.000847s | 0.000859s | byte equality observed |
| scoped_allocator_adaptor_test.test | 10 | 0.000870s | 0.000868s | 0.000849s | byte equality observed |
| scoped_allocator_usage_test.test | 10 | 0.000866s | 0.000864s | 0.000847s | byte equality observed |
| segtor_options_test.test | 10 | 0.000883s | 0.000875s | 0.000878s | byte equality observed |
| segtor_test.test | 1 | 0.275695s | 0.259687s | 0.252490s | byte equality observed |
| set_test.test | 3 | 0.008140s | 0.007156s | 0.007969s | byte equality observed |
| slist_test.test | 2 | 0.011967s | 0.011902s | 0.012038s | byte equality observed |
| small_vector_options_test.test | 10 | 0.000892s | 0.000884s | 0.000907s | byte equality observed |
| small_vector_test.test | 8 | 0.001569s | 0.001500s | 0.002597s | byte equality observed |
| stable_vector_test.test | 1 | 0.019634s | 0.019998s | 0.020383s | byte equality observed |
| static_vector_options_test.test | 10 | 0.000869s | 0.000843s | 0.000877s | byte equality observed |
| static_vector_test.test | 10 | 0.001098s | 0.001058s | 0.001057s | byte equality observed |
| string_options_test.test | 10 | 0.000862s | 0.000872s | 0.000855s | byte equality observed |
| string_test.test | 3 | 0.007682s | 0.007934s | 0.007912s | byte equality observed |
| string_view_compat_test.test | 10 | 0.000811s | 0.000843s | 0.000825s | byte equality observed |
| synchronized_pool_resource_test.test | 10 | 0.000852s | 0.000875s | 0.000866s | byte equality observed |
| throw_exception_test.test | 10 | 0.000792s | 0.000818s | 0.000827s | byte equality observed |
| tree_test.test | 10 | 0.000798s | 0.000818s | 0.000814s | byte equality observed |
| unsynchronized_pool_resource_test.test | 10 | 0.000846s | 0.000882s | 0.000874s | byte equality observed |
| uses_allocator_construction_test.test | 10 | 0.000795s | 0.000818s | 0.000817s | byte equality observed |
| uses_allocator_test.test | 10 | 0.000353s | 0.000385s | 0.000824s | byte equality observed |
| vector_options_test.test | 10 | 0.000920s | 0.000935s | 0.000942s | byte equality observed |
| vector_test.test | 5 | 0.002608s | 0.002886s | 0.003658s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409407).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
