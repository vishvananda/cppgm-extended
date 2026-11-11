# libs/type_erasure/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| test_binding.test | RUN | 0.449211 | 1.231451 | 1.116976 | 0.004378 | 0.004366 | 0.004724 |
| test_increment.test | RUN | 0.472647 | 1.091625 | 1.159498 | 0.003902 | 0.004010 | 0.004047 |
| test_add.test | RUN | 0.572030 | 1.799176 | 1.602033 | 0.004110 | 0.004653 | 0.004191 |
| test_add_assign.test | RUN | 0.580573 | 1.903286 | 1.689366 | 0.004066 | 0.004556 | 0.004430 |
| test_callable.test | RUN | 0.861902 | 4.259108 | 3.435738 | 0.006210 | 0.006955 | 0.006122 |
| test_reference.test | RUN | 0.470063 | 1.121438 | 1.140289 | 0.004104 | 0.004426 | 0.004160 |
| test_construct.test | RUN | 1.191372 | 8.528067 | 6.056903 | 0.008461 | 0.009738 | 0.009185 |
| test_relaxed.test | RUN | 0.454160 | 1.025518 | 1.057085 | 0.003875 | 0.004297 | 0.004017 |
| test_assign.test | RUN | 2.238177 | 29.537015 | 15.659768 | 0.014773 | 0.011062 | 0.015173 |
| test_construct_ref.test | RUN | 0.523871 | 1.431143 | 1.521062 | 0.004680 | 0.005241 | 0.005242 |
| test_construct_cref.test | RUN | 0.534700 | 1.703005 | 1.646179 | 0.005719 | 0.006155 | 0.005845 |
| test_any_cast.test | RUN | 0.490910 | 1.369487 | 1.245274 | 0.004620 | 0.005016 | 0.005311 |
| test_binding_of.test | RUN | 0.435315 | 1.007661 | 1.020319 | 0.003655 | 0.003872 | 0.003526 |
| test_typeid_of.test | RUN | 0.462029 | 1.081274 | 1.098245 | 0.003894 | 0.004653 | 0.003984 |
| test_nested.test | RUN | 0.502550 | 1.168043 | 1.203974 | 0.003803 | 0.004126 | 0.003745 |
| test_less.test | RUN | 0.576513 | 1.985985 | 1.671617 | 0.004970 | 0.005276 | 0.004830 |
| test_equal.test | RUN | 0.566216 | 1.953623 | 1.580316 | 0.004451 | 0.005001 | 0.004430 |
| test_negate.test | RUN | 0.433575 | 1.102282 | 0.994447 | 0.003843 | 0.003581 | 0.003548 |
| test_dereference.test | RUN | 0.483059 | 1.219389 | 1.201137 | 0.003698 | 0.004228 | 0.004051 |
| test_subscript.test | RUN | 0.497441 | 1.310483 | 1.303030 | 0.003977 | 0.004234 | 0.004135 |
| test_forward_iterator.test | RUN | 0.561970 | 1.854305 | 1.518767 | 0.004050 | 0.004457 | 0.004115 |
| test_tuple.test | RUN | 0.496566 | 1.254756 | 1.216566 | 0.003961 | 0.004364 | 0.003866 |
| test_stream.test | RUN | 0.603787 | 2.243416 | 1.785203 | 0.005171 | 0.005607 | 0.005609 |
| test_deduced.test | RUN | 0.451603 | 0.971523 | 0.999297 | 0.003584 | 0.004062 | 0.003997 |
| test_same_type.test | RUN | 0.484183 | 1.213258 | 1.217663 | 0.003868 | 0.004126 | 0.004028 |
| test_member.test | RUN | 0.586995 | 2.102297 | 1.808234 | 0.005675 | 0.006247 | 0.006045 |
| test_null.test | RUN | 0.499782 | 1.036751 | 1.000385 | 0.003671 | 0.004401 | 0.003903 |
| test_free.test | RUN | 0.582267 | 1.476969 | 1.397150 | 0.004769 | 0.004903 | 0.004236 |
| test_is_empty.test | RUN | 0.499580 | 0.998312 | 1.020319 | 0.003840 | 0.004339 | 0.004012 |
| test_dynamic_any_cast.test | RUN | 0.693326 | 2.821411 | 2.347282 | 0.005502 | 0.005809 | 0.005802 |
| test_limits.test | RUN | 0.685826 | 2.770726 | 1.990188 | 0.003873 | 0.004843 | 0.003924 |
| test_param.test | COMPILE | 0.308112 | 0.664120 | 0.619457 | — | — | — |
| test_is_subconcept.test | COMPILE | 0.297921 | 0.556740 | 0.525408 | — | — | — |
| test_sfinae.test | COMPILE | 0.305426 | 0.554121 | 0.550415 | — | — | — |
| test_interface_order.test | COMPILE | 0.305881 | 0.516109 | 0.575404 | — | — | — |
| test_is_placeholder.test | COMPILE | 0.041854 | 0.049703 | 0.058870 | — | — | — |
| fail_default_construct.test | COMPILE_FAIL | 0.292931 | 0.528213 | 0.549343 | — | — | — |
| fail_construct_mismatch.test | COMPILE_FAIL | 0.291730 | 0.509729 | 0.551247 | — | — | — |
| fail_construct_mismatch_ref.test | COMPILE_FAIL | 0.283332 | 0.511821 | 0.545201 | — | — | — |
| fail_construct_mismatch_cref.test | COMPILE_FAIL | 0.280849 | 0.471088 | 0.537725 | — | — | — |
| fail_construct_mismatch_rref.test | COMPILE_FAIL | 0.281334 | 0.499715 | 0.545037 | — | — | — |
| fail_binding_convert_no_mapping.test | COMPILE_FAIL | 0.276425 | 0.479924 | 0.520962 | — | — | — |
| fail_increment_discard_const.test | COMPILE_FAIL | 0.284384 | 0.479833 | 0.539317 | — | — | — |
| fail_not_incrementable.test | COMPILE_FAIL | 0.295177 | 0.523984 | 0.577630 | — | — | — |
| fail_ref_assign.test | COMPILE_FAIL | 0.291534 | 0.499006 | 0.534607 | — | — | — |
| fail_cref_assign.test | COMPILE_FAIL | 0.290001 | 0.488991 | 0.551028 | — | — | — |
| fail_rref_assign.test | COMPILE_FAIL | 0.288829 | 0.492993 | 0.536976 | — | — | — |
| fail_ref_discard_const.test | COMPILE_FAIL | 0.281502 | 0.510770 | 0.548222 | — | — | — |
| fail_ref_discard_const_convert.test | COMPILE_FAIL | 0.283868 | 0.516921 | 0.565502 | — | — | — |
| fail_ref_discard_const_convert_ref.test | COMPILE_FAIL | 0.280891 | 0.529438 | 0.571685 | — | — | — |
| fail_ref_discard_const_convert_cref.test | COMPILE_FAIL | 0.283950 | 0.543202 | 0.570004 | — | — | — |
| fail_ref_discard_const_init.test | COMPILE_FAIL | 0.281776 | 0.494325 | 0.538470 | — | — | — |
| fail_any_cast_discard_const1.test | COMPILE_FAIL | 0.330194 | 0.506613 | 0.582753 | — | — | — |
| fail_any_cast_discard_const2.test | COMPILE_FAIL | 0.328099 | 0.519419 | 0.578449 | — | — | — |
| fail_any_cast_discard_const3.test | COMPILE_FAIL | 0.314495 | 0.540052 | 0.582616 | — | — | — |
| fail_any_cast_discard_const4.test | COMPILE_FAIL | 0.290996 | 0.498801 | 0.567342 | — | — | — |
| fail_any_cast_discard_const5.test | COMPILE_FAIL | 0.292178 | 0.517939 | 0.557886 | — | — | — |
| fail_any_cast_discard_const6.test | COMPILE_FAIL | 0.272839 | 0.552827 | 0.538356 | — | — | — |
| fail_any_cast_pointer_to_ref.test | COMPILE_FAIL | 0.248283 | 0.506884 | 0.552809 | — | — | — |
| fail_any_cast_pointer_to_val.test | COMPILE_FAIL | 0.242735 | 0.472121 | 0.390514 | — | — | — |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42562778965) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42562778965) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42562778965).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
