#!/bin/bash

app=$1
if [[ "$app" == */* ]]; then
  runner="$app"
else
  runner="./$app"
fi

"$runner" -o "$2" "$3".* &> "$2.stdout"
 
