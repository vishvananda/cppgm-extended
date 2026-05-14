#!/bin/bash
set -u

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
tool_dir="${1:-${repo_root}/dev}"
tmpdir="$(mktemp -d "${TMPDIR:-/tmp}/cppgm-semantic-hotspot-smoke.XXXXXX")"
trap 'rm -rf "${tmpdir}"' EXIT

src="${tmpdir}/semantic-hotspot-smoke.cpp"
out="${tmpdir}/semantic-hotspot-smoke.sem"
stdout="${tmpdir}/semantic-hotspot-smoke.stdout"
stderr="${tmpdir}/semantic-hotspot-smoke.stderr"

cat > "${src}" <<'EOF'
template<class T>
struct Box {
  T value;
  Box() : value() {}
  T get() const { return value; }
};

template<class T>
T twice(T value)
{
  return value + value;
}

int main()
{
  Box<int> a;
  Box<int> b;
  return twice(a.get()) + twice(b.get());
}
EOF

if ! CPPGM_SEMANTIC_HOTSPOT=1 \
    CPPGM_SEMANTIC_HOTSPOT_DUMP_QUERY='*' \
    CPPGM_SEMANTIC_HOTSPOT_TRACE_QUERY='Box' \
    CPPGM_SEMANTIC_HOTSPOT_TRACE_NODE='id-expression' \
    CPPGM_SEMANTIC_HOTSPOT_TRACE_LIMIT=2 \
    CPPGM_SEMANTIC_HOTSPOT_DUMP_LIMIT=2 \
    "${tool_dir}/cppgm++" --emit-semantics -o "${out}" "${src}" \
      > "${stdout}" 2> "${stderr}"; then
  echo "cppgm++ semantic hotspot smoke failed" >&2
  sed -n '1,80p' "${stderr}" >&2
  exit 1
fi

for marker in \
    'SEMANTIC_HOTSPOT summary' \
    'SEMANTIC_HOTSPOT query_trace' \
    'SEMANTIC_HOTSPOT node_trace' \
    'SEMANTIC_HOTSPOT semantic_queries' \
    'SEMANTIC_HOTSPOT query_dump'; do
  if ! grep -F "${marker}" "${stderr}" >/dev/null; then
    echo "semantic hotspot smoke did not emit ${marker}" >&2
    sed -n '1,80p' "${stderr}" >&2
    exit 1
  fi
done

if [ ! -s "${out}" ]; then
  echo "semantic hotspot smoke produced empty semantic output" >&2
  exit 1
fi
