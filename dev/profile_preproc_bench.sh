#!/bin/bash

set -euo pipefail

repeat="${1:-2048}"
outfile="${2:-../obj/profile-data/preproc.out}"

inputs=(
  "../pa5/tests/100-empty.t"
  "../pa5/tests/100-nodefs.t"
  "../pa5/tests/150-no-error.t"
  "../pa5/tests/200-fnlike.t"
  "../pa5/tests/200-if.t"
  "../pa5/tests/500-tricky-join.t"
  "../pa5/tests/900-recurse.t"
)

args=()
i=0
while [ "$i" -lt "$repeat" ]; do
  args+=("${inputs[@]}")
  i=$((i + 1))
done

exec ./preproc-profile -o "$outfile" "${args[@]}"
