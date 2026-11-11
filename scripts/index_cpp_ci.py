#!/usr/bin/env python3
"""Build, measure and publish the pinned reference-only index-cpp comparison."""
import argparse
import benchmark_hosts
import json
import os
from pathlib import Path
import re
import shlex
import shutil
import statistics
import subprocess
from types import SimpleNamespace

import benchmark_timing as timing
import index_cpp_benchmarks as bench
import public_benchmarks

ROOT = Path(__file__).resolve().parents[1]
UPSTREAM = 'https://github.com/kostya/index-cpp.git'
LABELS = ('cppgm', 'gcc', 'clang')


def save(path, value):
    path.write_text(json.dumps(value, indent=2) + '\n')


def verify_source(source):
    revision = subprocess.check_output(['git', '-C', str(source), 'rev-parse', 'HEAD'], text=True).strip()
    if revision != bench.UPSTREAM_COMMIT or timing.sha(source / 'index.cpp') != bench.SOURCE_SHA256:
        raise ValueError('index-cpp source differs from the pinned upstream')


def fetch(source):
    source.mkdir(parents=True)
    for command in [('init', '-q'), ('remote', 'add', 'origin', UPSTREAM),
                    ('fetch', '-q', '--depth', '1', 'origin', bench.UPSTREAM_COMMIT),
                    ('checkout', '-q', '--detach', 'FETCH_HEAD')]:
        subprocess.run(['git', '-C', str(source), *command], check=True)
    verify_source(source)


def headers(source, compilers):
    inventory = {}
    for label in ('gcc', 'clang'):
        output = subprocess.check_output([str(compilers[label]), '-std=c++17', '-M', str(source / 'index.cpp')], text=True)
        paths = shlex.split(output.split(':', 1)[1].replace('\\\n', ' '))
        inventory[label] = {str(Path(path).resolve()): timing.sha(path) for path in sorted(set(paths))}
    return inventory


def counter_support(directory):
    path = directory / 'counter-probe'
    if shutil.which('perf') is None:
        return False, 'perf is not installed on this host'
    result = subprocess.run(['perf', 'stat', '-x', ';', '-o', str(path), '-e', ','.join(bench.COUNTERS), '--', 'true'],
                            capture_output=True, text=True)
    try:
        if result.returncode:
            raise ValueError(result.stderr.strip() or path.read_text().strip())
        bench.parse_counters(path)
    except ValueError as error:
        return False, str(error)
    return True, None


def compile_inputs(source, directory, compilers, cpus, blocks, aa_blocks, counters, metadata):
    observations = directory / 'compile-observations.jsonl'
    serial, expected = 0, {}
    objects = {label: directory / (label + '.o') for label in LABELS}

    def one(label, phase, block, position):
        nonlocal serial
        if metadata['compiler_hashes'] != {key: timing.sha(value) for key, value in compilers.items()}:
            raise ValueError('a compiler changed during measurement')
        verify_source(source)
        command = [str(compilers[label]), '-std=c++17', '-O3', '-g0', '-c', str(source / 'index.cpp'), '-o', str(objects[label])]
        row = bench.run(command, directory / f'compile-{serial:03}-{label}', cpus, 900, counters=counters)
        serial += 1
        row.update(label=label, phase=phase, block=block, position=position)
        if row['status'] == 0:
            row['object_sha256'] = timing.sha(objects[label])
        timing.retain(observations, row)
        if row['status'] != 0 or row.get('counter_error'):
            raise ValueError('index-cpp compilation failed: ' + label)
        if expected.setdefault(label, row['object_sha256']) != row['object_sha256']:
            raise ValueError('repeated compilation changed its output: ' + label)
        return row

    comparison = {}
    for reference in ('gcc', 'clang'):
        calibration = timing.observations(one, reference, reference, aa_blocks, phase='AA-' + reference)
        paired = timing.observations(one, reference, 'cppgm', blocks, phase='AB-' + reference)
        metrics = ('cpu', 'wall', 'max_rss_kib') + (('instructions',) if counters else ())
        comparison[reference] = {name: bench.paired_speed(paired, lambda row: row[name]) for name in metrics}
        comparison[reference]['calibration'] = {name: bench.paired_speed(calibration, lambda row: row[name]) for name in metrics}
        save(directory / 'compile-comparison.json', comparison)
    if headers(source, compilers) != metadata['headers']:
        raise ValueError('the source/header epoch changed during compilation')
    executables, sizes = {}, {}
    for label, obj in objects.items():
        exe = directory / ('index-' + label)
        command = [str(compilers['gcc']), str(obj), '-pthread', '-o', str(exe)]
        with (directory / ('link-' + label + '.log')).open('w') as log:
            subprocess.run(command, stdout=log, stderr=subprocess.STDOUT, check=True)
        text = subprocess.check_output(['size', '-A', str(exe)], text=True)
        match = re.search(r'^\.text\s+(\d+)\s', text, re.M)
        if match is None:
            raise ValueError('could not measure executable .text')
        sizes[label] = dict(object_bytes=obj.stat().st_size, executable_bytes=exe.stat().st_size,
                            text_bytes=int(match[1]), object_sha256=timing.sha(obj), executable_sha256=timing.sha(exe),
                            link_command=command)
        executables[label] = exe
    return executables, sizes, comparison


