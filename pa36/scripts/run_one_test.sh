#!/bin/bash

extra_args=()
if [ -n "${CPPGM_APP_ARGS:-}" ]; then
	read -r -a extra_args <<< "$CPPGM_APP_ARGS"
fi

: > "$3"
./"$1" "${extra_args[@]}" -o "$3" "$2" &> "$3.stdout"
