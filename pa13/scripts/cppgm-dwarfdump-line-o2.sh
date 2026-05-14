#!/bin/sh
set -e

if [ "$1" != "-o" ] || [ $# -lt 3 ]; then
  echo "usage: $0 -o <outfile> <input>..." >&2
  exit 1
fi

outfile=$2
shift 2

objfile="${outfile}.o"
dwarfdump_bin=
if command -v dwarfdump >/dev/null 2>&1; then
  dwarfdump_bin=dwarfdump
elif command -v llvm-dwarfdump >/dev/null 2>&1; then
  dwarfdump_bin=llvm-dwarfdump
else
  echo "missing dwarfdump or llvm-dwarfdump" >&2
  exit 1
fi

"${CPPGM_CPPGM_APP:-../dev/cppgm++}" -c -gline-tables-only -O2 -o "$objfile" "$@"
"$dwarfdump_bin" --debug-line --debug-info --debug-loc "$objfile" \
  | sed -E '1s#^.*:#object:#; s#0x[0-9a-fA-F]+#<hex>#g' \
  > "$outfile"
rm -f "$objfile"
