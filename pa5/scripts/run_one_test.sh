#!/bin/bash

set -e

if [ -f "${2%.t}.env" ]; then
  set -a
  . "${2%.t}.env"
  set +a
fi

app=$1
if [[ "$app" == */* ]]; then
  runner="$app"
else
  runner="./$app"
fi

"$runner" -o "$3" "$2"* &> "$3.stdout"
 
