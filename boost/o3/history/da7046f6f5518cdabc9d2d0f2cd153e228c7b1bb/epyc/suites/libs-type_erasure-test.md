# libs/type_erasure/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_binding.test | RUN | 0.417135 | 1.461848 | 1.288105 | 0.002891 | 0.002830 | 0.002849 |
| test_increment.test | RUN | 0.416047 | 1.124468 | 1.115715 | 0.002900 | 0.002836 | 0.002945 |
| test_add.test | RUN | 0.517998 | 1.782568 | 1.657350 | 0.003123 | 0.003131 | 0.003146 |
| test_add_assign.test | RUN | 0.565429 | 1.878183 | 1.747120 | 0.003190 | 0.003028 | 0.003168 |
| test_callable.test | RUN | 0.840008 | 3.357463 | 3.855068 | 0.004660 | 0.005019 | 0.005101 |
| test_reference.test | RUN | 0.431928 | 1.330641 | 1.198917 | 0.002781 | 0.003050 | 0.002939 |
| test_construct.test | RUN | 1.666751 | 17.793309 | 11.991768 | 0.006440 | 0.006468 | 0.006230 |
| test_relaxed.test | RUN | 0.393818 | 0.983172 | 1.058484 | 0.002832 | 0.002781 | 0.002807 |
| test_assign.test | RUN | 4.546217 | 51.679832 | 39.401020 | 0.008522 | 0.005911 | 0.006628 |
| test_construct_ref.test | RUN | 0.515162 | 2.257220 | 1.740825 | 0.004178 | 0.003687 | 0.003677 |
| test_construct_cref.test | RUN | 0.587075 | 2.223862 | 2.243535 | 0.004470 | 0.004386 | 0.004486 |
| test_any_cast.test | RUN | 0.489637 | 2.232662 | 1.637912 | 0.003559 | 0.003313 | 0.003859 |
| test_binding_of.test | RUN | 0.404241 | 0.865606 | 1.019116 | 0.002768 | 0.002691 | 0.002783 |
| test_typeid_of.test | RUN | 0.429714 | 1.276152 | 1.177664 | 0.003030 | 0.003077 | 0.003165 |
| test_nested.test | RUN | 0.462717 | 1.172213 | 1.328294 | 0.002917 | 0.002839 | 0.002812 |
| test_less.test | RUN | 0.591468 | 2.453925 | 2.153016 | 0.003965 | 0.003764 | 0.004501 |
| test_equal.test | RUN | 0.545804 | 2.312507 | 1.932213 | 0.003272 | 0.003500 | 0.003534 |
| test_negate.test | RUN | 0.414692 | 0.901752 | 1.122872 | 0.002558 | 0.002729 | 0.002844 |
| test_dereference.test | RUN | 0.437367 | 1.203744 | 1.165206 | 0.003026 | 0.002945 | 0.003242 |
| test_subscript.test | RUN | 0.470134 | 1.288196 | 1.286183 | 0.003083 | 0.003190 | 0.003208 |
| test_forward_iterator.test | RUN | 0.535609 | 1.552706 | 1.593678 | 0.003058 | 0.002934 | 0.002993 |
| test_tuple.test | RUN | 0.480597 | 1.590109 | 1.271708 | 0.003180 | 0.003099 | 0.003040 |
| test_stream.test | RUN | 0.621881 | 2.567261 | 2.198626 | 0.004115 | 0.004144 | 0.004114 |
| test_deduced.test | RUN | 0.413305 | 0.969991 | 1.052822 | 0.002658 | 0.002662 | 0.002774 |
| test_same_type.test | RUN | 0.457188 | 1.245091 | 1.183153 | 0.002780 | 0.002850 | 0.002895 |
| test_member.test | RUN | 0.574898 | 2.546716 | 2.132070 | 0.004234 | 0.004638 | 0.004441 |
| test_null.test | RUN | 0.421996 | 1.161897 | 1.104722 | 0.003109 | 0.003031 | 0.003093 |
| test_free.test | RUN | 0.492646 | 2.119222 | 1.402943 | 0.003161 | 0.003589 | 0.003435 |
| test_is_empty.test | RUN | 0.395915 | 1.052184 | 1.056326 | 0.002721 | 0.002881 | 0.002889 |
| test_dynamic_any_cast.test | RUN | 0.695984 | 3.121023 | 2.876817 | 0.003999 | 0.004092 | 0.004231 |
| test_limits.test | RUN | 0.655889 | 2.132450 | 2.073508 | 0.003463 | 0.003137 | 0.003121 |
| test_param.test | COMPILE | 0.271144 | 0.564505 | 0.693274 | — | — | — |
| test_is_subconcept.test | COMPILE | 0.263551 | 0.435202 | 0.573368 | — | — | — |
| test_sfinae.test | COMPILE | 0.268951 | 0.459015 | 0.588577 | — | — | — |
| test_interface_order.test | COMPILE | 0.263057 | 0.453263 | 0.566161 | — | — | — |
| test_is_placeholder.test | COMPILE | 0.037303 | 0.048004 | 0.056151 | — | — | — |
| fail_default_construct.test | COMPILE_FAIL | 0.246907 | 0.431666 | 0.517120 | — | — | — |
| fail_construct_mismatch.test | COMPILE_FAIL | 0.247261 | 0.442339 | 0.490121 | — | — | — |
| fail_construct_mismatch_ref.test | COMPILE_FAIL | 0.247229 | 0.429765 | 0.529040 | — | — | — |
| fail_construct_mismatch_cref.test | COMPILE_FAIL | 0.251034 | 0.426204 | 0.544535 | — | — | — |
| fail_construct_mismatch_rref.test | COMPILE_FAIL | 0.245695 | 0.430612 | 0.489935 | — | — | — |
| fail_binding_convert_no_mapping.test | COMPILE_FAIL | 0.240258 | 0.413125 | 0.515437 | — | — | — |
| fail_increment_discard_const.test | COMPILE_FAIL | 0.248726 | 0.431431 | 0.522871 | — | — | — |
| fail_not_incrementable.test | COMPILE_FAIL | 0.263205 | 0.456219 | 0.540333 | — | — | — |
| fail_ref_assign.test | COMPILE_FAIL | 0.246843 | 0.400610 | 0.514638 | — | — | — |
| fail_cref_assign.test | COMPILE_FAIL | 0.244027 | 0.414257 | 0.531245 | — | — | — |
| fail_rref_assign.test | COMPILE_FAIL | 0.244490 | 0.392617 | 0.482478 | — | — | — |
| fail_ref_discard_const.test | COMPILE_FAIL | 0.246873 | 0.443355 | 0.545005 | — | — | — |
| fail_ref_discard_const_convert.test | COMPILE_FAIL | 0.248899 | 0.463783 | 0.549319 | — | — | — |
| fail_ref_discard_const_convert_ref.test | COMPILE_FAIL | 0.248821 | 0.444847 | 0.529886 | — | — | — |
| fail_ref_discard_const_convert_cref.test | COMPILE_FAIL | 0.247329 | 0.472093 | 0.549591 | — | — | — |
| fail_ref_discard_const_init.test | COMPILE_FAIL | 0.251578 | 0.444320 | 0.516783 | — | — | — |
| fail_any_cast_discard_const1.test | COMPILE_FAIL | 0.246886 | 0.439658 | 0.509145 | — | — | — |
| fail_any_cast_discard_const2.test | COMPILE_FAIL | 0.248542 | 0.451703 | 0.552636 | — | — | — |
| fail_any_cast_discard_const3.test | COMPILE_FAIL | 0.247459 | 0.430776 | 0.552735 | — | — | — |
| fail_any_cast_discard_const4.test | COMPILE_FAIL | 0.254210 | 0.440577 | 0.547178 | — | — | — |
| fail_any_cast_discard_const5.test | COMPILE_FAIL | 0.244929 | 0.446486 | 0.556264 | — | — | — |
| fail_any_cast_discard_const6.test | COMPILE_FAIL | 0.247121 | 0.443778 | 0.559346 | — | — | — |
| fail_any_cast_pointer_to_ref.test | COMPILE_FAIL | 0.243986 | 0.434491 | 0.498008 | — | — | — |
| fail_any_cast_pointer_to_val.test | COMPILE_FAIL | 0.223540 | 0.396267 | 0.399740 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.942× | 0.940×, 0.944× | 0.985× |
| clang | 0.971× | 0.971×, 0.970× | 0.996× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| test_add.test | 10 | 0.001830s | 0.001764s | 0.001848s | byte equality observed |
| test_add_assign.test | 10 | 0.001847s | 0.001767s | 0.001902s | byte equality observed |
| test_any_cast.test | 10 | 0.002126s | 0.001982s | 0.002136s | byte equality observed |
| test_assign.test | 4 | 0.005266s | 0.004563s | 0.005122s | byte equality observed |
| test_binding.test | 10 | 0.001773s | 0.001671s | 0.001764s | byte equality observed |
| test_binding_of.test | 10 | 0.001662s | 0.001582s | 0.001677s | byte equality observed |
| test_callable.test | 8 | 0.002689s | 0.002588s | 0.002815s | byte equality observed |
| test_construct.test | 6 | 0.003614s | 0.003338s | 0.003555s | byte equality observed |
| test_construct_cref.test | 8 | 0.002478s | 0.002348s | 0.002446s | byte equality observed |
| test_construct_ref.test | 9 | 0.002177s | 0.002108s | 0.002196s | byte equality observed |
| test_deduced.test | 10 | 0.001687s | 0.001561s | 0.001671s | byte equality observed |
| test_dereference.test | 10 | 0.001817s | 0.001649s | 0.001733s | byte equality observed |
| test_dynamic_any_cast.test | 8 | 0.002492s | 0.002319s | 0.002476s | byte equality observed |
| test_equal.test | 9 | 0.002125s | 0.001960s | 0.002034s | byte equality observed |
| test_forward_iterator.test | 10 | 0.001805s | 0.001650s | 0.001710s | byte equality observed |
| test_free.test | 9 | 0.002085s | 0.001977s | 0.002071s | byte equality observed |
| test_increment.test | 10 | 0.001712s | 0.001581s | 0.001707s | byte equality observed |
| test_is_empty.test | 10 | 0.001749s | 0.001589s | 0.001687s | byte equality observed |
| test_less.test | 9 | 0.002288s | 0.002099s | 0.002217s | byte equality observed |
| test_limits.test | 10 | 0.001863s | 0.001729s | 0.001868s | byte equality observed |
| test_member.test | 8 | 0.002527s | 0.002390s | 0.002527s | byte equality observed |
| test_negate.test | 10 | 0.001688s | 0.001559s | 0.001626s | byte equality observed |
| test_nested.test | 10 | 0.001725s | 0.001600s | 0.001715s | byte equality observed |
| test_null.test | 10 | 0.001766s | 0.001636s | 0.001757s | byte equality observed |
| test_reference.test | 10 | 0.001804s | 0.001711s | 0.001798s | byte equality observed |
| test_relaxed.test | 10 | 0.001707s | 0.001578s | 0.001672s | byte equality observed |
| test_same_type.test | 10 | 0.001782s | 0.001662s | 0.001710s | byte equality observed |
| test_stream.test | 8 | 0.002375s | 0.002246s | 0.002352s | byte equality observed |
| test_subscript.test | 10 | 0.001811s | 0.001724s | 0.001860s | byte equality observed |
| test_tuple.test | 10 | 0.001832s | 0.001716s | 0.001784s | byte equality observed |
| test_typeid_of.test | 10 | 0.001852s | 0.001753s | 0.001778s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
