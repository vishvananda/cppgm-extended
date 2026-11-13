# libs/fusion/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| all.test | RUN | 0.252102 | 0.579601 | 0.641797 | 0.001127 | 0.001484 | 0.001197 |
| any.test | RUN | 0.240998 | 0.556896 | 0.607137 | 0.001262 | 0.001504 | 0.001304 |
| clear.test | RUN | 0.177413 | 0.518579 | 0.505683 | 0.001321 | 0.001675 | 0.001333 |
| copy.test | RUN | 0.149940 | 0.380684 | 0.408678 | 0.001359 | 0.001623 | 0.001444 |
| count.test | RUN | 0.167041 | 0.429390 | 0.474302 | 0.001310 | 0.001528 | 0.001481 |
| count_if.test | RUN | 0.155707 | 0.394973 | 0.432643 | 0.001495 | 0.001502 | 0.001230 |
| erase.test | RUN | 0.229077 | 0.722777 | 0.712056 | 0.001486 | 0.001690 | 0.001262 |
| erase_key.test | RUN | 0.220097 | 0.711309 | 0.666445 | 0.001227 | 0.001558 | 0.001266 |
| filter.test | RUN | 0.194020 | 0.583169 | 0.606009 | 0.001542 | 0.001719 | 0.001531 |
| filter_if.test | RUN | 0.217085 | 0.692684 | 0.699440 | 0.001364 | 0.001802 | 0.001568 |
| find.test | RUN | 0.190891 | 0.490596 | 0.530547 | 0.001206 | 0.001597 | 0.001248 |
| find_if.test | RUN | 0.185630 | 0.441876 | 0.491258 | 0.001470 | 0.001749 | 0.001356 |
| fold.test | RUN | 0.197532 | 0.544127 | 0.585280 | 0.001762 | 0.001744 | 0.001396 |
| for_each.test | RUN | 0.178560 | 0.495600 | 0.517832 | 0.001396 | 0.001755 | 0.001519 |
| insert.test | RUN | 0.265532 | 0.870052 | 0.837721 | 0.001382 | 0.001788 | 0.001425 |
| insert_range.test | RUN | 0.255705 | 0.889657 | 0.842786 | 0.001448 | 0.001578 | 0.001367 |
| iter_fold.test | RUN | 0.305056 | 0.838019 | 0.991742 | 0.001440 | 0.001786 | 0.001505 |
| move.test | RUN | 0.156653 | 0.403212 | 0.437326 | 0.001526 | 0.001534 | 0.001431 |
| none.test | RUN | 0.260061 | 0.571021 | 0.655207 | 0.001170 | 0.001730 | 0.001259 |
| pop_back.test | RUN | 0.241505 | 0.769957 | 0.752072 | 0.001458 | 0.001732 | 0.001221 |
| pop_front.test | RUN | 0.207810 | 0.630342 | 0.610311 | 0.001656 | 0.001595 | 0.001362 |
| push_back.test | RUN | 0.253326 | 0.879718 | 0.831739 | 0.001599 | 0.001838 | 0.001413 |
| push_front.test | RUN | 0.223266 | 0.734754 | 0.716326 | 0.001426 | 0.001730 | 0.001345 |
| remove.test | RUN | 0.249381 | 0.840018 | 0.827843 | 0.001314 | 0.001741 | 0.001463 |
| remove_if.test | RUN | 0.231408 | 0.698905 | 0.707041 | 0.001449 | 0.001602 | 0.001295 |
| replace.test | RUN | 0.210807 | 0.690689 | 0.656323 | 0.001461 | 0.001784 | 0.001378 |
| replace_if.test | RUN | 0.209090 | 0.674102 | 0.655475 | 0.001596 | 0.001792 | 0.001513 |
| reverse_fold.test | RUN | 0.345583 | 0.954274 | 1.092974 | 0.001470 | 0.001628 | 0.001430 |
| reverse_iter_fold.test | RUN | 0.352684 | 0.946105 | 1.156707 | 0.001561 | 0.001672 | 0.001419 |
| reverse.test | RUN | 0.214413 | 0.715107 | 0.663394 | 0.001508 | 0.001735 | 0.001202 |
| segmented_for_each.test | RUN | 0.371458 | 1.285780 | 1.331855 | 0.001991 | 0.001512 | 0.001432 |
| segmented_find.test | RUN | 0.224634 | 0.673233 | 0.745563 | 0.001340 | 0.001465 | 0.001549 |
| segmented_find_if.test | RUN | 0.229623 | 0.697404 | 0.759238 | 0.001398 | 0.001651 | 0.001472 |
| segmented_fold.test | RUN | 0.288518 | 0.892041 | 0.921706 | 0.001651 | 0.001802 | 0.001473 |
| transform.test | RUN | 0.227839 | 0.746669 | 0.694659 | 0.001396 | 0.001859 | 0.001529 |
| join.test | RUN | 0.167270 | 0.434107 | 0.492521 | 0.001587 | 0.001705 | 0.001506 |
| zip.test | RUN | 0.299119 | 0.843260 | 0.927314 | 0.001431 | 0.001732 | 0.001490 |
| zip2.test | RUN | 0.277337 | 0.775373 | 0.881208 | 0.001353 | 0.001748 | 0.001332 |
| zip_ignore.test | RUN | 0.220283 | 0.570789 | 0.592077 | 0.001327 | 0.001512 | 0.001360 |
| flatten.test | RUN | 0.200727 | 0.613072 | 0.603660 | 0.001186 | 0.001776 | 0.001339 |
| ticket-5490.test | COMPILE | 0.107414 | 0.262551 | 0.277784 | — | — | — |
| as_deque.test | RUN | 0.196145 | 0.566737 | 0.640522 | 0.001362 | 0.001745 | 0.001503 |
| as_list.test | RUN | 0.228441 | 0.705973 | 0.702993 | 0.001529 | 0.001462 | 0.001291 |
| as_map.test | RUN | 0.224375 | 0.688251 | 0.705497 | 0.001158 | 0.001559 | 0.001334 |
| as_map_assoc.test | RUN | 0.219986 | 0.659293 | 0.661445 | 0.001159 | 0.001694 | 0.001495 |
| as_set.test | RUN | 0.246461 | 0.833978 | 0.814343 | 0.001382 | 0.001618 | 0.001408 |
| as_vector.test | RUN | 0.228886 | 0.750508 | 0.720228 | 0.001167 | 0.001480 | 0.001286 |
| boost_tuple.test | RUN | 0.229041 | 0.724255 | 0.710404 | 0.001434 | 0.001553 | 0.001447 |
| boost_tuple_iterator.test | RUN | 0.157166 | 0.436934 | 0.435401 | 0.001314 | 0.001442 | 0.001240 |
| cons.test | RUN | 0.285505 | 0.822132 | 0.828267 | 0.001135 | 0.001617 | 0.001307 |
| convert_boost_tuple.test | RUN | 0.179538 | 0.626479 | 0.557914 | 0.001406 | 0.001646 | 0.001369 |
| convert_deque.test | RUN | 0.182072 | 0.628583 | 0.562048 | 0.001258 | 0.001548 | 0.001223 |
| convert_list.test | RUN | 0.183305 | 0.641153 | 0.562045 | 0.001366 | 0.001516 | 0.001481 |
| convert_std_pair.test | RUN | 0.185292 | 0.631569 | 0.563955 | 0.001663 | 0.001492 | 0.001427 |
| convert_std_tuple.test | RUN | 0.179672 | 0.622564 | 0.545172 | 0.001200 | 0.001523 | 0.001240 |
| convert_vector.test | RUN | 0.179683 | 0.621077 | 0.555865 | 0.001365 | 0.001428 | 0.001440 |
| filter_view.test | RUN | 0.219772 | 0.662633 | 0.653309 | 0.001237 | 0.001674 | 0.001519 |
| hash.test | RUN | 0.213502 | 0.585027 | 0.611647 | 0.001636 | 0.001751 | 0.001524 |
| io.test | RUN | 0.227731 | 0.850296 | 0.762700 | 0.001323 | 0.001986 | 0.001578 |
| iterator_range.test | RUN | 0.208361 | 0.630453 | 0.645105 | 0.001415 | 0.001661 | 0.001401 |
| joint_view.test | RUN | 0.276979 | 1.034149 | 0.913228 | 0.001688 | 0.001371 | 0.001522 |
| list_comparison.test | RUN | 0.160387 | 0.438399 | 0.449527 | 0.001498 | 0.001659 | 0.001500 |
| list_construction.test | RUN | 0.158824 | 0.438609 | 0.436025 | 0.001367 | 0.001673 | 0.001469 |
| list_copy.test | RUN | 0.240322 | 0.762884 | 0.751210 | 0.001476 | 0.001396 | 0.001429 |
| list_iterator.test | RUN | 0.159710 | 0.442155 | 0.417733 | 0.001311 | 0.001464 | 0.001324 |
| list_hash.test | RUN | 0.220939 | 0.822726 | 0.627750 | 0.001327 | 0.001667 | 0.001322 |
| list_make.test | RUN | 0.164951 | 0.468674 | 0.475036 | 0.001494 | 0.001598 | 0.001343 |
| list_misc.test | RUN | 0.219505 | 0.545710 | 0.576266 | 0.001315 | 0.001686 | 0.001489 |
| list_mutate.test | RUN | 0.151344 | 0.377709 | 0.390344 | 0.001485 | 0.001552 | 0.001371 |
| list_nest.test | RUN | 0.286036 | 1.031801 | 1.046239 | 0.001210 | 0.001504 | 0.001356 |
| list_tie.test | RUN | 0.156394 | 0.431129 | 0.421851 | 0.001406 | 0.001807 | 0.001428 |
| list_value_at.test | RUN | 0.156234 | 0.389074 | 0.427630 | 0.001336 | 0.001863 | 0.001379 |
| deque_comparison.test | RUN | 0.157347 | 0.438834 | 0.445604 | 0.001253 | 0.001576 | 0.001404 |
| deque_construction.test | RUN | 0.157917 | 0.422255 | 0.463096 | 0.001335 | 0.001665 | 0.001239 |
| deque_copy.test | RUN | 0.223981 | 0.708177 | 0.702939 | 0.001317 | 0.001561 | 0.001437 |
| deque_iterator.test | RUN | 0.157130 | 0.458606 | 0.444772 | 0.001508 | 0.001568 | 0.001483 |
| deque_hash.test | RUN | 0.214443 | 0.794232 | 0.623884 | 0.001273 | 0.001782 | 0.001515 |
| deque_is_constructible.test | COMPILE | 0.085519 | 0.201047 | 0.200655 | — | — | — |
| deque_make.test | RUN | 0.159474 | 0.443200 | 0.469213 | 0.001520 | 0.001680 | 0.001406 |
| deque_misc.test | RUN | 0.207556 | 0.561151 | 0.549402 | 0.001280 | 0.001544 | 0.001399 |
| deque_move.test | RUN | 0.161306 | 0.466902 | 0.457047 | 0.001293 | 0.001622 | 0.001500 |
| deque_mutate.test | RUN | 0.145736 | 0.373631 | 0.396518 | 0.001424 | 0.001673 | 0.001421 |
| deque_nest.test | RUN | 0.444264 | 1.642303 | 1.719895 | 0.001569 | 0.001431 | 0.001279 |
| deque_tie.test | RUN | 0.149971 | 0.405912 | 0.407356 | 0.001363 | 0.001473 | 0.001461 |
| deque_value_at.test | RUN | 0.153407 | 0.376355 | 0.421415 | 0.001370 | 0.001666 | 0.001381 |
| front_extended_deque.test | RUN | 0.215843 | 0.569648 | 0.585860 | 0.001398 | 0.001771 | 0.001525 |
| back_extended_deque.test | RUN | 0.218985 | 0.555303 | 0.636717 | 0.001297 | 0.001743 | 0.001480 |
| make_list.test | RUN | 0.171340 | 0.488870 | 0.469735 | 0.001443 | 0.001741 | 0.001380 |
| make_vector.test | RUN | 0.172344 | 0.464550 | 0.461362 | 0.001387 | 0.001708 | 0.001478 |
| map.test | RUN | 0.214485 | 0.699495 | 0.672532 | 0.001507 | 0.001717 | 0.001432 |
| map_comparison.test | RUN | 0.164094 | 0.442781 | 0.445800 | 0.001332 | 0.001741 | 0.001457 |
| map_construction.test | RUN | 0.161808 | 0.441090 | 0.468361 | 0.001157 | 0.001512 | 0.001183 |
| map_copy.test | RUN | 0.202357 | 0.567863 | 0.554675 | 0.001427 | 0.001468 | 0.001434 |
| map_misc.test | RUN | 0.208891 | 0.553785 | 0.565780 | 0.001428 | 0.001812 | 0.001347 |
| map_move.test | RUN | 0.166492 | 0.470056 | 0.459876 | 0.001304 | 0.001480 | 0.001197 |
| map_mutate.test | RUN | 0.148437 | 0.383954 | 0.393331 | 0.001088 | 0.001458 | 0.001261 |
| map_tie.test | RUN | 0.159111 | 0.380793 | 0.402255 | 0.001457 | 0.001523 | 0.001438 |
| nil.test | RUN | 0.104872 | 0.266210 | 0.283621 | 0.001490 | 0.001751 | 0.001421 |
| nview.test | RUN | 0.204698 | 0.552967 | 0.586920 | 0.001506 | 0.001788 | 0.001511 |
| reverse_view.test | RUN | 0.255822 | 0.889284 | 0.851172 | 0.001518 | 0.001660 | 0.001526 |
| segmented_iterator_range.test | RUN | 0.326287 | 1.048363 | 1.086984 | 0.001603 | 0.001666 | 0.001368 |
| set.test | RUN | 0.196161 | 0.615731 | 0.601603 | 0.001424 | 0.001576 | 0.001495 |
| single_view.test | RUN | 0.180873 | 0.556117 | 0.560727 | 0.001361 | 0.001853 | 0.001470 |
| std_pair.test | RUN | 0.237664 | 0.713826 | 0.690664 | 0.001674 | 0.001644 | 0.001229 |
| boost_array.test | RUN | 0.198887 | 0.493854 | 0.536188 | 0.001373 | 0.001449 | 0.001196 |
| array.test | RUN | 0.189621 | 0.496041 | 0.518354 | 0.001257 | 0.001306 | 0.001371 |
| std_array.test | RUN | 0.193989 | 0.508345 | 0.511323 | 0.001368 | 0.001738 | 0.001424 |
| tuple_comparison.test | RUN | 0.175970 | 0.467042 | 0.487612 | 0.001537 | 0.001765 | 0.001363 |
| tuple_construction.test | RUN | 0.176066 | 0.466287 | 0.495094 | 0.001724 | 0.001598 | 0.001173 |
| tuple_conversion.test | RUN | 0.380465 | 1.410268 | 1.612770 | 0.001207 | 0.001742 | 0.001377 |
| tuple_copy.test | RUN | 0.255680 | 0.824327 | 0.856322 | 0.001508 | 0.001672 | 0.001212 |
| tuple_element.test | RUN | 0.165823 | 0.437611 | 0.436560 | 0.001382 | 0.001333 | 0.001511 |
| tuple_make.test | RUN | 0.178673 | 0.501820 | 0.475770 | 0.001484 | 0.001706 | 0.001285 |
| tuple_misc.test | RUN | 0.225558 | 0.581002 | 0.594294 | 0.001314 | 0.001746 | 0.001383 |
| tuple_mutate.test | RUN | 0.157101 | 0.392883 | 0.426307 | 0.001492 | 0.001776 | 0.001411 |
| tuple_nest.test | RUN | 0.646147 | 2.942242 | 3.048096 | 0.001464 | 0.001701 | 0.001446 |
| tuple_hash.test | RUN | 0.238059 | 0.834428 | 0.635405 | 0.001496 | 0.001575 | 0.001300 |
| tuple_tie.test | RUN | 0.165610 | 0.430279 | 0.474290 | 0.001452 | 0.001737 | 0.001286 |
| tuple_traits__maybe_variadic.test | RUN | 0.178690 | 0.524042 | 0.486865 | 0.001353 | 0.001594 | 0.001472 |
| tuple_traits__no_variadic.test | RUN | 0.201475 | 0.543053 | 0.524823 | 0.001570 | 0.001714 | 0.001511 |
| transform_view.test | RUN | 0.261559 | 0.899377 | 0.849463 | 0.001621 | 0.001513 | 0.001424 |
| identity_view.test | RUN | 0.250615 | 0.835126 | 0.800934 | 0.001445 | 0.001522 | 0.001376 |
| vector_comparison.test | RUN | 0.160273 | 0.449258 | 0.450293 | 0.001206 | 0.001633 | 0.001418 |
| vector_construction.test | RUN | 0.164681 | 0.412518 | 0.440450 | 0.001436 | 0.001606 | 0.001525 |
| vector_conversion.test | RUN | 0.444848 | 1.881869 | 2.039020 | 0.001522 | 0.001536 | 0.001399 |
| vector_copy.test | RUN | 0.242483 | 0.826013 | 0.832143 | 0.001392 | 0.001570 | 0.001332 |
| vector_iterator.test | RUN | 0.155092 | 0.433864 | 0.424563 | 0.001417 | 0.001718 | 0.001524 |
| vector_make.test | RUN | 0.174144 | 0.466906 | 0.457985 | 0.001423 | 0.001700 | 0.001462 |
| vector_misc.test | RUN | 0.211710 | 0.587980 | 0.578407 | 0.001409 | 0.001501 | 0.001340 |
| vector_move.test | RUN | 0.163908 | 0.458831 | 0.464952 | 0.001368 | 0.001483 | 0.001316 |
| vector_mutate.test | RUN | 0.149002 | 0.382688 | 0.395201 | 0.001379 | 0.001596 | 0.001327 |
| vector_n.test | RUN | 0.183250 | 0.472730 | 0.534277 | 0.001534 | 0.001768 | 0.001465 |
| vector_nest.test | RUN | 0.624239 | 3.052183 | 3.013060 | 0.001236 | 0.001648 | 0.001285 |
| vector_hash.test | RUN | 0.216668 | 0.800588 | 0.600913 | 0.001390 | 0.001793 | 0.001328 |
| vector_tie.test | RUN | 0.155934 | 0.415795 | 0.436277 | 0.001422 | 0.001507 | 0.001420 |
| vector_traits__maybe_variadic.test | RUN | 0.171802 | 0.503388 | 0.461891 | 0.001387 | 0.001536 | 0.001545 |
| vector_traits__no_variadic.test | RUN | 0.186680 | 0.506531 | 0.480997 | 0.001344 | 0.001613 | 0.001317 |
| vector_value_at.test | RUN | 0.151519 | 0.391375 | 0.417340 | 0.001404 | 0.001752 | 0.001299 |
| zip_view.test | RUN | 0.313869 | 0.901099 | 1.048193 | 0.001427 | 0.001546 | 0.001505 |
| zip_view2.test | RUN | 0.250064 | 0.739663 | 0.788424 | 0.001296 | 0.001778 | 0.001464 |
| zip_view_ignore.test | RUN | 0.231627 | 0.663997 | 0.698631 | 0.001428 | 0.001584 | 0.001107 |
| repetitive_view.test | RUN | 0.395997 | 1.197922 | 1.353926 | 0.001413 | 0.001747 | 0.001566 |
| deduce_sequence.test | RUN | 0.195394 | 0.506830 | 0.535861 | 0.001466 | 0.001765 | 0.001500 |
| adapt_adt_named.test | RUN | 0.230305 | 0.732839 | 0.710207 | 0.001385 | 0.001686 | 0.001424 |
| adapt_adt_named_empty.test | RUN | 0.199627 | 0.610799 | 0.570218 | 0.001412 | 0.001798 | 0.001324 |
| adapt_adt.test | RUN | 0.253134 | 0.945343 | 0.833049 | 0.001481 | 0.001834 | 0.001422 |
| adapt_adt_empty.test | RUN | 0.199202 | 0.582700 | 0.567064 | 0.001362 | 0.001671 | 0.001183 |
| adapt_assoc_adt_named.test | RUN | 0.253295 | 0.759845 | 0.746181 | 0.001354 | 0.001681 | 0.001466 |
| adapt_assoc_adt_named_empty.test | RUN | 0.203170 | 0.571575 | 0.583862 | 0.001536 | 0.001520 | 0.001267 |
| adapt_assoc_adt.test | RUN | 0.263554 | 0.791461 | 0.797141 | 0.001423 | 0.001585 | 0.001443 |
| adapt_assoc_adt_empty.test | RUN | 0.198116 | 0.593485 | 0.571659 | 0.001596 | 0.001696 | 0.001374 |
| adapt_assoc_struct_named.test | RUN | 0.256587 | 0.754647 | 0.719540 | 0.001425 | 0.001734 | 0.001404 |
| adapt_assoc_struct_named_empty.test | RUN | 0.199727 | 0.598551 | 0.576971 | 0.001512 | 0.001759 | 0.001515 |
| adapt_assoc_struct.test | RUN | 0.231926 | 0.701866 | 0.656286 | 0.001520 | 0.001460 | 0.001562 |
| adapt_assoc_struct_empty.test | RUN | 0.194826 | 0.585442 | 0.573774 | 0.001328 | 0.001705 | 0.001440 |
| adapt_assoc_tpl_adt.test | RUN | 0.264014 | 0.808685 | 0.808653 | 0.001399 | 0.001804 | 0.001407 |
| adapt_assoc_tpl_adt_empty.test | RUN | 0.203574 | 0.588414 | 0.562299 | 0.001476 | 0.001763 | 0.001494 |
| adapt_assoc_tpl_struct.test | RUN | 0.219641 | 0.684333 | 0.662996 | 0.001365 | 0.001605 | 0.001504 |
| adapt_assoc_tpl_struct_empty.test | RUN | 0.196768 | 0.593783 | 0.578531 | 0.001405 | 0.001794 | 0.001388 |
| adapt_struct_named.test | RUN | 0.221497 | 0.708595 | 0.667305 | 0.001295 | 0.001676 | 0.001543 |
| adapt_struct_named_empty.test | RUN | 0.199225 | 0.572958 | 0.581674 | 0.001403 | 0.001814 | 0.001413 |
| adapt_struct.test | RUN | 0.245352 | 0.806049 | 0.769750 | 0.001423 | 0.001803 | 0.001449 |
| adapt_struct_empty.test | RUN | 0.195820 | 0.585982 | 0.577166 | 0.001539 | 0.001769 | 0.001535 |
| adapt_tpl_adt.test | RUN | 0.254409 | 0.935864 | 0.814691 | 0.001452 | 0.001654 | 0.001359 |
| adapt_tpl_adt_empty.test | RUN | 0.201555 | 0.592580 | 0.574350 | 0.001638 | 0.001819 | 0.001402 |
| adapt_tpl_struct.test | RUN | 0.215061 | 0.669889 | 0.628449 | 0.001413 | 0.001576 | 0.001377 |
| adapt_tpl_struct_empty.test | RUN | 0.196832 | 0.582230 | 0.583360 | 0.001274 | 0.001775 | 0.001435 |
| adt_attribute_proxy.test | RUN | 0.227326 | 0.580071 | 0.603799 | 0.001558 | 0.001802 | 0.001337 |
| define_struct.test | RUN | 0.264611 | 0.803388 | 0.794633 | 0.001350 | 0.001611 | 0.001414 |
| define_struct_empty.test | RUN | 0.236413 | 0.659564 | 0.654823 | 0.001358 | 0.001799 | 0.001400 |
| define_struct_move.test | RUN | 0.156396 | 0.372006 | 0.382318 | 0.001465 | 0.001709 | 0.001454 |
| define_struct_inline.test | RUN | 0.278653 | 0.952670 | 0.905943 | 0.001545 | 0.001701 | 0.001453 |
| define_struct_inline_empty.test | RUN | 0.240191 | 0.642487 | 0.637796 | 0.001296 | 0.001711 | 0.001316 |
| define_struct_inline_move.test | RUN | 0.151296 | 0.387806 | 0.394802 | 0.001403 | 0.001716 | 0.001269 |
| define_assoc_struct.test | RUN | 0.271159 | 0.773414 | 0.773544 | 0.001484 | 0.001653 | 0.001476 |
| define_assoc_struct_empty.test | RUN | 0.232424 | 0.656198 | 0.648872 | 0.001376 | 0.001797 | 0.001456 |
| define_assoc_struct_move.test | RUN | 0.150657 | 0.365979 | 0.390980 | 0.001466 | 0.001593 | 0.001458 |
| define_tpl_struct.test | RUN | 0.262991 | 0.778306 | 0.763182 | 0.001223 | 0.001680 | 0.001272 |
| define_tpl_struct_empty.test | RUN | 0.235632 | 0.670468 | 0.670289 | 0.001413 | 0.001649 | 0.001336 |
| define_tpl_struct_move.test | RUN | 0.155516 | 0.375241 | 0.386562 | 0.001471 | 0.001572 | 0.001314 |
| define_tpl_struct_inline.test | RUN | 0.274769 | 0.878670 | 0.821863 | 0.001518 | 0.001738 | 0.001250 |
| define_tpl_struct_inline_empty.test | RUN | 0.237181 | 0.649619 | 0.667337 | 0.001334 | 0.001484 | 0.001476 |
| define_tpl_struct_inline_move.test | RUN | 0.156873 | 0.393658 | 0.405488 | 0.001307 | 0.001549 | 0.001439 |
| define_assoc_tpl_struct.test | RUN | 0.264794 | 0.780789 | 0.774357 | 0.001509 | 0.001782 | 0.001458 |
| define_assoc_tpl_struct_empty.test | RUN | 0.233744 | 0.660120 | 0.657124 | 0.001258 | 0.001653 | 0.001512 |
| define_assoc_tpl_struct_move.test | RUN | 0.152251 | 0.377334 | 0.395722 | 0.001487 | 0.001676 | 0.001446 |
| std_tuple.test | RUN | 0.158619 | 0.426154 | 0.442676 | 0.001352 | 0.001485 | 0.001358 |
| std_tuple_iterator.test | RUN | 0.157782 | 0.442157 | 0.438341 | 0.001329 | 0.001448 | 0.001386 |
| ref_vector.test | RUN | 0.175781 | 0.452103 | 0.467688 | 0.001538 | 0.001693 | 0.001465 |
| flatten_view.test | RUN | 0.189022 | 0.553845 | 0.560337 | 0.001535 | 0.001688 | 0.001324 |
| github-159.test | COMPILE | 0.086268 | 0.201802 | 0.201470 | — | — | — |
| github-176.test | RUN | 0.180092 | 0.488919 | 0.477390 | 0.001406 | 0.001617 | 0.001399 |
| size.test | COMPILE | 0.152082 | 0.352834 | 0.355330 | — | — | — |
| fused.test | RUN | 0.206557 | 0.522965 | 0.562192 | 0.001327 | 0.001732 | 0.001450 |
| fused_function_object.test | RUN | 0.157815 | 0.407228 | 0.410059 | 0.001378 | 0.001554 | 0.001449 |
| fused_procedure.test | RUN | 0.203677 | 0.498030 | 0.537052 | 0.001215 | 0.001506 | 0.001378 |
| unfused.test | RUN | 0.162848 | 0.415985 | 0.441424 | 0.001335 | 0.001721 | 0.001263 |
| unfused_typed.test | RUN | 0.162134 | 0.441389 | 0.434067 | 0.001227 | 0.001702 | 0.001400 |
| make_fused.test | RUN | 0.209520 | 0.535280 | 0.546933 | 0.001379 | 0.001694 | 0.001450 |
| make_fused_function_object.test | RUN | 0.160403 | 0.403496 | 0.428098 | 0.001186 | 0.001710 | 0.001207 |
| make_fused_procedure.test | RUN | 0.210939 | 0.506417 | 0.522172 | 0.001526 | 0.001664 | 0.001413 |
| make_unfused.test | RUN | 0.158808 | 0.402582 | 0.435353 | 0.001426 | 0.001711 | 0.001431 |
| invoke.test | RUN | 0.404409 | 1.428278 | 1.457884 | 0.001385 | 0.001568 | 0.001263 |
| invoke_function_object.test | RUN | 0.192411 | 0.505282 | 0.531828 | 0.001517 | 0.001831 | 0.001425 |
| invoke_procedure.test | RUN | 0.320339 | 1.048552 | 1.101083 | 0.001261 | 0.001568 | 0.001510 |
| swap.test | RUN | 0.241234 | 0.673588 | 0.673610 | 0.001249 | 0.001674 | 0.001481 |
| is_sequence.test | COMPILE | 0.041584 | 0.100168 | 0.092707 | — | — | — |
| is_view.test | COMPILE | 0.041391 | 0.099747 | 0.093197 | — | — | — |
| pair_deque.test | COMPILE | 0.088071 | 0.218081 | 0.204235 | — | — | — |
| pair_list.test | COMPILE | 0.085828 | 0.205023 | 0.211931 | — | — | — |
| pair_map.test | COMPILE | 0.088734 | 0.220883 | 0.210226 | — | — | — |
| pair_set.test | COMPILE | 0.090530 | 0.218338 | 0.218534 | — | — | — |
| pair_vector.test | COMPILE | 0.088200 | 0.215512 | 0.214982 | — | — | — |
| pair_nest.test | COMPILE | 0.037479 | 0.090857 | 0.083722 | — | — | — |
| index_sequence.test | COMPILE | 0.031791 | 0.061832 | 0.067741 | — | — | — |
| and.test | COMPILE | 0.023982 | 0.049588 | 0.050521 | — | — | — |
| tag_of.test | COMPILE | 0.041268 | 0.094569 | 0.087311 | — | — | — |
| unused.test | COMPILE | 0.022780 | 0.062222 | 0.055351 | — | — | — |
| tag_of_fallback.test | COMPILE | 0.036881 | 0.078250 | 0.086620 | — | — | — |
| make_pair_r-value.test | COMPILE | 0.028880 | 0.080398 | 0.062671 | — | — | — |
| quick.test | RUN | 0.120472 | 0.291032 | 0.262439 | 0.000378 | 0.001168 | 0.001001 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.002× | 1.001×, 1.002× | 1.004× |
| clang | 1.003× | 1.002×, 1.003× | 0.998× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409407).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adapt_adt.test | 10 | 0.000884s | 0.000892s | 0.000869s | byte equality observed |
| adapt_adt_empty.test | 10 | 0.000856s | 0.000871s | 0.000855s | byte equality observed |
| adapt_adt_named.test | 10 | 0.000871s | 0.000883s | 0.000851s | byte equality observed |
| adapt_adt_named_empty.test | 10 | 0.000867s | 0.000865s | 0.000841s | byte equality observed |
| adapt_assoc_adt.test | 10 | 0.000870s | 0.000871s | 0.000854s | byte equality observed |
| adapt_assoc_adt_empty.test | 10 | 0.000853s | 0.000866s | 0.000852s | byte equality observed |
| adapt_assoc_adt_named.test | 10 | 0.000853s | 0.000861s | 0.000855s | byte equality observed |
| adapt_assoc_adt_named_empty.test | 10 | 0.000853s | 0.000857s | 0.000847s | byte equality observed |
| adapt_assoc_struct.test | 10 | 0.000863s | 0.000880s | 0.000847s | byte equality observed |
| adapt_assoc_struct_empty.test | 10 | 0.000856s | 0.000861s | 0.000847s | byte equality observed |
| adapt_assoc_struct_named.test | 10 | 0.000862s | 0.000865s | 0.000850s | byte equality observed |
| adapt_assoc_struct_named_empty.test | 10 | 0.000852s | 0.000850s | 0.000845s | byte equality observed |
| adapt_assoc_tpl_adt.test | 10 | 0.000866s | 0.000883s | 0.000859s | byte equality observed |
| adapt_assoc_tpl_adt_empty.test | 10 | 0.000852s | 0.000867s | 0.000848s | byte equality observed |
| adapt_assoc_tpl_struct.test | 10 | 0.000874s | 0.000878s | 0.000879s | byte equality observed |
| adapt_assoc_tpl_struct_empty.test | 10 | 0.000861s | 0.000882s | 0.000857s | byte equality observed |
| adapt_struct.test | 10 | 0.000871s | 0.000868s | 0.000871s | byte equality observed |
| adapt_struct_empty.test | 10 | 0.000860s | 0.000853s | 0.000860s | byte equality observed |
| adapt_struct_named.test | 10 | 0.000858s | 0.000863s | 0.000856s | byte equality observed |
| adapt_struct_named_empty.test | 10 | 0.000855s | 0.000859s | 0.000858s | byte equality observed |
| adapt_tpl_adt.test | 10 | 0.000851s | 0.000866s | 0.000878s | byte equality observed |
| adapt_tpl_adt_empty.test | 10 | 0.000852s | 0.000859s | 0.000886s | byte equality observed |
| adapt_tpl_struct.test | 10 | 0.000851s | 0.000857s | 0.000857s | byte equality observed |
| adapt_tpl_struct_empty.test | 10 | 0.000848s | 0.000873s | 0.000855s | byte equality observed |
| adt_attribute_proxy.test | 10 | 0.000859s | 0.000849s | 0.000842s | byte equality observed |
| all.test | 10 | 0.000846s | 0.000840s | 0.000841s | byte equality observed |
| any.test | 10 | 0.000840s | 0.000827s | 0.000855s | byte equality observed |
| array.test | 10 | 0.000848s | 0.000854s | 0.000849s | byte equality observed |
| as_deque.test | 10 | 0.000853s | 0.000851s | 0.000833s | byte equality observed |
| as_list.test | 10 | 0.000875s | 0.000881s | 0.000883s | byte equality observed |
| as_map.test | 10 | 0.000858s | 0.000897s | 0.000872s | byte equality observed |
| as_map_assoc.test | 10 | 0.000859s | 0.000858s | 0.000878s | byte equality observed |
| as_set.test | 10 | 0.000859s | 0.000878s | 0.000873s | byte equality observed |
| as_vector.test | 10 | 0.000865s | 0.000878s | 0.000871s | byte equality observed |
| back_extended_deque.test | 10 | 0.000857s | 0.000844s | 0.000842s | byte equality observed |
| boost_array.test | 10 | 0.000849s | 0.000839s | 0.000860s | byte equality observed |
| boost_tuple.test | 10 | 0.000875s | 0.000872s | 0.000872s | byte equality observed |
| boost_tuple_iterator.test | 10 | 0.000852s | 0.000854s | 0.000859s | byte equality observed |
| clear.test | 10 | 0.000867s | 0.000871s | 0.000865s | byte equality observed |
| cons.test | 10 | 0.000882s | 0.000875s | 0.000877s | byte equality observed |
| convert_boost_tuple.test | 10 | 0.000857s | 0.000845s | 0.000849s | byte equality observed |
| convert_deque.test | 10 | 0.000867s | 0.000866s | 0.000856s | byte equality observed |
| convert_list.test | 10 | 0.000845s | 0.000862s | 0.000846s | byte equality observed |
| convert_std_pair.test | 10 | 0.000840s | 0.000851s | 0.000843s | byte equality observed |
| convert_std_tuple.test | 10 | 0.000852s | 0.000843s | 0.000852s | byte equality observed |
| convert_vector.test | 10 | 0.000849s | 0.000851s | 0.000855s | byte equality observed |
| copy.test | 10 | 0.000853s | 0.000857s | 0.000862s | byte equality observed |
| count.test | 10 | 0.000861s | 0.000865s | 0.000838s | byte equality observed |
| count_if.test | 10 | 0.000842s | 0.000845s | 0.000842s | byte equality observed |
| deduce_sequence.test | 10 | 0.000850s | 0.000844s | 0.000851s | byte equality observed |
| define_assoc_struct.test | 10 | 0.000864s | 0.000861s | 0.000875s | byte equality observed |
| define_assoc_struct_empty.test | 10 | 0.000864s | 0.000874s | 0.000851s | byte equality observed |
| define_assoc_struct_move.test | 10 | 0.000847s | 0.000831s | 0.000867s | byte equality observed |
| define_assoc_tpl_struct.test | 10 | 0.000865s | 0.000877s | 0.000865s | byte equality observed |
| define_assoc_tpl_struct_empty.test | 10 | 0.000859s | 0.000871s | 0.000877s | byte equality observed |
| define_assoc_tpl_struct_move.test | 10 | 0.000850s | 0.000853s | 0.000844s | byte equality observed |
| define_struct.test | 10 | 0.000882s | 0.000865s | 0.000871s | byte equality observed |
| define_struct_empty.test | 10 | 0.000862s | 0.000849s | 0.000855s | byte equality observed |
| define_struct_inline.test | 10 | 0.000866s | 0.000868s | 0.000874s | byte equality observed |
| define_struct_inline_empty.test | 10 | 0.000864s | 0.000869s | 0.000859s | byte equality observed |
| define_struct_inline_move.test | 10 | 0.000842s | 0.000855s | 0.000840s | byte equality observed |
| define_struct_move.test | 10 | 0.000853s | 0.000841s | 0.000866s | byte equality observed |
| define_tpl_struct.test | 10 | 0.000861s | 0.000876s | 0.000882s | byte equality observed |
| define_tpl_struct_empty.test | 10 | 0.000866s | 0.000865s | 0.000869s | byte equality observed |
| define_tpl_struct_inline.test | 10 | 0.000875s | 0.000883s | 0.000875s | byte equality observed |
| define_tpl_struct_inline_empty.test | 10 | 0.000851s | 0.000879s | 0.000866s | byte equality observed |
| define_tpl_struct_inline_move.test | 10 | 0.000844s | 0.000833s | 0.000839s | byte equality observed |
| define_tpl_struct_move.test | 10 | 0.000846s | 0.000857s | 0.000844s | byte equality observed |
| deque_comparison.test | 10 | 0.000846s | 0.000841s | 0.000829s | byte equality observed |
| deque_construction.test | 10 | 0.000867s | 0.000857s | 0.000866s | byte equality observed |
| deque_copy.test | 10 | 0.000854s | 0.000845s | 0.000853s | byte equality observed |
| deque_hash.test | 10 | 0.000853s | 0.000857s | 0.000851s | byte equality observed |
| deque_iterator.test | 10 | 0.000846s | 0.000852s | 0.000848s | byte equality observed |
| deque_make.test | 10 | 0.000846s | 0.000844s | 0.000838s | byte equality observed |
| deque_misc.test | 10 | 0.000841s | 0.000843s | 0.000857s | byte equality observed |
| deque_move.test | 10 | 0.000862s | 0.000861s | 0.000859s | byte equality observed |
| deque_mutate.test | 10 | 0.000847s | 0.000845s | 0.000834s | byte equality observed |
| deque_nest.test | 10 | 0.000842s | 0.000856s | 0.000826s | byte equality observed |
| deque_tie.test | 10 | 0.000839s | 0.000841s | 0.000851s | byte equality observed |
| deque_value_at.test | 10 | 0.000840s | 0.000843s | 0.000852s | byte equality observed |
| erase.test | 10 | 0.000873s | 0.000875s | 0.000849s | byte equality observed |
| erase_key.test | 10 | 0.000882s | 0.000875s | 0.000874s | byte equality observed |
| filter.test | 10 | 0.000875s | 0.000844s | 0.000851s | byte equality observed |
| filter_if.test | 10 | 0.000869s | 0.000878s | 0.000874s | byte equality observed |
| filter_view.test | 10 | 0.000874s | 0.000887s | 0.000865s | byte equality observed |
| find.test | 10 | 0.000873s | 0.000859s | 0.000874s | byte equality observed |
| find_if.test | 10 | 0.000871s | 0.000864s | 0.000880s | byte equality observed |
| flatten.test | 10 | 0.000862s | 0.000855s | 0.000873s | byte equality observed |
| flatten_view.test | 10 | 0.000863s | 0.000845s | 0.000874s | byte equality observed |
| fold.test | 10 | 0.000868s | 0.000861s | 0.000868s | byte equality observed |
| for_each.test | 10 | 0.000876s | 0.000869s | 0.000874s | byte equality observed |
| front_extended_deque.test | 10 | 0.000838s | 0.000849s | 0.000844s | byte equality observed |
| fused.test | 10 | 0.000837s | 0.000845s | 0.000851s | byte equality observed |
| fused_function_object.test | 10 | 0.000829s | 0.000855s | 0.000836s | byte equality observed |
| fused_procedure.test | 10 | 0.000839s | 0.000843s | 0.000840s | byte equality observed |
| github-176.test | 10 | 0.000851s | 0.000824s | 0.000849s | byte equality observed |
| hash.test | 10 | 0.000841s | 0.000846s | 0.000835s | byte equality observed |
| identity_view.test | 10 | 0.000884s | 0.000886s | 0.000876s | byte equality observed |
| insert.test | 10 | 0.000892s | 0.000883s | 0.000883s | byte equality observed |
| insert_range.test | 10 | 0.000872s | 0.000879s | 0.000901s | byte equality observed |
| invoke.test | 10 | 0.000842s | 0.000847s | 0.000856s | byte equality observed |
| invoke_function_object.test | 10 | 0.000839s | 0.000853s | 0.000842s | byte equality observed |
| invoke_procedure.test | 10 | 0.000843s | 0.000846s | 0.000857s | byte equality observed |
| io.test | 10 | 0.000964s | 0.000966s | 0.000976s | byte equality observed |
| iter_fold.test | 10 | 0.000865s | 0.000871s | 0.000860s | byte equality observed |
| iterator_range.test | 10 | 0.000890s | 0.000876s | 0.000873s | byte equality observed |
| join.test | 10 | 0.000844s | 0.000851s | 0.000862s | byte equality observed |
| joint_view.test | 10 | 0.000883s | 0.000897s | 0.000896s | byte equality observed |
| list_comparison.test | 10 | 0.000843s | 0.000845s | 0.000858s | byte equality observed |
| list_construction.test | 10 | 0.000862s | 0.000845s | 0.000846s | byte equality observed |
| list_copy.test | 10 | 0.000847s | 0.000853s | 0.000860s | byte equality observed |
| list_hash.test | 10 | 0.000851s | 0.000839s | 0.000848s | byte equality observed |
| list_iterator.test | 10 | 0.000848s | 0.000840s | 0.000841s | byte equality observed |
| list_make.test | 10 | 0.000858s | 0.000859s | 0.000844s | byte equality observed |
| list_misc.test | 10 | 0.000856s | 0.000845s | 0.000839s | byte equality observed |
| list_mutate.test | 10 | 0.000847s | 0.000854s | 0.000846s | byte equality observed |
| list_nest.test | 10 | 0.000854s | 0.000844s | 0.000868s | byte equality observed |
| list_tie.test | 10 | 0.000853s | 0.000845s | 0.000846s | byte equality observed |
| list_value_at.test | 10 | 0.000855s | 0.000855s | 0.000872s | byte equality observed |
| make_fused.test | 10 | 0.000852s | 0.000845s | 0.000861s | byte equality observed |
| make_fused_function_object.test | 10 | 0.000838s | 0.000840s | 0.000844s | byte equality observed |
| make_fused_procedure.test | 10 | 0.000853s | 0.000846s | 0.000853s | byte equality observed |
| make_list.test | 10 | 0.000857s | 0.000860s | 0.000876s | byte equality observed |
| make_unfused.test | 10 | 0.000842s | 0.000855s | 0.000856s | byte equality observed |
| make_vector.test | 10 | 0.000853s | 0.000879s | 0.000866s | byte equality observed |
| map.test | 10 | 0.000881s | 0.000883s | 0.000878s | byte equality observed |
| map_comparison.test | 10 | 0.000841s | 0.000852s | 0.000846s | byte equality observed |
| map_construction.test | 10 | 0.000860s | 0.000851s | 0.000835s | byte equality observed |
| map_copy.test | 10 | 0.000848s | 0.000844s | 0.000839s | byte equality observed |
| map_misc.test | 10 | 0.000852s | 0.000859s | 0.000849s | byte equality observed |
| map_move.test | 10 | 0.000860s | 0.000848s | 0.000871s | byte equality observed |
| map_mutate.test | 10 | 0.000847s | 0.000843s | 0.000864s | byte equality observed |
| map_tie.test | 10 | 0.000840s | 0.000855s | 0.000833s | byte equality observed |
| move.test | 10 | 0.000842s | 0.000868s | 0.000850s | byte equality observed |
| nil.test | 10 | 0.000846s | 0.000841s | 0.000838s | byte equality observed |
| none.test | 10 | 0.000844s | 0.000855s | 0.000839s | byte equality observed |
| nview.test | 10 | 0.000841s | 0.000849s | 0.000858s | byte equality observed |
| pop_back.test | 10 | 0.000893s | 0.000885s | 0.000890s | byte equality observed |
| pop_front.test | 10 | 0.000881s | 0.000875s | 0.000888s | byte equality observed |
| push_back.test | 10 | 0.000885s | 0.000886s | 0.000873s | byte equality observed |
| push_front.test | 10 | 0.000885s | 0.000875s | 0.000873s | byte equality observed |
| quick.test | 10 | 0.000385s | 0.000830s | 0.000824s | byte equality observed |
| ref_vector.test | 10 | 0.000856s | 0.000861s | 0.000849s | byte equality observed |
| remove.test | 10 | 0.000887s | 0.000881s | 0.000885s | byte equality observed |
| remove_if.test | 10 | 0.000874s | 0.000877s | 0.000887s | byte equality observed |
| repetitive_view.test | 10 | 0.000850s | 0.000835s | 0.000844s | byte equality observed |
| replace.test | 10 | 0.000865s | 0.000863s | 0.000875s | byte equality observed |
| replace_if.test | 10 | 0.000868s | 0.000889s | 0.000881s | byte equality observed |
| reverse.test | 10 | 0.000868s | 0.000871s | 0.000881s | byte equality observed |
| reverse_fold.test | 10 | 0.000872s | 0.000850s | 0.000857s | byte equality observed |
| reverse_iter_fold.test | 10 | 0.000873s | 0.000851s | 0.000857s | byte equality observed |
| reverse_view.test | 10 | 0.000885s | 0.000872s | 0.000854s | byte equality observed |
| segmented_find.test | 10 | 0.000837s | 0.000843s | 0.000828s | byte equality observed |
| segmented_find_if.test | 10 | 0.000852s | 0.000849s | 0.000865s | byte equality observed |
| segmented_fold.test | 10 | 0.000876s | 0.000870s | 0.000883s | byte equality observed |
| segmented_for_each.test | 10 | 0.000874s | 0.000865s | 0.000871s | byte equality observed |
| segmented_iterator_range.test | 10 | 0.000877s | 0.000870s | 0.000877s | byte equality observed |
| set.test | 10 | 0.000872s | 0.000870s | 0.000868s | byte equality observed |
| single_view.test | 10 | 0.000861s | 0.000886s | 0.000869s | byte equality observed |
| std_array.test | 10 | 0.000844s | 0.000853s | 0.000847s | byte equality observed |
| std_pair.test | 10 | 0.000872s | 0.000882s | 0.000869s | byte equality observed |
| std_tuple.test | 10 | 0.000836s | 0.000863s | 0.000854s | byte equality observed |
| std_tuple_iterator.test | 10 | 0.000849s | 0.000860s | 0.000836s | byte equality observed |
| swap.test | 10 | 0.000861s | 0.000844s | 0.000830s | byte equality observed |
| transform.test | 10 | 0.000869s | 0.000875s | 0.000864s | byte equality observed |
| transform_view.test | 10 | 0.000890s | 0.000888s | 0.000884s | byte equality observed |
| tuple_comparison.test | 10 | 0.000838s | 0.000836s | 0.000857s | byte equality observed |
| tuple_construction.test | 10 | 0.000845s | 0.000858s | 0.000847s | byte equality observed |
| tuple_conversion.test | 10 | 0.000866s | 0.000857s | 0.000843s | byte equality observed |
| tuple_copy.test | 10 | 0.000852s | 0.000849s | 0.000837s | byte equality observed |
| tuple_element.test | 10 | 0.000848s | 0.000838s | 0.000843s | byte equality observed |
| tuple_hash.test | 10 | 0.000850s | 0.000842s | 0.000847s | byte equality observed |
| tuple_make.test | 10 | 0.000843s | 0.000849s | 0.000833s | byte equality observed |
| tuple_misc.test | 10 | 0.000859s | 0.000857s | 0.000848s | byte equality observed |
| tuple_mutate.test | 10 | 0.000849s | 0.000854s | 0.000847s | byte equality observed |
| tuple_nest.test | 10 | 0.000846s | 0.000851s | 0.000837s | byte equality observed |
| tuple_tie.test | 10 | 0.000851s | 0.000846s | 0.000863s | byte equality observed |
| tuple_traits__maybe_variadic.test | 10 | 0.000843s | 0.000846s | 0.000845s | byte equality observed |
| tuple_traits__no_variadic.test | 10 | 0.000855s | 0.000844s | 0.000859s | byte equality observed |
| unfused.test | 10 | 0.000836s | 0.000844s | 0.000857s | byte equality observed |
| unfused_typed.test | 10 | 0.000856s | 0.000854s | 0.000849s | byte equality observed |
| vector_comparison.test | 10 | 0.000842s | 0.000835s | 0.000842s | byte equality observed |
| vector_construction.test | 10 | 0.000853s | 0.000847s | 0.000847s | byte equality observed |
| vector_conversion.test | 10 | 0.000857s | 0.000847s | 0.000843s | byte equality observed |
| vector_copy.test | 10 | 0.000854s | 0.000843s | 0.000848s | byte equality observed |
| vector_hash.test | 10 | 0.000849s | 0.000848s | 0.000851s | byte equality observed |
| vector_iterator.test | 10 | 0.000859s | 0.000833s | 0.000831s | byte equality observed |
| vector_make.test | 10 | 0.000850s | 0.000852s | 0.000846s | byte equality observed |
| vector_misc.test | 10 | 0.000847s | 0.000858s | 0.000838s | byte equality observed |
| vector_move.test | 10 | 0.000863s | 0.000875s | 0.000868s | byte equality observed |
| vector_mutate.test | 10 | 0.000834s | 0.000854s | 0.000850s | byte equality observed |
| vector_n.test | 10 | 0.000874s | 0.000878s | 0.000884s | byte equality observed |
| vector_nest.test | 10 | 0.000848s | 0.000846s | 0.000864s | byte equality observed |
| vector_tie.test | 10 | 0.000855s | 0.000854s | 0.000838s | byte equality observed |
| vector_traits__maybe_variadic.test | 10 | 0.000849s | 0.000846s | 0.000848s | byte equality observed |
| vector_traits__no_variadic.test | 10 | 0.000855s | 0.000859s | 0.000853s | byte equality observed |
| vector_value_at.test | 10 | 0.000845s | 0.000849s | 0.000856s | byte equality observed |
| zip.test | 10 | 0.000842s | 0.000847s | 0.000846s | byte equality observed |
| zip2.test | 10 | 0.000847s | 0.000863s | 0.000836s | byte equality observed |
| zip_ignore.test | 10 | 0.000843s | 0.000847s | 0.000854s | byte equality observed |
| zip_view.test | 10 | 0.000837s | 0.000853s | 0.000847s | byte equality observed |
| zip_view2.test | 10 | 0.000837s | 0.000854s | 0.000844s | byte equality observed |
| zip_view_ignore.test | 10 | 0.000851s | 0.000845s | 0.000850s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409407).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
