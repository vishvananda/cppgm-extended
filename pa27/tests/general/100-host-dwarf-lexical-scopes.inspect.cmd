set -eu
info=$(readelf --debug-dump=info "__OBJ1__")
[ "$(printf '%s' "$info" | grep -c DW_TAG_lexical_block)" -ge 6 ]
printf '%s' "$info" | grep -q DW_AT_ranges
readelf --debug-dump=Ranges "__OBJ1__" | grep -q 'End of list'
echo lexical_scope_dwarf 1
