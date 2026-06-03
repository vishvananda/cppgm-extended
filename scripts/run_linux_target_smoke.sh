#!/bin/bash
set -u

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
tool_dir="${1:-${repo_root}/dev}"
tmpdir="$(mktemp -d "${TMPDIR:-/tmp}/cppgm-linux-target-smoke.XXXXXX")"
trap 'rm -rf "${tmpdir}"' EXIT

out="${tmpdir}/ret42-linux"

if ! "${tool_dir}/cy86" --target linux -o "${out}" "${repo_root}/pa9/tests/100-ret42.t.1"; then
  echo "cy86 linux target smoke failed" >&2
  exit 1
fi

magic="$(od -An -tx1 -N4 "${out}" | tr -d ' \n')"
if [ "${magic}" != "7f454c46" ]; then
  echo "cy86 linux target smoke did not produce an ELF executable" >&2
  exit 1
fi

if [ ! -x "${out}" ]; then
  echo "cy86 linux target smoke output is not executable" >&2
  exit 1
fi

obj="${tmpdir}/main-linux.o"
roundtrip_obj="${tmpdir}/main-linux-roundtrip.o"

if ! CPPGM_WRITE_LOCAL_SYMBOL_MAP=1 \
    "${tool_dir}/cpplink" -c --target linux -o "${obj}" \
      "${repo_root}/pa28/tests/strict/100-ret0.t"; then
  echo "cpplink linux object smoke failed" >&2
  exit 1
fi

obj_magic="$(od -An -tx1 -N4 "${obj}" | tr -d ' \n')"
if [ "${obj_magic}" != "7f454c46" ]; then
  echo "cpplink linux object smoke did not produce an ELF object" >&2
  exit 1
fi

if [ ! -s "${obj}.localsymmap" ]; then
  echo "cpplink linux object smoke did not produce a local symbol map" >&2
  exit 1
fi

if ! "${tool_dir}/mobjroundtrip" "${obj}" "${roundtrip_obj}"; then
  echo "mobjroundtrip ELF smoke failed" >&2
  exit 1
fi

roundtrip_magic="$(od -An -tx1 -N4 "${roundtrip_obj}" | tr -d ' \n')"
if [ "${roundtrip_magic}" != "7f454c46" ]; then
  echo "mobjroundtrip ELF smoke did not preserve an ELF object" >&2
  exit 1
fi
