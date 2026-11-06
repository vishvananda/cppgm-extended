#!/usr/bin/env python3
"""Extract pinned Clang RUN metadata with LLVM's own lit parser.

This inventories commands; it neither evaluates features nor executes tests.
An existing output directory is never overwritten or treated as a resumable run.
"""
import argparse
from collections import Counter
import csv
import hashlib
import json
from pathlib import Path
import subprocess
import sys


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def write_json(path, value):
    path.write_text(json.dumps(value, indent=2) + '\n')


def selected_sources(path):
    if path is None:
        return None
    with path.open(newline='') as stream:
        return {row['source'] for row in csv.DictReader(stream)
                if row['source'].startswith('llvm-project/')}


def native_parser(root, revision):
    actual = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=root, text=True).strip()
    if actual != revision:
        raise ValueError('LLVM checkout does not match the declared revision')
    changes = subprocess.check_output(
        ['git', 'status', '--porcelain', '--untracked-files=no', '--', 'llvm/utils/lit'],
        cwd=root, text=True)
    if changes:
        raise ValueError('tracked native lit helpers have local changes')
    package = root / 'llvm/utils/lit'
    helpers = {str(path): digest(path) for path in package.rglob('*.py')}
    if not helpers:
        raise ValueError('native LLVM lit helpers are missing')
    sys.dont_write_bytecode = True
    sys.path.insert(0, str(package))
    import lit.TestRunner
    try:
        Path(lit.TestRunner.__file__).resolve().relative_to(package)
    except ValueError:
        raise ValueError('imported lit parser is outside the pinned checkout') from None
    return lit.TestRunner._parseKeywords, helpers


def extract_case(before, root, revision, selected, parse_keywords):
    relative = Path(before['path'])
    if relative.is_absolute() or '..' in relative.parts:
        raise ValueError('inventory path escapes LLVM checkout: ' + str(relative))
    if relative.as_posix() != before['path']:
        raise ValueError('inventory path is not canonical: ' + before['path'])
    source = root / relative
    try:
        source.resolve().relative_to(root)
    except ValueError:
        raise ValueError('inventory source resolves outside LLVM checkout') from None
    if before['revision'] != revision:
        raise ValueError('inventory revision differs from LLVM checkout')
    if not source.is_file() or digest(source) != before['sha256']:
        raise ValueError('source differs from pinned inventory: ' + str(relative))
    identity = 'llvm-project/' + relative.as_posix()
    row = dict(identity=identity, revision=revision, sha256=before['sha256'],
               selected=selected is None or identity in selected,
               applicability='unresolved', execution='not-executed')
    try:
        parsed = parse_keywords(str(source), require_script=False)
        # RUN, DEFINE and REDEFINE share one ordered list in native lit.
        # Preserve that ordering: a redefinition can change later commands.
        directives = [dict(vars(directive)) for directive in parsed['RUN:']]
        commands = [directive for directive in directives if directive['keyword'] == 'RUN:']
        row.update(status='metadata-extracted' if commands else 'no-run-driver-ownership-unresolved',
                   directives=directives, commands=commands,
                   metadata={key: value for key, value in parsed.items()
                             if key not in ['RUN:', 'DEFINE:', 'REDEFINE:']})
    except ValueError as error:
        row.update(status='metadata-parse-blocked', reason=str(error))
    if digest(source) != before['sha256']:
        raise ValueError('source changed during native parsing: ' + str(relative))
    return row


def extract_inventory(inventory, root, revision, selected, parse_keywords, destination):
    seen, counts = set(), Counter()
    with inventory.open() as source, destination.open('x') as output:
        for line in source:
            before = json.loads(line)
            if before['repository'] != 'llvm-project' or before['role'] != 'candidate':
                continue
            identity = 'llvm-project/' + before['path']
            if identity in seen:
                raise ValueError('duplicate inventory identity: ' + identity)
            row = extract_case(before, root, revision, selected, parse_keywords)
            seen.add(identity)
            counts['files'] += 1
            counts[row['status']] += 1
            counts['selected-files'] += int(row['selected'])
            counts['command-directives'] += len(row.get('commands', []))
            output.write(json.dumps(row, sort_keys=True) + '\n')
    unmatched = sorted((selected or set()) - seen)
    if unmatched:
        raise ValueError('selected identities absent from inventory: ' + ', '.join(unmatched))
    if not seen:
        raise ValueError('inventory contains no Clang candidate files')
    return dict(counts)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--llvm-root', type=Path, required=True)
    parser.add_argument('--revision', required=True)
    parser.add_argument('--inventory', type=Path, required=True)
    parser.add_argument('--selection', type=Path)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    root, output = args.llvm_root.resolve(), args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    state = dict(status='incomplete', scope='metadata extraction only; no test execution or applicability claim')
    write_json(output / 'state.json', state)
    try:
        parse_keywords, helpers = native_parser(root, args.revision)
        inputs = {str(args.inventory.resolve()): digest(args.inventory)}
        if args.selection is not None:
            inputs[str(args.selection.resolve())] = digest(args.selection)
        inputs[str(Path(__file__).resolve())] = digest(Path(__file__))
        write_json(output / 'manifest.json', dict(schema=1, llvm_root=str(root), revision=args.revision,
                   python_version=sys.version, python_executable=sys.executable,
                   inputs=inputs, native_helpers=helpers, selection='all' if args.selection is None
                   else str(args.selection.resolve()), scope=state['scope']))
        counts = extract_inventory(args.inventory, root, args.revision, selected_sources(args.selection),
                                   parse_keywords, output / 'inventory.jsonl')
        for path, expected in {**inputs, **helpers}.items():
            if digest(Path(path)) != expected:
                raise ValueError('input or native helper changed: ' + path)
        state.update(status='complete-with-parse-blockers' if counts.get('metadata-parse-blocked') else 'complete',
                     counts=counts, output_sha256=digest(output / 'inventory.jsonl'))
        write_json(output / 'state.json', state)
        print(json.dumps(state, indent=2))
        return int(bool(counts.get('metadata-parse-blocked')))
    except Exception as error:
        state['reason'] = str(error)
        write_json(output / 'state.json', state)
        raise


if __name__ == '__main__':
    sys.exit(main())
