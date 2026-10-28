set -eu
readelf --debug-dump=info "__OBJ1__" | grep -q DW_TAG_compile_unit
readelf --debug-dump=info "__OBJ1__" | grep -q DW_TAG_subprogram
readelf --debug-dump=decodedline "__OBJ1__" | grep -q 100-host-dwarf-source-lines.t.1
echo source_line_dwarf 1
