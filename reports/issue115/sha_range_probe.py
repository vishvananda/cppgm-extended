#!/usr/bin/env python3
"""Two narrowly scoped counterfactuals for a proven nonnegative loop IV.

This is localization, not a proposed optimizer pattern. The loop starts at
zero, increments by one, and continues while next < a signed i64 bound.
No entered iteration or its increment can overflow. A general implementation
must derive those facts from dominance, the exit test and the induction.
"""
from pathlib import Path
import re
import sys

source, output = map(Path, sys.argv[1:3])
mode = sys.argv[3]
text = source.read_text()
pattern = r'^function @BufferHashSHA256_run\([^\n]+\n.*?^}'
function = re.search(pattern, text, re.M | re.S)[0]
assert '%t55 = phi i64 [^body_4601_1: %t58_73, ^alloca: 0]' in function
assert '%t74 = cmp lt i64 %t58_73, %t21' in function
if mode == 'nowrap':
    old, new = '%t58_73 = binary add i64 %t55, 1', '%t58_73 = binary add nowrap i64 %t55, 1'
elif mode == 'mask':
    old, new = '%t19_56 = binary mod i64 %t55, 8', '%t19_56 = binary and i64 %t55, 7'
else:
    raise ValueError(mode)
assert function.count(old) == 1
output.write_text(text.replace(function, function.replace(old, new)))
