#!/usr/bin/env python3
"""Freeze self-contained cpp0x language checks using native dg-get-options.

This deliberately small adapter has no target-predicate emulation. Unsupported
metadata is an explicit exclusion, chosen without running either compiler.
"""
import argparse
import base64
from collections import defaultdict
import json
from pathlib import Path
import re
import shlex
import subprocess

from run_upstream_dg import parse_metadata
from upstream_dg_proxy import sha256, write_json
from build_clang_cxx11_ci_manifest import code_only


REVISION = '5115c7e447fc07457443df874bf57840e8316d5f'
EXTRACT = r'''
proc verbose {args} {}
source [file join [lindex $argv 0] utils.exp]
source [file join [lindex $argv 0] dg.exp]
while {[gets stdin encoded] >= 0} {
    set path [encoding convertfrom utf-8 [binary decode base64 $encoded]]
    puts "FILE\t$encoded"
    foreach directive [dg-get-options $path] {
        set words {}
        foreach word $directive {
            lappend words [binary encode base64 -maxlen 0 [encoding convertto utf-8 $word]]
        }
        puts [join $words \t]
    }
}
'''


class Excluded(ValueError):
    pass


def condition(value):
    words = value.strip().split()
    if len(words) != 2 or words[0] != 'target':
        raise Excluded('Unreviewed target selector: ' + value)
    if words[1] in {'c++11', 'c++11_only'}:
        return True
    if words[1] in {'c++14', 'c++17', 'c++20', 'c++23', 'c++26', 'c++98_only'}:
        return False
    raise Excluded('Unreviewed target selector: ' + value)


def project(directives, text):
    logical = re.sub(r'\\\r?\n', '', text).lstrip('\ufeff')
    code = code_only(logical)
    if re.search(r'\?\?[=/\'()!<>-]', text):
        raise Excluded('Trigraph context needs a preprocessing projection')
    if re.search(r'^\s*(?:#|%:)\s*(?:include|include_next|if|ifdef|ifndef|elif|pragma)\b', code, re.M):
        raise Excluded('Header, conditional preprocessing or pragma context needs a closure projection')
    if re.search(r'\b(?:__GNUC__|__clang__|__GNUG__|__has\w*|__SIZEOF_\w*|__cpp_\w*)\b', code):
        raise Excluded('Compiler-specific predefined macro context needs review')
    if re.search(r'\b(?:__complex__|__real__|__imag__|__attribute__|__typeof__|typeof|__float128)\b', code):
        raise Excluded('Nonstandard GNU language syntax needs a separate extension disposition')
    action, negative, flags = 'compile', False, []
    ambiguous_diagnostics = False
    adaptations = ['Native compile-to-assembly action uses object generation; diagnostic spelling/count ignored',
                   'Native -pedantic-errors becomes upstream -pedantic: preserve diagnostics without severity parity',
                   'Candidate uses its normal language diagnostics; optional upstream presentation/warning controls omitted']
    for directive in directives:
        command, args = directive['command'], directive['arguments']
        if command == 'dg-do':
            if len(args) > 2 or (len(args) == 2 and not condition(args[1])):
                raise Excluded('Action is outside declared C++11 configuration')
            if args[0] not in {'compile', 'run'}:
                raise Excluded('Unprojected native action: ' + args[0])
            action = args[0]
        elif command == 'dg-require-effective-target':
            if args != ['c++11']:
                raise Excluded('Unreviewed effective-target requirement: ' + str(args))
        elif command in {'dg-options', 'dg-additional-options'}:
            if len(args) != 1:
                raise Excluded('Conditional compiler options need native configuration')
            for flag in shlex.split(args[0]):
                if flag in {'-O0', '-O1', '-O2', '-O3'}:
                    flags.append(flag)
                elif flag in {'-std=c++11', '-std=c++0x'}:
                    pass
                elif flag in {'-w', '-Wall', '-Wextra', '-pedantic'} or (
                        flag.startswith('-W') and not flag.startswith(('-Werror', '-Wno-'))):
                    adaptations.append('Optional diagnostic option omitted from both compilers: ' + flag)
                else:
                    raise Excluded('Unreviewed semantic/diagnostic option: ' + flag)
        elif command == 'dg-error':
            if len(args) > 4:
                raise Excluded('Unreviewed error expectation arguments')
            if len(args) < 3 or condition(args[2]):
                negative = True
        elif command in {'dg-message', 'dg-prune-output'}:
            ambiguous_diagnostics = True
            adaptations.append('Diagnostic context oracle omitted: ' + command)
        elif command in {'dg-warning', 'dg-bogus'}:
            adaptations.append('Optional diagnostic oracle omitted: ' + command)
        else:
            raise Excluded('Unprojected native obligation: ' + command)
    if action == 'run' and negative:
        raise Excluded('Mixed negative/runtime obligations need rule adjudication')
    if ambiguous_diagnostics and not negative:
        raise Excluded('Message/pruned-error-only metadata does not establish positive language validity')
    if not directives:
        raise Excluded('No explicit native metadata')
    return action, 'diagnostic' if negative else 'accept', flags, sorted(set(adaptations))


