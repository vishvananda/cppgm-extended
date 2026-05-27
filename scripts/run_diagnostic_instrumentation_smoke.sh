#!/bin/bash
set -u

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
tool_dir="${1:-${repo_root}/dev}"
tmpdir="$(mktemp -d "${TMPDIR:-/tmp}/cppgm-diagnostic-smoke.XXXXXX")"
trap 'rm -rf "${tmpdir}"' EXIT

status=0

require_marker() {
  local file="$1"
  local marker="$2"
  local label="$3"
  if ! grep -F "${marker}" "${file}" >/dev/null; then
    echo "diagnostic smoke missing ${label}: ${marker}" >&2
    sed -n '1,80p' "${file}" >&2
    status=1
  fi
}

run_timing_probe() {
  local src="${tmpdir}/timing-smoke.cpp"
  local out="${tmpdir}/timing-smoke.ast"
  local stdout="${tmpdir}/timing-smoke.stdout"
  local stderr="${tmpdir}/timing-smoke.stderr"

  {
    echo 'int f0(){return 0;}'
    for i in $(seq 1 300); do
      echo "int f${i}(){return f$((i - 1))()+${i};}"
    done
    echo 'int main(){return f300();}'
  } > "${src}"

  if ! CPPGM_STARTUP_TIMING=1 \
      CPPGM_FILE_TIMING=1 \
      CPPGM_FILE_TIMING_FILTER=timing-smoke.cpp \
      CPPGM_FILE_TIMING_LIMIT=5 \
      CPPGM_FILE_TIMING_LIVE=1 \
      CPPGM_FILE_TIMING_STEP_MS=1 \
      "${tool_dir}/cppgm++" --emit-ast -o "${out}" "${src}" \
        > "${stdout}" 2> "${stderr}"; then
    echo "cppgm++ timing diagnostic smoke failed" >&2
    sed -n '1,80p' "${stderr}" >&2
    status=1
    return
  fi

  require_marker "${stderr}" '[startup.timing] main.enter' 'startup timing'
  require_marker "${stderr}" '[startup.timing] main.exit' 'startup timing exit'
  require_marker "${stderr}" '[file.timing] parser.declaration' 'live file timing'
  require_marker "${stderr}" '[file.timing.summary] parser.declaration' 'file timing summary'
  if [ ! -s "${out}" ]; then
    echo "timing diagnostic smoke produced empty AST output" >&2
    status=1
  fi
}

run_parser_trace_probe() {
  local src="${tmpdir}/trace-smoke.cpp"
  local out="${tmpdir}/trace-smoke.sem"
  local stdout="${tmpdir}/trace-smoke.stdout"
  local stderr="${tmpdir}/trace-smoke.stderr"

  cat > "${src}" <<'EOF'
template<class T>
struct Box { T value; };

int main()
{
  Box<int> box;
  return 0;
}
EOF

  if ! CPPGM_TRACE=parser.decl \
      CPPGM_TRACE_LIVE=1 \
      CPPGM_TRACE_LIMIT=2 \
      CPPGM_TRACE_FILE=trace-smoke.cpp \
      CPPGM_TRACE_SYMBOL=Box \
      "${tool_dir}/cppgm++" --emit-semantics -o "${out}" "${src}" \
        > "${stdout}" 2> "${stderr}"; then
    echo "cppgm++ parser trace diagnostic smoke failed" >&2
    sed -n '1,80p' "${stderr}" >&2
    status=1
    return
  fi

  require_marker "${stderr}" '[parser.decl]' 'parser trace'
  require_marker "${stderr}" 'TT_IDENTIFIER:Box' 'parser trace symbol filter'
  if [ ! -s "${out}" ]; then
    echo "parser trace diagnostic smoke produced empty semantic output" >&2
    status=1
  fi
}

