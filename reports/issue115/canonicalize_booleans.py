#!/usr/bin/env python3
"""Normalize byte loads of nonescaping i1 slots, retaining result width."""
import json
from pathlib import Path
import re
import sys

source, output = map(Path, sys.argv[1:3])
changes = []

def rewrite(match):
    text = match[0]
    slots = re.findall(r'^  slot (\$[\w.]+) : i1$', text, re.M)
    safe = set()
    for slot in slots:
        uses = [line.strip() for line in text.splitlines() if slot in re.findall(r'\$[\w.]+', line) and not line.strip().startswith('slot ')]
        if all(re.fullmatch(r'%[\w.]+ = load i8 '+re.escape(slot), line) or
               re.fullmatch(r'store i1 [^,]+, '+re.escape(slot), line) for line in uses):
            safe.add(slot)
    serial = 0
    def load(m):
        nonlocal serial
        if m[3] not in safe:
            return m[0]
        serial += 1
        temp = '%issue115_bool_'+str(serial)
        assert temp not in text
        changes.append(dict(function=text.splitlines()[0], slot=m[3], value=m[2]))
        return m[1]+temp+' = load i1 '+m[3]+'\n'+m[1]+m[2]+' = convert zext i8 i1 '+temp
    return re.sub(r'^(\s*)(%[\w.]+) = load i8 (\$[\w.]+)$', load, text, flags=re.M)

output.write_text(re.sub(r'^function [^\n]+\{\n.*?^}', rewrite, source.read_text(), flags=re.M|re.S))
output.with_suffix('.rewrites.json').write_text(json.dumps(changes, indent=2)+'\n')
print(len(changes), 'nonescaping boolean-slot loads normalized', flush=True)
