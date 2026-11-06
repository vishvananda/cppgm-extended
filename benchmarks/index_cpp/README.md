# index-cpp compatibility check

[index-cpp](https://github.com/kostya/index-cpp) is an upstream C++17 compiler
stress test with 50 checksummed workloads. Keep its source unchanged. The
reference compiler passes all 50 checks at `-O0`, `-O2` and `-O3` with GCC 15's
hosted headers and libraries, including the full `-O3` workload run.

From the repository root:

```sh
git clone https://github.com/kostya/index-cpp /tmp/cppgm-index-cpp
git -C /tmp/cppgm-index-cpp checkout 79e0b423094d420da37fb714f20c53fc35eeb2a3
make
dev/cppgm++ -std=c++17 -O3 -c /tmp/cppgm-index-cpp/index.cpp -o /tmp/cppgm-index.o
g++ /tmp/cppgm-index.o -lpthread -o /tmp/cppgm-index
TEST=1 /tmp/cppgm-index
```

Omit `TEST=1` for the full workload. The host compiler links the generated
object against its standard library; use the same host toolchain throughout.
This exercises selected C++14/17 features and does not establish full C++17
conformance. The check is optional and requires network access for the clone.
The local reducers run with `make test-modern-cxx`; they also ship as optional
PA29/PA31 extensions and are excluded from the required C++11 course suites.

For paired runtime measurements, supply immutable executables built from the
pinned source with the same flags and host library:

```sh
python3 scripts/index_cpp_benchmarks.py --source-root /tmp/cppgm-index-cpp \
  --binary baseline=/path/to/previous-index --binary cppgm=/tmp/cppgm-index \
  --binary gcc=/path/to/gcc-index --binary clang=/path/to/clang-index \
  --candidate cppgm --out /tmp/index-paired --cpus 0-21 --blocks 3
```

The receipt directory must be new. `--only Brainfuck::Recursion` selects the
upstream substring filter; repeat `--only` for disjoint groups. Every binary
first passes all 50 checks with `TEST=1`; timed runs always use full inputs.
Choose a CPU list suitable for the host, with enough cores for threaded cases.
The Linux runner retains raw output, executable/source hashes, load samples,
process counters and A/A plus ABBA/BAAB observations. `report.json` separates
upstream loop wall time from process CPU time and reports higher-is-better
speed ratios. `--metadata` attaches build commands, toolchain versions and
header manifests. Calibration exposes noise; it never discards observations.
