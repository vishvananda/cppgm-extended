#!/bin/bash

set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
app="${CPPGM_APP:-../dev/cppgm++}"
app_args=()
if [ -n "${CPPGM_APP_ARGS:-}" ]; then
  read -r -a app_args <<< "${CPPGM_APP_ARGS}"
fi

cases=(
  "pa21/tests/spec/302-template-static-constant-minimum-chain"
  "pa21/tests/spec/303-template-static-constant-nontype-argument"
  "pa21/tests/spec/347-reference-shell-current-specialization-alias"
  "pa21/tests/spec/349-reference-shell-out-of-class-current-specialization-iterator"
  "pa21/tests/spec/385-member-template-shadowing-dependent-enable-if"
  "pa21/tests/spec/390-out-of-class-conversion-operator-definition"
  "pa21/tests/spec/391-explicit-specialization-cross-converting-ctor-body"
  "pa21/tests/spec/445-default-template-argument-merge"
  "pa18/tests/spec/190-bad-deduction"
  "pa18/tests/spec/207-constructor-template-direct-other-specialization"
  "pa18/tests/spec/210-defaulted-nested-class-template-deduction"
  "pa16/tests/spec/314-base-rvalue-reference-assignment"
  "pa15/tests/spec/255-derived-pointer-member-init"
)

status_name() {
  case "$1" in
    0) echo "EXIT_SUCCESS" ;;
    *) echo "EXIT_FAILURE" ;;
  esac
}

failures=0

for base in "${cases[@]}"; do
  pa="${base%%/tests/*}"
  rel="${base#${pa}/}"
  test_src="${rel}.t"
  my_out="${rel}.my"
  my_stdout="${my_out}.stdout"
  my_status="${my_out}.exit_status"
  ref_out="${rel}.ref"
  ref_status="${ref_out}.exit_status"

  printf '=== %s ===\n' "${base}"
  (
    cd "${repo_root}/${pa}"
    set +e
    : > "${my_out}"
    "./${app}" "${app_args[@]}" -o "${my_out}" "${test_src}" &> "${my_stdout}"
    run_status=$?
    set -e
    status_name "${run_status}" > "${my_status}"

    case_ok=1
    if ! diff -u "${ref_out}" "${my_out}" > /tmp/cppgm-current-failure.diff 2>&1; then
      case_ok=0
      echo "output mismatch"
      sed -n '1,20p' /tmp/cppgm-current-failure.diff
    fi
    if [ -f "${ref_status}" ]; then
      if ! diff -u "${ref_status}" "${my_status}" > /tmp/cppgm-current-failure.status.diff 2>&1; then
        case_ok=0
        echo "status mismatch"
        sed -n '1,20p' /tmp/cppgm-current-failure.status.diff
      fi
    fi

    if [ "${case_ok}" -eq 1 ]; then
      echo "PASS"
      exit 0
    fi
    exit 1
  ) || failures=$((failures + 1))
  echo
done

if [ "${failures}" -ne 0 ]; then
  printf 'FAILURES: %d / %d\n' "${failures}" "${#cases[@]}"
  exit 1
fi

printf 'PASS: %d / %d\n' "${#cases[@]}" "${#cases[@]}"
