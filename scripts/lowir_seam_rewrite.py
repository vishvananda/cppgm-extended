#!/usr/bin/env python3
"""Equivalence-preserving rewrites of emitted LowIR text, one mode per call.

usage: lowir_seam_rewrite.py <mode> <in.lowir> <out.lowir>
       lowir_seam_rewrite.py --list

Each mode changes something a frontend might reasonably spell differently
from the course solution while meaning the same thing.  MODES classifies
every mode by what the course says about that difference
(../pa13/lowir.md, "What The Comparison Absorbs And What It Enforces"):

  presentation   names, order and layout the comparison has always ignored;
                 the rewritten output must still pass
  normalization  a spelling the comparison absorbs by rule; the rewritten
                 output must still pass
  convention     a choice the course fixes in words instead of absorbing;
                 the rewritten output must fail wherever the rewrite
                 changed something

`rule` is a phrase from the sentence in pa13/lowir.md that states the
normalization or convention.  check_lowir_seams.py verifies the phrase is
still there, so that everything the comparison rejects is a written
convention and everything it absorbs is a written normalization.
"""
import re
import sys

MODES = [
    # (mode, kind, rule phrase in pa13/lowir.md)
    ('R1-temps', 'presentation', 'Names of temporaries, slots, blocks and parameters'),
    ('R2-labels', 'presentation', 'Names of temporaries, slots, blocks and parameters'),
    ('R3-slots', 'presentation', 'Names of temporaries, slots, blocks and parameters'),
    ('R20-param-names', 'presentation', 'Names of temporaries, slots, blocks and parameters'),
    ('R4-slot-order', 'presentation', 'The order of slot declarations'),
    ('R9-function-order', 'presentation', 'The order of top-level entries'),
    ('R6-extra-declare', 'presentation', 'A declaration nothing uses'),
    ('R5-metadata-order', 'presentation', 'The order of items inside a metadata group'),
    ('R10-default-metadata', 'presentation', 'the metadata keys that belong to later stages'),
    ('R12-indent', 'presentation', 'Indentation and blank lines'),
    ('R13-blank-lines', 'presentation', 'Indentation and blank lines'),
    ('R11-hex-literal', 'normalization', 'A literal is its value, not its spelling'),
    ('R16-literal-spelling', 'normalization', 'A literal is its value, not its spelling'),
    ('R15-cmp-commute', 'normalization', 'The operand order of a commutative operation'),
    ('R17-binary-commute', 'normalization', 'The operand order of a commutative operation'),
    ('R14-branch-negate', 'convention', 'Branch sense follows the source'),
    ('R8-copy-elision', 'convention', 'A retype is a `copy`'),
    ('R7-pure-swap', 'convention', "Instructions follow the source's evaluation order"),
]

PURE = ('addr', 'const', 'index', 'binary', 'cmp', 'convert', 'copy')
INVERT = {'eq': 'ne', 'ne': 'eq', 'lt': 'ge', 'ge': 'lt', 'gt': 'le', 'le': 'gt',
          'ult': 'uge', 'uge': 'ult', 'ugt': 'ule', 'ule': 'ugt'}

# A literal operand: preceded by a separator, followed by one, so that the
# digits of `%t12`, `i32`, `^b3` and `16x8` are left alone.
LITERAL_BEFORE = r'(?<=[\s,(\[:])'
LITERAL_AFTER = r'(?=[\s,:)\]]|$)'
TYPED_LINE = re.compile(
    r'^\s*(?:%[A-Za-z0-9_]+\s*=\s*)?'
    r'(?:const|copy|phi|store(?:\s+volatile)?|return|unary\s+\w+|binary\s+\w+|cmp\s+\w+|atomic_\w+)'
    r'\s+([A-Za-z0-9_]+)\b')


def split_functions(text):
    """Yield ('text', chunk) and ('fn', chunk) pieces; a function chunk spans
    'function ...{' to its closing '}\\n'."""
    out = []
    pos = 0
    for m in re.finditer(r'^function @[^\n]*\{\n.*?^\}\n', text, re.S | re.M):
        out.append(('text', text[pos:m.start()]))
        out.append(('fn', m.group(0)))
        pos = m.end()
    out.append(('text', text[pos:]))
    return out


