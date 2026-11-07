#!/usr/bin/env python3
"""Run one frozen language manifest twice and report whole-suite elapsed time.

Downloads, compiler builds and manifest generation precede this command. Each
compiler gets fresh outputs, the same worker count and one complete cold-output
run. OS caches are left alone; cppgm runs first. This is a CI timing observation,
not a statistical performance gate or a claim of full upstream harness parity.
"""
import argparse
from collections import Counter
from concurrent.futures import ProcessPoolExecutor
import json
import os
from pathlib import Path
import platform
import re
import resource
import shutil
import subprocess
import time

from upstream_clang_language import elf_object
from upstream_dg_proxy import execute, sha256, write_json


LIMITS = dict(timeout_seconds=30, cpu_seconds=30, address_space_bytes=2 * 1024 ** 3,
              file_bytes=64 * 1024 ** 2, compiler_error_scan=True)


def validate_manifest(manifest, root):
    if manifest['schema'] != 1 or manifest['suite'] not in {'gcc', 'clang'}:
        raise ValueError('Unknown suite/schema')
    if not manifest['cases']:
        raise ValueError('An empty manifest cannot pass CI')
    seen = set()
    for case in manifest['cases']:
        if not re.fullmatch(r'[A-Za-z0-9_+.-]+', case['id']) or case['id'] in {'.', '..'}:
            raise ValueError('Unsafe case output identity')
        if case['id'] in seen:
            raise ValueError('Duplicate case identity: ' + case['id'])
        seen.add(case['id'])
        path = Path(case['path'])
        if path.is_absolute() or '..' in path.parts or not (root / path).resolve().is_relative_to(root):
            raise ValueError('Source outside corpus')
        if sha256(root / path) != case['sha256']:
            raise ValueError('Source hash mismatch: ' + str(path))
        for dependency in case.get('input_files', []):
            path = Path(dependency['path'])
            if (path.is_absolute() or '..' in path.parts or
                    not (root / path).resolve().is_relative_to(root)):
                raise ValueError('Input dependency outside corpus')
            if sha256(root / path) != dependency['sha256']:
                raise ValueError('Input dependency hash mismatch: ' + str(path))
        for spelling in case.get('include_directories', []):
            path = Path(spelling)
            if (path.is_absolute() or '..' in path.parts or
                    not (root / path).resolve().is_relative_to(root) or not (root / path).is_dir()):
                raise ValueError('Include directory missing or outside corpus')
        inputs = {dependency['path'] for dependency in case.get('input_files', [])}
        for edge in case.get('include_edges', []):
            if edge['source'] not in inputs or edge['resolved'] not in inputs:
                raise ValueError('Include edge lacks frozen input identity')
            if edge.get('kind', 'quoted') not in {'quoted', 'angled'}:
                raise ValueError('Unknown include search kind')
            searched = ([(root / edge['source']).parent / edge['spelling']]
                        if edge.get('kind', 'quoted') == 'quoted' else []) + [
                            root / directory / edge['spelling']
                            for directory in case.get('include_directories', [])]
            selected = next((p.resolve() for p in searched if p.is_file()), None)
            if selected != (root / edge['resolved']).resolve():
                raise ValueError('Include resolution changed: ' + edge['spelling'])
        if case['dialect'] not in {'c++11', 'gnu++11'} or case['action'] not in {'compile', 'run'}:
            raise ValueError('Unsupported dialect/action')
        if case['expect'] not in {'accept', 'diagnostic'}:
            raise ValueError('Unknown language expectation')
        if case['action'] == 'run' and case['expect'] != 'accept':
            raise ValueError('A required diagnostic has no defined runtime oracle')
        for key in ['candidate_flags', 'upstream_flags']:
            if not isinstance(case[key], list) or not all(isinstance(x, str) for x in case[key]):
                raise ValueError('Flags must be a literal argument list')
            if any(x in {'-o', '-c', '-S', '-E', '-emit-obj', '-fsyntax-only'} or
                   x.startswith('-std=') for x in case[key]):
                raise ValueError('Action/dialect/output must be owned by the runner')


