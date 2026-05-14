#!/bin/bash

app=$1
if [[ "$app" == */* ]]; then
  runner="$app"
else
  runner="./$app"
fi

"$runner" < "$2" &> "$3"
 
