#!/bin/bash
set -u

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
tool_dir="${1:-${repo_root}/dev}"
tmpdir="$(mktemp -d "${TMPDIR:-/tmp}/cppgm-host-eh-object-smoke.XXXXXX")"
trap 'rm -rf "${tmpdir}"' EXIT

if [ "$(uname -s)" != "Darwin" ]; then
  echo "host EH object smoke skipped: Mach-O sections only apply on macOS"
  exit 0
fi

src="${tmpdir}/eh.cpp"
obj="${tmpdir}/eh.o"
roundtrip_obj="${tmpdir}/eh-roundtrip.o"
stdout="${tmpdir}/stdout"
stderr="${tmpdir}/stderr"

cat > "${src}" <<'EOF'
int f() { throw 7; }

int main()
{
  try {
    return f();
  } catch(int x) {
    return x;
  }
}
EOF

if ! "${tool_dir}/cppgm++" -c -o "${obj}" "${src}" > "${stdout}" 2> "${stderr}"; then
  echo "cppgm++ host EH object smoke failed" >&2
  sed -n '1,80p' "${stderr}" >&2
  exit 1
fi

require_section() {
  local object="$1"
  local section="$2"
  if ! otool -l "${object}" | grep -F "sectname ${section}" >/dev/null; then
    echo "host EH object smoke missing ${section} in ${object}" >&2
    exit 1
  fi
}

require_section "${obj}" "__eh_frame"
require_section "${obj}" "__gcc_except_tab"
require_section "${obj}" "__compact_unwind"

if ! "${tool_dir}/mobjroundtrip" "${obj}" "${roundtrip_obj}" > "${stdout}" 2> "${stderr}"; then
  echo "mobjroundtrip host EH object smoke failed" >&2
  sed -n '1,80p' "${stderr}" >&2
  exit 1
fi

require_section "${roundtrip_obj}" "__eh_frame"
require_section "${roundtrip_obj}" "__gcc_except_tab"
require_section "${roundtrip_obj}" "__compact_unwind"
