#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

status=0

check_output_required_writes() {
  local hits
  hits="$(rg -n '(->|\\.)output_required\\s*=' dev/src || true)"
  [[ -z "$hits" ]] && return 0

  while IFS= read -r line; do
    [[ -z "$line" ]] && continue
    case "$line" in
      *"binding->output_required = true;"*) ;;
      *"binding->output_required = false;"*) ;;
      *"resolved->output_required = resolved->output_required || binding->output_required;"*) ;;
      *)
        echo "unexpected output_required write: $line" >&2
        status=1
        ;;
    esac
  done <<< "$hits"
}

check_output_requirement_bit_twiddles() {
  local hits
  hits="$(rg -n '\boutput_requirements\s*(\|=|&=|\^=)' dev/src || true)"
  [[ -z "$hits" ]] && return 0

  while IFS= read -r line; do
    [[ -z "$line" ]] && continue
    case "$line" in
      *"output_requirements |= binding->output_requirements;"*) ;;
      *)
        echo "unexpected output_requirements mutation: $line" >&2
        status=1
        ;;
    esac
  done <<< "$hits"
}

check_output_required_writes
check_output_requirement_bit_twiddles

exit "$status"