def compiler_path(spelling):
    # Driver names select C++ mode and may select an installed configuration.
    # Hash the target, but preserve the invoked symlink's name and directory.
    path = Path(os.path.abspath(shutil.which(spelling) or spelling))
    if not path.is_file():
        raise ValueError('missing compiler: ' + spelling)
    return path


def run(args):
    directory = args.output.resolve()
    directory.mkdir(parents=True, exist_ok=False)
    source = args.source.resolve() if args.source else directory / 'source'
    document = dict(status='running', source_commit=bench.UPSTREAM_COMMIT, source_sha256=bench.SOURCE_SHA256)
    save(directory / 'results.json', document)
    try:
        if args.source is None:
            fetch(source)
        verify_source(source)
        cppgm = directory / 'compiler'
        shutil.copy2(args.compiler.resolve(strict=True), cppgm)
        cppgm.chmod(0o555)
        compilers = dict(cppgm=cppgm, gcc=compiler_path(args.gcc), clang=compiler_path(args.clang))
        cpus = args.cpus or ','.join(map(str, sorted(os.sched_getaffinity(0))))
        counters, reason = counter_support(directory)
        metadata = dict(**benchmark_hosts.source_metadata(getattr(args, 'repo_root', ROOT)),
                        cpu=public_benchmarks.cpu_model(), cpus=cpus,
                        toolchains={label: subprocess.check_output([str(path), '--version'], text=True).strip()
                                    for label, path in compilers.items()},
                        compiler_hashes={label: timing.sha(path) for label, path in compilers.items()},
                        counters_available=counters, counter_unavailable_reason=reason,
                        flags=['-std=c++17', '-O3', '-g0'], headers=headers(source, compilers))
        save(directory / 'metadata.json', metadata)
        binaries, sizes, compilation = compile_inputs(source, directory, compilers, cpus, args.blocks,
                                                       args.aa_blocks, counters, metadata)
        bench.measure(SimpleNamespace(source_root=source, out=directory / 'runtime',
                                      binary=[label + '=' + str(path) for label, path in binaries.items()],
                                      candidate='cppgm', reference=['gcc', 'clang'], cpus=cpus, blocks=args.blocks,
                                      aa_blocks=args.aa_blocks, timeout=900, no_counters=not counters,
                                      only=None, drift_warning=.02, metadata=directory / 'metadata.json'))
        runtime = json.loads((directory / 'runtime/report.json').read_text())
        document.update(status='complete', metadata=metadata, sizes=sizes, compilation=compilation, runtime=runtime)
    except BaseException as error:
        document.update(status='failed', error=repr(error))
        raise
    finally:
        save(directory / 'results.json', document)
        (directory / 'details.md').write_text(report(document, directory))


