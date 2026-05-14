#!/bin/bash
set -u

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
tool_dir="${1:-${repo_root}/dev}"
tmpdir="$(mktemp -d "${TMPDIR:-/tmp}/cppgm-memory-census-smoke.XXXXXX")"
trap 'rm -rf "${tmpdir}"' EXIT

src="${tmpdir}/memory-census-smoke.cpp"
out="${tmpdir}/memory-census-smoke.out"
stdout="${tmpdir}/memory-census-smoke.stdout"
stderr="${tmpdir}/memory-census-smoke.stderr"

cat > "${src}" <<'EOF'
template<class T>
struct Box {
  T value;
  T get() { return value; }
};

template<class T>
using BoxAlias = Box<T>;

int add(int a, int b)
{
  return a + b;
}

int main()
{
  BoxAlias<int> box;
  box.value = 3;
  return add(box.get(), 4);
}
EOF

if ! CPPGM_MEMORY_CENSUS=1 "${tool_dir}/cppgm++" --emit-semantics -o "${out}" "${src}" \
    > "${stdout}" 2> "${stderr}"; then
  echo "cppgm++ memory census smoke failed" >&2
  sed -n '1,40p' "${stderr}" >&2
  exit 1
fi

if ! grep -F 'semantic-memory kind=' "${stderr}" >/dev/null; then
  echo "memory census smoke did not emit semantic-memory entries" >&2
  sed -n '1,40p' "${stderr}" >&2
  exit 1
fi

if ! grep -F 'semantic-memory-total' "${stderr}" >/dev/null; then
  echo "memory census smoke did not emit semantic-memory-total" >&2
  sed -n '1,40p' "${stderr}" >&2
  exit 1
fi

if [ ! -s "${out}" ]; then
  echo "memory census smoke produced empty semantic output" >&2
  exit 1
fi
