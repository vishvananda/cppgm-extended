#!/usr/bin/env python3
"""Compare the object names of LowIR references with clang's symbols.

Maintainer tool; not part of the student export. The course names C++
entities as clang names them (clang is the oracle for every mangled name,
internal ones included). LowIR comparison relaxes `object=` spellings, so a
wrong symbol name never fails a fixture; this check compiles each LowIR
fixture's source with clang and compares the reference's object names with
the symbols in clang's object file.

For every C++-source fixture whose `.ref` is LowIR, each defined or declared
symbol's object name (its `object=` attribute, or its LowIR name when it has
none) is compared with clang's symbols for the same source:

- a name clang also has, or one that differs from a clang symbol only in the
  constructor or destructor entry (C1/C2/C3, CI1/CI2, D0/D1/D2: which entries
  a unit emits is an emission choice, not a name), matches;
- a name that demangles like a clang symbol of a different spelling (closure
  and unnamed-type numbers ignored) is a mismatch;
- so is one that names the same entity as a clang symbol with another
  signature: the same parameters, the same parameters up to the qualifiers
  of a type, or a clang signature that mangles an instantiation-dependent
  expression;
- a LowIR spelling that stands for a C++ entity clang names (a function-local
  static, its guard, an unnamed or local type, a lambda) is a mismatch;
- any other name (a course runtime helper, an entity clang does not emit in
  that unit) has no clang counterpart and is not judged.

A mismatch whose clang signature mangles an instantiation-dependent
expression (decltype, an expression template argument or array bound) is
reported as such: the course retains those expressions in its dependent
signatures. Known differences are listed in DOCUMENTED with their reason.

Usage: scripts/check_clang_names.py [--clang CXX] [--jobs N] [--verbose]
           [paN ...]
Exits 1 when a fixture has a mismatch that is not documented.
"""

import argparse
import collections
import concurrent.futures
import glob
import os
import re
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DEFAULT_PAS = ['pa%d' % n for n in range(10, 24)] + ['pa32']

SYMBOL_LINE = re.compile(
    r'^\s*(?:declare\s+)?(?:global|function)\s+@(?P<name>[^\s(:\[]+)')
ATTRIBUTE_LIST = re.compile(r'\[([^\[\]]*)\]')
OBJECT_ATTR = re.compile(r'(?:^|,\s*)object=(?P<object>[^\s,\]]+)')
BINDING_ATTR = re.compile(r'(?:^|,\s*)binding=(?P<binding>\w+)')
LOWIR_ENTITY_SPELLINGS = (
    '__local_static__', '__local_type', '__lambda_', '__anonymous_union')
# Course runtime helpers (static initialization and destruction wrappers):
# they stand for no C++ entity clang names.
COURSE_HELPER_PREFIXES = ('__cppgm_', '@__cppgm_')
# Placeholder types of the course's template analysis, which a signature
# mangles where it does not retain the written type.
COURSE_PLACEHOLDERS = ('__function_template_nondeduced_type_shape',
                       '__dependent_qualified_type_shape')
# Itanium expression forms inside a type: decltype, an expression template
# argument, an unresolved name, sizeof/alignof, a function parameter, a
# call, an entity reference or an expression array bound.
DEPENDENT_EXPRESSION = re.compile(
    r'D[Tt]|[IJ]X|sr\d|s[tzZ]|fp|fL\d|cl[^E]|L_Z|RA[^0-9_]')
RESIDUAL = 'instantiation-dependent signature'

# Documented differences: (fixture path relative to the repository root,
# reference name) -> reason. Keep this list short and explained.
DOCUMENTED = {
}


def reference_names(path):
    names = {}
    with open(path, errors='replace') as stream:
        for line in stream:
            match = SYMBOL_LINE.match(line)
            if not match:
                continue
            lists = ATTRIBUTE_LIST.findall(line.split(' = ')[0].split('{')[0])
            attrs = lists[-1] if lists else ''
            if re.search(r'(?:^|,\s*)role=', attrs):
                continue
            obj = OBJECT_ATTR.search(attrs)
            name = obj.group('object') if obj else match.group('name')
            binding = BINDING_ATTR.search(attrs)
            names[name] = binding.group('binding') if binding else '?'
    return names


