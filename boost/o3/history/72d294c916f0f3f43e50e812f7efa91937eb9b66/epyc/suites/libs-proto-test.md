# libs/proto/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| calculator.test | RUN | 0.727191 | 1.955190 | 2.580736 | 0.003514 | 0.003537 | 0.003751 |
| constrained_ops.test | RUN | 0.832335 | 2.089402 | 2.777018 | 0.003526 | 0.003673 | 0.003541 |
| cpp-next_bug.test | RUN | 0.847233 | 2.154315 | 2.898354 | 0.003485 | 0.003546 | 0.003750 |
| deep_copy.test | RUN | 0.724665 | 1.902806 | 2.521950 | 0.003354 | 0.003506 | 0.003812 |
| display_expr.test | RUN | 0.866843 | 2.362816 | 2.889724 | 0.003556 | 0.003240 | 0.003627 |
| deduce_domain.test | RUN | 0.674007 | 1.635183 | 2.337700 | 0.003615 | 0.003539 | 0.003091 |
| env_var.test | RUN | 0.913379 | 4.054129 | 3.654758 | 0.003993 | 0.004246 | 0.004769 |
| examples.test | RUN | 0.885371 | 2.626209 | 3.337731 | 0.003519 | 0.003322 | 0.003562 |
| external_transforms.test | RUN | 0.869572 | 2.214406 | 3.037552 | 0.003645 | 0.003371 | 0.003797 |
| lambda.test | RUN | 0.840405 | 2.249619 | 3.151365 | 0.003361 | 0.003487 | 0.003541 |
| make_expr.test | RUN | 0.834555 | 3.672104 | 3.302937 | 0.004602 | 0.005121 | 0.005018 |
| matches.test | RUN | 0.756732 | 1.862454 | 2.728154 | 0.003567 | 0.003165 | 0.003995 |
| flatten.test | RUN | 1.470846 | 5.182173 | 6.272447 | 0.003978 | 0.004063 | 0.003853 |
| switch.test | RUN | 0.740119 | 1.870666 | 2.628383 | 0.003724 | 0.003982 | 0.003761 |
| toy_spirit.test | RUN | 0.854107 | 2.354505 | 3.165478 | 0.003541 | 0.003862 | 0.003707 |
| toy_spirit2.test | RUN | 0.900929 | 2.437679 | 3.404742 | 0.003511 | 0.003450 | 0.003199 |
| make.test | RUN | 0.741118 | 2.354449 | 2.541125 | 0.004217 | 0.004370 | 0.003804 |
| mem_ptr.test | RUN | 0.916067 | 3.902604 | 3.597881 | 0.004292 | 0.004364 | 0.004034 |
| mpl.test | RUN | 0.815619 | 1.965301 | 2.783929 | 0.003243 | 0.003890 | 0.003680 |
| noinvoke.test | RUN | 0.717614 | 1.688272 | 2.313582 | 0.003584 | 0.003460 | 0.003169 |
| pack_expansion.test | RUN | 0.778673 | 2.375249 | 2.916793 | 0.002792 | 0.002749 | 0.002535 |
| protect.test | RUN | 0.654992 | 1.688698 | 1.663931 | 0.002857 | 0.002672 | 0.003078 |
| bug2407.test | COMPILE | 0.620382 | 1.244315 | 1.442928 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.981× | 0.988×, 0.974× | 1.011× |
| clang | 0.965× | 0.962×, 0.967× | 1.007× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824563).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| calculator.test | 9 | 0.002070s | 0.002085s | 0.001941s | byte equality observed |
| constrained_ops.test | 7 | 0.002019s | 0.001987s | 0.001970s | byte equality observed |
| cpp-next_bug.test | 9 | 0.002114s | 0.001956s | 0.002012s | byte equality observed |
| deduce_domain.test | 8 | 0.002039s | 0.002084s | 0.001983s | byte equality observed |
| deep_copy.test | 8 | 0.002071s | 0.001997s | 0.002011s | byte equality observed |
| display_expr.test | 9 | 0.002017s | 0.001992s | 0.002044s | byte equality observed |
| env_var.test | 9 | 0.002409s | 0.002319s | 0.002259s | byte equality observed |
| examples.test | 8 | 0.002092s | 0.001962s | 0.001953s | byte equality observed |
| external_transforms.test | 9 | 0.002027s | 0.002143s | 0.002017s | byte equality observed |
| flatten.test | 8 | 0.002124s | 0.002080s | 0.002081s | byte equality observed |
| lambda.test | 9 | 0.002028s | 0.002005s | 0.001906s | byte equality observed |
| make.test | 8 | 0.002270s | 0.002228s | 0.002150s | byte equality observed |
| make_expr.test | 8 | 0.002471s | 0.002474s | 0.002352s | byte equality observed |
| matches.test | 8 | 0.002015s | 0.001982s | 0.001990s | byte equality observed |
| mem_ptr.test | 8 | 0.002340s | 0.002364s | 0.002323s | byte equality observed |
| mpl.test | 9 | 0.002048s | 0.001975s | 0.001905s | byte equality observed |
| noinvoke.test | 9 | 0.002062s | 0.002016s | 0.002104s | byte equality observed |
| pack_expansion.test | 9 | 0.002162s | 0.002045s | 0.001981s | byte equality observed |
| protect.test | 9 | 0.002156s | 0.002087s | 0.002051s | byte equality observed |
| switch.test | 9 | 0.002093s | 0.002034s | 0.002042s | byte equality observed |
| toy_spirit.test | 9 | 0.002046s | 0.001988s | 0.001970s | byte equality observed |
| toy_spirit2.test | 9 | 0.002097s | 0.002090s | 0.002018s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824563).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