def map_functions(text, f):
    return ''.join(piece if kind == 'text' else f(piece)
                   for kind, piece in split_functions(text))


def rename_tokens(fn, pattern, prefix, skip=()):
    names = {}

    def sub(m):
        name = m.group(1)
        if name in skip:
            return m.group(0)
        if name not in names:
            names[name] = f'{prefix}{len(names)}'
        return m.group(0)[0] + names[name]
    return re.sub(pattern, sub, fn)


def instr_dest(line):
    m = re.match(r'^\s+%([A-Za-z0-9_]+)\s*=', line)
    return m.group(1) if m else None


def instr_kind(line):
    body = re.sub(r'^%[A-Za-z0-9_]+\s*=\s*', '', line.strip())
    return body.split(' ')[0] if body else ''


def typed_width(line):
    """(signed, width) of the operand type an instruction line names, or None."""
    m = TYPED_LINE.match(line)
    if not m:
        return None
    t = re.match(r'^([iu])(8|16|32|64|128)$', m.group(1))
    return (t.group(1) == 'i', int(t.group(2))) if t else None


def respell_literals(line):
    """Every literal operand of one line in another spelling of the same value."""
    shape = typed_width(line)

    def integer(m):
        sign, digits = m.group(1), m.group(2)
        value = int(digits)
        if sign == '-' and shape and shape[0] and value != 0:
            return str((1 << shape[1]) - value)      # -1 as an i32 is 4294967295
        return sign + '0x%X' % value
    line = re.sub(LITERAL_BEFORE + r'(-?)(\d+)' + LITERAL_AFTER, integer, line)

    def floating(m):
        sign, integer_part, fraction, suffix = m.groups()
        digits = (integer_part + fraction).lstrip('0') or '0'
        return f'{sign}{digits}e{-len(fraction)}{suffix}'
    line = re.sub(LITERAL_BEFORE + r'(-?)(\d+)\.(\d*)([fFlL]?)' + LITERAL_AFTER, floating, line)
    line = re.sub(LITERAL_BEFORE + r'nullptr' + LITERAL_AFTER, '0', line)
    return line


