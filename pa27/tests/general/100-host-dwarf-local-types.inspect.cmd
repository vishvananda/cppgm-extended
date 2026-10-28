set -eu
readelf --debug-dump=info "__OBJ1__" | grep -q DW_TAG_variable
readelf --debug-dump=info "__OBJ1__" | grep -q DW_AT_location
readelf --debug-dump=info "__OBJ1__" | grep -q DW_TAG_structure_type
readelf --debug-dump=info "__OBJ1__" | grep -q DW_TAG_enumeration_type
echo source_local_dwarf 1
