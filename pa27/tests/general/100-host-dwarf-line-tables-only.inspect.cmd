set -eu
readelf --debug-dump=info "__OBJ1__" | grep -q DW_TAG_compile_unit
if readelf --debug-dump=info "__OBJ1__" | grep -Eq "DW_TAG_(variable|base_type)"; then exit 1; fi
echo line_tables_only 1
