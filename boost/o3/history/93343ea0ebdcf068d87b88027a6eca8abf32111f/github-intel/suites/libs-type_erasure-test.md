# libs/type_erasure/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_binding.test | RUN | 0.733887 | 2.642755 | 2.205246 | 0.003176 | 0.003054 | 0.003004 |
| test_increment.test | RUN | 0.686239 | 1.977683 | 1.929617 | 0.002643 | 0.002857 | 0.002801 |
| test_add.test | RUN | 0.897134 | 3.173981 | 2.883472 | 0.003207 | 0.003284 | 0.003079 |
| test_add_assign.test | RUN | 0.879942 | 3.418285 | 3.005723 | 0.002995 | 0.003181 | 0.003097 |
| test_callable.test | RUN | 1.418172 | 5.789180 | 6.493346 | 0.004755 | 0.004785 | 0.004462 |
| test_reference.test | RUN | 0.730483 | 2.377565 | 2.058050 | 0.003135 | 0.003126 | 0.003187 |
| test_construct.test | RUN | 2.948249 | 27.720384 | 19.246400 | 0.006118 | 0.005757 | 0.005725 |
| test_relaxed.test | RUN | 0.684231 | 1.682139 | 1.820327 | 0.002988 | 0.002909 | 0.002856 |
| test_assign.test | RUN | 8.582429 | 85.612869 | 65.476319 | 0.011482 | 0.005286 | 0.005812 |
| test_construct_ref.test | RUN | 0.883098 | 4.175483 | 2.975009 | 0.003983 | 0.003479 | 0.003713 |
| test_construct_cref.test | RUN | 0.976801 | 3.795605 | 3.654829 | 0.004313 | 0.004019 | 0.004178 |
| test_any_cast.test | RUN | 0.799514 | 3.982821 | 2.871584 | 0.003779 | 0.003634 | 0.003539 |
| test_binding_of.test | RUN | 0.650654 | 1.511908 | 1.739915 | 0.002952 | 0.002783 | 0.002809 |
| test_typeid_of.test | RUN | 0.703605 | 2.297388 | 1.937653 | 0.003292 | 0.003093 | 0.002997 |
| test_nested.test | RUN | 0.763326 | 2.140546 | 2.241651 | 0.003171 | 0.002923 | 0.002838 |
| test_less.test | RUN | 0.947797 | 4.266148 | 3.646131 | 0.004045 | 0.003764 | 0.003615 |
| test_equal.test | RUN | 0.912229 | 4.007440 | 3.047719 | 0.003428 | 0.003549 | 0.003440 |
| test_negate.test | RUN | 0.673512 | 1.589595 | 1.804757 | 0.002637 | 0.002786 | 0.002719 |
| test_dereference.test | RUN | 0.722496 | 2.057720 | 2.110092 | 0.003248 | 0.002947 | 0.002958 |
| test_subscript.test | RUN | 0.749427 | 2.296193 | 2.030272 | 0.003319 | 0.003122 | 0.002680 |
| test_forward_iterator.test | RUN | 0.885349 | 2.723351 | 2.720017 | 0.003118 | 0.003110 | 0.003002 |
| test_tuple.test | RUN | 0.772300 | 2.793824 | 2.211100 | 0.003247 | 0.003213 | 0.002857 |
| test_stream.test | RUN | 0.995300 | 4.566672 | 3.670485 | 0.004125 | 0.003833 | 0.003996 |
| test_deduced.test | RUN | 0.661563 | 1.664685 | 1.794215 | 0.003070 | 0.002921 | 0.002617 |
| test_same_type.test | RUN | 0.752386 | 2.248100 | 1.996602 | 0.003282 | 0.003018 | 0.002856 |
| test_member.test | RUN | 0.956337 | 4.406715 | 3.556840 | 0.004419 | 0.004335 | 0.004192 |
| test_null.test | RUN | 0.683416 | 2.093229 | 1.872900 | 0.003239 | 0.002995 | 0.003007 |
| test_free.test | RUN | 0.827707 | 3.628298 | 2.372484 | 0.003678 | 0.003345 | 0.003346 |
| test_is_empty.test | RUN | 0.660026 | 1.765264 | 1.775342 | 0.002975 | 0.002839 | 0.002868 |
| test_dynamic_any_cast.test | RUN | 1.215116 | 4.970069 | 4.709341 | 0.004349 | 0.003870 | 0.004013 |
| test_limits.test | RUN | 1.058793 | 3.570999 | 3.500945 | 0.003328 | 0.003239 | 0.002926 |
| test_param.test | COMPILE | 0.469542 | 1.026515 | 1.146870 | — | — | — |
| test_is_subconcept.test | COMPILE | 0.436988 | 0.757851 | 0.991990 | — | — | — |
| test_sfinae.test | COMPILE | 0.453547 | 0.798959 | 1.030794 | — | — | — |
| test_interface_order.test | COMPILE | 0.442939 | 0.774172 | 1.005447 | — | — | — |
| test_is_placeholder.test | COMPILE | 0.056910 | 0.071930 | 0.077569 | — | — | — |
| fail_default_construct.test | COMPILE_FAIL | 0.422201 | 0.756285 | 0.942090 | — | — | — |
| fail_construct_mismatch.test | COMPILE_FAIL | 0.426073 | 0.770722 | 0.858387 | — | — | — |
| fail_construct_mismatch_ref.test | COMPILE_FAIL | 0.425450 | 0.755321 | 0.940877 | — | — | — |
| fail_construct_mismatch_cref.test | COMPILE_FAIL | 0.426187 | 0.769026 | 0.937125 | — | — | — |
| fail_construct_mismatch_rref.test | COMPILE_FAIL | 0.415419 | 0.709471 | 0.844888 | — | — | — |
| fail_binding_convert_no_mapping.test | COMPILE_FAIL | 0.408834 | 0.739463 | 0.913380 | — | — | — |
| fail_increment_discard_const.test | COMPILE_FAIL | 0.425009 | 0.767394 | 0.941769 | — | — | — |
| fail_not_incrementable.test | COMPILE_FAIL | 0.440434 | 0.732786 | 0.899053 | — | — | — |
| fail_ref_assign.test | COMPILE_FAIL | 0.415469 | 0.735966 | 0.913778 | — | — | — |
| fail_cref_assign.test | COMPILE_FAIL | 0.419251 | 0.745245 | 0.938742 | — | — | — |
| fail_rref_assign.test | COMPILE_FAIL | 0.421308 | 0.751972 | 0.861752 | — | — | — |
| fail_ref_discard_const.test | COMPILE_FAIL | 0.420299 | 0.759076 | 0.939813 | — | — | — |
| fail_ref_discard_const_convert.test | COMPILE_FAIL | 0.419381 | 0.760347 | 0.968055 | — | — | — |
| fail_ref_discard_const_convert_ref.test | COMPILE_FAIL | 0.423324 | 0.807093 | 0.894731 | — | — | — |
| fail_ref_discard_const_convert_cref.test | COMPILE_FAIL | 0.426218 | 0.800922 | 0.969344 | — | — | — |
| fail_ref_discard_const_init.test | COMPILE_FAIL | 0.417971 | 0.707054 | 0.914624 | — | — | — |
| fail_any_cast_discard_const1.test | COMPILE_FAIL | 0.420790 | 0.784264 | 0.897523 | — | — | — |
| fail_any_cast_discard_const2.test | COMPILE_FAIL | 0.423900 | 0.775197 | 0.970431 | — | — | — |
| fail_any_cast_discard_const3.test | COMPILE_FAIL | 0.425408 | 0.720089 | 0.979691 | — | — | — |
| fail_any_cast_discard_const4.test | COMPILE_FAIL | 0.425501 | 0.779282 | 0.960396 | — | — | — |
| fail_any_cast_discard_const5.test | COMPILE_FAIL | 0.424036 | 0.747459 | 0.977292 | — | — | — |
| fail_any_cast_discard_const6.test | COMPILE_FAIL | 0.371483 | 0.772357 | 0.981668 | — | — | — |
| fail_any_cast_pointer_to_ref.test | COMPILE_FAIL | 0.409250 | 0.709631 | 0.870549 | — | — | — |
| fail_any_cast_pointer_to_val.test | COMPILE_FAIL | 0.390872 | 0.508318 | 0.642596 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.955× | 0.964×, 0.947× | 0.991× |
| clang | 0.951× | 0.952×, 0.951× | 0.990× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| test_add.test | 10 | 0.001827s | 0.001722s | 0.001712s | byte equality observed |
| test_add_assign.test | 10 | 0.001734s | 0.001741s | 0.001679s | byte equality observed |
| test_any_cast.test | 10 | 0.001903s | 0.001836s | 0.001839s | byte equality observed |
| test_assign.test | 4 | 0.005312s | 0.004536s | 0.004985s | byte equality observed |
| test_binding.test | 10 | 0.001667s | 0.001592s | 0.001622s | byte equality observed |
| test_binding_of.test | 10 | 0.001607s | 0.001571s | 0.001535s | byte equality observed |
| test_callable.test | 8 | 0.002308s | 0.002184s | 0.002211s | byte equality observed |
| test_construct.test | 7 | 0.002987s | 0.002877s | 0.002815s | byte equality observed |
| test_construct_cref.test | 9 | 0.002122s | 0.002015s | 0.002026s | byte equality observed |
| test_construct_ref.test | 8 | 0.002023s | 0.001959s | 0.001856s | byte equality observed |
| test_deduced.test | 10 | 0.001672s | 0.001599s | 0.001561s | byte equality observed |
| test_dereference.test | 10 | 0.001661s | 0.001614s | 0.001612s | byte equality observed |
| test_dynamic_any_cast.test | 8 | 0.002115s | 0.002104s | 0.002046s | byte equality observed |
| test_equal.test | 9 | 0.001889s | 0.001899s | 0.001837s | byte equality observed |
| test_forward_iterator.test | 10 | 0.001676s | 0.001564s | 0.001612s | byte equality observed |
| test_free.test | 9 | 0.001873s | 0.001851s | 0.001831s | byte equality observed |
| test_increment.test | 10 | 0.001685s | 0.001548s | 0.001536s | byte equality observed |
| test_is_empty.test | 9 | 0.001646s | 0.001567s | 0.001606s | byte equality observed |
| test_less.test | 9 | 0.001999s | 0.001937s | 0.001898s | byte equality observed |
| test_limits.test | 10 | 0.001765s | 0.001688s | 0.001650s | byte equality observed |
| test_member.test | 9 | 0.002183s | 0.002067s | 0.002071s | byte equality observed |
| test_negate.test | 10 | 0.001625s | 0.001585s | 0.001533s | byte equality observed |
| test_nested.test | 10 | 0.001672s | 0.001572s | 0.001563s | byte equality observed |
| test_null.test | 8 | 0.001677s | 0.001597s | 0.001689s | byte equality observed |
| test_reference.test | 10 | 0.001676s | 0.001617s | 0.001604s | byte equality observed |
| test_relaxed.test | 10 | 0.001688s | 0.001565s | 0.001672s | byte equality observed |
| test_same_type.test | 10 | 0.001648s | 0.001571s | 0.001601s | byte equality observed |
| test_stream.test | 9 | 0.002081s | 0.002052s | 0.001952s | byte equality observed |
| test_subscript.test | 10 | 0.001767s | 0.001669s | 0.001737s | byte equality observed |
| test_tuple.test | 10 | 0.001678s | 0.001631s | 0.001610s | byte equality observed |
| test_typeid_of.test | 10 | 0.001737s | 0.001685s | 0.001671s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