def extract(paths, library):
    # Native Tcl parsing preserves quoted/braced dg arguments and line numbers.
    import tempfile
    with tempfile.TemporaryDirectory() as directory:
        script = Path(directory) / 'extract.tcl'
        script.write_text(EXTRACT)
        inputs = ''.join(base64.b64encode(str(path).encode()).decode() + '\n' for path in paths)
        raw = subprocess.run(['tclsh', str(script), str(library)], input=inputs,
                             capture_output=True, text=True, check=True).stdout
    result = defaultdict(list)
    current = None
    for line in raw.splitlines():
        if line.startswith('FILE\t'):
            current = base64.b64decode(line.split('\t')[1]).decode()
        elif line:
            result[current].extend(parse_metadata(line))
    return result


def build(root, library, dispositions):
    names = subprocess.check_output(['git', '-C', str(root), 'ls-files', '-z', '--',
                                     'gcc/testsuite/g++.dg/cpp0x'], text=True).split('\0')
    paths = sorted(root / name for name in names if name and Path(name).suffix == '.C' and
                   Path(name).parent.as_posix() == 'gcc/testsuite/g++.dg/cpp0x')
    if not paths:
        raise ValueError('No pinned GCC cpp0x sources found')
    metadata = extract(paths, library)
    cases, exclusions = [], []
    for source in paths:
        path = source.relative_to(root).as_posix()
        digest = sha256(source)
        try:
            disposition = dispositions.get('gcc/' + path)
            if disposition:
                if disposition['sha256'] != digest:
                    raise ValueError('Stale language disposition: ' + path)
                raise Excluded(disposition['status'] + ': ' + disposition['reason'])
            action, expect, flags, adaptations = project(metadata[str(source)], source.read_text())
        except Excluded as error:
            exclusions.append(dict(path=path, reason=str(error)))
            continue
        for dialect in ['c++11', 'gnu++11']:
            cases.append(dict(id=source.stem + '-' + dialect, path=path, sha256=digest,
                              dialect=dialect, action=action, expect=expect,
                              candidate_flags=flags, upstream_flags=['-pedantic', *flags],
                              original=metadata[str(source)], adaptations=adaptations))
    return dict(schema=1, suite='gcc', revision=REVISION, cases=cases, exclusions=exclusions,
                coverage='Self-contained GCC g++.dg/cpp0x language translations and zero-exit runtime checks; '
                         'native diagnostic text, counts and recovery are not correctness obligations.',
                limitations=['Not the complete GCC suite; other families and unsupported cpp0x metadata excluded',
                             'No headers, conditional preprocessing, specialized target or additional-source support yet',
                             'Required-diagnostic outcomes follow active dg-error declarations; mismatches need language-rule triage',
                             'Native pedantic escalation becomes diagnostic-only upstream qualification; baseline failures remain failures'],
                metadata_parser={name: sha256(library / name) for name in ['dg.exp', 'utils.exp']})


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--gcc-root', type=Path, required=True)
    parser.add_argument('--dejagnu-library', type=Path, required=True)
    parser.add_argument('--dispositions', type=Path,
                        default=Path(__file__).resolve().parents[1] / 'reports/cxx11-language-dispositions.json')
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    root = args.gcc_root.resolve()
    revision = subprocess.check_output(['git', '-C', str(root), 'rev-parse', 'HEAD'], text=True).strip()
    if revision != REVISION:
        raise ValueError('GCC sources must match pinned revision')
    subprocess.run(['git', '-C', str(root), 'diff', '--quiet', 'HEAD', '--',
                    'gcc/testsuite/g++.dg/cpp0x'], check=True)
    manifest = build(root, args.dejagnu_library.resolve(), json.loads(args.dispositions.read_text()))
    subprocess.run(['git', '-C', str(root), 'diff', '--quiet', 'HEAD', '--',
                    'gcc/testsuite/g++.dg/cpp0x'], check=True)
    write_json(args.output, manifest)
    print(f"Selected {len(manifest['cases'])} configurations; {len(manifest['exclusions'])} explicit exclusions")


if __name__ == '__main__':
    main()