def run_case(arguments):
    case, root, compiler, linker, role, output = arguments
    directory = Path(output) / case['id']
    previous_directory = Path.cwd()
    previous_temporary = os.environ.get('TMPDIR')
    result = dict(id=case['id'], path=case['path'], expect=case['expect'],
                  action=case['action'], dialect=case['dialect'], stages=[])

    def stage(name, command, compiler_error_scan=True):
        before = resource.getrusage(resource.RUSAGE_CHILDREN)
        receipt, _, stderr = execute(command, directory / name,
                                     dict(LIMITS, compiler_error_scan=compiler_error_scan))
        after = resource.getrusage(resource.RUSAGE_CHILDREN)
        receipt.update(child_user_seconds=after.ru_utime - before.ru_utime,
                       child_system_seconds=after.ru_stime - before.ru_stime)
        result['stages'].append(dict(name=name, **receipt))
        return receipt, stderr

    def finish(status):
        result['status'] = status
        try:
            write_json(directory / 'result.json', result)
        except OSError as error:
            result['receipt_error'] = str(error)
            result['status'] = 'harness-error'
        return result

    try:
        directory.mkdir()
        os.chdir(directory)
        os.environ['TMPDIR'] = str(directory)
        source = str(Path(root) / case['path'])
        obj = directory / 'test.o'
        flags = case['candidate_flags' if role == 'cppgm' else 'upstream_flags']
        include_flags = ['-I' + str(Path(root) / path) for path in case.get('include_directories', [])]
        cc1 = flags[:1] == ['-cc1']
        command = [compiler, *(['-cc1'] if cc1 else []), '-std=' + case['dialect'],
                   *(flags[1:] if cc1 else flags), *include_flags, '-emit-obj' if cc1 else '-c',
                   source, '-o', str(obj)]
        compile_result, diagnostic = stage('compile', command)
        classification = compile_result['classification']
        if classification not in {'success', 'rejection'}:
            return finish('compiler-abnormal')
        if case['expect'] == 'diagnostic':
            return finish('passed' if diagnostic.strip() else 'diagnostic-missing')
        if classification != 'success':
            return finish('compile-failed')
        if not obj.is_file() or not elf_object(obj):
            return finish('invalid-object')
        if case['action'] == 'run':
            executable = directory / 'test.exe'
            linked, _ = stage('link', [linker, str(obj), '-o', str(executable)])
            if linked['classification'] != 'success' or not executable.is_file():
                return finish('link-failed')
            runtime, _ = stage('execute', [str(executable)], compiler_error_scan=False)
            if runtime['classification'] != 'success':
                return finish('runtime-failed')
        return finish('passed')
    except Exception as error:
        result['error'] = str(error)
        return finish('harness-error')
    finally:
        os.chdir(previous_directory)
        if previous_temporary is None:
            os.environ.pop('TMPDIR', None)
        else:
            os.environ['TMPDIR'] = previous_temporary


def run_suite(cases, root, compiler, linker, role, output, workers):
    before = dict(load_average=os.getloadavg(), unix_seconds=time.time())
    start = time.monotonic()
    tasks = [(case, str(root), compiler, linker, role, str(output)) for case in cases]
    results, error = [], None
    try:
        output.mkdir()
        with ProcessPoolExecutor(max_workers=workers) as pool:
            for result in pool.map(run_case, tasks):
                results.append(result)
    except Exception as failure:
        # A broken worker pool must still leave a receipt and allow the other
        # compiler's complete run. Never call unreturned records passes.
        error = str(failure)
    elapsed = time.monotonic() - start
    counts = dict(Counter(row['status'] for row in results))
    executed = sum(any(stage['name'] == 'compile' for stage in row['stages']) for row in results)
    return dict(elapsed_seconds=elapsed, selected=len(cases), executed=executed, error=error,
                passed=counts.get('passed', 0), counts=counts, results=results,
                child_user_seconds=sum(stage['child_user_seconds'] for row in results for stage in row['stages']),
                child_system_seconds=sum(stage['child_system_seconds'] for row in results for stage in row['stages']),
                before=before, after=dict(load_average=os.getloadavg(), unix_seconds=time.time()))


def describe(command):
    binary = shutil.which(command)
    if not binary:
        raise ValueError('Compiler not found: ' + command)
    binary = str(Path(binary).resolve())
    version = subprocess.run([binary, '--version'], check=True, capture_output=True, text=True).stdout
    return dict(path=binary, sha256=sha256(binary), version=version)


