#!/usr/bin/env python3
"""Freeze a conservative positive C++11 language cohort from pinned Clang RUNs.

Selection never invokes either compiler. Native lit parses RUN structure, but
this first cohort deliberately needs no generated site configuration or helper.
Unmapped tests remain exclusions, not successes or a full-suite conformance rate.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys

from upstream_clang_lit_metadata import native_parser


REVISION = '2078da43e25a4623cab2d0d60decddf709aaea28'
DIALECTS = {'c++11': 'c++11', 'c++0x': 'c++11',
            'gnu++11': 'gnu++11', 'gnu++0x': 'gnu++11'}
STANDARD = re.compile(r'-std=(?:gnu\+\+|c\+\+)(?:11|0x)\b')
TARGETS = {'x86_64-pc-linux-gnu', 'x86_64-unknown-linux-gnu', 'x86_64-linux-gnu'}
SUFFIXES = {'.cpp', '.cc', '.cxx', '.C'}
DIAGNOSTIC = re.compile(r'\b[\w-]+-(?:error|warning|note|remark)(?:-re)?\b')


class Excluded(ValueError):
    pass


def code_only(text):
    """Mask comments/literals before checking policy-sensitive source tokens."""
    pattern = re.compile(r'//[^\n]*|/\*[\s\S]*?\*/|R"([^ ()\\\t\r\n]{0,16})\([\s\S]*?\)\1"'
                         r'|"(?:\\[\s\S]|[^"\\])*"|\'(?:\\[\s\S]|[^\'\\])*\'')
    return pattern.sub(lambda m: ''.join('\n' if c == '\n' else ' ' for c in m[0]), text)


def source_policy(text):
    # A backslash-newline can extend a comment or construct a directive/token.
    logical = re.sub(r'\\\r?\n', '', text.lstrip('\ufeff'))
    code = code_only(logical)
    if re.search(r'\?\?[=/\'()!<>-]', text):
        raise Excluded('Trigraph translation needs an explicit preprocessing-context review')
    if re.search(r'^\s*(?:#|%:)', code, re.MULTILINE):
        raise Excluded('Preprocessing directives need active-context/input-closure validation')
    if DIAGNOSTIC.search(text):
        raise Excluded('Diagnostic expectations need active language-rule adjudication')
    if re.search(r'\b__builtin_\w+\b', code):
        raise Excluded('Builtin function semantics need an explicit language/extension review')
    if re.search(r'\b__\w+\b', code):
        raise Excluded('Predefined/compiler tokens need an explicit context review')
    return set(re.findall(r'\b(?:throw|try|catch)\b', code))


def project_run(command, native, policy_tokens):
    match = re.fullmatch(native.TestRunner.kPdbgRegex, command)
    body = match.group(2) if match else command
    tree = native.ShUtil.ShParser(body, win32Escapes=False, pipefail=True).parse()
    if (not isinstance(tree, native.ShCommands.Pipeline) or tree.negate or
            len(tree.commands) != 1):
        raise Excluded('Setup, pipeline, negation or runtime RUN needs a separate projection')
    node = tree.commands[0]
    if (not isinstance(node, native.ShCommands.Command) or node.redirects or
            not all(isinstance(v, str) for v in node.args)):
        raise Excluded('Redirection/glob/nonliteral command needs a separate projection')
    return project_arguments(node.args, policy_tokens)


def project_arguments(args, policy_tokens):
    if not args or args[0] != '%clang_cc1':
        raise Excluded('Only an explicit native clang cc1 syntax action is mapped initially')
    standard = None
    syntax = False
    source_count = 0
    upstream, candidate, adaptations = ['-cc1'], [], []
    exceptions = False
    i = 1
    while i < len(args):
        value = args[i]
        i += 1
        if value.startswith('-std='):
            if standard is not None or value[5:] not in DIALECTS:
                raise Excluded('Conflicting or non-C++11 language standard')
            standard = DIALECTS[value[5:]]
        elif value == '%s':
            source_count += 1
        elif value == '-fsyntax-only':
            syntax = True
        elif value == '-verify' or value.startswith('-verify='):
            adaptations.append('Native verifier annotations omitted; positive language acceptance only')
        elif value in {'-fexceptions', '-fcxx-exceptions'}:
            upstream.append(value)
            exceptions |= value == '-fcxx-exceptions'
        elif value in {'-O0', '-O1', '-O2', '-O3'}:
            upstream.append(value)
            candidate.append(value)
        elif value in {'-triple', '-target'}:
            if i == len(args) or args[i] not in TARGETS:
                raise Excluded('Unmapped target configuration')
            upstream.extend(['-triple', args[i]])
            candidate.append('--target=' + args[i])
            i += 1
        elif value.startswith('-W') and not (value.startswith(('-Werror', '-Wno-')) or 'narrowing' in value):
            upstream.append(value)
            adaptations.append('Optional warning policy retained only for upstream')
        elif value == '-pedantic':
            upstream.append(value)
        else:
            raise Excluded('Unmapped semantic/input/action option: ' + value)
    if standard is None or not syntax or source_count != 1:
        raise Excluded('Exactly one explicit C++11 source and syntax-only action are required')
    if policy_tokens and not exceptions:
        raise Excluded('Native cc1 disabled-exception policy is not equivalent for these tokens')
    adaptations.append('Syntax-only acceptance strengthened to relocatable object generation')
    return dict(dialect=standard, upstream_flags=upstream, candidate_flags=candidate,
                adaptations=list(dict.fromkeys(adaptations)))


def local_configs(source, root):
    directory = source.parent
    while directory == root or root in directory.parents:
        if any((directory / n).exists() for n in ['lit.local.cfg', 'lit.local.cfg.py']):
            return True
        if directory == root:
            break
        directory = directory.parent
    return False


def collect(root, parse_keywords, native):
    files = subprocess.check_output(['git', 'ls-files', '-z', '--', 'clang/test'], cwd=root)
    cases, exclusions, grouped = [], [], {}
    for name in sorted(files.decode().split('\0')):
        if not name:
            continue
        path = root / name
        if not path.is_file():
            exclusions.append(dict(path=name, reason='Tracked input not materialized; applicability unresolved'))
            continue
        if path.suffix not in SUFFIXES:
            continue
        if not path.resolve().is_relative_to(root):
            exclusions.append(dict(path=name, reason='Tracked source resolves outside pinned corpus'))
            continue
        data = path.read_bytes()
        text = data.decode('utf-8', errors='replace')
        if not STANDARD.search(text):
            continue
        try:
            if 'Inputs' in Path(name).parts:
                raise Excluded('Native Inputs support ownership is not a standalone language test')
            if local_configs(path, root / 'clang/test'):
                raise Excluded('Native inherited local configuration is not mapped initially')
            policy_tokens = source_policy(text)
            parsed = parse_keywords(str(path), require_script=False)
            if any(parsed.get(key) for key in ['REQUIRES:', 'UNSUPPORTED:', 'XFAIL:']):
                raise Excluded('Native feature/expected-failure configuration needs evaluation')
            directives = parsed['RUN:']
            if any(v.keyword != 'RUN:' for v in directives):
                raise Excluded('File-owned substitutions need complete configuration expansion')
        except (Excluded, ValueError) as error:
            exclusions.append(dict(path=name, reason=str(error)))
            continue
        for index, directive in enumerate(directives):
            try:
                projection = project_run(directive.command, native, policy_tokens)
            except (Excluded, ValueError) as error:
                exclusions.append(dict(path=name, reason='RUN ' + str(index) + ': ' + str(error)))
                continue
            key = json.dumps([name, projection], sort_keys=True)
            if key in grouped:
                grouped[key]['original'].append(directive.command)
                continue
            case = dict(id='clang-' + hashlib.sha256(key.encode()).hexdigest()[:20],
                        path=name, sha256=hashlib.sha256(data).hexdigest(), action='compile',
                        expect='accept', original=[directive.command], **projection)
            grouped[key] = case
            cases.append(case)
    return dict(schema=1, suite='clang', revision=REVISION,
                coverage='Positive object acceptance from self-contained explicit C++11/0x syntax RUNs; '
                         'both ISO and GNU spellings retain their declared dialect; no full Clang-suite claim',
                limitations=['Selection is independent of cppgm acceptance; upstream qualification failures stay visible',
                             'No negative, link, runtime, diagnostic wording, AST, LLVM or assembly parity claim',
                             'Sources without literal C++11/0x spellings and non-C++ suffixes are outside this initial cohort',
                             'Preprocessing, builtins, local configs, features, support inputs and unmapped RUNs remain gaps'],
                exclusions=exclusions, cases=cases)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--llvm-root', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--revision', default=REVISION, choices=[REVISION])
    args = parser.parse_args(argv)
    root = args.llvm_root.resolve()
    parse_keywords, _ = native_parser(root, args.revision)
    tracked = ['clang/test', 'llvm/utils/lit']
    subprocess.run(['git', 'diff', '--quiet', 'HEAD', '--', *tracked], cwd=root, check=True)
    native = sys.modules['lit']
    from lit import ShCommands, ShUtil
    native.ShCommands, native.ShUtil = ShCommands, ShUtil
    manifest = collect(root, parse_keywords, native)
    subprocess.run(['git', 'diff', '--quiet', 'HEAD', '--', *tracked], cwd=root, check=True)
    if not manifest['cases']:
        raise ValueError('The conservative selection produced no cases')
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open('x') as stream:
        json.dump(manifest, stream, indent=2)
        stream.write('\n')
    print(json.dumps(dict(cases=len(manifest['cases']), sources=len({v['path'] for v in manifest['cases']}),
                          exclusions=len(manifest['exclusions']), output=str(args.output))))


if __name__ == '__main__':
    main()
