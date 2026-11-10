#!/usr/bin/env python3
"""Counterfactual IR inputs for localization; does not modify the compiler or canonical input."""
from pathlib import Path
import json
import re
import sys

root = Path(sys.argv[1])
source = (root/'index-c.lowir').read_text()
types = {}
lines = []
replaced = []
function = None
for line in source.splitlines():
    if line.startswith('function '):
        function = line.split('(', 1)[0].split()[1]
        types = dict(re.findall(r'(%[\w.]+)\s*:\s*(obj<\d+x\d+>)', line))
    definition = re.match(r'\s*(%[\w.]+) = (?:load|copy|phi|call) (obj<\d+x\d+>)\b', line)
    # '>' is not a word character; inspect a literal following space instead.
    if definition is None:
        definition = re.match(r'\s*(%[\w.]+) = (?:load|copy|phi|call) (obj<\d+x\d+>) ', line)
    if definition:
        types[definition[1]] = definition[2]
    copy = re.match(r'(\s*)copyobj (\d+x\d+) (%[\w.]+), (.+)$', line)
    if copy and types.get(copy[3]) == 'obj<'+copy[2]+'>':
        line = copy[1]+'store obj<'+copy[2]+'> '+copy[3]+', '+copy[4]
        replaced.append(dict(function=function, value=copy[3], span=copy[2]))
    lines.append(line)
(root/'ir/copy-to-store.lowir').write_text('\n'.join(lines)+'\n')
(root/'copy-to-store-rewrites.json').write_text(json.dumps(replaced, indent=2)+'\n')
print('Equivalent object-value copy/store rewrites:', len(replaced))
name='@static_fn_index_c_b64_encode'
pattern=re.compile(r'^(function '+re.escape(name)+r'\([^\n]+\[)([^\n]+)(\] \{)$', re.M)
forced,count=pattern.subn(lambda m:m[1]+m[2]+', force_inline=yes'+m[3],source)
assert count == 1
(root/'ir/b64-force-inline.lowir').write_text(forced)