run_semantic_metrics_probe() {
  local src="${tmpdir}/semantic-metrics-smoke.cpp"
  local out="${tmpdir}/semantic-metrics-smoke.sem"
  local stdout="${tmpdir}/semantic-metrics-smoke.stdout"
  local stderr="${tmpdir}/semantic-metrics-smoke.stderr"

  cat > "${src}" <<'EOF'
template<class T>
struct Box {
  T value;
  T get() const { return value; }
};

int add(int a, int b) { return a + b; }

int main()
{
  Box<int> box;
  return add(box.get(), 4);
}
EOF

  if ! CPPGM_SEMANTIC_STATS=1 \
      CPPGM_SEMANTIC_CACHE_STATS=1 \
      CPPGM_SEMANTIC_PHASE_STATS=1 \
      "${tool_dir}/cppgm++" --emit-semantics -o "${out}" "${src}" \
        > "${stdout}" 2> "${stderr}"; then
    echo "cppgm++ semantic metrics diagnostic smoke failed" >&2
    sed -n '1,80p' "${stderr}" >&2
    status=1
    return
  fi

  require_marker "${stderr}" 'semantic-phase name=semantic.collect_declarations' 'semantic phase stats'
  require_marker "${stderr}" 'semantic-metrics template-instantiation-requests=' 'semantic metrics'
  require_marker "${stderr}" 'semantic-cache name=' 'semantic cache counters'
  require_marker "${stderr}" 'semantic-cache capture.scope=' 'semantic cache summary'
  if [ ! -s "${out}" ]; then
    echo "semantic metrics diagnostic smoke produced empty semantic output" >&2
    status=1
  fi
}

run_output_requirement_trace_probe() {
  local src="${tmpdir}/output-require-smoke.cpp"
  local out="${tmpdir}/output-require-smoke.lowir"
  local stdout="${tmpdir}/output-require-smoke.stdout"
  local stderr="${tmpdir}/output-require-smoke.stderr"

  cat > "${src}" <<'EOF'
struct Box {
  int value;
  Box();
  int get();
};

Box::Box() : value(7) {}
int Box::get() { return value; }

int main()
{
  Box box;
  return box.get();
}
EOF

  if ! CPPGM_TRACE=output.require \
      CPPGM_TRACE_LIVE=1 \
      CPPGM_TRACE_LIMIT=80 \
      "${tool_dir}/cppgm++" --emit-lowir -o "${out}" "${src}" \
        > "${stdout}" 2> "${stderr}"; then
    echo "cppgm++ output requirement trace diagnostic smoke failed" >&2
    sed -n '1,80p' "${stderr}" >&2
    status=1
    return
  fi

  require_marker "${stderr}" '[output.require]' 'output requirement trace'
  require_marker "${stderr}" 'action=require-definition entity=Box::Box' 'constructor output requirement trace'
  require_marker "${stderr}" 'action=insert-required-definition entity=Box::get' 'method output requirement trace'
  if [ ! -s "${out}" ]; then
    echo "output requirement trace diagnostic smoke produced empty LowIR output" >&2
    status=1
  fi
}

run_template_audit_probe() {
  local src="${repo_root}/pa34/tests/compile/641-incomplete-vector-signature.t"
  local out="${tmpdir}/template-audit-smoke.sem"
  local stdout="${tmpdir}/template-audit-smoke.stdout"
  local stderr="${tmpdir}/template-audit-smoke.stderr"

  if ! CPPGM_TEMPLATE_AUDIT=1 \
      "${tool_dir}/cppgm++" --emit-semantics -o "${out}" "${src}" \
        > "${stdout}" 2> "${stderr}"; then
    echo "cppgm++ template audit diagnostic smoke failed" >&2
    sed -n '1,80p' "${stderr}" >&2
    status=1
    return
  fi

  require_marker "${stderr}" 'UPGRADE:' 'template audit upgrade trace'
  require_marker "${stderr}" 'UPGRADE_FAIL:' 'template audit failure trace'
  require_marker "${stderr}" 'created_by=' 'template audit creation context'
  if [ ! -s "${out}" ]; then
    echo "template audit diagnostic smoke produced empty semantic output" >&2
    status=1
  fi
}

run_timing_probe
run_parser_trace_probe
run_semantic_metrics_probe
run_output_requirement_trace_probe
run_template_audit_probe

exit "${status}"
