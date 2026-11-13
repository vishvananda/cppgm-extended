# libs/json/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| array.test | RUN | 0.893303 | 5.004519 | 4.255748 | 0.002502 | 0.002151 | 0.002396 |
| basic_parser.test | RUN | 6.039410 | 114.565010 | 68.894656 | 0.530631 | 0.227651 | 0.747495 |
| conversion.test | RUN | 0.615426 | 1.854563 | 2.073741 | 0.001360 | 0.001446 | 0.001365 |
| doc_background.test | RUN | 0.627714 | 1.865942 | 2.036009 | 0.001595 | 0.001349 | 0.001410 |
| doc_forward_conversion.test | RUN | 0.660891 | 2.171523 | 2.305039 | 0.001506 | 0.001433 | 0.001363 |
| doc_parsing.test | RUN | 0.671363 | 2.038270 | 2.098232 | 0.001558 | 0.001429 | 0.001431 |
| doc_quick_look.test | RUN | 0.869874 | 2.434043 | 2.887596 | 0.001336 | 0.001359 | 0.001300 |
| doc_serializing.test | RUN | 0.605305 | 1.816768 | 1.951927 | 0.001609 | 0.001314 | 0.001258 |
| doc_storage_ptr.test | RUN | 0.619098 | 1.854237 | 2.009556 | 0.001701 | 0.001466 | 0.001227 |
| doc_uses_allocator.test | RUN | 0.646262 | 1.800155 | 1.967198 | 0.001631 | 0.001406 | 0.001331 |
| doc_using_numbers.test | RUN | 0.665958 | 1.830868 | 2.198308 | 0.001439 | 0.001407 | 0.001397 |
| double.test | RUN | 0.704380 | 3.185161 | 2.498787 | 1.673365 | 1.544322 | 1.562513 |
| error.test | RUN | 0.521073 | 1.815284 | 1.711388 | 0.001536 | 0.001438 | 0.001456 |
| fwd.test | RUN | 0.587225 | 1.773617 | 2.113828 | 0.001347 | 0.001652 | 0.001356 |
| json.test | RUN | 0.631625 | 2.027079 | 2.226864 | 0.001434 | 0.001497 | 0.001433 |
| kind.test | RUN | 0.470759 | 1.642468 | 1.625814 | 0.001356 | 0.001508 | 0.001467 |
| memory_resource.test | RUN | 0.647036 | 2.141803 | 2.098129 | 0.001417 | 0.001435 | 0.001314 |
| monotonic_resource.test | RUN | 0.613485 | 2.049408 | 2.061352 | 0.002100 | 0.002076 | 0.002100 |
| natvis.test | RUN | 0.609489 | 1.927224 | 1.976061 | 0.001446 | 0.001499 | 0.001391 |
| null_resource.test | RUN | 0.503173 | 1.672487 | 1.619095 | 0.001512 | 0.001464 | 0.001427 |
| object.test | RUN | 1.043821 | 5.967931 | 5.188123 | 0.006895 | 0.005168 | 0.006343 |
| parse.test | RUN | 0.662964 | 3.016604 | 2.273006 | 0.002045 | 0.002046 | 0.002012 |
| parser.test | RUN | 0.635550 | 2.492659 | 2.166715 | 0.001659 | 0.001596 | 0.001558 |
| pilfer.test | RUN | 0.342175 | 1.121656 | 1.039655 | 0.001303 | 0.001428 | 0.001421 |
| pointer.test | RUN | 0.746561 | 3.449836 | 2.881811 | 0.001704 | 0.001644 | 0.001838 |
| result_for.test | RUN | 0.625230 | 2.045895 | 2.129384 | 0.001263 | 0.001468 | 0.001421 |
| serialize.test | RUN | 0.621862 | 2.106371 | 2.099442 | 0.001503 | 0.001550 | 0.001495 |
| serializer.test | RUN | 1.020180 | 6.094243 | 4.976886 | 0.002586 | 0.003287 | 0.002364 |
| snippets.test | RUN | 1.624051 | 8.764915 | 6.858392 | 0.001816 | 0.001815 | 0.001644 |
| static_resource.test | RUN | 0.603023 | 1.870083 | 1.998857 | 0.001531 | 0.001765 | 0.001450 |
| storage_ptr.test | RUN | 0.626680 | 2.066075 | 2.146154 | 0.001506 | 0.001585 | 0.001522 |
| stream_parser.test | RUN | 0.768877 | 3.824943 | 3.113988 | 0.096948 | 0.110397 | 0.074676 |
| string.test | RUN | 1.361115 | 9.446496 | 7.616069 | 0.002494 | 0.002272 | 0.002414 |
| string_view.test | RUN | 0.371438 | 1.202789 | 1.148520 | 0.001417 | 0.001456 | 0.001370 |
| value.test | RUN | 1.308750 | 9.542142 | 6.314636 | 0.003234 | 0.003087 | 0.003605 |
| value_from.test | RUN | 1.174437 | 6.010706 | 5.355822 | 0.001843 | 0.001732 | 0.001553 |
| value_stack.test | RUN | 0.592705 | 1.737906 | 1.883479 | 0.001428 | 0.001378 | 0.001404 |
| value_to.test | RUN | 3.082894 | 18.619287 | 14.237115 | 0.002888 | 0.002893 | 0.002737 |
| value_ref.test | RUN | 0.667630 | 2.740144 | 2.467212 | 0.001519 | 0.001625 | 0.001449 |
| utf8.test | RUN | 0.511622 | 1.662711 | 1.680350 | 0.001357 | 0.001450 | 0.001515 |
| visit.test | RUN | 0.663800 | 2.739946 | 2.336887 | 0.001443 | 0.001370 | 0.001401 |
| d2s_intrinsics_test.test | RUN | 0.498800 | 1.594445 | 1.655940 | 0.001329 | 0.001734 | 0.001557 |
| d2s_table_test.test | RUN | 0.512469 | 1.764021 | 1.734001 | 0.001463 | 0.001459 | 0.001506 |
| d2s_test.test | RUN | 0.540015 | 2.171107 | 1.925236 | 0.001592 | 0.001589 | 0.001517 |
| parse_into.test | RUN | 19.490290 | 149.475479 | 91.041475 | 0.002832 | 0.002619 | 0.002460 |
| limits.test | RUN | 2.123563 | 13.682757 | 9.652522 | 0.001821 | 0.001751 | 0.001701 |
| header_only.test | RUN | 1.425360 | 10.211779 | 6.930298 | 0.001374 | 0.001437 | 0.001431 |
| no_exceptions.test | RUN | 2.025656 | 11.702699 | 9.007910 | 0.001536 | 0.001433 | 0.001507 |
| intrusive_macros.test | RUN | 1.716750 | 11.294675 | 7.999561 | 0.001462 | 0.001505 | 0.001486 |
| _gdb-printers.test | RUN | 0.298129 | 0.780910 | 0.914796 | 0.191740 | 0.198963 | 0.195256 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.839× | 0.832×, 0.846× | 1.021× |
| clang | 1.053× | 1.046×, 1.059× | 0.997× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345325).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| array.test | 10 | 0.001565s | 0.001359s | 0.001474s | native test oracle; output varies |
| basic_parser.test | 1 | 0.357568s | 0.246409s | 0.548789s | native test oracle; output varies |
| conversion.test | 10 | 0.000845s | 0.000844s | 0.000817s | byte equality observed |
| d2s_intrinsics_test.test | 10 | 0.000853s | 0.000826s | 0.000818s | byte equality observed |
| d2s_table_test.test | 10 | 0.000892s | 0.000849s | 0.000843s | byte equality observed |
| d2s_test.test | 10 | 0.000926s | 0.000887s | 0.000868s | byte equality observed |
| doc_background.test | 10 | 0.000878s | 0.000834s | 0.000829s | byte equality observed |
| doc_forward_conversion.test | 10 | 0.000906s | 0.000857s | 0.000877s | byte equality observed |
| doc_parsing.test | 10 | 0.000928s | 0.000867s | 0.000896s | byte equality observed |
| doc_quick_look.test | 10 | 0.000892s | 0.000839s | 0.000876s | byte equality observed |
| doc_serializing.test | 10 | 0.000890s | 0.000819s | 0.000853s | byte equality observed |
| doc_storage_ptr.test | 10 | 0.000880s | 0.000849s | 0.000885s | byte equality observed |
| doc_uses_allocator.test | 10 | 0.000884s | 0.000817s | 0.000859s | byte equality observed |
| doc_using_numbers.test | 10 | 0.000892s | 0.000827s | 0.000856s | byte equality observed |
| double.test | 1 | 1.075638s | 0.933691s | 0.971106s | native test oracle; output varies |
| error.test | 10 | 0.000962s | 0.000945s | 0.000962s | byte equality observed |
| fwd.test | 10 | 0.000846s | 0.000872s | 0.000883s | byte equality observed |
| header_only.test | 10 | 0.000821s | 0.000866s | 0.000833s | byte equality observed |
| intrusive_macros.test | 10 | 0.000871s | 0.000890s | 0.000887s | byte equality observed |
| json.test | 10 | 0.000886s | 0.000905s | 0.000873s | byte equality observed |
| kind.test | 10 | 0.000890s | 0.000897s | 0.000894s | byte equality observed |
| limits.test | 10 | 0.001067s | 0.001033s | 0.001034s | byte equality observed |
| memory_resource.test | 10 | 0.000898s | 0.000892s | 0.000893s | byte equality observed |
| monotonic_resource.test | 10 | 0.001324s | 0.001314s | 0.001254s | native test oracle; output varies |
| natvis.test | 10 | 0.000898s | 0.000899s | 0.000893s | byte equality observed |
| no_exceptions.test | 10 | 0.000892s | 0.000885s | 0.000903s | byte equality observed |
| null_resource.test | 10 | 0.000918s | 0.000907s | 0.000913s | byte equality observed |
| object.test | 5 | 0.003976s | 0.003165s | 0.003750s | native test oracle; output varies |
| parse.test | 10 | 0.001314s | 0.001248s | 0.001272s | byte equality observed |
| parse_into.test | 8 | 0.002137s | 0.001963s | 0.001936s | native test oracle; output varies |
| parser.test | 10 | 0.001023s | 0.000985s | 0.001007s | byte equality observed |
| pilfer.test | 10 | 0.000858s | 0.000867s | 0.000864s | byte equality observed |
| pointer.test | 10 | 0.001077s | 0.001027s | 0.001051s | byte equality observed |
| result_for.test | 10 | 0.000856s | 0.000888s | 0.000866s | byte equality observed |
| serialize.test | 10 | 0.000908s | 0.000943s | 0.000905s | byte equality observed |
| serializer.test | 10 | 0.001602s | 0.001548s | 0.001527s | native test oracle; output varies |
| snippets.test | 10 | 0.001066s | 0.001054s | 0.001040s | byte equality observed |
| static_resource.test | 10 | 0.000974s | 0.000940s | 0.000935s | byte equality observed |
| storage_ptr.test | 10 | 0.000922s | 0.000915s | 0.000907s | byte equality observed |
| stream_parser.test | 1 | 0.051874s | 0.067770s | 0.048866s | native test oracle; output varies |
| string.test | 10 | 0.001572s | 0.001327s | 0.001539s | byte equality observed |
| string_view.test | 10 | 0.000866s | 0.000843s | 0.000867s | byte equality observed |
| utf8.test | 10 | 0.000872s | 0.000852s | 0.000868s | byte equality observed |
| value.test | 8 | 0.002019s | 0.001814s | 0.002347s | native test oracle; output varies |
| value_from.test | 10 | 0.001130s | 0.001002s | 0.001034s | byte equality observed |
| value_ref.test | 10 | 0.000926s | 0.000902s | 0.000921s | byte equality observed |
| value_stack.test | 10 | 0.000909s | 0.000887s | 0.000898s | byte equality observed |
| value_to.test | 10 | 0.001743s | 0.001635s | 0.001641s | native test oracle; output varies |
| visit.test | 10 | 0.000908s | 0.000878s | 0.000894s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345325).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
