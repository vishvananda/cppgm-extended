# Course optimization kernel calibration

This maintainer record is outside the student document manifest. Raw results,
frozen inputs, executable hashes, counts and command observations are retained
in `obj/tmp/optimization-course/calibration/`. The runner records build wall
cost and peak RSS separately from generated-code measurements.

Calibration used GCC 15.2.0 and Clang 22.1.2 on the Intel Xeon host. Native
variants use the same GCC-built driver; LowIR variants use the supplied
frontend and backend at O0, changing only the optimizer level. Every variant
passed the independent scalar oracle over the edge counts and alias scenarios.
Counts subtract an identical zero-work invocation. These are work ratios,
with lower being better; they are not hardware speed claims.

| Implementation | Search | Fields | Narrow arithmetic | Cold-call state |
| --- | ---: | ---: | ---: | ---: |
| gcc-o2 | 0.9999 | 0.9998 | 1.0000 | 1.0000 |
| gcc-o3 | 0.9999 | 0.9998 | 1.0000 | 1.0000 |
| clang-o2 | 0.7583 | 0.9287 | 1.1000 | 1.1253 |
| clang-o3 | 1.0127 | 0.9287 | 1.1000 | 1.1261 |
| lowir-o1 | 1.2630 | 1.4993 | 1.6331 | 1.9055 |
| lowir-o2 | 1.3218 | 1.4993 | 1.6331 | 1.9030 |
| lowir-o3 | 1.2864 | 1.4993 | 1.5998 | 1.8219 |

The native bound is 2.0 for every family. The LowIR bound is 2.0 for
search and 3.0 for the other families. This admits the measured alternative
compilers and several optimizer levels, with substantial room for different
register allocation and CFG choices. Per-fixture `.quality` files remain the
authoritative limits. Source and instruction matching play no part.

Negative controls force the candidate to O0 while retaining the normal GCC
comparison level. The otherwise-correct native search takes 2.4722 times GCC
work and fails; LowIR search takes 2.5161 times GCC work and fails its 2.0
limit. The weaker implementations remain correct under the scalar oracle.
The other families need not reject every weak implementation individually; the
course also retains its existing static outcome and growth checks.

Reproduce an alternative compiler or optimizer policy from the repository root:

```sh
python3 scripts/check_optimization_quality.py pa32/tests/kernels \
  --mode native --compiler clang++ --level 2 --scale 4 --out obj/calibration/clang-o2
python3 scripts/check_optimization_quality.py pa32/tests/kernels \
  --mode lowir --level 1 --scale 4 --out obj/calibration/lowir-o1
```

All six optional exercise families also passed both modes, including every
published length and overlap scenario. Their ratios are diagnostic and have no
completion threshold. The manual Public benchmarks workflow can collect their
Cachegrind and paired timing evidence on both CPU lanes with `course_quality`.
Each run retains its CPU identity, input/tool/output hashes, A/A blocks, every
ABBA/BAAB observation, compile cost and peak RSS.

## Compiler cost check

The pre-course compiler and this compiler were copied to immutable files.
The frozen Boost semantic-overload input was compiled at O0 and O3, with two
A/A blocks, four alternating ABBA/BAAB blocks and one precise-RSS block per
level. Both sides emitted identical object hashes at each level. All
observations remain in `obj/tmp/optimization-course/compiler-cost/`.

| Level | Instructions B/A | Peak RSS B/A | Wall B/A | A/A wall range |
| --- | ---: | ---: | ---: | --- |
| -O0 | 0.999969 | 0.999757 | 1.0125 | 0.822–1.082 |
| -O3 | 0.999997 | 1.000442 | 1.0109 | 0.963–1.062 |

Instruction work and peak memory are effectively unchanged. Concurrent host
activity made wall time noisy, including A/A; these timings do not support a
claim of a small speed change. The complete raw samples are retained.

## Two-CPU CI exercise evidence

Both manual jobs measured compiler revision `4c53891b4` in Ubuntu 26.04.
Both passed all 81 public programs, the three compiler builds and all six
optional exercise families. The exercise input hashes match across hosts;
every published tail and overlap scenario passed the scalar oracle.

- [EPYC run](https://github.com/vishvananda/cppgm-extended/actions/runs/41528163188): AMD EPYC 9V74 80-Core Processor.
- [Intel run](https://github.com/vishvananda/cppgm-extended/actions/runs/41528203472): Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz.

The following are candidate/GCC ratios, so lower is better. Work subtracts
startup instructions. Time is the median of six paired ABBA/BAAB blocks;
two A/A blocks precede them. These diagnostic timings impose no course gate.

| Optional exercise | EPYC work | Intel work | EPYC time | Intel time |
| --- | ---: | ---: | ---: | ---: |
| adjacent-store-groups | 1.667 | 1.667 | 1.279 | 1.313 |
| independent-math | 3.000 | 3.000 | 8.387 | 6.891 |
| mixed-width-lanes | 1.751 | 1.751 | 2.291 | 1.589 |
| ordered-floating-sum | 1.132 | 1.132 | 1.000 | 1.034 |
| strided-matrix | 4.000 | 4.000 | 2.040 | 2.138 |
| vector-overlap | 1.111 | 1.111 | 1.080 | 1.117 |

A/A block ratios span 0.971–1.011 on epyc, 0.820–1.115 on intel.
Keep these ranges with the individual timing blocks rather than treating a
small difference as a speed claim. The retained artifacts include all command
observations, counts, stdout hashes, tool and executable hashes, frozen inputs,
compile wall costs and peak RSS. They are under
`obj/tmp/optimization-course/cpu-epyc/` and `cpu-intel/`.
The Intel runner registration and credentials were removed after its one job.
