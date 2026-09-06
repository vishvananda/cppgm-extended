#!/bin/bash
# Time one compiler binary on the eight largest translation units of the
# compiler, serially, one process each, best of N runs.  This is the workload
# PLAN-LIBCXX-PERFORMANCE.md defines its targets on: run it once per lane
# binary (host-built and self-built, each cell) at -O1 and at -O3 and read the
# self-built over host-built ratio.
#
#   scripts/lane_workload_benchmark.sh <compiler> <label> <O1|O3> [generated-dir] [runs]
#
# <generated-dir> is the cell's generated header directory, obj/generated for
# the default cell and obj-clang-libcxx/generated for the libc++ cell; it
# defaults to obj/generated.  Objects go under $LANE_BENCH_OUT, which defaults
# to $HOME/lane-bench-out, not the scratchpad, whose quota surfaces as
# spurious compiler failures.
set -u
cd "$(dirname "$0")/.."
cxx=$1; label=$2; opt=$3; generated=${4:-obj/generated}; runs=${5:-3}
out=${LANE_BENCH_OUT:-$HOME/lane-bench-out}
mkdir -p "$out"
workload="
dev/src/native/lowering/function.cpp
dev/src/lowir/optimize/pipeline.cpp
dev/src/semantic/declarations/analysis.cpp
dev/src/semantic/analysis/analyzer.cpp
dev/src/native/object/elf_writer.cpp
dev/src/semantic/initialization/analysis.cpp
dev/src/semantic/templates/classes.cpp
dev/src/semantic/constants/scalar_evaluator.cpp
"
best=""
for run in $(seq 1 "$runs"); do
  start=$(date +%s%N)
  for f in $workload; do
    "$cxx" -std=gnu++11 -Wall "-$opt" -I dev/src -I "$generated" \
      -c -o "$out/$(basename "$f").$label.o" "$f" >/dev/null 2>&1 || {
      echo "$label FAILED on $f" >&2; exit 1; }
  done
  end=$(date +%s%N)
  t=$(( (end - start) / 1000000 ))
  if [ -z "$best" ] || [ "$t" -lt "$best" ]; then best=$t; fi
done
printf '%-28s %-3s %6d.%03d s\n' "$label" "$opt" $((best / 1000)) $((best % 1000))
