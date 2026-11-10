#!/usr/bin/env python3
"""Localize LLVM's dynamic 32-bit fast path for unsigned 64-bit division.

The check and fallback preserve full-range semantics. This diagnostic changes
only the two profiled functions; a real implementation belongs in instruction
selection and requires a target cost model.
"""
from pathlib import Path
import re
import sys

source, output = map(Path, sys.argv[1:3])
targets = {'@static_fn_index_c_bwt_transform', '@static_fn_index_c_arith_decode_run'}
count = 0

def rewrite(match):
    global count
    text = match[0]
    if text.split('(', 1)[0].split()[1] not in targets:
        return text
    output_lines = []
    predecessor = None
    replacements = {}
    serial = 0
    for line in text.splitlines():
        block = re.fullmatch(r'  block (\^[\w.]+):', line)
        if block:
            predecessor = block[1]
        div = re.fullmatch(r'    (%[\w.]+) = binary (udiv|umod) i64 ([^,]+), (%[\w.]+)', line)
        if not div:
            output_lines.append(line)
            continue
        serial += 1
        count += 1
        prefix = 'issue115_div_'+str(serial)
        assert prefix not in text
        result, operation, left, right = div.groups()
        fast, slow, join = ('^'+prefix+'_'+part for part in ('fast','slow','join'))
        temp = '%'+prefix
        output_lines += [
            f'    {temp}_bits = binary or i64 {left}, {right}',
            f'    {temp}_high = binary ushr i64 {temp}_bits, 32',
            f'    {temp}_fits = cmp eq i64 {temp}_high, 0',
            f'    branch {temp}_fits, {fast}, {slow}',
            '', f'  block {fast}:',
            f'    {temp}_left = convert trunc u32 i64 {left}',
            f'    {temp}_right = convert trunc u32 i64 {right}',
            f'    {temp}_narrow = binary {operation} u32 {temp}_left, {temp}_right',
            f'    {temp}_wide = convert zext i64 u32 {temp}_narrow',
            f'    jump {join}',
            '', f'  block {slow}:',
            f'    {temp}_fallback = binary {operation} i64 {left}, {right}',
            f'    jump {join}',
            '', f'  block {join}:',
            f'    {result} = phi i64 [{fast}: {temp}_wide, {slow}: {temp}_fallback]',
        ]
        replacements[predecessor] = join
        predecessor = join
    def last(block):
        while block in replacements:
            block = replacements[block]
        return block
    return re.sub(r'(\^[\w.]+)(?=: )', lambda m:last(m[1]), '\n'.join(output_lines))

output.write_text(re.sub(r'^function [^\n]+\{\n.*?^}', rewrite, source.read_text(), flags=re.M|re.S))
print(count, 'unsigned divisions given a checked 32-bit fast path', flush=True)