def is_lowir(path):
    with open(path, errors='replace') as stream:
        for line in stream:
            if SYMBOL_LINE.match(line):
                return True
    return False


def is_cxx_source(path):
    with open(path, errors='replace') as stream:
        head = stream.read(4096)
    return not re.search(r'^\s*(?:declare\s+)?(?:function|global)\s+@', head,
                         re.M)


def clang_symbols(clang, source, scratch):
    obj = os.path.join(scratch, os.path.basename(source) + '.o')
    result = subprocess.run(
        [clang, '-std=c++11', '-O0', '-c', '-w', '-x', 'c++', source,
         '-o', obj], capture_output=True, timeout=120)
    if result.returncode != 0:
        return None
    listing = subprocess.run(['nm', obj], capture_output=True, text=True)
    os.unlink(obj)
    symbols = {}
    for line in listing.stdout.splitlines():
        parts = line.split()
        if len(parts) >= 2:
            symbols[parts[-1]] = parts[-2]
    return symbols


def scan_fixture(job):
    clang, base, scratch = job
    names = reference_names(base + '.ref')
    if not names:
        return None
    symbols = clang_symbols(clang, base + '.t', scratch)
    return base, names, symbols


# Constructor and destructor entries (C1/C2/C3, CI1/CI2 before the base
# class, D0/D1/D2) name one entity; which ones a unit emits is its choice.
VARIANT = re.compile(r'(CI|C|D)[0-5](?=E|I|[0-9A-Z])')


def variant_free(name):
    return VARIANT.sub(r'\g<1>9', name)


def demangle_all(names, *options):
    names = sorted(names)
    if not names:
        return {}
    result = subprocess.run(['c++filt'] + list(options),
                            input='\n'.join(names),
                            capture_output=True, text=True)
    return dict(zip(names, result.stdout.split('\n')))


def normalized(demangled, name):
    text = demangled.get(name, name)
    text = re.sub(r'\{lambda\([^{}]*\)#\d+\}', '<closure>', text)
    text = re.sub(r'\$_\d+', '<closure>', text)
    text = re.sub(r'\{unnamed type#\d+\}', '<closure>', text)
    text = re.sub(r'\b__local_type\d+\b', '<closure>', text)
    return text


def parameters(demangled, entities, name):
    """The demangled parameter list of a function name, or None."""
    full = demangled.get(name, name)
    entity = entities.get(name, name)
    at = full.find(entity)
    if at < 0 or not full[at + len(entity):].startswith('('):
        return None
    return full[at + len(entity):]


def same_entity_signature(demangled, entities, ours, theirs):
    """Whether two names of one entity differ in an encoded signature."""
    if any(placeholder in ours for placeholder in COURSE_PLACEHOLDERS) or \
            DEPENDENT_EXPRESSION.search(theirs):
        return True
    ours_parameters = parameters(demangled, entities, ours)
    theirs_parameters = parameters(demangled, entities, theirs)
    if ours_parameters is None or theirs_parameters is None:
        return False

    def unqualified(text):
        return re.sub(r'\b\w+::', '', text)

    return ours_parameters == theirs_parameters or \
        unqualified(ours_parameters) == unqualified(theirs_parameters)


def classify(ours, theirs, by_signature):
    if by_signature and (DEPENDENT_EXPRESSION.search(theirs) or any(
            placeholder in ours for placeholder in COURSE_PLACEHOLDERS)):
        return RESIDUAL
    if re.sub(r'L(?=\d)', '', ours.replace('_ZL', '_Z', 1)) == \
            re.sub(r'L(?=\d)', '', theirs.replace('_ZL', '_Z', 1)) or \
            ('_Z' not in ours[:2] and theirs.startswith('_ZL')):
        return 'internal-linkage marker'
    if '$_' in theirs or 'Ul' in theirs or 'Ut' in theirs:
        return 'closure or unnamed type'
    if theirs.startswith(('_ZZ', '_ZGVZ')):
        return 'local entity'
    return 'external name'