def comparison(cppgm, upstream, total):
    comparable = all(row['executed'] == total and row['passed'] == total
                     for row in [cppgm, upstream])
    return dict(comparable=comparable,
                cppgm_over_upstream=cppgm['elapsed_seconds'] / upstream['elapsed_seconds'],
                interpretation='Passing identical suite' if comparable else
                'Non-comparable: failed or incomplete work; raw elapsed times retained')


def markdown(report):
    lines = ['### ' + report['suite'].upper() + ' C++11 language checks', '',
             '| Compiler | Passed / selected | Suite elapsed (s) |',
             '|---|---:|---:|']
    for role in ['cppgm', 'upstream']:
        row = report[role]
        lines.append(f"| {role} | {row['passed']} / {row['selected']} | {row['elapsed_seconds']:.3f} |")
    lines += ['', f"cppgm / upstream elapsed ratio: {report['comparison']['cppgm_over_upstream']:.3f} "
              '(lower is faster). ' + report['comparison']['interpretation'] + '.', '',
              report['coverage'], '',
              f"{report['excluded']} source/RUN exclusions are recorded in manifest.json; they are not passes.",
              f"Workers: {report['workers']}; order: cppgm, upstream; fresh output files, OS caches unchanged.",
              'Setup and builds excluded; compilation, required linking/execution and result checks included.', '']
    return '\n'.join(lines)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--manifest', type=Path, required=True)
    parser.add_argument('--source-root', type=Path, required=True)
    parser.add_argument('--candidate', required=True)
    parser.add_argument('--upstream', required=True)
    parser.add_argument('--upstream-version', required=True)
    parser.add_argument('--workers', type=int, default=2)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    if args.workers < 1:
        parser.error('workers must be positive')
    root = args.source_root.resolve()
    manifest = json.loads(args.manifest.read_text())
    validate_manifest(manifest, root)
    binaries = dict(cppgm=describe(args.candidate), upstream=describe(args.upstream))
    if args.upstream_version not in binaries['upstream']['version']:
        raise ValueError('Upstream compiler does not match pinned suite version')
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    write_json(output / 'manifest.json', manifest)
    script_directory = Path(__file__).resolve().parent
    scripts = {name: sha256(script_directory / name) for name in
               ['run_cxx11_ci_suite.py', 'upstream_dg_proxy.py', 'upstream_clang_language.py']}
    report = dict(schema=1, suite=manifest['suite'], revision=manifest['revision'],
                  manifest_sha256=sha256(args.manifest), binaries=binaries,
                  runner_scripts=scripts,
                  workers=args.workers, platform=platform.platform(),
                  coverage=manifest['coverage'], limitations=manifest['limitations'],
                  excluded=len(manifest['exclusions']),
                  environment={name: os.environ.get(name) for name in
                               ['CPPGM_HOST_CXX', 'CPPGM_STDLIB_FLAGS', 'CPATH',
                                'CPLUS_INCLUDE_PATH', 'LIBRARY_PATH', 'LD_LIBRARY_PATH']})
    for role in ['cppgm', 'upstream']:
        report[role] = run_suite(manifest['cases'], root, binaries[role]['path'],
                                 binaries['upstream']['path'], role, output / role, args.workers)
        write_json(output / 'results.json', report)
    try:
        validate_manifest(manifest, root)
    except (ValueError, OSError) as error:
        report['input_verification_error'] = str(error)
    report['comparison'] = comparison(report['cppgm'], report['upstream'], len(manifest['cases']))
    if report.get('input_verification_error'):
        report['comparison'].update(comparable=False, interpretation='Non-comparable: corpus changed during execution')
    if (any(sha256(row['path']) != row['sha256'] for row in binaries.values()) or
            any(sha256(script_directory / name) != digest for name, digest in scripts.items())):
        report['comparison'].update(comparable=False, interpretation='Non-comparable: compiler/runner changed during execution')
    write_json(output / 'results.json', report)
    summary = markdown(report)
    (output / 'summary.md').write_text(summary)
    print(summary)
    return 0 if report['comparison']['comparable'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