def number(value, suffix=''):
    return '—' if value is None else f'{value:.3f}{suffix}'


def validate(document):
    if (document.get('status') != 'complete' or document['runtime']['status'] != 'complete'
            or document['source_commit'] != bench.UPSTREAM_COMMIT or document['source_sha256'] != bench.SOURCE_SHA256
            or set(document['runtime']['executables']) != set(LABELS) or len(document['runtime']['workloads']) != 50):
        raise ValueError('incomplete or unpinned index-cpp results')
    names = set(document['runtime']['workloads'])
    checks = document['runtime']['correctness']
    if set(checks) != set(LABELS) or any(check['status'] != 0 or set(check['workloads']) != names
                                       or len(check['workloads']) != 50 for check in checks.values()):
        raise ValueError('missing successful all-50 checksum evidence')
    for reference in ('gcc', 'clang'):
        if set(document['runtime']['pairs'][reference + ':all']['loops']) != names:
            raise ValueError('a comparison omits workloads')


def report(document, directory):
    title = '# index-cpp benchmarks\n\n'
    if document['status'] != 'complete':
        return title + '**Incomplete run:** ' + document.get('error', 'still running') + '\n'
    validate(document)
    metadata, runtime = document['metadata'], document['runtime']
    modified = ' (modified working tree)' if metadata['dirty'] else ''
    text = (title + f"Compiler checkout: `{metadata['revision']}`{modified} · CPU: {metadata['cpu']}\n\n"
            f"Unchanged upstream: `{bench.UPSTREAM_COMMIT}`. All 50 checksums pass with cppgm, GCC and Clang. "
            'Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. '
            'Speeds are reference time / cppgm time: higher is better. '
            'The aggregate gives all 50 workloads equal weight and is separate from the public suite.\n\n'
            '| Measurement | vs GCC | vs Clang |\n|---|---:|---:|\n')
    for caption, values in [('Generated code', {ref: runtime['pairs'][ref + ':all']['geometric_speed'] for ref in ('gcc', 'clang')}),
                            ('Compile CPU', {ref: document['compilation'][ref]['cpu']['speed'] for ref in ('gcc', 'clang')})]:
        text += f"| {caption} | {number(values['gcc'], '×')} | {number(values['clang'], '×')} |\n"
    if metadata['counters_available']:
        for caption, values in [('Compile instructions', {ref: document['compilation'][ref]['instructions']['speed'] for ref in ('gcc', 'clang')}),
                                ('Process instructions, including setup', {ref: runtime['pairs'][ref + ':all']['process_instructions']['speed'] for ref in ('gcc', 'clang')})]:
            text += f"| {caption} | {number(values['gcc'], '×')} | {number(values['clang'], '×')} |\n"
    text += '\n| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |\n|---|---:|---:|---:|\n'
    compile_rows = [json.loads(line) for line in (directory / 'compile-observations.jsonl').read_text().splitlines()]
    for label in LABELS:
        rss = statistics.median(row['max_rss_kib'] for row in compile_rows if row['label'] == label)
        text += f"| {label} | {document['sizes'][label]['executable_bytes']:,} | {document['sizes'][label]['text_bytes']:,} | {rss:,.0f} |\n"
    text += '\n| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |\n|---|---:|---:|---:|---:|---:|---|\n'
    rows = [json.loads(line) for line in (directory / 'runtime/observations.jsonl').read_text().splitlines()]
    for name in runtime['workloads']:
        times = {label: statistics.median(row['cases'][name] for row in rows
                                         if row['phase'] == 'AB' and row['label'] == label) for label in LABELS}
        warning = ', '.join(ref for ref in ('gcc', 'clang') if name in runtime['timing_warnings'].get(ref + ':all', [])) or '—'
        speeds = {ref: runtime['pairs'][ref + ':all']['loops'][name]['speed'] for ref in ('gcc', 'clang')}
        text += (f"| {name} | {number(times['cppgm'])} | {number(times['gcc'])} | {number(times['clang'])} | "
                 f"{number(speeds['gcc'], '×')} | {number(speeds['clang'], '×')} | {warning} |\n")
    text += '\nA/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.\n'
    for ref in ('gcc', 'clang'):
        value = document['compilation'][ref]['calibration']['cpu']['speed']
        text += f"Compile {ref} A/A CPU ratio: {number(value, '×')}.\n"
    text += '\nHardware counters: ' + ('available' if metadata['counters_available'] else 'unavailable: ' + metadata['counter_unavailable_reason']) + '.\n'
    text += '\n## Toolchains\n\n' + '\n'.join(f"- **{key}**: {value.splitlines()[0]}" for key, value in metadata['toolchains'].items()) + '\n'
    text += '\n[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](compile-observations.jsonl) · [Raw runtime observations](runtime-observations.jsonl)\n'
    return text


