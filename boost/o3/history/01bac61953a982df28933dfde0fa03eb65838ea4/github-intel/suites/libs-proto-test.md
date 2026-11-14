# libs/proto/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| calculator.test | RUN | 0.436834 | 1.164752 | 1.338736 | 0.002214 | 0.001768 | 0.002194 |
| constrained_ops.test | RUN | 0.495557 | 1.242694 | 1.592189 | 0.002188 | 0.001868 | 0.002010 |
| cpp-next_bug.test | RUN | 0.507864 | 1.346592 | 1.562361 | 0.001944 | 0.001938 | 0.001964 |
| deep_copy.test | RUN | 0.444572 | 1.164849 | 1.330589 | 0.001797 | 0.001839 | 0.001968 |
| display_expr.test | RUN | 0.527415 | 1.463230 | 1.661246 | 0.001906 | 0.001859 | 0.002065 |
| deduce_domain.test | RUN | 0.418784 | 1.045946 | 1.227181 | 0.002049 | 0.001579 | 0.002196 |
| env_var.test | RUN | 0.568417 | 2.551466 | 2.021045 | 0.002085 | 0.002200 | 0.002463 |
| examples.test | RUN | 0.540173 | 1.652451 | 1.851775 | 0.001876 | 0.001951 | 0.002035 |
| external_transforms.test | RUN | 0.537690 | 1.369619 | 1.672120 | 0.001979 | 0.001786 | 0.002178 |
| lambda.test | RUN | 0.520678 | 1.410406 | 1.664253 | 0.001990 | 0.002264 | 0.002028 |
| make_expr.test | RUN | 0.526706 | 2.377166 | 1.790296 | 0.002100 | 0.002443 | 0.002491 |
| matches.test | RUN | 0.467359 | 1.166600 | 1.434349 | 0.001869 | 0.002105 | 0.002223 |
| flatten.test | RUN | 0.876015 | 3.363430 | 3.601264 | 0.002104 | 0.002048 | 0.002016 |
| switch.test | RUN | 0.442648 | 1.177260 | 1.369488 | 0.001933 | 0.002070 | 0.001992 |
| toy_spirit.test | RUN | 0.504426 | 1.483071 | 1.743279 | 0.002040 | 0.002131 | 0.002200 |
| toy_spirit2.test | RUN | 0.562114 | 1.587321 | 1.890993 | 0.002196 | 0.002114 | 0.002052 |
| make.test | RUN | 0.427355 | 1.515664 | 1.354232 | 0.002288 | 0.002263 | 0.002240 |
| mem_ptr.test | RUN | 0.580606 | 2.458843 | 1.953698 | 0.002409 | 0.002356 | 0.002482 |
| mpl.test | RUN | 0.500463 | 1.221082 | 1.491614 | 0.001948 | 0.002000 | 0.002234 |
| noinvoke.test | RUN | 0.424901 | 1.047595 | 1.214440 | 0.001995 | 0.002005 | 0.002124 |
| pack_expansion.test | RUN | 0.522756 | 1.536370 | 1.626429 | 0.002446 | 0.002007 | 0.002213 |
| protect.test | RUN | 0.336258 | 0.993474 | 0.951816 | 0.001485 | 0.001481 | 0.001572 |
| bug2407.test | COMPILE | 0.376010 | 0.716394 | 0.876295 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.989× | 0.984×, 0.994× | 1.003× |
| clang | 0.967× | 0.963×, 0.972× | 1.004× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814672).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| calculator.test | 10 | 0.001267s | 0.001240s | 0.001248s | byte equality observed |
| constrained_ops.test | 10 | 0.001272s | 0.001262s | 0.001260s | byte equality observed |
| cpp-next_bug.test | 10 | 0.001261s | 0.001214s | 0.001251s | byte equality observed |
| deduce_domain.test | 10 | 0.001260s | 0.001226s | 0.001237s | byte equality observed |
| deep_copy.test | 10 | 0.001260s | 0.001228s | 0.001234s | byte equality observed |
| display_expr.test | 10 | 0.001249s | 0.001233s | 0.001221s | byte equality observed |
| env_var.test | 10 | 0.001387s | 0.001357s | 0.001307s | byte equality observed |
| examples.test | 10 | 0.001278s | 0.001280s | 0.001215s | byte equality observed |
| external_transforms.test | 10 | 0.001284s | 0.001245s | 0.001208s | byte equality observed |
| flatten.test | 10 | 0.001293s | 0.001267s | 0.001241s | byte equality observed |
| lambda.test | 10 | 0.001263s | 0.001244s | 0.001194s | byte equality observed |
| make.test | 10 | 0.001314s | 0.001299s | 0.001257s | byte equality observed |
| make_expr.test | 10 | 0.001406s | 0.001378s | 0.001341s | byte equality observed |
| matches.test | 10 | 0.001270s | 0.001270s | 0.001255s | byte equality observed |
| mem_ptr.test | 10 | 0.001386s | 0.001359s | 0.001316s | byte equality observed |
| mpl.test | 10 | 0.001254s | 0.001272s | 0.001190s | byte equality observed |
| noinvoke.test | 10 | 0.001257s | 0.001228s | 0.001205s | byte equality observed |
| pack_expansion.test | 10 | 0.001264s | 0.001283s | 0.001235s | byte equality observed |
| protect.test | 10 | 0.001278s | 0.001267s | 0.001260s | byte equality observed |
| switch.test | 10 | 0.001267s | 0.001247s | 0.001214s | byte equality observed |
| toy_spirit.test | 10 | 0.001247s | 0.001242s | 0.001200s | byte equality observed |
| toy_spirit2.test | 10 | 0.001248s | 0.001227s | 0.001220s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814672).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
