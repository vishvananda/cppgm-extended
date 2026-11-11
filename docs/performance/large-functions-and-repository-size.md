# Large-function compile work and repository storage

Measured against main `a0ff7d04a` on 2026-11-11. The storage figures below
describe tracked data, not local worktree builds or caches.

| Main checkout contents | MiB |
|---|---:|
| PA25 numeric runtime vectors and expected outputs | 163.76 |
| Repeated grammar explorers and parser binaries | 106.95 |
| Language/ABI references under `doc/` | 28.22 |
| Fixed benchmarks | 10.73 |
| Compiler source and scaffolds under `dev/` | 9.40 |
| Other handouts, fixtures and reports | 49.55 |
| Total | 368.60 |

The grammar directories contain 97.21 MiB of generated HTML and 16 copies of
the same 0.61 MiB Linux parser binary. Their unique blobs total 7.84 MiB.
Centralizing those copies would save about 99 MiB in a full checkout; Git
already deduplicates identical blobs. PA25's largest input is a 34.09 MiB,
million-operation floating calculator corpus. Compressing or generating these
vectors is a separate harness change: preserve their cases and oracles rather
than removing coverage. Moving every test out of the course is unnecessary.

The much larger clone-size outlier was the generated `benchmark-badges` branch:
18.5 GiB of checked-out data, including 8.1 GiB of Boost logs and 7.0 GiB of
Boost XML reports. Repeated command manifests and replay observations made up
most of the remainder. Its reachable objects occupied about 283 MiB in the
local pack layout, versus 134 MiB for main. The reviewed compact tree packs to
3.18 MiB. All 134 SVGs are byte-identical and all 112 retained metadata
documents have identical values. See [publication storage](publication-storage.md)
for the artifact policy and the generated-branch migration.

The README's compiler-only sparse checkout builds successfully from `dev/`
alone, without the course corpora, references or benchmarks.

## Brainfuck compile regression

The [Myc Brainfuck generator](https://github.com/kostya/myc/tree/d31bd5b0112d54471b544991be688be83607f40f/benchmark/brainfuck-compiler)
produces a 38,396-line, 1,185,704-byte Mandelbrot LowIR function. The input hash
is `b9ffd318e2bd62862cbddf979dbe36a0740aa83df52c077ea3ab02d0cf31dc4e`.

The aggregate-field carrier work added in `cf3e0fca5` compared every recorded
access with every other access before deduplicating identical descriptors.
Repeated scalar-slot accesses therefore incurred quadratic compile work.
Sampling attributed 88.85% of compile cycles to aggregate-slot settling.

The repair orders and deduplicates descriptors first, then examines only
overlapping intervals. Scalar widths and the finite type vocabulary bound
the overlap window. It preserves the existing carrier eligibility rules.

| Frozen input | Main instructions | Fixed instructions | Change |
|---|---:|---:|---:|
| Brainfuck, O3 | 13.988 billion | 0.602 billion | −95.70% |
| Corrected Myc index-c, O3 | 2.131 billion | 2.121 billion | −0.50% |
| Corrected Myc index-c, O0 | 400.726 million | 400.740 million | +0.003% |

Immutable compilers, frozen inputs, one pinned CPU, A/A calibration and paired
blocks were used. All object outputs are identical between main and the fixed
compiler at each measured level. Brainfuck compile wall time fell from about
3.0 seconds to 0.28–0.32 seconds on this host. These compile-work changes do not
change generated-program runtime.

## Host compiler crashes

[Issue #130](https://github.com/vishvananda/cppgm-extended/issues/130) contains
GCC GIMPLE/ranger internal compiler errors. `CC=clang make` leaves this C++
build using its default `CXX=g++`; it does not exercise the displayed Clang
version. Select Clang with `CXX=clang++ CPPGM_HOST_CXX=clang++` in a fresh or
consistently rebuilt toolchain cell.

The actual GCC version and a repeatable failing command are needed to identify
a particular GCC defect. Changing failing translation units and success on
retry also warrant checking host memory/resource pressure and hardware
stability. Default parallelism uses every logical CPU. The reported SIGSEGV
alone does not establish an out-of-memory kill. GCC 15 builds this branch,
and GCC 15 and Clang 21 compile the changed native/optimizer units at O3 with
warnings as errors.
