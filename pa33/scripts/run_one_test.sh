#!/bin/bash

set -e

if [ -f "${2%.t}.env" ]; then
  set -a
  . "${2%.t}.env"
  set +a
fi

app_args=()
if [ -n "${CPPGM_APP_ARGS:-}" ]; then
  read -r -a app_args <<< "$CPPGM_APP_ARGS"
fi

./"$1" "${app_args[@]}" -E -o "$3" "$2"* &> "$3.stdout"
