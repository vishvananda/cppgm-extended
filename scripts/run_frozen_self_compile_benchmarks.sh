#!/bin/bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

compiler="${COMPILER:-$repo_root/dev/cppgm++}"
host_cxx="${HOST_CXX:-${CPPGM_HOST_CXX:-clang++}}"
jobs="${JOBS:-1}"
timeout_sec="${TIMEOUT_SEC:-120}"
output_prefix="${OUTPUT_PREFIX:-/tmp/cppgm-frozen-self-compile}"

exec python3 "$repo_root/scripts/report_self_compile_sweep.py" \
  --repo-root "$repo_root" \
  --compiler "$compiler" \
  --host-cxx "$host_cxx" \
  --jobs "$jobs" \
  --timeout-sec "$timeout_sec" \
  --output-prefix "$output_prefix" \
  --source benchmarks/self_compile/stable/constant_value.cpp \
  --source benchmarks/self_compile/stable/recog_token_buffer.cpp \
  --source benchmarks/self_compile/stable/parser_trace.cpp \
  --source benchmarks/self_compile/stable/rtti_names.cpp \
  --source benchmarks/self_compile/stable/cppast_dump.cpp
