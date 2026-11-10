#!/usr/bin/env python3
"""Diagnostic: spell 16-byte moves as two bit-preserving f64 accesses.

Only the ray function is changed to localize mixed-width aggregate accesses.
Vector arithmetic stays vector arithmetic. This is not a vectorizer policy.
"""
from pathlib import Path
import re
import sys

source, output = map(Path, sys.argv[1:3])
text = source.read_text()
pattern = r'^function @TextRaytracer_run\([^\n]+\n.*?^}'
function = re.search(pattern, text, re.M | re.S)[0]
serial = 0

def load(m):
    global serial
    serial += 1
    prefix = '%issue115_v'+str(serial)
    assert prefix not in function
    indent, target, pointer = m.groups()
    lines = [prefix+'_p = index i8 '+pointer+', 8',
             prefix+'_a = load f64 '+pointer,
             prefix+'_b = load f64 '+prefix+'_p',
             prefix+'_x = vector fromf64 '+prefix+'_a',
             prefix+'_y = vector fromf64 '+prefix+'_b',
             target+' = vector unpcklpd '+prefix+'_x, '+prefix+'_y']
    return '\n'.join(indent+line for line in lines)

def store(m):
    global serial
    serial += 1
    prefix = '%issue115_v'+str(serial)
    assert prefix not in function
    indent, value, pointer = m.groups()
    lines = [prefix+'_p = index i8 '+pointer+', 8',
             prefix+'_a = vector tof64 '+value,
             prefix+'_y = vector pshufd '+value+', 238',
             prefix+'_b = vector tof64 '+prefix+'_y',
             'store f64 '+prefix+'_a, '+pointer,
             'store f64 '+prefix+'_b, '+prefix+'_p']
    return '\n'.join(indent+line for line in lines)

changed = re.sub(r'^( *)(%[\w.]+) = load v128 ([^\n]+)$', load, function, flags=re.M)
changed = re.sub(r'^( *)store v128 (%[\w.]+), ([^\n]+)$', store, changed, flags=re.M)
output.write_text(text.replace(function, changed))
print(serial, 'vector memory operations split', flush=True)