def compare(results):
    every = set()
    for _, names, symbols in results:
        if symbols is None:
            continue
        every.update(names)
        every.update(symbol for symbol in symbols if symbol.startswith('_Z'))
    demangled = demangle_all(every)
    # The entity alone (no parameters, no template result type).
    entities = demangle_all(every, '-p')

    failures = 0
    entries = []
    for base, names, symbols in results:
        if symbols is None:
            failures += 1
            continue
        relative = os.path.relpath(base, ROOT)
        variants = set(variant_free(symbol) for symbol in symbols)
        ours = set(variant_free(name) for name in names)
        by_name = collections.defaultdict(list)
        by_entity = collections.defaultdict(list)
        for symbol in symbols:
            if not symbol.startswith('_Z'):
                continue
            by_name[normalized(demangled, symbol)].append(symbol)
            if variant_free(symbol) not in ours:
                by_entity[normalized(entities, symbol)].append(symbol)
        for name, binding in sorted(names.items()):
            if name in symbols or variant_free(name) in variants:
                continue
            counterparts = [symbol for symbol in
                            by_name.get(normalized(demangled, name), [])
                            if symbol != name]
            by_signature = False
            if not counterparts and name.startswith('_Z'):
                counterparts = [
                    symbol for symbol in
                    by_entity.get(normalized(entities, name), [])
                    if same_entity_signature(demangled, entities, name,
                                             symbol)]
                by_signature = bool(counterparts)
            if counterparts:
                kind = classify(name, counterparts[0], by_signature)
                theirs = counterparts[0]
            elif any(spelling in name for spelling in LOWIR_ENTITY_SPELLINGS) \
                    and not name.startswith(COURSE_HELPER_PREFIXES):
                kind = 'LowIR spelling of a C++ entity'
                theirs = '-'
            else:
                continue
            entries.append((relative, name, theirs, kind, binding))
    return failures, entries


def main():
    parser = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    parser.add_argument('pas', nargs='*', default=DEFAULT_PAS)
    parser.add_argument('--clang', default=os.environ.get('CLANG', 'clang++'))
    parser.add_argument('--jobs', type=int, default=8)
    parser.add_argument('--verbose', action='store_true')
    args = parser.parse_args()

    bases = []
    for pa in args.pas:
        pattern = os.path.join(ROOT, pa, 'tests', '**', '*.t')
        for source in sorted(glob.glob(pattern, recursive=True)):
            base = source[:-2]
            if os.path.exists(base + '.ref') and is_lowir(base + '.ref') and \
                    is_cxx_source(source):
                bases.append(base)

    results = []
    with tempfile.TemporaryDirectory(prefix='clang-names-') as scratch:
        jobs = []
        for index, base in enumerate(bases):
            directory = os.path.join(scratch, str(index))
            os.mkdir(directory)
            jobs.append((args.clang, base, directory))
        with concurrent.futures.ThreadPoolExecutor(args.jobs) as pool:
            for result in pool.map(scan_fixture, jobs):
                if result:
                    results.append(result)

    failures, entries = compare(results)
    documented = [entry for entry in entries
                  if (entry[0] + '.t', entry[1]) in DOCUMENTED]
    mismatches = [entry for entry in entries if entry not in documented]
    kinds = collections.Counter(entry[3] for entry in mismatches)
    print('%d fixtures scanned, %d clang compile failures, '
          '%d mismatching names in %d fixtures, %d documented differences' %
          (len(results), failures, len(mismatches),
           len(set(entry[0] for entry in mismatches)), len(documented)))
    for kind, count in kinds.most_common():
        print('%6d %s' % (count, kind))
    limit = None if args.verbose else 3
    shown = collections.Counter()
    for relative, name, theirs, kind, binding in mismatches:
        shown[relative] += 1
        if limit is not None and shown[relative] > limit:
            continue
        print('%s.t: %s: ours %s [%s], clang %s' %
              (relative, kind, name, binding, theirs))
    for relative, name, theirs, kind, binding in documented:
        print('documented: %s.t: %s (clang %s): %s' %
              (relative, name, theirs, DOCUMENTED[(relative + '.t', name)]))
    return 1 if mismatches else 0


if __name__ == '__main__':
    sys.exit(main())
