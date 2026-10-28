set -eu
if readelf --sections "__OBJ1__" | grep -q "[.]debug_"; then
    echo unexpected_debug_sections
    exit 1
fi
echo source_line_dwarf_disabled 1
