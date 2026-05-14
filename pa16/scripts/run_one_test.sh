#!/bin/bash

app_args=()
if [ -n "${CPPGM_APP_ARGS:-}" ]; then
  read -r -a app_args <<< "$CPPGM_APP_ARGS"
fi

: > "$3"
./"$1" "${app_args[@]}" -o "$3" "$2" &> "$3.stdout"
