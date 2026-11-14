# libs/variant2/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| quick.test | RUN | 0.087268 | 0.204248 | 0.281779 | 0.001275 | 0.000517 | 0.001186 |
| variant_size.test | RUN | 0.160595 | 0.509059 | 0.535554 | 0.001258 | 0.001188 | 0.001235 |
| variant_alternative.test | RUN | 0.214283 | 1.724891 | 0.949396 | 0.001235 | 0.001193 | 0.001237 |
| variant_holds_alternative.test | RUN | 0.150547 | 0.380853 | 0.530938 | 0.001158 | 0.001223 | 0.001253 |
| variant_holds_alternative_cx.test | COMPILE | 0.091682 | 0.215520 | 0.302317 | — | — | — |
| variant_get_by_index.test | RUN | 0.245911 | 1.900161 | 1.174899 | 0.001339 | 0.001425 | 0.001417 |
| variant_get_by_index_cx.test | COMPILE | 0.092282 | 0.232994 | 0.314114 | — | — | — |
| variant_get_by_type.test | RUN | 0.245243 | 2.020142 | 1.252762 | 0.001103 | 0.001218 | 0.001220 |
| variant_get_by_type_cx.test | COMPILE | 0.086871 | 0.198933 | 0.255313 | — | — | — |
| variant_default_construct.test | RUN | 0.237092 | 1.484247 | 1.017699 | 0.001181 | 0.001214 | 0.001185 |
| variant_default_construct_cx.test | COMPILE | 0.095986 | 0.225907 | 0.295395 | — | — | — |
| variant_copy_construct.test | RUN | 0.375716 | 2.514329 | 1.838047 | 0.001081 | 0.001227 | 0.001372 |
| variant_copy_construct_cx.test | COMPILE | 0.108821 | 0.252017 | 0.365674 | — | — | — |
| variant_move_construct.test | RUN | 0.351299 | 2.123285 | 1.675168 | 0.001063 | 0.001170 | 0.001314 |
| variant_move_construct_cx.test | COMPILE | 0.084854 | 0.201097 | 0.260343 | — | — | — |
| variant_value_construct.test | RUN | 0.179181 | 0.525527 | 0.694414 | 0.001085 | 0.001263 | 0.001317 |
| variant_value_construct_cx.test | COMPILE | 0.098043 | 0.232198 | 0.319176 | — | — | — |
| variant_in_place_index_construct.test | RUN | 0.185948 | 0.527155 | 0.721551 | 0.001084 | 0.001289 | 0.001259 |
| variant_in_place_index_construct_cx.test | COMPILE | 0.098577 | 0.231776 | 0.325842 | — | — | — |
| variant_in_place_type_construct.test | RUN | 0.186682 | 0.531468 | 0.681350 | 0.001190 | 0.001182 | 0.001301 |
| variant_in_place_type_construct_cx.test | COMPILE | 0.095332 | 0.226291 | 0.330874 | — | — | — |
| variant_copy_assign.test | RUN | 0.247076 | 1.707925 | 1.117716 | 0.001122 | 0.001288 | 0.001279 |
| variant_copy_assign_cx.test | COMPILE | 0.085110 | 0.204525 | 0.255003 | — | — | — |
| variant_move_assign.test | RUN | 0.248435 | 1.715865 | 1.113808 | 0.001190 | 0.001323 | 0.001263 |
| variant_move_assign_cx.test | COMPILE | 0.084293 | 0.199449 | 0.262780 | — | — | — |
| variant_value_assign.test | RUN | 0.187233 | 0.587687 | 0.756700 | 0.001194 | 0.001379 | 0.000908 |
| variant_value_assign_cx.test | COMPILE | 0.088118 | 0.195363 | 0.263930 | — | — | — |
| variant_emplace_index.test | RUN | 0.219478 | 0.683281 | 0.825105 | 0.001148 | 0.001271 | 0.001318 |
| variant_emplace_index_cx.test | COMPILE | 0.084931 | 0.203829 | 0.260695 | — | — | — |
| variant_emplace_type.test | RUN | 0.195604 | 0.581093 | 0.742334 | 0.001074 | 0.001250 | 0.001250 |
| variant_emplace_type_cx.test | COMPILE | 0.088013 | 0.199482 | 0.269683 | — | — | — |
| variant_swap.test | RUN | 0.203145 | 0.735714 | 0.810625 | 0.001108 | 0.001218 | 0.001236 |
| variant_eq_ne.test | RUN | 0.158286 | 0.445307 | 0.555144 | 0.001100 | 0.001206 | 0.001309 |
| variant_eq_ne_cx.test | COMPILE | 0.085140 | 0.198806 | 0.259293 | — | — | — |
| variant_lt_gt.test | RUN | 0.164251 | 0.500727 | 0.606345 | 0.001166 | 0.001208 | 0.001253 |
| variant_lt_gt_cx.test | COMPILE | 0.087079 | 0.198274 | 0.250837 | — | — | — |
| variant_destroy.test | RUN | 0.164776 | 0.494144 | 0.617125 | 0.001144 | 0.001173 | 0.001216 |
| variant_visit.test | RUN | 0.964728 | 3.810305 | 10.626744 | 0.001205 | 0.001334 | 0.001295 |
| variant_convert_construct.test | RUN | 0.213815 | 0.666920 | 0.876514 | 0.001138 | 0.001225 | 0.001215 |
| variant_subset.test | RUN | 0.216285 | 0.693046 | 0.943734 | 0.001333 | 0.001166 | 0.001267 |
| variant_valueless.test | RUN | 0.161904 | 0.449302 | 0.586218 | 0.001359 | 0.001379 | 0.001220 |
| variant_copy_construct_throw.test | RUN | 0.126402 | 0.337587 | 0.433386 | 0.001106 | 0.001195 | 0.001272 |
| variant_move_construct_throw.test | RUN | 0.129569 | 0.336628 | 0.419638 | 0.001119 | 0.001211 | 0.001249 |
| variant_convert_construct_throw.test | RUN | 0.127783 | 0.368744 | 0.451174 | 0.001078 | 0.001105 | 0.001129 |
| variant_copy_assign_throw.test | RUN | 0.124091 | 0.316912 | 0.422893 | 0.001197 | 0.001192 | 0.001319 |
| variant_move_assign_throw.test | RUN | 0.123577 | 0.316216 | 0.423643 | 0.001198 | 0.001270 | 0.001363 |
| variant_get_by_index_nx.test | RUN | 0.316117 | 1.599127 | 1.109031 | 0.001101 | 0.001181 | 0.001360 |
| variant_get_by_index_cx_nx.test | COMPILE | 0.100133 | 0.235960 | 0.312924 | — | — | — |
| variant_get_by_type_nx.test | RUN | 0.308933 | 1.789302 | 1.210175 | 0.001257 | 0.001266 | 0.001495 |
| variant_get_by_type_cx_nx.test | COMPILE | 0.086025 | 0.202503 | 0.259338 | — | — | — |
| variant_subset_nx.test | RUN | 0.225601 | 0.677862 | 0.935809 | 0.001303 | 0.001192 | 0.001366 |
| variant_hash.test | RUN | 0.226558 | 0.800726 | 0.829360 | 0.001374 | 0.001319 | 0.001359 |
| variant_trivial.test | RUN | 1.278004 | 4.502896 | 6.177493 | 0.001149 | 0.001500 | 0.001229 |
| variant_special.test | RUN | 1.248297 | 4.945921 | 6.565098 | 0.001138 | 0.001421 | 0.001463 |
| variant_visit_derived.test | RUN | 0.155868 | 0.418393 | 0.633006 | 0.001278 | 0.001169 | 0.001382 |
| variant_many_types.test | RUN | 1.313336 | 6.001396 | 8.849010 | 0.001070 | 0.001216 | 0.001178 |
| variant_visit_r.test | RUN | 0.528344 | 1.930477 | 3.418620 | 0.001224 | 0.001252 | 0.001128 |
| variant_derived_construct.test | COMPILE | 0.091108 | 0.209757 | 0.262710 | — | — | — |
| variant_visit_by_index.test | RUN | 0.197696 | 0.529928 | 0.815808 | 0.001169 | 0.001236 | 0.001244 |
| variant_ostream_insert.test | RUN | 0.144319 | 0.444628 | 0.546371 | 0.001147 | 0.001157 | 0.001352 |
| is_output_streamable.test | RUN | 0.188798 | 0.923586 | 0.744681 | 0.001012 | 0.001253 | 0.001224 |
| variant_json_value_from.test | RUN | 0.279703 | 0.729738 | 0.951092 | 0.001292 | 0.001217 | 0.001303 |
| variant_json_value_to.test | RUN | 0.350832 | 1.000021 | 1.274177 | 0.001267 | 0.001288 | 0.001285 |
| variant_uses_double_storage.test | COMPILE | 0.094196 | 0.212601 | 0.303354 | — | — | — |
| variant_derived_construct2.test | RUN | 0.126705 | 0.353888 | 0.454417 | 0.001037 | 0.001180 | 0.001284 |
| variant_visit_cx.test | COMPILE | 0.088694 | 0.201467 | 0.241381 | — | — | — |
| variant_visit_cx_2.test | COMPILE | 0.084941 | 0.200480 | 0.263453 | — | — | — |
| variant_visit_r_cx.test | COMPILE | 0.084535 | 0.200711 | 0.261329 | — | — | — |
| variant_index_type_126.test | RUN | 0.793665 | 1.937128 | 3.836507 | 0.001264 | 0.001159 | 0.001470 |
| variant_index_type_127.test | RUN | 0.801880 | 1.940627 | 3.976865 | 0.001154 | 0.001220 | 0.001339 |
| variant_index_type_128.test | RUN | 0.806929 | 1.975283 | 3.995294 | 0.001108 | 0.001158 | 0.001270 |
| variant_index_type_129.test | RUN | 0.818465 | 1.901934 | 3.987030 | 0.001250 | 0.001332 | 0.001076 |
| variant_index_type_254.test | RUN | 2.999356 | 7.618693 | 12.708662 | 0.001193 | 0.001261 | 0.001388 |
| variant_index_type_255.test | RUN | 3.039883 | 7.672020 | 12.735862 | 0.001161 | 0.001376 | 0.001159 |
| variant_index_type_256.test | RUN | 3.050637 | 7.970494 | 12.695984 | 0.001047 | 0.001216 | 0.001209 |
| variant_index_type_257.test | RUN | 3.052441 | 8.094267 | 12.861633 | 0.001108 | 0.001098 | 0.001421 |
| variant_default_construct_cx_2.test | COMPILE | 0.085390 | 0.192023 | 0.242247 | — | — | — |
| variant_default_construct_cx_3.test | COMPILE | 0.089337 | 0.210802 | 0.263906 | — | — | — |
| variant_default_construct_cx_4.test | COMPILE | 0.085155 | 0.213975 | 0.259431 | — | — | — |
| variant_default_construct_cx_5.test | COMPILE | 0.084652 | 0.193159 | 0.243885 | — | — | — |
| variant_value_construct_cx_2.test | COMPILE | 0.083202 | 0.197621 | 0.260428 | — | — | — |
| variant_value_construct_cx_3.test | COMPILE | 0.084180 | 0.193597 | 0.256702 | — | — | — |
| variant_value_construct_cx_4.test | COMPILE | 0.081180 | 0.143968 | 0.245196 | — | — | — |
| variant_issue_55.test | RUN | 0.145147 | 0.389913 | 0.468493 | 0.000882 | 0.000854 | 0.000856 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.985× | 0.988×, 0.981× | 1.001× |
| clang | 0.990× | 0.991×, 0.989× | 0.997× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814672).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| is_output_streamable.test | 10 | 0.000800s | 0.000824s | 0.000804s | byte equality observed |
| quick.test | 10 | 0.000782s | 0.000362s | 0.000792s | byte equality observed |
| variant_alternative.test | 10 | 0.000809s | 0.000798s | 0.000804s | byte equality observed |
| variant_convert_construct.test | 10 | 0.000818s | 0.000795s | 0.000811s | byte equality observed |
| variant_convert_construct_throw.test | 10 | 0.000846s | 0.000829s | 0.000823s | byte equality observed |
| variant_copy_assign.test | 10 | 0.000811s | 0.000847s | 0.000809s | byte equality observed |
| variant_copy_assign_throw.test | 10 | 0.000826s | 0.000837s | 0.000836s | byte equality observed |
| variant_copy_construct.test | 10 | 0.000847s | 0.000823s | 0.000834s | byte equality observed |
| variant_copy_construct_throw.test | 10 | 0.000852s | 0.000824s | 0.000859s | byte equality observed |
| variant_default_construct.test | 10 | 0.000824s | 0.000806s | 0.000801s | byte equality observed |
| variant_derived_construct2.test | 10 | 0.000836s | 0.000800s | 0.000810s | byte equality observed |
| variant_destroy.test | 10 | 0.000842s | 0.000815s | 0.000788s | byte equality observed |
| variant_emplace_index.test | 10 | 0.000841s | 0.000837s | 0.000820s | byte equality observed |
| variant_emplace_type.test | 10 | 0.000825s | 0.000831s | 0.000807s | byte equality observed |
| variant_eq_ne.test | 10 | 0.000793s | 0.000790s | 0.000826s | byte equality observed |
| variant_get_by_index.test | 10 | 0.000835s | 0.000865s | 0.000858s | byte equality observed |
| variant_get_by_index_nx.test | 10 | 0.000814s | 0.000809s | 0.000801s | byte equality observed |
| variant_get_by_type.test | 10 | 0.000839s | 0.000880s | 0.000830s | byte equality observed |
| variant_get_by_type_nx.test | 10 | 0.000840s | 0.000802s | 0.000830s | byte equality observed |
| variant_hash.test | 10 | 0.000815s | 0.000814s | 0.000805s | byte equality observed |
| variant_holds_alternative.test | 10 | 0.000827s | 0.000830s | 0.000791s | byte equality observed |
| variant_in_place_index_construct.test | 10 | 0.000824s | 0.000817s | 0.000811s | byte equality observed |
| variant_in_place_type_construct.test | 10 | 0.000806s | 0.000815s | 0.000826s | byte equality observed |
| variant_index_type_126.test | 10 | 0.000814s | 0.000832s | 0.000785s | byte equality observed |
| variant_index_type_127.test | 10 | 0.000816s | 0.000836s | 0.000809s | byte equality observed |
| variant_index_type_128.test | 10 | 0.000824s | 0.000809s | 0.000801s | byte equality observed |
| variant_index_type_129.test | 10 | 0.000803s | 0.000819s | 0.000812s | byte equality observed |
| variant_index_type_254.test | 10 | 0.000801s | 0.000785s | 0.000801s | byte equality observed |
| variant_index_type_255.test | 10 | 0.000805s | 0.000826s | 0.000799s | byte equality observed |
| variant_index_type_256.test | 10 | 0.000794s | 0.000835s | 0.000812s | byte equality observed |
| variant_index_type_257.test | 10 | 0.000807s | 0.000809s | 0.000820s | byte equality observed |
| variant_issue_55.test | 10 | 0.000828s | 0.000819s | 0.000839s | byte equality observed |
| variant_json_value_from.test | 10 | 0.000866s | 0.000834s | 0.000809s | byte equality observed |
| variant_json_value_to.test | 10 | 0.000849s | 0.000858s | 0.000805s | byte equality observed |
| variant_lt_gt.test | 10 | 0.000793s | 0.000836s | 0.000816s | byte equality observed |
| variant_many_types.test | 10 | 0.000819s | 0.000812s | 0.000830s | byte equality observed |
| variant_move_assign.test | 10 | 0.000825s | 0.000810s | 0.000811s | byte equality observed |
| variant_move_assign_throw.test | 10 | 0.000839s | 0.000837s | 0.000816s | byte equality observed |
| variant_move_construct.test | 10 | 0.000843s | 0.000833s | 0.000807s | byte equality observed |
| variant_move_construct_throw.test | 10 | 0.000828s | 0.000843s | 0.000814s | byte equality observed |
| variant_ostream_insert.test | 10 | 0.000840s | 0.000844s | 0.000801s | byte equality observed |
| variant_size.test | 10 | 0.000811s | 0.000805s | 0.000782s | byte equality observed |
| variant_special.test | 10 | 0.000873s | 0.000857s | 0.000810s | byte equality observed |
| variant_subset.test | 10 | 0.000859s | 0.000823s | 0.000814s | byte equality observed |
| variant_subset_nx.test | 10 | 0.000804s | 0.000814s | 0.000775s | byte equality observed |
| variant_swap.test | 10 | 0.000815s | 0.000824s | 0.000792s | byte equality observed |
| variant_trivial.test | 10 | 0.000843s | 0.000853s | 0.000829s | byte equality observed |
| variant_value_assign.test | 10 | 0.000837s | 0.000807s | 0.000840s | byte equality observed |
| variant_value_construct.test | 10 | 0.000838s | 0.000822s | 0.000820s | byte equality observed |
| variant_valueless.test | 10 | 0.000857s | 0.000847s | 0.000816s | byte equality observed |
| variant_visit.test | 10 | 0.000832s | 0.000829s | 0.000836s | byte equality observed |
| variant_visit_by_index.test | 10 | 0.000822s | 0.000825s | 0.000804s | byte equality observed |
| variant_visit_derived.test | 10 | 0.000799s | 0.000794s | 0.000823s | byte equality observed |
| variant_visit_r.test | 10 | 0.000777s | 0.000789s | 0.000782s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814672).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