def rewrite(mode, text):
    if not text.strip():
        return text
    if mode == 'R1-temps':          # renumber %tN temporaries (control)
        return map_functions(text, lambda fn: rename_tokens(fn, r'%(t[0-9]+)\b', 't9'))
    if mode == 'R2-labels':         # rename block labels
        return map_functions(text, lambda fn: rename_tokens(fn, r'\^([A-Za-z0-9_]+)', 'L'))
    if mode == 'R3-slots':          # rename slots
        return map_functions(text, lambda fn: rename_tokens(fn, r'\$([A-Za-z0-9_]+)', 's'))
    if mode == 'R4-slot-order':     # reverse the slot declaration list
        def f(fn):
            lines = fn.split('\n')
            i = 1
            while i < len(lines) and lines[i].startswith('  slot '):
                i += 1
            return '\n'.join(lines[:1] + lines[1:i][::-1] + lines[i:])
        return map_functions(text, f)
    if mode == 'R5-metadata-order':     # reverse items inside every [ ... ] group
        return re.sub(r'\[([^\]]+)\]',
                      lambda m: '[' + ', '.join(reversed([x.strip() for x in m.group(1).split(',')])) + ']',
                      text)
    if mode == 'R6-extra-declare':      # an unused external declaration
        lines = text.split('\n')
        last = max((i for i, l in enumerate(lines) if l.startswith('declare function')), default=-1)
        lines.insert(last + 1, 'declare function @spike_unused_helper(%arg0 : i64) -> void [unwind=no]')
        return '\n'.join(lines)
    if mode == 'R7-pure-swap':      # swap the first adjacent independent pure pair in each block
        def f(fn):
            lines = fn.split('\n')
            i = 0
            in_block = False
            while i + 1 < len(lines):
                if lines[i].startswith('  block '):
                    in_block = True
                    i += 1
                    continue
                if in_block and lines[i].startswith('    ') and lines[i + 1].startswith('    '):
                    a, b = lines[i], lines[i + 1]
                    da, db = instr_dest(a), instr_dest(b)
                    ka, kb = instr_kind(a), instr_kind(b)
                    if ka in PURE and kb in PURE and da and db and ('%' + da) not in b:
                        lines[i], lines[i + 1] = b, a
                        in_block = False
                i += 1
            return '\n'.join(lines)
        return map_functions(text, f)
    if mode == 'R8-copy-elision':       # remove '%a = copy T %b' and forward its uses
        def f(fn):
            out = []
            ren = {}
            for l in fn.split('\n'):
                m = re.match(r'^    %([A-Za-z0-9_]+) = copy (\S+) %([A-Za-z0-9_]+)$', l)
                if m:
                    ren[m.group(1)] = ren.get(m.group(3), m.group(3))
                    continue
                if ren:
                    l = re.sub(r'%([A-Za-z0-9_]+)\b', lambda mm: '%' + ren.get(mm.group(1), mm.group(1)), l)
                out.append(l)
            return '\n'.join(out)
        return map_functions(text, f)
    if mode == 'R9-function-order':     # reverse the order of function definitions
        pieces = split_functions(text)
        fns = iter([p for k, p in pieces if k == 'fn'][::-1])
        return ''.join(next(fns) if k == 'fn' else p for k, p in pieces)
    if mode == 'R10-default-metadata':  # spell the default unwind mode in a second group
        return re.sub(r'^(function @(?![^\n]*\bunwind=)[^\n]*?)( \{)$', r'\1 [unwind=may]\2', text, flags=re.M)
    if mode == 'R11-hex-literal':       # spell stored integer zeros in hex
        return map_functions(text, lambda fn: re.sub(r'(store (?:i|u)(?:8|16|32|64) )0,', r'\g<1>0x0,', fn))
    if mode == 'R12-indent':            # six-space instruction indentation
        return re.sub(r'^    (\S)', r'      \1', text, flags=re.M)
    if mode == 'R13-blank-lines':       # no blank lines inside functions
        return map_functions(text, lambda fn: re.sub(r'\n\n+', '\n', fn))
    if mode == 'R14-branch-negate':     # invert the compare feeding a branch and swap its arms
        def f(fn):
            lines = fn.split('\n')
            for i in range(1, len(lines)):
                mb = re.match(r'^    branch %([A-Za-z0-9_]+), (\^[A-Za-z0-9_]+), (\^[A-Za-z0-9_]+)$', lines[i])
                mc = re.match(r'^    %([A-Za-z0-9_]+) = cmp (\w+) (.*)$', lines[i - 1])
                if mb and mc and mb.group(1) == mc.group(1) and mc.group(2) in INVERT:
                    lines[i - 1] = f'    %{mc.group(1)} = cmp {INVERT[mc.group(2)]} {mc.group(3)}'
                    lines[i] = f'    branch %{mb.group(1)}, {mb.group(3)}, {mb.group(2)}'
                    break
            return '\n'.join(lines)
        return map_functions(text, f)
    if mode == 'R15-cmp-commute':       # swap the operands of every equality compare
        return re.sub(r'(= cmp (?:eq|ne) \S+ )([^,\n]+), ([^\n]+)$', r'\1\3, \2', text, flags=re.M)
    if mode == 'R16-literal-spelling':  # every literal in another spelling of its value
        return '\n'.join(respell_literals(l) for l in text.split('\n'))
    if mode == 'R17-binary-commute':    # swap the operands of every commutative binary
        return re.sub(r'(= binary (?:add|mul|and|or|xor) \S+ )([^,\n]+), ([^\n]+)$', r'\1\3, \2', text, flags=re.M)
    if mode == 'R20-param-names':       # rename parameters
        def f(fn):
            header_end = fn.index('\n')
            params = re.findall(r'%([A-Za-z_][A-Za-z0-9_]*)\s*:', fn[:header_end])
            for i, p in enumerate(params):
                fn = re.sub(r'%' + re.escape(p) + r'\b', f'%q{i}', fn)
            return fn
        return map_functions(text, f)
    sys.exit('unknown mode ' + mode)


def main(argv):
    if argv[1:] == ['--list']:
        for mode, kind, rule in MODES:
            print(f'{mode}\t{kind}\t{rule}')
        return 0
    if len(argv) != 4:
        sys.exit(__doc__)
    mode, src, dst = argv[1:]
    with open(src) as f:
        text = f.read()
    with open(dst, 'w') as f:
        f.write(rewrite(mode, text))
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
