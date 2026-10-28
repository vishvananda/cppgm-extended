set -eu
readelf --debug-dump=decodedline "__OBJ1__" | python3 -c '
import sys
lines=set()
for row in sys.stdin:
    words=row.split()
    if len(words)>2 and words[0].endswith("100-host-dwarf-closing-braces.t.1") and words[1].isdigit():
        lines.add(int(words[1]))
assert {11,19} <= lines, lines
print("closing_brace_source_lines 2")
'
