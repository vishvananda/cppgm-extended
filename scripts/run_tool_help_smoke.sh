#!/bin/bash
set -u

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
tool_dir="${1:-${repo_root}/dev}"
tmpdir="$(mktemp -d "${TMPDIR:-/tmp}/cppgm-tool-help-smoke.XXXXXX")"
trap 'rm -rf "${tmpdir}"' EXIT

status=0

check_contains() {
  local file="$1"
  local expected="$2"
  if ! grep -F -- "${expected}" "${file}" >/dev/null; then
    echo "missing expected help text '${expected}' in ${file}" >&2
    status=1
  fi
}

check_help() {
  local tool="$1"
  local flag="$2"
  local usage="$3"
  local marker="$4"
  local exe="${tool_dir}/${tool}"
  local out="${tmpdir}/${tool}.${flag//[^A-Za-z0-9_]/_}.stdout"
  local err="${tmpdir}/${tool}.${flag//[^A-Za-z0-9_]/_}.stderr"

  if [ ! -x "${exe}" ]; then
    echo "missing executable ${exe}" >&2
    status=1
    return
  fi

  if ! "${exe}" "${flag}" > "${out}" 2> "${err}"; then
    echo "${tool} ${flag} failed" >&2
    sed -n '1,20p' "${err}" >&2
    status=1
    return
  fi
  if [ -s "${err}" ]; then
    echo "${tool} ${flag} wrote unexpected stderr" >&2
    sed -n '1,20p' "${err}" >&2
    status=1
  fi
  check_contains "${out}" "${usage}"
  check_contains "${out}" "query flags:"
  check_contains "${out}" "${marker}"
}

check_help 'cppgm++' '--help' 'usage: cppgm++' '-dumpmachine'
check_help 'cppgm++' '-h' 'usage: cppgm++' '-print-search-dirs'
check_help 'lowir2cy86' '--help' 'usage: lowir2cy86' '-o <outfile>'
check_help 'lowir2native' '-h' 'usage: lowir2native' '--dump-machine-ir'
check_help 'lowiropt' '--help' 'usage: lowiropt' '-O0 / -O1 / -O2'
check_help 'cpplink' '--help' 'usage: cpplink' '--dump-link-map'
check_help 'cppeh' '-h' 'usage: cppeh' '--dump-link-map'

exit "${status}"
