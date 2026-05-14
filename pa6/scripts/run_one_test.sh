#!/bin/bash

app=$1
if [[ "$app" == */* ]]; then
  runner="$app"
else
  runner="./$app"
fi

"$runner" -o "$3" "$2" &> "$3.stdout"
 
