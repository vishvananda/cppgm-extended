#!/usr/bin/env python3
"""Execute declared Clang C++11 language/object projections with native metadata.

Every original RUN remains accounted for. Native AST/IR/assembly, setup and
runtime checks without a reviewed projection remain explicit coverage gaps.
Negative observations require a source-pinned language adjudication before
they can count as required-diagnostic matches or mismatches.
"""
import argparse
from collections import Counter
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import sys
from types import SimpleNamespace

from upstream_clang_language import ProjectionBlocked, assess, elf_object, project_arguments
from upstream_clang_lit_metadata import native_parser
from upstream_dg_proxy import execute, sha256, write_json


def feature_value(expression, states, native_boolean):
    """Unknown native feature identifiers never silently become false/skipped."""
    for token in native_boolean.tokenize(expression):
        if native_boolean.isMatchExpression(token) and token != 'true' and token not in states:
            raise ProjectionBlocked('Unresolved native feature expression: ' + expression)
    return native_boolean.evaluate(expression, {name for name, value in states.items() if value})


def metadata_disposition(metadata, states, native_boolean):
    for expression in metadata.get('REQUIRES:') or []:
        if not feature_value(expression, states, native_boolean):
            return 'configured-unsupported', 'Required feature absent in declared configuration: ' + expression
    for expression in metadata.get('UNSUPPORTED:') or []:
        if feature_value(expression, states, native_boolean):
            return 'configured-unsupported', 'Native UNSUPPORTED expression is true: ' + expression
    for expression in metadata.get('XFAIL:') or []:
        if expression == '*' or feature_value(expression, states, native_boolean):
            return 'expected-failure-review', 'Active native XFAIL needs a reviewed language disposition: ' + expression
    return None


def compiler_arguments(command, native):
    match = re.fullmatch(native.TestRunner.kPdbgRegex, command)
    body = match.group(2) if match else command
    tree = native.ShUtil.ShParser(body, win32Escapes=False, pipefail=True).parse()
    if not isinstance(tree, native.ShCommands.Pipeline) or tree.negate or not tree.commands:
        raise ProjectionBlocked('Compound/pipeline RUN retains setup, output or runtime obligations')
    node = tree.commands[0]
    interfaces = []
    if not isinstance(node, native.ShCommands.Command):
        raise ProjectionBlocked('Native compiler command is not a literal shell command')
    if node.redirects:
        interfaces.append('native shell redirection/output obligation')
    for checker in tree.commands[1:]:
        if not isinstance(checker, native.ShCommands.Command) or not checker.args or checker.args[0] != 'FileCheck':
            raise ProjectionBlocked('Pipeline has an unreviewed setup, execution or checker command')
        interfaces.append('native FileCheck output obligation')
    if not all(isinstance(value, str) for value in node.args):
        raise ProjectionBlocked('Native glob expansion needs an explicit input-closure projection')
    return node.args, interfaces


def local_configurations(source, root):
    configs = []
    directory = source.parent
    while directory != root.parent:
        for name in ['lit.local.cfg.py', 'lit.local.cfg']:
            path = directory / name
            if path.exists():
                configs.append(path)
                break
        if directory == root:
            break
        directory = directory.parent
    return configs


def expand_case(row, source, temporary, configuration, parse_keywords, native):
    parsed = parse_keywords(str(source), require_script=False)
    directives = parsed['RUN:']
    if [dict(vars(value)) for value in directives] != row['directives']:
        raise ValueError('Native directives differ from pinned metadata: ' + row['identity'])
    for directive in directives:
        text = getattr(directive, 'command', getattr(directive, 'value', ''))
        for expression in re.findall(r'%if\s+(.*?)%\{', text):
            feature_value(expression.strip(), configuration['features'], native.BooleanExpression)
    test = SimpleNamespace(config=SimpleNamespace(substitutions=[
        ('%resource_dir', configuration['resource_include']),
        ('%target_triple', configuration['target'])]), getSourcePath=lambda: str(source))
    substitutions = native.TestRunner.getDefaultSubstitutions(
        test, str(temporary), str(temporary / 'native'), normalize_slashes=True)
    commands = native.TestRunner.applySubstitutions(
        directives, substitutions,
        conditions={name for name, value in configuration['features'].items() if value})
    if len(commands) != len(row['commands']):
        raise ValueError('Native expansion lost a RUN directive')
    return commands


def source_path(row, llvm, revision):
    identity = row['identity']
    if not identity.startswith('llvm-project/clang/test/') or row['revision'] != revision:
        raise ValueError('Source identity/revision differs from the declared Clang corpus')
    relative = Path(identity.removeprefix('llvm-project/'))
    if '..' in relative.parts or relative.as_posix() != identity.removeprefix('llvm-project/'):
        raise ValueError('Noncanonical source identity')
    source = llvm / relative
    if not source.resolve().is_relative_to(llvm) or sha256(source) != row['sha256']:
        raise ValueError('Source differs from pinned metadata: ' + identity)
    return source


