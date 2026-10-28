set -eu
readelf --debug-dump=info "__OBJ1__" | grep -q DW_TAG_variable
readelf --debug-dump=info "__OBJ1__" | grep -q DW_AT_location
echo source_local_dwarf 1
