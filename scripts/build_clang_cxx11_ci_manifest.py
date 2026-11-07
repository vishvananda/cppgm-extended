#!/usr/bin/env python3
"""Freeze conservative C++11 language checks from pinned Clang RUNs.

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
from upstream_clang_input_closure import configurations, prepare


REVISION = '2078da43e25a4623cab2d0d60decddf709aaea28'
DIALECTS = {'c++11': 'c++11', 'c++0x': 'c++11',
            'gnu++11': 'gnu++11', 'gnu++0x': 'gnu++11'}
STANDARD = re.compile(r'-std=(?:gnu\+\+|c\+\+)(?:11|0x)\b')
TARGETS = {'x86_64-pc-linux-gnu', 'x86_64-unknown-linux-gnu', 'x86_64-linux-gnu'}
SUFFIXES = {'.cpp', '.cc', '.cxx', '.C'}
DIAGNOSTIC = re.compile(r'\b[\w-]+-(?:error|warning|note|remark)(?:-re)?\b')
SOURCE_PARTS = re.compile(r'//[^\n]*|/\*[\s\S]*?\*/|R"([^ ()\\\t\r\n]{0,16})\([\s\S]*?\)\1"'
                          r'|"(?:\\[\s\S]|[^"\\])*"|\'(?:\\[\s\S]|[^\'\\])*\'')
DIRECTIVE = re.compile(r'(?<![\w-])([A-Za-z][A-Za-z0-9_-]*)-'
                       r'(error|warning|note|remark|no-diagnostics)(-re)?(?![\w-])')
# Optional quality/deprecation warnings, not extension or narrowing diagnostics.
# Keep their original upstream ordering; cppgm need not implement warning parity.
OPTIONAL_WARNINGS = {'-Wno-' + name for name in [
    'tautological-compare', 'tautological-pointer-compare', 'unused', 'unused-value',
    'unused-lambda-capture', 'unused-but-set-variable', 'unused-local-typedefs',
    'unused-const-variable', 'unused-template', 'unused-comparison', 'uninitialized',
    'sometimes-uninitialized', 'inaccessible-base', 'dynamic-exception-spec',
    'defaulted-function-deleted', 'reorder', 'unevaluated-expression',
    'constant-logical-operand', 'bool-conversion', 'unreachable-code',
    'inconsistent-missing-override', 'sizeof-array-argument']}


class Excluded(ValueError):
    pass


def code_only(text):
    """Mask comments/literals before checking policy-sensitive source tokens."""
    return SOURCE_PARTS.sub(lambda m: ''.join('\n' if c == '\n' else ' ' for c in m[0]), text)


def source_policy(text, allow_diagnostics=False):
    # A backslash-newline can extend a comment or construct a directive/token.
    logical = re.sub(r'\\\r?\n', '', text.lstrip('\ufeff'))
    code = code_only(logical)
    if re.search(r'\?\?[=/\'()!<>-]', text):
        raise Excluded('Trigraph translation needs an explicit preprocessing-context review')
    if re.search(r'^\s*(?:#|%:)', code, re.MULTILINE):
        raise Excluded('Preprocessing directives need active-context/input-closure validation')
    if not allow_diagnostics and DIAGNOSTIC.search(text):
        raise Excluded('Diagnostic expectations need active language-rule adjudication')
    if re.search(r'\b__builtin_\w+\b', code):
        raise Excluded('Builtin function semantics need an explicit language/extension review')
    if re.search(r'\b__\w+\b', code):
        raise Excluded('Predefined/compiler tokens need an explicit context review')
    return set(re.findall(r'\b(?:throw|try|catch)\b', code))


def required_errors(text, verification, allow_positive=False):
    """Discover a supported active error, never compare compiler diagnostics.

    No source directives are allowed by source_policy. Only comments can own
    annotations; optional zero-minimum errors and inactive prefixes do not make
    a negative. Unsupported native verifier grammar remains an explicit gap.
    """
    if len(verification) != 1:
        raise Excluded('Negative discovery requires one explicit verifier prefix selection')
    flag = verification[0]
    prefixes = ['expected'] if flag == '-verify' else flag[len('-verify='):].split(',')
    if not prefixes or any(not re.fullmatch(r'[A-Za-z][A-Za-z0-9_-]*', p) for p in prefixes):
        raise Excluded('Unknown native verifier prefix syntax')
    logical = re.sub(r'\\\r?\n', '', text.lstrip('\ufeff'))
    code = code_only(logical)
    if any(v in code for v in ['"', "'", '/*']):
        raise Excluded('Unterminated literal/comment prevents safe expectation discovery')
    errors, no_diagnostics, other = 0, False, False
    for part in SOURCE_PARTS.finditer(logical):
        if not part[0].startswith(('//', '/*')):
            continue
        for directive in DIRECTIVE.finditer(part[0]):
            prefix, kind, regex = directive.groups()
            if prefix not in prefixes:
                continue
            if kind == 'no-diagnostics':
                no_diagnostics = True
                continue
            other = True
            if regex:
                raise Excluded('Native regex expectation grammar needs separate discovery')
            tail = part[0][directive.end():]
            location = re.match(r'@(?:[+-]?\d+|\*)', tail)
            if location:
                if location[0] == '@0':
                    raise Excluded('Invalid native absolute expectation line zero')
                tail = tail[location.end():]
            elif tail.startswith('@'):
                raise Excluded('Native file/marker expectation location needs separate discovery')
            count = re.match(r'\s*(\d+(?:\+|-\d+)?|\+)?\s*\{\{', tail)
            if not count or '}}' not in tail[count.end():]:
                raise Excluded('Unsupported or malformed native expectation count/body')
            bounds = count[1] or '1'
            minimum = 1 if bounds == '+' else int(re.match(r'\d+', bounds)[0])
            if '-' in bounds and int(bounds.split('-')[1]) < minimum:
                raise Excluded('Invalid native expectation count range')
            errors += int(kind == 'error' and minimum > 0)
    if no_diagnostics and other:
        raise Excluded('Conflicting active no-diagnostics and diagnostic expectations')
    if not errors and not allow_positive:
        raise Excluded('No active mandatory expected-error under selected verifier prefixes')
    return dict(prefixes=prefixes, active_mandatory_error=bool(errors))


def project_run(command, native, policy_tokens, implicit_cxx11=False, llvm_objects=False,
                input_context=None, optional_warnings=False):
    match = re.fullmatch(native.TestRunner.kPdbgRegex, command)
    body = match.group(2) if match else command
    tree = native.ShUtil.ShParser(body, win32Escapes=False, pipefail=True).parse()
    if (llvm_objects and isinstance(tree, native.ShCommands.Pipeline)
            and not tree.negate and len(tree.commands) == 2):
        if not all(isinstance(n, native.ShCommands.Command) and not n.redirects
                   and all(isinstance(v, str) for v in n.args) for n in tree.commands):
            raise Excluded('LLVM projection requires two literal commands without redirection')
        args, tokens, extra = (input_context(tree.commands[0].args) if input_context else
                               (tree.commands[0].args, policy_tokens, {}))
        return dict(project_llvm_arguments(args, tree.commands[1].args, tokens, implicit_cxx11,
                                          optional_warnings),
                    **extra)
    if (not isinstance(tree, native.ShCommands.Pipeline) or tree.negate or
            len(tree.commands) != 1):
        raise Excluded('Setup, pipeline, negation or runtime RUN needs a separate projection')
    node = tree.commands[0]
    if (not isinstance(node, native.ShCommands.Command) or node.redirects or
            not all(isinstance(v, str) for v in node.args)):
        raise Excluded('Redirection/glob/nonliteral command needs a separate projection')
    args, tokens, extra = input_context(node.args) if input_context else (node.args, policy_tokens, {})
    return dict(project_arguments(args, tokens, implicit_cxx11, optional_warnings), **extra)


def project_llvm_arguments(args, check, policy_tokens, implicit_cxx11=False, optional_warnings=False):
    if not args or args[0] != '%clang_cc1' or args.count('-emit-llvm') != 1:
        raise Excluded('Only an explicit cc1 emit-llvm to FileCheck pipeline is mapped')
    if '-fsyntax-only' in args or args.count('-o') != 1:
        raise Excluded('LLVM projection requires one unambiguous native action/output')
    if not check or check[0] != 'FileCheck' or check.count('%s') != 1:
        raise Excluded('LLVM projection requires FileCheck of the source itself')
    for option in check[1:]:
        if option != '%s' and not (re.fullmatch(
                r'--?check-prefix(?:es)?=[A-Za-z0-9_-]+(?:,[A-Za-z0-9_-]+)*', option)
                or option.startswith('--implicit-check-not=')):
            raise Excluded('Unmapped FileCheck input/policy: ' + option)
    projected, omitted = [], []
    i = 0
    while i < len(args):
        value = args[i]
        i += 1
        if value in {'-emit-llvm', '-disable-llvm-passes'}:
            omitted.append(value)
        elif value == '-o':
            if i == len(args) or args[i] != '-':
                raise Excluded('Only native LLVM stdout output is mapped')
            omitted.extend(['-o', '-'])
            i += 1
        else:
            projected.append(value)
    result = project_arguments([*projected, '-fsyntax-only'], policy_tokens, implicit_cxx11,
                              optional_warnings)
    if result['verification']:
        raise Excluded('Verifier metadata in an LLVM pipeline needs separate active-oracle review')
    result['projection_kind'] = 'llvm-object-acceptance'
    result['qualification'] = dict(
        kind='llvm-object-cxx11', additional_upstream_flags=['-pedantic-errors'],
        status='Separate strict native C++11 object qualification required, including explicit11 RUNs')
    result['adaptations'] = [v for v in result['adaptations'] if not v.startswith('Syntax-only acceptance')]
    result['adaptations'].append('Project LLVM emission to relocatable object acceptance only; '
                                 'original IR/FileCheck/setup/runtime obligations remain unmet')
    result['unmet_native_oracles'] = dict(status='not-executed', filecheck_arguments=check,
                                          original_action_output_options=omitted)
    return result


def project_arguments(args, policy_tokens, implicit_cxx11=False, optional_warnings=False):
    if not args or args[0] != '%clang_cc1':
        raise Excluded('Only an explicit native clang cc1 syntax action is mapped initially')
    standard = None
    syntax = False
    source_count = 0
    upstream, candidate, adaptations, verification = ['-cc1'], [], [], []
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
            verification.append(value)
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
        elif optional_warnings and value in OPTIONAL_WARNINGS:
            upstream.append(value)
            adaptations.append('Reviewed optional warning control retained only for upstream: ' + value)
        elif value == '-pedantic':
            upstream.append(value)
        else:
            raise Excluded('Unmapped semantic/input/action option: ' + value)
    qualification = None
    if standard is None and implicit_cxx11:
        standard = 'c++11'
        qualification = dict(kind='implicit-mode-cxx11', additional_upstream_flags=['-pedantic-errors'],
                             status='Native strict C++11 qualification required; failures are '
                                    'applicability/standard-specific triage, not cppgm defects')
        adaptations.append('RUN has no explicit standard; project into ISO C++11 with separate '
                           'strict native qualification, preserving active verifier prefixes')
    if standard is None or not syntax or source_count != 1:
        raise Excluded('Exactly one explicit C++11 source and syntax-only action are required')
    if policy_tokens and not exceptions:
        raise Excluded('Native cc1 disabled-exception policy is not equivalent for these tokens')
    adaptations.append('Syntax-only acceptance strengthened to relocatable object generation')
    result = dict(dialect=standard, upstream_flags=upstream, candidate_flags=candidate,
                  adaptations=list(dict.fromkeys(adaptations)), verification=verification)
    if qualification:
        result['qualification'] = qualification
    if optional_warnings and any(v in OPTIONAL_WARNINGS for v in args) and not qualification:
        result['qualification'] = dict(kind='local-input-cxx11',
            additional_upstream_flags=['-pedantic-errors'],
            status='Separate native11 qualification of optional warning-control projection')
    return result


def local_configs(source, root):
    directory = source.parent
    while directory == root or root in directory.parents:
        if any((directory / n).exists() for n in ['lit.local.cfg', 'lit.local.cfg.py']):
            return True
        if directory == root:
            break
        directory = directory.parent
    return False


def disposition_reason(name, digest, dispositions):
    reviewed = dispositions.get('llvm-project/' + name)
    if reviewed:
        if reviewed['sha256'] != digest:
            raise ValueError('Stale source-pinned language disposition: ' + name)
        if reviewed['status'] in {'documented-gnu-extension-positive', 'vendor-attribute-positive',
                                   'reviewed-iso-cxx11-positive', 'required-diagnostic'}:
            return None
        return reviewed['status'] + ': ' + reviewed['reason']
    return None


def collect(root, parse_keywords, native, dispositions=None, adjudications=None, verify_negatives=False,
            implicit_cxx11=False, llvm_objects=False, local_inputs=False):
    files = subprocess.check_output(['git', 'ls-files', '-z', '--', 'clang/test'], cwd=root)
    tracked = set(files.decode().split('\0'))
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
        digest = hashlib.sha256(data).hexdigest()
        text = data.decode('utf-8', errors='replace')
        if not STANDARD.search(text) and not implicit_cxx11:
            continue
        reason = disposition_reason(name, digest, dispositions or {})
        if reason:
            exclusions.append(dict(path=name, sha256=digest, reason=reason))
            continue
        try:
            if 'Inputs' in Path(name).parts:
                raise Excluded('Native Inputs support ownership is not a standalone language test')
            configs = configurations(path, root, Excluded) if local_inputs else []
            if not local_inputs and local_configs(path, root / 'clang/test'):
                raise Excluded('Native inherited local configuration is not mapped initially')
            try:
                policy_tokens = source_policy(text, allow_diagnostics=True)
            except Excluded:
                if not local_inputs:
                    raise
                policy_tokens = None
            parsed = parse_keywords(str(path), require_script=False)
            if any(parsed.get(key) for key in ['REQUIRES:', 'UNSUPPORTED:', 'XFAIL:']):
                raise Excluded('Native feature/expected-failure configuration needs evaluation')
            directives = parsed['RUN:']
            if any(v.keyword != 'RUN:' for v in directives):
                raise Excluded('File-owned substitutions need complete configuration expansion')
        except (Excluded, ValueError) as error:
            exclusions.append(dict(path=name, reason=str(error)))
            continue
        input_cache = {}
        for index, directive in enumerate(directives):
            try:
                needs_inputs = local_inputs and (policy_tokens is None or configs or
                                                 re.search(r'\s-I', directive.command))
                context = (lambda arguments: prepare(arguments, path, root, tracked, configs,
                           source_policy, SOURCE_PARTS, Excluded, input_cache)) if needs_inputs else None
                projection = dict(project_run(directive.command, native, policy_tokens,
                                              implicit_cxx11, llvm_objects, context, local_inputs))
                oracle_text = projection.pop('_oracle_text', text)
                if 'input_files' in projection:
                    projection['adaptations'].append('Freeze local input/config closure and identical '
                                                     'include search directories for both compilers')
                    if 'qualification' not in projection:
                        projection['qualification'] = dict(kind='local-input-cxx11',
                            additional_upstream_flags=['-pedantic-errors'],
                            status='Separate strict native11 input/config projection qualification')
                if projection.get('projection_kind') == 'llvm-object-acceptance':
                    if DIAGNOSTIC.search(oracle_text):
                        raise Excluded('Diagnostic annotations in LLVM sources need active-oracle review')
                    projection['unmet_native_oracles'] = dict(
                        projection['unmet_native_oracles'],
                        all_original_RUNs=[v.command for v in directives],
                        scope='Original LLVM/FileCheck and any setup/runtime RUNs are not executed; '
                              'only projected language object acceptance is graded')
                mode = (dispositions or {}).get('llvm-project/' + name, {})
                if mode.get('status') == 'documented-gnu-extension-positive':
                    if 'qualification' not in projection:
                        raise Excluded('Documented GNU projection requires a no-standard RUN')
                    projection['dialect'] = 'gnu++11'
                    projection['qualification'] = dict(
                        kind='documented-gnu-cxx11', additional_upstream_flags=[],
                        source_policy_issue=mode['issue'],
                        status='Source-pinned documented GNU extension; ordinary native GNU++11 '
                               'qualification, distinct from retained failed strict ISO observations')
                    projection['adaptations'] = [*projection['adaptations'], mode.get('coverage',
                        'GNU++11 acceptance covers documented anonymous struct/union semantics; '
                        'not an ISO11-positive claim')]
                elif mode.get('status') == 'vendor-attribute-positive':
                    projection['qualification'] = dict(
                        kind='vendor-attribute-cxx11', additional_upstream_flags=[],
                        source_policy_issue=mode['issue'],
                        status='Source-pinned balanced vendor attribute arguments; ordinary native11 '
                               'qualification, without requiring vendor processing parity')
                    projection['adaptations'] = [*projection['adaptations'],
                        'Vendor attribute arguments are balanced tokens; original annotation effects remain unmet']
                elif mode.get('status') == 'reviewed-iso-cxx11-positive':
                    projection['qualification'] = dict(kind='reviewed-iso-cxx11',
                        additional_upstream_flags=mode['qualification_flags'],
                        source_policy_issue=mode['issue'],
                        status='Source-pinned ISO11 validity review; suppress only optional comment warning '
                               'in strict qualification, retaining original normal warning flags')
                verification = projection.pop('verification', [])
                expected = 'accept'
                if mode.get('status') == 'required-diagnostic':
                    discovered = required_errors(oracle_text, verification, allow_positive=True)
                    expected = 'diagnostic'
                    projection['language_oracle'] = dict(discovered, source_disposition=mode,
                        required_language_rule=True)
                    projection.setdefault('qualification', dict(kind='local-input-cxx11',
                        additional_upstream_flags=['-pedantic-errors'],
                        status='Source-pinned mandatory language rule, independent of verifier severity'))
                    projection['adaptations'].append('Source-pinned primary rule requires any normal '
                        'diagnostic; native warning-only annotations do not establish valid acceptance')
                elif DIAGNOSTIC.search(oracle_text):
                    discovered = required_errors(oracle_text, verification, allow_positive=local_inputs)
                    if not discovered['active_mandatory_error']:
                        projection.pop('verification', None)
                        projection['language_oracle'] = dict(discovered,
                            scope='Active verifier has no required error; warning/note spelling is not graded')
                        if 'qualification' not in projection:
                            projection['qualification'] = dict(kind='local-input-cxx11',
                                additional_upstream_flags=['-pedantic-errors'],
                                status='Warning-only acceptance requires separate strict native11 qualification')
                    else:
                        projection['language_oracle'] = dict(discovered)
                    key = 'llvm-project/' + name + '#' + str(index) + '#' + projection['dialect']
                    adjudication = (adjudications or {}).get(key)
                    if discovered['active_mandatory_error'] and not adjudication and not verify_negatives:
                        raise Excluded('Active verify error awaits language-rule triage for CI inclusion')
                    if adjudication and adjudication['source_sha256'] != digest:
                        raise RuntimeError('Stale source-pinned negative adjudication: ' + name)
                    if adjudication and adjudication['expected'] != 'diagnose-invalid':
                        raise Excluded('Reviewed language rule does not require a diagnostic')
                    expected = 'diagnostic' if discovered['active_mandatory_error'] else 'accept'
                    if discovered['active_mandatory_error']:
                        projection['language_oracle'] = dict(adjudication=adjudication, **discovered)
                    if not adjudication and discovered['active_mandatory_error']:
                        projection['language_oracle']['standards_status'] = (
                            'Native active-error expectation; language/IFNDR applicability awaits failure-led triage')
                    projection['adaptations'] = [v for v in projection['adaptations']
                                                if 'positive language acceptance' not in v]
                    projection['adaptations'].append(
                        'Whole translation unit requires any normal diagnostic; no wording, count, location '
                        'or recovery matching' if discovered['active_mandatory_error'] else
                        'Warning-only active verifier projected to object acceptance; no warning parity')
            except (Excluded, ValueError) as error:
                exclusions.append(dict(path=name, reason='RUN ' + str(index) + ': ' + str(error)))
                continue
            key = json.dumps([name, projection], sort_keys=True)
            if key in grouped:
                grouped[key]['original'].append(directive.command)
                continue
            case = dict(id='clang-' + hashlib.sha256(key.encode()).hexdigest()[:20],
                        path=name, sha256=digest, action='compile',
                        expect=expected, original=[directive.command], **projection)
            grouped[key] = case
            cases.append(case)
    return dict(schema=1, suite='clang', revision=REVISION,
                coverage='Positive object acceptance and ' +
                         ('active-error required-diagnostic candidates' if verify_negatives else
                          'adjudicated required diagnostics') + ' from self-contained ' +
                         ('explicit C++11/0x and implicit-mode syntax RUNs requiring separate native qualification; '
                          if implicit_cxx11 else 'explicit C++11/0x syntax RUNs; ')
                         + 'both ISO and GNU spellings retain their declared dialect; no full Clang-suite claim' +
                         ('; LLVM pipelines add separately qualified object projections with unmet original oracles'
                          if llvm_objects else '') +
                         ('; audited local configs, frozen literal input closures and reviewed optional '
                          'warning controls add separately qualified language projections' if local_inputs else ''),
                limitations=['Selection is independent of cppgm acceptance; upstream qualification failures stay visible',
                             'Negative checks establish a whole-translation-unit diagnostic, not each annotation',
                             'No link, runtime, diagnostic wording, AST, LLVM or assembly parity claim',
                             ('Non-C++ suffixes remain outside this cohort' if implicit_cxx11 else
                              'Sources without literal C++11/0x spellings and non-C++ suffixes are outside this initial cohort'),
                             ('Arbitrary conditionals, generated/system headers, unreviewed configs, builtins, '
                              'features and unmapped RUNs remain explicit gaps' if local_inputs else
                              'Preprocessing, builtins, local configs, features, support inputs and unmapped RUNs remain gaps')] +
                            (['Implicit-mode projections require separate strict native C++11 qualification; '
                              'qualification failures remain applicability triage, without automatic oracle changes']
                             if implicit_cxx11 else []) +
                            (['LLVM pipelines establish projected object acceptance only; original '
                              'IR/FileCheck/setup/runtime obligations remain explicit unmet metadata']
                             if llvm_objects else []) +
                            (['Additional native active-error cases retain unreviewed language/IFNDR applicability; '
                              'all failures remain visible for triage, without candidate-based selection']
                             if verify_negatives else []),
                exclusions=exclusions, cases=cases)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--llvm-root', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--revision', default=REVISION, choices=[REVISION])
    parser.add_argument('--dispositions', type=Path,
                        default=Path(__file__).resolve().parents[1] / 'reports/cxx11-language-dispositions.json')
    parser.add_argument('--adjudications', type=Path,
                        default=Path(__file__).resolve().parents[1] / 'reports/cxx11-clang-language-adjudications.json')
    parser.add_argument('--verify-negatives', action='store_true',
                        help='Include supported active-error RUNs for failure-led language/IFNDR triage; '
                             'reviewed source-pinned dispositions still apply')
    parser.add_argument('--implicit-cxx11', action='store_true',
                        help='Also project supported no-standard RUNs into C++11; requires separate '
                             'strict native qualification, retaining all qualification failures')
    parser.add_argument('--llvm-object-projections', action='store_true',
                        help='Project audited simple cc1 emit-llvm to FileCheck RUNs into object '
                             'acceptance; qualify natively and retain all original oracle obligations')
    parser.add_argument('--local-inputs', action='store_true',
                        help='Resolve tracked quoted-header closures and audited simple local configs; '
                             'freeze dependencies, qualify natively and retain all other blockers')
    args = parser.parse_args(argv)
    root = args.llvm_root.resolve()
    parse_keywords, _ = native_parser(root, args.revision)
    tracked = ['clang/test', 'llvm/utils/lit']
    subprocess.run(['git', 'diff', '--quiet', 'HEAD', '--', *tracked], cwd=root, check=True)
    native = sys.modules['lit']
    from lit import ShCommands, ShUtil
    native.ShCommands, native.ShUtil = ShCommands, ShUtil
    adjudications = json.loads(args.adjudications.read_text())
    for row in adjudications.values():
        source = Path(__file__).resolve().parents[1] / row['standard_source']
        if hashlib.sha256(source.read_bytes()).hexdigest() != row['standard_sha256']:
            raise ValueError('Stale primary standard source: ' + str(source))
    manifest = collect(root, parse_keywords, native, json.loads(args.dispositions.read_text()),
                       adjudications, args.verify_negatives, args.implicit_cxx11, args.llvm_object_projections,
                       args.local_inputs)
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
