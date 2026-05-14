#!/bin/bash
set -u

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
tool_dir="${1:-${repo_root}/dev}"
tmpdir="$(mktemp -d "${TMPDIR:-/tmp}/cppgm-cli-batch-smoke.XXXXXX")"
trap 'rm -rf "${tmpdir}"' EXIT

tool="${tool_dir}/lowiropt"
input="${tmpdir}/input.lowir"
output="${tmpdir}/output.lowir"
request_stdout="${tmpdir}/request.stdout"
request_stderr="${tmpdir}/request.stderr"
batch_stdout="${tmpdir}/batch.stdout"
batch_stderr="${tmpdir}/batch.stderr"

cat > "${input}" <<'EOF'
function @main() -> i64 {
  block ^entry:
    return i64 0
}
EOF

if [ ! -x "${tool}" ]; then
  echo "missing executable ${tool}" >&2
  exit 1
fi

{
  printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' \
    "${request_stdout}" \
    "${request_stderr}" \
    "${tmpdir}" \
    'CPPGM_TRACE=all' \
    '-O0' \
    '-o' \
    "${output}" \
    "${input}"
  printf 'malformed\n'
} | WRAPPED_BATCH_STDIN=1 "${tool}" --batch-stdin > "${batch_stdout}" 2> "${batch_stderr}"

if [ "$?" -ne 0 ]; then
  echo "lowiropt cli batch smoke failed" >&2
  sed -n '1,80p' "${batch_stderr}" >&2
  exit 1
fi

if ! grep -Fx 'EXIT_SUCCESS' "${batch_stdout}" >/dev/null; then
  echo "cli batch smoke did not report EXIT_SUCCESS" >&2
  cat "${batch_stdout}" >&2
  exit 1
fi

if ! grep -Fx 'EXIT_FAILURE' "${batch_stdout}" >/dev/null; then
  echo "cli batch smoke did not report EXIT_FAILURE for malformed request" >&2
  cat "${batch_stdout}" >&2
  exit 1
fi

if [ -s "${batch_stderr}" ]; then
  echo "cli batch smoke wrote unexpected batch stderr" >&2
  sed -n '1,80p' "${batch_stderr}" >&2
  exit 1
fi

if [ -s "${request_stderr}" ]; then
  echo "cli batch smoke wrote unexpected request stderr" >&2
  sed -n '1,80p' "${request_stderr}" >&2
  exit 1
fi

if ! cmp -s "${input}" "${output}"; then
  echo "cli batch smoke did not round-trip O0 LowIR" >&2
  diff -u "${input}" "${output}" >&2 || true
  exit 1
fi
