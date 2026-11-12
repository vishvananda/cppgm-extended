# libs/proto/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| calculator.test | RUN | 0.781768 | 1.917821 | 2.623401 | 0.004382 | 0.003767 | 0.004011 |
| constrained_ops.test | RUN | 0.860140 | 2.030837 | 2.949161 | 0.003425 | 0.003769 | 0.003888 |
| cpp-next_bug.test | RUN | 0.863298 | 2.254554 | 3.037804 | 0.003443 | 0.003681 | 0.003923 |
| deep_copy.test | RUN | 0.729456 | 1.915581 | 2.523936 | 0.003442 | 0.003852 | 0.003619 |
| display_expr.test | RUN | 0.894617 | 2.385726 | 3.073553 | 0.003452 | 0.003666 | 0.004864 |
| deduce_domain.test | RUN | 0.709637 | 1.699813 | 2.270819 | 0.003490 | 0.003454 | 0.003784 |
| env_var.test | RUN | 0.946530 | 4.018409 | 3.640487 | 0.004630 | 0.004625 | 0.004982 |
| examples.test | RUN | 0.913922 | 2.659393 | 3.455092 | 0.003703 | 0.003449 | 0.003956 |
| external_transforms.test | RUN | 0.901251 | 2.355827 | 3.203347 | 0.003506 | 0.003370 | 0.003372 |
| lambda.test | RUN | 0.889141 | 2.298890 | 3.144231 | 0.003640 | 0.003776 | 0.004087 |
| make_expr.test | RUN | 0.878878 | 3.789074 | 3.337730 | 0.004637 | 0.004837 | 0.005059 |
| matches.test | RUN | 0.785374 | 1.914515 | 2.806927 | 0.003562 | 0.003417 | 0.003871 |
| flatten.test | RUN | 1.486052 | 5.278025 | 6.493114 | 0.003962 | 0.004135 | 0.004338 |
| switch.test | RUN | 0.770260 | 1.888732 | 2.599844 | 0.003626 | 0.004546 | 0.003937 |
| toy_spirit.test | RUN | 0.862153 | 2.394545 | 3.256103 | 0.003872 | 0.003966 | 0.003828 |
| toy_spirit2.test | RUN | 0.958018 | 2.509484 | 3.452100 | 0.003188 | 0.003493 | 0.003983 |
| make.test | RUN | 0.758583 | 2.369921 | 2.595897 | 0.004279 | 0.004193 | 0.004437 |
| mem_ptr.test | RUN | 0.965976 | 3.925184 | 3.706140 | 0.004214 | 0.004827 | 0.004298 |
| mpl.test | RUN | 0.887268 | 2.042939 | 2.794783 | 0.003652 | 0.003648 | 0.003430 |
| noinvoke.test | RUN | 0.755060 | 1.727957 | 2.390892 | 0.003681 | 0.004097 | 0.003923 |
| pack_expansion.test | RUN | 0.908284 | 2.524465 | 2.896043 | 0.002812 | 0.002775 | 0.004041 |
| protect.test | RUN | 0.665562 | 1.668202 | 1.691014 | 0.002759 | 0.003631 | 0.002664 |
| bug2407.test | COMPILE | 0.497406 | 1.185643 | 1.514984 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.988× | 0.976×, 1.001× | 0.997× |
| clang | 0.971× | 0.973×, 0.970× | 0.997× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898266).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| calculator.test | 9 | 0.002173s | 0.002113s | 0.001975s | byte equality observed |
| constrained_ops.test | 9 | 0.002149s | 0.002058s | 0.001943s | byte equality observed |
| cpp-next_bug.test | 9 | 0.002149s | 0.002079s | 0.001992s | byte equality observed |
| deduce_domain.test | 9 | 0.002087s | 0.002105s | 0.001990s | byte equality observed |
| deep_copy.test | 8 | 0.002115s | 0.002158s | 0.001976s | byte equality observed |
| display_expr.test | 8 | 0.002132s | 0.002087s | 0.002070s | byte equality observed |
| env_var.test | 8 | 0.002400s | 0.002387s | 0.002277s | byte equality observed |
| examples.test | 9 | 0.002066s | 0.002116s | 0.002087s | byte equality observed |
| external_transforms.test | 9 | 0.002060s | 0.002102s | 0.002052s | byte equality observed |
| flatten.test | 9 | 0.002188s | 0.002247s | 0.002109s | byte equality observed |
| lambda.test | 9 | 0.002090s | 0.002102s | 0.002050s | byte equality observed |
| make.test | 8 | 0.002257s | 0.002299s | 0.002223s | byte equality observed |
| make_expr.test | 8 | 0.002527s | 0.002574s | 0.002458s | byte equality observed |
| matches.test | 9 | 0.002095s | 0.002139s | 0.002047s | byte equality observed |
| mem_ptr.test | 8 | 0.002387s | 0.002393s | 0.002335s | byte equality observed |
| mpl.test | 8 | 0.002115s | 0.002092s | 0.002019s | byte equality observed |
| noinvoke.test | 9 | 0.002064s | 0.002151s | 0.001993s | byte equality observed |
| pack_expansion.test | 8 | 0.002200s | 0.002083s | 0.002105s | byte equality observed |
| protect.test | 9 | 0.002191s | 0.002194s | 0.002065s | byte equality observed |
| switch.test | 9 | 0.002098s | 0.002082s | 0.001991s | byte equality observed |
| toy_spirit.test | 9 | 0.002111s | 0.002131s | 0.002015s | byte equality observed |
| toy_spirit2.test | 8 | 0.002147s | 0.002082s | 0.002111s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898266).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
