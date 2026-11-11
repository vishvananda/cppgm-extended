# libs/variant2/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| quick.test | RUN | 0.092205 | 0.171689 | 0.216918 | 0.001438 | 0.001499 | 0.001449 |
| variant_size.test | RUN | 0.150221 | 0.338582 | 0.421583 | 0.001423 | 0.001683 | 0.001500 |
| variant_alternative.test | RUN | 0.161189 | 0.603464 | 0.555698 | 0.001326 | 0.001622 | 0.001559 |
| variant_holds_alternative.test | RUN | 0.144329 | 0.325078 | 0.422470 | 0.001332 | 0.001320 | 0.001344 |
| variant_holds_alternative_cx.test | COMPILE | 0.099225 | 0.182419 | 0.254586 | — | — | — |
| variant_get_by_index.test | RUN | 0.177276 | 0.719520 | 0.711316 | 0.001582 | 0.001703 | 0.001583 |
| variant_get_by_index_cx.test | COMPILE | 0.098887 | 0.195494 | 0.261225 | — | — | — |
| variant_get_by_type.test | RUN | 0.183048 | 0.763614 | 0.714948 | 0.001384 | 0.001484 | 0.001449 |
| variant_get_by_type_cx.test | COMPILE | 0.090711 | 0.168213 | 0.220277 | — | — | — |
| variant_default_construct.test | RUN | 0.189303 | 0.644593 | 0.674635 | 0.001468 | 0.001661 | 0.001519 |
| variant_default_construct_cx.test | COMPILE | 0.098043 | 0.181976 | 0.246652 | — | — | — |
| variant_copy_construct.test | RUN | 0.243653 | 1.349979 | 1.120626 | 0.001503 | 0.001631 | 0.001335 |
| variant_copy_construct_cx.test | COMPILE | 0.112594 | 0.217970 | 0.308467 | — | — | — |
| variant_move_construct.test | RUN | 0.246501 | 1.214292 | 1.078899 | 0.001343 | 0.001510 | 0.001407 |
| variant_move_construct_cx.test | COMPILE | 0.093502 | 0.167026 | 0.216524 | — | — | — |
| variant_value_construct.test | RUN | 0.164053 | 0.429698 | 0.509402 | 0.001264 | 0.001608 | 0.001418 |
| variant_value_construct_cx.test | COMPILE | 0.105473 | 0.197996 | 0.267920 | — | — | — |
| variant_in_place_index_construct.test | RUN | 0.163046 | 0.431138 | 0.515717 | 0.001425 | 0.001696 | 0.001504 |
| variant_in_place_index_construct_cx.test | COMPILE | 0.100912 | 0.204038 | 0.263750 | — | — | — |
| variant_in_place_type_construct.test | RUN | 0.166125 | 0.424788 | 0.502248 | 0.001406 | 0.001655 | 0.001485 |
| variant_in_place_type_construct_cx.test | COMPILE | 0.102099 | 0.195952 | 0.265841 | — | — | — |
| variant_copy_assign.test | RUN | 0.191089 | 0.719586 | 0.716950 | 0.001521 | 0.001577 | 0.001471 |
| variant_copy_assign_cx.test | COMPILE | 0.092182 | 0.168459 | 0.222191 | — | — | — |
| variant_move_assign.test | RUN | 0.195497 | 0.707512 | 0.702747 | 0.001489 | 0.001642 | 0.001235 |
| variant_move_assign_cx.test | COMPILE | 0.089810 | 0.162687 | 0.218538 | — | — | — |
| variant_value_assign.test | RUN | 0.174005 | 0.480261 | 0.543921 | 0.001530 | 0.001658 | 0.001525 |
| variant_value_assign_cx.test | COMPILE | 0.092609 | 0.166597 | 0.221807 | — | — | — |
| variant_emplace_index.test | RUN | 0.178185 | 0.540480 | 0.581813 | 0.001568 | 0.001502 | 0.001498 |
| variant_emplace_index_cx.test | COMPILE | 0.092482 | 0.166306 | 0.203126 | — | — | — |
| variant_emplace_type.test | RUN | 0.169833 | 0.466382 | 0.546948 | 0.001566 | 0.001488 | 0.001524 |
| variant_emplace_type_cx.test | COMPILE | 0.093694 | 0.162759 | 0.216835 | — | — | — |
| variant_swap.test | RUN | 0.169315 | 0.501781 | 0.557141 | 0.001533 | 0.001522 | 0.001398 |
| variant_eq_ne.test | RUN | 0.156553 | 0.358794 | 0.445814 | 0.001341 | 0.001418 | 0.001479 |
| variant_eq_ne_cx.test | COMPILE | 0.090445 | 0.167564 | 0.224893 | — | — | — |
| variant_lt_gt.test | RUN | 0.158240 | 0.367675 | 0.458068 | 0.001531 | 0.001653 | 0.001585 |
| variant_lt_gt_cx.test | COMPILE | 0.091503 | 0.160916 | 0.216391 | — | — | — |
| variant_destroy.test | RUN | 0.158171 | 0.394683 | 0.466158 | 0.001222 | 0.001528 | 0.001357 |
| variant_visit.test | RUN | 0.645867 | 5.079538 | 4.324995 | 0.001451 | 0.001746 | 0.001660 |
| variant_convert_construct.test | RUN | 0.181914 | 0.595843 | 0.631849 | 0.001566 | 0.001571 | 0.001499 |
| variant_subset.test | RUN | 0.185275 | 0.674145 | 0.653611 | 0.001470 | 0.001446 | 0.001532 |
| variant_valueless.test | RUN | 0.148523 | 0.423794 | 0.502490 | 0.001503 | 0.001476 | 0.001350 |
| variant_copy_construct_throw.test | RUN | 0.130105 | 0.252975 | 0.340232 | 0.001479 | 0.001554 | 0.001524 |
| variant_move_construct_throw.test | RUN | 0.126418 | 0.256443 | 0.339273 | 0.001494 | 0.001573 | 0.001407 |
| variant_convert_construct_throw.test | RUN | 0.129952 | 0.285153 | 0.355537 | 0.001197 | 0.001516 | 0.001515 |
| variant_copy_assign_throw.test | RUN | 0.128935 | 0.263350 | 0.343666 | 0.001160 | 0.001773 | 0.001502 |
| variant_move_assign_throw.test | RUN | 0.126152 | 0.255554 | 0.317602 | 0.001518 | 0.001390 | 0.001525 |
| variant_get_by_index_nx.test | RUN | 0.205524 | 0.632065 | 0.691517 | 0.001440 | 0.001644 | 0.001528 |
| variant_get_by_index_cx_nx.test | COMPILE | 0.099655 | 0.191132 | 0.242191 | — | — | — |
| variant_get_by_type_nx.test | RUN | 0.206201 | 0.735233 | 0.704508 | 0.001451 | 0.001597 | 0.001533 |
| variant_get_by_type_cx_nx.test | COMPILE | 0.090839 | 0.167032 | 0.223682 | — | — | — |
| variant_subset_nx.test | RUN | 0.206229 | 0.658186 | 0.691458 | 0.001413 | 0.001529 | 0.001546 |
| variant_hash.test | RUN | 0.212681 | 0.568562 | 0.629330 | 0.001616 | 0.001629 | 0.001522 |
| variant_trivial.test | RUN | 1.140591 | 3.426456 | 4.924791 | 0.001754 | 0.001643 | 0.001526 |
| variant_special.test | RUN | 1.093896 | 3.486529 | 4.894619 | 0.001530 | 0.001850 | 0.001422 |
| variant_visit_derived.test | RUN | 0.147800 | 0.369221 | 0.477318 | 0.001664 | 0.001672 | 0.001514 |
| variant_many_types.test | RUN | 0.945824 | 11.022959 | 7.088123 | 0.001782 | 0.001834 | 0.001683 |
| variant_visit_r.test | RUN | 0.341742 | 2.418585 | 2.194049 | 0.001562 | 0.001456 | 0.001565 |
| variant_derived_construct.test | COMPILE | 0.097911 | 0.167176 | 0.232455 | — | — | — |
| variant_visit_by_index.test | RUN | 0.190279 | 0.548818 | 0.620098 | 0.001144 | 0.001423 | 0.001399 |
| variant_ostream_insert.test | RUN | 0.127956 | 0.323996 | 0.396588 | 0.001646 | 0.001563 | 0.001300 |
| is_output_streamable.test | RUN | 0.162562 | 0.454961 | 0.506240 | 0.001184 | 0.001479 | 0.001502 |
| variant_json_value_from.test | RUN | 0.259630 | 0.611729 | 0.743844 | 0.001385 | 0.001724 | 0.001450 |
| variant_json_value_to.test | RUN | 0.289742 | 0.819728 | 0.932991 | 0.001417 | 0.001654 | 0.001585 |
| variant_uses_double_storage.test | COMPILE | 0.098759 | 0.176771 | 0.252296 | — | — | — |
| variant_derived_construct2.test | RUN | 0.122879 | 0.277237 | 0.322351 | 0.001454 | 0.001630 | 0.001461 |
| variant_visit_cx.test | COMPILE | 0.091961 | 0.168188 | 0.222496 | — | — | — |
| variant_visit_cx_2.test | COMPILE | 0.090275 | 0.169322 | 0.233047 | — | — | — |
| variant_visit_r_cx.test | COMPILE | 0.087408 | 0.172302 | 0.197646 | — | — | — |
| variant_index_type_126.test | RUN | 0.768901 | 1.669559 | 3.181446 | 0.001413 | 0.001539 | 0.001361 |
| variant_index_type_127.test | RUN | 0.777243 | 1.811361 | 3.206566 | 0.001363 | 0.001559 | 0.001179 |
| variant_index_type_128.test | RUN | 0.820861 | 1.746892 | 3.111000 | 0.001437 | 0.001575 | 0.001530 |
| variant_index_type_129.test | RUN | 0.811273 | 1.749262 | 3.134839 | 0.001177 | 0.001584 | 0.001569 |
| variant_index_type_254.test | RUN | 2.807633 | 6.319143 | 10.086722 | 0.001442 | 0.001391 | 0.001376 |
| variant_index_type_255.test | RUN | 2.808966 | 6.470597 | 10.190481 | 0.001391 | 0.001366 | 0.001504 |
| variant_index_type_256.test | RUN | 2.850916 | 6.325398 | 10.234482 | 0.001416 | 0.001152 | 0.001507 |
| variant_index_type_257.test | RUN | 2.877766 | 5.186231 | 9.898618 | 0.001371 | 0.001105 | 0.001087 |
| variant_default_construct_cx_2.test | COMPILE | 0.090744 | 0.159051 | 0.209190 | — | — | — |
| variant_default_construct_cx_3.test | COMPILE | 0.091071 | 0.158420 | 0.204059 | — | — | — |
| variant_default_construct_cx_4.test | COMPILE | 0.090694 | 0.153867 | 0.206353 | — | — | — |
| variant_default_construct_cx_5.test | COMPILE | 0.089807 | 0.155420 | 0.204736 | — | — | — |
| variant_value_construct_cx_2.test | COMPILE | 0.089896 | 0.159592 | 0.211502 | — | — | — |
| variant_value_construct_cx_3.test | COMPILE | 0.080531 | 0.161565 | 0.203130 | — | — | — |
| variant_value_construct_cx_4.test | COMPILE | 0.081857 | 0.155779 | 0.207101 | — | — | — |
| variant_issue_55.test | RUN | 0.111825 | 0.397061 | 0.312815 | 0.001103 | 0.001669 | 0.001135 |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872153) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872153) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872153).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
