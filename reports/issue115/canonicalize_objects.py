#!/usr/bin/env python3
"""Diagnostic equivalences for object SSA values in a frozen LowIR input."""
import json
from pathlib import Path
import re
import sys

source, output = map(Path, sys.argv[1:3])
changes = []

def rewrite(match):
    text = match[0]
    aliases = dict(re.findall(r'^\s*(%[\w.]+) = copy obj<\d+x\d+> (%[\w.]+)\s*$', text, re.M))
    def original(value):
        seen = set()
        while value in aliases:
            assert value not in seen
            seen.add(value)
            value = aliases[value]
        return value
    lines = []
    for line in text.splitlines():
        alias = re.match(r'\s*(%[\w.]+) = copy obj<\d+x\d+> (%[\w.]+)\s*$', line)
        if alias:
            changes.append(dict(kind='object-value alias', value=alias[1]))
            continue
        lines.append(re.sub(r'%[\w.]+', lambda m:original(m[0]), line))
    text = '\n'.join(lines)
    # A memory-backed object copied into a value is a snapshot, just like load.
    if '--slot-loads' in sys.argv:
        def load(m):
            changes.append(dict(kind='object-slot snapshot', value=m[2]))
            return m[1]+'load '+m[2]+' '+m[3]
        text = re.sub(r'^(\s*%[\w.]+ = )copy (obj<\d+x\d+>) (\$[\w.]+)$', load, text, flags=re.M)
    types = dict(re.findall(r'(%[\w.]+)\s*:\s*(obj<\d+x\d+>)', lines[0]))
    for name, kind in re.findall(r'^\s*(%[\w.]+) = (?:load|copy|phi|call) (obj<\d+x\d+>) ', text, re.M):
        types[name] = kind
    def store(m):
        if types.get(m[3]) != 'obj<'+m[2]+'>':
            return m[0]
        changes.append(dict(kind='object-value store', value=m[3], span=m[2]))
        return m[1]+'store obj<'+m[2]+'> '+m[3]+', '+m[4]
    return re.sub(r'^(\s*)copyobj (\d+x\d+) (%[\w.]+), (.+)$', store, text, flags=re.M)

text = re.sub(r'^function [^\n]+\{\n.*?^}', rewrite, source.read_text(), flags=re.M|re.S)
output.write_text(text)
output.with_suffix('.rewrites.json').write_text(json.dumps(changes, indent=2)+'\n')
print(len(changes), 'equivalent object-value rewrites', flush=True)