def freeze_file(source, destination, inputs, origins, live_guard):
    source = Path(source).resolve()
    destination.parent.mkdir(parents=True, exist_ok=True)
    content = source.read_bytes()
    digest = hashlib.sha256(content).hexdigest()
    with destination.open('xb') as stream:
        stream.write(content)
    destination.chmod(0o444)
    inputs[str(destination)] = digest
    origins[str(destination)] = str(source)
    live_guard[str(source)] = digest
    return destination


def execute_cell(row, run_index, command, dialect, source, cell, configuration, binaries, adjudications, native):
    receipt = dict(identity=row['identity'], source_sha256=row['sha256'], run_index=run_index,
                   dialect=dialect, original_run=row['commands'][run_index], expanded_command=command,
                   full_upstream_pass=False, stages=[])
    try:
        arguments, interfaces = compiler_arguments(command, native)
        projection = project_arguments(arguments, source, dialect, configuration['target'],
                                       configuration['resource_include'])
        projection['unprojected_interfaces'].extend(interfaces)
    except (ProjectionBlocked, ValueError) as error:
        receipt.update(status='projection-blocked', reason=str(error))
        return receipt
    receipt['projection'] = projection
    cell.mkdir()
    shutil.copyfile(source, cell / 'source.snapshot')
    limits = dict(timeout_seconds=90, cpu_seconds=30, address_space_bytes=1024 ** 3,
                  file_bytes=64 * 1024 ** 2, compiler_error_scan=True)

    def run(name, arguments, output=None):
        result, _, _ = execute(arguments, cell / name, limits)
        if output is not None and result['classification'] == 'success':
            if not output.is_file() or not elf_object(output):
                result.update(classification='abnormal', artifact_failure='Missing or non-x86-64 ELF relocatable object')
            else:
                result['output'] = dict(path=str(output), sha256=sha256(output))
        receipt['stages'].append(dict(name=name, **result))
        if sha256(source) != row['sha256']:
            raise ValueError('Source changed during compilation')
        write_json(cell / 'receipt.json', receipt)
        return result

    previous = Path.cwd()
    os.chdir(cell)
    try:
        tokens = cell / 'candidate.tokens'
        pp = run('candidate-preprocess', [binaries['candidate'], *projection['candidate_args'],
                                        '-E', str(source), '-o', str(tokens)])
        if pp['classification'] in {'abnormal', 'timeout'}:
            receipt.update(status='compiler-abnormal', reason='Candidate preprocessing crashed, timed out or failed internally')
            return receipt
        if pp['classification'] != 'success' or not tokens.is_file():
            receipt.update(status='candidate-preprocessing-review', reason='Candidate preprocessing did not yield a complete comparable token stream')
            return receipt
        receipt['candidate_token_stream'] = dict(path=str(tokens), sha256=sha256(tokens), bytes=tokens.stat().st_size)
        context = dict(cc1_args=[*projection['native_args'], *projection['verification'],
                                '-fsyntax-only', str(source)], cppgm_stream=str(tokens),
                       allow_positive_without_verify=not projection['verification'])
        write_json(cell / 'context.json', context)
        receipt['native_context'] = dict(path=str(cell / 'context.json'), sha256=sha256(cell / 'context.json'))
        discover = run('native-expectations', [binaries['helper'], str(cell / 'context.json')])
        try:
            discovery = json.loads(Path(discover['stdout_path']).read_text())
        except (ValueError, OSError):
            discovery = dict(status='harness-blocked', reason='Native helper produced no complete metadata')
        receipt['discovery'] = discovery
        closure = {}
        for name in discovery.get('preprocessed_files', []):
            path = Path(name).resolve()
            closure[str(path)] = sha256(path)
        receipt['native_source_closure'] = closure
        if discover['classification'] != 'success' or discovery.get('status') != 'expectations-discovered':
            receipt.update(status='expectation-discovery-blocked', reason=discovery.get('reason', 'Native metadata helper failed'))
            return receipt
        if not discovery['same_preprocessed_tokens']:
            receipt.update(status='preprocessing-context-review', reason='Different source tokens/branches; no language score inferred')
            return receipt
        policy = set(discovery['language_policy_tokens'])
        if discovery['active_pragma'] or (not discovery['native_cxx_exceptions'] and policy & {'throw', 'try', 'catch'}):
            receipt.update(status='configuration-blocked', reason='Active pragma or disabled-exception policy needs proven equivalence')
            return receipt
        host_obj, candidate_obj = cell / 'host.o', cell / 'candidate.o'
        host = run('host-object', [binaries['clang'], '-cc1', *projection['native_args'],
                                 '-emit-obj', str(source), '-o', str(host_obj)], host_obj)
        candidate = run('candidate-object', [binaries['candidate'], *projection['candidate_args'],
                                           '-c', str(source), '-o', str(candidate_obj)], candidate_obj)
        key = row['identity'] + '#' + str(run_index) + '#' + dialect
        adjudication = adjudications.get(key)
        if adjudication and adjudication['source_sha256'] != row['sha256']:
            raise ValueError('Language adjudication differs from pinned source')
        receipt.update(assess(discovery, host, candidate, projection, adjudication))
        for path, digest in closure.items():
            if sha256(path) != digest:
                raise ValueError('Included source changed during language execution: ' + path)
        return receipt
    finally:
        os.chdir(previous)
        write_json(cell / 'receipt.json', receipt)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--llvm-root', type=Path, required=True)
    parser.add_argument('--revision', required=True)
    parser.add_argument('--metadata', type=Path, required=True)
    parser.add_argument('--configuration', type=Path, required=True)
    parser.add_argument('--adjudications', type=Path, required=True)
    parser.add_argument('--clang', type=Path, required=True)
    parser.add_argument('--candidate', type=Path, required=True)
    parser.add_argument('--helper', type=Path, required=True)
    parser.add_argument('--dialect', choices=['c++11', 'gnu++11'], action='append')
    parser.add_argument('--all', action='store_true', help='Schedule all metadata sources; default is the selected original cohort')
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    llvm, output = args.llvm_root.resolve(), args.output.resolve()
    parse_keywords, helpers = native_parser(llvm, args.revision)
    from lit import BooleanExpression, ShCommands, ShUtil, TestRunner
    native = SimpleNamespace(BooleanExpression=BooleanExpression.BooleanExpression,
                             ShCommands=ShCommands, ShUtil=ShUtil, TestRunner=TestRunner)
    output.mkdir(parents=True, exist_ok=False)
    inputs, origins, live_guard = {}, {}, {}
    metadata = freeze_file(args.metadata, output / 'inputs/metadata.jsonl', inputs, origins, live_guard)
    config_path = freeze_file(args.configuration, output / 'inputs/configuration.json', inputs, origins, live_guard)
    adjudication_path = freeze_file(args.adjudications, output / 'inputs/adjudications.json', inputs, origins, live_guard)
    for name in ['run_upstream_clang_language.py', 'upstream_clang_language.py', 'upstream_clang_expectations.cpp',
                 'upstream_clang_lit_metadata.py', 'upstream_dg_proxy.py']:
        freeze_file(Path(__file__).with_name(name), output / 'adapter' / name, inputs, origins, live_guard)
    archived_helpers = {}
    for original, digest in helpers.items():
        saved = output / 'native-lit' / Path(original).relative_to(llvm / 'llvm/utils/lit')
        freeze_file(original, saved, archived_helpers, origins, live_guard)
        if archived_helpers[str(saved)] != digest:
            raise ValueError('Native helper changed while freezing: ' + original)
    configuration = json.loads(config_path.read_text())
    adjudications = json.loads(adjudication_path.read_text())
    if configuration['scope'] != 'declared-cxx11-language-projection':
        raise ValueError('An explicit language-projection configuration is required')
    if configuration['target'] not in {'x86_64-pc-linux-gnu', 'x86_64-unknown-linux-gnu', 'x86_64-linux-gnu'}:
        raise ValueError('Unsupported target configuration')
    for path, digest in configuration['evidence'].items():
        if sha256(path) != digest:
            raise ValueError('Configuration evidence changed: ' + path)
        freeze_file(path, output / 'configuration-evidence' / (digest[:16] + '-' + Path(path).name),
                    inputs, origins, live_guard)
    if not isinstance(configuration['features'], dict) or any(type(value) is not bool for value in configuration['features'].values()):
        raise ValueError('Feature states must be explicit known booleans; omit unprobed features')
    binaries = {}
    for name in ['candidate', 'clang', 'helper']:
        original = getattr(args, name).resolve()
        path = output / name
        shutil.copyfile(original, path)
        path.chmod(0o555)
        binaries[name] = str(path)
    environment = {name: os.environ[name] for name in ['PATH', 'HOME', 'LD_LIBRARY_PATH'] if name in os.environ}
    environment.update(LANG='C', LC_ALL='C', TMPDIR=str(output / 'tmp'), CLANG_NO_DEFAULT_CONFIG='1')
    (output / 'tmp').mkdir()
    os.environ.clear()
    os.environ.update(environment)
    manifest = dict(schema=1, configuration=configuration, llvm_root=str(llvm), llvm_revision=args.revision,
                    inputs=inputs, native_helpers=archived_helpers, input_origins=origins,
                    binaries={path: sha256(path) for path in binaries.values()},
                    environment=environment, dialects=args.dialect or ['c++11', 'gnu++11'],
                    resource_limits=dict(wall_seconds=90, cpu_seconds=30, address_space_bytes=1024 ** 3,
                                         output_file_bytes=64 * 1024 ** 2),
                    scope='Projected language obligations; full native suite configuration/output/runtime coverage remains incomplete')
    if len(set(manifest['dialects'])) != len(manifest['dialects']):
        raise ValueError('Duplicate dialects cannot inflate variant accounting')
    write_json(output / 'manifest.json', manifest)
    rows, accounting, planned = [], [], []
    seen = set()
    with metadata.open() as stream:
        for line in stream:
            row = json.loads(line)
            if row['identity'] in seen:
                raise ValueError('Duplicate metadata identity')
            seen.add(row['identity'])
            scheduled = args.all or row['selected']
            source = source_path(row, llvm, args.revision)
            record = dict(identity=row['identity'], sha256=row['sha256'], selected=scheduled,
                          declared_runs=len(row.get('commands', [])))
            record['status'] = 'planned' if scheduled else 'not-scheduled-in-this-cohort'
            accounting.append(record)
            planned.append((row, source, record))
    write_json(output / 'accounting.json', accounting)
    write_json(output / 'accounting-initial.json', accounting)
    tasks = [dict(identity=row['identity'], source_sha256=row['sha256'], run_index=index,
                  dialect=dialect, original_run=command)
             for row, _, record in planned if record['selected']
             for index, command in enumerate(row.get('commands', []))
             for dialect in manifest['dialects']]
    write_json(output / 'tasks.json', tasks)
    manifest['plan_hashes'] = {name: sha256(output / name) for name in ['tasks.json', 'accounting-initial.json']}
    write_json(output / 'manifest.json', manifest)
    write_json(output / 'state.json', dict(state='running', scheduled_variants=len(tasks)))
    for row, source, record in planned:
        scheduled = record['selected']
        if not scheduled:
            continue
        local = local_configurations(source, llvm / 'clang/test')
        reason = None
        try:
            if local:
                raise ProjectionBlocked('Native local configurations need evaluation: ' + ', '.join(map(str, local)))
            disposition = metadata_disposition(row['metadata'], configuration['features'], native.BooleanExpression)
            if disposition:
                reason = disposition
        except ProjectionBlocked as error:
            reason = ('configuration-blocked', str(error))
        temporary = output / ('file-' + hashlib.sha256(row['identity'].encode()).hexdigest()[:16])
        temporary.mkdir()
        commands = []
        if reason is None:
            try:
                commands = expand_case(row, source, temporary, configuration, parse_keywords, native)
            except (ProjectionBlocked, ValueError) as error:
                reason = ('substitution-blocked', str(error))
        if not row.get('commands'):
            record.update(status='no-run-ownership-unresolved')
        else:
            record.update(status='variants-accounted')
        write_json(output / 'accounting.json', accounting)
        for run_index, original in enumerate(row.get('commands', [])):
            for dialect in manifest['dialects']:
                if reason:
                    result = dict(identity=row['identity'], source_sha256=row['sha256'], run_index=run_index,
                                  dialect=dialect, original_run=original, status=reason[0], reason=reason[1],
                                  full_upstream_pass=False)
                else:
                    cell = temporary / (str(run_index) + '-' + dialect)
                    result = execute_cell(row, run_index, commands[run_index], dialect, source, cell,
                                          configuration, binaries, adjudications, native)
                rows.append(result)
                write_json(output / 'results.json', rows)
                print(row['identity'], run_index, dialect, result['status'], flush=True)
    for path, digest in {**inputs, **archived_helpers, **manifest['binaries'], **live_guard}.items():
        if sha256(path) != digest:
            raise ValueError('Input/tool changed during campaign: ' + path)
    write_json(output / 'accounting.json', accounting)
    summary = dict(status='complete-accounting-with-coverage-limits', inventory_sources=len(accounting),
                   selected_sources=sum(row['selected'] for row in accounting), variants=len(rows),
                   outcomes=dict(Counter(row['status'] for row in rows)),
                   language_obligation_only=True, full_upstream_pass=False, full_corpus_correctness_percent=None)
    if len(rows) != len(tasks):
        raise ValueError('Scheduled variant accounting is incomplete')
    write_json(output / 'summary.json', summary)
    write_json(output / 'state.json', dict(state='complete', scheduled_variants=len(tasks), terminal_variants=len(rows)))
    print(json.dumps(summary, indent=2))


if __name__ == '__main__':
    main()