def publish(artifact, target, revision, run_url, platform='github'):
    if not re.fullmatch('[0-9a-f]{40}', revision):
        raise ValueError('expected a full measured commit hash')
    document = json.loads((artifact / 'results.json').read_text())
    validate(document)
    if document['metadata']['revision'] != revision or document['metadata']['dirty']:
        raise ValueError('results do not describe the clean measured revision')
    files = {'results.json': artifact / 'results.json', 'metadata.json': artifact / 'metadata.json'}
    # Read and validate everything before replacing the previous successful run.
    payloads = {name: path.read_bytes() for name, path in files.items()}
    if json.loads(payloads['metadata.json']) != document['metadata']:
        raise ValueError('provenance differs from the measured results')
    details = report(document, artifact) + '\n[CI run and complete artifacts](' + run_url + ')\n'
    platform = benchmark_hosts.publication_platform(document['metadata']['cpu'], platform)
    destinations = [target / 'index-cpp' / platform,
                    target / 'index-cpp/history' / revision / platform]
    for destination in destinations:
        destination.mkdir(parents=True, exist_ok=True)
        for name, data in payloads.items():
            (destination / name).write_bytes(data)
        (destination / 'report.md').write_text(details)
    benchmark_hosts.library_index(target / 'index-cpp', 'index-cpp benchmark hosts')
    from library_benchmark_badges import library_badges
    library_badges(target / 'index-cpp', 'index-cpp')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest='command', required=True)
    runner = commands.add_parser('run')
    runner.add_argument('--compiler', type=Path, default=ROOT / 'dev/cppgm++')
    runner.add_argument('--repo-root', type=Path, default=ROOT, help='clean source checkout used to build the compiler')
    runner.add_argument('--source', type=Path, help='existing pinned checkout, otherwise fetch upstream')
    runner.add_argument('--output', type=Path, required=True)
    runner.add_argument('--gcc', default='g++')
    runner.add_argument('--clang', default='clang++')
    runner.add_argument('--cpus')
    runner.add_argument('--blocks', type=int, default=3)
    runner.add_argument('--aa-blocks', type=int, default=1)
    publisher = commands.add_parser('publish')
    publisher.add_argument('artifact', type=Path)
    publisher.add_argument('target', type=Path)
    publisher.add_argument('--revision', required=True)
    publisher.add_argument('--run-url', required=True)
    publisher.add_argument('--platform', choices=['github', 'intel'], default='github')
    args = parser.parse_args()
    if args.command == 'run':
        if args.blocks < 1 or args.aa_blocks < 1:
            parser.error('block counts must be positive')
        run(args)
    else:
        publish(args.artifact, args.target, args.revision, args.run_url, args.platform)


if __name__ == '__main__':
    main()
